/// @file
/// @brief @copybrief psyq::if_then_engine::_private::handler
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_HPP_
#define PSYQ_IF_THEN_ENGINE_HANDLER_HPP_

#include <cstdint>
#include <functional>
#include <memory>

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            template<typename, typename, typename> class handler;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 条件挙動ハンドラ。関数を呼び出す条件を保持し、呼び出される関数を弱参照する。
/// @tparam template_expression_key @copydoc handler::expression_key
/// @tparam template_evaluation     @copydoc handler::evaluation
/// @tparam template_priority       @copydoc handler::priority
template<
    typename template_expression_key,
    typename template_evaluation,
    typename template_priority>
class psyq::if_then_engine::_private::handler
{
    /// @brief this が指す値の型。
    private: typedef handler this_type;

    //-------------------------------------------------------------------------
    /// @copydoc evaluator::expression_key
    public: typedef template_expression_key expression_key;
    /// @copydoc expression::evaluation
    public: typedef template_evaluation evaluation;
    /// @brief handler::function の呼び出し優先順位。
    /// @details 優先順位の昇順で呼び出される。
    public: typedef template_priority priority;
    /// @brief 挙動条件。条件挙動関数を呼び出す条件。
    /// @details
    ///   expression_monitor::cache_handlers で条件式の評価の変化を検知した際に、
    ///   挙動条件と条件式の評価が合致すると、 handler::function が呼び出される。
    /// @sa
    ///   handler::make_condition
    ///   で、条件式の最新の評価と前回の評価を組み合わせて作る。
    public: typedef std::uint8_t condition;
    /// @brief 単位条件。 handler::condition を構成する単位となる条件。
    /// @details handler::make_condition で条件を作る引数として使う。
    public: enum unit_condition: typename this_type::condition
    {
        /// @brief 無効な条件。
        INVALID_CONDITION = 0,
        /// @brief 条件式の評価に失敗していることが条件。
        unit_condition_NULL = 1,
        /// @brief 条件式の評価が偽であることが条件。
        unit_condition_FALSE = 2,
        /// @brief 条件式の評価が真であることが条件。
        unit_condition_TRUE = 4,
        /// @brief 条件式の評価に失敗してないことが条件。
        unit_condition_NOT_NULL = unit_condition_FALSE | unit_condition_TRUE,
        /// @brief 条件式の評価が偽以外であることが条件。
        unit_condition_NOT_FALSE = unit_condition_NULL | unit_condition_TRUE,
        /// @brief 条件式の評価が真以外であることが条件。
        unit_condition_NOT_TRUE = unit_condition_FALSE | unit_condition_NULL,
        /// @brief 条件式の評価を問わない。
        unit_condition_ANY =
            unit_condition_NULL | unit_condition_FALSE | unit_condition_TRUE,
    };

    //-------------------------------------------------------------------------
    /// @brief 条件挙動関数。挙動条件に合致すると呼び出される関数。
    /// @details
    ///   expression_monitor::cache_handlers で条件式の評価の変化を検知した際に、
    ///   handler::condition と条件式の評価が合致すると、呼び出される関数。
    ///   - 引数#0は、評価が変化した条件式の識別値。
    ///   - 引数#1は、 evaluator::evaluate_expression の最新の戻り値。
    ///   - 引数#2は、 evaluator::evaluate_expression の前回の戻り値。
    public: typedef
        std::function<
            void (
                typename this_type::expression_key const&,
                typename this_type::evaluation const,
                typename this_type::evaluation const)>
        function;
    /// @brief handler::function の強参照スマートポインタ。
    public: typedef
        std::shared_ptr<typename this_type::function>
        function_shared_ptr;
    /// @brief handler::function の弱参照スマートポインタ。
    public: typedef
        std::weak_ptr<typename this_type::function>
        function_weak_ptr;

    //-------------------------------------------------------------------------
    private: enum: std::uint8_t
    {
        UNIT_CONDITION_BIT_WIDTH = 3, ///< 単位条件に使うビット幅。
    };

    //-------------------------------------------------------------------------
    /// @brief 条件挙動ハンドラのキャッシュ。
    public: class cache: public handler
    {
        /// @brief this の指す値の型。
        private: typedef cache this_type;
        /// @brief this_type の基底型。
        public: typedef handler base_type;

        /// @brief 条件挙動ハンドラのキャッシュを構築する。
        public: cache(
            /// [in] キャッシュする条件挙動ハンドラ。
            handler const& in_handler,
            /// [in] this_type::expression_key_ の初期値。
            typename base_type::expression_key in_expression_key,
            /// [in] this_type::current_evaluation_ の初期値。
            typename base_type::evaluation const in_current_evaluation,
            /// [in] this_type::last_evaluation_ の初期値。
            typename base_type::evaluation const in_last_evaluation):
        base_type(in_handler),
        expression_key_(std::move(in_expression_key)),
        current_evaluation_(in_current_evaluation),
        last_evaluation_(in_last_evaluation)
        {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
        /// @brief ムーブ構築子。
        public: cache(
            /// [in,out] ムーブ元となるインスタンス。
            this_type&& io_source):
        base_type(std::move(io_source)),
        expression_key_(std::move(io_source.expression_key_)),
        current_evaluation_(std::move(io_source.current_evaluation_)),
        last_evaluation_(std::move(io_source.last_evaluation_))
        {}

        /// @brief ムーブ代入演算子。
        /// @return *this
        public: this_type& operator=(
            /// [in,out] ムーブ元となるインスタンス。
            this_type&& io_source)
        {
            this->base_type::operator=(std::move(io_source));
            this->expression_key_ = std::move(io_source.expression_key_);
            this->current_evaluation_ = std::move(io_source.current_evaluation_);
            this->last_evaluation_ = std::move(io_source.last_evaluation_);
            return *this;
        }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

        /// @brief 条件挙動関数を呼び出す。
        public: void call_function() const
        {
            auto const local_function_holder(this->get_function().lock());
            auto const local_function(local_function_holder.get());
            if (local_function != nullptr)
            {
                (*local_function)(
                    this->expression_key_,
                    this->current_evaluation_,
                    this->last_evaluation_);
            }
        }

        /// @brief 条件式の識別値。
        private: typename base_type::expression_key expression_key_;
        /// @brief 条件式の最新の評価結果。
        private: typename base_type::evaluation current_evaluation_;
        /// @brief 条件式の前回の評価結果。
        private: typename base_type::evaluation last_evaluation_;

    }; // class cache

    //---------------------------------------------------------------------
    /// @brief 条件挙動ハンドラを構築する。
    public: handler(
        /// [in] handler::condition_ の初期値。 handler::make_condition で作る。
        typename this_type::condition const in_condition,
        /// [in] handler::function_ の初期値。
        typename this_type::function_weak_ptr in_function,
        /// [in] handler::priority_ の初期値。
        typename this_type::priority const in_priority):
    function_(std::move(in_function)),
    priority_(in_priority),
    condition_(in_condition)
    {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /// @brief ムーブ構築子。
    public: handler(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source):
    function_(std::move(io_source.function_)),
    priority_(std::move(io_source.priority_)),
    condition_(std::move(io_source.condition_))
    {}

    /// @brief ムーブ代入演算子。
    /// @return *this
    public: this_type& operator=(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source)
    {
        this->function_ = std::move(io_source.function_);
        this->priority_ = std::move(io_source.priority_);
        this->condition_ = std::move(io_source.condition_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    //-------------------------------------------------------------------------
    /// @brief 挙動条件を取得する。
    /// @return @copydoc handler::condition_
    public: typename this_type::condition get_condition() const PSYQ_NOEXCEPT
    {
        return this->condition_;
    }

    /// @brief 条件挙動関数を取得する。
    /// @return @copydoc handler::function_
    public: typename this_type::function_weak_ptr const& get_function()
    const PSYQ_NOEXCEPT
    {
        return this->function_;
    }

    /// @brief 条件挙動関数の呼び出し優先順位を取得する。
    /// @return @copydoc handler::priority_
    public: typename this_type::priority get_priority() const PSYQ_NOEXCEPT
    {
        return this->priority_;
    }

    //-------------------------------------------------------------------------
    /// @brief 条件式の評価の遷移と挙動条件が合致するか判定する。
    public: bool is_matched(
        /// [in] 条件式の評価の、最新と前回を合成した値。
        typename this_type::condition const in_transition)
    const
    {
        return (
            PSYQ_ASSERT(in_transition != this_type::INVALID_CONDITION),
            in_transition == (in_transition & this->get_condition()));
    }

    /// @brief 単位条件を合成して挙動条件を作る。
    /// @warning
    ///   条件式の評価が最新と前回で同じ場合は、
    ///   expression_monitor::cache_handlers で挙動条件の判定が行われない。
    ///   このため、以下の単位条件の組み合わせは無効となることに注意。
    ///   @code
    ///   make_condition(handler::unit_condition_NULL, handler::unit_condition_NULL);
    ///   make_condition(handler::unit_condition_FALSE, handler::unit_condition_FALSE);
    ///   make_condition(handler::unit_condition_TRUE, handler::unit_condition_TRUE);
    ///   @endcode
    /// @return
    ///   関数が呼び出される挙動条件。単位条件の組み合わせが無効な場合は
    ///   this_type::INVALID_CONDITION を返す。
    public: static typename this_type::condition make_condition(
        /// [in] 条件式の、最新の評価の単位条件。
        typename this_type::unit_condition const in_now_condition,
        /// [in] 条件式の、前回の評価の単位条件。
        typename this_type::unit_condition const in_last_condition)
    {
        return this_type::mix_unit_condition(
            in_now_condition != this_type::INVALID_CONDITION
                && in_last_condition != this_type::INVALID_CONDITION
                && (in_now_condition != in_last_condition
                    // 2のべき乗か判定する。
                    || (in_now_condition & (in_now_condition - 1)) != 0),
            in_now_condition,
            in_last_condition);
    }

    /// @brief 条件式の評価を合成して挙動条件を作る。
    /// @warning
    ///   条件式の評価が最新と前回で同じ場合は、
    ///   expression_monitor::cache_handlers で挙動条件の判定が行われない。
    ///   このため、以下の評価の組み合わせは無効となることに注意。
    ///   @code
    ///   // NとMは、それぞれ任意の正の整数。
    ///   make_condition(N, M);
    ///   make_condition(0, 0);
    ///   make_condition(-N, -M);
    ///   @endcode
    /// @return
    ///   関数が呼び出される挙動条件。評価の組み合わせが無効な場合は
    ///   this_type::INVALID_CONDITION を返す。
    public: static typename this_type::condition make_condition(
        /// [in] 条件となる、条件式の最新の評価。
        typename this_type::evaluation const in_now_evaluation,
        /// [in] 条件となる、条件式の前回の評価。
        typename this_type::evaluation const in_last_evaluation)
    {
        auto const local_now_condition(
            this_type::make_unit_condition(in_now_evaluation));
        auto const local_last_condition(
            this_type::make_unit_condition(in_last_evaluation));
        return this_type::mix_unit_condition(
            local_now_condition != local_last_condition,
            local_now_condition,
            local_last_condition);
    }

    /// @brief 条件式の評価から単位条件を作る。
    public: static typename this_type::unit_condition make_unit_condition(
        /// [in] 条件式の評価。
        typename this_type::evaluation const in_evaluation)
    {
        return 0 < in_evaluation?
            this_type::unit_condition_TRUE:
            in_evaluation < 0?
                this_type::unit_condition_NULL:
                this_type::unit_condition_FALSE;
    }


    /// @brief 単位条件を合成して挙動条件を作る。
    /// @return 挙動条件。
    private: static typename this_type::condition mix_unit_condition(
        /// [in] 合成可能かどうか。
        bool const in_mix,
        /// [in] 条件となる、最新の条件式の評価。
        typename this_type::unit_condition const in_now_condition,
        /// [in] 条件となる、前回の条件式の評価。
        typename this_type::unit_condition const in_last_condition)
    {
        return in_mix?
            in_now_condition | (
                in_last_condition << this_type::UNIT_CONDITION_BIT_WIDTH):
            this_type::INVALID_CONDITION;
    }

    //---------------------------------------------------------------------
    /// @brief 条件挙動関数を指すスマートポインタ。
    private: typename this_type::function_weak_ptr function_;
    /// @brief 条件挙動関数の呼び出し優先順位。
    private: typename this_type::priority priority_;
    /// @brief 条件挙動関数を呼び出す挙動条件。
    private: typename this_type::condition condition_;

}; // class psyq::if_then_engine::_private::handler

#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_HPP_)
// vim: set expandtab:
