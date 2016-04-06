// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::TEvaluator
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <unordered_map>
#include <vector>
#include "../PrimitiveBits.h"
#include "./Expression.h"

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		namespace _private
		{
			template<typename, typename> class TEvaluator;
		} // namespace _private
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 条件評価器。条件式を保持し、評価する。
/// @par 使い方の概略
///   - TEvaluator::RegisterExpression で、条件式を登録する。
///   - TEvaluator::EvaluateExpression で、条件式を評価する。
/// @tparam TemplateReservoir     @copydoc TEvaluator::FReservoir
/// @tparam TemplateExpressionKey @copydoc TEvaluator::FExpressionKey
template<typename TemplateReservoir, typename TemplateExpressionKey>
class Psyque::RulesEngine::_private::TEvaluator
{
	private: using ThisClass = TEvaluator; ///< @copydoc RulesEngine::TDriver::ThisClass

	//-------------------------------------------------------------------------
	/// @brief 条件評価器で使う TReservoir の型。
	public: using FReservoir = TemplateReservoir;
	/// @brief 条件評価器で使うメモリ割当子の型。
	public: using FAllocator = typename ThisClass::FReservoir::FAllocator;

	//-------------------------------------------------------------------------
	/// @brief 条件評価器で使う条件式の識別値。
	public: using FExpressionKey = TemplateExpressionKey;
	/// @brief 条件評価器で使う条件式。
	public: using FExpression = Psyque::RulesEngine::_private::TExpression<
		typename TemplateReservoir::FChunkKey, uint32>;

	//-------------------------------------------------------------------------
	/// @brief 条件式が参照する要素条件チャンク。
	public: using FChunk = Psyque::RulesEngine::_private::TExpressionChunk<
		std::vector<
			Psyque::RulesEngine::_private::TSubExpression<
				typename ThisClass::FExpressionKey>,
			typename TEvaluator::FAllocator>,
		std::vector<
			Psyque::RulesEngine::_private::TStatusTransition<
				typename ThisClass::FReservoir::FStatusKey>,
			typename ThisClass::FAllocator>,
		std::vector<
			typename ThisClass::FReservoir::FStatusComparison,
			typename ThisClass::FAllocator>>;
	/// @brief 要素条件チャンクの識別値。
	public: using FChunkKey = typename ThisClass::FReservoir::FChunkKey;

	//-------------------------------------------------------------------------
	/// @brief 条件式の辞書。
	private: using FExpressionMap = std::unordered_map<
		typename ThisClass::FExpressionKey,
		typename ThisClass::FExpression,
		Psyque::Hash::TPrimitiveBits<typename ThisClass::FExpressionKey>,
		std::equal_to<typename ThisClass::FExpressionKey>,
		typename ThisClass::FAllocator>;
	/// @brief 要素条件チャンクの辞書。
	private: using FChunkMap = std::unordered_map<
		typename ThisClass::FChunkKey,
		typename ThisClass::FChunk,
		Psyque::Hash::TPrimitiveBits<typename ThisClass::FChunkKey>,
		std::equal_to<typename ThisClass::FChunkKey>,
		typename ThisClass::FAllocator>;

	//-------------------------------------------------------------------------
	/// @name 構築と代入
	/// @{

	/// @brief 空の条件評価器を構築する。
	public: TEvaluator(
		/// [in] チャンク辞書のバケット数。
		std::size_t const InChunkCapacity,
		/// [in] 条件式辞書のバケット数。
		std::size_t const InExpressionCapacity,
		/// [in] メモリ割当子の初期値。
		typename ThisClass::FAllocator const& InAllocator =
			typename ThisClass::FAllocator()):
	Chunks(
		InChunkCapacity,
		typename ThisClass::FChunkMap::hasher(),
		typename ThisClass::FChunkMap::key_equal(),
		InAllocator),
	Expressions(
		InExpressionCapacity,
		typename ThisClass::FExpressionMap::hasher(),
		typename ThisClass::FExpressionMap::key_equal(),
		InAllocator)
	{}

#ifdef PSYQUE_NO_STD_DEFAULTED_FUNCTION
	/// @brief ムーブ構築子。
	public: TEvaluator(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource):
	Chunks(MoveTemp(OutSource.Chunks)),
	Expressions(MoveTemp(OutSource.Expressions))
	{}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: ThisClass& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource)
	{
		this->Chunks = MoveTemp(OutSource.Chunks);
		this->Expressions = MoveTemp(OutSource.Expressions);
		return *this;
	}
#endif // defined(PSYQUE_NO_STD_DEFAULTED_FUNCTION)

	/// @brief 条件評価器で使われているメモリ割当子を取得する。
	/// @return *this で使われているメモリ割当子のコピー。
	public: typename ThisClass::FAllocator get_allocator()
	const PSYQUE_NOEXCEPT
	{
		return this->Expressions.get_allocator();
	}

	/// @brief 条件評価器を再構築する。
	public: void Rebuild(
		/// [in] 要素条件チャンク辞書のバケット数。
		std::size_t const InChunkCapacity,
		/// [in] 条件式辞書のバケット数。
		std::size_t const InExpressionCapacity)
	{
		this->Expressions.rehash(InExpressionCapacity);
		this->Chunks.rehash(InChunkCapacity);
		for (auto& LocalChunk: this->Chunks)
		{
			LocalChunk.second.SubExpressions.shrink_to_fit();
			LocalChunk.second.StatusTransitions.shrink_to_fit();
			LocalChunk.second.StatusComparisons.shrink_to_fit();
		}
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 条件式
	/// @{

	/// @brief 条件式が登録されているか判定する。
	/// @retval true  InExpressionKey に対応する条件式が *this に登録されている。
	/// @retval false InExpressionKey に対応する条件式は *this に登録されてない。
	public: bool IsRegistered(
		/// [in] 判定する状態値に対応する識別値。
		typename ThisClass::FExpressionKey const InExpressionKey)
	const
	{
		return this->Expressions.find(InExpressionKey)
			!= this->Expressions.end();
	}

	/// @brief 条件式を登録する。
	/// @sa ThisClass::EvaluateExpression で、登録した条件式を評価できる。
	/// @sa ThisClass::RemoveChunk で、登録した条件式をチャンク単位で削除できる。
	/// @retval true 成功。条件式を *this に登録した。
	/// @retval false
	///   失敗。条件式は登録されなかった。
	///   - InExpressionKey に対応する条件式が既にあると失敗する。
	///   - InBeginIterator と InEndIterator が等価だと失敗する。
	public: template<typename TemplateIterator>
	bool RegisterExpression(
		/// [in] 条件式を登録する要素条件チャンクの識別値。
		typename ThisClass::FChunkKey const InChunkKey,
		/// [in] 登録する条件式の識別値。
		typename ThisClass::FExpressionKey const InExpressionKey,
		/// [in] 要素条件を結合する論理演算子。
		RulesEngine::EExpressionLogic const InLogic,
		/// [in] 登録する条件式の要素条件コンテナの先頭を指す反復子。
		TemplateIterator const& InBeginIterator,
		/// [in] 登録する条件式の要素条件コンテナの末尾を指す反復子。
		TemplateIterator const& InEndIterator)
	{
		check(
			ThisClass::IsValidElements(
				InBeginIterator, InEndIterator, this->Expressions));
		if (InBeginIterator == InEndIterator
			|| this->IsRegistered(InExpressionKey))
		{
			return false;
		}

		// 要素条件の種類を判定する。
		auto const local_emplace_chunk(
			this->Chunks.emplace(
				InChunkKey,
				typename ThisClass::FChunkMap::mapped_type(
					this->Chunks.get_allocator())));
		auto const local_element_kind(
			ThisClass::MakeElementKind(
				local_emplace_chunk.first->second, *InBeginIterator));

		// 要素条件を挿入する。
		auto& local_elements(*local_element_kind.second);
		auto const local_begin_index(
			static_cast<typename ThisClass::FExpression::FElementIndex>(
				local_elements.size()));
		check(local_begin_index == local_elements.size());
		local_elements.insert(
			local_elements.end(), InBeginIterator, InEndIterator);

		// 条件式を挿入する。
		auto const local_emplace_expression(
			this->Expressions.emplace(
				InExpressionKey,
				typename ThisClass::FExpressionMap::mapped_type(
					InChunkKey,
					InLogic,
					local_element_kind.first,
					local_begin_index,
					static_cast<typename ThisClass::FExpression::FElementIndex>(
						local_elements.size()))));
		check(
			local_emplace_expression.second
			&& local_emplace_expression.first->second.GetEndIndex()
				== local_elements.size()
			&& local_begin_index
				< local_emplace_expression.first->second.GetEndIndex());
		return local_emplace_expression.second;
	}

	/// @brief 条件式を登録する。
	/// @sa ThisClass::EvaluateExpression で、登録した条件式を評価できる。
	/// @sa ThisClass::RemoveChunk で、登録した条件式をチャンク単位で削除できる。
	/// @retval true 成功。条件式を *this に登録した。
	/// @retval false
	///   失敗。条件式は登録されなかった。
	///   - InExpressionKey に対応する条件式が既にあると失敗する。
	///   - InElements が空だと失敗する。
	public: template<typename template_element_container>
	bool RegisterExpression(
		/// [in] 条件式を登録する要素条件チャンクの識別値。
		typename ThisClass::FChunkKey const InChunkKey,
		/// [in] 登録する条件式の識別値。
		typename ThisClass::FExpressionKey const InExpressionKey,
		/// [in] 要素条件を結合する論理演算子。
		RulesEngine::EExpressionLogic const InLogic,
		/// [in] 登録する条件式の要素条件コンテナ。
		template_element_container const& InElements)
	{
		return this->RegisterExpression(
			InChunkKey,
			InExpressionKey,
			InLogic,
			std::begin(InElements),
			std::end(InElements));
	}

	/// @brief 状態値を比較する条件式を登録する。
	/// @sa ThisClass::EvaluateExpression で、登録した条件式を評価できる。
	/// @sa ThisClass::RemoveChunk で、登録した条件式をチャンク単位で削除できる。
	/// @retval true
	///   成功。条件式を *this に登録した。
	///   条件式を登録した要素条件チャンクの識別値は、 InComparison.GetKey()
	///   に対応する状態値が登録されている状態値ビット列チャンクの識別値と同じ。
	/// @retval false
	///   失敗。条件式は登録されなかった。
	///   - InExpressionKey に対応する条件式が既にあると失敗する。
	///   - InComparison.GetKey() に対応する状態値が
	///		InReservoir にないと失敗する。
	public: bool RegisterExpression(
		/// [in] 条件式が参照する状態貯蔵器。
		typename ThisClass::FReservoir const InReservoir,
		/// [in] 登録する条件式の識別値。
		typename ThisClass::FExpressionKey const InExpressionKey,
		/// [in] 登録する状態比較要素条件。
		typename ThisClass::FReservoir::FStatusComparison const& InComparison)
	{
		auto const LocalStatusProperty(
			InReservoir.FindProperty(InComparison.GetKey()));
		return !LocalStatusProperty.IsEmpty()
			&& this->RegisterExpression(
				LocalStatusProperty.GetChunkKey(),
				InExpressionKey,
				RulesEngine::EExpressionLogic::And,
				&InComparison,
				&InComparison + 1);
	}

	/// @brief 論理型の状態値を比較する条件式を登録する。
	/// @sa ThisClass::EvaluateExpression で、登録した条件式を評価できる。
	/// @sa ThisClass::RemoveChunk で、登録した条件式をチャンク単位で削除できる。
	/// @retval true
	///   成功。条件式を *this に登録した。
	///   条件式を登録した要素条件チャンクの識別値は、 InStatusKey
	///   に対応する状態値が登録されている状態値ビット列チャンクの識別値と同じ。
	/// @retval false
	///   失敗。条件式は登録されなかった。
	///   - InExpressionKey に対応する条件式が既にあると失敗する。
	///   - InStatusKey に対応する状態値が論理型以外だと失敗する。
	public: bool RegisterExpression(
		/// [in] 条件式が参照する状態貯蔵器。
		typename ThisClass::FReservoir const& InReservoir,
		/// [in] 登録する条件式の識別値。
		typename ThisClass::FExpressionKey const InExpressionKey,
		/// [in] 評価する論理型の状態値の識別値。
		typename ThisClass::FReservoir::FStatusKey const InStatusKey,
		/// [in] 条件となる論理値。
		bool const InCondition)
	{
		return
			InReservoir.FindKind(InStatusKey) == RulesEngine::EStatusKind::Bool
			&& this->RegisterExpression(
				InReservoir,
				InExpressionKey,
				typename ThisClass::FReservoir::FStatusComparison(
					InStatusKey,
					InCondition?
						ThisClass::FReservoir::FStatusValue::EComparison::NotEqual:
						ThisClass::FReservoir::FStatusValue::EComparison::Equal,
					typename ThisClass::FReservoir::FStatusValue(false)));
	}

	/// @brief 条件式を取得する。
	/// @return
	///   InExpressionKey に対応する ThisClass::FExpression のコピー。
	///   該当する条件式がない場合は ThisClass::FExpression::IsEmpty
	///   が真となる値を返す。
	public: typename ThisClass::FExpression FindExpression(
		/// [in] 取得する条件式に対応する識別値。
		typename ThisClass::FExpressionKey const InExpressionKey)
	const
	{
		auto const LocalFind(this->Expressions.find(InExpressionKey));
		return LocalFind != this->Expressions.end()?
			LocalFind->second:
			typename ThisClass::FExpression(
				typename ThisClass::FChunkKey(),
				RulesEngine::EExpressionLogic::Or,
				RulesEngine::EExpressionKind::SubExpression,
				0,
				0);
	}

	/// @brief 登録されている条件式を評価する。
	/// @sa ThisClass::RegisterExpression で、条件式を登録できる。
	/// @return 条件式の評価結果。
	/// - 条件式が登録されていないと、 EPsyqueKleene::TernaryUnknown となる。
	/// - 条件式が参照する状態値が登録されていないと、
	///   EPsyqueKleene::TernaryUnknown となる。
	public: EPsyqueKleene EvaluateExpression(
		/// [in] 評価する条件式に対応する識別値。
		typename ThisClass::FExpressionKey const InExpressionKey,
		/// [in] 条件式が参照する状態貯蔵器。
		typename ThisClass::FReservoir const& InReservoir)
	const
	{
		// 条件式の辞書から、該当する条件式を検索する。
		auto const local_expression_iterator(
			this->Expressions.find(InExpressionKey));
		if (local_expression_iterator == this->Expressions.end())
		{
			return EPsyqueKleene::TernaryUnknown;
		}
		auto const& LocalExpression(local_expression_iterator->second);
		auto const LocalChunk(
			this->_find_chunk(LocalExpression.GetChunkKey()));
		if (LocalChunk == nullptr)
		{
			// 条件式があれば、要素条件チャンクもあるはず。
			check(false);
			return EPsyqueKleene::TernaryUnknown;
		}

		// 条件式の種別によって評価方法を分岐する。
		switch (LocalExpression.GetKind())
		{
			// 複合条件式を評価する。
			case RulesEngine::EExpressionKind::SubExpression:
			using FSubExpression =
				typename ThisClass::FChunk::FSubExpressionArray::value_type;
			return LocalExpression.Evaluate(
				LocalChunk->SubExpressions,
				[&InReservoir, this](FSubExpression const& InExpression)
				->EPsyqueKleene
				{
					auto const LocalEvaluateExpression(
						this->EvaluateExpression(
							InExpression.GetKey(), InReservoir));
					if (LocalEvaluateExpression == EPsyqueKleene::TernaryUnknown)
					{
						return EPsyqueKleene::TernaryUnknown;
					}
					return static_cast<EPsyqueKleene>(
						InExpression.CompareCondition(
							LocalEvaluateExpression == EPsyqueKleene::TernaryTrue));
				});

			// 状態変化条件式を評価する。
			case RulesEngine::EExpressionKind::StatusTransition:
			using FStatusTransition =
				typename ThisClass::FChunk::FStatusTransitionArray::value_type;
			return LocalExpression.Evaluate(
				LocalChunk->StatusTransitions,
				[&InReservoir](FStatusTransition const& InTransition)
				->EPsyqueKleene
				{
					return InReservoir.FindTransition(InTransition.GetKey());
				});

			// 状態比較条件式を評価する。
			case RulesEngine::EExpressionKind::StatusComparison:
			using FStatusComparison =
				typename ThisClass::FChunk::FStatusComparisonArray::value_type;
			return LocalExpression.Evaluate(
				LocalChunk->StatusComparisons,
				[&InReservoir](FStatusComparison const& InComparison)
				->EPsyqueKleene
				{
					return InReservoir.CompareStatus(InComparison);
				});

			// 条件式の種別が未知だった。
			default:
			check(false);
			return EPsyqueKleene::TernaryUnknown;
		}
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 要素条件チャンク
	/// @{

	/// @brief 要素条件チャンクを予約する。
	public: void ReserveChunk(
		/// [in] 予約する要素条件チャンクに対応する識別値。
		typename ThisClass::FChunkKey const InChunkKey,
		/// [in] 複合条件式の要素条件の予約数。
		std::size_t const InSubExpressionCapacity,
		/// [in] 状態変更条件式の要素条件の予約数。
		std::size_t const InStatusTransitionCapacity,
		/// [in] 状態比較条件式の要素条件の予約数。
		std::size_t const InStatusComparisonCapacity)
	{
		ThisClass::ReserveChunk(
			this->Chunks,
			InChunkKey,
			InSubExpressionCapacity,
			InStatusTransitionCapacity,
			InStatusComparisonCapacity);
	}

	/// @brief 要素条件チャンクと、それを使っている条件式を破棄する。
	/// @retval true  成功。チャンクを破棄した。
	/// @retval false 失敗。 InChunkKey に対応するチャンクがない。
	public: bool RemoveChunk(
		/// [in] 破棄する要素条件チャンクに対応する識別値。
		typename ThisClass::FChunkKey const InChunkKey)
	{
		// 要素条件チャンクを削除する。
		if (this->Chunks.erase(InChunkKey) == 0)
		{
			return false;
		}

		// 条件式を削除する。
		for (
			auto i(this->Expressions.begin());
			i != this->Expressions.end();)
		{
			if (InChunkKey != i->second.GetChunkKey())
			{
				++i;
			}
			else
			{
				i = this->Expressions.erase(i);
			}
		}
		return true;
	}

	/// @brief 要素条件チャンクを取得する。
	/// @warning Psyque::RulesEngine 管理者以外は、この関数は使用禁止。
	/// @return
	/// InChunkKey に対応する要素条件チャンクを指すポインタ。
	/// 該当する要素条件チャンクがない場合は nullptr を返す。
	public: typename ThisClass::FChunk const* _find_chunk(
		/// [in] 取得する要素条件チャンクに対応する識別値。
		typename ThisClass::FChunkKey const InChunkKey)
	const
	{
		auto const local_iterator(this->Chunks.find(InChunkKey));
		return local_iterator != this->Chunks.end()?
			&local_iterator->second: nullptr;
	}
	/// @}
	//-------------------------------------------------------------------------
	private: static std::pair<
		 RulesEngine::EExpressionKind,
		 typename ThisClass::FChunk::FSubExpressionArray*>
	MakeElementKind(
		typename ThisClass::FChunk& InChunk,
		typename ThisClass::FChunk::FSubExpressionArray::value_type const&)
	{
		return std::make_pair(
			RulesEngine::EExpressionKind::SubExpression,
			&InChunk.SubExpressions);
	}

	private: static std::pair<
		 RulesEngine::EExpressionKind,
		 typename ThisClass::FChunk::FStatusTransitionArray*>
	MakeElementKind(
		typename ThisClass::FChunk& InChunk,
		typename ThisClass::FChunk::FStatusTransitionArray::value_type const&)
	{
		return std::make_pair(
			RulesEngine::EExpressionKind::StatusTransition,
			&InChunk.StatusTransitions);
	}

	private: static std::pair<
		 RulesEngine::EExpressionKind,
		 typename ThisClass::FChunk::FStatusComparisonArray*>
	MakeElementKind(
		typename ThisClass::FChunk& InChunk,
		typename ThisClass::FChunk::FStatusComparisonArray::value_type const&)
	{
		return std::make_pair(
			RulesEngine::EExpressionKind::StatusComparison,
			&InChunk.StatusComparisons);
	}

	//-------------------------------------------------------------------------
	private: template<typename TemplateElementIterator>
	static bool IsValidElements(
		TemplateElementIterator const& InBeginIterator,
		TemplateElementIterator const& InEndIterator,
		typename ThisClass::FExpressionMap const& InExpressions)
	{
		for (auto i(InBeginIterator); i != InEndIterator; ++i)
		{
			if (!ThisClass::IsValidElement(*i, InExpressions))
			{
				return false;
			}
		}
		return true;
	}

	private: static bool IsValidElement(
		typename ThisClass::FChunk::FSubExpressionArray::value_type const&
			InSubExpression,
		typename ThisClass::FExpressionMap const& InExpressions)
	{
		// 要素条件にある条件式がすでにあることを確認する。
		auto const local_validation(
			InExpressions.find(InSubExpression.GetKey())
			!= InExpressions.end());
		check(local_validation);
		return local_validation;
	}

	private: template<typename TemplateElement>
	static bool IsValidElement(
		TemplateElement const&,
		typename ThisClass::FExpressionMap const&)
	{
		return true;
	}

	//-------------------------------------------------------------------------
	/// @brief 要素条件チャンクを予約する。
	private: static typename ThisClass::FChunk& ReserveChunk(
		/// [in] 予約する要素条件チャンクの辞書。
		typename ThisClass::FChunkMap& OutChunks,
		/// [in] 予約するチャンクに対応する識別値。
		typename ThisClass::FChunkKey const InChunkKey,
		/// [in] 複合条件式の要素条件の予約数。
		std::size_t const InSubExpressionCapacity,
		/// [in] 状態変更条件式の要素条件の予約数。
		std::size_t const InStatusTransitionCapacity,
		/// [in] 状態比較条件式の要素条件の予約数。
		std::size_t const InStatusComparisonCapacity)
	{
		auto const LocalEmplace(
			OutChunks.emplace(
				InChunkKey,
				typename ThisClass::FChunkMap::mapped_type(
					OutChunks.get_allocator())));
		auto& LocalChunk(LocalEmplace.first->second);
		LocalChunk.SubExpressions.reserve(InSubExpressionCapacity);
		LocalChunk.StatusTransitions.reserve(InStatusTransitionCapacity);
		LocalChunk.StatusComparisons.reserve(InStatusComparisonCapacity);
		return LocalChunk;
	}

	//-------------------------------------------------------------------------
	/// @brief 要素条件チャンクの辞書。
	private: typename ThisClass::FChunkMap Chunks;
	/// @brief 条件式の辞書。
	private: typename ThisClass::FExpressionMap Expressions;

}; // class Psyque::RulesEngine::_private::TEvaluator

// vim: set noexpandtab:
