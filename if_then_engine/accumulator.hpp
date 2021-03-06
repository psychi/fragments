﻿/// @file
/// @brief @copybrief psyq::if_then_engine::_private::accumulator
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_ACCUMULATOR_HPP_
#define PSYQ_IF_THEN_ENGINE_ACCUMULATOR_HPP_

#include <cstdint>
#include <vector>
#include "../assert.hpp"

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            template<typename> class accumulator;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態変更器。状態変更を予約し、バッチ処理でまとめて状態値を変更する。
/// @par 使い方の概略
/// - accumulator::accumulate で、状態変更を予約する。
///   - 実際に状態変更が適用される順序は、 this_type::delay
///     によって決める予約系列が同じなら、予約順となることが保証される。
///     予約系列が異なると、予約順となることは保証されない。
/// - accumulator::_flush で、予約した状態変更が実際に適用される。
///   - 1度の this_type::_flush
///     で1つの状態値に対し複数回の状態変更が予約されていると、
///     初回の状態変更のみが適用され、2回目以降の状態変更が次回の
///     this_type::_flush まで遅延する場合がある。
///   - 遅延するかどうかは、 this_type::accumulate に渡す
///     this_type::delay によって決まる。
/// @tparam template_reservoir @copydoc accumulator::reservoir
template<typename template_reservoir>
class psyq::if_then_engine::_private::accumulator
{
    /// @brief this が指す値の型。
    private: typedef accumulator this_type;

    //-------------------------------------------------------------------------
    /// @brief 状態変更を適用する _private::reservoir 。
    public: typedef template_reservoir reservoir;

    /// @brief 状態変更器で使うメモリ割当子の型。
    public: typedef
        typename this_type::reservoir::allocator_type
        allocator_type;

    /// @brief 状態変更の予約系列と遅延方法。
    /// @details
    /// 1度の this_type::_flush で状態変更を適用する際に、
    /// 1つの状態値に対して異なる予約系列から複数回の状態変更がある場合の、
    /// 2回目以降の状態変更の遅延方法を決める。
    /// this_type::delay_FOLLOW と this_type::delay_YIELD の使用を推奨する。
    public: enum delay: std::uint8_t
    {
        /// 予約系列を切り替えず、
        /// 直前の状態変更の予約と同じタイミングで状態変更を適用する。
        delay_FOLLOW,
        /// 予約系列を切り替える。1度の this_type::_flush
        /// で、対象となる状態値が既に変更されていた場合、
        /// 同じ予約系列の状態変更の適用が次回以降の
        /// this_type::_flush まで遅延する。
        delay_YIELD,
        /// 予約系列を切り替える。1度の this_type::_flush
        /// で、対象となる状態値が既に変更されていた場合、
        /// 以後にある全ての状態変更の適用が次回以降の
        /// this_type::_flush まで遅延する。
        /// @warning
        /// this_type::delay_BLOCK を this_type::accumulate に何度も渡すと、
        /// 状態変更の予約がどんどん蓄積する場合があるので、注意すること。
        delay_BLOCK,
        /// 予約系列を切り替える。1度の this_type::_flush
        /// で、対象となる状態値が既に変更されていた場合でも、
        /// 遅延せずに状態変更を適用する。
        /// @warning
        /// this_type::delay_NONBLOCK を this_type::accumulate に渡すと、
        /// それ以前の状態変更が無視されることになるので、注意すること。
        delay_NONBLOCK,
    };

    //-------------------------------------------------------------------------
    /// @brief 状態変更予約のコンテナ。
    private: typedef
        std::vector<
            std::pair<
                typename this_type::reservoir::status_assignment,
                typename this_type::delay>,
            typename this_type::allocator_type>
        status_container;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    /// @{

    /// @brief 空の状態変更器を構築する。
    public: explicit accumulator(
        /// [in] 状態値の予約数。
        typename this_type::status_container::size_type const in_reserve_statuses,
        /// [in] 使用するメモリ割当子の初期値。
        typename this_type::allocator_type const& in_allocator =
            allocator_type())
    :
    accumulated_statuses_(in_allocator),
    delay_statuses_(in_allocator)
    {
        this->accumulated_statuses_.reserve(in_reserve_statuses);
        this->delay_statuses_.reserve(in_reserve_statuses);
    }

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /// @brief ムーブ構築子。
    public: accumulator(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source):
    accumulated_statuses_(std::move(io_source.accumulated_statuses_)),
    delay_statuses_(std::move(io_source.delay_statuses_))
    {}

    /// @brief ムーブ代入演算子。
    /// @return *this
    public: this_type& operator=(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source)
    {
        this->accumulated_statuses_ = std::move(io_source.accumulated_statuses_);
        this->delay_statuses_ = std::move(io_source.delay_statuses_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /// @brief 状態変更器で使われているメモリ割当子を取得する。
    /// @return 状態変更器で使われているメモリ割当子。
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->accumulated_statuses_.get_allocator();
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 状態変更
    /// @{

    /// @brief 状態変更の予約数を取得する。
    /// @return 状態変更の予約数。
    public: std::size_t count_accumulation() const PSYQ_NOEXCEPT
    {
        return this->accumulated_statuses_.size();
    }

    /// @brief 状態変更を予約する。
    /// @sa 実際の状態変更は this_type::_flush で適用される。
    /// @warning
    /// 1つの予約系列で reservoir::assign_status に失敗が発生すると、
    /// その予約系列の以降の状態変更はキャンセルされ、次の予約系列に移行する。
    public: void accumulate(
        /// [in] 予約する状態変更。
        typename this_type::reservoir::status_assignment const& in_assignment,
        /// [in] 予約系列の切り替えと遅延方法の指定。
        typename this_type::delay const in_delay)
    {
        this->accumulated_statuses_.emplace_back(in_assignment, in_delay);
    }

    /// @copydoc this_type::accumulate
    public: template<typename template_container>
    void accumulate(
        /// [in] 予約する reservoir::status_assignment のコンテナ。
        template_container const& in_assignments,
        /// [in] 予約系列の切り替えと遅延方法の指定。
        typename this_type::delay const in_delay)
    {
        auto local_delay(in_delay);
        for (auto& local_assignment: in_assignments)
        {
            this->accumulate(local_assignment, local_delay);
            local_delay = this_type::delay_FOLLOW;
        }
    }

    /// @copydoc this_type::accumulate
    public: template<typename template_value>
    void accumulate(
        /// [in] 変更する状態値の識別値。
        typename this_type::reservoir::status_key const& in_key,
        /// [in] 状態値に設定する値。
        template_value const in_value,
        /// [in] 予約系列の切り替えと遅延方法の指定。
        typename this_type::delay const in_delay)
    {
        this->accumulate(
            typename this_type::reservoir::status_assignment(
                in_key,
                this_type::reservoir::status_value::assignment_COPY,
                typename this_type::reservoir::status_value(in_value)),
            in_delay);
    }

    /// @copydoc this_type::accumulate
    public: template<typename template_value>
    void accumulate(
        /// [in] 変更する状態値の識別値。
        typename this_type::reservoir::status_key const& in_key,
        /// [in] 代入演算子の種別。
        typename this_type::reservoir::status_value::assignment const in_operator,
        /// [in] 代入演算子の右辺。
        template_value const in_value,
        /// [in] 予約系列の切り替えと遅延方法の指定。
        typename this_type::delay const in_delay)
    {
        this->accumulate(
            typename this_type::reservoir::status_assignment(
                in_key,
                in_operator,
                typename this_type::reservoir::status_value(in_value)),
            in_delay);
    }

    /// @brief psyq::if_then_engine 管理者以外は、この関数は使用禁止。
    /// @details this_type::accumulate で予約した状態変更を、実際に適用する。
    public: void _flush(
        /// [in,out] 状態変更を適用する状態貯蔵器。
        typename this_type::reservoir& io_reservoir)
    {
        auto const local_end(this->accumulated_statuses_.cend());
        for (auto i(this->accumulated_statuses_.cbegin()); i != local_end;)
        {
            // 同じ予約系列の末尾を決定する。
            auto const local_nonblock(i->second == this_type::delay_NONBLOCK);
            auto local_flush(!local_nonblock);
            auto j(i);
            for (;;)
            {
                if (local_flush
                    && 0 < io_reservoir.find_transition(j->first.get_key()))
                {
                    // すでに状態変更されていたら、今回は状態変更しない。
                    local_flush = false;
                }
                ++j;
                if (j == local_end || j->second != this_type::delay_FOLLOW)
                {
                    break;
                }
            }

            // 同じ予約系列の状態変更をまとめて適用する。
            if (local_nonblock || local_flush)
            {
                for (; i != j; ++i)
                {
                    if (!io_reservoir.assign_status(i->first))
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
                if (i->second == this_type::delay_BLOCK)
                {
                    // ブロックする場合は、残り全部を次回以降に遅延する。
                    j = local_end;
                }
                this->delay_statuses_.insert(this->delay_statuses_.end(), i, j);
                i = j;
            }
        }
        this->accumulated_statuses_.clear();
        this->accumulated_statuses_.swap(this->delay_statuses_);
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @brief 予約された状態変更のコンテナ。
    private: typename this_type::status_container accumulated_statuses_;
    /// @brief 次回以降に遅延させる状態変更のコンテナ。
    private: typename this_type::status_container delay_statuses_;

}; // class psyq::if_then_engine::_private::accumulator

#endif // !defined(PSYQ_IF_THEN_ENGINE_ACCUMULATOR_HPP_)
// vim: set expandtab:
