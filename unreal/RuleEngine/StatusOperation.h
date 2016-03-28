// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TStatusOperation
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

//#include "../string/numeric_parser.h"

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_EQUAL
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_EQUAL "=="
#endif // !define(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_EQUAL)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL "!="
#endif // !define(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS "<"
#endif // !define(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL "<="
#endif // !define(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER ">"
#endif // !define(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL ">="
#endif // !define(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_COPY
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_COPY ":="
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_COPY)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_ADD
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_ADD "+="
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_ADD)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_SUB
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_SUB "-="
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_SUB)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MULT
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MULT "*="
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MULT)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_DIV
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_DIV "/="
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_DIV)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MOD
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MOD "%="
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MOD)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_OR
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_OR "|="
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_OR)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_XOR
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_XOR "^="
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_XOR)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_AND
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_AND "&="
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_AND)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_STATUS
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_STATUS "STATUS:"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_STATUS)

#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_HASH
#define PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_HASH "HASH:"
#endif // !define(PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_HASH)

/// @cond
namespace Psyque
{
	namespace RuleEngine
	{
		namespace _private
		{
			template<typename, typename, typename> class TStatusOperation;
		} // namespace _private
	} // namespace RuleEngine
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
class Psyque::RuleEngine::_private::TStatusOperation
{
	private: using This = TStatusOperation; ///< @copydoc TReservoir::This

	//-------------------------------------------------------------------------
	/// @brief 状態操作引数を構築する。
	public: TStatusOperation(
		/// [in] This::Key の初期値。
		TemplateStatusKey InKey,
		/// [in] This::Operator の初期値。
		TemplateStatusOperator InOperator,
		/// [in] This::Value の初期値。
		TemplateStatusValue InValue)
	PSYQUE_NOEXCEPT:
	Value(MoveTemp(InValue)),
	Key(MoveTemp(InKey)),
	Operator(MoveTemp(InOperator)),
	RightKey(false)
	{}

	/// @brief 状態操作引数を構築する。
	public: TStatusOperation(
		/// [in] This::Key の初期値。
		TemplateStatusKey InKey,
		/// [in] This::Operator の初期値。
		TemplateStatusOperator InOperator,
		/// [in] 右辺値となる状態値の識別値。
		TemplateStatusKey const InRightKey)
	PSYQUE_NOEXCEPT:
	Value(static_cast<typename TemplateStatusValue::FUnsigned>(InRightKey)),
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
	/// @return @copydoc This::Key
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
	/// @return 
	/// 右辺値となる状態値の識別値が格納されている、符号なし整数を指すポインタ。
	/// 右辺値が定数の場合は nullptr を返す。
	public: typename TemplateStatusValue::FUnsigned const* GetRightKey()
	const PSYQUE_NOEXCEPT
	{
		return this->RightKey? this->Value.GetUnsigned(): nullptr;
	}

	//-------------------------------------------------------------------------
	/// @brief 文字列表を解析して TStatusOperation を構築し、コンテナに追加する。
	/// @warning Psyque::RuleEngine 管理者以外は、この関数は使用禁止。
	public: template<
		typename TemplateContainer,
		typename TemplateHasher,
		typename TemplateTable>
	static void _build_container(
		/// [in,out] 構築した TStatusOperation を格納するコンテナ。
		TemplateContainer& io_operations,
		/// [in,out] 文字列からハッシュ値を作る std::hash 。
		TemplateHasher& OutHashFunction,
		/// [in] 解析する Psyque::string::table 。
		TemplateTable const& InTable,
		/// [in] 解析する Psyque::string::table の行番号。
		typename TemplateTable::FNumber const InRowNumber,
		/// [in] 解析する Psyque::string::table の属性の列番号。
		typename TemplateTable::FNumber const InColumnNumber,
		/// [in] 解析する Psyque::string::table の属性の列数。
		typename TemplateTable::FNumber const in_column_count)
	{
		typename TemplateTable::FNumber const local_unit_count(3);
		auto const local_operation_count(in_column_count / local_unit_count);
		if (local_operation_count < 1)
		{
			return;
		}
		io_operations.reserve(io_operations.size() + local_operation_count);
		auto const local_column_end(
			InColumnNumber + in_column_count - local_unit_count);
		for (auto i(InColumnNumber); i <= local_column_end; i += local_unit_count)
		{
			auto const local_operation(
				This::_build(OutHashFunction, InTable, InRowNumber, i));
			if (!local_operation.GetValue().IsEmpty())
			{
				io_operations.push_back(local_operation);
			}
		}
	}

	/// @brief 文字列表を解析し、状態操作引数を構築する。
	/// @warning Psyque::RuleEngine 管理者以外は、この関数は使用禁止。
	public: template<typename TemplateHasher, typename TemplateTable>
	static This _build(
		/// [in,out] 文字列からハッシュ値を作る関数オブジェクト。
		TemplateHasher& OutHashFunction,
		/// [in] 解析する Psyque::string::table 。
		TemplateTable const& InTable,
		/// [in] 解析する Psyque::string::table の行番号。
		typename TemplateTable::FNumber const InRowNumber,
		/// [in] 解析する Psyque::string::table の列番号。
		typename TemplateTable::FNumber const InColumnNumber)
	{
		check(InRowNumber != InTable.GetAttributeRow());
		This local_operation;

		// 演算子の左辺となる状態値の識別値を取得する。
		typename TemplateHasher::argument_type const local_left_key_cell(
			InTable.FindCell(InRowNumber, InColumnNumber));
		local_operation.Key = OutHashFunction(local_left_key_cell);
		if (local_operation.Key
			== OutHashFunction(typename TemplateHasher::argument_type()))
		{
			check(local_left_key_cell.empty());
			return local_operation;
		}

		// 演算子を取得する。
		auto const local_make_operator(
			This::MakeOperator(
				local_operation.Operator,
				typename TemplateHasher::argument_type(
					InTable.FindCell(InRowNumber, InColumnNumber + 1))));
		if (!local_make_operator)
		{
			check(false);
			return local_operation;
		}

		// 演算子の右辺値を取得する。
		local_operation.make_right_value(
			OutHashFunction,
			InTable.FindCell(InRowNumber, InColumnNumber + 2));
		check(!local_operation.Value.IsEmpty());
		return local_operation;
	}

	//-------------------------------------------------------------------------
	private: TStatusOperation() PSYQUE_NOEXCEPT = default;

	/// @brief 文字列を解析し、比較演算子を構築する。
	private: template<typename TemplateString>
	static bool MakeOperator(
		/// [out] 比較演算子の格納先。
		typename TemplateStatusValue::EComparison::Type& out_operator,
		/// [in] 解析する文字列。
		TemplateString const& InString)
	{
		if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_EQUAL)
		{
			out_operator = TemplateStatusValue::EComparison::Equal;
		}
		else if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL)
		{
			out_operator = TemplateStatusValue::EComparison::NotEqual;
		}
		else if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS)
		{
			out_operator = TemplateStatusValue::EComparison::Less;
		}
		else if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL)
		{
			out_operator = TemplateStatusValue::EComparison::LessEqual;
		}
		else if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER)
		{
			out_operator = TemplateStatusValue::EComparison::Greater;
		}
		else if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL)
		{
			out_operator = TemplateStatusValue::EComparison::GreaterEqual;
		}
		else
		{
			// 比較演算子が見つからなかった。
			check(false);
			return false;
		}
		return true;
	}

	/// @brief 文字列を解析し、代入演算子を構築する。
	private: template<typename TemplateString>
	static bool MakeOperator(
		/// [out] 代入演算子の格納先。
		typename TemplateStatusValue::EAssignment::Type& out_operator,
		/// [in] 解析する文字列。
		TemplateString const& InString)
	{
		if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_COPY)
		{
			out_operator = TemplateStatusValue::EAssignment::Copy;
		}
		else if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_ADD)
		{
			out_operator = TemplateStatusValue::EAssignment::Add;
		}
		else if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_SUB)
		{
			out_operator = TemplateStatusValue::EAssignment::Sub;
		}
		else if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MULT)
		{
			out_operator = TemplateStatusValue::EAssignment::Mul;
		}
		else if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_DIV)
		{
			out_operator = TemplateStatusValue::EAssignment::Div;
		}
		else if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MOD)
		{
			out_operator = TemplateStatusValue::EAssignment::Mod;
		}
		else if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_OR)
		{
			out_operator = TemplateStatusValue::EAssignment::Or;
		}
		else if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_XOR)
		{
			out_operator = TemplateStatusValue::EAssignment::Xor;
		}
		else if (InString == PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_AND)
		{
			out_operator = TemplateStatusValue::EAssignment::And;
		}
		else
		{
			// 演算子が見つからなかった。
			check(false);
			return false;
		}
		return true;
	}

	/// @brief 文字列を解析し、演算子の右辺値を構築する。
	private: template<typename TemplateHasher>
	void make_right_value(
		/// [in,out] 文字列のハッシュ関数。
		TemplateHasher& OutHashFunction,
		/// [in] 解析する文字列。
		typename TemplateHasher::argument_type const& InString)
	{
		// 状態値の接頭辞があるなら、状態値の識別値を構築する。
		typename TemplateHasher::argument_type const local_status_header(
			PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_STATUS); 
		if (local_status_header == InString.substr(0, local_status_header.size()))
		{
			this->RightKey = true;
			this->Value = TemplateStatusValue(
				static_cast<typename TemplateStatusValue::FUnsigned>(
					OutHashFunction(InString.substr(local_status_header.size()))));
			return;
		}

		// ハッシュ値の接頭辞があるなら、ハッシュ値を構築する。
		this->RightKey = false;
		typename TemplateHasher::argument_type const local_hash_header(
			PSYQUE_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_HASH); 
		if (local_hash_header == InString.substr(0, local_hash_header.size()))
		{
			static_assert(
				sizeof(typename TemplateHasher::result_type)
				<= sizeof(typename TemplateStatusValue::FUnsigned)
				&& std::is_unsigned<typename TemplateHasher::result_type>::value,
				"");
			this->Value = TemplateStatusValue(
				static_cast<typename TemplateStatusValue::FUnsigned>(
					OutHashFunction(InString.substr(local_status_header.size()))));
		}
		else
		{
			// 定数を構築する。
			this->Value = This::make_status_value(InString);
		}
	}

	/// @brief 文字列を解析し、状態値を構築する。
	/// @return
	/// InString を解析して構築した状態値。解析に失敗した場合は、空値を返す。
	private: template<typename TemplateString>
	static TemplateStatusValue make_status_value(
		/// [in] 解析する文字列。
		TemplateString const& InString,
		/// [in] 構築する状態値の型。
		/// FStatusValue::EKind::Empty の場合は、自動決定する。
		typename TemplateStatusValue::EKind const InKind =
			TemplateStatusValue::EKind::Empty)
	{
		// 論理値として構築する。
		if (InKind == TemplateStatusValue::EKind::Bool
			|| InKind == TemplateStatusValue::EKind::Empty)
		{
			Psyque::string::numeric_parser<bool> const local_bool_parser(InString);
			if (local_bool_parser.IsCompleted())
			{
				return TemplateStatusValue(local_bool_parser.GetValue());
			}
			else if (InKind == TemplateStatusValue::EKind::Bool)
			{
				return TemplateStatusValue();
			}
		}
		check(InKind != TemplateStatusValue::EKind::Bool);

		// 符号なし整数として構築する。
		Psyque::string::numeric_parser<
			typename TemplateStatusValue::FUnsigned>
				const local_unsigned_parser(InString);
		if (local_unsigned_parser.IsCompleted())
		{
			switch (InKind)
			{
				case TemplateStatusValue::EKind::Float:
				return TemplateStatusValue(
					static_cast<typename TemplateStatusValue::FFloat>(
						local_unsigned_parser.GetValue()));

				case TemplateStatusValue::EKind::Signed:
				return TemplateStatusValue(
					static_cast<typename TemplateStatusValue::FSigned>(
						local_unsigned_parser.GetValue()));

				default:
				return TemplateStatusValue(local_unsigned_parser.GetValue());
			}
		}

		// 符号あり整数として構築する。
		Psyque::string::numeric_parser<
			typename TemplateStatusValue::FSigned>
				const local_signed_parser(InString);
		if (local_unsigned_parser.IsCompleted())
		{
			switch (InKind)
			{
				case TemplateStatusValue::EKind::Float:
				return TemplateStatusValue(
					static_cast<typename TemplateStatusValue::FFloat>(
						local_signed_parser.GetValue()));

				case TemplateStatusValue::EKind::Unsigned:
				return TemplateStatusValue();

				default:
				return TemplateStatusValue(local_signed_parser.GetValue());
			}
		}

		// 浮動小数点数として構築する。
		Psyque::string::numeric_parser<typename TemplateStatusValue::FFloat>
			const local_float_parser(InString);
		if (local_float_parser.IsCompleted())
		{
			switch (InKind)
			{
				case TemplateStatusValue::EKind::Empty:
				case TemplateStatusValue::EKind::Float:
				return TemplateStatusValue(local_float_parser.GetValue());

				default: break;
			}
		}
		return TemplateStatusValue();
	}

	//-------------------------------------------------------------------------
	/// @brief 演算の右辺値となる値。
	private: TemplateStatusValue Value;
	/// @brief 演算の左辺値となる状態値の識別値。
	private: TemplateStatusKey Key;
	/// @brief 演算子の種類。
	private: TemplateStatusOperator Operator;
	/// @brief 右辺値を状態値から取得するか。
	private: bool RightKey;

}; // class Psyque::RuleEngine::_private::TStatusOperation

// vim: set noexpandtab:
