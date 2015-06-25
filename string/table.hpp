/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::string::table
 */
#ifndef PSYQ_STRING_TABLE_HPP_
#define PSYQ_STRING_TABLE_HPP_

#include "./flyweight.hpp"

/// @copydoc psyq::string::table::MAX_COLUMN_COUNT
#ifndef PSYQ_STRING_TABLE_MAX_COLUMN_COUNT
#define PSYQ_STRING_TABLE_MAX_COLUMN_COUNT 16384
#endif // !defined(PSYQ_STRING_TABLE_MAX_COLUMN_COUNT)

/// @cond
namespace psyq
{
    namespace string
    {
        template<typename, typename, typename> class table;
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 行番号と列番号で参照する、フライ級文字列の表。
    @tparam template_char_type   @copybrief psyq::string::view::value_type
    @tparam template_char_traits @copybrief psyq::string::view::traits_type
    @tparam template_allocator   @copybrief table::allocator_type
*/
template<
    typename template_char_type,
    typename template_char_traits = PSYQ_STRING_VIEW_TRAITS_DEFAULT,
    typename template_allocator = PSYQ_STRING_FLYWEIGHT_ALLOCATOR_DEFAULT>
class psyq::string::table
{
    /// @brief thisが指す値の型。
    private: typedef table this_type;

    //-------------------------------------------------------------------------
    /// @brief 使用するメモリ割当子を表す型。
    public: typedef template_allocator allocator_type;

    /// @brief 文字列表で使う、フライ級文字列の型。
    public: typedef
        psyq::string::flyweight<
            template_char_type, template_char_traits, template_allocator>
        string;

    public: enum: typename this_type::string::size_type
    {
        /// @brief 文字列表の列の最大数。
        MAX_COLUMN_COUNT = PSYQ_STRING_TABLE_MAX_COLUMN_COUNT,
        /// @brief 文字列表の行の最大数。
        MAX_ROW_COUNT = 1 +
            static_cast<typename this_type::string::size_type>(-1)
            / MAX_COLUMN_COUNT,
    };

    //-------------------------------------------------------------------------
    /** @brief 文字列表のセルのコンテナ。

        - 要素の第1属性は、セル番号。
        - 要素の第2属性は、セル文字列。
     */
    protected: typedef
        std::vector<
            std::pair<
                typename this_type::string::size_type,
                typename this_type::string>,
            typename this_type::allocator_type>
        cell_container;

    /// @brief セル番号を比較する関数オブジェクト。
    protected: struct cell_number_less
    {
        bool operator()(
            typename table::string::size_type const in_left_number,
            typename table::cell_container::value_type const& in_right_cell)
        const PSYQ_NOEXCEPT
        {
            return in_left_number < in_right_cell.first;
        }

        bool operator()(
            typename table::cell_container::value_type const& in_left_cell,
            typename table::string::size_type const in_right_number)
        const PSYQ_NOEXCEPT
        {
            return in_left_cell.first < in_right_number;
        }

    }; // struct cell_number_less

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 文字列表をコピー構築する。
        @param[in] in_source コピー元となる文字列表。
     */
    public: table(this_type const& in_source):
    cells_(in_source.cells_),
    row_count_(in_source.get_row_count()),
    column_count_(in_source.get_column_count())
    {}

    /** @brief 文字列表をムーブ構築する。
        @param[in,out] io_source ムーブ元となる文字列表。
     */
    public: table(this_type&& io_source):
    cells_(std::move(io_source.cells_)),
    row_count_(io_source.get_row_count()),
    column_count_(io_source.get_column_count())
    {
        io_source.set_size(0, 0);
    }

    /** @brief 文字列表をコピー代入する。
        @param[in] in_source コピー元となる文字列表。
        @return *this
     */
    public: this_type& operator=(this_type const& in_source)
    {
        this->cells_ = in_source.cells_;
        this->set_size(
            in_source.get_row_count(), in_source.get_column_count());
        return *this;
    }

    /** @brief 文字列表をムーブ代入する。
        @param[in,out] io_source ムーブ元となる文字列表。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->cells_ = std::move(io_source.cells_);
            this->set_size(
                io_source.get_row_count(), io_source.get_column_count());
            io_source.set_size(0, 0);
        }
        return *this;
    }

    /** @brief メモリ割当子を取得する。
        @return メモリ割当子。
     */
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->cells_.get_allocator();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name セル
    //@{
    /** @brief 文字列表が空か判定する。
        @retval true  文字列表は空だった。
        @retval false 文字列表は空ではなかった。
     */
    public: bool is_empty() const PSYQ_NOEXCEPT
    {
        return this->cells_.empty();
    }

    /** @brief 文字列表の行数を取得する。
        @return 文字列表の行数。
     */
    public: typename this_type::string::size_type get_row_count()
    const PSYQ_NOEXCEPT
    {
        return this->row_count_;
    }

    /** @brief 文字列表の列数を取得する。
        @return 文字列表の列数。
     */
    public: typename this_type::string::size_type get_column_count()
    const PSYQ_NOEXCEPT
    {
        return this->column_count_;
    }

    /** @brief 行番号と属性から、文字列表のセルを検索する。
        @param[in] in_row_number    検索するセルの行番号。
        @param[in] in_column_number 検索するセルの列番号。
        @return
            行番号と列番号に対応するセルの文字列。
            対応するセルがない場合は、空文字列を返す。
     */
    public: typename this_type::string const& find_cell(
        typename this_type::string::size_type const in_row_number,
        typename this_type::string::size_type const in_column_number)
    const PSYQ_NOEXCEPT
    {
        if (in_column_number < this->get_column_count()
            && in_row_number < this->get_row_count())
        {
            auto const local_cell_number(
                this_type::compute_cell_number(in_row_number, in_column_number));
            auto const local_lower_bound(
                std::lower_bound(
                    this->cells_.begin(),
                    this->cells_.end(),
                    local_cell_number,
                    typename this_type::cell_number_less()));
            if (local_lower_bound != this->cells_.end()
                && local_lower_bound->first == local_cell_number)
            {
                return local_lower_bound->second;
            }
        }
        static typename this_type::string const static_empty_string;
        return static_empty_string;
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief 空の文字列表を構築する。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    protected: explicit table(
        typename this_type::allocator_type const& in_allocator)
    : cells_(in_allocator), row_count_(0), column_count_(0)
    {}

    protected: typename this_type::cell_container const& get_cells()
    const PSYQ_NOEXCEPT
    {
        return this->cells_;
    }

    /** @brief セルを置き換える。
        @param[in] in_row_number    置き換えるセルの行番号。
        @param[in] in_column_number 置き換えるセルの列番号。
        @param[in] in_string        置き換えるセルの文字列。
     */
    protected: void replace_cell(
        typename this_type::string::size_type const in_row_number,
        typename this_type::string::size_type const in_column_number,
        typename this_type::string in_string)
    {
        // 最大行数か最大桁数を超えるセルは、追加できない。
        if (this_type::MAX_ROW_COUNT <= in_row_number
            || this_type::MAX_COLUMN_COUNT <= in_column_number)
        {
            PSYQ_ASSERT(false);
            return;
        }

        // セルの挿入位置を決定する。
        auto const local_cell_number(
            this_type::compute_cell_number(in_row_number, in_column_number));
        auto const local_lower_bound(
            std::lower_bound(
                this->cells_.begin(),
                this->cells_.end(),
                local_cell_number,
                typename this_type::cell_number_less()));
        if (local_lower_bound != this->cells_.end()
            && local_lower_bound->first == local_cell_number)
        {
            if (in_string.empty())
            {
                // 置き換える文字列が空なので、セルを削除する。
                this->cells_.erase(local_lower_bound);
            }
            else
            {
                // セル文字列を置き換える。
                local_lower_bound->second = std::move(in_string);
            }
        }
        else if (!in_string.empty())
        {
            // 新たにセルを追加する。
            this->cells_.insert(
                local_lower_bound,
                typename this_type::cell_container::value_type(
                    local_cell_number, std::move(in_string)));
        }
    }

    protected: void set_size(
        typename this_type::string::size_type const in_row_count,
        typename this_type::string::size_type const in_column_count)
    {
        this->row_count_ = (std::min<typename this_type::string::size_type>)(
            in_row_count, this_type::MAX_ROW_COUNT);
        this->column_count_ = (std::min<typename this_type::string::size_type>)(
            in_column_count, this_type::MAX_COLUMN_COUNT);
    }

    protected: void shrink_to_fit()
    {
        this->cells_.shrink_to_fit();
    }

    /// @brief 文字列表を空にする。
    protected: void clear_container(
        typename this_type::cell_container::size_type const in_cell_capacity)
    {
        this->cells_.clear();
        this->cells_.reserve(in_cell_capacity);
        this->row_count_ = 0;
        this->column_count_ = 0;
    }

    /** @brief セル番号から、セルの行番号を算出する。
        @param[in] in_cell_number セル番号。
        @return セルの行番号。
     */
    protected: static typename this_type::string::size_type compute_row_number(
        typename this_type::string::size_type const in_cell_number)
    PSYQ_NOEXCEPT
    {
        return in_cell_number / this_type::MAX_COLUMN_COUNT;
    }

    /** @brief セル番号から、セルの列番号を算出する。
        @param[in] in_cell_number セル番号。
        @return セルの列番号。
     */
    protected:
    static typename this_type::string::size_type compute_column_number(
        typename this_type::string::size_type const in_cell_number)
    PSYQ_NOEXCEPT
    {
        return in_cell_number % this_type::MAX_COLUMN_COUNT;
    }

    /** @brief セルの行番号と列番号から、セル番号を算出する。
        @param[in] in_row_number    セルの行番号。
        @param[in] in_column_number セルの列番号。
        @return セル番号。
     */
    protected:
    static typename this_type::string::size_type compute_cell_number(
        typename this_type::string::size_type const in_row_number,
        typename this_type::string::size_type const in_column_number)
    PSYQ_NOEXCEPT
    {
        auto const local_cell_number(
            in_row_number * this_type::MAX_COLUMN_COUNT + in_column_number);
        PSYQ_ASSERT(
            in_row_number == this_type::compute_row_number(local_cell_number));
        return local_cell_number;
    }

    //-------------------------------------------------------------------------
    /// @brief セル番号でソートされたセルの辞書。
    private: typename this_type::cell_container cells_;
    /// @brief 文字列表の行数。
    private: typename this_type::string::size_type row_count_;
    /// @brief 文字列表の列数。
    private: typename this_type::string::size_type column_count_;

}; // class psyq::string::table

#endif // !defined(PSYQ_STRING_TABLE_HPP_)
// vim: set expandtab:
