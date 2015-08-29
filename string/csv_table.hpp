/*
Copyright (c) 2015, Hillco Psychi, All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 
ソースコード形式かバイナリ形式か、変更するかしないかを問わず、
以下の条件を満たす場合に限り、再頒布および使用が許可されます。

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer. 
   ソースコードを再頒布する場合、上記の著作権表示、本条件一覧、
   および下記の免責条項を含めること。
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 
   バイナリ形式で再頒布する場合、頒布物に付属のドキュメント等の資料に、
   上記の著作権表示、本条件一覧、および下記の免責条項を含めること。

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
本ソフトウェアは、著作権者およびコントリビューターによって
「現状のまま」提供されており、明示黙示を問わず、商業的な使用可能性、
および特定の目的に対する適合性に関する暗黙の保証も含め、
またそれに限定されない、いかなる保証もありません。
著作権者もコントリビューターも、事由のいかんを問わず、
損害発生の原因いかんを問わず、かつ責任の根拠が契約であるか厳格責任であるか
（過失その他の）不法行為であるかを問わず、
仮にそのような損害が発生する可能性を知らされていたとしても、
本ソフトウェアの使用によって発生した（代替品または代用サービスの調達、
使用の喪失、データの喪失、利益の喪失、業務の中断も含め、
またそれに限定されない）直接損害、間接損害、偶発的な損害、特別損害、
懲罰的損害、または結果損害について、一切責任を負わないものとします。
 */
/// @file
/// @author Hillco Psychi (https://twitter.com/psychi)
/// @brief @copybrief psyq::string::csv_table
#ifndef PSYQ_STRING_CSV_TABLE_HPP_
#define PSYQ_STRING_CSV_TABLE_HPP_

#include "./relation_table.hpp"

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

/// @brief CSV文字列の引用符開始文字のデフォルト。
#ifndef PSYQ_STRING_CSV_TABLE_QUOTE_BEGIN_DEFAULT
#define PSYQ_STRING_CSV_TABLE_QUOTE_BEGIN_DEFAULT '"'
#endif // !defined(PSYQ_STRING_CSV_TABLE_QUOTE_BEGIN_DEFAULT)

/// @brief CSV文字列の引用符終了文字のデフォルト。
#ifndef PSYQ_STRING_CSV_TABLE_QUOTE_END_DEFAULT
#define PSYQ_STRING_CSV_TABLE_QUOTE_END_DEFAULT '"'
#endif // !defined(PSYQ_STRING_CSV_TABLE_QUOTE_END_DEFAULT)

/// @brief CSV文字列の引用符エスケープ文字のデフォルト。
#ifndef PSYQ_STRING_CSV_TABLE_QUOTE_ESCAPE_DEFAULT
#define PSYQ_STRING_CSV_TABLE_QUOTE_ESCAPE_DEFAULT '"'
#endif // !defined(PSYQ_STRING_CSV_TABLE_QUOTE_ESCAPE_DEFAULT)

/// @cond
namespace psyq
{
    namespace string
    {
        template<typename, typename, typename, typename, int, int, int, int, int>
            class csv_table;
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief CSV形式の文字列から構築する、フライ級文字列表。
/// @tparam template_number           @copydoc table::number
/// @tparam template_char_type        @copydoc psyq::string::view::value_type
/// @tparam template_char_traits      @copydoc psyq::string::view::traits_type
/// @tparam template_allocator        @copydoc table::allocator_type
/// @tparam template_row_separator    @copydoc csv_table::delimiter_ROW_SEPARATOR
/// @tparam template_column_separator @copydoc csv_table::delimiter_COLUMN_SEPARATOR
/// @tparam template_quote_begin      @copydoc csv_table::delimiter_QUOTE_BEGIN
/// @tparam template_quote_end        @copydoc csv_table::delimiter_QUOTE_END
/// @tparam template_quote_escape     @copydoc csv_table::delimiter_QUOTE_ESCAPE
template<
    typename template_number,
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
    template_number, template_char_type, template_char_traits, template_allocator>
{
    /// @brief this が指す値の型。
    private: typedef csv_table this_type;
    /// @brief this_type の基底型。
    public: typedef
        psyq::string::table<
            template_number,
            template_char_type,
            template_char_traits,
            template_allocator>
        base_type;

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
    /// @brief 空のCSV文字列表を構築する。
    public: explicit csv_table(
        /// [in] メモリ割当子の初期値。
        typename base_type::allocator_type const& in_allocator):
    base_type(in_allocator)
    {}

    /// @brief CSV形式の文字列を解析し、文字列表を構築する。
    public: template<typename template_workspace>
    csv_table(
        /// [out] 作業領域として使う std::basic_string 互換の文字列。
        /// 入力した値は破壊され、出力される値には意味がない。
        template_workspace& out_workspace,
        /// psyq::string::flyweight::factory の強参照。空ではないこと。
        typename base_type::string::factory::shared_ptr const&
            in_string_factory,
        /// [in] 解析するCSV形式の文字列。
        typename base_type::string::view const& in_csv_string):
    base_type((
        PSYQ_ASSERT(in_string_factory.get() != nullptr),
        in_string_factory->get_allocator()))
    {
        auto const local_build(
            this->build(out_workspace, in_csv_string, in_string_factory));
        PSYQ_ASSERT(local_build);
    }

    /// @brief CSV文字列から関係文字列表を作る。
    /// @return
    /// in_csv_string から作った psyq::string::relation_table 。
    /// in_csv_string が空の場合は、関係文字列表も空となる。
    template<typename template_workspace_string>
    static psyq::string::relation_table<
        typename base_type::number,
        typename base_type::string::value_type,
        typename base_type::string::traits_type,
        typename base_type::allocator_type>
    build_relation_table(
        /// [out] 作業領域として使う std::basic_string 互換の文字列。
        /// 入力した値は破壊され、出力される値には意味がない。
        template_workspace_string& out_workspace,
        /// [in] CSV文字列表の構築に使う
        /// psyq::string::flyweight::factory の強参照。空ではないこと。
        typename base_type::string::factory::shared_ptr const& in_string_factory,
        /// [in] 解析するCSV形式の文字列。空の場合は、空の表が構築される。
        typename base_type::string::view const& in_csv_string,
        /// [in] 関係文字列表の属性に使う行の番号。
        /// base_type::INVALID_NUMBER の場合は、属性の辞書を構築しない。
        typename base_type::number const in_attribute_row,
        /// [in] 関係文字列表の主キーに使う属性の文字列。
        /// 空の場合は、主キーの辞書を構築しない。
        typename base_type::string::view const& in_attribute_key =
            typename base_type::string::view(),
        /// [in] 関係文字列表の主キーに使う属性のインデクス番号。
        typename base_type::number const in_attribute_index = 0)
    {
        typedef
            psyq::string::relation_table<
                typename base_type::number,
                typename base_type::string::value_type,
                typename base_type::string::traits_type,
                typename base_type::allocator_type>
            relation_table;
        return in_csv_string.empty()?
            relation_table(
                in_string_factory.get() != nullptr?
                    in_string_factory->get_allocator():
                    (PSYQ_ASSERT(false), typename base_type::allocator_type())):
            relation_table(
                this_type(out_workspace, in_string_factory, in_csv_string),
                in_attribute_row,
                in_attribute_key,
                in_attribute_index);
    }

    //-------------------------------------------------------------------------
    /// @brief CSV形式の文字列を解析し、文字列表を構築する。
    private: template<typename template_workspace_string>
    bool build(
        /// [out] 作業領域として使う文字列。
        /// std::basic_string 互換のインターフェイスを持つこと。
        template_workspace_string& out_workspace,
        /// [in] 解析するCSV形式の文字列。
        typename base_type::string::view const& in_csv_string,
        /// [in] フライ級文字列の生成器を指すスマートポインタ。
        /// スマートポインタが空ではないこと。
        typename base_type::string::factory::shared_ptr const& in_string_factory)
    {
        if (in_string_factory.get() == nullptr)
        {
            return false;
        }
        out_workspace.clear();
        this->clear_container(in_csv_string.size() / 8);
        bool local_quote(false);
        typename base_type::number local_row(0);
        typename base_type::number local_column(0);
        typename base_type::number local_column_max(0);
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
                            in_string_factory));
                    out_workspace.clear();
                    local_cell_size = 0;
                    if (local_column_max < local_column)
                    {
                        local_column_max = local_column;
                    }
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
                            in_string_factory));
                    out_workspace.clear();
                    local_cell_size = 0;
                    if (local_column_max < local_column)
                    {
                        local_column_max = local_column;
                    }
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
                    in_string_factory));
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
        if (!this->is_empty())
        {
            if (local_column_max < local_column)
            {
                local_column_max = local_column;
            }
            this->set_size(local_row + 1, local_column_max + 1);
        }
        return true;
    }

}; // class psyq::string::csv_table

#endif // !defined(PSYQ_STRING_CSV_TABLE_HPP_)
// vim: set expandtab:
