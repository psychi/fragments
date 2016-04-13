// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @copydoc Psyque::String::FNumericParser
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include "./View.h"

/// @cond
namespace Psyque
{
	namespace String
	{
		class FNumericParser;
	} // namespace String
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 文字列を解析し、数値を構築する。
class Psyque::String::FNumericParser
{
	/// @copydoc FView::ThisClass
	private: using ThisClass = FNumericParser;

	/// @brief 格納されている値の種類。
	public: enum class EKind: uint8
	{
		Empty,    ///< 空値。
		Bool,     ///< 真偽値。
		Unsigned, ///< 符号なし整数値。
		Negative, ///< 負の整数値。
		Float,    ///< 浮動小数点数値。
	};

	//-------------------------------------------------------------------------
	/// @brief 空値を構築する。
	public: FNumericParser() PSYQUE_NOEXCEPT: Kind(ThisClass::EKind::Empty) {} 

	/// @brief 真偽値を構築する。
	public: explicit FNumericParser(
		/// [in] 初期値となる真偽値。
		bool const InBool)
	PSYQUE_NOEXCEPT
	{
		this->SetBool(InBool);
	}

	/// @brief 符号なし整数値を構築する。
	public: explicit FNumericParser(
		/// [in] 初期値となる符号なし整数値。
		uint64 const InUnsigned)
	PSYQUE_NOEXCEPT
	{
		this->SetUnsigned(InUnsigned);
	}

	/// @brief 整数値を構築する。
	public: explicit FNumericParser(
		/// [in] 初期値となる整数値。
		int64 const InInteger)
	PSYQUE_NOEXCEPT
	{
		this->SetInteger(InInteger);
	}

	/// @brief 浮動小数点数値を構築する。
	public: explicit FNumericParser(
		/// [in] 初期値となる浮動小数点数値。
		double const InFloat)
	PSYQUE_NOEXCEPT
	{
		this->SetFloat(InFloat);
	}

	/// @brief 文字列を解析し、数値を構築する。
	public: FNumericParser(
		/// [out] ThisClass::Parse の戻り値の格納先。ただし
		/// nullptr の場合は格納されない。
		int32* const OutIndex,
		/// [in] 解析する文字列。
		Psyque::String::FView const& InString)
	{
		auto const LocalIndex(this->Parse(InString));
		if (OutIndex != nullptr)
		{
			*OutIndex = LocalIndex;
		}
	}

	//-------------------------------------------------------------------------
	/// @brief 格納されている値の種別を取得する。
	public: PSYQUE_CONSTEXPR ThisClass::EKind GetKind() const PSYQUE_NOEXCEPT
	{
		return this->Kind;
	}

	/// @brief 格納されている真偽値を取得する。
	/// @retval !=nullptr 格納されている真偽値を指すポインタ。
	/// @retval ==nullptr 真偽値が格納されていない。
	public: bool const* GetBool() const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == ThisClass::EKind::Bool? &this->Bool: nullptr;
	}

	/// @brief 格納されている符号なし整数値を取得する。
	/// @retval !=nullptr 格納されている符号なし整数値を指すポインタ。
	/// @retval ==nullptr 符号なし整数値が格納されていない。
	public: uint64 const* GetUnsigned() const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == ThisClass::EKind::Unsigned?
			&this->Unsigned: nullptr;
	}

	/// @brief 格納されている負の整数値を取得する。
	/// @retval !=nullptr 格納されている負の整数値を指すポインタ。
	/// @retval ==nullptr 負の整数値が格納されていない。
	public: int64 const* GetNegative() const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == ThisClass::EKind::Negative?
			reinterpret_cast<int64 const*>(&this->Unsigned): nullptr;
	}

	/// @brief 格納されている浮動小数点数値を取得する。
	/// @retval !=nullptr 格納されている浮動小数点数値を指すポインタ。
	/// @retval ==nullptr 浮動小数点数値が格納されていない。
	public: double const* GetFloat() const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == ThisClass::EKind::Float?
			&this->Float: nullptr;
	}

	//-------------------------------------------------------------------------
	/// @brief 真偽値を格納する。
	public: void SetBool(
		/// [in] 新たに格納する真偽値。
		bool const InBool)
	{
		this->Kind = ThisClass::EKind::Bool;
		this->Bool = InBool;
	}

	/// @brief 符号なし整数値を格納する。
	public: void SetUnsigned(
		/// [in] 新たに格納する符号なし整数値。
		uint64 const InUnsigned)
	{
		this->Kind = ThisClass::EKind::Unsigned;
		this->Unsigned = InUnsigned;
	}

	/// @brief 負の整数値を格納する。
	public: void SetInteger(
		/// [in] 新たに格納する負の整数値。
		int64 const InInteger)
	{
		this->Kind = InInteger < 0?
			ThisClass::EKind::Negative: ThisClass::EKind::Unsigned;
		this->Unsigned = static_cast<uint64>(InInteger);
	}

	/// @brief 浮動小数点数値を格納する。
	public: void SetFloat(
		/// [in] 新たに格納する浮動小数点数値。
		double const InFloat)
	{
		this->Kind = ThisClass::EKind::Float;
		this->Float = InFloat;
	}

	/// @brief 文字列を解析し、数値を構築する。
	/// @details
	///   - 真偽値 "true" / "false"
	///   - 2進数表記 "-0b110101"
	///   - 8進数表記 "-0245"
	///   - 10進数表記 "-255"
	///   - 16進数表記 "-0xFE"
	///   - 指数表記 "-6.25e-3"
	/// @return 解析を停止した文字のインデクス番号。
	public: int32 Parse(
		Psyque::String::FView const& InString)
	{
		// 先頭と末尾の空白文字を取り除く。
		auto const LocalString(InString.TrimTrailing().Trim());
		if (LocalString.IsEmpty())
		{
			this->Kind = ThisClass::EKind::Empty;
			return InString.Len();
		}

		// 符号を決定する。
		auto LocalStringIterator(*LocalString);
		auto const LocalStringEnd(LocalStringIterator + LocalString.Len());
		auto const LocalForwardStringIterator(
			[&LocalStringIterator, LocalStringEnd]()->bool
			{
				++LocalStringIterator;
				return LocalStringEnd <= LocalStringIterator;
			});
		int8 LocalSign(1);
		switch (*LocalStringIterator)
		{
			case '-': LocalSign = -1;
			// case '+' に続く。

			case '+':
			if (LocalForwardStringIterator())
			{
				this->Kind = ThisClass::EKind::Empty;
				return LocalStringIterator - *InString - 1;
			}
			break;

			default:
			if (LocalString == TEXT("false"))
			{
				this->SetBool(false);
			}
			else if (LocalString == TEXT("true"))
			{
				this->SetBool(true);
			}
			else
			{
				break;
			}
			return InString.Len();
		}

		// 基数を決定する。
		uint8 LocalRadix;
		if (*LocalStringIterator == '0')
		{
			if (LocalForwardStringIterator())
			{
				this->SetUnsigned(0);
				return InString.Len();
			}
			switch (*LocalStringIterator)
			{
				case '.': LocalRadix = 10; break;
				case 'b': LocalRadix =  2; goto FORWARD_STRING_ITERATOR;
				case 'x': LocalRadix = 16; goto FORWARD_STRING_ITERATOR;
				default:  LocalRadix =  8; break;

				FORWARD_STRING_ITERATOR:
				if (LocalForwardStringIterator())
				{
					this->Kind = ThisClass::EKind::Empty;
					return LocalStringIterator - *InString - 1;
				}
				break;
			}
		}
		else if (
			*LocalStringIterator == '.'
			|| ('1' <= *LocalStringIterator && *LocalStringIterator <= '9'))
		{
			LocalRadix = 10;
		}
		else
		{
			this->Kind = ThisClass::EKind::Empty;
			return LocalStringIterator - *InString;
		}

		// 整数部と小数部を指数部を解析する。
		auto const LocalInteger(
			ThisClass::ParseUnsigned(
				LocalStringIterator, LocalStringEnd, LocalRadix));
		auto const LocalIntegerBack(LocalStringIterator - 1);
		auto const LocalFraction(
			ThisClass::ParseFraction(
				LocalStringIterator, LocalStringEnd, LocalRadix));
		auto const LocalExponent(
			ThisClass::ParseExponent(
				LocalStringIterator, LocalStringEnd, LocalRadix));
		if (0 < LocalFraction || LocalExponent != 0)
		{
			// 整数部と小数部と指数部を合成する。
			this->SetFloat(
				(double(LocalInteger) * LocalSign + LocalFraction)
				* std::pow(LocalRadix, LocalExponent));
		}
		else if (0 < LocalSign)
		{
			this->SetUnsigned(LocalInteger);
		}
		else
		{
			this->SetInteger(static_cast<int64>(LocalInteger) * LocalSign);
			if (MAX_int64 < LocalInteger)
			{
				LocalStringIterator = LocalIntegerBack;
			}
		}
		return LocalStringIterator == LocalStringEnd?
			InString.Len(): LocalStringIterator - *InString;
	}

	//-------------------------------------------------------------------------
	/// @brief 文字列を解析し、小数部を構築する。
	private: static double ParseFraction(
		TCHAR const*& OutStringIterator,
		TCHAR const* const InStringEnd,
		uint32 const InRadix)
	{
		auto i(OutStringIterator);
		if (InStringEnd <= i || *i != '.')
		{
			return 0;
		}
		double LocalValue(0);
		uint32 LocalNumerator(0);
		uint32 LocalDenominator(1);
		double LocalDenominatorInverse(1);
		auto const LocalDenominatorLimit(MAX_uint32 / (InRadix * InRadix));
		for (++i; i< InStringEnd; ++i)
		{
			auto const LocalNumber(ThisClass::ParseNumericChar(*i, InRadix));
			if (InRadix <= LocalNumber)
			{
				break; // 数字ではなかった。
			}
			LocalNumerator = LocalNumerator * InRadix + LocalNumber;
			LocalDenominator *= InRadix;
			if (LocalDenominatorLimit <= LocalDenominator)
			{
				LocalDenominatorInverse /= LocalDenominator;
				LocalValue += LocalNumerator * LocalDenominatorInverse;
				LocalNumerator = 0;
				LocalDenominator = 1;
			}
		}
		LocalValue += (LocalNumerator * LocalDenominatorInverse)
			/ LocalDenominator;
		OutStringIterator = i;
		return LocalValue;
	}

	/// @brief 文字列を解析し、指数部を構築する。
	private: static int64 ParseExponent(
		TCHAR const*& OutStringIterator,
		TCHAR const* const InStringEnd,
		uint32 const InRadix)
	{
		if (InStringEnd <= OutStringIterator)
		{
			return 0;
		}
		switch (*OutStringIterator)
		{
			case 'e':
			case 'E':
			if (0xE <= InRadix)
			{
				return 0;
			}
			break;

			case 'x':
			case 'X':
			break;

			default:
			return 0;
		}

		// 符号を決定する。
		auto const LocalForwardStringIterator(
			[&OutStringIterator, InStringEnd]()->bool
			{
				++OutStringIterator;
				return InStringEnd <= OutStringIterator;
			});
		if (LocalForwardStringIterator())
		{
			return 0;
		}
		int8 LocalSign(1);
		switch (*OutStringIterator)
		{
			case '-': LocalSign = -1;
			// case '+' へ続く。

			case '+':
			if (LocalForwardStringIterator())
			{
				return 0;
			}
			// defalt へ続く。

			default: break;
		}
		auto LocalExponent(
			ThisClass::ParseUnsigned(OutStringIterator, InStringEnd, InRadix));
		if (MAX_int64 < LocalExponent)
		{
			LocalExponent /= InRadix;
			--OutStringIterator;
		}
		return static_cast<int64>(LocalExponent) * LocalSign;
	}

	/// @brief 文字列を解析し、符号なし整数を構築する。
	private: static uint64 ParseUnsigned(
		TCHAR const*& OutStringIterator,
		TCHAR const* const InStringEnd,
		uint32 const InRadix)
	{
		uint64 LocalValue(0);
		auto i(OutStringIterator);
		for (; i < InStringEnd; ++i)
		{
			auto const LocalNumber(ThisClass::ParseNumericChar(*i, InRadix));
			if (InRadix <= LocalNumber)
			{
				break; // 数字ではなかった。
			}
			auto const LocalNewValue(LocalValue * InRadix + LocalNumber);
			if (LocalNewValue < LocalValue)
			{
				break; // 桁あふれした。
			}
			LocalValue = LocalNewValue;
		}
		OutStringIterator = i;
		return LocalValue;
	}

	/// @brief 文字を解析し、符号なし整数を構築する。
	/// @return 文字から構築した符号なし整数値。ただし
	///   InRadix 以上の値を返した場合は、解析に失敗した。
	private: static uint32 ParseNumericChar(
		/// [in] 解析する文字。
		TCHAR InChar,
		/// [in] 符号なし整数の基数。
		uint32 const InRadix)
	{
		if (InChar <= '9')
		{
			InChar -= '0';
			if (InChar < 0)
			{
				return InRadix;
			}
		}
		else
		{
			InChar -= InChar <= 'Z'? 'A': 'a';
			if (InChar < 0)
			{
				return InRadix;
			}
			InChar += 10;
		}
		return static_cast<uint32>(InChar);
	}

	//-------------------------------------------------------------------------
	private:
	union
	{
		uint64 Unsigned;
		double Float;
		bool Bool;
	};
	ThisClass::EKind Kind;

}; // class Psyque::String::FNumericParser

// vim: set noexpandtab:
