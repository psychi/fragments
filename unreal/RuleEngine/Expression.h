// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TExpression
/// @author Hillco Psychi (https://twitter.com/psychi)

#include "Containers/Array.h"
#include "../Assert.h"

/// @cond
namespace Psyque
{
	namespace RuleEngine
	{
		namespace _private
		{
			template<typename, typename, typename> class TExpression;
			template<typename> class TSubExpression;
			template<typename> class TStatusTransition;
			template<typename, typename, typename, typename>
				class TExpressionChunk;
		} // namespace _private
	} // namespace RuleEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 条件式。
/// @tparam TemplateEvaluation   @copydoc TExpression::FEvaluation
/// @tparam TemplateChunkKey     @copydoc TExpression::FChunkKey
/// @tparam TemplateElementIndex @copydoc TExpression::FElementIndex
template<
	typename TemplateEvaluation,
	typename TemplateChunkKey,
	typename TemplateElementIndex>
class Psyque::RuleEngine::_private::TExpression
{
	using This = TExpression; ///< @copydoc TEvaluator::This

	//-------------------------------------------------------------------------
	public:
	/// @brief 条件式の評価結果。
	/// @details
	/// - 正なら、条件式の評価は真だった。
	/// - 0 なら、条件式の評価は偽だった。
	/// - 負なら、条件式の評価に失敗した。
	using FEvaluation = TemplateEvaluation;
	static_assert(
		std::is_signed<TemplateEvaluation>::value
		&& std::is_integral<TemplateEvaluation>::value,
		"TemplateEvaluation is not signed integer type.");
	/// @brief 要素条件チャンクの識別値を表す型。
	using FChunkKey = TemplateChunkKey;
	/// @brief 要素条件のインデクス番号を表す型。
	using FElementIndex = TemplateElementIndex;
	/// @brief 条件式の要素条件を結合する論理演算子を表す列挙型。
	enum class ELogic: uint8
	{
		OR,  ///< 論理和。
		AND, ///< 論理積。
	};
	/// @brief 条件式の種類を表す列挙型。
	enum class EKind: uint8
	{
		SUB_EXPRESSION,    ///< 複合条件式。
		STATUS_TRANSITION, ///< 状態変化条件式。
		STATUS_COMPARISON, ///< 状態比較条件式。
	};

	//-------------------------------------------------------------------------
	public:
	/// @brief 条件式を構築する。
	PSYQUE_CONSTEXPR TExpression(
		/// [in] This::ChunkKey の初期値。
		TemplateChunkKey InChunkKey,
		/// [in] This::Logic の初期値。
		typename This::ELogic const InLogic,
		/// [in] This::Kind の初期値。
		typename This::EKind const InKind,
		/// [in] This::ElementBegin の初期値。
		TemplateElementIndex const InElementBegin,
		/// [in] This::ElementEnd の初期値。
		TemplateElementIndex const InElementEnd)
	PSYQUE_NOEXCEPT:
	ChunkKey(MoveTemp(InChunkKey)),
	ElementBegin((
		PSYQUE_ASSERT(0 <= InElementBegin && InElementBegin <= InElementEnd),
		InElementBegin)),
	ElementEnd(InElementEnd),
	Logic(InLogic),
	Kind(InKind)
	{}

	/// @brief 空の条件式か判定する。
	/// @retval true  *this は空の条件式。
	/// @retval false *this は空の条件式ではない。
	bool IsEmpty() const PSYQUE_NOEXCEPT
	{
		return this->GetElementBegin() == this->GetElementEnd();
	}

	/// @brief 条件式が格納されている要素条件チャンクの識別値を取得する。
	/// @return @copydoc This::ChunkKey
	TemplateChunkKey GetChunkKey() const PSYQUE_NOEXCEPT
	{
		return this->ChunkKey;
	}

	/// @brief 条件式が使う要素条件チャンクの先頭インデクス番号を取得する。
	/// @return @copydoc This::ElementBegin
	TemplateElementIndex GetElementBegin() const PSYQUE_NOEXCEPT
	{
		return this->ElementBegin;
	}

	/// @brief 条件式が使う要素条件チャンクの末尾インデクス番号を取得する。
	/// @return @copydoc This::ElementEnd
	TemplateElementIndex GetElementEnd() const PSYQUE_NOEXCEPT
	{
		return this->ElementEnd;
	}

	/// @brief 条件式の種類を取得する。
	/// @return @copydoc This::Kind
	typename This::EKind GetKind() const PSYQUE_NOEXCEPT
	{
		return this->Kind;
	}

	/// @brief 条件式を評価する。
	/// @retval 正 条件式の評価は真。
	/// @retval 0  条件式の評価は偽。
	/// @retval 負 条件式の評価に失敗。
	template<
		typename TemplateElementContainer,
		typename TemplateElementEvaluator>
	typename This::FEvaluation Evaluate(
		/// [in] 評価に用いる要素条件のコンテナ。
		TemplateElementContainer const& InElements,
		/// [in] 要素条件を評価する関数オブジェクト。
		TemplateElementEvaluator const& InEvaluator)
	const PSYQUE_NOEXCEPT
	{
		if (this->IsEmpty()
			|| InElements.Num() <= this->GetElementBegin()
			|| InElements.Num() < this->GetElementEnd())
		{
			// 条件式が空か、範囲外の要素条件を参照している。
			check(this->IsEmpty());
			return -1;
		}
		auto const LocalBegin(&InElements[0]);
		auto const LocalEnd(LocalBegin + this->ElementEnd);
		auto const LocalAnd(this->Logic == This::ELogic::AND);
		for (auto i(LocalBegin + this->ElementBegin); i != LocalEnd; ++i)
		{
			auto const LocalEvaluation(InEvaluator(*i));
			if (LocalEvaluation < 0)
			{
				return -1;
			}
			else if (0 < LocalEvaluation)
			{
				if (!LocalAnd)
				{
					return 1;
				}
			}
			else if (LocalAnd)
			{
				return 0;
			}
		}
		return LocalAnd;
	}

	//-------------------------------------------------------------------------
	private:
	/// @brief 要素条件チャンクに対応する識別値。
	TemplateChunkKey ChunkKey;
	/// @brief 条件式が使う要素条件の先頭インデクス番号。
	TemplateElementIndex ElementBegin;
	/// @brief 条件式が使う要素条件の末尾インデクス番号。
	TemplateElementIndex ElementEnd;
	/// @brief 条件式の要素条件を結合する論理演算子。
	typename This::ELogic Logic;
	/// @brief 条件式の種類。
	typename This::EKind Kind;

}; // class Psyque::RuleEngine::_private::TExpression

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 複合条件式の要素条件。
/// @tparam TemplateExpressionKey @copydoc Psyque::RuleEngine::_private::TEvaluator::FExpressionKey
template<typename TemplateExpressionKey>
class Psyque::RuleEngine::_private::TSubExpression
{
	using This = TSubExpression; ///< @copydoc TExpression::This

	//-------------------------------------------------------------------------
	public:
	/// @brief 複合条件式の要素条件を構築する。
	PSYQUE_CONSTEXPR TSubExpression(
		/// [in] This::Key の初期値。
		TemplateExpressionKey InKey,
		/// [in] This::condition の初期値。
		bool const InCondition)
	PSYQUE_NOEXCEPT: Key(TempMove(InKey)), Condition(InCondition) {}

	TemplateExpressionKey GetKey() const PSYQUE_NOEXCEPT
	{
		return this->Key;
	}

	bool CompareCondition(bool const InCondition) const PSYQUE_NOEXCEPT
	{
		return InCondition == this->Condition;
	}

	//-------------------------------------------------------------------------
	private:
	TemplateExpressionKey Key; ///< 結合する条件式の識別値。
	bool Condition;            ///< 結合する際の条件。

}; // class Psyque::RuleEngine::_private::TSubExpression

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態変化条件式の要素条件。
/// @tparam TemplateStatusKey @copydoc Psyque::RuleEngine::_private::TReservoir::FStatusKey
template<typename TemplateStatusKey>
class Psyque::RuleEngine::_private::TStatusTransition
{
	using This = TStatusTransition; ///< @copydoc TExpression::This

	//-------------------------------------------------------------------------
	/// @brief 状態変化条件式の要素条件を構築する。
	explicit PSYQUE_CONSTEXPR TStatusTransition(
		/// [in] This::Key の初期値。
		TemplateStatusKey InKey)
	PSYQUE_NOEXCEPT: Key(TempMove(InKey))
	{}

	PSYQUE_CONSTEXPR TemplateStatusKey const GetKey() const PSYQUE_NOEXCEPT
	{
		return this->Key;
	}

	//-------------------------------------------------------------------------
	private:
	TemplateStatusKey Key; ///< 変化を検知する状態値の識別値。

}; // class Psyque::RuleEngine::_private::TStatusTransition

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 要素条件チャンク。
/// @tparam TemplateSubExpressionContainer    @copydoc TExpressionChunk::FSubExpressionContainer
/// @tparam TemplateStatusTransitionContainer @copydoc TExpressionChunk::FStatusTransitionContainer
/// @tparam TemplateStatusComparisonContainer @copydoc TExpressionChunk::FStatusComparisonContainer
template<
	typename TemplateExpressionKey,
	typename TemplateStatusKey,
	typename TemplateStatusComparison,
	typename TemplateHeapAllocator>
class Psyque::RuleEngine::_private::TExpressionChunk
{
	using This = TExpressionChunk; ///< @copydoc TExpression::This

	//-------------------------------------------------------------------------
	public:
	/// @brief 複合条件式の要素条件のコンテナの型。
	using FSubExpressionContainer = TArray<
		Psyque::RuleEngine::_private::TSubExpression<TemplateExpressionKey>,
		TemplateHeapAllocator>;
	/// @brief 状態変化条件式の要素条件のコンテナの型。
	using FStatusTransitionContainer = TArray<
		Psyque::RuleEngine::_private::TStatusTransition<TemplateStatusKey>,
		TemplateHeapAllocator>;
	/// @brief 状態比較条件式の要素条件のコンテナの型。
	using FStatusComparisonContainer = TArray<
		TemplateStatusComparison, TemplateHeapAllocator>;

	//-------------------------------------------------------------------------
	public:
	/// @brief 複合条件式で使う要素条件のコンテナ。
	typename This::FSubExpressionContainer SubExpressions;
	/// @brief 状態変化条件式で使う要素条件のコンテナ。
	typename This::FStatusTransitionContainer StatusTransitions;
	/// @brief 状態比較条件式で使う要素条件のコンテナ。
	typename This::FStatusComparisonContainer StatusComparisons;

}; // class Psyque::RuleEngine::_private::TExpressionChunk

// vim: set noexpandtab:
