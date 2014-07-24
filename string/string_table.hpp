/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

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

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
    @brief @copybrief psyq::string_table
 */
#ifndef PSYQ_STRING_TABLE_HPP_
#define PSYQ_STRING_TABLE_HPP_

#include <unordered_map>
//#include "psyq/string/shared_string.hpp"

namespace psyq
{
    /// @cond
    template<typename> class string_table;
    /// @endcond

    /// psyq::shared_string を使った、文字列の表。
    typedef psyq::string_table<
        std::unordered_map<std::size_t, psyq::shared_string>>
            shared_string_table;

    //ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
    namespace internal
    {
        /** @brief 文字列を解析し、数値に変換する。
            @param[out] out_value
              - 文字列の解析に成功した場合、解析して取り出した値が代入される。
              - 文字列の解析に失敗した場合、代入は行われない。
            @param[in]  in_string  解析する文字列。
            @retval true  文字列の解析に成功した。
            @retval false 文字列の解析に失敗した。
         */
        template<typename template_number_type, typename template_string_type>
        bool string_table_deserialize_number(
            template_number_type& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            if (in_string == nullptr)
            {
                return false;
            }

            // 文字列の前後の空白を取り除く。
            typedef psyq::basic_string_view<
                typename template_string_type::value_type,
                typename template_string_type::traits_type>
                    string_view;
            auto const local_string(string_view(*in_string).trim_copy());
            if (local_string.empty())
            {
                return false;
            }

            // 文字列を解析し、値を取り出す。
            std::size_t local_rest_size(0);
            auto const local_value(
                local_string.template make_number<template_number_type>(
                    &local_rest_size));
            if (0 < local_rest_size)
            {
                return false;
            }
            out_value = local_value;
            return true;
        }

        /** @brief 文字列を解析し、文字列に変換する。
            @param[out] out_value
              - 文字列の解析に成功した場合、解析して取り出した値が代入される。
              - 文字列の解析に失敗した場合、代入は行われない。
            @param[in]  in_string  解析する文字列。
            @retval true  文字列の解析に成功した。
            @retval false 文字列の解析に失敗した。
         */
        template<typename template_value_type, typename template_string_type>
        bool string_table_deserialize(
            template_value_type& out_value,
            template_string_type const* const in_string)
        {
            if (in_string == nullptr)
            {
                return false;
            }

            // 文字列の前後の空白を取り除く。
            typedef psyq::basic_string_view<
                typename template_string_type::value_type,
            typename template_string_type::traits_type>
                string_view;
            auto const local_string(string_view(*in_string).trim_copy());
            if (local_string.empty())
            {
                return false;
            }
            out_value = local_string;
            return true;
        }

        /// @copydoc psyq::internal::string_table_deserialize_number()
        template<typename template_string_type>
        bool string_table_deserialize(
            signed char& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            return psyq::internal::string_table_deserialize_number(
                out_value, in_string);
        }

        /// @copydoc psyq::internal::string_table_deserialize_number()
        template<typename template_string_type>
        bool string_table_deserialize(
            unsigned char& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            return psyq::internal::string_table_deserialize_number(
                out_value, in_string);
        }

        /// @copydoc psyq::internal::string_table_deserialize_number()
        template<typename template_string_type>
        bool string_table_deserialize(
            signed short& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            return psyq::internal::string_table_deserialize_number(
                out_value, in_string);
        }

        /// @copydoc psyq::internal::string_table_deserialize_number()
        template<typename template_string_type>
        bool string_table_deserialize(
            unsigned short& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            return psyq::internal::string_table_deserialize_number(
                out_value, in_string);
        }

        /// @copydoc psyq::internal::string_table_deserialize_number()
        template<typename template_string_type>
        bool string_table_deserialize(
            signed int& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            return psyq::internal::string_table_deserialize_number(
                out_value, in_string);
        }

        /// @copydoc psyq::internal::string_table_deserialize_number()
        template<typename template_string_type>
        bool string_table_deserialize(
            unsigned int& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            return psyq::internal::string_table_deserialize_number(
                out_value, in_string);
        }

        /// @copydoc psyq::internal::string_table_deserialize_number()
        template<typename template_string_type>
        bool string_table_deserialize(
            signed long& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            return psyq::internal::string_table_deserialize_number(
                out_value, in_string);
        }

        /// @copydoc psyq::internal::string_table_deserialize_number()
        template<typename template_string_type>
        bool string_table_deserialize(
            unsigned long& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            return psyq::internal::string_table_deserialize_number(
                out_value, in_string);
        }

        /// @copydoc psyq::internal::string_table_deserialize_number()
        template<typename template_string_type>
        bool string_table_deserialize(
            signed long long& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            return psyq::internal::string_table_deserialize_number(
                out_value, in_string);
        }

        /// @copydoc psyq::internal::string_table_deserialize_number()
        template<typename template_string_type>
        bool string_table_deserialize(
            unsigned long long& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            return psyq::internal::string_table_deserialize_number(
                out_value, in_string);
        }

        /// @copydoc psyq::internal::string_table_deserialize_number()
        template<typename template_string_type>
        bool string_table_deserialize(
            float& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            return psyq::internal::string_table_deserialize_number(
                out_value, in_string);
        }

        /// @copydoc psyq::internal::string_table_deserialize_number()
        template<typename template_string_type>
        bool string_table_deserialize(
            double& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            return psyq::internal::string_table_deserialize_number(
                out_value, in_string);
        }

        /// @copydoc psyq::internal::string_table_deserialize_number()
        template<typename template_string_type>
        bool string_table_deserialize(
            long double& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            return psyq::internal::string_table_deserialize_number(
                out_value, in_string);
        }
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 文字列の表。CSV形式の文字列から構築する。

    使い方の概略。
    -# this_type::make_column_map() で、列の辞書を構築する。
    -# this_type::string_table() に列の辞書を渡し、文字列表を構築する。
    -# this_type::deserialize_body_cell() で任意のcell文字列を解析し、値を取り出す。

    @tparam template_cell_map @copydoc this_type::cell_map
 */
template<typename template_cell_map>
class psyq::string_table
{
    private: typedef string_table this_type; ///< thisが指す値の型。

    /** @brief 文字列表のcellの辞書。

        - std::unordered_map 互換の型。
        - this_type::cell_map::key_type は、文字列表の行の番号。
        - this_type::cell_map::mapped_type は、文字列表のcell文字列となる、
          psyq::basic_shared_string 相当の文字列。
     */
    public: typedef template_cell_map cell_map;

    /// @brief 文字列表の列辞書の要素。
    private: typedef std::pair<
        typename this_type::cell_map::key_type const, typename this_type::cell_map>
            column_map_element;

    /** @brief 文字列表の列の辞書。

        - this_type::column_map::key_type は、文字列表の列の番号。
        - this_type::column_map::mapped_type は、文字列表の列が持つ、cellの辞書。

        @note 2014.04.05
            文字列表では行の辞書を保持することも考えたが、
            属性の追加や削除などといった表の編集を考えると、
            列の辞書を保持したほうがいいように思う。
     */
    public: typedef std::unordered_map<
        typename this_type::cell_map::key_type,
        typename this_type::cell_map,
        std::hash<typename this_type::cell_map::key_type>,
        std::equal_to<typename this_type::cell_map::key_type>,
        typename this_type::cell_map::allocator_type::template
            rebind<typename this_type::column_map_element>::other>
                column_map;

    /// @brief 文字列表のcellとなる、 psyq::basic_shared_string 相当の文字列。
    public: typedef typename this_type::cell_map::mapped_type cell;

    /// @brief 文字列表のcell文字列への参照。
    public: typedef psyq::basic_string_view<
        typename this_type::cell::value_type const,
        typename this_type::cell::traits_type>
            cell_view;

    /// @brief 文字列表の列の属性。
    public: struct attribute
    {
        PSYQ_CONSTEXPR attribute(
            typename this_type::column_map::key_type const in_column,
            typename this_type::column_map::key_type const in_size)
        PSYQ_NOEXCEPT:
            column(in_column),
            size(in_size)
        {}

        typename this_type::column_map::key_type column; ///< 属性の列番号。
        typename this_type::column_map::key_type size;   ///< 属性の要素数。
    };

    /// @brief 文字列表の列属性の辞書の要素。
    private: typedef std::pair<
        typename this_type::cell_view const, typename this_type::attribute>
            attribute_map_element;

    /** @brief 文字列表の列属性の辞書。

        - this_type::attribute_map::key_type は、文字列表の列属性の名前。
        - this_type::attribute_map::mapped_type は、文字列表の列属性。
     */
    public: typedef std::unordered_map<
        typename this_type::cell_view,
        typename this_type::attribute,
        typename this_type::cell_view::fnv1_hash,
        std::equal_to<typename this_type::cell_view>,
        typename this_type::cell_map::allocator_type::template
            rebind<typename this_type::attribute_map_element>::other>
                attribute_map;

    //-------------------------------------------------------------------------
    /// @name 文字列表の構築と破棄
    //@{
    /** @brief 列の辞書から文字列表を構築する。
        @param[in] in_column_map
            this_type::make_column_map() で構築した、列の辞書。
        @param[in] in_attribute_row 文字列表の列属性として使う、行の番号。
     */
    public: string_table(
        typename this_type::column_map in_column_map,
        typename this_type::cell_map::key_type const in_attribute_row)
    :
        attribute_map_(
            this_type::make_attribute_map(in_column_map, in_attribute_row)),
        attribute_row_(in_attribute_row),
        column_map_(std::move(in_column_map))
    {}

    /** @brief move構築子。
        @param[in,out] io_source move元となる文字列表。
     */
    public: string_table(this_type&& io_source):
        attribute_map_(std::move(io_source.attribute_map_)),
        attribute_row_(std::move(io_source.attribute_row_)),
        column_map_(std::move(io_source.column_map_))
    {
        if (this != &io_source)
        {
            io_source.clear();
        }
    }

    //public: this_type& operator=(this_type const& in_source) = default;

    /** @brief move代入演算子。
        @param[in,out] io_source move元となる文字列表。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->attribute_map_ = std::move(io_source.attribute_map_);
            this->attribute_row_ = std::move(io_source.attribute_row_);
            this->column_map_ = std::move(io_source.column_map_);
            io_source.clear();
        }
        return *this;
    }

    /** @brief 文字列表を空にする。
     */
    public: void clear()
    {
        this->attribute_map_.clear();
        this->attribute_row_ = 0;
        this->column_map_.clear();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列表の情報
    //@{
    /** @brief 文字列表の列の辞書を取得する。
        @return @copydoc column_map
     */
    public: typename this_type::column_map const& get_column_map()
    const PSYQ_NOEXCEPT
    {
        return this->column_map_;
    }

    /** @brief 文字列表の列属性の辞書を取得する。
        @return @copydoc attribute_map
     */
    public: typename this_type::attribute_map const& get_attribute_map()
    const PSYQ_NOEXCEPT
    {
        return this->attribute_map_;
    }

    /** @brief 文字列表の列属性として使っている行の番号を取得する。
        @return @copydoc attribute_row_
     */
    public: typename this_type::cell_map::key_type get_attribute_row()
    const PSYQ_NOEXCEPT
    {
        return this->attribute_row_;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name cellの解析
    //@{
    /** @brief 文字列表の本体のcell文字列を解析し、値を取り出す。
        @param[out] out_value
          - cell文字列の解析に成功した場合、解析して取り出した値が代入される。
          - cell文字列の解析に失敗した場合、代入は行われない。
        @param[in] in_row_key         解析するcellの、行番号。
        @param[in] in_attribute_key   解析するcellの、列属性の名前。
        @param[in] in_attribute_index 解析するcellの、列属性のindex番号。
        @retval true  cell文字列の解析に成功した。
        @retval false cell文字列の解析に失敗した。
     */
    public: template<typename template_value_type>
    bool deserialize_body_cell(
        template_value_type& out_value,
        typename this_type::cell_map::key_type const in_row_key,
        typename this_type::cell_view const& in_attribute_key,
        typename this_type::column_map::key_type const in_attribute_index = 0)
    const
    {
        return psyq::internal::string_table_deserialize(
            out_value,
            this->find_body_cell(
                in_row_key, in_attribute_key, in_attribute_index));
    }

    /** @brief 文字列表の本体のcell文字列を解析し、値を取り出す。
        @param[out] out_value
          - cell文字列の解析に成功した場合、解析して取り出した値が代入される。
          - cell文字列の解析に失敗した場合、代入は行われない。
        @param[in] in_row_key    解析するcellの、行番号。
        @param[in] in_column_key 解析するcellの、列番号。
        @retval true  cell文字列の解析に成功した。
        @retval false cell文字列の解析に失敗した。
     */
    public: template<typename template_value_type>
    bool deserialize_body_cell(
        template_value_type& out_value,
        typename this_type::cell_map::key_type const in_row_key,
        typename this_type::column_map::key_type const in_column_key)
    const
    {
        return psyq::internal::string_table_deserialize(
            out_value, this->find_body_cell(in_row_key, in_column_key));
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name cellの取得
    //@{
    /** @brief 文字列表の本体を検索し、cell文字列を取得する。
        @param[in] in_row_key         取得したいcellの、行番号。
        @param[in] in_attribute_key   取得したいcellの、列属性の名前。
        @param[in] in_attribute_index 取得したいcellの、列属性のindex番号。
        @retval !=nullptr 見つかったcell文字列。
        @retval ==nullptr 対応するcellが見つからなかった。
     */
    public: typename this_type::cell const* find_body_cell(
        typename this_type::cell_map::key_type const in_row_key,
        typename this_type::cell_view const& in_attribute_key,
        typename this_type::column_map::key_type const in_attribute_index = 0)
    const
    {
        return in_row_key != this->get_attribute_row()?
            this_type::find_column_cell(
                this->get_column_map(),
                in_row_key,
                this->get_attribute_map(),
                in_attribute_key,
                in_attribute_index):
            nullptr;
    }

    /** @brief 文字列表の本体を検索し、cell文字列を取得する。
        @param[in] in_row_key    取得したいcellの、行番号。
        @param[in] in_column_key 取得したいcellの、列番号。
        @retval !=nullptr 見つかったcell文字列。
        @retval ==nullptr 対応するcellが見つからなかった。
     */
    public: typename this_type::cell const* find_body_cell(
        typename this_type::cell_map::key_type const in_row_key,
        typename this_type::column_map::key_type const in_column_key)
    const
    {
        return in_row_key != this->get_attribute_row()?
            this_type::find_column_cell(
                this->get_column_map(), in_row_key, in_column_key):
            nullptr;
    }
    //@}
    /** @brief 列の辞書を検索し、cell文字列を取得する。
        @param[in] in_column_map      検索する列の辞書。
        @param[in] in_row_key         取得したいcellの、行番号。
        @param[in] in_attribute_map   検索する列属性の辞書。
        @param[in] in_attribute_key   取得したいcellの、列属性の名前。
        @param[in] in_attribute_index 取得したいcellの、列属性のindex番号。
        @retval !=nullptr 見つかったcell文字列。
        @retval ==nullptr 対応するcellが見つからなかった。
     */
    public: static typename this_type::cell const* find_column_cell(
        typename this_type::column_map const& in_column_map,
        typename this_type::cell_map::key_type const in_row_key,
        typename this_type::attribute_map const& in_attribute_map,
        typename this_type::cell_view const& in_attribute_key,
        typename this_type::column_map::key_type const in_attribute_index = 0)
    {
        auto const local_cell_map(
            this_type::find_cell_map(
                in_column_map,
                in_attribute_map,
                in_attribute_key,
                in_attribute_index));
        if (local_cell_map != nullptr)
        {
            auto const local_row_iterator(local_cell_map->find(in_row_key));
            if (local_row_iterator != local_cell_map->end())
            {
                return &local_row_iterator->second;
            }
        }
        return nullptr;
    }

    /** @brief 列の辞書を検索し、cell文字列を取得する。
        @param[in] in_column_map 検索する列の辞書。
        @param[in] in_row_key    取得したいcellの、行番号。
        @param[in] in_column_key 取得したいcellの、列番号。
        @retval !=nullptr 見つかったcell文字列。
        @retval ==nullptr 対応するcellが見つからなかった。
     */
    public: static typename this_type::cell const* find_column_cell(
        typename this_type::column_map const& in_column_map,
        typename this_type::cell_map::key_type const in_row_key,
        typename this_type::column_map::key_type const in_column_key)
    {
        auto const local_cell_map(
            this_type::find_cell_map(in_column_map, in_column_key));
        if (local_cell_map != nullptr)
        {
            auto const local_row_iterator(local_cell_map->find(in_row_key));
            if (local_row_iterator != local_cell_map->end())
            {
                return &local_row_iterator->second;
            }
        }
        return nullptr;
    }

    //-------------------------------------------------------------------------
    /// @name cellの辞書の取得
    //@{
    /** @brief 文字列表を検索し、cellの辞書を取得する。
        @param[in] in_attribute_key   取得したいcellの辞書の、列属性の名前。
        @param[in] in_attribute_index 取得したいcellの辞書の、列属性のindex番号。
        @retval !=nullptr 見つかったcellの辞書。
        @retval ==nullptr 対応するcellの辞書が見つからなかった。
     */
    public: typename this_type::cell_map const* find_cell_map(
        typename this_type::cell_view const& in_attribute_key,
        typename this_type::column_map::key_type const in_attribute_index = 0)
    const
    {
        return this_type::find_cell_map(
            this->get_column_map(),
            this->get_attribute_map(),
            in_attribute_key,
            in_attribute_index);
    }

    /** @brief 文字列表を検索し、cellの辞書を取得する。
        @param[in] in_column_key 取得したいcellの辞書の、列番号。
        @retval !=nullptr 見つかったcellの辞書。
        @retval ==nullptr 対応するcellの辞書が見つからなかった。
     */
    public: typename this_type::cell_map const* find_cell_map(
        typename this_type::column_map::key_type const in_column_key)
    const
    {
        return this_type::find_cell_map(this->get_column_map(), in_column_key);
    }
    //@}
    /** @brief 列の辞書を検索し、cellの辞書を取得する。
        @param[in] in_column_map      検索する列の辞書。
        @param[in] in_attribute_map   検索する列属性の辞書。
        @param[in] in_attribute_key   取得したいcellの辞書の、列属性の名前。
        @param[in] in_attribute_index 取得したいcellの辞書の、列属性のindex番号。
        @retval !=nullptr 見つかったcellの辞書。
        @retval ==nullptr 対応するcellの辞書が見つからなかった。
     */
    public: static typename this_type::cell_map const* find_cell_map(
        typename this_type::column_map const& in_column_map,
        typename this_type::attribute_map const& in_attribute_map,
        typename this_type::cell_view const& in_attribute_key,
        typename this_type::column_map::key_type const in_attribute_index = 0)
    {
        auto const local_attribute_iterator(
            in_attribute_map.find(
                typename this_type::cell_view(in_attribute_key).trim_copy()));
        if (local_attribute_iterator != in_attribute_map.end())
        {
            auto& local_attribute(local_attribute_iterator->second);
            if (in_attribute_index < local_attribute.size)
            {
                return this_type::find_cell_map(
                    in_column_map,
                    local_attribute.column + in_attribute_index);
            }
        }
        return nullptr;
    }

    /** @brief 列の辞書を検索し、cellの辞書を取得する。
        @param[in] in_column_map 検索する列の辞書。
        @param[in] in_column_key 取得したいcellの辞書の、列番号。
        @retval !=nullptr 見つかったcellの辞書。
        @retval ==nullptr 対応するcellの辞書が見つからなかった。
     */
    public: static typename this_type::cell_map const* find_cell_map(
        typename this_type::column_map const& in_column_map,
        typename this_type::column_map::key_type const in_column_key)
    {
        auto const local_column_iterator(in_column_map.find(in_column_key));
        return local_column_iterator != in_column_map.end()?
            &local_column_iterator->second: nullptr;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表の列の辞書から、列属性の辞書を構築する。
        @param[in] in_column_map    列の辞書。
        @param[in] in_attribute_row 列属性として使う行の番号。
     */
    private: static typename this_type::attribute_map make_attribute_map(
        typename this_type::column_map const& in_column_map,
        typename this_type::column_map::key_type const in_attribute_row)
    {
        // 列属性の辞書を構築する。
        typename this_type::attribute_map local_attribute_map(
            in_column_map.get_allocator());
        typename this_type::column_map::key_type local_column_max(0);
        for (auto& local_column_value: in_column_map)
        {
            // 最大行を更新する。
            if (local_column_max < local_column_value.first)
            {
                local_column_max = local_column_value.first;
            }

            // 属性行を取得する。
            auto& local_cell_map(local_column_value.second);
            auto const local_row_iterator(
                local_cell_map.find(in_attribute_row));
            if (local_row_iterator == local_cell_map.end())
            {
                continue; // 属性行ではなかった。
            }

            // 列属性の辞書を更新する。
            auto const local_attribute_iterator(
                local_attribute_map.find(
                    typename this_type::attribute_map::key_type(
                        local_row_iterator->second)));
            if (local_attribute_iterator == local_attribute_map.end())
            {
                local_attribute_map.emplace(
                    typename this_type::attribute_map::key_type(
                        local_row_iterator->second).trim_copy(),
                    typename this_type::attribute_map::mapped_type(
                        local_column_value.first, 0));
            }
            else
            {
                PSYQ_ASSERT(false); // 名前が重複する列属性がある。
            }
        }

        // 属性ごとの要素数を決定する。
        this_type::adjust_attribute_size(local_attribute_map, local_column_max);
        return local_attribute_map;
    }

    /** @brief 列属性の要素数を決定する。
        @param[in,out] io_attribute_map 要素数を決定する、列属性の辞書。
        @param[in]     in_column_max    列番号の最大値。
     */
    private: static void adjust_attribute_size(
        typename this_type::attribute_map& io_attribute_map,
        typename this_type::column_map::key_type const in_column_max)
    {
        if (io_attribute_map.empty())
        {
            return;
        }

        // 属性の配列を構築する。
        typename this_type::attribute_map::allocator_type::template
            rebind<typename this_type::attribute_map::value_type*>::other
                local_allocator(io_attribute_map.get_allocator());
        auto const local_attribute_begin(
            local_allocator.allocate(io_attribute_map.size()));
        PSYQ_ASSERT(local_attribute_begin != nullptr);
        auto local_attribute_end(local_attribute_begin);
        for (auto& local_attribute_value: io_attribute_map)
        {
            *local_attribute_end = &local_attribute_value;
            ++local_attribute_end;
        }

        // 属性の配列を、列番号でsortする。
        std::sort(
            local_attribute_begin,
            local_attribute_end,
            [](
                typename this_type::attribute_map::value_type const* const in_left,
                typename this_type::attribute_map::value_type const* const in_right)
            ->bool
            {
                return in_left->second.column < in_right->second.column;
            });

        // 属性の要素数を決定する。
        auto local_last_column(in_column_max + 1);
        for (auto i(local_attribute_end - 1); local_attribute_begin <= i; --i)
        {
            auto& local_attribute((**i).second);
            local_attribute.size = local_last_column - local_attribute.column;
            local_last_column = local_attribute.column;
        }

        // 属性の配列を破棄する。
        local_allocator.deallocate(
            local_attribute_begin, io_attribute_map.size());
    }

    //-------------------------------------------------------------------------
    /** @brief CSV形式の文字列を解析し、列の辞書を構築する。
        @tparam template_string std::basic_string 互換の文字列型。
        @param[in] in_csv_string       解析するCSV形式の文字列。
        @param[in] in_column_ceparator 列の区切り文字。
        @param[in] in_row_separator    行の区切り文字。
        @param[in] in_quote_begin      引用符の開始文字。
        @param[in] in_quote_end        引用符の終了文字。
        @param[in] in_quote_escape     引用符のescape文字。
        @return CSV形式の文字列を解析して構築した、列の辞書。
     */
    public: template<typename template_string>
    static typename this_type::column_map make_column_map(
        template_string const& in_csv_string,
        typename template_string::value_type const in_column_ceparator = ',',
        typename template_string::value_type const in_row_separator = '\n',
        typename template_string::value_type const in_quote_begin = '"',
        typename template_string::value_type const in_quote_end = '"',
        typename template_string::value_type const in_quote_escape = '"')
    {
        PSYQ_ASSERT(in_quote_escape != 0);
        bool local_quote(false);
        typename this_type::cell_map::key_type local_row(0);
        typename this_type::column_map::key_type local_column(0);
        typename this_type::column_map::key_type local_max_column(0);
        template_string local_cell(in_csv_string.get_allocator());
        typename this_type::column_map local_column_map(
            in_csv_string.get_allocator());
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
                            local_cell.push_back(*i);
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
                    local_cell.push_back(*i);
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
                    local_cell.push_back(local_last_char);
                    local_cell.push_back(*i);
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
                if (!local_cell.empty())
                {
                    local_column_map[local_column][local_row]
                        = typename this_type::cell(local_cell);
                    local_cell.clear();
                }
                ++local_column;
            }
            else if (*i == in_row_separator)
            {
                // 行の区切り。
                if (!local_cell.empty())
                {
                    local_column_map[local_column][local_row]
                        = typename this_type::cell(local_cell);
                    local_cell.clear();
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
                local_cell.push_back(*i);
            }
        }

        // 最終cellの処理。
        if (local_quote)
        {
            // 引用符の開始はあったが、終了がなかった場合。
            //local_cell.insert(local_cell.begin(), in_quote_begin);
        }
        if (!local_cell.empty())
        {
            local_column_map[local_column][local_row]
                = typename this_type::cell(local_cell);
        }
        else if (0 < local_column)
        {
            --local_column;
        }
        if (local_max_column < local_column)
        {
            local_max_column = local_column;
        }
        return local_column_map;
    }

    //-------------------------------------------------------------------------
    /// 文字列表の列属性の辞書。
    private: typename this_type::attribute_map attribute_map_;
    /// 文字列表の列属性として使っている行の番号。
    private: typename this_type::cell_map::key_type attribute_row_;
    /// 文字列表の列の辞書。
    private: typename this_type::column_map column_map_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        inline void string_table()
        {
            psyq::shared_string_table const local_string_table(
                psyq::shared_string_table::make_column_map(
                    std::string(
                        "name,   path,            count                \n"
                        "taro,   taro.txt,        -12.34567890e+2, 3, 2\n"
                        "yamada, data/yamada.txt, 1234567890           \n")),
                0);

            psyq::shared_string local_shared_string("abc");
            local_shared_string.get_allocator();

            psyq::string_view local_name;
            PSYQ_ASSERT(
                local_string_table.deserialize_body_cell(
                    local_name, 1, "name"));
            PSYQ_ASSERT(local_name == "taro");
            PSYQ_ASSERT(
                local_string_table.deserialize_body_cell(
                    local_name, 2, "  name  "));
            PSYQ_ASSERT(local_name == "yamada");
            PSYQ_ASSERT(
                !local_string_table.deserialize_body_cell(
                    local_name, 2, "name", 1));
            PSYQ_ASSERT(local_name == "yamada");
            PSYQ_ASSERT(
                !local_string_table.deserialize_body_cell(
                    local_name, 3, "name"));
            PSYQ_ASSERT(local_name == "yamada");
            PSYQ_ASSERT(
                local_string_table.deserialize_body_cell(
                    local_name, 1, "path"));
            PSYQ_ASSERT(local_name == "taro.txt");
            int local_integer;
            PSYQ_ASSERT(
                !local_string_table.deserialize_body_cell(
                    local_integer, 1, "count", 0));
            double local_real(0);
            PSYQ_ASSERT(
                local_string_table.deserialize_body_cell(
                    local_real, 1, "count", 0));
            PSYQ_ASSERT(
                local_string_table.deserialize_body_cell(
                    local_integer, 2, "count", 0));
            PSYQ_ASSERT(local_integer == 1234567890);
        }
    }
}
#endif // !defined(PSYQ_STRING_TABLE_HPP_)
