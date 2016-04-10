/// @file
/// @brief @copybrief Psyque::RulesEngine::TExpressionBuilder
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include "Json.h"
#include "./Assert.h"

/// @brief 文字列表で、条件式の識別値が記述されている属性の名前。
/// @details
/// Psyque::RulesEngine::TExpressionBuilder で解析する文字列表で、
/// Psyque::RulesEngine::TDriver::FEvaluator::FExpressionKey
/// として解析する属性の名前。
#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY "KEY"
#endif // !defined(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY)

/// @brief 文字列表で、条件式の論理演算子が記述されている属性の名前。
/// @details
/// Psyque::RulesEngine::TExpressionBuilder で解析する文字列表で、
/// EPsyqueRulesExpressionLogic として解析する属性の名前。
#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC "LOGIC"
#endif // !defined(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC)

/// @brief 文字列表で、条件式の種別が記述されている属性の名前。
/// @details
/// Psyque::RulesEngine::TExpressionBuilder で解析する文字列表で、
/// EPsyqueRulesExpressionKind として解析する属性の名前。
#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND "KIND"
#endif // !defined(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND)

/// @brief 文字列表で、条件式の論理項要素が記述されている属性の名前。
/// @details
/// Psyque::RulesEngine::TExpressionBuilder で解析する文字列表で、
/// Psyque::RulesEngine::TDriver::FEvaluator::FExpression
/// の論理項要素として解析する属性の名前。
#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT "ELEMENT"
#endif // !defined(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT)

/// @brief 文字列表で、条件式の論理和演算子に対応する文字列。
/// @details
/// Psyque::RulesEngine::TExpressionBuilder で解析する文字列表で、
/// EPsyqueRulesExpressionLogic::Or として解析する文字列。
#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LOGIC_OR
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LOGIC_OR "Or"
#endif // !defined(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LOGIC_OR)

/// @brief 文字列表で、条件式の論理積演算子に対応する文字列。
/// @details
/// Psyque::RulesEngine::TExpressionBuilder で解析する文字列表で、
/// EPsyqueRulesExpressionLogic::And として解析する文字列。
#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LOGIC_AND
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LOGIC_AND "And"
#endif // !defined(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LOGIC_AND)

/// @brief 文字列表で、複合条件式の種別に対応する文字列。
/// @details
/// Psyque::RulesEngine::TExpressionBuilder で解析する文字列表で、
/// EPsyqueRulesExpressionKind::SubExpression
/// として解析する文字列。
#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION "SubExpression"
#endif // !defined(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION)

/// @brief 文字列表で、状態変化条件式の種別に対応する文字列。
/// @details
/// Psyque::RulesEngine::TExpressionBuilder で解析する文字列表で、
/// EPsyqueRulesExpressionKind::StatusTransition
/// として解析する文字列。
#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_TRANSITION
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_TRANSITION "StatusTransition"
#endif // !define(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_TRANSITION)

/// @brief 文字列表で、状態比較条件式の種別に対応する文字列。
/// @details
/// Psyque::RulesEngine::TExpressionBuilder で解析する文字列表で、
/// EPsyqueRulesExpressionKind::StatusComparison
/// として解析する文字列。
#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_COMPARISON
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_COMPARISON "StatusComparison"
#endif // !define(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_COMPARISON)

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		class TExpressionBuilder;
		class TExpressionBuilder_;
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 文字列表から条件式を構築して登録する関数オブジェクト。
/// @details Psyque::RulesEngine::TDriver::ExtendChunk の引数として使う。
class Psyque::RulesEngine::TExpressionBuilder
{
	private: using ThisClass = TExpressionBuilder; ///< @copydoc TDriver::ThisClass

	//-------------------------------------------------------------------------
	/// @brief 中間表現を解析して条件式を構築し、条件評価器に登録する。
	/// @return 登録した条件式の数。
	public: template<
		typename TemplateEvaluator,
		typename TemplateHasher,
		typename TemplateIntermediation>
	typename std::size_t operator()(
		/// [in,out] 構築した条件式を登録する TDriver::FEvaluator インスタンス。
		TemplateEvaluator& OutEvaluator,
		/// [in] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 条件式を登録するチャンクの識別値。
		typename TemplateEvaluator::FChunkKey const InChunkKey,
		/// [in] 条件式が参照する TDriver::FReservoir インスタンス。
		typename TemplateEvaluator::FReservoir const& InReservoir,
		/// [in] 解析する中間表現。
		TemplateIntermediation const& InIntermediation)
	const
	{
		return ThisClass::RegisterExpressions(
			OutEvaluator,
			InHashFunction,
			InChunkKey,
			InReservoir,
			InIntermediation);
	}

	/// @brief UDataTable を解析して条件式を構築し、条件評価器に登録する。
	/// @return 登録した条件式の数。
	public: template<typename TemplateEvaluator, typename TemplateHasher>
	static typename std::size_t RegisterExpressions(
		/// [in,out] 構築した条件式を登録する TDriver::FEvaluator インスタンス。
		TemplateEvaluator& OutEvaluator,
		/// [in] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 条件式を登録するチャンクの名前ハッシュ値。
		typename TemplateEvaluator::FChunkKey const InChunkKey,
		/// [in] 条件式が参照する TDriver::FReservoir インスタンス。
		typename TemplateEvaluator::FReservoir const& InReservoir,
		/// [in] 登録する状態値のもととなる
		/// FPsyqueRulesStatusTableRow で構成される UDataTable 。
		UDataTable const& InExpressionTable)
	{
		FString const LocalContextName(
			TEXT("PsyqueRulesPlugin/ExpressionBuilder::RegisterExpressions"));
		auto const LocalRowNames(InExpressionTable.GetRowNames());
		std::size_t LocalCount(0);
		TArray<typename TemplateEvaluator::FChunk::FStatusComparisonArray::value_type>
			LocalStatusComparisons;
		TArray<typename TemplateEvaluator::FChunk::FStatusTransitionArray::value_type>
			LocalStatusTransitions;
		TArray<typename TemplateEvaluator::FChunk::FSubExpressionArray::value_type>
			LocalSubExpressions;
		for (auto& LocalExpressionName: LocalRowNames)
		{
			auto const LocalExpression(
				InExpressionTable.FindRow<FPsyqueRulesExpressionTableRow>(
					LocalExpressionName, LocalContextName));
			if (LocalExpression == nullptr)
			{
				continue;
			}
			auto const LocalExpressionKey(InHashFunction(LocalExpressionName));
			switch (LocalExpression->Kind)
			{
				case EPsyqueRulesExpressionKind::StatusComparison:
				LocalCount += ThisClass::RegisterExpression(
					LocalStatusComparisons,
					OutEvaluator,
					InHashFunction,
					InChunkKey,
					InReservoir,
					LocalExpressionKey,
					*LocalExpression);
				break;

				case EPsyqueRulesExpressionKind::StatusTransition:
				LocalCount += ThisClass::RegisterExpression(
					LocalStatusTransitions,
					OutEvaluator,
					InHashFunction,
					InChunkKey,
					InReservoir,
					LocalExpressionKey,
					*LocalExpression);
				break;

				case EPsyqueRulesExpressionKind::SubExpression:
				LocalCount += ThisClass::RegisterExpression(
					LocalSubExpressions,
					OutEvaluator,
					InHashFunction,
					InChunkKey,
					OutEvaluator,
					LocalExpressionKey,
					*LocalExpression);
				break;
			}
		}
		return LocalCount;
	}

	/// @brief JSONを解析して条件式を構築し、条件評価器に登録する。
	/// @return 登録した条件式の数。
	public: template<typename TemplateEvaluator, typename TemplateHasher>
	static typename std::size_t RegisterExpressions(
		/// [in,out] 構築した条件式を登録する TDriver::FEvaluator インスタンス。
		TemplateEvaluator& OutEvaluator,
		/// [in] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 条件式を登録するチャンクの識別値。
		typename TemplateEvaluator::FChunkKey const InChunkKey,
		/// [in] 登録する条件式が参照する TDriver::FReservoir インスタンス。
		typename TemplateEvaluator::FReservoir const& InReservoir,
		/// [in] 登録する条件式が記述されているJSON値の配列。
		TArray<TSharedPtr<FJsonValue>> const& InJsonArray)
	{
		std::size_t LocalCount(0);
		for (auto const& LocalJsonValue: InJsonArray)
		{
			// 下位要素が要素数4以上の配列か判定する。
			auto const LocalRow(LocalJsonValue.Get());
			if (LocalRow == nullptr || LocalRow->Type != EJson::Array)
			{
				check(LocalRow != nullptr);
				continue;
			}
			auto const& LocalColumns(LocalRow->AsArray());
			if (LocalColumns.Num() < 4)
			{
				continue;
			}
			/// @todo 未実装
			check(false);
		}
		return LocalCount;
	}

	//-------------------------------------------------------------------------
	/// @brief データテーブル行を解析して条件式を構築し、条件評価器へ登録する。
	private: template<
		typename TemplateTermArray,
		typename TemplateEvaluator,
		typename TemplateHasher,
		typename TemplateDictionary>
	static bool RegisterExpression(
		TemplateTermArray& OutWorkspaceTerms,
		/// [in,out] 構築した条件式を登録する TDriver::FEvaluator インスタンス。
		TemplateEvaluator& OutEvaluator,
		/// [in] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 条件式を登録するチャンクの名前ハッシュ値。
		typename TemplateEvaluator::FChunkKey const InChunkKey,
		/// [in] 条件式が参照するインスタンス。
		TemplateDictionary const& InDictionary,
		/// [in] 登録する条件式の名前ハッシュ値。
		typename TemplateEvaluator::FExpressionKey const InExpressionKey,
		/// [in] 登録する条件式のもととなるデータテーブル行。
		FPsyqueRulesExpressionTableRow const& InExpression)
	{
		OutWorkspaceTerms.Reset(0);
		int32 LocalElementIndex(0);
		for (;;)
		{
			LocalElementIndex = ThisClass::BuildTerm(
				OutWorkspaceTerms,
				InHashFunction,
				InDictionary,
				InExpression.Elements,
				LocalElementIndex);
			if (LocalElementIndex <= 0)
			{
				return false;
			}
			if (InExpression.Elements.Num() <= LocalElementIndex)
			{
				return 0 < OutWorkspaceTerms.Num()
					&& OutEvaluator.RegisterExpression(
						InChunkKey,
						InExpressionKey,
						InExpression.Logic,
						&OutWorkspaceTerms[0],
						&OutWorkspaceTerms[0] + OutWorkspaceTerms.Num());
			}
		}
	}

	/// @brief 文字列をもとに、状態比較条件式の論理項を構築する。
	private: template<typename TemplateHasher, typename TemplateReservoir>
	static int32 BuildTerm(
		TArray<typename TemplateReservoir::FStatusComparison>&
			OutStatusComparisons,
		/// [in] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 条件式が参照する TDriver::FReservoir インスタンス。
		TemplateReservoir const& InReservoir,
		/// [in] 状態比較条件式の論理項要素のもととなる文字列の配列。
		TArray<FString> const& InElements,
		/// [in] 状態比較条件式の論理項要素として解析する文字列配列のインデクス番号。
		int32 const InElementIndex)
	{
		auto const LocalLastIndex(InElementIndex + 3);
		if (InElements.Num() < LocalLastIndex)
		{
			return 0;
		}
		/// @todo 未実装
		return LocalLastIndex;
	}

	/// @brief 文字列をもとに、状態変化条件式の論理項を構築する。
	private: template<typename TemplateHasher, typename TemplateReservoir>
	static int32 BuildTerm(
		TArray<
			_private::TStatusTransition<
				typename TemplateReservoir::FStatusKey>>&
					OutStatusTransitions,
		/// [in] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 条件式が参照する TDriver::FReservoir インスタンス。
		TemplateReservoir const& InReservoir,
		/// [in] 状態変化条件式の論理項要素のもととなる文字列の配列。
		TArray<FString> const& InElements,
		/// [in] 状態変化条件式の論理項要素として解析する文字列配列のインデクス番号。
		int32 const InElementIndex)
	{
		auto const LocalLastIndex(InElementIndex + 1);
		if (LocalLastIndex <= InElements.Num())
		{
			// 状態変化を検知する状態値が状態貯蔵器に登録されていれば、
			// 状態変化条件式の論理項を構築する。
			auto const LocalStatusKey(
				InHashFunction(FName(*InElements[InElementIndex])));
			if (0 < InReservoir.FindBitWidth(LocalStatusKey))
			{
				OutStatusTransitions.Emplace(LocalStatusKey);
				return LocalLastIndex;
			}
		}
		return 0;
	}

	/// @brief 文字列をもとに、複合条件式の論理項を構築する。
	private: template<typename TemplateHasher, typename TemplateEvaluator>
	static int32 BuildTerm(
		TArray<
			_private::TSubExpression<
				typename TemplateEvaluator::FExpressionKey>>&
					OutSubExpressions,
		/// [in] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 複合条件式が参照する TDriver::FEvaluator インスタンス。
		TemplateEvaluator const& InEvaluator,
		/// [in] 複合条件式の論理項要素のもととなる文字列の配列。
		TArray<FString> const& InElements,
		/// [in] 複合条件式の論理項要素として解析する文字列配列のインデクス番号。
		int32 const InElementIndex)
	{
		auto const LocalLastIndex(InElementIndex + 2);
		if (LocalLastIndex <= InElements.Num())
		{
			// 参照する条件式を結合する条件を取得する。
			auto const LocalCondition(
				Psyque::ParseKleene(InElements[InElementIndex + 1]));
			if (LocalCondition != EPsyqueKleene::TernaryUnknown)
			{
				// 結合する条件式が条件判定器に登録されていれば、
				// 複合条件式の論理項を構築する。
				auto const LocalExpressionKey(
					InHashFunction(FName(*InElements[InElementIndex])));
				if (InEvaluator.FindExpression(LocalExpressionKey) != nullptr)
				{
					OutSubExpressions.Emplace(
						LocalExpressionKey,
						LocalCondition != EPsyqueKleene::TernaryFalse);
					return LocalLastIndex;
				}
			}
		}
		return 0;
	}

}; // class Psyque::RulesEngine::TExpressionBuilder

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 文字列表から条件式を構築して登録する関数オブジェクト。
/// @details Psyque::RulesEngine::TDriver::ExtendChunk の引数として使う。
class Psyque::RulesEngine::TExpressionBuilder_
{
	private: using ThisClass = TExpressionBuilder_; ///< @copydoc TDriver::ThisClass

	//-------------------------------------------------------------------------
	/// @brief 条件式の文字列表の属性。
	private: template<typename TemplateRelationTable>
	class FTableAttribute
	{
		public: explicit FTableAttribute(
			TemplateRelationTable const& InTable)
		PSYQUE_NOEXCEPT:
		Key(
			InTable.FindAttribute(
				PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY)),
		Logic(
			InTable.FindAttribute(
				PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC)),
		Kind(
			InTable.FindAttribute(
				PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND)),
		Elements(
			InTable.FindAttribute(
				PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT))
		{}

		public: bool IsValid() const PSYQUE_NOEXCEPT
		{
			return 1 <= this->Key.second
				&& 1 <= this->Logic.second
				&& 1 <= this->Kind.second
				&& 1 <= this->Elements.second;
		}

		/// @brief 文字列表で条件式の識別値が記述されている属性の列番号と列数。
		public: typename TemplateRelationTable::FAttribute Key;
		/// @brief 文字列表で条件式の論理演算子が記述されている属性の列番号と列数。
		public: typename TemplateRelationTable::FAttribute Logic;
		/// @brief 文字列表で条件式の種別が記述されている属性の列番号と列数。
		public: typename TemplateRelationTable::FAttribute Kind;
		/// @brief 文字列表で条件式の論理項要素が記述されている属性の列番号と列数。
		public: typename TemplateRelationTable::FAttribute Elements;

	}; // class FTableAttribute

	//-------------------------------------------------------------------------
	/// @brief 中間表現を解析して条件式を構築し、条件評価器に登録する。
	/// @return 登録した条件式の数。
	public: template<
		typename TemplateEvaluator,
		typename TemplateHasher,
		typename TemplateIntermediation>
	typename std::size_t operator()(
		/// [in,out] 構築した条件式を登録する TDriver::FEvaluator インスタンス。
		TemplateEvaluator& OutEvaluator,
		/// [in] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 条件式を登録するチャンクの識別値。
		typename TemplateEvaluator::FChunkKey const& InChunkKey,
		/// [in] 条件式が参照する TDriver::FReservoir インスタンス。
		typename TemplateEvaluator::FReservoir const& InReservoir,
		/// [in] 解析する中間表現。
		TemplateIntermediation const& InIntermediation)
	const
	{
		return ThisClass::RegisterExpressions(
			OutEvaluator,
			InHashFunction,
			InChunkKey,
			InReservoir,
			InIntermediation);
	}

	/// @brief 文字列表を解析して条件式を構築し、条件評価器に登録する。
	/// @return 登録した条件式の数。
	public: template<
		typename TemplateEvaluator,
		typename TemplateHasher,
		typename TemplateRelationTable>
	static typename std::size_t RegisterExpressions(
		/// [in,out] 文字列表から構築した条件式を登録する TDriver::FEvaluator 。
		TemplateEvaluator& OutEvaluator,
		/// [in,out] 文字列からハッシュ値を作る TDriver::FHasher 。
		TemplateHasher const& InHashFunction,
		/// [in] 条件式を登録する論理項要素チャンクの識別値。
		typename TemplateEvaluator::FChunkKey const& InChunkKey,
		/// [in] 条件式が参照する TDriver::FReservoir 。
		typename TemplateEvaluator::FReservoir const& InReservoir,
		/// [in] 条件式が記述されている Psyque::string::TRelationTable 。
		/// 空の場合は、条件式は登録されない。
		TemplateRelationTable const& InTable)
	{
		// 文字列表の属性を取得する。
		ThisClass::FTableAttribute<TemplateRelationTable> const
			LocalAttribute(InTable);
		if (!LocalAttribute.IsValid())
		{
			PSYQUE_ASSERT(InTable.GetCells().empty());
			return 0;
		}

		// 作業領域を用意する。
		typename TemplateEvaluator::FChunk local_workspace(
			OutEvaluator.get_allocator());
		local_workspace.SubExpressions.reserve(
			LocalAttribute.Elements.second);
		local_workspace.StatusTransitions.reserve(
			LocalAttribute.Elements.second);
		local_workspace.StatusComparisons.reserve(
			LocalAttribute.Elements.second);

		// 文字列表を行ごとに解析し、条件式を構築して、条件評価器へ登録する。
		auto const local_empty_key(
			InHashFunction(typename TemplateHasher::argument_type()));
		auto const LocalRowCount(InTable.GetRowCount());
		decltype(InTable.GetRowCount()) local_count(0);
		for (
			typename TemplateRelationTable::FNumber i(0);
			i < LocalRowCount;
			++i)
		{
			if (i == InTable.GetAttributeRow())
			{
				continue;
			}
			auto const LocalExpressionKey(
				InHashFunction(InTable.FindCell(i, LocalAttribute.Key.first)));
			if (LocalExpressionKey != local_empty_key
				&& !OutEvaluator.IsRegistered(LocalExpressionKey)
				&& ThisClass::RegisterExpression(
					OutEvaluator,
					InHashFunction,
					local_workspace,
					InChunkKey,
					LocalExpressionKey,
					InReservoir,
					InTable,
					i,
					LocalAttribute.Logic,
					LocalAttribute.Kind,
					LocalAttribute.Elements))
			{
				++local_count;
			}
			else
			{
				// 条件式の識別値が空だったか、重複していた。
				PSYQUE_ASSERT(false);
			}
		}
		return local_count;
	}

	/// @brief 文字列表の行を解析して条件式を構築し、条件評価器へ登録する。
	/// @retval true  成功。条件式を OutEvaluator に登録した。
	/// @retval false 失敗。条件式は登録されなかった。
	/// - InExpressionKey に対応する条件式が既にあると失敗する。
	/// - InElements が空だと失敗する。
	public: template<
		typename TemplateEvaluator,
		typename TemplateHasher,
		typename TemplateRelationTable>
	static bool RegisterExpression(
		/// [in,out] 文字列表から構築した条件式を登録する TDriver::FEvaluator 。
		TemplateEvaluator& OutEvaluator,
		/// [in,out] 文字列からハッシュ値を生成する TDriver::FHasher 。
		TemplateHasher const& InHashFunction,
		/// [in,out] 作業領域として使う TDriver::FEvaluator::FChunk 。
		typename TemplateEvaluator::FChunk& io_workspace,
		/// [in] 条件式を登録する論理項要素チャンクの識別値。
		typename TemplateEvaluator::FChunkKey const& InChunkKey,
		/// [in] 登録する条件式の識別値。
		typename TemplateEvaluator::FExpressionKey const& InExpressionKey,
		/// [in] 条件式が参照する TDriver::FReservoir 。
		typename TemplateEvaluator::FReservoir const& InReservoir,
		/// [in] 解析する Psyque::string::TRelationTable 。
		TemplateRelationTable const& InTable,
		/// [in] 解析する InTable の行番号。
		typename TemplateRelationTable::FNumber const InRowNumber,
		/// [in] InTable で条件式の論理演算子が記述されている属性の列番号と列数。
		typename TemplateRelationTable::FAttribute const& InLogic,
		/// [in] InTable で条件式の種別が記述されている属性の列番号と列数。
		typename TemplateRelationTable::FAttribute const& InKind,
		/// [in] InTable で条件式の論理項要素が記述されている属性の列番号と列数。
		typename TemplateRelationTable::FAttribute const& InElements)
	{
		if (InLogic.second < 1 || InKind.second < 1 || InElements.second < 1)
		{
			return nullptr;
		}

		// 論理項要素の論理演算子を、文字列表から取得する。
		auto const& local_logic_cell(
			InTable.FindCell(InRowNumber, InLogic.first));
		EPsyqueRulesExpressionLogic local_logic;
		if (local_logic_cell ==
				PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LOGIC_AND)
		{
			local_logic = EPsyqueRulesExpressionLogic::And;
		}
		else if (
			local_logic_cell ==
				PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LOGIC_OR)
		{
			local_logic = EPsyqueRulesExpressionLogic::Or;
		}
		else
		{
			// 未知の論理演算子だった。
			PSYQUE_ASSERT(false);
			return nullptr;
		}

		// 条件式の種別を文字列表から取得し、
		// 種別ごとに条件式の論理項要素を構築して登録する。
		auto const& LocalKindCell(
			InTable.FindCell(InRowNumber, InKind.first));
		auto const local_elements_end(InElements.first + InElements.second);
		if (LocalKindCell ==
				PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION)
		{
			// 複合条件式の論理項要素を構築して登録する。
			return ThisClass::RegisterExpression(
				OutEvaluator,
				InHashFunction,
				io_workspace.SubExpressions,
				InChunkKey,
				InExpressionKey,
				local_logic,
				OutEvaluator,
				InTable,
				InRowNumber,
				InElements.first,
				local_elements_end);
		}
		else if (
			LocalKindCell ==
				PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_TRANSITION)
		{
			// 状態変化条件式の論理項要素を構築して登録する。
			return ThisClass::RegisterExpression(
				OutEvaluator,
				InHashFunction,
				io_workspace.StatusTransitions,
				InChunkKey,
				InExpressionKey,
				local_logic,
				InReservoir,
				InTable,
				InRowNumber,
				InElements.first,
				local_elements_end);
		}
		else if (
			LocalKindCell ==
				PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_COMPARISON)
		{
			// 状態比較条件式の論理項要素を構築して登録する。
			return ThisClass::RegisterExpression(
				OutEvaluator,
				InHashFunction,
				io_workspace.StatusComparisons,
				InChunkKey,
				InExpressionKey,
				local_logic,
				InReservoir,
				InTable,
				InRowNumber,
				InElements.first,
				local_elements_end);
		}
		else
		{
			// 未知の条件式種別だった。
			PSYQUE_ASSERT(false);
			return nullptr;
		}
	}

	//-------------------------------------------------------------------------
	/// @copydoc ThisClass::RegisterExpression
	private: template<
		typename TemplateEvaluator,
		typename template_element_container,
		typename TemplateHasher,
		typename template_element_server,
		typename TemplateRelationTable>
	static bool RegisterExpression(
		/// [in,out] 条件式を登録する TDriver::FEvaluator 。
		TemplateEvaluator& OutEvaluator,
		/// [in,out] 文字列からハッシュ値を生成する TDriver::FHasher 。
		TemplateHasher const& InHashFunction,
		/// [in,out] 条件式の論理項要素を構築する作業領域として使うコンテナ。
		template_element_container& OutElements,
		/// [in] 条件式を登録する論理項要素チャンクの識別値。
		typename TemplateEvaluator::FChunkKey const& InChunkKey,
		/// [in] 登録する条件式の識別値。
		typename TemplateEvaluator::FExpressionKey const& InExpressionKey,
		/// [in] 登録する条件式の EPsyqueRulesExpressionLogic 。
		EPsyqueRulesExpressionLogic const InLogic,
		/// [in] 論理項要素が参照する値。
		template_element_server const& InElements,
		/// [in] 解析する Psyque::string::TRelationTable 。
		TemplateRelationTable const& InTable,
		/// [in] 解析する InTable の行番号。
		typename TemplateRelationTable::FNumber const InRowNumber,
		/// [in] InTable で条件式の論理項要素が記述されている属性の先頭の列番号。
		typename TemplateRelationTable::FNumber const InColumnBegin,
		/// [in] InTable で条件式の論理項要素が記述されている属性の末尾の列番号。
		typename TemplateRelationTable::FNumber const InColumnEnd)
	{
		// 論理項要素のコンテナを構築し、条件式を条件評価器へ登録する。
		OutElements.clear();
		for (
			auto i(InColumnBegin);
			i < InColumnEnd;
			i += ThisClass::BuildElement<TemplateEvaluator>(
				OutElements,
				InHashFunction,
				InElements,
				InTable,
				InRowNumber,
				i));
		return OutEvaluator.RegisterExpression(
			InChunkKey, InExpressionKey, InLogic, OutElements);
	}

	/// @brief 文字列表を解析し、複合条件式の論理項要素を構築する。
	/// @return 解析した列の数。
	private: template<
		typename TemplateEvaluator,
		typename TemplateHasher,
		typename TemplateRelationTable>
	static typename TemplateRelationTable::FNumber BuildElement(
		/// [in,out] 構築した論理項要素を追加する
		/// TDriver::FEvaluator::FChunk::FSubExpressionArray 。
		typename TemplateEvaluator::FChunk::FSubExpressionArray& OutElements,
		/// [in,out] 文字列からハッシュ値を生成する TDriver::FHasher 。
		TemplateHasher const& InHashFunction,
		/// [in] 複合条件式を追加する TDriver::FEvaluator 。
		TemplateEvaluator const& InEvaluator,
		/// [in] 解析する Psyque::string::TRelationTable 。
		TemplateRelationTable const& InTable,
		/// [in] 解析する InTable の行番号。
		typename TemplateRelationTable::FNumber const InRowNumber,
		/// [in] InTable で条件式の論理項要素が記述されている属性の列番号。
		typename TemplateRelationTable::FNumber const InColumnNumber)
	{
		// 複合条件式の下位条件式の識別値を取得する。
		auto const& local_sub_key_cell(
			InTable.FindCell(InRowNumber, InColumnNumber));
		auto const local_sub_key(InHashFunction(local_sub_key_cell));
		if (local_sub_key != InHashFunction(typename TemplateHasher::argument_type()))
		{
			/// @note 無限ループを防ぐため、複合条件式で使う下位条件式は、
			/// 条件評価器で定義済みのものしか使わないようにする。
			PSYQUE_ASSERT(InEvaluator.IsRegistered(local_sub_key));

			// 複合条件式の条件を取得する。
			auto const& local_condition_cell(
				InTable.FindCell(InRowNumber, InColumnNumber + 1));
			Psyque::string::numeric_parser<bool> const local_condition_parser(
				local_condition_cell);
			if (local_condition_parser.IsCompleted())
			{
				// 複合条件式に論理項要素を追加する。
				OutElements.emplace_back(
					local_sub_key, local_condition_parser.GetValue());
			}
			else
			{
				PSYQUE_ASSERT(false);
			}
		}
		else
		{
			PSYQUE_ASSERT(local_sub_key_cell.empty());
		}
		return 2;
	}

	/// @brief 文字列表を解析し、状態変化条件式の論理項要素を構築する。
	/// @return 解析した列の数。
	private: template<
		typename TemplateEvaluator,
		typename TemplateHasher,
		typename TemplateRelationTable>
	static typename TemplateRelationTable::FNumber BuildElement(
		/// [in,out] 構築した論理項要素を追加する
		/// TDriver::FEvaluator::FChunk::FStatusTransitionArray 。
		typename TemplateEvaluator::FChunk::FStatusTransitionArray&
			OutElements,
		/// [in,out] 文字列からハッシュ値を生成する TDriver::FHasher 。
		TemplateHasher const& InHashFunction,
		/// [in] 条件式が参照する TDriver::FReservoir 。
		typename TemplateEvaluator::FReservoir const& InReservoir,
		/// [in] 解析する Psyque::string::TRelationTable 。
		TemplateRelationTable const& InTable,
		/// [in] 解析する InTable の行番号。
		typename TemplateRelationTable::FNumber const InRowNumber,
		/// [in] InTable で条件式の論理項要素が記述されている属性の列番号。
		typename TemplateRelationTable::FNumber const InColumnNumber)
	{
		// 状態値の識別値を取得する。
		auto const& local_status_key_cell(
			InTable.FindCell(InRowNumber, InColumnNumber));
		auto const LocalStatusKey(InHashFunction(local_status_key_cell));
		if (LocalStatusKey !=
				InHashFunction(typename TemplateHasher::argument_type()))
		{
			// 状態変化条件式に論理項要素を追加する。
			OutElements.push_back(LocalStatusKey);
		}
		else
		{
			PSYQUE_ASSERT(local_status_key_cell.empty());
		}
		return 1;
	}

	/// @brief 文字列表を解析し、状態比較条件式の論理項要素を構築する。
	/// @return 解析した列の数。
	private: template<
		typename TemplateEvaluator,
		typename TemplateHasher,
		typename TemplateRelationTable>
	static typename TemplateRelationTable::FNumber BuildElement(
		/// [in,out] 構築した論理項要素を追加する
		/// TDriver::FEvaluator::FChunk::FStatusComparisonArray 。
		typename TemplateEvaluator::FChunk::FStatusComparisonArray& OutElements,
		/// [in,out] 文字列からハッシュ値を生成する TDriver::FHasher 。
		TemplateHasher const& InHashFunction,
		/// [in] 条件式が参照する TDriver::FReservoir 。
		typename TemplateEvaluator::FReservoir const& InReservoir,
		/// [in] 解析する Psyque::string::TRelationTable 。
		TemplateRelationTable const& InTable,
		/// [in] 解析する InTable の行番号。
		typename TemplateRelationTable::FNumber const InRowNumber,
		/// [in] InTable で条件式の論理項要素が記述されている属性の列番号。
		typename TemplateRelationTable::FNumber const InColumnNumber)
	{
		auto const LocalComparison(
			TemplateEvaluator::FChunk::FStatusComparisonArray::value_type
			::_build(InHashFunction, InTable, InRowNumber, InColumnNumber));
		if (!LocalComparison.GetValue().IsEmpty())
		{
			OutElements.push_back(LocalComparison);
		}
		return 3;
	}

}; // class Psyque::RulesEngine::TExpressionBuilder

// vim: set noexpandtab:
