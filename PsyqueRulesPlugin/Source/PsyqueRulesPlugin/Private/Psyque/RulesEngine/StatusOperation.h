// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::TStatusOperation
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

//#include "../string/numeric_parser.h"

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_EQUAL
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_EQUAL "=="
#endif // !define(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_EQUAL)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL "!="
#endif // !define(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_LESS
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_LESS "<"
#endif // !define(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_LESS)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL "<="
#endif // !define(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_GREATER
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_GREATER ">"
#endif // !define(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_GREATER)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL ">="
#endif // !define(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_COPY
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_COPY ":="
#endif // !defined(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_COPY)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_ADD
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_ADD "+="
#endif // !defined(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_ADD)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_SUB
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_SUB "-="
#endif // !defined(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_SUB)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_MULT
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_MULT "*="
#endif // !defined(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_MULT)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_DIV
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_DIV "/="
#endif // !defined(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_DIV)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_MOD
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_MOD "%="
#endif // !defined(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_MOD)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_OR
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_OR "|="
#endif // !defined(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_OR)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_XOR
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_XOR "^="
#endif // !defined(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_XOR)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_AND
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_AND "&="
#endif // !defined(PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_AND)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_RIGHT_STATUS
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_RIGHT_STATUS "STATUS:"
#endif // !defined(PSYQUE_RULES_ENGINE_STATUS_OPERATION_RIGHT_STATUS)

#ifndef PSYQUE_RULES_ENGINE_STATUS_OPERATION_RIGHT_HASH
#define PSYQUE_RULES_ENGINE_STATUS_OPERATION_RIGHT_HASH "HASH:"
#endif // !define(PSYQUE_RULES_ENGINE_STATUS_OPERATION_RIGHT_HASH)

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
	/// @warning Psyque::RulesEngine 管理者以外は、この関数は使用禁止。
	public: template<
		typename TemplateContainer,
		typename TemplateHasher,
		typename TemplateTable>
	static void _build_container(
		/// [in,out] 構築した TStatusOperation を格納するコンテナ。
		TemplateContainer& OutOperations,
		/// [in,out] 文字列からハッシュ値を作る std::hash 。
		TemplateHasher& OutHashFunction,
		/// [in] 解析する Psyque::string::table 。
		TemplateTable const& InTable,
		/// [in] 解析する Psyque::string::table の行番号。
		typename TemplateTable::FNumber const InRowNumber,
		/// [in] 解析する Psyque::string::table の属性の列番号。
		typename TemplateTable::FNumber const InColumnNumber,
		/// [in] 解析する Psyque::string::table の属性の列数。
		typename TemplateTable::FNumber const InColumnCount)
	{
		typename TemplateTable::FNumber const LocalUnitCount(3);
		auto const LocalOperationCount(InColumnCount / LocalUnitCount);
		if (LocalOperationCount < 1)
		{
			return;
		}
		OutOperations.reserve(OutOperations.size() + LocalOperationCount);
		auto const LocalColumnEnd(
			InColumnNumber + InColumnCount - LocalUnitCount);
		for (auto i(InColumnNumber); i <= LocalColumnEnd; i += LocalUnitCount)
		{
			auto const LocalOperation(
				ThisClass::_build(OutHashFunction, InTable, InRowNumber, i));
			if (!LocalOperation.GetValue().IsEmpty())
			{
				OutOperations.push_back(LocalOperation);
			}
		}
	}

	/// @brief 文字列表を解析し、状態操作引数を構築する。
	/// @warning Psyque::RulesEngine 管理者以外は、この関数は使用禁止。
	public: template<typename TemplateHasher, typename TemplateTable>
	static ThisClass _build(
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
		ThisClass LocalOperation;

		// 演算子の左辺となる状態値の識別値を取得する。
		typename TemplateHasher::argument_type const LocalLeftKeyCell(
			InTable.FindCell(InRowNumber, InColumnNumber));
		LocalOperation.Key = OutHashFunction(LocalLeftKeyCell);
		if (LocalOperation.Key
			== OutHashFunction(typename TemplateHasher::argument_type()))
		{
			check(LocalLeftKeyCell.empty());
			return LocalOperation;
		}

		// 演算子を取得する。
		auto const LocalMakeOperator(
			ThisClass::MakeOperator(
				LocalOperation.Operator,
				typename TemplateHasher::argument_type(
					InTable.FindCell(InRowNumber, InColumnNumber + 1))));
		if (!LocalMakeOperator)
		{
			check(false);
			return LocalOperation;
		}

		// 演算子の右辺値を取得する。
		LocalOperation.make_right_value(
			OutHashFunction,
			InTable.FindCell(InRowNumber, InColumnNumber + 2));
		check(!LocalOperation.Value.IsEmpty());
		return LocalOperation;
	}

	//-------------------------------------------------------------------------
	private: TStatusOperation() PSYQUE_NOEXCEPT = default;

	/// @brief 文字列を解析し、比較演算子を構築する。
	private: template<typename TemplateString>
	static bool MakeOperator(
		/// [out] 比較演算子の格納先。
		RulesEngine::EStatusComparison& OutOperator,
		/// [in] 解析する文字列。
		TemplateString const& InString)
	{
		if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_EQUAL)
		{
			OutOperator = RulesEngine::EStatusComparison::Equal;
		}
		else if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL)
		{
			OutOperator = RulesEngine::EStatusComparison::NotEqual;
		}
		else if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_LESS)
		{
			OutOperator = RulesEngine::EStatusComparison::Less;
		}
		else if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL)
		{
			OutOperator = RulesEngine::EStatusComparison::LessEqual;
		}
		else if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_GREATER)
		{
			OutOperator = RulesEngine::EStatusComparison::Greater;
		}
		else if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL)
		{
			OutOperator = RulesEngine::EStatusComparison::GreaterEqual;
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
		RulesEngine::EStatusAssignment& OutOperator,
		/// [in] 解析する文字列。
		TemplateString const& InString)
	{
		if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_COPY)
		{
			OutOperator = RulesEngine::EStatusAssignment::Copy;
		}
		else if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_ADD)
		{
			OutOperator = RulesEngine::EStatusAssignment::Add;
		}
		else if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_SUB)
		{
			OutOperator = RulesEngine::EStatusAssignment::Sub;
		}
		else if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_MULT)
		{
			OutOperator = RulesEngine::EStatusAssignment::Mul;
		}
		else if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_DIV)
		{
			OutOperator = RulesEngine::EStatusAssignment::Div;
		}
		else if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_MOD)
		{
			OutOperator = RulesEngine::EStatusAssignment::Mod;
		}
		else if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_OR)
		{
			OutOperator = RulesEngine::EStatusAssignment::Or;
		}
		else if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_XOR)
		{
			OutOperator = RulesEngine::EStatusAssignment::Xor;
		}
		else if (InString == PSYQUE_RULES_ENGINE_STATUS_OPERATION_BUILDER_AND)
		{
			OutOperator = RulesEngine::EStatusAssignment::And;
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
			PSYQUE_RULES_ENGINE_STATUS_OPERATION_RIGHT_STATUS); 
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
		typename TemplateHasher::argument_type const LocalHashHeader(
			PSYQUE_RULES_ENGINE_STATUS_OPERATION_RIGHT_HASH); 
		if (LocalHashHeader == InString.substr(0, LocalHashHeader.size()))
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
			this->Value = ThisClass::make_status_value(InString);
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
		/// RulesEngine::EStatusKind::Empty の場合は、自動決定する。
		RulesEngine::EStatusKind const InKind = RulesEngine::EStatusKind::Empty)
	{
		// 論理値として構築する。
		if (InKind == RulesEngine::EStatusKind::Bool
			|| InKind == RulesEngine::EStatusKind::Empty)
		{
			Psyque::string::numeric_parser<bool> const local_bool_parser(InString);
			if (local_bool_parser.IsCompleted())
			{
				return TemplateStatusValue(local_bool_parser.GetValue());
			}
			else if (InKind == RulesEngine::EStatusKind::Bool)
			{
				return TemplateStatusValue();
			}
		}
		check(InKind !=RulesEngine::EStatusKind::Bool);

		// 符号なし整数として構築する。
		Psyque::string::numeric_parser<
			typename TemplateStatusValue::FUnsigned>
				const local_unsigned_parser(InString);
		if (local_unsigned_parser.IsCompleted())
		{
			switch (InKind)
			{
				case RulesEngine::EStatusKind::Float:
				return TemplateStatusValue(
					static_cast<typename TemplateStatusValue::FFloat>(
						local_unsigned_parser.GetValue()));

				case RulesEngine::EStatusKind::Signed:
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
				case RulesEngine::EStatusKind::Float:
				return TemplateStatusValue(
					static_cast<typename TemplateStatusValue::FFloat>(
						local_signed_parser.GetValue()));

				case RulesEngine::EStatusKind::Unsigned:
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
				case RulesEngine::EStatusKind::Empty:
				case RulesEngine::EStatusKind::Float:
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

}; // class Psyque::RulesEngine::_private::TStatusOperation

// vim: set noexpandtab:
