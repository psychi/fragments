/// @file
/// @brief @copybrief psyq::if_then_engine::_private::expression_monitor
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_MONITOR_HPP_
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_MONITOR_HPP_

#include <cstdint>
#include <bitset>
#include <vector>

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            template<typename> class expression_monitor;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 条件式監視器。条件式の評価の変化を検知し、条件挙動ハンドラに通知する。
/// @tparam template_handler_container @copydoc expression_monitor::handler_container
template<typename template_handler_container>
class psyq::if_then_engine::_private::expression_monitor
{
    /// @brief thisが指す値の型。
    private: typedef expression_monitor this_type;

    //-------------------------------------------------------------------------
    /// @brief 条件式監視器で保持する _private::handler のコンテナ。
    public: typedef template_handler_container handler_container;
    /// @brief 条件式監視器で保持する _private::handler 。
    public: typedef typename template_handler_container::value_type handler;

    //-------------------------------------------------------------------------
    /// @brief this_type::flags_ の構成。
    private: enum flag: std::uint8_t
    {
        flag_VALID_TRANSITION,   ///< 状態変化の取得に成功。
        flag_INVALID_TRANSITION, ///< 状態変化の取得に失敗。
        flag_LAST_EVALUATION,    ///< 条件式の前回の評価の成功／失敗。
        flag_LAST_CONDITION,     ///< 条件式の前回の評価。
        flag_FLUSH_CONDITION,    ///< 条件式の前回の評価を無視する。
        flag_REGISTERED,         ///< 条件式の登録済みフラグ。
    };
    /// @brief handler::function の優先順位を比較する関数オブジェクト。
    private: struct handler_priority_less
    {
        bool operator()(
            typename expression_monitor::handler const& in_left,
            typename expression_monitor::handler const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.get_priority() < in_right.get_priority();
        }

        bool operator()(
            typename expression_monitor::handler const& in_left,
            typename expression_monitor::handler::priority const in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.get_priority() < in_right;
        }

        bool operator()(
            typename expression_monitor::handler::priority const in_left,
            typename expression_monitor::handler const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left < in_right.get_priority();
        }

    }; // struct handler_priority_less

    //-------------------------------------------------------------------------
    /// @brief 条件式監視器を構築する。
    public: explicit expression_monitor(
        /// [in] メモリ割当子の初期値。
        typename this_type::handler_container::allocator_type const& in_allocator):
    handlers_(in_allocator)
    {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /// @brief ムーブ構築子。
    public: expression_monitor(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source):
    handlers_(std::move(io_source.handlers_)),
    flags_(std::move(io_source.flags_))
    {}

    /// @brief ムーブ代入演算子。
    /// @return *this
    public: this_type& operator=(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->handlers_ = std::move(io_source.handlers_);
            this->flags_ = std::move(io_source.flags_);
        }
        return *this;
    }
#endif // !defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    //-------------------------------------------------------------------------
    /// @brief 条件挙動ハンドラを登録する。
    /// @sa
    /// dispatcher::_dispatch で、 in_expression_key に対応する条件式の評価が変化し
    /// in_condition と合致すると、 in_function の指す条件挙動関数が呼び出される。
    /// @sa
    /// in_function の指す条件挙動関数が解体されると、それを弱参照している
    /// this_type::handler は自動的に削除される。
    /// 明示的に削除するには this_type::unregister_handler を使う。
    /// @retval true
    /// 成功。 in_function の指す条件挙動関数を弱参照する
    /// this_type::handler を構築し、 io_expression_monitors に登録した。
    /// @retval false
    /// 失敗。 this_type::handler は構築されなかった。
    /// - in_condition が this_type::handler::INVALID_CONDITION だと、失敗する。
    /// - in_function が空か、空の関数を指していると、失敗する。
    /// - in_expression_key と対応する this_type::handler に、
    ///   in_function の指す条件挙動関数が既に登録されていると、失敗する。
    public: template<typename template_expression_monitor_map>
    static bool register_handler(
        /// [in,out] this_type::handler を登録する expression_monitor の辞書。
        template_expression_monitor_map& io_expression_monitors,
        /// [in] in_function の指す条件挙動関数に対応する
        /// evaluator::expression の識別値。
        typename this_type::handler::expression_key const& in_expression_key,
        /// [in] in_function の指す条件挙動関数を呼び出す挙動条件。
        /// handler::make_condition から作る。
        typename this_type::handler::condition const in_condition,
        /// [in] 登録する handler::function を指すスマートポインタ。
        /// in_expression_key に対応する条件式の評価が変化して
        /// in_condition に合致すると、呼び出される。
        typename this_type::handler::function_shared_ptr const& in_function,
        /// [in] in_function の指す条件挙動関数の呼び出し優先順位。
        /// 昇順に呼び出される。
        typename this_type::handler::priority const in_priority)
    {
        auto const local_function(in_function.get());
        if (in_condition != this_type::handler::INVALID_CONDITION
            && local_function != nullptr
            && static_cast<bool>(*local_function))
        {
            // 条件式監視器を用意し、同じ関数が登録されてないか判定する。
            auto const local_emplace(
                io_expression_monitors.emplace(
                    in_expression_key,
                    this_type(io_expression_monitors.get_allocator())));
            auto& local_handlers(local_emplace.first->second.handlers_);
            if (local_emplace.second
                || !this_type::trim_handlers(local_handlers, local_function, false))
            {
                // 条件式監視器へ条件挙動ハンドラを追加する。
                local_handlers.emplace_back(in_condition, in_function, in_priority);
                return true;
            }
        }
        return false;
    }

    /// @brief 条件挙動関数を弱参照している条件挙動ハンドラを削除する。
    /// @retval true  in_function を弱参照している this_type::handler を削除した。
    /// @retval false 該当する this_type::handler がない。
    public: bool unregister_handler(
        /// [in] 削除する this_type::handler に対応する条件挙動関数。
        typename this_type::handler::function const& in_function)
    {
        return this_type::trim_handlers(this->handlers_, &in_function, true);
    }

    /// @brief 条件挙動ハンドラを整理する。
    /// @retval true  this_type::handler がなくなった。
    /// @retval false this_type::handler はまだある。
    public: bool shrink_handlers()
    {
        this_type::trim_handlers(this->handlers_, nullptr, false);
        this->handlers_.shrink_to_fit();
        return this->handlers_.empty();
    }

    //-------------------------------------------------------------------------
    /// @brief 条件式を状態監視器へ登録する。
    /// @details
    /// io_expression_monitors の要素が監視している条件式から参照する
    /// 状態値が変化した際に通知されるよう、監視している条件式を
    /// status_monitor へ登録する。
    public: template<
        typename template_status_monitor_map,
        typename template_expression_monitor_map,
        typename template_evaluator>
    static void register_expressions(
        /// [in,out] 条件式を登録する status_monitor の辞書。
        template_status_monitor_map& io_status_monitors,
        /// [in,out] 条件式を監視している expression_monitor の辞書。
        template_expression_monitor_map& io_expression_monitors,
        /// [in] 監視している条件式を持つ _private::evaluator 。
        template_evaluator const& in_evaluator)
    {
        for (auto& local_expression_monitor: io_expression_monitors)
        {
            auto& local_flags(local_expression_monitor.second.flags_);
            if (!local_flags.test(this_type::flag_REGISTERED))
            {
                auto const local_register_expression(
                    this_type::register_expression(
                        io_status_monitors,
                        io_expression_monitors,
                        local_expression_monitor.first,
                        local_expression_monitor.first,
                        in_evaluator));
                if (local_register_expression != 0)
                {
                    local_flags.set(this_type::flag_REGISTERED);
                    local_flags.set(
                        this_type::flag_FLUSH_CONDITION,
                        local_register_expression < 0);
                }
            }
        }
    }

    /// @brief 状態値の変化を条件式監視器へ通知する。
    public: template<
        typename template_expression_map,
        typename template_key_container>
    static void notify_status_transition(
        /// [in,out] 状態変化の通知を受け取る expression_monitor の辞書。
        template_expression_map& io_expression_monitors,
        /// [in,out] 状態変化を通知する evaluator::expression_key のコンテナ。
        template_key_container& io_expression_keys,
        /// [in] 状態値が存在するかどうか。
        bool const in_status_existence)
    {
        auto const local_flag_key(
            in_status_existence?
                this_type::flag_VALID_TRANSITION:
                this_type::flag_INVALID_TRANSITION);
        for (auto i(io_expression_keys.begin()); i != io_expression_keys.end();)
        {
            // 状態変化を通知する条件式監視器を取得する。
            auto const local_find(io_expression_monitors.find(*i));
            if (local_find == io_expression_monitors.end())
            {
                // 監視器のない条件式を削除し、コンテナを整理する。
                i = io_expression_keys.erase(i);
            }
            else
            {
                ++i;
                // 状態変化を条件式監視器へ知らせる。
                auto& local_flags(local_find->second.flags_);
                if (local_flags.test(this_type::flag_REGISTERED))
                {
                    local_flags.set(local_flag_key);
                }
            }
        }
    }

    /// @brief 条件式の評価の変化を検知し、条件挙動ハンドラをキャッシュに貯める。
    /// @details
    /// evaluator::expression の評価が最新と前回で異なっており、且つ
    /// this_type::register_handler で登録した handler::condition と合致するなら、
    /// this_type::handler を io_cached_handlers に貯める。
    public: template<
        typename template_handler_cache_container,
        typename template_expression_monitor_map,
        typename template_evaluator>
    static void cache_handlers(
        /// [in,out] 挙動条件に合致した handler::cache を貯めるコンテナ。
        template_handler_cache_container& io_cached_handlers,
        /// [in,out] evaluator::expression の評価の変化を検知する
        /// expression_monitor の辞書。
        template_expression_monitor_map& io_expression_monitors,
        /// [in] 評価する evaluator::expression から参照する _private::reservoir 。
        typename template_evaluator::reservoir const& in_reservoir,
        /// [in] 評価する evaluator::expression を持つ _private::evaluator 。
        template_evaluator const& in_evaluator)
    {
        for (
            auto i(io_expression_monitors.begin());
            i != io_expression_monitors.end();)
        {
            // 条件式の評価の要求を検知する。
            auto& local_expression_key(i->first);
            auto& local_expression_monitor(i->second);
            if (local_expression_monitor.detect_transition(
                    in_evaluator, local_expression_key))
            {
                // 条件挙動ハンドラをキャッシュに貯める。
                local_expression_monitor.cache_handlers(
                    io_cached_handlers,
                    in_reservoir,
                    in_evaluator,
                    local_expression_key);
                if (local_expression_monitor.handlers_.empty())
                {
                    // 条件挙動コンテナが空になったら、条件式監視器を削除する。
                    i = io_expression_monitors.erase(i);
                    continue;
                }
            }
            ++i;
        }
    }

    //-------------------------------------------------------------------------
    /// @brief 条件式が参照する状態値を状態監視器へ登録する。
    /// @retval 正 成功。条件式の評価を維持する。
    /// @retval 負 成功。条件式の評価を維持しない。
    /// @retval 0  失敗。
    private: template<
        typename template_status_monitor_map,
        typename template_expression_monitor_map,
        typename template_evaluator>
    static std::int8_t register_expression(
        /// [in,out] 監視する状態値を登録する status_monitor の辞書。
        template_status_monitor_map& io_status_monitors,
        /// [in] 条件式を監視する expression_monitor の辞書。
        template_expression_monitor_map const& in_expression_monitors,
        /// [in] 登録する条件式の識別値。
        typename template_evaluator::expression_key const& in_register_key,
        /// [in] 走査する条件式の識別値。
        typename template_evaluator::expression_key const& in_scan_key,
        /// [in] 走査する条件式を持つ _private::evaluator 。
        template_evaluator const& in_evaluator)
    {
        // in_scan_key に対応する条件式と要素条件チャンクを取得する。
        auto const local_expression(in_evaluator.find_expression(in_scan_key));
        if (local_expression.is_empty())
        {
            return 0;
        }
        auto const local_chunk(
            in_evaluator._find_chunk(local_expression.get_chunk_key()));
        if (local_chunk == nullptr)
        {
            // 条件式があれば、要素条件チャンクもあるはず。
            PSYQ_ASSERT(false);
            return 0;
        }

        // in_scan_key に対応する条件式の種類によって、
        // in_register_key の登録先を選択する。
        switch (local_expression.get_kind())
        {
            case template_evaluator::expression::kind_SUB_EXPRESSION:
            return this_type::register_compound_expression(
                io_status_monitors,
                in_expression_monitors,
                in_register_key,
                local_expression,
                local_chunk->sub_expressions_,
                in_evaluator);

            case template_evaluator::expression::kind_STATUS_TRANSITION:
            template_status_monitor_map::mapped_type::register_expression(
                io_status_monitors,
                in_register_key,
                local_expression,
                local_chunk->status_transitions_);
            return -1;

            case template_evaluator::expression::kind_STATUS_COMPARISON:
            template_status_monitor_map::mapped_type::register_expression(
                io_status_monitors,
                in_register_key,
                local_expression,
                local_chunk->status_comparisons_);
            return 1;

            default:
            // 未対応の条件式の種類だった。
            PSYQ_ASSERT(false);
            return 0;
        }
    }

    /// @brief 複合条件式を状態監視器へ登録する。
    /// @retval 正 成功。条件式の評価を維持する。
    /// @retval 負 成功。条件式の評価を維持しない。
    /// @retval 0  失敗。
    private: template<
        typename template_status_monitor_map,
        typename template_expression_monitor_map,
        typename template_evaluator>
    static std::int8_t register_compound_expression(
        /// [in,out] 状態変化を条件式監視器に知らせる status_monitor の辞書。
        template_status_monitor_map& io_status_monitors,
        /// [in] 条件式の評価の変化を検知する expression_monitor の辞書。
        template_expression_monitor_map const& in_expression_monitors,
        /// [in] 登録する複合条件式の識別値。
        typename template_evaluator::expression_key const& in_expression_key,
        /// [in] 走査する複合条件式。
        typename template_evaluator::expression const& in_expression,
        /// [in] in_expression が参照する要素条件コンテナ。
        typename template_evaluator::chunk::sub_expression_container const&
            in_sub_expressions,
        /// [in] 条件式を持つ _private::evaluator 。
        template_evaluator const& in_evaluator)
    {
        // in_expression の要素条件を走査し、
        // in_expression_key を状態監視器へ登録する。
        std::int8_t local_result(1);
        for (
            auto i(in_expression.get_begin_element());
            i < in_expression.get_end_element();
            ++i)
        {
            auto const local_register_expression(
                this_type::register_expression(
                    io_status_monitors,
                    in_expression_monitors,
                    in_expression_key,
                    in_sub_expressions.at(i).get_key(),
                    in_evaluator));
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

    /// @brief 条件挙動ハンドラを検索しつつ、コンテナを整理する。
    /// @retval true  io_handlers から in_function が見つかった。
    /// @retval false io_handlers から in_function が見つからなかった。
    private: static bool trim_handlers(
        /// [in,out] 走査する条件挙動ハンドラのコンテナ。
        typename this_type::handler_container& io_handlers,
        /// [in] 検索する挙動挙動ハンドラに対応する条件挙動関数。
        typename this_type::handler::function const* const in_function,
        /// [in] 検索する条件挙動ハンドラを削除するかどうか。
        bool const in_erase)
    {
        auto local_find(in_function == nullptr);
        for (auto i(io_handlers.begin()); i != io_handlers.end();)
        {
            auto& local_observer(i->get_function());
            bool local_erase;
            if (local_find)
            {
                local_erase = local_observer.expired();
            }
            else
            {
                auto const local_pointer(local_observer.lock().get());
                local_find = local_pointer == in_function;
                local_erase = local_find? in_erase: local_pointer == nullptr;
            }
            if (local_erase)
            {
                i = io_handlers.erase(i);
            }
            else
            {
                ++i;
            }
        }
        return local_find && in_function != nullptr;
    }

    /// @copydoc this_type::cache_handlers
    private: template<
        typename template_handler_cache_container, typename template_evaluator>
    void cache_handlers(
        /// [in,out] handler::cache を貯めるコンテナ。
        template_handler_cache_container& io_cached_handlers,
        /// [in] 評価する evaluator::expression から参照する _private::reservoir 。
        typename template_evaluator::reservoir const& in_reservoir,
        /// [in] 評価する evaluator::expression を持つ _private::evaluator 。
        template_evaluator const& in_evaluator,
        /// [in] 評価する evaluator::expression の識別値。
        typename template_evaluator::expression_key const& in_expression_key)
    {
        // 条件式を評価し、結果が前回から変化してないか判定する。
        auto const local_flush_condition(
            this->flags_.test(this_type::flag_FLUSH_CONDITION));
        auto const local_last_evaluation(
            this->get_last_evaluation(local_flush_condition));
        auto const local_current_evaluation(
            this->evaluate_expression(
                in_reservoir,
                in_evaluator,
                in_expression_key,
                local_flush_condition));
        if (local_last_evaluation == local_current_evaluation)
        {
            // 前回と今回で評価結果が同じなら、何もしない。
            return;
        }

        // 条件式の評価の変化が挙動条件と合致すれば、
        // 条件挙動ハンドラをキャッシュに貯める。
        for (auto i(this->handlers_.begin()); i != this->handlers_.end();)
        {
            auto const& local_handler(*i);
            if (local_handler.get_function().expired())
            {
                i = this->handlers_.erase(i);
                continue;
            }
            ++i;
            if (this_type::handler::is_matched_condition(
                    local_handler.get_condition(),
                    local_current_evaluation,
                    local_last_evaluation))
            {
                // 優先順位の昇順となるよう、条件挙動キャッシュを挿入する。
                io_cached_handlers.emplace(
                    std::upper_bound(
                        io_cached_handlers.cbegin(),
                        io_cached_handlers.cend(),
                        local_handler.get_priority(),
                        typename this_type::handler_priority_less()),
                    local_handler,
                    in_expression_key,
                    local_current_evaluation,
                    local_last_evaluation);
            }
        }
    }

    /// @brief 条件式を評価する。
    /// @retval 正 条件式の評価は真となった。
    /// @retval 0  条件式の評価は偽となった。
    /// @retval 負 条件式の評価に失敗した。
    private: template<typename template_evaluator>
    typename this_type::handler::evaluation evaluate_expression(
        /// [in] 条件式から参照する _private::reservoir 。
        typename template_evaluator::reservoir const& in_reservoir,
        /// [in] 評価する条件式を持つ _private::evaluator 。
        template_evaluator const& in_evaluator,
        /// [in] 評価する条件式の識別値。
        typename template_evaluator::expression_key const& in_expression_key,
        /// [in] 前回の評価を無視するかどうか。
        bool const in_flush)
    {
        // 状態変化フラグを更新する。
        auto const local_invalid_transition(
            this->flags_.test(this_type::flag_INVALID_TRANSITION));
        this->flags_.reset(this_type::flag_VALID_TRANSITION);
        this->flags_.reset(this_type::flag_INVALID_TRANSITION);

        // 状態値の取得の失敗を検知したら、条件式の評価も失敗とみなす。
        if (local_invalid_transition)
        //if (in_flush && local_invalid_transition)
        {
            this->flags_.reset(this_type::flag_LAST_EVALUATION);
            this->flags_.reset(this_type::flag_LAST_CONDITION);
            return -1;
        }

        // 条件式を評価し、結果を記録する。
        auto const local_evaluate_expression(
            in_evaluator.evaluate_expression(in_expression_key, in_reservoir));
        this->flags_.set(
            this_type::flag_LAST_EVALUATION, 0 <= local_evaluate_expression);
        this->flags_.set(
            this_type::flag_LAST_CONDITION, 0 < local_evaluate_expression);
        return this->get_last_evaluation(false);
    }

    /// @brief 条件式の評価要求を検知する。
    private: template<typename template_evaluator>
    bool detect_transition(
        /// [in] 条件式の評価に使う _private::evaluator 。
        template_evaluator const& in_evaluator,
        /// [in] 評価する条件式の識別値。
        typename template_evaluator::expression_key const& in_expression_key)
    {
        if (this->flags_.test(this_type::flag_INVALID_TRANSITION)
            || this->flags_.test(this_type::flag_VALID_TRANSITION))
        {
            return true;
        }

        // 条件式の生成と削除を検知する。
        auto const local_exist(in_evaluator.is_registered(in_expression_key));
        auto const local_last_evaluation(
            this->flags_.test(this_type::flag_LAST_EVALUATION));
        auto const local_invalid(!local_exist && local_last_evaluation);
        auto const local_valid(local_exist && !local_last_evaluation);
        this->flags_.set(this_type::flag_INVALID_TRANSITION, local_invalid);
        this->flags_.set(this_type::flag_VALID_TRANSITION, local_valid);
        return local_invalid || local_valid;
    }

    /// @brief 監視している条件式の前回の評価を取得する。
    /// @retval 正 条件式の評価は真となった。
    /// @retval 0  条件式の評価は偽となった。
    /// @retval 負 条件式の評価に失敗した。
    private: typename this_type::handler::evaluation get_last_evaluation(
        /// [in] 前回の評価を無視する。
        bool const in_flush)
    const PSYQ_NOEXCEPT
    {
        return this->flags_.test(this_type::flag_LAST_EVALUATION)?
            !in_flush && this->flags_.test(this_type::flag_LAST_CONDITION): -1;
    }

    /// @brief 登録されている条件挙動ハンドラを取得する。
    /// @warning psyq::if_then_engine 管理者以外は使用禁止。
    /// @return
    /// in_function に対応する this_type::handler を指すポインタ。
    /// 該当する this_type::handler がない場合は nullptr を返す。
    public: typename this_type::handler const* _find_handler_ptr(
        /// [in] 取得する this_type::handler に対応する条件挙動関数。
        typename this_type::handler::function const& in_function)
    const PSYQ_NOEXCEPT
    {
        for (auto& local_handler: this->handlers_)
        {
            if (!local_handler.expired()
                && local_handler.lock().get() == &in_function)
            {
                return &local_handler;
            }
        }
        return nullptr;
    }

    //-------------------------------------------------------------------------
    /// @copydoc this_type::handler_container
    private: typename this_type::handler_container handlers_;
    /// @brief 条件式の評価結果を記録するフラグの集合。
    private: std::bitset<8> flags_;

}; // class psyq::if_then_engine::_private::expression_monitor

#endif // defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_MONITOR_HPP_)
// vim: set expandtab:
