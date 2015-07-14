/** @file
    @brief @copybrief psyq::if_then_engine::_private::dispatcher
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_IF_THEN_ENGINE_DISPATCHER_HPP_
#define PSYQ_IF_THEN_ENGINE_DISPATCHER_HPP_

#include "./expression_monitor.hpp"

/// @brief 挙動関数の呼び出し優先順位のデフォルト値。
#ifndef PSYQ_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT
#define PSYQ_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT 0
#endif // !defined(PSYQ_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT)

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            template<typename, typename> class dispatcher;
            template<typename, typename, typename> class status_monitor;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 条件挙動器。条件式の評価が変化条件と合致すると、挙動関数を呼び出す。

    ### 使い方の概略
    - dispatcher::register_function で、
      変化条件が一致した際に呼び出す挙動関数を登録する。
    - dispatcher::_dispatch で条件式の評価の変化を検知し、
      変化後条件に合致する挙動関数を呼び出す。

    @tparam template_evaluator @copydoc dispatcher::evaluator
    @tparam template_priority  @copydoc dispatcher::function_priority
 */
template<typename template_evaluator, typename template_priority>
class psyq::if_then_engine::_private::dispatcher
{
    /// @brief thisが指す値の型。
    private: typedef dispatcher this_type;

    /** @brief 条件挙動器で使う条件評価器の型。

        psyq::if_then_engine::_private::evaluator と互換性があること。
     */
    public: typedef template_evaluator evaluator;

    /** @brief 挙動関数の呼び出し優先順位の型。

        挙動関数は、優先順位の昇順で呼び出される。
     */
    public: typedef template_priority function_priority;

    /// @brief コンテナに用いるメモリ割当子の型。
    public:
        typedef typename this_type::evaluator::allocator_type
        allocator_type;

    //-------------------------------------------------------------------------
    /// @brief 状態監視器。
    private: typedef
         psyq::if_then_engine::_private::status_monitor<
             typename this_type::evaluator::reservoir::status_key,
             typename this_type::evaluator::expression::key,
             typename this_type::allocator_type>
         status_monitor;

    /// @brief 条件式監視器。
    private: typedef
        psyq::if_then_engine::_private::expression_monitor<
            typename this_type::evaluator::expression::key,
            psyq::if_then_engine::evaluation,
            typename this_type::function_priority,
            typename this_type::allocator_type>
        expression_monitor;

    //-------------------------------------------------------------------------
    /// @copydoc expression_monitor::condition
    public: typedef
        typename this_type::expression_monitor::condition
        condition;

    /// @copydoc expression_monitor::behavior::function
    public: typedef
        typename this_type::expression_monitor::behavior::function
        function;

    /// @copydoc expression_monitor::behavior::function_shared_ptr
    public: typedef
        typename this_type::expression_monitor::behavior::function_shared_ptr
        function_shared_ptr;

    /// @copydoc expression_monitor::behavior::function_weak_ptr
    public: typedef
        typename this_type::expression_monitor::behavior::function_weak_ptr
        function_weak_ptr;

    //-------------------------------------------------------------------------
    /** @name 構築と代入
        @{
     */
    /** @brief 空の条件挙動器を構築する。
        @param[in] in_reserve_expressions 監視する条件式の予約数。
        @param[in] in_reserve_statuses    監視する状態値の予約数。
        @param[in] in_reserve_caches      条件挙動キャッシュの予約数。
        @param[in] in_allocator           メモリ割当子の初期値。
     */
    public: dispatcher(
        std::size_t const in_reserve_expressions,
        std::size_t const in_reserve_statuses,
        std::size_t const in_reserve_caches,
        typename this_type::allocator_type const& in_allocator)
    :
    expression_monitors_(in_allocator),
    status_monitors_(in_allocator),
    behavior_caches_(in_allocator),
    dispatch_lock_(false)
    {
        this->expression_monitors_.reserve(in_reserve_expressions);
        this->status_monitors_.reserve(in_reserve_statuses);
        this->behavior_caches_.reserve(in_reserve_caches);
    }

    /** @brief コピー構築子。
        @param[in] in_source コピー元となるインスタンス。
     */
    public: dispatcher(this_type const& in_source):
    expression_monitors_(in_source.expression_monitors_),
    status_monitors_(in_source.status_monitors_),
    behavior_caches_(in_source.behavior_caches_.get_allocator()),
    dispatch_lock_(false)
    {
        this->behavior_caches_.reserve(in_source.behavior_caches_.capacity());
    }

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @note this_type::_dispatch 実行中はムーブできない。
     */
    public: dispatcher(this_type&& io_source):
    expression_monitors_(std::move(io_source.expression_monitors_)),
    status_monitors_(std::move(io_source.status_monitors_)),
    behavior_caches_(std::move(io_source.behavior_caches_)),
    dispatch_lock_((PSYQ_ASSERT(!io_source.dispatch_lock_), false))
    {}

    /** @brief コピー代入演算子。
        @param[in] in_source コピー元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type const& in_source)
    {
        /// @note this_type::_dispatch 実行中はムーブできない。
        PSYQ_ASSERT(!this->dispatch_lock_ && !in_source.dispatch_lock_);
        this->expression_monitors_ = in_source.expression_monitors_;
        this->status_monitors_ = in_source.status_monitors_;
        return *this;
    }

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        /// @note this_type::_dispatch 実行中はムーブできない。
        PSYQ_ASSERT(!this->dispatch_lock_ && !io_source.dispatch_lock_);
        this->expression_monitors_ = std::move(io_source.expression_monitors_);
        this->status_monitors_ = std::move(io_source.status_monitors_);
        this->behavior_caches_ = std::move(io_source.behavior_caches_);
        return *this;
    }

    /// @brief 条件挙動器を解体する。
    public: ~dispatcher()
    {
        /// @note this_type::_dispatch 実行中は解体できない。
        PSYQ_ASSERT(!this->dispatch_lock_);
    }

    /** @brief 条件挙動器で使われているメモリ割当子を取得する。
        @return 条件挙動器で使われているメモリ割当子。
     */
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->expression_monitors_.get_allocator();
    }

    /// @brief 条件挙動器を再構築し、メモリ領域を必要最小限にする。
    public: void shrink_to_fit()
    {
        this_type::rebuild_expression_monitor(this->expression_monitors_);
        this_type::rebuild_status_monitor(
            this->status_monitors_, this->expression_monitors_);
        //this->behavior_caches_.shrink_to_fit();
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 条件挙動
        @{
     */
    /** @brief 条件式に対応する挙動関数を登録する。

        this_type::_dispatch で変化条件が合致した際に呼び出す挙動関数を登録する。

        登録された挙動関数は、スマートポインタが空になると、
        自動的に取り除かれる。明示的に挙動関数を取り除くには、
        this_type::unregister_function を呼び出す。

        @param[in] in_expression_key 評価に用いる条件式の識別値。
        @param[in] in_condition
            this_type::make_condition から取得した、挙動関数を呼び出す変化条件。
        @param[in] in_function 登録する挙動関数を指すスマートポインタ。
        @param[in] in_priority
            挙動関数の呼び出し優先順位。優先順位の昇順に呼び出される。
        @param[in] in_reserve_functions
            条件式監視器が持つ挙動関数コンテナの予約容量。
        @retval true 成功。挙動関数を登録した。
        @retval false
            失敗。挙動関数は登録されなかった。
            挙動関数を指すスマートポインタが空だったか、
            同じ条件式に同じ挙動関数がすでに登録されていたのが原因。
     */
    public: bool register_function(
        typename this_type::evaluator::expression::key const&
            in_expression_key,
        typename this_type::condition const in_condition,
        typename this_type::function_shared_ptr const& in_function,
        typename this_type::function_priority const in_priority =
            PSYQ_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT,
        std::size_t const in_reserve_functions = 1)
    {
        return this_type::expression_monitor::register_function(
            this->expression_monitors_,
            in_expression_key,
            in_condition,
            in_function,
            in_priority,
            in_reserve_functions);
    }

    /** @brief 条件式に対応する挙動関数を取り除く。

        this_type::register_function で登録した挙動関数を、
        条件式監視器から取り除く。

        @param[in] in_expression_key 条件式の識別値。
        @param[in] in_function       取り除く挙動関数。
     */
    public: void unregister_function(
        typename this_type::evaluator::expression::key const&
            in_expression_key,
        typename this_type::function const& in_function)
    {
        auto const local_expression_monitor(
            this_type::expression_monitor::key_less::find_pointer(
                this->expression_monitors_, in_expression_key));
        if (local_expression_monitor != nullptr)
        {
            local_expression_monitor->remove_function(in_function);
        }
    }

    /** @brief 条件式に対応する挙動関数をすべて取り除く。

        条件式監視器を削除し、 this_type::register_function
        で同じ条件式に登録した挙動関数を、すべて取り除く。

        @param[in] in_expression_key 条件式の識別値。
     */
    public: void unregister_function(
        typename this_type::evaluator::expression::key const&
            in_expression_key)
    {
        auto const local_expression_monitor(
            this_type::expression_monitor::key_less::find_const_iterator(
                this->expression_monitors_, in_expression_key));
        if (local_expression_monitor != this->expression_monitors_.end())
        {
            this->expression_monitors_.erase(local_expression_monitor);
        }
    }

    /** @brief 挙動関数を取り除く。

        this_type::register_function で登録した挙動関数を、
        すべての条件式監視器から取り除く。

        @param[in] in_function 削除する挙動関数。
     */
    public: void unregister_function(
        typename this_type::function const& in_function)
    {
        for (auto& local_expression_monitor: this->expression_monitors_)
        {
            local_expression_monitor.remove_function(in_function);
        }
    }

    /** @brief psyq::if_then_engine 管理者以外は、この関数は使用禁止。

        前回の this_type::_dispatch と今回の this_type::_dispatch
        で条件式の評価が異なっている場合に、 this_type::register_function
        で指定した変化条件と合致していると、挙動関数を呼び出す。

        @param[in] in_evaluator     条件式の評価に使う条件評価器。
        @param[in,out] io_reservoir 条件式の評価に使う状態貯蔵器。
        @param[in] in_reserve_expressions
            状態監視器で使う条件式識別値コンテナの予約容量。

        @warning
        前回から今回の間（基本的には1フレームの間）で条件式の評価が変化しても、
        前回の時点と今回の時点の評価が同じ場合は、挙動関数が呼び出されない。
        たとえば、前回から今回の間で条件式の評価が
        「true（前回）／false（前回と今回の間）／true（今回）」
        と変化した場合、挙動関数は呼び出されない。

        @note
        状態値が削除された場合は detect_status_transition で検知できるが、
        条件式が削除された場合を検知してない。検知すべき？
     */
    public: void _dispatch(
        typename this_type::evaluator const& in_evaluator,
        typename this_type::evaluator::reservoir& io_reservoir,
        std::size_t const in_reserve_expressions = 1)
    {
        // _dispatch を多重に実行しないようにロックする。
        if (this->dispatch_lock_)
        {
            PSYQ_ASSERT(false);
            return;
        }
        this->dispatch_lock_ = true;

        // 条件式を状態監視器へ登録する。
        this_type::register_expressions(
            this->status_monitors_,
            this->expression_monitors_,
            in_evaluator,
            in_reserve_expressions);

        // 状態変化を検知し、条件式監視器へ知らせる。
        this_type::detect_status_transition(
            this->status_monitors_, this->expression_monitors_, io_reservoir);

        // 状態変化した条件式を評価し、挙動関数をキャッシュに貯める。
        this->behavior_caches_.clear();
        this_type::expression_monitor::cache_behaviors(
            this->behavior_caches_,
            this->expression_monitors_,
            in_evaluator,
            io_reservoir);

        // 条件式の評価が済んだので、状態変化フラグを初期化する。
        io_reservoir._reset_transition();

        // キャッシュに貯まった挙動関数を呼び出す。
        for (auto const& local_cache: this->behavior_caches_)
        {
            local_cache.second.call_function(local_cache.first);
        }

        // 挙動関数のキャッシュを片づける。
        this->behavior_caches_.clear();
        PSYQ_ASSERT(this->dispatch_lock_);
        this->dispatch_lock_ = false;
    }
    /// @}
    /** @brief 挙動関数を呼び出す変化条件を作る。
        @param[in] in_now_true    条件式の最新の評価が、真に変化したことが条件。
        @param[in] in_now_false   条件式の最新の評価が、偽に変化したことが条件。
        @param[in] in_now_failed  条件式の最新の評価が、失敗に変化したことが条件。
        @param[in] in_last_true   条件式の前回の評価が、真だったことが条件。
        @param[in] in_last_false  条件式の前回の評価が、偽だったことが条件。
        @param[in] in_last_failed 条件式の前回の評価が、失敗だったことが条件。
        @return 挙動関数を呼び出す変化条件。
     */
    public: static typename this_type::condition make_condition(
        bool const in_now_true,
        bool const in_now_false,
        bool const in_now_failed,
        bool const in_last_true,
        bool const in_last_false,
        bool const in_last_failed)
    PSYQ_NOEXCEPT
    {
        return static_cast<typename this_type::condition>(in_now_failed)
            | (in_now_false << 1)
            | (in_now_true << 2)
            | (in_last_failed << this_type::expression_monitor::CONDITION_BIT_WIDTH)
            | (in_last_false << (this_type::expression_monitor::CONDITION_BIT_WIDTH + 1))
            | (in_last_true << (this_type::expression_monitor::CONDITION_BIT_WIDTH + 2));
    }

    /** @brief 挙動関数を呼び出す変化条件を作る。
        @param[in] in_now_evaluation  条件式の最新の評価。
        @param[in] in_last_evaluation 条件式の前回の評価。
        @return 挙動関数を呼び出す変化条件。
     */
    public: static typename this_type::condition make_condition(
        psyq::if_then_engine::evaluation const in_now_evaluation,
        psyq::if_then_engine::evaluation const in_last_evaluation)
    {
        return this_type::make_condition(
            0 < in_now_evaluation,
            in_now_evaluation == 0,
            in_now_evaluation < 0,
            0 < in_last_evaluation,
            in_last_evaluation == 0,
            in_last_evaluation < 0);
    }

    //-------------------------------------------------------------------------
    /** @brief 条件式監視器を再構築する。
        @param[in,out] io_expression_monitors 再構築する条件式監視器のコンテナ。
     */
    private: static void rebuild_expression_monitor(
        typename this_type::expression_monitor::container&
            io_expression_monitors)
    {
        /// @note std::vector なら、逆順で走査したほうが効率いいかも。
        for (
            auto i(io_expression_monitors.begin());
            i != io_expression_monitors.end();)
        {
            auto& local_behaviors(i->behaviors_);
            for (auto j(local_behaviors.begin()); j != local_behaviors.end();)
            {
                if (j->function_.expired())
                {
                    j = local_behaviors.erase(j);
                }
                else
                {
                    ++j;
                }
            }
            if (local_behaviors.empty())
            {
                i = io_expression_monitors.erase(i);
            }
            else
            {
                local_behaviors.shrink_to_fit();
                ++i;
            }
        }
        io_expression_monitors.shrink_to_fit();
    }

    /** @brief 状態監視器を再構築する。
        @param[in,out] io_status_monitors  再構築する状態監視器のコンテナ。
        @param[in] in_expression_monitors 状態監視器が使っている条件式監視器のコンテナ。
     */
    private: static void rebuild_status_monitor(
        typename this_type::status_monitor::container& io_status_monitors,
        typename this_type::expression_monitor::container const&
            in_expression_monitors)
    {
        /// @note std::vector なら、逆順で走査したほうが効率いいかも。
        for (auto i(io_status_monitors.begin()); i != io_status_monitors.end();)
        {
            auto& local_expression_keys(i->expression_keys_);
            for (
                auto j(local_expression_keys.begin());
                j != local_expression_keys.end();)
            {
                auto const local_expression_monitor(
                    this_type::expression_monitor::key_less::find_const_pointer(
                        in_expression_monitors, *j));
                if (local_expression_monitor == nullptr)
                {
                    j = local_expression_keys.erase(j);
                }
                else
                {
                    ++j;
                }
            }
            if (local_expression_keys.empty())
            {
                i = io_status_monitors.erase(i);
            }
            else
            {
                local_expression_keys.shrink_to_fit();
                ++i;
            }
            local_expression_keys.shrink_to_fit();
        }
        io_status_monitors.shrink_to_fit();
    }

    //-------------------------------------------------------------------------
    /** @brief 条件式を状態監視器へ登録する。
        @param[in,out] io_status_monitors
            状態変化を条件式監視器に知らせる、状態監視器。
        @param[in,out] io_expression_monitors
            条件式の評価の変化を挙動関数へ知らせる、条件式監視器。
        @param[in] in_evaluator 登録する条件式を持つ条件評価器。
        @param[in] in_reserve_expressions
            状態監視器が持つ条件式識別値コンテナの予約容量。
     */
    private: static void register_expressions(
        typename this_type::status_monitor::container& io_status_monitors,
        typename this_type::expression_monitor::container&
            io_expression_monitors,
        typename this_type::evaluator const& in_evaluator,
        std::size_t const in_reserve_expressions)
    {
        // 条件式監視器のコンテナを走査し、
        // 登録が完了してないものは、状態監視器へ登録する。
        for (auto& local_expression_monitor: io_expression_monitors)
        {
            auto const local_registered(
                local_expression_monitor.flags_.test(
                    this_type::expression_monitor::flag_REGISTERED));
            if (!local_registered)
            {
                auto const local_register_expression(
                    this_type::register_expression(
                        io_status_monitors,
                        io_expression_monitors,
                        local_expression_monitor.key_,
                        local_expression_monitor.key_,
                        in_evaluator,
                        in_reserve_expressions));
                if (local_register_expression != 0)
                {
                    local_expression_monitor.flags_.set(
                        this_type::expression_monitor::flag_FLUSH_CONDITION,
                        local_register_expression < 0);
                    local_expression_monitor.flags_.set(
                        this_type::expression_monitor::flag_REGISTERED);
                }
            }
        }
    }

    /** @brief 条件式を状態監視器へ登録する。
        @param[in,out] io_status_monitors
            状態変化を条件式監視器に知らせる、状態監視器。
        @param[in] in_expression_monitors
            条件式の評価の変化を挙動関数へ知らせる、条件式監視器。
        @param[in] in_register_key 登録する条件式の識別値。
        @param[in] in_scan_key     走査する条件式の識別値。
        @param[in] in_evaluator    走査する条件式を持つ条件評価器。
        @param[in] in_reserve_expressions
            状態監視器が持つ条件式識別値コンテナの予約容量。
        @retval 正 成功。条件式の評価を維持する。
        @retval 負 成功。条件式の評価を維持しない。
        @retval 0  失敗。
     */
    private: static std::int8_t register_expression(
        typename this_type::status_monitor::container& io_status_monitors,
        typename this_type::expression_monitor::container const&
            in_expression_monitors,
        typename this_type::evaluator::expression::key const& in_register_key,
        typename this_type::evaluator::expression::key const& in_scan_key,
        typename this_type::evaluator const& in_evaluator,
        std::size_t const in_reserve_expressions)
    {
        // in_scan_key に対応する条件式と要素条件チャンクを取得する。
        auto const local_expression(
            in_evaluator._find_expression(in_scan_key));
        if (local_expression == nullptr)
        {
            return 0;
        }
        auto const local_chunk(
            in_evaluator._find_chunk(local_expression->chunk_key_));
        if (local_chunk == nullptr)
        {
            // 条件式があれば、要素条件チャンクもあるはず。
            PSYQ_ASSERT(false);
            return 0;
        }

        // in_scan_key に対応する条件式の種類によって、
        // in_register_key の登録先を選択する。
        switch (local_expression->kind_)
        {
            case this_type::evaluator::expression::kind_SUB_EXPRESSION:
            return this_type::register_compound_expression(
                io_status_monitors,
                in_expression_monitors,
                in_register_key,
                *local_expression,
                local_chunk->sub_expressions_,
                in_evaluator,
                in_reserve_expressions);

            case this_type::evaluator::expression::kind_STATE_TRANSITION:
            this_type::register_expression(
                io_status_monitors,
                in_register_key,
                *local_expression,
                local_chunk->status_transitions_,
                in_reserve_expressions);
            return -1;

            case this_type::evaluator::expression::kind_STATE_COMPARISON:
            this_type::register_expression(
                io_status_monitors,
                in_register_key,
                *local_expression,
                local_chunk->status_comparisons_,
                in_reserve_expressions);
            return 1;

            default:
            // 未対応の条件式の種類だった。
            PSYQ_ASSERT(false);
            return 0;
        }
    }

    /** @brief 条件式を状態監視器へ登録する。
        @param[in,out] io_status_monitors
            状態変化を条件式監視器に知らせる、状態監視器。
        @param[in] in_register_key 登録する条件式の識別値。
        @param[in] in_expression   走査する条件式。
        @param[in] in_elements     条件式が参照する要素条件のコンテナ。
        @param[in] in_reserve_expressions
            状態監視器が持つ条件式識別値コンテナの予約容量。
     */
    private: template<typename template_element_container>
    static void register_expression(
        typename this_type::status_monitor::container& io_status_monitors,
        typename this_type::evaluator::expression::key const& in_register_key,
        typename this_type::evaluator::expression const& in_expression,
        template_element_container const& in_elements,
        std::size_t const in_reserve_expressions)
    {
        // in_expression が使う要素条件を走査し、
        // in_register_key を要素条件ごとに状態監視器へ登録する。
        for (auto i(in_expression.begin_); i < in_expression.end_; ++i)
        {
            // in_register_key を登録する状態監視器を取得する。
            auto const& local_status_key(in_elements.at(i).key_);
            auto local_status_monitor(
                std::lower_bound(
                    io_status_monitors.begin(),
                    io_status_monitors.end(),
                    local_status_key,
                    typename this_type::status_monitor::key_less()));
            if (local_status_monitor == io_status_monitors.end()
                || local_status_monitor->key_ != local_status_key)
            {
                // 要素条件に対応する状態監視器がなかったので、
                // 状態監視器を新たに生成する。
                local_status_monitor = io_status_monitors.insert(
                    local_status_monitor,
                    typename this_type::status_monitor(
                        local_status_key, io_status_monitors.get_allocator()));
            }

            // in_register_key を状態監視器へ登録する。
            auto& local_expression_keys(
                local_status_monitor->expression_keys_);
            local_expression_keys.reserve(in_reserve_expressions);
            auto const local_lower_bound(
                std::lower_bound(
                    local_expression_keys.begin(),
                    local_expression_keys.end(),
                    in_register_key));
            if (local_lower_bound == local_expression_keys.end()
                || *local_lower_bound != in_register_key)
            {
                local_expression_keys.insert(
                    local_lower_bound, in_register_key);
            }
        }
    }

    /** @brief 複合条件式を状態監視器へ登録する。
        @param[in,out] io_status_monitors
            状態変化を条件式監視器に知らせる、状態監視器。
        @param[in] in_expression_monitors
            条件式の評価の変化を挙動関数へ知らせる、条件式監視器。
        @param[in] in_register_key 登録する複合条件式の識別値。
        @param[in] in_expression   走査する複合条件式。
        @param[in] in_sub_expressions
            登録する複合条件式が参照する要素条件コンテナ。
        @param[in] in_evaluator    登録する条件式を持つ条件評価器。
        @param[in] in_reserve_expressions
            状態監視器が持つ条件式識別値コンテナの予約容量。
        @retval 正 成功。条件式の評価を維持する。
        @retval 負 成功。条件式の評価を維持しない。
        @retval 0  失敗。
     */
    private: static std::int8_t register_compound_expression(
        typename this_type::status_monitor::container& io_status_monitors,
        typename this_type::expression_monitor::container const&
            in_expression_monitors,
        typename this_type::evaluator::expression::key const& in_register_key,
        typename this_type::evaluator::expression const& in_expression,
        typename this_type::evaluator::sub_expression_container const&
            in_sub_expressions,
        typename this_type::evaluator const& in_evaluator,
        std::size_t const in_reserve_expressions)
    {
        // in_expression の要素条件を走査し、
        // in_register_key を状態監視器へ登録する。
        std::int8_t local_result(1);
        for (auto i(in_expression.begin_); i < in_expression.end_; ++i)
        {
            auto const local_register_expression(
                this_type::register_expression(
                    io_status_monitors,
                    in_expression_monitors,
                    in_register_key,
                    in_sub_expressions.at(i).key_,
                    in_evaluator,
                    in_reserve_expressions));
            if (local_register_expression == 0)
            {
                // 無限ループを防ぐため、
                // まだ存在しない条件式を複合条件式で使うのは禁止する。
                PSYQ_ASSERT(false);
                return 0;
            }
            else if (local_register_expression < 0)
            {
                local_result = -1;
            }
        }
        return local_result;
    }

    /** @brief 状態変化を検知し、条件式監視器へ知らせる。
        @param[in,out] io_status_monitors     状態監視器のコンテナ。
        @param[in,out] io_expression_monitors 条件式監視器のコンテナ。
        @param[in] in_reservoir               状態変化を把握している状態貯蔵器。
     */
    private: static void detect_status_transition(
        typename this_type::status_monitor::container& io_status_monitors,
        typename this_type::expression_monitor::container&
            io_expression_monitors,
        typename this_type::evaluator::reservoir const& in_reservoir)
    {
        // io_status_monitors を走査しつつ、
        // 不要になった要素を削除してコンテナを整理する。
        for (
            auto i(io_status_monitors.begin());
            i != io_status_monitors.end();)
        {
            // 状態変化を検知したら、条件式監視器へ知らせる。
            /** @note
                reservoir::_get_transition は二分探索を行うが、
                監視しているすべての状態値に対し二分探索を毎回行うのは、
                計算量として問題にならないか気になる。計算量が問題になるなら、
                reservoir::status_container にハッシュ辞書を使うべきか。

                @todo
                状態値が存在しない場合、状態変化の通知を毎回行ってしまう。
                状態値が存在しなくなった瞬間を検知できるようにしたい。
             */
            auto& local_status_monitor(*i);
            auto const local_status_transition(
                in_reservoir._get_transition(local_status_monitor.key_));
            if (local_status_transition != 0)
            {
                this_type::notify_status_transition(
                    io_expression_monitors,
                    local_status_monitor.expression_keys_,
                    0 <= local_status_transition);

                // 状態監視器に対応する条件式監視器がなくなったら、
                // 状態監視器を削除する。
                if (local_status_monitor.expression_keys_.empty())
                {
                    i = io_status_monitors.erase(i);
                    continue;
                }
            }
            ++i;
        }
    }

    /** @brief 状態変化を条件式監視器へ知らせる。
        @param[in,out] io_expression_monitors 条件式監視器のコンテナ。
        @param[in,out] io_expression_keys     状態変化を知らせる条件式の識別値のコンテナ。
        @param[in] in_valid_transition        状態値があるかどうか。
     */
    private: static void notify_status_transition(
        typename this_type::expression_monitor::container&
            io_expression_monitors,
        typename this_type::status_monitor::expression_key_container&
            io_expression_keys,
        bool const in_valid_transition)
    {
        // 条件式識別値のコンテナを走査しつつ、
        // 不要になった要素を削除し、条件識別値コンテナを整理する。
        for (
            auto i(io_expression_keys.begin());
            i != io_expression_keys.end();)
        {
            auto const local_expression_monitor(
                this_type::expression_monitor::key_less::find_pointer(
                    io_expression_monitors, *i));
            if (local_expression_monitor != nullptr)
            {
                // 状態変化を条件式監視器へ知らせる。
                local_expression_monitor->flags_.set(
                    in_valid_transition?
                        this_type::expression_monitor::flag_VALID_TRANSITION:
                        this_type::expression_monitor::flag_INVALID_TRANSITION);
                ++i;
            }
            else
            {
                i = io_expression_keys.erase(i);
            }
        }
    }

    //-------------------------------------------------------------------------
    /// @brief 条件式監視器の辞書。
    private: typename this_type::expression_monitor::container
         expression_monitors_;

    /// @brief 状態監視器の辞書。
    private: typename this_type::status_monitor::container status_monitors_;

    /// @brief 条件挙動キャッシュのコンテナ。
    private: typename this_type::expression_monitor::behavior_cache_container
        behavior_caches_;

    /// @brief 多重に挙動関数を呼び出さないためのロック。
    private: bool dispatch_lock_;

}; // class psyq::if_then_engine::_private::dispatcher

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態監視器。

    条件式の要素条件が参照する状態を監視し、
    状態変化した際に、条件式の評価を更新するために使う。
 */
template<
    typename template_status_key,
    typename template_expression_key,
    typename template_allocator>
class psyq::if_then_engine::_private::status_monitor
{
    /// @brief thisが指す値の型。
    private: typedef status_monitor this_type;

    /// @brief 状態監視器のコンテナ。
    public: typedef std::vector<this_type, template_allocator> container;

    /// @brief 状態監視器を状態値の識別値の昇順で並び替えるのに使う、比較関数オブジェクト。
    public: typedef
        psyq::if_then_engine::_private::key_less<
            psyq::if_then_engine::_private::object_key_getter<
                this_type, template_status_key>>
        key_less;

    /// @brief 条件式の識別値のコンテナの型。
    public: typedef
        std::vector<template_expression_key, template_allocator>
        expression_key_container;

    /** @brief 状態監視器を構築する。
        @param[in] in_key       状態値の識別値。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: status_monitor(
        template_status_key in_key,
        template_allocator const& in_allocator)
    :
    expression_keys_(in_allocator),
    key_(std::move(in_key))
    {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: status_monitor(this_type&& io_source):
    expression_keys_(std::move(io_source.expression_keys_)),
    key_(std::move(io_source.key_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->expression_keys_ = std::move(io_source.expression_keys_);
        this->key_ = std::move(io_source.key_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /// @brief 評価の更新を要求する条件式の識別値のコンテナ。
    public: typename this_type::expression_key_container expression_keys_;
    /// @brief 状態値の識別値。
    public: template_status_key key_;

}; // class psyq::if_then_engine::_private::status_monitor

#endif // !defined(PSYQ_IF_THEN_ENGINE_DISPATCHER_HPP_)
// vim: set expandtab:
