/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_CSV_TABLE_HPP_
#define PSYQ_CSV_TABLE_HPP_

namespace psyq
{
    /// @cond
    struct csv_table_key;
    template<typename template_cell_map> class csv_table;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief CSV cellの辞書に使うキーの型。
struct psyq::csv_table_key
{
    /** @param[in] in_row    @copydoc key::row
        @param[in] in_column @copydoc key::column
     */
    csv_table_key(
        std::size_t const in_row,
        std::size_t const in_column)
    :
        row(in_row),
        column(in_column)
    {}

    bool operator<(
        csv_table_key const& in_right)
    const
    {
        if (this->column != in_right.column)
        {
            return this->column < in_right.column;
        }
        return this->row < in_right.row;
    }

    bool operator <=(
        csv_table_key const& in_right)
    const
    {
        if (this->column != in_right.column)
        {
            return this->column <= in_right.column;
        }
        return this->row <= in_right.row;
    }

    bool operator>(
        csv_table_key const& in_right)
    const
    {
        return in_right.operator<(*this);
    }

    bool operator>=(
        csv_table_key const& in_right)
    const
    {
        return in_right.operator<=(*this);
    }

    std::size_t row;    ///< cellの行番号。
    std::size_t column; ///< cellの列番号。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_cell_map>
class psyq::csv_table
{
    private: typedef csv_table self;

    /** @brief CSV-cellの辞書。

        以下の条件を満たしている必要がある。
        - std::map 互換の型。
        - cell_map::key_type は、 csv_table_key 型。
        - cell_map::mapped_type は、 std::basic_string
          互換の文字列型の引数を1つだけ受け取るconstructorを持つ型。
     */
    public: typedef template_cell_map cell_map;

    //-------------------------------------------------------------------------
    /** @brief CSV形式の文字列を解析し、辞書に変換する。
        @tparam template_string std::basic_string 互換の文字列型。
        @param[in] in_csv_string       解析するCSV形式の文字列。
        @param[in] in_column_ceparator 列の区切り文字。
        @param[in] in_row_separator    行の区切り文字。
        @param[in] in_quote_begin      引用符の開始文字。
        @param[in] in_quote_end        引用符の終了文字。
        @param[in] in_quote_escape     引用符のescape文字。
     */
    template<typename template_string>
    public: static self::cell_map parse(
        template_string const&                     in_csv_string,
        typename template_string::value_type const in_column_ceparator = ',',
        typename template_string::value_type const in_row_separator = '\n',
        typename template_string::value_type const in_quote_begin = '"',
        typename template_string::value_type const in_quote_end = '"',
        typename template_string::value_type const in_quote_escape = '"')
    {
        PSYQ_ASSERT(in_quote_escape != 0);
        bool local_quote(false);
        unsigned local_row(0);
        unsigned local_column(0);
        template_string local_field;
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
                            local_field.push_back(*i);
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
                    local_field.push_back(*i);
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
                    local_field.push_back(local_last_char);
                    local_field.push_back(*i);
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
                if (!local_field.empty())
                {
                    self::emplace_cell(
                        local_cells,
                        local_row,
                        local_column,
                        std::move(local_field));
                    local_field.clear();
                }
                ++local_column;
            }
            else if (*i == in_row_separator)
            {
                // 行の区切り。
                if (!local_field.empty())
                {
                    self::emplace_cell(
                        local_cells,
                        local_row,
                        local_column,
                        std::move(local_field));
                    local_field.clear();
                }
                ++local_row;
                local_column = 0;
            }
            else
            {
                local_field.push_back(*i);
            }
        }

        // 引用符の開始はあったが終了がなかった場合。
        if (local_quote)
        {
            //local_field.insert(local_field.begin(), in_quote_begin);
        }

        // 最終fieldの処理。
        if (!local_field.empty())
        {
            self::emplace_cell(
                local_cells, local_row, local_column, std::move(local_field));
        }
        return local_cells;
    }
 
    template<typename template_string>
    private: static void emplace_cell(
        self::cell_map&   io_cells,
        std::size_t const in_row,
        std::size_t const in_column,
        template_string   in_field)
    {
        io_cells.emplace(
            typename self::cell_map::value_type(
                typename self::cell_map::key_type(local_row, local_column),
                typename self::cell_map::mapped_type(std::move(in_field))));
    }

   //-------------------------------------------------------------------------
    static std::size_t get_row_size(
        cell_map const& in_cells)
    {
        if (in_cells.empty())
        {
            return 0;
        }
        std::size_t local_max_row(0);
        std::size_t local_column(in_cells.begin()->first.column + 1);
        for (;;)
        {
            auto local_last_cell(
                in_cells.lower_bound(
                    typename cell_map::key_type(0, local_column)));
            const auto local_position(local_last_cell);
            --local_last_cell;
            if (local_max_row < local_last_cell->first.row)
            {
                local_max_row = local_last_cell->first.row;
            }

            if (local_position != in_cells.end())
            {
                local_column = local_last_cell->first.column + 1;
            }
            else
            {
                return local_max_row + 1;
            }
        }
    }

    static std::size_t get_column_size(
        cell_map const& in_cells)
    {
        return in_cells.empty()? 0: (--in_cells.end())->first.column + 1;
    }

    static std::size_t get_column_size(
        cell_map const&   in_cells,
        std::size_t const in_row);

    static cell_map::const_iterator find(
        cell_map const&                in_cells,
        std::size_t const              in_row,
        attribute_map const&           in_attribute_map,
        attribute_map::key_type const& in_attribute,
        std::size_t const              in_attribute_index)
    {
        const auto local_attribute_iterator(
            in_attribute_map.find(in_attribute));
        if (local_attribute_iterator == in_attribute_map.end() ||
            local_attribute_iterator->second.size <= in_attribute_index)
        {
            return in_cells.end();
        }
        return in_cells.find(
            typename cell_map::key_type(
                in_row,
                local_attribute_iterator->second.index + in_attribute_index));
    }

};

#endif // PSYQ_CSV_TABLE_HPP_
