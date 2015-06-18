/** @file
    @brief @copybrief psyq::scenario_engine::_private::behavior
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_HPP_
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_HPP_

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        namespace _private
        {
            template<typename, typename, typename> class behavior;
        } // namespace _private
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//-------------------------------------------------------------------------
/** @brief 条件挙動。

    @tparam template_expression_key @copydoc psyq::scenario_engine::_private::expression::key
    @tparam template_evaluation     @copydoc psyq::scenario_engine::_private::expression::evaluation
    @tparam template_priority       @copydoc psyq::scenario_engine::dispatcher::function_priority
 */
template<
    typename template_expression_key,
    typename template_evaluation,
    typename template_priority>
class psyq::scenario_engine::_private::behavior
{
    /// @brief thisが指す値の型。
    private: typedef behavior this_type;

    /** @brief 条件式の評価結果が変化した際に呼び出す、条件挙動関数オブジェクトの型。

        - 引数#0は、評価に用いた条件式の識別値。
        - 引数#1は、 evaluator::evaluate_expression の今回の戻り値。
        - 引数#2は、 evaluator::evaluate_expression の前回の戻り値。
     */
    public: typedef std::function<
        void (
            template_expression_key const&,
            template_evaluation const,
            template_evaluation const)>
                function;

    /// @brief this_type::function の、所有権ありスマートポインタ。
    public: typedef std::shared_ptr<typename this_type::function>
        function_shared_ptr;

    /// @brief this_type::function の、所有権なしスマートポインタ。
    public: typedef std::weak_ptr<typename this_type::function>
        function_weak_ptr;

    /// @brief 条件式の評価結果のキャッシュ。
    public: class cache
    {
        private: typedef cache this_type;

        public: cache(
            template_expression_key in_expression_key,
            template_evaluation const in_evaluation,
            template_evaluation const in_last_evaluation)
        :
        expression_key_(std::move(in_expression_key)),
        evaluation_(in_evaluation),
        last_evaluation_(in_last_evaluation)
        {}

        /** @brief 条件挙動関数を呼び出す。
            @param[in] in_behavior 呼び出す条件挙動。
         */
        public: void call_function(typename behavior const& in_behavior)
        const
        {
            auto const local_function_holder(in_behavior.function_.lock());
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
        public: template_expression_key expression_key_;
        /// @brief 条件式の今回の評価結果。
        public: template_evaluation evaluation_;
        /// @brief 条件式の前回の評価結果。
        public: template_evaluation last_evaluation_;

    }; // class cache

    //---------------------------------------------------------------------
    public: behavior(
        typename this_type::function_weak_ptr in_function,
        template_priority const in_priority)
    :
    function_(std::move(in_function)),
    priority_(in_priority)
    {}

    public: behavior(this_type&& io_source):
    function_(std::move(io_source.function_)),
    priority_(std::move(io_source.priority_))
    {}

    public: this_type& operator=(this_type&& io_source)
    {
        this->function_ = std::move(io_source.function_);
        this->priority_ = std::move(io_source.priority_);
        return *this;
    }

    //---------------------------------------------------------------------
    public: typename this_type::function_weak_ptr function_;
    public: template_priority priority_;

}; // class behavior

#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_HPP_)
// vim: set expandtab:
