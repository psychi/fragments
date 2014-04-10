/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

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

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::string_table
 */
#ifndef PSYQ_STRING_TABLE_HPP_
#define PSYQ_STRING_TABLE_HPP_

#include <unordered_map>
//#include "psyq/string/shared_string.hpp"

namespace psyq
{
    /// @cond
    struct string_table_attribute;
    template<typename, typename> class string_table;
    /// @endcond

    /// 共有文字列の表。
    typedef psyq::string_table<
        std::unordered_map<
            std::size_t,
            std::unordered_map<std::size_t, psyq::shared_string>>,
        std::unordered_map<
            psyq::shared_string,
            psyq::string_table_attribute,
            psyq::shared_string::fnv1_hash>>
                shared_string_table;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 文字列表の属性辞書の値。
struct psyq::string_table_attribute
{
    string_table_attribute(
        std::size_t const in_column,
        std::size_t const in_size)
    :
        column(in_column),
        size(in_size)
    {}

    std::size_t column; ///< 属性の列番号。
    std::size_t size;   ///< 属性の要素数。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 文字列の表。CSV形式の文字列から構築する。

    使い方の概略。
    -# string_table::make_column_map() で文字列表の列の辞書を構築する。
    -# string_table::string_table() に文字列表の列yの辞書を渡し、
       文字列表を構築する。
    -# string_table::find_body_cell() で、任意のcellを検索する。

    @tparam template_column_map    @copydoc string_table::column_map
    @tparam template_attribute_map @copydoc string_table::attribute_map
 */
template<typename template_column_map, typename template_attribute_map>
class psyq::string_table
{
    /// thisが指す値の型。
    private: typedef string_table<template_column_map, template_attribute_map>
        self;

    /** @brief 文字列表の列の辞書。

        以下の条件を満たしている必要がある。
        - std::unordered_map 相当の型。要素がsortされてなくともよい。
        - template_column_map::key_type は、文字列表の列の番号。
        - template_column_map::mapped_type は、文字列表の行の辞書。
          以下の条件を満たしている必要がある。
          - std::unordered_map 相当の型。要素がsortされてなくともよい。
          - template_column_map::mapped_type::key_type は、文字列表の行の番号。
          - template_column_map::mapped_type::mapped_type は、
            文字列表の要素となる、 psyq::basic_shared_string 相当の文字列。

        @note 2014.04.05
            文字列表では行の辞書を保持することも考えたが、
            表を編集し属性の追加や削除を考えると、
            列の辞書を保持したほうがいいように思う。
     */
    public: typedef template_column_map column_map;

    /** @brief 文字列表の行の辞書。

        - std::unordered_map 相当の型。要素がsortされてなくともよい。
        - row_map::key_type は、文字列表の行の番号。
        - row_map::mapped_type は、文字列表の要素となる、
          psyq::basic_shared_string 相当の文字列。
     */
    public: typedef typename template_column_map::mapped_type row_map;

    /** @brief 文字列表の要素となる、 psyq::basic_shared_string 相当の文字列。
     */
    public: typedef typename template_column_map::mapped_type::mapped_type
        cell_string;

    /** @brief 文字列表の属性の辞書。

        以下の条件を満たしている必要がある。
        - std::map 相当の型。要素がsortされてなくともよい。
        - attribute_map::key_type は、 psyq::basic_shared_string 相当の文字列。
        - attribute_map::mapped_type は、 psyq::string_table_attribute 型。
     */
    public: typedef template_attribute_map attribute_map;

    //-------------------------------------------------------------------------
    /** @brief 文字列表を構築する。
        @param[in,out] io_column_map
            make_column_map() で構築した、文字列表の列の辞書。
        @param[in] in_attribute_row 文字列表の属性行として使う行の番号。
     */
    public: string_table(
        typename self::column_map io_column_map,
        std::size_t const         in_attribute_row)
    :
        attribute_map_(
            self::make_attribute_map(io_column_map, in_attribute_row)),
        attribute_row_(in_attribute_row),
        column_map_(std::move(io_column_map))
    {}

    /** @brief move-constructor
        @param[in,out] io_source move元となる文字列表。
     */
    public: string_table(self&& io_source):
        attribute_map_(std::move(io_source.attribute_map_)),
        attribute_row_(std::move(io_source.attribute_row_)),
        cell_map_(std::move(io_source.cell_map_))
    {
        io_source.clear();
    }

    /** @brief move代入演算子。
        @param[in,out] io_source move元となる文字列表。
        @return *this
     */
    public: self& operator=(self&& io_source)
    {
        if (this != &io_source)
        {
            this->attribute_map_ = std::move(io_source.attribute_map_);
            this->attribute_row_ = std::move(io_source.attribute_row_);
            this->cell_map_ = std::move(io_source.cell_map_);
            io_source.clear();
        }
        return *this;
    }

    /** @brief 文字列表を空にする。
     */
    public: void clear()
    {
        this->attribute_map_.clear();
        this->attribute_row_ = 0;
        this->column_map_.clear();
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表の列の辞書を取得する。
        @return @copydoc column_map
     */
    public: typename self::column_map const& get_column_map() const
    {
        return this->column_map_;
    }

    /** @brief 文字列表の属性の辞書を取得する。
        @return @copydoc attribute_map
     */
    public: typename self::attribute_map const& get_attribute_map() const
    {
        return this->attribute_map_;
    }

    /** @brief 文字列表の属性行の番号を取得する。
        @return @copydoc attribute_row_
     */
    public: std::size_t get_attribute_row() const
    {
        return this->attribute_row_;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表の本体のcell文字列を解析し、値を取り出す。
        @tparam template_value_type 取り出す値の型。
        @param[in]  in_default         解析に失敗した場合の戻り値。
        @param[in]  in_row             解析するcellの行番号。
        @param[in]  in_attribute_name  解析するcellの属性の名前。
        @param[in]  in_attribute_index 解析するcellの属性のindex番号。
        @param[out] out_succeed
            解析に成功したらtrueが、失敗したらfalseが書き込まれる。
            nullptrの場合は、何も書き込まない。
        @return
            cell文字列を解析して取り出した値。
            解析に失敗したら in_default を返す。
     */
    public: template<typename template_value_type>
    template_value_type deserialize_body_cell(
        template_value_type const& in_default,
        std::size_t const in_row,
        typename self::attribute_map::key_type const& in_attribute_name,
        std::size_t const in_attribute_index = 0,
        bool* const out_succeed = nullptr)
    const
    {
        return psyq::deserialize_string(
            in_default,
            this->find_body_cell(
                in_row, in_attribute_name, in_attribute_index),
            out_succeed);
    }

    /** @brief 文字列表の本体のcell文字列を解析し、値を取り出す。
        @tparam template_value_type 取り出す値の型。
        @param[in]  in_default 解析に失敗した場合の戻り値。
        @param[in]  in_row     解析するcellの行番号。
        @param[in]  in_column  解析するcellの列番号。
        @param[out] out_succeed
            解析に成功したらtrueが、失敗したらfalseが書き込まれる。
            nullptrの場合は、何も書き込まない。
        @return
            cell文字列を解析して取り出した値。
            解析に失敗したら in_default を返す。
     */
    public: template<typename template_value_type>
    template_value_type deserialize_body_cell(
        template_value_type const& in_default,
        std::size_t const in_row,
        std::size_t const in_column,
        bool* const out_succeed = nullptr)
    const
    {
        return psyq::deserialize_string(
            in_default, this->find_body_cell(in_row, in_column), out_succeed);
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表の本体から、cell文字列を検索する。
        @param[in] in_row             検索するcellの行番号。
        @param[in] in_attribute_name  検索するcellの属性の名前。
        @param[in] in_attribute_index 検索するcellの属性のindex番号。
        @retval !=nullptr 見つかったcell文字列。
        @retval ==nullptr 対応するcellが見つからなかった。
     */
    public: typename self::cell_string const* find_body_cell(
        std::size_t const in_row,
        typename self::attribute_map::key_type const& in_attribute_name,
        std::size_t const in_attribute_index = 0)
    const
    {
        if (in_row == this->get_attribute_row())
        {
            return nullptr;
        }
        return self::find_cell(
            this->get_column_map(),
            in_row,
            this->get_attribute_map(),
            in_attribute_name,
            in_attribute_index);
    }

    /** @brief 文字列表の本体から、cell文字列を検索する。
        @param[in] in_row    検索するcellの行番号。
        @param[in] in_column 検索するcellの列番号。
        @retval !=nullptr 見つかったcell文字列。
        @retval ==nullptr 対応するcellが見つからなかった。
     */
    public: typename self::cell_string const* find_body_cell(
        std::size_t const in_row,
        std::size_t const in_column)
    const
    {
        if (in_row == this->get_attribute_row())
        {
            return nullptr;
        }
        return self::find_cell(this->get_column_map(), in_row, in_column);
    }

    /** @brief 文字列表の列の辞書から、cell文字列を検索する。
        @param[in] in_column_map      検索する文字列表の列辞書。
        @param[in] in_row             検索するcellの行番号。
        @param[in] in_attribute_map   検索する文字列表の属性辞書。
        @param[in] in_attribute_name  検索するcellの属性名。
        @param[in] in_attribute_index 検索するcellの属性index番号。
        @retval !=nullptr 見つかったcell文字列。
        @retval ==nullptr 対応するcellが見つからなかった。
     */
    public: static typename self::cell_string const* find_cell(
        typename self::column_map const&              in_column_map,
        std::size_t const                             in_row,
        typename self::attribute_map const&           in_attribute_map,
        typename self::attribute_map::key_type const& in_attribute_name,
        std::size_t const                             in_attribute_index)
    {
        auto const local_attribute_iterator(
            in_attribute_map.find(in_attribute_name));
        if (local_attribute_iterator != in_attribute_map.end())
        {
            auto& local_attribute(local_attribute_iterator->second);
            if (in_attribute_index < local_attribute.size)
            {
                return self::find_cell(
                    in_column_map,
                    in_row,
                    local_attribute.column + in_attribute_index);
            }
        }
        return nullptr;
    }

    /** @brief 文字列表の列の辞書から、cell文字列を検索する。
        @param[in] in_column_map 検索する文字列表の列辞書。
        @param[in] in_row        検索するcellの行番号。
        @param[in] in_column     検索するcellの列番号。
        @retval !=nullptr 見つかったcell文字列。
        @retval ==nullptr 対応するcellが見つからなかった。
     */
    public: static typename self::cell_string const* find_cell(
        typename self::column_map const& in_column_map,
        std::size_t const                in_row,
        std::size_t const                in_column)
    {
        auto const local_row_map(
            self::find_row_map(in_column_map, in_column));
        if (local_row_map != nullptr)
        {
            auto const local_row_iterator(local_row_map->find(in_row));
            if (local_row_iterator != local_row_map->end())
            {
                return &local_row_iterator->second;
            }
        }
        return nullptr;
    }

    /** @brief 文字列表の列の辞書から、行の辞書を検索する。
        @param[in] in_column_map 検索する文字列表の列辞書。
        @param[in] in_column     検索するcellの列番号。
        @retval !=nullptr 見つかった行の辞書。
        @retval ==nullptr 対応する行の辞書が見つからなかった。
     */
    public: static typename self::row_map const* find_row_map(
        typename self::column_map const& in_column_map,
        std::size_t const                in_column)
    {
        auto const local_column_iterator(in_column_map.find(in_column));
        return local_column_iterator != in_column_map.end()?
            &local_column_iterator->second: nullptr;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表の列の辞書から、文字列表の属性の辞書を構築する。
        @param[in] in_column_map    文字列表の列の辞書。
        @param[in] in_attribute_row 属性行の番号。
     */
    public: static typename self::attribute_map make_attribute_map(
        typename self::column_map const&          in_column_map,
        typename self::column_map::key_type const in_attribute_row)
    {
        // 属性の辞書を構築する。
        typename self::attribute_map local_attribute_map(
            in_column_map.get_allocator());
        std::size_t local_column_max(0);
        for (auto& local_column_value: in_column_map)
        {
            // 最大行を更新する。
            if (local_column_max < local_column_value.first)
            {
                local_column_max = local_column_value.first;
            }

            // 属性行を取得する。
            auto& local_row_map(local_column_value.second);
            auto const local_row_iterator(
                local_row_map.find(in_attribute_row));
            if (local_row_iterator == local_row_map.end())
            {
                continue; // 属性行ではなかった。
            }

            // 属性辞書を更新する。
            auto const local_attribute_iterator(
                local_attribute_map.find(local_row_iterator->second));
            if (local_attribute_iterator == local_attribute_map.end())
            {
                local_attribute_map.emplace(
                    local_row_iterator->second,
                    typename self::attribute_map::mapped_type(
                        local_column_value.first, 0));
            }
            else
            {
                PSYQ_ASSERT(false); // 名前が重複する属性がある。
            }
        }

        // 属性ごとの要素数を決定する。
        self::adjust_attribute_size(local_attribute_map, local_column_max);
        return local_attribute_map;
    }

    /** @brief 属性の要素数を決定する。
        @param[in,out] io_attribute_map 要素数を決定する、属性の辞書。
        @param[in]     in_column_max    列番号の最大値。
     */
    private: static void adjust_attribute_size(
        typename self::attribute_map& io_attribute_map,
        std::size_t const             in_column_max)
    {
        if (io_attribute_map.empty())
        {
            return;
        }

        // 属性の配列を構築する。
        typename self::attribute_map::allocator_type::template
            rebind<typename self::attribute_map::value_type*>::other
                local_allocator(io_attribute_map.get_allocator());
        auto const local_attribute_begin(
            local_allocator.allocate(io_attribute_map.size()));
        PSYQ_ASSERT(local_attribute_begin != nullptr);
        auto local_attribute_end(local_attribute_begin);
        for (auto& local_attribute_value: io_attribute_map)
        {
            *local_attribute_end = &local_attribute_value;
            ++local_attribute_end;
        }

        // 属性の配列を、列番号でsortする。
        std::sort(
            local_attribute_begin,
            local_attribute_end,
            [=](
                typename self::attribute_map::value_type const* const in_left,
                typename self::attribute_map::value_type const* const in_right)
            ->bool
            {
                return in_left->second.column < in_right->second.column;
            });

        // 属性の要素数を決定する。
        auto local_last_column(in_column_max + 1);
        for (auto i(local_attribute_end - 1); local_attribute_begin <= i; --i)
        {
            auto& local_attribute((**i).second);
            local_attribute.size = local_last_column - local_attribute.column;
            local_last_column = local_attribute.column;
        }

        // 属性の配列を破棄する。
        local_allocator.deallocate(
            local_attribute_begin, io_attribute_map.size());
    }

    //-------------------------------------------------------------------------
    /** @brief CSV形式の文字列を解析し、文字列表の列の辞書を構築する。
        @tparam template_string std::basic_string 互換の文字列型。
        @param[in] in_csv_string       解析するCSV形式の文字列。
        @param[in] in_column_ceparator 列の区切り文字。
        @param[in] in_row_separator    行の区切り文字。
        @param[in] in_quote_begin      引用符の開始文字。
        @param[in] in_quote_end        引用符の終了文字。
        @param[in] in_quote_escape     引用符のescape文字。
        @return CSV形式の文字列を解析して構築した、文字列表の列の辞書。
     */
    public: template<typename template_string>
    static typename self::column_map make_column_map(
        template_string const&                     in_csv_string,
        typename template_string::value_type const in_column_ceparator = ',',
        typename template_string::value_type const in_row_separator = '\n',
        typename template_string::value_type const in_quote_begin = '"',
        typename template_string::value_type const in_quote_end = '"',
        typename template_string::value_type const in_quote_escape = '"')
    {
        PSYQ_ASSERT(in_quote_escape != 0);
        bool local_quote(false);
        std::size_t local_row(0);
        std::size_t local_column(0);
        std::size_t local_max_column(0);
        template_string local_cell(in_csv_string.get_allocator());
        typename self::column_map local_column_map(
            in_csv_string.get_allocator());
        typename template_string::value_type local_last_char(0);
        for (auto i(in_csv_string.begin()); i != in_csv_string.end(); ++i)
        {
            if (local_quote)
            {
                if (local_last_char != in_quote_escape)
                {
                    if (*i != in_quote_end)
                    {
                        if (*i != in_quote_escape)
                        {
                            local_cell.push_back(*i);
                        }
                        local_last_char = *i;
                    }
                    else
                    {
                        // 引用符を終了。
                        local_quote = false;
                        local_last_char = 0;
                    }
                }
                else if (*i == in_quote_end)
                {
                    // 引用符の終了文字をescapeする。
                    local_cell.push_back(*i);
                    local_last_char = 0;
                }
                else if (local_last_char == in_quote_end)
                {
                    // 引用符を終了し、文字を巻き戻す。
                    local_quote = false;
                    --i;
                    local_last_char = 0;
                }
                else
                {
                    local_cell.push_back(local_last_char);
                    local_cell.push_back(*i);
                    local_last_char = *i;
                }
            }
            else if (*i == in_quote_begin)
            {
                // 引用符の開始。
                local_quote = true;
            }
            else if (*i == in_column_ceparator)
            {
                // 列の区切り。
                if (!local_cell.empty())
                {
                    local_column_map[local_column][local_row]
                        = typename self::cell_string(local_cell);
                    local_cell.clear();
                }
                ++local_column;
            }
            else if (*i == in_row_separator)
            {
                // 行の区切り。
                if (!local_cell.empty())
                {
                    local_column_map[local_column][local_row]
                        = typename self::cell_string(local_cell);
                    local_cell.clear();
                }
                else if (0 < local_column)
                {
                    --local_column;
                }
                if (local_max_column < local_column)
                {
                    local_max_column = local_column;
                }
                local_column = 0;
                ++local_row;
            }
            else
            {
                local_cell.push_back(*i);
            }
        }

        // 最終cellの処理。
        if (local_quote)
        {
            // 引用符の開始はあったが、終了がなかった場合。
            //local_cell.insert(local_cell.begin(), in_quote_begin);
        }
        if (!local_cell.empty())
        {
            local_column_map[local_column][local_row]
                = typename self::cell_string(local_cell);
        }
        else if (0 < local_column)
        {
            --local_column;
        }
        if (local_max_column < local_column)
        {
            local_max_column = local_column;
        }
        return local_column_map;
    }

    //-------------------------------------------------------------------------
    /// 文字列表の属性の辞書。
    private: typename self::attribute_map attribute_map_;
    /// 文字列表の属性行の番号。
    private: std::size_t attribute_row_;
    /// 文字列表の列の辞書。
    private: typename self::column_map column_map_;
};

#endif // !defined(PSYQ_STRING_TABLE_HPP_)
