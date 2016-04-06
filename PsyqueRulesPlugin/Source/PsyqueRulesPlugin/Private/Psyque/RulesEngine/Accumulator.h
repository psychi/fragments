// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::TAccumulator
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <cstdint>
#include <vector>
#include "../Assert.h"
#include "./Enum.h"

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		namespace _private
		{
			template<typename> class TAccumulator;
		} // namespace _private
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態変更器。状態変更を予約し、バッチ処理でまとめて状態値を変更する。
/// @par 使い方の概略
/// - TAccumulator::Accumulate で、状態変更を予約する。
///   - 実際に状態変更が適用される順序は、 EAccumulationDelay
///     によって決める予約系列が同じなら、予約順となることが保証される。
///     予約系列が異なると、予約順となることは保証されない。
/// - TAccumulator::_flush で、予約した状態変更が実際に適用される。
///   - 1度の ThisClass::_flush
///     で1つの状態値に対し複数回の状態変更が予約されていると、
///     初回の状態変更のみが適用され、2回目以降の状態変更が次回の
///     ThisClass::_flush まで遅延する場合がある。
///   - 遅延するかどうかは、
///     ThisClass::Accumulate に渡す EAccumulationDelay によって決まる。
/// @tparam TemplateReservoir @copydoc TAccumulator::FReservoir
template<typename TemplateReservoir>
class Psyque::RulesEngine::_private::TAccumulator
{
	private: using ThisClass = TAccumulator; ///< @copydoc TReservoir::ThisClass

	//-------------------------------------------------------------------------
	/// @brief 状態変更を適用する TReservoir の型。
	public: using FReservoir = TemplateReservoir;

	/// @brief 状態変更器で使うメモリ割当子の型。
	public: using FAllocator = typename ThisClass::FReservoir::FAllocator;

	//-------------------------------------------------------------------------
	/// @brief 状態変更予約のコンテナ。
	private: using FStatusArray = std::vector<
		std::pair<
			typename ThisClass::FReservoir::FStatusAssignment,
			typename EAccumulationDelay>,
		typename ThisClass::FAllocator>;

	//-------------------------------------------------------------------------
	/// @name 構築と代入
	/// @{

	/// @brief 空の状態変更器を構築する。
	public: explicit TAccumulator(
		/// [in] 状態値の予約数。
		typename ThisClass::FStatusArray::size_type const InStatusCapacity,
		/// [in] 使用するメモリ割当子の初期値。
		typename ThisClass::FAllocator const& InAllocator = FAllocator()):
	AccumulatedStatuses(InAllocator),
	DelayStatuses(InAllocator)
	{
		this->AccumulatedStatuses.reserve(InStatusCapacity);
		this->DelayStatuses.reserve(InStatusCapacity);
	}

#ifdef PSYQUE_NO_STD_DEFAULTED_FUNCTION
	/// @brief ムーブ構築子。
	public: TAccumulator(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource):
	AccumulatedStatuses(MoveTemp(OutSource.AccumulatedStatuses)),
	DelayStatuses(MoveTemp(OutSource.DelayStatuses))
	{}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: ThisClass& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource)
	{
		this->AccumulatedStatuses = MoveTemp(OutSource.AccumulatedStatuses);
		this->DelayStatuses = MoveTemp(OutSource.DelayStatuses);
		return *this;
	}
#endif // defined(PSYQUE_NO_STD_DEFAULTED_FUNCTION)

	/// @brief 状態変更器で使われているメモリ割当子を取得する。
	/// @return 状態変更器で使われているメモリ割当子。
	public: typename ThisClass::FAllocator get_allocator()
	const PSYQUE_NOEXCEPT
	{
		return this->AccumulatedStatuses.get_allocator();
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態変更
	/// @{

	/// @brief 状態変更の予約数を取得する。
	/// @return 状態変更の予約数。
	public: std::size_t CountAccumulations() const PSYQUE_NOEXCEPT
	{
		return this->AccumulatedStatuses.size();
	}

	/// @brief 状態変更を予約する。
	/// @sa 実際の状態変更は ThisClass::_flush で適用される。
	/// @warning
	/// 1つの予約系列で TReservoir::AssignStatus に失敗が発生すると、
	/// その予約系列の以降の状態変更はキャンセルされ、次の予約系列に移行する。
	public: void Accumulate(
		/// [in] 予約する状態変更。
		typename ThisClass::FReservoir::FStatusAssignment const& InAssignment,
		/// [in] 予約系列の切り替えと遅延方法の指定。
		typename EAccumulationDelay const InDelay)
	{
		this->AccumulatedStatuses.emplace_back(InAssignment, InDelay);
	}

	/// @copydoc ThisClass::Accumulate
	public: template<typename TemplateAssignmentArray>
	void Accumulate(
		/// [in] 予約する TReservoir::FStatusAssignment のコンテナ。
		TemplateAssignmentArray const& InAssignments,
		/// [in] 予約系列の切り替えと遅延方法の指定。
		typename EAccumulationDelay const InDelay)
	{
		auto LocalDelay(InDelay);
		for (auto& LocalAssignment: InAssignments)
		{
			this->Accumulate(LocalAssignment, LocalDelay);
			LocalDelay = EAccumulationDelay::Follow;
		}
	}

	/// @copydoc ThisClass::Accumulate
	public: template<typename TemplateValue>
	void Accumulate(
		/// [in] 変更する状態値の識別値。
		typename ThisClass::FReservoir::FStatusKey const InKey,
		/// [in] 状態値に設定する値。
		TemplateValue const InValue,
		/// [in] 予約系列の切り替えと遅延方法の指定。
		typename EAccumulationDelay const InDelay)
	{
		this->Accumulate(
			typename ThisClass::FReservoir::FStatusAssignment(
				InKey,
				RulesEngine::EStatusAssignment::Copy,
				typename ThisClass::FReservoir::FStatusValue(InValue)),
			InDelay);
	}

	/// @copydoc ThisClass::Accumulate
	public: template<typename TemplateValue>
	void Accumulate(
		/// [in] 変更する状態値の識別値。
		typename ThisClass::FReservoir::FStatusKey const InKey,
		/// [in] 代入演算子の種別。
		typename RulesEngine::EStatusAssignment const InOperator,
		/// [in] 代入演算子の右辺。
		TemplateValue const InValue,
		/// [in] 予約系列の切り替えと遅延方法の指定。
		typename EAccumulationDelay const InDelay)
	{
		this->Accumulate(
			typename ThisClass::FReservoir::FStatusAssignment(
				InKey,
				InOperator,
				typename ThisClass::FReservoir::FStatusValue(InValue)),
			InDelay);
	}

	/// @brief Psyque::RulesEngine 管理者以外は、この関数は使用禁止。
	/// @details ThisClass::Accumulate で予約した状態変更を、実際に適用する。
	public: void _flush(
		/// [in,out] 状態変更を適用する状態貯蔵器。
		typename ThisClass::FReservoir& OutReservoir)
	{
		auto const LocalEnd(this->AccumulatedStatuses.cend());
		for (auto i(this->AccumulatedStatuses.cbegin()); i != LocalEnd;)
		{
			// 同じ予約系列の末尾を決定する。
			auto const LocalNonblock(i->second == EAccumulationDelay::Nonblock);
			auto LocalFlush(!LocalNonblock);
			auto j(i);
			for (;;)
			{
				if (LocalFlush
					&& OutReservoir.FindTransition(j->first.GetKey())
						== EPsyqueKleene::TernaryTrue)
				{
					// すでに状態変更されていたら、今回は状態変更しない。
					LocalFlush = false;
				}
				++j;
				if (j == LocalEnd || j->second != EAccumulationDelay::Follow)
				{
					break;
				}
			}

			// 同じ予約系列の状態変更をまとめて適用する。
			if (LocalNonblock || LocalFlush)
			{
				for (; i != j; ++i)
				{
					if (!OutReservoir.AssignStatus(i->first))
					{
						// 状態変更に失敗した場合は、
						// 同じ予約系列の以後の状態変更を行わない。
						i = j;
						break;
					}
				}
			}
			else
			{
				// 状態変更を次回まで遅延する。
				if (i->second == EAccumulationDelay::Block)
				{
					// ブロックする場合は、残り全部を次回以降に遅延する。
					j = LocalEnd;
				}
				this->DelayStatuses.insert(this->DelayStatuses.end(), i, j);
				i = j;
			}
		}
		this->AccumulatedStatuses.clear();
		this->AccumulatedStatuses.swap(this->DelayStatuses);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @brief 予約された状態変更のコンテナ。
	private: typename ThisClass::FStatusArray AccumulatedStatuses;
	/// @brief 次回以降に遅延させる状態変更のコンテナ。
	private: typename ThisClass::FStatusArray DelayStatuses;

}; // class Psyque::RulesEngine::_private::TAccumulator

// vim: set noexpandtab:
