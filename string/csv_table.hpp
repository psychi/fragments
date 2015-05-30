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
        template<typename, typename> class csv_table;
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief CSV形式の文字列から構築する、文字列の表。
    @tparam template_string    @copybrief psyq::string::csv_table::string
    @tparam template_allocator @copybrief psyq::string::csv_table::allocator_type
*/
template<
    typename template_string = std::string,
    typename template_allocator = typename template_string::allocator_type>
class psyq::string::csv_table
{
    private: typedef csv_table this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// @brief 文字列表で使う、std::basic_string互換の文字列型。
    public: typedef template_string string;

    /// @brief メモリ割当子を表す型。
    public: typedef template_allocator allocator_type;

    /// @brief 文字列表で使う、文字列参照の型。
    public: typedef psyq::string::view<
        typename this_type::string::value_type,
        typename this_type::string::traits_type> 
            string_view;

    /// @brief 文字列表で使う、インデクス番号の型。
    public: typedef std::uint32_t index_type;

    public: enum: typename this_type::index_type
    {
        /// @brief 空のインデクス番号。
        NULL_INDEX = static_cast<typename this_type::index_type>(-1),
        /// @brief 列の最大数。
        MAX_COLUMN_COUNT = 16384,
        /// @brief 行の最大数。
        MAX_ROW_COUNT = NULL_INDEX / MAX_COLUMN_COUNT + 1,
    };

    //-------------------------------------------------------------------------
    /// @brief CSV文字列で使う区切り文字。
    public: struct delimiter
    {
        typedef typename csv_table::string::allocator_type allocator_type;

        /** @brief 区切り文字を構築する。
            @param[in] in_allocator CSV文字列で使うメモリ割当子。
         */
        explicit delimiter(allocator_type const& in_allocator = allocator_type())
        :
        row_separator(PSYQ_STRING_CSV_TABLE_ROW_SEPARATOR_DEFAULT),
        column_separator(PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_DEFAULT),
        quote_begin(PSYQ_STRING_CSV_TABLE_QUOTE_BEGIN_DEFAULT),
        quote_end(PSYQ_STRING_CSV_TABLE_QUOTE_END_DEFAULT),
        quote_escape(PSYQ_STRING_CSV_TABLE_QUOTE_ESCAPE_DEFAULT),
        allocator(in_allocator)
        {}

        /** @brief 区切り文字を構築する。
            @param[in] in_row_separator    CSV文字列の行の区切り文字。
            @param[in] in_column_separator CSV文字列の列の区切り文字。
            @param[in] in_quote_begin      CSV文字列の引用符の開始文字。
            @param[in] in_quote_end        CSV文字列の引用符の終了文字。
            @param[in] in_quote_escape     CSV文字列の引用符のエスケープ文字。
            @param[in] in_allocator        CSV文字列で使うメモリ割当子。
         */
        delimiter(
            typename csv_table::string::value_type const in_row_separator,
            typename csv_table::string::value_type const in_column_separator,
            typename csv_table::string::value_type const in_quote_begin,
            typename csv_table::string::value_type const in_quote_end,
            typename csv_table::string::value_type const in_quote_escape,
            allocator_type const& in_allocator = allocator_type())
        :
        row_separator(in_row_separator),
        column_separator((
            PSYQ_ASSERT(
                PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_VALIDATION(
                    in_row_separator, in_column_separator)),
            in_column_separator)),
        quote_begin((
            PSYQ_ASSERT(
                PSYQ_STRING_CSV_TABLE_QUOTE_VALIDATION(
                    in_row_separator, in_column_separator, in_quote_begin)),
            in_quote_begin)),
        quote_end((
            PSYQ_ASSERT(
                PSYQ_STRING_CSV_TABLE_QUOTE_VALIDATION(
                    in_row_separator, in_column_separator, in_quote_end)),
            in_quote_end)),
        quote_escape((
            PSYQ_ASSERT(
                PSYQ_STRING_CSV_TABLE_QUOTE_VALIDATION(
                    in_row_separator, in_column_separator, in_quote_escape)),
            in_quote_escape)),
        allocator(in_allocator)
        {}

        /// @brief CSV文字列の行の区切り文字。
        typename csv_table::string::value_type const row_separator;
        /// @brief CSV文字列の列の区切り文字。
        typename csv_table::string::value_type const column_separator;
        /// @brief CSV文字列の引用符の開始文字。
        typename csv_table::string::value_type const quote_begin;
        /// @brief CSV文字列の引用符の終了文字。
        typename csv_table::string::value_type const quote_end;
        /// @brief CSV文字列の引用符のエスケープ文字。
        typename csv_table::string::value_type const quote_escape;
        /// @brief CSV文字列で使うメモリ割当子。
        allocator_type allocator;

    }; // struct delimiter

    //-------------------------------------------------------------------------
    /** @brief 文字列表のセルのコンテナ。

        - this_type::cell_vector::value_type::first_type は、
          文字列表のセルのインデクス番号。
        - this_type::cell_vector::value_type::second_type は、
          文字列表のセルが持つ文字列。
     */
    private: typedef std::vector<
        std::pair<
            typename this_type::index_type, typename this_type::string_view>,
        typename this_type::allocator_type>
            cell_vector;

    /// @brief セルのインデクス番号を比較する関数オブジェクト。
    private: struct cell_index_less
    {
        bool operator()(
            typename csv_table::index_type const in_left_index,
            typename csv_table::cell_vector::value_type const& in_right_cell)
        const PSYQ_NOEXCEPT
        {
            return in_left_index < in_right_cell.first;
        }
        bool operator()(
            typename csv_table::cell_vector::value_type const& in_left_cell,
            typename csv_table::index_type const in_right_index)
        const PSYQ_NOEXCEPT
        {
            return in_left_cell.first < in_right_index;
        }
    }; // struct cell_index_less

    /// @brief 文字列表の列の属性。
    public: struct attribute
    {
        attribute(
            typename csv_table::string_view const& in_name,
            typename csv_table::index_type const in_column,
            typename csv_table::index_type const in_size)
        PSYQ_NOEXCEPT:
        name(in_name),
        column(in_column),
        size(in_size)
        {}

        typename csv_table::string_view name;  ///< 属性の名前。
        typename csv_table::index_type column; ///< 属性の列番号。
        typename csv_table::index_type size;   ///< 属性の要素数。
    }; // struct attribute

    /// @brief 属性を名前で比較する関数オブジェクト。
    private: struct attribute_name_less
    {
        bool operator()(
            typename csv_table::attribute const& in_left,
            typename csv_table::attribute const& in_right)
        const PSYQ_NOEXCEPT
        {
            auto const local_compare(in_left.name.compare(in_right.name));
            return local_compare != 0?
                local_compare < 0: in_left.column < in_right.column;
        }
        bool operator()(
            typename csv_table::attribute const& in_left,
            typename csv_table::string_view const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.name < in_right;
        }
        bool operator()(
            typename csv_table::string_view const& in_left,
            typename csv_table::attribute const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left < in_right.name;
        }

    }; // struct attribute_name_less

    /// @brief 属性の辞書。
    private: typedef std::vector<
        typename this_type::attribute, typename this_type::allocator_type>
            attribute_vector;

    /** @brief 文字列表の主キーの配列。

        - this_type::primary_key_vector::value_type::first_type は、
          文字列表の主キーの値。
        - this_type::primary_key_vector::value_type::second_type は、
          文字列表の主キーの行番号。
     */
    private: typedef std::vector<
        std::pair<
            typename this_type::string_view, typename this_type::index_type>,
        typename this_type::allocator_type>
            primary_key_vector;

    /// @brief 文字列表の主キーを比較する関数オブジェクト。
    private: struct primary_key_less
    {
        bool operator()(
            typename csv_table::primary_key_vector::value_type const& in_left,
            typename csv_table::primary_key_vector::value_type const& in_right)
        const PSYQ_NOEXCEPT
        {
            auto const local_compare(in_left.first.compare(in_right.first));
            return local_compare != 0?
                local_compare < 0: in_left.second < in_right.second;
        }
        bool operator()(
            typename csv_table::primary_key_vector::value_type const& in_left,
            typename csv_table::string_view const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.first < in_right;
        }
        bool operator()(
            typename csv_table::string_view const& in_left,
            typename csv_table::primary_key_vector::value_type const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left < in_right.first;
        }

    }; // struct primary_key_less

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief CSV文字列から、文字列表を構築する。
        @param[in] in_csv_string    CSV文字列。
        @param[in] in_csv_delimiter CSV文字列の区切り文字。
        @param[in] in_allocator     メモリ割当子の初期値。
     */
    public: explicit csv_table(
        typename this_type::string_view const& in_csv_string,
        typename this_type::delimiter const& in_csv_delimiter = delimiter(),
        typename this_type::allocator_type const& in_allocator = allocator_type())
    :
    combined_string_(in_csv_delimiter.allocator),
    cells_(
        this_type::make_cell_map(
            this->combined_string_,
            in_csv_string,
            in_csv_delimiter,
            in_allocator)),
    attributes_(in_allocator),
    primary_keys_(in_allocator),
    attribute_row_(this_type::NULL_INDEX),
    primary_key_column_(this_type::NULL_INDEX)
    {}

    /** @brief 文字列表をコピー構築する。
        @param[in] in_source コピー元となる文字列表。
     */
    public: csv_table(this_type const& in_source):
    combined_string_(in_source.combined_string_.get_allocator()),
    cells_(in_source.cells_.get_allocator()),
    attributes_(in_source.attributes_.get_allocator()),
    primary_keys_(in_source.primary_keys_.get_allocator())
    {
        this->copy(in_source);
    }

    /** @brief 文字列表をムーブ構築する。
        @param[in,out] io_source ムーブ元となる文字列表。
     */
    public: csv_table(this_type&& io_source):
    combined_string_(std::move(io_source.combined_string_)),
    cells_(std::move(io_source.cells_)),
    attributes_(std::move(io_source.attributes_)),
    primary_keys_(std::move(io_source.primary_keys_)),
    attribute_row_(std::move(io_source.attribute_row_)),
    primary_key_column_(std::move(io_source.primary_key_column_))
    {}

    /** @brief 文字列表をコピー代入する。
        @param[in] in_source コピー元となる文字列表。
        @return *this
     */
    public: this_type& operator=(this_type const& in_source)
    {
        this->copy(in_source);
        return *this;
    }

    /** @brief 文字列表をムーブ代入する。
        @param[in,out] io_source ムーブ元となる文字列表。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->combined_string_ = std::move(io_source.combined_string_);
        this->cells_ = std::move(io_source.cells_);
        this->attributes_ = std::move(io_source.attributes_);
        this->primary_keys_ = std::move(io_source.primary_keys_);
        this->attribute_row_ = std::move(io_source.attribute_row_);
        this->primary_key_column_ = std::move(io_source.primary_key_column_);
        return *this;
    }

    public: void swap(this_type& io_target) PSYQ_NOEXCEPT
    {
        this->combined_string_.swap(io_target.combined_string_);
        this->cells_.swap(io_target.cells_);
        this->attributes_.swap(io_target.attributes_);
        this->primary_keys_.swap(io_target.primary_keys_);
        std::swap(this->attribute_row_, io_target.attribute_row_);
        std::swap(this->primary_key_column_, io_target.primary_key_column_);
    }
    //@}
    private: void copy(this_type const& in_source)
    {
        this->combined_string_ = in_source.combined_string_;
        this->cells_ = in_source.cells_;
        this->attributes_ = in_source.attributes_;
        this->primary_keys_ = in_source.primary_keys_;
        this->attribute_row_ = in_source.get_attribute_row();
        this->primary_key_column_ = in_source.get_primary_key_column();

        // 文字列参照が、新しい連結文字列を参照するよう調整する。
        auto const local_string_distance(
            this->combined_string_.data() - in_source.combined_string_.data());
        for (auto& local_cell: this->cells_)
        {
            auto& local_string(local_cell.second);
            local_string = typename this_type::string_view(
                local_string.data() + local_string_distance,
                local_string.size());
        }
        for (auto& local_attribute: this->attributes_)
        {
            auto& local_string(local_attribute.name);
            local_string = typename this_type::string_view(
                local_string.data() + local_string_distance,
                local_string.size());
        }
        for (auto& local_primary_key: this->primary_keys_)
        {
            auto& local_string(local_primary_key.first);
            local_string = typename this_type::string_view(
                local_string.data() + local_string_distance,
                local_string.size());
        }
    }

    //-------------------------------------------------------------------------
    /// @name セル
    //@{
    /** @brief 文字列表の行数を取得する。
        @return 文字列表の行数。
     */
    public: typename this_type::index_type get_row_count() const PSYQ_NOEXCEPT
    {
        return this->cells_.empty()?
            0: this_type::compute_row_index(this->cells_.back().first);
    }

    /** @brief 文字列表の列数を取得する。
        @return 文字列表の列数。
     */
    public: typename this_type::index_type get_column_count()
    const PSYQ_NOEXCEPT
    {
        return this->cells_.empty()?
            0: this_type::compute_column_index(this->cells_.back().first);
    }

    /** @brief 主キーから、行番号を検索する。

        this_type::constraint_primary_key で、主キーの構築を事前にしておくこと。

        @param[in] in_primary_key  検索する主キー。
        @retval !=NULL_INDEX 主キーに対応する行番号。
        @retval ==NULL_INDEX 主キーに対応する行番号が存在しない。
     */
    public: typename this_type::index_type find_row_index(
        typename this_type::string_view const& in_primary_key)
    const PSYQ_NOEXCEPT
    {
        auto const local_primary_key(
            std::lower_bound(
                this->primary_keys_.begin(),
                this->primary_keys_.end(),
                in_primary_key,
                typename this_type::primary_key_less()));
        return local_primary_key != this->primary_keys_.end()
            && local_primary_key->first == in_primary_key?
                local_primary_key->second: this_type::NULL_INDEX;
    }

    /** @brief 属性名から、列番号を検索する。

        this_type::constraint_attribute で、属性の構築を事前にしておくこと。

        @param[in] in_attribute_name  検索する属性の名前。
        @param[in] in_attribute_index 検索する属性のインデックス番号。
        @retval !=NULL_INDEX 属性名に対応する列番号。
        @retval ==NULL_INDEX 属性名に対応する列番号が存在しない。
     */
    public: typename this_type::index_type find_column_index(
        typename this_type::string_view const& in_attribute_name,
        std::size_t const in_attribute_index = 0)
    const PSYQ_NOEXCEPT
    {
        auto const local_attribute(this->find_attribute(in_attribute_name));
        return local_attribute != nullptr
            && local_attribute->name == in_attribute_name
            && in_attribute_index < local_attribute->size?
                local_attribute->column +
                    static_cast<typename this_type::index_type>(
                        in_attribute_index):
                this_type::NULL_INDEX;
    }

    /** @brief 行番号と属性から、文字列表の本体セルを検索する。
        @param[in] in_row_index    検索する本体セルの行番号。
        @param[in] in_column_index 検索する本体セルの列番号。
        @return
            行番号と列番号に対応する本体セル。
            対応する本体セルがない場合は、空文字列を返す。
     */
    public: typename this_type::string_view find_body_cell(
        std::size_t const in_row_index,
        std::size_t const in_column_index)
    const PSYQ_NOEXCEPT
    {
        if (in_column_index < this->get_column_count()
            && in_row_index < this->get_row_count()
            && in_row_index != this->get_attribute_row())
        {
            auto const local_cell_index(
                this_type::compute_cell_index(
                    static_cast<typename this_type::index_type>(in_row_index),
                    static_cast<typename this_type::index_type>(
                        in_column_index)));
            auto const local_lower_bound(
                std::lower_bound(
                    this->cells_.begin(),
                    this->cells_.end(),
                    local_cell_index,
                    typename this_type::cell_index_less()));
            if (local_lower_bound != this->cells_.end()
                && local_lower_bound->first == local_cell_index)
            {
                return local_lower_bound->second;
            }
        }
        return typename this_type::string_view();
    }
    //@}
    /** @brief セルのインデクス番号から、セルの行番号を算出する。
        @param[in] in_cell_index セルのインデクス番号。
        @return セルの行番号。
     */
    private: static typename this_type::index_type compute_row_index(
        typename this_type::index_type const in_cell_index)
    PSYQ_NOEXCEPT
    {
        return in_cell_index / this_type::MAX_COLUMN_COUNT;
    }

    /** @brief セルのインデクス番号から、セルの列番号を算出する。
        @param[in] in_cell_index セルのインデクス番号。
        @return セルの列番号。
     */
    private: static typename this_type::index_type compute_column_index(
        typename this_type::index_type const in_cell_index)
    PSYQ_NOEXCEPT
    {
        return in_cell_index % this_type::MAX_COLUMN_COUNT;
    }

    /** @brief セルの行番号と列番号から、セルのインデクス番号を算出する。
        @param[in] in_row_index    セルの行番号。
        @param[in] in_column_index セルの列番号。
        @return セルのインデクス番号。
     */
    private: static typename this_type::index_type compute_cell_index(
        typename this_type::index_type const in_row_index,
        typename this_type::index_type const in_column_index)
    PSYQ_NOEXCEPT
    {
        auto const local_cell_index(
            in_row_index * this_type::MAX_COLUMN_COUNT + in_column_index);
        PSYQ_ASSERT(
            in_row_index == this_type::compute_row_index(local_cell_index));
        return local_cell_index;
    }

    //-------------------------------------------------------------------------
    /// @name 属性
    //@{
    /** @brief 属性の行番号を取得する。
        @retval !=NULL_INDEX 属性の行番号。
        @retval ==NULL_INDEX 属性辞書が空。
     */
    public: typename this_type::index_type get_attribute_row()
    const PSYQ_NOEXCEPT
    {
        return this->attribute_row_;
    }

    /** @brief 属性辞書を構築する。
        @param[in] in_attribute_row 属性の行番号。
        @retval true  成功。
        @retval false 失敗。
     */
    public: bool constraint_attribute(std::size_t const in_attribute_row)
    {
        if (this->get_row_count() <= in_attribute_row)
        {
            return false;
        }
        this->attribute_row_ =
            static_cast<typename this_type::index_type>(in_attribute_row);
        this->attributes_ = this_type::make_attribute_map(
            this->cells_, this->get_attribute_row(), this->get_column_count());
        return true;
    }

    /** @brief 属性辞書を空にする。
     */
    public: void clear_attribute()
    {
        this->attributes_.clear();
        this->attribute_row_ = this_type::NULL_INDEX;
    }

    /** @brief 属性名から、文字列表の属性を検索する。
        @param[in] in_attribute_name 検索する属性の名前。
        @retval !=nullptr 属性名に対応する属性を指すポインタ。
        @retval ==nullptr 属性名に対応する属性が存在しない。
     */
    public: typename this_type::attribute const* find_attribute(
        typename this_type::string_view const& in_attribute_name)
    const PSYQ_NOEXCEPT
    {
        auto const local_lower_bound(
            std::lower_bound(
                this->attributes_.begin(),
                this->attributes_.end(),
                in_attribute_name,
                typename this_type::attribute_name_less()));
        return local_lower_bound != this->attributes_.end()
            && local_lower_bound->name == in_attribute_name?
                &(*local_lower_bound): nullptr;
    }
   //@}
    /** @brief 属性辞書を構築する。
        @param[in] in_cells         文字列表のセル辞書。
        @param[in] in_attribute_row 属性として使う行の番号。
        @param[in] in_num_columns   属性行の列数。
        @return セル辞書から構築した属性辞書。
     */
    private: static typename this_type::attribute_vector make_attribute_map(
        typename this_type::cell_vector const& in_cells,
        typename this_type::index_type const in_attribute_row,
        typename this_type::index_type const in_num_columns)
    {
        // 属性行の先頭位置と末尾インデクスを決定する。
        PSYQ_ASSERT(in_attribute_row < this_type::MAX_COLUMN_COUNT);
        auto const local_attribute_begin(
            std::lower_bound(
                in_cells.begin(),
                in_cells.end(),
                this_type::compute_cell_index(in_attribute_row, 0),
                typename this_type::cell_index_less()));
        auto const local_attribute_end(
            this_type::compute_cell_index(in_attribute_row + 1, 0));

        // 属性行を読み取り、属性配列を構築する。
        typename this_type::attribute_vector local_attributes(
            in_cells.get_allocator());
        local_attributes.reserve(in_num_columns);
        for (
            auto i(local_attribute_begin);
            i != in_cells.end() && i->first < local_attribute_end;
            ++i)
        {
            auto const local_column_index(
                this_type::compute_column_index(i->first));
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
                typename this_type::attribute_name_less());
        }
        local_attributes.shrink_to_fit();
        return local_attributes;
    }

    //-------------------------------------------------------------------------
    /// @name 主キー
    //@{
    /** @brief 主キーの列番号を取得する。
        @retval !=NULL_INDEX 主キーの列番号。
        @retval ==NULL_INDEX 主キーが決定されてない。
        @sa this_type::constraint_primary_key
        @sa this_type::clear_primary_key
     */
    public: typename this_type::index_type get_primary_key_column()
    const PSYQ_NOEXCEPT
    {
        return this->primary_key_column_;
    }

    /** @brief 主キーの辞書を構築する。
        @param[in] in_attribute_name  主キーとする属性の名前。
        @param[in] in_attribute_index 主キーとする属性のインデックス番号。
        @retval true  成功。
        @retval false 失敗。
     */
    public: bool constraint_primary_key(
        typename this_type::string_view const& in_attribute_name,
        std::size_t const in_attribute_index = 0)
    {
        return this->constraint_primary_key(
            this->find_column_index(in_attribute_name, in_attribute_index));
    }

    /** @brief 主キーの辞書を構築する。
        @param[in] in_column_index 主キーとする列番号。
        @retval true  成功。
        @retval false 失敗。
     */
    public: bool constraint_primary_key(std::size_t const in_column_index)
    {
        if (this->get_column_count() <= in_column_index)
        {
            return false;
        }
        auto const local_column_index(
            static_cast<typename this_type::index_type>(in_column_index));
        auto local_primary_keys(
            this_type::make_primary_key_map(
                this->cells_, local_column_index, this->get_attribute_row()));
        this->primary_keys_.swap(local_primary_keys);
        this->primary_key_column_ = local_column_index;
        return true;
    }

    /** @brief 主キー辞書を空にする。
     */
    public: bool clear_primary_key()
    {
        this->primary_keys_.clear();
        this->primary_key_column_ = this_type::NULL_INDEX;
    }

    /** @brief 等価な主キーを数える。
        @param[in] in_primary_key 検索する主キー。
        @return in_primary_key と等価な主キーの数。
        @sa this_type::constraint_primary_key
        @sa this_type::clear_primary_key
     */
    public: typename this_type::index_type count_primary_key(
        typename this_type::string_view const& in_primary_key)
    const PSYQ_NOEXCEPT
    {
        typename this_type::index_type local_count(0);
        for (
            auto i(
                std::lower_bound(
                    this->primary_keys_.begin(),
                    this->primary_keys_.end(),
                    in_primary_key,
                    typename this_type::primary_key_less()));
            i != this->primary_keys_.end() && i->first == in_primary_key;
            ++i)
        {
            ++local_count;
        }
        return local_count;
    }
    //@}
    /** @brief 主キーの辞書を構築する。
        @param[in] in_cells          文字列表のセル辞書。
        @param[in] in_primary_column 主キーとして使う列の番号。
        @param[in] in_attribute_row  属性として使われている行の番号。
        @return セル辞書から構築した主キーの辞書。
     */
    private: static typename this_type::primary_key_vector make_primary_key_map(
        typename this_type::cell_vector const& in_cells,
        typename this_type::index_type const in_primary_column,
        typename this_type::index_type const in_attribute_row)
    {
        typename this_type::primary_key_vector
            local_primary_keys(in_cells.get_allocator());
        if (in_cells.empty())
        {
            return local_primary_keys;
        }
        local_primary_keys.reserve(
            this_type::compute_row_index(in_cells.back().first));

        // セル辞書から主キーの列を読み取り、主キーの配列を構築する。
        auto local_row_index(
            this_type::compute_row_index(in_cells.front().first));
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
                typename this_type::cell_index_less());
            if (local_cell == in_cells.end())
            {
                break;
            }
            if (this_type::compute_column_index(local_cell->first)
                == in_primary_column)
            {
                local_primary_keys.push_back(
                    typename this_type::primary_key_vector::value_type(
                        local_cell->second, local_row_index));
            }
            local_row_index =
                this_type::compute_row_index(local_cell->first) + 1;
        }

        // 主キーの配列を並び替え、主キーの辞書として正規化する。
        local_primary_keys.shrink_to_fit();
        std::sort(
            local_primary_keys.begin(),
            local_primary_keys.end(),
            typename this_type::primary_key_less());
        return local_primary_keys;
    }

    //-------------------------------------------------------------------------
    /** @brief CSV形式の文字列を解析し、セルの辞書を構築する。
        @param[out] out_combined_string セル文字列をまとめて保存する連結文字列。
        @param[in] in_csv_string        解析するCSV形式の文字列。
        @param[in] in_csv_delimiter     CSV文字列の区切り文字。
        @param[in] in_allocator         セルコンテナで使うメモリ割当子。
        @return CSV形式の文字列を解析して構築した、セルの辞書。
     */
    private: static typename this_type::cell_vector make_cell_map(
        typename this_type::string& out_combined_string,
        typename this_type::string_view const& in_csv_string,
        typename this_type::delimiter const& in_csv_delimiter,
        typename this_type::allocator_type const& in_allocator)
    {
        bool local_quote(false);
        typename this_type::index_type local_row(0);
        typename this_type::index_type local_column(0);
        typename this_type::index_type local_column_max(0);
        typename this_type::string_view::value_type local_last_char(0);
        typename this_type::string local_cell_string(
            in_csv_delimiter.allocator);
        typename this_type::string::size_type local_cell_size(0);
        typename this_type::cell_vector local_cells(in_allocator);
        typename this_type::string local_combined_string(
            in_csv_delimiter.allocator);
        local_combined_string.reserve(in_csv_string.size());
        for (auto i(in_csv_string.begin()); i != in_csv_string.end(); ++i)
        {
            auto const local_char(*i);
            if (local_quote)
            {
                if (local_last_char != in_csv_delimiter.quote_escape)
                {
                    if (local_char != in_csv_delimiter.quote_end)
                    {
                        // エスケープ文字でなければ、文字をセルに追加する。
                        if (local_char != in_csv_delimiter.quote_escape)
                        {
                            local_cell_string.push_back(local_char);
                            local_cell_size = local_cell_string.size();
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
                else if (local_char == in_csv_delimiter.quote_end)
                {
                    // 引用符の終了文字をエスケープする。
                    local_cell_string.push_back(local_char);
                    local_cell_size = local_cell_string.size();
                    local_last_char = 0;
                }
                else if (in_csv_delimiter.quote_escape != in_csv_delimiter.quote_end)
                {
                    // 直前の文字がエスケープとして働かなかったので、
                    // 直前の文字と現在の文字をセルに追加する。
                    local_cell_string.push_back(local_last_char);
                    local_cell_string.push_back(local_char);
                    local_cell_size = local_cell_string.size();
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
            else if (local_char == in_csv_delimiter.quote_begin)
            {
                // 引用符の開始。
                local_quote = true;
            }
            else if (local_char == in_csv_delimiter.column_separator)
            {
                // 列の区切り。
                if (!local_cell_string.empty())
                {
                    this_type::add_cell(
                        local_cells,
                        local_combined_string,
                        local_row,
                        local_column,
                        local_cell_string.substr(0, local_cell_size));
                    local_cell_string.clear();
                    local_cell_size = 0;
                }
                ++local_column;
            }
            else if (local_char == in_csv_delimiter.row_separator)
            {
                // 行の区切り。
                if (!local_cell_string.empty())
                {
                    this_type::add_cell(
                        local_cells,
                        local_combined_string,
                        local_row,
                        local_column,
                        local_cell_string.substr(0, local_cell_size));
                    local_cell_string.clear();
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
                auto const local_is_space(std::isspace(local_char));
                if (!local_is_space || !local_cell_string.empty())
                {
                    local_cell_string.push_back(local_char);
                    if (!local_is_space)
                    {
                        local_cell_size = local_cell_string.size();
                    }
                }
            }
        }
        PSYQ_ASSERT(local_row < this_type::MAX_ROW_COUNT);

        // 最終セルの処理。
        if (local_quote)
        {
            // 引用符の開始はあったが終了がなかった場合は、
            // 引用符として処理しない。
            //local_cell_string.insert(
            //    local_cell_string.begin(), in_csv_delimiter.quote_begin);
        }
        if (!local_cell_string.empty())
        {
            this_type::add_cell(
                local_cells,
                local_combined_string,
                local_row,
                local_column,
                local_cell_string.substr(0, local_cell_size));
        }
        else if (0 < local_column)
        {
            --local_column;
        }
        else
        {
            --local_row;
        }
        if (local_column_max < local_column)
        {
            local_column_max = local_column;
        }

        // テーブル文字列を必要最小限にする。
        out_combined_string.assign(local_combined_string);
        auto const local_string_distance(
            out_combined_string.data() - local_combined_string.data());
        for (auto& local_cell: local_cells)
        {
            auto& local_string(local_cell.second);
            local_string = typename this_type::string_view(
                local_string.data() + local_string_distance,
                local_string.size());
        }

        // 行数と列数を記録したセルを末尾に追加する。
        local_cells.push_back(
            typename this_type::cell_vector::value_type(
                this_type::compute_cell_index(
                    (std::min<typename this_type::index_type>)(
                        local_row + 1, this_type::MAX_ROW_COUNT),
                    (std::min<typename this_type::index_type>)(
                        local_column_max + 1, this_type::MAX_COLUMN_COUNT)),
                typename this_type::string_view()));
        local_cells.shrink_to_fit();
        return local_cells;
    }

    /** @brief セルを追加する。
        @param[in,out] io_cells           セルを追加するセル配列。
        @param[in,out] io_combined_string セル文字列を追加する文字列。
        @param[in] in_row_index           追加するセルの行番号。
        @param[in] in_column_index        追加するセルの列番号。
        @param[in] in_cell_string         追加するセルの文字列。
     */
    private: static void add_cell(
        typename this_type::cell_vector& io_cells,
        typename this_type::string& io_combined_string,
        typename this_type::index_type const in_row_index,
        typename this_type::index_type const in_column_index,
        typename this_type::string const& in_cell_string)
    {
        // 最大行数か最大桁数を超えるセルは、追加できない。
        if (this_type::MAX_ROW_COUNT <= in_row_index
            || this_type::MAX_COLUMN_COUNT <= in_column_index)
        {
            PSYQ_ASSERT(false);
            return;
        }

        // セル文字列と同じ部分が連結文字列になければ、新たに追加する。
        auto local_position(io_combined_string.find(in_cell_string));
        if (local_position == this_type::string::npos)
        {
            if (io_combined_string.capacity() - io_combined_string.size()
                < in_cell_string.size())
            {
                // 予約しておいた容量より大きくなるはずはない。
                PSYQ_ASSERT(false);
                return;
            }
            local_position = io_combined_string.size();
            io_combined_string.append(in_cell_string);
        }

        // セルを追加する。
        io_cells.push_back(
            typename this_type::cell_vector::value_type(
                this_type::compute_cell_index(in_row_index, in_column_index),
                typename this_type::string_view(
                    io_combined_string.data() + local_position,
                    in_cell_string.size())));
    }

    //-------------------------------------------------------------------------
    /// @brief 文字列表のすべての文字列を連結した文字列。
    private: typename this_type::string combined_string_;
    /// @brief セルのインデクス番号でソート済のセルの辞書。
    private: typename this_type::cell_vector cells_;
    /// @brief 属性名でソート済の属性の辞書。
    private: typename this_type::attribute_vector attributes_;
    /// @brief 主キーでソート済の主キーの辞書。
    private: typename this_type::primary_key_vector primary_keys_;
    /// @brief 属性として使っている行の番号。
    private: typename this_type::index_type attribute_row_;
    /// @brief 主キーとして使っている列の番号。
    private: typename this_type::index_type primary_key_column_;

}; // class psyq::string::csv_table

#endif // !defined(PSYQ_STRING_CSV_TABLE_HPP_)
// vim: set expandtab:
