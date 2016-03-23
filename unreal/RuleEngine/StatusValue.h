// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TStatusValue
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include "../BitAlgorithm.h"

#ifndef PSYQUE_RULE_ENGINE_STATUS_VALUE_EPSILON_MAG
#define PSYQUE_RULE_ENGINE_STATUS_VALUE_EPSILON_MAG 4
#endif // !default(PSYQUE_RULE_ENGINE_STATUS_VALUE_EPSILON_MAG)

/// @brief Unreal Engine 4 で動作する、ビデオゲーム開発のためのライブラリ。
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
/// @tparam TemplateUnsigned @copydoc TStatusValue::UnsignedType
/// @tparam TemplateFloat    @copydoc TStatusValue::FloatType
template<typename TemplateUnsigned, typename TemplateFloat>
class Psyque::RuleEngine::_private::TStatusValue
{
	/// @brief thisが指す値の型。
	typedef TStatusValue This;

	//-------------------------------------------------------------------------
	public:
	/// @brief 状態値で扱う符号なし整数の型。
	/// @details この型の大きさを超える型は、状態値で扱えない。
	typedef TemplateUnsigned UnsignedType;
	static_assert(
		std::is_integral<TemplateUnsigned>::value
		&& std::is_unsigned<TemplateUnsigned>::value,
		"'TemplateUnsigned' is not unsigned integer.");
	/// @brief 状態値で扱う符号あり整数の型。
	typedef
		typename std::make_signed<typename This::UnsignedType>::type
		SignedType;
	/// @brief 状態値で扱う浮動小数点数の型。
	typedef TemplateFloat FloatType;
	static_assert(
		std::is_floating_point<TemplateFloat>::value,
		"'TemplateFloat' is not floating-point number.");
	static_assert(
		 sizeof(TemplateFloat) <= sizeof(TemplateUnsigned),
		 "sizeof(TemplateFloat) is greater than sizeof(TemplateUnsigned).");
	/// @brief 状態値の型の種別。
	enum class EKind: int8
	{
		SIGNED = -2, ///< 符号あり整数。
		FLOAT,       ///< 浮動小数点数。
		EMPTY,       ///< 空。
		BOOL,        ///< 論理値。
		UNSIGNED,    ///< 符号なし整数。
	};

	//-------------------------------------------------------------------------
	public:
	/// @brief 比較式の評価。
	/// @details
	///   - 正なら、比較式の評価は真。
	///   - 0 なら、比較式の評価は偽。
	///   - 負なら、比較式の評価に失敗。
	typedef int8 Evaluation;
	/// @brief 値の大小関係。
	enum class EOrder: int8
	{
		NONE = -2, ///< 比較に失敗。
		LESS,      ///< 左辺のほうが小さい。
		EQUAL,     ///< 左辺と右辺は等価。
		GREATER,   ///< 左辺のほうが大きい。
	};
	/// @brief 状態値を比較する演算子の種類。
	enum class EComparison: uint8
	{
		EQUAL,         ///< 等価。
		NOT_EQUAL,     ///< 非等価。
		LESS,          ///< 小なり。
		LESS_EQUAL,    ///< 以下。
		GREATER,       ///< 大なり。
		GREATER_EQUAL, ///< 以上。
	};
	/// @brief 状態値を代入する演算子の種類。
	enum class EAssignment: uint8
	{
		COPY, ///< 単純代入。
		ADD,  ///< 加算代入。
		SUB,  ///< 減算代入。
		MULT, ///< 乗算代入。
		DIV,  ///< 除算代入。
		MOD,  ///< 除算の余りの代入。
		OR,   ///< 論理和の代入。
		XOR,  ///< 排他的論理和の代入。
		AND,  ///< 論理積の代入。
	};

	//-------------------------------------------------------------------------
	/// @name 構築
	/// @{
	public:
	/// @brief 空値を構築する。
	PSYQUE_CONSTEXPR TStatusValue()
	PSYQUE_NOEXCEPT: Kind(This::EKind::EMPTY) {}

	/// @brief 論理型の値を構築する。
	explicit TStatusValue(
		/// [in] 初期値となる論理値。
		bool const InBool)
	PSYQUE_NOEXCEPT: Kind(This::EKind::BOOL)
	{
		this->Bool = InBool;
	}

	/// @brief 符号なし整数型の値を構築する。
	explicit TStatusValue(
		/// [in] 初期値となる符号なし整数。
		typename This::UnsignedType const InUnsigned)
	PSYQUE_NOEXCEPT: Kind(This::EKind::UNSIGNED)
	{
		this->Unsigned = InUnsigned;
	}

	/// @brief 符号あり整数型の値を構築する。
	explicit TStatusValue(
		/// [in] 初期値となる符号あり整数。
		typename This::SignedType const InSigned)
	PSYQUE_NOEXCEPT: Kind(This::EKind::SIGNED)
	{
		this->Signed = InSigned;
	}

	/// @brief 浮動小数点数型の値を構築する。
	explicit TStatusValue(
		/// [in] 初期値となる浮動小数点数。
		typename This::FloatType const InFloat)
	PSYQUE_NOEXCEPT: Kind(This::EKind::FLOAT)
	{
		this->Float = InFloat;
	}

	/// @brief 任意型の値を構築する。
	template<typename TemplateValue>
	explicit TStatusValue(
		/// [in] 初期値。
		TemplateValue const InValue,
		/// [in] 値の型。 This::EKind::EMPTY の場合は、自動で決定する。
		typename This::EKind const InKind = This::EKind::EMPTY)
	PSYQUE_NOEXCEPT: Kind(This::EKind::EMPTY)
	{
		this->Assign(InValue, InKind);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 値の取得
	/// @{
	public:
	/// @brief 値が空か判定する。
	PSYQUE_CONSTEXPR bool IsEmpty() const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == This::EKind::EMPTY;
	}

	/// @brief 論理値を取得する。
	/// @return 論理値を指すポインタ。論理値が格納されてない場合は nullptr を返す。
	PSYQUE_CONSTEXPR bool const* GetBool() const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == This::EKind::BOOL? &this->Bool: nullptr;
	}

	/// @brief 符号なし整数値を取得する。
	/// @return
	/// 符号なし整数値を指すポインタ。
	/// 符号なし整数値が格納されてない場合は nullptr を返す。
	PSYQUE_CONSTEXPR typename This::UnsignedType const* GetUnsigned()
	const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == This::EKind::UNSIGNED?
			&this->Unsigned: nullptr;
	}

	/// @brief 符号あり整数値を取得する。
	/// @return
	/// 符号あり整数値を指すポインタ。
	/// 符号あり整数値が格納されてない場合は nullptr を返す。
	PSYQUE_CONSTEXPR typename This::SignedType const* GetSigned()
	const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == This::EKind::SIGNED?
			&this->Signed: nullptr;
	}

	/// @brief 浮動小数点数値を取得する。
	/// @return
	/// 浮動小数点数値を指すポインタ。
	/// 浮動小数点数値が格納されてない場合は nullptr を返す。
	PSYQUE_CONSTEXPR typename This::FloatType const* GetFloat()
	const PSYQUE_NOEXCEPT
	{
		return this->GetKind() == This::EKind::FLOAT?
			&this->Float: nullptr;
	}

	typename This::UnsignedType GetBitField() const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case This::EKind::EMPTY:
			return 0;

			case This::EKind::BOOL:
			return this->Bool;

			case This::EKind::FLOAT:
			typedef
				Psyque::FloatBitField<typename This::FloatType>
				FloatBitField;
			return FloatBitField(this->Float).BitField;

			default:
			return this->Unsigned;
		}
	}

	/// @brief 格納値の型の種類を取得する。
	/// @return 格納値の型の種類。
	PSYQUE_CONSTEXPR typename This::EKind GetKind() const PSYQUE_NOEXCEPT
	{
		return this->Kind;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 値の比較
	/// @{
	public:
	/// @brief 比較式を評価する。
	/// @retval 正 比較式の評価は真。
	/// @retval 0  比較式の評価は偽。
	/// @retval 負 比較式の評価に失敗。
	template<typename TemplateRight>
	typename This::Evaluation Compare(
		/// [in] 比較演算子の種類。
		typename This::EComparison const InComparison,
		/// [in] 比較演算子の右辺値。
		TemplateRight const& InRight)
	{
		auto const LocalOrder(this->Compare(InRight));
		if (LocalOrder != This::EOrder::NONE)
		{
			switch (InComparison)
			{
				case This::EComparison::EQUAL:
				return LocalOrder == This::EOrder::EQUAL;

				case This::EComparison::NOT_EQUAL:
				return LocalOrder != This::EOrder::EQUAL;

				case This::EComparison::LESS:
				return LocalOrder == This::EOrder::LESS;

				case This::EComparison::LESS_EQUAL:
				return LocalOrder != This::EOrder::GREATER;

				case This::EComparison::GREATER:
				return LocalOrder == This::EOrder::GREATER;

				case This::EComparison::GREATER_EQUAL:
				return LocalOrder != This::EOrder::LESS;

				default: check(false); break;
			}
		}
		return -1;
	}

	/// @brief 値を比較する。
	/// @return *this を左辺値とした比較結果。
	typename This::EOrder Compare(
		/// [in] 右辺値。
		This const& InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (InRight.GetKind())
		{
			case This::EKind::BOOL:     return this->Compare(InRight.Bool);
			case This::EKind::UNSIGNED: return this->Compare(InRight.Unsigned);
			case This::EKind::SIGNED:   return this->Compare(InRight.Signed);
			case This::EKind::FLOAT:    return this->Compare(InRight.Float);
			default:                    return This::EOrder::NONE;
		}
	}

	/// @brief 論理値と比較する。
	/// @return *this を左辺値とした比較結果。
	typename This::EOrder Compare(
		/// [in] 右辺値となる論理値。
		bool const InRight)
	const PSYQUE_NOEXCEPT
	{
		if (this->GetBool() == nullptr)
		{
			return This::EOrder::NONE;
		}
		else if (this->Bool == InRight)
		{
			return This::EOrder::EQUAL;
		}
		else if (this->Bool)
		{
			return This::EOrder::GREATER;
		}
		else
		{
			return This::EOrder::LESS;
		}
	}

	/// @brief 符号なし整数と比較する。
	/// @return *this を左辺値とした比較結果。
	typename This::EOrder Compare(
		/// [in] 右辺値となる符号なし整数。
		typename This::UnsignedType const InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case This::EKind::UNSIGNED:
			return This::CompareValue(this->Unsigned, InRight);

			case This::EKind::SIGNED:
			return this->Signed < 0?
				This::EOrder::LESS:
				This::CompareValue(this->Unsigned, InRight);

			case This::EKind::FLOAT:
			return This::CompareFloatLeft(this->Float, InRight);

			default: return This::EOrder::NONE;
		}
	}

	/// @brief 符号あり整数と比較する。
	/// @return *this を左辺値とした比較結果。
	typename This::EOrder Compare(
		/// [in] 右辺値となる符号あり整数。
		typename This::SignedType const InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case This::EKind::UNSIGNED:
			return InRight < 0?
				This::EOrder::GREATER:
				This::CompareValue(
					this->Unsigned,
					static_cast<typename This::UnsignedType>(InRight));

			case This::EKind::SIGNED:
			return This::CompareValue(this->Signed, InRight);

			case This::EKind::FLOAT:
			return This::CompareFloatLeft(this->Float, InRight);

			default: return This::EOrder::NONE;
		}
	}

	/// @brief 浮動小数点数と比較する。
	/// @return *this を左辺値とした比較結果。
	typename This::EOrder Compare(
		/// [in] 右辺値となる浮動小数点数。
		typename This::FloatType const InRight)
	const PSYQUE_NOEXCEPT
	{
		switch (this->GetKind())
		{
			case This::EKind::UNSIGNED:
			return InRight < 0?
				This::EOrder::GREATER:
				This::CompareFloatRight(this->Unsigned, InRight);

			case This::EKind::SIGNED:
			return This::CompareFloatRight(this->Signed, InRight);

			case This::EKind::FLOAT:
			return This::CompareFloat(this->Float, InRight);

			default: return This::EOrder::NONE;
		}
	}

	/// @brief 値を比較する。
	/// @return *this を左辺値とした比較結果。
	template<typename TemplateRight>
	typename This::EOrder Compare(
		/// [in] 右辺値。
		TemplateRight const& InRight)
	const PSYQUE_NOEXCEPT
	{
		if (std::is_floating_point<TemplateRight>::value)
		{
			auto const LocalRight(
				static_cast<typename This::FloatType>(InRight));
			if (LocalRight == InRight)
			{
				return this->Compare(LocalRight);
			}
		}
		else if (std::is_signed<TemplateRight>::value)
		{
			auto const LocalRight(
				static_cast<typename This::SignedType>(InRight));
			if (LocalRight == InRight)
			{
				return this->Compare(LocalRight);
			}
		}
		else if (std::is_unsigned<TemplateRight>::value)
		{
			auto const LocalRight(
				static_cast<typename This::UnsignedType>(InRight));
			if (LocalRight == InRight)
			{
				return this->Compare(LocalRight);
			}
		}
		return This::EOrder::NONE;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 値の代入
	/// @{
	public:
	/// @brief 状態値を空にする。
	void AssignEmpty() PSYQUE_NOEXCEPT
	{
		this->Kind = This::EKind::EMPTY;
	}

	/// @brief 論理値を代入する。
	void AssignBool(
		/// [in] 代入する論理値。
		bool const InBool)
	PSYQUE_NOEXCEPT
	{
		this->Bool = InBool;
		this->Kind = This::EKind::BOOL;
	}

	/// @brief 符号なし整数を代入する。
	/// @retval true 成功。 InValue を *this に設定した。
	/// @retval false
	///   失敗。InValue を符号なし整数に変換できなかった。 *this は変化しない。
	template<typename TemplateValue>
	bool AssignUnsigned(
		/// [in] 代入する値。
		TemplateValue const& InValue)
	PSYQUE_NOEXCEPT
	{
		if (0 <= InValue)
		{
			auto const LocalUnsigned(
				static_cast<typename This::UnsignedType>(InValue));
			if (static_cast<TemplateValue>(LocalUnsigned) == InValue)
			{
				this->Unsigned = LocalUnsigned;
				this->Kind = This::EKind::UNSIGNED;
				return true;
			}
		}
		return false;
	}

	/// @brief 符号あり整数を代入する。
	/// @retval true 成功。 InValue を *this に代入した。
	/// @retval false
	///   失敗。 InValue を符号あり整数に変換できなかった。 *this は変化しない。
	template<typename TemplateValue>
	bool AssignSigned(
		/// [in] 代入する値。
		TemplateValue const& InValue)
	PSYQUE_NOEXCEPT
	{
		auto const LocalSigned(
			static_cast<typename This::SignedType>(InValue));
		if (InValue <= 0 || 0 <= LocalSigned)
		{
			if (static_cast<TemplateValue>(LocalSigned) == InValue)
			{
				this->Signed = LocalSigned;
				this->Kind = This::EKind::SIGNED;
				return true;
			}
		}
		return false;
	}

	/// @brief 浮動小数点数を代入する。
	/// @retval true 成功。 InValue を *this に代入した。
	/// @retval false
	///   失敗。 InValue を浮動小数点数に変換できなかった。 *this は変化しない。
	template<typename TemplateValue>
	bool AssignFloat(
		/// [in] 代入する値。
		TemplateValue const& InValue)
	PSYQUE_NOEXCEPT
	{
		auto const LocalFloat(
			static_cast<typename This::FloatType>(InValue));
		auto const LocalDiff(
			static_cast<TemplateValue>(LocalFloat) - InValue);
		auto const LocalEpsilon(
			std::numeric_limits<typename This::FloatType>::epsilon()
			* PSYQUE_RULE_ENGINE_STATUS_VALUE_EPSILON_MAG);
		if (-LocalEpsilon <= LocalDiff && LocalDiff <= LocalEpsilon)
		{
			this->Float = LocalFloat;
			this->Kind = This::EKind::FLOAT;
			return true;
		}
		return false;
	}

	/// @brief 値を代入する。
	/// @retval true 成功。 InValue を *this に代入した。
	/// @retval false
	/// 失敗。 InValue を状態値に変換できなかった。 *this は変化しない。
	template<typename TemplateValue>
	bool Assign(
		/// [in] 代入する値。
		TemplateValue const& InValue,
		/// [in] 代入する型。 This::EKind::EMPTY の場合は、自動で決定する。
		typename This::EKind InKind = This::EKind::EMPTY)
	{
		if (InKind == This::EKind::EMPTY)
		{
			InKind = This::template ClassifyKind<TemplateValue>();
		}
		switch (InKind)
		{
			case This::EKind::UNSIGNED: return this->AssignUnsigned(InValue);
			case This::EKind::SIGNED:   return this->AssignSigned(InValue);
			case This::EKind::FLOAT:    return this->AssignFloat(InValue);
			default:                    return false;
		}
	}

	/// @copydoc Assign
	bool Assign(
		This const& InValue,
		typename This::EKind InKind = This::EKind::EMPTY)
	PSYQUE_NOEXCEPT
	{
		if (InKind == This::EKind::EMPTY)
		{
			InKind = InValue.GetKind();
		}
		switch (InValue.GetKind())
		{
			case This::EKind::BOOL:
			return this->Assign(InValue.Bool, InKind);

			case This::EKind::UNSIGNED:
			return this->Assign(InValue.Unsigned, InKind);

			case This::EKind::SIGNED:
			return this->Assign(InValue.Signed, InKind);

			case This::EKind::FLOAT:
			return this->Assign(InValue.Float, InKind);

			default:
			this->AssignEmpty();
			return true;
		}
	}

	/// @copydoc Assign
	bool Assign(
		bool const InBool,
		typename This::EKind const InKind = This::EKind::BOOL)
	{
		switch (InKind)
		{
			case This::EKind::EMPTY:
			case This::EKind::BOOL:
			this->AssignBool(InBool);
			return true;

			default: return false;
		}
	}

	/// @brief 代入演算する。
	/// @retval true  成功。演算結果を *this に代入した。
	/// @retval false 失敗。 *this は変化しない。
	template<typename TemplateValue>
	bool Assign(
		/// [in] 適用する代入演算子。
		typename This::EAssignment const InOperator,
		/// [in] 代入演算子の右辺。
		TemplateValue const& InRight)
	PSYQUE_NOEXCEPT
	{
		static_assert(!std::is_same<TemplateValue, bool>::value, "");
		auto const LocalKind(this->GetKind());
		if (InOperator == This::EAssignment::COPY)
		{
			return this->Assign(InRight, LocalKind);
		}
		switch (LocalKind)
		{
			case This::EKind::UNSIGNED:
			return this->AssignValue(
				std::is_integral<TemplateValue>(),
				LocalKind,
				InOperator,
				this->Unsigned,
				InRight);

			case This::EKind::SIGNED:
			return this->AssignValue(
				std::is_integral<TemplateValue>(),
				LocalKind,
				InOperator,
				this->Signed,
				InRight);

			case This::EKind::FLOAT:
			return this->AssignValue(
				std::false_type(),
				LocalKind,
				InOperator,
				this->Float,
				InRight);

			default: return false;
		}
	}

	/// @copydoc Assign(This::EAssignment const, TemplateValue const&)
	bool Assign(
		typename This::EAssignment const InOperator,
		This const& InRight)
	PSYQUE_NOEXCEPT
	{
		switch (InRight.GetKind())
		{
			case This::EKind::BOOL:
			return this->Assign(InOperator, InRight.Bool);

			case This::EKind::UNSIGNED:
			return this->Assign(InOperator, InRight.Unsigned);

			case This::EKind::SIGNED:
			return this->Assign(InOperator, InRight.Signed);

			case This::EKind::FLOAT:
			return this->Assign(InOperator, InRight.Float);

			default: return false;
		}
	}

	/// @copydoc Assign(This::EAssignment const, TemplateValue const&)
	bool Assign(
		typename This::EAssignment const InOperator,
		bool const InRight)
	PSYQUE_NOEXCEPT
	{
		if (this->GetKind() != This::EKind::BOOL)
		{
			return false;
		}
		switch (InOperator)
		{
			case This::EAssignment::COPY: this->Bool  = InRight; break;
			case This::EAssignment::OR:   this->Bool |= InRight; break;
			case This::EAssignment::XOR:  this->Bool ^= InRight; break;
			case This::EAssignment::AND:  this->Bool &= InRight; break;
			default: return false;
		}
		return true;
	}
	/// @}
	//-------------------------------------------------------------------------
	private:
	/// @brief 型の種類を決定する。
	/// @tparam TemplateValue 型。
	/// @return 型の種類。
	template<typename TemplateValue>
	static typename This::EKind ClassifyKind() PSYQUE_NOEXCEPT
	{
		if (std::is_same<TemplateValue, bool>::value)
		{
			return This::EKind::BOOL;
		}
		else if (std::is_floating_point<TemplateValue>::value)
		{
			return This::EKind::FLOAT;
		}
		else if (std::is_integral<TemplateValue>::value)
		{
			return std::is_unsigned<TemplateValue>::value?
				This::EKind::UNSIGNED: This::EKind::SIGNED;
		}
		return This::EKind::EMPTY;
	}

	//-------------------------------------------------------------------------
	private:
	/// @brief This::AssignUnsigned で論理値を設定させないためのダミー関数。
	bool AssignUnsigned(bool const) PSYQUE_NOEXCEPT;
	/// @brief This::AssignSigned で論理値を設定させないためのダミー関数。
	bool AssignSigned(bool const) PSYQUE_NOEXCEPT;
	/// @brief This::AssignFloat で論理値を設定させないためのダミー関数。
	bool AssignFloat(bool const) PSYQUE_NOEXCEPT;

	//-------------------------------------------------------------------------
	private:
	/// @brief 整数の演算を行い、結果を状態値へ格納する。
	/// @retval true  成功。 演算結果を *this に格納した。
	/// @retval false 失敗。 *this は変化しない。
	template<typename TemplateLeft, typename TemplateRight>
	bool AssignValue(
		std::true_type,
		/// [in] 演算した結果の型。
		typename This::EKind const InKind,
		/// [in] 適用する演算子。
		typename This::EAssignment const InOperator,
		/// [in] 演算子の左辺となる整数値。
		TemplateLeft const& InLeft,
		/// [in] 演算子の左辺となる整数値。
		TemplateRight const& InRight)
	PSYQUE_NOEXCEPT
	{
		switch (InOperator)
		{
			case This::EAssignment::MOD:
			if (InRight == 0)
			{
				return false;
			}
			return this->Assign(InLeft % InRight, InKind);

			case This::EAssignment::OR:
			return this->Assign(InLeft | InRight, InKind);

			case This::EAssignment::XOR:
			return this->Assign(InLeft ^ InRight, InKind);

			case This::EAssignment::AND:
			return this->Assign(InLeft & InRight, InKind);

			default:
			return this->AssignValue(
				std::false_type(), InKind, InOperator, InLeft, InRight);
		}
	}

	/// @brief 実数の演算を行い、結果を状態値へ格納する。
	/// @retval true  成功。 演算結果を *this に格納した。
	/// @retval false 失敗。 *this は変化しない。
	template<typename TemplateLeft, typename TemplateRight>
	bool AssignValue(
		std::false_type,
		/// [in] 演算した結果の型。
		typename This::EKind const InKind,
		/// [in] 適用する演算子。
		typename This::EAssignment const InOperator,
		/// [in] 演算子の左辺となる実数値。
		TemplateLeft const& InLeft,
		/// [in] 演算子の左辺となる実数値。
		TemplateRight const& InRight)
	PSYQUE_NOEXCEPT
	{
		switch (InOperator)
		{
			case This::EAssignment::ADD:
			return this->Assign(InLeft + InRight, InKind);

			case This::EAssignment::SUB:
			return this->Assign(InLeft - InRight, InKind);

			case This::EAssignment::MULT:
			return this->Assign(InLeft * InRight, InKind);

			case This::EAssignment::DIV:
			if (InRight == 0)
			{
				return false;
			}
			return this->Assign(InLeft / InRight, InKind);

			default: return false;
		}
	}

	//-------------------------------------------------------------------------
	private:
	/// @brief 浮動小数点数を比較する。
	/// @return 比較結果。
	static typename This::EOrder CompareFloat(
		/// [in] 左辺の浮動小数点数。
		typename This::FloatType const& InLeft,
		/// [in] 右辺の浮動小数点数。
		typename This::FloatType const& InRight)
	{
#if 0
		/// @note 浮動小数点数の誤差を考慮せずに比較する。
		return This::CompareValue(InLeft, InRight);
#else
		/// @note 浮動小数点数の誤差を考慮して比較する。
		auto const LocalDiff(InLeft - InRight);
		auto const LocalEpsilon(
			std::numeric_limits<typename This::FloatType>::epsilon()
			* PSYQUE_RULE_ENGINE_STATUS_VALUE_EPSILON_MAG);
		return LocalDiff < -LocalEpsilon?
			This::EOrder::LESS:
			(LocalEpsilon < LocalDiff?
				This::EOrder::GREATER: This::EOrder::EQUAL);
#endif
	}

	// @brief 浮動小数点数と値を比較する。
	/// @return 比較結果。
	template<typename TemplateValue>
	static typename This::EOrder CompareFloatLeft(
		/// [in] 左辺の浮動小数点数。
		typename This::FloatType const& InLeft,
		/// [in] 右辺の値。
		TemplateValue const& InRight)
	{
		This const LocalRight(InRight, This::EKind::FLOAT);
		return LocalRight.GetKind() != This::EKind::FLOAT?
			This::EOrder::NONE:
			This::CompareFloat(InLeft, LocalRight.Float);
	}

	/// @brief 値と浮動小数点数を比較する。
	/// @return 比較結果。
	template<typename TemplateValue>
	static typename This::EOrder CompareFloatRight(
		/// [in] 左辺の値。
		TemplateValue const& InLeft,
		/// [in] 右辺の浮動小数点数。
		typename This::FloatType const& InRight)
	{
		This const local_left(InLeft, This::EKind::FLOAT);
		return local_left.GetKind() != This::EKind::FLOAT?
			This::EOrder::NONE:
			This::CompareFloat(local_left.Float, InRight);
	}

	/// @brief 値を比較する。
	/// @return 比較結果。
	private: template<typename TemplateValue>
	static typename This::EOrder CompareValue(
		/// [in] 左辺の値。
		TemplateValue const& InLeft,
		/// [in] 右辺の値。
		TemplateValue const& InRight)
	{
		return InLeft < InRight?
			This::EOrder::LESS:
			(InRight < InLeft?
				This::EOrder::GREATER: This::EOrder::EQUAL);
	}

private:
	//-------------------------------------------------------------------------
	union
	{
		bool Bool;                            ///< 論理値。
		typename This::UnsignedType Unsigned; ///< 符号なし整数値。
		typename This::SignedType Signed;     ///< 符号あり整数値。
		typename This::FloatType Float;       ///< 浮動小数点数値。
	};
	typename This::EKind Kind; ///< 状態値の型の種類。

}; // class Psyque::RuleEngine::_private::TStatusValue

// vim: set noexpandtab:
