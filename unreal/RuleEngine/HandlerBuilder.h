/// @file
/// @brief @copybrief Psyque::RuleEngine::THandlerBuilder
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

/// @brief 文字列表で、条件式の識別値が記述されている属性の名前。
/// @details
///   Psyque::RuleEngine::THandlerBuilder で解析する文字列表で、
///   Psyque::RuleEngine::TDriver::FEvaluator::FExpressionKey として解析する属性の名前。
#ifndef PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KEY
#define PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KEY "KEY"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KEY)

/// @brief 文字列表で、挙動条件として解析する属性の名前。
#ifndef PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_CONDITION
#define PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_CONDITION "CONDITION"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_CONDITION)

/// @brief 文字列表で、NULL条件が記述されている属性の名前。
/// @details
///   Psyque::RuleEngine::THandlerBuilder で解析する文字列表で、
///   Psyque::RuleEngine::TDriver::FDispatcher::FHandler::EUnitCondition::Failed
///   として解析する文字列。
#ifndef PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_NULL
#define PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_NULL "NULL"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT)

/// @brief 文字列表で、Any条件が記述されている属性の名前。
/// @details
///   Psyque::RuleEngine::THandlerBuilder で解析する文字列表で、
///   Psyque::RuleEngine::TDriver::FDispatcher::FHandler::EUnitCondition::Any
///   として解析する文字列。
#ifndef PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_ANY
#define PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_ANY "Any"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT)

/// @brief 文字列表で、条件挙動関数の優先順位が記述されている属性の名前。
/// @details
///   Psyque::RuleEngine::THandlerBuilder で解析する文字列表で、
///   Psyque::RuleEngine::TDriver::FDispatcher::FHandler::FPriority
///   として解析する属性の名前。
#ifndef PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_PRIORITY
#define PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_PRIORITY "PRIORITY"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_PRIORITY)

/// @brief 文字列表で、条件挙動の種別として解析する属性の名前。
#ifndef PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KIND
#define PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KIND "KIND"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KIND)

/// @brief 文字列表で、代入演算する条件挙動の種別として解析する文字列。
#ifndef PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_KIND_STATUS_ASSIGNMENT
#define PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_KIND_STATUS_ASSIGNMENT "STATUS_ASSIGNMENT"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_KIND_STATUS_ASSIGNMENT)

/// @brief 文字列表で、条件挙動の引数として解析する属性の名前。
#ifndef PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT
#define PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT "ARGUMENT"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT)

/// @brief 文字列表で、遅延種別をYieldとして解析する文字列。
/// @details
///   Psyque::RuleEngine::THandlerBuilder で解析する文字列表で、
///   Psyque::RuleEngine::EAccumulationDelay::Yield として解析する文字列。
#ifndef PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_YIELD
#define PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_YIELD "Yield"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_YIELD)

/// @brief 文字列表で、遅延種別をBlockとして解析する文字列。
/// @details
/// Psyque::RuleEngine::THandlerBuilder で解析する文字列表で、
/// Psyque::RuleEngine::EAccumulationDelay::Block として解析する文字列。
#ifndef PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_BLOCK
#define PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_BLOCK "Block"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_BLOCK)

/// @brief 文字列表で、遅延種別をNonblockとして解析する文字列。
/// @details
///   Psyque::RuleEngine::THandlerBuilder で解析する文字列表で、
///   Psyque::RuleEngine::EAccumulationDelay::Nonblock として解析する文字列。
#ifndef PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_NONBLOCK
#define PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_NONBLOCK "Nonblock"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_NONBLOCK)

/// @cond
namespace Psyque
{
	namespace RuleEngine
	{
		class THandlerBuilder;
	} // namespace RuleEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 文字列表から条件挙動ハンドラを構築して登録する関数オブジェクト。
/// @details TDriver::ExtendChunk の引数として使う。
class Psyque::RuleEngine::THandlerBuilder
{
	/// @brief this が指す値の型。
	private: typedef THandlerBuilder This;

	//-------------------------------------------------------------------------
	/// @brief 条件挙動ハンドラの構築に使う文字列表の属性。
	private: template<typename TemplateRelationTable>
	class FTableAttribute
	{
		public: FTableAttribute(TemplateRelationTable const& InTable)
		PSYQUE_NOEXCEPT:
		Key(
			InTable.FindAttribute(
				PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KEY)),
		Condition(
			InTable.FindAttribute(
				PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_CONDITION)),
		Priority(
			InTable.FindAttribute(
				PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_PRIORITY)),
		Kind(
			InTable.FindAttribute(
				PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KIND)),
		argument_(
			InTable.FindAttribute(
				PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT))
		{}

		bool IsValid() const PSYQUE_NOEXCEPT
		{
			return 1 <= this->Key.second
				&& 1 <= this->Condition.second
				&& 1 <= this->Priority.second
				&& 1 <= this->Kind.second
				&& 1 <= this->argument_.second;
		}

		/// @brief 条件挙動ハンドラに対応する条件式の識別値の列番号と列数。
		public: typename TemplateRelationTable::FAttribute Key;
		/// @brief 挙動条件の列番号と列数。
		public: typename TemplateRelationTable::FAttribute Condition;
		/// @brief 条件挙動関数の呼び出し優先順位の列番号と列数。
		public: typename TemplateRelationTable::FAttribute Priority;
		/// @brief 条件挙動関数の種類の列番号と列数。
		public: typename TemplateRelationTable::FAttribute Kind;
		/// @brief 条件挙動関数の引数の列番号と列数。
		public: typename TemplateRelationTable::FAttribute argument_;

	}; // class FTableAttribute

	//-------------------------------------------------------------------------
	/// @copydoc This::register_handlers
	public: template<
		typename TemplateDispatcher,
		typename TemplateHasher,
		typename TemplateAccumulator,
		typename TemplateRelationTable>
	std::vector<
		typename TemplateDispatcher::FHandler::FFunctionSharedPtr,
		typename TemplateDispatcher::FAllocator>
	operator()(
		/// [in,out] 条件挙動ハンドラを登録する条件挙動器。
		TemplateDispatcher& io_dispatcher,
		/// [in,out] 文字列からハッシュ値を作る TDriver::FHasher 。
		TemplateHasher const& InHashFunction,
		/// [in,out] 条件挙動関数で使う TDriver::TAccumulator 。
		TemplateAccumulator& OutAccumulator,
		/// [in] 条件挙動関数が記述されている Psyque::string::TRelationTable 。
		/// 空の場合は条件挙動関数は構築されない。
		TemplateRelationTable const& InTable)
	const
	{
		return This::register_handlers(
			io_dispatcher, InHashFunction, OutAccumulator, InTable);
	}

	/// @brief 文字列表から条件挙動ハンドラを構築し、条件挙動器へ登録する。
	/// @details
	///   InTable から TDriver::TDispatcher::FHandler::FFunction
	///   を構築し、それを弱参照する TDriver::TDispatcher::FHandler を
	///   io_dispatcher へ登録する。
	/// @return 構築した TDriver::TDispatcher::FHandler::FFunction の強参照のコンテナ。
	public: template<
		typename TemplateDispatcher,
		typename TemplateHasher,
		typename TemplateAccumulator,
		typename TemplateRelationTable>
	static std::vector<
		typename TemplateDispatcher::FHandler::FFunctionSharedPtr,
		typename TemplateDispatcher::FAllocator>
	register_handlers(
		/// [in,out] 条件挙動ハンドラを登録する条件挙動器。
		TemplateDispatcher& io_dispatcher,
		/// [in,out] 文字列からハッシュ値を作る TDriver::FHasher 。
		TemplateHasher const& InHashFunction,
		/// [in,out] 条件挙動ハンドラの関数で使う TDriver::TAccumulator 。
		TemplateAccumulator& OutAccumulator,
		/// [in] 条件挙動関数が記述されている Psyque::string::TRelationTable 。
		/// 空の場合は条件挙動関数は構築されない。
		TemplateRelationTable const& InTable)
	{
		std::vector<
			typename TemplateDispatcher::FHandler::FFunctionSharedPtr,
			typename TemplateDispatcher::FAllocator>
				local_functions(io_dispatcher.get_allocator());

		// 文字列表の属性を取得する。
		typename This::FTableAttribute<TemplateRelationTable> const
			LocalAttribute(InTable);
		if (!LocalAttribute.IsValid())
		{
			PSYQUE_ASSERT(InTable.GetCells().empty());
			return local_functions;
		}

		// 文字列表を解析し、条件挙動ハンドラを登録する。
		auto const local_empty_key(
			InHashFunction(typename TemplateHasher::argument_type()));
		auto const LocalRowCount(InTable.GetRowCount());
		local_functions.reserve(LocalRowCount);
		for (
			typename TemplateRelationTable::FNumber i(0);
			i < LocalRowCount;
			++i)
		{
			if (i == InTable.GetAttributeRow())
			{
				continue;
			}

			// 条件式の識別値を取得する。
			auto const LocalExpressionKey(
				InHashFunction(InTable.FindCell(i, LocalAttribute.Key.first)));
			if (LocalExpressionKey == local_empty_key)
			{
				// 条件式の識別値が正しくなかった。
				PSYQUE_ASSERT(false);
				continue;
			}

			// 条件挙動関数の優先順位を取得する。
			typedef typename TemplateDispatcher::FHandler FHandler;
			typename FHandler::FPriority local_priority(
				PSYQUE_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT);
			if (!InTable.parse_cell(
					local_priority, i, LocalAttribute.Priority.first, true))
			{
				// 優先順位として解析しきれなかった。
				PSYQUE_ASSERT(false);
				continue;
			}

			// 条件挙動関数を構築し、条件挙動ハンドラを条件挙動器に登録する。
			auto LocalFunction(
				This::build_function<FHandler>(
					InHashFunction, OutAccumulator, InTable, i, LocalAttribute));
			if (io_dispatcher.RegisterHandler(
					LocalExpressionKey,
					This::BuildCondition<FHandler>(
						InTable, i, LocalAttribute.Condition),
					LocalFunction,
					local_priority))
			{
				local_functions.push_back(MoveTemp(LocalFunction));
			}
			else
			{
				// 条件挙動ハンドラの登録に失敗した。
				PSYQUE_ASSERT(false);
			}
		}
		local_functions.shrink_to_fit();
		return local_functions;
	}

	//-------------------------------------------------------------------------
	/// @name 挙動条件の構築
	/// @{

	/// @brief 文字列表の行を解析し、挙動条件を構築する。
	/// @return
	///   文字列表から構築した挙動条件。構築に失敗した場合は
	///   TDriver::TDispatcher::FHandler::EUnitCondition::Invalid を返す。
	/// @tparam TemplateHandler
	///   構築した挙動条件を使う TDriver::TDispatcher::FHandler 。
	public: template<typename TemplateHandler, typename TemplateRelationTable>
	static typename TemplateHandler::FCondition BuildCondition(
		/// [in] 解析する Psyque::string::TRelationTable 。
		TemplateRelationTable const& InTable,
		/// [in] InTable で解析する行の番号。
		typename TemplateRelationTable::FNumber const InRowNumber,
		/// [in] InTable で挙動条件が記述されている属性の列番号と列数。
		typename TemplateRelationTable::FAttribute const& InAttribute)
	{
		if (InAttribute.second < 2)
		{
			PSYQUE_ASSERT(false);
			return TemplateHandler::EUnitCondition::Invalid;
		}
		auto const LocalCondition(
			TemplateHandler::MakeCondition(
				This::ParseUnitCondition<TemplateHandler>(
					InTable.FindCell(InRowNumber, InAttribute.first + 1)),
				This::ParseUnitCondition<TemplateHandler>(
					InTable.FindCell(InRowNumber, InAttribute.first))));
		PSYQUE_ASSERT(LocalCondition != TemplateHandler::EUnitCondition::Invalid);
		return LocalCondition;
	}

	/// @brief 文字列を解析し、単位条件を取得する。
	/// @return
	///   文字列から取得した TDriver::TDispatcher::FHandler::EUnitCondition::Type 。
	///   取得に失敗した場合は
	///   TDriver::TDispatcher::FHandler::EUnitCondition::Invalid を返す。
	/// @tparam TemplateHandler
	///   構築した単位条件を使う TDriver::TDispatcher::FHandler 。
	public: template<typename TemplateHandler, typename TemplateString>
	static typename TemplateHandler::EUnitCondition::Type ParseUnitCondition(
		/// [in] 解析する std::basic_string_view 互換の文字列。
		TemplateString const& InString)
	{
		if (!InString.empty())
		{
			typedef
				Psyque::String::TView<
					typename TemplateString::value_type,
					typename TemplateString::traits_type>
				FStringView;
			auto const local_not(InString.at(0) == '!');
			auto const local_string(
				FStringView(InString).substr(local_not? 1: 0));
			Psyque::string::numeric_parser<bool> const LocalParser(local_string);
			if (LocalParser.IsCompleted())
			{
				if (LocalParser.GetValue())
				{
					return local_not?
						TemplateHandler::EUnitCondition::NotTrue:
						TemplateHandler::EUnitCondition::True;
				}
				else
				{
					return local_not?
						TemplateHandler::EUnitCondition::NotFalse:
						TemplateHandler::EUnitCondition::False;
				}
			}
			else if (
				local_string ==
					PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_NULL)
			{
				return local_not?
					TemplateHandler::EUnitCondition::NotFailed:
					TemplateHandler::EUnitCondition::Failed;
			}
			else if (
				!local_not
				&& local_string ==
					PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_ANY)
			{
				return TemplateHandler::EUnitCondition::Any;
			}
		}
		return TemplateHandler::EUnitCondition::Invalid;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 条件挙動関数の構築
	/// @{

	/// @brief 文字列表の行を解析し、状態値を代入演算する条件挙動関数を構築する。
	/// @return
	///   状態値を代入演算する TDriver::TDispatcher::FHandler::FFunction 。
	///   代入演算が記述されてない場合は、空となる。
	/// @tparam TemplateHandler
	///   構築した条件挙動関数を使う TDriver::TDispatcher::FHandler 。
	public: template<
		typename TemplateHandler,
		typename TemplateHasher,
		typename TemplateAccumulator,
		typename TemplateRelationTable>
	static typename TemplateHandler::FFunctionSharedPtr
	build_status_assignment_function(
		/// [in,out] 文字列からハッシュ値を作る TDriver::FHasher 。
		TemplateHasher const& InHashFunction,
		/// [in,out] 条件挙動関数から使う TDriver::TAccumulator 。
		TemplateAccumulator& OutAccumulator,
		/// [in] 解析する Psyque::string::TRelationTable 。
		TemplateRelationTable const& InTable,
		/// [in] InTable の解析する行の番号。
		typename TemplateRelationTable::FNumber const InRowNumber,
		/// [in] InTable で代入演算が記述されている属性の列番号と列数。
		typename TemplateRelationTable::FAttribute const& in_assignments)
	{
		if (in_assignments.second < 1)
		{
			return typename TemplateHandler::FFunctionSharedPtr();
		}

		// 状態値の代入演算のコンテナを構築する。
		std::vector<
			typename TemplateAccumulator::FReservoir::FStatusAssignment,
			typename TemplateAccumulator::FAllocator>
				local_assignments(OutAccumulator.get_allocator());
		TemplateAccumulator::FReservoir::FStatusAssignment::_build_container(
			local_assignments,
			InHashFunction,
			InTable,
			InRowNumber,
			in_assignments.first + 1,
			in_assignments.second - 1);
		return This::create_status_assignment_function<TemplateHandler>(
			OutAccumulator,
			This::parse_delay<TemplateAccumulator>(
				typename TemplateRelationTable::String::TView(
					InTable.FindCell(InRowNumber, in_assignments.first))),
			local_assignments);
	}

	/// @brief 状態値を代入演算する条件挙動関数を構築する。
	/// @return
	///   状態値を代入演算する TDriver::TDispatcher::FHandler::FFunction 。
	///   in_assignments が空の場合は空を返す。
	/// @tparam TemplateHandler
	///   構築した条件挙動関数を使う TDriver::TDispatcher::FHandler 。
	public: template<
		typename TemplateHandler,
		typename TemplateAccumulator,
		typename template_assignment_container>
	static typename TemplateHandler::FFunctionSharedPtr
	create_status_assignment_function(
		/// [in,out] 条件挙動関数から使う TDriver::TAccumulator 。
		TemplateAccumulator& OutAccumulator,
		/// [in] in_assignments の先頭要素の TDriver::TAccumulator::Accumulate
		/// に渡す、 TDriver::TAccumulator::EDelay::Type 。
		typename TemplateAccumulator::EDelay::Type const InFrontDelay,
		/// [in] 条件挙動関数から呼び出す TDriver::TAccumulator::Accumulate
		/// に渡す、 TDriver::FReservoir::FStatusAssignment のコンテナ。
		template_assignment_container const& in_assignments)
	{
		return std::begin(in_assignments) != std::end(in_assignments)?
			std::allocate_shared<typename TemplateHandler::FFunction>(
				in_assignments.get_allocator(),
				/// @todo OutAccumulator を参照渡しするのは危険。対策を考えたい。
				[=, &OutAccumulator](
					typename TemplateHandler::FExpressionKey const&,
					Psyque::ETernary const,
					Psyque::ETernary const)
				{
					OutAccumulator.Accumulate(in_assignments, InFrontDelay);
				}):
			typename TemplateHandler::FFunctionSharedPtr();
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @brief 文字列表を解析し、条件挙動関数を構築する。
	/// @return 構築した条件挙動関数。
	/// @tparam TemplateHandler
	///   構築した条件挙動関数を使う TDriver::TDispatcher::FHandler 。
	private: template<
		typename TemplateHandler,
		typename TemplateHasher,
		typename TemplateAccumulator,
		typename TemplateRelationTable>
	static typename TemplateHandler::FFunctionSharedPtr
	build_function(
		/// [in,out] 文字列からハッシュ値を作る TDriver::FHasher 。
		TemplateHasher const& InHashFunction,
		/// [in,out] 条件挙動で使う TDriver::TAccumulator 。
		TemplateAccumulator& OutAccumulator,
		/// [in] 解析する Psyque::string::TRelationTable 。
		TemplateRelationTable const& InTable,
		/// [in] InTable の解析する行の番号。
		typename TemplateRelationTable::FNumber const InRowNumber,
		/// [in] InTable の属性。
		typename This::FTableAttribute<TemplateRelationTable> const&
			InAttribute)
	{
		// 挙動関数の種類を取得する。
		typename TemplateRelationTable::String::TView const LocalKindCell(
			InTable.FindCell(InRowNumber, InAttribute.Kind.first));
		if (LocalKindCell
			== PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_KIND_STATUS_ASSIGNMENT)
		{
			return This::build_status_assignment_function<TemplateHandler>(
				InHashFunction,
				OutAccumulator,
				InTable,
				InRowNumber,
				InAttribute.argument_);
		}
		else
		{
			// 未知の種類だった。
			PSYQUE_ASSERT(false);
			return typename TemplateHandler::FFunctionSharedPtr();
		}
	}

	private: template<typename TemplateAccumulator, typename TemplateString>
	static typename TemplateAccumulator::EDelay::Type parse_delay(
		TemplateString const& InString)
	{
		if (InString == PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_BLOCK)
		{
			return TemplateAccumulator::EDelay::Block;
		}
		if (InString == PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_NONBLOCK)
		{
			return TemplateAccumulator::EDelay::Nonblock;
		}
		PSYQUE_ASSERT(InString == PSYQUE_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_YIELD);
		return TemplateAccumulator::EDelay::Yield;
	}

}; // class Psyque::RuleEngine::THandlerBuilder

// vim: set noexpandtab:
