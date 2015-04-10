/** @file
    @copydoc psyq::scenario_engine::state_builder
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_HPP_
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_HPP_

//#include "scenario_engine/reservoir.hpp"

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KEY
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KEY "KEY"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KEY)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KIND
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KIND "KIND"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KIND)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_VALUE
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_VALUE "VALUE"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_VALUE)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_BOOL
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_BOOL "BOOL"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_BOOL)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_UNSIGNED
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_UNSIGNED "UNSIGNED"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_UNSIGNED)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_SIGNED
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_SIGNED "SIGNED"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_SIGNED)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_FLOAT
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_FLOAT "FLOAT"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_FLOAT)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_INTEGER_SIZE_DEFAULT
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_INTEGER_SIZE_DEFAULT 32
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_INTEGER_SIZE_DEFAULT)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_FLOAT_SIZE_DEFAULT
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_FLOAT_SIZE_DEFAULT 32
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_FLOAT_SIZE_DEFAULT)

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        template<typename> class state_builder;
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 文字列表から状態値を構築する関数オブジェクト。

    driver::add_state_chunk の引数として使う。

    @tparam template_string 文字列表で使う文字列の型。
 */
template<typename template_string>
class psyq::scenario_engine::state_builder
{
    private: typedef state_builder this_type;

    public: typedef psyq::string::csv_table<template_string> string_table;

    //-------------------------------------------------------------------------
    public: explicit state_builder(
        typename this_type::string_table in_string_table):
    string_table_(std::move(in_string_table))
    {}

    public: template<typename template_reservoir, typename template_hasher>
    std::size_t operator()(
        template_reservoir& io_reservoir,
        template_hasher& io_hasher,
        typename template_reservoir::chunk_key const& in_chunk_key)
    const
    {
        return this_type::build(
            io_reservoir, io_hasher, in_chunk_key, this->string_table_);
    }

    public: template<typename template_reservoir, typename template_hasher>
    static std::size_t build(
        template_reservoir& io_reservoir,
        template_hasher& io_hasher,
        typename template_reservoir::chunk_key const& in_chunk_key,
        typename this_type::string_table const& in_string_table)
    {
        // 文字列表を行ごとに解析し、状態値を登録する。
        auto const local_row_count(in_string_table.get_row_count());
        std::size_t local_register_count(0);
        for (
            typename this_type::string_table::index_type i(0);
            i < local_row_count;
            ++i)
        {
            if (i != in_string_table.get_attribute_row())
            {
                auto const local_register_state(
                    this_type::register_state(
                        io_reservoir, io_hasher, in_chunk_key, in_string_table, i));
                if (local_register_state)
                {
                    ++local_register_count;
                }
            }
        }
        return local_register_count;
    }

    private: template<typename template_reservoir, typename template_hasher>
    static bool register_state(
        template_reservoir& io_reservoir,
        template_hasher& io_hasher,
        typename template_reservoir::chunk_key const& in_chunk_key,
        typename this_type::string_table const& in_string_table,
        typename this_type::string_table::index_type const in_row_index)
    {
        // 状態値のキーを取得する。
        auto const local_key_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KEY));
        if (local_key_cell.empty())
        {
            PSYQ_ASSERT(false);
            return false;
        }
        auto local_key(io_hasher(local_key_cell));
        if (local_key == io_hasher(typename template_hasher::argument_type())
            || io_reservoir.get_format(local_key)
               != template_reservoir::state_value::kind_NULL)
        {
            // 条件キーが重複している。
            PSYQ_ASSERT(false);
            return false;
        }

        // 状態値の種類と初期値を取得し、状態値を登録する。
        auto const local_kind_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KIND));
        auto const local_value_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_VALUE));
        if (local_kind_cell
            == PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_BOOL)
        {
            return this_type::register_bool(
                io_reservoir, in_chunk_key, local_key, local_value_cell);
        }
        if (local_kind_cell
            == PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_FLOAT)
        {
            return this_type::register_float(
                io_reservoir, in_chunk_key, local_key, local_value_cell);
        }
        std::size_t const local_default_size(8);
        auto const local_unsigned_size(
            this_type::get_integer_size(
                local_kind_cell,
                PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_UNSIGNED,
                local_default_size));
        if (0 < local_unsigned_size)
        {
            return this_type::register_unsigned(
                io_reservoir,
                in_chunk_key,
                local_key,
                local_value_cell,
                local_unsigned_size);
        }
        auto const local_signed_size(
            this_type::get_integer_size(
                local_kind_cell,
                PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_SIGNED,
                local_default_size));
        if (0 < local_signed_size)
        {
            return this_type::register_signed(
                io_reservoir,
                in_chunk_key,
                local_key,
                local_value_cell,
                local_signed_size);
        }

        PSYQ_ASSERT(false);
        return false;
    }

    private: template<typename template_reservoir>
    static bool register_bool(
        template_reservoir& io_reservoir,
        typename template_reservoir::chunk_key const& in_chunk_key,
        typename template_reservoir::state_key const& in_state_key,
        typename this_type::string_table::string_view const& in_value_cell)
    {
        auto const local_bool_state(in_value_cell.to_bool());
        if (local_bool_state < 0)
        {
            PSYQ_ASSERT(false);
            return false;
        }
        return io_reservoir.register_bool(
            in_chunk_key, in_state_key, local_bool_state != 0);
    }

    private: template<typename template_reservoir>
    static bool register_unsigned(
        template_reservoir& io_reservoir,
        typename template_reservoir::chunk_key const& in_chunk_key,
        typename template_reservoir::state_key const& in_state_key,
        typename this_type::string_table::string_view const& in_value_cell,
        std::size_t const in_size)
    {
        std::size_t local_rest_size;
        auto const local_value(
            in_value_cell.template to_integer<std::uint64_t>(
                &local_rest_size));
        if (local_rest_size != 0)
        {
            // 初期値セルを整数として解析しきれなかった。
            PSYQ_ASSERT(false);
            return false;
        }
        return io_reservoir.register_unsigned(
            in_chunk_key, in_state_key, local_value, in_size);
    }

    private: template<typename template_reservoir>
    static bool register_signed(
        template_reservoir& io_reservoir,
        typename template_reservoir::chunk_key const& in_chunk_key,
        typename template_reservoir::state_key const& in_state_key,
        typename this_type::string_table::string_view const& in_value_cell,
        std::size_t const in_size)
    {
        std::size_t local_rest_size;
        auto const local_value(
            in_value_cell.template to_integer<std::int64_t>(
                &local_rest_size));
        if (local_rest_size != 0)
        {
            // 初期値セルを整数として解析しきれなかった。
            PSYQ_ASSERT(false);
            return false;
        }
        return io_reservoir.register_signed(
            in_chunk_key, in_state_key, local_value, in_size);
    }

    private: template<typename template_reservoir>
    static bool register_float(
        template_reservoir& io_reservoir,
        typename template_reservoir::chunk_key const& in_chunk_key,
        typename template_reservoir::state_key const& in_state_key,
        typename this_type::string_table::string_view const& in_value_cell)
    {
        std::size_t local_rest_size;
        auto const local_value(
            in_value_cell.template
                to_real<typename template_reservoir::state_value::float_type>(
                    &local_rest_size));
        if (local_rest_size != 0)
        {
            // 初期値セルを整数として解析しきれなかった。
            PSYQ_ASSERT(false);
            return false;
        }
        return io_reservoir.register_float(
            in_chunk_key, in_state_key, local_value);
    }

    private: static std::size_t get_integer_size(
        typename this_type::string_table::string_view const& in_cell,
        typename this_type::string_table::string_view const& in_kind,
        std::size_t local_default_size)
    {
        if (in_cell.substr(0, in_kind.size()) == in_kind)
        {
            if (in_cell.size() == in_kind.size())
            {
                return local_default_size;
            }
            if (in_cell.at(in_kind.size()) == '_')
            {
                std::size_t local_rest_size;
                auto const local_size(
                    in_cell.substr(in_kind.size() + 1).template
                        to_integer<std::size_t>(&local_rest_size));
                if (local_rest_size == 0)
                {
                    return local_size;
                }
            }
        }
        return 0;
    }

    //-------------------------------------------------------------------------
    private: typename this_type::string_table string_table_;

}; // class psyq::scenario_engine::state_builder

#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_HPP_)
// vim: set expandtab:
