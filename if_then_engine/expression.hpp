/** @file
    @brief @copybrief psyq::if_then_engine::_private::expression
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_HPP_
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_HPP_

#include <cstdint>
#include "../assert.hpp"

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            template<typename, typename, typename> class expression;
            template<typename> class sub_expression;
            template<typename> class status_transition;
            template<typename, typename, typename, typename>
                class expression_chunk;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 条件式。
    @tparam template_key           @copydoc expression::key
    @tparam template_chunk_key     @copydoc expression::chunk_key
    @tparam template_element_index @copydoc expression::element_index
 */
template<
    typename template_key,
    typename template_chunk_key,
    typename template_element_index>
class psyq::if_then_engine::_private::expression
{
    /// @brief thisが指す値の型。
    private: typedef expression this_type;

    /// @brief 条件式の識別値を表す型。
    public: typedef template_key key;

    /// @brief 要素条件チャンクの識別値を表す型。
    public: typedef template_chunk_key chunk_key;

    /// @brief 要素条件のインデクス番号を表す型。
    public: typedef template_element_index element_index;

    /// @brief 条件式の要素条件を結合する論理演算子を表す列挙型。
    public: enum logic: std::uint8_t
    {
        logic_AND, ///< 論理積。
        logic_OR,  ///< 論理和。
    };

    /// @brief 条件式の種類を表す列挙型。
    public: enum kind: std::uint8_t
    {
        kind_SUB_EXPRESSION,   ///< 複合条件式。
        kind_STATE_TRANSITION, ///< 状態変化条件式。
        kind_STATE_COMPARISON, ///< 状態比較条件式。
    };

    //-------------------------------------------------------------------------
    /** @brief 条件式を構築する。
        @param[in] in_chunk_key      this_type::chunk_key_ の初期値。
        @param[in] in_expression_key this_type::key_ の初期値。
        @param[in] in_logic          this_type::logic_ の初期値。
        @param[in] in_kind           this_type::kind_ の初期値。
        @param[in] in_element_begin  this_type::begin_ の初期値。
        @param[in] in_element_end    this_type::end_ の初期値。
     */
    public: expression(
        typename this_type::chunk_key in_chunk_key,
        typename this_type::key in_expression_key,
        typename this_type::logic const in_logic,
        typename this_type::kind const in_kind,
        typename this_type::element_index const in_element_begin,
        typename this_type::element_index const in_element_end)
    PSYQ_NOEXCEPT:
    chunk_key_(std::move(in_chunk_key)),
    key_(std::move(in_expression_key)),
    begin_(in_element_begin),
    end_(in_element_end),
    logic_(in_logic),
    kind_(in_kind)
    {
        PSYQ_ASSERT(in_element_begin < in_element_end);
    }

    /** @brief 条件式を評価する。
        @param[in] in_elements  評価に用いる要素条件のコンテナ。
        @param[in] in_evaluator 要素条件を評価する関数オブジェクト。
        @retval 正 条件式の評価は真となった。
        @retval 0  条件式の評価は偽となった。
        @retval 負 条件式の評価に失敗した。
     */
    public: template<
        typename template_element_container,
        typename template_element_evaluator>
    psyq::if_then_engine::evaluation evaluate(
        template_element_container const& in_elements,
        template_element_evaluator const& in_evaluator)
    const PSYQ_NOEXCEPT
    {
        if (in_elements.size() <= this->begin_
            || in_elements.size() < this->end_)
        {
            // 条件式が範囲外の要素条件を参照している。
            PSYQ_ASSERT(false);
            return -1;
        }
        auto const local_end(in_elements.begin() + this->end_);
        auto const local_and(this->logic_ == this_type::logic_AND);
        for (auto i(in_elements.begin() + this->begin_); i != local_end; ++i)
        {
            auto const local_evaluation(in_evaluator(*i));
            if (local_evaluation < 0)
            {
                return -1;
            }
            else if (0 < local_evaluation)
            {
                if (!local_and)
                {
                    return 1;
                }
            }
            else if (local_and)
            {
                return 0;
            }
        }
        return local_and;
    }

    //-------------------------------------------------------------------------
    /// @brief 要素条件チャンクに対応する識別値。
    public: typename this_type::chunk_key chunk_key_;
    /// @brief 条件式に対応する識別値。
    public: typename this_type::key key_;
    /// @brief 条件式が使う要素条件の先頭インデクス番号。
    public: typename this_type::element_index begin_;
    /// @brief 条件式が使う要素条件の末尾インデクス番号。
    public: typename this_type::element_index end_;
    /// @brief 条件式の要素条件を結合する論理演算子。
    public: typename this_type::logic logic_;
    /// @brief 条件式の種類。
    public: typename this_type::kind kind_;

}; // class psyq::if_then_engine::_private::expression

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 複合条件式の要素条件。

    @tparam template_expression_key @copydoc psyq::if_then_engine::_private::expression::key
 */
template<typename template_expression_key>
class psyq::if_then_engine::_private::sub_expression
{
    private: typedef sub_expression this_type;

    /** @brief 複合条件式の要素条件を構築する。
        @param[in] in_key       this_type::key_ の初期値。
        @param[in] in_condition this_type::condition の初期値。
     */
    public: sub_expression(
        template_expression_key in_key,
        bool const in_condition)
    PSYQ_NOEXCEPT:
    key_(std::move(in_key)),
    condition_(in_condition)
    {}

    /// @brief 結合する条件式の識別値。
    public: template_expression_key key_;
    /// @brief 結合する際の条件。
    public: bool condition_;

}; // class psyq::if_then_engine::_private::sub_expression

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態変化条件式の要素条件。

    @tparam template_status_key @copydoc psyq::if_then_engine::_private::reservoir::status_key
 */
template<typename template_status_key>
class psyq::if_then_engine::_private::status_transition
{
    private: typedef status_transition this_type;

    /** @brief 状態変化条件式の要素条件を構築する。
        @param[in] in_key this_type::key_ の初期値。
     */
    public: status_transition(template_status_key in_key)
    PSYQ_NOEXCEPT: key_(std::move(in_key))
    {}

    /// @brief 変化を検知する状態値の識別値。
    public: template_status_key key_;

}; // class psyq::if_then_engine::_private::status_transition

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 要素条件チャンク。

    @tparam template_chunk_key                  @copydoc reservoir::chunk_key
    @tparam template_sub_expression_container   @copydoc evaluator::sub_expression_container
    @tparam template_status_transition_container @copydoc evaluator::status_transition_container
    @tparam template_status_comparison_container @copydoc evaluator::status_comparison_container
 */
template<
    typename template_chunk_key,
    typename template_sub_expression_container,
    typename template_status_transition_container,
    typename template_status_comparison_container>
class psyq::if_then_engine::_private::expression_chunk
{
    private: typedef expression_chunk this_type;

    /** @brief チャンクを構築する。
        @param[in] in_key       チャンクの識別値。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: expression_chunk(
        template_chunk_key in_key,
        typename template_sub_expression_container::allocator_type const& in_allocator)
    :
    sub_expressions_(in_allocator),
    status_transitions_(in_allocator),
    status_comparisons_(in_allocator),
    key_(std::move(in_key))
    {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: expression_chunk(this_type&& io_source):
    sub_expressions_(std::move(io_source.sub_expressions_)),
    status_transitions_(std::move(io_source.status_transitions_)),
    status_comparisons_(std::move(io_source.status_comparisons_)),
    key_(std::move(io_source.key_))
    {
        io_source.sub_expressions_.clear();
        io_source.status_comparisons_.clear();
        io_source.status_comparisons_.clear();
    }

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->sub_expressions_ = std::move(io_source.sub_expressions_);
            this->status_transitions_ = std::move(io_source.status_transitions_);
            this->status_comparisons_ = std::move(io_source.status_comparisons_);
            this->key_ = std::move(io_source.key_);
            io_source.sub_expressions_.clear();
            io_source.status_comparisons_.clear();
            io_source.status_comparisons_.clear();
        }
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /// @brief 複合条件式で使う要素条件のコンテナ。
    public: template_sub_expression_container sub_expressions_;
    /// @brief 状態変化条件式で使う要素条件のコンテナ。
    public: template_status_transition_container status_transitions_;
    /// @brief 状態比較条件式で使う要素条件のコンテナ。
    public: template_status_comparison_container status_comparisons_;
    /// @brief この要素条件チャンクに対応する識別値。
    public: template_chunk_key key_;

}; // class psyq::if_then_engine::_private::expression_chunk

#endif // defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_HPP_)
// vim: set expandtab:
