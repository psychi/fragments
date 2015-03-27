/** @file
    @copydoc psyq::scenario_engine::evaluator
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
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 条件評価器。条件式を評価する。

    使い方の概略。
    - evaluator::register_expression で、条件式を登録する。
    - evaluator::evaluate_expression で、条件式を評価する。

    @tparam template_reservoir @copydoc evaluator::reservoir
    @tparam template_key           @copydoc evaluator::expression::key_type
    @tparam template_allocator     @copydoc evaluator::allocator_type
 */
template<
    typename template_reservoir = psyq::scenario_engine::reservoir<>,
    typename template_key = typename template_reservoir::key_type,
    typename template_allocator = typename template_reservoir::allocator_type>
class psyq::scenario_engine::evaluator
{
    /// @brief thisが指す値の型。
    private: typedef evaluator this_type;

    /// @brief 条件評価器で用いる状態貯蔵器の型。
    public: typedef template_reservoir reservoir;

    /// @brief 条件評価器で用いるメモリ割当子の型。
    public: typedef template_allocator allocator_type;

    //-------------------------------------------------------------------------
    /// @brief 条件式。
    public: struct expression
    {
        typedef expression this_type;

        /// @brief 条件式の識別に使う値の型。
        typedef template_key key_type;

        /// @brief 条件式を識別値の昇順で並び替えるのに使う、比較関数オブジェクト。
        typedef psyq::scenario_engine::_private::key_less<
            this_type, typename evaluator::expression::key_type>
                key_less;

        /// @brief 要素条件のインデクス番号を表す型。
        typedef std::uint32_t element_index;

        /// @brief 条件式のコンテナを表す型。
        typedef std::vector<this_type, typename evaluator::allocator_type>
            vector;

        /// @brief 条件式の種類を表す列挙型。
        enum kind_enum: std::uint8_t
        {
            kind_SUB_EXPRESSION,   ///< 複合条件式。
            kind_STATE_COMPARISON, ///< 状態比較条件式。
        };

        /// @brief 条件式の要素条件を結合する論理演算子を表す列挙型。
        enum logic_enum: std::uint8_t
        {
            logic_AND, ///< 論理積。
            logic_OR,  ///< 論理和。
        };

        /// @brief 要素条件チャンクに対応する識別値。
        typename this_type::key_type chunk;
        /// @brief 条件式に対応する識別値。
        typename this_type::key_type key;
        /// @brief 条件式が使う要素条件の先頭インデクス番号。
        typename this_type::element_index begin;
        /// @brief 条件式が使う要素条件の末尾インデクス番号。
        typename this_type::element_index end;
        /// @brief 条件式の種類。
        typename this_type::kind_enum kind;
        /// @brief 条件式の要素条件を結合する論理演算子。
        typename this_type::logic_enum logic;

    }; // struct expression

    //-------------------------------------------------------------------------
    /// @brief 複合条件式の要素条件。
    public: struct sub_expression
    {
        typedef sub_expression this_type;

        /// @brief 複合条件式の要素条件のコンテナ。
        typedef std::vector<this_type, typename evaluator::allocator_type>
            vector;

        /// @brief 結合する条件式の識別値。
        typename evaluator::expression::key_type key;
        /// @brief 結合する際の条件。
        bool condition;

    }; // struct sub_expression

    /// @brief 複合条件式の要素条件を評価する関数オブジェクト。
    private: struct sub_expression_evaluator
    {
        sub_expression_evaluator(
            typename evaluator::reservoir const& in_reservoir,
            evaluator const& in_evaluator)
        PSYQ_NOEXCEPT:
        reservoir(in_reservoir),
        evaluator(in_evaluator)
        {}

        int operator()(
            typename evaluator::sub_expression const& in_sub_expression)
        const PSYQ_NOEXCEPT
        {
            auto const local_evaluate_expression(
                this->evaluator.evaluate_expression(
                    in_sub_expression.key, this->reservoir));
            if (local_evaluate_expression < 0)
            {
                return -1;
            }
            auto const local_condition(local_evaluate_expression != 0);
            return local_condition == in_sub_expression.condition? 1: 0;
        }

        typename evaluator::reservoir const& reservoir;
        evaluator const& evaluator;

    }; // struct sub_expression_evaluator

    //-------------------------------------------------------------------------
    /// @brief 状態比較条件式の要素条件。
    public: struct state_comparison
    {
        typedef state_comparison this_type;

        /// @brief 状態比較条件式の要素条件のコンテナ。
        typedef std::vector<this_type, typename evaluator::allocator_type>
            vector;

        /// @todo 32ビット整数以外も使いたい。
        typedef std::int32_t value_type;

        /// @brief 比較演算子の種類。
        enum operator_enum: std::uint8_t
        {
            operator_EQUAL,         ///< 等価演算子。
            operator_NOT_EQUAL,     ///< 不等価演算子。
            operator_LESS,          ///< 小なり演算子。
            operator_LESS_EQUAL,    ///< 小なりイコール演算子。
            operator_GREATER,       ///< 大なり演算子。
            operator_GREATER_EQUAL, ///< 大なりイコール演算子。
        };

        /// @brief 比較する値。
        typename this_type::value_type value;
        /// @brief 比較する状態値の識別値。
        typename evaluator::reservoir::key_type key;
        /// @brief 比較演算子の種類。
        typename this_type::operator_enum operation;

    }; // struct state_comparison

    /// @brief 状態比較条件式の要素条件を評価する関数オブジェクト。
    private: struct state_comparison_evaluator
    {
        explicit state_comparison_evaluator(
            typename evaluator::reservoir const& in_reservoir)
        PSYQ_NOEXCEPT:
        reservoir(in_reservoir)
        {}

        int operator()(
            typename evaluator::state_comparison const& in_state)
        const PSYQ_NOEXCEPT
        {
            typename evaluator::state_comparison::value_type
                local_value;
            if (this->reservoir.get_state(in_state.key, local_value) != nullptr)
            {
                switch (in_state.operation)
                {
                    case evaluator::state_comparison::operator_EQUAL:
                    return local_value == in_state.value? 1: 0;

                    case evaluator::state_comparison::operator_NOT_EQUAL:
                    return local_value != in_state.value? 1: 0;

                    case evaluator::state_comparison::operator_LESS:
                    return  local_value < in_state.value? 1: 0;

                    case evaluator::state_comparison::operator_LESS_EQUAL:
                    return local_value <= in_state.value? 1: 0;

                    case evaluator::state_comparison::operator_GREATER:
                    return  in_state.value < local_value? 1: 0;

                    case evaluator::state_comparison::operator_GREATER_EQUAL:
                    return in_state.value <= local_value? 1: 0;

                    default:
                    PSYQ_ASSERT(false);
                    break;
                }
            }
            return -1;
        }

        typename evaluator::reservoir const& reservoir;

    }; // struct state_comparison_evaluator

    //-------------------------------------------------------------------------
    /// @brief 要素条件チャンク。
    private: struct chunk
    {
        typedef chunk this_type;

        /// @brief 要素条件チャンクのコンテナ。
        typedef std::vector<this_type, typename evaluator::allocator_type>
             vector;

        /// @brief 要素条件チャンクの識別値を比較する関数オブジェクト。
        typedef psyq::scenario_engine::_private::key_less<
             this_type, typename evaluator::expression::key_type>
                 key_less;

        chunk(
            typename evaluator::expression::key_type in_key,
            typename evaluator::allocator_type const& in_allocator)
        :
        sub_expressions(in_allocator),
        state_comparisons(in_allocator),
        key(std::move(in_key))
        {}

        chunk(chunk&& io_source):
        sub_expressions(std::move(io_source.sub_expressions)),
        state_comparisons(std::move(io_source.state_comparisons)),
        key(std::move(io_source.key))
        {}

        chunk& operator=(chunk&& io_source)
        {
            this->sub_expressions = std::move(io_source.sub_expressions);
            this->state_comparisons = std::move(io_source.state_comparisons);
            this->key = std::move(io_source.key);
            return *this;
        }

        /// @brief 複合条件式で使う要素条件のコンテナ。
        typename evaluator::sub_expression::vector sub_expressions;
        /// @brief 状態比較条件式で使う要素条件のコンテナ。
        typename evaluator::state_comparison::vector state_comparisons;
        /// @brief この要素条件チャンクに対応する識別値。
        typename evaluator::expression::key_type key;

    }; // struct chunk

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 空の条件評価器を構築する。
        @param[in] in_reserve_expressions 予約しておく条件式の数。
        @param[in] in_reserve_chunks      予約しておくチャンクの数。
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
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->expressions_ = std::move(io_source.expressions_);
        this->chunks_ = std::move(io_source.chunks_);
        return *this;
    }

    /// @brief 条件評価器を再構築し、メモリ領域を必要最小限にする。
    public: void shrink_to_fit()
    {
        this->expressions_.shrink_to_fit();
        this->chunks_.shrink_to_fit();
        for (auto& local_chunk: this->chunks_)
        {
            local_chunk.sub_expressions.shrink_to_fit();
            local_chunk.state_comparisons.shrink_to_fit();
        }
    }

    /** @brief 条件評価器で使われているメモリ割当子を取得する。
        @return 条件評価器で使われているメモリ割当子。
     */
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->expressions_.get_allocator();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 条件式
    //@{
    /** @brief 条件式を評価する。
        @param[in] in_expression_key 評価する条件式に対応する識別値。
        @param[in] in_reservoir      評価に用いる状態貯蔵器。
        @retval 正 条件式の評価は true となった。
        @retval  0 条件式の評価は false となった。
        @retval 負 条件式の評価に失敗した。
     */
    public: int evaluate_expression(
        typename this_type::expression::key_type const in_expression_key,
        typename this_type::reservoir const& in_reservoir)
    const PSYQ_NOEXCEPT
    {
        // 条件式の辞書から、該当する条件式を検索する。
        auto const local_expression(this->find_expression(in_expression_key));
        if (local_expression != nullptr)
        {
            auto const local_chunk(this->find_chunk(local_expression->chunk));
            if (local_chunk != nullptr)
            {
                // 条件式の種別によって評価方法を分岐する。
                switch (local_expression->kind)
                {
                    case this_type::expression::kind_SUB_EXPRESSION:
                    return this_type::evaluate_elements(
                        *local_expression,
                        local_chunk->sub_expressions,
                        this_type::sub_expression_evaluator(in_reservoir, *this));

                    case this_type::expression::kind_STATE_COMPARISON:
                    return this_type::evaluate_elements(
                        *local_expression,
                        local_chunk->state_comparisons,
                        this_type::state_comparison_evaluator(in_reservoir));

                    default:
                    // 評価不能な条件式だった。
                    PSYQ_ASSERT(false);
                    break;
                }
            }
            else
            {
                // 要素条件チャンクが見つからなかった。
                PSYQ_ASSERT(false);
            }
        }
        return -1;
    }

    /** @brief 条件式を取得する。
        @param[in] in_expression_key 取得する条件式に対応する識別値。
        @retval !=nullptr 対応する条件式を指すポインタ。
        @retval ==nullptr 対応する条件式が見つからなかった。
     */
    public: typename this_type::expression const* find_expression(
        typename this_type::expression::key_type const in_expression_key)
    const PSYQ_NOEXCEPT
    {
        return this_type::expression::key_less::find_pointer(
            this->expressions_, in_expression_key);
    }

    /** @brief 複合条件式を登録する。
        @param[in] in_elements 登録する複合条件式の要素条件の配列。
        @param[in] in_chunk    登録する複合条件式が所属するチャンクに対応する識別値。
        @param[in] in_key      登録する複合条件式に対応する識別値。
        @param[in] in_logic    登録する複合条件式で用いる論理演算子。
        @retval true  成功。複合条件式を登録した。
        @retval false 失敗。複合条件式は登録されなかった。
     */
    public: bool register_expression(
        typename this_type::sub_expression::vector const& in_elements,
        typename this_type::expression::key_type const& in_chunk,
        typename this_type::expression::key_type in_key,
        typename this_type::expression::logic_enum const in_logic)
    {
        PSYQ_ASSERT(
            this_type::is_valid_sub_expression(in_elements, this->expressions_));
        return this_type::register_expression(
            this->expressions_,
            this_type::equip_chunk(this->chunks_, in_chunk).sub_expressions,
            in_elements,
            in_chunk,
            std::move(in_key),
            in_logic,
            this_type::expression::kind_SUB_EXPRESSION);
    }

    /** @brief 状態比較条件式を登録する。
        @param[in] in_elements  登録する状態比較条件式の要素条件の配列。
        @param[in] in_chunk     登録する状態比較条件式が所属するチャンクに対応する識別値。
        @param[in] in_key       登録する状態比較条件式に対応する識別値。
        @param[in] in_logic     登録する状態比較条件式で用いる論理演算子。
        @param[in] in_reservoir 条件式の評価に用いる状態貯蔵器。
        @retval true  成功。状態比較条件式を登録した。
        @retval false 失敗。状態比較条件式は登録されなかった。
     */
    public: bool register_expression(
        typename this_type::state_comparison::vector const& in_elements,
        typename this_type::expression::key_type const& in_chunk,
        typename this_type::expression::key_type in_key,
        typename this_type::expression::logic_enum const in_logic)
        //typename this_type::reservoir const& in_reservoir)
    {
        //PSYQ_ASSERT(
        //    this_type::is_valid_state_comparison(in_elements, in_reservoir));
        return this_type::register_expression(
            this->expressions_,
            this_type::equip_chunk(this->chunks_, in_chunk).state_comparisons,
            in_elements,
            in_chunk,
            std::move(in_key),
            in_logic,
            this_type::expression::kind_STATE_COMPARISON);
    }
    //@}
    /** @brief 条件式を登録する。
        @param[in,out] io_expressions 条件式を登録するコンテナ。
        @param[in,out] io_elements    要素条件を登録するコンテナ。
        @param[in] in_elements        登録する条件式の要素条件の配列。
        @param[in] in_chunk           登録する状態比較条件式が所属するチャンクに対応する識別値。
        @param[in] in_key             登録する条件式に対応する識別値。
        @param[in] in_logic           登録する条件式で用いる論理演算子。
        @param[in] in_kind            登録する条件式の種類。
        @retval true  成功。条件式を登録した。
        @retval false 失敗。条件式は登録されなかった。
     */
    private: template<typename template_element_container>
    static bool register_expression(
        typename this_type::expression::vector& io_expressions,
        template_element_container& io_elements,
        template_element_container const& in_elements,
        typename this_type::expression::key_type const& in_chunk,
        typename this_type::expression::key_type in_key,
        typename this_type::expression::logic_enum const in_logic,
        typename this_type::expression::kind_enum const in_kind)
    {
        if (in_elements.empty())
        {
            return false;
        }

        // 条件式を追加する。
        auto const local_lower_bound(
            std::lower_bound(
                io_expressions.begin(),
                io_expressions.end(),
                in_key,
                typename this_type::expression::key_less()));
        if (local_lower_bound != io_expressions.end()
            && local_lower_bound->key == in_key)
        {
            return false;
        }
        auto const local_insert(
            io_expressions.insert(
                local_lower_bound, typename this_type::expression()));

        // 要素条件を追加する。
        auto const local_element_begin(io_elements.size());
        io_elements.insert(
            io_elements.end(), in_elements.begin(), in_elements.end());

        // 条件式を初期化する。
        auto& local_expression(*local_insert);
        local_expression.chunk = in_chunk;
        local_expression.key = std::move(in_key);
        local_expression.logic = in_logic;
        local_expression.kind = in_kind;
        local_expression.begin =
            static_cast<typename this_type::expression::element_index>(
                local_element_begin);
        PSYQ_ASSERT(local_expression.begin == local_element_begin);
        auto const local_element_end(io_elements.size());
        local_expression.end =
            static_cast<typename this_type::expression::element_index>(
                local_element_end);
        PSYQ_ASSERT(local_expression.end == local_element_end);
        return true;
    }

    private: static bool is_valid_sub_expression(
        typename this_type::sub_expression::vector const& in_elements,
        typename this_type::expression::vector const& in_expressions)
    {
        for (auto& local_element: in_elements)
        {
            auto const local_find(
                std::binary_search(
                    in_expressions.begin(),
                    in_expressions.end(),
                    local_element.key,
                    typename this_type::expression::key_less()));
            if (!local_find)
            {
                return false;
            }
        }
        return true;
    }

    private: static bool is_valid_state_comparison(
        typename this_type::state_comparison::vector const& in_elements,
        typename this_type::reservoir const& in_reservoir)
    {
        for (auto& local_element: in_elements)
        {
            if (in_reservoir.find_entry(local_element.key) == nullptr)
            {
                return false;
            }
        }
        return true;
    }

    //-------------------------------------------------------------------------
    /// @name 要素条件チャンク
    //@{
    /** @brief 要素条件チャンクを取得する。
        @param[in] in_chunk 取得する要素条件チャンクに対応する識別値。
        @return 要素条件チャンク。
     */
    public: typename this_type::chunk const* find_chunk(
        typename this_type::expression::key_type const& in_chunk)
    const PSYQ_NOEXCEPT
    {
        return this_type::chunk::key_less::find_pointer(this->chunks_, in_chunk);
    }

    /** @brief 要素条件チャンクを予約する。
        @param[in] in_chunk 予約する要素条件チャンクに対応する識別値。
        @param[in] in_reserve_sub_expressions
            予約しておく複合条件式の要素条件数。
        @param[in] in_reserve_state_comparisons
            予約しておく状態比較条件式の要素条件数。
     */
    public: void reserve_chunk(
        typename this_type::expression::key_type const& in_chunk,
        std::size_t const in_reserve_sub_expressions,
        std::size_t const in_reserve_state_comparisons)
    {
        auto& local_chunk(
            this_type::equip_chunk(this->chunks_, in_chunk));
        local_chunk.sub_expressions.reserve(in_reserve_sub_expressions);
        local_chunk.state_comparisons.reserve(in_reserve_state_comparisons);
    }

    /** @brief 要素条件チャンクと、それを使っている条件式を破棄する。
        @param[in] in_chunk 破棄する要素条件チャンクに対応する識別値。
        @todo 未実装。
     */
    public: bool remove_chunk(
        typename this_type::expression::key_type const& in_chunk);
    //@}
    private: static typename this_type::chunk& equip_chunk(
        typename this_type::chunk::vector& io_chunks,
        typename this_type::expression::key_type const& in_chunk_key)
    {
        auto const local_lower_bound(
            std::lower_bound(
                io_chunks.begin(),
                io_chunks.end(),
                in_chunk_key,
                typename this_type::chunk::key_less()));
        if (local_lower_bound != io_chunks.end()
            && local_lower_bound->key == in_chunk_key)
        {
            return *local_lower_bound;
        }
        return *io_chunks.insert(
            local_lower_bound,
            typename this_type::chunk::vector::value_type(
                in_chunk_key, io_chunks.get_allocator()));
    }

    //-------------------------------------------------------------------------
    /** @brief 条件式を評価する。
        @param[in] in_expression 評価する条件式。
        @param[in] in_elements   評価に用いる要素条件の配列。
        @param[in] in_evaluator  要素条件を評価する関数オブジェクト。
        @retval 正 条件式の評価は true となった。
        @retval  0 条件式の評価は false となった。
        @retval 負 条件式の評価に失敗した。
     */
    private: template<
        typename template_container_type,
        typename template_evaluator_type>
    static int evaluate_elements(
        typename this_type::expression const& in_expression,
        template_container_type const& in_elements,
        template_evaluator_type const& in_evaluator)
    PSYQ_NOEXCEPT
    {
        if (in_elements.size() <= in_expression.begin
            || in_elements.size() < in_expression.end)
        {
            // 条件式が範囲外の要素条件を参照している。
            PSYQ_ASSERT(false);
            return -1;
        }
        auto const local_end(in_elements.begin() + in_expression.end);
        auto const local_and(
            in_expression.logic == this_type::expression::logic_AND);
        for (
            auto i(in_elements.begin() + in_expression.begin);
            i != local_end;
            ++i)
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
        return local_and? 1: 0;
    }

    //-------------------------------------------------------------------------
    /// @brief 条件式のコンテナ。
    private: typename this_type::expression::vector expressions_;
    /// @brief 要素条件チャンクのコンテナ。
    private: typename this_type::chunk::vector chunks_;

}; // class psyq::scenario_engine::evaluator

#endif // !defined(PSYQ_SCENARIO_ENGINE_EVALUATOR_HPP_)
// vim: set expandtab:
