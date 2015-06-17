/** @file
    @brief @copybrief psyq::scenario_engine::evaluator
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_HPP_
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_HPP_

//#include "scenario_engine/reservoir.hpp"

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        template<typename, typename, typename> class evaluator;

        namespace _private
        {
            template<typename, typename, typename> class expression;
        } // namespace _private
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 条件式。
    @tparam template_key           @copydoc expression::key_type
    @tparam template_chunk_key     @copydoc expression::chunk_key
    @tparam template_element_index @copydoc expression::element_index
 */
template<
    typename template_key,
    typename template_chunk_key,
    typename template_element_index>
class psyq::scenario_engine::_private::expression
{
    /// @brief thisが指す値の型。
    private: typedef expression this_type;

    /// @brief 条件式の識別値を表す型。
    public: typedef template_key key_type;

    /// @brief 要素条件チャンクの識別値を表す型。
    public: typedef template_chunk_key chunk_key;

    /// @brief 要素条件のインデクス番号を表す型。
    public: typedef template_element_index element_index;

    /// @brief 条件式の要素条件を結合する論理演算子を表す列挙型。
    public: enum logic_enum: std::uint8_t
    {
        logic_AND, ///< 論理積。
        logic_OR,  ///< 論理和。
    };

    /// @brief 条件式の種類を表す列挙型。
    public: enum kind_enum: std::uint8_t
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
        typename this_type::key_type in_expression_key,
        typename this_type::logic_enum const in_logic,
        typename this_type::kind_enum const in_kind,
        typename this_type::element_index const in_element_begin,
        typename this_type::element_index const in_element_end)
    PSYQ_NOEXCEPT:
    chunk_key_(std::move(in_chunk_key)),
    key_(std::move(in_expression_key)),
    begin_(in_element_begin),
    end_(in_element_end),
    logic_(in_logic),
    kind_(in_kind)
    {}

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
    std::int8_t evaluate(
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
    public: typename this_type::key_type key_;
    /// @brief 条件式が使う要素条件の先頭インデクス番号。
    public: typename this_type::element_index begin_;
    /// @brief 条件式が使う要素条件の末尾インデクス番号。
    public: typename this_type::element_index end_;
    /// @brief 条件式の要素条件を結合する論理演算子。
    public: typename this_type::logic_enum logic_;
    /// @brief 条件式の種類。
    public: typename this_type::kind_enum kind_;

}; // class psyq::scenario_engine::_private::expression

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief シナリオ条件評価器。条件式を保持して評価する。

    使い方の概略。
    - evaluator::register_expression で、条件式を登録する。
    - evaluator::evaluate_expression で、条件式を評価する。

    @tparam template_reservoir      @copydoc evaluator::reservoir
    @tparam template_expression_key @copydoc evaluator::expression_key
    @tparam template_allocator      @copydoc evaluator::allocator_type
 */
template<
    typename template_reservoir = psyq::scenario_engine::reservoir<>,
    typename template_expression_key = typename template_reservoir::state_key,
    typename template_allocator = typename template_reservoir::allocator_type>
class psyq::scenario_engine::evaluator
{
    /// @brief thisが指す値の型。
    private: typedef evaluator this_type;

    /// @brief 条件評価器で用いる状態貯蔵器の型。
    public: typedef template_reservoir reservoir;

    /// @brief 条件式の識別に使う値の型。
    public: typedef template_expression_key expression_key;

    /// @brief 条件評価器で用いるメモリ割当子の型。
    public: typedef template_allocator allocator_type;

    //-------------------------------------------------------------------------
    /// @brief 条件式
    public: typedef psyq::scenario_engine::_private::expression<
        template_expression_key,
        typename template_reservoir::chunk_key,
        std::uint32_t>
            expression;

    /// @brief 条件式のコンテナを表す型。
    private: typedef std::vector<
        typename this_type::expression, typename this_type::allocator_type>
            expression_vector;

    /// @brief 条件式の識別値のコンテナの型。
    private: typedef std::vector<
        typename this_type::expression_key, typename this_type::allocator_type>
            expression_key_vector;

    /// @brief 条件式を識別値の昇順で並び替えるのに使う、比較関数オブジェクト。
    private: typedef psyq::scenario_engine::_private::key_less<
        typename this_type::expression, typename this_type::expression_key>
            expression_key_less;

    //-------------------------------------------------------------------------
    /// @brief 複合条件式の要素条件。
    public: struct sub_expression
    {
        typedef sub_expression this_type;

        /** @brief 複合条件式の要素条件を構築する。
            @param[in] in_key       this_type::key_ の初期値。
            @param[in] in_condition this_type::condition の初期値。
         */
        sub_expression(
            typename evaluator::expression_key in_key,
            bool const in_condition)
        PSYQ_NOEXCEPT:
        key_(std::move(in_key)),
        condition(in_condition)
        {}

        /// @brief 結合する条件式の識別値。
        typename evaluator::expression_key key_;
        /// @brief 結合する際の条件。
        bool condition;

    }; // struct sub_expression

    /// @brief 複合条件式の要素条件のコンテナ。
    public: typedef std::vector<
        typename this_type::sub_expression,
        typename evaluator::allocator_type>
            sub_expression_vector;

    //-------------------------------------------------------------------------
    /// @brief 状態変化条件式の要素条件。
    public: struct state_transition
    {
        typedef state_transition this_type;

        /** @brief 状態変化条件式の要素条件を構築する。
            @param[in] in_key this_type::key_ の初期値。
         */
        state_transition(typename evaluator::reservoir::state_key in_key)
        PSYQ_NOEXCEPT: key_(std::move(in_key))
        {}

        /// @brief 変化を検知する状態値の識別値。
        typename evaluator::reservoir::state_key key_;

    }; // struct state_transition

    /// @brief 状態変化条件式の要素条件のコンテナ。
    public: typedef std::vector<
        typename this_type::state_transition,
        typename evaluator::allocator_type>
            state_transition_vector;

    //-------------------------------------------------------------------------
    /// @brief 状態比較条件式の要素条件。
    public: struct state_comparison
    {
        typedef state_comparison this_type;

        /** @brief 状態比較条件式の要素条件を構築する。
            @param[in] in_key        this_type::key_ の初期値。
            @param[in] in_comparison this_type::comparison の初期値。
            @param[in] in_value      this_type::value の初期値。
         */
        state_comparison(
            typename evaluator::reservoir::state_key in_key,
            typename evaluator::reservoir::state_value::comparison_enum const
                in_comparison,
            typename evaluator::reservoir::state_value in_value)
        PSYQ_NOEXCEPT:
        value(std::move(in_value)),
        key_(std::move(in_key)),
        comparison(in_comparison)
        {}

        /// @brief 比較の右辺値となる値。
        typename evaluator::reservoir::state_value value;
        /// @brief 比較の左辺値となる状態値の識別値。
        typename evaluator::reservoir::state_key key_;
        /// @brief 比較演算子の種類。
        typename evaluator::reservoir::state_value::comparison_enum comparison;

    }; // struct state_comparison

    /// @brief 状態比較条件式の要素条件のコンテナ。
    public: typedef std::vector<
        typename this_type::state_comparison,
        typename evaluator::allocator_type>
            state_comparison_vector;

    //-------------------------------------------------------------------------
    /// @brief 要素条件チャンク。
    public: struct chunk
    {
        typedef chunk this_type;

        /** @brief チャンクを構築する。
            @param[in] in_key       チャンクの識別値。
            @param[in] in_allocator メモリ割当子の初期値。
         */
        chunk(
            typename evaluator::reservoir::chunk_key in_key,
            typename evaluator::allocator_type const& in_allocator)
        :
        sub_expressions_(in_allocator),
        state_transitions_(in_allocator),
        state_comparisons_(in_allocator),
        key_(std::move(in_key))
        {}

        /** @brief ムーブ構築子。
            @param[in,out] io_source ムーブ元となるインスタンス。
         */
        chunk(this_type&& io_source):
        sub_expressions_(std::move(io_source.sub_expressions_)),
        state_transitions_(std::move(io_source.state_transitions_)),
        state_comparisons_(std::move(io_source.state_comparisons_)),
        key_(std::move(io_source.key_))
        {
            io_source.sub_expressions_.clear();
            io_source.state_comparisons_.clear();
            io_source.state_comparisons_.clear();
        }

        /** @brief ムーブ代入演算子。
            @param[in,out] io_source ムーブ元となるインスタンス。
            @return *this
         */
        this_type& operator=(this_type&& io_source)
        {
            if (this != &io_source)
            {
                this->sub_expressions_ = std::move(io_source.sub_expressions_);
                this->state_transitions_ = std::move(io_source.state_transitions_);
                this->state_comparisons_ = std::move(io_source.state_comparisons_);
                this->key_ = std::move(io_source.key_);
                io_source.sub_expressions_.clear();
                io_source.state_comparisons_.clear();
                io_source.state_comparisons_.clear();
            }
            return *this;
        }

        /// @brief 複合条件式で使う要素条件のコンテナ。
        typename evaluator::sub_expression_vector sub_expressions_;
        /// @brief 状態変化条件式で使う要素条件のコンテナ。
        typename evaluator::state_transition_vector state_transitions_;
        /// @brief 状態比較条件式で使う要素条件のコンテナ。
        typename evaluator::state_comparison_vector state_comparisons_;
        /// @brief この要素条件チャンクに対応する識別値。
        typename evaluator::reservoir::chunk_key key_;

    }; // struct chunk

    /// @brief 要素条件チャンクのコンテナ。
    private: typedef std::vector<
         typename this_type::chunk, typename this_type::allocator_type>
             chunk_vector;

    /// @brief 要素条件チャンクの識別値を比較する関数オブジェクト。
    private: typedef psyq::scenario_engine::_private::key_less<
         typename this_type::chunk, typename this_type::reservoir::chunk_key>
             chunk_key_less;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 空の条件評価器を構築する。
        @param[in] in_reserve_expressions 条件式の予約数。
        @param[in] in_reserve_chunks      要素条件チャンクの予約数。
        @param[in] in_allocator           メモリ割当子の初期値。
     */
    public: evaluator(
        std::size_t const in_reserve_expressions,
        std::size_t const in_reserve_chunks,
        typename this_type::allocator_type const& in_allocator =
            typename this_type::allocator_type())
    :
    expressions_(in_allocator),
    chunks_(in_allocator)
    {
        this->expressions_.reserve(in_reserve_expressions);
        this->chunks_.reserve(in_reserve_chunks);
    }

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: evaluator(this_type&& io_source):
    expressions_(std::move(io_source.expressions_)),
    chunks_(std::move(io_source.chunks_))
    {
        io_source.expressions_.clear();
        io_source.chunks_.clear();
    }

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->expressions_ = std::move(io_source.expressions_);
            this->chunks_ = std::move(io_source.chunks_);
            io_source.expressions_.clear();
            io_source.chunks_.clear();
        }
        return *this;
    }

    /** @brief 条件評価器で使われているメモリ割当子を取得する。
        @return 条件評価器で使われているメモリ割当子。
     */
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->expressions_.get_allocator();
    }

    /// @brief 条件評価器を再構築し、メモリ領域を必要最小限にする。
    public: void shrink_to_fit()
    {
        this->expressions_.shrink_to_fit();
        this->chunks_.shrink_to_fit();
        for (auto& local_chunk: this->chunks_)
        {
            local_chunk.sub_expressions_.shrink_to_fit();
            local_chunk.state_transitions_.shrink_to_fit();
            local_chunk.state_comparisons_.shrink_to_fit();
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 条件式
    //@{
    /** @brief 条件式を登録する。

        - this_type::evaluate_expression で、登録した条件式を評価できる。
        - this_type::remove_chunk で、登録した条件式をチャンク単位で破棄できる。

        @param[in] in_chunk_key      登録する条件式が所属する要素条件チャンクの識別値。
        @param[in] in_expression_key 登録する条件式の識別値。
        @param[in] in_logic          登録する条件式の論理演算子。
        @param[in] in_elements       登録する条件式の要素条件のコンテナ。
        @retval true  成功。条件式を登録した。
        @retval false 失敗。条件式は登録されなかった。
     */
    public: template<typename template_element_container>
    bool register_expression(
        typename this_type::reservoir::chunk_key in_chunk_key,
        typename this_type::expression_key in_expression_key,
        typename this_type::expression::logic_enum const in_logic,
        template_element_container const& in_elements)
    {
        if (in_elements.empty())
        {
            return false;
        }
        PSYQ_ASSERT(
            this_type::is_valid_elements(in_elements, this->expressions_));

        // 条件式を追加する位置を決定する。
        auto const local_lower_bound(
            std::lower_bound(
                this->expressions_.begin(),
                this->expressions_.end(),
                in_expression_key,
                typename this_type::expression_key_less()));
        if (local_lower_bound != this->expressions_.end()
            && local_lower_bound->key_ == in_expression_key)
        {
            return false;
        }

        // 要素条件の種類を決定し、要素条件を追加する。
        auto& local_chunk(this_type::equip_chunk(this->chunks_, in_chunk_key));
        auto const local_element_kind(
            this_type::make_element_kind(local_chunk, in_elements.front()));
        auto& local_elements(*local_element_kind.second);
        auto const local_element_begin(local_elements.size());
        local_elements.insert(
            local_elements.end(), in_elements.begin(), in_elements.end());
        auto const local_element_end(local_elements.size());

        // 条件式を追加する。
        auto const& local_expression(
            *this->expressions_.insert(
                local_lower_bound,
                typename this_type::expression(
                    std::move(in_chunk_key),
                    std::move(in_expression_key),
                    in_logic,
                    local_element_kind.first,
                    static_cast<typename this_type::expression::element_index>(
                        local_element_begin),
                    static_cast<typename this_type::expression::element_index>(
                        local_element_end))));
        PSYQ_ASSERT(
            local_expression.begin_ == local_element_begin
            && local_expression.end_ == local_element_end);
        return true;
    }

    /** @brief 登録されている条件式を評価する。

        this_type::register_expression で、あらかじめ条件式を登録しておくこと。

        @param[in] in_expression_key 評価する条件式に対応する識別値。
        @param[in] in_reservoir      評価に用いる状態貯蔵器。
        @retval 正 条件式の評価は true となった。
        @retval 0  条件式の評価は false となった。
        @retval 負 条件式の評価に失敗した。
     */
    public: std::int8_t evaluate_expression(
        typename this_type::expression_key const in_expression_key,
        typename this_type::reservoir const& in_reservoir)
    const PSYQ_NOEXCEPT
    {
        // 条件式の辞書から、該当する条件式を検索する。
        auto const local_expression(this->_find_expression(in_expression_key));
        if (local_expression != nullptr)
        {
            return -1;
        }
        auto const local_chunk(this->_find_chunk(local_expression->chunk_key_));
        if (local_chunk != nullptr)
        {
            // 条件式があれば、要素条件チャンクもあるはず。
            PSYQ_ASSERT(false);
            return -1;
        }

        // 条件式の種別によって評価方法を分岐する。
        switch (local_expression->kind_)
        {
            // 複合条件式を評価する。
            case this_type::expression::kind_SUB_EXPRESSION:
            return local_expression->evaluate(
                local_chunk->sub_expressions_,
                [&, this](
                    typename this_type::sub_expression const& in_expression)
                ->std::int8_t
                {
                    auto const local_evaluate_expression(
                        this->evaluate_expression(
                            in_expression.key_, in_reservoir));
                    if (local_evaluate_expression < 0)
                    {
                        return -1;
                    }
                    auto const local_condition(0 < local_evaluate_expression);
                    return local_condition == in_expression.condition;
                });

            // 状態変化条件式を評価する。
            case this_type::expression::kind_STATE_TRANSITION:
            return local_expression->evaluate(
                local_chunk->state_transitions_,
                [&](typename this_type::state_transition const& in_state)
                ->std::int8_t
                {
                    return in_reservoir._get_transition(in_state.key_);
                });

            // 状態比較条件式を評価する。
            case this_type::expression::kind_STATE_COMPARISON:
            return local_expression->evaluate(
                local_chunk->state_comparisons_,
                [&](typename this_type::state_comparison const& in_state)
                ->std::int8_t
                {
                    /** @todo
                        今のところ状態値と定数の比較しかできないが、
                        状態値と状態値の比較をできるようにしたい。
                     */
                    return in_reservoir.get_value(in_state.key_).compare(
                        in_state.comparison, in_state.value);
                });

            // 条件式の種別が未知だった。
            default:
            PSYQ_ASSERT(false);
            return -1;
        }
    }

    /** @brief psyq::scenario_engine 管理者以外は、この関数は使用禁止。

        条件式を取得する。

        @param[in] in_expression_key 取得する条件式に対応する識別値。
        @retval !=nullptr 対応する条件式を指すポインタ。
        @retval ==nullptr 対応する条件式が見つからなかった。
     */
    public: typename this_type::expression const* _find_expression(
        typename this_type::expression_key const& in_expression_key)
    const PSYQ_NOEXCEPT
    {
        return this_type::expression_key_less::find_const_pointer(
            this->expressions_, in_expression_key);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 要素条件チャンク
    //@{
    /** @brief 要素条件チャンクを予約する。
        @param[in] in_chunk_key 予約する要素条件チャンクに対応する識別値。
        @param[in] in_reserve_sub_expressions
            複合条件式の要素条件の予約数。
        @param[in] in_reserve_state_comparisons
            状態比較条件式の要素条件の予約数。
     */
    public: void reserve_chunk(
        typename this_type::reservoir::chunk_key const& in_chunk_key,
        std::size_t const in_reserve_sub_expressions,
        std::size_t const in_reserve_state_comparisons)
    {
        auto& local_chunk(
            this_type::equip_chunk(this->chunks_, in_chunk_key));
        local_chunk.sub_expressions_.reserve(in_reserve_sub_expressions);
        local_chunk.state_comparisons_.reserve(in_reserve_state_comparisons);
    }

    /** @brief 要素条件チャンクと、それを使っている条件式を破棄する。
        @param[in] in_chunk_key 破棄する要素条件チャンクに対応する識別値。
        @retval true  成功。チャンクを破棄した。
        @retval false 失敗。識別値に対応するチャンクが存在しない。
     */
    public: bool remove_chunk(
        typename this_type::reservoir::chunk_key const& in_chunk_key)
    {
        // 要素条件チャンクを削除する。
        auto const local_lower_bound(
            std::lower_bound(
                this->chunks_.begin(),
                this->chunks_.end(),
                in_chunk_key,
                typename this_type::chunk_key_less()));
        if (local_lower_bound == this->chunks_.end()
            || local_lower_bound->key_ != in_chunk_key)
        {
            return false;
        }
        this->chunks_.erase(local_lower_bound);

        // 条件式を削除する。
        for (auto i(this->expressions_.begin()); i != this->expressions_.end();)
        {
            if (in_chunk_key != i->chunk_key_)
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

    /** @brief psyq::scenario_engine 管理者以外は、この関数は使用禁止。

        要素条件チャンクを取得する。

        @param[in] in_chunk_key 取得する要素条件チャンクに対応する識別値。
        @return 要素条件チャンク。
     */
    public: typename this_type::chunk const* _find_chunk(
        typename this_type::reservoir::chunk_key const& in_chunk_key)
    const PSYQ_NOEXCEPT
    {
        return this_type::chunk_key_less::find_const_pointer(
            this->chunks_, in_chunk_key);
    }
    //@}
    //-------------------------------------------------------------------------
    private: static std::pair<
         typename this_type::expression::kind_enum,
         typename this_type::sub_expression_vector*>
    make_element_kind(
        typename this_type::chunk& in_chunk,
        typename this_type::sub_expression_vector::value_type const&)
    {
        return std::make_pair(
            this_type::expression::kind_SUB_EXPRESSION,
            &in_chunk.sub_expressions_);
    }

    private: static std::pair<
         typename this_type::expression::kind_enum,
         typename this_type::state_transition_vector*>
    make_element_kind(
        typename this_type::chunk& in_chunk,
        typename this_type::state_transition_vector::value_type const&)
    {
        return std::make_pair(
            this_type::expression::kind_STATE_TRANSITION,
            &in_chunk.state_transitions_);
    }

    private: static std::pair<
         typename this_type::expression::kind_enum,
         typename this_type::state_comparison_vector*>
    make_element_kind(
        typename this_type::chunk& in_chunk,
        typename this_type::state_comparison_vector::value_type const&)
    {
        return std::make_pair(
            this_type::expression::kind_STATE_COMPARISON,
            &in_chunk.state_comparisons_);
    }

    //-------------------------------------------------------------------------
    private: template<typename template_element_container>
    static bool is_valid_elements(
        template_element_container const& in_elements,
        typename this_type::expression_vector const& in_expressions)
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
        typename this_type::sub_expression const& in_element,
        typename this_type::expression_vector const& in_expressions)
    {
        // 要素条件にある条件式がすでにあることを確認する。
        auto const local_validation(
            std::binary_search(
                in_expressions.begin(),
                in_expressions.end(),
                in_element.key_,
                typename this_type::expression_key_less()));
        PSYQ_ASSERT(local_validation);
        return local_validation;
    }

    private: template<typename template_element>
    static bool is_valid_element(
        template_element const&,
        typename this_type::expression_vector const&)
    {
        return true;
    }

    //-------------------------------------------------------------------------
    private: static typename this_type::chunk& equip_chunk(
        typename this_type::chunk_vector& io_chunks,
        typename this_type::reservoir::chunk_key const& in_chunk_key)
    {
        auto const local_lower_bound(
            std::lower_bound(
                io_chunks.begin(),
                io_chunks.end(),
                in_chunk_key,
                typename this_type::chunk_key_less()));
        if (local_lower_bound != io_chunks.end()
            && local_lower_bound->key_ == in_chunk_key)
        {
            return *local_lower_bound;
        }
        return *io_chunks.insert(
            local_lower_bound,
            typename this_type::chunk_vector::value_type(
                in_chunk_key, io_chunks.get_allocator()));
    }

    //-------------------------------------------------------------------------
    /// @brief 条件式の辞書。
    private: typename this_type::expression_vector expressions_;
    /// @brief 要素条件チャンクの辞書。
    private: typename this_type::chunk_vector chunks_;

}; // class psyq::scenario_engine::evaluator

#endif // !defined(PSYQ_SCENARIO_ENGINE_EVALUATOR_HPP_)
// vim: set expandtab:
