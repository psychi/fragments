/** @file
    @copydoc psyq::scenario_engine::state_archive
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_HPP_
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_HPP_

//#include "scenario_engine/state_archive.hpp"

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_TRUE
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_TRUE "TRUE"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_TRUE)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_FALSE
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_FALSE "FALSE"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_FALSE)

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
        template<typename> class state_builder;

        namespace _private
        {
            template<typename template_string>
            int get_bool(template_string const& in_string)
            {
                if (in_string == PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_TRUE)
                {
                    return 1;
                }
                else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_FALSE)
                {
                    return 0;
                }
                return -1;
            }
        } // namespace _private
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

    public: template<typename template_state_archive, typename template_hasher>
    std::size_t operator()(
        template_state_archive& io_states,
        template_hasher& io_hasher,
        typename template_state_archive::key_type const& in_chunk)
    const
    {
        return this_type::build(
            io_states, io_hasher, in_chunk, this->string_table_);
    }

    public: template<typename template_state_archive, typename template_hasher>
    static std::size_t build(
        template_state_archive& io_states,
        template_hasher& io_hasher,
        typename template_state_archive::key_type const& in_chunk,
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
                        io_states, io_hasher, in_chunk, in_string_table, i));
                if (local_register_state)
                {
                    ++local_register_count;
                }
            }
        }
        return local_register_count;
    }

    private: template<typename template_state_archive, typename template_hasher>
    static bool register_state(
        template_state_archive& io_states,
        template_hasher& io_hasher,
        typename template_state_archive::key_type const& in_chunk,
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
            || io_states.is_registered(local_key))
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
            // 真偽値のビット数は必ず1ビットとなるので、ビット数の指定はナシ。
            PSYQ_ASSERT(local_size_cell.empty());
            local_size = 1;
        }
        else if (local_size_cell.empty())
        {
            // ビット数セルがからの場合は、型の種類ごとのデフォルト値を使う。
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
                // 状態値の型の種類が未知のものだった。
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
                // ビット数セルを整数として解析しきれなかった。
                PSYQ_ASSERT(false);
                return false;
            }
        }
        PSYQ_ASSERT(local_size <= template_state_archive::BLOCK_SIZE);

        // 状態値の初期値を取得し、状態値を登録する。
        auto const local_value_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_STATE_BUILDER_CSV_COLUMN_VALUE));
        switch (local_kind)
        {
            case template_state_archive::kind_BOOL:
            return this_type::register_bool(
                io_states, in_chunk, local_key, local_value_cell);

            case template_state_archive::kind_UNSIGNED:
            return this_type::register_unsigned(
                io_states, in_chunk, local_key, local_value_cell, local_size);

            case template_state_archive::kind_SIGNED:
            return this_type::register_signed(
                io_states, in_chunk, local_key, local_value_cell, local_size);

            case template_state_archive::kind_FLOAT:
            return this_type::register_float(
                io_states, in_chunk, local_key, local_value_cell, local_size);

            default:
            PSYQ_ASSERT(false);
            return false;
        }
    }

    private: template<typename template_state_archive>
    static bool register_bool(
        template_state_archive& io_states,
        typename template_state_archive::key_type const& in_chunk,
        typename template_state_archive::key_type const& in_key,
        typename this_type::string_table::string_view const& in_value_cell)
    {
        auto const local_get_bool(
            psyq::scenario_engine::_private::get_bool(in_value_cell));
        if (local_get_bool < 0)
        {
            PSYQ_ASSERT(false);
            return false;
        }
        return io_states.register_bool(in_chunk, in_key, local_get_bool != 0);
    }

    private: template<typename template_state_archive>
    static bool register_unsigned(
        template_state_archive& io_states,
        typename template_state_archive::key_type const& in_chunk,
        typename template_state_archive::key_type const& in_key,
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
        return io_states.register_unsigned(
            in_chunk, in_key, local_value, in_size);
    }

    private: template<typename template_state_archive>
    static bool register_signed(
        template_state_archive& io_states,
        typename template_state_archive::key_type const& in_chunk,
        typename template_state_archive::key_type const& in_key,
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
        return io_states.register_signed(
            in_chunk, in_key, local_value, in_size);
    }

    private: template<typename template_state_archive>
    static bool register_float(
        template_state_archive& io_states,
        typename template_state_archive::key_type const& in_chunk,
        typename template_state_archive::key_type const& in_key,
        typename this_type::string_table::string_view const& in_value_cell,
        std::size_t const in_size)
    {
        std::size_t local_rest_size;
        auto const local_value(
            in_value_cell.template to_real<long double>(
                &local_rest_size));
        if (local_rest_size != 0)
        {
            // 初期値セルを整数として解析しきれなかった。
            PSYQ_ASSERT(false);
            return false;
        }
        /// @todo 未実装。
        return false;
        //return io_states.register_float(in_chunk, in_key, local_value, in_size);
    }

    //-------------------------------------------------------------------------
    private: typename this_type::string_table string_table_;

}; // class psyq::scenario_engine::state_builder

#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_HPP_)
// vim: set expandtab:
