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
#ifndef PSYQ_STRING_RELATION_TABLE_HPP_
#define PSYQ_STRING_RELATION_TABLE_HPP_

#include "./table.hpp"

/// @cond
namespace psyq
{
    namespace string
    {
        template<typename, typename, typename> class relation_table;
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 関係データベース的なフライ級文字列表。属性と主キーを持つ。
    @tparam template_char_type   @copybrief psyq::string::view::value_type
    @tparam template_char_traits @copybrief psyq::string::view::traits_type
    @tparam template_allocator   @copybrief table::allocator_type
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

    private: typedef typename base_type::string::view string_view;

    //-------------------------------------------------------------------------
    /** @brief 関係文字列表の属性。

        - 第1属性は、属性セルの列番号。
        - 第2属性は、属性の要素数。
     */
    public: typedef
        std::pair<
            typename base_type::string::size_type,
            typename base_type::string::size_type>
        attribute;

    /** @brief 属性の辞書。

        - 要素の第1属性は、属性セルの配列インデクス番号。
        - 要素の第2属性は、属性の要素数。
     */
    private: typedef
        std::vector<
            std::pair<
                typename base_type::string::size_type,
                typename base_type::string::size_type>,
            typename base_type::allocator_type>
        attribute_container;

    /// @brief 属性を名前で比較する関数オブジェクト。
    private: class attribute_name_less
    {
        public: attribute_name_less(
            typename relation_table::cell_container const& in_cells,
            typename relation_table::string::factory::hash::value_type const
                in_hash)
        PSYQ_NOEXCEPT: cells_(in_cells), hash_(in_hash)
        {}

        public: bool operator()(
            typename relation_table::attribute_container::value_type const&
                in_left,
            typename relation_table::attribute_container::value_type const&
                in_right)
        const PSYQ_NOEXCEPT
        {
            auto& local_left(this->cells_.at(in_left.first).second);
            auto& local_right(this->cells_.at(in_right.first).second);
            return local_left.compare_fast(local_right) < 0;
        }

        public: bool operator()(
            typename relation_table::attribute_container::value_type const&
                in_left,
            typename relation_table::string::view const& in_right)
        const PSYQ_NOEXCEPT
        {
            auto& local_left(this->cells_.at(in_left.first).second);
            return local_left.compare_fast(in_right, this->hash_) < 0;
        }

        public: bool operator()(
            typename relation_table::string::view const& in_left,
            typename relation_table::attribute_container::value_type const&
                in_right)
        const PSYQ_NOEXCEPT
        {
            auto& local_right(this->cells_.at(in_right.first).second);
            return 0 < local_right.compare_fast(in_left, this->hash_);
        }

        private: typename relation_table::cell_container const& cells_;
        private:
        typename relation_table::string::factory::hash::value_type hash_;

    }; // class attribute_name_less

    //-------------------------------------------------------------------------
    /** @brief 関係文字列表の主キーのコンテナ。

        要素は、主キーとなるセルの配列インデクス番号。
     */
    private: typedef
        std::vector<
            typename base_type::string::size_type,
            typename base_type::allocator_type>
        key_container;

    /// @brief 関係文字列表の主キーを比較する関数オブジェクト。
    private: class key_less
    {
        public: key_less(
            typename relation_table::cell_container const& in_cells,
            typename relation_table::string::factory::hash::value_type const
                in_hash)
        PSYQ_NOEXCEPT: cells_(in_cells), hash_(in_hash)
        {}

        public: bool operator()(
            typename relation_table::key_container::value_type const&
                in_left,
            typename relation_table::key_container::value_type const&
                in_right)
        const PSYQ_NOEXCEPT
        {
            auto& local_left(this->cells_.at(in_left).second);
            auto& local_right(this->cells_.at(in_right).second);
            return local_left.compare_fast(local_right) < 0;
        }

        public: bool operator()(
            typename relation_table::key_container::value_type const& in_left,
            typename relation_table::string::view const& in_right)
        const PSYQ_NOEXCEPT
        {
            auto& local_left(this->cells_.at(in_left).second);
            return local_left.compare_fast(in_right, this->hash_) < 0;
        }

        public: bool operator()(
            typename relation_table::string::view const& in_left,
            typename relation_table::key_container::value_type const& in_right)
        const PSYQ_NOEXCEPT
        {
            auto& local_right(this->cells_.at(in_right).second);
            return 0 < local_right.compare_fast(in_left, this->hash_);
        }

        private: typename relation_table::cell_container const& cells_;
        private:
        typename relation_table::string::factory::hash::value_type hash_;

    }; // class key_less

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 関係文字列表を構築する。
        @param[in] in_source 元となる文字列表。
        @param[in] in_attribute_row
            属性として使う行の番号。
            base_type::string::npos の場合は、属性の辞書を構築しない。
        @param[in] in_attribute_key
            主キーとして使う属性の名前。
            空文字列の場合は、主キーの辞書を構築しない。
        @param[in] in_attribute_index 主キーとして使う属性のインデクス番号。
     */
    public: explicit relation_table(
        base_type in_source,
        typename base_type::string::size_type const in_attribute_row =
            base_type::string::npos,
        typename base_type::string::view const& in_attribute_key =
            string_view(),
        typename base_type::string::size_type const in_attribute_index = 0)
    :
    base_type(std::move(in_source)),
    attributes_(this->get_allocator()),
    keys_(this->get_allocator()),
    attribute_row_(base_type::string::npos),
    key_column_(base_type::string::npos)
    {
        if (!this->constraint_attribute(in_attribute_row))
        {
            PSYQ_ASSERT(
                in_attribute_row == base_type::string::npos
                && in_attribute_key.empty());
        }
        else if (!this->constraint_key(in_attribute_key, in_attribute_index))
        {
            PSYQ_ASSERT(in_attribute_key.empty());
        }
    }

    /** @brief 関係文字列表をコピー構築する。
        @param[in] in_source コピー元となる文字列表。
     */
    public: relation_table(this_type const& in_source):
    base_type(in_source),
    attributes_(in_source.attributes_),
    keys_(in_source.keys_),
    attribute_row_(in_source.get_attribute_row()),
    key_column_(in_source.get_key_column())
    {}

    /** @brief 関係文字列表をムーブ構築する。
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

    /** @brief 関係文字列表をコピー代入する。
        @param[in] in_source コピー元となる文字列表。
        @return *this
     */
    public: this_type& operator=(this_type const& in_source)
    {
        this->base_type::operator=(in_source);
        this->attributes_ = in_source.attributes_;
        this->keys_ = in_source.keys_;
        this->attribute_row_ = in_source.get_attribute_row();
        this->key_column_ = in_source.get_key_column();
        return *this;
    }

    /** @brief 関係文字列表をムーブ代入する。
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

        this_type::constraint_key で、事前に主キーの辞書を構築をしておくこと。

        @param[in] in_key  検索する主キー。
        @retval !=npos 主キーに対応する行番号。
        @retval ==npos 主キーに対応する行番号が存在しない。
     */
    public: typename base_type::string::size_type find_row_number(
        typename base_type::string::view const& in_key)
    const PSYQ_NOEXCEPT
    {
        if (!in_key.empty())
        {
            auto& local_cells(this->get_cells());
            auto const local_hash(
                this_type::string::factory::compute_hash(in_key));
            auto const local_lower_bound(
                std::lower_bound(
                    this->keys_.begin(),
                    this->keys_.end(),
                    in_key,
                    typename this_type::key_less(local_cells, local_hash)));
            if (local_lower_bound != this->keys_.end())
            {
                auto& local_cell(local_cells.at(*local_lower_bound));
                if (local_cell.second.compare_fast(in_key, local_hash) == 0)
                {
                    return this_type::compute_row_number(local_cell.first);
                }
            }
        }
        return base_type::string::npos;
    }

    /** @brief 属性名から、列番号を検索する。

        this_type::constraint_attribute で、事前に属性の辞書を構築しておくこと。

        @param[in] in_attribute_name  検索する属性の名前。
        @param[in] in_attribute_index 検索する属性のインデクス番号。
        @retval !=npos 属性名に対応する列番号。
        @retval ==npos 属性名に対応する列番号が存在しない。
     */
    public: typename base_type::string::size_type find_column_number(
        typename base_type::string::view const& in_attribute_name,
        typename base_type::string::size_type const in_attribute_index = 0)
    const PSYQ_NOEXCEPT
    {
        if (!in_attribute_name.empty())
        {
            auto const local_attribute(
                this->find_attribute(in_attribute_name));
            if (in_attribute_index < local_attribute.second)
            {
                return local_attribute.first + in_attribute_index;
            }
        }
        return base_type::string::npos;
    }

    /** @brief 行番号と属性から、文字列表の本体セルを検索する。
        @param[in] in_row_number    検索する本体セルの行番号。
        @param[in] in_column_number 検索する本体セルの列番号。
        @return
            行番号と列番号に対応する本体セル。
            対応する本体セルがない場合は、空文字列を返す。
     */
    public: typename base_type::string const& find_body_cell(
        typename base_type::string::size_type const in_row_number,
        typename base_type::string::size_type const in_column_number)
    const PSYQ_NOEXCEPT
    {
        return this->find_cell(
            in_row_number,
            in_row_number != this->get_attribute_row()?
                in_column_number: base_type::MAX_COLUMN_COUNT);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 属性
    //@{
    /** @brief 属性の行番号を取得する。
        @retval !=npos 属性の行番号。
        @retval ==npos 属性の辞書が構築されてない。
     */
    public: typename base_type::string::size_type get_attribute_row()
    const PSYQ_NOEXCEPT
    {
        return this->attribute_row_;
    }

    /** @brief 属性の辞書を構築する。
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

        // 属性行の先頭位置と末尾インデクスを決定する。
        auto& local_cells(this->get_cells());
        auto const local_attribute_begin(
            std::lower_bound(
                local_cells.begin(),
                local_cells.end(),
                base_type::compute_cell_number(in_attribute_row, 0),
                typename base_type::cell_number_less()));
        auto const local_attribute_end(
            base_type::compute_cell_number(in_attribute_row + 1, 0));

        // 属性行を読み取り、属性配列を構築する。
        typename this_type::attribute_container local_attributes(
            this->attributes_.get_allocator());
        local_attributes.reserve(this->get_column_count());
        for (
            auto i(local_attribute_begin);
            i != local_cells.end() && i->first < local_attribute_end;
            ++i)
        {
            auto const local_column_number(
                base_type::compute_column_number(i->first));
            if (!local_attributes.empty())
            {
                // 直前の属性の要素数を決定する。
                auto& local_back(local_attributes.back());
                local_back.second = local_column_number - local_back.second;
            }
            local_attributes.push_back(
                typename this_type::attribute_container::value_type(
                    std::distance(local_cells.begin(), i),
                    local_column_number));
        }

        // 属性となるセルが1つも存在しなかったら、失敗とみなす。
        if (local_attributes.empty())
        {
            return false;
        }

        // 末尾の属性の要素数を決定する。
        auto& local_back(local_attributes.back());
        PSYQ_ASSERT(local_back.second < this->get_column_count());
        local_back.second = this->get_column_count() - local_back.second;

        // 属性名で並び替える。
        std::sort(
            local_attributes.begin(),
            local_attributes.end(),
            typename this_type::attribute_name_less(local_cells, 0));
        this->attribute_row_ = in_attribute_row;
        this->attributes_ = local_attributes;
        this->attributes_.shrink_to_fit();
        return true;
    }

    /** @brief 属性の辞書を削除する。
     */
    public: void clear_attribute()
    {
        this->attributes_.clear();
        this->attribute_row_ = base_type::string::npos;
    }

    /** @brief 属性名から、文字列表の属性を検索する。
        @param[in] in_attribute_name 検索する属性の名前。
        @return 属性の列番号と要素数のペア。
     */
    public: typename this_type::attribute find_attribute(
        typename base_type::string::view const& in_attribute_name)
    const PSYQ_NOEXCEPT
    {
        if (!in_attribute_name.empty())
        {
            auto const local_hash(
                this_type::string::factory::compute_hash(in_attribute_name));
            auto const& local_cells(this->get_cells());
            auto const local_lower_bound(
                std::lower_bound(
                    this->attributes_.begin(),
                    this->attributes_.end(),
                    in_attribute_name,
                    typename this_type::attribute_name_less(
                        local_cells, local_hash)));
            if (local_lower_bound != this->attributes_.end())
            {
                auto& local_cell(local_cells.at(local_lower_bound->first));
                auto const local_compare(
                    local_cell.second.compare_fast(
                        in_attribute_name, local_hash));
                if (local_compare == 0)
                {
                    return typename this_type::attribute(
                        base_type::compute_column_number(local_cell.first),
                        local_lower_bound->second);
                }
            }
        }
        return typename this_type::attribute(base_type::string::npos, 0);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 主キー
    //@{
    /** @brief 主キーの列番号を取得する。
        @retval !=npos 主キーの列番号。
        @retval ==npos 主キーの辞書が構築されてない。
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
        @param[in] in_attribute_index 主キーとする属性のインデクス番号。
        @retval true  成功。
        @retval false 失敗。
     */
    public: bool constraint_key(
        typename base_type::string::view const& in_attribute_name,
        typename base_type::string::size_type const in_attribute_index = 0)
    {
        return this->constraint_key(
            this->find_column_number(in_attribute_name, in_attribute_index));
    }

    /** @brief 主キーの辞書を構築する。
        @param[in] in_key_column 主キーとする列番号。
        @retval true  成功。
        @retval false 失敗。
     */
    public: bool constraint_key(
        typename base_type::string::size_type const in_key_column)
    {
        if (this->get_column_count() <= in_key_column)
        {
            return false;
        }

        // セル辞書から主キー列のセルを読み取り、主キーの配列を構築する。
        auto& local_cells(this->get_cells());
        auto local_row_number(
            base_type::compute_row_number(local_cells.front().first));
        auto const local_cell_begin(local_cells.begin());
        auto const local_cell_end(local_cells.end());
        auto local_cell(local_cell_begin);
        typename this_type::key_container local_keys(
            this->keys_.get_allocator());
        local_keys.reserve(this->get_row_count());
        for (;;)
        {
            if (local_row_number == this->get_attribute_row())
            {
                ++local_row_number;
                continue;
            }
            local_cell = std::lower_bound(
                local_cell,
                local_cell_end, 
                base_type::compute_cell_number(
                    local_row_number, in_key_column),
                typename base_type::cell_number_less());
            if (local_cell == local_cells.end())
            {
                break;
            }
            if (base_type::compute_column_number(local_cell->first)
                == in_key_column)
            {
                local_keys.push_back(local_cell - local_cell_begin);
                PSYQ_ASSERT(local_keys.back() < local_cells.size());
            }
            local_row_number =
                base_type::compute_row_number(local_cell->first) + 1;
        }

        // 主キーとなるセルが1つも存在しなかったら、失敗とみなす。
        if (local_keys.empty())
        {
            return false;
        }

        // 主キーの配列を並び替え、主キーの辞書として正規化する。
        std::sort(
            local_keys.begin(),
            local_keys.end(),
            typename this_type::key_less(local_cells, 0));
        this->key_column_ = in_key_column;
        this->keys_ = local_keys;
        this->keys_.shrink_to_fit();
        return true;
    }

    /** @brief 主キーの辞書を削除する。
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
        auto const local_hash(
            this_type::string::factory::compute_hash(in_key));
        typename base_type::string::size_type local_count(0);
        for (
            auto i(
                std::lower_bound(
                    this->keys_.begin(),
                    this->keys_.end(),
                    in_key,
                    typename this_type::key_less(
                        this->get_cells(), local_hash)));
            i != this->keys_.end();
            ++i)
        {
            auto& local_cell(this->get_cells().at(*i));
            if (local_cell.second.compare_fast(in_key, local_hash) != 0)
            {
                break;
            }
            ++local_count;
        }
        return local_count;
    }
    //@}
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
