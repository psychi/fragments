/// @file
/// @brief @copybrief psyq::if_then_engine::_private::status_monitor
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_MONITOR_HPP_
#define PSYQ_IF_THEN_ENGINE_STATUS_MONITOR_HPP_

#include <cstdint>
#include <vector>

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            template<typename> class status_monitor;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態監視器。条件式の要素条件が参照する状態値を監視する。
/// @details 状態値が変化した際に、条件式の評価を更新するために使う。
/// @tparam template_expression_key_container @copydoc status_monitor::expression_keys_
template<typename template_expression_key_container>
class psyq::if_then_engine::_private::status_monitor
{
    /// @brief this が指す値の型。
    private: typedef status_monitor this_type;

    //-------------------------------------------------------------------------
    /// @copydoc this_type::expression_keys_
    public: typedef template_expression_key_container expression_key_container;

    //-------------------------------------------------------------------------
    /// @brief 状態監視器を構築する。
    public: explicit status_monitor(
        /// [in] メモリ割当子の初期値。
        typename this_type::expression_key_container::allocator_type const&
            in_allocator):
    expression_keys_(in_allocator),
    last_existence_(false)
    {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /// @brief ムーブ構築子。
    public: status_monitor(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source):
    expression_keys_(std::move(io_source.expression_keys_)),
    last_existence_(std::move(io_source.last_existence_))
    {}

    /// @brief ムーブ代入演算子。
    /// @return *this
    public: this_type& operator=(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source)
    {
        this->expression_keys_ = std::move(io_source.expression_keys_);
        this->last_existence_ = std::move(io_source.last_existence_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /// @brief 条件式識別値コンテナを整理する。
    /// @retval true  条件式識別値コンテナが空になった。
    /// @retval false 条件式識別値コンテナは空になってない。
    public: template<typename template_container>
    bool shrink_expression_keys(
        /// [in] 参照する expression_monitor の辞書。
        template_container const& in_expression_monitors)
    {
        return this_type::shrink_expression_keys(
            this->expression_keys_, in_expression_monitors);
    }

    //-------------------------------------------------------------------------
    /// @brief 条件式を状態監視器へ登録する。
    /// @details
    ///   in_expression が使う要素条件を走査し、要素条件が参照する状態値ごとに、
    ///   in_expression_key を status_monitor へ登録する。
    public: template<
        typename template_status_monitor_map,
        typename template_expression,
        typename template_expression_element_container>
    static void register_expression(
        /// [in,out] 状態変化を expression_monitor に知らせる、
        /// status_monitor の辞書。
        template_status_monitor_map& io_status_monitors,
        /// [in] 登録する evaluator::expression_key 。
        typename this_type::expression_key_container::value_type const&
            in_expression_key,
        /// [in] 要素条件を走査する evaluator::expression 。
        template_expression const& in_expression,
        /// [in] 条件式が参照する要素条件のコンテナ。
        template_expression_element_container const& in_expression_elements)
    {
        for (
            auto i(in_expression.get_begin_element());
            i < in_expression.get_end_element();
            ++i)
        {
            // 要素条件が参照する状態値の監視器を取得し、
            // in_register_key を状態監視器に登録する。
            auto const local_emplace(
                io_status_monitors.emplace(
                    in_expression_elements.at(i).get_key(),
                    this_type(io_status_monitors.get_allocator())));
            this_type::insert_expression_key(
                local_emplace.first->second.expression_keys_,
                in_expression_key);
        }
    }

    /// @brief 状態変化を検知し、条件式監視器へ知らせる。
    public: template<
        typename template_status_monitor_map,
        typename template_expression_monitor_map,
        typename template_reservoir>
    static void notify_status_transitions(
        /// [in,out] 状態変化を検知する status_monitor のコンテナ。
        template_status_monitor_map& io_status_monitors,
        /// [in,out] 状態変化を知らせる expression_monitor のコンテナ。
        template_expression_monitor_map& io_expression_monitors,
        /// [in] 状態変化を把握している _private::reservoir 。
        template_reservoir const& in_reservoir)
    {
        for (auto i(io_status_monitors.begin()); i != io_status_monitors.end();)
        {
            auto& local_status_monitor(i->second);
            local_status_monitor.notify_transition(
                io_expression_monitors, in_reservoir.find_transition(i->first));
            if (local_status_monitor.expression_keys_.empty())
            {
                i = io_status_monitors.erase(i);
            }
            else
            {
                ++i;
            }
        }
    }

    //-------------------------------------------------------------------------
    /// @brief 状態変化を通知する条件式を登録する。
    /// @retval true 条件式を登録した。
    /// @retval false 同じ識別値の条件式がすでに登録されていたので、何もしなかった。
    private: static bool insert_expression_key(
        /// [in,out] 状態変化を通知する条件式の識別値を挿入するコンテナ。
        typename this_type::expression_key_container& io_expression_keys,
        /// [in] 状態変化を通知する evaluator::expression_key 。
        typename this_type::expression_key_container::value_type const&
            in_expression_key)
    {
        auto const local_lower_bound(
            std::lower_bound(
                io_expression_keys.begin(),
                io_expression_keys.end(),
                in_expression_key));
        if (local_lower_bound != io_expression_keys.end()
            && *local_lower_bound == in_expression_key)
        {
            return false;
        }
        io_expression_keys.insert(local_lower_bound, in_expression_key);
        return true;
    }

    /// @brief 条件式識別値コンテナを整理する。
    /// @retval true  条件式識別値コンテナが空になった。
    /// @retval false 条件式識別値コンテナは空になってない。
    private: template<typename template_expression_monitor_map>
    static bool shrink_expression_keys(
        /// [in,out] 整理する条件式識別値のコンテナ。
        typename this_type::expression_key_container& io_expression_keys,
        /// [in] 参照する expression_monitor の辞書。
        template_expression_monitor_map const& in_expressions)
    {
        for (auto i(io_expression_keys.begin()); i != io_expression_keys.end();)
        {
            if (in_expressions.find(*i) != in_expressions.end())
            {
                ++i;
            }
            else
            {
                i = io_expression_keys.erase(i);
            }
        }
        io_expression_keys.shrink_to_fit();
        return io_expression_keys.empty();
    }

    /// @brief 状態変化を検知し、条件式監視器へ通知する。
    private: template<typename template_expression_monitor_map>
    void notify_transition(
        /// [in,out] 状態変化を通知する expression_monitor の辞書。
        template_expression_monitor_map& io_expression_monitors,
        /// [in] reservoir::find_transition の戻り値。
        std::int8_t const in_transition)
    {
        // 状態変化を検知する。
        auto const local_existence(0 <= in_transition);
        if (0 < in_transition || local_existence != this->last_existence_)
        {
            template_expression_monitor_map::mapped_type::notify_status_transition(
                io_expression_monitors, this->expression_keys_, local_existence);
        }
        this->last_existence_ = local_existence;
    }

    //-------------------------------------------------------------------------
    /// @brief 評価の更新を要求する evaluator::expression_key のコンテナ。
    private: typename this_type::expression_key_container expression_keys_;
    /// @brief 前回の notify_transition で、状態値が存在したか。
    private: bool last_existence_;

}; // class psyq::if_then_engine::_private::status_monitor

#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_MONITOR_HPP_)
// vim: set expandtab:
