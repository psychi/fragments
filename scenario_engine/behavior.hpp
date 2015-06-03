/** @file
    @brief @copybrief psyq::scenario_engine::behavior_chunk
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
        template<typename, typename> struct behavior_builder;
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

    /// @brief 条件挙動関数オブジェクトを登録する条件挙動器を表す型。
    public: typedef template_dispatcher dispatcher;

    /// @brief 条件挙動チャンクのコンテナを表す型。
    public: typedef std::vector<
        this_type, typename this_type::dispatcher::allocator_type>
            vector;

    /// @brief 条件挙動チャンクの識別値を表す型。
    /// @note ここは条件式キーでなくて、チャンクキーにしないと。
    public: typedef typename this_type::dispatcher::expression_key key_type;

    /// @brief 条件挙動チャンクの識別値を比較する関数オブジェクト。
    public: typedef psyq::scenario_engine::_private::key_less<
         this_type, typename this_type::key_type>
             key_less;

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
        @param[in] in_key       条件挙動チャンクの識別値。
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
    /** @brief 条件挙動チャンクに関数オブジェクトを追加する。
        @param[in,out] io_chunks 関数オブジェクトを追加する条件挙動チャンクのコンテナ。
        @param[in] in_key        関数オブジェクトを追加する条件挙動チャンクの識別値。
        @param[in] in_function   条件挙動チャンクに追加する関数オブジェクト。
     */
    public: static void add(
        typename this_type::vector& io_chunks,
        typename this_type::key_type const& in_key,
        typename this_type::dispatcher::function_shared_ptr in_function)
    {
        // 関数オブジェクトを条件挙動チャンクに追加する。
        this_type::equip(io_chunks, in_key).functions.push_back(
            std::move(in_function));
    }

    /** @brief 条件挙動チャンクに関数オブジェクトを追加する。
        @param[in,out] io_chunks 関数オブジェクトを追加する条件挙動チャンクのコンテナ。
        @param[in] in_key        関数オブジェクトを追加する条件挙動チャンクの識別値。
        @param[in] in_functions  条件挙動チャンクに追加する関数オブジェクトのコンテナ。
     */
    public: static void add(
        typename this_type::vector& io_chunks,
        typename this_type::key_type const& in_key,
        typename this_type::dispatcher::function_shared_ptr_vector in_functions)
    {
        // 関数オブジェクトを条件挙動チャンクに追加する。
        auto& local_chunk_functions(
            this_type::equip(io_chunks, in_key).functions);
        local_chunk_functions.reserve(
            local_chunk_functions.size() + in_functions.size());
        for (auto& local_function: in_functions)
        {
            local_chunk_functions.push_back(std::move(local_function));
        }
    }

    /** @brief コンテナから条件挙動チャンクを削除する。
        @param[in,out] io_chunks 条件挙動チャンクを削除するコンテナ。
        @param[in] in_key        削除する条件挙動チャンクの識別値。
        @retval true  in_key に対応する条件挙動チャンクを削除した。
        @retval false in_key に対応する条件挙動チャンクがコンテナになかった。
     */
    public: static bool remove(
        typename this_type::vector& io_chunks,
        typename this_type::key_type const& in_key)
    {
        auto const local_iterator(
            this_type::key_less::find_const_iterator(io_chunks, in_key));
        auto const local_find(local_iterator != io_chunks.end());
        if (local_find)
        {
            io_chunks.erase(local_iterator);
        }
        return local_find;
    }

    private: static this_type& equip(
        typename this_type::vector& io_chunks,
        typename this_type::key_type const& in_key)
    {
        // 関数オブジェクトを追加する条件挙動チャンクを用意する。
        auto const local_iterator(
            this_type::key_less::find_iterator(io_chunks, in_key));
        return local_iterator != io_chunks.end()?
            *local_iterator:
            *io_chunks.insert(
                local_iterator, this_type(in_key, io_chunks.get_allocator()));
    }

    //-------------------------------------------------------------------------
    /// @brief 条件挙動関数オブジェクトを所有するコンテナ。
    public: typename this_type::dispatcher::function_shared_ptr_vector functions;
    /// @brief 条件挙動チャンクの識別値。
    public: typename this_type::key_type key;

}; // struct psyq::scenario_engine::behavior_chunk

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 文字列表から条件挙動関数オブジェクトを構築する関数オブジェクト。

    @tparam template_dispatcher @copydoc dispatcher
    @tparam template_string     文字列表で使う文字列の型。
 */
template<typename template_string, typename template_dispatcher>
struct psyq::scenario_engine::behavior_builder
{
    private: typedef behavior_builder this_type;

    /// @brief 条件挙動関数オブジェクトの登録先となる条件挙動器を表す型。
    public: typedef template_dispatcher dispatcher;

    /// @brief 解析する文字列表の型。
    public: typedef psyq::string::csv_table<template_string> string_table;

    /// @brief 文字列表の属性。
    private: struct table_attribute
    {
        table_attribute(string_table const& in_table)
        PSYQ_NOEXCEPT:
        key(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_KEY)),
        condition(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_CONDITION)),
        kind(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_KIND)),
        argument(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_COLUMN_ARGUMENT))
        {}

        bool is_valid() const PSYQ_NOEXCEPT
        {
            return this->key != nullptr
                && this->condition != nullptr
                && this->kind != nullptr
                && this->argument != nullptr;
        }

        typename behavior_builder::string_table::attribute const* key;
        typename behavior_builder::string_table::attribute const* condition;
        typename behavior_builder::string_table::attribute const* kind;
        typename behavior_builder::string_table::attribute const* argument;

    }; // struct table_attribute

    //-------------------------------------------------------------------------
    /** @brief 文字列表から状態値を構築する関数オブジェクトを構築する。
        @param[in] in_table 解析する文字列表。
     */
    public: explicit behavior_builder(
        typename this_type::string_table in_table):
    string_table_(std::move(in_table))
    {}

    /** @brief 文字列表を解析して状態値を構築し、状態貯蔵器へ登録する。
        @param[in,out] io_dispatcher 生成した条件挙動関数オブジェクトを登録する条件挙動器。
        @param[in,out] io_hasher     文字列からキーへ変換するハッシュ関数オブジェクト。
        @param[in] in_evaluator      条件挙動関数から参照する条件評価器。
        @param[in] in_reservoir      条件挙動関数から参照する状態貯蔵器。
        @return 生成した条件挙動関数オブジェクトのコンテナ。
     */
    public: template<typename template_hasher, typename template_evaluator>
    typename this_type::dispatcher::function_shared_ptr_vector operator()(
        typename this_type::dispatcher& io_dispatcher,
        template_hasher& io_hasher,
        template_evaluator const& in_evaluator,
        typename template_evaluator::reservoir const& in_reservoir)
    const
    {
        return this_type::build(
            io_dispatcher,
            io_hasher,
            in_evaluator,
            in_reservoir,
            this->string_table_);
    }

    /** @brief 文字列表から条件挙動関数オブジェクトを生成し、条件評価器へ登録する。
        @param[in,out] io_dispatcher 生成した条件挙動関数オブジェクトを登録する条件挙動器。
        @param[in,out] io_hasher     文字列からキーへ変換するハッシュ関数オブジェクト。
        @param[in] in_evaluator      条件挙動関数から参照する条件評価器。
        @param[in] in_reservoir      条件挙動関数から参照する状態貯蔵器。
        @param[in] in_table          条件挙動の文字列表。
        @return 生成した条件挙動関数オブジェクトのコンテナ。
     */
    public: template<typename template_hasher, typename template_evaluator>
    static typename this_type::dispatcher::function_shared_ptr_vector build(
        typename this_type::dispatcher& io_dispatcher,
        template_hasher& io_hasher,
        template_evaluator const& in_evaluator,
        typename template_evaluator::reservoir const& in_reservoir,
        typename this_type::string_table const& in_table)
    {
        typename this_type::dispatcher::function_shared_ptr_vector
            local_functions(io_dispatcher.get_allocator());

        // 文字列表の属性の桁を取得する。
        typename this_type::table_attribute const local_attribute(in_table);
        if (!local_attribute.is_valid())
        {
            PSYQ_ASSERT(false);
            return local_functions;
        }

        // 文字列表を解析し、条件挙動関数オブジェクトの一覧を構築する。
        auto const local_row_count(in_table.get_row_count());
        local_functions.reserve(local_row_count);
        for (
            typename this_type::string_table::index_type i(0);
            i < local_row_count;
            ++i)
        {
            if (i == in_table.get_attribute_row())
            {
                continue;
            }

            // 条件式キーを取得する。
            auto const local_key_cell(
                in_table.find_body_cell(i, local_attribute.key->column));
            auto local_key(io_hasher(local_key_cell));
            if (local_key
                == io_hasher(typename template_hasher::argument_type()))
            {
                // 条件式キーが正しくなかった。
                PSYQ_ASSERT(false);
                continue;
            }
            // 条件評価器に条件式があることを確認する。
            PSYQ_ASSERT(in_evaluator._find_expression(local_key) != nullptr);

            // 条件挙動関数オブジェクトを生成し、条件監視器に登録する。
            auto local_function(
                this_type::make_function(
                    io_hasher,
                    in_evaluator,
                    in_reservoir,
                    in_table,
                    i,
                    local_attribute));
            auto const local_register_function(
                io_dispatcher.register_function(local_key, local_function));
            if (local_register_function)
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
        @param[in,out] io_hasher 文字列からキーへ変換するハッシュ関数オブジェクト。
        @param[in] in_evaluator  条件挙動関数から参照する条件評価器。
        @param[in] in_reservoir  条件挙動関数から参照する状態貯蔵器。
        @param[in] in_table      条件挙動の文字列表。
        @param[in] in_row_index  文字列表の行番号。
        @param[in] in_attribute  文字列表の属性。
        @return 生成した条件関数オブジェクト。
     */
    private: template<typename template_hasher, typename template_evaluator>
    static typename this_type::dispatcher::function_shared_ptr make_function(
        template_hasher& io_hasher,
        template_evaluator const& in_evaluator,
        typename template_evaluator::reservoir const& in_reservoir,
        typename this_type::string_table const& in_table,
        typename this_type::string_table::index_type const in_row_index,
        typename this_type::table_attribute const& in_attribute)
    {
        // 挙動が起こる条件を取得する。
        auto const local_condition_cell(
            in_table.find_body_cell(
                in_row_index, in_attribute.condition->column));
        auto const local_bool_state(local_condition_cell.to_bool());
        if (local_bool_state < 0)
        {
            // 未知の条件だった。
            PSYQ_ASSERT(false);
            return typename this_type::dispatcher::function_shared_ptr();
        }
        bool const local_condition(local_bool_state != 0);

        // 条件挙動関数の種類を取得する。
        auto const local_kind_cell(
            in_table.find_body_cell(in_row_index, in_attribute.kind->column));
        if (local_kind_cell
            == PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_KIND_STATE)
        {
            return this_type::make_state_operation_function(
                io_hasher,
                in_reservoir,
                local_condition,
                in_table,
                in_row_index,
                in_attribute);
        }
        else
        {
            // 未知の種類だった。
            PSYQ_ASSERT(false);
            return typename this_type::dispatcher::function_shared_ptr();
        }
    }

    /** @brief 文字列表から状態操作関数オブジェクトを生成する。
        @param[in,out] io_hasher 文字列からキーへ変換するハッシュ関数オブジェクト。
        @param[in] in_reservoir  条件挙動関数から参照する状態貯蔵器。
        @param[in] in_condition  条件挙動関数を起動する条件。
        @param[in] in_table      条件挙動の文字列表。
        @param[in] in_row_index  文字列表の行番号。
        @param[in] in_attribute  文字列表の属性。
        @return 生成した条件関数オブジェクト。
     */
    private: template<typename template_hasher, typename template_reservoir>
    static typename this_type::dispatcher::function_shared_ptr
    make_state_operation_function(
        template_hasher& io_hasher,
        template_reservoir const& in_reservoir,
        bool const in_condition,
        typename this_type::string_table const& in_table,
        typename this_type::string_table::index_type const in_row_index,
        typename this_type::table_attribute const& in_attribute)
    {
        PSYQ_ASSERT(2 <= in_attribute.argument->size);

        // 状態キーを取得する。
        auto const local_key_cell(
            in_table.find_body_cell(
                in_row_index, in_attribute.argument->column));
        auto const local_key(io_hasher(local_key_cell));
        if (in_reservoir.get_format(local_key)
            == template_reservoir::state_value::kind_NULL)
        {
            // 状態貯蔵器にキーが登録されていなかった。
            PSYQ_ASSERT(false);
            return typename this_type::dispatcher::function_shared_ptr();
        }

        // 演算子を取得する。
        typename template_reservoir::state_value::operator_enum local_operator;
        auto const local_get_operator(
            this_type::get_operator(
                local_operator,
                in_reservoir,
                in_table.find_body_cell(
                    in_row_index, in_attribute.argument->column + 1)));
        if (!local_get_operator)
        {
            PSYQ_ASSERT(false);
            return typename this_type::dispatcher::function_shared_ptr();
        }

        // 演算値を取得する。
        auto const local_value_cell(
            in_table.find_body_cell(
                in_row_index, in_attribute.argument->column + 2));
        auto const local_value(
            template_reservoir::state_value::make(local_value_cell));
        if (local_value.get_kind()
            == template_reservoir::state_value::kind_NULL)
        {
            PSYQ_ASSERT(false);
            return typename this_type::dispatcher::function_shared_ptr();
        }

        // 状態値を書き換える関数オブジェクトを生成する。
        return this_type::dispatcher::make_state_operation_function(
            const_cast<template_reservoir&>(in_reservoir),
            in_condition,
            local_key,
            local_operator,
            local_value,
            in_reservoir.get_allocator());
    }

    private: template<typename template_reservoir>
    static bool get_operator(
        typename template_reservoir::state_value::operator_enum& out_operator,
        template_reservoir const&,
        typename this_type::string_table::string_view const& in_string)
    {
        if (in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_COPY)
        {
            out_operator = template_reservoir::state_value::operator_COPY;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_ADD)
        {
            out_operator = template_reservoir::state_value::operator_ADD;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_SUB)
        {
            out_operator = template_reservoir::state_value::operator_SUB;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_MULT)
        {
            out_operator = template_reservoir::state_value::operator_MULT;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_DIV)
        {
            out_operator = template_reservoir::state_value::operator_DIV;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_MOD)
        {
            out_operator = template_reservoir::state_value::operator_MOD;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_OR)
        {
            out_operator = template_reservoir::state_value::operator_OR;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_XOR)
        {
            out_operator = template_reservoir::state_value::operator_XOR;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_CSV_OPERATOR_AND)
        {
            out_operator = template_reservoir::state_value::operator_AND;
        }
        else
        {
            // 演算子が見つからなかった。
            PSYQ_ASSERT(false);
            return false;
        }
        return true;
    }

    //-------------------------------------------------------------------------
    /// @brief 解析する文字列表。
    private: typename this_type::string_table string_table_;

}; // struct psyq::scenario_engine::behavior_builder

#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_HPP_)
// vim: set expandtab:
