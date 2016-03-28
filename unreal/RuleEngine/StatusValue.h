// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TStatusValue
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include "../BitAlgorithm.h"

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_VALUE_EPSILON_MAG
#define PSYQUE_IF_THEN_ENGINE_STATUS_VALUE_EPSILON_MAG 4
#endif // !default(PSYQUE_IF_THEN_ENGINE_STATUS_VALUE_EPSILON_MAG)

namespace Psyque
{
	/// @brief if-then規則で駆動する有限状態機械。
	namespace RuleEngine
	{
		/// @brief Psyque::RuleEngine の管理者以外は、直接アクセス禁止。
		namespace _private
		{
			/// @cond
			template<typename, typename> class TStatusValue;
			/// @endcond
		} // namespace _private
	} // namespace RuleEngine
} // namespace Psyque

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
	/// @brief 状態値の型の種別。
	public: struct EKind
	{
		enum Type: int8
		{
			Signed = -2, ///< 符号あり整数。
			Float,		 ///< 浮動小数点数。
			Empty,		 ///< 空。
			Bool,		 ///< 論理値。
			Unsigned,	 ///< 符号なし整数。
		};
	};

	//-------------------------------------------------------------------------
	/// @brief 比較式の評価。
	/// @details
	/// - 正なら、比較式の評価は真。
	/// - 0 なら、比較式の評価は偽。
	/// - 負なら、比較式の評価に失敗。
	public: using FEvaluation = int8;
	/// @brief 値の大小関係。
	public: struct EOrder
	{
		enum Type: int8
		{
			None = -2, ///< 比較に失敗。
			Less,      ///< 左辺のほうが小さい。
			Equal,     ///< 左辺と右辺は等価。
			Greater,   ///< 左辺のほうが大きい。
		};
	};
	/// @brief 状態値を比較する演算子の種類。
	public: struct EComparison
	{
		enum Type: uint8
		{
			Equal,        ///< 等価。
			NotEqual,     ///< 非等価。
			Less,         ///< 小なり。
			LessEqual,    ///< 以下。
			Greater,      ///< 大なり。
			GreaterEqual, ///< 以上。
		};
	};
	/// @brief 状態値を代入する演算子の種類。
	public: struct EAssignment
	{
		enum Type: uint8
		{
			Copy, ///< 単純代入。
			Add,  ///< 加算代入。
			Sub,  ///< 減算代入。
			Mul,  ///< 乗算代入。
			Div,  ///< 除算代入。
			Mod,  ///< 除算の余りの代入。
			Or,   ///< 論理和の代入。
			Xor,  ///< 排他的論理和の代入。
			And,  ///< 論理積の代入。
		};
	};

	//-------------------------------------------------------------------------
	/// @name 構築
	/// @{

	/// @brief 空値を構築する。
	public: TStatusValue() PSYQUE_NOEXCEPT: Kind(This::EKind::Empty) {}

	/// @brief 論理型の値を構築する。
	public: explicit TStatusValue(
		/// [in] 初期値となる論理値。
		bool const in_bool)
	PSYQUE_NOEXCEPT: Kind(This::EKind::Bool)
	{
		this->Bool = in_bool;
	}

	/// @brief 符号なし整数型の値を構築する。
	public: explicit TStatusValue(
		/// [in] 初期値となる符号なし整数。
		typename This::FUnsigned const& InUnsigned)
	PSYQUE_NOEXCEPT: Kind(This::EKind::Unsigned)
	{
		this->Unsigned = InUnsigned;
	}

	/// @brief 符号あり整数型の値を構築する。
	public: explicit TStatusValue(
		/// [in] 初期値となる符号あり整数。
		typename This::FSigned const& InSigned)
	PSYQUE_NOEXCEPT: Kind(This::EKind::Signed)
	{
		this->Signed = InSigned;
	}

	/// @brief 浮動小数点数型の値を構築する。
	public: explicit TStatusValue(
		/// [in] 初期値となる浮動小数点数。
		typename This::FFloat const& InFloat)
	PSYQUE_NOEXCEPT: Kind(This::EKind::Float)
	{
		this->Float = InFloat;
	}

	/// @brief 任意型の値を構築する。
	public: template<typename TemplateValue>
	explicit TStatusValue(
		/// [in] 初期値。
		TemplateValue const& InValue,
		/// [in] 値の型。 This::EKind::Empty の場合は、自動で決定する。
		typename This::EKind::Type const InKind = This::EKind::Empty)
	PSYQUE_NOEXCEPT: Kind(This::EKind::Empty)
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
		return this->GetKind() == This::EKind::Empty;
	}

	/// @brief 論理値を取得する。
	/// @return 論理値を指すポインタ。論理値が格納されてない場合は nullptr を返す。
	public: bool const* GetBool() const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == This::EKind::Bool? &this->Bool: nullptr;
	}

	/// @brief 符号なし整数値を取得する。
	/// @return
	/// 符号なし整数値を指すポインタ。
	/// 符号なし整数値が格納されてない場合は nullptr を返す。
	public: typename This::FUnsigned const* GetUnsigned()
	const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == This::EKind::Unsigned?
			&this->Unsigned: nullptr;
	}

	/// @brief 符号あり整数値を取得する。
	/// @return
	/// 符号あり整数値を指すポインタ。
	/// 符号あり整数値が格納されてない場合は nullptr を返す。
	public: typename This::FSigned const* GetSigned() const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == This::EKind::Signed? &this->Signed: nullptr;
	}

	/// @brief 浮動小数点数値を取得する。
	/// @return
	/// 浮動小数点数値を指すポインタ。
	/// 浮動小数点数値が格納されてない場合は nullptr を返す。
	public: typename This::FFloat const* GetFloat() const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == This::EKind::Float? &this->Float: nullptr;
	}

	public: typename This::FUnsigned GetBitset() const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case This::EKind::Empty:
			return 0;

			case This::EKind::Bool:
			return this->Bool;

			case This::EKind::Float:
			using FFloatBitset = Psyque::TFloatBitset<typename This::FFloat>;
			return FFloatBitset(this->Float).Bitset;

			default:
			return this->Unsigned;
		}
	}

	/// @brief 格納値の型の種類を取得する。
	/// @return 格納値の型の種類。
	public: typename This::EKind::Type GetKind() const PSYQUE_NOEXCEPT
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
	typename This::FEvaluation Compare(
		/// [in] 比較演算子の種類。
		typename This::EComparison::Type const InComparison,
		/// [in] 比較演算子の右辺値。
		TemplateRight const& InRight)
	const PSYQUE_NOEXCEPT
	{
		auto const LocalOrder(this->Compare(InRight));
		if (LocalOrder != This::EOrder::None)
		{
			switch (InComparison)
			{
				case This::EComparison::Equal:
				return LocalOrder == This::EOrder::Equal;

				case This::EComparison::NotEqual:
				return LocalOrder != This::EOrder::Equal;

				case This::EComparison::Less:
				return LocalOrder == This::EOrder::Less;

				case This::EComparison::LessEqual:
				return LocalOrder != This::EOrder::Greater;

				case This::EComparison::Greater:
				return LocalOrder == This::EOrder::Greater;

				case This::EComparison::GreaterEqual:
				return LocalOrder != This::EOrder::Less;

				default:
				check(false);
				break;
			}
		}
		return -1;
	}

	/// @brief 値を比較する。
	/// @return *this を左辺値とした比較結果。
	public: typename This::EOrder::Type Compare(
		/// [in] 右辺値。
		This const& InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (InRight.GetKind())
		{
			case This::EKind::Bool:		return this->Compare(InRight.Bool);
			case This::EKind::Unsigned: return this->Compare(InRight.Unsigned);
			case This::EKind::Signed:	return this->Compare(InRight.Signed);
			case This::EKind::Float:	return this->Compare(InRight.Float);
			default:					return This::EOrder::None;
		}
	}

	/// @brief 論理値と比較する。
	/// @return *this を左辺値とした比較結果。
	public: typename This::EOrder::Type Compare(
		/// [in] 右辺値となる論理値。
		bool const InRight)
	const PSYQUE_NOEXCEPT
	{
		if (this->GetBool() == nullptr)
		{
			return This::EOrder::None;
		}
		if (this->Bool == InRight)
		{
			return This::EOrder::Equal;
		}
		if (this->Bool)
		{
			return This::EOrder::Greater;
		}
		return This::EOrder::Less;
	}

	/// @brief 符号なし整数と比較する。
	/// @return *this を左辺値とした比較結果。
	public: typename This::EOrder::Type Compare(
		/// [in] 右辺値となる符号なし整数。
		typename This::FUnsigned const& InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case This::EKind::Unsigned:
			return This::CompareValue(this->Unsigned, InRight);

			case This::EKind::Signed:
			return this->Signed < 0?
				This::EOrder::Less:
				This::CompareValue(this->Unsigned, InRight);

			case This::EKind::Float:
			return This::CompareFloatLeft(this->Float, InRight);

			default: return This::EOrder::None;
		}
	}

	/// @brief 符号あり整数と比較する。
	/// @return *this を左辺値とした比較結果。
	public: typename This::EOrder::Type Compare(
		/// [in] 右辺値となる符号あり整数。
		typename This::FSigned const& InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case This::EKind::Unsigned:
			return InRight < 0?
				This::EOrder::Greater:
				This::CompareValue(
					this->Unsigned,
					static_cast<typename This::FUnsigned>(InRight));

			case This::EKind::Signed:
			return This::CompareValue(this->Signed, InRight);

			case This::EKind::Float:
			return This::CompareFloatLeft(this->Float, InRight);

			default: return This::EOrder::None;
		}
	}

	/// @brief 浮動小数点数と比較する。
	/// @return *this を左辺値とした比較結果。
	public: typename This::EOrder::Type Compare(
		/// [in] 右辺値となる浮動小数点数。
		typename This::FFloat const& InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case This::EKind::Unsigned:
			return InRight < 0?
				This::EOrder::Greater:
				This::CompareFloatRight(this->Unsigned, InRight);

			case This::EKind::Signed:
			return This::CompareFloatRight(this->Signed, InRight);

			case This::EKind::Float:
			return This::CompareFloat(this->Float, InRight);

			default: return This::EOrder::None;
		}
	}

	/// @brief 値を比較する。
	/// @return *this を左辺値とした比較結果。
	public: template<typename TemplateRight>
	typename This::EOrder::Type Compare(
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
		return This::EOrder::None;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 値の代入
	/// @{

	/// @brief 状態値を空にする。
	public: void AssignEmpty() PSYQUE_NOEXCEPT
	{
		this->Kind = This::EKind::Empty;
	}

	/// @brief 論理値を代入する。
	public: void AssignBool(
		/// [in] 代入する論理値。
		bool const InValue)
	PSYQUE_NOEXCEPT
	{
		this->Bool = InValue;
		this->Kind = This::EKind::Bool;
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
				this->Kind = This::EKind::Unsigned;
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
				this->Kind = This::EKind::Signed;
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
			this->Kind = This::EKind::Float;
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
		/// [in] 代入する型。 This::EKind::Empty の場合は、自動で決定する。
		typename This::EKind::Type InKind = This::EKind::Empty)
	{
		if (InKind == This::EKind::Empty)
		{
			InKind = This::template classify_kind<TemplateValue>();
		}
		switch (InKind)
		{
			case This::EKind::Unsigned: return this->AssignUnsigned(InValue);
			case This::EKind::Signed:	return this->AssignSigned(InValue);
			case This::EKind::Float:	return this->AssignFloat(InValue);
			default:					   return false;
		}
	}

	/// @copydoc Assign
	public: bool Assign(
		This const& InValue,
		typename This::EKind::Type InKind = This::EKind::Empty)
	PSYQUE_NOEXCEPT
	{
		if (InKind == This::EKind::Empty)
		{
			InKind = InValue.GetKind();
		}
		switch (InValue.GetKind())
		{
			case This::EKind::Bool:
			return this->Assign(InValue.Bool, InKind);

			case This::EKind::Unsigned:
			return this->Assign(InValue.Unsigned, InKind);

			case This::EKind::Signed:
			return this->Assign(InValue.Signed, InKind);

			case This::EKind::Float:
			return this->Assign(InValue.Float, InKind);

			default:
			this->AssignEmpty();
			return true;
		}
	}

	/// @copydoc Assign
	public: bool Assign(
		bool const InValue,
		typename This::EKind::Type const InKind = This::EKind::Bool)
	{
		switch (InKind)
		{
			case This::EKind::Empty:
			case This::EKind::Bool:
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
		typename This::EAssignment::Type const InOperator,
		/// [in] 代入演算子の右辺。
		TemplateValue const& InRight)
	PSYQUE_NOEXCEPT
	{
		static_assert(!std::is_same<TemplateValue, bool>::value, "");
		auto const LocalKind(this->GetKind());
		if (InOperator == This::EAssignment::Copy)
		{
			return this->Assign(InRight, LocalKind);
		}
		switch (LocalKind)
		{
			case This::EKind::Unsigned:
			return this->AssignValue(
				std::is_integral<TemplateValue>(),
				LocalKind,
				InOperator,
				this->Unsigned,
				InRight);

			case This::EKind::Signed:
			return this->AssignValue(
				std::is_integral<TemplateValue>(),
				LocalKind,
				InOperator,
				this->Signed,
				InRight);

			case This::EKind::Float:
			return this->AssignValue(
				std::false_type(),
				LocalKind,
				InOperator,
				this->Float,
				InRight);

			default: return false;
		}
	}

	/// @copydoc Assign(This::EAssignment::Type const, TemplateValue const&)
	public: bool Assign(
		typename This::EAssignment::Type const InOperator,
		This const& InRight)
	PSYQUE_NOEXCEPT
	{
		switch (InRight.GetKind())
		{
			case This::EKind::Bool:
			return this->Assign(InOperator, InRight.Bool);

			case This::EKind::Unsigned:
			return this->Assign(InOperator, InRight.Unsigned);

			case This::EKind::Signed:
			return this->Assign(InOperator, InRight.Signed);

			case This::EKind::Float:
			return this->Assign(InOperator, InRight.Float);

			default: return false;
		}
	}

	/// @copydoc Assign(This::EAssignment::Type const, TemplateValue const&)
	public: bool Assign(
		typename This::EAssignment::Type const InOperator,
		bool const InRight)
	PSYQUE_NOEXCEPT
	{
		if (this->GetKind() != This::EKind::Bool)
		{
			return false;
		}
		switch (InOperator)
		{
			case This::EAssignment::Copy: this->Bool  = InRight; break;
			case This::EAssignment::Or:   this->Bool |= InRight; break;
			case This::EAssignment::Xor:  this->Bool ^= InRight; break;
			case This::EAssignment::And:  this->Bool &= InRight; break;
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
	static typename This::EKind::Type classify_kind() PSYQUE_NOEXCEPT
	{
		if (std::is_same<TemplateValue, bool>::value)
		{
			return This::EKind::Bool;
		}
		else if (std::is_floating_point<TemplateValue>::value)
		{
			return This::EKind::Float;
		}
		else if (std::is_integral<TemplateValue>::value)
		{
			return std::is_unsigned<TemplateValue>::value?
				This::EKind::Unsigned: This::EKind::Signed;
		}
		return This::EKind::Empty;
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
		typename This::EKind::Type const InKind,
		/// [in] 適用する演算子。
		typename This::EAssignment::Type const InOperator,
		/// [in] 演算子の左辺となる整数値。
		TemplateLeft const& InLeft,
		/// [in] 演算子の左辺となる整数値。
		TemplateRight const& InRight)
	PSYQUE_NOEXCEPT
	{
		switch (InOperator)
		{
			case This::EAssignment::Mod:
			if (InRight == 0)
			{
				return false;
			}
			return this->Assign(InLeft % InRight, InKind);

			case This::EAssignment::Or:
			return this->Assign(InLeft | InRight, InKind);

			case This::EAssignment::Xor:
			return this->Assign(InLeft ^ InRight, InKind);

			case This::EAssignment::And:
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
		typename This::EKind::Type const InKind,
		/// [in] 適用する演算子。
		typename This::EAssignment::Type const InOperator,
		/// [in] 演算子の左辺となる実数値。
		TemplateLeft const& InLeft,
		/// [in] 演算子の左辺となる実数値。
		TemplateRight const& InRight)
	PSYQUE_NOEXCEPT
	{
		switch (InOperator)
		{
			case This::EAssignment::Add:
			return this->Assign(InLeft + InRight, InKind);

			case This::EAssignment::Sub:
			return this->Assign(InLeft - InRight, InKind);

			case This::EAssignment::Mul:
			return this->Assign(InLeft * InRight, InKind);

			case This::EAssignment::Div:
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
	private: static typename This::EOrder::Type CompareFloat(
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
			This::EOrder::Less:
			LocalEpsilon < LocalDiff? This::EOrder::Greater: This::EOrder::Equal;
#endif
	}

	// @brief 浮動小数点数と値を比較する。
	/// @return 比較結果。
	private: template<typename TemplateValue>
	static typename This::EOrder::Type CompareFloatLeft(
		/// [in] 左辺の浮動小数点数。
		typename This::FFloat const& InLeft,
		/// [in] 右辺の値。
		TemplateValue const& InRight)
	PSYQUE_NOEXCEPT
	{
		This const LocalRight(InRight, This::EKind::Float);
		return LocalRight.GetKind() != This::EKind::Float?
			This::EOrder::None: This::CompareFloat(InLeft, LocalRight.Float);
	}

	/// @brief 値と浮動小数点数を比較する。
	/// @return 比較結果。
	private: template<typename TemplateValue>
	static typename This::EOrder::Type CompareFloatRight(
		/// [in] 左辺の値。
		TemplateValue const& InLeft,
		/// [in] 右辺の浮動小数点数。
		typename This::FFloat const& InRight)
	PSYQUE_NOEXCEPT
	{
		This const LocalLeft(InLeft, This::EKind::Float);
		return LocalLeft.GetKind() != This::EKind::Float?
			This::EOrder::None: This::CompareFloat(LocalLeft.Float, InRight);
	}

	/// @brief 値を比較する。
	/// @return 比較結果。
	private: template<typename TemplateValue>
	static typename This::EOrder::Type CompareValue(
		/// [in] 左辺の値。
		TemplateValue const& InLeft,
		/// [in] 右辺の値。
		TemplateValue const& InRight)
	PSYQUE_NOEXCEPT
	{
		return InLeft < InRight?
			This::EOrder::Less:
			InRight < InLeft? This::EOrder::Greater: This::EOrder::Equal;
	}

	//-------------------------------------------------------------------------
	private: union
	{
		bool Bool;						   ///< 論理値。
		typename This::FUnsigned Unsigned; ///< 符号なし整数値。
		typename This::FSigned Signed;	   ///< 符号あり整数値。
		typename This::FFloat Float;	   ///< 浮動小数点数値。
	};
	private: typename This::EKind::Type Kind; ///< 状態値の型の種類。

}; // class Psyque::RuleEngine::_private::TStatusValue

// vim: set noexpandtab:
