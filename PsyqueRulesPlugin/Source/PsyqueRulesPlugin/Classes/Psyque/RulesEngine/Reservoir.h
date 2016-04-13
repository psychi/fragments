// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::TReservoir
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
	namespace RulesEngine
	{
		namespace _private
		{
			template<typename, typename, typename, typename, typename>
				class TReservoir;

			inline FString _find_key_string(int32 const InKey)
			{
				return FName(InKey, InKey, 0).ToString();
			}
		} // namespace _private
	} // namespace RulesEngine
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
class Psyque::RulesEngine::_private::TReservoir
{
	private: using ThisClass = TReservoir; ///< @copydoc RulesEngine::TDriver::ThisClass

	//-------------------------------------------------------------------------
	/// @brief 状態値を識別するための値を表す型。
	public: using FStatusKey = TemplateStatusKey;
	/// @brief チャンクを識別するための値を表す型。
	public: using FChunkKey = TemplateChunkKey;
	/// @brief 各種コンテナに用いるメモリ割当子の型。
	public: using FAllocator = TemplateAllocator;

	//-------------------------------------------------------------------------
	/// @brief 状態値。
	public: using FStatusValue = Psyque::RulesEngine::_private::TStatusValue<
		TemplateUnsigned, TemplateFloat, uint8>;
	/// @brief 状態値が格納されているビット領域を示す。
	public: using FStatusProperty =
		Psyque::RulesEngine::_private::TStatusProperty<
			typename ThisClass::FChunkKey,
			uint32,
			typename ThisClass::FStatusValue::FBitFormat>;
	/// @brief 状態値の比較演算の引数。
	public: using FStatusComparison =
		Psyque::RulesEngine::_private::TStatusOperation<
			typename ThisClass::FStatusKey,
			RulesEngine::EStatusComparison,
			typename ThisClass::FStatusValue>;
	/// @brief 状態値の代入演算の引数。
	public: using FStatusAssignment =
		Psyque::RulesEngine::_private::TStatusOperation<
			typename ThisClass::FStatusKey,
			EPsyqueStatusAssignment,
			typename ThisClass::FStatusValue>;

	//-------------------------------------------------------------------------
	/// @brief 状態値プロパティの辞書。
	private: using FPropertyMap = std::unordered_map<
		 typename ThisClass::FStatusKey,
		 typename ThisClass::FStatusProperty,
		 Psyque::Hash::TPrimitiveBits<typename ThisClass::FStatusKey>,
		 std::equal_to<typename ThisClass::FStatusKey>,
		 typename ThisClass::FAllocator>;
	/// @brief 状態値を格納するビット領域のコンテナ。
	private: using FStatusChunk = Psyque::RulesEngine::_private::TStatusChunk<
		typename ThisClass::FStatusValue::FUnsigned,
		typename ThisClass::FStatusProperty::FBitPosition,
		typename ThisClass::FStatusValue::FBitWidth,
		typename ThisClass::FAllocator>;
	/// @brief 状態値ビット列チャンクの辞書。
	private: using FChunkMap = std::unordered_map<
		 typename ThisClass::FChunkKey,
		 typename ThisClass::FStatusChunk,
		 Psyque::Hash::TPrimitiveBits<typename ThisClass::FChunkKey>,
		 std::equal_to<typename ThisClass::FChunkKey>,
		 typename ThisClass::FAllocator>;

	//-------------------------------------------------------------------------
	/// @brief 浮動小数点数とビット列を変換する。
	private: using FFloatBitset = Psyque::TFloatBitset<TemplateFloat>;
	/// @brief ビット列とビット幅のペア。
	private: using FBitset = std::pair<
		 typename ThisClass::FStatusChunk::FBitBlock,
		 typename ThisClass::FStatusValue::FBitWidth>;

	//-------------------------------------------------------------------------
	/// @name 構築と代入
	/// @{

	/// @brief 空の状態貯蔵器を構築する。
	public: TReservoir(
		/// [in] チャンク辞書のバケット数。
		uint32 const InChunkCapacity,
		/// [in] 状態値プロパティ辞書のバケット数。
		uint32 const InStatusCapacity,
		/// [in] 使用するメモリ割当子の初期値。
		typename ThisClass::FAllocator const& InAllocator = ThisClass::FAllocator()):
	Chunks(
		InChunkCapacity,
		typename ThisClass::FChunkMap::hasher(),
		typename ThisClass::FChunkMap::key_equal(),
		InAllocator),
	Properties(
		InStatusCapacity,
		typename ThisClass::FPropertyMap::hasher(),
		typename ThisClass::FPropertyMap::key_equal(),
		InAllocator)
	{}

#ifdef PSYQUE_NO_STD_DEFAULTED_FUNCTION
	/// @brief ムーブ構築子。
	public: TReservoir(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource):
	Chunks(MoveTemp(OutSource.Chunks)),
	Properties(MoveTemp(OutSource.Properties))
	{}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: ThisClass& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource)
	{
		this->Chunks = MoveTemp(OutSource.Chunks);
		this->Properties = MoveTemp(OutSource.Properties);
		return *this;
	}
#endif // defined(PSYQUE_NO_STD_DEFAULTED_FUNCTION)

	/// @brief 状態貯蔵器で使われているメモリ割当子を取得する。
	/// @return *this で使われているメモリ割当子のコピー。
	public: typename ThisClass::FAllocator get_allocator() const PSYQUE_NOEXCEPT
	{
		return this->Properties.get_allocator();
	}

	/// @brief 状態貯蔵器を再構築する。
	public: void Rebuild(
		/// [in] 状態値ビット列チャンク辞書のバケット数。
		uint32 const InChunkCapacity,
		/// [in] 状態値プロパティ辞書のバケット数。
		uint32 const InStatusCapacity)
	{
		// 新たな辞書を用意する。
		typename ThisClass::FChunkMap LocalChunks(
			InChunkCapacity,
			this->Chunks.hash_function(),
			this->Chunks.key_eq(),
			this->Chunks.get_allocator());
		typename ThisClass::FPropertyMap LocalProperties(
			InStatusCapacity,
			this->Properties.hash_function(),
			this->Properties.key_eq(),
			this->Properties.get_allocator());

		// 現在の辞書を新たな辞書にコピーして整理する。
		ThisClass::CopyBitsets(
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

	/// @brief 論理型の状態値を登録する。
	/// @sa
	/// - ThisClass::FindStatus と
	///   ThisClass::AssignStatus で、登録した状態値にアクセスできる。
	/// - ThisClass::RemoveChunk で、登録した状態値をチャンク毎に削除できる。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	public: bool RegisterStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		typename ThisClass::FChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別値。
		typename ThisClass::FStatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値。
		bool const InValue)
	{
		// 論理型の状態値を登録する。
		using FBitFormat = typename ThisClass::FStatusValue::FBitFormat;
		return nullptr != this->RegisterBitset(
			InChunkKey,
			InStatusKey,
			InValue,
			ThisClass::FStatusValue::GetBitFormat(EPsyqueRulesStatusKind::Bool));
	}

	/// @brief 状態値を登録する。
	/// @sa
	/// - ThisClass::FindStatus と
	///   ThisClass::AssignStatus で、登録した状態値にアクセスできる。
	/// - ThisClass::RemoveChunk で、登録した状態値をチャンク毎に削除できる。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	public: template<typename TemplateValue>
	bool RegisterStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		typename ThisClass::FChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別値。
		typename ThisClass::FStatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値。以下の型の値を登録できる。
		/// - C++ 組み込み整数型。
		/// - C++ 組み込み浮動小数点数型。
		TemplateValue const InValue)
	{
		using FBitFormat = typename ThisClass::FStatusValue::FBitFormat;
		if (std::is_floating_point<TemplateValue>::value)
		{
			// 浮動小数点数型の状態値を登録する。
			/// @note コンパイル時にここで警告かエラーが発生する場合は、
			///   TemplateValue が double 型で
			///   ThisClass::FFloatBitset::FFloat が float 型なのが原因。
			typename ThisClass::FFloatBitset::FFloat const LocalFloat(InValue);
			return nullptr != this->RegisterBitset(
				InChunkKey,
				InStatusKey,
				typename ThisClass::FFloatBitset(LocalFloat).Bitset,
				ThisClass::FStatusValue::GetBitFormat(
					EPsyqueRulesStatusKind::Float));
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
	/// - ThisClass::FindStatus と
	///   ThisClass::AssignStatus で、登録した状態値にアクセスできる。
	/// - ThisClass::RemoveChunk で、登録した状態値をチャンク毎に削除できる。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	/// - InValue のビット幅が InBitWidth を超えていると失敗する。
	/// - ThisClass::FStatusChunk::BLOCK_BIT_WIDTH より
	///   InBitWidth が大きいと失敗する。
	/// - InBitWidth が2未満だと失敗する。
	///   1ビットの値は論理型として登録すること。
	public: template<typename TemplateValue>
	bool RegisterStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		typename ThisClass::FChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別番号。
		typename ThisClass::FStatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値。 C++ 組み込み整数型であること。
		TemplateValue const InValue,
		/// [in] 登録する状態値のビット幅。
		uint32 const InBitWidth)
	{
		using FBitFormat = typename ThisClass::FStatusValue::FBitFormat;
		using FBitBlock = typename ThisClass::FStatusChunk::FBitBlock;
		if(!std::is_integral<TemplateValue>::value
			|| std::is_same<bool, TemplateValue>::value
			|| ThisClass::FStatusChunk::BLOCK_BIT_WIDTH < InBitWidth
			|| InBitWidth < 2)
		{
			// 適切な整数型ではないので、登録に失敗する。
			return false;
		}
		else if (std::is_signed<TemplateValue>::value)
		{
			// 符号あり整数型の状態値を登録する。
			using FSigned = typename ThisClass::FStatusValue::FSigned;
			auto const LocalValue(static_cast<FSigned>(InValue));
			return !ThisClass::IsOverflow(LocalValue, InBitWidth)
				&& nullptr != this->RegisterBitset(
					InChunkKey,
					InStatusKey,
					Psyque::MakeBitMask<FBitBlock>(InBitWidth) & LocalValue,
					ThisClass::FStatusValue::GetBitFormat(
						EPsyqueRulesStatusKind::Signed, InBitWidth));
		}
		else
		{
			// 符号なし整数型の状態値を登録する。
			auto const LocalValue(static_cast<FBitBlock>(InValue));
			return !ThisClass::IsOverflow(LocalValue, InBitWidth)
				&& nullptr != this->RegisterBitset(
					InChunkKey,
					InStatusKey,
					LocalValue,
					ThisClass::FStatusValue::GetBitFormat(
						EPsyqueRulesStatusKind::Unsigned, InBitWidth));
		}
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値の取得
	/// @{

	/// @brief 状態値のプロパティを取得する。
	/// @return InStatusKey に対応する状態値のプロパティを指すポインタ。。
	///   該当する状態値がない場合はnullptrを返す。
	public: typename ThisClass::FStatusProperty const* FindProperty(
		/// [in] 取得する状態値プロパティに対応する識別値。
		typename ThisClass::FStatusKey const InStatusKey)
	const
	{
		auto const LocalFind(this->Properties.find(InStatusKey));
		return LocalFind != this->Properties.end()?
			&LocalFind->second: nullptr;
	}

	/// @brief 状態値の型の種別を取得する。
	/// @return
	///   InStatusKey に対応する状態値の型の種別。該当する状態値がない場合は
	///   EPsyqueRulesStatusKind::Empty を返す。
	public: EPsyqueRulesStatusKind FindKind(
		/// [in] 状態値に対応する識別値。
		typename ThisClass::FStatusKey const InStatusKey)
	const
	{
		auto const LocalFind(this->Properties.find(InStatusKey));
		return LocalFind != this->Properties.end()?
			ThisClass::FStatusValue::GetKind(LocalFind->second.GetBitFormat()):
			EPsyqueRulesStatusKind::Empty;
	}

	/// @brief 状態値のビット構成を取得する。
	/// @return InStatusKey に対応する状態値のビット構成。
	///   該当する状態値がない場合は0を返す。
	public: typename ThisClass::FStatusValue::FBitWidth FindBitFormat(
		/// [in] 状態値に対応する識別値。
		typename ThisClass::FStatusKey const InStatusKey)
	const
	{
		auto const LocalFind(this->Properties.find(InStatusKey));
		return LocalFind != this->Properties.end()?
			LocalFind->second.GetBitFormat(): 0;
	}

	/// @brief 状態値のビット幅を取得する。
	/// @return InStatusKey に対応する状態値のビット幅。
	///   該当する状態値がない場合は0を返す。
	public: typename ThisClass::FStatusValue::FBitWidth FindBitWidth(
		/// [in] 状態値に対応する識別値。
		typename ThisClass::FStatusKey const InStatusKey)
	const
	{
		return ThisClass::FStatusValue::GetBitWidth(
			this->FindBitFormat(InStatusKey));
	}

	/// @brief 状態変化フラグを取得する。
	public: EPsyqueKleene FindTransition(
		/// [in] 状態変化フラグを取得する状態値に対応する識別値。
		typename ThisClass::FStatusKey const InStatusKey)
	const
	{
		auto const LocalFind(this->Properties.find(InStatusKey));
		return LocalFind != this->Properties.end()?
			static_cast<EPsyqueKleene>(LocalFind->second.GetTransition()):
			EPsyqueKleene::TernaryUnknown;
	}

	/// @brief 状態値を取得する。
	/// @return
	///   取得した状態値。 InStatusKey に対応する状態値がない場合は、
	///   ThisClass::FStatusValue::IsEmpty が真となる値を返す。
	/// @sa
	/// - ThisClass::RegisterStatus で、状態値を登録できる。
	/// - ThisClass::AssignStatus で、状態値を書き換えできる。
	public: typename ThisClass::FStatusValue FindStatus(
		/// [in] 取得する状態値に対応する識別値。
		typename ThisClass::FStatusKey const InStatusKey)
	const
	{
		// 状態値プロパティを取得する、
		auto const LocalPropertyIterator(this->Properties.find(InStatusKey));
		if (LocalPropertyIterator == this->Properties.end())
		{
			return typename ThisClass::FStatusValue();
		}
		auto const& LocalProperty(LocalPropertyIterator->second);

		// 状態値ビット列チャンクから状態値のビット列を取得する。
		auto const LocalChunkIterator(
			this->Chunks.find(LocalProperty.GetChunkKey()));
		if (LocalChunkIterator == this->Chunks.end())
		{
			// 状態値プロパティがあれば、
			// 対応する状態値ビット列チャンクもあるはず。
			check(false);
			return typename ThisClass::FStatusValue();
		}
		auto const LocalBitFormat(LocalProperty.GetBitFormat());
		auto const LocalBitWidth(
			ThisClass::FStatusValue::GetBitWidth(LocalBitFormat));
		auto const LocalBitset(
			LocalChunkIterator->second.GetBitset(
				LocalProperty.GetBitPosition(), LocalBitWidth));

		// 状態値のビット構成から、構築する状態値の型を分ける。
		if (ThisClass::FStatusValue::IsBool(LocalBitFormat))
		{
			// 論理型の状態値を構築する。
			return typename ThisClass::FStatusValue(LocalBitset != 0);
		}
		else if (ThisClass::FStatusValue::IsFloat(LocalBitFormat))
		{
			// 浮動小数点数型の状態値を構築する。
			using FBitset = typename ThisClass::FFloatBitset::FBitset;
			return typename ThisClass::FStatusValue(
				ThisClass::FFloatBitset(static_cast<FBitset>(LocalBitset)).Float);
		}
		else if (ThisClass::FStatusValue::IsUnsigned(LocalBitFormat))
		{
			// 符号なし整数型の状態値を構築する。
			return typename ThisClass::FStatusValue(LocalBitset);
		}
		else if (ThisClass::FStatusValue::IsSigned(LocalBitFormat))
		{
			// 符号あり整数型の状態値を構築する。
			using FSigned = typename ThisClass::FStatusValue::FSigned;
			auto const LocalRestBitWidth(
				ThisClass::FStatusChunk::BLOCK_BIT_WIDTH - LocalBitWidth);
			return typename ThisClass::FStatusValue(
				Psyque::ShiftRightBitwiseFast(
					Psyque::ShiftLeftBitwiseFast(
						static_cast<FSigned>(LocalBitset), LocalRestBitWidth),
					LocalRestBitWidth));
		}
		else
		{
			// 空の状態値は登録できないはず。
			check(false);
			return typename ThisClass::FStatusValue();
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
	public: EPsyqueKleene CompareStatus(
		/// [in] 状態値の比較式。
		typename ThisClass::FStatusComparison const& InComparison)
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
			static_cast<typename ThisClass::FStatusKey>(
				*LocalRightKeyPointer));
		if (LocalRightKey == *LocalRightKeyPointer)
		{
			return this->CompareStatus(
				InComparison.GetKey(),
				InComparison.GetOperator(),
				LocalRightKey);
		}
		return EPsyqueKleene::TernaryUnknown;
	}

	/// @brief 状態値を比較する。
	/// @retval 正 比較式の評価は真。
	/// @retval 0  比較式の評価は偽。
	/// @retval 負 比較式の評価に失敗。
	public: EPsyqueKleene CompareStatus(
		/// [in] 左辺となる状態値の識別値。
		typename ThisClass::FStatusKey const InLeftKey,
		/// [in] 適用する比較演算子。
		RulesEngine::EStatusComparison const InOperator,
		/// [in] 右辺となる値。
		typename ThisClass::FStatusValue const& InRightValue)
	const
	{
		return this->FindStatus(InLeftKey).Compare(InOperator, InRightValue);
	}

	/// @brief 状態値を比較する。
	public: EPsyqueKleene CompareStatus(
		/// [in] 左辺となる状態値の識別値。
		typename ThisClass::FStatusKey const InLeftKey,
		/// [in] 適用する比較演算子。
		RulesEngine::EStatusComparison const InOperator,
		/// [in] 右辺となる状態値の識別値。
		typename ThisClass::FStatusKey const InRightKey)
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
	///   ThisClass::RegisterStatus で登録されてないと、失敗する。
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
	/// @sa ThisClass::FindStatus で、代入した値を取得できる。
	public: template<typename TemplateValue>
	bool AssignStatus(
		/// [in] 代入先となる状態値に対応する識別値。
		typename ThisClass::FStatusKey const InStatusKey,
		/// [in] 状態値へ代入する値。以下の型の値を代入できる。
		/// - bool 型。
		/// - C++ 組み込み整数型。
		/// - C++ 組み込み浮動小数点数型。
		/// - ThisClass::FStatusValue 型。
		TemplateValue const& InValue)
	{
		auto const LocalPropertyIterator(this->Properties.find(InStatusKey));
		if (LocalPropertyIterator == this->Properties.end())
		{
			return false;
		}
		/// @note InValue が状態値のビット幅を超えている場合、
		/// 失敗となるように実装しておく。
		/// 失敗とせず、ビット列をマスクして代入する実装も可能。どちらが良い？
		auto const LocalMask(false);
		auto& LocalProperty(LocalPropertyIterator->second);
		return ThisClass::AssignBitset(
			LocalProperty,
			this->Chunks,
			ThisClass::MakeBitsetWidth(
				InValue, LocalProperty.GetBitFormat(), LocalMask));
	}

	/// @brief 状態値を演算し、結果を代入する。
	/// @retval true  成功。演算結果を状態値へ代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// 失敗する要因は ThisClass::AssignStatus を参照。
	public: bool AssignStatus(
		/// [in] 状態値の代入演算。
		typename ThisClass::FStatusAssignment const& InAssignment)
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
			static_cast<typename ThisClass::FStatusKey>(
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
	/// 失敗する要因は ThisClass::AssignStatus を参照。
	public: bool AssignStatus(
		/// [in] 代入演算子の左辺となる状態値の識別値。
		typename ThisClass::FStatusKey const InLeftKey,
		/// [in] 適用する代入演算子。
		EPsyqueStatusAssignment const InOperator,
		/// [in] 代入演算子の右辺となる値。
		typename ThisClass::FStatusValue const& InRightValue)
	{
		if (InOperator == EPsyqueStatusAssignment::Copy)
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
	/// 失敗する要因は ThisClass::AssignStatus を参照。
	public: bool AssignStatus(
		/// [in] 代入演算子の左辺となる状態値の識別値。
		typename ThisClass::FStatusKey const InLeftKey,
		/// [in] 適用する代入演算子。
		EPsyqueStatusAssignment const InOperator,
		/// [in] 代入演算子の右辺となる状態値の識別値。
		typename ThisClass::FStatusKey const InRightKey)
	{
		return this->AssignStatus(
			InLeftKey, InOperator, this->FindStatus(InRightKey));
	}

	/// @brief 状態変化フラグを初期化する。
	/// @warning Psyque::RulesEngine 管理者以外は、この関数は使用禁止。
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
		typename ThisClass::FChunkKey const InChunkKey,
		/// [in] 予約するビット列コンテナの容量。
		uint32 const InBlockCapacity,
		/// [in] 予約する空きビット領域コンテナの容量。
		uint32 const InEmptyCapacity)
	{
		// 状態値を登録する状態値ビット列チャンクを用意する。
		auto const LocalEmplace(
			this->Chunks.emplace(
				InChunkKey,
				typename ThisClass::FChunkMap::mapped_type(
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
		typename ThisClass::FChunkKey const InChunkKey)
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
	public: typename ThisClass::FStatusChunk::FBitBlockArray serialize_chunk(
		/// [in] シリアル化する状態値ビット列チャンクの識別番号。
		typename ThisClass::FChunkKey const InChunkKey)
	const;

	/// @brief シリアル化された状態値ビット列チャンクを復元する。
	/// @todo 未実装。
	public: bool deserialize_chunk(
		/// [in] 復元する状態値ビット列チャンクの識別値。
		typename ThisClass::FChunkKey const InChunkKey,
		/// [in] シリアル化された状態値ビット列チャンク。
		typename ThisClass::FStatusChunk::FBitBlockArray const&
			in_serialized_chunk);
	/// @}
	//-------------------------------------------------------------------------
	/// @brief 状態値を登録する。
	/// @return
	/// 登録した状態値のプロパティを指すポインタ。
	/// 登録に失敗した場合は nullptr を返す。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	/// @sa
	/// - ThisClass::FindStatus と ThisClass::AssignStatus
	///   で、登録した状態値にアクセスできる。
	/// - ThisClass::RemoveChunk で、登録した状態値をチャンク毎に削除できる。
	private: typename ThisClass::FStatusProperty const* RegisterBitset(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		typename ThisClass::FChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別番号。
		typename ThisClass::FStatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値となるビット列。
		typename ThisClass::FStatusChunk::FBitBlock const InBitset,
		/// [in] 登録する状態値のビット構成。
		typename ThisClass::FStatusValue::FBitFormat const InBitFormat)
	{
		// 状態値を登録する状態値ビット列チャンクを用意する。
		auto const LocalEmplace(
			this->Chunks.emplace(
				InChunkKey,
				typename ThisClass::FChunkMap::mapped_type(
					this->Chunks.get_allocator())));
		auto& LocalChunk(*LocalEmplace.first);

		// 状態値を登録する。
		auto const LocalProperty(
			ThisClass::AllocateBitset(
				this->Properties, LocalChunk, InStatusKey, InBitFormat));

		// 状態値に初期値を設定する。
		if (LocalProperty != nullptr
			&& EPsyqueKleene::TernaryUnknown != LocalChunk.second.SetBitset(
				LocalProperty->second.GetBitPosition(),
				ThisClass::FStatusValue::GetBitWidth(InBitFormat),
				InBitset))
		{
			return &LocalProperty->second;
		}
		return nullptr;
	}

	/// @brief 状態値を登録する。
	/// @return
	///   登録した状態値のプロパティを指すポインタ。
	///   状態値を登録できなかった場合は nullptr を返す。
	///   - InStatusKey に対応する状態値がすでに追加されていると失敗する。
	private: static typename ThisClass::FPropertyMap::value_type* AllocateBitset(
		/// [in,out] 状態値を登録する状態値プロパティの辞書。
		typename ThisClass::FPropertyMap& OutProperties,
		/// [in,out] 状態値を登録する状態値ビット列チャンク。
		typename ThisClass::FChunkMap::value_type& OutChunk,
		/// [in] 登録する状態値に対応する識別値。
		typename ThisClass::FStatusKey const InStatusKey,
		/// [in] 登録する状態値のビット構成。
		typename ThisClass::FStatusValue::FBitFormat const InBitFormat)
	{
		if (!ThisClass::FStatusValue::IsEmpty(InBitFormat))
		{
			// 状態値のビット領域を生成する。
			auto const LocalBitWidth(
				ThisClass::FStatusValue::GetBitWidth(InBitFormat));
			auto const LocalBitPosition(
				OutChunk.second.AllocateBitset(LocalBitWidth));
			if (LocalBitPosition != ThisClass::FStatusChunk::INVALID_BIT_POSITION)
			{
				// 状態値プロパティを生成する。
				auto const LocalEmplace(
					OutProperties.emplace(
						InStatusKey,
						typename ThisClass::FPropertyMap::mapped_type(
							OutChunk.first, LocalBitPosition, InBitFormat)));
				if (LocalEmplace.second)
				{
					auto& LocalProperty(*LocalEmplace.first);
					return &LocalProperty;
				}

				// 同じ名前ハッシュ値の状態値がすでに登録されていた。
				UE_LOG(
					LogPsyqueRulesEngine,
					Warning,
					TEXT(
						"TReservoir::AllocateBitset is failed."
						"\n\tStatus key '%s' is already registered."),
					*_find_key_string(InStatusKey));
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
	private: static bool AssignBitset(
		/// [in,out] 代入先となる状態値のプロパティ。
		typename ThisClass::FStatusProperty& OutProperty,
		/// [in,out] 状態値ビット列チャンクのコンテナ。
		typename ThisClass::FChunkMap& OutChunks,
		/// [in] 代入する状態値のビット列とビット幅。
		typename ThisClass::FBitset const& InBitset)
	{
		if (0 < InBitset.second)
		{
			// 状態値にビット列を設定する。
			auto const LocalChunkIterator(
				OutChunks.find(OutProperty.GetChunkKey()));
			if (LocalChunkIterator != OutChunks.end())
			{
				auto const LocalSetBitset(
					LocalChunkIterator->second.SetBitset(
						OutProperty.GetBitPosition(),
						InBitset.second,
						InBitset.first));
				if (LocalSetBitset != EPsyqueKleene::TernaryUnknown)
				{
					if (LocalSetBitset == EPsyqueKleene::TernaryTrue)
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
		typename ThisClass::FPropertyMap& OutProperties,
		/// [in,out] コピー先となる状態値ビット列チャンク辞書。
		typename ThisClass::FChunkMap& OutChunks,
		/// [in] コピー元となる状態値プロパティ辞書。
		typename ThisClass::FPropertyMap const& InProperties,
		/// [in] コピー元となる状態値ビット列チャンク辞書。
		typename ThisClass::FChunkMap const& InChunks)
	{
		// 状態値プロパティのポインタのコンテナを構築する。
		using FPropertyArray = std::vector<
			std::pair<
				typename ThisClass::FStatusChunk::FBitWidth,
				typename ThisClass::FPropertyMap::value_type const*>,
			typename ThisClass::FAllocator>;
		FPropertyArray LocalProperties(InProperties.get_allocator());
		LocalProperties.reserve(InProperties.size());
		for (auto& LocalProperty: InProperties)
		{
			LocalProperties.emplace_back(
				ThisClass::FStatusValue::GetBitWidth(
					LocalProperty.second.GetBitFormat()),
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
			ThisClass::CopyBitset(
				OutProperties, OutChunks, *LocalProperty.second, InChunks);
		}
	}

	/// @brief 状態値をコピーする。
	private: static void CopyBitset(
		///[in,out] コピー先となる状態値プロパティ辞書。
		typename ThisClass::FPropertyMap& OutProperties,
		/// [in,out] コピー先となる状態値ビット列チャンク辞書。
		typename ThisClass::FChunkMap& OutChunks,
		/// [in] コピー元となる状態値プロパティ。
		typename ThisClass::FPropertyMap::value_type const& InProperty,
		/// [in] コピー元となる状態値ビット列チャンク辞書。
		typename ThisClass::FChunkMap const& InChunks)
	{
		// コピー元となる状態値ビット列チャンクを取得する。
		auto const LocalSourceChunk_iterator(
			InChunks.find(InProperty.second.GetChunkKey()));
		if (LocalSourceChunk_iterator == InChunks.end())
		{
			check(false);
			return;
		}
		auto const& LocalSourceChunk(LocalSourceChunk_iterator->second);

		// コピー先となる状態値を用意する。
		auto const LocalEmplace(
			OutChunks.emplace(
				InProperty.second.GetChunkKey(),
				typename ThisClass::FChunkMap::mapped_type(
					OutChunks.get_allocator())));
		auto& LocalTargetChunk(*LocalEmplace.first);
		if (LocalEmplace.second)
		{
			LocalTargetChunk.second.BitBlocks.reserve(
				LocalSourceChunk.BitBlocks.size());
			LocalTargetChunk.second.EmptyBitsets.reserve(
				LocalSourceChunk.EmptyBitsets.size());
		}
		auto const LocalBitFormat(InProperty.second.GetBitFormat());
		auto const LocalTargetProperty(
			ThisClass::AllocateBitset(
				OutProperties,
				LocalTargetChunk,
				InProperty.first,
				LocalBitFormat));
		if (LocalTargetProperty == nullptr)
		{
			check(false);
			return;
		}

		// 状態値のビット領域をコピーする。
		auto const LocalBitWidth(
			ThisClass::FStatusValue::GetBitWidth(LocalBitFormat));
		LocalTargetChunk.second.SetBitset(
			LocalTargetProperty->second.GetBitPosition(),
			LocalBitWidth,
			LocalSourceChunk.GetBitset(
				InProperty.second.GetBitPosition(), LocalBitWidth));
		LocalTargetProperty->second.SetTransition(
			InProperty.second.GetTransition());
	}

	//-------------------------------------------------------------------------
	/// @brief 数値からビット列を構築する。
	/// @return
	/// 値から構築したビット列とビット幅のペア。
	/// 構築に失敗した場合は、ビット幅が0となる。
	private: static typename ThisClass::FBitset MakeBitsetWidth(
		/// [in] ビット列の元となる数値。
		typename ThisClass::FStatusValue const& InValue,
		/// [in] 構築するビット列の構成。
		typename ThisClass::FStatusValue::FBitFormat const InBitFormat,
		/// [in] 指定のビット幅に収まるようマスクするか。
		bool const InMask)
	{
		// 入力値のビット列を取得する。
		auto const LocalKind(ThisClass::FStatusValue::GetKind(InBitFormat));
		typename ThisClass::FStatusChunk::FBitBlock LocalBitset;
		if (LocalKind != InValue.GetKind())
		{
			typename ThisClass::FStatusValue const
				LocalValue(InValue, LocalKind);
			if (LocalValue.IsEmpty())
			{
				return typename ThisClass::FBitset(0, 0);
			}
			LocalBitset = LocalValue.GetBitset();
		}
		else
		{
			LocalBitset = InValue.GetBitset();
		}

		// ビット列とビット幅を構築する。
		auto const LocalBitWidth(
			ThisClass::FStatusValue::GetBitWidth(InBitFormat));
		switch (LocalKind)
		{
			case EPsyqueRulesStatusKind::Bool:
			case EPsyqueRulesStatusKind::Float:
			return typename ThisClass::FBitset(LocalBitset, LocalBitWidth);

			case EPsyqueRulesStatusKind::Unsigned:
			return ThisClass::MakeIntegerBitsetWidth<decltype(LocalBitset)>(
				LocalBitset, LocalBitWidth, InMask);

			case EPsyqueRulesStatusKind::Signed:
			using FSigned = typename ThisClass::FStatusValue::FSigned;
			return ThisClass::MakeIntegerBitsetWidth<FSigned>(
				LocalBitset, LocalBitWidth, InMask);

			default:
			check(false);
			return typename ThisClass::FBitset(0, 0);
		}
	}

	/// @brief 数値からビット列を構築する。
	/// @return
	///   値から構築したビット列とビット幅のペア。
	///   構築に失敗した場合は、ビット幅が0となる。
	private: template<typename TemplateValue>
	static typename ThisClass::FBitset MakeBitsetWidth(
		/// [in] ビット列の元となる数値。
		TemplateValue const& InValue,
		/// [in] 構築するビット列の構成。
		typename ThisClass::FStatusValue::FBitFormat const InBitFormat,
		/// [in] 指定のビット幅に収まるようマスクするか。
		bool const InMask)
	{
		auto const LocalBitWidth(
			ThisClass::FStatusValue::GetBitWidth(InBitFormat));
		if (ThisClass::FStatusValue::IsUnsigned(InBitFormat))
		{
			// 符号なし整数のビット列を構築する。
			using FUnsigned= typename ThisClass::FStatusValue::FUnsigned;
			return ThisClass::MakeIntegerBitsetWidth<FUnsigned>(
				InValue, LocalBitWidth, InMask);
		}
		else if (ThisClass::FStatusValue::IsSigned(InBitFormat))
		{
			// 符号あり整数のビット列を構築する。
			using FSigned = typename ThisClass::FStatusValue::FSigned;
			return ThisClass::MakeIntegerBitsetWidth<FSigned>(
				InValue, LocalBitWidth, InMask);
		}
		else if (ThisClass::FStatusValue::IsFloat(InBitFormat))
		{
			// 浮動小数点数のビット列を構築する。
			using FFloatBitset = typename ThisClass::FFloatBitset;
			using FFloat = typename FFloatBitset::FFloat;
			return typename ThisClass::FBitset(
				FFloatBitset(static_cast<FFloat>(InValue)).Bitset,
				LocalBitWidth);
		}
		else if (std::is_same<TemplateValue, bool>::value)
		{
			// 論理値のビット列を構築する。
			return typename ThisClass::FBitset(InValue != 0, LocalBitWidth);
		}
		check(false);
		return typename ThisClass::FBitset(0, 0);
	}

	/// @brief 数値を整数に変換してからビット列を構築する。
	/// @return
	///   数値から構築したビット列とビット幅のペア。
	///   構築に失敗した場合は、ビット幅が0となる。
	private: template<typename TemplateInteger, typename TemplateValue>
	static typename ThisClass::FBitset MakeIntegerBitsetWidth(
		/// [in] ビット列の元となる数値。
		TemplateValue const& InValue,
		/// [in] 構築するビット列の幅。
		typename ThisClass::FStatusValue::FBitWidth InBitWidth,
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
		else if (ThisClass::IsOverflow(LocalInteger, InBitWidth))
		{
			InBitWidth = 0;
		}
		return typename ThisClass::FBitset(LocalInteger, InBitWidth);
	}

	/// @brief 論理値を整数に変換してビット列を構築させないためのダミー関数。
	private: template<typename TemplateInteger>
	static typename ThisClass::FBitset MakeIntegerBitsetWidth(
		bool, typename ThisClass::FStatusValue::FBitWidth, bool)
	{
		// bool型の値を他の型へ変換できないようにする。
		return typename ThisClass::FBitset(0, 0);
	}

	//-------------------------------------------------------------------------
	/// @brief 符号なし整数がビット幅からあふれているか判定する。
	private: static bool IsOverflow(
		/// [in] 判定する整数。
		typename ThisClass::FStatusChunk::FBitBlock const InInteger,
		/// [in] 許容するビット幅。
		uint32 const InBitWidth)
	PSYQUE_NOEXCEPT
	{
		return Psyque::ShiftRightBitwise(InInteger, InBitWidth) != 0;
	}

	/// @brief 符号あり整数がビット幅からあふれているか判定する。
	private: static bool IsOverflow(
		/// [in] 判定する整数。
		typename ThisClass::FStatusValue::FSigned const InInteger,
		/// [in] 許容するビット幅。
		uint32 const InBitWidth)
	PSYQUE_NOEXCEPT
	{
		using FBitBlock = typename ThisClass::FStatusChunk::FBitBlock;
		auto const LocalRestBitset(
			Psyque::ShiftRightBitwiseFast(
				static_cast<FBitBlock>(InInteger), InBitWidth - 1));
		auto const LocalRestMask(
			Psyque::ShiftRightBitwiseFast(
				static_cast<FBitBlock>(
					Psyque::ShiftRightBitwiseFast(
						InInteger, sizeof(InInteger) * CHAR_BIT - 1)),
				InBitWidth - 1));
		return LocalRestBitset != LocalRestMask;
	}

	//-------------------------------------------------------------------------
	/// @brief 状態値ビット列チャンクの辞書。
	private: typename ThisClass::FChunkMap Chunks;
	/// @brief 状態値プロパティの辞書。
	private: typename ThisClass::FPropertyMap Properties;

}; // class Psyque::RulesEngine::_private::TReservoir

// vim: set noexpandtab:
