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
/// - TReservoir::RegisterStatus で、状態値を登録する。
/// - TReservoir::FindStatus で、状態値を取得する。
/// - TReservoir::AssignStatus で、状態値に代入する。
/// @tparam TemplateUnsigned     @copydoc TReservoir::FStatusValue::FUnsigned
/// @tparam TemplateFloat        @copydoc TReservoir::FStatusValue::FFloat
/// @tparam TemplateStatusKey    @copydoc TReservoir::FStatusKey
/// @tparam TemplateChunkKey     @copydoc TReservoir::FChunkKey
/// @tparam TemplateSetAllocator @copydoc TReservoir::FSetAllocator
template<
	typename TemplateUnsigned,
	typename TemplateFloat,
	typename TemplateStatusKey,
	typename TemplateChunkKey,
	typename TemplateSetAllocator>
class Psyque::RuleEngine::_private::TReservoir
{
	/// @brief this が指す値の型。
	using This = TReservoir;

	//-------------------------------------------------------------------------
	public:
	/// @brief 状態値を識別するための値を表す型。
	using FStatusKey = TemplateStatusKey;
	/// @brief チャンクを識別するための値を表す型。
	using FChunkKey = TemplateChunkKey;
	/// @brief TSet に適用する、 TSetAllocator 互換のメモリ割当子の型 。
	using FSetAllocator = TemplateSetAllocator;
	/// @copybrief TStatusValue
	using FStatusValue = Psyque::RuleEngine::_private::TStatusValue<
		TemplateUnsigned, TemplateFloat>;
	/// @copybrief TStatusProperty
	using FStatusProperty = Psyque::RuleEngine::_private::TStatusProperty<
		 TemplateChunkKey, uint32, int8>;
	/// @brief 状態値の比較演算の引数。
	using FStatusComparison = Psyque::RuleEngine::_private::TStatusOperation<
		TemplateStatusKey,
		typename This::FStatusValue::EComparison,
		typename This::FStatusValue>;
	/// @brief 状態値の代入演算の引数。
	using FStatusAssignment = Psyque::RuleEngine::_private::TStatusOperation<
		TemplateStatusKey,
		typename This::FStatusValue::EAssignment,
		typename This::FStatusValue>;

	//-------------------------------------------------------------------------
	private:
	/// @copybrief TStatusChunk
	using FStatusChunk = Psyque::RuleEngine::_private::TStatusChunk<
		TemplateUnsigned,
		typename This::FStatusProperty::FBitPosition,
		typename std::make_unsigned<
			typename This::FStatusProperty::FFormat>::type,
		typename TemplateSetAllocator::SparseArrayAllocator::ElementAllocator>;
	/// @brief 状態値ビット列チャンクの辞書。
	using FChunkMap = TMap<
		TemplateChunkKey, typename This::FStatusChunk, TemplateSetAllocator>;
	/// @brief 状態値プロパティの辞書。
	using FPropertyMap = TMap<
		TemplateStatusKey,
		typename This::FStatusProperty,
		TemplateSetAllocator>;
	/// @brief 浮動小数点数とビット列を変換する。
	using FFloatBitset = Psyque::TFloatBitset<TemplateFloat>;
	/// @brief ビット列とビット幅のペア。
	using FStatusBitset = std::pair<
		 typename This::FStatusChunk::FBitBlock,
		 typename This::FStatusChunk::FBitWidth>;

	//-------------------------------------------------------------------------
	/// @name 構築
	/// @{
	public:
	/// @brief 空の状態貯蔵器を構築する。
	TReservoir(
		/// [in] チャンクの予約容量。
		int32 const InChunkCapacity,
		/// [in] 状態値の予約容量。
		int32 const InStatusCapacity)
	{
		this->Chunks.Reserve(InChunkCapacity);
		this->Properties.Reserve(InStatusCapacity);
	}

	/// @brief 状態貯蔵器を再構築する。
	void Rebuild(
		/// [in] チャンクの予約容量。
		int32 const InChunkCapacity,
		/// [in] 状態値の予約容量。
		int32 const InStatusCapacity)
	{
		// 新たな辞書を用意する。
		typename This::FChunkMap LocalChunks;
		LocalChunks.Reserve(InChunkCapacity);
		typename This::FPropertyMap LocalProperties;
		LocalProperties.Reserve(InStatusCapacity);

		// 現在の辞書を新たな辞書にコピーして整理する。
		This::CopyBitsets(
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
	/// @brief 状態値を登録する。
	/// @sa
	/// - This::FindStatus と
	///   This::AssignStatus で、登録した状態値にアクセスできる。
	/// - This::RemoveChunk で、登録した状態値をチャンク毎に削除できる。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	public: template<typename TemplateValue>
	bool RegisterStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		TemplateChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別値。
		TemplateStatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値。以下の型の値を登録できる。
		/// - bool 型。
		/// - C++ 組み込み整数型。
		/// - C++ 組み込み浮動小数点数型。
		TemplateValue const InValue)
	{
		using FFormat = typename This::FStatusProperty::FFormat;
		if (std::is_floating_point<TemplateValue>::value)
		{
			// 浮動小数点数型の状態値を登録する。
			/// @note コンパイル時にここで警告かエラーが発生する場合は、
			/// TemplateValue が double 型で
			/// TemplateFloat が float 型なのが原因。
			TemplateFloat const LocalFloat(InValue);
			return nullptr != this->RegisterBitset(
				InChunkKey,
				InStatusKey,
				typename This::FFloatBitset(LocalFloat).Bitset,
				static_cast<FFormat>(This::FStatusValue::EKind::FLOAT));
		}
		else if (std::is_same<bool, TemplateValue>::value)
		{
			// 論理型の状態値を登録する。
			return nullptr != this->RegisterBitset(
				InChunkKey,
				InStatusKey,
				InValue != 0,
				static_cast<FFormat>(This::FStatusValue::EKind::BOOL));
		}
		else
		{
			// 整数型の状態値を登録する。
			return this->RegisterStatus(
				InChunkKey,
				InStatusKey,
				InValue,
				sizeof(TemplateValue) * CHAR_BIT);
		}
	}

	/// @brief 整数型の状態値を登録する。
	/// @sa
	/// - This::FindStatus と This::AssignStatus
	///   で、登録した状態値にアクセスできる。
	/// - This::RemoveChunk で、登録した状態値をチャンク毎に削除できる。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	/// - InValue のビット幅が InBitWidth を超えていると失敗する。
	/// - This::FStatusChunk::BLOCK_BIT_WIDTH より
	///   InBitWidth が大きいと失敗する。
	/// - InBitWidth が2未満だと失敗する。
	///   1ビットの値は論理型として登録すること。
	template<typename TemplateValue>
	bool RegisterStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		TemplateChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別番号。
		TemplateStatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値。 C++ 組み込み整数型であること。
		TemplateValue const InValue,
		/// [in] 登録する状態値のビット幅。
		SIZE_T const InBitWidth)
	{
		using FFormat = typename This::FStatusProperty::FFormat;
		using FBitBlock = typename This::FStatusChunk::FBitBlock;
		if(!std::is_integral<TemplateValue>::value
			|| std::is_same<bool, TemplateValue>::value
			|| This::FStatusChunk::BLOCK_BIT_WIDTH < InBitWidth
			|| InBitWidth < 2)
		{
			// 適切な整数型ではないので、登録に失敗する。
		}
		else if (std::is_signed<TemplateValue>::value)
		{
			// 符号あり整数型の状態値を登録する。
			using FSigned = typename This::FStatusValue::FSigned;
			auto const LocalValue(static_cast<FSigned>(InValue));
			return !This::IsOverflow(LocalValue, InBitWidth)
				&& nullptr != this->RegisterBitset(
					InChunkKey,
					InStatusKey,
					Psyque::MakeBitMask<FBitBlock>(InBitWidth) & LocalValue,
					-static_cast<FFormat>(InBitWidth));
		}
		else //if (std::is_unsigned<TemplateValue>::value)
		{
			// 符号なし整数型の状態値を登録する。
			auto const LocalValue(static_cast<FBitBlock>(InValue));
			return !This::IsOverflow(LocalValue, InBitWidth)
				&& nullptr != this->RegisterBitset(
					InChunkKey,
					InStatusKey,
					LocalValue,
					static_cast<FFormat>(InBitWidth));
		}
		return false;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値の取得
	/// @{
	public:
	/// @brief 状態値の数を取得する。
	/// @return 状態貯蔵器が保持している状態値の数。
	int32 GetStatusCount() const
	{
		return this->Properties.Num();
	}

	/// @brief 状態値が登録されているか判定する。
	/// @retval true  InStatusKey に対応する状態値が *this に登録されている。
	/// @retval false InStatusKey に対応する状態値は *this に登録されてない。
	bool IsRegisterd(
		/// [in] 判定する状態値に対応する識別値。
		TemplateStatusKey const InStatusKey)
	const
	{
		return this->Properties.Contains(InStatusKey);
	}

	/// @brief 状態値を取得する。
	/// @return 取得した状態値。 InStatusKey に対応する状態値がない場合は、
	/// This::FStatusValue::IsEmpty が真となる値を返す。
	/// @sa
	/// - This::RegisterStatus で、状態値を登録できる。
	/// - This::AssignStatus で、状態値を書き換えできる。
	typename This::FStatusValue FindStatus(
		/// [in] 取得する状態値に対応する識別値。
		TemplateStatusKey const InStatusKey)
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
				auto const LocalFBitWidth(This::GetBitWidth(LocalFormat));
				auto const LocalBitset(
					LocalChunk->GetBitset(
						LocalProperty->GetBitPosition(), LocalFBitWidth));

				// 状態値のビット構成から、構築する状態値の型を分ける。
				if (LocalFormat == static_cast<decltype(LocalFormat)>(
						This::FStatusValue::EKind::FLOAT))
				{
					// 浮動小数点数型の状態値を構築する。
					typename This::FFloatBitset const LocalValue(
						static_cast<typename This::FFloatBitset::BitsetType>(
							LocalBitset));
					return typename This::FStatusValue(LocalValue.Float);
				}
				else if (
					LocalFormat == static_cast<decltype(LocalFormat)>(
						This::FStatusValue::EKind::BOOL))
				{
					// 論理型の状態値を構築する。
					return typename This::FStatusValue(LocalBitset != 0);
				}
				else if (0 < LocalFormat)
				{
					// 符号なし整数型の状態値を構築する。
					return typename This::FStatusValue(LocalBitset);
				}
				else if (LocalFormat < 0)
				{
					// ビット幅より上位のビットを符号ビットで埋め、
					// 符号あり整数型の状態値を構築する。
					using FSigned = typename This::FStatusValue::FSigned;
					auto const LocalRestFBitWidth(
						This::FStatusChunk::BLOCK_BIT_WIDTH - LocalFBitWidth);
					return typename This::FStatusValue(
						Psyque::ShiftRightBitwiseFast(
							Psyque::ShiftLeftBitwiseFast(
								static_cast<FSigned>(LocalBitset),
								LocalRestFBitWidth),
							LocalRestFBitWidth));
				}
			}
			// 状態値プロパティがあれば、
			// 対応する状態値ビット列チャンクもあるはず。
			else {check(false);}
		}
		return typename This::FStatusValue();
	}

	/// @brief 状態値のプロパティを取得する。
	/// @return InStatusKey に対応する状態値のプロパティのコピー。
	/// 該当する状態値がない場合は
	/// This::FStatusProperty::IsEmpty が真となる値を返す。
	typename This::FStatusProperty FindProperty(
		/// [in] 取得する状態値プロパティに対応する識別値。
		TemplateStatusKey const InStatusKey)
	const
	{
		auto const LocalProperty(this->Properties.Find(InStatusKey));
		return LocalProperty != nullptr?
			*LocalProperty:
			typename This::FStatusProperty(TemplateChunkKey(), 0, 0);
	}

	/// @brief 状態値の型の種別を取得する。
	/// @return InStatusKey に対応する状態値の型の種別。
	/// 該当する状態値がない場合は This::FStatusValue::EKind::EMPTY を返す。
	typename This::FStatusValue::EKind FindKind(
		/// [in] 状態値に対応する識別値。
		TemplateStatusKey const InStatusKey)
	const
	{
		auto const LocalProperty(this->Properties.Find(InStatusKey));
		return LocalProperty != nullptr?
			This::GetKind(LocalProperty->GetFormat()):
			This::FStatusValue::EKind::EMPTY;
	}

	/// @brief 状態値のビット幅を取得する。
	/// @return InStatusKey に対応する状態値のビット幅。
	/// 該当する状態値がない場合は0を返す。
	typename This::FStatusChunk::FBitWidth FindBitWidth(
		/// [in] 状態値に対応する識別値。
		TemplateStatusKey const InStatusKey)
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
		TemplateStatusKey const InStatusKey)
	const
	{
		auto const LocalProperty(this->Properties.Find(InStatusKey));
		return LocalProperty != nullptr? LocalProperty->HasTransited(): -1;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値の比較
	/// @{
	public:
	/// @brief 状態値を比較する。
	/// @retval 正 比較式の評価は真。
	/// @retval 0  比較式の評価は偽。
	/// @retval 負 比較式の評価に失敗。
	typename This::FStatusValue::FEvaluation CompareStatus(
		/// [in] 状態値の比較式。
		typename This::FStatusComparison const& InComparison)
	const
	{
		auto const LocalRightKeyPointer(InComparison.GetRightKey());
		if (LocalRightKeyPointer == nullptr)
		{
			return this->CompareStatus(
				InComparison.GetKey(),
				InComparison.GetOperator(),
				InComparison.GetValue());
		}

		// 右辺となる状態値を取得して式を評価する。
		auto const LocalRightKey(
			static_cast<TemplateStatusKey>(*LocalRightKeyPointer));
		if (LocalRightKey == *LocalRightKeyPointer)
		{
			return this->CompareStatus(
				InComparison.GetKey(),
				InComparison.GetOperator(),
				LocalRightKey);
		}
		return -1;
	}

	/// @brief 状態値を比較する。
	/// @retval 正 比較式の評価は真。
	/// @retval 0  比較式の評価は偽。
	/// @retval 負 比較式の評価に失敗。
	typename This::FStatusValue::FEvaluation CompareStatus(
		/// [in] 左辺となる状態値の識別値。
		TemplateStatusKey const InLeftKey,
		/// [in] 適用する比較演算子。
		typename This::FStatusValue::EComparison const InOperator,
		/// [in] 右辺となる値。
		typename This::FStatusValue const& InRightValue)
	const
	{
		return this->FindStatus(InLeftKey).Compare(InOperator, InRightValue);
	}

	/// @brief 状態値を比較する。
	/// @retval 正 比較式の評価は真。
	/// @retval 0  比較式の評価は偽。
	/// @retval 負 比較式の評価に失敗。
	typename This::FStatusValue::FEvaluation CompareStatus(
		/// [in] 左辺となる状態値の識別値。
		TemplateStatusKey const InLeftKey,
		/// [in] 適用する比較演算子。
		typename This::FStatusValue::EComparison const InOperator,
		/// [in] 右辺となる状態値の識別値。
		TemplateStatusKey const InRightKey)
	const
	{
		return this->FindStatus(InLeftKey).Compare(
			InOperator, this->FindStatus(InRightKey));
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
	///   This::RegisterStatus で登録されてないと、失敗する。
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
	template<typename TemplateValue>
	bool AssignStatus(
		/// [in] 代入先となる状態値に対応する識別値。
		TemplateStatusKey const InStatusKey,
		/// [in] 状態値へ代入する値。以下の型の値を代入できる。
		/// - bool 型。
		/// - C++ 組み込み整数型。
		/// - C++ 組み込み浮動小数点数型。
		/// - This::FStatusValue 型。
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
		return This::AssignBitset(
			*LocalProperty,
			this->Chunks,
			This::MakeBitset(InValue, LocalProperty->GetFormat(), LocalMask));
	}

	/// @brief 状態値を演算し、結果を代入する。
	/// @retval true  成功。演算結果を状態値へ代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// 失敗する要因は This::AssignStatus を参照。
	bool AssignStatus(
		/// [in] 状態値の代入演算。
		typename This::FStatusAssignment const& InAssignment)
	{
		auto const LocalRightKeyPointer(InAssignment.GetRightKey());
		if (LocalRightKeyPointer == nullptr)
		{
			return this->AssignStatus(
				InAssignment.GetKey(),
				InAssignment.GetOperator(),
				InAssignment.GetValue());
		}

		// 右辺となる状態値を取得して演算する。
		auto const LocalRightKey(
			static_cast<TemplateStatusKey>(*LocalRightKeyPointer));
		return LocalRightKey == *LocalRightKeyPointer
			&& this->AssignStatus(
				InAssignment.GetKey(),
				InAssignment.GetOperator(),
				LocalRightKey);
	}

	/// @brief 状態値を演算し、結果を代入する。
	/// @retval true  成功。演算結果を状態値へ代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// 失敗する要因は This::AssignStatus を参照。
	bool AssignStatus(
		/// [in] 代入演算子の左辺となる状態値の識別値。
		TemplateStatusKey const InLeftKey,
		/// [in] 適用する代入演算子。
		typename This::FStatusValue::EAssignment const InOperator,
		/// [in] 代入演算子の右辺となる値。
		typename This::FStatusValue const& InRightValue)
	{
		if (InOperator == This::FStatusValue::EAssignment::COPY)
		{
			return this->AssignStatus(InLeftKey, InRightValue);
		}
		auto LocalLeftValue(this->FindStatus(InLeftKey));
		return LocalLeftValue.Assign(InOperator, InRightValue)
			&& this->AssignStatus(InLeftKey, LocalLeftValue);
	}

	/// @brief 状態値を演算し、結果を代入する。
	/// @retval true  成功。演算結果を状態値へ代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// 失敗する要因は This::AssignStatus を参照。
	bool AssignStatus(
		/// [in] 代入演算子の左辺となる状態値の識別値。
		TemplateStatusKey const InLeftKey,
		/// [in] 適用する代入演算子。
		typename This::FStatusValue::EAssignment const InOperator,
		/// [in] 代入演算子の右辺となる状態値の識別値。
		TemplateStatusKey const InRightKey)
	{
		return this->AssignStatus(
			InLeftKey, InOperator, this->FindStatus(InRightKey));
	}

	/// @brief Psyque::RuleEngine 管理者以外は、この関数は使用禁止。
	/// @details 状態変化フラグを初期化する。
	void _reset_transitions()
	{
		for (auto& LocalProperty: this->Properties)
		{
			LocalProperty.Value.Transit(false);
		}
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値ビット列チャンク
	/// @{
	public:
	/// @brief 状態値ビット列チャンクの数を取得する。
	/// @return 状態貯蔵器が保持している状態値ビット列チャンクの数。
	int32 GetChunkCount() const
	{
		return this->Chunks.Num();
	}

	/// @brief 状態値ビット列チャンクを予約する。
	void ReserveChunk(
		/// [in] 予約する状態値ビット列チャンクに対応する識別値。
		TemplateChunkKey const InChunkKey,
		/// [in] 予約するビット列コンテナの容量。
		int32 const InReserveBlocks,
		/// [in] 予約する空きビット領域コンテナの容量。
		int32 const InReserveEmpties)
	{
		auto& LocalChunk(this->Chunks.FindOrAdd(InChunkKey));
		LocalChunk.Reserve(InReserveBlocks, InReserveEmpties);
	}

	/// @brief 状態値ビット列チャンクを削除する。
	/// @retval true  成功。 InChunkKey に対応するチャンクを削除した。
	/// @retval false 失敗。該当するチャンクがない。
	bool RemoveChunk(
		/// [in] 削除する状態値ビット列チャンクの識別値。
		TemplateChunkKey const InChunkKey)
	{
		// 状態値ビット列チャンクを削除する。
		if (this->Chunks.Remove(InChunkKey) == 0)
		{
			return false;
		}

		// 状態値プロパティを削除する。
		for (auto i(this->Properties.CreateIterator()); i; ++i)
		{
			if (InChunkKey == i.Value().GetChunkKey())
			{
				i.RemoveCurrent();
			}
		}
		return true;
	}

	/// @brief 状態値ビット列チャンクをシリアル化する。
	/// @return シリアル化した状態値ビット列チャンク。
	/// @todo 未実装。
	typename This::FStatusChunk::FBitBlockArray SerializeChunk(
		/// [in] シリアル化する状態値ビット列チャンクの識別番号。
		TemplateChunkKey const InChunkKey)
	const;

	/// @brief シリアル化された状態値ビット列チャンクを復元する。
	/// @todo 未実装。
	bool DeserializeChunk(
		/// [in] 復元する状態値ビット列チャンクの識別値。
		TemplateChunkKey const InChunkKey,
		/// [in] シリアル化された状態値ビット列チャンク。
		typename This::FStatusChunk::FBitBlockArray const& InSerializedChunk);
	/// @}
	//-------------------------------------------------------------------------
	public:
	/// @brief 状態値のビット構成から、状態値の型の種別を取得する。
	/// @return 状態値の型の種別。
	static typename This::FStatusValue::EKind GetKind(
		/// [in] 状態値のビット構成。
		typename This::FStatusProperty::FFormat const InFormat)
	PSYQUE_NOEXCEPT
	{
		switch (InFormat)
		{
			case This::FStatusValue::EKind::EMPTY:
			case This::FStatusValue::EKind::BOOL:
			case This::FStatusValue::EKind::FLOAT:
			return static_cast<typename This::FStatusValue::EKind>(InFormat);

			default:
			return InFormat < 0?
				This::FStatusValue::EKind::SIGNED:
				This::FStatusValue::EKind::UNSIGNED;
		}
	}

	/// @brief 状態値のビット構成から、状態値のビット幅を取得する。
	/// @return 状態値のビット幅。
	static typename This::FStatusChunk::FBitWidth GetBitWidth(
		/// [in] 状態値のビット構成。
		typename This::FStatusProperty::FFormat const InFormat)
	PSYQUE_NOEXCEPT
	{
		switch (InFormat)
		{
			case This::FStatusValue::EKind::EMPTY:
			case This::FStatusValue::EKind::BOOL:
			static_assert(
				static_cast<unsigned>(This::FStatusValue::EKind::EMPTY) == 0
				&& static_cast<unsigned>(This::FStatusValue::EKind::BOOL) == 1,
				"");
			return InFormat;

			case This::FStatusValue::EKind::FLOAT:
			return sizeof(TemplateFloat) * CHAR_BIT;

			default: return Psyque::AbsInteger(InFormat);
		}
	}

	//-------------------------------------------------------------------------
	private:
	/// @brief 状態値を登録する。
	/// @return 登録した状態値のプロパティを指すポインタ。
	/// 登録に失敗した場合は nullptr を返す。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	/// @sa
	/// - This::FindStatus と This::AssignStatus
	///   で、登録した状態値にアクセスできる。
	/// - This::RemoveChunk で、登録した状態値をチャンク毎に削除できる。
	typename This::FStatusProperty const* RegisterBitset(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		TemplateChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別番号。
		TemplateStatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値となるビット列。
		typename This::FStatusChunk::FBitBlock const InBitset,
		/// [in] 登録する状態値のビット構成。
		typename This::FStatusProperty::FFormat const InFormat)
	{
		auto const LocalFBitWidth(This::GetBitWidth(InFormat));
		if (Psyque::ShiftRightBitwise(InBitset, LocalFBitWidth) == 0)
		{
			// 状態値を登録する状態値ビット列チャンクを用意する。
			auto& LocalChunk(this->Chunks.FindOrAdd(InChunkKey));

			// 状態値を登録して値を設定する。
			auto const LocalProperty(
				This::AddProperty(
					this->Properties,
					InChunkKey,
					LocalChunk,
					InStatusKey,
					InFormat));
			if (LocalProperty != nullptr
				&& 0 <= LocalChunk.SetBitset(
					LocalProperty->GetBitPosition(),
					LocalFBitWidth,
					InBitset))
			{
				return LocalProperty;
			}
		}
		else {check(false);}
		return nullptr;
	}

	/// @brief 状態値のプロパティを登録する。
	/// @return 登録した状態値のプロパティを指すポインタ。
	/// 登録できなかった場合は nullptr を返す。
	/// - InStatusKey に対応する状態値がすでに追加されていると失敗する。
	static typename This::FStatusProperty* AddProperty(
		/// [in,out] 状態値を登録する状態値プロパティの辞書。
		typename This::FPropertyMap& OutProperties,
		/// [in] 状態値を登録する状態値ビット列チャンクの識別値。
		TemplateChunkKey const InChunkKey,
		/// [in,out] 状態値を登録する状態値ビット列チャンク。
		typename This::FStatusChunk& OutChunk,
		/// [in] 登録する状態値に対応する識別値。
		TemplateStatusKey const InStatusKey,
		/// [in] 登録する状態値のビット構成。
		typename This::FStatusProperty::FFormat const InFormat)
	{
		auto const LocalDifferentFormat(
			InFormat != static_cast<decltype(InFormat)>(
				This::FStatusValue::EKind::EMPTY));
		if (LocalDifferentFormat && !OutProperties.Contains(InStatusKey))
		{
			// 状態値のビット領域を生成する。
			auto const LocalBitPosition(
				OutChunk.AddBitset(This::GetBitWidth(InFormat)));
			if (LocalBitPosition != This::FStatusChunk::INVALID_BIT_POSITION)
			{
				// 状態値プロパティを生成する。
				return &OutProperties.Emplace(
					InStatusKey,
					typename This::FStatusProperty(
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
	static bool AssignBitset(
		/// [in,out] 代入先となる状態値のプロパティ。
		typename This::FStatusProperty& OutProperty,
		/// [in,out] 状態値ビット列チャンクのコンテナ。
		typename This::FChunkMap& OutChunks,
		/// [in] 代入する状態値のビット列とビット幅。
		typename This::FStatusBitset const& InBitSet)
	PSYQUE_NOEXCEPT
	{
		if (0 < InBitSet.second)
		{
			// 状態値にビット列を設定する。
			auto const LocalChunk(OutChunks.Find(OutProperty.GetChunkKey()));
			if (LocalChunk != nullptr)
			{
				auto const LocalSetBitset(
					LocalChunk->SetBitset(
						OutProperty.GetBitPosition(),
						InBitSet.second,
						InBitSet.first));
				if (0 <= LocalSetBitset)
				{
					if (0 < LocalSetBitset)
					{
						// 状態値の変更を記録する。
						OutProperty.Transit(true);
					}
					return true;
				}
			}
			// 状態値プロパティがあるなら、状態値ビット列チャンクもあるはず。
			else {check(false);}
		}
		return false;
	}

	/// @brief 状態値のビット列をコピーして整理する。
	static void CopyBitsets(
		/// [in,out] コピー先となる状態値プロパティ辞書。
		typename This::FPropertyMap& OutProperties,
		/// [in,out] コピー先となる状態値ビット列チャンク辞書。
		typename This::FChunkMap& OutChunks,
		/// [in] コピー元となる状態値プロパティ辞書。
		typename This::FPropertyMap const& InProperties,
		/// [in] コピー元となる状態値ビット列チャンク辞書。
		typename This::FChunkMap const& InChunks)
	{
		// 状態値プロパティをビット幅の降順に並び替える。
		using FPropertyArray = TArray<
			std::pair<
				typename This::FStatusChunk::FBitWidth,
				This::FPropertyMap::TConstIterator>>;
		FPropertyArray LocalProperties;
		LocalProperties.Reserve(InProperties.Num());
		for (auto i(InProperties.CreateConstIterator()); i; ++i)
		{
			LocalProperties.Emplace(
				This::GetBitWidth(i.Value().GetFormat()), i);
		}
		LocalProperties.Sort(
			[](
				FPropertyArray::ElementType const& InLeft,
				FPropertyArray::ElementType const& InRight)
			->bool
			{
				return InRight.first < InLeft.first;
			});

		// 状態値をコピーする。
		check(OutProperties.Num() == 0 && OutChunks.Num() == 0);
		for (auto const& LocalProperty: LocalProperties)
		{
			This::CopyBitset(
				OutProperties, OutChunks, LocalProperty.second, InChunks);
		}
	}

	/// @brief 状態値のビット列をコピーする。
	static void CopyBitset(
		///[in,out] コピー先となる状態値プロパティ辞書。
		typename This::FPropertyMap& OutProperties,
		/// [in,out] コピー先となる状態値ビット列チャンク辞書。
		typename This::FChunkMap& OutChunks,
		/// [in] コピー元となる状態値プロパティを指す反復子。
		typename This::FPropertyMap::TConstIterator const& InProperty,
		/// [in] コピー元となる状態値ビット列チャンク辞書。
		typename This::FChunkMap const& InChunks)
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
		LocalTargetChunk.Reserve(
			LocalSourceChunk->BitBlocks.Num(),
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
			auto const LocalFBitWidth(This::GetBitWidth(LocalFormat));
			LocalTargetChunk.SetBitset(
				LocalTargetProperty->GetBitPosition(),
				LocalFBitWidth,
				LocalSourceChunk->GetBitset(
					InProperty.Value().GetBitPosition(), LocalFBitWidth));
			LocalTargetProperty->Transit(InProperty.Value().HasTransited());
		}
		else {check(false);}
	}

	//-------------------------------------------------------------------------
	/// @name ビット操作
	/// @{
	private:
	/// @brief 数値から This::FStatusBitset を構築する。
	/// @return
	/// 値から構築したビット列とビット幅のペア。
	/// 構築に失敗した場合は、ビット幅が0となる。
	static typename This::FStatusBitset MakeBitset(
		/// [in] ビット列の元となる数値。
		typename This::FStatusValue const& InValue,
		/// [in] 構築するビット列の構成。
		typename This::FStatusProperty::FFormat const InFormat,
		/// [in] 指定のビット幅に収まるようマスクするか。
		bool const InMask)
	{
		// 入力値のビット列を取得する。
		auto const LocalKind(This::GetKind(InFormat));
		typename This::FStatusChunk::FBitBlock LocalBitset;
		if (LocalKind != InValue.GetKind())
		{
			typename This::FStatusValue const LocalValue(InValue, LocalKind);
			if (LocalValue.IsEmpty())
			{
				return typename This::FStatusBitset(0, 0);
			}
			LocalBitset = LocalValue.GetBitset();
		}
		else
		{
			LocalBitset = InValue.GetBitset();
		}

		// ビット列とビット幅を構築する。
		/// @note 以下の処理は、テンプレート版の This::MakeBitset とまとめたい。
		using FBitWidth = typename This::FStatusChunk::BitWidth;
		if (InFormat == This::FStatusValue::EKind::BOOL)
		{
			return typename This::FStatusBitset(LocalBitet, 1);
		}
		else if (InFormat == This::FStatusValue::EKind::FLOAT)
		{
			return typename This::FStatusBitset(
				LocalBitset,
				static_cast<FBitWidth>(sizeof(TemplateFloat) * CHAR_BIT));
		}
		else if (InFormat < 0)
		{
			return This::MakeIntegerBitset<typename This::FStatusValue::FSigned>(
				LocalBitset, static_cast<FBitWidth>(-InFormat), InMask);
		}
		else if (0 < InFormat)
		{
			return This::MakeIntegerBitset<TemplateUnsigned>(
				LocalBitset, static_cast<FBitWidth>(InFormat), InMask);
		}
		check(false);
		return typename This::FStatusBitset(0, 0);
	}

	/// @brief 数値から This::FStatusBitset を構築する。
	/// @return 値から構築したビット列とビット幅のペア。
	/// 構築に失敗した場合は、ビット幅が0となる。
	template<typename TemplateValue>
	static typename This::FStatusBitset MakeBitset(
		/// [in] ビット列の元となる数値。
		TemplateValue const InValue,
		/// [in] 構築するビット列の構成。
		typename This::FStatusProperty::FFormat const InFormat,
		/// [in] 指定のビット幅に収まるようマスクするか。
		bool const InMask)
	{
		using FBitWidth = typename This::FStatusChunk::BitWidth;
		if (InFormat == This::FStatusValue::EKind::BOOL)
		{
			// 論理値のビット列を構築する。
			if (std::is_same<TemplateValue, bool>::value)
			{
				return typename This::FStatusBitset(InValue != 0, 1);
			}
		}
		else if (InFormat == This::FStatusValue::EKind::FLOAT)
		{
			// 浮動小数点数のビット列を構築する。
			typename This::FFloatBitset const LocalValue(
				static_cast<TemplateFloat>(InValue));
			return typename This::FStatusBitset(
				LocalValue.Bitset,
				static_cast<FBitWidth>(sizeof(TemplateFloat) * CHAR_BIT));
		}
		else if (InFormat < 0)
		{
			// 符号あり整数のビット列を構築する。
			return This::MakeIntegerBitset<typename This::FStatusValue::FSigned>(
				InValue, static_cast<FBitWidth>(-InFormat), InMask);
		}
		else if (0 < InFormat)
		{
			// 符号なし整数のビット列を構築する。
			return This::MakeIntegerBitset<TemplateUnsigned>(
				InValue, static_cast<FBitWidth>(InFormat), InMask);
		}
		check(false);
		return typename This::FStatusBitset(0, 0);
	}

	/// @brief 数値を整数に変換してから This::FStatusBitset を構築する。
	/// @return 数値から構築したビット列とビット幅のペア。
	/// 構築に失敗した場合は、ビット幅が0となる。
	template<typename TemplateInteger, typename TemplateValue>
	static typename This::FStatusBitset MakeIntegerBitset(
		/// [in] ビット列の元となる数値。
		TemplateValue const InValue,
		/// [in] 構築するビット列の幅。
		typename This::FStatusChunk::FBitWidth InBitWidth,
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
		return typename This::FStatusBitset(LocalInteger, InBitWidth);
	}

	/// @brief 論理値を整数に変換して
	/// This::FStatusBitset を構築させないためのダミー関数。
	template<typename TemplateInteger>
	static typename This::FStatusBitset MakeIntegerBitset(
		bool, typename This::FStatusChunk::FBitWidth, bool)
	{
		// bool型の値を他の型へ変換できないようにする。
		return typename This::FStatusBitset(0, 0);
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
		typename This::FStatusValue::FSigned const InInteger,
		/// [in] 許容するビット幅。
		SIZE_T const InBitWidth)
	PSYQUE_NOEXCEPT
	{
		using FBitBlock = typename This::FStatusChunk::FBitBlock;
		auto const LocalRestField(
			Psyque::ShiftRightBitwiseFast(
				static_cast<FBitBlock>(InInteger), InBitWidth - 1));
		auto const LocalRestMask(
			Psyque::ShiftRightBitwiseFast(
				static_cast<FBitBlock>(
					Psyque::ShiftRightBitwiseFast(
						InInteger, sizeof(InInteger) * CHAR_BIT - 1)),
				InBitWidth - 1));
		return LocalRestField != LocalRestMask;
	}
	/// @}
	//-------------------------------------------------------------------------
	private:
	typename This::FChunkMap Chunks;        ///< 状態値ビット列チャンクの辞書。
	typename This::FPropertyMap Properties; ///< 状態値プロパティの辞書。

}; // class Psyque::RuleEngine::_private::TReservoir

// vim: set noexpandtab:
