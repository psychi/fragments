/** @file
    @brief @copybrief psyq::scenario_engine::modifier
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_MODIFIER_HPP_
#define PSYQ_SCENARIO_ENGINE_MODIFIER_HPP_

/// @cond
namespace psyq
{
    /// @brief ビデオゲームでのシナリオ進行を管理するための実装
    namespace scenario_engine
    {
        template<typename, typename> class modifier;
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief シナリオ状態変更器。バッチ処理で状態値の変更を行う。

    - modifier::accumulate で、状態変更を予約する。
    - modifier::modify で、状態変更を実際に適用する。

    @tparam template_reservoir @copydoc modifier::reservoir
    @tparam template_allocator @copydoc modifier::allocator_type
 */
template<
    typename template_reservoir = psyq::scenario_engine::reservoir<>,
    typename template_allocator = typename template_reservoir::allocator_type>
class psyq::scenario_engine::modifier
{
    /// @brief thisが指す値の型。
    private: typedef modifier this_type;

    /// @brief 状態変更を適用する状態貯蔵器の型。
    public: typedef template_reservoir reservoir;

    /// @brief 状態変更器で使うメモリ割当子の型。
    public: typedef template_allocator allocator_type;

    /// @brief 状態変更の優先順位。
    public: enum priority_enum: std::int8_t
    {
        priority_NONE = -1,   ///< 優先順位がない。
        priority_CONTINUANCE, ///< 直前の優先順位を継続する。
        priority_ORDERED,     ///< 直前の優先順位の次になる。
    };

    /// @brief 状態変更のキャッシュ。
    private: struct state
    {
        state(
            typename modifier::reservoir::state_key const& in_key,
            typename modifier::reservoir::state_value const& in_value,
            bool const in_series,
            bool const in_ordered)
        PSYQ_NOEXCEPT:
        value(in_value),
        key(in_key),
        series(in_series),
        ordered(in_ordered)
        {}

        typename modifier::reservoir::state_value value;
        typename modifier::reservoir::state_key key;
        bool series;
        bool ordered;
    };

    private: typedef std::vector<
        typename this_type::state, typename this_type::allocator_type>
            state_vector;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 空の状態変更器を構築する。
        @param[in] in_reserve_states 状態値の予約数。
        @param[in] in_allocator      使用するメモリ割当子の初期値。
     */
    public: explicit modifier(
        typename this_type::state_vector::size_type const in_reserve_states,
        typename this_type::allocator_type const& in_allocator =
            allocator_type())
    :
    accumulated_states_(in_allocator),
    pass_states_(in_allocator)
    {
        this->accumulated_states_.reserve(in_reserve_states);
        this->pass_states_.reserve(in_reserve_states);
    }

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: modifier(this_type&& io_source):
    accumulated_states_(std::move(io_source.accumulated_states_)),
    pass_states_(std::move(io_source.pass_states_))
    {
        io_source.accumulated_states_.clear();
        io_source.pass_states_.clear();
    }

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->accumulated_states_ = std::move(io_source.accumulated_states_);
            this->pass_states_ = std::move(io_source.pass_states_);
            io_source.accumulated_states_.clear();
            io_source.pass_states_.clear();
        }
        return *this;
    }

    /** @brief 状態変更器で使われているメモリ割当子を取得する。
        @return 状態変更器で使われているメモリ割当子。
     */
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->accumulated_states_.get_allocator();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 状態変更
    //@{
    /** @brief 状態変更を予約する。

        実際の状態変更は this_type::modify で適用される。

        @param[in] in_state_key   変更する状態の識別値。
        @param[in] in_state_value 設定する状態値。
        @param[in] in_priority    状態変更の優先順位。
     */
    public: void accumulate(
        typename this_type::reservoir::state_key const& in_state_key,
        typename this_type::reservoir::state_value const& in_state_value,
        typename this_type::priority_enum const in_priority =
            this_type::priority_NONE)
    {
        this->accumulated_states_.emplace_back(
            in_state_key,
            in_state_value,
            this->accumulated_states_.empty()
            || this->accumulated_states_.back().series ^ (
                in_priority != this_type::priority_CONTINUANCE),
            in_priority == this_type::priority_ORDERED);
    }

    /** @brief 状態変更する。

        this_type::accumulate で予約した状態変更を、実際に適用する。

        @param[in,out] io_reservoir 状態変更を適用する状態貯蔵器。
     */
    public: void modify(typename this_type::reservoir& io_reservoir)
    {
        auto const local_end(this->accumulated_states_.cend());
        for (auto i(this->accumulated_states_.cbegin()); i != local_end;)
        {
            // 同系列の状態変更の末尾を検知する。
            auto local_modify(true);
            auto j(i);
            for (; j != local_end && i->series == j->series; ++j)
            {
                if (local_modify && 0 < io_reservoir._get_transition(j->key))
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
                    io_reservoir.set_value(i->key, i->value);
                }
            }
            else
            {
                // 状態変更を次回に繰り越す。
                auto const local_series(
                    this->pass_states_.empty()
                    || this->pass_states_.back().series == i->series);
                if (i->ordered)
                {
                    // 順序つきの場合は、残り全部を次回に繰り越す。
                    j = local_end;
                }
                for (; i != j; ++i)
                {
                    this->pass_states_.push_back(
                        typename this_type::state(
                            i->key,
                            i->value,
                            i->series ^ local_series,
                            i->ordered));
                }
            }
        }
        this->accumulated_states_.clear();
        this->accumulated_states_.swap(this->pass_states_);
    }
    //@}

    //-------------------------------------------------------------------------
    /// @brief 予約された状態変更のコンテナ。
    private: typename this_type::state_vector accumulated_states_;
    /// @brief 次回に行う状態変更のコンテナ。
    private: typename this_type::state_vector pass_states_;

}; // class psyq::scenario_engine::modifier

#endif // !defined(PSYQ_SCENARIO_ENGINE_MODIFIER_HPP_)
// vim: set expandtab:
