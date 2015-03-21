/** @file
    @copydoc psyq::scenario_engine::dispatcher
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_DISPATCHER_HPP_
#define PSYQ_SCENARIO_ENGINE_DISPATCHER_HPP_

#include <bitset>
#include <memory>
#include <functional>
#include <map>

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
/** @brief 条件監視器。条件式を監視し、評価結果が変化すると、登録されている関数を呼び出す。

    使い方の概略。
    - 条件式の評価に用いる状態が変化したときに、
      変化した状態に対応する以下の関数を呼び出すよう実装しておく。
      - dispatcher::notify_state_change
    - dispatcher::register_function を呼び出し、
      条件式の評価結果が変化したときに呼び出す関数を登録する。
    - dispatcher::dispatch をフレーム毎に呼び出し、
      条件式の評価結果を更新して、登録された関数を呼び出す。

    @tparam template_expression_key @copydoc dispatcher::expression_key
    @tparam template_monitor_key    @copydoc dispatcher::monitor::key_type
    @tparam template_allocator      @copydoc dispatcher::allocator_type
 */
template<
    typename template_expression_key,
    typename template_monitor_key,
    typename template_allocator>
class psyq::scenario_engine::dispatcher
{
    /// @brief thisが指す値の型。
    private: typedef dispatcher this_type;

    /// @brief コンテナに用いるメモリ割当子を表す型。
    public: typedef template_allocator allocator_type;

    /// @brief 評価に用いる条件式のキーを表す型。
    public: typedef template_expression_key expression_key;

    /// @brief 条件式キーのコンテナ。
    private: typedef std::vector<
        typename this_type::expression_key,
        typename this_type::allocator_type>
            expression_key_vector;

    //-------------------------------------------------------------------------
    /** @brief 条件式の評価結果が変化した際に呼び出す、関数オブジェクトの型。

        - 引数#0は、評価に用いた条件式のキー。
        - 引数#1は、変化した後の条件式の評価結果。
     */
    public: typedef std::function<
        void (typename this_type::expression_key const&, bool const)>
            function;

    /// @brief this_type::function の、所有権ありスマートポインタ。
    public: typedef std::shared_ptr<typename this_type::function>
        function_shared_ptr;

    /// @brief this_type::function の、所有権なしスマートポインタ。
    public: typedef std::weak_ptr<typename this_type::function>
        function_weak_ptr;

    //-------------------------------------------------------------------------
    /** @brief 条件評価受信オブジェクト。

        条件式の評価結果と、
        条件式の評価結果が変化した際に呼び出す関数オブジェクトを保持する。
     */
    private: struct listener
    {
        typedef listener this_type;

        /** @brief 条件評価受信オブジェクトの辞書。

            - map::key_type は、条件式のキー。
            - map::mapped_type は、条件評価受信オブジェクト。
         */
        typedef std::map<
            typename dispatcher::expression_key,
            this_type,
            std::less<typename dispatcher::expression_key>,
            typename dispatcher::allocator_type>
                map;

        /// @brief 条件挙動関数オブジェクトの所有権なしスマートポインタ。
        typedef std::vector<
            typename dispatcher::function_weak_ptr,
            typename dispatcher::allocator_type>
                function_weak_ptr_vector;

        /// @brief フラグの位置。
        enum flag_enum: std::uint8_t
        {
            flag_LAST_EVALUATION,  ///< 条件式の前回の評価結果。
            flag_EVALUATE_REQUEST, ///< 条件式の評価の更新要求。
        };

        listener(
            std::size_t const in_reserve_functions,
            typename dispatcher::allocator_type const& in_allocator)
        :
        functions(in_allocator)
        {
            this->functions.reserve(in_reserve_functions);
        }

        /// @brief 条件挙動関数オブジェクトのコンテナ。
        typename this_type::function_weak_ptr_vector functions;
        /// @brief フラグの集合。
        std::bitset<8> flags;

    }; // struct listener

    //-------------------------------------------------------------------------
    /** @brief 条件監視オブジェクト。

        条件式の要素条件が用いる状態を監視し、
        状態が更新された際に、条件式の評価を更新するために使う。
     */
    private: struct monitor
    {
        typedef monitor this_type;

        /// @brief 監視する要素条件のキーを表す型。
        typedef template_monitor_key key_type;

        /** @brief 条件監視オブジェクトの辞書。

            - map::key_type は、監視する要素条件のキー。
            - map::mapped_type は、条件監視オブジェクト。
         */
        typedef std::map<
            typename this_type::key_type,
            this_type,
            std::less<typename this_type::key_type>,
            typename dispatcher::allocator_type>
                map;

        monitor(
            std::size_t const in_reserve_keys,
            typename dispatcher::allocator_type const& in_allocator)
        :
        expression_keys(in_allocator),
        notify(false)
        {
            this->expression_keys.reserve(in_reserve_keys);
        }

        /// @brief 評価の更新を要求する条件式のキーのコンテナ。
        typename dispatcher::expression_key_vector expression_keys;
        /// @brief 更新通知フラグ。
        bool notify;

    }; // struct monitor

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 空の条件監視器を構築する。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: explicit dispatcher(
        typename this_type::allocator_type const& in_allocator)
    :
    listeners_(in_allocator),
    state_transition_monitors_(in_allocator),
    state_comparison_monitors_(in_allocator)
    {}

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: dispatcher(this_type&& io_source):
    listeners_(std::move(io_source.listeners_)),
    state_transition_monitors_(std::move(io_source.state_transition_monitors_)),
    state_comparison_monitors_(std::move(io_source.state_comparison_monitors_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->listeners_ = std::move(io_source.listeners_);
        this->state_transition_monitors_ =
            std::move(io_source.state_transition_monitors_);
        this->state_comparison_monitors_ =
            std::move(io_source.state_comparison_monitors_);
        return *this;
    }

    /// @brief 条件監視器を再構築し、メモリ領域を必要最小限にする。
    public: void shrink_to_fit()
    {
        //this->listeners_.shrink_to_fit();
        //this->state_transition_monitors_.shrink_to_fit();
        //this->state_comparison_monitors_.shrink_to_fit();
        for (auto& local_listener: this->listeners_)
        {
            local_listener.functions.shrink_to_fit();
        }
        for (auto& local_monitor: this->state_transition_monitors_)
        {
            local_monitor.expression_keys.shrink_to_fit();
        }
        for (auto& local_monitor: this->state_comparison_monitors_)
        {
            local_monitor.expression_keys.shrink_to_fit();
        }
    }

    /** @brief 条件評価器で使われているメモリ割当子を取得する。
        @return 条件評価器で使われているメモリ割当子。
     */
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->listeners_.get_allocator();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 条件挙動
    //@{
    /** @brief 条件式の評価の変化を検知して呼び出す、関数オブジェクトを登録する。

        this_type::dispatch で条件式の評価が変化した際に、
        呼び出す関数オブジェクトを登録する。

        @param[in] in_expression_key    評価に用いる条件式のキー。
        @param[in] in_function          登録する関数オブジェクト。
        @param[in] in_evaluator         評価の初期値の決定に用いる条件評価器。
        @param[in] in_states            評価の初期値の決定に用いる状態値書庫。
        @param[in] in_reserve_functions 予約する関数オブジェクトコンテナの容量。
        @retval true  成功。関数オブジェクトを登録した。
        @retval false 失敗。関数オブジェクトは登録されなかった。
     */
    public: template<typename template_evaluator>
    bool register_function(
        typename this_type::expression_key const& in_expression_key,
        typename this_type::function_shared_ptr const& in_function,
        template_evaluator const& in_evaluator,
        typename template_evaluator::state_archive const& in_states,
        std::size_t const in_reserve_functions = 1)
    {
        auto const local_function(in_function.get());
        if (local_function == nullptr)
        {
            return false;
        }

        // 関数オブジェクトの登録先となる、条件評価受信オブジェクトを検索する。
        auto local_listener(this->listeners_.find(in_expression_key));
        if (local_listener != this->listeners_.end())
        {
            //local_listener->second.functions.reserve(in_reserve_functions);
            // 同じ関数オブジェクトがすでに登録されているなら、
            // 何もせずに成功する。
            bool const local_find(
                this_type::arrange_function_container(
                    local_listener->second.functions, *local_function));
            if (local_find)
            {
                return true;
            }
        }
        else
        {
            // 適切な条件評価受信オブジェクトが見つからなかったので、
            // 新たな条件評価受信オブジェクトを追加する。
            local_listener = this->add_listener(
                in_expression_key,
                in_evaluator,
                in_states,
                in_reserve_functions);
            if (local_listener == this->listeners_.end())
            {
                return false;
            }
        }

        // 関数オブジェクトを条件評価受信オブジェクトへ登録する。
        local_listener->second.functions.push_back(in_function);
        return true;
    }
    //@}
    /** @brief 関数オブジェクトを検索しつつ、コンテナを整理する。
        @param[in,out] io_functions 整理する関数オブジェクトコンテナ。
        @param[in] in_function      検索する関数オブジェクト。
        @retval true  検索対象がコンテナから見つかった。
        @retval false 検索対象がコンテナから見つからなかった。
     */
    private: static bool arrange_function_container(
        typename this_type::listener::function_weak_ptr_vector& io_functions,
        typename this_type::function const& in_function)
    {
        // 関数オブジェクトのコンテナを走査し、検索対象を見つけながら、
        // 空になった要素を削除する。
        auto local_last(io_functions.begin());
        auto local_find(false);
        for (auto& local_function: io_functions)
        {
            if (!local_find && local_function.lock().get() == &in_function)
            {
                local_find = true;
            }
            local_last->swap(local_function);
            if (!local_last->expired())
            {
                ++local_last;
            }
        }
        io_functions.erase(local_last, io_functions.end());
        return local_find;
    }

    /** @brief 新たな条件評価受信オブジェクトを生成して追加する。
        @param[in] in_expression_key    評価に用いる条件式の識別番号。
        @param[in] in_evaluator         評価の初期値の決定に用いる条件評価器。
        @param[in] in_states            評価の初期値の決定に用いる状態値のコンテナ。
        @param[in] in_reserve_functions 予約しておく関数オブジェクトコンテナの容量。
        @return 追加した条件評価受信オブジェクトを指す反復子。
     */
    private: template<typename template_evaluator>
    typename this_type::listener::map::iterator add_listener(
        typename this_type::expression_key const& in_expression_key,
        template_evaluator const& in_evaluator,
        typename template_evaluator::state_archive const& in_states,
        std::size_t const in_reserve_functions)
    {
        // 条件式を条件監視オブジェクトへ登録する。
        if (this->add_expression(in_expression_key, in_evaluator))
        {
            // 条件評価受信オブジェクトを生成し、辞書に挿入する。
            auto const local_insert(
                this->listeners_.insert(
                    typename this_type::listener::map::value_type(
                        in_expression_key,
                        typename this_type::listener::map::mapped_type(
                            in_reserve_functions,
                            this->listeners_.get_allocator()))));
            if (local_insert.second)
            {
                // 条件評価の初期値を設定しておく。
                local_insert.first->second.flags.set(
                    this_type::listener::flag_LAST_EVALUATION,
                    in_evaluator.evaluate_expression(
                        in_expression_key, in_states));
                return local_insert.first;
            }
            // 挿入に失敗した。
            PSYQ_ASSERT(false);
        }
        return this->listeners_.end();
    }

    /** @brief 条件式を、条件監視オブジェクトへ登録する。
        @param[in] in_expression_key 登録する条件式の識別番号。
        @param[in] in_evaluator      登録する条件式を持つ条件評価器。
        @retval true  成功。
        @retval false 失敗。
     */
    private: template<typename template_evaluator>
    bool add_expression(
        typename this_type::expression_key const& in_expression_key,
        template_evaluator const& in_evaluator)
    {
        // 条件式と要素条件チャンクを検索する。
        auto const local_expression(
            in_evaluator.find_expression(in_expression_key));
        if (local_expression == nullptr)
        {
            return false;
        }
        auto const local_chunk(
            in_evaluator.find_chunk(local_expression->chunk));
        if (local_chunk== nullptr)
        {
            // 条件式の要素条件チャンクが存在しなかった。
            PSYQ_ASSERT(false);
            return false;
        }

        // 条件式の種類によって、監視する条件式の追加先を選別する。
        switch (local_expression->kind)
        {
            case template_evaluator::expression_struct::kind_COMPOUND:
            this->add_compound_expression(
                in_evaluator, *local_expression, local_chunk->compounds);
            break;

            case template_evaluator::expression_struct::kind_STATE_COMPARISON:
            this_type::add_monitor_expression(
                this->state_comparison_monitors_,
                *local_expression,
                local_chunk->state_comparisons);
            break;

            default:
            // 未対応の条件式の種類だった。
            PSYQ_ASSERT(false);
            break;
        }
        return true;
    }

    /** @brief 複合条件式を、条件監視オブジェクトへ登録する。
        @param[in] in_evaluator  登録する条件式を持つ条件評価器。
        @param[in] in_expression 登録する複合条件式。
        @param[in] in_compounds  登録する複合条件式が参照する要素条件コンテナ。
     */
    private: template<typename template_evaluator>
    void add_compound_expression(
        template_evaluator const& in_evaluator,
        typename template_evaluator::expression_struct const& in_expression,
        typename template_evaluator::compound_struct::vector const& in_compounds)
    {
        // 条件監視オブジェクトに、複合条件式を追加する。
        auto const local_begin(in_compounds.begin());
        auto const local_end(local_begin + in_expression.end);
        for (auto i(local_begin + in_expression.begin); i != local_end; ++i)
        {
            auto& local_element(*i);
            auto const local_listener(
                this->listeners_.find(local_element.key));
            if (local_listener == this->listeners_.end()
                && !this->add_expression(local_element.key, in_evaluator))
            {
                // 複合条件式の追加に失敗した。
                PSYQ_ASSERT(false);
            }
        }
    }

    /** @brief 条件式を、条件監視オブジェクトへ登録する。
        @param[in,out] io_monitors 条件式を登録する条件監視オブジェクトのコンテナ。
        @param[in] in_expression   登録する条件式。
        @param[in] in_elements     条件式が参照する要素条件のコンテナ。
        @retval true  成功。
        @retval false 失敗。
     */
    private: template<
        typename template_expression,
        typename template_element_container>
    static void add_monitor_expression(
        typename this_type::monitor::map& io_monitors,
        template_expression const& in_expression,
        template_element_container const& in_elements)
    {
        // 条件式が持つ要素条件を走査し、
        // 要素条件ごとに条件監視オブジェクトへ条件式を登録する。
        auto const local_begin(in_elements.begin());
        auto const local_end(local_begin + in_expression.end);
        for (auto i(local_begin + in_expression.begin); i != local_end; ++i)
        {
            // 条件式を登録する条件監視オブジェクトを取得する。
            auto& local_element_key(i->key);
            auto local_monitor(io_monitors.find(local_element_key));
            if (local_monitor == io_monitors.end())
            {
                // 要素条件に対応する条件監視オブジェクトがなかったので、
                // 条件監視オブジェクトを新たに生成する。
                std::size_t const local_reserve_expression_keys(4);
                auto const local_insert(
                    io_monitors.insert(
                        typename this_type::monitor::map::value_type(
                            local_element_key,
                            this_type::monitor::map::mapped_type(
                                local_reserve_expression_keys,
                                io_monitors.get_allocator()))));
                if (!local_insert.second)
                {
                    // 挿入に失敗した。
                    PSYQ_ASSERT(false);
                    continue;
                }
                local_monitor = local_insert.first;
            }

            // 条件式キーを、条件監視オブジェクトへ登録する。
            auto& local_expression_keys(local_monitor->second.expression_keys);
            auto const local_lower_bound(
                std::lower_bound(
                    local_expression_keys.begin(),
                    local_expression_keys.end(),
                    in_expression.key));
            if (local_lower_bound == local_expression_keys.end()
                || *local_lower_bound != in_expression.key)
            {
                local_expression_keys.insert(
                    local_lower_bound, in_expression.key);
            }
        }
    }

    //-------------------------------------------------------------------------
    /// @name 条件挙動
    //@{
    /** @brief 状態値の変更通知を受け取る。
        @param[in] in_state_key 変更された状態値のキー。
     */
    public: void notify_state_transition(
        typename this_type::monitor::map::key_type const& in_state_key)
    {
        this_type::notify_monitor(
            this->state_transition_monitors_, in_state_key);
        this_type::notify_monitor(
            this->state_comparison_monitors_, in_state_key);
    }
    //@}
    private: static void notify_monitor(
        typename this_type::monitor::map& io_monitors,
        typename this_type::monitor::map::key_type const& in_monitor_key)
    {
        auto const local_find(io_monitors.find(in_monitor_key));
        if (local_find != io_monitors.end())
        {
            local_find->second.notify = true;
        }
    }

    /** @brief 条件監視オブジェクトへの更新通知を、条件評価受信オブジェクトへ転送する。
        @param[in,out] io_listeners 条件評価受信オブジェクトのコンテナ。
        @param[in,out] io_monitors  条件監視オブジェクトのコンテナ。
     */
    private: static void notify_listener_container(
        typename this_type::listener::map& io_listeners,
        typename this_type::monitor::map& io_monitors)
    {
        // 条件監視オブジェクトのコンテナを走査しつつ、
        // 不要になった要素を削除し、条件監視オブジェクトのコンテナを整理する。
        for (auto i(io_monitors.begin()); i != io_monitors.end();)
        {
            auto& local_monitor(i->second);
            if (local_monitor.notify)
            {
                // 条件評価受信オブジェクトへ更新通知を転送する。
                local_monitor.notify = false;
                this_type::notify_listener(
                    io_listeners, local_monitor.expression_keys);

                // 条件監視オブジェクトに対応する条件評価受信オブジェクトが
                // なくなったら、条件監視オブジェクトを削除する。
                if (local_monitor.expression_keys.empty())
                {
                    i = io_monitors.erase(i);
                    continue;
                }
            }
            ++i;
        }
    }

    /** @brief 条件評価受信オブジェクトへ、状態の更新を通知する。。
        @param[in,out] io_listeners       条件評価受信オブジェクトのコンテナ。
        @param[in,out] io_expression_keys 更新を通知する、条件式の識別番号のコンテナ。
     */
    private: static void notify_listener(
        typename this_type::listener::map& io_listeners,
        typename this_type::expression_key_vector& io_expression_keys)
    {
        // 条件式識別番号のコンテナを走査しつつ、
        // 不要になった要素を削除し、条件識別番号コンテナを整理する。
        auto local_last(io_expression_keys.begin());
        for (auto& local_expression_key: io_expression_keys)
        {
            auto const local_listener(io_listeners.find(local_expression_key));
            *local_last = std::move(local_expression_key);
            if (local_listener != io_listeners.end())
            {
                local_listener->second.flags.set(
                    this_type::listener::flag_EVALUATE_REQUEST);
                ++local_last;
            }
        }
        io_expression_keys.erase(local_last, io_expression_keys.end());
    }

    //-------------------------------------------------------------------------
    /// @name 条件挙動
    //@{
    /** @brief 条件式の評価の変化を検知する。

        条件式の評価が変化していれば、
        this_type::register_function で登録した関数を呼び出す。

        @param[in] in_evaluator 条件式の評価に使う条件評価器。
        @param[in] in_states    条件式の評価に状態値のコンテナ。

        @note
        前回の this_type::dispatch と今回の self::dispatch
        で条件式の評価が異なる場合に、登録関数の呼び出し判定が行われる。
        前回から今回の間（基本的には1フレームの間）で条件式の評価が
        true → false → true と変化したとしても、
        登録関数の呼び出し判定は行われないことに注意。
        this_type::dispatch の間でも
        条件式の評価の変化を検知するような実装も可能と思われるが、
        this_type::dispatch を呼び出すたびに
        登録関数を呼び出すような事態になりかねないため、
        this_type::dispatch の間の条件式の評価の変化は
        今のところ検知してない。
     */
    public: template<typename template_evaluator>
    void dispatch(
        template_evaluator const& in_evaluator,
        typename template_evaluator::state_archive const& in_states)
    {
        // 条件監視オブジェクトへの更新通知を、
        // 条件評価受信オブジェクトに転送する。
        this_type::notify_listener_container(
            this->listeners_, this->state_transition_monitors_);
        this_type::notify_listener_container(
            this->listeners_, this->state_comparison_monitors_);

        // 要素条件で用いる状態が更新された条件式を再評価し、
        // 評価結果が変化していれば、ディスパッチ関数を呼び出す。
        this_type::update_listener_container(
            this->listeners_, in_evaluator, in_states);
    }
    //@}
    /** @brief 条件式を評価し、ディスパッチ関数を呼び出す。

        条件評価受信オブジェクトのコンテナを走査し、条件式の結果によって、
        this_type::register_function で登録された関数オブジェクトを呼び出す。

        @param[in,out] io_listeners 条件評価受信オブジェクトのコンテナ。
        @param[in] in_evaluator     評価に使う条件評価器。
        @param[in] in_states        条件式の評価に状態値のコンテナ。
     */
    private: template<typename template_evaluator>
    static void update_listener_container(
        typename this_type::listener::map& io_listeners,
        template_evaluator const& in_evaluator,
        typename template_evaluator::state_archive const& in_states)
    {
        for (auto i(io_listeners.begin()); i != io_listeners.end();)
        {
            // 条件評価受信オブジェクトの評価要求フラグを取得し、初期化する。
            auto& local_listener(*i);
            bool const local_evaluate_request(
                local_listener.second.flags.test(
                    this_type::listener::flag_EVALUATE_REQUEST));
            local_listener.second.flags.reset(
                this_type::listener::flag_EVALUATE_REQUEST);

            // 評価要求フラグが立っていれば、条件式を評価する。
            if (local_evaluate_request)
            {
                this_type::update_listener(
                    local_listener, in_evaluator, in_states);

                // 関数オブジェクトのコンテナが空になったら、
                // 条件監視オブジェクトを削除する。
                if (local_listener.second.functions.empty())
                {
                    i = io_listeners.erase(i);
                    continue;
                }
            }
            ++i;
        }
    }

    /** @brief 条件式を評価し、ディスパッチ関数を呼び出す。

        条件式を評価し、前回の結果と異なるなら、
        this_type::register_function で登録されたディスパッチ関数を呼び出す。

        @param[in,out] io_listener 更新する条件評価受信オブジェクト。
        @param[in] in_evaluator    評価に用いる条件評価器。
        @param[in] in_states       条件式の評価に用いる状態値のコンテナ。
     */
    private: template<typename template_evaluator>
    static void update_listener(
        typename this_type::listener::map::value_type& io_listener,
        template_evaluator const& in_evaluator,
        typename template_evaluator::state_archive const& in_states)
    {
        // 条件式を評価し、結果が以前と異なっているか判定する。
        bool const local_old_evaluation(
            io_listener.second.flags.test(
                this_type::listener::flag_LAST_EVALUATION));
        bool const local_new_evaluation(
            in_evaluator.Evaluate(io_listener.first, in_states));
        if (local_old_evaluation == local_new_evaluation)
        {
            return;
        }

        // 条件式の結果が以前と異なるので、ディスパッチ関数を呼び出す。
        io_listener.second.flags.set(
            this_type::listener::flag_LAST_EVALUATION,
            local_new_evaluation);
        auto local_last(io_listener.second.functions.begin());
        for (auto& local_function_observer: io_listener.second.functions)
        {
            local_last->swap(local_function_observer);
            if (!local_last->expired())
            {
                auto const local_function(local_last->lock());
                (*local_function)(io_listener.first, local_new_evaluation);
                ++local_last;
            }
        }
        io_listener.second.functions.erase(
            local_last, io_listener.second.functions.end());
    }

    //-------------------------------------------------------------------------
    /// 条件評価受信オブジェクトの辞書。
    private: typename this_type::listener::map listeners_;

    /// 状態変化監視オブジェクトの辞書。
    private: typename this_type::monitor::map state_transition_monitors_;

    /// 状態比較監視オブジェクトの辞書。
    private: typename this_type::monitor::map state_comparison_monitors_;

}; // class psyq::scenario_engine::dispatcher

#endif // !defined(PSYQ_SCENARIO_ENGINE_DISPATCHER_HPP_)
// vim: set expandtab:
