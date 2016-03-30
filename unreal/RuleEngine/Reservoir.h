// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TReservoir
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <unordered_map>
#include "../PrimitiveBits.h"
#include "./StatusValue.h"
#include "./StatusProperty.h"
#include "./StatusChunk.h"
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
/// .
/// @tparam TemplateUnsigned  @copydoc TReservoir::FStatusValue::FUnsigned
/// @tparam TemplateFloat     @copydoc TReservoir::FStatusValue::FFloat
/// @tparam TemplateStatusKey @copydoc TReservoir::FStatusKey
/// @tparam TemplateChunkKey  @copydoc TReservoir::FChunkKey
/// @tparam TemplateAllocator @copydoc TReservoir::FAllocator
template<
	typename TemplateUnsigned,
	typename TemplateFloat,
	typename TemplateStatusKey,
	typename TemplateChunkKey,
	typename TemplateAllocator>
class Psyque::RuleEngine::_private::TReservoir
{
	private: using This = TReservoir; ///< @copydoc RuleEngine::TDriver::This

	//-------------------------------------------------------------------------
	/// @brief 状態値を識別するための値を表す型。
	public: using FStatusKey = TemplateStatusKey;
	/// @brief チャンクを識別するための値を表す型。
	public: using FChunkKey = TemplateChunkKey;
	/// @brief 各種コンテナに用いるメモリ割当子の型。
	public: using FAllocator = TemplateAllocator;

	//-------------------------------------------------------------------------
	/// @brief 状態値。
	public: using FStatusValue = Psyque::RuleEngine::_private::TStatusValue<
		TemplateUnsigned, TemplateFloat>;
	/// @brief 状態値が格納されているビット領域を示す。
	public: using FStatusProperty =
		Psyque::RuleEngine::_private::TStatusProperty<
			typename This::FChunkKey, uint32, int8>;
	/// @brief 状態値の比較演算の引数。
	public: using FStatusComparison =
		Psyque::RuleEngine::_private::TStatusOperation<
			typename This::FStatusKey,
			EStatusComparison,
			typename This::FStatusValue>;
	/// @brief 状態値の代入演算の引数。
	public: using FStatusAssignment =
		Psyque::RuleEngine::_private::TStatusOperation<
			typename This::FStatusKey,
			typename RuleEngine::EStatusAssignment,
			typename This::FStatusValue>;

	//-------------------------------------------------------------------------
	/// @brief 状態値プロパティの辞書。
	private: using FPropertyMap = std::unordered_map<
		 typename This::FStatusKey,
		 typename This::FStatusProperty,
		 Psyque::Hash::TPrimitiveBits<typename This::FStatusKey>,
		 std::equal_to<typename This::FStatusKey>,
		 typename This::FAllocator>;
	/// @brief 状態値を格納するビット領域のコンテナ。
	private: using FStatusChunk = Psyque::RuleEngine::_private::TStatusChunk<
		typename This::FStatusValue::FUnsigned,
		typename This::FStatusProperty::FBitPosition,
		typename std::make_unsigned<
			typename This::FStatusProperty::FFormat>::type,
		typename This::FAllocator>;
	/// @brief 状態値ビット列チャンクの辞書。
	private: using FChunkMap = std::unordered_map<
		 typename This::FChunkKey,
		 typename This::FStatusChunk,
		 Psyque::Hash::TPrimitiveBits<typename This::FChunkKey>,
		 std::equal_to<typename This::FChunkKey>,
		 typename This::FAllocator>;

	//-------------------------------------------------------------------------
	/// @brief 浮動小数点数とビット列を変換する。
	private: using FFloatBitset = Psyque::TFloatBitset<TemplateFloat>;
	/// @brief ビット列とビット幅のペア。
	private: using FBitset = std::pair<
		 typename This::FStatusChunk::FBitBlock,
		 typename This::FStatusChunk::FBitWidth>;

	//-------------------------------------------------------------------------
	/// @name 構築と代入
	/// @{

	/// @brief 空の状態貯蔵器を構築する。
	public: TReservoir(
		/// [in] チャンク辞書のバケット数。
		std::size_t const InChunkCapacity,
		/// [in] 状態値プロパティ辞書のバケット数。
		std::size_t const InStatusCapacity,
		/// [in] 使用するメモリ割当子の初期値。
		typename This::FAllocator const& InAllocator = This::FAllocator()):
	Chunks(
		InChunkCapacity,
		typename This::FChunkMap::hasher(),
		typename This::FChunkMap::key_equal(),
		InAllocator),
	Properties(
		InStatusCapacity,
		typename This::FPropertyMap::hasher(),
		typename This::FPropertyMap::key_equal(),
		InAllocator)
	{}

#ifdef PSYQUE_NO_STD_DEFAULTED_FUNCTION
	/// @brief ムーブ構築子。
	public: TReservoir(
		/// [in,out] ムーブ元となるインスタンス。
		This&& OutSource):
	Chunks(MoveTemp(OutSource.Chunks)),
	Properties(MoveTemp(OutSource.Properties))
	{}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: This& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		This&& OutSource)
	{
		this->Chunks = MoveTemp(OutSource.Chunks);
		this->Properties = MoveTemp(OutSource.Properties);
		return *this;
	}
#endif // defined(PSYQUE_NO_STD_DEFAULTED_FUNCTION)

	/// @brief 状態貯蔵器で使われているメモリ割当子を取得する。
	/// @return *this で使われているメモリ割当子のコピー。
	public: typename This::FAllocator get_allocator()
	const PSYQUE_NOEXCEPT
	{
		return this->Properties.get_allocator();
	}

	/// @brief 状態貯蔵器を再構築する。
	public: void Rebuild(
		/// [in] 状態値ビット列チャンク辞書のバケット数。
		std::size_t const InChunkCapacity,
		/// [in] 状態値プロパティ辞書のバケット数。
		std::size_t const InStatusCapacity)
	{
		// 新たな辞書を用意する。
		typename This::FChunkMap LocalChunks(
			InChunkCapacity,
			this->Chunks.hash_function(),
			this->Chunks.key_eq(),
			this->Chunks.get_allocator());
		typename This::FPropertyMap LocalProperties(
			InStatusCapacity,
			this->Properties.hash_function(),
			this->Properties.key_eq(),
			this->Properties.get_allocator());

		// 現在の辞書を新たな辞書にコピーして整理する。
		This::CopyBitsets(
			LocalProperties, LocalChunks, this->Properties, this->Chunks);
		for (auto i(LocalChunks.begin()); i != LocalChunks.end();)
		{
			auto& LocalChunk(i->second);
			if (LocalChunk.BitBlocks.empty())
			{
				i = LocalChunks.erase(i);
			}
			else
			{
				LocalChunk.BitBlocks.shrink_to_fit();
				LocalChunk.EmptyBitsets.shrink_to_fit();
				++i;
			}
		}
		this->Properties = MoveTemp(LocalProperties);
		this->Chunks = MoveTemp(LocalChunks);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値の登録
	/// @{

	/// @brief 状態値が登録されているか判定する。
	/// @retval true  InStatusKey に対応する状態値が *this に登録されている。
	/// @retval false InStatusKey に対応する状態値は *this に登録されてない。
	public: bool IsRegistered(
		/// [in] 判定する状態値に対応する識別値。
		typename This::FStatusKey const InStatusKey)
	const
	{
		return this->Properties.find(InStatusKey) != this->Properties.end();
	}

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
		typename This::FChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別値。
		typename This::FStatusKey const InStatusKey,
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
			/// This::FFloatBitset::FFloat が float 型なのが原因。
			typename This::FFloatBitset::FFloat const
				LocalFloat(InValue);
			return nullptr != this->RegisterBitset(
				InChunkKey,
				InStatusKey,
				typename This::FFloatBitset(LocalFloat).Bitset,
				static_cast<int8>(RuleEngine::EStatusKind::Float));
		}
		else if (std::is_same<bool, TemplateValue>::value)
		{
			// 論理型の状態値を登録する。
			return nullptr != this->RegisterBitset(
				InChunkKey,
				InStatusKey,
				InValue != 0,
				static_cast<int8>(RuleEngine::EStatusKind::Bool));
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
	public: template<typename TemplateValue>
	bool RegisterStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		typename This::FChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別番号。
		typename This::FStatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値。 C++ 組み込み整数型であること。
		TemplateValue const InValue,
		/// [in] 登録する状態値のビット幅。
		std::size_t const InBitWidth)
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

	/// @brief 状態値のプロパティを取得する。
	/// @return
	/// InStatusKey に対応する状態値のプロパティのコピー。
	/// 該当する状態値がない場合は
	/// This::FStatusProperty::IsEmpty が真となる値を返す。
	public: typename This::FStatusProperty FindProperty(
		/// [in] 取得する状態値プロパティに対応する識別値。
		typename This::FStatusKey const InStatusKey)
	const
	{
		auto const LocalFind(this->Properties.find(InStatusKey));
		return LocalFind != this->Properties.end()?
			LocalFind->second:
			typename This::FStatusProperty(
				typename This::FChunkKey(), 0, 0);
	}

	/// @brief 状態値の型の種別を取得する。
	/// @return
	/// InStatusKey に対応する状態値の型の種別。該当する状態値がない場合は
	/// RuleEngine::EStatusKind::Empty を返す。
	public: RuleEngine::EStatusKind FindKind(
		/// [in] 状態値に対応する識別値。
		typename This::FStatusKey const InStatusKey)
	const
	{
		auto const LocalFind(this->Properties.find(InStatusKey));
		return LocalFind != this->Properties.end()?
			This::GetKind(LocalFind->second.GetFormat()):
			RuleEngine::EStatusKind::Empty;
	}

	/// @brief 状態値のビット幅を取得する。
	/// @return
	/// InStatusKey に対応する状態値のビット幅。
	/// 該当する状態値がない場合は0を返す。
	public: std::size_t FindBitWidth(
		/// [in] 状態値に対応する識別値。
		typename This::FStatusKey const InStatusKey)
	const
	{
		auto const LocalFind(this->Properties.find(InStatusKey));
		return LocalFind != this->Properties.end()?
			This::GetBitWidth(LocalFind->second.GetFormat()): 0;
	}

	/// @brief 状態変化フラグを取得する。
	public: Psyque::ETernary FindTransition(
		/// [in] 状態変化フラグを取得する状態値に対応する識別値。
		typename This::FStatusKey const InStatusKey)
	const
	{
		auto const LocalFind(this->Properties.find(InStatusKey));
		return LocalFind != this->Properties.end()?
			static_cast<Psyque::ETernary>(LocalFind->second.GetTransition()):
			Psyque::ETernary::Unknown;
	}

	/// @brief 状態値を取得する。
	/// @return
	///   取得した状態値。 InStatusKey に対応する状態値がない場合は、
	///   This::FStatusValue::IsEmpty が真となる値を返す。
	/// @sa
	/// - This::RegisterStatus で、状態値を登録できる。
	/// - This::AssignStatus で、状態値を書き換えできる。
	public: typename This::FStatusValue FindStatus(
		/// [in] 取得する状態値に対応する識別値。
		typename This::FStatusKey const InStatusKey)
	const
	{
		// 状態値プロパティを取得する、
		auto const local_property_iterator(this->Properties.find(InStatusKey));
		if (local_property_iterator == this->Properties.end())
		{
			return typename This::FStatusValue();
		}
		auto const& LocalProperty(local_property_iterator->second);

		// 状態値ビット列チャンクから状態値のビット列を取得する。
		auto const local_chunk_iterator(
			this->Chunks.find(LocalProperty.GetChunkKey()));
		if (local_chunk_iterator == this->Chunks.end())
		{
			// 状態値プロパティがあれば、
			// 対応する状態値ビット列チャンクもあるはず。
			check(false);
			return typename This::FStatusValue();
		}
		auto const LocalFormat(LocalProperty.GetFormat());
		auto const LocalBitWidth(This::GetBitWidth(LocalFormat));
		auto const LocalBitset(
			local_chunk_iterator->second.GetBitset(
				LocalProperty.GetBitPosition(), LocalBitWidth));

		// 状態値のビット構成から、構築する状態値の型を分ける。
		if (0 < LocalFormat)
		{
			return LocalFormat == static_cast<int8>(RuleEngine::EStatusKind::Bool)?
				// 論理型の状態値を構築する。
				typename This::FStatusValue(LocalBitset != 0):
				// 符号なし整数型の状態値を構築する。
				typename This::FStatusValue(LocalBitset);
		}
		else if (LocalFormat == static_cast<int8>(RuleEngine::EStatusKind::Float))
		{
			// 浮動小数点数型の状態値を構築する。
			using FBitset = typename This::FFloatBitset::FBitset;
			return typename This::FStatusValue(
				This::FFloatBitset(static_cast<FBitset>(LocalBitset)).Float);
		}
		else if (LocalFormat < 0)
		{
			// 符号あり整数型の状態値を構築する。
			using FSigned = typename This::FStatusValue::FSigned;
			auto const LocalRestBitWidth(
				This::FStatusChunk::BLOCK_BIT_WIDTH - LocalBitWidth);
			return typename This::FStatusValue(
				Psyque::ShiftRightBitwiseFast(
					Psyque::ShiftLeftBitwiseFast(
						static_cast<FSigned>(LocalBitset), LocalRestBitWidth),
					LocalRestBitWidth));
		}
		else
		{
			// 空の状態値は登録できないはず。
			check(false);
			return typename This::FStatusValue();
		}
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値の比較
	/// @{

	/// @brief 状態値を比較する。
	/// @retval 正 比較式の評価は真。
	/// @retval 0  比較式の評価は偽。
	/// @retval 負 比較式の評価に失敗。
	public: Psyque::ETernary CompareStatus(
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
			static_cast<typename This::FStatusKey>(
				*LocalRightKeyPointer));
		if (LocalRightKey == *LocalRightKeyPointer)
		{
			return this->CompareStatus(
				InComparison.GetKey(),
				InComparison.GetOperator(),
				LocalRightKey);
		}
		return Psyque::ETernary::Unknown;
	}

	/// @brief 状態値を比較する。
	/// @retval 正 比較式の評価は真。
	/// @retval 0  比較式の評価は偽。
	/// @retval 負 比較式の評価に失敗。
	public: Psyque::ETernary CompareStatus(
		/// [in] 左辺となる状態値の識別値。
		typename This::FStatusKey const InLeftKey,
		/// [in] 適用する比較演算子。
		RuleEngine::EStatusComparison const InOperator,
		/// [in] 右辺となる値。
		typename This::FStatusValue const& InRightValue)
	const
	{
		return this->FindStatus(InLeftKey).Compare(InOperator, InRightValue);
	}

	/// @brief 状態値を比較する。
	public: Psyque::ETernary CompareStatus(
		/// [in] 左辺となる状態値の識別値。
		typename This::FStatusKey const InLeftKey,
		/// [in] 適用する比較演算子。
		RuleEngine::EStatusComparison const InOperator,
		/// [in] 右辺となる状態値の識別値。
		typename This::FStatusKey const InRightKey)
	const
	{
		return this->FindStatus(InLeftKey).Compare(
			InOperator, this->FindStatus(InRightKey));
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値の代入
	/// @{

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
	public: template<typename TemplateValue>
	bool AssignStatus(
		/// [in] 代入先となる状態値に対応する識別値。
		typename This::FStatusKey const InStatusKey,
		/// [in] 状態値へ代入する値。以下の型の値を代入できる。
		/// - bool 型。
		/// - C++ 組み込み整数型。
		/// - C++ 組み込み浮動小数点数型。
		/// - This::FStatusValue 型。
		TemplateValue const& InValue)
	{
		auto const local_property_iterator(
			this->Properties.find(InStatusKey));
		if (local_property_iterator == this->Properties.end())
		{
			return false;
		}
		/// @note InValue が状態値のビット幅を超えている場合、
		/// 失敗となるように実装しておく。
		/// 失敗とせず、ビット列をマスクして代入する実装も可能。どちらが良い？
		auto const local_mask(false);
		auto& LocalProperty(local_property_iterator->second);
		return This::AssignBitset(
			LocalProperty,
			this->Chunks,
			This::MakeBitsetWidth(
				InValue, LocalProperty.GetFormat(), local_mask));
	}

	/// @brief 状態値を演算し、結果を代入する。
	/// @retval true  成功。演算結果を状態値へ代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// 失敗する要因は This::AssignStatus を参照。
	public: bool AssignStatus(
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
			static_cast<typename This::FStatusKey>(
				*LocalRightKeyPointer));
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
	public: bool AssignStatus(
		/// [in] 代入演算子の左辺となる状態値の識別値。
		typename This::FStatusKey const InLeftKey,
		/// [in] 適用する代入演算子。
		typename RuleEngine::EStatusAssignment const InOperator,
		/// [in] 代入演算子の右辺となる値。
		typename This::FStatusValue const& InRightValue)
	{
		if (InOperator == RuleEngine::EStatusAssignment::Copy)
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
	public: bool AssignStatus(
		/// [in] 代入演算子の左辺となる状態値の識別値。
		typename This::FStatusKey const InLeftKey,
		/// [in] 適用する代入演算子。
		typename RuleEngine::EStatusAssignment const InOperator,
		/// [in] 代入演算子の右辺となる状態値の識別値。
		typename This::FStatusKey const InRightKey)
	{
		return this->AssignStatus(
			InLeftKey, InOperator, this->FindStatus(InRightKey));
	}

	/// @brief 状態変化フラグを初期化する。
	/// @warning Psyque::RuleEngine 管理者以外は、この関数は使用禁止。
	public: void _reset_transitions()
	{
		for (auto& LocalProperty: this->Properties)
		{
			LocalProperty.second.SetTransition(false);
		}
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値ビット列チャンク
	/// @{

	/// @brief 状態値ビット列チャンクを予約する。
	public: void ReserveChunk(
		/// [in] 予約する状態値ビット列チャンクに対応する識別値。
		typename This::FChunkKey const InChunkKey,
		/// [in] 予約するビット列コンテナの容量。
		std::size_t const InBlockCapacity,
		/// [in] 予約する空きビット領域コンテナの容量。
		std::size_t const InEmptyCapacity)
	{
		// 状態値を登録する状態値ビット列チャンクを用意する。
		auto const LocalEmplace(
			this->Chunks.emplace(
				InChunkKey,
				typename This::FChunkMap::mapped_type(
					this->Chunks.get_allocator())));
		auto& LocalChunk(*LocalEmplace.first);
		LocalChunk.second.BitBlocks.reserve(InBlockCapacity);
		LocalChunk.second.EmptyBitsets.reserve(InEmptyCapacity);
	}

	/// @brief 状態値ビット列チャンクを削除する。
	/// @retval true  成功。 InChunkKey に対応するチャンクを削除した。
	/// @retval false 失敗。該当するチャンクがない。
	public: bool RemoveChunk(
		/// [in] 削除する状態値ビット列チャンクの識別値。
		typename This::FChunkKey const InChunkKey)
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
	public: typename This::FStatusChunk::FBitBlockArray serialize_chunk(
		/// [in] シリアル化する状態値ビット列チャンクの識別番号。
		typename This::FChunkKey const InChunkKey)
	const;

	/// @brief シリアル化された状態値ビット列チャンクを復元する。
	/// @todo 未実装。
	public: bool deserialize_chunk(
		/// [in] 復元する状態値ビット列チャンクの識別値。
		typename This::FChunkKey const InChunkKey,
		/// [in] シリアル化された状態値ビット列チャンク。
		typename This::FStatusChunk::FBitBlockArray const&
			in_serialized_chunk);
	/// @}
	//-------------------------------------------------------------------------
	/// @brief 状態値のビット構成から、状態値の型の種別を取得する。
	/// @return 状態値の型の種別。
	public: static RuleEngine::EStatusKind GetKind(
		/// [in] 状態値のビット構成。
		typename This::FStatusProperty::FFormat const InFormat)
	PSYQUE_NOEXCEPT
	{
		switch (static_cast<uint8>(InFormat))
		{
			case RuleEngine::EStatusKind::Empty:
			case RuleEngine::EStatusKind::Bool:
			case RuleEngine::EStatusKind::Float:
			return static_cast<typename RuleEngine::EStatusKind>(InFormat);

			default:
			return InFormat < 0?
				RuleEngine::EStatusKind::Signed:
				RuleEngine::EStatusKind::Unsigned;
		}
	}

	/// @brief 状態値のビット構成から、状態値のビット幅を取得する。
	/// @return 状態値のビット幅。
	public: static typename This::FStatusChunk::FBitWidth GetBitWidth(
		/// [in] 状態値のビット構成。
		typename This::FStatusProperty::FFormat const InFormat)
	PSYQUE_NOEXCEPT
	{
		switch (static_cast<uint8>(InFormat))
		{
			case RuleEngine::EStatusKind::Empty:
			case RuleEngine::EStatusKind::Bool:
			static_assert(
				static_cast<int8>(RuleEngine::EStatusKind::Empty) == 0
				&& static_cast<int8>(RuleEngine::EStatusKind::Bool) == 1,
				"");
			return InFormat;

			case RuleEngine::EStatusKind::Float:
			return sizeof(typename This::FStatusValue::FFloat)
				* CHAR_BIT;

			default: return Psyque::AbsInteger(InFormat);
		}
	}

	//-------------------------------------------------------------------------
	/// @brief 状態値を登録する。
	/// @return
	/// 登録した状態値のプロパティを指すポインタ。
	/// 登録に失敗した場合は nullptr を返す。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	/// @sa
	/// - This::FindStatus と This::AssignStatus
	///   で、登録した状態値にアクセスできる。
	/// - This::RemoveChunk で、登録した状態値をチャンク毎に削除できる。
	private: typename This::FStatusProperty const* RegisterBitset(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		typename This::FChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別番号。
		typename This::FStatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値となるビット列。
		typename This::FStatusChunk::FBitBlock const& InBitset,
		/// [in] 登録する状態値のビット構成。
		typename This::FStatusProperty::FFormat const InFormat)
	{
		// 状態値を登録する状態値ビット列チャンクを用意する。
		auto const LocalEmplace(
			this->Chunks.emplace(
				InChunkKey,
				typename This::FChunkMap::mapped_type(
					this->Chunks.get_allocator())));
		auto& LocalChunk(*LocalEmplace.first);

		// 状態値を登録する。
		auto const LocalProperty(
			This::AllocateBitset(
				this->Properties, LocalChunk, InStatusKey, InFormat));

		// 状態値に初期値を設定する。
		if (LocalProperty != nullptr
			&& 0 <= LocalChunk.second.SetBitset(
				LocalProperty->second.GetBitPosition(),
				This::GetBitWidth(InFormat),
				InBitset))
		{
			return &LocalProperty->second;
		}
		return nullptr;
	}

	/// @brief 状態値を登録する。
	/// @return
	/// 登録した状態値のプロパティを指すポインタ。
	/// 状態値を登録できなかった場合は nullptr を返す。
	/// - InStatusKey に対応する状態値がすでに追加されていると失敗する。
	private: static typename This::FPropertyMap::value_type* AllocateBitset(
		/// [in,out] 状態値を登録する状態値プロパティの辞書。
		typename This::FPropertyMap& OutProperties,
		/// [in,out] 状態値を登録する状態値ビット列チャンク。
		typename This::FChunkMap::value_type& OutChunk,
		/// [in] 登録する状態値に対応する識別値。
		typename This::FStatusKey const InStatusKey,
		/// [in] 登録する状態値のビット構成。
		typename This::FStatusProperty::FFormat const InFormat)
	{
		if (InFormat != static_cast<int8>(RuleEngine::EStatusKind::Empty))
		{
			// 状態値のビット領域を生成する。
			auto const LocalBitWidth(This::GetBitWidth(InFormat));
			auto const LocalBitPosition(
				OutChunk.second.AllocateBitset(LocalBitWidth));
			if (LocalBitPosition != This::FStatusChunk::INVALID_BIT_POSITION)
			{
				// 状態値プロパティを生成する。
				auto const LocalEmplace(
					OutProperties.emplace(
						InStatusKey,
						typename This::FPropertyMap::mapped_type(
							OutChunk.first, LocalBitPosition, InFormat)));
				if (LocalEmplace.second)
				{
					auto& LocalProperty(*LocalEmplace.first);
					return &LocalProperty;
				}
				check(false);
			}
			else
			{
				check(false);
			}
		}
		else
		{
			check(false);
		}
		return nullptr;
	}

	/// @brief 状態値へ値を代入する。
	/// @retval true  成功。状態値へ値を代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// - 代入する値のビット幅が0だと失敗する。
	/// - 代入する値のビット幅が状態値のビット幅を超えると失敗する。
	/// - 状態値を格納する状態値ビット列チャンクがないと失敗する。
	private: static bool AssignBitset(
		/// [in,out] 代入先となる状態値のプロパティ。
		typename This::FStatusProperty& OutProperty,
		/// [in,out] 状態値ビット列チャンクのコンテナ。
		typename This::FChunkMap& OutChunks,
		/// [in] 代入する状態値のビット列とビット幅。
		typename This::FBitset const& InBitset)
	PSYQUE_NOEXCEPT
	{
		if (0 < InBitset.second)
		{
			// 状態値にビット列を設定する。
			auto const local_chunk_iterator(
				OutChunks.find(OutProperty.GetChunkKey()));
			if (local_chunk_iterator != OutChunks.end())
			{
				auto const local_set_bit_field(
					local_chunk_iterator->second.SetBitset(
						OutProperty.GetBitPosition(),
						InBitset.second,
						InBitset.first));
				if (0 <= local_set_bit_field)
				{
					if (0 < local_set_bit_field)
					{
						// 状態値の変更を記録する。
						OutProperty.SetTransition(true);
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

	//-------------------------------------------------------------------------
	/// @brief 状態値をコピーして整理する。
	private: static void CopyBitsets(
		/// [in,out] コピー先となる状態値プロパティ辞書。
		typename This::FPropertyMap& OutProperties,
		/// [in,out] コピー先となる状態値ビット列チャンク辞書。
		typename This::FChunkMap& OutChunks,
		/// [in] コピー元となる状態値プロパティ辞書。
		typename This::FPropertyMap const& InProperties,
		/// [in] コピー元となる状態値ビット列チャンク辞書。
		typename This::FChunkMap const& InChunks)
	{
		// 状態値プロパティのポインタのコンテナを構築する。
		using FPropertyArray = std::vector<
			std::pair<
				typename This::FStatusChunk::FBitWidth,
				typename This::FPropertyMap::value_type const*>,
			typename This::FAllocator>;
		FPropertyArray LocalProperties(InProperties.get_allocator());
		LocalProperties.reserve(InProperties.size());
		for (auto& LocalProperty: InProperties)
		{
			LocalProperties.emplace_back(
				This::GetBitWidth(LocalProperty.second.GetFormat()),
				&LocalProperty);
		}

		// 状態値プロパティをビット幅の降順に並び替える。
		std::sort(
			LocalProperties.begin(),
			LocalProperties.end(),
			[](
				typename FPropertyArray::value_type const& InLeft,
				typename FPropertyArray::value_type const& InRight)
			->bool
			{
				return InRight.first < InLeft.first;
			});

		// 状態値をコピーする。
		for (auto& LocalProperty: LocalProperties)
		{
			This::CopyBitset(
				OutProperties, OutChunks, *LocalProperty.second, InChunks);
		}
	}

	/// @brief 状態値をコピーする。
	private: static void CopyBitset(
		///[in,out] コピー先となる状態値プロパティ辞書。
		typename This::FPropertyMap& OutProperties,
		/// [in,out] コピー先となる状態値ビット列チャンク辞書。
		typename This::FChunkMap& OutChunks,
		/// [in] コピー元となる状態値プロパティ。
		typename This::FPropertyMap::value_type const& InProperty,
		/// [in] コピー元となる状態値ビット列チャンク辞書。
		typename This::FChunkMap const& InChunks)
	{
		// コピー元となる状態値ビット列チャンクを取得する。
		auto const local_source_chunk_iterator(
			InChunks.find(InProperty.second.GetChunkKey()));
		if (local_source_chunk_iterator == InChunks.end())
		{
			check(false);
			return;
		}
		auto const& local_source_chunk(local_source_chunk_iterator->second);

		// コピー先となる状態値を用意する。
		auto const LocalEmplace(
			OutChunks.emplace(
				InProperty.second.GetChunkKey(),
				typename This::FChunkMap::mapped_type(
					OutChunks.get_allocator())));
		auto& local_target_chunk(*LocalEmplace.first);
		if (LocalEmplace.second)
		{
			local_target_chunk.second.BitBlocks.reserve(
				local_source_chunk.BitBlocks.size());
			local_target_chunk.second.EmptyBitsets.reserve(
				local_source_chunk.EmptyBitsets.size());
		}
		auto const LocalFormat(InProperty.second.GetFormat());
		auto const local_target_property(
			This::AllocateBitset(
				OutProperties,
				local_target_chunk,
				InProperty.first,
				LocalFormat));
		if (local_target_property == nullptr)
		{
			check(false);
			return;
		}

		// 状態値のビット領域をコピーする。
		auto const LocalBitWidth(This::GetBitWidth(LocalFormat));
		local_target_chunk.second.SetBitset(
			local_target_property->second.GetBitPosition(),
			LocalBitWidth,
			local_source_chunk.GetBitset(
				InProperty.second.GetBitPosition(), LocalBitWidth));
		local_target_property->second.SetTransition(
			InProperty.second.GetTransition());
	}

	//-------------------------------------------------------------------------
	/// @brief 数値からビット列を構築する。
	/// @return
	/// 値から構築したビット列とビット幅のペア。
	/// 構築に失敗した場合は、ビット幅が0となる。
	private: static typename This::FBitset MakeBitsetWidth(
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
				return typename This::FBitset(0, 0);
			}
			LocalBitset = LocalValue.GetBitset();
		}
		else
		{
			LocalBitset = InValue.GetBitset();
		}

		// ビット列とビット幅を構築する。
		using FBitWidth = typename This::FStatusChunk::FBitWidth;
		if (InFormat == static_cast<int8>(RuleEngine::EStatusKind::Bool))
		{
			return typename This::FBitset(LocalBitset, 1);
		}
		else if (InFormat == static_cast<int8>(RuleEngine::EStatusKind::Float))
		{
			return typename This::FBitset(
				LocalBitset,
				static_cast<FBitWidth>(
					sizeof(typename This::FStatusValue::FFloat)
					* CHAR_BIT));
		}
		else if (InFormat < 0)
		{
			using FSigned = typename This::FStatusValue::FSigned;
			return This::MakeBitsetWidth<FSigned>(
				LocalBitset, static_cast<FBitWidth>(-InFormat), InMask);
		}
		else if (0 < InFormat)
		{
			return This::MakeBitsetWidth<decltype(LocalBitset)>(
				LocalBitset, static_cast<FBitWidth>(InFormat), InMask);
		}
		else
		{
			check(false);
			return typename This::FBitset(0, 0);
		}
	}

	/// @brief 数値からビット列を構築する。
	/// @return
	/// 値から構築したビット列とビット幅のペア。
	/// 構築に失敗した場合は、ビット幅が0となる。
	private: template<typename TemplateValue>
	static typename This::FBitset MakeBitsetWidth(
		/// [in] ビット列の元となる数値。
		TemplateValue const& InValue,
		/// [in] 構築するビット列の構成。
		typename This::FStatusProperty::FFormat const InFormat,
		/// [in] 指定のビット幅に収まるようマスクするか。
		bool const InMask)
	{
		using FBitWidth = typename This::FStatusChunk::FBitWidth;
		using FBitBlock = typename This::FStatusChunk::FBitBlock;
		if (InFormat == static_cast<int8>(RuleEngine::EStatusKind::Bool))
		{
			// 論理値のビット列を構築する。
			if (std::is_same<TemplateValue, bool>::value)
			{
				return typename This::FBitset(InValue != 0, 1);
			}
		}
		else if (InFormat == static_cast<int8>(RuleEngine::EStatusKind::Float))
		{
			// 浮動小数点数のビット列を構築する。
			using FFloatBitset = typename This::FFloatBitset;
			using FFloat = typename FFloatBitset::FFloat;
			return typename This::FBitset(
				FFloatBitset(static_cast<FFloat>(InValue)).Bitset,
				static_cast<FBitWidth>(sizeof(FFloat) * CHAR_BIT));
		}
		else if (InFormat < 0)
		{
			// 符号あり整数のビット列を構築する。
			using FSigned = typename This::FStatusValue::FSigned;
			return This::MakeBitsetWidth<FSigned>(
				InValue, static_cast<FBitWidth>(-InFormat), InMask);
		}
		else if (0 < InFormat)
		{
			// 符号なし整数のビット列を構築する。
			return This::MakeBitsetWidth<FBitBlock>(
				InValue, static_cast<FBitWidth>(InFormat), InMask);
		}
		else
		{
			check(false);
			return typename This::FBitset(0, 0);
		}
	}

	/// @brief 数値を整数に変換してからビット列を構築する。
	/// @return
	/// 数値から構築したビット列とビット幅のペア。
	/// 構築に失敗した場合は、ビット幅が0となる。
	private: template<typename TemplateInteger, typename TemplateValue>
	static typename This::FBitset MakeBitsetWidth(
		/// [in] ビット列の元となる数値。
		TemplateValue const& InValue,
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
		return typename This::FBitset(LocalInteger, InBitWidth);
	}

	/// @brief 論理値を整数に変換してビット列を構築させないためのダミー関数。
	private: template<typename TemplateInteger>
	static typename This::FBitset MakeBitsetWidth(
		bool, typename This::FStatusChunk::FBitWidth, bool)
	{
		// bool型の値を他の型へ変換できないようにする。
		return typename This::FBitset(0, 0);
	}

	//-------------------------------------------------------------------------
	/// @brief 符号なし整数がビット幅からあふれているか判定する。
	private: static bool IsOverflow(
		/// [in] 判定する整数。
		typename This::FStatusChunk::FBitBlock const& InInteger,
		/// [in] 許容するビット幅。
		std::size_t const InBitWidth)
	PSYQUE_NOEXCEPT
	{
		return Psyque::ShiftRightBitwise(InInteger, InBitWidth) != 0;
	}

	/// @brief 符号あり整数がビット幅からあふれているか判定する。
	private: static bool IsOverflow(
		/// [in] 判定する整数。
		typename This::FStatusValue::FSigned const& InInteger,
		/// [in] 許容するビット幅。
		std::size_t const InBitWidth)
	PSYQUE_NOEXCEPT
	{
		using FBitBlock = typename This::FStatusChunk::FBitBlock;
		auto const LocalRestBitset(
			Psyque::ShiftRightBitwiseFast(
				static_cast<FBitBlock>(InInteger), InBitWidth - 1));
		auto const LocalRestMask(
			Psyque::ShiftRightBitwiseFast(
				static_cast<FBitBlock>(
					Psyque::ShiftRightBitwiseFast(
						InInteger,
						sizeof(InInteger) * CHAR_BIT - 1)),
				InBitWidth - 1));
		return LocalRestBitset != LocalRestMask;
	}

	//-------------------------------------------------------------------------
	/// @brief 状態値ビット列チャンクの辞書。
	private: typename This::FChunkMap Chunks;
	/// @brief 状態値プロパティの辞書。
	private: typename This::FPropertyMap Properties;

}; // class Psyque::RuleEngine::_private::TReservoir

// vim: set noexpandtab:
