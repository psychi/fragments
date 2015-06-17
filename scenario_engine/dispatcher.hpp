/** @file
    @brief @copybrief psyq::scenario_engine::dispatcher
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_DISPATCHER_HPP_
#define PSYQ_SCENARIO_ENGINE_DISPATCHER_HPP_

#include <bitset>
#include <memory>
#include <functional>
#include <vector>

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        template<typename, typename, typename> class dispatcher;
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief シナリオ条件挙動器。条件式の評価結果が変化すると、条件挙動関数を呼び出す。

    使い方の概略。
    - dispatcher::register_function を呼び出し、
      条件式の評価結果が変化したときに呼び出す条件挙動関数を登録する。
    - driver::update をフレーム毎に呼び出し、
      条件式の評価結果が変化を検知して、条件挙動関数を呼び出す。

    @tparam template_state_key      @copydoc dispatcher::state_key
    @tparam template_expression_key @copydoc dispatcher::expression_key
    @tparam template_allocator      @copydoc dispatcher::allocator_type
 */
template<
    typename template_state_key,
    typename template_expression_key,
    typename template_allocator>
class psyq::scenario_engine::dispatcher
{
    /// @brief thisが指す値の型。
    private: typedef dispatcher this_type;

    /// @brief 評価に用いる状態値の識別値を表す型。
    public: typedef template_state_key state_key;

    /// @brief 評価に用いる条件式の識別値を表す型。
    public: typedef template_expression_key expression_key;

    /// @brief コンテナに用いるメモリ割当子の型。
    public: typedef template_allocator allocator_type;

    //-------------------------------------------------------------------------
    /** @brief 条件式の評価結果が変化した際に呼び出す、条件挙動関数オブジェクトの型。

        - 引数#0は、評価に用いた条件式の識別値。
        - 引数#1は、 evaluator::evaluate_expression の今回の戻り値。
        - 引数#2は、 evaluator::evaluate_expression の前回の戻り値。
     */
    public: typedef std::function<
        void (
            typename this_type::expression_key const&,
            std::int8_t const,
            std::int8_t const)>
                function;

    /// @brief this_type::function の、所有権ありスマートポインタ。
    public: typedef std::shared_ptr<typename this_type::function>
        function_shared_ptr;

    /// @brief this_type::function の、所有権なしスマートポインタ。
    public: typedef std::weak_ptr<typename this_type::function>
        function_weak_ptr;

    /// @brief 条件挙動関数オブジェクトの所有権ありスマートポインタのコンテナを表す型。
    public: typedef std::vector<
        typename this_type::function_shared_ptr,
        typename this_type::allocator_type>
            function_shared_ptr_vector;

    /// @brief 条件挙動関数オブジェクトの所有権なしスマートポインタのコンテナを表す型。
    public: typedef std::vector<
        typename this_type::function_weak_ptr,
        typename this_type::allocator_type>
            function_weak_ptr_vector;

    //-------------------------------------------------------------------------
    private: struct expression_monitor;

    /// @brief 条件式監視器のコンテナ。
    private: typedef std::vector<
        typename this_type::expression_monitor,
        typename this_type::allocator_type>
            expression_monitor_vector;

    /// @brief 条件式監視器を条件式の識別値の昇順で並び替えるのに使う、比較関数オブジェクト。
    private: typedef psyq::scenario_engine::_private::key_less<
        typename this_type::expression_monitor,
        typename this_type::expression_key>
            expression_monitor_key_less;

    /// @brief 条件式の識別値のコンテナの型。
    private: typedef std::vector<
        typename this_type::expression_key, typename this_type::allocator_type>
            expression_key_vector;

    //-------------------------------------------------------------------------
    private: struct state_monitor;

    /// @brief 状態監視器のコンテナ。
    private: typedef std::vector<
        typename this_type::state_monitor, typename this_type::allocator_type>
            state_monitor_vector;

    /// @brief 状態監視器を状態値の識別値の昇順で並び替えるのに使う、比較関数オブジェクト。
    private: typedef psyq::scenario_engine::_private::key_less<
        typename this_type::state_monitor, typename this_type::state_key>
            state_monitor_key_less;

    //-------------------------------------------------------------------------
    /// @brief 条件挙動関数キャッシュ。
    private: struct function_cache
    {
        function_cache(
            typename dispatcher::function_weak_ptr in_function,
            typename dispatcher::expression_key in_expression_key,
            std::int8_t const in_evaluation,
            std::int8_t const in_last_evaluation)
        :
        function(std::move(in_function)),
        expression_key(std::move(in_expression_key)),
        evaluation(in_evaluation),
        last_evaluation(in_last_evaluation)
        {}

        void call_function() const
        {
            auto const local_function_holder(this->function.lock());
            auto const local_function(local_function_holder.get());
            if (local_function != nullptr)
            {
                (*local_function)(
                    this->expression_key,
                    this->evaluation,
                    this->last_evaluation);
            }
        }

        typename dispatcher::function_weak_ptr function;
        typename dispatcher::expression_key expression_key;
        std::int8_t evaluation;
        std::int8_t last_evaluation;

    }; // struct function_cache

    /// @brief 条件挙動関数キャッシュのコンテナ。
    private: typedef std::vector<
         typename this_type::function_cache,
         typename this_type::allocator_type>
             function_cache_vector;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 空の条件挙動器を構築する。
        @param[in] in_reserve_expressions 監視する条件式の予約数。
        @param[in] in_reserve_states      監視する状態値の予約数。
        @param[in] in_reserve_caches      条件挙動キャッシュの予約数。
        @param[in] in_allocator           メモリ割当子の初期値。
     */
    public: dispatcher(
        std::size_t const in_reserve_expressions,
        std::size_t const in_reserve_states,
        std::size_t const in_reserve_caches,
        typename this_type::allocator_type const& in_allocator)
    :
    expression_monitors_(in_allocator),
    state_monitors_(in_allocator),
    function_caches_(in_allocator),
    dispatch_lock_(false)
    {
        this->expression_monitors_.reserve(in_reserve_expressions);
        this->state_monitors_.reserve(in_reserve_states);
        this->function_caches_.reserve(in_reserve_caches);
    }

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: dispatcher(this_type&& io_source):
    expression_monitors_(std::move(io_source.expression_monitors_)),
    state_monitors_(std::move(io_source.state_monitors_)),
    function_caches_(std::move(io_source.function_caches_)),
    dispatch_lock_(false)
    {
        /// @note this_type::_dispatch 実行中はムーブできない。
        PSYQ_ASSERT(!io_source.dispatch_lock_);
        io_source.expression_monitors_.clear();
        io_source.state_monitors_.clear();
        io_source.function_caches_.clear();
    }

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        /// @note this_type::_dispatch 実行中はムーブできない。
        PSYQ_ASSERT(!this->dispatch_lock_ && !io_source.dispatch_lock_);
        if (this != &io_source)
        {
            this->expression_monitors_ = std::move(io_source.expression_monitors_);
            this->state_monitors_ = std::move(io_source.state_monitors_);
            this->function_caches_ = std::move(io_source.function_caches_);
            io_source.expression_monitors_.clear();
            io_source.state_monitors_.clear();
            io_source.function_caches_.clear();
        }
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
        this_type::rebuild_state_monitor(
            this->state_monitors_, this->expression_monitors_);
        //this->function_caches_.shrink_to_fit();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 条件挙動
    //@{
    /** @brief 条件式に対応する条件挙動関数を登録する。

        this_type::_dispatch で条件式の評価が変化した際に呼び出す、
        条件挙動関数を登録する。

        登録された条件挙動関数は、スマートポインタが空になると、
        自動的に取り除かれる。明示的に条件挙動関数を取り除くには、
        this_type::unregister_function を呼び出す。

        @param[in] in_expression_key 評価に用いる条件式の識別値。
        @param[in] in_function
            登録する条件挙動関数オブジェクトを指すスマートポインタ。
        @param[in] in_reserve_functions
            条件式監視器が持つ条件挙動関数オブジェクトコンテナの予約容量。
        @retval true 成功。条件挙動関数オブジェクトを登録した。
        @retval false
            失敗。条件挙動関数オブジェクトは登録されなかった。
            条件挙動関数を指すスマートポインタが空だったか、
            同じ条件式に同じ条件挙動関数がすでに登録されていたのが原因。

        @todo
        実際の条件挙動関数の登録は、 _dispatch で識別値の昇順に行われる。
        このため register_function を呼び出した順序と異なってしまう。
        register_function で優先順位を指定できるようにしたい。
     */
    public: bool register_function(
        typename this_type::expression_key const& in_expression_key,
        typename this_type::function_shared_ptr const& in_function,
        //std::int32_t const in_priority = 0,
        std::size_t const in_reserve_functions = 1)
    {
        auto const local_function(in_function.get());
        if (local_function == nullptr)
        {
            return false;
        }

        // 関数オブジェクトを登録する条件式監視器を決定する。
        auto local_expression_monitor(
            std::lower_bound(
                this->expression_monitors_.begin(),
                this->expression_monitors_.end(),
                in_expression_key,
                typename this_type::expression_monitor_key_less()));
        if (local_expression_monitor != this->expression_monitors_.end()
            && local_expression_monitor->key_ == in_expression_key)
        {
            // 同じ関数オブジェクトがすでに登録済みなら、失敗する。
            auto const local_find_function(
                this_type::find_function(
                    local_expression_monitor->functions, *local_function));
            if (local_find_function)
            {
                return false;
            }
        }
        else
        {
            // 条件式監視器を新たに生成し、コンテナに挿入する。
            local_expression_monitor = this->expression_monitors_.insert(
                local_expression_monitor,
                typename this_type::expression_monitor(
                    in_expression_key, this->get_allocator()));
        }

        // 関数オブジェクトを条件式監視器へ追加する。
        local_expression_monitor->functions.reserve(in_reserve_functions);
        local_expression_monitor->functions.push_back(in_function);
        return true;
    }

    /** @brief 条件式に対応する条件挙動関数を取り除く。

        this_type::register_function で登録した条件挙動関数を、
        条件式監視器から取り除く。

        @param[in] in_expression_key 条件式の識別値。
        @param[in] in_function       取り除く条件挙動関数。
     */
    public: void unregister_function(
        typename this_type::expression_key const& in_expression_key,
        typename this_type::function const& in_function)
    {
        auto const local_expression_monitor(
            this_type::expression_monitor_key_less::find_pointer(
                this->expression_monitors_, in_expression_key));
        if (local_expression_monitor != nullptr)
        {
            this_type::remove_function(
                local_expression_monitor->functions, in_function);
        }
    }

    /** @brief 条件式に対応する条件挙動関数をすべて取り除く。

        条件式監視器を削除し、 this_type::register_function
        で同じ条件式に登録した条件挙動関数を、すべて取り除く。

        @param[in] in_expression_key 条件式の識別値。
     */
    public: void unregister_function(
        typename this_type::expression_key const& in_expression_key)
    {
        auto const local_expression_monitor(
            this_type::expression_monitor_key_less::find_const_iterator(
                this->expression_monitors_, in_expression_key));
        if (local_expression_monitor != this->expression_monitors_.end())
        {
            this->expression_monitors_.erase(local_expression_monitor);
        }
    }

    /** @brief 条件挙動関数を取り除く。

        this_type::register_function で登録した条件挙動関数を、
        すべての条件式監視器から取り除く。

        @param[in] in_function 削除する条件挙動関数。
     */
    public: void unregister_function(
        typename this_type::function const& in_function)
    {
        for (auto& local_expression_monitor: this->expression_monitors_)
        {
            this_type::remove_function(
                local_expression_monitor.functions, in_function);
        }
    }

    /** @brief psyq::scenario_engine 管理者以外は、この関数は使用禁止。

        条件式の評価が変化していれば、
        this_type::register_function で登録した条件挙動関数を呼び出す。

        @param[in] in_evaluator     条件式の評価に使う条件評価器。
        @param[in,out] io_reservoir 条件式の評価に使う状態貯蔵器。
        @param[in] in_reserve_expressions
            状態監視器で使う条件式識別値コンテナの予約容量。

        @note
        前回の this_type::_dispatch と今回の this_type::_dispatch
        で条件式の評価が違った場合に、条件挙動関数を呼び出す。
        このため、前回から今回の間（基本的には1フレームの間）で条件式の評価が
        true → false → true と変化した場合、
        条件挙動関数を呼び出さないことに注意。
     */
    public: template<typename template_evaluator>
    void _dispatch(
        template_evaluator const& in_evaluator,
        typename template_evaluator::reservoir& io_reservoir,
        std::size_t const in_reserve_expressions = 1)
    {
        // _dispatch を多重に実行しないようにロックする。
        if (this->dispatch_lock_)
        {
            PSYQ_ASSERT(false);
            return;
        }
        this->dispatch_lock_ = true;

        // 状態変化を検知する条件式を、状態監視器へ登録する。
        this->register_expressions(in_evaluator, in_reserve_expressions);

        // 状態変化を検知し、条件式監視器へ知らせる。
        this_type::detect_state_transition(
            this->expression_monitors_, this->state_monitors_, io_reservoir);

        // 状態変化した条件式を評価し、条件挙動関数をキャッシュに貯める。
        this->function_caches_.clear();
        this_type::cache_functions(
            this->function_caches_,
            this->expression_monitors_,
            in_evaluator,
            io_reservoir);

        // 条件式の評価が済んだので、状態変化フラグを初期化する。
        io_reservoir._reset_transition();

        // キャッシュに貯まった条件挙動関数を呼び出す。
        for (auto const& local_cache: this->function_caches_)
        {
            local_cache.call_function();
        }

        // 条件挙動関数のキャッシュを片づける。
        this->function_caches_.clear();
        PSYQ_ASSERT(this->dispatch_lock_);
        this->dispatch_lock_ = false;
    }
    //@}
    /** @brief 状態値を操作する関数オブジェクトを生成する。
        @param[in,out] io_reservoir 関数から参照する状態貯蔵器。
        @param[in] in_condition     関数の起動条件。
        @param[in] in_state_key     操作する状態値の識別値。
        @param[in] in_operator      状態値の操作で使う演算子。
        @param[in] in_value         状態値の操作で使う演算値。
        @param[in] in_allocator     生成に使うメモリ割当子。
        @return 生成した条件挙動関数オブジェクト。
     */
    public: template<typename template_reservoir>
    static typename this_type::function_shared_ptr
    make_state_operation_function(
        template_reservoir& io_reservoir,
        bool const in_condition,
        typename template_reservoir::state_key const& in_state_key,
        typename template_reservoir::state_value::operation_enum const in_operator,
        typename template_reservoir::state_value const& in_value,
        typename this_type::allocator_type const& in_allocator)
    {
        // 状態値を書き換える関数オブジェクトを生成する。
        return std::allocate_shared<typename this_type::function>(
            in_allocator,
            typename this_type::function(
                /// @todo io_reservoir を参照渡しするのは危険。対策を考えたい。
                [=, &io_reservoir](
                    typename this_type::expression_key const&,
                    std::int8_t const in_evaluation,
                    std::int8_t const in_last_evaluation)
                {
                    // 条件と評価が合致すれば、状態値を書き換える。
                    if (0 <= in_last_evaluation
                        && 0 <= in_evaluation
                        && in_condition == (0 < in_evaluation))
                    {
                        this_type::operate_state(
                            io_reservoir, in_state_key, in_operator, in_value);
                    }
                }));
    }

    //-------------------------------------------------------------------------
    /** @brief 条件式監視器を再構築する。
        @param[in,out] io_expression_monitors 再構築する条件式監視器のコンテナ。
     */
    private: static void rebuild_expression_monitor(
        typename this_type::expression_monitor_vector& io_expression_monitors)
    {
        /// @note std::vector なら、逆順で走査したほうが効率いいかも。
        for (
            auto i(io_expression_monitors.begin());
            i != io_expression_monitors.end();)
        {
            auto& local_functions(i->functions);
            for (auto j(local_functions.begin()); j != local_functions.end();)
            {
                if (j->expired())
                {
                    j = local_functions.erase(j);
                }
                else
                {
                    ++j;
                }
            }
            if (local_functions.empty())
            {
                i = io_expression_monitors.erase(i);
            }
            else
            {
                local_functions.shrink_to_fit();
                ++i;
            }
        }
        io_expression_monitors.shrink_to_fit();
    }

    /** @brief 状態監視器を再構築する。
        @param[in,out] io_state_monitors  再構築する状態監視器のコンテナ。
        @param[in] in_expression_monitors 状態監視器が使っている条件式監視器のコンテナ。
     */
    private: static void rebuild_state_monitor(
        typename this_type::state_monitor_vector& io_state_monitors,
        typename this_type::expression_monitor_vector const& in_expression_monitors)
    {
        /// @note std::vector なら、逆順で走査したほうが効率いいかも。
        for (auto i(io_state_monitors.begin()); i != io_state_monitors.end();)
        {
            auto& local_expression_keys(i->expression_keys_);
            for (
                auto j(local_expression_keys.begin());
                j != local_expression_keys.end();)
            {
                auto const local_expression_monitor(
                    this_type::expression_monitor_key_less::find_const_pointer(
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
                i = io_state_monitors.erase(i);
            }
            else
            {
                local_expression_keys.shrink_to_fit();
                ++i;
            }
            local_expression_keys.shrink_to_fit();
        }
        io_state_monitors.shrink_to_fit();
    }

    //-------------------------------------------------------------------------
    /** @brief 関数オブジェクトを検索しつつ、コンテナを整理する。
        @param[in,out] io_functions 整理する関数オブジェクトコンテナ。
        @param[in] in_function      検索する関数オブジェクト。
        @retval true  検索対象がコンテナから見つかった。
        @retval false 検索対象がコンテナから見つからなかった。
     */
    private: static bool find_function(
        typename this_type::function_weak_ptr_vector& io_functions,
        typename this_type::function const& in_function)
    {
        // 関数オブジェクトのコンテナを走査し、検索対象を見つけながら、
        // 空になった要素を削除する。
        auto local_find(false);
        for (auto i(io_functions.begin()); i != io_functions.end();)
        {
            if (!local_find && i->lock().get() == &in_function)
            {
                local_find = true;
            }
            if (i->expired())
            {
                i = io_functions.erase(i);
            }
            else
            {
                ++i;
            }
        }
        return local_find;
    }

    /** @brief 関数オブジェクトを削除しつつ、コンテナを整理する。
        @param[in,out] io_functions 整理する関数オブジェクトコンテナ。
        @param[in] in_function      削除する関数オブジェクト。
        @retval true  削除対象がコンテナから見つかった。
        @retval false 削除対象がコンテナから見つからなかった。
     */
    private: static bool remove_function(
        typename this_type::function_weak_ptr_vector& io_functions,
        typename this_type::function const& in_function)
    {
        // 関数オブジェクトのコンテナを走査し、削除対象を見つけながら、
        // 空になった要素を削除する。
        auto local_find(false);
        for (auto i(io_functions.begin()); i != io_functions.end();)
        {
            auto const local_function(i->lock().get());
            if (local_function != nullptr)
            {
                if (local_find || local_function != &in_function)
                {
                    ++i;
                    continue;
                }
                local_find = true;
            }
            i = io_functions.erase(i);
        }
        return local_find;
    }

    //-------------------------------------------------------------------------
    /** @brief 状態変化を検知する条件式を、状態監視器へ登録する。
        @param[in] in_evaluator 登録する条件式を持つ条件評価器。
        @param[in] in_reserve_expressions
            状態監視器が持つ条件式識別値コンテナの予約容量。
     */
    private: template<typename template_evaluator>
    void register_expressions(
        template_evaluator const& in_evaluator,
        std::size_t const in_reserve_expressions)
    {
        // 条件式監視器のコンテナを走査し、
        // 登録が完了してないものは、状態監視器に条件式を登録する。
        for (auto& local_expression_monitor: this->expression_monitors_)
        {
            auto const local_registered(
                local_expression_monitor.flags.test(
                    this_type::expression_monitor::flag_REGISTERED));
            if (!local_registered)
            {
                auto const local_register_expression(
                    this->register_expression(
                        local_expression_monitor.key_,
                        local_expression_monitor.key_,
                        in_evaluator,
                        in_reserve_expressions));
                if (local_register_expression != 0)
                {
                    local_expression_monitor.flags.set(
                        this_type::expression_monitor::flag_FLUSH_CONDITION,
                        local_register_expression < 0);
                    local_expression_monitor.flags.set(
                        this_type::expression_monitor::flag_REGISTERED);
                }
            }
        }
    }

    /** @brief 状態変化を検知する条件式を、状態監視器へ登録する。
        @param[in] in_register_key   登録する条件式の識別値。
        @param[in] in_expression_key 走査する条件式の識別値。
        @param[in] in_evaluator      走査する条件式を持つ条件評価器。
        @param[in] in_reserve_expressions
            状態監視器が持つ条件式識別値コンテナの予約容量。
        @retval 正 成功。条件式の評価を維持する。
        @retval 負 成功。条件式の評価を維持しない。
        @retval 0  失敗。
     */
    private: template<typename template_evaluator>
    std::int8_t register_expression(
        typename this_type::expression_key const& in_register_key,
        typename this_type::expression_key const& in_expression_key,
        template_evaluator const& in_evaluator,
        std::size_t const in_reserve_expressions)
    {
        // 条件式と要素条件チャンクを検索する。
        auto const local_expression(
            in_evaluator._find_expression(in_expression_key));
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

        // 条件式の種類によって、監視する条件式の追加先を選別する。
        switch (local_expression->kind_)
        {
            case template_evaluator::expression::kind_SUB_EXPRESSION:
            return this->register_sub_expression(
                in_register_key,
                *local_expression,
                local_chunk->sub_expressions_,
                in_evaluator,
                in_reserve_expressions);

            case template_evaluator::expression::kind_STATE_TRANSITION:
            this_type::register_expression(
                this->state_monitors_,
                in_register_key,
                *local_expression,
                local_chunk->state_transitions_,
                in_reserve_expressions);
            return -1;

            case template_evaluator::expression::kind_STATE_COMPARISON:
            this_type::register_expression(
                this->state_monitors_,
                in_register_key,
                *local_expression,
                local_chunk->state_comparisons_,
                in_reserve_expressions);
            return 1;

            default:
            // 未対応の条件式の種類だった。
            PSYQ_ASSERT(false);
            return 0;
        }
    }

    /** @brief 状態変化を検知する条件式を、状態監視器へ登録する。
        @param[in,out] io_state_monitors 条件式を登録する状態監視器のコンテナ。
        @param[in] in_register_key       登録する条件式の識別値。
        @param[in] in_expression         走査する条件式。
        @param[in] in_elements           条件式が参照する要素条件のコンテナ。
        @param[in] in_reserve_expressions
            状態監視器が持つ条件式識別値コンテナの予約容量。
     */
    private: template<
        typename template_expression,
        typename template_element_container>
    static void register_expression(
        typename this_type::state_monitor_vector& io_state_monitors,
        typename this_type::expression_key const& in_register_key,
        template_expression const& in_expression,
        template_element_container const& in_elements,
        std::size_t const in_reserve_expressions)
    {
        // 条件式が使う要素条件を走査し、
        // 要素条件ごとに状態監視器へ条件式を登録する。
        auto const local_begin(in_elements.begin());
        auto const local_end(local_begin + in_expression.end_);
        for (auto i(local_begin + in_expression.begin_); i != local_end; ++i)
        {
            // 条件式を登録する状態監視器を取得する。
            auto const& local_state_key(i->key_);
            auto local_state_monitor(
                std::lower_bound(
                    io_state_monitors.begin(),
                    io_state_monitors.end(),
                    local_state_key,
                    typename this_type::state_monitor_key_less()));
            if (local_state_monitor == io_state_monitors.end()
                || local_state_monitor->key_ != local_state_key)
            {
                // 要素条件に対応する状態監視器がなかったので、
                // 状態監視器を新たに生成する。
                local_state_monitor = io_state_monitors.insert(
                    local_state_monitor,
                    typename this_type::state_monitor(
                        local_state_key, io_state_monitors.get_allocator()));
            }

            // 条件式の識別値を、状態監視器へ登録する。
            auto& local_expression_keys(local_state_monitor->expression_keys_);
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

    /** @brief 状態変化を検知する複合条件式を、状態監視器へ登録する。
        @param[in] in_register_key 登録する条件式の識別値。
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
    private: template<typename template_evaluator>
    std::int8_t register_sub_expression(
        typename this_type::expression_key const& in_register_key,
        typename template_evaluator::expression const& in_expression,
        typename template_evaluator::sub_expression_vector const&
            in_sub_expressions,
        template_evaluator const& in_evaluator,
        std::size_t const in_reserve_expressions)
    {
        // 複合条件式の要素条件を走査し、状態監視器に条件式を登録する。
        auto const local_begin(in_sub_expressions.begin());
        auto const local_end(local_begin + in_expression.end_);
        std::int8_t local_result(1);
        for (auto i(local_begin + in_expression.begin_); i != local_end; ++i)
        {
            auto const& local_sub_key(i->key_);
            auto const local_expression_monitor(
                this_type::expression_monitor_key_less::find_const_pointer(
                    this->expression_monitors_, local_sub_key));
            if (local_expression_monitor == nullptr
                || !local_expression_monitor->flags.test(
                    this_type::expression_monitor::flag_REGISTERED))
            {
                auto const local_register_expression(
                    this->register_expression(
                        in_register_key,
                        local_sub_key,
                        in_evaluator,
                        in_reserve_expressions));
                if (local_register_expression == 0)
                {
                    // 無限ループを防ぐため、
                    // まだ存在しない条件式を複合条件式で使うのは禁止する。
                    PSYQ_ASSERT(false);
                    return 0;
                }
                if (local_register_expression < 0)
                {
                    local_result = -1;
                }
            }
        }
        return local_result;
    }

    /** @brief 状態変化を検知し、条件式監視器へ知らせる。
        @param[in,out] io_expression_monitors 条件式監視器のコンテナ。
        @param[in,out] io_state_monitors      状態監視器のコンテナ。
        @param[in] in_reservoir               変化を検知する状態貯蔵器。
     */
    private: template<typename template_reservoir>
    static void detect_state_transition(
        typename this_type::expression_monitor_vector& io_expression_monitors,
        typename this_type::state_monitor_vector& io_state_monitors,
        template_reservoir const& in_reservoir)
    {
        // 状態監視器のコンテナを走査しつつ、
        // 不要になった要素を削除し、状態監視器のコンテナを整理する。
        for (auto i(io_state_monitors.begin()); i != io_state_monitors.end();)
        {
            // 状態変化を検知したら、条件式監視器へ知らせる。
            auto& local_state_monitor(*i);
            auto const local_state_transition(
                in_reservoir._get_transition(local_state_monitor.key_));
            if (local_state_transition != 0)
            {
                this_type::notify_state_transition(
                    io_expression_monitors,
                    local_state_monitor.expression_keys_,
                    0 <= local_state_transition);

                // 状態監視器に対応する条件式監視器がなくなったら、
                // 状態監視器を削除する。
                if (local_state_monitor.expression_keys_.empty())
                {
                    i = io_state_monitors.erase(i);
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
    private: static void notify_state_transition(
        typename this_type::expression_monitor_vector& io_expression_monitors,
        typename this_type::expression_key_vector& io_expression_keys,
        bool const in_valid_transition)
    {
        // 条件式識別値のコンテナを走査しつつ、
        // 不要になった要素を削除し、条件識別値コンテナを整理する。
        for (auto i(io_expression_keys.begin()); i != io_expression_keys.end();)
        {
            auto const local_expression_monitor(
                this_type::expression_monitor_key_less::find_pointer(
                    io_expression_monitors, *i));
            if (local_expression_monitor != nullptr)
            {
                // 状態変化を条件式監視器へ知らせる。
                local_expression_monitor->flags.set(
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

    /** @brief 条件式を評価し、条件挙動関数をキャッシュに貯める。

        条件式監視器のコンテナを走査し、条件式の結果によって、
        this_type::register_function で登録された条件挙動関数をキャッシュに貯める。

        @param[in,out] io_function_caches     条件挙動関数キャッシュのコンテナ。
        @param[in,out] io_expression_monitors 条件式監視器のコンテナ。
        @param[in] in_evaluator               評価に使う条件評価器。
        @param[in] in_reservoir               条件式の評価に状態値のコンテナ。
     */
    private: template<typename template_evaluator>
    static void cache_functions(
        typename this_type::function_cache_vector& io_function_caches,
        typename this_type::expression_monitor_vector& io_expression_monitors,
        template_evaluator const& in_evaluator,
        typename template_evaluator::reservoir const& in_reservoir)
    {
        // 条件式監視器のコンテナを走査し、評価要求があれば、条件式を評価する。
        for (
            auto i(io_expression_monitors.begin());
            i != io_expression_monitors.end();)
        {
            auto& local_expression_monitor(*i);
            if (local_expression_monitor.get_evaluation_request())
            {
                this_type::cache_function(
                    io_function_caches,
                    local_expression_monitor,
                    in_evaluator,
                    in_reservoir);
                if (local_expression_monitor.functions.empty())
                {
                    // 条件挙動関数のコンテナが空になったら、
                    // 条件式監視器を削除する。
                    i = io_expression_monitors.erase(i);
                    continue;
                }
            }
            ++i;
        }
    }

    /** @brief 条件式を評価し、条件挙動関数をキャッシュに貯める。

        条件式を評価し、前回の結果と異なるなら、 this_type::register_function
        で登録された条件挙動関数をキャッシュに貯める。

        @param[in,out] io_function_caches    条件挙動関数キャッシュのコンテナ。
        @param[in,out] io_expression_monitor 更新する条件式監視器。
        @param[in] in_evaluator              評価に用いる条件評価器。
        @param[in] in_reservoir              評価に用いる状態貯蔵器。
        @retval true  条件挙動関数をキャッシュに貯めた。
        @retval false 条件挙動関数はキャッシュに貯めなかった。
     */
    private: template<typename template_evaluator>
    static bool cache_function(
        typename this_type::function_cache_vector& io_function_caches,
        typename this_type::expression_monitor& io_expression_monitor,
        template_evaluator const& in_evaluator,
        typename template_evaluator::reservoir const& in_reservoir)
    {
        // 条件式を評価し、評価結果が前回と同じか判定する。
        auto const local_flush_condition(
            io_expression_monitor.flags.test(
                this_type::expression_monitor::flag_FLUSH_CONDITION));
        auto const local_last_evaluation(
            io_expression_monitor.get_last_evaluation(local_flush_condition));
        auto const local_evaluation(
            io_expression_monitor.evaluate_expression(
                in_evaluator, in_reservoir, local_flush_condition));
        if (local_evaluation == local_last_evaluation
            || local_evaluation * local_last_evaluation < 0)
        {
            return false;
        }

        // 条件式の評価結果が前回と違うので、条件挙動関数をキャッシュに貯める。
        for (
            auto i(io_expression_monitor.functions.begin());
            i != io_expression_monitor.functions.end();)
        {
            if (i->expired())
            {
                i = io_expression_monitor.functions.erase(i);
            }
            else
            {
                io_function_caches.emplace_back(
                    *i,
                    io_expression_monitor.key_,
                    local_evaluation,
                    local_last_evaluation);
                ++i;
            }
        }
        return true;
    }

    //-------------------------------------------------------------------------
    private: template<typename template_reservoir>
    static bool operate_state(
        template_reservoir& io_reservoir,
        typename template_reservoir::state_key const& in_state_key,
        typename template_reservoir::state_value::operation_enum const in_operator,
        typename template_reservoir::state_value const& in_value)
    {
        /** @todo
            今のところ状態値に定数を設定するしかできないが、
            状態値に他の状態値を設定できるようにしたい。
         */
        auto local_state(io_reservoir.get_value(in_state_key));
        auto const local_set_value(
            local_state.compute(in_operator, in_value)
            && io_reservoir.set_value(in_state_key, local_state));
        PSYQ_ASSERT(local_set_value);
        return local_set_value;
    }

    //-------------------------------------------------------------------------
    /// @brief 条件式監視器の辞書。
    private: typename this_type::expression_monitor_vector expression_monitors_;

    /// @brief 状態監視器の辞書。
    private: typename this_type::state_monitor_vector state_monitors_;

    /// @brief 条件挙動関数キャッシュのコンテナ。
    private: typename this_type::function_cache_vector function_caches_;

    /// @brief 多重に条件挙動関数を呼び出さないためのロック。
    private: bool dispatch_lock_;

}; // class psyq::scenario_engine::dispatcher

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 条件式監視器。

    条件式の評価結果と、
    条件式の評価結果が変化した際に呼び出す関数オブジェクトを保持する。
 */
template<
    typename template_state_key,
    typename template_expression_key,
    typename template_allocator>
struct psyq::scenario_engine::dispatcher<
    template_state_key,
    template_expression_key,
    template_allocator>
        ::expression_monitor
{
    typedef expression_monitor this_type;

    /// @brief フラグの位置。
    enum flag_enum: std::uint8_t
    {
        flag_VALID_TRANSITION,   ///< 状態変化の取得に成功。
        flag_INVALID_TRANSITION, ///< 状態変化の取得に失敗。
        flag_LAST_EVALUATION,    ///< 条件の前回の評価の成功／失敗。
        flag_LAST_CONDITION,     ///< 条件の前回の評価。
        flag_FLUSH_CONDITION,    ///< 条件の前回の評価を無視する。
        flag_REGISTERED,         ///< 条件式の登録済みフラグ。
    };

    /** @brief 条件式監視器を構築する。
        @param[in] in_key       監視する条件式の識別値。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    expression_monitor(
        typename dispatcher::expression_key in_key,
        typename dispatcher::allocator_type const& in_allocator)
    :
    key_(std::move(in_key)),
    functions(in_allocator)
    {}

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    expression_monitor(this_type&& io_source):
    functions(std::move(io_source.functions)),
    key_(std::move(io_source.key_)),
    flags(std::move(io_source.flags))
    {
        io_source.functions.clear();
    }

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->functions = std::move(io_source.functions);
            this->key_ = std::move(io_source.key_);
            this->flags = std::move(io_source.flags);
            io_source.functions.clear();
        }
        return *this;
    }

    bool get_evaluation_request() const PSYQ_NOEXCEPT
    {
        return this->flags.test(this_type::flag_VALID_TRANSITION)
            || this->flags.test(this_type::flag_INVALID_TRANSITION);
    }

    /** @brief 監視している条件式の前回の評価を取得する。
        @param[in] in_flush 前回の評価を無視する。
        @retval 正 条件式の評価は true となった。
        @retval  0 条件式の評価は false となった。
        @retval 負 条件式の評価に失敗した。
     */
    std::int8_t get_last_evaluation(bool const in_flush)
    const PSYQ_NOEXCEPT
    {
        return this->flags.test(this_type::flag_LAST_EVALUATION)?
            !in_flush && this->flags.test(this_type::flag_LAST_CONDITION):
            -1;
    }

    /** @brief 監視している条件式を評価する。
        @param[in] in_evaluator 評価に用いる条件評価器。
        @param[in] in_reservoir 評価に用いる状態貯蔵器。
        @param[in] in_flush     前回の評価を無視するかどうか。
        @retval 正 条件式の評価は true となった。
        @retval  0 条件式の評価は false となった。
        @retval 負 条件式の評価に失敗した。
     */
    template<typename template_evaluator>
    std::int8_t evaluate_expression(
        template_evaluator const& in_evaluator,
        typename template_evaluator::reservoir const& in_reservoir,
        bool const in_flush)
    {
        // 状態変化フラグを更新する。
        auto const local_invalid_transition(
            this->flags.test(this_type::flag_INVALID_TRANSITION));
        this->flags.reset(this_type::flag_VALID_TRANSITION);
        this->flags.reset(this_type::flag_INVALID_TRANSITION);

        // 状態値の取得に失敗していたら、条件式の評価も失敗とみなす。
        if (local_invalid_transition)
        //if (in_flush && local_invalid_transition)
        {
            this->flags.reset(this_type::flag_LAST_EVALUATION);
            this->flags.reset(this_type::flag_LAST_CONDITION);
            return -1;
        }

        // 条件式を評価し、結果を記録する。
        auto const local_evaluate_expression(
            in_evaluator.evaluate_expression(this->key_, in_reservoir));
        this->flags.set(
            this_type::flag_LAST_EVALUATION,
            0 <= local_evaluate_expression);
        this->flags.set(
            this_type::flag_LAST_CONDITION,
            0 < local_evaluate_expression);
        return this->get_last_evaluation(false);
    }

    /// @brief 条件挙動関数オブジェクトのコンテナ。
    typename dispatcher::function_weak_ptr_vector functions;
    /// @brief 監視している条件式の識別値。
    typename dispatcher::expression_key key_;
    /// @brief フラグの集合。
    std::bitset<8> flags;

}; // struct psyq::scenario_engine::dispatcher::expression_monitor

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態監視器。

    条件式の要素条件が参照する状態を監視し、
    状態変化した際に、条件式の評価を更新するために使う。
 */
template<
    typename template_state_key,
    typename template_expression_key,
    typename template_allocator>
struct psyq::scenario_engine::dispatcher<
    template_state_key,
    template_expression_key,
    template_allocator>
        ::state_monitor
{
    typedef state_monitor this_type;

    /** @brief 状態監視器を構築する。
        @param[in] in_key       状態値の識別値。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    state_monitor(
        typename dispatcher::state_key in_key,
        typename dispatcher::allocator_type const& in_allocator)
    :
    expression_keys_(in_allocator),
    key_(std::move(in_key))
    {}

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    state_monitor(this_type&& io_source):
    expression_keys_(std::move(io_source.expression_keys_)),
    key_(std::move(io_source.key_))
    {
        io_source.expression_keys_.clear();
    }

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->expression_keys_ = std::move(io_source.expression_keys_);
            this->key_ = std::move(io_source.key_);
            io_source.expression_keys_.clear();
        }
        return *this;
    }

    /// @brief 評価の更新を要求する条件式の識別値のコンテナ。
    typename dispatcher::expression_key_vector expression_keys_;
    /// @brief 状態値の識別値。
    typename dispatcher::state_key key_;

}; // struct psyq::scenario_engine::dispatcher::state_monitor

#endif // !defined(PSYQ_SCENARIO_ENGINE_DISPATCHER_HPP_)
// vim: set expandtab:
