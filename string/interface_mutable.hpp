/*
Copyright (c) 2013, Hillco Psychi, All rights reserved.

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
    @brief @copybrief psyq::string::_private::interface_mutable
 */
#ifndef PSYQ_STRING_MUTABLE_INTERFACE_HPP_
#define PSYQ_STRING_MUTABLE_INTERFACE_HPP_

//#include "string/interface_immutable.hpp"

/// @cond
namespace psyq
{
    namespace string
    {
        namespace _private
        {
            template<typename> class interface_mutable;
        } // namespace _private
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string を模した、mutableな文字列のinterface。
    @tparam template_string_type 
        操作する文字列型。

        - psyq::string::_private::interface_immutable::string_type
          の要件を満たしていること。
        - 文字を挿入するため、以下に相当するメンバ関数が使えること。
          @code
          // @param[in] in_offset 文字を挿入するオフセット位置。
          // @param[in] in_count  挿入する文字の数。
          // @param[in] in_char   挿入する文字。
          void template_string_type::insert(
              std::size_t const in_offset, 
              std::size_t const in_count,
              typename template_string_type::traits_type::char_type const in_char)
          @endcode
        - 文字列を挿入するため、以下に相当するメンバ関数が使えること。
          @code
          // @param[in] in_position 文字列を挿入する位置。
          // @param[in] in_begin    挿入する文字列の先頭を指す反復子。
          // @param[in] in_end      挿入する文字列の末尾を指す反復子。
          template<typename template_iterator>
          typename template_string_type::traits_type::char_type* template_string_type::insert(
              typename template_string_type::traits_type::char_type const* const in_position,
              template_iterator const in_begin,
              template_iterator const in_end)
          @endcode
        - 文字列の要素を削除するため、以下に相当するメンバ関数が使えること。
          @code
          // @param[in] in_begin 削除範囲の先頭を指す反復子。
          // @param[in] in_end   削除範囲の末尾を指す反復子。
          // @return 最後に削除した要素の次を指す反復子。
          typename template_string_type::traits_type::char_type* template_string_type::erase(
              typename template_string_type::traits_type::char_type const* const in_begin,
              typename template_string_type::traits_type::char_type const* const in_end)
          @endcode
 */
template<typename template_string_type>
class psyq::string::_private::interface_mutable:
public psyq::string::_private::interface_immutable<template_string_type>
{
    /// thisが指す値の型。
    private: typedef interface_mutable this_type;

    /// this_type の基底型。
    public: typedef psyq::string::_private::interface_immutable<
        template_string_type>
            base_type;

    //-------------------------------------------------------------------------
    /// 文字へのpointer。
    public: typedef typename base_type::value_type* pointer;
    /// 文字への参照。
    public: typedef typename base_type::value_type& reference;
    /// 文字を指す反復子。
    public: typedef typename this_type::pointer iterator;
    /// 文字を指す逆反復子。
    public: typedef std::reverse_iterator<iterator> reverse_iterator;

    //-------------------------------------------------------------------------
    /// @name コンストラクタ
    //@{
    /** @brief 文字列をコピー構築する。
        @param[in] in_string コピー元となる文字列。
     */
    protected: interface_mutable(this_type const& in_string):
    base_type(in_string)
    {}

    /** @brief 文字列をムーブ構築する。
        @param[in,out] io_string ムーブ元となる文字列。
     */
    protected: interface_mutable(this_type&& io_string) PSYQ_NOEXCEPT:
    base_type(std::move(io_string))
    {}

    /** @brief 文字列をコピー構築する。
        @param[in] in_string コピー元となる文字列。
     */
    protected: explicit interface_mutable(
        typename base_type::base_type const& in_string):
    base_type(in_string)
    {}

    /** @brief 文字列をムーブ構築する。
        @param[in,out] io_string ムーブ元となる文字列。
     */
    protected: explicit interface_mutable(
        typename base_type::base_type&& io_string)
    PSYQ_NOEXCEPT:
    base_type(std::move(io_string))
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の要素を参照
    //@{
    /// @copydoc psyq::string::_private::interface_immutable::at
    public: typename base_type::const_reference at(
        typename base_type::size_type const in_index)
    const
    {
        return this->base_type::at(in_index);
    }

    /// @copydoc psyq::string::_private::interface_immutable::at
    public: typename this_type::reference at(
        typename base_type::size_type const in_index)
    {
        return *base_type::get_char_pointer(
            this->begin(), this->size(), in_index);
    }

    /// @copydoc psyq::string::_private::interface_immutable::operator[]
    public: typename base_type::const_reference operator[](
        typename base_type::size_type const in_index)
    const PSYQ_NOEXCEPT
    {
        return this->base_type::operator[](in_index);
    }

    /// @copydoc psyq::string::_private::interface_immutable::operator[]
    public: typename this_type::reference operator[](
        typename base_type::size_type const in_index)
    PSYQ_NOEXCEPT
    {
        return *base_type::get_char_pointer_noexcept(
            this->begin(), this->size(), in_index);
    }

    /// @copydoc psyq::string::_private::interface_immutable::front
    public: typename base_type::const_reference front() const
    {
        return this->base_type::front();
    }

    /// @copydoc psyq::string::_private::interface_immutable::front
    public: typename this_type::reference front()
    {
        return this->at(0);
    }

    /// @copydoc psyq::string::_private::interface_immutable::back
    public: typename base_type::const_reference back() const
    {
        return this->base_type::back();
    }

    /// @copydoc psyq::string::_private::interface_immutable::back
    public: typename this_type::reference back()
    {
        return this->at(this->size() - 1);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 反復子の取得
    //@{
    /// @copydoc psyq::string::_private::interface_immutable::begin
    public: typename base_type::const_iterator begin() const PSYQ_NOEXCEPT
    {
        return this->base_type::begin();
    }

    /// @copydoc psyq::string::_private::interface_immutable::begin
    public: typename this_type::iterator begin() PSYQ_NOEXCEPT
    {
        return &(*this->string_type::begin());
    }

    /// @copydoc psyq::string::_private::interface_immutable::end
    public: typename base_type::const_iterator end() const PSYQ_NOEXCEPT
    {
        return this->base_type::end();
    }

    /// @copydoc psyq::string::_private::interface_immutable::end
    public: typename this_type::iterator end() PSYQ_NOEXCEPT
    {
        return this->begin() + this->size();
    }

    /// @copydoc psyq::string::_private::interface_immutable::rbegin
    public: typename base_type::const_reverse_iterator rbegin()
    const PSYQ_NOEXCEPT
    {
        return this->base_type::rbegin();
    }

    /// @copydoc psyq::string::_private::interface_immutable::rbegin
    public: typename this_type::reverse_iterator rbegin() PSYQ_NOEXCEPT
    {
        return typename this_type::reverse_iterator(this->end());
    }

    /// @copydoc psyq::string::_private::interface_immutable::rend
    public: typename base_type::const_reverse_iterator rend()
    const PSYQ_NOEXCEPT
    {
        return this->base_type::rend();
    }

    /// @copydoc psyq::string::_private::interface_immutable::rend
    public: typename this_type::reverse_iterator rend() PSYQ_NOEXCEPT
    {
        return typename this_type::reverse_iterator(this->begin());
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の追加
    //@{
    /** @brief 末尾に文字列を追加する。
        @param[in] in_string 追加する文字列。
        @return *this
     */
    public: this_type& append(typename base_type::view const& in_string)
    {
        this->string_type::insert(
            this->size(), in_string.begin(), in_string.end());
        return *this;
    }

    /** @brief 末尾に文字列を追加する。
        @param[in] in_string 追加する文字列。
        @param[in] in_offset 追加する文字列の開始オフセット位置。
        @param[in] in_count  追加する文字列の開始オフセット位置からの要素数。
        @return *this
     */
    public: this_type& append(
        typename base_type::view const& in_string,
        typename base_type::size_type const in_offset,
        typename base_type::size_type const in_count)
    {
        return this->insert(in_string.substr(in_offset, in_count));
    }

    /** @brief 末尾に文字列を追加する。
        @param[in] in_data 追加する文字列の先頭位置。
        @param[in] in_size 追加する文字列の要素数。
        @return *this
     */
    public: this_type& append(
        typename base_type::const_pointer const in_data,
        typename base_type::size_type const in_size)
    {
        return this->insert(typename base_type::view(in_data, in_size));
    }

    /** @brief 末尾に文字を追加する。
        @param[in] in_count 追加する文字の数。
        @param[in] in_char  追加する文字。
        @return *this
     */
    public: this_type& append(
        typename base_type::size_type const in_count,
        typename base_type::value_type const in_char)
    {
        this->string_type::insert(this->size(), in_count, in_char);
        return *this;
    }

    /** @brief 末尾に文字列を追加する。
        @param[in] in_begin 追加する文字列の先頭を指す反復子。
        @param[in] in_end   追加する文字列の末尾を指す反復子。
        @return *this
     */
    public: template<typename template_iterator>
    this_type& append(
        template_iterator const in_begin,
        template_iterator const in_end)
    {
        this->string_type::insert(this->size(), in_begin, in_end);
        return *this;
    }

    /** @brief 末尾に要素をひとつ挿入する。
        @param[in] in_char 挿入する要素。
     */
    public: void push_back(typename base_type::value_type const in_char)
    {
        this->append(1, in_char);
    }

    /** @brief 末尾に文字列を追加する。
        @param[in] in_string 追加する文字列。
        @return *this
     */
    public: this_type& operator+=(typename base_type::view const& in_string)
    {
        return this->insert(in_string);
    }

    /** @brief 末尾に文字を追加する。
        @param[in] in_char 追加する文字。
        @return *this
     */
    public: this_type& operator+=(typename base_type::value_type const in_char)
    {
        return this->insert(1, in_char);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の挿入
    //@{
    /** @brief 文字列を挿入する。
        @param[in] in_position 文字列を挿入するオフセット位置。
        @param[in] in_string   挿入する文字列。
        @return *this
     */
    public: this_type& insert(
        typename base_type::size_type const in_position,
        typename base_type::view const& in_string)
    {
        this->string_type::insert(
            this->begin() + in_position, in_string.begin(), in_string.end());
        return *this;
    }

    /** @brief 文字列を挿入する。
        @param[in] in_position 文字列を挿入するオフセット位置。
        @param[in] in_string   挿入する文字列。
        @param[in] in_offset   挿入する文字列の開始オフセット位置。
        @param[in] in_count    挿入する文字列の、開始オフセット位置からの要素数。
        @return *this
     */
    public: this_type& insert(
        typename base_type::size_type const in_position,
        typename base_type::view const& in_string,
        typename base_type::size_type const in_offset,
        typename base_type::size_type const in_count)
    {
        return this->insert(
            in_position, in_string.substr(in_offset, in_count));
    }

    /** @brief 文字列を挿入する。
        @param[in] in_position 文字列を挿入するオフセット位置。
        @param[in] in_data     挿入する文字列の先頭位置。
        @param[in] in_size     挿入する文字列の要素数。
        @return *this
     */
    public: this_type& insert(
        typename base_type::size_type const in_position,
        typename base_type::const_pointer const in_data,
        typename base_type::size_type const in_size)
    {
        return this->insert(
            in_position, typename base_type::view(in_data, in_size));
    }

    /** @brief 文字を挿入する。
        @param[in] in_position 文字を挿入するオフセット位置。
        @param[in] in_count    挿入する文字の数。
        @param[in] in_char     挿入する文字。
        @return *this
     */
    public: this_type& insert(
        typename base_type::size_type const in_position,
        typename base_type::size_type const in_count,
        typename base_type::value_type const in_char)
    {
        this->string_type::insert(in_position, in_count, in_char);
        return *this;
    }

    /** @brief 文字を挿入する。
        @param[in] in_position 文字を挿入する位置を指す反復子。
        @param[in] in_char     挿入する文字。
        @return 挿入された文字を指す反復子。
     */
    public: typename this_type::iterator insert(
        typename base_type::const_iterator const in_position,
        typename base_type::value_type const in_char)
    {
        return this->insert(in_position, 1, in_char);
    }

    /** @brief 文字を挿入する。
        @param[in] in_position 文字を挿入する位置を指す反復子。
        @param[in] in_count    挿入する文字の数。
        @param[in] in_char     挿入する文字。
        @return 挿入された最初の文字を指す反復子。
     */
    public: typename this_type::iterator insert(
        typename base_type::const_iterator const in_position,
        typename base_type::size_type const in_count,
        typename base_type::value_type const in_char)
    {
        return this->string_type::insert(
            in_position - this->begin(), in_count, in_char);
    }

    /** @brief 文字列を挿入する。
        @param[in] in_position 文字列を挿入する位置を指す反復子。
        @param[in] in_begin    挿入する文字列の先頭を指す反復子。
        @param[in] in_end      挿入する文字列の末尾を指す反復子。
        @return 挿入された最初の文字を指す反復子。
     */
    public: template<typename template_iterator>
    typename this_type::iterator insert(
        typename base_type::const_iterator const in_position,
        template_iterator const in_begin,
        template_iterator const in_end)
    {
        return this->string_type::insert(in_position, in_begin, in_end);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の削除
    //@{
    /** @brief 文字列の要素を削除する。
        @param[in] in_offset 削除を開始するオフセット位置。
        @param[in] in_count  削除する要素数。
     */
    public: this_type& erase(
        std::size_t const in_offset,
        std::size_t const in_count)
    {
        auto const local_erase_begin(this->data() + in_offset)
        this->erase(local_erase_begin, local_erase_begin + in_count);
        return *this;
    }

    /** @brief 文字列の要素を削除する。
        @param[in] in_position 削除する要素を指す反復子。
        @return 削除した要素の次を指す反復子。
     */
    public: typename this_type::iterator erase(
        typename base_type::const_iterator const in_position)
    {
        return this->erase(in_position, std::next(in_position, 1));
    }

    /** @brief 文字列の要素を削除する。
        @param[in] in_begin 削除範囲の先頭を指す反復子。
        @param[in] in_end   削除範囲の末尾を指す反復子。
        @return 最後に削除した要素の次を指す反復子。
     */
    public: typename this_type::iterator erase(
        typename base_type::const_iterator const in_begin,
        typename base_type::const_iterator const in_end)
    {
        return this->string_type::erase(in_begin, in_end);
    }

    /** @brief 末尾の要素をひとつ削除する。
     */
    public: void pop_back()
    {
        this->erase(this->size() - 1, 1);
    }
    //@}
}; // class psyq::string::_private::interface_mutable

#endif // !defined(PSYQ_STRING_MUTABLE_INTERFACE_HPP_)
