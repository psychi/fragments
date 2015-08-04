/** @file
    @brief @copybrief psyq::if_then_engine::_private::evaluator
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_IF_THEN_ENGINE_EVALUATOR_HPP_
#define PSYQ_IF_THEN_ENGINE_EVALUATOR_HPP_

#include <unordered_map>
#include <vector>
#include "./expression.hpp"

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            template<typename, typename> class evaluator;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 条件評価器。条件式を保持し、評価する。

    ### 使い方の概略
    - evaluator::register_expression で、条件式を登録する。
    - evaluator::evaluate_expression で、条件式を評価する。

    @tparam template_reservoir      @copydoc evaluator::reservoir
    @tparam template_expression_key @copydoc evaluator::expression_key
 */
template<typename template_reservoir, typename template_expression_key>
class psyq::if_then_engine::_private::evaluator
{
    /// @brief thisが指す値の型。
    private: typedef evaluator this_type;

    /** @brief 条件評価器で使う状態貯蔵器の型。

        psyq::if_then_engine::_private::reservoir と互換性があること。
     */
    public: typedef template_reservoir reservoir;

    /// @brief 条件評価器で使うメモリ割当子の型。
    public: typedef
        typename this_type::reservoir::allocator_type
        allocator_type;

    //-------------------------------------------------------------------------
    /// @brief 条件式の識別値。
    public: typedef template_expression_key expression_key;

    /// @brief 条件式
    public: typedef
        psyq::if_then_engine::_private::expression<
            typename template_reservoir::chunk_key, std::uint32_t>
        expression;

    /// @brief 条件式の辞書を表す型。
    private: typedef
        std::unordered_map<
            typename this_type::expression_key,
            typename this_type::expression,
            psyq::integer_hash<typename this_type::expression_key>,
            std::equal_to<typename this_type::expression_key>,
            typename this_type::allocator_type>
        expression_map;

    //-------------------------------------------------------------------------
    /// @brief 複合条件式の要素条件。
    public: typedef
        psyq::if_then_engine::_private::sub_expression<
            typename this_type::expression_key>
        sub_expression;

    /// @brief 複合条件式の要素条件のコンテナ。
    public: typedef
        std::vector<
            typename this_type::sub_expression,
            typename evaluator::allocator_type>
        sub_expression_container;

    //-------------------------------------------------------------------------
    /// @brief 状態変化条件式の要素条件。
    public: typedef
        psyq::if_then_engine::_private::status_transition<
            typename this_type::reservoir::status_key>
        status_transition;

    /// @brief 状態変化条件式の要素条件のコンテナ。
    public: typedef
        std::vector<
            typename this_type::status_transition,
            typename this_type::allocator_type>
        status_transition_container;

    //-------------------------------------------------------------------------
    /// @brief 状態比較条件式の要素条件。
    public: typedef
        typename this_type::reservoir::status_comparison
        status_comparison;

    /// @brief 状態比較条件式の要素条件のコンテナ。
    public: typedef
        std::vector<
            typename this_type::status_comparison,
            typename this_type::allocator_type>
        status_comparison_container;

    //-------------------------------------------------------------------------
    /// @brief 要素条件チャンク。
    public: typedef
        psyq::if_then_engine::_private::expression_chunk<
            typename this_type::sub_expression_container,
            typename this_type::status_transition_container,
            typename this_type::status_comparison_container>
        chunk;

    /// @brief 要素条件チャンクの辞書。
    private: typedef
        std::unordered_map<
            typename this_type::reservoir::chunk_key,
            typename this_type::chunk,
            psyq::integer_hash<typename this_type::reservoir::chunk_key>,
            std::equal_to<typename this_type::reservoir::chunk_key>,
            typename this_type::allocator_type>
        chunk_map;

    //-------------------------------------------------------------------------
    /** @name 構築と代入
        @{
     */
    /** @brief 空の条件評価器を構築する。
        @param[in] in_chunk_buckets      チャンク辞書のバケット数。
        @param[in] in_expression_buckets 条件式辞書のバケット数。
        @param[in] in_allocator          メモリ割当子の初期値。
     */
    public: evaluator(
        std::size_t const in_chunk_buckets,
        std::size_t const in_expression_buckets,
        typename this_type::allocator_type const& in_allocator =
            typename this_type::allocator_type()):
    chunks_(
        in_chunk_buckets,
        typename this_type::chunk_map::hasher(),
        typename this_type::chunk_map::key_equal(),
        in_allocator),
    expressions_(
        in_expression_buckets,
        typename this_type::expression_map::hasher(),
        typename this_type::expression_map::key_equal(),
        in_allocator)
    {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: evaluator(this_type&& io_source):
    chunks_(std::move(io_source.chunks_)),
    expressions_(std::move(io_source.expressions_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->chunks_ = std::move(io_source.chunks_);
        this->expressions_ = std::move(io_source.expressions_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /** @brief 条件評価器で使われているメモリ割当子を取得する。
        @return 条件評価器で使われているメモリ割当子。
     */
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->expressions_.get_allocator();
    }

    /** @brief 条件評価器を再構築する。
        @param[in] in_chunk_buckets      チャンク辞書のバケット数。
        @param[in] in_expression_buckets 条件式辞書のバケット数。
     */
    public: void rebuild(
        std::size_t const in_chunk_buckets,
        std::size_t const in_expression_buckets)
    {
        this->expressions_.rehash(in_expression_buckets);
        this->chunks_.rehash(in_chunk_buckets);
        for (auto& local_chunk: this->chunks_)
        {
            local_chunk.second.sub_expressions_.shrink_to_fit();
            local_chunk.second.status_transitions_.shrink_to_fit();
            local_chunk.second.status_comparisons_.shrink_to_fit();
        }
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 条件式
        @{
     */
    /** @brief 条件式を登録する。

        - this_type::evaluate_expression で、登録した条件式を評価できる。
        - this_type::erase_chunk で、登録した条件式をチャンク単位で削除できる。

        @param[in] in_chunk_key      登録する条件式が所属する要素条件チャンクの識別値。
        @param[in] in_expression_key 登録する条件式の識別値。
        @param[in] in_logic          登録する条件式の論理演算子。
        @param[in] in_elements       登録する条件式の要素条件のコンテナ。
        @retval true  成功。条件式を登録した。
        @retval false 失敗。条件式は登録されなかった。
     */
    public: template<typename template_element_container>
    bool register_expression(
        typename this_type::reservoir::chunk_key const& in_chunk_key,
        typename this_type::expression_key const& in_expression_key,
        typename this_type::expression::logic const in_logic,
        template_element_container const& in_elements)
    {
        if (in_elements.empty())
        {
            return false;
        }
        PSYQ_ASSERT(
            this_type::is_valid_elements(in_elements, this->expressions_));
        if (this->_find_expression(in_expression_key) != nullptr)
        {
            return false;
        }

        // 要素条件の種類を決定し、要素条件を追加する。
        auto& local_chunk(this_type::equip_chunk(this->chunks_, in_chunk_key));
        auto const local_element_kind(
            this_type::make_element_kind(local_chunk, in_elements.front()));
        auto& local_elements(*local_element_kind.second);
        auto const local_element_begin(
            static_cast<typename this_type::expression::element_index>(
                local_elements.size()));
        PSYQ_ASSERT(local_element_begin == local_elements.size());
        local_elements.insert(
            local_elements.end(), in_elements.begin(), in_elements.end());
        auto const local_element_end(
            static_cast<typename this_type::expression::element_index>(
                local_elements.size()));
        PSYQ_ASSERT(local_element_end == local_elements.size());

        // 条件式を辞書に挿入する。
        auto const local_emplace(
            this->expressions_.emplace(
                in_expression_key,
                typename this_type::expression_map::mapped_type(
                    in_chunk_key,
                    in_logic,
                    local_element_kind.first,
                    local_element_begin,
                    local_element_end)));
        PSYQ_ASSERT(local_emplace.second);
        return local_emplace.second;
    }

    /** @brief 登録されている条件式を評価する。

        this_type::register_expression で、あらかじめ条件式を登録しておくこと。

        @param[in] in_expression_key 評価する条件式に対応する識別値。
        @param[in] in_reservoir      評価に用いる状態貯蔵器。
        @retval 正 条件式の評価は真となった。
        @retval 0  条件式の評価は偽となった。
        @retval 負 条件式の評価に失敗した。
     */
    public: psyq::if_then_engine::evaluation evaluate_expression(
        typename this_type::expression_key const in_expression_key,
        typename this_type::reservoir const& in_reservoir)
    const PSYQ_NOEXCEPT
    {
        // 条件式の辞書から、該当する条件式を検索する。
        auto const local_expression(this->_find_expression(in_expression_key));
        if (local_expression == nullptr)
        {
            return -1;
        }
        auto const local_chunk(
            this->_find_chunk(local_expression->get_chunk_key()));
        if (local_chunk == nullptr)
        {
            // 条件式があれば、要素条件チャンクもあるはず。
            PSYQ_ASSERT(false);
            return -1;
        }

        // 条件式の種別によって評価方法を分岐する。
        switch (local_expression->get_kind())
        {
            // 複合条件式を評価する。
            case this_type::expression::kind_SUB_EXPRESSION:
            return local_expression->evaluate(
                local_chunk->sub_expressions_,
                [&, this](
                    typename this_type::sub_expression const& in_expression)
                ->psyq::if_then_engine::evaluation
                {
                    auto const local_evaluate_expression(
                        this->evaluate_expression(
                            in_expression.get_key(), in_reservoir));
                    if (local_evaluate_expression < 0)
                    {
                        return -1;
                    }
                    return in_expression.compare_condition(
                        0 < local_evaluate_expression);
                });

            // 状態変化条件式を評価する。
            case this_type::expression::kind_STATE_TRANSITION:
            return local_expression->evaluate(
                local_chunk->status_transitions_,
                [&](typename this_type::status_transition const& in_transition)
                ->psyq::if_then_engine::evaluation
                {
                    return in_reservoir._get_transition(in_transition.get_key());
                });

            // 状態比較条件式を評価する。
            case this_type::expression::kind_STATE_COMPARISON:
            return local_expression->evaluate(
                local_chunk->status_comparisons_,
                [&](typename this_type::status_comparison const& in_comparison)
                ->psyq::if_then_engine::evaluation
                {
                    return in_reservoir.compare_status(in_comparison);
                });

            // 条件式の種別が未知だった。
            default:
            PSYQ_ASSERT(false);
            return -1;
        }
    }

    /** @brief psyq::if_then_engine 管理者以外は、この関数は使用禁止。

        条件式を取得する。

        @param[in] in_expression_key 取得する条件式に対応する識別値。
        @retval !=nullptr 対応する条件式を指すポインタ。
        @retval ==nullptr 対応する条件式が見つからなかった。
     */
    public: typename this_type::expression const* _find_expression(
        typename this_type::expression_key const& in_expression_key)
    const PSYQ_NOEXCEPT
    {
        auto const local_iterator(this->expressions_.find(in_expression_key));
        return local_iterator != this->expressions_.end()?
            &local_iterator->second: nullptr;
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 要素条件チャンク
        @{
     */
    /** @brief 要素条件チャンクを予約する。
        @param[in] in_chunk_key 予約する要素条件チャンクに対応する識別値。
        @param[in] in_reserve_sub_expressions
            複合条件式の要素条件の予約数。
        @param[in] in_reserve_status_transitions
            状態変更条件式の要素条件の予約数。
        @param[in] in_reserve_status_comparisons
            状態比較条件式の要素条件の予約数。
     */
    public: void reserve_chunk(
        typename this_type::reservoir::chunk_key const& in_chunk_key,
        std::size_t const in_reserve_sub_expressions,
        std::size_t const in_reserve_status_transitions,
        std::size_t const in_reserve_status_comparisons)
    {
        auto& local_chunk(
            this_type::equip_chunk(this->chunks_, in_chunk_key));
        local_chunk.sub_expressions_.reserve(in_reserve_sub_expressions);
        local_chunk.status_transitions_.reserve(in_reserve_status_transitions);
        local_chunk.status_comparisons_.reserve(in_reserve_status_comparisons);
    }

    /** @brief 要素条件チャンクと、それを使っている条件式を破棄する。
        @param[in] in_chunk_key 破棄する要素条件チャンクに対応する識別値。
        @retval true  成功。チャンクを破棄した。
        @retval false 失敗。識別値に対応するチャンクが存在しない。
     */
    public: bool erase_chunk(
        typename this_type::reservoir::chunk_key const& in_chunk_key)
    {
        // 要素条件チャンクを削除する。
        if (this->chunks_.erase(in_chunk_key) == 0)
        {
            return false;
        }

        // 条件式を削除する。
        for (
            auto i(this->expressions_.begin());
            i != this->expressions_.end();)
        {
            if (in_chunk_key != i->second.get_chunk_key())
            {
                ++i;
            }
            else
            {
                i = this->expressions_.erase(i);
            }
        }
        return true;
    }

    /** @brief psyq::if_then_engine 管理者以外は、この関数は使用禁止。

        要素条件チャンクを取得する。

        @param[in] in_chunk_key 取得する要素条件チャンクに対応する識別値。
        @return 要素条件チャンク。
     */
    public: typename this_type::chunk const* _find_chunk(
        typename this_type::reservoir::chunk_key const& in_chunk_key)
    const PSYQ_NOEXCEPT
    {
        auto const local_iterator(this->chunks_.find(in_chunk_key));
        return local_iterator != this->chunks_.end()?
            &local_iterator->second: nullptr;
    }
    /// @}
    //-------------------------------------------------------------------------
    private: static std::pair<
         typename this_type::expression::kind,
         typename this_type::sub_expression_container*>
    make_element_kind(
        typename this_type::chunk& in_chunk,
        typename this_type::sub_expression_container::value_type const&)
    {
        return std::make_pair(
            this_type::expression::kind_SUB_EXPRESSION,
            &in_chunk.sub_expressions_);
    }

    private: static std::pair<
         typename this_type::expression::kind,
         typename this_type::status_transition_container*>
    make_element_kind(
        typename this_type::chunk& in_chunk,
        typename this_type::status_transition_container::value_type const&)
    {
        return std::make_pair(
            this_type::expression::kind_STATE_TRANSITION,
            &in_chunk.status_transitions_);
    }

    private: static std::pair<
         typename this_type::expression::kind,
         typename this_type::status_comparison_container*>
    make_element_kind(
        typename this_type::chunk& in_chunk,
        typename this_type::status_comparison_container::value_type const&)
    {
        return std::make_pair(
            this_type::expression::kind_STATE_COMPARISON,
            &in_chunk.status_comparisons_);
    }

    //-------------------------------------------------------------------------
    private: template<typename template_element_container>
    static bool is_valid_elements(
        template_element_container const& in_elements,
        typename this_type::expression_map const& in_expressions)
    {
        for (auto& local_element: in_elements)
        {
            if (!this_type::is_valid_element(local_element, in_expressions))
            {
                return false;
            }
        }
        return true;
    }

    private: static bool is_valid_element(
        typename this_type::sub_expression const& in_sub_expression,
        typename this_type::expression_map const& in_expressions)
    {
        // 要素条件にある条件式がすでにあることを確認する。
        auto const local_validation(
            in_expressions.find(in_sub_expression.get_key())
            != in_expressions.end());
        PSYQ_ASSERT(local_validation);
        return local_validation;
    }

    private: template<typename template_element>
    static bool is_valid_element(
        template_element const&,
        typename this_type::expression_map const&)
    {
        return true;
    }

    //-------------------------------------------------------------------------
    private: static typename this_type::chunk& equip_chunk(
        typename this_type::chunk_map& io_chunks,
        typename this_type::reservoir::chunk_key const& in_chunk_key)
    {
        auto const local_emplace(
            io_chunks.emplace(
                in_chunk_key,
                typename this_type::chunk_map::mapped_type(
                    io_chunks.get_allocator())));
        return local_emplace.first->second;
    }

    //-------------------------------------------------------------------------
    /// @brief 要素条件チャンクの辞書。
    private: typename this_type::chunk_map chunks_;

    /// @brief 条件式の辞書。
    private: typename this_type::expression_map expressions_;

}; // class psyq::if_then_engine::_private::evaluator

#endif // !defined(PSYQ_IF_THEN_ENGINE_EVALUATOR_HPP_)
// vim: set expandtab:
