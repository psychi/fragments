/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

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
 */
/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_STRING_TABLE_HPP_
#define PSYQ_STRING_TABLE_HPP_
#include <unordered_map>

namespace psyq
{
    /// @cond
    struct string_table_attribute;
    struct string_table_key;
    struct string_table_key_hash;
    template<typename, typename> class string_table;
    /// @endcond

    /// 共有文字列の表。
    typedef psyq::string_table<
        std::unordered_map<
            psyq::shared_string,
            psyq::string_table_attribute,
            psyq::shared_string::fnv1_hash>,
        std::unordered_map<
            psyq::string_table_key,
            psyq::shared_string,
            psyq::string_table_key_hash>>
                shared_string_table;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 文字列表の属性の辞書の値の型。
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

/// @brief 文字列表のcellの辞書のキーの型。
struct psyq::string_table_key
{
    string_table_key(
        std::size_t const in_row,
        std::size_t const in_column)
    :
        row(in_row),
        column(in_column)
    {}

    bool operator==(string_table_key const& in_right) const
    {
        return this->row == in_right.row && this->column == in_right.column;
    }

    bool operator!=(string_table_key const& in_right) const
    {
        return !this->operator==(in_right);
    }

    bool operator<(string_table_key const& in_right) const
    {
        if (this->row != in_right.row)
        {
            return this->row < in_right.row;
        }
        return this->column < in_right.column;
    }

    bool operator <=(string_table_key const& in_right) const
    {
        if (this->row != in_right.row)
        {
            return this->row < in_right.row;
        }
        return this->column <= in_right.column;
    }

    bool operator>(string_table_key const& in_right) const
    {
        return in_right.operator<(*this);
    }

    bool operator>=( string_table_key const& in_right) const
    {
        return in_right.operator<=(*this);
    }

    std::size_t row;    ///< cellの行番号。
    std::size_t column; ///< cellの列番号。
};

struct psyq::string_table_key_hash
{
    std::size_t operator()(psyq::string_table_key const& in_key) const
    {
        return (in_key.row << (sizeof(in_key.row) * 4)) ^ in_key.column;
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief CSV形式の文字列から構築できる文字列の表。
    @tparam template_attribute_map @copydoc string_table::attribute_map
    @tparam template_cell_map      @copydoc string_table::cell_map
 */
template<typename template_attribute_map, typename template_cell_map>
class psyq::string_table
{
    /// thisが指す値の型。
    private: typedef string_table<template_attribute_map, template_cell_map>
        self;

    /** @brief 文字列表の属性の辞書。

        以下の条件を満たしている必要がある。
        - std::unorderd_map 相当の型。要素がsortされてなくともよい。
        - attribute_map::key_type は、 psyq::basic_string_piece 互換の文字列型。
        - attribute_map::mapped_type は、 string_table_attribute 型。
     */
    public: typedef template_attribute_map attribute_map;

    /** @brief 文字列表のcellの辞書。

        以下の条件を満たしている必要がある。
        - std::unorderd_map 相当の型。要素がsortされてなくともよい。
        - cell_map::key_type は、 string_table_key 型。
        - cell_map::mapped_type は、 attribute_map::key_type と同じ文字列型。
     */
    public: typedef template_cell_map cell_map;

    /** @brief cellの部分文字列の型。
     */
    public: typedef psyq::basic_string_piece<
        typename self::cell_map::mapped_type::value_type,
        typename self::cell_map::mapped_type::traits_type>
            string_piece;

    //-------------------------------------------------------------------------
    /** @brief 文字列表を構築する。
        @param[in] in_cell_map      元となる文字列表。
        @param[in] in_attribute_row 属性として使う行の番号。
     */
    public: string_table(
        typename self::cell_map in_cell_map,
        std::size_t const       in_attribute_row)
    :
        attribute_map_(in_cell_map.get_allocator()),
        attribute_row_(in_attribute_row),
        cell_map_(std::move(in_cell_map)),
        begin_key_(0, 0),
        end_key_(0, 0)
    {
        // cell辞書の範囲を決定する。
        self::adjust_cell_range(
            this->begin_key_,
            this->end_key_,
            this->cell_map_,
            in_attribute_row);

        // 属性の要素数を調整する。
        self::adjust_attribute_size(this->attribute_map_, this->end_key_.row);
    }

    public: string_table(self&& io_source):
        attribute_map_(std::move(io_source.attribute_map_)),
        attribute_row_(std::move(io_source.attribute_row_)),
        cell_map_(std::move(io_source.cell_map_)),
        begin_key_(std::move(io_source.begin_key_)),
        end_key_(std::move(io_source.end_key_))
    {
        io_source.attribute_row_ = 0;
        io_source.begin_key_.row = 0;
        io_source.begin_key_.column = 0;
        io_source.end_key_ = io_source.begin_key_;
    }

    public: self& operator=(self&& io_source)
    {
        if (this != &io_source)
        {
            this->attribute_map_ = std::move(io_source.attribute_map_);
            this->attribute_row_ = std::move(io_source.attribute_row_);
            this->cell_map_  = std::move(io_source.cell_map_);
            this->begin_key_ = std::move(io_source.begin_key_);
            this->end_key_ = std::move(io_source.end_key_);
            io_source.attribute_row_ = 0;
            io_source.begin_key_.row = 0;
            io_source.begin_key_.column = 0;
            io_source.end_key_ = io_source.begin_key_;
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    private: static void adjust_cell_range(
        typename self::cell_map::key_type& out_begin_key,
        typename self::cell_map::key_type& out_end_key,
        typename self::cell_map const&     in_cell_map,
        std::size_t const                  in_attribute_row)
    {
        bool local_first(true);
        for (auto i(in_cell_map.begin()); i != in_cell_map.end(); ++i)
        {
            auto const& local_key(i->first);
            if (local_key.row == in_attribute_row)
            {
                continue;
            }
            if (local_first)
            {
                local_first = false;
                out_begin_key = local_key;
                out_end_key.row = local_key.row + 1;
                out_end_key.column = local_key.column + 1;
                continue;
            }
            if (local_key.row < out_begin_key.row)
            {
                out_begin_key.row = local_key.row;
            }
            else if (out_end_key.row <= local_key.row)
            {
                out_end_key.row = local_key.row + 1;
            }
            if (local_key.column < out_begin_key.column)
            {
                out_begin_key.column = local_key.column;
            }
            else if (out_end_key.column <= local_key.column)
            {
                out_end_key.column = local_key.column + 1;
            }
        }
    }

    //-------------------------------------------------------------------------
    private: static void adjust_attribute_size(
        typename self::attribute_map& io_attribute_map,
        std::size_t const             in_end_row)
    {
        if (io_attribute_map.empty())
        {
            return;
        }
        typename self::attribute_map::allocator_type::template
            rebind<typename self::attribute_map::value_type*>::other
                local_allocator(io_attribute_map.get_allocator());
        auto const local_attribute_begin(
            local_allocator.allocate(io_attribute_map.size()));
        PSYQ_ASSERT(local_attribute_begin != nullptr);
        auto local_attribute_end(local_attribute_begin);
        for (auto& local_value: io_attribute_map)
        {
            *local_attribute_end = &local_value;
            ++local_attribute_end;
        }
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
        auto local_last_column(in_end_row);
        for (auto i(local_attribute_end - 1); local_attribute_begin <= i; --i)
        {
            auto& local_attribute((**i).second);
            local_attribute.size = local_last_column - local_attribute.column;
            local_last_column = local_attribute.column;
        }
        local_allocator.deallocate(
            local_attribute_begin, io_attribute_map.size());
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表の属性辞書を取得する。
        @return @copydoc attribute_map_
     */
    public: typename self::attribute_map const& get_attribute_map() const
    {
        return this->attribute_map_;
    }

    public: std::size_t get_attribute_row() const
    {
        return this->attribute_row_;
    }

    /** @brief 文字列表のcell辞書を取得する。
        @return @copydoc string_table::cell_map_
     */
    public: typename self::cell_map const& get_cell_map() const
    {
        return this->cell_map_;
    }

    /** @brief 文字列表の行と桁の最小値を取得する。
        @return @copydoc string_table::begin_key_
     */
    public: typename self::cell_map::key_type const& get_begin_key() const
    {
        return this->begin_key_;
    }

    /** @brief 文字列表の行と桁の最大値+1を取得する。
        @return @copydoc string_table::end_key_
     */
    public: typename self::cell_map::key_type const& get_end_key() const
    {
        return this->end_key_;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表からcellを検索する。
        @param[in] in_cell_key 検索するcellのkey。
        @retval !=nullptr 見つかったcell。
        @retval ==nullptr 対応するcellが見つからなかった。
     */
    public: typename self::cell_map::mapped_type const* find_cell(
        typename self::cell_map::key_type const& in_cell_key)
    const
    {
        auto const local_cell(this->get_cell_map().find(in_cell_key));
        if (local_cell == this->get_cell_map().end())
        {
            return nullptr;
        }
        return &local_cell->second;
    }

    /** @brief 文字列表からcellを検索する。
        @param[in] in_row             検索するcellの行番号。
        @param[in] in_attribute_key   検索するcellの属性名。
        @param[in] in_attribute_index 検索するcellの属性index番号。
        @retval !=nullptr 見つかったcell。
        @retval ==nullptr 対応するcellが見つからなかった。
     */
    public: typename self::cell_map::mapped_type const* find_cell(
        std::size_t const                       in_row,
        typename attribute_map::key_type const& in_attribute_key,
        std::size_t const                       in_attribute_index = 0)
    const
    {
        if (in_row == this->attribute_row_)
        {
            return nullptr;
        }
        auto const local_attribute(
            this->get_attribute_map().find(in_attribute_key));
        if (local_attribute == this->get_attribute_map().end() ||
            local_attribute->second.size <= in_attribute_index)
        {
            return nullptr;
        }
        return this->find_cell(
            typename self::cell_map::key_type(
                in_row, local_attribute->second.column + in_attribute_index));
    }

    /** @brief 文字列表のcellから、前後の空白を削除した部分文字列を作る。
        @param[in] in_cell_key 検索するcellのkey。
        @return cellの前後から空白を削除した部分文字列。
     */
    public: typename self::string_piece trim_cell(
        typename self::cell_map::key_type const& in_cell_key)
    const
    {
        auto const local_cell(this->find_cell(in_cell_key));
        if (local_cell == nullptr)
        {
            return typename self::string_piece();
        }
        return typename self::string_piece(*local_cell).trim();
    }

    /** @brief 文字列表のcellから、前後の空白を削除した部分文字列を作る。
        @param[in] in_row             検索するcellの行番号。
        @param[in] in_attribute_key   検索するcellの属性名。
        @param[in] in_attribute_index 検索するcellの属性index番号。
        @return cellの前後から空白を削除した部分文字列。
     */
    public: typename self::string_piece trim_cell(
        std::size_t const                       in_row,
        typename attribute_map::key_type const& in_attribute_key,
        std::size_t const                       in_attribute_index = 0)
    const
    {
        auto const local_cell(
            this->find_cell(in_row, in_attribute_key, in_attribute_index));
        if (local_cell == nullptr)
        {
            return typename self::string_piece();
        }
        return typename self::string_piece(*local_cell).trim();
    }

    //-------------------------------------------------------------------------
    /** @brief CSV形式の文字列を解析し、cellの辞書を構築する。
        @tparam template_string std::basic_string 互換の文字列型。
        @param[in] in_csv_string       解析するCSV形式の文字列。
        @param[in] in_column_ceparator 列の区切り文字。
        @param[in] in_row_separator    行の区切り文字。
        @param[in] in_quote_begin      引用符の開始文字。
        @param[in] in_quote_end        引用符の終了文字。
        @param[in] in_quote_escape     引用符のescape文字。
     */
    public: template<typename template_string>
    static typename self::cell_map make_cell_map(
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
        template_string local_cell;
        typename self::cell_map local_cells;
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
                    self::emplace_cell(
                        local_cells, local_row, local_column, local_cell);
                    local_cell.clear();
                }
                ++local_column;
            }
            else if (*i == in_row_separator)
            {
                // 行の区切り。
                if (!local_cell.empty())
                {
                    self::emplace_cell(
                        local_cells, local_row, local_column, local_cell);
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

        // 引用符の開始はあったが終了がなかった場合。
        if (local_quote)
        {
            //local_cell.insert(local_cell.begin(), in_quote_begin);
        }

        // 最終cellの処理。
        if (!local_cell.empty())
        {
            self::emplace_cell(
                local_cells, local_row, local_column, local_cell);
        }
        else if (0 < local_column)
        {
            --local_column;
        }
        if (local_max_column < local_column)
        {
            local_max_column = local_column;
        }
        auto local_max_row(
            local_cells.empty()? 0: (--local_cells.end())->first.row);
        return local_cells;
    }

    private: template<typename template_string>
    static void emplace_cell(
        typename self::cell_map& io_cells,
        std::size_t const        in_row,
        std::size_t const        in_column,
        template_string const&   in_cell)
    {
        io_cells.emplace(
            typename self::cell_map::key_type(in_row, in_column),
            typename self::cell_map::mapped_type(in_cell));
    }

    //-------------------------------------------------------------------------
    /// 文字列表の属性辞書。
    private: typename self::attribute_map attribute_map_;
    /// 属性名として使う行の番号。
    private: std::size_t attribute_row_;
    /// 文字列表のcell辞書。
    private: typename self::cell_map cell_map_;
    /// 文字列表の行と列の最小値。
    private: typename self::cell_map::key_type begin_key_;
    /// 文字列表の行と列の最大値+1。
    private: typename self::cell_map::key_type end_key_;
};

#endif // !defined(PSYQ_STRING_TABLE_HPP_)
