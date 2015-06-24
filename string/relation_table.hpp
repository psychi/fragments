/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::string::table
 */
#ifndef PSYQ_STRING_RELATION_TABLE_HPP_
#define PSYQ_STRING_RELATION_TABLE_HPP_

#include "./table.hpp"
#include "./csv_table.hpp"

/// @cond
namespace psyq
{
    namespace string
    {
        template<typename, typename, typename> class csv_table_builder;
        template<typename, typename, typename> class relation_table;
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief CSV形式の文字列から構築する文字列表。
    @tparam template_string    @copybrief psyq::string::csv_table::string
    @tparam template_allocator @copybrief psyq::string::csv_table::allocator_type
*/
template<
    typename template_char_type,
    typename template_char_traits = PSYQ_STRING_VIEW_TRAITS_DEFAULT,
    typename template_allocator = PSYQ_STRING_FLYWEIGHT_ALLOCATOR_DEFAULT>
class psyq::string::csv_table_builder:
public psyq::string::table<
    template_char_type, template_char_traits, template_allocator>
{
    /// @brief thisが指す値の型。
    private: typedef csv_table_builder this_type;
    /// @brief this_type の基底型。
    public: typedef
        psyq::string::table<
            template_char_type, template_char_traits, template_allocator>
        base_type;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 空のCSV文字列表を構築する。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: explicit csv_table_builder(
        typename base_type::allocator_type const& in_allocator =
            allocator_type())
    :
    base_type(in_allocator),
    row_separator_(PSYQ_STRING_CSV_TABLE_ROW_SEPARATOR_DEFAULT),
    column_separator_(PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_DEFAULT),
    quote_begin_(PSYQ_STRING_CSV_TABLE_QUOTE_BEGIN_DEFAULT),
    quote_end_(PSYQ_STRING_CSV_TABLE_QUOTE_END_DEFAULT),
    quote_escape_(PSYQ_STRING_CSV_TABLE_QUOTE_ESCAPE_DEFAULT)
    {}

    /** @brief 空のCSV文字列表を構築する。
        @param[in] in_row_separator    CSV文字列の行の区切り文字。
        @param[in] in_column_separator CSV文字列の列の区切り文字。
        @param[in] in_quote_begin      CSV文字列の引用符の開始文字。
        @param[in] in_quote_end        CSV文字列の引用符の終了文字。
        @param[in] in_quote_escape     CSV文字列の引用符のエスケープ文字。
        @param[in] in_allocator        メモリ割当子の初期値。
     */
    public: csv_table_builder(
        typename base_type::string::value_type const in_row_separator,
        typename base_type::string::value_type const in_column_separator,
        typename base_type::string::value_type const in_quote_begin,
        typename base_type::string::value_type const in_quote_end,
        typename base_type::string::value_type const in_quote_escape,
        typename base_type::allocator_type const& in_allocator =
            allocator_type())
    :
    base_type(in_allocator),
    row_separator_(in_row_separator),
    column_separator_((
        PSYQ_ASSERT(
            PSYQ_STRING_CSV_TABLE_COLUMN_SEPARATOR_VALIDATION(
                in_row_separator, in_column_separator)),
        in_column_separator)),
    quote_begin_((
        PSYQ_ASSERT(
            PSYQ_STRING_CSV_TABLE_QUOTE_VALIDATION(
                in_row_separator, in_column_separator, in_quote_begin)),
        in_quote_begin)),
    quote_end_((
        PSYQ_ASSERT(
            PSYQ_STRING_CSV_TABLE_QUOTE_VALIDATION(
                in_row_separator, in_column_separator, in_quote_end)),
        in_quote_end)),
    quote_escape_((
        PSYQ_ASSERT(
            PSYQ_STRING_CSV_TABLE_QUOTE_VALIDATION(
                in_row_separator, in_column_separator, in_quote_escape)),
        in_quote_escape))
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name CSV文字列の解析
    //@{
    /** @brief 行の区切り文字を取得する。
        @return 行の区切り文字。
     */
    public: typename base_type::string::value_type get_row_separator()
    const PSYQ_NOEXCEPT
    {
        return this->row_separator_;
    }

    /** @brief 列の区切り文字を取得する。
        @return 列の区切り文字。
     */
    public: typename base_type::string::value_type get_column_separator()
    const PSYQ_NOEXCEPT
    {
        return this->column_separator_;
    }

    /** @brief 引用符の開始文字を取得する。
        @return 引用符の開始文字。
     */
    public: typename base_type::string::value_type get_quote_begin()
    const PSYQ_NOEXCEPT
    {
        return this->quote_begin_;
    }

    /** @brief 引用符の終了文字を取得する。
        @return 引用符の終了文字。
     */
    public: typename base_type::string::value_type get_quote_end()
    const PSYQ_NOEXCEPT
    {
        return this->quote_end_;
    }

    /** @brief 引用符のエスケープ文字を取得する。
        @return 引用符のエスケープ文字。
     */
    public: typename base_type::string::value_type get_quote_escape()
    const PSYQ_NOEXCEPT
    {
        return this->quote_escape_;
    }

    /** @brief CSV形式の文字列を解析し、文字列表を構築する。
        @param[in,out] io_workspace
            作業領域として使う文字列。
            std::string 互換のインターフェイスを持つこと。
        @param[in] in_factory
            フライ級文字列生成器を指すスマートポインタ。
            スマートポインタが空ではないこと。
        @param[in] in_csv_string 解析するCSV形式の文字列。
     */
    public: template<typename template_workspace>
    void build(
        template_workspace& io_workspace,
        typename base_type::string::factory::shared_ptr const& in_factory,
        typename base_type::string::view const& in_csv_string)
    {
        if (in_factory.get() == nullptr)
        {
            PSYQ_ASSERT(false);
            return;
        }
        io_workspace.clear();
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
                if (local_last_char != this->get_quote_escape())
                {
                    if (local_char != this->get_quote_end())
                    {
                        // エスケープ文字でなければ、文字をセルに追加する。
                        if (local_char != this->get_quote_escape())
                        {
                            io_workspace.push_back(local_char);
                            local_cell_size = io_workspace.size();
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
                else if (local_char == this->get_quote_end())
                {
                    // 引用符の終了文字をエスケープする。
                    io_workspace.push_back(local_char);
                    local_cell_size = io_workspace.size();
                    local_last_char = 0;
                }
                else if (this->get_quote_escape() != this->get_quote_end())
                {
                    // 直前の文字がエスケープとして働かなかったので、
                    // 直前の文字と現在の文字をセルに追加する。
                    io_workspace.push_back(local_last_char);
                    io_workspace.push_back(local_char);
                    local_cell_size = io_workspace.size();
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
            else if (local_char == this->get_quote_begin())
            {
                // 引用符の開始。
                local_quote = true;
            }
            else if (local_char == this->get_column_separator())
            {
                // 列の区切り。
                if (!io_workspace.empty())
                {
                    this->replace_cell(
                        local_row,
                        local_column,
                        typename base_type::string(
                            typename base_type::string::view(
                                io_workspace.data(), local_cell_size),
                            in_factory));
                    io_workspace.clear();
                    local_cell_size = 0;
                }
                ++local_column;
            }
            else if (local_char == this->get_row_separator())
            {
                // 行の区切り。
                if (!io_workspace.empty())
                {
                    this->replace_cell(
                        local_row,
                        local_column,
                        typename base_type::string(
                            typename base_type::string::view(
                                io_workspace.data(), local_cell_size),
                            in_factory));
                    io_workspace.clear();
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
                if (!local_is_space || !io_workspace.empty())
                {
                    io_workspace.push_back(local_char);
                    if (!local_is_space)
                    {
                        local_cell_size = io_workspace.size();
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
            io_workspace.insert(io_workspace.begin(), this->get_quote_begin());
        }
#endif // 0
        if (!io_workspace.empty())
        {
            this->replace_cell(
                local_row,
                local_column,
                typename base_type::string(
                    typename base_type::string::view(
                        io_workspace.data(), local_cell_size),
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
            io_workspace.clear();
            this->shrink_to_fit();
            return;
        }
        io_workspace.clear();
        this->shrink_to_fit();

        // 文字列表の大きさを決定する。
        if (local_column_max < local_column)
        {
            local_column_max = local_column;
        }
        this->set_size(local_row + 1, local_column_max + 1);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @brief CSV文字列の行の区切り文字。
    private: typename base_type::string::value_type row_separator_;
    /// @brief CSV文字列の列の区切り文字。
    private: typename base_type::string::value_type column_separator_;
    /// @brief CSV文字列の引用符の開始文字。
    private: typename base_type::string::value_type quote_begin_;
    /// @brief CSV文字列の引用符の終了文字。
    private: typename base_type::string::value_type quote_end_;
    /// @brief CSV文字列の引用符のエスケープ文字。
    private: typename base_type::string::value_type quote_escape_;

}; // class psyq::string::csv_table_builder

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 関係データベース的な文字列表。属性と主キーを持つ。
    @tparam template_string    @copybrief psyq::string::relation_table::string
    @tparam template_allocator @copybrief psyq::string::relation_table::allocator_type
*/
template<
    typename template_char_type,
    typename template_char_traits = PSYQ_STRING_VIEW_TRAITS_DEFAULT,
    typename template_allocator = PSYQ_STRING_FLYWEIGHT_ALLOCATOR_DEFAULT>
class psyq::string::relation_table:
public psyq::string::table<
    template_char_type, template_char_traits, template_allocator>
{
    /// @brief thisが指す値の型。
    private: typedef relation_table this_type;
    /// @brief this_type の基底型。
    public: typedef
        psyq::string::table<
            template_char_type, template_char_traits, template_allocator>
        base_type;

    //-------------------------------------------------------------------------
    /// @brief RDB文字列表の列の属性。
    public: class attribute
    {
        public: attribute(
            typename relation_table::string::view const& in_name,
            typename relation_table::string::size_type const in_column,
            typename relation_table::string::size_type const in_size)
        PSYQ_NOEXCEPT:
        name_(in_name),
        column_(in_column),
        size_(in_size)
        {}

        /// @brief 属性の名前。
        public: typename relation_table::string::view name_;
        /// @brief 属性の列番号。
        public: typename relation_table::string::size_type column_;
        /// @brief 属性の要素数。
        public: typename relation_table::string::size_type size_;

    }; // class attribute

    /// @brief 属性を名前で比較する関数オブジェクト。
    private: struct attribute_name_less
    {
        bool operator()(
            typename relation_table::attribute const& in_left,
            typename relation_table::attribute const& in_right)
        const PSYQ_NOEXCEPT
        {
            auto const local_compare(in_left.name_.compare(in_right.name_));
            return local_compare != 0?
                local_compare < 0: in_left.column_ < in_right.column_;
        }
        bool operator()(
            typename relation_table::attribute const& in_left,
            typename relation_table::string::view const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.name_ < in_right;
        }
        bool operator()(
            typename relation_table::string::view const& in_left,
            typename relation_table::attribute const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left < in_right.name_;
        }

    }; // struct attribute_name_less

    /// @brief 属性の辞書。
    private: typedef
        std::vector<
            typename this_type::attribute, typename base_type::allocator_type>
        attribute_container;

    //-------------------------------------------------------------------------
    /** @brief RDB文字列表の主キーの配列。

        - this_type::key_container::value_type::first_type は、
          文字列表の主キーの値。
        - this_type::key_container::value_type::second_type は、
          文字列表の主キーの行番号。
     */
    private: typedef
        std::vector<
            std::pair<
                typename base_type::string::view,
                typename base_type::string::size_type>,
            typename base_type::allocator_type>
        key_container;

    /// @brief RDB文字列表の主キーを比較する関数オブジェクト。
    private: struct key_less
    {
        bool operator()(
            typename relation_table::key_container::value_type const&
                in_left,
            typename relation_table::key_container::value_type const&
                in_right)
        const PSYQ_NOEXCEPT
        {
            auto const local_compare(in_left.first.compare(in_right.first));
            return local_compare != 0?
                local_compare < 0: in_left.second < in_right.second;
        }
        bool operator()(
            typename relation_table::key_container::value_type const&
                in_left,
            typename relation_table::string::view const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.first < in_right;
        }
        bool operator()(
            typename relation_table::string::view const& in_left,
            typename relation_table::key_container::value_type const&
                in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left < in_right.first;
        }

    }; // struct key_less

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief RDB文字列表を構築する。
        @param[in] in_source 元となる文字列表。
     */
    public: explicit relation_table(base_type in_source):
    base_type(std::move(in_source)),
    attributes_(this->get_allocator()),
    keys_(this->get_allocator()),
    attribute_row_(base_type::string::npos),
    key_column_(base_type::string::npos)
    {}

    /** @brief RDB文字列表をムーブ構築する。
        @param[in,out] io_source ムーブ元となる文字列表。
     */
    public: relation_table(this_type&& io_source):
    base_type(std::move(io_source)),
    attributes_(std::move(io_source.attributes_)),
    keys_(std::move(io_source.keys_)),
    attribute_row_(io_source.get_attribute_row()),
    key_column_(io_source.get_key_column())
    {
        io_source.attribute_row_ = base_type::string::npos;
        io_source.key_column_ = base_type::string::npos;
    }

    /** @brief RDB文字列表をムーブ代入する。
        @param[in,out] io_source ムーブ元となる文字列表。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->base_type::operator=(std::move(io_source));
            this->attributes_ = std::move(io_source.attributes_);
            this->keys_ = std::move(io_source.keys_);
            this->attribute_row_ = io_source.get_attribute_row();
            this->key_column_ = io_source.get_key_column();
            io_source.attribute_row_ = base_type::string::npos;
            io_source.key_column_ = base_type::string::npos;
        }
        return *this;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name セル
    //@{
    /** @brief 主キーから、行番号を検索する。

        this_type::constraint_key で、主キーの構築を事前にしておくこと。

        @param[in] in_key  検索する主キー。
        @retval !=NULL_INDEX 主キーに対応する行番号。
        @retval ==NULL_INDEX 主キーに対応する行番号が存在しない。
     */
    public: typename base_type::string::size_type find_row_index(
        typename base_type::string::view const& in_key)
    const PSYQ_NOEXCEPT
    {
        auto const local_key(
            std::lower_bound(
                this->keys_.begin(),
                this->keys_.end(),
                in_key,
                typename this_type::key_less()));
        return local_key != this->keys_.end()
            && local_key->first == in_key?
                local_key->second: base_type::string::npos;
    }

    /** @brief 属性名から、列番号を検索する。

        this_type::constraint_attribute で、属性の構築を事前にしておくこと。

        @param[in] in_attribute_name  検索する属性の名前。
        @param[in] in_attribute_index 検索する属性のインデックス番号。
        @retval !=npos 属性名に対応する列番号。
        @retval ==npos 属性名に対応する列番号が存在しない。
     */
    public: typename base_type::string::size_type find_column_index(
        typename base_type::string::view const& in_attribute_name,
        typename base_type::string::size_type const in_attribute_index = 0)
    const PSYQ_NOEXCEPT
    {
        auto const local_attribute(this->find_attribute(in_attribute_name));
        return local_attribute != nullptr
            && local_attribute->name_ == in_attribute_name
            && in_attribute_index < local_attribute->size_?
                local_attribute->column_ + in_attribute_index:
                base_type::string::npos;
    }

    /** @brief 行番号と属性から、文字列表の本体セルを検索する。
        @param[in] in_row_index    検索する本体セルの行番号。
        @param[in] in_column_index 検索する本体セルの列番号。
        @return
            行番号と列番号に対応する本体セル。
            対応する本体セルがない場合は、空文字列を返す。
     */
    public: typename base_type::string const& find_body_cell(
        typename base_type::string::size_type const in_row_index,
        typename base_type::string::size_type const in_column_index)
    const PSYQ_NOEXCEPT
    {
        this->find_cell(
            in_row_index,
            in_row_index != this->get_attribute_row()?
                in_column_index: base_type::MAX_COLUMN_COUNT);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 属性
    //@{
    /** @brief 属性の行番号を取得する。
        @retval !=npos 属性の行番号。
        @retval ==npos 属性辞書が空。
     */
    public: typename base_type::string::size_type get_attribute_row()
    const PSYQ_NOEXCEPT
    {
        return this->attribute_row_;
    }

    /** @brief 属性辞書を構築する。
        @param[in] in_attribute_row 属性の行番号。
        @retval true  成功。
        @retval false 失敗。
     */
    public: bool constraint_attribute(
        typename base_type::string::size_type const in_attribute_row)
    {
        if (this->get_row_count() <= in_attribute_row)
        {
            return false;
        }
        this->attribute_row_ = in_attribute_row;
        this->attributes_ = this_type::make_attribute_map(
            this->get_cells(), this->get_attribute_row(), this->get_column_count());
        return true;
    }

    /** @brief 属性辞書を空にする。
     */
    public: void clear_attribute()
    {
        this->attributes_.clear();
        this->attribute_row_ = base_type::string::npos;
    }

    /** @brief 属性名から、文字列表の属性を検索する。
        @param[in] in_attribute_name 検索する属性の名前。
        @retval !=nullptr 属性名に対応する属性を指すポインタ。
        @retval ==nullptr 属性名に対応する属性が存在しない。
     */
    public: typename this_type::attribute const* find_attribute(
        typename base_type::string::view const& in_attribute_name)
    const PSYQ_NOEXCEPT
    {
        auto const local_lower_bound(
            std::lower_bound(
                this->attributes_.begin(),
                this->attributes_.end(),
                in_attribute_name,
                typename this_type::attribute_name_less()));
        return local_lower_bound != this->attributes_.end()
            && local_lower_bound->name_ == in_attribute_name?
                &(*local_lower_bound): nullptr;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 主キー
    //@{
    /** @brief 主キーの列番号を取得する。
        @retval !=npos 主キーの列番号。
        @retval ==npos 主キーが決定されてない。
        @sa this_type::constraint_key
        @sa this_type::clear_key
     */
    public: typename base_type::string::size_type get_key_column()
    const PSYQ_NOEXCEPT
    {
        return this->key_column_;
    }

    /** @brief 主キーの辞書を構築する。
        @param[in] in_attribute_name  主キーとする属性の名前。
        @param[in] in_attribute_index 主キーとする属性のインデックス番号。
        @retval true  成功。
        @retval false 失敗。
     */
    public: bool constraint_key(
        typename base_type::string::view const& in_attribute_name,
        typename base_type::string::size_type const in_attribute_index = 0)
    {
        return this->constraint_key(
            this->find_column_index(in_attribute_name, in_attribute_index));
    }

    /** @brief 主キーの辞書を構築する。
        @param[in] in_column_index 主キーとする列番号。
        @retval true  成功。
        @retval false 失敗。
     */
    public: bool constraint_key(
        typename base_type::string::size_type const in_column_index)
    {
        if (this->get_column_count() <= in_column_index)
        {
            return false;
        }
        auto const local_column_index(
            static_cast<typename base_type::string::size_type>(in_column_index));
        auto local_keys(
            this_type::make_key_map(
                this->get_cells(), local_column_index, this->get_attribute_row()));
        this->keys_.swap(local_keys);
        this->key_column_ = local_column_index;
        return true;
    }

    /** @brief 主キー辞書を空にする。
     */
    public: void clear_key()
    {
        this->keys_.clear();
        this->key_column_ = this_type::string::npos;
    }

    /** @brief 等価な主キーを数える。
        @param[in] in_key 検索する主キー。
        @return in_key と等価な主キーの数。
        @sa this_type::constraint_key
        @sa this_type::clear_key
     */
    public: typename base_type::string::size_type count_key(
        typename base_type::string::view const& in_key)
    const PSYQ_NOEXCEPT
    {
        typename base_type::string::size_type local_count(0);
        for (
            auto i(
                std::lower_bound(
                    this->keys_.begin(),
                    this->keys_.end(),
                    in_key,
                    typename this_type::key_less()));
            i != this->keys_.end() && i->first == in_key;
            ++i)
        {
            ++local_count;
        }
        return local_count;
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief 文字列表をコピー構築する。
        @param[in] in_source コピー元となる文字列表。
        @todo 属性と主キーのコピーを実装すること。
     */
    private: relation_table(this_type const& in_source):
    base_type(in_source),
    attributes_(in_source.attributes_),
    keys_(in_source.keys_),
    attribute_row_(in_source.get_attribute_row()),
    key_column_(in_source.get_key_column())
    {}

    /** @brief 文字列表をコピー代入する。
        @param[in] in_source コピー元となる文字列表。
        @return *this
        @todo 属性と主キーのコピーを実装すること。
     */
    private: this_type& operator=(this_type const& in_source)
    {
        this->base_type::operator=(in_source);
        this->attributes_ = in_source.attributes_;
        this->keys_ = in_source.keys_;
        this->attribute_row_ = in_source.get_attribute_row();
        this->key_column_ = in_source.get_key_column();
        return *this;
    }

    /** @brief 属性辞書を構築する。
        @param[in] in_cells         文字列表のセル辞書。
        @param[in] in_attribute_row 属性として使う行の番号。
        @param[in] in_num_columns   属性行の列数。
        @return セル辞書から構築した属性辞書。
     */
    private: static typename this_type::attribute_container make_attribute_map(
        typename base_type::cell_container const& in_cells,
        typename base_type::string::size_type const in_attribute_row,
        typename base_type::string::size_type const in_num_columns)
    {
        // 属性行の先頭位置と末尾インデクスを決定する。
        PSYQ_ASSERT(in_attribute_row < base_type::MAX_COLUMN_COUNT);
        auto const local_attribute_begin(
            std::lower_bound(
                in_cells.begin(),
                in_cells.end(),
                base_type::compute_cell_index(in_attribute_row, 0),
                typename base_type::cell_index_less()));
        auto const local_attribute_end(
            base_type::compute_cell_index(in_attribute_row + 1, 0));

        // 属性行を読み取り、属性配列を構築する。
        typename this_type::attribute_container local_attributes(
            in_cells.get_allocator());
        local_attributes.reserve(in_num_columns);
        for (
            auto i(local_attribute_begin);
            i != in_cells.end() && i->index_ < local_attribute_end;
            ++i)
        {
            auto const local_column_index(
                base_type::compute_column_index(i->index_));
            if (!local_attributes.empty())
            {
                // 直前の属性の要素数を決定する。
                auto& local_back(local_attributes.back());
                local_back.size_ = local_column_index - local_back.column_;
            }
            local_attributes.push_back(
                typename this_type::attribute(
                    i->string_, local_column_index, 0));
        }

        // 属性配列を並び替え、属性辞書として正規化する。
        if (!local_attributes.empty())
        {
            // 末尾の属性の要素数を決定する。
            auto& local_back(local_attributes.back());
            local_back.size_ = local_back.column_ < in_num_columns?
                in_num_columns - local_back.column_: 1;
            // 属性名で並び替える。
            std::sort(
                local_attributes.begin(),
                local_attributes.end(),
                typename this_type::attribute_name_less());
        }
        local_attributes.shrink_to_fit();
        return local_attributes;
    }

    /** @brief 主キーの辞書を構築する。
        @param[in] in_cells         文字列表のセル辞書。
        @param[in] in_key_column    主キーとして使う列の番号。
        @param[in] in_attribute_row 属性として使われている行の番号。
        @return セル辞書から構築した主キーの辞書。
     */
    private:
    static typename this_type::key_container make_key_map(
        typename base_type::cell_container const& in_cells,
        typename base_type::string::size_type const in_key_column,
        typename base_type::string::size_type const in_attribute_row)
    {
        typename this_type::key_container local_keys(in_cells.get_allocator());
        if (in_cells.empty())
        {
            return local_keys;
        }
        local_keys.reserve(
            base_type::compute_row_index(in_cells.back().index_));

        // セル辞書から主キーの列を読み取り、主キーの配列を構築する。
        auto local_row_index(
            base_type::compute_row_index(in_cells.front().index_));
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
                base_type::compute_cell_index(local_row_index, in_key_column),
                typename base_type::cell_index_less());
            if (local_cell == in_cells.end())
            {
                break;
            }
            if (base_type::compute_column_index(local_cell->index_)
                == in_key_column)
            {
                local_keys.push_back(
                    typename this_type::key_container::value_type(
                        local_cell->string_, local_row_index));
            }
            local_row_index =
                base_type::compute_row_index(local_cell->index_) + 1;
        }

        // 主キーの配列を並び替え、主キーの辞書として正規化する。
        local_keys.shrink_to_fit();
        std::sort(
            local_keys.begin(),
            local_keys.end(),
            typename this_type::key_less());
        return local_keys;
    }

    //-------------------------------------------------------------------------
    /// @brief 属性名でソート済の属性の辞書。
    private: typename this_type::attribute_container attributes_;
    /// @brief 主キーでソート済の主キーの辞書。
    private: typename this_type::key_container keys_;
    /// @brief 属性として使っている行の番号。
    private: typename base_type::string::size_type attribute_row_;
    /// @brief 主キーとして使っている列の番号。
    private: typename base_type::string::size_type key_column_;

}; // class psyq::string::relation_table

#endif // !defined(PSYQ_STRING_RELATION_TABLE_HPP_)
// vim: set expandtab:
