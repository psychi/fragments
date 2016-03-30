// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TExpression
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <cstdint>
#include "../Assert.h"
#include "./Enum.h"

/// @cond
namespace Psyque
{
	namespace RuleEngine
	{
		namespace _private
		{
			template<typename, typename> class TExpression;
			template<typename> class TSubExpression;
			template<typename> class TStatusTransition;
			template<typename, typename, typename> class TExpressionChunk;
		} // namespace _private
	} // namespace RuleEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 条件式。
/// @tparam TemplateChunkKey     @copydoc TExpression::FChunkKey
/// @tparam TemplateElementIndex @copydoc TExpression::FElementIndex
template<typename TemplateChunkKey, typename TemplateElementIndex>
class Psyque::RuleEngine::_private::TExpression
{
	private: using This = TExpression; ///< @copydoc TEvaluator::This

	//-------------------------------------------------------------------------
	/// @brief 要素条件チャンクの識別値を表す型。
	public: using FChunkKey = TemplateChunkKey;
	/// @brief 要素条件のインデクス番号を表す型。
	public: using FElementIndex = TemplateElementIndex;

	//-------------------------------------------------------------------------
	/// @brief 条件式を構築する。
	public: TExpression(
		/// [in] This::ChunkKey の初期値。
		typename This::FChunkKey InChunkKey,
		/// [in] This::Logic の初期値。
		RuleEngine::EExpressionLogic const InLogic,
		/// [in] This::Kind の初期値。
		RuleEngine::EExpressionKind const InKind,
		/// [in] This::BeginIndex の初期値。
		typename This::FElementIndex const InBeginIndex,
		/// [in] This::EndIndex の初期値。
		typename This::FElementIndex const InEndIndex)
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

	/// @brief 条件式が格納されている要素条件チャンクの識別値を取得する。
	/// @return @copydoc This::ChunkKey
	public: typename This::FChunkKey const GetChunkKey() const PSYQUE_NOEXCEPT
	{
		return this->ChunkKey;
	}

	/// @brief 条件式が使う要素条件チャンクの先頭インデクス番号を取得する。
	/// @return @copydoc This::BeginIndex
	public: typename This::FElementIndex GetBeginIndex() const PSYQUE_NOEXCEPT
	{
		return this->BeginIndex;
	}

	/// @brief 条件式が使う要素条件チャンクの末尾インデクス番号を取得する。
	/// @return @copydoc This::EndIndex
	public: typename This::FElementIndex GetEndIndex() const PSYQUE_NOEXCEPT
	{
		return this->EndIndex;
	}

	/// @brief 条件式の種類を取得する。
	/// @return @copydoc This::Kind
	public: RuleEngine::EExpressionKind GetKind() const PSYQUE_NOEXCEPT
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
	Psyque::ETernary Evaluate(
		/// [in] 評価に用いる要素条件のコンテナ。
		TemplateElementArray const& InElements,
		/// [in] 要素条件を評価する関数オブジェクト。
		TemplateElementEvaluator const& InEvaluator)
	const PSYQUE_NOEXCEPT
	{
		if (this->IsEmpty()
			|| InElements.size() <= this->GetBeginIndex()
			|| InElements.size() < this->GetEndIndex())
		{
			// 条件式が空か、範囲外の要素条件を参照している。
			check(this->IsEmpty());
			return Psyque::ETernary::Unknown;
		}
		auto const LocalEnd(InElements.begin() + this->EndIndex);
		auto const LocalAnd(this->Logic == RuleEngine::EExpressionLogic::And);
		for (auto i(InElements.begin() + this->BeginIndex); i != LocalEnd; ++i)
		{
			auto const LocalEvaluation(InEvaluator(*i));
			if (LocalEvaluation == Psyque::ETernary::Unknown)
			{
				return Psyque::ETernary::Unknown;
			}
			else if (LocalEvaluation == Psyque::ETernary::True)
			{
				if (!LocalAnd)
				{
					return Psyque::ETernary::True;
				}
			}
			else if (LocalAnd)
			{
				return Psyque::ETernary::False;
			}
		}
		return static_cast<Psyque::ETernary>(LocalAnd);
	}

	//-------------------------------------------------------------------------
	/// @brief 要素条件チャンクに対応する識別値。
	private: typename This::FChunkKey ChunkKey;
	/// @brief 条件式が使う要素条件の先頭インデクス番号。
	private: typename This::FElementIndex BeginIndex;
	/// @brief 条件式が使う要素条件の末尾インデクス番号。
	private: typename This::FElementIndex EndIndex;
	/// @brief 条件式の要素条件を結合する論理演算子。
	private: RuleEngine::EExpressionLogic Logic;
	/// @brief 条件式の種類。
	private: RuleEngine::EExpressionKind Kind;

}; // class Psyque::RuleEngine::_private::TExpression

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 複合条件式の要素条件。
/// @tparam TemplateExpressionKey @copydoc Psyque::RuleEngine::_private::TEvaluator::FExpressionKey
template<typename TemplateExpressionKey>
class Psyque::RuleEngine::_private::TSubExpression
{
	private: using This = TSubExpression; ///< @copydoc TEvaluator::This

	/// @brief 複合条件式の要素条件を構築する。
	public: PSYQUE_CONSTEXPR TSubExpression(
		/// [in] This::Key の初期値。
		TemplateExpressionKey InKey,
		/// [in] This::FCondition の初期値。
		bool const InCondition)
	PSYQUE_NOEXCEPT:
	Key(MoveTemp(InKey)),
	Condition(InCondition)
	{}

	public: PSYQUE_CONSTEXPR TemplateExpressionKey const GetKey()
	const PSYQUE_NOEXCEPT
	{
		return this->Key;
	}

	public: PSYQUE_CONSTEXPR bool CompareCondition(bool InCondition)
	const PSYQUE_NOEXCEPT
	{
		return InCondition == this->Condition;
	}

	/// @brief 結合する条件式の識別値。
	private: TemplateExpressionKey Key;
	/// @brief 結合する際の条件。
	private: bool Condition;

}; // class Psyque::RuleEngine::_private::TSubExpression

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態変化条件式の要素条件。
/// @tparam TemplateStatusKey @copydoc Psyque::RuleEngine::_private::TReservoir::FStatusKey
template<typename TemplateStatusKey>
class Psyque::RuleEngine::_private::TStatusTransition
{
	private: using This = TStatusTransition; ///< @copydoc TEvaluator::This

	/// @brief 状態変化条件式の要素条件を構築する。
	public: PSYQUE_CONSTEXPR TStatusTransition(
		/// [in] This::Key の初期値。
		TemplateStatusKey InKey)
	PSYQUE_NOEXCEPT: Key(MoveTemp(InKey))
	{}

	public: PSYQUE_CONSTEXPR TemplateStatusKey const GetKey()
	const PSYQUE_NOEXCEPT
	{
		return this->Key;
	}

	/// @brief 変化を検知する状態値の識別値。
	private: TemplateStatusKey Key;

}; // class Psyque::RuleEngine::_private::TStatusTransition

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 要素条件チャンク。
/// @tparam TemplateSubExpressionArray	  @copydoc TExpressionChunk::FSubExpressionArray
/// @tparam TemplateStatusTransitionArray @copydoc TExpressionChunk::FStatusTransitionArray
/// @tparam TemplateStatusComparisonArray @copydoc TExpressionChunk::FStatusComparisonArray
template<
	typename TemplateSubExpressionArray,
	typename TemplateStatusTransitionArray,
	typename TemplateStatusComparisonArray>
class Psyque::RuleEngine::_private::TExpressionChunk
{
	private: using This = TExpressionChunk; ///< @copydoc TEvaluator::This

	//-------------------------------------------------------------------------
	/// @brief 複合条件式の要素条件のコンテナの型。
	public: using FSubExpressionArray = TemplateSubExpressionArray;
	/// @brief 状態変化条件式の要素条件のコンテナの型。
	public: using FStatusTransitionArray = TemplateStatusTransitionArray;
	/// @brief 状態比較条件式の要素条件のコンテナの型。
	public: using FStatusComparisonArray = TemplateStatusComparisonArray;

	//-------------------------------------------------------------------------
	/// @brief 空の要素条件チャンクを構築する。
	public: explicit TExpressionChunk(
		/// [in] メモリ割当子の初期値。
		typename This::FSubExpressionArray::allocator_type const& InAllocator):
	SubExpressions(InAllocator),
	StatusTransitions(InAllocator),
	StatusComparisons(InAllocator)
	{}

#ifdef PSYQUE_NO_STD_DEFAULTED_FUNCTION
	/// @brief ムーブ構築子。
	public: TExpressionChunk(
		/// [in,out] ムーブ元となるインスタンス。
		This&& OutSource):
	SubExpressions(MoveTemp(OutSource.SubExpressions)),
	StatusTransitions(MoveTemp(OutSource.StatusTransitions)),
	StatusComparisons(MoveTemp(OutSource.StatusComparisons))
	{}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: This& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		This&& OutSource)
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
	/// @brief 複合条件式で使う要素条件のコンテナ。
	public: typename This::FSubExpressionArray SubExpressions;
	/// @brief 状態変化条件式で使う要素条件のコンテナ。
	public: typename This::FStatusTransitionArray StatusTransitions;
	/// @brief 状態比較条件式で使う要素条件のコンテナ。
	public: typename This::FStatusComparisonArray StatusComparisons;

}; // class Psyque::RuleEngine::_private::TExpressionChunk

// vim: set noexpandtab:
