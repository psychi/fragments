/** @file
    @copybrief psyq::scenario_engine::state_builder
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_HPP_
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_HPP_

#include "../string/relation_table.hpp"

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_COLUMN_KEY
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_COLUMN_KEY "KEY"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_COLUMN_KEY)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_COLUMN_KIND
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_COLUMN_KIND "KIND"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_COLUMN_KIND)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_COLUMN_VALUE
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_COLUMN_VALUE "VALUE"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_COLUMN_VALUE)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_BOOL
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_BOOL "BOOL"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_BOOL)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_UNSIGNED
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_UNSIGNED "UNSIGNED"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_UNSIGNED)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_SIGNED
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_SIGNED "SIGNED"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_SIGNED)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_FLOAT
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_FLOAT "FLOAT"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_FLOAT)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_INTEGER_SIZE_DEFAULT
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_INTEGER_SIZE_DEFAULT 32
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_INTEGER_SIZE_DEFAULT)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_BUILDER_FLOAT_SIZE_DEFAULT
#define PSYQ_SCENARIO_ENGINE_STATE_BUILDER_FLOAT_SIZE_DEFAULT 32
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_FLOAT_SIZE_DEFAULT)

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

    driver::extend_chunk の引数として使う。

    @tparam template_relation_table @copydoc state_builder::relation_table
 */
template<typename template_relation_table>
class psyq::scenario_engine::state_builder
{
    /// @brief thisが指す値の型。
    private: typedef state_builder this_type;

    /// @brief 解析する関係文字列表の型。
    public: typedef template_relation_table relation_table;

    /// @brief 文字列表の属性。
    private: class table_attribute
    {
        public: explicit table_attribute(
            typename state_builder::relation_table const& in_table)
        PSYQ_NOEXCEPT:
        key_(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_STATE_BUILDER_COLUMN_KEY)),
        kind_(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_STATE_BUILDER_COLUMN_KIND)),
        value_(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_STATE_BUILDER_COLUMN_VALUE))
        {}

        public: bool is_valid() const PSYQ_NOEXCEPT
        {
            return 0 < this->key_.second
                && 0 < this->kind_.second
                && 0 < this->value_.second;
        }

        public: typename this_type::relation_table::attribute key_;
        public: typename this_type::relation_table::attribute kind_;
        public: typename this_type::relation_table::attribute value_;

    }; // class table_attribute

    //-------------------------------------------------------------------------
    /** @brief 文字列表から状態値を構築する関数オブジェクトを構築する。
        @param[in] in_table 解析する文字列表。
     */
    public: explicit state_builder(typename this_type::relation_table in_table):
    relation_table_(std::move(in_table))
    {}

    /** @brief 文字列表を解析して状態値を構築し、状態貯蔵器へ登録する。
        @param[in,out] io_reservoir 状態値を登録する状態貯蔵器。
        @param[in,out] io_hasher    文字列からハッシュ値を作る関数オブジェクト。
        @param[in] in_chunk_key     状態値を登録するチャンクの識別値。
        @return 登録した状態値の数。
     */
    public: template<typename template_reservoir, typename template_hasher>
    std::size_t operator()(
        template_reservoir& io_reservoir,
        template_hasher& io_hasher,
        typename template_reservoir::chunk_key const& in_chunk_key)
    const
    {
        return this_type::build(
            io_reservoir, io_hasher, in_chunk_key, this->relation_table_);
    }

    /** @brief 文字列表を解析して状態値を構築し、状態貯蔵器へ登録する。
        @param[in,out] io_reservoir 状態値を登録する状態貯蔵器。
        @param[in,out] io_hasher    文字列からハッシュ値を作る関数オブジェクト。
        @param[in] in_chunk_key     状態値を登録するチャンクの識別値。
        @param[in] in_table         解析する文字列表。
        @return 登録した状態値の数。
     */
    public: template<typename template_reservoir, typename template_hasher>
    static std::size_t build(
        template_reservoir& io_reservoir,
        template_hasher& io_hasher,
        typename template_reservoir::chunk_key const& in_chunk_key,
        typename this_type::relation_table const& in_table)
    {
        // 文字列表の属性を取得する。
        typename this_type::table_attribute const local_attribute(in_table);
        if (!local_attribute.is_valid())
        {
            PSYQ_ASSERT(false);
            return 0;
        }

        // 文字列表を行ごとに解析し、状態値を登録する。
        auto const local_row_count(in_table.get_row_count());
        std::size_t local_register_count(0);
        for (
            typename this_type::relation_table::string::size_type i(0);
            i < local_row_count;
            ++i)
        {
            if (i != in_table.get_attribute_row())
            {
                auto const local_register_state(
                    this_type::register_state(
                        io_reservoir,
                        io_hasher,
                        in_chunk_key,
                        in_table,
                        i,
                        local_attribute));
                if (local_register_state)
                {
                    ++local_register_count;
                }
            }
        }
        return local_register_count;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表を解析して状態値を構築し、状態貯蔵器へ登録する。
        @param[in,out] io_reservoir 状態値を登録する状態貯蔵器。
        @param[in,out] io_hasher    文字列からハッシュ値を作る関数オブジェクト。
        @param[in] in_chunk_key     状態値を登録するチャンクの識別値。
        @param[in] in_table         解析する文字列表。
        @param[in] in_row_index     解析する文字列表の行番号。
        @param[in] in_attribute     文字列表の属性。
        @retval true  成功。構築した状態値を状態貯蔵器へ登録した。
        @retval false 失敗。状態値は状態貯蔵器へ登録されなかった。
     */
    private: template<typename template_reservoir, typename template_hasher>
    static bool register_state(
        template_reservoir& io_reservoir,
        template_hasher& io_hasher,
        typename template_reservoir::chunk_key const& in_chunk_key,
        typename this_type::relation_table const& in_table,
        typename this_type::relation_table::string::size_type const in_row_index,
        typename this_type::table_attribute const& in_attribute)
    {
        // 状態値のキーを取得する。
        auto const& local_key_cell(
            in_table.find_body_cell(in_row_index, in_attribute.key_.first));
        if (local_key_cell.empty())
        {
            PSYQ_ASSERT(false);
            return false;
        }
        auto const local_key(io_hasher(local_key_cell));
        if (local_key == io_hasher(typename template_hasher::argument_type())
            || io_reservoir.get_variety(local_key)
               != template_reservoir::state_value::kind_NULL)
        {
            // 条件キーが重複している。
            PSYQ_ASSERT(false);
            return false;
        }

        // 状態値の種類と初期値を取得し、状態値を登録する。
        return this_type::register_state(
            io_reservoir,
            in_chunk_key,
            local_key,
            in_table.find_body_cell(in_row_index, in_attribute.kind_.first),
            in_table.find_body_cell(in_row_index, in_attribute.value_.first));
    }

    /** @brief 状態値の型と初期値を解析して状態値を構築し、状態貯蔵器へ登録する。
        @param[in,out] io_reservoir 状態値を登録する状態貯蔵器。
        @param[in] in_chunk_key     状態値を登録するチャンクの識別値。
        @param[in] in_state_key     登録する状態値の識別値。
        @param[in] in_kind          登録する状態値の型を表す文字列。
        @param[in] in_value         登録する状態値の初期値を表す文字列。
        @retval true  成功。構築した状態値を状態貯蔵器へ登録した。
        @retval false 失敗。状態値は状態貯蔵器へ登録されなかった。
     */
    private: template<typename template_reservoir>
    static bool register_state(
        template_reservoir& io_reservoir,
        typename template_reservoir::chunk_key const& in_chunk_key,
        typename template_reservoir::state_key const& in_state_key,
        typename this_type::relation_table::string::view const& in_kind,
        typename this_type::relation_table::string::view const& in_value)
    {
        if (in_kind == PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_BOOL)
        {
            // 論理型の状態値を登録する。
            return this_type::register_bool(
                io_reservoir, in_chunk_key, in_state_key, in_value);
        }
        if (in_kind == PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_FLOAT)
        {
            // 浮動小数点数型の状態値を登録する。
            return this_type::register_float(
                io_reservoir, in_chunk_key, in_state_key, in_value);
        }
        std::size_t const local_default_size(8);
        auto const local_unsigned_size(
            this_type::get_integer_size(
                in_kind,
                PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_UNSIGNED,
                local_default_size));
        if (0 < local_unsigned_size)
        {
            // 符号なし整数型の状態値を登録する。
            return this_type::register_unsigned(
                io_reservoir,
                in_chunk_key,
                in_state_key,
                in_value,
                local_unsigned_size);
        }
        auto const local_signed_size(
            this_type::get_integer_size(
                in_kind,
                PSYQ_SCENARIO_ENGINE_STATE_BUILDER_KIND_SIGNED,
                local_default_size));
        if (0 < local_signed_size)
        {
            // 符号あり整数型の状態値を登録する。
            return this_type::register_signed(
                io_reservoir,
                in_chunk_key,
                in_state_key,
                in_value,
                local_signed_size);
        }

        // 適切な型が見つからなかった。
        PSYQ_ASSERT(false);
        return false;
    }

    /** @brief 文字列を解析して論理型の状態値を構築し、状態貯蔵器へ登録する。
        @param[in,out] io_reservoir 状態値を登録する状態貯蔵器。
        @param[in] in_chunk_key     状態値を登録するチャンクの識別値。
        @param[in] in_state_key     登録する状態値に対応する識別値。
        @param[in] in_value_cell    解析する状態値の文字列。
        @retval true  成功。構築した状態値を状態貯蔵器へ登録した。
        @retval false 失敗。状態値は状態貯蔵器へ登録されなかった。
     */
    private: template<typename template_reservoir>
    static bool register_bool(
        template_reservoir& io_reservoir,
        typename template_reservoir::chunk_key const& in_chunk_key,
        typename template_reservoir::state_key const& in_state_key,
        typename this_type::relation_table::string::view const& in_value_cell)
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

    /** @brief 文字列を解析して符号なし整数型の状態値を構築し、状態貯蔵器へ登録する。
        @param[in,out] io_reservoir 状態値を登録する状態貯蔵器。
        @param[in] in_chunk_key     状態値を登録するチャンクの識別値。
        @param[in] in_state_key     登録する状態値に対応する識別値。
        @param[in] in_value_cell    解析する状態値の文字列。
        @param[in] in_size          状態値のビット数。
        @retval true  成功。構築した状態値を状態貯蔵器へ登録した。
        @retval false 失敗。状態値は状態貯蔵器へ登録されなかった。
     */
    private: template<typename template_reservoir>
    static bool register_unsigned(
        template_reservoir& io_reservoir,
        typename template_reservoir::chunk_key const& in_chunk_key,
        typename template_reservoir::state_key const& in_state_key,
        typename this_type::relation_table::string::view const& in_value_cell,
        std::size_t const in_size)
    {
        std::size_t local_rest_size;
        auto const local_value(
            in_value_cell.template
                to_integer<typename template_reservoir::state_value::unsigned_type>(
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

    /** @brief 文字列を解析して符号あり整数型の状態値を構築し、状態貯蔵器へ登録する。
        @param[in,out] io_reservoir 状態値を登録する状態貯蔵器。
        @param[in] in_chunk_key     状態値を登録するチャンクの識別値。
        @param[in] in_state_key     登録する状態値に対応する識別値。
        @param[in] in_value_cell    解析する状態値の文字列。
        @param[in] in_size          状態値のビット数。
        @retval true  成功。構築した状態値を状態貯蔵器へ登録した。
        @retval false 失敗。状態値は状態貯蔵器へ登録されなかった。
     */
    private: template<typename template_reservoir>
    static bool register_signed(
        template_reservoir& io_reservoir,
        typename template_reservoir::chunk_key const& in_chunk_key,
        typename template_reservoir::state_key const& in_state_key,
        typename this_type::relation_table::string::view const& in_value_cell,
        std::size_t const in_size)
    {
        std::size_t local_rest_size;
        auto const local_value(
            in_value_cell.template
                to_integer<typename template_reservoir::state_value::signed_type>(
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

    /** @brief 文字列を解析して浮動小数点数型の状態値を構築し、状態貯蔵器へ登録する。
        @param[in,out] io_reservoir 状態値を登録する状態貯蔵器。
        @param[in] in_chunk_key     状態値を登録するチャンクの識別値。
        @param[in] in_state_key     登録する状態値に対応する識別値。
        @param[in] in_value_cell    解析する状態値の文字列。
        @retval true  成功。構築した状態値を状態貯蔵器へ登録した。
        @retval false 失敗。状態値は状態貯蔵器へ登録されなかった。
     */
    private: template<typename template_reservoir>
    static bool register_float(
        template_reservoir& io_reservoir,
        typename template_reservoir::chunk_key const& in_chunk_key,
        typename template_reservoir::state_key const& in_state_key,
        typename this_type::relation_table::string::view const& in_value_cell)
    {
        std::size_t local_rest_size;
        auto const local_value(
            in_value_cell.template
                to_real<typename template_reservoir::state_value::float_type>(
                    &local_rest_size));
        if (local_rest_size != 0)
        {
            // 初期値セルを実数として解析しきれなかった。
            PSYQ_ASSERT(false);
            return false;
        }
        return io_reservoir.register_float(
            in_chunk_key, in_state_key, local_value);
    }

    /** @brief 整数型のビット数を取得する。
        @param[in] in_cell         セルの文字列。
        @param[in] in_kind         整数型を表す文字列。
        @param[in] in_default_size ビット数がない場合のデフォルト値。
        @return !=0 成功。整数型のビット数。
        @return ==0 失敗。
     */
    private: static std::size_t get_integer_size(
        typename this_type::relation_table::string::view const& in_cell,
        typename this_type::relation_table::string::view const& in_kind,
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
    /// @brief 解析する文字列表。
    private: typename this_type::relation_table relation_table_;

}; // class psyq::scenario_engine::state_builder

#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_BUILDER_HPP_)
// vim: set expandtab:
