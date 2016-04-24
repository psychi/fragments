// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::TExpression
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include "./Assert.h"
#include "./Enum.h"

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		namespace _private
		{
			template<typename, typename> class TExpression;
			template<typename> class TSubExpression;
			template<typename> class TStatusTransition;
			template<typename, typename, typename> class TExpressionChunk;
		} // namespace _private
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態値を参照する条件式。
/// @details TEvaluator::Expressions の要素として使われる。
/// @tparam TemplateChunkKey     @copydoc TExpression::FChunkKey
/// @tparam TemplateElementIndex @copydoc TExpression::FElementIndex
template<typename TemplateChunkKey, typename TemplateElementIndex>
class Psyque::RulesEngine::_private::TExpression
{
	private: using ThisClass = TExpression; ///< @copydoc TEvaluator::ThisClass

	//-------------------------------------------------------------------------
	/// @brief 論理項要素チャンクの識別値を表す型。
	public: using FChunkKey = TemplateChunkKey;
	/// @brief 論理項要素のインデクス番号を表す型。
	public: using FElementIndex = TemplateElementIndex;

	//-------------------------------------------------------------------------
	/// @brief 条件式を構築する。
	public: TExpression(
		/// [in] ThisClass::ChunkKey の初期値。
		typename ThisClass::FChunkKey InChunkKey,
		/// [in] ThisClass::Logic の初期値。
		EPsyqueRulesExpressionLogic const InLogic,
		/// [in] ThisClass::Kind の初期値。
		EPsyqueRulesExpressionKind const InKind,
		/// [in] ThisClass::BeginIndex の初期値。
		typename ThisClass::FElementIndex const InBeginIndex,
		/// [in] ThisClass::EndIndex の初期値。
		typename ThisClass::FElementIndex const InEndIndex)
	PSYQUE_NOEXCEPT:
	ChunkKey(MoveTemp(InChunkKey)),
	BeginIndex((PSYQUE_ASSERT(InBeginIndex <= InEndIndex), InBeginIndex)),
	EndIndex(InEndIndex),
	Logic(InLogic),
	Kind(InKind)
	{}

	/// @brief 空の条件式か判定する。
	/// @retval true  *this は空の条件式。
	/// @retval false *this は空の条件式ではない。
	public: bool IsEmpty() const PSYQUE_NOEXCEPT
	{
		return this->GetBeginIndex() == this->GetEndIndex();
	}

	/// @brief 条件式が格納されている論理項要素チャンクの識別値を取得する。
	/// @return @copydoc ThisClass::ChunkKey
	public: typename ThisClass::FChunkKey const GetChunkKey() const PSYQUE_NOEXCEPT
	{
		return this->ChunkKey;
	}

	/// @brief 条件式が使う論理項要素チャンクの先頭インデクス番号を取得する。
	/// @return @copydoc ThisClass::BeginIndex
	public: typename ThisClass::FElementIndex GetBeginIndex() const PSYQUE_NOEXCEPT
	{
		return this->BeginIndex;
	}

	/// @brief 条件式が使う論理項要素チャンクの末尾インデクス番号を取得する。
	/// @return @copydoc ThisClass::EndIndex
	public: typename ThisClass::FElementIndex GetEndIndex() const PSYQUE_NOEXCEPT
	{
		return this->EndIndex;
	}

	/// @brief 条件式の種類を取得する。
	/// @return @copydoc ThisClass::Kind
	public: EPsyqueRulesExpressionKind GetKind() const PSYQUE_NOEXCEPT
	{
		return this->Kind;
	}

	/// @brief 条件式を評価する。
	/// @retval 正 条件式の評価は真。
	/// @retval 0  条件式の評価は偽。
	/// @retval 負 条件式の評価に失敗。
	public: template<
		typename TemplateElementArray,
		typename TemplateElementEvaluator>
	EPsyqueKleene Evaluate(
		/// [in] 評価に用いる論理項要素のコンテナ。
		TemplateElementArray const& InElements,
		/// [in] 論理項要素を評価する関数オブジェクト。
		TemplateElementEvaluator const& InEvaluator)
	const PSYQUE_NOEXCEPT
	{
		if (this->IsEmpty()
			|| InElements.size() <= this->GetBeginIndex()
			|| InElements.size() < this->GetEndIndex())
		{
			// 条件式が空か、範囲外の論理項要素を参照している。
			check(this->IsEmpty());
			return EPsyqueKleene::Unknown;
		}
		auto const LocalEnd(InElements.begin() + this->EndIndex);
		auto const LocalAnd(this->Logic == EPsyqueRulesExpressionLogic::And);
		for (auto i(InElements.begin() + this->BeginIndex); i != LocalEnd; ++i)
		{
			auto const LocalEvaluation(InEvaluator(*i));
			if (LocalEvaluation == EPsyqueKleene::Unknown)
			{
				return EPsyqueKleene::Unknown;
			}
			else if (LocalEvaluation == EPsyqueKleene::IsTrue)
			{
				if (!LocalAnd)
				{
					return EPsyqueKleene::IsTrue;
				}
			}
			else if (LocalAnd)
			{
				return EPsyqueKleene::IsFalse;
			}
		}
		return static_cast<EPsyqueKleene>(LocalAnd);
	}

	//-------------------------------------------------------------------------
	/// @brief 論理項要素チャンクに対応する識別値。
	private: typename ThisClass::FChunkKey ChunkKey;
	/// @brief 条件式が含む論理項の要素の先頭インデクス番号。
	private: typename ThisClass::FElementIndex BeginIndex;
	/// @brief 条件式が含む論理項の要素の末尾インデクス番号。
	private: typename ThisClass::FElementIndex EndIndex;
	/// @brief 条件式が含む論理項を結合する論理演算子。
	private: EPsyqueRulesExpressionLogic Logic;
	/// @brief 条件式の種類。
	private: EPsyqueRulesExpressionKind Kind;

}; // class Psyque::RulesEngine::_private::TExpression

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 複合条件式の論理項要素。
/// @details TExpressionChunk::SubExpressions の要素として使われる。
/// @tparam TemplateExpressionKey @copydoc Psyque::RulesEngine::_private::TEvaluator::FExpressionKey
template<typename TemplateExpressionKey>
class Psyque::RulesEngine::_private::TSubExpression
{
	private: using ThisClass = TSubExpression; ///< @copydoc TEvaluator::ThisClass

	/// @brief 複合条件式の論理項要素を構築する。
	public: PSYQUE_CONSTEXPR TSubExpression(
		/// [in] ThisClass::Key の初期値。
		TemplateExpressionKey InKey,
		/// [in] ThisClass::FCondition の初期値。
		bool const InCondition)
	PSYQUE_NOEXCEPT:
	Key(MoveTemp(InKey)),
	Condition(InCondition)
	{}

	public: TemplateExpressionKey const PSYQUE_CONSTEXPR GetKey()
	const PSYQUE_NOEXCEPT
	{
		return this->Key;
	}

	public: bool PSYQUE_CONSTEXPR CompareCondition(bool InCondition)
	const PSYQUE_NOEXCEPT
	{
		return InCondition == this->Condition;
	}

	/// @brief 結合する条件式の識別値。
	private: TemplateExpressionKey Key;
	/// @brief 結合する際の条件。
	private: bool Condition;

}; // class Psyque::RulesEngine::_private::TSubExpression

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態変化条件式の論理項要素。
/// @details TExpressionChunk::StatusTransitions の要素として使われる。
/// @tparam TemplateStatusKey @copydoc Psyque::RulesEngine::_private::TReservoir::FStatusKey
template<typename TemplateStatusKey>
class Psyque::RulesEngine::_private::TStatusTransition
{
	private: using ThisClass = TStatusTransition; ///< @copydoc TEvaluator::ThisClass

	/// @brief 状態変化条件式の論理項要素を構築する。
	public: PSYQUE_CONSTEXPR TStatusTransition(
		/// [in] ThisClass::Key の初期値。
		TemplateStatusKey InKey)
	PSYQUE_NOEXCEPT: Key(MoveTemp(InKey))
	{}

	public: TemplateStatusKey const PSYQUE_CONSTEXPR GetKey()
	const PSYQUE_NOEXCEPT
	{
		return this->Key;
	}

	/// @brief 変化を検知する状態値の識別値。
	private: TemplateStatusKey Key;

}; // class Psyque::RulesEngine::_private::TStatusTransition

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 論理項要素チャンク。
/// @details TEvaluator::Chunks の要素として使われる。
/// @tparam TemplateSubExpressionArray	  @copydoc TExpressionChunk::FSubExpressionArray
/// @tparam TemplateStatusTransitionArray @copydoc TExpressionChunk::FStatusTransitionArray
/// @tparam TemplateStatusComparisonArray @copydoc TExpressionChunk::FStatusComparisonArray
template<
	typename TemplateSubExpressionArray,
	typename TemplateStatusTransitionArray,
	typename TemplateStatusComparisonArray>
class Psyque::RulesEngine::_private::TExpressionChunk
{
	private: using ThisClass = TExpressionChunk; ///< @copydoc TEvaluator::ThisClass

	//-------------------------------------------------------------------------
	/// @brief 複合条件式の論理項要素のコンテナの型。
	public: using FSubExpressionArray = TemplateSubExpressionArray;
	/// @brief 状態変化条件式の論理項要素のコンテナの型。
	public: using FStatusTransitionArray = TemplateStatusTransitionArray;
	/// @brief 状態比較条件式の論理項要素のコンテナの型。
	public: using FStatusComparisonArray = TemplateStatusComparisonArray;

	//-------------------------------------------------------------------------
	/// @brief 空の論理項要素チャンクを構築する。
	public: explicit TExpressionChunk(
		/// [in] メモリ割当子の初期値。
		typename ThisClass::FSubExpressionArray::allocator_type const& InAllocator):
	SubExpressions(InAllocator),
	StatusTransitions(InAllocator),
	StatusComparisons(InAllocator)
	{}

#ifdef PSYQUE_NO_STD_DEFAULTED_FUNCTION
	/// @brief ムーブ構築子。
	public: TExpressionChunk(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource):
	SubExpressions(MoveTemp(OutSource.SubExpressions)),
	StatusTransitions(MoveTemp(OutSource.StatusTransitions)),
	StatusComparisons(MoveTemp(OutSource.StatusComparisons))
	{}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: ThisClass& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource)
	{
		if (this != &OutSource)
		{
			this->SubExpressions = MoveTemp(OutSource.SubExpressions);
			this->StatusTransitions = MoveTemp(OutSource.StatusTransitions);
			this->StatusComparisons = MoveTemp(OutSource.StatusComparisons);
		}
		return *this;
	}
#endif // defined(PSYQUE_NO_STD_DEFAULTED_FUNCTION)

	//-------------------------------------------------------------------------
	/// @brief 複合条件式で使う論理項要素のコンテナ。
	public: typename ThisClass::FSubExpressionArray SubExpressions;
	/// @brief 状態変化条件式で使う論理項要素のコンテナ。
	public: typename ThisClass::FStatusTransitionArray StatusTransitions;
	/// @brief 状態比較条件式で使う論理項要素のコンテナ。
	public: typename ThisClass::FStatusComparisonArray StatusComparisons;

}; // class Psyque::RulesEngine::_private::TExpressionChunk

// vim: set noexpandtab:
