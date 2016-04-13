// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::TStatusOperation
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		namespace _private
		{
			template<typename, typename, typename> class TStatusOperation;
		} // namespace _private
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態値を操作するための引数の集合。
/// @tparam TemplateStatusKey	   状態値を操作する演算子の左辺値となる状態値の識別値の型。
/// @tparam TemplateStatusOperator 状態値を操作する演算子の型。
/// @tparam TemplateStatusValue    状態値を操作する演算子の右辺値となる状態値の型。
template<
	typename TemplateStatusKey,
	typename TemplateStatusOperator,
	typename TemplateStatusValue>
class Psyque::RulesEngine::_private::TStatusOperation
{
	private: using ThisClass = TStatusOperation; ///< @copydoc TReservoir::ThisClass

	//-------------------------------------------------------------------------
	/// @brief 状態操作引数を構築する。
	public: TStatusOperation(
		/// [in] ThisClass::Key の初期値。
		TemplateStatusKey InKey,
		/// [in] ThisClass::Operator の初期値。
		TemplateStatusOperator InOperator,
		/// [in] ThisClass::Value の初期値。
		TemplateStatusValue InValue)
	PSYQUE_NOEXCEPT:
	Value(MoveTemp(InValue)),
	Key(MoveTemp(InKey)),
	Operator(MoveTemp(InOperator)),
	RightKey(false)
	{}

	/// @brief 状態操作引数を構築する。
	public: TStatusOperation(
		/// [in] ThisClass::Key の初期値。
		TemplateStatusKey InKey,
		/// [in] ThisClass::Operator の初期値。
		TemplateStatusOperator InOperator,
		/// [in] 右辺値となる状態値の識別値。
		TemplateStatusKey const InRightKey)
	PSYQUE_NOEXCEPT:
	Value(InRightKey),
	Key(MoveTemp(InKey)),
	Operator(MoveTemp(InOperator)),
	RightKey(true)
	{}
	static_assert(
		sizeof(TemplateStatusKey)
		<= sizeof(typename TemplateStatusValue::FUnsigned)
		&& std::is_unsigned<TemplateStatusKey>::value,
		"");

	/// @brief 左辺値となる状態値に対応する識別値を取得する。
	/// @return @copydoc ThisClass::Key
	public: TemplateStatusKey const GetKey() const PSYQUE_NOEXCEPT
	{
		return this->Key;
	}

	public: TemplateStatusOperator const GetOperator() const PSYQUE_NOEXCEPT
	{
		return this->Operator;
	}

	public: TemplateStatusValue const& GetValue() const PSYQUE_NOEXCEPT
	{
		return this->Value;
	}

	/// @brief 右辺値となる状態値の識別値を取得する。
	/// @return 右辺値となる状態値の識別値が格納されている、
	///   符号なし整数を指すポインタ。右辺値が定数の場合は nullptr を返す。
	public: typename TemplateStatusValue::FUnsigned const* GetRightKey()
	const PSYQUE_NOEXCEPT
	{
		return this->RightKey? this->Value.GetUnsigned(): nullptr;
	}

	//-------------------------------------------------------------------------
	private: TStatusOperation() PSYQUE_NOEXCEPT = default;

	//-------------------------------------------------------------------------
	/// @brief 演算の右辺値となる値。
	private: TemplateStatusValue Value;
	/// @brief 演算の左辺値となる状態値の識別値。
	private: TemplateStatusKey Key;
	/// @brief 演算子の種類。
	private: TemplateStatusOperator Operator;
	/// @brief 右辺値を状態値から取得するか。
	private: bool RightKey;

}; // class Psyque::RulesEngine::_private::TStatusOperation

// vim: set noexpandtab:
