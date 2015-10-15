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
/// @file
/// @author Hillco Psychi (https://twitter.com/psychi)
/// @brief @copybrief psyq::string::table
#ifndef PSYQ_STRING_TABLE_HPP_
#define PSYQ_STRING_TABLE_HPP_

#include "./flyweight.hpp"
#include "./numeric_parser.hpp"
#include "../container/sorted_sequence.hpp"

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
/// @brief 行番号と列番号で参照する、フライ級文字列の表。
/// @tparam template_number    @copydoc table::number
/// @tparam template_hasher    @copydoc psyq::string::_private::flyweight_factory::hasher
/// @tparam template_allocator @copydoc _private::flyweight_handle::allocator_type
template<
    typename template_number,
    typename template_hasher = PSYQ_STRING_FLYWEIGHT_HASHER_DEFAULT,
    typename template_allocator = PSYQ_STRING_FLYWEIGHT_ALLOCATOR_DEFAULT>
class psyq::string::table
{
    /// @copydoc psyq::string::view::this_type
    private: typedef table this_type;

    //-------------------------------------------------------------------------
    /// @brief 文字列表で使う番号を表す型。
    public: typedef template_number number;
    /// @brief 文字列表で使う、フライ級文字列の型。
    public: typedef
        psyq::string::flyweight<template_hasher, template_allocator>
        string;
    /// @brief セル番号をキーとする、文字列表のセルの辞書。
    public: typedef
        psyq::container::sequence_map<
            std::vector<
                std::pair<
                    typename this_type::number, typename this_type::string>,
                template_allocator>>
        cell_map;
    public: enum: typename this_type::number
    {
        /// @brief 文字列表の無効な番号。
        INVALID_NUMBER = static_cast<typename this_type::number>(-1),
        /// @brief 文字列表の列の最大数。
        MAX_COLUMN_COUNT = PSYQ_STRING_TABLE_MAX_COLUMN_COUNT,
        /// @brief 文字列表の行の最大数。
        MAX_ROW_COUNT = 1 + INVALID_NUMBER / MAX_COLUMN_COUNT,
    };

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    /// @{

    /// @brief 文字列表をコピー構築する。
    public: table(
        /// [in] in_source コピー元となる文字列表。
        this_type const& in_source):
    cells_(in_source.get_cells()),
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
        io_source.cells_.clear();
        io_source.set_size(0, 0);
    }

    /// @brief 文字列表をコピー代入する。
    /// @return *this
    public: this_type& operator=(
        /// [in] コピー元となる文字列表。
        this_type const& in_source)
    {
        this->cells_ = in_source.get_cells();
        this->set_size(
            in_source.get_row_count(), in_source.get_column_count());
        return *this;
    }

    /// @brief 文字列表をムーブ代入する。
    /// @return *this
    public: this_type& operator=(
        /// [in,out] io_source ムーブ元となる文字列表。
        this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->cells_ = std::move(io_source.cells_);
            this->set_size(
                io_source.get_row_count(), io_source.get_column_count());
            io_source.cells_.clear();
            io_source.set_size(0, 0);
        }
        return *this;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name セル
    /// @{

    /// @brief 文字列表のセルの辞書を取得する。
    /// @return 文字列表のセルの辞書。
    public: typename this_type::cell_map const& get_cells() const PSYQ_NOEXCEPT
    {
        return this->cells_;
    }

    /// @brief 文字列表の行数を取得する。
    /// @return 文字列表の行数。
    public: typename this_type::number get_row_count() const PSYQ_NOEXCEPT
    {
        return this->row_count_;
    }

    /// @brief 文字列表の列数を取得する。
    /// @return 文字列表の列数。
    public: typename this_type::number get_column_count() const PSYQ_NOEXCEPT
    {
        return this->column_count_;
    }

    /// @brief 行番号と属性から、文字列表のセルを検索する。
    /// @return
    ///   行番号と列番号に対応するセルの文字列。
    ///   対応するセルがない場合は、空文字列を返す。
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
            auto const local_find(
                this->get_cells().find(
                    this_type::compute_cell_number(
                        in_row_number, in_column_number)));
            if (local_find != std::end(this->get_cells()))
            {
                return local_find->second;
            }
        }
        static typename this_type::string const static_empty_string;
        return static_empty_string;
    }

    /// @brief セル文字列を解析し、値を抽出する。
    /// @retval true
    ///   成功。セル文字列から抽出した値が out_value へ代入された。
    ///   ただし in_empty_permission が真で、セル文字列が空の場合は、
    ///   out_value への代入は行われずに変化しない。
    /// @retval false 失敗。 out_value は変化しない。
    /// @tparam template_value
    ///   セル文字列から抽出する値の型。以下の型の値を抽出できる。
    ///   - bool 型。
    ///   - sizeof(std::uint64_t) 以下の大きさの、組み込み符号なし整数型。
    ///   - sizeof(std::int64_t) 以下の大きさの、組み込み符号あり整数型。
    ///   - sizeof(double) 以下の大きさの、組み込み浮動小数点数型。
    ///   - this_type::string 型。
    public: template<typename template_value>
    bool parse_cell(
        /// [out] 抽出した値の代入先。
        template_value& out_value,
        /// [in] 解析するセルの行番号。
        typename this_type::number const in_row_number,
        /// [in] 解析するセルの列番号。
        typename this_type::number const in_column_number,
        /// [in] セル文字列が空の場合…
        /// - この引数が真の場合は、成功と判定する。
        /// - この引数が偽の場合は、失敗と判定する。
        bool const in_empty_permission)
    const
    {
        auto const& local_cell(
            this->find_cell(in_row_number, in_column_number));
        return (in_empty_permission && local_cell.empty())
            || this_type::parse_string(out_value, local_cell);
    }
    /// @}

    /// @brief セル番号から、行番号を算出する。
    /// @return in_cell_number の行番号。
    public: static typename this_type::number compute_row_number(
        /// [in] 行番号を算出するセル番号。
        typename this_type::number const in_cell_number)
    PSYQ_NOEXCEPT
    {
        return in_cell_number / this_type::MAX_COLUMN_COUNT;
    }

    /// @brief セル番号から、列番号を算出する。
    /// @return in_cell_number の列番号。
    public: static typename this_type::number compute_column_number(
        /// [in] 列番号を算出するセル番号。
        typename this_type::number const in_cell_number)
    PSYQ_NOEXCEPT
    {
        return in_cell_number % this_type::MAX_COLUMN_COUNT;
    }

    /// @brief セルの行番号と列番号から、セル番号を算出する。
    /// @return セル番号。
    public: static typename this_type::number compute_cell_number(
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
    /// @brief 空の文字列表を構築する。
    protected: explicit table(
        /// [in] メモリ割当子の初期値。
        template_allocator const& in_allocator):
    cells_(typename this_type::cell_map::container_type(in_allocator)),
    row_count_(0),
    column_count_(0)
    {}

    /// @brief 文字列表を再構築する。
    protected: bool assign_cells(
        /// [in] ソート済のセルのコンテナ。
        typename this_type::cell_map::container_type in_cells,
        /// [in] 文字列表の行数。
        typename this_type::number const in_row_count,
        /// [in] 文字列表の列数。
        typename this_type::number const in_column_count)
    {
        if (!this->cells_.assign(std::move(in_cells)))
        {
            PSYQ_ASSERT(false);
            return false;
        }
        this->set_size(in_row_count, in_column_count);
        return true;
    }

    //-------------------------------------------------------------------------
    /// @brief 文字列表の大きさを決定する。
    private: void set_size(
        /// [in] 文字列表の行数。
        typename this_type::number const in_row_count,
        /// [in] 文字列表の列数。
        typename this_type::number const in_column_count)
    {
        this->row_count_ = (std::min<typename this_type::number>)(
            in_row_count, this_type::MAX_ROW_COUNT);
        this->column_count_ = (std::min<typename this_type::number>)(
            in_column_count, this_type::MAX_COLUMN_COUNT);
    }

    /// @brief 文字列を解析し、値を抽出する。
    /// @retval true  成功。文字列を解析して抽出した値を out_value へ代入した。
    /// @retval false 失敗。 out_value は変化しない。
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
    private: typename this_type::cell_map cells_;
    /// @brief 文字列表の行数。
    private: typename this_type::number row_count_;
    /// @brief 文字列表の列数。
    private: typename this_type::number column_count_;

}; // class psyq::string::table

#endif // !defined(PSYQ_STRING_TABLE_HPP_)
// vim: set expandtab:
