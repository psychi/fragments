/** @file
    @copydoc psyq::scenario_engine::state_archive
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_HPP_
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_HPP_

//#include "scenario_engine/state_archive.hpp"

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KEY
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KEY "KEY"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KEY)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KIND
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KIND "KIND"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KIND)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_SIZE
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_SIZE "SIZE"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_SIZE)

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
        struct state_builder;
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
struct psyq::scenario_engine::state_builder
{
    private: typedef state_builder this_type;

    public: template<
        typename template_state_archive,
        typename template_hasher,
        typename template_string>
    static std::size_t build(
        template_state_archive& io_states,
        template_hasher& io_hasher,
        typename template_state_archive::key_type const& in_chunk,
        psyq::string::csv_table<template_string> const& in_string_table)
    {
        // 文字列表を行ごとに解析し、状態値を登録する。
        auto const local_row_count(in_string_table.get_row_count());
        std::size_t local_count(0);
        for (
            typename psyq::string::csv_table<template_string>::index_type i(0);
            i < local_row_count;
            ++i)
        {
            if (i != in_string_table.get_attribute_row())
            {
                this_type::build_state(
                    io_states, io_hasher, in_chunk, in_string_table, i);
            }
        }
    }

    private: template<
        typename template_state_archive,
        typename template_hasher,
        typename template_string>
    static bool build_state(
        template_state_archive& io_states,
        template_hasher& io_hasher,
        typename template_state_archive::key_type const& in_chunk,
        psyq::string::csv_table<template_string> const& in_string_table,
        typename psyq::string::csv_table<template_string>::index_type const
            in_row_index)
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
            || io_state.is_registered(local_key))
        {
            // 条件キーが重複している。
            PSYQ_ASSERT(false);
            return false;
        }

        // 状態値の種類を取得する。
        auto const local_kind_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_KIND));
        typename template_state_archive::kind_enum local_kind;
        if (local_kind_cell
            == PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_BOOL)
        {
            local_kind = template_state_archive::kind_BOOL;
        }
        else if (
            local_kind_cell
            == PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_UNSIGNED)
        {
            local_kind = template_state_archive::kind_UNSIGNED;
        }
        else if (
            local_kind_cell
            == PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_SIGNED)
        {
            local_kind = template_state_archive::kind_SIGNED;
        }
        else if (
            local_kind_cell
            == PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_KIND_FLOAT)
        {
            local_kind = template_state_archive::kind_FLOAT;
        }
        else
        {
            PSYQ_ASSERT(false);
            return false;
        }

        // 状態値のビット数を取得する。
        auto const local_size_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_SIZE));
        std::size_t local_size;
        if (local_kind == template_state_archive::kind_BOOL)
        {
            PSYQ_ASSERT(local_size_cell.empty());
            local_size = 1;
        }
        else if (local_size_cell.empty())
        {
            switch (local_kind)
            {
                case template_state_archive::kind_UNSIGNED:
                case template_state_archive::kind_SIGNED:
                local_size =
                    PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_INTEGER_SIZE_DEFAULT;
                break;

                case template_state_archive::kind_FLOAT:
                local_size =
                    PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_FLOAT_SIZE_DEFAULT;
                break;

                default:
                PSYQ_ASSERT(false);
                return false;
            }
        }
        else
        {
            std::size_t local_rest_size;
            local_size = local_size_cell.template to_integer<std::size_t>(
                &local_rest_size);
            if (local_rest_size != 0)
            {
                PSYQ_ASSERT(false);
                return false;
            }
        }

        // 状態値の初期値を取得し、状態値を登録する。
        auto const local_value_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_VALUE));
        switch (local_kind)
        {
            case template_state_archive::kind_BOOL:
            return this_type::register_bool(
                io_state, in_chunk, local_key, local_value_cell);

            case template_state_archive::kind_UNSIGNED:
            return this_type::register_unsigned(
                io_state, in_chunk, local_key, local_value_cell);

            case template_state_archive::kind_SIGNED:
            return this_type::register_signed(
                io_state, in_chunk, local_key, local_value_cell);

            case template_state_archive::kind_FLOAT:
            return this_type::register_float(
                io_state, in_chunk, local_key, local_value_cell);

            default:
            PSYQ_ASSERT(false);
            return false;
        }
    }

    private: template<
        typename template_state_archive,
        typename template_string>
    static bool register_bool(
        template_state_archive& io_states,
        typename template_state_archive::key_type const& in_chunk,
        typename template_state_archive::key_type const& in_key,
        template_string const& in_value)
    {
        auto const local_get_bool(
            psyq::scenario_engine::_private::get_bool(in_value));
        if (local_get_bool < 0)
        {
            PSYQ_ASSERT(false);
            return false;
        }
        return io_state.register_bool(in_chunk, in_key, local_get_bool != 0);
    }

}; // psyq::scenario_engine::state_builder

#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_HPP_)
// vim: set expandtab:
