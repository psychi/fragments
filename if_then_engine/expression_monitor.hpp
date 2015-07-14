/** @file
    @brief @copybrief psyq::if_then_engine::_private::expression_monitor
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_MONITOR_HPP_
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_MONITOR_HPP_

#include <cstdint>
#include <bitset>
#include <vector>
#include "./behavior.hpp"
#include "./key_less.hpp"

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            template<typename, typename, typename, typename>
                class expression_monitor;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 条件式監視器。

    条件式の評価結果と、
    条件式の評価結果が変化した際に呼び出す挙動関数オブジェクトを保持する。

    @tparam template_expression_key @copydoc evaluator::expression::key
    @tparam template_evaluation     @copydoc psyq::if_then_engine::evaluation
    @tparam template_priority       @copydoc dispatcher::function_priority
    @tparam template_allocator      @copydoc reservoir::allocator_type
 */
template<
    typename template_expression_key,
    typename template_evaluation,
    typename template_priority,
    typename template_allocator>
class psyq::if_then_engine::_private::expression_monitor
{
    /// @brief thisが指す値の型。
    private: typedef expression_monitor this_type;

    /** @brief 挙動関数を呼び出す変化条件を表す型。

        dispatcher::make_condition で変化条件を作る。
     */
    public: typedef std::uint8_t condition;

    public: enum: std::uint8_t
    {
        CONDITION_BIT_WIDTH = 3, ///< 変化条件に使うビット数。
    };

    /// @brief フラグの位置。
    public: enum flag: std::uint8_t
    {
        flag_VALID_TRANSITION,   ///< 状態変化の取得に成功。
        flag_INVALID_TRANSITION, ///< 状態変化の取得に失敗。
        flag_LAST_EVALUATION,    ///< 条件式の前回の評価の成功／失敗。
        flag_LAST_CONDITION,     ///< 条件式の前回の評価。
        flag_FLUSH_CONDITION,    ///< 条件式の前回の評価を無視する。
        flag_REGISTERED,         ///< 条件式の登録済みフラグ。
    };

    /// @brief 条件式監視器のコンテナ。
    public: typedef std::vector<this_type, template_allocator> container;

    /// @brief 条件式監視器を条件式の識別値の昇順で並び替えるのに使う、比較関数オブジェクト。
    public: typedef
        psyq::if_then_engine::_private::key_less<
            psyq::if_then_engine::_private::object_key_getter<
                this_type, template_expression_key>>
        key_less;

    /// @brief 条件挙動。
    public: typedef
        psyq::if_then_engine::_private::behavior<
            template_expression_key, template_evaluation, template_priority>
        behavior;

    /// @brief 条件挙動のコンテナ。
    public: typedef
        std::vector<typename this_type::behavior, template_allocator>
        behavior_container;

    /// @brief 条件挙動と条件式評価キャッシュのコンテナ。
    public: typedef
        std::vector<
            std::pair<
                typename this_type::behavior,
                typename this_type::behavior::cache>,
            template_allocator>
        behavior_cache_container;

    //-------------------------------------------------------------------------
    /// @brief 条件挙動キャッシュから優先順位を取得する関数オブジェクト。
    private: struct behavior_cache_key_getter
    {
        /// @brief オブジェクトの型。
        typedef
            typename expression_monitor::behavior_cache_container::value_type
            object;

        /// @brief オブジェクトから取り出すキーの型。
        typedef template_priority key;

        /** @brief 条件挙動キャッシュから優先順位を取得する。
            @param[in] in_behavior_cache 優先順位を取得する条件挙動キャッシュ。
            @return 条件挙動の優先順位。
         */
        static key const& get(object const& in_behavior_cache) PSYQ_NOEXCEPT
        {
            return in_behavior_cache.first.priority_;
        }

    }; // struct behavior_cache_key_getter

    //-------------------------------------------------------------------------
    /** @brief 条件式監視器を構築する。
        @param[in] in_key       監視する条件式の識別値。
        @param[in] in_condition
            dispatcher::make_condition から取得した、挙動関数を呼び出す変化条件。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: expression_monitor(
        template_expression_key in_key,
        typename this_type::condition const in_condition,
        template_allocator const& in_allocator)
    :
    key_(std::move(in_key)),
    behaviors_(in_allocator),
    condition_(in_condition)
    {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: expression_monitor(this_type&& io_source):
    behaviors_(std::move(io_source.behaviors_)),
    key_(std::move(io_source.key_)),
    condition_(std::move(io_source.condition_)),
    flags_(std::move(io_source.flags_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->behaviors_ = std::move(io_source.behaviors_);
            this->key_ = std::move(io_source.key_);
            this->condition_ = std::move(io_source.condition_);
            this->flags_ = std::move(io_source.flags_);
        }
        return *this;
    }
#endif // !defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /** @brief 条件式に対応する挙動関数を登録する。

        登録された挙動関数は、スマートポインタが空になると、
        自動的に取り除かれる。明示的に挙動関数を取り除くには、
        this_type::remove_function を呼び出す。

        @param[in,out] io_expression_monitors
            挙動関数を登録する条件式監視器のコンテナ。
        @param[in] in_expression_key 評価に用いる条件式の識別値。
        @param[in] in_condition
            dispatcher::make_condition から取得した、挙動関数を呼び出す変化条件。
        @param[in] in_function
            登録する挙動関数を指すスマートポインタ。
        @param[in] in_priority
            挙動関数の呼び出し優先順位。優先順位の昇順に呼び出される。
        @param[in] in_reserve_behaviors
            条件式監視器が持つ条件挙動コンテナの予約容量。
        @retval true 成功。挙動関数を登録した。
        @retval false
            失敗。挙動関数は登録されなかった。
            挙動関数を指すスマートポインタが空だったか、
            同じ条件式に同じ挙動関数がすでに登録されていたのが原因。
     */
    public: static bool register_function(
        typename this_type::container& io_expression_monitors,
        template_expression_key const& in_expression_key,
        typename this_type::condition const in_condition,
        typename this_type::behavior::function_shared_ptr const& in_function,
        template_priority const in_priority,
        std::size_t const in_reserve_behaviors)
    {
        if (in_condition == 0)
        {
            return false;
        }
        auto const local_function(in_function.get());
        if (local_function == nullptr)
        {
            return false;
        }

        // 関数オブジェクトを登録する条件式監視器を決定する。
        auto local_expression_monitor(
            std::lower_bound(
                io_expression_monitors.begin(),
                io_expression_monitors.end(),
                in_expression_key,
                typename this_type::key_less()));
        if (local_expression_monitor != io_expression_monitors.end()
            && local_expression_monitor->key_ == in_expression_key)
        {
            // 同じ関数オブジェクトがすでに登録済みなら、失敗する。
            auto const local_find_function(
                this_type::find_function(
                    local_expression_monitor->behaviors_,
                    *local_function,
                    false));
            if (local_find_function)
            {
                return false;
            }
        }
        else
        {
            // 条件式監視器を新たに生成し、コンテナに挿入する。
            local_expression_monitor = io_expression_monitors.insert(
                local_expression_monitor,
                this_type(
                    in_expression_key,
                    in_condition,
                    io_expression_monitors.get_allocator()));
        }

        // 関数オブジェクトを条件式監視器へ追加する。
        local_expression_monitor->behaviors_.reserve(in_reserve_behaviors);
        local_expression_monitor->behaviors_.emplace_back(
            in_function, in_priority);
        return true;
    }

    /** @brief 関数オブジェクトを削除しつつ、コンテナを整理する。
        @param[in] in_function 削除する関数オブジェクト。
        @retval true  削除対象がコンテナから見つかった。
        @retval false 削除対象がコンテナから見つからなかった。
     */
    public: bool remove_function(
        typename this_type::behavior::function const& in_function)
    {
        return this_type::find_function(this->behaviors_, in_function, true);
    }

    /** @brief 条件式を評価し、条件挙動をキャッシュに貯める。

        条件式監視器のコンテナを走査し、条件式の結果によって、
        dispacher::register_function で登録された条件挙動をキャッシュに貯める。

        @param[in,out] io_behavior_caches     条件挙動キャッシュのコンテナ。
        @param[in,out] io_expression_monitors 条件式監視器のコンテナ。
        @param[in] in_evaluator               評価に使う条件評価器。
        @param[in] in_reservoir               条件式の評価に状態値のコンテナ。
     */
    public: template<typename template_evaluator>
    static void cache_behaviors(
        typename this_type::behavior_cache_container& io_behavior_caches,
        typename this_type::container& io_expression_monitors,
        template_evaluator const& in_evaluator,
        typename template_evaluator::reservoir const& in_reservoir)
    {
        // io_expression_monitors 走査し、評価要求があれば条件式を評価しつつ、
        // 空になった条件式監視器を削除する。
        for (
            auto i(io_expression_monitors.begin());
            i != io_expression_monitors.end();)
        {
            auto& local_expression_monitor(*i);
            if (local_expression_monitor.get_evaluation_request())
            {
                local_expression_monitor.cache_behavior(
                    io_behavior_caches, in_evaluator, in_reservoir);
                if (local_expression_monitor.behaviors_.empty())
                {
                    // 挙動関数のコンテナが空になったら、
                    // 条件式監視器を削除する。
                    i = io_expression_monitors.erase(i);
                    continue;
                }
            }
            ++i;
        }
    }

    //-------------------------------------------------------------------------
    /** @brief 関数オブジェクトを検索しつつ、コンテナを整理する。
        @param[in,out] io_behaviors 走査する条件挙動のコンテナ。
        @param[in] in_function      検索する関数オブジェクト。
        @param[in] in_erase         検索する関数オブジェクトを削除するかどうか。
        @retval true  io_behaviors から in_function が見つかった。
        @retval false io_behaviors から in_function が見つからなかった。
     */
    private: static bool find_function(
        typename this_type::behavior_container& io_behaviors,
        typename this_type::behavior::function const& in_function,
        bool const in_erase)
    {
        auto local_find(false);
        for (auto i(io_behaviors.begin()); i != io_behaviors.end();)
        {
            auto& local_observer(i->function_);
            bool local_erase;
            if (local_find)
            {
                local_erase = local_observer.expired();
            }
            else
            {
                auto const local_pointer(local_observer.lock().get());
                local_find = local_pointer == &in_function;
                local_erase = local_find? in_erase: local_pointer == nullptr;
            }
            if (local_erase)
            {
                i = io_behaviors.erase(i);
            }
            else
            {
                ++i;
            }
        }
        return local_find;
    }

    /** @brief 変化条件に合致する挙動関数をキャッシュに貯める。

        条件式を評価し、変化条件に合致するなら、
        this_type::register_function で登録された挙動関数をキャッシュに貯める。

        @param[in,out] io_behavior_caches 条件挙動キャッシュのコンテナ。
        @param[in] in_evaluator           評価に用いる条件評価器。
        @param[in] in_reservoir           評価に用いる状態貯蔵器。
        @retval true  挙動関数をキャッシュに貯めた。
        @retval false 挙動関数をキャッシュに貯めなかった。
     */
    private: template<typename template_evaluator>
    bool cache_behavior(
        typename this_type::behavior_cache_container& io_behavior_caches,
        template_evaluator const& in_evaluator,
        typename template_evaluator::reservoir const& in_reservoir)
    {
        // 条件式を評価し、変化条件と合致するか判定する。
        auto const local_flush_condition(
            this->flags_.test(this_type::flag_FLUSH_CONDITION));
        auto const local_last_evaluation(
            this->get_last_evaluation(local_flush_condition));
        auto const local_evaluation(
            this->evaluate_expression(
                in_evaluator, in_reservoir, local_flush_condition));
        if (!this->is_matched_condition(
                local_evaluation, local_last_evaluation))
        {
            return false;
        }

        // 変化条件に合致したので、挙動関数をキャッシュに貯める。
        typename this_type::behavior::cache const local_cache(
            this->key_, local_evaluation, local_last_evaluation);
        for (auto i(this->behaviors_.begin()); i != this->behaviors_.end();)
        {
            auto const& local_behavior(*i);
            if (local_behavior.function_.expired())
            {
                i = this->behaviors_.erase(i);
            }
            else
            {
                // 優先順位の昇順となるように、条件挙動キャッシュを挿入する。
                io_behavior_caches.emplace(
                    std::upper_bound(
                        io_behavior_caches.cbegin(),
                        io_behavior_caches.cend(),
                        local_behavior.priority_,
                        psyq::if_then_engine::_private::key_less<
                            typename this_type::behavior_cache_key_getter>()),
                    local_behavior,
                    local_cache);
                ++i;
            }
        }
        return true;
    }

    /** @brief 監視している条件式を評価する。
        @param[in] in_evaluator 評価に用いる条件評価器。
        @param[in] in_reservoir 評価に用いる状態貯蔵器。
        @param[in] in_flush     前回の評価を無視するかどうか。
        @retval 正 条件式の評価は真となった。
        @retval 0  条件式の評価は偽となった。
        @retval 負 条件式の評価に失敗した。
     */
    private: template<typename template_evaluator>
    template_evaluation evaluate_expression(
        template_evaluator const& in_evaluator,
        typename template_evaluator::reservoir const& in_reservoir,
        bool const in_flush)
    {
        // 状態変化フラグを更新する。
        auto const local_invalid_transition(
            this->flags_.test(this_type::flag_INVALID_TRANSITION));
        this->flags_.reset(this_type::flag_VALID_TRANSITION);
        this->flags_.reset(this_type::flag_INVALID_TRANSITION);

        // 状態値の取得に失敗していたら、条件式の評価も失敗とみなす。
        if (local_invalid_transition)
        //if (in_flush && local_invalid_transition)
        {
            this->flags_.reset(this_type::flag_LAST_EVALUATION);
            this->flags_.reset(this_type::flag_LAST_CONDITION);
            return -1;
        }

        // 条件式を評価し、結果を記録する。
        auto const local_evaluate_expression(
            in_evaluator.evaluate_expression(this->key_, in_reservoir));
        this->flags_.set(
            this_type::flag_LAST_EVALUATION, 0 <= local_evaluate_expression);
        this->flags_.set(
            this_type::flag_LAST_CONDITION, 0 < local_evaluate_expression);
        return this->get_last_evaluation(false);
    }

    private: bool get_evaluation_request() const PSYQ_NOEXCEPT
    {
        return this->flags_.test(this_type::flag_VALID_TRANSITION)
            || this->flags_.test(this_type::flag_INVALID_TRANSITION);
    }

    /** @brief 監視している条件式の前回の評価を取得する。
        @param[in] in_flush 前回の評価を無視する。
        @retval 正 条件式の評価は真となった。
        @retval  0 条件式の評価は偽となった。
        @retval 負 条件式の評価に失敗した。
     */
    private: template_evaluation get_last_evaluation(bool const in_flush)
    const PSYQ_NOEXCEPT
    {
        return this->flags_.test(this_type::flag_LAST_EVALUATION)?
            !in_flush && this->flags_.test(this_type::flag_LAST_CONDITION): -1;
    }

    /** @brief 挙動関数を呼び出す変化条件と条件式の評価が一致するか判定する。
        @param[in] in_evaluation      条件式の最新の評価結果。
        @param[in] in_last_evaluation 条件式の前回の評価結果。
        @retval true  変化条件が一致した。
        @retval false 変化条件が一致しなかった。
     */
    private: bool is_matched_condition(
        template_evaluation const in_evaluation,
        template_evaluation const in_last_evaluation)
    const PSYQ_NOEXCEPT
    {
        // 最新と前回で条件式の評価が変わってなければ、条件判定しない。
        if (in_evaluation == in_last_evaluation)
        {
            return false;
        }

        // 変化条件に合致するか判定する。
        auto const local_condition(
            (1 << ((0 <= in_evaluation) + (0 < in_evaluation))) | (
                (1 << this_type::CONDITION_BIT_WIDTH)
                << ((0 <= in_last_evaluation) + (0 < in_last_evaluation))));
        return local_condition == (local_condition & this->condition_);
    }

    //-------------------------------------------------------------------------
    /// @brief 条件挙動のコンテナ。
    public: typename this_type::behavior_container behaviors_;
    /// @brief 監視している条件式の識別値。
    public: template_expression_key key_;
    /// @brief 挙動関数を呼び出す変化条件。
    public: typename this_type::condition condition_;
    /// @brief フラグの集合。
    public: std::bitset<8> flags_;

}; // class psyq::if_then_engine::_private::expression_monitor

#endif // defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_MONITOR_HPP_)
// vim: set expandtab:
