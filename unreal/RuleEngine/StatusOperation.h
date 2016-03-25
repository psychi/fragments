// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TStatusOperation
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

//#include "../string/numeric_parser.hpp"

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_EQUAL
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_EQUAL "=="
#endif // !define(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_EQUAL)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL "!="
#endif // !define(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_LESS
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_LESS "<"
#endif // !define(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_LESS)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL "<="
#endif // !define(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_GREATER
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_GREATER ">"
#endif // !define(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_GREATER)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL ">="
#endif // !define(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_COPY
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_COPY ":="
#endif // !defined(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_COPY)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_ADD
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_ADD "+="
#endif // !defined(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_ADD)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_SUB
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_SUB "-="
#endif // !defined(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_SUB)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_MULT
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_MULT "*="
#endif // !defined(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_MULT)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_DIV
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_DIV "/="
#endif // !defined(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_DIV)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_MOD
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_MOD "%="
#endif // !defined(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_MOD)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_OR
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_OR "|="
#endif // !defined(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_OR)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_XOR
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_XOR "^="
#endif // !defined(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_XOR)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_AND
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_AND "&="
#endif // !defined(PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_AND)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_RIGHT_STATUS
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_RIGHT_STATUS "STATUS:"
#endif // !defined(PSYQUE_RULE_ENGINE_STATUS_OPERATION_RIGHT_STATUS)

#ifndef PSYQUE_RULE_ENGINE_STATUS_OPERATION_RIGHT_HASH
#define PSYQUE_RULE_ENGINE_STATUS_OPERATION_RIGHT_HASH "HASH:"
#endif // !define(PSYQUE_RULE_ENGINE_STATUS_OPERATION_RIGHT_HASH)

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
/// @tparam TemplateStatusKey      状態値を操作する演算子の左辺値となる状態値の識別値の型。
/// @tparam TemplateStatusOperator 状態値を操作する演算子の型。
/// @tparam TemplateStatusValue    状態値を操作する演算子の右辺値となる状態値の型。
template<
	typename TemplateStatusKey,
	typename TemplateStatusOperator,
	typename TemplateStatusValue>
class Psyque::RuleEngine::_private::TStatusOperation
{
	typedef TStatusOperation This; ///< @copydoc TReservoir::This

	//-------------------------------------------------------------------------
	public:
	/// @brief 状態操作引数を構築する。
	PSYQUE_CONSTEXPR TStatusOperation(
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
	bIsValidRightKey(false)
	{}

	/// @brief 状態操作引数を構築する。
	PSYQUE_CONSTEXPR TStatusOperation(
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
	bIsValidRightKey(true)
	{}
	static_assert(
		sizeof(TemplateStatusKey)
		<= sizeof(typename TemplateStatusValue::FUnsigned)
		&& std::is_unsigned<TemplateStatusKey>::value,
		"");

	/// @brief 左辺値となる状態値に対応する識別値を取得する。
	/// @return @copydoc This::Key
	PSYQUE_CONSTEXPR TemplateStatusKey const& GetKey() const PSYQUE_NOEXCEPT
	{
		return this->Key;
	}

	PSYQUE_CONSTEXPR TemplateStatusOperator const& GetOperator()
	const PSYQUE_NOEXCEPT
	{
		return this->Operator;
	}

	PSYQUE_CONSTEXPR TemplateStatusValue const& GetValue()
	const PSYQUE_NOEXCEPT
	{
		return this->Value;
	}

	/// @brief 右辺値となる状態値の識別値を取得する。
	/// @return 右辺値となる状態値の識別値が格納されている、
	/// 符号なし整数を指すポインタ。右辺値が定数の場合は nullptr を返す。
	typename TemplateStatusValue::FUnsigned const* GetRightKey()
	const PSYQUE_NOEXCEPT
	{
		return this->bIsValidRightKey? this->Value.GetUnsigned(): nullptr;
	}

	//-------------------------------------------------------------------------
	/// @brief 文字列表を解析して TStatusOperation を構築し、コンテナに追加する。
	/// @warning Psyque::RuleEngine 管理者以外は、この関数は使用禁止。
	public: template<
		typename template_container,
		typename template_hasher,
		typename template_table>
	static void _build_container(
		/// [in,out] 構築した TStatusOperation を格納するコンテナ。
		template_container& io_operations,
		/// [in,out] 文字列からハッシュ値を作る std::hash 。
		template_hasher& io_hasher,
		/// [in] 解析する Psyque::string::table 。
		template_table const& in_table,
		/// [in] 解析する Psyque::string::table の行番号。
		typename template_table::number const in_row_number,
		/// [in] 解析する Psyque::string::table の属性の列番号。
		typename template_table::number const in_column_number,
		/// [in] 解析する Psyque::string::table の属性の列数。
		typename template_table::number const in_column_count)
	{
		typename template_table::number const local_unit_count(3);
		auto const local_operation_count(in_column_count / local_unit_count);
		if (local_operation_count < 1)
		{
			return;
		}
		io_operations.reserve(io_operations.size() + local_operation_count);
		auto const local_column_end(
			in_column_number + in_column_count - local_unit_count);
		for (auto i(in_column_number); i <= local_column_end; i += local_unit_count)
		{
			auto const local_operation(
				This::_build(io_hasher, in_table, in_row_number, i));
			if (!local_operation.GetValue().is_empty())
			{
				io_operations.push_back(local_operation);
			}
		}
	}

	/// @brief 文字列表を解析し、状態操作引数を構築する。
	/// @warning Psyque::RuleEngine 管理者以外は、この関数は使用禁止。
	public: template<typename template_hasher, typename template_table>
	static This _build(
		/// [in,out] 文字列からハッシュ値を作る関数オブジェクト。
		template_hasher& io_hasher,
		/// [in] 解析する Psyque::string::table 。
		template_table const& in_table,
		/// [in] 解析する Psyque::string::table の行番号。
		typename template_table::number const in_row_number,
		/// [in] 解析する Psyque::string::table の列番号。
		typename template_table::number const in_column_number)
	{
		check(in_row_number != in_table.get_attribute_row());
		This local_operation;

		// 演算子の左辺となる状態値の識別値を取得する。
		typename template_hasher::argument_type const local_left_key_cell(
			in_table.find_cell(in_row_number, in_column_number));
		local_operation.Key = io_hasher(local_left_key_cell);
		if (local_operation.Key
			== io_hasher(typename template_hasher::argument_type()))
		{
			check(local_left_key_cell.empty());
			return local_operation;
		}

		// 演算子を取得する。
		auto const LocalMakeOperator(
			This::MakeOperator(
				local_operation.Operator,
				typename template_hasher::argument_type(
					in_table.find_cell(in_row_number, in_column_number + 1))));
		if (!LocalMakeOperator)
		{
			check(false);
			return local_operation;
		}

		// 演算子の右辺値を取得する。
		local_operation.make_right_value(
			io_hasher,
			in_table.find_cell(in_row_number, in_column_number + 2));
		check(!local_operation.Value.is_empty());
		return local_operation;
	}

	//-------------------------------------------------------------------------
	private:
	PSYQUE_CONSTEXPR TStatusOperation() PSYQUE_NOEXCEPT {}

	/// @brief 文字列を解析し、比較演算子を構築する。
	template<typename TemplateString>
	static bool MakeOperator(
		/// [out] 比較演算子の格納先。
		typename TemplateStatusValue::EComparison& OutOperator,
		/// [in] 解析する文字列。
		TemplateString const& InString)
	{
		if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_EQUAL)
		{
			OutOperator = TemplateStatusValue::EComparison::EQUAL;
		}
		else if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL)
		{
			OutOperator = TemplateStatusValue::EComparison::NOT_EQUAL;
		}
		else if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_LESS)
		{
			OutOperator = TemplateStatusValue::EComparison::LESS;
		}
		else if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL)
		{
			OutOperator = TemplateStatusValue::EComparison::LESS_EQUAL;
		}
		else if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_GREATER)
		{
			OutOperator = TemplateStatusValue::EComparison::GREATER;
		}
		else if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL)
		{
			OutOperator = TemplateStatusValue::EComparison::GREATER_EQUAL;
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
		typename TemplateStatusValue::EAssignment& OutOperator,
		/// [in] 解析する文字列。
		TemplateString const& InString)
	{
		if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_COPY)
		{
			OutOperator = TemplateStatusValue::EAssignment::COPY;
		}
		else if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_ADD)
		{
			OutOperator = TemplateStatusValue::EAssignment::ADD;
		}
		else if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_SUB)
		{
			OutOperator = TemplateStatusValue::EAssignment::SUB;
		}
		else if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_MULT)
		{
			OutOperator = TemplateStatusValue::EAssignment::MULT;
		}
		else if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_DIV)
		{
			OutOperator = TemplateStatusValue::EAssignment::DIV;
		}
		else if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_MOD)
		{
			OutOperator = TemplateStatusValue::EAssignment::MOD;
		}
		else if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_OR)
		{
			OutOperator = TemplateStatusValue::EAssignment::OR;
		}
		else if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_XOR)
		{
			OutOperator = TemplateStatusValue::EAssignment::XOR;
		}
		else if (InString == PSYQUE_RULE_ENGINE_STATUS_OPERATION_BUILDER_AND)
		{
			OutOperator = TemplateStatusValue::EAssignment::AND;
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
	private: template<typename template_hasher>
	void make_right_value(
		/// [in,out] 文字列のハッシュ関数。
		template_hasher& io_hasher,
		/// [in] 解析する文字列。
		typename template_hasher::argument_type const& InString)
	{
		// 状態値の接頭辞があるなら、状態値の識別値を構築する。
		typename template_hasher::argument_type const local_status_header(
			PSYQUE_RULE_ENGINE_STATUS_OPERATION_RIGHT_STATUS); 
		if (local_status_header == InString.substr(0, local_status_header.size()))
		{
			this->bIsValidRightKey = true;
			this->Value = TemplateStatusValue(
				static_cast<typename TemplateStatusValue::Type>(
					io_hasher(InString.substr(local_status_header.size()))));
			return;
		}

		// ハッシュ値の接頭辞があるなら、ハッシュ値を構築する。
		this->bIsValidRightKey = false;
		typename template_hasher::argument_type const local_hash_header(
			PSYQUE_RULE_ENGINE_STATUS_OPERATION_RIGHT_HASH); 
		if (local_hash_header == InString.substr(0, local_hash_header.size()))
		{
			static_assert(
				sizeof(typename template_hasher::result_type)
				<= sizeof(typename TemplateStatusValue::Type)
				&& std::is_unsigned<typename template_hasher::result_type>::value,
				"");
			this->Value = TemplateStatusValue(
				static_cast<typename TemplateStatusValue::Type>(
					io_hasher(InString.substr(local_status_header.size()))));
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
		/// TStatusValue::EKind::EMPTY の場合は、自動決定する。
		typename TemplateStatusValue::EKind const in_kind =
			TemplateStatusValue::EKind::EMPTY)
	{
		// 論理値として構築する。
		if (in_kind == TemplateStatusValue::EKind::BOOL
			|| in_kind == TemplateStatusValue::EKind::EMPTY)
		{
			Psyque::string::numeric_parser<bool> const local_bool_parser(InString);
			if (local_bool_parser.is_completed())
			{
				return TemplateStatusValue(local_bool_parser.GetValue());
			}
			else if (in_kind == TemplateStatusValue::EKind::BOOL)
			{
				return TemplateStatusValue();
			}
		}
		check(in_kind != TemplateStatusValue::EKind::BOOL);

		// 符号なし整数として構築する。
		Psyque::string::numeric_parser<
			typename TemplateStatusValue::Type>
				const local_unsigned_parser(InString);
		if (local_unsigned_parser.is_completed())
		{
			switch (in_kind)
			{
				case TemplateStatusValue::EKind::FLOAT:
				return TemplateStatusValue(
					static_cast<typename TemplateStatusValue::FloatType>(
						local_unsigned_parser.GetValue()));

				case TemplateStatusValue::EKind::SIGNED:
				return TemplateStatusValue(
					static_cast<typename TemplateStatusValue::SignedType>(
						local_unsigned_parser.GetValue()));

				default:
				return TemplateStatusValue(local_unsigned_parser.GetValue());
			}
		}

		// 符号あり整数として構築する。
		Psyque::string::numeric_parser<
			typename TemplateStatusValue::SignedType>
				const local_signed_parser(InString);
		if (local_unsigned_parser.is_completed())
		{
			switch (in_kind)
			{
				case TemplateStatusValue::EKind::FLOAT:
				return TemplateStatusValue(
					static_cast<typename TemplateStatusValue::FloatType>(
						local_signed_parser.GetValue()));

				case TemplateStatusValue::EKind::UNSIGNED:
				return TemplateStatusValue();

				default:
				return TemplateStatusValue(local_signed_parser.GetValue());
			}
		}

		// 浮動小数点数として構築する。
		Psyque::string::numeric_parser<typename TemplateStatusValue::FloatType>
			const local_float_parser(InString);
		if (local_float_parser.is_completed())
		{
			switch (in_kind)
			{
				case TemplateStatusValue::EKind::EMPTY:
				case TemplateStatusValue::EKind::FLOAT:
				return TemplateStatusValue(local_float_parser.GetValue());

				default: break;
			}
		}
		return TemplateStatusValue();
	}

	//-------------------------------------------------------------------------
	private:
	TemplateStatusValue Value;       ///< 演算の右辺値となる値。
	TemplateStatusKey Key;           ///< 演算の左辺値となる状態値の識別値。
	TemplateStatusOperator Operator; ///< 演算子の種類。
	bool bIsValidRightKey;           ///< 右辺値を状態値から取得するか。

}; // class Psyque::RuleEngine::_private::TStatusOperation

// vim: set noexpandtab:
