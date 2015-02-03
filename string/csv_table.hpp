/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::string::csv_table
*/
#ifndef PSYQ_STRING_CSV_TABLE_HPP_
#define PSYQ_STRING_CSV_TABLE_HPP_

#include <cstdint>
#include <algorithm>
#include <vector>
//#include "string/view.hpp"

/// @cond
namespace psyq
{
    namespace string
    {
        template<typename> class csv_table;
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief CSV形式の文字列から構築する、文字列の表。
@tparam template_string @copybrief psyq::string::csv_table::string
*/
template<typename template_string = std::string>
class psyq::string::csv_table
{
    private: typedef csv_table this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// 文字列表で使う、std::basic_string互換の文字列型。
    public: typedef template_string string;

    /// 文字列表で使う、文字列参照の型。
    public: typedef psyq::string::view<
        typename this_type::string::value_type,
        typename this_type::string::traits_type> 
            string_view;

    /// 文字列表で使う、インデクス番号の型。
    public: typedef std::uint32_t index;

    public: enum: typename this_type::index
    {
        /// 空のインデクス番号。
        NULL_INDEX = static_cast<typename this_type::index>(-1),
        /// 列の最大数。
        MAX_COLUMNS = 16384,
    };

    //-------------------------------------------------------------------------
    /** @brief 文字列表のセルの配列。

        - this_type::cell_array::value_type::first_type は、
          文字列表のセルのインデクス番号。
        - this_type::cell_array::value_type::second_type は、
          文字列表のセルが持つ文字列。
     */
    private: typedef std::vector<
        std::pair<typename this_type::index, typename this_type::string_view>,
        typename this_type::string::allocator_type>
            cell_array;

    /// @brief セルのインデクス番号を比較する関数オブジェクト。
    private: struct compare_cell_index
    {
        bool operator()(
            typename csv_table::index const in_left_index,
            typename csv_table::cell_array::value_type const& in_right_cell)
        const
        {
            return in_left_index < in_right_cell.first;
        }
        bool operator()(
            typename csv_table::cell_array::value_type const& in_left_cell,
            typename csv_table::index const in_right_index)
        const
        {
            return in_left_cell.first < in_right_index;
        }
    }; // struct compare_cell_index

    /// @brief 文字列表の列の属性。
    private: struct attribute
    {
        attribute(
            typename csv_table::string_view const& in_name,
            typename csv_table::index const in_column,
            typename csv_table::index const in_size)
        :
        name(in_name),
        column(in_column),
        size(in_size)
        {}

        typename csv_table::string_view name; ///< 属性の名前。
        typename csv_table::index column;     ///< 属性の列番号。
        typename csv_table::index size;       ///< 属性の要素数。
    }; // struct attribute

    /// 属性を名前で比較する関数オブジェクト。
    private: struct compare_attribute
    {
        bool operator()(
            typename csv_table::attribute const& in_left,
            typename csv_table::attribute const& in_right)
        const
        {
            return in_left.name < in_right.name;
        }
        bool operator()(
            typename csv_table::attribute const& in_left,
            typename csv_table::string_view const& in_right)
        const
        {
            return in_left.name < in_right;
        }
        bool operator()(
            typename csv_table::string_view const& in_left,
            typename csv_table::attribute const& in_right)
        const
        {
            return in_left < in_right.name;
        }
    }; // struct compare_attribute

    /// @brief 属性の辞書。
    private: typedef std::vector<
        typename this_type::attribute,
        typename this_type::string::allocator_type>
            attribute_array;

    /** @brief 文字列表の主キーの配列。

        - this_type::primary_key_array::value_type::first_type は、
          文字列表の主キーの値。
        - this_type::primary_key_array::value_type::second_type は、
          文字列表の主キーの行番号。
     */
    private: typedef std::vector<
        std::pair<typename this_type::string_view, typename this_type::index>,
        typename this_type::string::allocator_type>
            primary_key_array;

    /// 文字列表の主キーを比較する関数オブジェクト。
    private: struct compare_primary_key
    {
        bool operator()(
            typename csv_table::primary_key_array::value_type const& in_left,
            typename csv_table::primary_key_array::value_type const& in_right)
        const
        {
            return in_left.first < in_right.first;
        }
        bool operator()(
            typename csv_table::primary_key_array::value_type const& in_left,
            typename csv_table::string_view const& in_right)
        const
        {
            return in_left.first < in_right;
        }
        bool operator()(
            typename csv_table::string_view const& in_left,
            typename csv_table::primary_key_array::value_type const& in_right)
        const
        {
            return in_left < in_right.first;
        }
    }; // struct compare_primary_key

    //-------------------------------------------------------------------------
    /** @brief CSV文字列から、文字列表を構築する。
        @param[in] in_csv_string      CSV文字列。
        @param[in] in_attribute_row   属性行の番号。
        @param[in] in_attribute_name  主キーとして使う列の、属性名。
        @param[in] in_attribute_index 主キーとして使う列の、属性インデクス番号。
     */
    public: csv_table(
        typename this_type::string const& in_csv_string,
        typename this_type::index const in_attribute_row,
        typename this_type::string_view const& in_attribute_name,
        typename this_type::index const in_attribute_index = 0)
    :
    string_buffer_(in_csv_string.get_allocator()),
    cells_(in_csv_string.get_allocator()),
    attributes_(in_csv_string.get_allocator()),
    primary_keys_(in_csv_string.get_allocator()),
    attribute_row_(in_attribute_row),
    primary_key_column_(this_type::NULL_INDEX)
    {
        // セル辞書を構築する。
        this_type::make_cell_map(
            this->string_buffer_, in_csv_string).swap(this->cells_);
        // 属性辞書を構築する。
        this_type::make_attribute_map(
            this->cells_, in_attribute_row, this->get_column_count()).swap(
                this->attributes_);
        // 主キー辞書を構築する。
        this->constraint_primary_key(in_attribute_name, in_attribute_index);
    }

    /** @brief 文字列表をコピー構築する。
        @param[in] in_source コピー元となる文字列表。
     */
    public: csv_table(this_type const& in_source):
    string_buffer_(in_source.string_buffer_.get_allocator()),
    cells_(in_source.cells_.get_allocator()),
    attributes_(in_source.attributes_.get_allocator()),
    primary_keys_(in_source.primary_keys_.get_allocator())
    {
        this->copy(in_source);
    }

    /** @brief 文字列表をコピー代入する。
        @param[in] in_source コピー元となる文字列表。
        @return *this
     */
    public: this_type& operator=(this_type const& in_source)
    {
        this->copy(in_source);
        return *this;
    }

    public: void swap(this_type& io_target)
    {
        this->string_buffer_.swap(io_target.string_buffer_);
        this->cells_.swap(io_target.cells_);
        this->attributes_.swap(io_target.attributes_);
        this->primary_keys_.swap(io_target.primary_keys_);
        std::swap(this->attribute_row_, io_target.attribute_row_);
        std::swap(this->primary_key_column_, io_target.primary_key_column_);
    }

    private: void copy(this_type const& in_source)
    {
        this->string_buffer_ = in_source.string_buffer_;
        this->cells_ = in_source.cells_;
        this->attributes_ = in_source.attributes_;
        this->primary_keys_ = in_source.primary_keys_;
        this->attribute_row_ = in_source.get_attribute_row();
        this->primary_key_column_ = in_source.get_primary_key_column();

        // 文字列参照が、新しい文字列バッファを参照するよう調整する。
        auto const local_buffer_distance(
            this->string_buffer_.data() - in_source.string_buffer_.data());
        for (auto i(this->cells_.begin()); i != this->cells_.end(); ++i)
        {
            auto& local_string(i->second);
            local_string = typename this_type::string_view(
                local_string.data() + local_buffer_distance,
                local_string.size());
        }
        for (auto i(this->attributes_.begin()); i != this->attributes_.end(); ++i)
        {
            auto& local_string(i->name);
            local_string = typename this_type::string_view(
                local_string.data() + local_buffer_distance,
                local_string.size());
        }
        for (auto i(this->primary_keys_.begin()); i != this->primary_keys_.end(); ++i)
        {
            auto& local_string(i->first);
            local_string = typename this_type::string_view(
                local_string.data() + local_buffer_distance,
                local_string.size());
        }
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表の行数を取得する。
        @return 文字列表の行数。
     */
    public: typename this_type::index get_row_count() const
    {
        return this->cells_.empty()?
            0: this->cells_.back().first / this_type::MAX_COLUMNS + 1;
    }

    /** @brief 文字列表の列数を取得する。
        @return 文字列表の列数。
     */
    public: typename this_type::index get_column_count() const
    {
        return this->cells_.empty()?
            0: this->cells_.back().first % this_type::MAX_COLUMNS + 1;
    }

    /** @brief 属性の行番号を取得する。
        @return 属性の行番号。
     */
    public: typename this_type::index get_attribute_row() const
    {
        return this->attribute_row_;
    }

    /** @brief 主キーの列番号を取得する。
        @return 主キーの列番号。
     */
    public: typename this_type::index get_primary_key_column() const
    {
        return this->primary_key_column_;
    }

    //-------------------------------------------------------------------------
    /** @brief 主キーと属性から、文字列表の本体セルを検索する。
        @param[in] in_primary_key     検索する主キー。
        @param[in] in_attribute_name  検索する属性の名前。
        @param[in] in_attribute_index 検索する属性のインデックス番号。
     */
    public: typename this_type::string_view find_body_cell(
        typename this_type::string_view const& in_primary_key,
        typename this_type::string_view const& in_attribute_name,
        typename this_type::index const in_attribute_index = 0)
    const
    {
        return this->find_body_cell(
            this->find_row_index(in_primary_key),
            this->find_column_index(in_attribute_name, in_attribute_index));
    }

    /** @brief 行番号と属性から、文字列表の本体セルを検索する。
        @param[in] in_row_index       検索する行番号。
        @param[in] in_attribute_name  検索する属性の名前。
        @param[in] in_attribute_index 検索する属性のインデックス番号。
     */
    public: typename this_type::string_view find_body_cell(
        typename this_type::index const in_row_index,
        typename this_type::string_view const& in_attribute_name,
        typename this_type::index const in_attribute_index = 0)
    const
    {
        return this->find_body_cell(
            in_row_index,
            this->find_column_index(in_attribute_name, in_attribute_index));
    }

    /** @brief 主キーと列番号から、文字列表の本体セルを検索する。
        @param[in] in_primary_key  検索する主キー。
        @param[in] in_column_index 検索する列番号。
     */
    public: typename this_type::string_view find_body_cell(
        typename this_type::string_view const& in_primary_key,
        typename this_type::index const in_column_index)
    const
    {
        return this->find_body_cell(
            this->find_row_index(in_primary_key), in_column_index);
    }

    /** @brief 行番号と属性から、文字列表の本体セルを検索する。
        @param[in] in_row_index    検索する行番号。
        @param[in] in_column_index 検索する列番号。
        @return
            行番号と列番号に対応する本体セル。
            対応する本体セルがない場合は、空文字列を返す。
     */
    public: typename this_type::string_view find_body_cell(
        typename this_type::index const in_row_index,
        typename this_type::index const in_column_index)
    const
    {
        if (in_row_index != this->get_attribute_row())
        {
            auto const local_cell_index(
                this_type::compute_cell_index(in_row_index, in_column_index));
            auto const local_position(
                std::lower_bound(
                    this->cells_.begin(),
                    this->cells_.end(),
                    local_cell_index,
                    typename this_type::compare_cell_index()));
            if (local_position != this->cells_.end()
                && local_position->first == local_cell_index)
            {
                return local_position->second;
            }
        }
        return typename this_type::string_view();
    }

    //-------------------------------------------------------------------------
    /** @brief 主キーから、文字列表の行番号を検索する。
        @param[in] in_primary_key  検索する主キー。
        @retval !=NULL_INDEX 主キーに対応する行番号。
        @retval ==NULL_INDEX 主キーに対応する行番号が存在しない。
     */
    public: typename this_type::index find_row_index(
        typename this_type::string_view const& in_primary_key)
    const
    {
        auto const local_primary_key(
            std::lower_bound(
                this->primary_keys_.begin(),
                this->primary_keys_.end(),
                in_primary_key,
                typename this_type::compare_primary_key()));
        return local_primary_key != this->primary_keys_.end()
            && local_primary_key->first == in_primary_key?
                local_primary_key->second: this_type::NULL_INDEX;
    }

    /** @brief 属性名から、文字列表の列番号を検索する。
        @param[in] in_attribute_name  検索する属性の名前。
        @param[in] in_attribute_index 検索する属性のインデックス番号。
        @retval !=NULL_INDEX 属性名に対応する列番号。
        @retval ==NULL_INDEX 属性名に対応する列番号が存在しない。
     */
    public: typename this_type::index find_column_index(
        typename this_type::string_view const& in_attribute_name,
        typename this_type::index const in_attribute_index = 0)
    const
    {
        if (in_attribute_name.empty())
        {
            return in_attribute_index < this->get_column_count()?
                in_attribute_index: this_type::NULL_INDEX;
        }
        auto const local_attribute(
            std::lower_bound(
                this->attributes_.begin(),
                this->attributes_.end(),
                in_attribute_name,
                typename this_type::compare_attribute()));
        return local_attribute != this->attributes_.end()
            && local_attribute->name == in_attribute_name
            && in_attribute_index < local_attribute->size?
                local_attribute->column + in_attribute_index:
                this_type::NULL_INDEX;
    }

    /** @brief 主キーを決定する。
        @param[in] in_attribute_name  主キーとする属性の名前。
        @param[in] in_attribute_index 主キーとする属性のインデックス番号。
     */
    public: bool constraint_primary_key(
        typename this_type::string_view const& in_attribute_name,
        typename this_type::index const in_attribute_index = 0)
    {
        return this->constraint_primary_key(
            this->find_column_index(in_attribute_name, in_attribute_index));
    }

    /** @brief 主キーを決定する。
        @param[in] in_column_index 主キーとする列番号。
     */
    public: bool constraint_primary_key(
        typename this_type::index const in_column_index)
    {
        if (this->get_column_count() <= in_column_index)
        {
            return false;
        }
        auto local_primary_keys(
            this_type::make_primary_key_map(
                this->cells_, in_column_index, this->get_attribute_row()));
        this->primary_keys_.swap(local_primary_keys);
        this->primary_key_column_ = in_column_index;
        return true;
    }

    //-------------------------------------------------------------------------
    /** @brief CSV形式の文字列を解析し、セルの辞書を構築する。
        @param[in] in_csv_string       解析するCSV形式の文字列。
        @param[in] in_column_ceparator 列の区切り文字。
        @param[in] in_row_separator    行の区切り文字。
        @param[in] in_quote_begin      引用符の開始文字。
        @param[in] in_quote_end        引用符の終了文字。
        @param[in] in_quote_escape     引用符のescape文字。
        @return CSV形式の文字列を解析して構築した、セルの辞書。
     */
    private: static typename this_type::cell_array make_cell_map(
        typename this_type::string& out_string_buffer,
        typename this_type::string_view const& in_csv_string,
        typename this_type::string_view::value_type const in_column_ceparator = ',',
        typename this_type::string_view::value_type const in_row_separator = '\n',
        typename this_type::string_view::value_type const in_quote_begin = '"',
        typename this_type::string_view::value_type const in_quote_end = '"',
        typename this_type::string_view::value_type const in_quote_escape = '"')
    {
        PSYQ_ASSERT(in_quote_escape != 0);
        bool local_quote(false);
        typename this_type::index local_row(0);
        typename this_type::index local_column(0);
        typename this_type::index local_max_column(0);
        typename this_type::string_view::value_type local_last_char(0);
        typename this_type::string local_cell_string(
            out_string_buffer.get_allocator());
        typename this_type::cell_array local_cells(
            out_string_buffer.get_allocator());
        typename this_type::string local_string_buffer(
            out_string_buffer.get_allocator());
        local_string_buffer.reserve(in_csv_string.size());
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
                            local_cell_string.push_back(*i);
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
                    // 引用符の終了文字をエスケープする。
                    local_cell_string.push_back(*i);
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
                    local_cell_string.push_back(local_last_char);
                    local_cell_string.push_back(*i);
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
                if (!local_cell_string.empty())
                {
                    this_type::add_cell(
                        local_cells,
                        local_string_buffer,
                        local_row,
                        local_column,
                        local_cell_string);
                    local_cell_string.clear();
                }
                ++local_column;
            }
            else if (*i == in_row_separator)
            {
                // 行の区切り。
                if (!local_cell_string.empty())
                {
                    this_type::add_cell(
                        local_cells,
                        local_string_buffer,
                        local_row,
                        local_column,
                        local_cell_string);
                    local_cell_string.clear();
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
                local_cell_string.push_back(*i);
            }
        }

        // 最終セルの処理。
        if (local_quote)
        {
            // 引用符の開始はあったが、終了がなかった場合。
            //local_cell_string.insert(local_cell_string.begin(), in_quote_begin);
        }
        if (!local_cell_string.empty())
        {
            this_type::add_cell(
                local_cells,
                local_string_buffer,
                local_row,
                local_column,
                local_cell_string);
        }
        else if (0 < local_column)
        {
            --local_column;
        }
        if (local_max_column < local_column)
        {
            local_max_column = local_column;
        }

        // テーブル文字列を必要最小限にする。
        out_string_buffer = local_string_buffer;
        auto const local_buffer_distance(
            out_string_buffer.data() - local_string_buffer.data());
        for (auto i(local_cells.begin()); i != local_cells.end(); ++i)
        {
            auto& local_string(i->second);
            local_string = typename this_type::string_view(
                local_string.data() + local_buffer_distance,
                local_string.size());
        }

        // 列数を記録したセルを末尾に追加する。
        local_cells.push_back(
            typename this_type::cell_array::value_type(
                this_type::compute_cell_index(local_row, local_max_column),
                typename this_type::string_view()));
        return typename this_type::cell_array(local_cells);
    }

    private: static void add_cell(
        typename this_type::cell_array& io_cells,
        typename this_type::string& io_table_string,
        typename this_type::index const in_row_index,
        typename this_type::index const in_column_index,
        typename this_type::string const& in_cell_string)
    {
        auto local_position(io_table_string.find(in_cell_string));
        if (local_position == this_type::string::npos)
        {
            if (io_table_string.capacity() - io_table_string.size() < in_cell_string.size())
            {
                // 予約しておいた容量より大きくなるはずはない。
                PSYQ_ASSERT(false);
                return;
            }
            local_position = io_table_string.size();
            io_table_string.append(in_cell_string);
        }
        io_cells.push_back(
            typename this_type::cell_array::value_type(
                this_type::compute_cell_index(in_row_index, in_column_index),
                typename this_type::string_view(
                    io_table_string.data() + local_position,
                    in_cell_string.size())));
    }

    /** @brief 属性辞書を構築する。
        @param[in] in_cells         文字列表のセル辞書。
        @param[in] in_attribute_row 属性として使う行の番号。
        @param[in] in_num_columns   属性行の列数。
        @return セル辞書から構築した属性辞書。
     */
    private: static typename this_type::attribute_array make_attribute_map(
        typename this_type::cell_array const& in_cells,
        typename this_type::index const in_attribute_row,
        typename this_type::index const in_num_columns)
    {
        // 属性行の先頭位置と末尾インデクスを決定する。
        auto const local_attribute_begin(
            std::lower_bound(
                in_cells.begin(),
                in_cells.end(),
                this_type::compute_cell_index(in_attribute_row, 0),
                typename this_type::compare_cell_index()));
        auto const local_attribute_end(
            this_type::compute_cell_index(in_attribute_row + 1, 0));

        // 属性行を読み取り、属性配列を構築する。
        typename this_type::attribute_array local_attributes(
            in_cells.get_allocator());
        for (
            auto i(local_attribute_begin);
            i != in_cells.end() && i->first < local_attribute_end;
            ++i)
        {
            auto const local_column_index(i->first % this_type::MAX_COLUMNS);
            if (!local_attributes.empty())
            {
                // 直前の属性の要素数を決定する。
                auto& local_back(local_attributes.back());
                local_back.size = local_column_index - local_back.column;
            }
            local_attributes.push_back(
                typename this_type::attribute(
                    i->second, local_column_index, 0));
        }

        // 属性配列を並び替え、属性辞書として正規化する。
        if (!local_attributes.empty())
        {
            // 末尾の属性の要素数を決定する。
            auto& local_back(local_attributes.back());
            local_back.size = local_back.column < in_num_columns?
                in_num_columns - local_back.column: 1;
            // 属性名で並び替える。
            std::sort(
                local_attributes.begin(),
                local_attributes.end(),
                typename this_type::compare_attribute());
        }
        return typename this_type::attribute_array(local_attributes);
    }

    /** @brief 主キーの辞書を構築する。
        @param[in] in_cells          文字列表のセル辞書。
        @param[in] in_primary_column 主キーとして使う列の番号。
        @param[in] in_attribute_row  属性として使う行の番号。
        @return セル辞書から構築した主キーの辞書。
     */
    private: static typename this_type::primary_key_array make_primary_key_map(
        typename this_type::cell_array const& in_cells,
        typename this_type::index const in_primary_column,
        typename this_type::index const in_attribute_row)
    {
        if (in_cells.empty())
        {
            return typename this_type::primary_key_array();
        }

        // セル辞書から主キーの列を読み取り、主キーの配列を構築する。
        typename this_type::primary_key_array local_primary_keys(
            in_cells.get_allocator());
        auto local_row_index(in_cells.front().first / this_type::MAX_COLUMNS);
        auto local_cell(in_cells.begin());
        for (;;)
        {
            if (local_row_index == in_attribute_row)
            {
                ++local_row_index;
                continue;
            }
            local_cell = std::lower_bound(
                local_cell,
                in_cells.end(), 
                this_type::compute_cell_index(
                    local_row_index, in_primary_column),
                typename this_type::compare_cell_index());
            if (local_cell == in_cells.end())
            {
                break;
            }
            if (local_cell->first % this_type::MAX_COLUMNS == in_primary_column)
            {
                local_primary_keys.push_back(
                    typename this_type::primary_key_array::value_type(
                        local_cell->second, local_row_index));
            }
            local_row_index = local_cell->first / this_type::MAX_COLUMNS + 1;
        }

        // 主キーの配列を並び替え、主キーの辞書として正規化する。
        std::sort(
            local_primary_keys.begin(),
            local_primary_keys.end(),
            typename this_type::compare_primary_key());
        return typename this_type::primary_key_array(local_primary_keys);
    }

    /** @brief セルの行番号と列番号から、セルのインデクス番号を算出する。
        @param[in] in_row_index    セルの行番号。
        @param[in] in_column_index セルの列番号。
        @return セルのインデクス番号。
     */
    private: static typename this_type::index compute_cell_index(
        typename this_type::index const in_row_index,
        typename this_type::index const in_column_index)
    {
        PSYQ_ASSERT(in_column_index < this_type::MAX_COLUMNS);
        return in_row_index * this_type::MAX_COLUMNS + in_column_index;
    }

    //-------------------------------------------------------------------------
    private: typename this_type::string string_buffer_;
    /// セルのインデクス番号でソート済のセルの辞書。
    private: typename this_type::cell_array cells_;
    /// 属性名でソート済の属性の辞書。
    private: typename this_type::attribute_array attributes_;
    /// 主キーでソート済の主キーの辞書。
    private: typename this_type::primary_key_array primary_keys_;
    /// 属性として使っている行の番号。
    private: typename this_type::index attribute_row_;
    /// 主キーとして使っている列の番号。
    private: typename this_type::index primary_key_column_;

}; // class psyq::string::csv_table

#endif // !defined(PSYQ_STRING_CSV_TABLE_HPP_)
