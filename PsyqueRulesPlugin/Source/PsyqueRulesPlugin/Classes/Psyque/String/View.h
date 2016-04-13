// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @copydoc Psyque::String::FView
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <algorithm>
#include <functional>

namespace Psyque
{
	/// @brief 動的メモリ割当てを抑制した文字列。
	namespace String
	{
		/// @cond
		class FView;
		/// @endcond
	} // namespace String
} // namespace Psyque

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief FString を模した、immutableな文字列への参照。
/// @par
///   - 動的メモリ割当てを一切行わない。
///   - 文字列の終端が空文字となっている保証はない。
/// @warning
///   文字の配列を単純にconst参照しているので、
///   参照してる文字の配列が変更／破壊されると、動作を保証できなくなる。
///   ローカル変数や引数などの、一時的なインスタンスとして使うこと。
///   クラスのメンバ変数など、恒久的に保持するインスタンスとして使わないこと。
class Psyque::String::FView
{
	/// @brief thisが指す値の型。
	private: using ThisClass = FView;

	//-------------------------------------------------------------------------
	/// @brief 小文字に変換して等価比較する関数オブジェクト。
	private: struct EqualLower
	{
		bool operator()(
			TCHAR const InLeft,
			TCHAR const InRight)
		const PSYQUE_NOEXCEPT
		{
			return TChar<TCHAR>::ToLower(InLeft)
				== TChar<TCHAR>::ToLower(InRight);
		}
	};

	//-------------------------------------------------------------------------
	/// @name 構築と代入
	/// @{

	/// @brief 空の文字列を構築する。
	public: PSYQUE_CONSTEXPR FView()
	PSYQUE_NOEXCEPT: Data(nullptr), Length(0)
	{}

	/// @brief 文字列を参照する。
	public: PSYQUE_CONSTEXPR FView(
		/// [in] 参照する文字列。
		ThisClass const& InString)
	= default;

	/// @brief 文字列を参照する。
	public: PSYQUE_CONSTEXPR FView(
		///[in] 参照する文字列。
		ThisClass&& InString)
	= default;

	/// @brief 文字列を参照する。
	public: FView(FString const& InString)
	PSYQUE_NOEXCEPT:
	Data(InString.IsEmpty()? nullptr: *InString),
	Length(InString.Len())
	{}

	/// @brief 文字列リテラルを参照する。
	public: template<std::size_t TemplateLen> PSYQUE_CONSTEXPR FView(
		/// [in] 参照する文字列リテラル。
		TCHAR const (&InLiteral)[TemplateLen])
	PSYQUE_NOEXCEPT:
	Data((
		PSYQUE_ASSERT(
			TemplateLen <= MAX_int32
			&& 1 <= TemplateLen
			&& InLiteral[TemplateLen - 1] == 0),
		&InLiteral[0])),
	Length(static_cast<int32>(TemplateLen - 1))
	{}

	/// @brief 文字列を参照する。
	public: PSYQUE_CONSTEXPR FView(
		/// [in] 参照する文字列の要素数。 INDEX_NONE
		/// が指定された場合は、null文字を検索し要素数を自動決定する。
		int32 const InLength,
		/// [in] 参照する文字列の先頭位置。
		TCHAR const* const InData)
	PSYQUE_NOEXCEPT:
	Data(InData),
	Length(ThisClass::CountLength(InLength, InData))
	{}

	/// @brief コピー代入演算子。
	public: ThisClass& operator=(
		/// [in] コピー元となる文字列。
		ThisClass const& InString)
	PSYQUE_NOEXCEPT = default;

	/// @brief コピー代入演算子。
	public: ThisClass& operator=(
		/// [in] ムーブ元となる文字列。
		ThisClass&& OutString)
	PSYQUE_NOEXCEPT = default;
	/// @}
	//-------------------------------------------------------------------------
	/// @name 文字列のプロパティ
	/// @{

	/// @brief 空文字列か判定する。
	public: PSYQUE_CONSTEXPR bool IsEmpty() const PSYQUE_NOEXCEPT
	{
		return this->Len() <= 0;
	}

	/// @brief 数値の文字列か判定する。
	public: bool IsNumeric() const PSYQUE_NOEXCEPT
	{
		bool LocalHasDot(false);
		for (int32 i(0); i < this->Len(); ++i)
		{
			if (!TChar<TCHAR>::IsDigit(this->GetData()[i]))
			{
				if (LocalHasDot || this->GetData()[i] != '.')
				{
					return false;
				}
				LocalHasDot = true;
			}
		}
		return true;
	}

	/// @brief 文字列の要素数を取得する。
	public: PSYQUE_CONSTEXPR int32 Len() const PSYQUE_NOEXCEPT
	{
		return this->Length;
	}

	/// @brief 有効なインデクス番号か判定する。
	public: PSYQUE_CONSTEXPR bool IsValidIndex(
		/// [in] 判定するインデクス番号。
		int32 const InIndex)
	const PSYQUE_NOEXCEPT
	{
		return 0 <= InIndex && InIndex < this->Len();
	}

	/// @brief 文字列の先頭位置を取得する。
	public: PSYQUE_CONSTEXPR TCHAR const* operator*() const PSYQUE_NOEXCEPT
	{
		return this->GetData();
	}

	/// @brief 文字を参照する。
	public: PSYQUE_CONSTEXPR TCHAR const& operator[](
		/// [in] 取得する文字のインデクス番号。
		int32 const InIndex)
	const PSYQUE_NOEXCEPT
	{
		return this->IsValidIndex(InIndex)?
			this->GetData()[InIndex]:
			(PSYQUE_ASSERT(false), *static_cast<TCHAR*>(nullptr));
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 文字列の比較
	/// @{

	/// @brief 文字列を等価比較する。
	/// @details *thisを左辺として、右辺の文字列と等価比較する。
	/// @retval true  左辺と右辺は等価。
	/// @retval false 左辺と右辺は非等価。
	public: bool operator==(
		/// [in] 右辺の文字列。
		ThisClass const& InRight)
	const PSYQUE_NOEXCEPT
	{
		return this->Equals(InRight);
	}

	/// @brief 文字列を非等価比較する。
	/// @details *thisを左辺として、右辺の文字列と等価比較する。
	/// @retval true  左辺と右辺は非等価。
	/// @retval false 左辺と右辺は等価。
	public: bool operator!=(
		/// [in] 右辺の文字列。
		ThisClass const& InRight)
	const PSYQUE_NOEXCEPT
	{
		return !this->Equals(InRight);
	}

	/// @brief 文字列を比較する。
	/// @details *thisを左辺として、右辺の文字列と比較する。
	/// @retval true  *this <  InRight
	/// @retval false *this >= InRight
	public: bool operator<(ThisClass const& InRight) const PSYQUE_NOEXCEPT
	{
		return this->Compare(InRight) < 0;
	}

	/// @brief 文字列を比較する。
	/// @details *thisを左辺として、右辺の文字列と比較する。
	/// @retval true  *this <= InRight
	/// @retval false *this > InRight
	public: bool operator<=(ThisClass const& InRight) const PSYQUE_NOEXCEPT
	{
		return this->Compare(InRight) <= 0;
	}

	/// @brief 文字列を比較する。
	/// @details *thisを左辺として、右辺の文字列と比較する。
	/// @retval true  *this >  InRight
	/// @retval false *this <= InRight
	public: bool operator>(ThisClass const& InRight) const PSYQUE_NOEXCEPT
	{
		return 0 < this->Compare(InRight);
	}

	/// @brief 文字列を比較する。
	/// @details *thisを左辺として、右辺の文字列と比較する。
	/// @retval true  *this >= InRight
	/// @retval false *this <  InRight
	public: bool operator>=(ThisClass const& InRight) const PSYQUE_NOEXCEPT
	{
		return 0 <= this->Compare(InRight);
	}

	/// @brief 文字列を等価比較する。
	/// @details *thisを左辺として、右辺の文字列と等価比較する。
	/// @retval true  左辺と右辺は等価。
	/// @retval false 左辺と右辺は非等価。
	public: bool Equals(
		/// [in] 右辺の文字列。
		ThisClass const& InRight,
		/// [in] 大文字／小文字の区別。
		ESearchCase::Type const InSearchCase = ESearchCase::CaseSensitive)
	const PSYQUE_NOEXCEPT
	{
		return this->Len() == InRight.Len() && 0 == (
			InSearchCase == ESearchCase::CaseSensitive?
				TCString<TCHAR>::Strncmp(
					this->GetData(), InRight.GetData(), this->Len()):
				TCString<TCHAR>::Strnicmp(
					this->GetData(), InRight.GetData(), this->Len()));
	}

	/// @brief 文字列を比較する。
	/// @details *thisを左辺として、右辺の文字列と比較する。
	/// @retval 負 右辺のほうが大きい。
	/// @retval 正 左辺のほうが大きい。
	/// @retval 0  左辺と右辺は等価。
	public: int32 Compare(
		/// [in] 右辺の文字列。
		ThisClass const& InRight,
		/// [in] 大文字／小文字の区別。
		ESearchCase::Type const InSearchCase = ESearchCase::CaseSensitive)
	const PSYQUE_NOEXCEPT
	{
		auto const LocalDiff(this->Len() - InRight.Len());
		auto const LocalLength(LocalDiff < 0? this->Len(): InRight.Len());
		auto const LocalCompare(
			InSearchCase == ESearchCase::CaseSensitive?
				TCString<TCHAR>::Strncmp(
					this->GetData(), InRight.GetData(), LocalLength):
				TCString<TCHAR>::Strnicmp(
					this->GetData(), InRight.GetData(), LocalLength));
		return LocalCompare != 0? LocalCompare: LocalDiff;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 文字列の検索
	/// @{

	/// @brief 文字列から部分文字列を検索する。
	/// @return InSubString が始まるインデクス番号。ただし
	///   INDEX_NONE が戻ってきた場合は、含まれていなかった。
	public: int32 Find(
		/// [in] 検索する部分文字列。
		ThisClass const& InSubString,
		/// [in] 大文字・小文字を区別するか。
		ESearchCase::Type const InSearchCase = ESearchCase::IgnoreCase,
		/// [in] 検索を開始する方向。
		ESearchDir::Type const InSearchDir = ESearchDir::FromStart,
		/// [in] 検索を開始する位置。
		int32 const InStartIndex = INDEX_NONE)
	const
	{
		if (InSubString.IsEmpty())
		{
			return INDEX_NONE;
		}
		if (InSearchDir == ESearchDir::FromStart)
		{
			auto const LocalStart(
				InStartIndex != INDEX_NONE?
					FMath::Clamp(InStartIndex, 0, this->Len() - 1): 0);
			return InSearchCase == ESearchCase::IgnoreCase?
				this->PredicatedFind(
					InSubString.GetData(),
					LocalStart,
					InSubString.Len(),
					ThisClass::EqualLower()):
				this->PredicatedFind(
					InSubString.GetData(),
					LocalStart,
					InSubString.Len(),
					std::equal_to<TCHAR>());
		}
		else
		{
			auto const LocalStart(
				InStartIndex != INDEX_NONE? InStartIndex: this->Len());
			return InSearchCase == ESearchCase::IgnoreCase?
				this->PredicatedFindReverse(
					InSubString.GetData(),
					LocalStart,
					InSubString.Len(),
					ThisClass::EqualLower()):
				this->PredicatedFindReverse(
					InSubString.GetData(),
					LocalStart,
					InSubString.Len(),
					std::equal_to<TCHAR>());
		}
	}

	/// @brief 文字列から部分文字列を検索する。
	/// @return *this に InSubString が 含まれているか否か。
	public: bool Contains(
		/// [in] 検索する部分文字列。
		ThisClass const& InSubString,
		/// [in] 大文字・小文字を区別するか否か。
		ESearchCase::Type const InSearchCase = ESearchCase::IgnoreCase,
		/// [in] 検索を開始する方向。
		ESearchDir::Type const InSearchDir = ESearchDir::FromStart)
	const
	{
		return this->Find(InSubString, InSearchCase, InSearchDir) != INDEX_NONE;
	}

	/// @brief 文字列の先頭から文字を検索する。
	/// @return 文字が見つかったか否か。
	public: bool FindChar(
		/// [in] 検索する文字。
		TCHAR const InChar,
		/// [out] 検索した文字のインデクス番号の格納先。見つからなかった場合は
		/// INDEX_NONE となる。
		int32& OutIndex)
	const
	{
		OutIndex = this->FindFirstOf(InChar);
		return OutIndex != INDEX_NONE;
	}

	/// @brief 文字列の末尾から文字を検索する。
	/// @return 文字が見つかったか否か。
	public: bool FindLastChar(
		/// [in] 検索する文字。
		TCHAR const InChar,
		/// [out] 検索した文字のインデクス番号の格納先。見つからなかった場合は
		/// INDEX_NONE となる。
		int32& OutIndex)
	const
	{
		OutIndex = this->FindLastOf(InChar);
		return OutIndex != INDEX_NONE;
	}

	/// @brief 文字列の末尾から文字を検索する。
	/// @return 発見した文字のインデクス番号。見つからなかった場合は
	/// INDEX_NONE を戻す。
	public: template <typename TemplatePredicate>
	int32 FindLastCharByPredicate(
		/// [in] Predicate that takes TCHAR and returns true
		/// if TCHAR matches search criteria, false otherwise.
		TemplatePredicate InPredicate,
		/// [in] Index of element from which to start searching.
		/// Defaults to last TCHAR in string.
		int32 const InStartIndex)
	const
	{
		check(0 <= InStartIndex && InStartIndex <= this->Len());
		return ThisClass::PredicatedFindLastOf(
			this->GetData() + InStartIndex,
			this->Len(),
			0,
			[&InPredicate](TCHAR const InLeft, TCHAR const)->bool
			{
				return InPredicate(InLeft);
			});
	}

	/// @copydoc ThisClass::FindLastCharByPredicate
	public: template <typename TemplatePredicate>
	int32 FindLastCharByPredicate(
		/// [in] Predicate that takes TCHAR and returns true
		/// if TCHAR matches search criteria, false otherwise.
		TemplatePredicate InPredicate)
	const
	{
		return this->FindLastCharByPredicate(
			MoveTemp(InPredicate), this->Len());
	}

	/// @brief 指定された文字を含む位置を、前から検索する。
	/// @return 発見した文字のインデクス番号。見つからなかった場合は
	/// INDEX_NONE を戻す。
	public: int32 FindFirstOf(
		/// [in] 検索する文字。
		TCHAR const InChar)
	const
	{
		return ThisClass::PredicatedFindFirstOf(
			this->GetData(), this->Len(), InChar, std::equal_to<TCHAR>());
	}

	/// @brief 指定された文字を含む位置を、後ろから検索する。
	/// @return 発見した文字のインデクス番号。見つからなかった場合は
	/// INDEX_NONE を戻す。
	public: int32 FindLastOf(
		/// [in] 検索する文字。
		TCHAR const InChar)
	const
	{
		return ThisClass::PredicatedFindLastOf(
			this->GetData(), this->Len(), InChar, std::equal_to<TCHAR>());
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 文字列の構築
	/// @{

	/// @return the left most given number of characters
	public: ThisClass Left(int32 const InCount) const
	{
		return ThisClass(
			FMath::Clamp(InCount, 0, this->Len()), this->GetData());
	}

	/// @return the left most characters from the string
	///   chopping the given number of characters from the end.
	public: ThisClass LeftChop(int32 const InCount ) const
	{
		return ThisClass(
			FMath::Clamp(this->Len() - InCount, 0, this->Len()),
			this->GetData());
	}

	/// @return the string to the right of the specified location,
	///   counting back from the right (end of the word).
	public: ThisClass Right(int32 const InCount) const
	{
		auto const LocalLength(FMath::Clamp(InCount, 0, this->Len()));
		return ThisClass(
			LocalLength, this->GetData() + this->Len() - LocalLength);
	}

	/// @return the string to the right of the specified location,
	///   counting forward from the left (from the beginning of the word).
	public: ThisClass RightChop(int32 const InCount) const
	{
		auto const LocalLength(
			FMath::Clamp(this->Len() - InCount, 0, this->Len()));
		return ThisClass(
			LocalLength, this->GetData() + this->Len() - LocalLength);
	}

	/// @return the substring from InStart position for InCount characters.
	public: ThisClass Mid(int32 InStart, int32 const InCount = MAX_int32) const
	{
		uint32 LocalEnd(InStart + InCount);
		InStart = FMath::Clamp(
			static_cast<uint32>(InStart),
			static_cast<uint32>(0),
			static_cast<uint32>(this->Len()));
		LocalEnd = FMath::Clamp(
			LocalEnd,
			static_cast<uint32>(InStart),
			static_cast<uint32>(this->Len()));
		return ThisClass(LocalEnd - InStart, this->GetData() + InStart);
	}

	/// @brief 先頭にある空白文字を取り除いた文字列を作る。
	/// @return 先頭にある空白文字を取り除いた文字列。
	public: ThisClass Trim() const PSYQUE_NOEXCEPT
	{
		auto const LocalData(this->GetData());
		auto const LocalLength(this->Len());
		auto const LocalEnd(LocalData + LocalLength);
		for (auto i(LocalData); i < LocalEnd; ++i)
		{
			if (!TChar<TCHAR>::IsWhitespace(*i))
			{
				auto const LocalPosition(i - LocalData);
				return ThisClass(
					LocalLength - LocalPosition, LocalData + LocalPosition);
			}
		}
		return ThisClass(0, LocalData + LocalLength);
	}

	/// @brief 末尾にある空白文字を取り除いた文字列を作る。
	/// @return 末尾にある空白文字を取り除いた文字列。
	public: ThisClass TrimTrailing() const PSYQUE_NOEXCEPT
	{
		auto const LocalData(this->GetData());
		auto const LocalLength(this->Len());
		for (auto i(LocalData + LocalLength); LocalData < i; --i)
		{
			if (!TChar<TCHAR>::IsWhitespace(*(i - 1)))
			{
				return ThisClass(i - LocalData, LocalData);
			}
		}
		return ThisClass(0, LocalData);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @brief 文字列の先頭位置を取得する。
	private: PSYQUE_CONSTEXPR TCHAR const* GetData() const PSYQUE_NOEXCEPT
	{
		return this->Data;
	}

	/// @brief 文字列の要素数を決定する。
	private: static int32 PSYQUE_CONSTEXPR CountLength(
		/// [in] 参照する文字列の要素数。 INDEX_NONE
		/// が指定された場合は、null文字を検索して自動で要素数を決定する。
		int32 const InLength,
		/// [in] 参照する文字列の先頭位置。
		TCHAR const* const InData)
	PSYQUE_NOEXCEPT
	{
		return InLength != INDEX_NONE?
			0 <= InLength? InLength: (PSYQUE_ASSERT(false), 0):
			InData != nullptr? TCString<TCHAR>::Strlen(InData): 0;
	}

	/// @brief 文字列を検索する。
	/// @return 検索文字列が現れた位置。現れない場合は INDEX_NONE を返す。
	private: template<typename TemplatePredicate> int32 PredicatedFind(
		/// [in] 検索する部分文字列の先頭位置。
		TCHAR const* const InSubString,
		/// [in] 検索を開始する位置。
		int32 const InOffset,
		/// [in] 検索する部分文字列の要素数。
		int32 const InSubLength,
		/// [in] 文字を比較する関数オブジェクト。
		TemplatePredicate InPredicate)
	const PSYQUE_NOEXCEPT
	{
		if (InOffset < 0)
		{
			return INDEX_NONE;
		}
		auto const LocalThisLength(this->Len());
		if (InSubLength <= 0)
		{
			return INDEX_NONE;
		}
		if (InSubString == nullptr)
		{
			check(false);
			return INDEX_NONE;
		}

		auto LocalRestLength(LocalThisLength - InOffset);
		if (LocalThisLength <= InOffset || LocalRestLength < InSubLength)
		{
			return INDEX_NONE;
		}
		LocalRestLength -= InSubLength - 1;
		auto const LocalThisData(this->GetData());
		auto LocalRestString(LocalThisData + InOffset);
		for (;;)
		{
			// 検索文字列の先頭文字と合致する位置を見つける。
			auto const LocalFind(
				ThisClass::PredicatedFindFirstOf(
					LocalRestString,
					LocalRestLength,
					InSubLength,
					InPredicate));
			if (LocalFind == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			// 検索文字列と合致するか判定。
			auto const LocalIsEqual(
				ThisClass::PredicatedEqual(
					LocalRestString + LocalFind,
					InSubString,
					InSubLength,
					InPredicate));
			if (LocalIsEqual)
			{
				return LocalFind;
			}

			// 次の候補へ。
			LocalRestLength -= LocalFind + 1;
			LocalRestString += LocalFind + 1;
		}
	}

	/// @brief 後ろから文字列を検索する。
	/// @return 検索文字列が現れた位置。現れない場合は INDEX_NONE を返す。
	private: template<typename TemplatePredicate>
	int32 PredicatedFindReverse(
		/// [in] 検索文字列の先頭位置。
		TCHAR const* const InSubString,
		/// [in] 検索を開始する位置。
		int32 const InOffset,
		/// [in] 検索文字列の要素数。
		int32 const InSubLength,
		/// [in] 文字を等価比較する二項関数オブジェクト。
		TemplatePredicate InPredicate)
	const PSYQUE_NOEXCEPT
	{
		auto const LocalThisLength(this->Len());
		if (InSubLength <= 0)
		{
			return INDEX_NONE;
		}
		check(InSubString != nullptr);
		if (InSubLength <= LocalThisLength)
		{
			auto const LocalThisData(this->GetData());
			auto const LocalOffset(
				(std::min)(InOffset, LocalThisLength - InSubLength));
			for (auto i(LocalThisData + LocalOffset); ; --i)
			{
				if (ThisClass::PredicatedEqual(i, InSubString, InSubLength, InPredicate))
				{
					return i - LocalThisData;
				}
				if (i <= LocalThisData)
				{
					break;
				}
			}
		}
		return INDEX_NONE;
	}

	private: template<typename TemplatePredicate>
	static bool PredicatedEqual(
		/// [in] 左辺となる文字列の先頭位置。
		TCHAR const* const InLeft,
		/// [in] 右辺となる文字列の先頭位置。
		TCHAR const* const InRight,
		/// [in] 文字列の要素数。
		int32 const InLength,
		/// [in] 文字を等価比較する二項関数オブジェクト。
		TemplatePredicate InPredicate)
	{
		check(InLength <= 0 || (InLeft != nullptr && InRight != nullptr));
		for (int32 i(0); i < InLength; ++i)
		{
			if (!InPredicate(InLeft[i], InRight[i]))
			{
				return false;
			}
		}
		return true;
	}

	/// @brief 文字列から文字を前方検索する。
	private: template<typename TemplatePredicate>
	static int32 PredicatedFindFirstOf(
		/// [in] 文字列の先頭位置。
		TCHAR const* const InData,
		/// [in] 文字列の要素数。
		int32 const InLength,
		/// [in] 探す文字。
		TCHAR const InChar,
		/// [in] 文字を等価比較する二項関数オブジェクト。
		TemplatePredicate InPredicate)
	{
		check(InData != nullptr || InLength <= 0);
		for (int32 i(0); i < InLength; ++i)
		{
			if (InPredicate(InData[i], InChar))
			{
				return i;
			}
		}
		return INDEX_NONE;
	}

	/// @brief 文字列から文字を後方検索する。
	private: template<typename TemplatePredicate>
	static int32 PredicatedFindLastOf(
		/// [in] 文字列の先頭位置。
		TCHAR const* const InData,
		/// [in] 文字列の要素数。
		int32 const InLength,
		/// [in] 探す文字。
		TCHAR const InChar,
		/// [in] 文字を等価比較する二項関数オブジェクト。
		TemplatePredicate InPredicate)
	{
		check(InData != nullptr || InLength <= 0);
		for (auto i(InLength - 1); 0 <= i; --i)
		{
			if (InPredicate(InData[i], InChar))
			{
				return i;
			}
		}
		return INDEX_NONE;
	}

	//-------------------------------------------------------------------------
	private:
	TCHAR const* Data; ///< @brief 文字列の先頭位置。
	int32 Length;      ///< @brief 文字列の要素数。

}; // class Psyque::String::FView

namespace std
{
	inline TCHAR const* begin(Psyque::String::FView const& InString)
	{
		return *InString;
	}
	inline TCHAR const* end(Psyque::String::FView const& InString)
	{
		return *InString + InString.Len();
	}
}

// vim: set noexpandtab:
