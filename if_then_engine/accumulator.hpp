/** @file
    @brief @copybrief psyq::if_then_engine::_private::accumulator
    @author Hillco Psychi (https://twitter.com/psychi)
 */
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
/** @brief 状態変更器。バッチ処理で状態値の変更を行う。

    ### 使い方の概略
    - accumulator::accumulate で、状態変更を予約する。
    - accumulator::_flush で、状態変更を実際に適用する。

    @tparam template_reservoir @copydoc accumulator::reservoir
 */
template<typename template_reservoir>
class psyq::if_then_engine::_private::accumulator
{
    /// @brief thisが指す値の型。
    private: typedef accumulator this_type;

    /** @brief 状態変更を適用する状態貯蔵器の型。

        psyq::if_then_engine::_private::reservoir と互換性があること。
     */
    public: typedef template_reservoir reservoir;

    /// @brief 状態変更器で使うメモリ割当子の型。
    public: typedef
        typename this_type::reservoir::allocator_type
        allocator_type;

    /** @brief 状態変更の予約系列と遅延方法。

        this_type::_flush で状態変更を適用する際に、
        異なる予約系列からすでに状態変更されていた場合の遅延方法を決める。
     */
    public: enum delay: std::uint8_t
    {
        /** 予約系列を切り替え、以後、同じ予約系列の状態変更の適用が、
            次回以降の this_type::_flush まで遅延する。
         */
        delay_NONBLOCK,

        /** 予約系列を切り替え、以後にある全ての状態変更の適用が、
            次回以降の this_type::_flush まで遅延する。
         */
        delay_BLOCK,

        /** 予約系列を切り替えず、直前の予約系列と同じタイミングになるまで、
            状態変更の適用を遅延する。
         */
        delay_FOLLOW,
    };

    /// @brief 状態変更の予約。
    private: class status_accumulation
    {
        public: status_accumulation(
            typename accumulator::reservoir::status_assignment in_assignment,
            bool const in_series,
            bool const in_block)
        PSYQ_NOEXCEPT:
        assignment_(std::move(in_assignment)),
        series_(in_series),
        block_(in_block)
        {}

        /// @brief 適用する代入演算。
        public: typename accumulator::reservoir::status_assignment assignment_;
        /// @brief 状態変更の系列の識別値。
        public: bool series_;
        /// @brief 状態変更の遅延方法。
        public: bool block_;

    }; // class status_accumulation

    private: typedef
        std::vector<
            typename this_type::status_accumulation,
            typename this_type::allocator_type>
        status_container;

    //-------------------------------------------------------------------------
    /** @name 構築と代入
        @{
     */
    /** @brief 空の状態変更器を構築する。
        @param[in] in_reserve_statuses 状態値の予約数。
        @param[in] in_allocator        使用するメモリ割当子の初期値。
     */
    public: explicit accumulator(
        typename this_type::status_container::size_type const in_reserve_statuses,
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
    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: accumulator(this_type&& io_source):
    accumulated_statuses_(std::move(io_source.accumulated_statuses_)),
    delay_statuses_(std::move(io_source.delay_statuses_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->accumulated_statuses_ = std::move(io_source.accumulated_statuses_);
        this->delay_statuses_ = std::move(io_source.delay_statuses_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /** @brief 状態変更器で使われているメモリ割当子を取得する。
        @return 状態変更器で使われているメモリ割当子。
     */
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->accumulated_statuses_.get_allocator();
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 状態変更
        @{
     */
    /** @brief 累積している状態変更の数を取得する。
        @return 累積している状態変更の数。
     */
    public: std::size_t count_accumulation() const PSYQ_NOEXCEPT
    {
        return this->accumulated_statuses_.size();
    }

    /** @brief 状態変更を予約する。

        実際の状態変更は this_type::_flush で適用される。

        @param[in] in_assignment 予約する状態変更。
        @param[in] in_delay      予約系列と遅延方法の指定。

        @return 累積している状態変更の数。

        @warning
        this_type::_flush では、異なる予約系列からの状態変更を重複させないため、
        状態変更の適用を遅延させる場合がある。このため、1つの状態値に対し、
        異なる複数の予約系列からの this_type::accumulate を毎フレーム行うと、
        状態変更の予約が蓄積して増え続ける。1つの状態値に対し、
        異なる複数の予約系列からの状態変更を毎フレーム行いたい場合は、
        reservoir::assign_status で直接状態変更するほうが良い。
     */
    public: std::size_t accumulate(
        typename this_type::reservoir::status_assignment in_assignment,
        typename this_type::delay const in_delay = this_type::delay_NONBLOCK)
    {
        this->accumulated_statuses_.emplace_back(
            std::move(in_assignment),
            this->accumulated_statuses_.empty()
            || this->accumulated_statuses_.back().series_ ^ (
                in_delay != this_type::delay_FOLLOW),
            in_delay == this_type::delay_BLOCK);
        return this->count_accumulation();
    }

    /** @brief 状態変更を予約する。

        実際の状態変更は this_type::_flush で適用される。

        @param[in] in_key   変更する状態値の識別値。
        @param[in] in_value 状態値に設定する値。
        @param[in] in_delay 予約系列と遅延方法の指定。

        @return 累積している状態変更の数。
     */
    public: template<typename template_value>
    std::size_t accumulate(
        typename this_type::reservoir::status_key const& in_key,
        template_value in_value,
        typename this_type::delay const in_delay = this_type::delay_NONBLOCK)
    {
        return this->accumulate(
            typename this_type::reservoir::status_assignment(
                in_key,
                this_type::reservoir::status_value::assignment_COPY,
                typename this_type::reservoir::status_value(std::move(in_value))),
            in_delay);
    }

    /** @brief 状態変更を予約する。

        実際の状態変更は this_type::_flush で適用される。

        @param[in] in_key      変更する状態値の識別値。
        @param[in] in_operator 代入演算子の種別。
        @param[in] in_value    代入演算子の右辺。
        @param[in] in_delay    予約系列と遅延方法の指定。

        @return 累積している状態変更の数。
     */
    public: template<typename template_value>
    std::size_t accumulate(
        typename this_type::reservoir::status_key const& in_key,
        typename this_type::reservoir::status_value::assignment const in_operator,
        template_value in_value,
        typename this_type::delay const in_delay = this_type::delay_NONBLOCK)
    {
        return this->accumulate(
            typename this_type::reservoir::status_assignment(
                in_key,
                in_operator,
                typename this_type::reservoir::status_value(std::move(in_value))),
            in_delay);
    }

    /** @brief psyq::if_then_engine 管理者以外は、この関数は使用禁止。

        this_type::accumulate で予約した状態変更を、実際に適用する。

        1度の this_type::_flush で、1つの状態値に対し、
        異なる複数の予約系列から状態変更がある場合は、
        最初の予約系列の状態変更のみが適用される。
        それより後の予約系列の適用は、次回の this_type::_flush まで遅延する。

        @param[in,out] io_reservoir 状態変更を適用する状態貯蔵器。
     */
    public: void _flush(typename this_type::reservoir& io_reservoir)
    {
        auto const local_end(this->accumulated_statuses_.cend());
        for (auto i(this->accumulated_statuses_.cbegin()); i != local_end;)
        {
            // 同じ予約系列の末尾を決定する。
            auto local_flush(true);
            auto j(i);
            for (; j != local_end && i->series_ == j->series_; ++j)
            {
                if (local_flush
                    && 0 < io_reservoir._get_transition(j->assignment_.get_key()))
                {
                    // すでに状態変更されていたら、今回は状態変更しない。
                    local_flush = false;
                }
            }

            // 同系列の状態変更をまとめて適用する。
            if (local_flush)
            {
                for (; i != j; ++i)
                {
                    if (!io_reservoir.assign_status(i->assignment_))
                    {
                        /** @note
                            状態変更に失敗した場合、どう対応するのがよい？
                            とりえあえずassertしておく。
                         */
                        PSYQ_ASSERT(false);
                    }
                }
            }
            else
            {
                // 状態変更を次回まで遅延するす。
                auto const local_series(
                    this->delay_statuses_.empty()
                    || this->delay_statuses_.back().series_ == i->series_);
                if (i->block_)
                {
                    // ブロックする場合は、残り全部を次回以降に遅延する。
                    j = local_end;
                }
                for (; i != j; ++i)
                {
                    this->delay_statuses_.push_back(
                        typename this_type::status_accumulation(
                            i->assignment_,
                            i->series_ ^ local_series,
                            i->block_));
                }
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
