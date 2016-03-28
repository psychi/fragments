﻿// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TAccumulator
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <cstdint>
#include <vector>
#include "./Assert.h"

/// @cond
namespace Psyque
{
	namespace RuleEngine
	{
		namespace _private
		{
			template<typename> class TAccumulator;
		} // namespace _private
	} // namespace RuleEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態変更器。状態変更を予約し、バッチ処理でまとめて状態値を変更する。
/// @par 使い方の概略
/// - TAccumulator::Accumulate で、状態変更を予約する。
///   - 実際に状態変更が適用される順序は、 This::EDelay
///     によって決める予約系列が同じなら、予約順となることが保証される。
///     予約系列が異なると、予約順となることは保証されない。
/// - TAccumulator::_flush で、予約した状態変更が実際に適用される。
///   - 1度の This::_flush
///     で1つの状態値に対し複数回の状態変更が予約されていると、
///     初回の状態変更のみが適用され、2回目以降の状態変更が次回の
///     This::_flush まで遅延する場合がある。
///   - 遅延するかどうかは、 This::Accumulate に渡す
///     This::EDelay::Type によって決まる。
/// @tparam TemplateReservoir @copydoc TAccumulator::FReservoir
template<typename TemplateReservoir>
class Psyque::RuleEngine::_private::TAccumulator
{
	private: using This = TAccumulator; ///< @copydoc TReservoir::This

	//-------------------------------------------------------------------------
	/// @brief 状態変更を適用する TReservoir の型。
	public: using FReservoir = TemplateReservoir;

	/// @brief 状態変更器で使うメモリ割当子の型。
	public: using FAllocator = typename This::FReservoir::FAllocator;

	/// @brief 状態変更の予約系列と遅延方法。
	/// @details
	///   1度の This::_flush で状態変更を適用する際に、
	///   1つの状態値に対して異なる予約系列から複数回の状態変更がある場合の、
	///   2回目以降の状態変更の遅延方法を決める。
	///   This::EDelay::Follow と This::EDelay::Yield の使用を推奨する。
	public: struct EDelay
	{
		enum Type: uint8
		{
			/// 予約系列を切り替えず、
			/// 直前の状態変更の予約と同じタイミングで状態変更を適用する。
			Follow,
			/// 予約系列を切り替える。1度の This::_flush
			/// で、対象となる状態値が既に変更されていた場合、
			/// 同じ予約系列の状態変更の適用が次回以降の
			/// This::_flush まで遅延する。
			Yield,
			/// 予約系列を切り替える。1度の This::_flush
			/// で、対象となる状態値が既に変更されていた場合、
			/// 以後にある全ての状態変更の適用が次回以降の
			/// This::_flush まで遅延する。
			/// @warning
			///   This::EDelay::Block を This::Accumulate に何度も渡すと、
			///   状態変更の予約がどんどん蓄積する場合があるので、注意すること。
			Block,
			/// 予約系列を切り替える。1度の This::_flush
			/// で、対象となる状態値が既に変更されていた場合でも、
			/// 遅延せずに状態変更を適用する。
			/// @warning
			///   This::EDelay::Nonblock を This::Accumulate に渡すと、
			///   それ以前の状態変更が無視されることになるので、注意すること。
			Nonblock,
		};
	};

	//-------------------------------------------------------------------------
	/// @brief 状態変更予約のコンテナ。
	private: using FStatusArray = std::vector<
		std::pair<
			typename This::FReservoir::FStatusAssignment,
			typename This::EDelay::Type>,
		typename This::FAllocator>;

	//-------------------------------------------------------------------------
	/// @name 構築と代入
	/// @{

	/// @brief 空の状態変更器を構築する。
	public: explicit TAccumulator(
		/// [in] 状態値の予約数。
		typename This::FStatusArray::size_type const InStatusCapacity,
		/// [in] 使用するメモリ割当子の初期値。
		typename This::FAllocator const& InAllocator = FAllocator()):
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
		This&& OutSource):
	AccumulatedStatuses(MoveTemp(OutSource.AccumulatedStatuses)),
	DelayStatuses(MoveTemp(OutSource.DelayStatuses))
	{}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: This& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		This&& OutSource)
	{
		this->AccumulatedStatuses = MoveTemp(OutSource.AccumulatedStatuses);
		this->DelayStatuses = MoveTemp(OutSource.DelayStatuses);
		return *this;
	}
#endif // defined(PSYQUE_NO_STD_DEFAULTED_FUNCTION)

	/// @brief 状態変更器で使われているメモリ割当子を取得する。
	/// @return 状態変更器で使われているメモリ割当子。
	public: typename This::FAllocator get_allocator()
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
	/// @sa 実際の状態変更は This::_flush で適用される。
	/// @warning
	/// 1つの予約系列で FReservoir::AssignStatus に失敗が発生すると、
	/// その予約系列の以降の状態変更はキャンセルされ、次の予約系列に移行する。
	public: void Accumulate(
		/// [in] 予約する状態変更。
		typename This::FReservoir::FStatusAssignment const& InAssignment,
		/// [in] 予約系列の切り替えと遅延方法の指定。
		typename This::EDelay::Type const InDelay)
	{
		this->AccumulatedStatuses.emplace_back(InAssignment, InDelay);
	}

	/// @copydoc This::Accumulate
	public: template<typename TemplateAssignmentArray>
	void Accumulate(
		/// [in] 予約する TReservoir::FStatusAssignment のコンテナ。
		TemplateAssignmentArray const& InAssignments,
		/// [in] 予約系列の切り替えと遅延方法の指定。
		typename This::EDelay::Type const InDelay)
	{
		auto LocalDelay(InDelay);
		for (auto& LocalAssignment: InAssignments)
		{
			this->Accumulate(LocalAssignment, LocalDelay);
			LocalDelay = This::EDelay::Follow;
		}
	}

	/// @copydoc This::Accumulate
	public: template<typename TemplateValue>
	void Accumulate(
		/// [in] 変更する状態値の識別値。
		typename This::FReservoir::FStatusKey const InKey,
		/// [in] 状態値に設定する値。
		TemplateValue const InValue,
		/// [in] 予約系列の切り替えと遅延方法の指定。
		typename This::EDelay::Type const InDelay)
	{
		this->Accumulate(
			typename This::FReservoir::FStatusAssignment(
				InKey,
				This::FReservoir::FStatusValue::EAssignment::Copy,
				typename This::FReservoir::FStatusValue(InValue)),
			InDelay);
	}

	/// @copydoc This::Accumulate
	public: template<typename TemplateValue>
	void Accumulate(
		/// [in] 変更する状態値の識別値。
		typename This::FReservoir::FStatusKey const InKey,
		/// [in] 代入演算子の種別。
		typename This::FReservoir::FStatusValue::EAssignment::Type const InOperator,
		/// [in] 代入演算子の右辺。
		TemplateValue const InValue,
		/// [in] 予約系列の切り替えと遅延方法の指定。
		typename This::EDelay::Type const InDelay)
	{
		this->Accumulate(
			typename This::FReservoir::FStatusAssignment(
				InKey,
				InOperator,
				typename This::FReservoir::FStatusValue(InValue)),
			InDelay);
	}

	/// @brief Psyque::RuleEngine 管理者以外は、この関数は使用禁止。
	/// @details This::Accumulate で予約した状態変更を、実際に適用する。
	public: void _flush(
		/// [in,out] 状態変更を適用する状態貯蔵器。
		typename This::FReservoir& OutReservoir)
	{
		auto const LocalEnd(this->AccumulatedStatuses.cend());
		for (auto i(this->AccumulatedStatuses.cbegin()); i != LocalEnd;)
		{
			// 同じ予約系列の末尾を決定する。
			auto const LocalNonblock(i->second == This::EDelay::Nonblock);
			auto LocalFlush(!LocalNonblock);
			auto j(i);
			for (;;)
			{
				if (LocalFlush
					&& 0 < OutReservoir.FindTransition(j->first.GetKey()))
				{
					// すでに状態変更されていたら、今回は状態変更しない。
					LocalFlush = false;
				}
				++j;
				if (j == LocalEnd || j->second != This::EDelay::Follow)
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
				if (i->second == This::EDelay::Block)
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
	private: typename This::FStatusArray AccumulatedStatuses;
	/// @brief 次回以降に遅延させる状態変更のコンテナ。
	private: typename This::FStatusArray DelayStatuses;

}; // class Psyque::RuleEngine::_private::TAccumulator

// vim: set noexpandtab: