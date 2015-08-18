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

#include "./flyweight.hpp"
#include "../string/numeric_parser.hpp"

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
    /// @brief 文字列表で使うメモリ割当子を表す型。
    public: typedef template_allocator allocator_type;

    /// @brief 文字列表で使う番号を表す型。
    public: typedef std::size_t number;

    public: enum: typename this_type::number
    {
        /// @brief 文字列表の無効な番号。
        INVALID_NUMBER = static_cast<typename this_type::number>(-1),
        /// @brief 文字列表の列の最大数。
        MAX_COLUMN_COUNT = PSYQ_STRING_TABLE_MAX_COLUMN_COUNT,
        /// @brief 文字列表の行の最大数。
        MAX_ROW_COUNT = 1 + INVALID_NUMBER / MAX_COLUMN_COUNT,
    };

    /// @brief 文字列表で使う、フライ級文字列の型。
    public: typedef
        psyq::string::flyweight<
            template_char_type, template_char_traits, template_allocator>
        string;

    //-------------------------------------------------------------------------
    /** @brief 文字列表のセルのコンテナ。

        - 要素の第1属性は、セル番号。
        - 要素の第2属性は、セル文字列。
     */
    protected: typedef
        std::vector<
            std::pair<typename this_type::number, typename this_type::string>,
            typename this_type::allocator_type>
        cell_container;

    /// @brief セル番号を比較する関数オブジェクト。
    protected: struct cell_number_less
    {
        bool operator()(
            typename table::number const in_left_number,
            typename table::cell_container::value_type const& in_right_cell)
        const PSYQ_NOEXCEPT
        {
            return in_left_number < in_right_cell.first;
        }

        bool operator()(
            typename table::cell_container::value_type const& in_left_cell,
            typename table::number const in_right_number)
        const PSYQ_NOEXCEPT
        {
            return in_left_cell.first < in_right_number;
        }

    }; // struct cell_number_less

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /// @brief 文字列表をコピー構築する。
    public: table(
        /// [in] in_source コピー元となる文字列表。
        this_type const& in_source):
    cells_(in_source.cells_),
    row_count_(in_source.get_row_count()),
    column_count_(in_source.get_column_count())
    {}

    /// @brief 文字列表をムーブ構築する。
    public: table(
        /// [in,out] ムーブ元となる文字列表。
        this_type&& io_source):
    cells_(std::move(io_source.cells_)),
    row_count_(io_source.get_row_count()),
    column_count_(io_source.get_column_count())
    {
        io_source.set_size(0, 0);
    }

    /** @brief 文字列表をコピー代入する。
        @return *this
     */
    public: this_type& operator=(
        /// [in] コピー元となる文字列表。
        this_type const& in_source)
    {
        this->cells_ = in_source.cells_;
        this->set_size(
            in_source.get_row_count(), in_source.get_column_count());
        return *this;
    }

    /** @brief 文字列表をムーブ代入する。
        @return *this
     */
    public: this_type& operator=(
        /// [in,out] io_source ムーブ元となる文字列表。
        this_type&& io_source)
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
    public: typename this_type::number get_row_count() const PSYQ_NOEXCEPT
    {
        return this->row_count_;
    }

    /** @brief 文字列表の列数を取得する。
        @return 文字列表の列数。
     */
    public: typename this_type::number get_column_count() const PSYQ_NOEXCEPT
    {
        return this->column_count_;
    }

    /** @brief 行番号と属性から、文字列表のセルを検索する。
        @return
            行番号と列番号に対応するセルの文字列。
            対応するセルがない場合は、空文字列を返す。
     */
    public: typename this_type::string const& find_cell(
        /// [in] 検索するセルの行番号。
        typename this_type::number const in_row_number,
        /// [in] 検索するセルの列番号。
        typename this_type::number const in_column_number)
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

    /** @brief セル文字列を解析し、値を抽出する。
        @retval true
            成功。セル文字列から抽出した値が out_value へ代入された。
            ただし in_empty_permission が真で、セル文字列が空の場合は、
            out_value への代入は行われず、変化しない。
        @retval false 失敗。 out_value は変化しない。
        @tparam template_value
            セル文字列から抽出する値の型。以下の型の値を抽出できる。
            - bool 型。
            - sizeof(std::uint64_t) 以下の大きさの、組み込み符号なし整数型。
            - sizeof(std::int64_t) 以下の大きさの、組み込み符号あり整数型。
            - sizeof(double) 以下の大きさの、組み込み浮動小数点数型。
            - this_type::string 型。
     */
    public: template<typename template_value>
    bool parse_cell(
        /// [out] 抽出した値の代入先。
        template_value& out_value,
        /// [in] 解析するセルの行番号。
        typename this_type::number const in_row_number,
        /// [in] 解析するセルの列番号。
        typename this_type::number const in_column_number,
        /** [in] セル文字列が空の場合…
            - この引数が真の場合は、成功と判定する。
            - この引数が偽の場合は、失敗と判定する。
         */
        bool const in_empty_permission)
    const
    {
        auto const& local_cell(this->find_cell(in_row_number, in_column_number));
        return (in_empty_permission && local_cell.empty())
            || this_type::parse_string(out_value, local_cell);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @brief 空の文字列表を構築する。
    protected: explicit table(
        /// [in] メモリ割当子の初期値。
        typename this_type::allocator_type const& in_allocator)
    : cells_(in_allocator), row_count_(0), column_count_(0)
    {}

    protected: typename this_type::cell_container const& get_cells()
    const PSYQ_NOEXCEPT
    {
        return this->cells_;
    }

    /// @brief セルを置き換える。
    protected: void replace_cell(
        /// [in] 置き換えるセルの行番号。
        typename this_type::number const in_row_number,
        /// [in] 置き換えるセルの列番号。
        typename this_type::number const in_column_number,
        /// [in] 置き換えるセルの文字列。
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
        typename this_type::number const in_row_count,
        typename this_type::number const in_column_count)
    {
        this->row_count_ = (std::min<typename this_type::number>)(
            in_row_count, this_type::MAX_ROW_COUNT);
        this->column_count_ = (std::min<typename this_type::number>)(
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
        @return セルの行番号。
     */
    protected: static typename this_type::number compute_row_number(
        /// [in] セル番号。
        typename this_type::number const in_cell_number)
    PSYQ_NOEXCEPT
    {
        return in_cell_number / this_type::MAX_COLUMN_COUNT;
    }

    /** @brief セル番号から、セルの列番号を算出する。
        @return セルの列番号。
     */
    protected: static typename this_type::number compute_column_number(
        /// [in] セル番号。
        typename this_type::number const in_cell_number)
    PSYQ_NOEXCEPT
    {
        return in_cell_number % this_type::MAX_COLUMN_COUNT;
    }

    /** @brief セルの行番号と列番号から、セル番号を算出する。
        @return セル番号。
     */
    protected: static typename this_type::number compute_cell_number(
        /// [in] セルの行番号。
        typename this_type::number const in_row_number,
        /// [in] セルの列番号。
        typename this_type::number const in_column_number)
    PSYQ_NOEXCEPT
    {
        auto const local_cell_number(
            in_row_number * this_type::MAX_COLUMN_COUNT + in_column_number);
        PSYQ_ASSERT(
            in_row_number == this_type::compute_row_number(local_cell_number));
        return local_cell_number;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列を解析し、値を抽出する。
        @retval true  成功。文字列を解析して抽出した値を out_value へ代入した。
        @retval false 失敗。 out_value は変化しない。
     */
    private: template<typename template_value>
    static bool parse_string(
        /// [out] 抽出した値の代入先。
        template_value& out_value,
        /// [in] 解析する文字列。
        typename this_type::string::view const& in_string)
    {
        psyq::string::numeric_parser<template_value> const local_parser(
            in_string);
        if (!local_parser.is_completed())
        {
            return false;
        }
        out_value = local_parser.get_value();
        return true;
    }

    private: static bool parse_string(
        typename this_type::string& out_value,
        typename this_type::string const& in_string)
    {
        if (in_string.empty())
        {
            return false;
        }
        out_value = in_string;
        return true;
    }

    //-------------------------------------------------------------------------
    /// @brief セル番号でソートされたセルの辞書。
    private: typename this_type::cell_container cells_;
    /// @brief 文字列表の行数。
    private: typename this_type::number row_count_;
    /// @brief 文字列表の列数。
    private: typename this_type::number column_count_;

}; // class psyq::string::table

#endif // !defined(PSYQ_STRING_TABLE_HPP_)
// vim: set expandtab:
