// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::TStatusValue
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include "../BitAlgorithm.h"
#include "./Enum.h"

#ifndef PSYQUE_RULES_ENGINE_STATUS_VALUE_EPSILON_MAG
#define PSYQUE_RULES_ENGINE_STATUS_VALUE_EPSILON_MAG 4
#endif // !default(PSYQUE_RULES_ENGINE_STATUS_VALUE_EPSILON_MAG)

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		namespace _private
		{
			template<typename, typename, typename> class TStatusValue;
		} // namespace _private
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態値のやりとりに使う容れ物。
/// @tparam TemplateUnsigned @copydoc TStatusValue::FUnsigned
/// @tparam TemplateFloat	 @copydoc TStatusValue::FFloat
template<typename TemplateUnsigned, typename TemplateFloat, typename TemplateBitWidth>
class Psyque::RulesEngine::_private::TStatusValue
{
	private: using ThisClass = TStatusValue; ///< @copydoc TReservoir::ThisClass

	//-------------------------------------------------------------------------
	/// @brief 状態値で扱う符号なし整数の型。
	/// @details この型の大きさを超える型は、状態値で扱えない。
	public: using FUnsigned = TemplateUnsigned;
	static_assert(
		std::is_integral<TemplateUnsigned>::value
		&& std::is_unsigned<TemplateUnsigned>::value,
		"'TemplateUnsigned' is not unsigned integer.");

	/// @brief 状態値で扱う符号あり整数の型。
	public: using FSigned =
		typename std::make_signed<typename ThisClass::FUnsigned>::type;

	/// @brief 状態値で扱う浮動小数点数の型。
	public: using FFloat = TemplateFloat;
	static_assert(
		std::is_floating_point<TemplateFloat>::value,
		"'TemplateFloat' is not floating-point FNumber.");
	static_assert(
		 sizeof(TemplateFloat) <= sizeof(TemplateUnsigned),
		 "sizeof(TemplateFloat) is greater than sizeof(TemplateUnsigned).");

	/// @brief 状態値のビット幅を表す、符号なし整数型。
	public: using FBitWidth = TemplateBitWidth;
	static_assert(
		sizeof(EPsyqueRulesStatusKind) <= sizeof(TemplateBitWidth)
		&& std::is_integral<TemplateBitWidth>::value
		&& std::is_unsigned<TemplateBitWidth>::value,
		"'TemplateBitWidth' is not unsigned integer.");

	/// @brief 状態値のビット構成を表す型。
	public: using FBitFormat = TemplateBitWidth;

	private: enum: TemplateBitWidth
	{
		UnsignedBase = static_cast<TemplateBitWidth>(
			EPsyqueRulesStatusKind::Unsigned),
		SignedBase = static_cast<TemplateBitWidth>(
			EPsyqueRulesStatusKind::Signed),
	};

	//-------------------------------------------------------------------------
	/// @name 構築
	/// @{

	/// @brief 空値を構築する。
	public: TStatusValue() PSYQUE_NOEXCEPT:
	BitFormat(ThisClass::MakeBitFormat(EPsyqueRulesStatusKind::Empty))
	{}

	/// @brief 論理型の値を構築する。
	public: explicit TStatusValue(
		/// [in] 初期値となる論理値。
		bool const InBool)
	PSYQUE_NOEXCEPT:
	BitFormat(ThisClass::MakeBitFormat(EPsyqueRulesStatusKind::Bool))
	{
		this->Bool = InBool;
	}

	/// @brief 符号なし整数型の値を構築する。
	public: explicit TStatusValue(
		/// [in] 初期値となる符号なし整数。
		typename ThisClass::FUnsigned const InUnsigned)
	PSYQUE_NOEXCEPT:
	BitFormat(
		ThisClass::MakeBitFormat(
			EPsyqueRulesStatusKind::Unsigned, sizeof(InUnsigned) * CHAR_BIT))
	{
		this->Unsigned = InUnsigned;
	}

	/// @brief 符号あり整数型の値を構築する。
	public: explicit TStatusValue(
		/// [in] 初期値となる符号あり整数。
		typename ThisClass::FSigned const InSigned)
	PSYQUE_NOEXCEPT:
	BitFormat(
		ThisClass::MakeBitFormat(
			EPsyqueRulesStatusKind::Signed, sizeof(InSigned) * CHAR_BIT))
	{
		this->Signed = InSigned;
	}

	/// @brief 浮動小数点数型の値を構築する。
	public: explicit TStatusValue(
		/// [in] 初期値となる浮動小数点数。
		typename ThisClass::FFloat const InFloat)
	PSYQUE_NOEXCEPT:
	BitFormat(ThisClass::MakeBitFormat(EPsyqueRulesStatusKind::Float))
	{
		this->Float = InFloat;
	}

	/// @brief 任意型の値を構築する。
	public: template<typename TemplateValue>
	explicit TStatusValue(
		/// [in] 初期値。
		TemplateValue const& InValue,
		/// [in] 値の型。
		/// EPsyqueRulesStatusKind::Empty の場合は、自動で決定する。
		EPsyqueRulesStatusKind const InKind = EPsyqueRulesStatusKind::Empty)
	PSYQUE_NOEXCEPT:
	BitFormat(ThisClass::MakeBitFormat(EPsyqueRulesStatusKind::Empty))
	{
		this->Assign(InValue, InKind);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 値の取得
	/// @{

	/// @brief 値が空か判定する。
	public: bool IsEmpty() const PSYQUE_NOEXCEPT
	{
		return ThisClass::IsEmpty(this->BitFormat);
	}

	/// @brief 論理値を取得する。
	/// @return 論理値を指すポインタ。論理値が格納されてない場合は nullptr を返す。
	public: bool const* GetBool() const PSYQUE_NOEXCEPT
	{
		return ThisClass::IsBool(this->BitFormat)? &this->Bool: nullptr;
	}

	/// @brief 符号なし整数値を取得する。
	/// @return 符号なし整数値を指すポインタ。
	///   符号なし整数値が格納されてない場合は nullptr を返す。
	public: typename ThisClass::FUnsigned const* GetUnsigned()
	const PSYQUE_NOEXCEPT
	{
		return ThisClass::IsUnsigned(this->BitFormat)? &this->Unsigned: nullptr;
	}

	/// @brief 符号あり整数値を取得する。
	/// @return 符号あり整数値を指すポインタ。
	///   符号あり整数値が格納されてない場合は nullptr を返す。
	public: typename ThisClass::FSigned const* GetSigned() const PSYQUE_NOEXCEPT
	{
		return ThisClass::IsSigned(this->BitFormat)? &this->Signed: nullptr;
	}

	/// @brief 浮動小数点数値を取得する。
	/// @return 浮動小数点数値を指すポインタ。
	///   浮動小数点数値が格納されてない場合は nullptr を返す。
	public: typename ThisClass::FFloat const* GetFloat() const PSYQUE_NOEXCEPT
	{
		return ThisClass::IsFloat(this->BitFormat)? &this->Float: nullptr;
	}

	public: typename ThisClass::FUnsigned GetBitset() const PSYQUE_NOEXCEPT
	{
		switch (this->BitFormat)
		{
			case EPsyqueRulesStatusKind::Empty: return 0;
			case EPsyqueRulesStatusKind::Bool:  return this->Bool;

			case EPsyqueRulesStatusKind::Float:
			using FFloatBitset = Psyque::TFloatBitset<typename ThisClass::FFloat>;
			return FFloatBitset(this->Float).Bitset;

			default: return this->Unsigned;
		}
	}

	/// @brief 格納値の型の種類を取得する。
	/// @return 格納値の型の種類。
	public: EPsyqueRulesStatusKind GetKind() const PSYQUE_NOEXCEPT
	{
		return ThisClass::MakeKind(this->BitFormat);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 値の比較
	/// @{

	/// @brief 比較式を評価する。
	/// @retval 正 比較式の評価は真。
	/// @retval 0  比較式の評価は偽。
	/// @retval 負 比較式の評価に失敗。
	public: template<typename TemplateRight>
	EPsyqueKleene Compare(
		/// [in] 比較演算子の種類。
		RulesEngine::EStatusComparison const InComparison,
		/// [in] 比較演算子の右辺値。
		TemplateRight const& InRight)
	const PSYQUE_NOEXCEPT
	{
		auto const LocalOrder(this->Compare(InRight));
		if (LocalOrder != RulesEngine::EStatusOrder::Failed)
		{
			switch (InComparison)
			{
				case RulesEngine::EStatusComparison::Equal:
				return static_cast<EPsyqueKleene>(
					LocalOrder == RulesEngine::EStatusOrder::Equal);

				case RulesEngine::EStatusComparison::NotEqual:
				return static_cast<EPsyqueKleene>(
					LocalOrder != RulesEngine::EStatusOrder::Equal);

				case RulesEngine::EStatusComparison::Less:
				return static_cast<EPsyqueKleene>(
					LocalOrder == RulesEngine::EStatusOrder::Less);

				case RulesEngine::EStatusComparison::LessEqual:
				return static_cast<EPsyqueKleene>(
					LocalOrder != RulesEngine::EStatusOrder::Greater);

				case RulesEngine::EStatusComparison::Greater:
				return static_cast<EPsyqueKleene>(
					LocalOrder == RulesEngine::EStatusOrder::Greater);

				case RulesEngine::EStatusComparison::GreaterEqual:
				return static_cast<EPsyqueKleene>(
					LocalOrder != RulesEngine::EStatusOrder::Less);

				default:
				check(false);
				break;
			}
		}
		return EPsyqueKleene::Unknown;
	}

	/// @brief 値を比較する。
	/// @return *this を左辺値とした比較結果。
	public: RulesEngine::EStatusOrder Compare(
		/// [in] 右辺値。
		ThisClass const& InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (InRight.GetKind())
		{
			case EPsyqueRulesStatusKind::Bool:
			return this->Compare(InRight.Bool);

			case EPsyqueRulesStatusKind::Unsigned:
			return this->Compare(InRight.Unsigned);

			case EPsyqueRulesStatusKind::Signed:
			return this->Compare(InRight.Signed);

			case EPsyqueRulesStatusKind::Float:
			return this->Compare(InRight.Float);

			default:
			return RulesEngine::EStatusOrder::Failed;
		}
	}

	/// @brief 論理値と比較する。
	/// @return *this を左辺値とした比較結果。
	public: RulesEngine::EStatusOrder Compare(
		/// [in] 右辺値となる論理値。
		bool const InRight)
	const PSYQUE_NOEXCEPT
	{
		if (this->GetBool() == nullptr)
		{
			return RulesEngine::EStatusOrder::Failed;
		}
		if (this->Bool == InRight)
		{
			return RulesEngine::EStatusOrder::Equal;
		}
		if (this->Bool)
		{
			return RulesEngine::EStatusOrder::Greater;
		}
		return RulesEngine::EStatusOrder::Less;
	}

	/// @brief 符号なし整数と比較する。
	/// @return *this を左辺値とした比較結果。
	public: RulesEngine::EStatusOrder Compare(
		/// [in] 右辺値となる符号なし整数。
		typename ThisClass::FUnsigned const InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case EPsyqueRulesStatusKind::Unsigned:
			return ThisClass::CompareValue(this->Unsigned, InRight);

			case EPsyqueRulesStatusKind::Signed:
			return this->Signed < 0?
				RulesEngine::EStatusOrder::Less:
				ThisClass::CompareValue(this->Unsigned, InRight);

			case EPsyqueRulesStatusKind::Float:
			return ThisClass::CompareFloatLeft(this->Float, InRight);

			default: return RulesEngine::EStatusOrder::Failed;
		}
	}

	/// @brief 符号あり整数と比較する。
	/// @return *this を左辺値とした比較結果。
	public: RulesEngine::EStatusOrder Compare(
		/// [in] 右辺値となる符号あり整数。
		typename ThisClass::FSigned const InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case EPsyqueRulesStatusKind::Unsigned:
			return InRight < 0?
				RulesEngine::EStatusOrder::Greater:
				ThisClass::CompareValue(
					this->Unsigned,
					static_cast<typename ThisClass::FUnsigned>(InRight));

			case EPsyqueRulesStatusKind::Signed:
			return ThisClass::CompareValue(this->Signed, InRight);

			case EPsyqueRulesStatusKind::Float:
			return ThisClass::CompareFloatLeft(this->Float, InRight);

			default: return RulesEngine::EStatusOrder::Failed;
		}
	}

	/// @brief 浮動小数点数と比較する。
	/// @return *this を左辺値とした比較結果。
	public: RulesEngine::EStatusOrder Compare(
		/// [in] 右辺値となる浮動小数点数。
		typename ThisClass::FFloat const InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case EPsyqueRulesStatusKind::Unsigned:
			return InRight < 0?
				RulesEngine::EStatusOrder::Greater:
				ThisClass::CompareFloatRight(this->Unsigned, InRight);

			case EPsyqueRulesStatusKind::Signed:
			return ThisClass::CompareFloatRight(this->Signed, InRight);

			case EPsyqueRulesStatusKind::Float:
			return ThisClass::CompareFloat(this->Float, InRight);

			default: return RulesEngine::EStatusOrder::Failed;
		}
	}

	/// @brief 値を比較する。
	/// @return *this を左辺値とした比較結果。
	public: template<typename TemplateRight>
	RulesEngine::EStatusOrder Compare(
		/// [in] 右辺値。
		TemplateRight const& InRight)
	const PSYQUE_NOEXCEPT
	{
		if (std::is_floating_point<TemplateRight>::value)
		{
			auto const LocalRight(
				static_cast<typename ThisClass::FFloat>(InRight));
			if (LocalRight == InRight)
			{
				return this->Compare(LocalRight);
			}
		}
		else if (std::is_signed<TemplateRight>::value)
		{
			auto const LocalRight(
				static_cast<typename ThisClass::FSigned>(InRight));
			if (LocalRight == InRight)
			{
				return this->Compare(LocalRight);
			}
		}
		else if (std::is_unsigned<TemplateRight>::value)
		{
			auto const LocalRight(
				static_cast<typename ThisClass::FUnsigned>(InRight));
			if (LocalRight == InRight)
			{
				return this->Compare(LocalRight);
			}
		}
		return RulesEngine::EStatusOrder::Failed;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 値の代入
	/// @{

	/// @brief 状態値を空にする。
	public: void AssignEmpty() PSYQUE_NOEXCEPT
	{
		this->BitFormat = ThisClass::MakeBitFormat(EPsyqueRulesStatusKind::Empty);
	}

	/// @brief 論理値を代入する。
	public: void AssignBool(
		/// [in] 代入する論理値。
		bool const InValue)
	PSYQUE_NOEXCEPT
	{
		this->Bool = InValue;
		this->BitFormat = ThisClass::MakeBitFormat(EPsyqueRulesStatusKind::Bool);
	}

	/// @brief 符号なし整数を代入する。
	/// @retval true 成功。 InValue を *this に設定した。
	/// @retval false
	/// 失敗。InValue を符号なし整数に変換できなかった。 *this は変化しない。
	public: template<typename TemplateValue>
	bool AssignUnsigned(
		/// [in] 代入する値。
		TemplateValue const& InValue)
	PSYQUE_NOEXCEPT
	{
		if (0 <= InValue)
		{
			auto const LocalUnsigned(
				static_cast<typename ThisClass::FUnsigned>(InValue));
			if (static_cast<TemplateValue>(LocalUnsigned) == InValue)
			{
				this->Unsigned = LocalUnsigned;
				this->BitFormat = ThisClass::MakeBitFormat(
					EPsyqueRulesStatusKind::Unsigned,
					sizeof(LocalUnsigned) * CHAR_BIT);
				return true;
			}
		}
		return false;
	}

	/// @brief 符号あり整数を代入する。
	/// @retval true  成功。 InValue を *this に代入した。
	/// @retval false 失敗。 InValue を符号あり整数に変換できなかった。
	///   *this は変化しない。
	public: template<typename TemplateValue>
	bool AssignSigned(
		/// [in] 代入する値。
		TemplateValue const& InValue)
	PSYQUE_NOEXCEPT
	{
		auto const LocalSigned(static_cast<typename ThisClass::FSigned>(InValue));
		if (InValue <= 0 || 0 <= LocalSigned)
		{
			if (static_cast<TemplateValue>(LocalSigned) == InValue)
			{
				this->Signed = LocalSigned;
				this->BitFormat = ThisClass::MakeBitFormat(
					EPsyqueRulesStatusKind::Signed,
					sizeof(LocalSigned) * CHAR_BIT);
				return true;
			}
		}
		return false;
	}

	/// @brief 浮動小数点数を代入する。
	/// @retval true  成功。 InValue を *this に代入した。
	/// @retval false 失敗。 InValue を浮動小数点数に変換できなかった。
	///   *this は変化しない。
	public: template<typename TemplateValue>
	bool AssignFloat(
		/// [in] 代入する値。
		TemplateValue const& InValue)
	PSYQUE_NOEXCEPT
	{
		auto const LocalFloat(
			static_cast<typename ThisClass::FFloat>(InValue));
		auto const LocalDiff(
			static_cast<TemplateValue>(LocalFloat) - InValue);
		auto const LocalEpsilon(
			std::numeric_limits<typename ThisClass::FFloat>::epsilon()
			* PSYQUE_RULES_ENGINE_STATUS_VALUE_EPSILON_MAG);
		if (-LocalEpsilon <= LocalDiff && LocalDiff <= LocalEpsilon)
		{
			this->Float = LocalFloat;
			this->BitFormat =
				ThisClass::MakeBitFormat(EPsyqueRulesStatusKind::Float);
			return true;
		}
		return false;
	}

	/// @brief 値を代入する。
	/// @retval true  成功。 InValue を *this に代入した。
	/// @retval false 失敗。 InValue を状態値に変換できなかった。
	///   *this は変化しない。
	public: template<typename TemplateValue>
	bool Assign(
		/// [in] 代入する値。
		TemplateValue const& InValue,
		/// [in] 代入する型。
		/// EPsyqueRulesStatusKind::Empty の場合は、自動で決定する。
		EPsyqueRulesStatusKind InKind = EPsyqueRulesStatusKind::Empty)
	{
		if (InKind == EPsyqueRulesStatusKind::Empty)
		{
			InKind = ThisClass::template ClassifyKind<TemplateValue>();
		}
		switch (InKind)
		{
			case EPsyqueRulesStatusKind::Unsigned:
			return this->AssignUnsigned(InValue);

			case EPsyqueRulesStatusKind::Signed:
			return this->AssignSigned(InValue);

			case EPsyqueRulesStatusKind::Float:
			return this->AssignFloat(InValue);

			default: return false;
		}
	}

	/// @copydoc Assign
	public: bool Assign(
		ThisClass const& InValue,
		EPsyqueRulesStatusKind InKind = EPsyqueRulesStatusKind::Empty)
	PSYQUE_NOEXCEPT
	{
		if (InKind == EPsyqueRulesStatusKind::Empty)
		{
			InKind = InValue.GetKind();
		}
		switch (InValue.GetKind())
		{
			case EPsyqueRulesStatusKind::Bool:
			return this->Assign(InValue.Bool, InKind);

			case EPsyqueRulesStatusKind::Unsigned:
			return this->Assign(InValue.Unsigned, InKind);

			case EPsyqueRulesStatusKind::Signed:
			return this->Assign(InValue.Signed, InKind);

			case EPsyqueRulesStatusKind::Float:
			return this->Assign(InValue.Float, InKind);

			default:
			this->AssignEmpty();
			return true;
		}
	}

	/// @copydoc Assign
	public: bool Assign(
		bool const InValue,
		EPsyqueRulesStatusKind const InKind = EPsyqueRulesStatusKind::Bool)
	{
		switch (InKind)
		{
			case EPsyqueRulesStatusKind::Empty:
			case EPsyqueRulesStatusKind::Bool:
			this->AssignBool(InValue);
			return true;

			default: return false;
		}
	}

	/// @brief 代入演算する。
	/// @retval true  成功。演算結果を *this に代入した。
	/// @retval false 失敗。 *this は変化しない。
	public: template<typename TemplateValue>
	bool Assign(
		/// [in] 適用する代入演算子。
		EPsyqueStatusAssignment const InOperator,
		/// [in] 代入演算子の右辺。
		TemplateValue const& InRight)
	PSYQUE_NOEXCEPT
	{
		static_assert(!std::is_same<TemplateValue, bool>::value, "");
		auto const LocalKind(this->GetKind());
		if (InOperator == EPsyqueStatusAssignment::Copy)
		{
			return this->Assign(InRight, LocalKind);
		}
		switch (LocalKind)
		{
			case EPsyqueRulesStatusKind::Unsigned:
			return this->AssignValue(
				std::is_integral<TemplateValue>(),
				LocalKind,
				InOperator,
				this->Unsigned,
				InRight);

			case EPsyqueRulesStatusKind::Signed:
			return this->AssignValue(
				std::is_integral<TemplateValue>(),
				LocalKind,
				InOperator,
				this->Signed,
				InRight);

			case EPsyqueRulesStatusKind::Float:
			return this->AssignValue(
				std::false_type(),
				LocalKind,
				InOperator,
				this->Float,
				InRight);

			default: return false;
		}
	}

	/// @copydoc Assign(EPsyqueStatusAssignment const, TemplateValue const&)
	public: bool Assign(
		EPsyqueStatusAssignment const InOperator,
		ThisClass const& InRight)
	PSYQUE_NOEXCEPT
	{
		switch (InRight.GetKind())
		{
			case EPsyqueRulesStatusKind::Bool:
			return this->Assign(InOperator, InRight.Bool);

			case EPsyqueRulesStatusKind::Unsigned:
			return this->Assign(InOperator, InRight.Unsigned);

			case EPsyqueRulesStatusKind::Signed:
			return this->Assign(InOperator, InRight.Signed);

			case EPsyqueRulesStatusKind::Float:
			return this->Assign(InOperator, InRight.Float);

			default: return false;
		}
	}

	/// @copydoc Assign(EPsyqueStatusAssignment const, TemplateValue const&)
	public: bool Assign(
		EPsyqueStatusAssignment const InOperator,
		bool const InRight)
	PSYQUE_NOEXCEPT
	{
		if (!ThisClass::IsBool(this->BitFormat))
		{
			return false;
		}
		switch (InOperator)
		{
			case EPsyqueStatusAssignment::Copy:
			this->Bool  = InRight;
			break;

			case EPsyqueStatusAssignment::Or:
			this->Bool |= InRight;
			break;

			case EPsyqueStatusAssignment::Xor:
			this->Bool ^= InRight;
			break;

			case EPsyqueStatusAssignment::And:
			this->Bool &= InRight;
			break;

			default: return false;
		}
		return true;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @brief 空のビット構成か判定する。
	public: static bool PSYQUE_CONSTEXPR IsEmpty(
		/// [in] 状態値のビット構成。
		typename ThisClass::FBitFormat const InBitFormat)
	PSYQUE_NOEXCEPT
	{
		return InBitFormat == static_cast<uint8>(EPsyqueRulesStatusKind::Empty);
	}

	/// @brief 真偽型のビット構成か判定する。
	public: static bool PSYQUE_CONSTEXPR IsBool(
		/// [in] 状態値のビット構成。
		typename ThisClass::FBitFormat const InBitFormat)
	PSYQUE_NOEXCEPT
	{
		return InBitFormat == static_cast<uint8>(EPsyqueRulesStatusKind::Bool);
	}

	/// @brief 符号なし整数型のビット構成か判定する。
	public: static bool PSYQUE_CONSTEXPR IsUnsigned(
		/// [in] 状態値のビット構成。
		typename ThisClass::FBitFormat const InBitFormat)
	PSYQUE_NOEXCEPT
	{
		return static_cast<uint8>(EPsyqueRulesStatusKind::Unsigned) <= InBitFormat
			&& !ThisClass::IsSigned(InBitFormat);
	}

	/// @brief 符号あり整数型のビット構成か判定する。
	public: static bool PSYQUE_CONSTEXPR IsSigned(
		/// [in] 状態値のビット構成。
		typename ThisClass::FBitFormat const InBitFormat)
	PSYQUE_NOEXCEPT
	{
		return static_cast<uint8>(EPsyqueRulesStatusKind::Signed) <= InBitFormat;
	}

	/// @brief 浮動小数点数型のビット構成か判定する。
	public: static bool PSYQUE_CONSTEXPR IsFloat(
		/// [in] 状態値のビット構成。
		typename ThisClass::FBitFormat const InBitFormat)
	PSYQUE_NOEXCEPT
	{
		return InBitFormat == static_cast<uint8>(EPsyqueRulesStatusKind::Float);
	}

	/// @brief 状態値のビット構成から、状態値の型を構築する。
	/// @return 状態値の型の種類。
	public: static EPsyqueRulesStatusKind PSYQUE_CONSTEXPR MakeKind(
		/// [in] 状態値のビット構成。
		typename ThisClass::FBitFormat const InBitFormat)
	PSYQUE_NOEXCEPT
	{
		return ThisClass::IsUnsigned(InBitFormat)?
			EPsyqueRulesStatusKind::Unsigned:
			ThisClass::IsSigned(InBitFormat)?
				EPsyqueRulesStatusKind::Signed:
				static_cast<typename EPsyqueRulesStatusKind>(InBitFormat);
	}

	/// @brief 状態値の型とビット幅から、状態値のビット構成を構築する。
	/// @return 状態値のビット構成。ただしビット構成が存在しない場合は0を戻す。
	public: static typename ThisClass::FBitFormat MakeBitFormat(
		/// [in] 状態値の型の種類。
		EPsyqueRulesStatusKind const InKind,
		/// [in] 状態値が整数型の場合の、ビット幅。
		/// 状態値が整数型以外の場合は、必ず0を指定すること。
		typename ThisClass::FBitWidth const InBitWidth = 0)
	PSYQUE_NOEXCEPT
	{
		switch (InKind)
		{
			case EPsyqueRulesStatusKind::Unsigned:
			if (InBitWidth <= sizeof(typename ThisClass::FUnsigned) * CHAR_BIT
				&& 2 <= InBitWidth)
			{
				return InBitWidth - 2 + ThisClass::UnsignedBase;
			}
			break;

			case EPsyqueRulesStatusKind::Signed:
			if (InBitWidth <= sizeof(typename ThisClass::FSigned) * CHAR_BIT
				&& 2 <= InBitWidth)
			{
				return InBitWidth - 2 + ThisClass::SignedBase;
			}
			break;

			default:
			if (InBitWidth == 0)
			{
				return static_cast<ThisClass::FBitFormat>(InKind);
			}
			check(false);
			break;
		}
		return 0;
	}

	/// @brief 状態値のビット構成から、状態値のビット幅を構築する。
	/// @return 状態値のビット幅。
	public: static typename ThisClass::FBitWidth MakeBitWidth(
		/// [in] 状態値のビット構成。
		typename ThisClass::FBitFormat const InBitFormat)
	PSYQUE_NOEXCEPT
	{
		switch (InBitFormat)
		{
			case EPsyqueRulesStatusKind::Empty:
			case EPsyqueRulesStatusKind::Bool:
			static_assert(
				static_cast<uint8>(EPsyqueRulesStatusKind::Empty) + 1
				== static_cast<uint8>(EPsyqueRulesStatusKind::Bool),
				"");
			return InBitFormat - static_cast<typename ThisClass::FBitWidth>(
				EPsyqueRulesStatusKind::Empty);

			case EPsyqueRulesStatusKind::Float:
			return sizeof(typename ThisClass::FFloat) * CHAR_BIT;

			default:
			static_assert(ThisClass::UnsignedBase < ThisClass::SignedBase, "");
			if (ThisClass::SignedBase <= InBitFormat)
			{
				return InBitFormat - ThisClass::SignedBase + 2;
			}
			if (ThisClass::UnsignedBase <= InBitFormat)
			{
				return InBitFormat - ThisClass::UnsignedBase + 2;
			}
			return 0;
		}
	}

	//-------------------------------------------------------------------------
	/// @brief 型の種類を決定する。
	/// @tparam TemplateValue 型。
	/// @return 型の種類。
	private: template<typename TemplateValue>
	static EPsyqueRulesStatusKind ClassifyKind() PSYQUE_NOEXCEPT
	{
		if (std::is_same<TemplateValue, bool>::value)
		{
			return EPsyqueRulesStatusKind::Bool;
		}
		else if (std::is_floating_point<TemplateValue>::value)
		{
			return EPsyqueRulesStatusKind::Float;
		}
		else if (std::is_integral<TemplateValue>::value)
		{
			return std::is_unsigned<TemplateValue>::value?
				EPsyqueRulesStatusKind::Unsigned:
				EPsyqueRulesStatusKind::Signed;
		}
		return EPsyqueRulesStatusKind::Empty;
	}

	//-------------------------------------------------------------------------
	/// @brief ThisClass::AssignUnsigned で論理値を設定させないためのダミー関数。
	private: bool AssignUnsigned(bool const);
	/// @brief ThisClass::AssignSigned で論理値を設定させないためのダミー関数。
	private: bool AssignSigned(bool const);
	/// @brief ThisClass::AssignFloat で論理値を設定させないためのダミー関数。
	private: bool AssignFloat(bool const);

	//-------------------------------------------------------------------------
	/// @brief 整数の演算を行い、結果を状態値へ格納する。
	/// @retval true  成功。 演算結果を *this に格納した。
	/// @retval false 失敗。 *this は変化しない。
	private: template<typename TemplateLeft, typename TemplateRight>
	bool AssignValue(
		std::true_type,
		/// [in] 演算した結果の型。
		EPsyqueRulesStatusKind const InKind,
		/// [in] 適用する演算子。
		EPsyqueStatusAssignment const InOperator,
		/// [in] 演算子の左辺となる整数値。
		TemplateLeft const& InLeft,
		/// [in] 演算子の左辺となる整数値。
		TemplateRight const& InRight)
	PSYQUE_NOEXCEPT
	{
		switch (InOperator)
		{
			case EPsyqueStatusAssignment::Mod:
			if (InRight == 0)
			{
				return false;
			}
			return this->Assign(InLeft % InRight, InKind);

			case EPsyqueStatusAssignment::Or:
			return this->Assign(InLeft | InRight, InKind);

			case EPsyqueStatusAssignment::Xor:
			return this->Assign(InLeft ^ InRight, InKind);

			case EPsyqueStatusAssignment::And:
			return this->Assign(InLeft & InRight, InKind);

			default:
			return this->AssignValue(
				std::false_type(), InKind, InOperator, InLeft, InRight);
		}
	}

	/// @brief 実数の演算を行い、結果を状態値へ格納する。
	/// @retval true  成功。 演算結果を *this に格納した。
	/// @retval false 失敗。 *this は変化しない。
	private: template<typename TemplateLeft, typename TemplateRight>
	bool AssignValue(
		std::false_type,
		/// [in] 演算した結果の型。
		EPsyqueRulesStatusKind const InKind,
		/// [in] 適用する演算子。
		EPsyqueStatusAssignment const InOperator,
		/// [in] 演算子の左辺となる実数値。
		TemplateLeft const& InLeft,
		/// [in] 演算子の左辺となる実数値。
		TemplateRight const& InRight)
	PSYQUE_NOEXCEPT
	{
		switch (InOperator)
		{
			case EPsyqueStatusAssignment::Add:
			return this->Assign(InLeft + InRight, InKind);

			case EPsyqueStatusAssignment::Sub:
			return this->Assign(InLeft - InRight, InKind);

			case EPsyqueStatusAssignment::Mul:
			return this->Assign(InLeft * InRight, InKind);

			case EPsyqueStatusAssignment::Div:
			if (InRight == 0)
			{
				return false;
			}
			return this->Assign(InLeft / InRight, InKind);

			default: return false;
		}
	}

	//-------------------------------------------------------------------------
	/// @brief 浮動小数点数を比較する。
	/// @return 比較結果。
	private: static RulesEngine::EStatusOrder CompareFloat(
		/// [in] 左辺の浮動小数点数。
		typename ThisClass::FFloat const& InLeft,
		/// [in] 右辺の浮動小数点数。
		typename ThisClass::FFloat const& InRight)
	PSYQUE_NOEXCEPT
	{
#if 0
		/// @note 浮動小数点数の誤差を考慮せずに比較する。
		return ThisClass::CompareValue(InLeft, InRight);
#else
		/// @note 浮動小数点数の誤差を考慮して比較する。
		auto const LocalDiff(InLeft - InRight);
		auto const LocalEpsilon(
			std::numeric_limits<typename ThisClass::FFloat>::epsilon()
			* PSYQUE_RULES_ENGINE_STATUS_VALUE_EPSILON_MAG);
		return LocalDiff < -LocalEpsilon?
			RulesEngine::EStatusOrder::Less:
			LocalEpsilon < LocalDiff?
				RulesEngine::EStatusOrder::Greater:
				RulesEngine::EStatusOrder::Equal;
#endif
	}

	// @brief 浮動小数点数と値を比較する。
	/// @return 比較結果。
	private: template<typename TemplateValue>
	static RulesEngine::EStatusOrder CompareFloatLeft(
		/// [in] 左辺の浮動小数点数。
		typename ThisClass::FFloat const& InLeft,
		/// [in] 右辺の値。
		TemplateValue const& InRight)
	PSYQUE_NOEXCEPT
	{
		ThisClass const LocalRight(InRight, EPsyqueRulesStatusKind::Float);
		return LocalRight.GetKind() != EPsyqueRulesStatusKind::Float?
			RulesEngine::EStatusOrder::Failed:
			ThisClass::CompareFloat(InLeft, LocalRight.Float);
	}

	/// @brief 値と浮動小数点数を比較する。
	/// @return 比較結果。
	private: template<typename TemplateValue>
	static RulesEngine::EStatusOrder CompareFloatRight(
		/// [in] 左辺の値。
		TemplateValue const& InLeft,
		/// [in] 右辺の浮動小数点数。
		typename ThisClass::FFloat const& InRight)
	PSYQUE_NOEXCEPT
	{
		ThisClass const LocalLeft(InLeft, EPsyqueRulesStatusKind::Float);
		return LocalLeft.GetKind() != EPsyqueRulesStatusKind::Float?
			RulesEngine::EStatusOrder::Failed:
			ThisClass::CompareFloat(LocalLeft.Float, InRight);
	}

	/// @brief 値を比較する。
	/// @return 比較結果。
	private: template<typename TemplateValue>
	static RulesEngine::EStatusOrder CompareValue(
		/// [in] 左辺の値。
		TemplateValue const& InLeft,
		/// [in] 右辺の値。
		TemplateValue const& InRight)
	PSYQUE_NOEXCEPT
	{
		return InLeft < InRight?
			RulesEngine::EStatusOrder::Less:
			InRight < InLeft?
				RulesEngine::EStatusOrder::Greater:
				RulesEngine::EStatusOrder::Equal;
	}

	//-------------------------------------------------------------------------
	private: union
	{
		bool Bool;                              ///< 論理値。
		typename ThisClass::FUnsigned Unsigned; ///< 符号なし整数値。
		typename ThisClass::FSigned Signed;     ///< 符号あり整数値。
		typename ThisClass::FFloat Float;       ///< 浮動小数点数値。
	};
	private: typename ThisClass::FBitFormat BitFormat; ///< 状態値のビット構成。

}; // class Psyque::RulesEngine::_private::TStatusValue

// vim: set noexpandtab:
