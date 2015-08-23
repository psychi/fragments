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
/// @brief 条件式挙動ハンドラ。条件式の評価の変化を検知した際に呼び出す関数を保持する。
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
    /// @copydoc psyq::if_then_engine::_private::evaluator::expression_key
    public: typedef template_expression_key expression_key;
    /// @copydoc psyq::if_then_engine::evaluation
    public: typedef template_evaluation evaluation;
    /// @brief this_type::function の呼び出し優先順位の型。
    /// @details 関数は、優先順位の昇順で呼び出される。
    public: typedef template_priority priority;
    /// @brief this_type::function を呼び出す、条件式の評価の変化の条件を表す型。
    /// @details dispatcher::make_condition で条件を作る。
    public: typedef std::uint8_t condition;

    //-------------------------------------------------------------------------
    /// @brief 条件式の評価が条件と合致した際に呼び出す、関数オブジェクトの型。
    /// @details
    /// - 引数#0は、評価した条件式の識別値。
    /// - 引数#1は、 evaluator::evaluate_expression の今回の戻り値。
    /// - 引数#2は、 evaluator::evaluate_expression の前回の戻り値。
    public: typedef
        std::function<
            void (
                typename this_type::expression_key const&,
                typename this_type::evaluation const,
                typename this_type::evaluation const)>
        function;
    /// @brief this_type::function の、所有権ありスマートポインタ。
    public: typedef
        std::shared_ptr<typename this_type::function>
        function_shared_ptr;
    /// @brief this_type::function の、所有権なしスマートポインタ。
    public: typedef
        std::weak_ptr<typename this_type::function>
        function_weak_ptr;

    //-------------------------------------------------------------------------
    private: enum: std::uint8_t
    {
        CONDITION_BIT_WIDTH = 3, ///< 変化条件に使うビット数。
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
            /// [in] this_type::evaluation_ の初期値。
            typename base_type::evaluation const in_evaluation,
            /// [in] this_type::last_evaluation_ の初期値。
            typename base_type::evaluation const in_last_evaluation):
        base_type(in_handler),
        expression_key_(std::move(in_expression_key)),
        evaluation_(in_evaluation),
        last_evaluation_(in_last_evaluation)
        {}

        /// @brief handler::function を呼び出す。
        public: void call_function() const
        {
            auto const local_function_holder(this->get_function().lock());
            auto const local_function(local_function_holder.get());
            if (local_function != nullptr)
            {
                (*local_function)(
                    this->expression_key_,
                    this->evaluation_,
                    this->last_evaluation_);
            }
        }

        /// @brief 条件式の識別値。
        private: typename base_type::expression_key expression_key_;
        /// @brief 条件式の今回の評価結果。
        private: typename base_type::evaluation evaluation_;
        /// @brief 条件式の前回の評価結果。
        private: typename base_type::evaluation last_evaluation_;

    }; // class cache

    //---------------------------------------------------------------------
    /// @brief 条件挙動ハンドラを構築する。
    public: handler(
        /// [in] this_type::condition_ の初期値。
        typename this_type::condition const in_condition,
        /// [in] this_type::function_ の初期値。
        typename this_type::function_weak_ptr in_function,
        /// [in] this_type::priority_ の初期値。
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
    /// @brief 関数を呼び出す条件となる、条件式の評価の変化を取得する。
    /// @return @copydoc this_type::condition_
    /// @sa this_type::make_condition で、条件を構築できる。
    public: typename this_type::condition get_condition() const PSYQ_NOEXCEPT
    {
        return this->condition_;
    }

    /// @brief 条件と合致した際に呼び出す関数を取得する。
    /// @return @copydoc this_type::function_
    public: typename this_type::function_weak_ptr const& get_function()
    const PSYQ_NOEXCEPT
    {
        return this->function_;
    }

    /// @brief 条件と合致した際の、関数を呼び出す優先順位を取得する。
    /// @return @copydoc this_type::priority_
    public: typename this_type::priority get_priority() const PSYQ_NOEXCEPT
    {
        return this->priority_;
    }

    //-------------------------------------------------------------------------
    /// @brief 条件式の評価と条件が合致するか判定する。
    /// @retval true  合致した。
    /// @retval false 合致しなかった。
    public: static bool is_matched_condition(
        /// [in] 条件となる評価の変化。
        /// this_type::make_condition で構築できる。
        typename this_type::condition const in_condition,
        /// [in] 条件式の最新の評価結果。
        typename this_type::evaluation const in_evaluation,
        /// [in] 条件式の前回の評価結果。
        typename this_type::evaluation const in_last_evaluation)
    PSYQ_NOEXCEPT
    {
        // 評価変化条件に合致するか判定する。
        auto const local_make_condition(
            [](typename this_type::evaluation const in_eval)
            ->typename this_type::evaluation
            {
                return in_eval < 0? 1: (in_eval <= 0? 2: 4);
            });
        auto const local_condition(
            local_make_condition(in_evaluation) | (
                local_make_condition(in_last_evaluation)
                << this_type::CONDITION_BIT_WIDTH));
        return local_condition == (local_condition & in_condition);
    }

    /// @brief 関数を呼び出す条件を作る。
    /// @return 関数を呼び出す条件。
    public: static typename this_type::condition make_condition(
        /// [in] 条件式の最新の評価が、真に変化したことが条件。
        bool const in_now_true,
        /// [in] 条件式の最新の評価が、偽に変化したことが条件。
        bool const in_now_false,
        /// [in] 条件式の最新の評価が、失敗に変化したことが条件。
        bool const in_now_failed,
        /// [in] 条件式の前回の評価が、真だったことが条件。
        bool const in_last_true,
        /// [in] 条件式の前回の評価が、偽だったことが条件。
        bool const in_last_false,
        /// [in] 条件式の前回の評価が、失敗だったことが条件。
        bool const in_last_failed)
    PSYQ_NOEXCEPT
    {
        return static_cast<typename this_type::condition>(in_now_failed)
            | (in_now_false << 1)
            | (in_now_true << 2)
            | (in_last_failed << this_type::CONDITION_BIT_WIDTH)
            | (in_last_false << (this_type::CONDITION_BIT_WIDTH + 1))
            | (in_last_true << (this_type::CONDITION_BIT_WIDTH + 2));
    }

    /// @brief 関数を呼び出す条件を作る。
    /// @return 関数を呼び出す条件。
    public: static typename this_type::condition make_condition(
        /// [in] 条件式の最新の評価。
        typename this_type::evaluation const in_now_evaluation,
        /// [in] 条件式の前回の評価。
        typename this_type::evaluation const in_last_evaluation)
    {
        return this_type::make_condition(
            0 < in_now_evaluation,
            in_now_evaluation == 0,
            in_now_evaluation < 0,
            0 < in_last_evaluation,
            in_last_evaluation == 0,
            in_last_evaluation < 0);
    }

    //---------------------------------------------------------------------
    /// @brief 条件と合致した際に呼び出す関数を指すスマートポインタ。
    private: typename this_type::function_weak_ptr function_;
    /// @brief 条件と合致した際に、関数を呼び出す優先順位。昇順に呼び出される。
    private: typename this_type::priority priority_;
    /// @brief 関数を呼び出す条件となる、条件式の評価の変化。
    private: typename this_type::condition condition_;

}; // class handler

#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_HPP_)
// vim: set expandtab:
