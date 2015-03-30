/** @file
    @copydoc psyq::scenario_engine::dispatcher
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
/** @brief 条件挙動器。条件式を監視し、評価結果が変化すると、登録されている関数を呼び出す。

    使い方の概略。
    - dispatcher::register_expression を呼び出し、
      条件式の評価結果が変化したときに呼び出す関数を登録する。
    - driver::update をフレーム毎に呼び出し、登録された関数を呼び出す。

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

    /// @brief コンテナに用いるメモリ割当子を表す型。
    public: typedef template_allocator allocator_type;

    /// @brief 条件式の識別値のコンテナ。
    private: typedef std::vector<
        typename this_type::expression_key,
        typename this_type::allocator_type>
            expression_key_vector;

    //-------------------------------------------------------------------------
    /** @brief 条件式の評価結果が変化した際に呼び出す、条件挙動関数オブジェクトの型。

        - 引数#0は、評価に用いた条件式の識別値。
        - 引数#1は、条件式の今回の評価結果。
        - 引数#2は、条件式の前回の評価結果。
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

    /// @brief 条件挙動関数オブジェクトの所有権なしスマートポインタのコンテナ。
    private: typedef std::vector<
        typename this_type::function_weak_ptr,
        typename this_type::allocator_type>
            function_weak_ptr_vector;

    //-------------------------------------------------------------------------
    /** @brief 条件式監視器。

        条件式の評価結果と、
        条件式の評価結果が変化した際に呼び出す関数オブジェクトを保持する。
     */
    private: struct expression_monitor
    {
        typedef expression_monitor this_type;

        /// @brief 条件式監視器のコンテナ。
        typedef std::vector<this_type, typename dispatcher::allocator_type>
            vector;

        /// @brief 条件式監視器を条件式の識別値の昇順で並び替えるのに使う、比較関数オブジェクト。
        typedef psyq::scenario_engine::_private::key_less<
            this_type, typename dispatcher::expression_key>
                key_less;

        /// @brief フラグの位置。
        enum flag_enum: std::uint8_t
        {
            flag_LAST_EVALUATION,    ///< 条件の前回の評価の成功／失敗。
            flag_LAST_CONDITION,     ///< 条件の前回の評価。
            flag_EVALUATION_REQUEST, ///< 条件評価の更新要求。
            flag_CONSTRUCTED,        ///< 構築済みフラグ。
        };

        /** @brief 条件式監視器を構築する。
            @param[in] in_key       条件式の識別値。
            @param[in] in_allocator メモリ割当子の初期値。
         */
        expression_monitor(
            typename dispatcher::expression_key in_key,
            typename dispatcher::allocator_type const& in_allocator)
        :
        key(std::move(in_key)),
        functions(in_allocator)
        {}

        /** @brief ムーブ構築子。
            @param[in,out] io_source ムーブ元となるインスタンス。
         */
        expression_monitor(this_type&& io_source):
        functions(std::move(io_source.functions)),
        key(std::move(io_source.key)),
        flags(std::move(io_source.flags))
        {}

        /** @brief ムーブ代入演算子。
            @param[in,out] io_source ムーブ元となるインスタンス。
            @return *this
         */
        this_type& operator=(this_type&& io_source)
        {
            if (this != &io_source)
            {
                this->functions = std::move(io_source.functions);
                this->key = std::move(io_source.key);
                this->flags = std::move(io_source.flags);
            }
            return *this;
        }

        std::int8_t get_last_evaluation() const PSYQ_NOEXCEPT
        {
            return this->flags.test(this_type::flag_LAST_EVALUATION)?
                (this->flags.test(this_type::flag_LAST_CONDITION)? 1: 0):
                -1;
        }

        /// @brief 条件挙動関数オブジェクトのコンテナ。
        typename dispatcher::function_weak_ptr_vector functions;
        /// @brief 条件式の識別値。
        typename dispatcher::expression_key key;
        /// @brief フラグの集合。
        std::bitset<8> flags;

    }; // struct expression_monitor

    //-------------------------------------------------------------------------
    /** @brief 状態監視器。

        条件式の要素条件が用いる状態を監視し、
        状態が更新された際に、条件式の評価を更新するために使う。
     */
    private: struct state_monitor
    {
        typedef state_monitor this_type;

        /// @brief 状態監視器のコンテナ。
        typedef std::vector<this_type, typename dispatcher::allocator_type>
            vector;

        /// @brief 状態監視器を状態値の識別値の昇順で並び替えるのに使う、比較関数オブジェクト。
        typedef psyq::scenario_engine::_private::key_less<
            this_type, typename dispatcher::state_key>
                key_less;

        /** @brief 状態監視器を構築する。
            @param[in] in_key       状態値の識別値。
            @param[in] in_allocator メモリ割当子の初期値。
         */
        state_monitor(
            typename dispatcher::state_key in_key,
            typename dispatcher::allocator_type const& in_allocator)
        :
        expression_keys(in_allocator),
        key(std::move(in_key))
        {}

        /** @brief ムーブ構築子。
            @param[in,out] io_source ムーブ元となるインスタンス。
         */
        state_monitor(this_type&& io_source):
        expression_keys(std::move(io_source.expression_keys)),
        key(std::move(io_source.key))
        {}

        /** @brief ムーブ代入演算子。
            @param[in,out] io_source ムーブ元となるインスタンス。
            @return *this
         */
        this_type& operator=(this_type&& io_source)
        {
            if (this != &io_source)
            {
                this->expression_keys = std::move(io_source.expression_keys);
                this->key = std::move(io_source.key);
            }
            return *this;
        }

        /// @brief 評価の更新を要求する条件式の識別値のコンテナ。
        typename dispatcher::expression_key_vector expression_keys;
        /// @brief 状態値の識別値。
        typename dispatcher::state_key key;

    }; // struct state_monitor

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 空の条件挙動器を構築する。
        @param[in] in_reserve_expressions 監視する条件式の予約数。
        @param[in] in_reserve_states      監視する状態値の予約数。
        @param[in] in_allocator           メモリ割当子の初期値。
     */
    public: dispatcher(
        std::size_t const in_reserve_expressions,
        std::size_t const in_reserve_states,
        typename this_type::allocator_type const& in_allocator)
    :
    expression_monitors_(in_allocator),
    state_monitors_(in_allocator)
    {
        this->expression_monitors_.reserve(in_reserve_expressions);
        this->state_monitors_.reserve(in_reserve_states);
    }

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: dispatcher(this_type&& io_source):
    expression_monitors_(std::move(io_source.expression_monitors_)),
    state_monitors_(std::move(io_source.state_monitors_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->expression_monitors_ = std::move(io_source.expression_monitors_);
        this->state_monitors_ = std::move(io_source.state_monitors_);
        return *this;
    }

    /// @brief 条件挙動器を再構築し、メモリ領域を必要最小限にする。
    public: void shrink_to_fit()
    {
        this->expression_monitors_.shrink_to_fit();
        this->state_monitors_.shrink_to_fit();
        for (auto& local_monitor: this->expression_monitors_)
        {
            local_monitor.functions.shrink_to_fit();
        }
        for (auto& local_monitor: this->state_monitors_)
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
        return this->expression_monitors_.get_allocator();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 条件挙動
    //@{
    /** @brief 条件式の評価の変化を検知して呼び出す、関数オブジェクトを登録する。

        this_type::_dispatch で条件式の評価が変化した際に、
        呼び出す関数オブジェクトを登録する。

        @param[in] in_expression_key    評価に用いる条件式の識別値。
        @param[in] in_function          登録する関数オブジェクト。
        @param[in] in_reserve_functions 関数オブジェクトの予約数。
        @retval true  成功。関数オブジェクトを登録した。
        @retval false 失敗。関数オブジェクトは登録されなかった。
     */
    public: bool register_expression(
        typename this_type::expression_key const& in_expression_key,
        typename this_type::function_shared_ptr const& in_function,
        std::size_t const in_reserve_functions = 1)
    {
        auto const local_function(in_function.get());
        if (local_function == nullptr)
        {
            return false;
        }

        // 関数オブジェクトの登録先となる、条件式監視器を検索する。
        auto local_expression_monitor(
            std::lower_bound(
                this->expression_monitors_.begin(),
                this->expression_monitors_.end(),
                in_expression_key,
                typename this_type::expression_monitor::key_less()));
        if (local_expression_monitor != this->expression_monitors_.end()
            && local_expression_monitor->key == in_expression_key)
        {
            //local_expression_monitor->functions.reserve(in_reserve_functions);
            // 同じ関数オブジェクトがすでに登録されているなら、
            // 何もせずに成功する。
            bool const local_find(
                this_type::arrange_function_container(
                    local_expression_monitor->functions, *local_function));
            if (local_find)
            {
                return true;
            }
        }
        // 条件式を状態監視器へ登録する。
        else
        {
            // 条件式監視器を生成し、コンテナに挿入する。
            local_expression_monitor = this->expression_monitors_.insert(
                local_expression_monitor,
                this_type::expression_monitor(
                    in_expression_key, this->get_allocator()));
            local_expression_monitor->functions.reserve(in_reserve_functions);
        }

        // 関数オブジェクトを条件式監視器へ登録する。
        local_expression_monitor->functions.push_back(in_function);
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
        typename this_type::function_weak_ptr_vector& io_functions,
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

    /** @brief 状態の更新を通知する条件式を、状態監視器へ登録する。
        @param[in] in_expression_key      登録する条件式の識別値。
        @param[in] in_evaluator           登録する条件式を持つ条件評価器。
        @param[in] in_reserve_expressions 条件式の予約数。
        @retval true  成功。
        @retval false 失敗。
     */
    private: template<typename template_evaluator>
    bool add_expression(
        typename this_type::expression_key const& in_expression_key,
        template_evaluator const& in_evaluator,
        std::size_t const in_reserve_expressions = 1)
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
        if (local_chunk == nullptr)
        {
            // 条件式の要素条件チャンクが存在しなかった。
            PSYQ_ASSERT(false);
            return false;
        }

        // 条件式の種類によって、監視する条件式の追加先を選別する。
        switch (local_expression->kind)
        {
            case template_evaluator::expression::kind_SUB_EXPRESSION:
            return this->add_sub_expression(
                in_evaluator,
                *local_expression,
                local_chunk->sub_expressions,
                in_reserve_expressions);

            case template_evaluator::expression::kind_STATE_COMPARISON:
            this_type::add_notify_expression(
                this->state_monitors_,
                *local_expression,
                local_chunk->state_comparisons,
                in_reserve_expressions);
            return true;

            default:
            // 未対応の条件式の種類だった。
            PSYQ_ASSERT(false);
            return false;
        }
    }

    /** @brief 複合条件式を、状態監視器へ登録する。
        @param[in] in_evaluator  登録する条件式を持つ条件評価器。
        @param[in] in_expression 登録する複合条件式。
        @param[in] in_sub_expressions
            登録する複合条件式が参照する要素条件コンテナ。
        @param[in] in_reserve_expressions 条件式の予約数。
     */
    private: template<typename template_evaluator>
    bool add_sub_expression(
        template_evaluator const& in_evaluator,
        typename template_evaluator::expression const& in_expression,
        typename template_evaluator::sub_expression::vector const& in_sub_expressions,
        std::size_t const in_reserve_expressions)
    {
        // 状態監視器に、複合条件式を追加する。
        auto const local_begin(in_sub_expressions.begin());
        auto const local_end(local_begin + in_expression.end);
        for (auto i(local_begin + in_expression.begin); i != local_end; ++i)
        {
            auto const& local_sub_key(i->key);
            auto const local_expression_monitor(
                this_type::expression_monitor::key_less::find_const_pointer(
                    this->expression_monitors_, local_sub_key));
            if (local_expression_monitor == nullptr
                && !this->add_expression(local_sub_key, in_evaluator, in_reserve_expressions))
            {
                // 無限ループを防ぐため、
                // まだ存在しない条件式を複合条件式で使うのは禁止する。
                PSYQ_ASSERT(false);
                return false;
            }
        }
        return true;
    }

    /** @brief 状態の更新を知らせる条件式を、状態監視器へ登録する。
        @param[in,out] io_state_monitors  条件式を登録する状態監視器のコンテナ。
        @param[in] in_expression          登録する条件式。
        @param[in] in_elements            条件式が参照する要素条件のコンテナ。
        @param[in] in_reserve_expressions 条件式の予約数。
        @retval true  成功。
        @retval false 失敗。
     */
    private: template<
        typename template_expression,
        typename template_element_container>
    static void add_notify_expression(
        typename this_type::state_monitor::vector& io_state_monitors,
        template_expression const& in_expression,
        template_element_container const& in_elements,
        std::size_t const in_reserve_expressions)
    {
        // 条件式が持つ要素条件を走査し、
        // 要素条件ごとに状態監視器へ条件式を登録する。
        auto const local_begin(in_elements.begin());
        auto const local_end(local_begin + in_expression.end);
        for (auto i(local_begin + in_expression.begin); i != local_end; ++i)
        {
            // 条件式を登録する状態監視器を取得する。
            auto& local_element_key(i->key);
            auto local_state_monitor(
                std::lower_bound(
                    io_state_monitors.begin(),
                    io_state_monitors.end(),
                    local_element_key,
                    typename this_type::state_monitor::key_less()));
            if (local_state_monitor == io_state_monitors.end()
                || local_state_monitor->key != local_element_key)
            {
                // 要素条件に対応する状態監視器がなかったので、
                // 状態監視器を新たに生成する。
                local_state_monitor = io_state_monitors.insert(
                    local_state_monitor,
                    typename this_type::state_monitor(
                        local_element_key, io_state_monitors.get_allocator()));
            }

            // 条件式の識別値を、状態監視器へ登録する。
            auto& local_expression_keys(local_state_monitor->expression_keys);
            local_expression_keys.reserve(in_reserve_expressions);
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
    /** @brief psyq::scenario_engine 管理者以外は、この関数は使用禁止。

        状態の変化を検知する。

        @param[in] in_evaluator 条件式の評価に使う条件評価器。
        @param[in] in_reservoir 条件式の評価に使う状態貯蔵器。
     */
    public: template<typename template_evaluator>
    void _detect(
        template_evaluator const& in_evaluator,
        typename template_evaluator::reservoir const& in_reservoir)
    {
        // 条件式監視器を組み立てる。
        for (auto& local_expression_monitor: this->expression_monitors_)
        {
            auto const local_constructed(
                local_expression_monitor.flags.test(
                    this_type::expression_monitor::flag_CONSTRUCTED));
            if (!local_constructed)
            {
                local_expression_monitor.flags.set(
                    this_type::expression_monitor::flag_CONSTRUCTED,
                    this->add_expression(
                        local_expression_monitor.key, in_evaluator));
            }
        }

        // 状態値の更新通知を条件式監視器に転送する。
        this_type::detect_state_transition(
            this->expression_monitors_, this->state_monitors_, in_reservoir);
    }

    /** @brief psyq::scenario_engine 管理者以外は、この関数は使用禁止。

        条件式の評価が変化していれば、
        this_type::register_expression で登録した関数を呼び出す。

        @param[in] in_evaluator 条件式の評価に使う条件評価器。
        @param[in] in_reservoir 条件式の評価に使う状態貯蔵器。

        @note
        前回の this_type::_dispatch と今回の this_type::_dispatch
        で条件式の評価が異なる場合に、登録関数の呼び出し判定が行われる。
        前回から今回の間（基本的には1フレームの間）で条件式の評価が
        true → false → true と変化したとしても、
        登録関数の呼び出し判定は行われないことに注意。
        this_type::_dispatch の間でも
        条件式の評価の変化を検知するような実装も可能と思われるが、
        this_type::_dispatch を呼び出すたびに
        登録関数を呼び出すような事態になりかねないため、
        this_type::_dispatch の間の条件式の評価の変化は
        今のところ検知してない。

        @todo
        条件挙動関数の中で this / in_evaluator / in_reservoir
        が破棄される可能性もある。何らかの対策をすること。
     */
    public: template<typename template_evaluator>
    void _dispatch(
        template_evaluator const& in_evaluator,
        typename template_evaluator::reservoir const& in_reservoir)
    {
        // 要素条件で用いる状態が更新された条件式を再評価し、
        // 評価結果が変化していれば、条件挙動関数を呼び出す。
        this_type::update_expression_monitor_container(
            this->expression_monitors_, in_evaluator, in_reservoir);
    }
    //@}
    /** @brief 状態の更新を検知する。
        @param[in,out] io_expression_monitors 条件式監視器のコンテナ。
        @param[in,out] io_state_monitors      状態監視器のコンテナ。
        @param[in] in_reservoir               更新を検知する状態貯蔵器。
     */
    private: template<typename template_reservoir>
    static void detect_state_transition(
        typename this_type::expression_monitor::vector& io_expression_monitors,
        typename this_type::state_monitor::vector& io_state_monitors,
        template_reservoir const& in_reservoir)
    {
        // 状態監視器のコンテナを走査しつつ、
        // 不要になった要素を削除し、状態監視器のコンテナを整理する。
        for (auto i(io_state_monitors.begin()); i != io_state_monitors.end();)
        {
            // 状態値の更新を検知する。
            auto& local_state_monitor(*i);
            auto const local_state(
                in_reservoir.find_state(local_state_monitor.key));
            if (local_state == nullptr || local_state->_get_transition())
            {
                // 状態値の更新を条件式監視器へ知らせる。
                this_type::notify_state_transition(
                    io_expression_monitors,
                    local_state_monitor.expression_keys,
                    local_state != nullptr);

                // 状態監視器に対応する条件式監視器がなくなったら、
                // 状態監視器を削除する。
                if (local_state_monitor.expression_keys.empty())
                {
                    i = io_state_monitors.erase(i);
                    continue;
                }
            }
            ++i;
        }
    }

    /** @brief 状態の更新を条件式監視器へ知らせる。
        @param[in,out] io_expression_monitors 条件式監視器のコンテナ。
        @param[in,out] io_expression_keys     更新を知らせる条件式の識別値のコンテナ。
        @param[in] in_valid_state             状態値が有効かどうか。
     */
    private: static void notify_state_transition(
        typename this_type::expression_monitor::vector& io_expression_monitors,
        typename this_type::expression_key_vector& io_expression_keys,
        bool const in_valid_state)
    {
        // 条件式識別番号のコンテナを走査しつつ、
        // 不要になった要素を削除し、条件識別番号コンテナを整理する。
        auto local_last(io_expression_keys.begin());
        for (auto& local_expression_key: io_expression_keys)
        {
            auto const local_expression_monitor(
                this_type::expression_monitor::key_less::find_pointer(
                    io_expression_monitors, local_expression_key));
            *local_last = std::move(local_expression_key);
            if (local_expression_monitor != nullptr)
            {
                local_expression_monitor->flags.set(
                    this_type::expression_monitor::flag_EVALUATION_REQUEST,
                    in_valid_state || local_expression_monitor->flags.test(
                        this_type::expression_monitor::flag_LAST_EVALUATION));
                ++local_last;
            }
        }
        io_expression_keys.erase(local_last, io_expression_keys.end());
    }

    /** @brief 条件式を評価し、条件挙動関数を呼び出す。

        条件式監視器のコンテナを走査し、条件式の結果によって、
        this_type::register_expression で登録された条件挙動関数を呼び出す。

        @param[in,out] io_expression_monitors 条件式監視器のコンテナ。
        @param[in] in_evaluator               評価に使う条件評価器。
        @param[in] in_reservoir               条件式の評価に状態値のコンテナ。
     */
    private: template<typename template_evaluator>
    static void update_expression_monitor_container(
        typename this_type::expression_monitor::vector& io_expression_monitors,
        template_evaluator const& in_evaluator,
        typename template_evaluator::reservoir const& in_reservoir)
    {
        for (
            auto i(io_expression_monitors.begin());
            i != io_expression_monitors.end();)
        {
            // 条件式監視器の評価要求があれば、条件式を評価する。
            auto& local_expression_monitor(*i);
            auto const local_evaluation_request(
                local_expression_monitor.flags.test(
                    this_type::expression_monitor::flag_EVALUATION_REQUEST));
            if (local_evaluation_request)
            {
                local_expression_monitor.flags.reset(
                    this_type::expression_monitor::flag_EVALUATION_REQUEST);
                auto const local_evaluate_expression(
                    this_type::evaluate_expression(
                        local_expression_monitor, in_evaluator, in_reservoir));
                if (local_evaluate_expression
                    && local_expression_monitor.functions.empty())
                {
                    // 関数オブジェクトのコンテナが空になったら、
                    // 状態監視器を削除する。
                    i = io_expression_monitors.erase(i);
                    continue;
                }
            }
            ++i;
        }
    }

    /** @brief 条件式を評価し、条件挙動関数を呼び出す。

        条件式を評価し、前回の結果と異なるなら、
        this_type::register_expression で登録された条件挙動関数を呼び出す。

        @param[in,out] io_expression_monitor 更新する条件式監視器。
        @param[in] in_evaluator              評価に用いる条件評価器。
        @param[in] in_reservoir              評価に用いる状態貯蔵器。
        @retval true  条件挙動関数を呼び出した。
        @retval false 条件挙動関数を呼び出さなかった。
     */
    private: template<typename template_evaluator>
    static bool evaluate_expression(
        typename this_type::expression_monitor& io_expression_monitor,
        template_evaluator const& in_evaluator,
        typename template_evaluator::reservoir const& in_reservoir)
    {
        // 条件式を評価し、評価結果が以前と異なっているか判定する。
        auto const local_last_evaluation(
            io_expression_monitor.get_last_evaluation());
        auto const local_evaluate_expression(
            in_evaluator.evaluate_expression(
                io_expression_monitor.key, in_reservoir));
        io_expression_monitor.flags.set(
            this_type::expression_monitor::flag_LAST_EVALUATION,
            0 <= local_evaluate_expression);
        io_expression_monitor.flags.set(
            this_type::expression_monitor::flag_LAST_CONDITION,
            0 < local_evaluate_expression);
        auto const local_evaluation(
            io_expression_monitor.get_last_evaluation());
        if (local_evaluation == local_last_evaluation)
        {
            return false;
        }

        // 条件式の結果が以前と異なるので、条件挙動関数を呼び出す。
        auto local_last(io_expression_monitor.functions.begin());
        for (auto& local_function_observer: io_expression_monitor.functions)
        {
            local_last->swap(local_function_observer);
            if (!local_last->expired())
            {
                auto const local_function(local_last->lock());
                (*local_function)(
                    io_expression_monitor.key,
                    local_evaluation,
                    local_last_evaluation);
                ++local_last;
            }
        }
        io_expression_monitor.functions.erase(
            local_last, io_expression_monitor.functions.end());
        return true;
    }

    //-------------------------------------------------------------------------
    /// 条件式監視器の辞書。
    private: typename this_type::expression_monitor::vector expression_monitors_;

    /// 状態監視器の辞書。
    private: typename this_type::state_monitor::vector state_monitors_;

}; // class psyq::scenario_engine::dispatcher

#endif // !defined(PSYQ_SCENARIO_ENGINE_DISPATCHER_HPP_)
// vim: set expandtab:
