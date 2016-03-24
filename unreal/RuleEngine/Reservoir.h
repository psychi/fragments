// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TReservoir
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include "Containers/Map.h"
#include "./StatusChunk.h"
#include "./StatusProperty.h"
#include "./StatusValue.h"
#include "./StatusOperation.h"

/// @cond
namespace Psyque
{
	namespace RuleEngine
	{
		namespace _private
		{
			template<typename, typename, typename, typename, typename>
				class TReservoir;
		} // namespace _private
	} // namespace RuleEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態貯蔵器。任意のビット長の状態値を保持する。
/// @par 使い方の概略
/// - TReservoir::register_status で、状態値を登録する。
/// - TReservoir::FindStatus で、状態値を取得する。
/// - TReservoir::assign_status で、状態値に代入する。
/// @tparam TemplateUnsigned  @copydoc TReservoir::StatusValue::UnsignedType
/// @tparam TemplateFloat     @copydoc TReservoir::StatusValue::FloatType
/// @tparam TemplateStatusKey @copydoc TReservoir::StatusKey
/// @tparam TemplateChunkKey  @copydoc TReservoir::ChunkKey
/// @tparam TemplateAllocator @copydoc TReservoir::Allocator
template<
	typename TemplateUnsigned,
	typename TemplateFloat,
	typename TemplateStatusKey,
	typename TemplateChunkKey,
	typename TemplateAllocator>
class Psyque::RuleEngine::_private::TReservoir
{
	/// @brief this が指す値の型。
	using This = TReservoir;

	//-------------------------------------------------------------------------
	public:
	/// @brief 状態値を識別するための値を表す型。
	using StatusKey = TemplateStatusKey;
	/// @brief チャンクを識別するための値を表す型。
	using ChunkKey = TemplateChunkKey;
	/// @copydoc TStatusChunk::Allocator
	using Allocator = TemplateAllocator;
	/// @copybrief TStatusValue
	using StatusValue = Psyque::RuleEngine::_private::TStatusValue<
		TemplateUnsigned, TemplateFloat>;
	/// @copybrief TStatusProperty
	using StatusProperty = Psyque::RuleEngine::_private::TStatusProperty<
		 typename This::ChunkKey, uint32, int8>;
	/// @brief 状態値の比較演算の引数。
	using StatusComparison = Psyque::RuleEngine::_private::TStatusOperation<
		typename This::StatusKey,
		typename This::StatusValue::EComparison,
		typename This::StatusValue>;
	/// @brief 状態値の代入演算の引数。
	using StatusAssignment = Psyque::RuleEngine::_private::TStatusOperation<
		typename This::StatusKey,
		typename This::StatusValue::EAssignment,
		typename This::StatusValue>;

	//-------------------------------------------------------------------------
	private:
	/// @copybrief TStatusChunk
	using StatusChunk = Psyque::RuleEngine::_private::TStatusChunk<
		TemplateUnsigned,
		typename This::StatusProperty::BitPositionType,
		typename std::make_unsigned<
			typename This::StatusProperty::FormatType>::type,
		typename TemplateAllocator::SparseArrayAllocator::ElementAllocator>;
	/// @brief 状態値ビット列チャンクの辞書。
	using ChunkMap = TMap<
		typename This::ChunkKey,
		typename This::StatusChunk,
		TemplateAllocator>;
	/// @brief 状態値プロパティの辞書。
	using PropertyMap = TMap<
		typename This::StatusKey,
		typename This::StatusProperty,
		TemplateAllocator>;
	/// @brief 浮動小数点数とビット列を変換する。
	using FloatBitset = Psyque::TFloatBitset<TemplateFloat>;
	/// @brief ビット列とビット幅のペア。
	using StatusBitset = std::pair<
		 typename This::StatusChunk::BitBlock,
		 typename This::StatusChunk::BitWidth>;

	//-------------------------------------------------------------------------
	/// @name 構築と代入
	/// @{
	public:
	/// @brief 空の状態貯蔵器を構築する。
	TReservoir(
		/// [in] チャンクの予約数。
		int32 const InChunkCount,
		/// [in] 状態値の予約数。
		int32 const InStatusCount)
	{
		this->Chunks.Reserve(InChunkCount);
		this->Properties.Reserve(InStatusCount);
	}

	/// @brief 状態貯蔵器を再構築する。
	void Rebuild(
		/// [in] チャンクの予約数。
		int32 const InChunkCount,
		/// [in] 状態値の予約数。
		int32 const InStatusCount)
	{
		// 新たな辞書を用意する。
		typename This::ChunkMap LocalChunks;
		LocalChunks.Reserve(InChunkCount);
		typename This::PropertyMap LocalProperties;
		LocalProperties.Reserve(InChunkCount);

		// 現在の辞書を新たな辞書にコピーして整理する。
		This::copy_bit_fields(
			LocalProperties, LocalChunks, this->Properties, this->Chunks);
		for (auto i(LocalChunks.CreateIterator()); i; ++i)
		{
			auto& LocalChunk(i.Value());
			if (0 < LocalChunk.BitBlocks.Num())
			{
				LocalChunk.BitBlocks.Shrink();
				LocalChunk.EmptyBitsets.Shrink();
			}
			else
			{
				/// @note コンテナ要素を削除しても、反復子は有効らしい。
				/// https://docs.unrealengine.com/latest/JPN/Programming/Introduction/index.html
				i.RemoveCurrent();
			}
		}
		this->Properties = MoveTemp(LocalProperties);
		this->Chunks = MoveTemp(LocalChunks);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値の登録
	/// @{
	public:
	/// @brief 状態値が登録されているか判定する。
	/// @retval true  InStatusKey に対応する状態値が *this に登録されている。
	/// @retval false InStatusKey に対応する状態値は *this に登録されてない。
	bool IsRegisterd(
		/// [in] 判定する状態値に対応する識別値。
		typename This::StatusKey const InStatusKey)
	const
	{
		return this->Properties.Find(InStatusKey) != nullptr;
	}

	/// @brief 状態値を登録する。
	/// @sa
	/// - This::FindStatus と
	///   This::assign_status で、登録した状態値にアクセスできる。
	/// - This::erase_chunk で、登録した状態値をチャンク毎に削除できる。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	public: template<typename TemplateValue>
	bool register_status(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		typename This::ChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別値。
		typename This::StatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値。以下の型の値を登録できる。
		/// - bool 型。
		/// - C++ 組み込み整数型。
		/// - C++ 組み込み浮動小数点数型。
		TemplateValue const InValue)
	{
		if (std::is_floating_point<TemplateValue>::value)
		{
			// 浮動小数点数型の状態値を登録する。
			/// @note コンパイル時にここで警告かエラーが発生する場合は、
			/// TemplateValue が double 型で
			/// TemplateFloat が float 型なのが原因。
			TemplateFloat const LocalFloat(InValue);
			return nullptr != this->register_bit_field(
				InChunkKey,
				InStatusKey,
				typename This::FloatBitset(LocalFloat).Bitset,
				This::StatusValue::EKind::FLOAT);
		}
		else if (std::is_same<bool, TemplateValue>::value)
		{
			// 論理型の状態値を登録する。
			return nullptr != this->register_bit_field(
				InChunkKey,
				InStatusKey,
				InValue != 0,
				This::StatusValue::EKind::BOOL);
		}
		else
		{
			// 整数型の状態値を登録する。
			return this->register_status(
				InChunkKey,
				InStatusKey,
				InValue,
				sizeof(TemplateValue) * CHAR_BIT);
		}
	}

	/// @brief 整数型の状態値を登録する。
	/// @sa
	/// - This::FindStatus と This::assign_status
	///   で、登録した状態値にアクセスできる。
	/// - This::erase_chunk で、登録した状態値をチャンク毎に削除できる。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	/// - InValue のビット幅が InBitWidth を超えていると失敗する。
	/// - This::StatusChunk::BLOCK_BIT_WIDTH より
	///   InBitWidth が大きいと失敗する。
	/// - InBitWidth が2未満だと失敗する。
	///   1ビットの値は論理型として登録すること。
	public: template<typename TemplateValue>
	bool register_status(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		typename This::ChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別番号。
		typename This::StatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値。 C++ 組み込み整数型であること。
		TemplateValue const InValue,
		/// [in] 登録する状態値のビット幅。
		SIZE_T const InBitWidth)
	{
		using FormatType = typename This::StatusProperty::FormatType;
		using BitBlock = typename This::StatusChunk::BitBlock;
		if(!std::is_integral<TemplateValue>::value
			|| std::is_same<bool, TemplateValue>::value
			|| This::StatusChunk::BLOCK_BIT_WIDTH < InBitWidth
			|| InBitWidth < 2)
		{
			// 適切な整数型ではないので、登録に失敗する。
		}
		else if (std::is_signed<TemplateValue>::value)
		{
			// 符号あり整数型の状態値を登録する。
			using SignedType = typename This::StatusValue::SignedType;
			auto const LocalValue(static_cast<SignedType>(InValue));
			return !This::IsOverflow(LocalValue, InBitWidth)
				&& nullptr != this->register_bit_field(
					InChunkKey,
					InStatusKey,
					Psyque::MakeBitMask<BitBlock>(InBitWidth) & LocalValue,
					-static_cast<FormatType>(InBitWidth));
		}
		else //if (std::is_unsigned<TemplateValue>::value)
		{
			// 符号なし整数型の状態値を登録する。
			auto const LocalValue(static_cast<BitBlock>(InValue));
			return !This::IsOverflow(LocalValue, InBitWidth)
				&& nullptr != this->register_bit_field(
					InChunkKey,
					InStatusKey,
					LocalValue,
					static_cast<FormatType>(InBitWidth));
		}
		return false;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値の取得
	/// @{
	public:
	/// @brief 状態値の数を取得する。
	int32 GetStatusCount() const
	{
		return this->Properties.Num();
	}

	/// @brief チャンクの数を取得する。
	int32 GetChunkCount() const
	{
		return this->Chunks.Num();
	}

	/// @brief 状態値のプロパティを取得する。
	/// @return InStatusKey に対応する状態値のプロパティのコピー。
	/// 該当する状態値がない場合は
	/// This::StatusProperty::IsEmpty が真となる値を返す。
	typename This::StatusProperty FindProperty(
		/// [in] 取得する状態値プロパティに対応する識別値。
		typename This::StatusKey const InStatusKey)
	const
	{
		auto const LocalProperty(this->Properties.Find(InStatusKey));
		return LocalProperty != nullptr?
			*LocalProperty:
			typename This::StatusProperty(typename This::ChunkKey(), 0, 0);
	}

	/// @brief 状態値の型の種別を取得する。
	/// @return InStatusKey に対応する状態値の型の種別。
	/// 該当する状態値がない場合は This::StatusValue::EKind::EMPTY を返す。
	typename This::StatusValue::EKind FindKind(
		/// [in] 状態値に対応する識別値。
		typename This::StatusKey const InStatusKey)
	const
	{
		auto const LocalProperty(this->Properties.Find(InStatusKey));
		return LocalProperty != nullptr?
			This::GetKind(LocalProperty->GetFormat()):
			This::StatusValue::EKind::EMPTY;
	}

	/// @brief 状態値のビット幅を取得する。
	/// @return InStatusKey に対応する状態値のビット幅。
	/// 該当する状態値がない場合は0を返す。
	typename This::StatusChunk::BitWidth FindBitWidth(
		/// [in] 状態値に対応する識別値。
		typename This::StatusKey const InStatusKey)
	const
	{
		auto const LocalProperty(this->Properties.Find(InStatusKey));
		return LocalProperty != nullptr?
			This::GetBitWidth(LocalProperty->GetFormat()): 0;
	}

	/// @brief 状態変化フラグを取得する。
	/// @retval 正 状態変化フラグは真。
	/// @retval 0  状態変化フラグは偽。
	/// @retval 負 InStatusKey に対応する状態値がない。
	int8 FindTransition(
		/// [in] 状態変化フラグを取得する状態値に対応する識別値。
		typename This::StatusKey const InStatusKey)
	const
	{
		auto const LocalProperty(this->Properties.Find(InStatusKey));
		return LocalProperty != nullptr? LocalProperty->HasTransited(): -1;
	}

	/// @brief 状態値を取得する。
	/// @return 取得した状態値。 InStatusKey に対応する状態値がない場合は、
	/// This::StatusValue::IsEmpty が真となる値を返す。
	/// @sa
	/// - This::register_status で、状態値を登録できる。
	/// - This::assign_status で、状態値を書き換えできる。
	typename This::StatusValue FindStatus(
		/// [in] 取得する状態値に対応する識別値。
		typename This::StatusKey const InStatusKey)
	const
	{
		// 状態値プロパティを取得する、
		auto const LocalProperty(this->Properties.Find(InStatusKey));
		if (LocalProperty != nullptr)
		{
			// 状態値ビット列チャンクから状態値のビット列を取得する。
			auto const LocalChunk(
				this->Chunks.Find(LocalProperty->GetChunkKey()));
			if (LocalChunk != nullptr)
			{
				auto const LocalFormat(LocalProperty->GetFormat());
				check(LocalFormat != 0);
				auto const LocalBitWidth(This::GetBitWidth(LocalFormat));
				auto const LocalBitset(
					LocalChunk->GetBitset(
						LocalProperty->GetBitPosition(), LocalBitWidth));

				// 状態値のビット構成から、構築する状態値の型を分ける。
				if (LocalFormat == This::StatusValue::EKind::FLOAT)
				{
					// 浮動小数点数型の状態値を構築する。
					typename This::FloatBitset const LocalValue(
						static_cast<typename This::FloatBitset::BitsetType>(
							LocalBitset));
					return typename This::StatusValue(LocalValue.Float);
				}
				else if (LocalFormat == This::StatusValue::EKind::BOOL)
				{
					// 論理型の状態値を構築する。
					return typename This::StatusValue(LocalBitset != 0);
				}
				else if (0 < LocalFormat)
				{
					// 符号なし整数型の状態値を構築する。
					return typename This::StatusValue(LocalBitset);
				}
				else if (LocalFormat < 0)
				{
					// ビット幅より上位のビットを符号ビットで埋め、
					// 符号あり整数型の状態値を構築する。
					using SignedType = typename This::StatusValue::SignedType;
					auto const LocalRestBitWidth(
						This::StatusChunk::BLOCK_BIT_WIDTH - LocalBitWidth);
					return typename This::StatusValue(
						Psyque::ShiftRightBitwiseFast(
							Psyque::ShiftLeftBitwiseFast(
								static_cast<SignedType>(LocalBitset),
								LocalRestBitWidth),
							LocalRestBitWidth));
				}
			}
			// 状態値プロパティがあれば、
			// 対応する状態値ビット列チャンクもあるはず。
			else {check(false);}
		}
		return typename This::StatusValue();
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値の比較
	/// @{

	/// @brief 状態値を比較する。
	/// @retval 正 比較式の評価は真。
	/// @retval 0  比較式の評価は偽。
	/// @retval 負 比較式の評価に失敗。
	public: typename This::StatusValue::Evaluation compare_status(
		/// [in] 状態値の比較式。
		typename This::StatusComparison const& in_comparison)
	const
	{
		auto const local_right_key_pointer(in_comparison.get_right_key());
		if (local_right_key_pointer == nullptr)
		{
			return this->compare_status(
				in_comparison.GetKey(),
				in_comparison.GetOperator(),
				in_comparison.GetValue());
		}

		// 右辺となる状態値を取得して式を評価する。
		auto const local_right_key(
			static_cast<typename This::StatusKey>(
				*local_right_key_pointer));
		if (local_right_key == *local_right_key_pointer)
		{
			return this->compare_status(
				in_comparison.GetKey(),
				in_comparison.GetOperator(),
				local_right_key);
		}
		return -1;
	}

	/// @brief 状態値を比較する。
	/// @retval 正 比較式の評価は真。
	/// @retval 0  比較式の評価は偽。
	/// @retval 負 比較式の評価に失敗。
	public: typename This::StatusValue::Evaluation compare_status(
		/// [in] 左辺となる状態値の識別値。
		typename This::StatusKey const in_left_key,
		/// [in] 適用する比較演算子。
		typename This::StatusValue::EComparison const in_operator,
		/// [in] 右辺となる値。
		typename This::StatusValue const& in_right_value)
	const
	{
		return this->FindStatus(in_left_key).compare(
			in_operator, in_right_value);
	}

	/// @brief 状態値を比較する。
	/// @retval 正 比較式の評価は真。
	/// @retval 0  比較式の評価は偽。
	/// @retval 負 比較式の評価に失敗。
	public: typename This::StatusValue::Evaluation compare_status(
		/// [in] 左辺となる状態値の識別値。
		typename This::StatusKey const in_left_key,
		/// [in] 適用する比較演算子。
		typename This::StatusValue::EComparison const in_operator,
		/// [in] 右辺となる状態値の識別値。
		typename This::StatusKey const in_right_key)
	const
	{
		return this->FindStatus(in_left_key).compare(
			in_operator, this->FindStatus(in_right_key));
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値の代入
	/// @{
	public:
	/// @brief 状態値へ値を代入する。
	/// @retval true  成功。 InValue を状態値へ代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// - InStatusKey に対応する状態値が
	///   This::register_status で登録されてないと、失敗する。
	/// - InValue の値が
	///   InStatusKey に対応する状態値のビット幅を超えていると、失敗する。
	/// - InValue が論理型以外で、
	///   InStatusKey に対応する状態値が論理型だと、失敗する。
	/// - InValue が論理型で、
	///   InStatusKey に対応する状態値が論理型以外だと、失敗する。
	/// - InValue が負の数で、
	///   InStatusKey に対応する状態値が符号なし整数型だと、失敗する。
	/// - InValue が整数ではない浮動小数点数で、
	///   InStatusKey に対応する状態値が整数型だと、失敗する。
	/// @sa This::FindStatus で、代入した値を取得できる。
	public: template<typename TemplateValue>
	bool assign_status(
		/// [in] 代入先となる状態値に対応する識別値。
		typename This::StatusKey const InStatusKey,
		/// [in] 状態値へ代入する値。以下の型の値を代入できる。
		/// - bool 型。
		/// - C++ 組み込み整数型。
		/// - C++ 組み込み浮動小数点数型。
		/// - This::StatusValue 型。
		TemplateValue const& InValue)
	{
		auto const LocalProperty(this->Properties.Find(InStatusKey));
		if (LocalProperty == nullptr)
		{
			return false;
		}
		/// @note InValue が状態値のビット幅を超えている場合、
		/// 失敗となるように実装しておく。
		/// 失敗とせず、ビット列をマスクして代入する実装も可能。どちらが良い？
		auto const LocalMask(false);
		return This::assign_bit_field(
			*LocalProperty,
			this->Chunks,
			This::MakeBitset(InValue, LocalProperty->GetFormat(), LocalMask));
	}

	/// @brief 状態値を演算し、結果を代入する。
	/// @retval true  成功。演算結果を状態値へ代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// 失敗する要因は This::assign_status を参照。
	public: bool assign_status(
		/// [in] 状態値の代入演算。
		typename This::StatusAssignment const& InAssignment)
	{
		auto const local_right_key_pointer(InAssignment.get_right_key());
		if (local_right_key_pointer == nullptr)
		{
			return this->assign_status(
				InAssignment.GetKey(),
				InAssignment.GetOperator(),
				InAssignment.GetValue());
		}

		// 右辺となる状態値を取得して演算する。
		auto const local_right_key(
			static_cast<typename This::StatusKey>(*local_right_key_pointer));
		return local_right_key == *local_right_key_pointer
			&& this->assign_status(
				InAssignment.GetKey(),
				InAssignment.GetOperator(),
				local_right_key);
	}

	/// @brief 状態値を演算し、結果を代入する。
	/// @retval true  成功。演算結果を状態値へ代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// 失敗する要因は This::assign_status を参照。
	public: bool assign_status(
		/// [in] 代入演算子の左辺となる状態値の識別値。
		typename This::StatusKey const in_left_key,
		/// [in] 適用する代入演算子。
		typename This::StatusValue::EAssignment const in_operator,
		/// [in] 代入演算子の右辺となる値。
		typename This::StatusValue const in_right_value)
	{
		if (in_operator == This::StatusValue::assignment_COPY)
		{
			return this->assign_status(in_left_key, in_right_value);
		}
		auto local_left_value(this->FindStatus(in_left_key));
		return local_left_value.assign(in_operator, in_right_value)
			&& this->assign_status(in_left_key, local_left_value);
	}

	/// @brief 状態値を演算し、結果を代入する。
	/// @retval true  成功。演算結果を状態値へ代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// 失敗する要因は This::assign_status を参照。
	public: bool assign_status(
		/// [in] 代入演算子の左辺となる状態値の識別値。
		typename This::StatusKey const in_left_key,
		/// [in] 適用する代入演算子。
		typename This::StatusValue::EAssignment const in_operator,
		/// [in] 代入演算子の右辺となる状態値の識別値。
		typename This::StatusKey const in_right_key)
	{
		return this->assign_status(
			in_left_key, in_operator, this->FindStatus(in_right_key));
	}

	/// @brief 状態変化フラグを初期化する。
	/// @warning Psyque::RuleEngine 管理者以外は、この関数は使用禁止。
	public: void _reset_transitions()
	{
		for (auto& LocalProperty: this->Properties)
		{
			LocalProperty.second.set_transition(false);
		}
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値ビット列チャンク
	/// @{
	public:
	/// @brief 状態値ビット列チャンクを予約する。
	public: void reserve_chunk(
		/// [in] 予約する状態値ビット列チャンクに対応する識別値。
		typename This::ChunkKey const InChunkKey,
		/// [in] 予約するビット列コンテナの容量。
		SIZE_T const in_reserve_blocks,
		/// [in] 予約する空きビット領域コンテナの容量。
		SIZE_T const in_reserve_empty_fields)
	{
		// 状態値を登録する状態値ビット列チャンクを用意する。
		auto const local_emplace(
			this->Chunks.emplace(
				InChunkKey,
				typename This::ChunkMap::mapped_type(
					this->Chunks.get_allocator())));
		auto& LocalChunk(*local_emplace.first);
		LocalChunk.second.BitBlocks.reserve(in_reserve_blocks);
		LocalChunk.second.EmptyBitsets.reserve(in_reserve_empty_fields);
	}

	/// @brief 状態値ビット列チャンクを削除する。
	/// @retval true  成功。 InChunkKey に対応するチャンクを削除した。
	/// @retval false 失敗。該当するチャンクがない。
	public: bool erase_chunk(
		/// [in] 削除する状態値ビット列チャンクの識別値。
		typename This::ChunkKey const InChunkKey)
	{
		// 状態値ビット列チャンクを削除する。
		if (this->Chunks.erase(InChunkKey) == 0)
		{
			return false;
		}

		// 状態値プロパティを削除する。
		for (auto i(this->Properties.begin()); i != this->Properties.end();)
		{
			if (InChunkKey != i->second.GetChunkKey())
			{
				++i;
			}
			else
			{
				i = this->Properties.erase(i);
			}
		}
		return true;
	}

	/// @brief 状態値ビット列チャンクをシリアル化する。
	/// @return シリアル化した状態値ビット列チャンク。
	/// @todo 未実装。
	typename This::StatusChunk::BitBlockArray SerializeChunk(
		/// [in] シリアル化する状態値ビット列チャンクの識別番号。
		typename This::ChunkKey const InChunkKey)
	const;

	/// @brief シリアル化された状態値ビット列チャンクを復元する。
	/// @todo 未実装。
	bool DeserializeChunk(
		/// [in] 復元する状態値ビット列チャンクの識別値。
		typename This::ChunkKey const InChunkKey,
		/// [in] シリアル化された状態値ビット列チャンク。
		typename This::StatusChunk::BitBlockArray const& InSerializedChunk);
	/// @}
	//-------------------------------------------------------------------------
	public:
	/// @brief 状態値のビット構成から、状態値の型の種別を取得する。
	/// @return 状態値の型の種別。
	static typename This::StatusValue::EKind GetKind(
		/// [in] 状態値のビット構成。
		typename This::StatusProperty::FormatType const InFormat)
	PSYQUE_NOEXCEPT
	{
		switch (InFormat)
		{
			case This::StatusValue::EKind::EMPTY:
			case This::StatusValue::EKind::BOOL:
			case This::StatusValue::EKind::FLOAT:
			return static_cast<typename This::StatusValue::EKind>(InFormat);

			default:
			return InFormat < 0?
				This::StatusValue::EKind::SIGNED:
				This::StatusValue::EKind::UNSIGNED;
		}
	}

	/// @brief 状態値のビット構成から、状態値のビット幅を取得する。
	/// @return 状態値のビット幅。
	static typename This::StatusChunk::BitWidth GetBitWidth(
		/// [in] 状態値のビット構成。
		typename This::StatusProperty::FormatType const InFormat)
	PSYQUE_NOEXCEPT
	{
		switch (InFormat)
		{
			case This::StatusValue::EKind::EMPTY:
			case This::StatusValue::EKind::BOOL:
			static_assert(
				static_cast<unsigned>(This::StatusValue::EKind::EMPTY) == 0
				&& static_cast<unsigned>(This::StatusValue::EKind::BOOL) == 1,
				"");
			return InFormat;

			case This::StatusValue::EKind::FLOAT:
			return sizeof(TemplateFloat) * CHAR_BIT;

			default: return Psyque::AbsInteger(InFormat);
		}
	}

	//-------------------------------------------------------------------------
	private:
	/// @brief 状態値を登録する。
	/// @return
	/// 登録した状態値のプロパティを指すポインタ。
	/// 登録に失敗した場合は nullptr を返す。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	/// @sa
	/// - This::FindStatus と This::assign_status
	///   で、登録した状態値にアクセスできる。
	/// - This::erase_chunk で、登録した状態値をチャンク毎に削除できる。
	private: typename This::StatusProperty const* register_bit_field(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		typename This::ChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別番号。
		typename This::StatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値となるビット列。
		typename This::StatusChunk::BitBlock const& InBitField,
		/// [in] 登録する状態値のビット構成。
		typename This::StatusProperty::FormatType const InFormat)
	{
		// 状態値を登録する状態値ビット列チャンクを用意する。
		auto const& LocalChunk(this->Chunks.FindOrAdd(InChunkKey));

		// 状態値を登録して値を設定する。
		auto const LocalProperty(
			This::AddProperty(
				this->Properties, InChunkKey, LocalChunk, InStatusKey, InFormat));
		if (LocalProperty != nullptr
			&& 0 <= LocalChunk.second.set_bit_field(
				LocalProperty->GetBitPosition(),
				This::GetBitWidth(InFormat),
				InBitField))
		{
			return LocalProperty;
		}
		return nullptr;
	}

	/// @brief 状態値のプロパティを登録する。
	/// @return 登録した状態値のプロパティを指すポインタ。
	/// 登録できなかった場合は nullptr を返す。
	/// - InStatusKey に対応する状態値がすでに追加されていると失敗する。
	static typename This::StatusProperty* AddProperty(
		/// [in,out] 状態値を登録する状態値プロパティの辞書。
		typename This::PropertyMap& OutProperties,
		/// [in] 状態値を登録する状態値ビット列チャンクの識別値。
		typename This::ChunkKey const InChunkKey,
		/// [in,out] 状態値を登録する状態値ビット列チャンク。
		typename This::StatusChunk& OutChunk,
		/// [in] 登録する状態値に対応する識別値。
		typename This::StatusKey const InStatusKey,
		/// [in] 登録する状態値のビット構成。
		typename This::StatusProperty::FormatType const InFormat)
	{
		if (InFormat != static_cast<decltype(InFormat)>(This::StatusValue::EKind::EMPTY)
			&& !OutProperties.Contains(InStatusKey))
		{
			// 状態値のビット領域を生成する。
			auto const LocalBitPosition(
				OutChunk.AddBitset(This::GetBitWidth(InFormat)));
			if (LocalBitPosition != This::StatusChunk::INVALID_BIT_POSITION)
			{
				// 状態値プロパティを生成する。
				return &OutProperties.Emplace(
					InStatusKey,
					typename This::StatusProperty(
						InChunkKey, LocalBitPosition, InFormat));
			}
			else {check(false);}
		}
		else {check(false);}
		return nullptr;
	}

	/// @brief 状態値へ値を代入する。
	/// @retval true  成功。状態値へ値を代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// - 代入する値のビット幅が0だと失敗する。
	/// - 代入する値のビット幅が状態値のビット幅を超えると失敗する。
	/// - 状態値を格納する状態値ビット列チャンクがないと失敗する。
	private: static bool assign_bit_field(
		/// [in,out] 代入先となる状態値のプロパティ。
		typename This::StatusProperty& io_property,
		/// [in,out] 状態値ビット列チャンクのコンテナ。
		typename This::ChunkMap& OutChunks,
		/// [in] 代入する状態値のビット列とビット幅。
		typename This::StatusBitset const& in_BitFieldWidth)
	PSYQUE_NOEXCEPT
	{
		if (0 < in_BitFieldWidth.second)
		{
			// 状態値にビット列を設定する。
			auto const LocalChunk_iterator(
				OutChunks.find(io_property.GetChunkKey()));
			if (LocalChunk_iterator != OutChunks.end())
			{
				auto const local_set_bit_field(
					LocalChunk_iterator->second.set_bit_field(
						io_property.GetBitPosition(),
						in_BitFieldWidth.second,
						in_BitFieldWidth.first));
				if (0 <= local_set_bit_field)
				{
					if (0 < local_set_bit_field)
					{
						// 状態値の変更を記録する。
						io_property.set_transition(true);
					}
					return true;
				}
			}
			else
			{
				// 状態値プロパティがあるなら、状態値ビット列チャンクもあるはず。
				check(false);
			}
		}
		return false;
	}

	/// @brief 状態値をコピーして整理する。
	private: static void copy_bit_fields(
		/// [in,out] コピー先となる状態値プロパティ辞書。
		typename This::PropertyMap& OutProperties,
		/// [in,out] コピー先となる状態値ビット列チャンク辞書。
		typename This::ChunkMap& OutChunks,
		/// [in] コピー元となる状態値プロパティ辞書。
		typename This::PropertyMap const& InProperties,
		/// [in] コピー元となる状態値ビット列チャンク辞書。
		typename This::ChunkMap const& InChunks)
	{
		// 状態値プロパティをビット幅の降順に並び替える。
		using PropertyArray = TArray<
			std::pair<
				typename This::StatusChunk::BitWidth,
				This::PropertyMap::TConstIterator>>;
		PropertyArray LocalProperties;
		LocalProperties.Reserve(InProperties.Num());
		for (auto i(InProperties.CreateConstIterator()); i; ++i)
		{
			LocalProperties.Emplace(
				This::GetBitWidth(i.Value().GetFormat()), i);
		}
		LocalProperties.Sort(
			[](
				PropertyArray::ElementType const& InLeft,
				PropertyArray::ElementType const& InRight)
			->bool
			{
				return InRight.first < InLeft.first;
			});

		// 状態値をコピーする。
		for (auto const& LocalProperty: LocalProperties)
		{
			This::CopyBitset(
				OutProperties, OutChunks, LocalProperty.second, InChunks);
		}
	}

	/// @brief 状態値のビット列をコピーする。
	static void CopyBitset(
		///[in,out] コピー先となる状態値プロパティ辞書。
		typename This::PropertyMap& OutProperties,
		/// [in,out] コピー先となる状態値ビット列チャンク辞書。
		typename This::ChunkMap& OutChunks,
		/// [in] コピー元となる状態値プロパティを指す反復子。
		typename This::PropertyMap::TConstIterator const& InProperty,
		/// [in] コピー元となる状態値ビット列チャンク辞書。
		typename This::ChunkMap const& InChunks)
	{
		// コピー元となる状態値ビット列チャンクを取得する。
		auto const LocalSourceChunk(
			InChunks.Find(InProperty.Value().GetChunkKey()));
		if (LocalSourceChunk == nullptr)
		{
			check(false);
			return;
		}

		// コピー先となる状態値を用意する。
		auto& LocalTargetChunk(
			OutChunks.FindOrAdd(InProperty.Value().GetChunkKey()));
		LocalTargetChunk.BitBlocks.Reserve(LocalSourceChunk->BitBlocks.Num());
		LocalTargetChunk.EmptyBitsets.Reserve(
			LocalSourceChunk->EmptyBitsets.Num());
		auto const LocalFormat(InProperty.Value().GetFormat());
		auto const LocalTargetProperty(
			This::AddProperty(
				OutProperties,
				InProperty.Value().GetChunkKey(),
				LocalTargetChunk,
				InProperty.Key(),
				LocalFormat));
		if (LocalTargetProperty != nullptr)
		{
			// 状態値のビット領域をコピーする。
			auto const LocalBitWidth(This::GetBitWidth(LocalFormat));
			LocalTargetChunk.SetBitset(
				LocalTargetProperty->GetBitPosition(),
				LocalBitWidth,
				LocalSourceChunk->GetBitset(
					InProperty.Value().GetBitPosition(), LocalBitWidth));
			LocalTargetProperty->Transit(InProperty.Value().HasTransited());
		}
		else {check(false);}
	}

	//-------------------------------------------------------------------------
	/// @name ビット操作
	/// @{
	private:
	/// @brief 数値から This::StatusBitset を構築する。
	/// @return
	/// 値から構築したビット列とビット幅のペア。
	/// 構築に失敗した場合は、ビット幅が0となる。
	static typename This::StatusBitset MakeBitset(
		/// [in] ビット列の元となる数値。
		typename This::StatusValue const& InValue,
		/// [in] 構築するビット列の構成。
		typename This::StatusProperty::FormatType const InFormat,
		/// [in] 指定のビット幅に収まるようマスクするか。
		bool const InMask)
	{
		// 入力値のビット列を取得する。
		auto const LocalKind(This::GetKind(InFormat));
		typename This::StatusChunk::BitBlock LocalBitset;
		if (LocalKind != InValue.GetKind())
		{
			typename This::StatusValue const LocalValue(InValue, LocalKind);
			if (LocalValue.IsEmpty())
			{
				return typename This::StatusBitset(0, 0);
			}
			LocalBitset = LocalValue.GetBitset();
		}
		else
		{
			LocalBitset = InValue.GetBitset();
		}

		// ビット列とビット幅を構築する。
		/// @note 以下の処理は、テンプレート版の This::MakeBitset とまとめたい。
		using BitWidth = typename This::StatusChunk::BitWidth;
		if (InFormat == This::StatusValue::EKind::BOOL)
		{
			return typename This::StatusBitset(LocalBitet, 1);
		}
		else if (InFormat == This::StatusValue::EKind::FLOAT)
		{
			return typename This::StatusBitset(
				LocalBitset,
				static_cast<BitWidth>(sizeof(TemplateFloat) * CHAR_BIT));
		}
		else if (InFormat < 0)
		{
			return This::MakeIntegerBitset<typename This::StatusValue::SignedType>(
				LocalBitset, static_cast<BitWidth>(-InFormat), InMask);
		}
		else if (0 < InFormat)
		{
			return This::MakeIntegerBitset<TemplateUnsigned>(
				LocalBitset, static_cast<BitWidth>(InFormat), InMask);
		}
		check(false);
		return typename This::StatusBitset(0, 0);
	}

	/// @brief 数値から This::StatusBitset を構築する。
	/// @return 値から構築したビット列とビット幅のペア。
	/// 構築に失敗した場合は、ビット幅が0となる。
	template<typename TemplateValue>
	static typename This::StatusBitset MakeBitset(
		/// [in] ビット列の元となる数値。
		TemplateValue const InValue,
		/// [in] 構築するビット列の構成。
		typename This::StatusProperty::FormatType const InFormat,
		/// [in] 指定のビット幅に収まるようマスクするか。
		bool const InMask)
	{
		using BitWidth = typename This::StatusChunk::BitWidth;
		if (InFormat == This::StatusValue::EKind::BOOL)
		{
			// 論理値のビット列を構築する。
			if (std::is_same<TemplateValue, bool>::value)
			{
				return typename This::StatusBitset(InValue != 0, 1);
			}
		}
		else if (InFormat == This::StatusValue::EKind::FLOAT)
		{
			// 浮動小数点数のビット列を構築する。
			typename This::FloatBitset const LocalValue(
				static_cast<TemplateFloat>(InValue));
			return typename This::StatusBitset(
				LocalValue.Bitset,
				static_cast<BitWidth>(sizeof(TemplateFloat) * CHAR_BIT));
		}
		else if (InFormat < 0)
		{
			// 符号あり整数のビット列を構築する。
			return This::MakeIntegerBitset<typename This::StatusValue::SignedType>(
				InValue, static_cast<BitWidth>(-InFormat), InMask);
		}
		else if (0 < InFormat)
		{
			// 符号なし整数のビット列を構築する。
			return This::MakeIntegerBitset<TemplateUnsigned>(
				InValue, static_cast<BitWidth>(InFormat), InMask);
		}
		check(false);
		return typename This::StatusBitset(0, 0);
	}

	/// @brief 数値を整数に変換してから This::StatusBitset を構築する。
	/// @return 数値から構築したビット列とビット幅のペア。
	/// 構築に失敗した場合は、ビット幅が0となる。
	template<typename TemplateInteger, typename TemplateValue>
	static typename This::StatusBitset MakeIntegerBitset(
		/// [in] ビット列の元となる数値。
		TemplateValue const InValue,
		/// [in] 構築するビット列の幅。
		typename This::StatusChunk::BitWidth InBitWidth,
		/// [in] 指定のビット幅に収まるようマスクするか。
		bool const InMask)
	{
		auto LocalInteger(static_cast<TemplateInteger>(InValue));
		if (InValue != static_cast<TemplateValue>(LocalInteger))
		{
			InBitWidth = 0;
		}
		else if (InMask)
		{
			LocalInteger &= Psyque::MakeBitMask<TemplateInteger>(InBitWidth);
		}
		else if (This::IsOverflow(LocalInteger, InBitWidth))
		{
			InBitWidth = 0;
		}
		return typename This::StatusBitset(LocalInteger, InBitWidth);
	}

	/// @brief 論理値を整数に変換して
	/// This::StatusBitset を構築させないためのダミー関数。
	template<typename TemplateInteger>
	static typename This::StatusBitset MakeIntegerBitset(
		bool, typename This::StatusChunk::BitWidth, bool)
	{
		// bool型の値を他の型へ変換できないようにする。
		return typename This::StatusBitset(0, 0);
	}

	/// @brief 符号なし整数がビット幅からあふれているか判定する。
	static bool IsOverflow(
		/// [in] 判定する符号なし整数。
		TemplateUnsigned const InInteger,
		/// [in] 許容するビット幅。
		SIZE_T const InBitWidth)
	PSYQUE_NOEXCEPT
	{
		return Psyque::ShiftRightBitwise(InInteger, InBitWidth) != 0;
	}

	/// @brief 符号あり整数がビット幅からあふれているか判定する。
	static bool IsOverflow(
		/// [in] 判定する符号あり整数。
		typename This::StatusValue::SignedType const InInteger,
		/// [in] 許容するビット幅。
		SIZE_T const InBitWidth)
	PSYQUE_NOEXCEPT
	{
		using BitBlock = typename This::StatusChunk::BitBlock;
		auto const LocalRestField(
			Psyque::ShiftRightBitwiseFast(
				static_cast<BitBlock>(InInteger), InBitWidth - 1));
		auto const LocalRestMask(
			Psyque::ShiftRightBitwiseFast(
				static_cast<BitBlock>(
					Psyque::ShiftRightBitwiseFast(
						InInteger, sizeof(InInteger) * CHAR_BIT - 1)),
				InBitWidth - 1));
		return LocalRestField != LocalRestMask;
	}
	/// @}
	//-------------------------------------------------------------------------
	private:
	typename This::ChunkMap Chunks;        ///< 状態値ビット列チャンクの辞書。
	typename This::PropertyMap Properties; ///< 状態値プロパティの辞書。

}; // class Psyque::RuleEngine::_private::TReservoir

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace PsyqueTest
{
	inline void RuleEngine()
	{
		using Reservoir = Psyque::RuleEngine::_private::TReservoir<
			uint64, float, uint32, uint32, FDefaultSetAllocator>;
		int32 const LocalCapacity(256);
		Reservoir LocalReservoir(LocalCapacity, LocalCapacity);
		LocalReservoir.Rebuild(LocalCapacity, LocalCapacity);
	}
}

// vim: set noexpandtab:
