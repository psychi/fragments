/// @file
/// @brief @copybrief psyq::if_then_engine::_private::evaluator
/// @author Hillco Psychi (https://twitter.com/psychi)
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
/// @brief 条件評価器。条件式を保持し、評価する。
/// @par 使い方の概略
/// - evaluator::register_expression で、条件式を登録する。
/// - evaluator::evaluate_expression で、条件式を評価する。
/// @tparam template_reservoir      @copydoc evaluator::reservoir
/// @tparam template_expression_key @copydoc evaluator::expression_key
template<typename template_reservoir, typename template_expression_key>
class psyq::if_then_engine::_private::evaluator
{
    /// @brief thisが指す値の型。
    private: typedef evaluator this_type;

    //-------------------------------------------------------------------------
    /// @brief 条件評価器で使う状態貯蔵器の型。
    /// @details psyq::if_then_engine::_private::reservoir と互換性があること。
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
    /// @copydoc psyq::if_then_engine::evaluation
    public: typedef psyq::if_then_engine::evaluation evaluation;

    //-------------------------------------------------------------------------
    /// @brief 複合条件式の要素条件のコンテナ。
    public: typedef
        std::vector<
            psyq::if_then_engine::_private::sub_expression<
                typename this_type::expression_key>,
            typename evaluator::allocator_type>
        sub_expression_container;
    /// @brief 状態変化条件式の要素条件のコンテナ。
    public: typedef
        std::vector<
            psyq::if_then_engine::_private::status_transition<
                typename this_type::reservoir::status_key>,
            typename this_type::allocator_type>
        status_transition_container;
    /// @brief 状態比較条件式の要素条件のコンテナ。
    public: typedef
        std::vector<
            typename this_type::reservoir::status_comparison,
            typename this_type::allocator_type>
        status_comparison_container;
    /// @brief 要素条件チャンク。
    public: typedef
        psyq::if_then_engine::_private::expression_chunk<
            typename this_type::sub_expression_container,
            typename this_type::status_transition_container,
            typename this_type::status_comparison_container>
        chunk;
    /// @brief 要素条件チャンクの識別値。
    public: typedef typename this_type::reservoir::chunk_key chunk_key;

    //-------------------------------------------------------------------------
    /// @brief 条件式の辞書を表す型。
    private: typedef
        std::unordered_map<
            typename this_type::expression_key,
            typename this_type::expression,
            psyq::integer_hash<typename this_type::expression_key>,
            std::equal_to<typename this_type::expression_key>,
            typename this_type::allocator_type>
        expression_map;
    /// @brief 要素条件チャンクの辞書。
    private: typedef
        std::unordered_map<
            typename this_type::chunk_key,
            typename this_type::chunk,
            psyq::integer_hash<typename this_type::chunk_key>,
            std::equal_to<typename this_type::chunk_key>,
            typename this_type::allocator_type>
        chunk_map;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    /// @{

    /// @brief 空の条件評価器を構築する。
    public: evaluator(
        /// [in] チャンク辞書のバケット数。
        std::size_t const in_chunk_count,
        ///[in] 条件式辞書のバケット数。
        std::size_t const in_expression_count,
        /// [in] メモリ割当子の初期値。
        typename this_type::allocator_type const& in_allocator =
            typename this_type::allocator_type()):
    chunks_(
        in_chunk_count,
        typename this_type::chunk_map::hasher(),
        typename this_type::chunk_map::key_equal(),
        in_allocator),
    expressions_(
        in_expression_count,
        typename this_type::expression_map::hasher(),
        typename this_type::expression_map::key_equal(),
        in_allocator)
    {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /// @brief ムーブ構築子。
    public: evaluator(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source):
    chunks_(std::move(io_source.chunks_)),
    expressions_(std::move(io_source.expressions_))
    {}

    /// @brief ムーブ代入演算子。
    /// @return *this
    public: this_type& operator=(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source)
    {
        this->chunks_ = std::move(io_source.chunks_);
        this->expressions_ = std::move(io_source.expressions_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /// @brief 条件評価器で使われているメモリ割当子を取得する。
    /// @return 条件評価器で使われているメモリ割当子。
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->expressions_.get_allocator();
    }

    /// @brief 条件評価器を再構築する。
    public: void rebuild(
        /// [in] チャンク辞書のバケット数。
        std::size_t const in_chunk_count,
        /// [in] 条件式辞書のバケット数。
        std::size_t const in_expression_count)
    {
        this->expressions_.rehash(in_expression_count);
        this->chunks_.rehash(in_chunk_count);
        for (auto& local_chunk: this->chunks_)
        {
            local_chunk.second.sub_expressions_.shrink_to_fit();
            local_chunk.second.status_transitions_.shrink_to_fit();
            local_chunk.second.status_comparisons_.shrink_to_fit();
        }
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 条件式
    /// @{

    /// @brief 条件式を登録する。
    /// @par
    /// - this_type::evaluate_expression で、登録した条件式を評価できる。
    /// - this_type::erase_chunk で、登録した条件式をチャンク単位で削除できる。
    /// @return
    /// 登録した条件式を指すポインタ。登録に失敗した場合は nullptr を返す。
    /// - in_expression_key に対応する条件式が既にあると失敗する。
    /// - in_elements が空だと失敗する。
    public: template<typename template_element_container>
    typename this_type::expression const* register_expression(
        /// [in] 登録する条件式が所属する要素条件チャンクの識別値。
        typename this_type::chunk_key const& in_chunk_key,
        /// [in] 登録する条件式の識別値。
        typename this_type::expression_key const& in_expression_key,
        /// [in] 登録する条件式の論理演算子。
        typename this_type::expression::logic const in_logic,
        /// [in] 登録する条件式の要素条件のコンテナ。
        template_element_container const& in_elements)
    {
        auto const local_elements_begin(std::begin(in_elements));
        auto const local_elements_end(std::end(in_elements));
        PSYQ_ASSERT(
            this_type::is_valid_elements(
                local_elements_begin, local_elements_end, this->expressions_));
        if (local_elements_begin == local_elements_end
            || this->find_expression(in_expression_key) != nullptr)
        {
            return nullptr;
        }

        // 要素条件の種類を決定し、要素条件を追加する。
        auto const local_emplace_chunk(
            this->chunks_.emplace(
                in_chunk_key,
                typename this_type::chunk_map::mapped_type(
                    this->chunks_.get_allocator())));
        auto const local_element_kind(
            this_type::make_element_kind(
                local_emplace_chunk.first->second, *local_elements_begin));
        auto& local_elements(*local_element_kind.second);
        auto const local_begin_index(
            static_cast<typename this_type::expression::element_index>(
                local_elements.size()));
        PSYQ_ASSERT(local_begin_index == local_elements.size());
        local_elements.insert(
            local_elements.end(), local_elements_begin, local_elements_end);
        auto const local_end_index(
            static_cast<typename this_type::expression::element_index>(
                local_elements.size()));
        PSYQ_ASSERT(local_end_index == local_elements.size());

        // 条件式を辞書に挿入する。
        auto const local_emplace(
            this->expressions_.emplace(
                in_expression_key,
                typename this_type::expression_map::mapped_type(
                    in_chunk_key,
                    in_logic,
                    local_element_kind.first,
                    local_begin_index,
                    local_end_index)));
        if (local_emplace.second)
        {
            return &local_emplace.first->second;
        }
        PSYQ_ASSERT(false);
        return nullptr;
    }

    /// @brief 状態値を比較する条件式を登録する。
    /// @return
    /// 登録した条件式を指すポインタ。登録に失敗した場合は nullptr を返す。
    /// - in_expression_key に対応する条件式が既にあると失敗する。
    /// - in_comparison.get_key() に対応する状態値が
    ///   in_reservoir にないと失敗する。
    public: typename this_type::expression const* register_expression(
        /// [in] 条件式が参照する状態貯蔵器。
        typename this_type::reservoir const& in_reservoir,
        /// [in] 登録する条件式の識別値。
        typename this_type::expression_key const& in_expression_key,
        /// [in] 登録する状態比較要素条件。
        typename this_type::reservoir::status_comparison const& in_comparison)
    {
        auto const local_status_property(
            in_reservoir.find_status(in_comparison.get_key()));
        if (local_status_property == nullptr)
        {
            return nullptr;
        }
        typename this_type::reservoir::status_comparison const
            local_comparisons[1] = {in_comparison};
        return this->register_expression(
            local_status_property->get_chunk_key(),
            in_expression_key,
            this_type::expression::logic_AND,
            local_comparisons);
    }

    /// @brief 論理型の状態値を比較する条件式を登録する。
    /// @return
    /// 登録した条件式を指すポインタ。登録に失敗した場合は nullptr を返す。
    /// - in_expression_key に対応する条件式が既にあると失敗する。
    /// - in_status_key に対応する状態値が論理型以外だと失敗する。
    public: typename this_type::expression const* register_expression(
        /// [in] 条件式が参照する状態貯蔵器。
        typename this_type::reservoir const& in_reservoir,
        /// [in] 登録する条件式の識別値。
        typename this_type::expression_key const& in_expression_key,
        /// [in] 評価する論理型の状態値の識別値。
        typename this_type::reservoir::status_key const& in_status_key,
        /// [in] 条件となる論理値。
        bool const in_condition)
    {
        if (in_reservoir.find_kind(in_status_key) !=
                this_type::reservoir::status_value::kind_BOOL)
        {
            return nullptr;
        }
        return this->register_expression(
            in_reservoir,
            in_expression_key,
            typename this_type::reservoir::status_comparison(
                in_status_key,
                in_condition?
                    this_type::reservoir::status_value::comparison_NOT_EQUAL:
                    this_type::reservoir::status_value::comparison_EQUAL,
                typename this_type::reservoir::status_value(false)));
    }

    /// @brief 条件式を取得する。
    /// @return
    /// in_expression_key に対応する条件式を指すポインタ。
    /// 該当する条件式がない場合は nullptr を返す。
    public: typename this_type::expression const* find_expression(
        /// [in] 取得する条件式に対応する識別値。
        typename this_type::expression_key const& in_expression_key)
    const
    {
        auto const local_find(this->expressions_.find(in_expression_key));
        return local_find != this->expressions_.end()?
            &local_find->second: nullptr;
    }

    /// @brief 登録されている条件式を評価する。
    /// @retval 正 条件式の評価は真となった。
    /// @retval 0  条件式の評価は偽となった。
    /// @retval 負 条件式の評価に失敗した。
    /// - 条件式が登録されていないと、失敗する。
    /// - 条件式が参照する状態値が登録されていないと、失敗する。
    /// @sa this_type::register_expression で条件式を登録できる。
    public: typename this_type::evaluation evaluate_expression(
        /// [in] 評価する条件式に対応する識別値。
        typename this_type::expression_key const in_expression_key,
        /// [in] 条件式が参照する状態貯蔵器。
        typename this_type::reservoir const& in_reservoir)
    const
    {
        // 条件式の辞書から、該当する条件式を検索する。
        auto const local_expression(this->find_expression(in_expression_key));
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
                [&in_reservoir, this](
                    typename this_type::sub_expression_container::value_type
                        const& in_expression)
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
                [&in_reservoir](
                    typename this_type::status_transition_container::value_type
                        const& in_transition)
                ->psyq::if_then_engine::evaluation
                {
                    return in_reservoir.find_transition(in_transition.get_key());
                });

            // 状態比較条件式を評価する。
            case this_type::expression::kind_STATE_COMPARISON:
            return local_expression->evaluate(
                local_chunk->status_comparisons_,
                [&in_reservoir](
                    typename this_type::status_comparison_container::value_type
                        const& in_comparison)
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
    /// @}
    //-------------------------------------------------------------------------
    /// @name 要素条件チャンク
    /// @{

    /// @brief 要素条件チャンクが登録されているか判定する。
    /// @retval true
    /// in_chunk_key に対応する要素条件チャンクが登録されている。
    /// @retval false 該当する要素条件チャンクがない。
    public: bool is_chunk_exist(
        /// [in] 判定する要素条件チャンクに対応する識別値。
        typename this_type::chunk_key const& in_chunk_key)
    const
    {
        return this->chunks_.find(in_chunk_key) != this->chunks_.end();
    }

    /// @brief 要素条件チャンクを予約する。
    public: void reserve_chunk(
        /// [in] 予約する要素条件チャンクに対応する識別値。
        typename this_type::chunk_key const& in_chunk_key,
        /// [in] 複合条件式の要素条件の予約数。
        std::size_t const in_sub_expression_capacity,
        /// [in] 状態変更条件式の要素条件の予約数。
        std::size_t const in_status_transition_capacity,
        /// [in] 状態比較条件式の要素条件の予約数。
        std::size_t const in_status_comparison_capacity)
    {
        this_type::reserve_chunk(
            this->chunks_,
            in_chunk_key,
            in_sub_expression_capacity,
            in_status_transition_capacity,
            in_status_comparison_capacity);
    }

    /// @brief 要素条件チャンクと、それを使っている条件式を破棄する。
    /// @retval true  成功。チャンクを破棄した。
    /// @retval false 失敗。 in_chunk_key に対応するチャンクがない。
    public: bool erase_chunk(
        /// [in] 破棄する要素条件チャンクに対応する識別値。
        typename this_type::chunk_key const& in_chunk_key)
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

    /// @brief 要素条件チャンクを取得する。
    /// @warning psyq::if_then_engine 管理者以外は、この関数は使用禁止。
    /// @return
    /// in_chunk_key に対応する要素条件チャンクを指すポインタ。
    /// 該当する要素条件チャンクがない場合は nullptr を返す。
    public: typename this_type::chunk const* _find_chunk(
        /// [in] 取得する要素条件チャンクに対応する識別値。
        typename this_type::chunk_key const& in_chunk_key)
    const
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
    private: template<typename template_element_iterator>
    static bool is_valid_elements(
        template_element_iterator const& in_elements_begin,
        template_element_iterator const& in_elements_end,
        typename this_type::expression_map const& in_expressions)
    {
        for (auto i(in_elements_begin); i != in_elements_end; ++i)
        {
            if (!this_type::is_valid_element(*i, in_expressions))
            {
                return false;
            }
        }
        return true;
    }

    private: static bool is_valid_element(
        typename this_type::sub_expression_container::value_type const&
            in_sub_expression,
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
    /// @brief 要素条件チャンクを予約する。
    private: static typename this_type::chunk& reserve_chunk(
        /// [in] 予約する要素条件チャンクの辞書。
        typename this_type::chunk_map& io_chunks,
        /// [in] 予約するチャンクに対応する識別値。
        typename this_type::chunk_key const& in_chunk_key,
        /// [in] 複合条件式の要素条件の予約数。
        std::size_t const in_sub_expression_capacity,
        /// [in] 状態変更条件式の要素条件の予約数。
        std::size_t const in_status_transition_capacity,
        /// [in] 状態比較条件式の要素条件の予約数。
        std::size_t const in_status_comparison_capacity)
    {
        auto const local_emplace(
            io_chunks.emplace(
                in_chunk_key,
                typename this_type::chunk_map::mapped_type(
                    io_chunks.get_allocator())));
        auto& local_chunk(local_emplace.first->second);
        local_chunk.sub_expressions_.reserve(in_sub_expression_capacity);
        local_chunk.status_transitions_.reserve(in_status_transition_capacity);
        local_chunk.status_comparisons_.reserve(in_status_comparison_capacity);
        return local_chunk;
    }

    //-------------------------------------------------------------------------
    /// @brief 要素条件チャンクの辞書。
    private: typename this_type::chunk_map chunks_;
    /// @brief 条件式の辞書。
    private: typename this_type::expression_map expressions_;

}; // class psyq::if_then_engine::_private::evaluator

#endif // !defined(PSYQ_IF_THEN_ENGINE_EVALUATOR_HPP_)
// vim: set expandtab:
