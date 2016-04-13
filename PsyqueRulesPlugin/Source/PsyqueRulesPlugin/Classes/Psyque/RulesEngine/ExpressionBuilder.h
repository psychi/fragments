/// @file
/// @brief @copybrief Psyque::RulesEngine::TExpressionBuilder
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include "../Assert.h"

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

#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_EQUAL
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_EQUAL "=="
#endif // !define(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_EQUAL)

#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_NOT_EQUAL
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_NOT_EQUAL "!="
#endif // !define(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_NOT_EQUAL)

#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LESS
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LESS "<"
#endif // !define(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LESS)

#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LESS_EQUAL
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LESS_EQUAL "<="
#endif // !define(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LESS_EQUAL)

#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_GREATER
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_GREATER ">"
#endif // !define(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_GREATER)

#ifndef PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_GREATER_EQUAL
#define PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_GREATER_EQUAL ">="
#endif // !define(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_GREATER_EQUAL)

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
/// @brief UDataTable から条件式を構築して登録する関数オブジェクト。
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
	typename uint32 operator()(
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
	static typename uint32 RegisterExpressions(
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
		uint32 LocalCount(0);
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
	/// @todo 実装途中
	public: template<typename TemplateEvaluator, typename TemplateHasher>
	static uint32 RegisterExpressions(
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
		uint32 LocalCount(0);
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
		/// [out] 構築した条件式の論理項を格納する配列。
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
		/// [out] 構築した状態比較条件式の論理項を格納する配列。
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
		int32 const LocalParseCount(3);
		auto const LocalLastIndex(InElementIndex + LocalParseCount);
		if (InElements.Num() < LocalLastIndex)
		{
			if (InElements.Num() <= 0 || InElements.Num() % LocalParseCount != 0)
			{
				UE_LOG(
					LogPsyqueRulesEngine,
					Warning,
					TEXT(
						"TExpressionBuilder::BuildTerm is failed."
						"\n\tTStatusComparison elements is not enough."),
					*InElements[InElementIndex + 1]);
			}
			return 0;
		}

		// 比較演算子を取得する。
		Psyque::RulesEngine::EStatusComparison LocalComparison;
		Psyque::String::FView const LocalComparisonString(InElements[InElementIndex + 1]);
		if (LocalComparisonString == TEXT(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_EQUAL))
		{
			LocalComparison = Psyque::RulesEngine::EStatusComparison::Equal;
		}
		else if (LocalComparisonString == TEXT(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_NOT_EQUAL))
		{
			LocalComparison = Psyque::RulesEngine::EStatusComparison::NotEqual;
		}
		else if (LocalComparisonString == TEXT(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LESS))
		{
			LocalComparison = Psyque::RulesEngine::EStatusComparison::Less;
		}
		else if (LocalComparisonString == TEXT(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_LESS_EQUAL))
		{
			LocalComparison = Psyque::RulesEngine::EStatusComparison::LessEqual;
		}
		else if (LocalComparisonString == TEXT(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_GREATER))
		{
			LocalComparison = Psyque::RulesEngine::EStatusComparison::Greater;
		}
		else if (LocalComparisonString == TEXT(PSYQUE_RULES_ENGINE_EXPRESSION_BUILDER_GREATER_EQUAL))
		{
			LocalComparison = Psyque::RulesEngine::EStatusComparison::GreaterEqual;
		}
		else
		{
			UE_LOG(
				LogPsyqueRulesEngine,
				Warning,
				TEXT(
					"TExpressionBuilder::BuildTerm is failed."
					"\n\t'%s' is not Psyque::RulesEngine::EStatusComparison."),
				*LocalComparisonString);
			return 0;
		}

		// 比較の右辺を取得する。
		Psyque::String::FView const LocalRightString(
			InElements[InElementIndex + 2]);
		auto const LocalRightValue(
			ThisClass::Parse<typename TemplateReservoir::FStatusValue>(
				LocalRightString));
		typename TemplateReservoir::FStatusKey LocalRightKey(0);
		if (LocalRightValue.IsEmpty())
		{
			// 右辺となる状態値の名前ハッシュ値を取得する。
			LocalRightKey = InHashFunction(*LocalRightString);
			if (InReservoir.FindBitFormat(LocalRightKey) == 0)
			{
				UE_LOG(
					LogPsyqueRulesEngine,
					Warning,
					TEXT(
						"TExpressionBuilder::BuildTerm is failed."
						"\n\tRight status key '%s' is not registered"
						" in InReservoir."),
					*LocalRightString);
				return 0;
			}
		}

		// 比較の左辺となる状態値の名前を取得する。
		auto const LocalLeftKey(
			InHashFunction(FName(*InElements[InElementIndex])));
		if (InReservoir.FindBitFormat(LocalLeftKey) == 0)
		{
			UE_LOG(
				LogPsyqueRulesEngine,
				Warning,
				TEXT(
					"TExpressionBuilder::BuildTerm is failed."
					"\n\tLeft status key '%s' is not registered in InReservoir."),
				*InElements[InElementIndex]);
			return 0;
		}

		/// 状態比較条件式の論理項を構築する。
		if (LocalRightValue.IsEmpty())
		{
			OutStatusComparisons.Emplace(
				LocalLeftKey, LocalComparison, LocalRightKey);
		}
		else
		{
			OutStatusComparisons.Emplace(
				LocalLeftKey, LocalComparison, LocalRightValue);
		}
		return LocalLastIndex;
	}

	/// @brief 文字列をもとに、状態変化条件式の論理項を構築する。
	private: template<typename TemplateHasher, typename TemplateReservoir>
	static int32 BuildTerm(
		/// [out] 構築した状態変化条件式の論理項を格納する配列。
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
		if (InElements.Num() < LocalLastIndex)
		{
			if (InElements.Num() <= 0)
			{
				UE_LOG(
					LogPsyqueRulesEngine,
					Warning,
					TEXT(
						"TExpressionBuilder::BuildTerm is failed."
						"\n\tTStatusComparison elements is not enough."),
					*InElements[InElementIndex + 1]);
			}
			return 0;
		}

		// 状態変化を検知する状態値が状態貯蔵器に登録されていれば、
		// 状態変化条件式の論理項を構築する。
		auto const LocalStatusKey(
			InHashFunction(FName(*InElements[InElementIndex])));
		if (InReservoir.FindBitFormat(LocalStatusKey) == 0)
		{
			UE_LOG(
				LogPsyqueRulesEngine,
				Warning,
				TEXT(
					"TExpressionBuilder::BuildTerm is failed."
					"\n\tStatus key '%s' is not registered in InReservoir."),
				*InElements[InElementIndex]);
			return 0;
		}
		OutStatusTransitions.Emplace(LocalStatusKey);
		return LocalLastIndex;
	}

	/// @brief 文字列をもとに、複合条件式の論理項を構築する。
	private: template<typename TemplateHasher, typename TemplateEvaluator>
	static int32 BuildTerm(
		/// [out] 構築した複合条件式の論理項を格納する配列。
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
		int32 const LocalParseCount(2);
		auto const LocalLastIndex(InElementIndex + LocalParseCount);
		if (InElements.Num() < LocalLastIndex)
		{
			if (InElements.Num() <= 0 || InElements.Num() % LocalParseCount != 0)
			{
				UE_LOG(
					LogPsyqueRulesEngine,
					Warning,
					TEXT(
						"TExpressionBuilder::BuildTerm is failed."
						"\n\tTSubExpression elements is not enough."),
					*InElements[InElementIndex + 1]);
			}
			return 0;
		}

		// 参照する条件式を結合する条件を取得する。
		int32 LocalParseResult;
		auto const& LocalString(InElements[InElementIndex + 1]);
		auto const LocalCondition(
			Psyque::String::FNumericParser(&LocalParseResult, LocalString));
		auto const LocalBool(LocalCondition.GetBool());
		if (LocalParseResult != LocalString.Len() || LocalBool != nullptr)
		{
			UE_LOG(
				LogPsyqueRulesEngine,
				Warning,
				TEXT(
					"TExpressionBuilder::BuildTerm is failed."
					"\n\t'%s' is not bool."),
				*InElements[InElementIndex + 1]);
			return 0;
		}

		// 結合する条件式が条件判定器に登録されていれば、
		// 複合条件式の論理項を構築する。
		auto const LocalExpressionKey(
			InHashFunction(FName(*InElements[InElementIndex])));
		if (InEvaluator.FindExpression(LocalExpressionKey) == nullptr)
		{
			UE_LOG(
				LogPsyqueRulesEngine,
				Warning,
				TEXT(
					"TExpressionBuilder::BuildTerm is failed."
					"\n\tExpression key '%s' is not registered"
					" in InEvaluator."),
				*InElements[InElementIndex]);
			return 0;
		}
		OutSubExpressions.Emplace(LocalExpressionKey, *LocalBool);
		return LocalLastIndex;
	}

	private: template<typename TemplateStatusValue>
	static TemplateStatusValue Parse(
		Psyque::String::FView const& InString)
	{
		int32 LocalParseResult;
		Psyque::String::FNumericParser const LocalValue(
			&LocalParseResult, InString);
		if (LocalParseResult == InString.Len())
		{
			auto const LocalBool(LocalValue.GetBool());
			if (LocalBool != nullptr)
			{
				return TemplateStatusValue(*LocalBool);
			}
			auto const LocalUnsigned(LocalValue.GetUnsigned());
			if (LocalUnsigned != nullptr)
			{
				return TemplateStatusValue(*LocalUnsigned);
			}
			auto const LocalNegative(LocalValue.GetNegative());
			if (LocalNegative != nullptr)
			{
				return TemplateStatusValue(*LocalNegative);
			}
			auto const LocalFloat(LocalValue.GetFloat());
			if (LocalFloat != nullptr)
			{
				return TemplateStatusValue(
					static_cast<typename TemplateStatusValue::FFloat>(
						*LocalFloat));
			}
			check(false);
		}
		return TemplateStatusValue();
	}

}; // class Psyque::RulesEngine::TExpressionBuilder

// vim: set noexpandtab:
