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
/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::string::table
 */
#ifndef PSYQ_STRING_TABLE_HPP_
#define PSYQ_STRING_TABLE_HPP_

#include <vector>
#include "./view.hpp"

#ifndef PSYQ_STRING_TABLE_MAX_COLUMN_COUNT
#define PSYQ_STRING_TABLE_MAX_COLUMN_COUNT 16384
#endif // !defined(PSYQ_STRING_TABLE_MAX_COLUMN_COUNT)

/// @cond
namespace psyq
{
    namespace string
    {
        template<typename, typename> class table;
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 行番号と列番号で参照する、文字列の表。
    @tparam template_string    @copybrief table::string
    @tparam template_allocator @copybrief table::allocator_type
*/
template<
    typename template_string = std::string,
    typename template_allocator = typename template_string::allocator_type>
class psyq::string::table
{
    private: typedef table this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// @brief 文字列表で使う、std::basic_string互換の文字列型。
    public: typedef template_string string;

    /// @brief メモリ割当子を表す型。
    public: typedef template_allocator allocator_type;

    /// @brief 文字列のメモリ割当子を表す型。
    protected: typedef
        typename this_type::string::allocator_type
        string_allocator;

    /// @brief 文字列表で使う、文字列参照の型。
    public: typedef
        psyq::string::view<
            typename this_type::string::value_type,
            typename this_type::string::traits_type> 
        string_view;

    public: enum: typename this_type::string::size_type
    {
        /// @brief 列の最大数。
        MAX_COLUMN_COUNT = PSYQ_STRING_TABLE_MAX_COLUMN_COUNT,
        /// @brief 行の最大数。
        MAX_ROW_COUNT = this_type::string::npos / MAX_COLUMN_COUNT + 1,
    };

    //-------------------------------------------------------------------------
    /// @brief 文字列表のセル。
    private: class cell
    {
        public: cell(
            typename this_type::string::size_type const in_index,
            typename this_type::string::size_type const in_offset,
            typename this_type::string::size_type const in_size)
        PSYQ_NOEXCEPT: index_(in_index), offset_(in_offset), size_(in_size)
        {}

        /// @brief セルのインデクス番号。
        public: typename this_type::string::size_type index_;
        /// @brief セル文字列のオフセット位置。
        public: typename this_type::string::size_type offset_;
        /// @brief セル文字列の要素数。
        public: typename this_type::string::size_type size_;

    }; // class cell

    /// @brief 文字列表のセルのコンテナ。
    private: typedef
        std::vector<
            typename this_type::cell, typename this_type::allocator_type>
        cell_container;

    /// @brief セルのインデクス番号を比較する関数オブジェクト。
    private: struct cell_index_less
    {
        bool operator()(
            typename table::string::size_type const in_left_index,
            typename table::cell_container::value_type const& in_right_cell)
        const PSYQ_NOEXCEPT
        {
            return in_left_index < in_right_cell.index_;
        }
        bool operator()(
            typename table::cell_container::value_type const& in_left_cell,
            typename table::string::size_type const in_right_index)
        const PSYQ_NOEXCEPT
        {
            return in_left_cell.index_ < in_right_index;
        }
    }; // struct cell_index_less

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 文字列表をコピー構築する。
        @param[in] in_source コピー元となる文字列表。
     */
    public: table(this_type const& in_source):
    combined_string_(in_source.combined_string_),
    cells_(in_source.cells_.cells_),
    row_count_(in_source.get_row_count()),
    column_count_(in_source.get_column_count())
    {}

    /** @brief 文字列表をムーブ構築する。
        @param[in,out] io_source ムーブ元となる文字列表。
     */
    public: table(this_type&& io_source):
    combined_string_(std::move(io_source.combined_string_)),
    cells_(std::move(io_source.cells_)),
    row_count_(in_source.get_row_count()),
    column_count_(in_source.get_column_count())
    {
        io_source.row_count_ = 0;
        io_source.column_count = 0;
    }

    /** @brief 文字列表をコピー代入する。
        @param[in] in_source コピー元となる文字列表。
        @return *this
     */
    public: this_type& operator=(this_type const& in_source)
    {
        this->combined_string_ = in_source.combined_string_;
        this->cells_ = in_source.cells_.cells_;
        this->row_count_ = in_source.get_row_count();
        this->column_count_ = in_source.get_column_count();
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
            this->combined_string_ = std::move(io_source.combined_string_);
            this->cells_ = std::move(io_source.cells_.cells_);
            this->row_count_ = io_source.get_row_count();
            this->column_count_ = io_source.get_column_count();
            io_source.row_count_ = 0;
            io_source.column_count_ = 0;
        }
        return *this;
    }

    public: typename this_type::string::allocator_type get_string_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->combined_string_.get_allocator();
    }

    public: typename this_type::allocator_type get_cell_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->cells_.get_allocator();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name セル
    //@{
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

    /** @brief 行番号と属性から、文字列表の本体セルを検索する。
        @param[in] in_row_index    検索する本体セルの行番号。
        @param[in] in_column_index 検索する本体セルの列番号。
        @return
            行番号と列番号に対応する本体セルの文字列。
            対応する本体セルがない場合は、空文字列を返す。
     */
    public: typename this_type::string_view find_cell(
        typename this_type::string::size_type const in_row_index,
        typename this_type::string::size_type const in_column_index)
    const PSYQ_NOEXCEPT
    {
        if (in_column_index < this->get_column_count()
            && in_row_index < this->get_row_count())
        {
            auto const local_cell_index(
                this_type::compute_cell_index(in_row_index, in_column_index));
            auto const local_lower_bound(
                std::lower_bound(
                    this->cells_.begin(),
                    this->cells_.end(),
                    local_cell_index,
                    typename this_type::cell_index_less()));
            if (local_lower_bound != this->cells_.end()
                && local_lower_bound->first == local_cell_index)
            {
                return typename this_type::string_view(
                    this->combined_string_.data() +
                        local_lower_bound->second.offset_,
                    local_lower_bound->second.size_);
            }
        }
        return typename this_type::string_view();
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief 空の文字列表を構築する。
        @param[in] in_string_allocator 文字列のメモリ割当子。
        @param[in] in_cell_allocator   セルのメモリ割当子。
     */
    protected: explicit table(
        typename this_type::string::allocator_type const& in_string_allocator,
        typename this_type::allocator_type const& in_cell_allocator)
    :
    combined_string_(in_string_allocator),
    cells_(in_cell_allocator),
    row_count_(0),
    column_count(0)
    {}

    /** @brief セルを置き換える。
        @param[in] in_row_index    置き換えるセルの行番号。
        @param[in] in_column_index 置き換えるセルの列番号。
        @param[in] in_string       置き換えるセルの文字列。
     */
    protected: void replace_cell(
        typename this_type::string::size_type const in_row_index,
        typename this_type::string::size_type const in_column_index,
        typename this_type::string_view const& in_string)
    {
        // 最大行数か最大桁数を超えるセルは、追加できない。
        if (this_type::MAX_ROW_COUNT <= in_row_index
            || this_type::MAX_COLUMN_COUNT <= in_column_index)
        {
            PSYQ_ASSERT(false);
            return;
        }

        // セルの挿入位置を決定する。
        auto const local_cell_index(
            this_type::compute_cell_index(in_row_index, in_column_index));
        auto const local_lower_bound(
            std::lower_bound(
                this->cells_.begin(),
                this->cells_.end(),
                local_cell_index,
                typename this_type::cell_index_less()));
        if (local_lower_bound == this->cells_.end()
            || local_lower_bound->index_ != local_cell_index)
        {
            // セルを追加する。
            this->cells_.insert(
                local_lower_bound,
                typename this_type::cell(
                    local_cell_index,
                    this_type::equip_string(this->combined_string_, in_string),
                    in_string.size()));
        }
        else if (in_string.empty())
        {
            // 置き換える文字列が空なので、セルを削除する。
            this->cells_.erase(local_lower_bound);
        }
        else
        {
            // セルを書き換える。
            local_lower_bound->offset_ = this_type::equip_string(
                    this->combined_string_, in_string);
            local_lower_bound->size_ = in_string.size();
        }
    }

    protected: void set_size(
        typename this_type::string::size_type const in_row_count,
        typename this_type::string::size_type const in_column_count)
    {
        this->row_count_ = (std::min)(in_row_count, this_type::MAX_ROW_COUNT);
        this->column_count_ = (std::min)(
            in_column_count, this_type::MAX_COLUMN_COUNT);
    }

    protected: void shrink_to_fit()
    {
        this->combined_string_.shrink_to_fit();
        this->cells_.shrink_to_fit();
    }

    /// @brief 文字列表を空にする。
    protected: void clear_container()
    {
        this->combined_string_.clear();
        this->cells_.clear();
        this->row_count_ = 0;
        this->column_count_ = 0;
    }

    protected: void reserve_container(
        typename this_type::string::size_type const in_string_capacity,
        typename this_type::cell_container::size_type const in_cell_capacity)
    {
        this->combined_string_.reserve(in_string_capacity);
        this->cells_.reserve(in_cell_capacity);
    }

    //-------------------------------------------------------------------------
    private: static typename this_type::string::size_type equip_string(
        typename this_type::string& io_combined_string,
        typename this_type::string_view const& in_string)
    {
        auto local_position(
            io_combined_string.find(in_string.data(), 0, in_string.size()));
        if (local_position == this_type::string::npos)
        {
            local_position = io_combined_string.size();
            io_combined_string.append(in_string.data(), in_string.size());
        }
        return local_position;
    }

    /** @brief セルのインデクス番号から、セルの行番号を算出する。
        @param[in] in_cell_index セルのインデクス番号。
        @return セルの行番号。
     */
    private: static typename this_type::string::size_type compute_row_index(
        typename this_type::string::size_type const in_cell_index)
    PSYQ_NOEXCEPT
    {
        return in_cell_index / this_type::MAX_COLUMN_COUNT;
    }

    /** @brief セルのインデクス番号から、セルの列番号を算出する。
        @param[in] in_cell_index セルのインデクス番号。
        @return セルの列番号。
     */
    private: static typename this_type::string::size_type compute_column_index(
        typename this_type::string::size_type const in_cell_index)
    PSYQ_NOEXCEPT
    {
        return in_cell_index % this_type::MAX_COLUMN_COUNT;
    }

    /** @brief セルの行番号と列番号から、セルのインデクス番号を算出する。
        @param[in] in_row_index    セルの行番号。
        @param[in] in_column_index セルの列番号。
        @return セルのインデクス番号。
     */
    private: static typename this_type::string::size_type compute_cell_index(
        typename this_type::string::size_type const in_row_index,
        typename this_type::string::size_type const in_column_index)
    PSYQ_NOEXCEPT
    {
        auto const local_cell_index(
            in_row_index * this_type::MAX_COLUMN_COUNT + in_column_index);
        PSYQ_ASSERT(
            in_row_index == this_type::compute_row_index(local_cell_index));
        return local_cell_index;
    }

    //-------------------------------------------------------------------------
    /// @brief 文字列表のすべての文字列を連結した文字列。
    private: typename this_type::string combined_string_;
    /// @brief セルのインデクス番号でソートされたセルの辞書。
    private: typename this_type::cell_container cells_;
    /// @brief 文字列表の行数。
    private: typename this_type::string::size_type row_count_;
    /// @brief 文字列表の列数。
    private: typename this_type::string::size_type column_count_;

}; // class psyq::string::table

#endif // !defined(PSYQ_STRING_TABLE_HPP_)
// vim: set expandtab:
