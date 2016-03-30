// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TStatusValue
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include "./Enum.h"
#include "../BitAlgorithm.h"

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_VALUE_EPSILON_MAG
#define PSYQUE_IF_THEN_ENGINE_STATUS_VALUE_EPSILON_MAG 4
#endif // !default(PSYQUE_IF_THEN_ENGINE_STATUS_VALUE_EPSILON_MAG)

/// @cond
namespace Psyque
{
	namespace RuleEngine
	{
		namespace _private
		{
			template<typename, typename> class TStatusValue;
		} // namespace _private
	} // namespace RuleEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態値のやりとりに使う容れ物。
/// @tparam TemplateUnsigned @copydoc TStatusValue::FUnsigned
/// @tparam TemplateFloat	 @copydoc TStatusValue::FFloat
template<typename TemplateUnsigned, typename TemplateFloat>
class Psyque::RuleEngine::_private::TStatusValue
{
	private: using This = TStatusValue; ///< @copydoc TReservoir::This

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
		typename std::make_signed<typename This::FUnsigned>::type;
	/// @brief 状態値で扱う浮動小数点数の型。
	public: using FFloat = TemplateFloat;
	static_assert(
		std::is_floating_point<TemplateFloat>::value,
		"'TemplateFloat' is not floating-point FNumber.");
	static_assert(
		 sizeof(TemplateFloat) <= sizeof(TemplateUnsigned),
		 "sizeof(TemplateFloat) is greater than sizeof(TemplateUnsigned).");

	//-------------------------------------------------------------------------
	/// @name 構築
	/// @{

	/// @brief 空値を構築する。
	public: TStatusValue() PSYQUE_NOEXCEPT:
	Kind(RuleEngine::EStatusKind::Empty) {}

	/// @brief 論理型の値を構築する。
	public: explicit TStatusValue(
		/// [in] 初期値となる論理値。
		bool const InBool)
	PSYQUE_NOEXCEPT: Kind(RuleEngine::EStatusKind::Bool)
	{
		this->Bool = InBool;
	}

	/// @brief 符号なし整数型の値を構築する。
	public: explicit TStatusValue(
		/// [in] 初期値となる符号なし整数。
		typename This::FUnsigned const& InUnsigned)
	PSYQUE_NOEXCEPT: Kind(RuleEngine::EStatusKind::Unsigned)
	{
		this->Unsigned = InUnsigned;
	}

	/// @brief 符号あり整数型の値を構築する。
	public: explicit TStatusValue(
		/// [in] 初期値となる符号あり整数。
		typename This::FSigned const& InSigned)
	PSYQUE_NOEXCEPT: Kind(RuleEngine::EStatusKind::Signed)
	{
		this->Signed = InSigned;
	}

	/// @brief 浮動小数点数型の値を構築する。
	public: explicit TStatusValue(
		/// [in] 初期値となる浮動小数点数。
		typename This::FFloat const& InFloat)
	PSYQUE_NOEXCEPT: Kind(RuleEngine::EStatusKind::Float)
	{
		this->Float = InFloat;
	}

	/// @brief 任意型の値を構築する。
	public: template<typename TemplateValue>
	explicit TStatusValue(
		/// [in] 初期値。
		TemplateValue const& InValue,
		/// [in] 値の型。 RuleEngine::EStatusKind::Empty の場合は、自動で決定する。
		RuleEngine::EStatusKind const InKind = RuleEngine::EStatusKind::Empty)
	PSYQUE_NOEXCEPT: Kind(RuleEngine::EStatusKind::Empty)
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
		return this->GetKind() == RuleEngine::EStatusKind::Empty;
	}

	/// @brief 論理値を取得する。
	/// @return 論理値を指すポインタ。論理値が格納されてない場合は nullptr を返す。
	public: bool const* GetBool() const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == RuleEngine::EStatusKind::Bool?
			&this->Bool: nullptr;
	}

	/// @brief 符号なし整数値を取得する。
	/// @return
	/// 符号なし整数値を指すポインタ。
	/// 符号なし整数値が格納されてない場合は nullptr を返す。
	public: typename This::FUnsigned const* GetUnsigned()
	const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == RuleEngine::EStatusKind::Unsigned?
			&this->Unsigned: nullptr;
	}

	/// @brief 符号あり整数値を取得する。
	/// @return
	/// 符号あり整数値を指すポインタ。
	/// 符号あり整数値が格納されてない場合は nullptr を返す。
	public: typename This::FSigned const* GetSigned() const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == RuleEngine::EStatusKind::Signed?
			&this->Signed: nullptr;
	}

	/// @brief 浮動小数点数値を取得する。
	/// @return
	/// 浮動小数点数値を指すポインタ。
	/// 浮動小数点数値が格納されてない場合は nullptr を返す。
	public: typename This::FFloat const* GetFloat() const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == RuleEngine::EStatusKind::Float?
			&this->Float: nullptr;
	}

	public: typename This::FUnsigned GetBitset() const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case RuleEngine::EStatusKind::Empty:
			return 0;

			case RuleEngine::EStatusKind::Bool:
			return this->Bool;

			case RuleEngine::EStatusKind::Float:
			using FFloatBitset = Psyque::TFloatBitset<typename This::FFloat>;
			return FFloatBitset(this->Float).Bitset;

			default:
			return this->Unsigned;
		}
	}

	/// @brief 格納値の型の種類を取得する。
	/// @return 格納値の型の種類。
	public: RuleEngine::EStatusKind GetKind() const PSYQUE_NOEXCEPT
	{
		return this->Kind;
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
	Psyque::ETernary Compare(
		/// [in] 比較演算子の種類。
		RuleEngine::EStatusComparison const InComparison,
		/// [in] 比較演算子の右辺値。
		TemplateRight const& InRight)
	const PSYQUE_NOEXCEPT
	{
		auto const LocalOrder(this->Compare(InRight));
		if (LocalOrder != RuleEngine::EStatusOrder::Failed)
		{
			switch (InComparison)
			{
				case RuleEngine::EStatusComparison::Equal:
				return static_cast<Psyque::ETernary>(
					LocalOrder == RuleEngine::EStatusOrder::Equal);

				case RuleEngine::EStatusComparison::NotEqual:
				return static_cast<Psyque::ETernary>(
					LocalOrder != RuleEngine::EStatusOrder::Equal);

				case RuleEngine::EStatusComparison::Less:
				return static_cast<Psyque::ETernary>(
					LocalOrder == RuleEngine::EStatusOrder::Less);

				case RuleEngine::EStatusComparison::LessEqual:
				return static_cast<Psyque::ETernary>(
					LocalOrder != RuleEngine::EStatusOrder::Greater);

				case RuleEngine::EStatusComparison::Greater:
				return static_cast<Psyque::ETernary>(
					LocalOrder == RuleEngine::EStatusOrder::Greater);

				case RuleEngine::EStatusComparison::GreaterEqual:
				return static_cast<Psyque::ETernary>(
					LocalOrder != RuleEngine::EStatusOrder::Less);

				default:
				check(false);
				break;
			}
		}
		return Psyque::ETernary::Unknown;
	}

	/// @brief 値を比較する。
	/// @return *this を左辺値とした比較結果。
	public: RuleEngine::EStatusOrder Compare(
		/// [in] 右辺値。
		This const& InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (InRight.GetKind())
		{
			case RuleEngine::EStatusKind::Bool:
			return this->Compare(InRight.Bool);

			case RuleEngine::EStatusKind::Unsigned:
			return this->Compare(InRight.Unsigned);

			case RuleEngine::EStatusKind::Signed:
			return this->Compare(InRight.Signed);

			case RuleEngine::EStatusKind::Float:
			return this->Compare(InRight.Float);

			default:
			return RuleEngine::EStatusOrder::Failed;
		}
	}

	/// @brief 論理値と比較する。
	/// @return *this を左辺値とした比較結果。
	public: RuleEngine::EStatusOrder Compare(
		/// [in] 右辺値となる論理値。
		bool const InRight)
	const PSYQUE_NOEXCEPT
	{
		if (this->GetBool() == nullptr)
		{
			return RuleEngine::EStatusOrder::Failed;
		}
		if (this->Bool == InRight)
		{
			return RuleEngine::EStatusOrder::Equal;
		}
		if (this->Bool)
		{
			return RuleEngine::EStatusOrder::Greater;
		}
		return RuleEngine::EStatusOrder::Less;
	}

	/// @brief 符号なし整数と比較する。
	/// @return *this を左辺値とした比較結果。
	public: RuleEngine::EStatusOrder Compare(
		/// [in] 右辺値となる符号なし整数。
		typename This::FUnsigned const& InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case RuleEngine::EStatusKind::Unsigned:
			return This::CompareValue(this->Unsigned, InRight);

			case RuleEngine::EStatusKind::Signed:
			return this->Signed < 0?
				RuleEngine::EStatusOrder::Less:
				This::CompareValue(this->Unsigned, InRight);

			case RuleEngine::EStatusKind::Float:
			return This::CompareFloatLeft(this->Float, InRight);

			default: return RuleEngine::EStatusOrder::Failed;
		}
	}

	/// @brief 符号あり整数と比較する。
	/// @return *this を左辺値とした比較結果。
	public: RuleEngine::EStatusOrder Compare(
		/// [in] 右辺値となる符号あり整数。
		typename This::FSigned const& InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case RuleEngine::EStatusKind::Unsigned:
			return InRight < 0?
				RuleEngine::EStatusOrder::Greater:
				This::CompareValue(
					this->Unsigned,
					static_cast<typename This::FUnsigned>(InRight));

			case RuleEngine::EStatusKind::Signed:
			return This::CompareValue(this->Signed, InRight);

			case RuleEngine::EStatusKind::Float:
			return This::CompareFloatLeft(this->Float, InRight);

			default: return RuleEngine::EStatusOrder::Failed;
		}
	}

	/// @brief 浮動小数点数と比較する。
	/// @return *this を左辺値とした比較結果。
	public: RuleEngine::EStatusOrder Compare(
		/// [in] 右辺値となる浮動小数点数。
		typename This::FFloat const& InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case RuleEngine::EStatusKind::Unsigned:
			return InRight < 0?
				RuleEngine::EStatusOrder::Greater:
				This::CompareFloatRight(this->Unsigned, InRight);

			case RuleEngine::EStatusKind::Signed:
			return This::CompareFloatRight(this->Signed, InRight);

			case RuleEngine::EStatusKind::Float:
			return This::CompareFloat(this->Float, InRight);

			default: return RuleEngine::EStatusOrder::Failed;
		}
	}

	/// @brief 値を比較する。
	/// @return *this を左辺値とした比較結果。
	public: template<typename TemplateRight>
	RuleEngine::EStatusOrder Compare(
		/// [in] 右辺値。
		TemplateRight const& InRight)
	const PSYQUE_NOEXCEPT
	{
		if (std::is_floating_point<TemplateRight>::value)
		{
			auto const LocalRight(
				static_cast<typename This::FFloat>(InRight));
			if (LocalRight == InRight)
			{
				return this->Compare(LocalRight);
			}
		}
		else if (std::is_signed<TemplateRight>::value)
		{
			auto const LocalRight(
				static_cast<typename This::FSigned>(InRight));
			if (LocalRight == InRight)
			{
				return this->Compare(LocalRight);
			}
		}
		else if (std::is_unsigned<TemplateRight>::value)
		{
			auto const LocalRight(
				static_cast<typename This::FUnsigned>(InRight));
			if (LocalRight == InRight)
			{
				return this->Compare(LocalRight);
			}
		}
		return RuleEngine::EStatusOrder::Failed;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 値の代入
	/// @{

	/// @brief 状態値を空にする。
	public: void AssignEmpty() PSYQUE_NOEXCEPT
	{
		this->Kind = RuleEngine::EStatusKind::Empty;
	}

	/// @brief 論理値を代入する。
	public: void AssignBool(
		/// [in] 代入する論理値。
		bool const InValue)
	PSYQUE_NOEXCEPT
	{
		this->Bool = InValue;
		this->Kind = RuleEngine::EStatusKind::Bool;
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
			auto const local_unsigned(
				static_cast<typename This::FUnsigned>(InValue));
			if (static_cast<TemplateValue>(local_unsigned) == InValue)
			{
				this->Unsigned = local_unsigned;
				this->Kind = RuleEngine::EStatusKind::Unsigned;
				return true;
			}
		}
		return false;
	}

	/// @brief 符号あり整数を代入する。
	/// @retval true 成功。 InValue を *this に代入した。
	/// @retval false
	/// 失敗。 InValue を符号あり整数に変換できなかった。 *this は変化しない。
	public: template<typename TemplateValue>
	bool AssignSigned(
		/// [in] 代入する値。
		TemplateValue const& InValue)
	PSYQUE_NOEXCEPT
	{
		auto const LocalSigned(static_cast<typename This::FSigned>(InValue));
		if (InValue <= 0 || 0 <= LocalSigned)
		{
			if (static_cast<TemplateValue>(LocalSigned) == InValue)
			{
				this->Signed = LocalSigned;
				this->Kind = RuleEngine::EStatusKind::Signed;
				return true;
			}
		}
		return false;
	}

	/// @brief 浮動小数点数を代入する。
	/// @retval true 成功。 InValue を *this に代入した。
	/// @retval false
	/// 失敗。 InValue を浮動小数点数に変換できなかった。 *this は変化しない。
	public: template<typename TemplateValue>
	bool AssignFloat(
		/// [in] 代入する値。
		TemplateValue const& InValue)
	PSYQUE_NOEXCEPT
	{
		auto const LocalFloat(
			static_cast<typename This::FFloat>(InValue));
		auto const LocalDiff(
			static_cast<TemplateValue>(LocalFloat) - InValue);
		auto const LocalEpsilon(
			std::numeric_limits<typename This::FFloat>::epsilon()
			* PSYQUE_IF_THEN_ENGINE_STATUS_VALUE_EPSILON_MAG);
		if (-LocalEpsilon <= LocalDiff && LocalDiff <= LocalEpsilon)
		{
			this->Float = LocalFloat;
			this->Kind = RuleEngine::EStatusKind::Float;
			return true;
		}
		return false;
	}

	/// @brief 値を代入する。
	/// @retval true 成功。 InValue を *this に代入した。
	/// @retval false
	/// 失敗。 InValue を状態値に変換できなかった。 *this は変化しない。
	public: template<typename TemplateValue>
	bool Assign(
		/// [in] 代入する値。
		TemplateValue const& InValue,
		/// [in] 代入する型。 RuleEngine::EStatusKind::Empty の場合は、自動で決定する。
		RuleEngine::EStatusKind InKind = RuleEngine::EStatusKind::Empty)
	{
		if (InKind == RuleEngine::EStatusKind::Empty)
		{
			InKind = This::template classify_kind<TemplateValue>();
		}
		switch (InKind)
		{
			case RuleEngine::EStatusKind::Unsigned:
			return this->AssignUnsigned(InValue);

			case RuleEngine::EStatusKind::Signed:
			return this->AssignSigned(InValue);

			case RuleEngine::EStatusKind::Float:
			return this->AssignFloat(InValue);

			default:
			return false;
		}
	}

	/// @copydoc Assign
	public: bool Assign(
		This const& InValue,
		RuleEngine::EStatusKind InKind = RuleEngine::EStatusKind::Empty)
	PSYQUE_NOEXCEPT
	{
		if (InKind == RuleEngine::EStatusKind::Empty)
		{
			InKind = InValue.GetKind();
		}
		switch (InValue.GetKind())
		{
			case RuleEngine::EStatusKind::Bool:
			return this->Assign(InValue.Bool, InKind);

			case RuleEngine::EStatusKind::Unsigned:
			return this->Assign(InValue.Unsigned, InKind);

			case RuleEngine::EStatusKind::Signed:
			return this->Assign(InValue.Signed, InKind);

			case RuleEngine::EStatusKind::Float:
			return this->Assign(InValue.Float, InKind);

			default:
			this->AssignEmpty();
			return true;
		}
	}

	/// @copydoc Assign
	public: bool Assign(
		bool const InValue,
		RuleEngine::EStatusKind const InKind = RuleEngine::EStatusKind::Bool)
	{
		switch (InKind)
		{
			case RuleEngine::EStatusKind::Empty:
			case RuleEngine::EStatusKind::Bool:
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
		RuleEngine::EStatusAssignment const InOperator,
		/// [in] 代入演算子の右辺。
		TemplateValue const& InRight)
	PSYQUE_NOEXCEPT
	{
		static_assert(!std::is_same<TemplateValue, bool>::value, "");
		auto const LocalKind(this->GetKind());
		if (InOperator == RuleEngine::EStatusAssignment::Copy)
		{
			return this->Assign(InRight, LocalKind);
		}
		switch (LocalKind)
		{
			case RuleEngine::EStatusKind::Unsigned:
			return this->AssignValue(
				std::is_integral<TemplateValue>(),
				LocalKind,
				InOperator,
				this->Unsigned,
				InRight);

			case RuleEngine::EStatusKind::Signed:
			return this->AssignValue(
				std::is_integral<TemplateValue>(),
				LocalKind,
				InOperator,
				this->Signed,
				InRight);

			case RuleEngine::EStatusKind::Float:
			return this->AssignValue(
				std::false_type(),
				LocalKind,
				InOperator,
				this->Float,
				InRight);

			default: return false;
		}
	}

	/// @copydoc Assign(RuleEngine::EStatusAssignment const, TemplateValue const&)
	public: bool Assign(
		RuleEngine::EStatusAssignment const InOperator,
		This const& InRight)
	PSYQUE_NOEXCEPT
	{
		switch (InRight.GetKind())
		{
			case RuleEngine::EStatusKind::Bool:
			return this->Assign(InOperator, InRight.Bool);

			case RuleEngine::EStatusKind::Unsigned:
			return this->Assign(InOperator, InRight.Unsigned);

			case RuleEngine::EStatusKind::Signed:
			return this->Assign(InOperator, InRight.Signed);

			case RuleEngine::EStatusKind::Float:
			return this->Assign(InOperator, InRight.Float);

			default: return false;
		}
	}

	/// @copydoc Assign(RuleEngine::EStatusAssignment const, TemplateValue const&)
	public: bool Assign(
		RuleEngine::EStatusAssignment const InOperator,
		bool const InRight)
	PSYQUE_NOEXCEPT
	{
		if (this->GetKind() != RuleEngine::EStatusKind::Bool)
		{
			return false;
		}
		switch (InOperator)
		{
			case RuleEngine::EStatusAssignment::Copy:
			this->Bool  = InRight;
			break;

			case RuleEngine::EStatusAssignment::Or:
			this->Bool |= InRight;
			break;

			case RuleEngine::EStatusAssignment::Xor:
			this->Bool ^= InRight;
			break;

			case RuleEngine::EStatusAssignment::And:
			this->Bool &= InRight;
			break;

			default: return false;
		}
		return true;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @brief 型の種類を決定する。
	/// @tparam TemplateValue 型。
	/// @return 型の種類。
	private: template<typename TemplateValue>
	static RuleEngine::EStatusKind classify_kind() PSYQUE_NOEXCEPT
	{
		if (std::is_same<TemplateValue, bool>::value)
		{
			return RuleEngine::EStatusKind::Bool;
		}
		else if (std::is_floating_point<TemplateValue>::value)
		{
			return RuleEngine::EStatusKind::Float;
		}
		else if (std::is_integral<TemplateValue>::value)
		{
			return std::is_unsigned<TemplateValue>::value?
				RuleEngine::EStatusKind::Unsigned:
				RuleEngine::EStatusKind::Signed;
		}
		return RuleEngine::EStatusKind::Empty;
	}

	//-------------------------------------------------------------------------
	/// @brief This::AssignUnsigned で論理値を設定させないためのダミー関数。
	private: bool AssignUnsigned(bool const);
	/// @brief This::AssignSigned で論理値を設定させないためのダミー関数。
	private: bool AssignSigned(bool const);
	/// @brief This::AssignFloat で論理値を設定させないためのダミー関数。
	private: bool AssignFloat(bool const);

	//-------------------------------------------------------------------------
	/// @brief 整数の演算を行い、結果を状態値へ格納する。
	/// @retval true  成功。 演算結果を *this に格納した。
	/// @retval false 失敗。 *this は変化しない。
	private: template<typename TemplateLeft, typename TemplateRight>
	bool AssignValue(
		std::true_type,
		/// [in] 演算した結果の型。
		RuleEngine::EStatusKind const InKind,
		/// [in] 適用する演算子。
		RuleEngine::EStatusAssignment const InOperator,
		/// [in] 演算子の左辺となる整数値。
		TemplateLeft const& InLeft,
		/// [in] 演算子の左辺となる整数値。
		TemplateRight const& InRight)
	PSYQUE_NOEXCEPT
	{
		switch (InOperator)
		{
			case RuleEngine::EStatusAssignment::Mod:
			if (InRight == 0)
			{
				return false;
			}
			return this->Assign(InLeft % InRight, InKind);

			case RuleEngine::EStatusAssignment::Or:
			return this->Assign(InLeft | InRight, InKind);

			case RuleEngine::EStatusAssignment::Xor:
			return this->Assign(InLeft ^ InRight, InKind);

			case RuleEngine::EStatusAssignment::And:
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
		RuleEngine::EStatusKind const InKind,
		/// [in] 適用する演算子。
		RuleEngine::EStatusAssignment const InOperator,
		/// [in] 演算子の左辺となる実数値。
		TemplateLeft const& InLeft,
		/// [in] 演算子の左辺となる実数値。
		TemplateRight const& InRight)
	PSYQUE_NOEXCEPT
	{
		switch (InOperator)
		{
			case RuleEngine::EStatusAssignment::Add:
			return this->Assign(InLeft + InRight, InKind);

			case RuleEngine::EStatusAssignment::Sub:
			return this->Assign(InLeft - InRight, InKind);

			case RuleEngine::EStatusAssignment::Mul:
			return this->Assign(InLeft * InRight, InKind);

			case RuleEngine::EStatusAssignment::Div:
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
	private: static RuleEngine::EStatusOrder CompareFloat(
		/// [in] 左辺の浮動小数点数。
		typename This::FFloat const& InLeft,
		/// [in] 右辺の浮動小数点数。
		typename This::FFloat const& InRight)
	PSYQUE_NOEXCEPT
	{
#if 0
		/// @note 浮動小数点数の誤差を考慮せずに比較する。
		return This::CompareValue(InLeft, InRight);
#else
		/// @note 浮動小数点数の誤差を考慮して比較する。
		auto const LocalDiff(InLeft - InRight);
		auto const LocalEpsilon(
			std::numeric_limits<typename This::FFloat>::epsilon()
			* PSYQUE_IF_THEN_ENGINE_STATUS_VALUE_EPSILON_MAG);
		return LocalDiff < -LocalEpsilon?
			RuleEngine::EStatusOrder::Less:
			LocalEpsilon < LocalDiff?
				RuleEngine::EStatusOrder::Greater:
				RuleEngine::EStatusOrder::Equal;
#endif
	}

	// @brief 浮動小数点数と値を比較する。
	/// @return 比較結果。
	private: template<typename TemplateValue>
	static RuleEngine::EStatusOrder CompareFloatLeft(
		/// [in] 左辺の浮動小数点数。
		typename This::FFloat const& InLeft,
		/// [in] 右辺の値。
		TemplateValue const& InRight)
	PSYQUE_NOEXCEPT
	{
		This const LocalRight(InRight, RuleEngine::EStatusKind::Float);
		return LocalRight.GetKind() != RuleEngine::EStatusKind::Float?
			RuleEngine::EStatusOrder::Failed:
			This::CompareFloat(InLeft, LocalRight.Float);
	}

	/// @brief 値と浮動小数点数を比較する。
	/// @return 比較結果。
	private: template<typename TemplateValue>
	static RuleEngine::EStatusOrder CompareFloatRight(
		/// [in] 左辺の値。
		TemplateValue const& InLeft,
		/// [in] 右辺の浮動小数点数。
		typename This::FFloat const& InRight)
	PSYQUE_NOEXCEPT
	{
		This const LocalLeft(InLeft, RuleEngine::EStatusKind::Float);
		return LocalLeft.GetKind() != RuleEngine::EStatusKind::Float?
			RuleEngine::EStatusOrder::Failed:
			This::CompareFloat(LocalLeft.Float, InRight);
	}

	/// @brief 値を比較する。
	/// @return 比較結果。
	private: template<typename TemplateValue>
	static RuleEngine::EStatusOrder CompareValue(
		/// [in] 左辺の値。
		TemplateValue const& InLeft,
		/// [in] 右辺の値。
		TemplateValue const& InRight)
	PSYQUE_NOEXCEPT
	{
		return InLeft < InRight?
			RuleEngine::EStatusOrder::Less:
			InRight < InLeft?
				RuleEngine::EStatusOrder::Greater:
				RuleEngine::EStatusOrder::Equal;
	}

	//-------------------------------------------------------------------------
	private: union
	{
		bool Bool;                         ///< 論理値。
		typename This::FUnsigned Unsigned; ///< 符号なし整数値。
		typename This::FSigned Signed;     ///< 符号あり整数値。
		typename This::FFloat Float;       ///< 浮動小数点数値。
	};
	private: RuleEngine::EStatusKind Kind; ///< 状態値の型の種類。

}; // class Psyque::RuleEngine::_private::TStatusValue

// vim: set noexpandtab:
