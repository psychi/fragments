/// @file
/// @brief @copybrief psyq::if_then_engine::status_builder
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_HPP_
#define PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_HPP_

#include "../string/numeric_parser.hpp"

/// @brief 文字列表で、状態値の識別値が記述されている属性の名前。
/// @details
/// psyq::if_then_engine::status_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::reservoir::status_key
/// の初期値として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KEY
#define PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KEY "KEY"
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KEY)

/// @brief 文字列表で、状態値の種別が記述されている属性の名前。
/// @details
/// psyq::if_then_engine::status_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::reservoir::status_value::kind
/// の初期値として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KIND
#define PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KIND "KIND"
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KIND)

/// @brief 文字列表で、状態値の初期値が記述されている属性の名前。
/// @details
/// psyq::if_then_engine::status_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::reservoir::status_value
/// の初期値として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_VALUE
#define PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_VALUE "VALUE"
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_VALUE)

/// @brief 文字列表で、状態値の種別が論理型と対応する文字列。
/// @details
/// psyq::if_then_engine::status_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::reservoir::status_value::kind_BOOL
/// として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_BOOL
#define PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_BOOL "BOOL"
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_BOOL)

/// @brief 文字列表で、状態値の種別が符号なし整数型と対応する文字列。
/// @details
/// psyq::if_then_engine::status_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::reservoir::status_value::kind_UNSIGNED
/// として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_UNSIGNED
#define PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_UNSIGNED "UNSIGNED"
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_UNSIGNED)

/// @brief 文字列表で、状態値の種別が符号あり整数型と対応する文字列。
/// @details
/// psyq::if_then_engine::status_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::reservoir::status_value::kind_SIGNED
/// として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_SIGNED
#define PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_SIGNED "SIGNED"
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_SIGNED)

/// @brief 文字列表で、状態値の種別が浮動小数点数型と対応する文字列。
/// @details
/// psyq::if_then_engine::status_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::reservoir::status_value::kind_FLOAT
/// として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_FLOAT
#define PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_FLOAT "FLOAT"
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_FLOAT)

/// 整数型のデフォルトのビット幅。
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_INTEGER_WIDTH_DEFAULT
#define PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_INTEGER_WIDTH_DEFAULT 8
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_INTEGER_WIDTH_DEFAULT)

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        class status_builder;
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 文字列表から状態値を構築する関数オブジェクト。
/// @details  driver::extend_chunk の引数として使う。
class psyq::if_then_engine::status_builder
{
    /// @brief this が指す値の型。
    private: typedef status_builder this_type;

    //-------------------------------------------------------------------------
    /// @brief 状態値の文字列表で使う属性。
    private: template<typename template_relation_table>
    class table_attribute
    {
        public: explicit table_attribute(
            template_relation_table const& in_table)
        PSYQ_NOEXCEPT:
        key_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KEY)),
        kind_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KIND)),
        value_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_VALUE))
        {}

        public: bool is_valid() const PSYQ_NOEXCEPT
        {
            return 1 <= this->key_.second
                && 1 <= this->kind_.second
                && 1 <= this->value_.second;
        }

        /// @brief 文字列表で状態値の識別値が記述されている属性の列番号と列数。
        public: typename template_relation_table::attribute key_;
        /// @brief 文字列表で状態値の種別が記述されている属性の列番号と列数。
        public: typename template_relation_table::attribute kind_;
        /// @brief 文字列表で状態値の初期値が記述されている属性の列番号と列数。
        public: typename template_relation_table::attribute value_;

    }; // class table_attribute

    //-------------------------------------------------------------------------
    /// @brief 文字列表を解析して状態値を構築し、状態貯蔵器へ登録する。
    /// @return 登録した状態値の数。
    public: template<
        typename template_reservoir,
        typename template_hasher,
        typename template_relation_table>
    std::size_t operator()(
        /// [in,out] 状態値を登録する driver::reservoir 。
        template_reservoir& io_reservoir,
        /// [in,out] 文字列からハッシュ値を作る driver::hasher 。
        template_hasher& io_hasher,
        /// [in] 状態値を登録するチャンクの識別値。
        typename template_reservoir::chunk_key const& in_chunk_key,
        /// [in] 状態値が記述されている psyq::string::relation_table 。
        /// 空の場合は、状態値は登録されない。
        template_relation_table const& in_table)
    const
    {
        return this_type::register_statuses(
            io_reservoir, io_hasher, in_chunk_key, in_table);
    }

    /// @brief 文字列表を解析して状態値を構築し、状態貯蔵器へ登録する。
    /// @return 登録した状態値の数。
    public: template<
        typename template_reservoir,
        typename template_hasher,
        typename template_relation_table>
    static std::size_t register_statuses(
        /// [in,out] 状態値を登録する driver::reservoir 。
        template_reservoir& io_reservoir,
        /// [in,out] 文字列からハッシュ値を作る driver::hasher 。
        template_hasher& io_hasher,
        /// [in] 状態値を登録するチャンクの識別値。
        typename template_reservoir::chunk_key const& in_chunk_key,
        /// [in] 状態値が記述されている psyq::string::relation_table 。
        /// 空の場合は、状態値は登録されない。
        template_relation_table const& in_table)
    {
        // 文字列表の属性を取得する。
        this_type::table_attribute<template_relation_table> const
            local_attribute(in_table);
        if (!local_attribute.is_valid())
        {
            PSYQ_ASSERT(in_table.is_empty());
            return 0;
        }

        // 文字列表を行ごとに解析し、状態値を登録する。
        auto const local_row_count(in_table.get_row_count());
        std::size_t local_register_count(0);
        for (
            typename template_relation_table::number i(0);
            i < local_row_count;
            ++i)
        {
            if (i != in_table.get_attribute_row()
                && this_type::register_status(
                    io_reservoir,
                    io_hasher,
                    in_chunk_key,
                    in_table,
                    i,
                    local_attribute))
            {
                ++local_register_count;
            }
        }
        return local_register_count;
    }

    //-------------------------------------------------------------------------
    /// @brief 文字列表を解析して状態値を構築し、状態貯蔵器へ登録する。
    /// @retval true  成功。構築した状態値を状態貯蔵器へ登録した。
    /// @retval false 失敗。状態値は状態貯蔵器へ登録されなかった。
    private: template<
        typename template_reservoir,
        typename template_hasher,
        typename template_relation_table>
    static bool register_status(
        /// [in,out] 状態値を登録する状態貯蔵器。
        template_reservoir& io_reservoir,
        /// [in,out] 文字列からハッシュ値を作る関数オブジェクト。
        template_hasher& io_hasher,
        /// [in] 状態値を登録するチャンクの識別値。
        typename template_reservoir::chunk_key const& in_chunk_key,
        /// [in] 解析する psyq::string::relation_table 。
        template_relation_table const& in_table,
        /// [in] 解析する in_table の行番号。
        typename template_relation_table::number const in_row_number,
        /// [in] in_table の属性。
        typename this_type::table_attribute<template_relation_table> const&
            in_attribute)
    {
        // 状態値の識別値を取得する。
        auto const local_status_key(
            io_hasher(in_table.find_cell(in_row_number, in_attribute.key_.first)));
        if (local_status_key == io_hasher(
                typename template_hasher::argument_type())
            || 0 < io_reservoir.find_bit_width(local_status_key))
        {
            // 状態値の識別値が空だったか、重複していた。
            PSYQ_ASSERT(false);
            return false;
        }

        // 状態値の型と初期値を取得し、状態値を登録する。
        typedef template_relation_table::string::view string_view;
        typename string_view const local_kind_cell(
            in_table.find_cell(in_row_number, in_attribute.kind_.first));
        typename string_view const local_value_cell(
            in_table.find_cell(in_row_number, in_attribute.value_.first));
        if (local_kind_cell == PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_BOOL)
        {
            // 論理型の状態値を登録する。
            return this_type::register_status<bool>(
                io_reservoir, in_chunk_key, local_status_key, local_value_cell);
        }
        if (local_kind_cell == PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_FLOAT)
        {
            // 浮動小数点数型の状態値を登録する。
            typedef
                typename template_reservoir::status_value::float_type
                float_type;
            return this_type::register_status<float_type>(
                io_reservoir, in_chunk_key, local_status_key, local_value_cell);
        }
        auto const local_unsigned_width(
            this_type::fetch_integer_width(
                local_kind_cell,
                string_view(PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_UNSIGNED),
                PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_INTEGER_WIDTH_DEFAULT));
        if (0 < local_unsigned_width)
        {
            // 符号なし整数型の状態値を登録する。
            typedef
                typename template_reservoir::status_value::unsigned_type
                unsigned_type;
            return this_type::register_status<unsigned_type>(
                io_reservoir,
                in_chunk_key,
                local_status_key,
                local_value_cell,
                local_unsigned_width);
        }
        auto const local_signed_width(
            this_type::fetch_integer_width(
                local_kind_cell,
                string_view(PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_KIND_SIGNED),
                PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_INTEGER_WIDTH_DEFAULT));
        if (0 < local_signed_width)
        {
            // 符号あり整数型の状態値を登録する。
            typedef
                typename template_reservoir::status_value::signed_type
                signed_type;
            return this_type::register_status<signed_type>(
                io_reservoir,
                in_chunk_key,
                local_status_key,
                local_value_cell,
                local_signed_width);
        }

        // 適切な型が見つからなかった。
        PSYQ_ASSERT(false);
        return false;
    }

    /// @brief 文字列を解析して状態値を構築し、状態貯蔵器へ登録する。
    /// @retval true  成功。構築した状態値を状態貯蔵器へ登録した。
    /// @retval false 失敗。状態値は状態貯蔵器へ登録されなかった。
    /// @tparam template_value 構築する状態値の型。
    private: template<
        typename template_value,
        typename template_reservoir,
        typename template_string>
    static bool register_status(
        /// [in,out] 状態値を登録する状態貯蔵器。
        template_reservoir& io_reservoir,
        /// [in] 状態値を登録するチャンクの識別値。
        typename template_reservoir::chunk_key const& in_chunk_key,
        /// [in] 登録する状態値に対応する識別値。
        typename template_reservoir::status_key const& in_status_key,
        /// [in] 解析する状態値の文字列。
        template_string const& in_value_cell)
    {
        psyq::string::numeric_parser<template_value> const local_parser(
            in_value_cell);
        if (local_parser.is_completed())
        {
            return io_reservoir.register_status(
                in_chunk_key, in_status_key, local_parser.get_value());
        }
        PSYQ_ASSERT(false);
        return false;
    }

    /// @brief 文字列を解析して整数型の状態値を構築し、状態貯蔵器へ登録する。
    /// @retval true  成功。構築した状態値を状態貯蔵器へ登録した。
    /// @retval false 失敗。状態値は状態貯蔵器へ登録されなかった。
    private: template<
        typename template_value,
        typename template_reservoir,
        typename template_string>
    static bool register_status(
        /// [in,out] 状態値を登録する状態貯蔵器。
        template_reservoir& io_reservoir,
        /// [in] 状態値を登録するチャンクの識別値。
        typename template_reservoir::chunk_key const& in_chunk_key,
        /// [in] 登録する状態値に対応する識別値。
        typename template_reservoir::status_key const& in_status_key,
        /// [in] 解析する状態値の文字列。
        template_string const& in_value_cell,
        /// [in] 状態値のビット幅。
        std::size_t const in_bit_width)
    {
        psyq::string::numeric_parser<template_value> const local_parser(
            in_value_cell);
        if (local_parser.is_completed())
        {
            return io_reservoir.register_status(
                in_chunk_key,
                in_status_key,
                local_parser.get_value(),
                in_bit_width);
        }
        PSYQ_ASSERT(false);
        return false;
    }

    /// @brief 整数型のビット数を取得する。
    /// @return 整数型のビット数。失敗した場合は0を返す。
    private: template<typename template_string>
    static std::size_t fetch_integer_width(
        /// [in] セルの文字列。
        template_string const& in_cell,
        /// [in] 整数型を表す文字列。
        template_string const& in_kind,
        /// [in] ビット数がない場合のデフォルト値。
        std::size_t const in_default_size)
    {
        PSYQ_ASSERT(!in_kind.empty());
        if (in_kind.size() <= in_cell.size()
            && in_kind == in_cell.substr(0, in_kind.size()))
        {
            if (in_kind.size() == in_cell.size())
            {
                return in_default_size;
            }
            if (in_kind.size() + 2 <= in_cell.size()
                && in_cell.at(in_kind.size()) == '_')
            {
                psyq::string::numeric_parser<std::size_t> const
                    local_parser(in_cell.substr(in_kind.size() + 1));
                if (local_parser.is_completed())
                {
                    return local_parser.get_value();
                }
            }
        }
        return 0;
    }

}; // class psyq::if_then_engine::status_builder

#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_BUILDER_HPP_)
// vim: set expandtab:
