/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_CSV_TABLE_HPP_
#define PSYQ_CSV_TABLE_HPP_
#include <tuple>

namespace psyq
{
    /// @cond
    struct csv_table_attribute;
    struct csv_table_key;
    template<typename, typename> class csv_table;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief CSV表の属性の辞書の値の型。
struct psyq::csv_table_attribute
{
    csv_table_attribute(
        std::size_t const in_column,
        std::size_t const in_size)
    :
        column(in_column),
        size(in_size)
    {}

    std::size_t column; ///< 属性の列番号。
    std::size_t size;   ///< 属性の要素数。
};

/// @brief CSV表のcellの辞書のキーの型。
struct psyq::csv_table_key
{
    csv_table_key(
        std::size_t const in_row,
        std::size_t const in_column)
    :
        row(in_row),
        column(in_column)
    {}

    bool operator<(csv_table_key const& in_right) const
    {
        if (this->row != in_right.row)
        {
            return this->row < in_right.row;
        }
        return this->column < in_right.column;
    }

    bool operator <=(csv_table_key const& in_right) const
    {
        if (this->row != in_right.row)
        {
            return this->row < in_right.row;
        }
        return this->column <= in_right.column;
    }

    bool operator>(csv_table_key const& in_right) const
    {
        return in_right.operator<(*this);
    }

    bool operator>=( csv_table_key const& in_right) const
    {
        return in_right.operator<=(*this);
    }

    std::size_t row;    ///< cellの行番号。
    std::size_t column; ///< cellの列番号。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief CSV形式の文字列から構築する表。
    @tparam template_attribute_map @copydoc csv_table::attribute_map
    @tparam template_cell_map      @copydoc csv_table::cell_map
 */
template<typename template_attribute_map, typename template_cell_map>
class psyq::csv_table
{
    /// thisが指す値の型。
    private: typedef csv_table<template_attribute_map, template_cell_map> self;

    /** @brief CSV表の属性の辞書。

        以下の条件を満たしている必要がある。
        - std::map 互換の型。
        - attribute_map::key_type は、 psyq::const_string 互換の文字列型。
        - attribute_map::mapped_type は、 csv_table_attribute 型。
     */
    public: typedef template_attribute_map attribute_map;

    /** @brief CSV表のcellの辞書。

        以下の条件を満たしている必要がある。
        - std::map 互換の型。
        - cell_map::key_type は、 csv_table_key 型。
        - cell_map::mapped_type は、 attribute_map::key_type 互換の文字列型。
     */
    public: typedef template_cell_map cell_map;

    //-------------------------------------------------------------------------
    /** @brief CSV形式の文字列を解析し、CSV表に変換する。
        @tparam template_string std::basic_string 互換の文字列型。
        @param[in] in_csv_string       解析するCSV形式の文字列。
        @param[in] in_attribute_row    CSV表の属性として使う行の番号。
        @param[in] in_column_ceparator 列の区切り文字。
        @param[in] in_row_separator    行の区切り文字。
        @param[in] in_quote_begin      引用符の開始文字。
        @param[in] in_quote_end        引用符の終了文字。
        @param[in] in_quote_escape     引用符のescape文字。
     */
    public: template<typename template_string>
    explicit csv_table(
        template_string const&                     in_csv_string,
        std::size_t const                          in_attribute_row = 0,
        typename template_string::value_type const in_column_ceparator = ',',
        typename template_string::value_type const in_row_separator = '\n',
        typename template_string::value_type const in_quote_begin = '"',
        typename template_string::value_type const in_quote_end = '"',
        typename template_string::value_type const in_quote_escape = '"')
    :
        attribute_row_(in_attribute_row)
    {
        // CSV表のcell辞書を構築する。
        auto local_make_cell_map_result(
            self::make_cell_map(
                in_csv_string,
                in_column_ceparator,
                in_row_separator,
                in_quote_begin,
                in_quote_end,
                in_quote_escape));
        this->cell_map_ = std::move(std::get<0>(local_make_cell_map_result));
        this->max_row_ = std::get<1>(local_make_cell_map_result);
        this->max_column_ = std::get<2>(local_make_cell_map_result);

        // CSV表の属性辞書を構築する。
        this->attribute_map_ = self::make_attribute_map(
            this->get_cell_map(),
            this->get_attribute_row(),
            this->get_max_column());
    }

    //-------------------------------------------------------------------------
    /** @brief CSV表の行番号の最大値を取得する。
        @return @copydoc csv_table::max_row_
     */
    public: std::size_t get_max_row() const
    {
        return this->max_row_;
    }

    /** @brief CSV表の桁番号の最大値を取得する。
        @return @copydoc csv_table::max_column_
     */
    public: std::size_t get_max_column() const
    {
        return this->max_column_;
    }

    /** @brief CSV表の属性として使っている行の番号を取得する。
        @return @copydoc csv_table::attribute_row_
     */
    public: std::size_t get_attribute_row() const
    {
        return this->attribute_row_;
    }

    /** @brief CSV表の属性辞書を取得する。
        @return @copydoc csv_table::attribute_map_
     */
    public: typename self::attribute_map const& get_attribute_map() const
    {
        return this->attribute_map_;
    }

    /** @brief CSV表のcell辞書を取得する。
        @return @copydoc csv_table::cell_map_
     */
    public: typename self::cell_map const& get_cell_map() const
    {
        return this->cell_map_;
    }

    //-------------------------------------------------------------------------
    /** @brief CSV表からcellを検索する。
        @param[in] in_row    検索するcellの行番号。
        @param[in] in_column 検索するcellの列番号。
        @retval !=nullptr 見つかったcell。
        @retval ==nullptr 対応するcellが見つからなかった。
     */
    public: typename self::cell_map::mapped_type const* find_cell(
        std::size_t const in_row,
        std::size_t const in_column)
    const
    {
        if (in_row != this->get_attribute_row())
        {
            auto const local_cell(
                this->get_cell_map().find(
                    typename self::cell_map::key_type(in_row, in_column)));
            if (local_cell != this->get_cell_map().end())
            {
                return &local_cell->second;
            }
        }
        return nullptr;
    }

    /** @brief CSV表からcellを検索する。
        @param[in] in_row             検索するcellの行番号。
        @param[in] in_attribute_key   検索するcellの属性名。
        @param[in] in_attribute_index 検索するcellの属性index番号。
        @retval !=nullptr 見つかったcell。
        @retval ==nullptr 対応するcellが見つからなかった。
     */
    public: typename self::cell_map::mapped_type const* find_cell(
        std::size_t const                       in_row,
        typename attribute_map::key_type const& in_attribute_key,
        std::size_t const                       in_attribute_index)
    const
    {
        auto const local_attribute(
            this->get_attribute_map().find(in_attribute_key));
        if (local_attribute == this->get_attribute_map().end() ||
            local_attribute->second.size <= in_attribute_index)
        {
            return nullptr;
        }
        return this->find_cell(
            in_row, local_attribute->second.column + in_attribute_index);
    }

    //-------------------------------------------------------------------------
    /** @brief CSV表の属性の辞書を作る。

        @param[in] in_cells      解析するCSV表。
        @param[in] in_row        属性として使う行の番号。
        @param[in] in_max_column CSV表の桁数の最大値。
     */
    private: static typename self::attribute_map make_attribute_map(
        typename self::cell_map const& in_cells,
        std::size_t const              in_row,
        std::size_t const              in_max_column)
    {
        typename self::attribute_map local_attributes;
        auto local_last_attribute(local_attributes.end());
        for (
            auto i(
                in_cells.lower_bound(
                    typename self::cell_map::key_type(in_row, 0)));
            i != in_cells.end() && i->first.row == in_row;
            ++i)
        {
            if (!local_attributes.empty())
            {
                local_last_attribute->second.size =
                    i->first.column - local_last_attribute->second.column;
            }
            auto const local_emplace_result(
                local_attributes.emplace(
                    typename self::attribute_map::key_type(i->second),
                    typename self::attribute_map::mapped_type(
                        i->first.column, 0)));
            if (std::get<1>(local_emplace_result))
            {
                local_last_attribute = std::get<0>(local_emplace_result);
            }
        }
        if (!local_attributes.empty())
        {
            local_last_attribute->second.size =
                1 + in_max_column - local_last_attribute->second.column;
        }
        return local_attributes;
    }

    //-------------------------------------------------------------------------
    /** @brief CSV形式の文字列を解析し、cellの辞書に変換する。
        @tparam template_string std::basic_string 互換の文字列型。
        @param[in] in_csv_string       解析するCSV形式の文字列。
        @param[in] in_column_ceparator 列の区切り文字。
        @param[in] in_row_separator    行の区切り文字。
        @param[in] in_quote_begin      引用符の開始文字。
        @param[in] in_quote_end        引用符の終了文字。
        @param[in] in_quote_escape     引用符のescape文字。
     */
    private: template<typename template_string>
    static std::tuple<typename self::cell_map, std::size_t, std::size_t>
    make_cell_map(
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
        template_string local_field;
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
                        local_cells, local_row, local_column, local_field);
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
                        local_cells, local_row, local_column, local_field);
                    local_field.clear();
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
                local_cells, local_row, local_column, local_field);
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
        return std::tuple<typename self::cell_map, std::size_t, std::size_t>(
            std::move(local_cells),
            std::move(local_max_row),
            std::move(local_max_column));
    }

    private: template<typename template_string>
    static void emplace_cell(
        typename self::cell_map& io_cells,
        std::size_t const        in_row,
        std::size_t const        in_column,
        template_string const&   in_field)
    {
        io_cells.emplace(
            typename self::cell_map::key_type(in_row, in_column),
            typename self::cell_map::mapped_type(in_field));
    }

    //-------------------------------------------------------------------------
    /// CSV表の属性辞書。
    private: typename self::attribute_map attribute_map_;
    /// CSV表のcell辞書。
    private: typename self::cell_map cell_map_;
    /// CSV表の属性として使っている行の番号。
    private: std::size_t attribute_row_;
    /// CSV表の行番号の最大値。
    private: std::size_t max_row_;
    /// CSV表の桁番号の最大値。
    private: std::size_t max_column_;
};

#endif // PSYQ_CSV_TABLE_HPP_
