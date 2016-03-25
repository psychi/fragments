// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TEvaluator
/// @author Hillco Psychi (https://twitter.com/psychi)

#include <utility>
#include "Containers/Map.h"
#include "./Expression.h"

/// @cond
namespace Psyque
{
	namespace RuleEngine
	{
		namespace _private
		{
			template<typename, typename> class TEvaluator;
		} // namespace _private
	} // namespace RuleEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 条件評価器。条件式を保持し、評価する。
/// @par 使い方の概略
/// - TEvaluator::RegisterExpression で、条件式を登録する。
/// - TEvaluator::EvaluateExpression で、条件式を評価する。
/// .
/// @tparam TemplateReservoir     @copydoc TEvaluator::FReservoir
/// @tparam TemplateExpressionKey @copydoc TEvaluator::FExpressionKey
template<typename TemplateReservoir, typename TemplateExpressionKey>
class Psyque::RuleEngine::_private::TEvaluator
{
	using This = TEvaluator; ///< @copydoc TReservoir::This

	//-------------------------------------------------------------------------
	public:
	/// @brief 条件評価器で使う TReservoir 型。
	using FReservoir = TemplateReservoir;
	/// @copydoc TReservoir::FSetAllocator
	using FSetAllocator = typename TemplateReservoir::FSetAllocator;

	//-------------------------------------------------------------------------
	public:
	/// @brief 条件評価器で使う条件式の識別値。
	using FExpressionKey = TemplateExpressionKey;
	/// @brief 条件評価器で使う条件式。
	using FExpression = Psyque::RuleEngine::_private::TExpression<
		typename TemplateReservoir::FStatusValue::FEvaluation,
		typename TemplateReservoir::FChunkKey,
		uint32>;

	//-------------------------------------------------------------------------
	public:
	/// @brief 要素条件チャンクの識別値。
	using FChunkKey = typename TemplateReservoir::FChunkKey;
	/// @brief 条件式が参照する要素条件チャンク。
	using FChunk = Psyque::RuleEngine::_private::TExpressionChunk<
		typename This::FExpressionKey,
		typename This::FReservoir::FStatusKey,
		typename This::FReservoir::FStatusComparison,
		typename This::FSetAllocator::SparseArrayAllocator::ElementAllocator>;

	//-------------------------------------------------------------------------
	private:
	/// @brief 条件式の辞書。
	using FExpressionMap = TMap<
		typename This::FExpressionKey,
		typename This::FExpression,
		typename This::FSetAllocator>;
	/// @brief 要素条件チャンクの辞書。
	using FChunkMap = TMap<
		typename This::FChunkKey,
		typename This::FChunk,
		typename This::FSetAllocator>;

	//-------------------------------------------------------------------------
	/// @name 構築
	/// @{
	public:
	/// @brief 空の条件評価器を構築する。
	TEvaluator(
		/// [in] チャンク辞書の予約容量。
		int32 const InChunkCapacity,
		/// [in] 条件式辞書の予約容量。
		int32 const InExpressionCapacity)
	{
		this->Chunks.Reserve(InChunkCapacity);
		this->Expressions.Reserve(InExpressionCapacity);
	}

	/// @brief 条件評価器を再構築する。
	void Rebuild(
		/// [in] 要素条件チャンク辞書のバケット数。
		int32 const InChunkCapacity,
		/// [in] 条件式辞書のバケット数。
		int32 const InExpressionCapacity)
	{
		//this->Expressions.rehash(InExpressionCapacity);
		//this->Chunks.rehash(InChunkCapacity);
		for (auto& LocalChunk: this->Chunks)
		{
			LocalChunk.SubExpressions.Shrink();
			LocalChunk.StatusTransitions.Shrink();
			LocalChunk.StatusComparisons.Shrink();
		}
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 条件式
	/// @{
	public:
	/// @brief 条件式が登録されているか判定する。
	/// @retval true  InExpressionKey に対応する条件式が *this に登録されている。
	/// @retval false InExpressionKey に対応する条件式は *this に登録されてない。
	bool IsRegisterd(
		/// [in] 判定する状態値に対応する識別値。
		typename This::FExpressionKey const InExpressionKey)
	const
	{
		return this->Expressions.Contains(InExpressionKey);
	}

	/// @brief 条件式を登録する。
	/// @sa This::EvaluateExpression で、登録した条件式を評価できる。
	/// @sa This::RemoveChunk で、登録した条件式をチャンク単位で削除できる。
	/// @retval true  成功。条件式を *this に登録した。
	/// @retval false 失敗。条件式は登録されなかった。
	/// - InExpressionKey に対応する条件式が既にあると失敗する。
	/// - InElementBegin と InElementEnd が等価だと失敗する。
	template<typename TemplateElementIterator>
	bool RegisterExpression(
		/// [in] 条件式を登録する要素条件チャンクの識別値。
		typename This::FChunkKey const& InChunkKey,
		/// [in] 登録する条件式の識別値。
		typename This::FExpressionKey const& InExpressionKey,
		/// [in] 要素条件を結合する論理演算子。
		typename This::FExpression::ELogic const InLogic,
		/// [in] 登録する条件式の要素条件コンテナの先頭を指す反復子。
		TemplateElementIterator const& InElementBegin,
		/// [in] 登録する条件式の要素条件コンテナの末尾を指す反復子。
		TemplateElementIterator const& InElementEnd)
	{
		check(
			This::IsValidElements(
				InElementBegin, InElementEnd, this->Expressions));
		if (InElementBegin == InElementEnd
			|| this->IsRegisterd(InExpressionKey))
		{
			return false;
		}

		// 要素条件の種類を判定する。
		auto& LocalChunk(this->Chunks.FindOrAdd(InChunkKey));
		auto const LocalElementKind(
			This::GetElementContainer(LocalChunk, *InElementBegin));

		// 要素条件を挿入する。
		auto& LocalElements(*LocalElementKind.second);
		auto const LocalBeginIndex(
			static_cast<typename This::FExpression::FElementIndex>(
				LocalElements.Num()));
		check(LocalBeginIndex == LocalElements.Num());
		for (auto i(InElementBegin); i != InElementEnd; ++i)
		{
			LocalElements.Emplace(*i);
		}

		// 条件式を挿入する。
		auto const& LocalExpression(
			this->Expressions.Emplace(
				InExpressionKey,
				typename This::FExpression(
					InChunkKey,
					InLogic,
					LocalElementKind.first,
					LocalBeginIndex,
					static_cast<typename This::FExpression::FElementIndex>(
						LocalElements.Num()))));
		check(
			LocalExpression.GetElementEnd() == LocalElements.Num()
			&& LocalBeginIndex < LocalExpression.GetElementEnd());
		return true;
	}

	/// @brief 条件式を登録する。
	/// @sa This::EvaluateExpression で、登録した条件式を評価できる。
	/// @sa This::RemoveChunk で、登録した条件式をチャンク単位で削除できる。
	/// @retval true  成功。条件式を *this に登録した。
	/// @retval false 失敗。条件式は登録されなかった。
	/// - InExpressionKey に対応する条件式が既にあると失敗する。
	/// - InElements が空だと失敗する。
	template<typename TemplateElementContainer>
	bool RegisterExpression(
		/// [in] 条件式を登録する要素条件チャンクの識別値。
		typename This::FChunkKey const InChunkKey,
		/// [in] 登録する条件式の識別値。
		typename This::FExpressionKey const InExpressionKey,
		/// [in] 要素条件を結合する論理演算子。
		typename This::FExpression::ELogic const InLogic,
		/// [in] 登録する条件式の要素条件コンテナ。
		TemplateElementContainer const& InElements)
	{
		return this->RegisterExpression(
			InChunkKey,
			InExpressionKey,
			InLogic,
			begin(InElements),
			end(InElements));
	}

	/// @brief 状態値を比較する条件式を登録する。
	/// @sa This::EvaluateExpression で、登録した条件式を評価できる。
	/// @sa This::RemoveChunk で、登録した条件式をチャンク単位で削除できる。
	/// @retval true 成功。条件式を *this に登録した。
	/// 条件式を登録した要素条件チャンクの識別値は、 InComparison.GetKey()
	/// に対応する状態値が登録されている状態値ビット列チャンクの識別値と同じ。
	/// @retval false 失敗。条件式は登録されなかった。
	/// - InExpressionKey に対応する条件式が既にあると失敗する。
	/// - InComparison.GetKey() に対応する状態値が InReservoir にないと失敗する。
	bool RegisterExpression(
		/// [in] 条件式が参照する状態貯蔵器。
		typename This::FReservoir const& InReservoir,
		/// [in] 登録する条件式の識別値。
		typename This::FExpressionKey const InExpressionKey,
		/// [in] 登録する状態比較要素条件。
		typename This::FReservoir::FStatusComparison const& InComparison)
	{
		auto const LocalStatusProperty(
			InReservoir.FindProperty(InComparison.GetKey()));
		return !LocalStatusProperty.IsEmpty()
			&& this->RegisterExpression(
				LocalStatusProperty.GetChunkKey(),
				InExpressionKey,
				This::FExpression::ELogic::AND,
				&InComparison,
				&InComparison + 1);
	}

	/// @brief 論理型の状態値を比較する条件式を登録する。
	/// @sa This::EvaluateExpression で、登録した条件式を評価できる。
	/// @sa This::RemoveChunk で、登録した条件式をチャンク単位で削除できる。
	/// @retval true 成功。条件式を *this に登録した。
	/// 条件式を登録した要素条件チャンクの識別値は、 InStatusKey
	/// に対応する状態値が登録されている状態値ビット列チャンクの識別値と同じ。
	/// @retval false 失敗。条件式は登録されなかった。
	/// - InExpressionKey に対応する条件式が既にあると失敗する。
	/// - InStatusKey に対応する状態値が論理型以外だと失敗する。
	bool RegisterExpression(
		/// [in] 条件式が参照する状態貯蔵器。
		typename This::FReservoir const& InReservoir,
		/// [in] 登録する条件式の識別値。
		typename This::FExpressionKey const InExpressionKey,
		/// [in] 評価する論理型の状態値の識別値。
		typename This::FReservoir::FStatusKey const InStatusKey,
		/// [in] 条件となる論理値。
		bool const InCondition)
	{
		return
			InReservoir.FindKind(InStatusKey) ==
				This::FReservoir::StatusValue::EKind::BOOL
			&& this->RegisterExpression(
				InReservoir,
				InExpressionKey,
				typename This::FReservoir::StatusComparison(
					InStatusKey,
					InCondition?
						This::FReservoir::StatusValue::EComparison::NOT_EQUAL:
						This::FReservoir::StatusValue::EComparison::EQUAL,
					typename This::FReservoir::StatusValue(false)));
	}

	/// @brief 条件式を取得する。
	/// @return InExpressionKey に対応する This::FExpression のコピー。
	/// 該当する条件式がない場合は
	/// This::FExpression::IsEmpty が真となる値を返す。
	typename This::FExpression FindExpression(
		/// [in] 取得する条件式に対応する識別値。
		typename This::FExpressionKey const InExpressionKey)
	const
	{
		auto const LocalExpression(this->Expressions.Find(InExpressionKey));
		return LocalExpression != nullptr?
			*LocalExpression:
			typename This::FExpression(
				typename This::FChunkKey(),
				This::FExpression::ELogic::OR,
				This::FExpression::EKind::SUB_EXPRESSION,
				0,
				0);
	}

	/// @brief 登録されている条件式を評価する。
	/// @sa This::RegisterExpression で、条件式を登録できる。
	/// @retval 正 条件式の評価は真となった。
	/// @retval 0  条件式の評価は偽となった。
	/// @retval 負 条件式の評価に失敗した。
	///   - 条件式が登録されていないと、失敗する。
	///   - 条件式が参照する状態値が登録されていないと、失敗する。
	typename This::FExpression::FEvaluation EvaluateExpression(
		/// [in] 評価する条件式に対応する識別値。
		typename This::FExpressionKey const InExpressionKey,
		/// [in] 条件式が参照する状態貯蔵器。
		typename This::FReservoir const& InReservoir)
	const
	{
		// 条件式の辞書から、該当する条件式を検索する。
		auto const LocalExpression(this->Expressions.Find(InExpressionKey));
		if (LocalExpression == nullptr)
		{
			return -1;
		}
		auto const LocalChunk(
			this->_find_chunk(LocalExpression->GetChunkKey()));
		if (LocalChunk == nullptr)
		{
			// 条件式があれば、要素条件チャンクもあるはず。
			check(false);
			return -1;
		}

		// 条件式の種別によって評価方法を分岐する。
		switch (LocalExpression->GetKind())
		{
			// 複合条件式を評価する。
			case This::FExpression::EKind::SUB_EXPRESSION:
			using FSubExpression =
				typename This::FChunk::FSubExpressionContainer::ElementType;
			return LocalExpression->Evaluate(
				LocalChunk->SubExpressions,
				[&InReservoir, this](FSubExpression const& InExpression)
				->typename This::FExpression::FEvaluation
				{
					auto const LocalEvaluateExpression(
						this->EvaluateExpression(
							InExpression.GetKey(), InReservoir));
					if (LocalEvaluateExpression < 0)
					{
						return -1;
					}
					return InExpression.CompareCondition(
						0 < LocalEvaluateExpression);
				});

			// 状態変化条件式を評価する。
			case This::FExpression::EKind::STATUS_TRANSITION:
			using FStatusTransition =
				typename This::FChunk::StatusTransitionContainer::ElementType;
			return LocalExpression->Evaluate(
				LocalChunk->StatusTransitions,
				[&InReservoir](FStatusTransition const& InTransition)
				->typename This::FExpression::FEvaluation
				{
					return InReservoir.FindTransition(InTransition.GetKey());
				});

			// 状態比較条件式を評価する。
			case This::FExpression::EKind::STATUS_COMPARISON:
			using FStatusComparison =
				typename This::FChunk::StatusComparisonContainer::ElementType;
			return LocalExpression->Evaluate(
				LocalChunk->StatusComparisons,
				[&InReservoir](FStatusComparison const& InComparison)
				->typename This::FExpression::FEvaluation
				{
					return InReservoir.CompareStatus(InComparison);
				});

			// 条件式の種別が未知だった。
			default:
			check(false);
			return -1;
		}
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 要素条件チャンク
	/// @{
	public:
	/// @brief 要素条件チャンクを予約する。
	void ReserveChunk(
		/// [in] 予約する要素条件チャンクに対応する識別値。
		typename This::FChunkKey const InChunkKey,
		/// [in] 複合条件式の要素条件の予約数。
		int32 const InSubExpressionCapacity,
		/// [in] 状態変更条件式の要素条件の予約数。
		int32 const InStatusTransitionCapacity,
		/// [in] 状態比較条件式の要素条件の予約数。
		int32 const InStatusComparisonCapacity)
	{
		This::ReserveChunk(
			this->Chunks,
			InChunkKey,
			InSubExpressionCapacity,
			InStatusTransitionCapacity,
			InStatusComparisonCapacity);
	}

	/// @brief 要素条件チャンクと、それを使っている条件式を破棄する。
	/// @retval true  成功。チャンクを破棄した。
	/// @retval false 失敗。 InChunkKey に対応するチャンクがない。
	bool RemoveChunk(
		/// [in] 破棄する要素条件チャンクに対応する識別値。
		typename This::FChunkKey const& InChunkKey)
	{
		// 要素条件チャンクを削除する。
		if (this->Chunks.Remove(InChunkKey) == 0)
		{
			return false;
		}

		// 条件式を削除する。
		for (auto i(this->Expressions.CreateIterator()); i; ++i)
		{
			if (InChunkKey == i.Value().GetChunkKey())
			{
				i.RemoveCurrent();
			}
		}
		return true;
	}

	/// @brief Psyque::RuleEngine 管理者以外は、この関数は使用禁止。
	/// @details 要素条件チャンクを取得する。
	/// @return InChunkKey に対応する要素条件チャンクを指すポインタ。
	/// 該当する要素条件チャンクがない場合は nullptr を返す。
	typename This::FChunk const* _find_chunk(
		/// [in] 取得する要素条件チャンクに対応する識別値。
		typename This::FChunkKey const InChunkKey)
	const
	{
		return this->Chunks.Find(InChunkKey);
	}
	/// @}
	//-------------------------------------------------------------------------
	private:
	/// @brief 要素条件チャンクを予約する。
	static typename This::FChunk& ReserveChunk(
		/// [in,out] 予約する要素条件チャンクの辞書。
		typename This::FChunkMap& OutChunks,
		/// [in] 予約するチャンクに対応する識別値。
		typename This::FChunkKey const& InChunkKey,
		/// [in] 複合条件式の要素条件の予約数。
		int32 const InSubExpressionCapacity,
		/// [in] 状態変更条件式の要素条件の予約数。
		int32 const InStatusTransitionCapacity,
		/// [in] 状態比較条件式の要素条件の予約数。
		int32 const InStatusComparisonCapacity)
	{
		auto& LocalChunk(OutChunks.FindOrAdd(InChunkKey));
		LocalChunk.SubExpressions.Reserve(InSubExpressionCapacity);
		LocalChunk.StatusTransitions.Reserve(InStatusTransitionCapacity);
		LocalChunk.StatusComparisons.Reserve(InStatusComparisonCapacity);
		return LocalChunk;
	}

	/// @brief 条件式チャンクから複合条件コンテナを取得する。
	/// @return 複合条件の種別とコンテナのペア。
	static std::pair<
		 typename This::FExpression::EKind,
		 typename This::FChunk::FSubExpressionContainer*>
	GetElementContainer(
		/// [in] 複合条件コンテナを取り出すチャンク。
		typename This::FChunk& InChunk,
		typename This::FChunk::FSubExpressionContainer::ElementType const&)
	{
		return std::make_pair(
			This::FExpression::EKind::SUB_EXPRESSION,
			&InChunk.SubExpressions);
	}

	/// @brief 条件式チャンクから状態変化条件コンテナを取得する。
	/// @return 状態変化条件の種別とコンテナのペア。
	static std::pair<
		 typename This::FExpression::EKind,
		 typename This::FChunk::FStatusTransitionContainer*>
	GetElementContainer(
		/// [in] 状態変化条件コンテナを取り出すチャンク。
		typename This::FChunk& InChunk,
		typename This::FChunk::FStatusTransitionContainer::ElementType const&)
	{
		return std::make_pair(
			This::FExpression::EKind::STATUS_TRANSITION,
			&InChunk.StatusTransitions);
	}

	/// @brief 条件式チャンクから状態比較条件コンテナを取得する。
	/// @return 状態比較条件の種別とコンテナのペア。
	static std::pair<
		typename This::FExpression::EKind,
		typename This::FChunk::FStatusComparisonContainer*>
	GetElementContainer(
		/// [in] 状態比較条件コンテナを取り出すチャンク。
		typename This::FChunk& InChunk,
		typename This::FChunk::FStatusComparisonContainer::ElementType const&)
	{
		return std::make_pair(
			This::FExpression::EKind::STATUS_COMPARISON,
			&InChunk.StatusComparisons);
	}

	template<typename TemplateElementIterator>
	static bool IsValidElements(
		TemplateElementIterator const& InElementBegin,
		TemplateElementIterator const& InElementEnd,
		typename This::FExpressionMap const& InExpressions)
	{
		for (auto i(InElementBegin); i != InElementEnd; ++i)
		{
			if (!This::IsValidElement(*i, InExpressions))
			{
				return false;
			}
		}
		return true;
	}

	static bool IsValidElement(
		typename This::FChunk::FSubExpressionContainer::ElementType const&
			InSubExpression,
		typename This::FExpressionMap const& InExpressions)
	{
		// 要素条件にある条件式がすでにあることを確認する。
		auto const LocalContains(
			InExpressions.Contains(InSubExpression.GetKey()));
		check(LocalContains);
		return LocalContains;
	}

	template<typename TemplateElement>
	PSYQUE_CONSTEXPR static bool IsValidElement(
		TemplateElement const&, typename This::FExpressionMap const&)
	PSYQUE_NOEXCEPT
	{
		return true;
	}

	//-------------------------------------------------------------------------
	private:
	typename This::FChunkMap Chunks;           ///< 要素条件チャンクの辞書。
	typename This::FExpressionMap Expressions; ///< 条件式の辞書。

}; // class Psyque::RuleEngine::_private::TEvaluator

// vim: set noexpandtab:
