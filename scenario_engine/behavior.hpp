/** @file
    @copydoc psyq::scenario_engine::behavior_chunk
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_HPP_
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_HPP_

//#include "scenario_engine/dispatcher.hpp"

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        template<typename> struct behavior_chunk;
        template<typename> struct behavior_builder;
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_KEY
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_KEY "KEY"
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_KEY)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_CONDITION
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_CONDITION "CONDITION"
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_CONDITION)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_KIND
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_KIND "KIND"
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_KIND)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_KIND_STATE
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_KIND_STATE "STATE"
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_KIND_STATE)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_ARGUMENT
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_ARGUMENT "ARGUMENT"
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_ARGUMENT)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_COPY
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_COPY ":="
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_COPY)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_ADD
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_ADD "+="
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_ADD)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_SUB
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_SUB "-="
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_SUB)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_MULT
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_MULT "*="
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_MULT)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_DIV
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_DIV "/="
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_DIV)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_MOD
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_MOD "%="
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_MOD)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_OR
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_OR "|="
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_OR)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_XOR
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_XOR "^="
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_XOR)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_AND
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_AND "&="
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_AND)

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 条件挙動チャンク。
           条件式の評価が変化した際に呼び出す関数オブジェクトを所有する。
 */
template<typename template_dispatcher>
struct psyq::scenario_engine::behavior_chunk
{
    /// @brief thisが指す値の型。
    private: typedef behavior_chunk this_type;

    /// @brief 条件挙動関数オブジェクトの登録先となる条件評価器を表す型。
    public: typedef template_dispatcher dispatcher;

    /// @brief 条件挙動チャンクのコンテナを表す型。
    public: typedef std::vector<
        this_type, typename this_type::dispatcher::allocator_type>
            vector;

    /// @brief 条件挙動チャンクを識別するキーを表す型。
    /// @note ここは条件式キーでなくて、チャンクキーにしないと。
    public: typedef typename this_type::dispatcher::expression_key key_type;

    /// @brief 条件挙動チャンクを識別するキーを比較する関数オブジェクト。
    public: typedef psyq::scenario_engine::_private::key_less<
         this_type, typename this_type::key_type>
             key_less;

    /// @brief 条件挙動関数オブジェクトのコンテナを表す型。
    public: typedef std::vector<
        typename this_type::dispatcher::function_shared_ptr,
        typename this_type::dispatcher::allocator_type>
            function_shared_ptr_vector;

    /// @brief 状態値を操作する演算子の種類。
    public: enum state_operator_enum: std::uint8_t
    {
        state_operator_COPY, ///< 代入。
        state_operator_ADD,  ///< 加算。
        state_operator_SUB,  ///< 減算。
        state_operator_MULT, ///< 乗算。
        state_operator_DIV,  ///< 除算。
        state_operator_MOD,  ///< 除算の余り。
        state_operator_OR,   ///< 論理和。
        state_operator_XOR,  ///< 排他的論理和。
        state_operator_AND,  ///< 論理積。
    };

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: behavior_chunk(this_type&& io_source):
    functions(std::move(io_source.functions)),
    key(std::move(io_source.key))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->functions = std::move(io_source.functions);
        this->key = std::move(io_source.key);
        return *this;
    }
    //@}
    /** @brief 空の条件挙動チャンクを構築する。
        @param[in] in_key       条件挙動チャンクを識別するキー。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    private: behavior_chunk(
        typename this_type::key_type in_key,
        typename this_type::dispatcher::allocator_type const& in_allocator)
    :
    functions(in_allocator),
    key(std::move(in_key))
    {}

    //-------------------------------------------------------------------------
    /// @name 関数オブジェクト
    //@{
    /** @brief 条件挙動チャンクに関数オブジェクトを追加する。
        @param[in,out] io_chunks 関数オブジェクトを追加する条件挙動チャンクのコンテナ。
        @param[in] in_key        関数オブジェクトを追加する条件挙動チャンクのキー。
        @param[in] in_functions  条件挙動チャンクに追加する関数オブジェクトのコンテナ。
     */
    public: static void add(
        typename this_type::vector& io_chunks,
        typename this_type::key_type const& in_key,
        typename this_type::function_shared_ptr_vector in_functions)
    {
        // 関数オブジェクトを追加する条件挙動チャンクを用意する。
        auto local_iterator(
            this_type::key_less::find_iterator(io_chunks, in_key));
        if (local_iterator == io_chunks.end())
        {
            local_iterator = io_chunks.insert(
                local_iterator,
                this_type(in_key, in_functions.get_allocator()));
        }

        // 関数オブジェクトを条件挙動チャンクに追加する。
        auto& local_chunk_functions(
            const_cast<typename this_type::function_shared_ptr_vector&>(
                local_iterator->functions));
        local_chunk_functions.reserve(
            local_chunk_functions.size() + in_functions.size());
        for (auto& local_function: in_functions)
        {
            local_chunk_functions.emplace_back(std::move(local_function));
        }
    }

    /** @brief コンテナから条件挙動チャンクを削除する。
        @param[in,out] io_chunks 条件挙動チャンクを削除するコンテナ。
        @param[in] in_key        削除する条件挙動チャンクのキー。
        @retval true  in_key に対応する条件挙動チャンクを削除した。
        @retval false in_key に対応する条件挙動チャンクがコンテナになかった。
     */
    public: static bool remove(
        typename this_type::vector& io_chunks,
        typename this_type::key_type const& in_key)
    {
        auto const local_iterator(
            this_type::key_less::find_iterator(io_chunks, in_key));
        auto const local_find(local_iterator != io_chunks.end());
        if (local_find)
        {
            io_chunks.erase(local_iterator);
        }
        return local_find;
    }

    /** @brief 状態値を操作する関数オブジェクトを生成する。
        @param[in,out] io_states 関数から参照する状態値書庫。
        @param[in] in_condition  関数の起動条件。
        @param[in] in_key        操作する状態値のキー。
        @param[in] in_operator   状態値の操作で使う演算子。
        @param[in] in_value      状態値の操作で使う演算値。
        @param[in] in_allocator  生成に使うメモリ割当子。
        @return 生成した条件挙動関数オブジェクト。
     */
    public: template<
        typename template_state_archive,
        typename template_value,
        typename template_allocator>
    static typename this_type::dispatcher::function_shared_ptr
    make_state_operation_function(
        template_state_archive& io_states,
        bool const in_condition,
        typename template_state_archive::key_type const& in_key,
        typename this_type::state_operator_enum const in_operator,
        template_value const& in_value,
        template_allocator const& in_allocator)
    {
        // 状態値を書き換える関数オブジェクトを生成する。
        return std::allocate_shared<typename this_type::dispatcher::function>(
            in_allocator,
            typename this_type::dispatcher::function(
                /// @todo io_states を参照渡しするのは危険。対策を考えたい。
                [=, &io_states](
                    typename this_type::dispatcher::expression_key const&,
                    bool const in_evaluation)
                {
                    // 条件と評価が合致すれば、状態値を書き換える。
                    if (in_condition == in_evaluation)
                    {
                        this_type::operate_state(
                            io_states, in_key, in_operator, in_value);
                    }
                }));
    }
    //@}
    private: template<typename template_state_archive, typename template_value>
    static bool operate_state(
        template_state_archive& io_states,
        typename template_state_archive::key_type const& in_key,
        typename this_type::state_operator_enum const in_operator,
        template_value const& in_value)
    {
        if (in_operator == this_type::state_operator_COPY)
        {
            auto const local_set_value(io_states.set_value(in_key, in_value));
            PSYQ_ASSERT(local_set_value);
            return local_set_value;
        }
        template_value local_value;
        if (!io_states.get_value(in_key, local_value))
        {
            PSYQ_ASSERT(false);
            return false;
        }
        switch (in_operator)
        {
            case this_type::state_operator_ADD:  local_value += in_value; break;
            case this_type::state_operator_SUB:  local_value -= in_value; break;
            case this_type::state_operator_MULT: local_value *= in_value; break;
            case this_type::state_operator_OR:   local_value |= in_value; break;
            case this_type::state_operator_XOR:  local_value ^= in_value; break;
            case this_type::state_operator_AND:  local_value &= in_value; break;

            case this_type::state_operator_DIV:
            PSYQ_ASSERT(in_value != 0);
            local_value /= in_value;
            break;

            case this_type::state_operator_MOD:
            PSYQ_ASSERT(in_value != 0);
            local_value %= in_value;
            break;

            default:
            PSYQ_ASSERT(false);
            return false;
        }
        auto const local_set_value(io_states.set_value(in_key, local_value));
        PSYQ_ASSERT(local_set_value);
        return local_set_value;
    }

    //-------------------------------------------------------------------------
    /// @brief 条件挙動関数オブジェクトの所有権ありスマートポインタ。
    public: typename this_type::function_shared_ptr_vector functions;
    /// @brief 条件挙動チャンクを識別するキー。
    public: typename this_type::key_type key;

}; // struct psyq::scenario_engine::behavior_chunk

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_chunk>
struct psyq::scenario_engine::behavior_builder
{
    private: typedef behavior_builder this_type;

    private: typedef template_chunk chunk;

    //-------------------------------------------------------------------------
    /** @brief 文字列表から条件挙動関数オブジェクトを生成し、条件評価器へ登録する。
        @param[in,out] io_dispatcher 生成した条件挙動関数オブジェクトを登録する条件監視器。
        @param[in,out] io_hasher     文字列からキーへ変換するハッシュ関数オブジェクト。
        @param[in] in_evaluator      条件挙動関数から参照する条件評価器。
        @param[in] in_states         条件挙動関数から参照する状態値書庫。
        @param[in] in_string_table   条件挙動の文字列表。
        @return 生成した条件挙動関数オブジェクトのコンテナ。
     */
    public: template<
        typename template_hasher,
        typename template_evaluator,
        typename template_string>
    static typename this_type::chunk::function_shared_ptr_vector build(
        typename this_type::chunk::dispatcher& io_dispatcher,
        template_hasher& io_hasher,
        template_evaluator const& in_evaluator,
        typename template_evaluator::state_archive const& in_states,
        psyq::string::csv_table<template_string> const& in_string_table)
    {
        typename this_type::chunk::function_shared_ptr_vector
            local_functions(io_dispatcher.get_allocator());
        auto const local_row_count(in_string_table.get_row_count());
        local_functions.reserve(local_row_count);

        // 文字列表を解析し、条件挙動関数オブジェクトの一覧を構築する。
        for (
            typename psyq::string::csv_table<template_string>::index_type i(0);
            i < local_row_count;
            ++i)
        {
            if (i == in_string_table.get_attribute_row())
            {
                continue;
            }

            // 条件式キーを取得する。
            auto const local_key_cell(
                in_string_table.find_body_cell(
                    i, PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_KEY));
            auto local_key(io_hasher(local_key_cell));
            if (local_key
                == io_hasher(typename template_hasher::argument_type()))
            {
                // 条件式キーが正しくなかった。
                PSYQ_ASSERT(false);
                continue;
            }
            // 条件評価器に条件式があることを確認する。
            PSYQ_ASSERT(in_evaluator.find_expression(local_key) != nullptr);

            // 条件挙動関数オブジェクトを生成し、条件監視器に登録する。
            auto local_function(
                this_type::make_function(
                    io_hasher, in_evaluator, in_states, in_string_table, i));
            auto const local_register(
                io_dispatcher.register_function(
                    local_key, local_function, in_evaluator, in_states));
            if (local_register)
            {
                local_functions.push_back(std::move(local_function));
            }
            else
            {
                // 条件挙動関数オブジェクトの登録に失敗した。
                PSYQ_ASSERT(false);
            }
        }
        local_functions.shrink_to_fit();
        return local_functions;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表から条件挙動関数オブジェクトを生成する。
        @param[in,out] io_hasher   文字列からキーへ変換するハッシュ関数オブジェクト。
        @param[in] in_evaluator    条件挙動関数から参照する条件評価器。
        @param[in] in_states       条件挙動関数から参照する状態値書庫。
        @param[in] in_string_table 条件挙動の文字列表。
        @param[in] in_row_index    文字列表の行番号。
        @return 生成した条件関数オブジェクト。
     */
    private: template<
         typename template_hasher,
         typename template_evaluator,
         typename template_string>
    static typename this_type::chunk::dispatcher::function_shared_ptr
    make_function(
        template_hasher& io_hasher,
        template_evaluator const& in_evaluator,
        typename template_evaluator::state_archive const& in_states,
        psyq::string::csv_table<template_string> const& in_string_table,
        typename psyq::string::csv_table<template_string>::index_type const
            in_row_index)
    {
        // 挙動が起こる条件を取得する。
        auto const local_condition_cell(
            psyq::scenario_engine::_private::get_bool(
                in_string_table.find_body_cell(
                    in_row_index,
                    PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_CONDITION)));
        if (local_condition_cell < 0)
        {
            // 未知の条件だった。
            PSYQ_ASSERT(false);
            return typename
                this_type::chunk::dispatcher::function_shared_ptr();
        }
        bool const local_condition(local_condition_cell != 0);

        // 条件挙動関数の種類を取得する。
        auto const local_kind_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_KIND));
        if (local_kind_cell
            == PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_KIND_STATE)
        {
            return this_type::make_state_operation_function(
                io_hasher,
                in_states,
                local_condition,
                in_string_table,
                in_row_index);
        }
        else
        {
            // 未知の種類だった。
            PSYQ_ASSERT(false);
            return typename
                this_type::chunk::dispatcher::function_shared_ptr();
        }
    }

    /** @brief 文字列表から状態操作関数オブジェクトを生成する。
        @param[in,out] io_hasher   文字列からキーへ変換するハッシュ関数オブジェクト。
        @param[in] in_states       条件挙動関数から参照する状態値書庫。
        @param[in] in_condition    条件挙動関数を起動する条件。
        @param[in] in_string_table 条件挙動の文字列表。
        @param[in] in_row_index    文字列表の行番号。
        @return 生成した条件関数オブジェクト。
     */
    private: template<
        typename template_hasher,
        typename template_state_archive,
        typename template_string>
    static typename this_type::chunk::dispatcher::function_shared_ptr
    make_state_operation_function(
        template_hasher& io_hasher,
        template_state_archive const& in_states,
        bool const in_condition,
        psyq::string::csv_table<template_string> const& in_string_table,
        typename psyq::string::csv_table<template_string>::index_type const
            in_row_index)
    {
        // 状態キーを取得する。
        auto const local_key_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_ARGUMENT));
        auto const local_key(io_hasher(local_key_cell));
        if (!in_states.is_registered(local_key))
        {
            // 状態値書庫にキーが登録されていなかった。
            PSYQ_ASSERT(false);
            return typename
                this_type::chunk::dispatcher::function_shared_ptr();
        }

        // 演算子を取得する。
        typename this_type::chunk::state_operator_enum local_operator;
        auto const local_get_operator(
            this_type::get_operator(
                local_operator,
                in_string_table.find_body_cell(
                    in_row_index,
                    PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_ARGUMENT,
                    1)));
        if (!local_get_operator)
        {
            PSYQ_ASSERT(false);
            return typename
                this_type::chunk::dispatcher::function_shared_ptr();
        }

        // 演算値を取得する。
        auto const local_value_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_ARGUMENT,
                2));
        /// @note とりあえず整数値のみ対応しておく。
        std::size_t local_rest_size;
        auto const local_value(
            local_value_cell.template to_integer<std::int32_t>(
                &local_rest_size));
        if (local_rest_size != 0)
        {
            PSYQ_ASSERT(false);
            return typename
                this_type::chunk::dispatcher::function_shared_ptr();
        }
        PSYQ_ASSERT(
            local_value != 0 || (
                local_operator != this_type::chunk::state_operator_DIV
                && local_operator != this_type::chunk::state_operator_MOD));

        // 状態値を書き換える関数オブジェクトを生成する。
        return this_type::chunk::make_state_operation_function(
            const_cast<template_state_archive&>(in_states),
            in_condition,
            local_key,
            local_operator,
            local_value,
            in_states.get_allocator());
    }

    private: template<typename template_string>
    static bool get_operator(
        typename this_type::chunk::state_operator_enum& out_operator,
        template_string const& in_string)
    {
        if (in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_COPY)
        {
            out_operator = this_type::chunk::state_operator_COPY;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_ADD)
        {
            out_operator = this_type::chunk::state_operator_ADD;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_SUB)
        {
            out_operator = this_type::chunk::state_operator_SUB;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_MULT)
        {
            out_operator = this_type::chunk::state_operator_MULT;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_DIV)
        {
            out_operator = this_type::chunk::state_operator_DIV;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_MOD)
        {
            out_operator = this_type::chunk::state_operator_MOD;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_OR)
        {
            out_operator = this_type::chunk::state_operator_OR;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_XOR)
        {
            out_operator = this_type::chunk::state_operator_XOR;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_AND)
        {
            out_operator = this_type::chunk::state_operator_AND;
        }
        else
        {
            // 演算子が見つからなかった。
            PSYQ_ASSERT(false);
            return false;
        }
        return true;
    }

}; // struct psyq::scenario_engine::behavior_builder

#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_HPP_)
// vim: set expandtab:
