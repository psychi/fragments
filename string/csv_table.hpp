/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::string::csv_table
*/
#ifndef PSYQ_STRING_CSV_TABLE_HPP_
#define PSYQ_STRING_CSV_TABLE_HPP_

#include <vector>
#include "./table.hpp"

/// @brief CSV文字列の列の区切り文字の有効判定。
#define PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_VALIDATION(\
    define_row_separator, define_column_separator) (\
        (define_column_separator) != (define_row_separator))

/// @brief CSV文字列の引用符文字の有効判定。
#define PSYQ_STRING_CSV_TABLE_QUOTE_VALIDATION(\
    define_row_separator, define_column_separator, define_quote) (\
        (define_quote) != (define_row_separator)\
        && (define_quote) != (define_column_separator))

/// @brief CSV文字列の行の区切り文字のデフォルト。
#ifndef PSYQ_STRING_CSV_TABLE_ROW_SEPARATOR_DEFAULT
#define PSYQ_STRING_CSV_TABLE_ROW_SEPARATOR_DEFAULT '\n'
#endif // !defined(PSYQ_STRING_CSV_TABLE_ROW_SEPARATOR_DEFAULT)

/// @brief CSV文字列の列の区切り文字のデフォルト。
#ifndef PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_DEFAULT
#define PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_DEFAULT ','
#endif // !defined(PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_DEFAULT)
static_assert(
    PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_VALIDATION(
        PSYQ_STRING_CSV_TABLE_ROW_SEPARATOR_DEFAULT,
        PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_DEFAULT),
    "PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_DEFAULT is invalid.");

/// @brief CSV文字列の引用符開始文字のデフォルト。
#ifndef PSYQ_STRING_CSV_TABLE_QUOTE_BEGIN_DEFAULT
#define PSYQ_STRING_CSV_TABLE_QUOTE_BEGIN_DEFAULT '"'
#endif // !defined(PSYQ_STRING_CSV_TABLE_QUOTE_BEGIN_DEFAULT)
static_assert(
    PSYQ_STRING_CSV_TABLE_QUOTE_VALIDATION(
        PSYQ_STRING_CSV_TABLE_ROW_SEPARATOR_DEFAULT,
        PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_DEFAULT,
        PSYQ_STRING_CSV_TABLE_QUOTE_BEGIN_DEFAULT),
    "PSYQ_STRING_CSV_TABLE_QUOTE_BEGIN_DEFAULT is invalid.");

/// @brief CSV文字列の引用符終了文字のデフォルト。
#ifndef PSYQ_STRING_CSV_TABLE_QUOTE_END_DEFAULT
#define PSYQ_STRING_CSV_TABLE_QUOTE_END_DEFAULT '"'
#endif // !defined(PSYQ_STRING_CSV_TABLE_QUOTE_END_DEFAULT)
static_assert(
    PSYQ_STRING_CSV_TABLE_QUOTE_VALIDATION(
        PSYQ_STRING_CSV_TABLE_ROW_SEPARATOR_DEFAULT,
        PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_DEFAULT,
        PSYQ_STRING_CSV_TABLE_QUOTE_END_DEFAULT),
    "PSYQ_STRING_CSV_TABLE_QUOTE_ESCAPE_END is invalid.");

/// @brief CSV文字列の引用符エスケープ文字のデフォルト。
#ifndef PSYQ_STRING_CSV_TABLE_QUOTE_ESCAPE_DEFAULT
#define PSYQ_STRING_CSV_TABLE_QUOTE_ESCAPE_DEFAULT '"'
#endif // !defined(PSYQ_STRING_CSV_TABLE_QUOTE_ESCAPE_DEFAULT)
static_assert(
    PSYQ_STRING_CSV_TABLE_QUOTE_VALIDATION(
        PSYQ_STRING_CSV_TABLE_ROW_SEPARATOR_DEFAULT,
        PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_DEFAULT,
        PSYQ_STRING_CSV_TABLE_QUOTE_ESCAPE_DEFAULT),
    "PSYQ_STRING_CSV_TABLE_QUOTE_ESCAPE_DEFAULT is invalid.");

/// @cond
namespace psyq
{
    namespace string
    {
        template<typename, typename, typename, int, int, int, int, int>
            class csv_table;
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief CSV形式の文字列から構築する、フライ級文字列表。
    @tparam template_char_type        @copydoc psyq::string::view::value_type
    @tparam template_char_traits      @copydoc psyq::string::view::traits_type
    @tparam template_allocator        @copydoc table::allocator_type
    @tparam template_row_separator    @copydoc csv_table::delimiter_ROW_SEPARATOR
    @tparam template_column_separator @copydoc csv_table::delimiter_COLUMN_SEPARATOR
    @tparam template_quote_begin      @copydoc csv_table::delimiter_QUOTE_BEGIN
    @tparam template_quote_end        @copydoc csv_table::delimiter_QUOTE_END
    @tparam template_quote_escape     @copydoc csv_table::delimiter_QUOTE_ESCAPE
*/
template<
    typename template_char_type,
    typename template_char_traits = PSYQ_STRING_VIEW_TRAITS_DEFAULT,
    typename template_allocator = PSYQ_STRING_FLYWEIGHT_ALLOCATOR_DEFAULT,
    int template_row_separator = PSYQ_STRING_CSV_TABLE_ROW_SEPARATOR_DEFAULT,
    int template_column_separator = PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_DEFAULT,
    int template_quote_begin = PSYQ_STRING_CSV_TABLE_QUOTE_BEGIN_DEFAULT,
    int template_quote_end = PSYQ_STRING_CSV_TABLE_QUOTE_END_DEFAULT,
    int template_quote_escape = PSYQ_STRING_CSV_TABLE_QUOTE_ESCAPE_DEFAULT>
class psyq::string::csv_table:
public psyq::string::table<
    template_char_type, template_char_traits, template_allocator>
{
    /// @brief thisが指す値の型。
    private: typedef csv_table this_type;

    /// @brief this_type の基底型。
    public: typedef
        psyq::string::table<
            template_char_type, template_char_traits, template_allocator>
        base_type;

    private: typedef typename base_type::allocator_type cell_allocator;

    //-------------------------------------------------------------------------
    /// @brief CSV文字列の区切り文字。
    public: enum delimiter: template_char_type
    {
        /// @brief CSV文字列の行の区切り文字。
        delimiter_ROW_SEPARATOR =
            static_cast<template_char_type>(template_row_separator),
        /// @brief CSV文字列の列の区切り文字。
        delimiter_COLUMN_SEPARATOR =
            static_cast<template_char_type>(template_column_separator),
        /// @brief CSV文字列の引用符の開始文字。
        delimiter_QUOTE_BEGIN =
            static_cast<template_char_type>(template_quote_begin),
        /// @brief CSV文字列の引用符の終了文字。
        delimiter_QUOTE_END =
            static_cast<template_char_type>(template_quote_end),
        /// @brief CSV文字列の引用符のエスケープ文字。
        delimiter_QUOTE_ESCAPE =
            static_cast<template_char_type>(template_quote_escape),
    };
    static_assert(
        this_type::delimiter_ROW_SEPARATOR == template_row_separator,
        "this_type::delimiter_ROW_SEPARATOR is invalid.");
    static_assert(
        this_type::delimiter_COLUMN_SEPARATOR == template_column_separator
        && PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_VALIDATION(
            this_type::delimiter_ROW_SEPARATOR,
            this_type::delimiter_COLUMN_SEPARATOR),
        "this_type::delimiter_COLUMN_SEPARATOR is invalid.");
    static_assert(
        this_type::delimiter_QUOTE_BEGIN == template_quote_begin
        && PSYQ_STRING_CSV_TABLE_QUOTE_VALIDATION(
            this_type::delimiter_ROW_SEPARATOR,
            this_type::delimiter_COLUMN_SEPARATOR,
            this_type::delimiter_QUOTE_BEGIN),
        "this_type::delimiter_QUOTE_BEGIN is invalid.");
    static_assert(
        this_type::delimiter_QUOTE_END == template_quote_end
        && PSYQ_STRING_CSV_TABLE_QUOTE_VALIDATION(
            this_type::delimiter_ROW_SEPARATOR,
            this_type::delimiter_COLUMN_SEPARATOR,
            this_type::delimiter_QUOTE_END),
        "this_type::delimiter_QUOTE_END is invalid.");
    static_assert(
        this_type::delimiter_QUOTE_ESCAPE == template_quote_escape
        && PSYQ_STRING_CSV_TABLE_QUOTE_VALIDATION(
            this_type::delimiter_ROW_SEPARATOR,
            this_type::delimiter_COLUMN_SEPARATOR,
            this_type::delimiter_QUOTE_ESCAPE),
        "this_type::delimiter_QUOTE_ESCAPE is invalid.");

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief CSV形式の文字列を解析し、文字列表を構築する。
        @param[out] out_workspace
            作業領域として使う文字列。
            std::basic_string 互換のインターフェイスを持つこと。
        @param[in] in_csv_string 解析するCSV形式の文字列。
        @param[in] in_factory
            フライ級文字列生成器を指すスマートポインタ。
            空のスマートポインタではないこと。
     */
    public: template<typename template_workspace>
    csv_table(
        template_workspace& out_workspace,
        typename base_type::string::view const& in_csv_string,
        typename base_type::string::factory::shared_ptr const& in_factory)
    :
    base_type((
        PSYQ_ASSERT(in_factory.get() != nullptr),
        in_factory->get_allocator()))
    {
        auto const local_build(
            this->build(out_workspace, in_csv_string, in_factory));
        PSYQ_ASSERT(local_build);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name CSV文字列の解析
    //@{
    /** @brief CSV形式の文字列を解析し、文字列表を構築する。
        @param[out] out_workspace
            作業領域として使う文字列。
            std::basic_string 互換のインターフェイスを持つこと。
        @param[in] in_csv_string 解析するCSV形式の文字列。
        @param[in] in_factory
            フライ級文字列生成器を指すスマートポインタ。
            スマートポインタが空ではないこと。
     */
    public: template<typename template_workspace>
    bool build(
        template_workspace& out_workspace,
        typename base_type::string::view const& in_csv_string,
        typename base_type::string::factory::shared_ptr const& in_factory)
    {
        if (in_factory.get() == nullptr)
        {
            return false;
        }
        out_workspace.clear();
        this->clear_container(in_csv_string.size() / 8);
        bool local_quote(false);
        typename base_type::string::size_type local_row(0);
        typename base_type::string::size_type local_column(0);
        typename base_type::string::size_type local_column_max(0);
        typename base_type::string::value_type local_last_char(0);
        typename base_type::string::size_type local_cell_size(0);
        for (auto i(in_csv_string.begin()); i != in_csv_string.end(); ++i)
        {
            auto const local_char(*i);
            if (local_quote)
            {
                if (local_last_char != this_type::delimiter_QUOTE_ESCAPE)
                {
                    if (local_char != this_type::delimiter_QUOTE_END)
                    {
                        // エスケープ文字でなければ、文字をセルに追加する。
                        if (local_char != this_type::delimiter_QUOTE_ESCAPE)
                        {
                            out_workspace.push_back(local_char);
                            local_cell_size = out_workspace.size();
                        }
                        local_last_char = local_char;
                    }
                    else
                    {
                        // 引用符を終了。
                        local_quote = false;
                        local_last_char = 0;
                    }
                }
                else if (local_char == this_type::delimiter_QUOTE_END)
                {
                    // 引用符の終了文字をエスケープする。
                    out_workspace.push_back(local_char);
                    local_cell_size = out_workspace.size();
                    local_last_char = 0;
                }
                else if (
                    this_type::delimiter_QUOTE_ESCAPE
                    != this_type::delimiter_QUOTE_END)
                {
                    // 直前の文字がエスケープとして働かなかったので、
                    // 直前の文字と現在の文字をセルに追加する。
                    out_workspace.push_back(local_last_char);
                    out_workspace.push_back(local_char);
                    local_cell_size = out_workspace.size();
                    local_last_char = local_char;
                }
                else
                {
                    // 引用符を終了し、文字を巻き戻す。
                    local_quote = false;
                    local_last_char = 0;
                    --i;
                }
            }
            else if (local_char == this_type::delimiter_QUOTE_BEGIN)
            {
                // 引用符の開始。
                local_quote = true;
            }
            else if (local_char == this_type::delimiter_COLUMN_SEPARATOR)
            {
                // 列の区切り。
                if (!out_workspace.empty())
                {
                    this->replace_cell(
                        local_row,
                        local_column,
                        typename base_type::string(
                            typename base_type::string::view(
                                out_workspace.data(), local_cell_size),
                            in_factory));
                    out_workspace.clear();
                    local_cell_size = 0;
                }
                ++local_column;
            }
            else if (local_char == this_type::delimiter_ROW_SEPARATOR)
            {
                // 行の区切り。
                if (!out_workspace.empty())
                {
                    this->replace_cell(
                        local_row,
                        local_column,
                        typename base_type::string(
                            typename base_type::string::view(
                                out_workspace.data(), local_cell_size),
                            in_factory));
                    out_workspace.clear();
                    local_cell_size = 0;
                }
                else if (0 < local_column)
                {
                    --local_column;
                }
                if (local_column_max < local_column)
                {
                    local_column_max = local_column;
                }
                local_column = 0;
                ++local_row;
            }
            else
            {
                // セルに文字を追加する。
                auto const local_is_space(std::isspace(local_char));
                if (!local_is_space || !out_workspace.empty())
                {
                    out_workspace.push_back(local_char);
                    if (!local_is_space)
                    {
                        local_cell_size = out_workspace.size();
                    }
                }
            }
        }

        // 最終セルの処理。
#if 0
        if (local_quote)
        {
            // 引用符の開始はあったが終了がなかった場合は、
            // 引用符として処理しない。
            out_workspace.insert(out_workspace.begin(), this_type::delimiter_QUOTE_BEGIN);
        }
#endif // 0
        if (!out_workspace.empty())
        {
            this->replace_cell(
                local_row,
                local_column,
                typename base_type::string(
                    typename base_type::string::view(
                        out_workspace.data(), local_cell_size),
                    in_factory));
        }
        else if (0 < local_column)
        {
            --local_column;
        }
        else if (0 < local_row)
        {
            --local_row;
        }
        else
        {
            out_workspace.clear();
            this->shrink_to_fit();
            return true;
        }
        out_workspace.clear();
        this->shrink_to_fit();

        // 文字列表の大きさを決定する。
        if (local_column_max < local_column)
        {
            local_column_max = local_column;
        }
        this->set_size(local_row + 1, local_column_max + 1);
        return true;
    }
    //@}

}; // class psyq::string::csv_table

#endif // !defined(PSYQ_STRING_CSV_TABLE_HPP_)
// vim: set expandtab:
