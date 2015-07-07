/** @file
    @brief @copybrief psyq::scenario_engine::_private::modifier
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_MODIFIER_HPP_
#define PSYQ_SCENARIO_ENGINE_MODIFIER_HPP_

#include <cstdint>
#include <vector>
#include "../assert.hpp"

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        namespace _private
        {
            template<typename> class modifier;
        } // namespace _private
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief シナリオ状態変更器。バッチ処理で状態値の変更を行う。

    ### 使い方の概略
    - modifier::accumulate で、状態変更を予約する。
    - modifier::_modify で、状態変更を実際に適用する。

    @tparam template_reservoir @copydoc modifier::reservoir
 */
template<typename template_reservoir>
class psyq::scenario_engine::_private::modifier
{
    /// @brief thisが指す値の型。
    private: typedef modifier this_type;

    /** @brief 状態変更を適用する状態貯蔵器の型。

        psyq::scenario_engine::_private::reservoir と互換性があること。
     */
    public: typedef template_reservoir reservoir;

    /// @brief 状態変更器で使うメモリ割当子の型。
    public: typedef
        typename this_type::reservoir::allocator_type
        allocator_type;

    /** @brief 状態変更の遅延方法。

        this_type::_modify で状態変更を適用する際に、
        すでに状態変更されていた場合の遅延方法を決める。
     */
    public: enum delay: std::uint8_t
    {
        /** 系列が切り替わり、以後の系列が切り替わるまでの状態変更予約の適用が、
            次回以降の this_type::_modify まで遅延する。
         */
        delay_NONBLOCK,

        /** 系列が切り替わり、以後にある全ての状態変更予約の適用が、
            次回以降の this_type::_modify まで遅延する。
         */
        delay_BLOCK,

        /** 系列は切り替わらず、
            直前の予約とタイミングになるまで状態変更予約の適用を遅延する。
         */
        delay_FOLLOW,
    };

    /// @brief 状態変更の予約。
    private: class state_reservation
    {
        public: state_reservation(
            typename modifier::reservoir::state_assignment const& in_assignment,
            bool const in_series,
            bool const in_block)
        PSYQ_NOEXCEPT:
        assignment_(in_assignment),
        series_(in_series),
        block_(in_block)
        {}

        /// @brief 適用する代入演算。
        public: typename modifier::reservoir::state_assignment assignment_;
        /// @brief 状態変更の系列の識別値。
        public: bool series_;
        /// @brief 状態変更の遅延方法。
        public: bool block_;

    }; // class state_reservation

    private: typedef
        std::vector<
            typename this_type::state_reservation,
            typename this_type::allocator_type>
        state_container;

    //-------------------------------------------------------------------------
    /** @name 構築と代入
        @{
     */
    /** @brief 空の状態変更器を構築する。
        @param[in] in_reserve_states 状態値の予約数。
        @param[in] in_allocator      使用するメモリ割当子の初期値。
     */
    public: explicit modifier(
        typename this_type::state_container::size_type const in_reserve_states,
        typename this_type::allocator_type const& in_allocator =
            allocator_type())
    :
    accumulated_states_(in_allocator),
    pass_states_(in_allocator)
    {
        this->accumulated_states_.reserve(in_reserve_states);
        this->pass_states_.reserve(in_reserve_states);
    }

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: modifier(this_type&& io_source):
    accumulated_states_(std::move(io_source.accumulated_states_)),
    pass_states_(std::move(io_source.pass_states_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->accumulated_states_ = std::move(io_source.accumulated_states_);
        this->pass_states_ = std::move(io_source.pass_states_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /** @brief 状態変更器で使われているメモリ割当子を取得する。
        @return 状態変更器で使われているメモリ割当子。
     */
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->accumulated_states_.get_allocator();
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 状態変更
        @{
     */
    /** @brief 状態変更を予約する。

        実際の状態変更は this_type::_modify で適用される。

        @param[in] in_assignment  予約する状態変更。
        @param[in] in_delay       状態変更の遅延方法。

        @warning
        this_type::_modify では、異なる系列での状態変更を重複させないために、
        状態変更の適用を遅延させる場合がある。
        このため1つの状態値に対し、異なる複数の系列から this_type::accumulate
        を毎フレーム行うと、状態変更の予約が蓄積して増え続ける。
        1つの状態値に対し、異なる系列から複数回の状態変更を毎フレームする場合は、
        reservoir::assign_state で直接状態変更するべき。
     */
    public: void accumulate(
        typename this_type::reservoir::state_assignment const& in_assignment,
        typename this_type::delay const in_delay = this_type::delay_NONBLOCK)
    {
        this->accumulated_states_.emplace_back(
            in_assignment,
            this->accumulated_states_.empty()
            || this->accumulated_states_.back().series_ ^ (
                in_delay != this_type::delay_FOLLOW),
            in_delay == this_type::delay_BLOCK);
    }

    /** @brief psyq::scenario_engine 管理者以外は、この関数は使用禁止。

        this_type::accumulate で予約した状態変更を、実際に適用する。

        1度の this_type::_modify で、
        1つの状態値に対して異なる複数の系列から状態変更の予約があった場合は、
        最初の系列の状態変更のみが適用され、それより後の系列の状態変更の予約は、
        次回以降の this_type::_modify まで遅延させる。

        @param[in,out] io_reservoir 状態変更を適用する状態貯蔵器。
     */
    public: void _modify(typename this_type::reservoir& io_reservoir)
    {
        auto const local_end(this->accumulated_states_.cend());
        for (auto i(this->accumulated_states_.cbegin()); i != local_end;)
        {
            // 同系列の状態変更の末尾を検知する。
            auto local_modify(true);
            auto j(i);
            for (; j != local_end && i->series_ == j->series_; ++j)
            {
                if (local_modify
                    && 0 < io_reservoir._get_transition(j->assignment_.key_))
                {
                    // すでに状態変更されていたら、今回は状態変更しない。
                    local_modify = false;
                }
            }

            // 同系列の状態変更をまとめて適用する。
            if (local_modify)
            {
                for (; i != j; ++i)
                {
                    if (!io_reservoir.assign_state(i->assignment_))
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
                // 状態変更を次回に繰り越す。
                auto const local_series(
                    this->pass_states_.empty()
                    || this->pass_states_.back().series_ == i->series_);
                if (i->block_)
                {
                    // ブロックする場合は、残り全部を次回以降に遅延する。
                    j = local_end;
                }
                for (; i != j; ++i)
                {
                    this->pass_states_.push_back(
                        typename this_type::state_reservation(
                            i->assignment_,
                            i->series_ ^ local_series,
                            i->block_));
                }
            }
        }
        this->accumulated_states_.clear();
        this->accumulated_states_.swap(this->pass_states_);
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @brief 予約された状態変更のコンテナ。
    private: typename this_type::state_container accumulated_states_;
    /// @brief 次回に行う状態変更のコンテナ。
    private: typename this_type::state_container pass_states_;

}; // class psyq::scenario_engine::_private::modifier

#endif // !defined(PSYQ_SCENARIO_ENGINE_MODIFIER_HPP_)
// vim: set expandtab:
