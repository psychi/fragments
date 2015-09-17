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
/// @brief @copybrief psyq::string::_private::interface_mutable
#ifndef PSYQ_STRING_INTERFACE_MUTABLE_HPP_
#define PSYQ_STRING_INTERFACE_MUTABLE_HPP_

#include "./interface_immutable.hpp"

namespace psyq
{
    namespace string
    {
        namespace _private
        {
            /// @cond
            template<typename> class interface_mutable;
            /// @endcond
        } // namespace _private
    } // namespace string
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief std::basic_string を模した、mutableな文字列のインタフェイス。
/// @tparam template_string
/// 操作する文字列の型。
///  - psyq::string::_private::interface_immutable::base_type
///    の要件を満たしていること。
///  - 要素を挿入するため、以下に相当するメンバ関数が使えること。
///    @code
///    void template_string::insert(
///        // [in] 要素を挿入するオフセット位置。
///        std::size_t const in_offset, 
///        // [in] 挿入する要素の数。
///        std::size_t const in_count,
///        // [in] 挿入する要素。
///        typename template_string::traits_type::char_type const in_char)
///    @endcode
///  - 文字列を挿入するため、以下に相当するメンバ関数が使えること。
///    @code
///    template<typename template_iterator>
///    typename template_string::traits_type::char_type* template_string::insert(
///        // [in] 文字列を挿入する位置。
///        typename template_string::traits_type::char_type const* const in_position,
///        // [in] 挿入する文字列の先頭を指す反復子。
///        template_iterator const in_begin,
///        // [in] 挿入する文字列の末尾を指す反復子。
///        template_iterator const in_end)
///    @endcode
///  - 文字列の要素を削除するため、以下に相当するメンバ関数が使えること。
///    @code
///    // return 最後に削除した要素の次を指す反復子。
///    typename template_string::traits_type::char_type* template_string::erase(
///        // [in] 削除範囲の先頭を指す反復子。
///        typename template_string::traits_type::char_type const* const in_begin,
///        // [in] 削除範囲の末尾を指す反復子。
///        typename template_string::traits_type::char_type const* const in_end)
///    @endcode
template<typename template_string>
class psyq::string::_private::interface_mutable:
public psyq::string::_private::interface_immutable<template_string>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef interface_mutable this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::string::_private::interface_immutable<template_string>
        base_type;

    //-------------------------------------------------------------------------
    /// @copydoc psyq::string::_private::interface_immutable::pointer
    public: typedef typename base_type::value_type* pointer;
    /// @copydoc psyq::string::_private::interface_immutable::reference
    public: typedef typename base_type::value_type& reference;
    /// @copydoc psyq::string::_private::interface_immutable::iterator
    public: typedef typename this_type::pointer iterator;
    /// @copydoc psyq::string::_private::interface_immutable::reverse_iterator
    public: typedef
        std::reverse_iterator<typename this_type::iterator>
        reverse_iterator;

    //-------------------------------------------------------------------------
    /// @name 反復子の取得
    /// @{

    /// @copydoc psyq::string::_private::interface_immutable::begin
    public: typename base_type::const_iterator begin() const PSYQ_NOEXCEPT
    {
        return this->base_type::begin();
    }

    /// @copydoc psyq::string::_private::interface_immutable::begin
    public: typename this_type::iterator begin() PSYQ_NOEXCEPT
    {
        return const_cast<typename this_type::iterator>(this->data());
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
    /// @}
    //-------------------------------------------------------------------------
    /// @name 要素の参照
    /// @{

    /// @copydoc psyq::string::_private::interface_immutable::at
    public: typename base_type::const_reference at(
        /// [in] 参照する要素のインデクス番号。
        typename base_type::size_type const in_index)
    const
    {
        return this->base_type::at(in_index);
    }

    /// @copydoc psyq::string::_private::interface_immutable::at
    public: typename this_type::reference at(
        /// [in] 参照する要素のインデクス番号。
        typename base_type::size_type const in_index)
    {
        return *base_type::get_char_pointer(
            this->begin(), this->size(), in_index);
    }

    /// @copydoc psyq::string::_private::interface_immutable::operator[]
    public: typename base_type::const_reference operator[](
        /// [in] 参照する要素のインデクス番号。
        typename base_type::size_type const in_index)
    const PSYQ_NOEXCEPT
    {
        return this->base_type::operator[](in_index);
    }

    /// @copydoc psyq::string::_private::interface_immutable::operator[]
    public: typename this_type::reference operator[](
        /// [in] 参照する要素のインデクス番号。
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
    /// @}
    //-------------------------------------------------------------------------
    /// @name 要素の追加
    /// @{

    /// @brief 末尾に文字列を追加する。
    /// @return *this
    public: this_type& append(
        /// [in] 追加する文字列。
        typename base_type::view const& in_string)
    {
        this->base_type::base_type::insert(
            this->end(), in_string.begin(), in_string.end());
        return *this;
    }

    /// @brief 末尾に文字列を追加する。
    /// @return *this
    public: this_type& append(
        /// [in] 追加する文字列。
        typename base_type::view const& in_string,
        /// [in] 追加する文字列の開始オフセット位置。
        typename base_type::size_type const in_offset,
        /// [in] 追加する文字列の開始オフセット位置からの要素数。
        typename base_type::size_type const in_count)
    {
        return this->append(in_string.substr(in_offset, in_count));
    }

    /// @brief 末尾に文字列を追加する。
    /// @return *this
    public: this_type& append(
        /// [in] 追加する文字列の先頭位置。
        typename base_type::const_pointer const in_data,
        /// [in] 追加する文字列の要素数。
        typename base_type::size_type const in_size)
    {
        return this->append(typename base_type::view(in_data, in_size));
    }

    /// @brief 末尾に文字を追加する。
    /// @return *this
    public: this_type& append(
        /// [in] 追加する文字の数。
        typename base_type::size_type const in_count,
        /// [in] 追加する文字。
        typename base_type::value_type const in_char)
    {
        this->base_type::base_type::insert(this->size(), in_count, in_char);
        return *this;
    }

    /// @brief 末尾に文字列を追加する。
    /// @return *this
    public: template<typename template_iterator>
    this_type& append(
        /// [in] 追加する文字列の先頭を指す反復子。
        template_iterator const in_begin,
        /// [in] 追加する文字列の末尾を指す反復子。
        template_iterator const in_end)
    {
        this->base_type::base_type::insert(this->end(), in_begin, in_end);
        return *this;
    }

    /// @brief 末尾に要素を追加する。
    public: void push_back(
        /// [in] 追加する要素。
        typename base_type::value_type const in_char)
    {
        this->append(1, in_char);
    }

    /// @brief 末尾に文字列を追加する。
    /// @return *this
    public: this_type& operator+=(
        /// [in] 追加する文字列。
        typename base_type::view const& in_string)
    {
        return this->append(in_string);
    }

    /// @brief 末尾に要素を追加する。
    /// @return *this
    public: this_type& operator+=(
        /// [in] 追加する要素。
        typename base_type::value_type const in_char)
    {
        return this->append(1, in_char);
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 要素の挿入
    /// @{

    /// @brief 文字列を挿入する。
    /// @return *this
    public: this_type& insert(
        /// [in] 文字列を挿入するオフセット位置。
        typename base_type::size_type const in_position,
        /// [in] 挿入する文字列。
        typename base_type::view const& in_string)
    {
        this->base_type::base_type::insert(
            this->begin() + in_position, in_string.begin(), in_string.end());
        return *this;
    }

    /// @brief 文字列を挿入する。
    /// @return *this
    public: this_type& insert(
        /// [in] 文字列を挿入するオフセット位置。
        typename base_type::size_type const in_position,
        /// [in] 挿入する文字列。
        typename base_type::view const& in_string,
        /// [in] 挿入する文字列の開始オフセット位置。
        typename base_type::size_type const in_offset,
        /// [in] 挿入する文字列の、開始オフセット位置からの要素数。
        typename base_type::size_type const in_count)
    {
        return this->insert(
            in_position, in_string.substr(in_offset, in_count));
    }

    /// @brief 文字列を挿入する。
    /// @return *this
    public: this_type& insert(
        /// [in] 文字列を挿入するオフセット位置。
        typename base_type::size_type const in_position,
        /// [in] 挿入する文字列の先頭位置。
        typename base_type::const_pointer const in_data,
        /// [in] 挿入する文字列の要素数。
        typename base_type::size_type const in_size)
    {
        return this->insert(
            in_position, typename base_type::view(in_data, in_size));
    }

    /// @brief 要素を挿入する。
    /// @return *this
    public: this_type& insert(
        /// [in] 要素を挿入するオフセット位置。
        typename base_type::size_type const in_position,
        /// [in] 挿入する要素の数。
        typename base_type::size_type const in_count,
        /// [in] 挿入する要素。
        typename base_type::value_type const in_char)
    {
        this->base_type::base_type::insert(in_position, in_count, in_char);
        return *this;
    }

    /// @brief 要素を挿入する。
    /// @return 挿入した要素を指す反復子。
    public: typename this_type::iterator insert(
        /// [in] 要素を挿入する位置を指す反復子。
        typename base_type::const_iterator const in_position,
        /// [in] 挿入する要素。
        typename base_type::value_type const in_char)
    {
        return this->insert(in_position, 1, in_char);
    }

    /// @brief 要素を挿入する。
    /// @return 挿入した最初の要素を指す反復子。
    public: typename this_type::iterator insert(
        /// [in] 要素を挿入する位置を指す反復子。
        typename base_type::const_iterator const in_position,
        /// [in] 挿入する要素の数。
        typename base_type::size_type const in_count,
        /// [in] 挿入する要素。
        typename base_type::value_type const in_char)
    {
        return this->base_type::base_type::insert(
            in_position - this->begin(), in_count, in_char);
    }

    /// @brief 文字列を挿入する。
    /// @return 挿入した文字列の最初の要素を指す反復子。
    public: template<typename template_iterator>
    typename this_type::iterator insert(
        /// [in] 文字列を挿入する位置を指す反復子。
        typename base_type::const_iterator const in_position,
        /// [in] 挿入する文字列の先頭を指す反復子。
        template_iterator const in_begin,
        /// [in] 挿入する文字列の末尾を指す反復子。
        template_iterator const in_end)
    {
        return this->base_type::base_type::insert(in_position, in_begin, in_end);
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 要素の削除
    /// @{

    /// @brief 文字列の要素を削除する。
    public: this_type& erase(
        /// [in] 削除を開始するオフセット位置。
        std::size_t const in_offset,
        /// [in] 削除する要素数。
        std::size_t const in_count)
    {
        auto const local_erase_begin(this->data() + in_offset);
        this->erase(local_erase_begin, local_erase_begin + in_count);
        return *this;
    }

    /// @brief 文字列の要素を削除する。
    /// @return 削除した要素の次を指す反復子。
    public: typename this_type::iterator erase(
        /// [in] 削除する要素を指す反復子。
        typename base_type::const_iterator const in_position)
    {
        return this->erase(in_position, std::next(in_position, 1));
    }

    /// @brief 文字列の要素を削除する。
    /// @return 最後に削除した要素の次を指す反復子。
    public: typename this_type::iterator erase(
        /// [in] 削除範囲の先頭を指す反復子。
        typename base_type::const_iterator const in_begin,
        /// [in] 削除範囲の末尾を指す反復子。
        typename base_type::const_iterator const in_end)
    {
        return this->base_type::base_type::erase(in_begin, in_end);
    }

    /// @brief 末尾の要素を削除する。
    public: void pop_back()
    {
        this->erase(this->size() - 1, 1);
    }
    // @}
    //-------------------------------------------------------------------------
    /// @name 文字列の代入
    /// @{

    /// @brief 文字列をコピー代入する。
    /// @return *this
    public: this_type& assign(
        /// [in] コピー元となる文字列。
        this_type const& in_string)
    {
        this->base_type::base_type::operator=(in_string);
        return *this;
    }

    /// @brief 文字列をムーブ代入する。
    /// @return *this
    public: this_type& assign(
        /// [in] ムーブ元となる文字列。
        this_type&& in_string)
    {
        this->base_type::base_type::operator=(std::move(in_string));
        return *this;
    }

    /// @brief 文字列をコピー代入する。
    /// @return *this
    public: this_type& assign(
        /// [in] コピー元となる文字列。
        typename base_type::view const& in_string)
    {
        this->assign(in_string.begin(), in_string.end());
        return *this;
    }

    /// @brief 文字列をコピー代入する。
    /// @return *this
    public: this_type& assign(
        /// [in] コピー元となる文字列。
        typename base_type::view const& in_string,
        /// [in] コピー元となる文字列の開始オフセット位置。
        typename base_type::size_type const in_offset,
        /// [in] コピー元となる文字列の、開始オフセット位置からの要素数。
        typename base_type::size_type const in_count)
    {
        return this->assing(in_string.substr(in_offset, in_count));
    }

    /// @brief 文字列をコピー代入する。
    /// @return *this
    public: this_type& assign(
        /// [in] コピー元となる文字列の先頭位置。
        typename base_type::const_pointer const in_data,
        /// [in] コピー元となる文字列の要素数。
        typename base_type::size_type const in_size)
    {
        return this->assign(typename base_type::view(in_data, in_size));
    }

    /// @brief 文字をコピー代入する。
    /// @return *this
    public: this_type& assign(
        /// [in] コピーする文字の数。
        typename base_type::size_type const in_count,
        /// [in] コピーする文字。
        typename base_type::value_type const in_char)
    {
        this->clear();
        return this->append(in_count, in_char);
    }

    /// @brief 文字をコピー代入する。
    /// @return *this
    public: template<typename template_iterator>
    this_type& assign(
        /// [in] コピーする文字列の先頭を指す反復子。
        template_iterator const in_begin,
        /// [in] コピーする文字列の末尾を指す反復子。
        template_iterator const in_end)
    {
        this->clear();
        return this->append(in_begin, in_end);
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 文字列の置換
    /// @{

    /// @brief 文字列を一部を置換する。
    /// @return *this
    public: this_type& replace(
        /// [in] 置き換えられる文字列の開始オフセット位置。
        typename base_type::size_type const in_target_offset,
        /// [in] 置き換えられる文字列の、開始オフセット位置からの要素数。
        typename base_type::size_type const in_target_count,
        /// [in] 置き換える文字列。
        typename base_type::view const& in_source_string)
    {
        auto const local_target_begin(this->begin() + in_target_offset);
        return this->replace(
            local_target_begin,
            local_target_begin + in_target_count,
            in_source_string.begin(),
            in_source_string.end());
    }

    /// @brief 文字列を一部を置換する。
    /// @return *this
    public: this_type& replace(
        ///[in] 置き換えられる文字列の開始オフセット位置。
        typename base_type::size_type const in_target_offset,
        /// [in] 置き換えられる文字列の、開始オフセット位置からの要素数。
        typename base_type::size_type const in_target_count,
        /// [in] 置き換える文字列。
        typename base_type::view const& in_source_string,
        /// [in] 置き換える文字列の開始オフセット位置。
        typename base_type::size_type const in_source_offset,
        /// [in] 置き換えられる文字列の、開始オフセット位置からの要素数。
        typename base_type::size_type const in_source_count)
    {
        return this->replace(
            in_target_offset,
            in_target_count,
            in_source_string.substr(in_source_offset, in_target_count));
    }

    /// @brief 文字列を一部を置換する。
    /// @return *this
    public: this_type& replace(
        /// [in] 置き換えられる文字列の開始オフセット位置。
        typename base_type::size_type const in_target_offset,
        /// [in] 置き換えられる文字列の、開始オフセット位置からの要素数。
        typename base_type::size_type const in_target_count,
        /// [in] 置き換える文字列の先頭位置。
        typename base_type::const_pointer const in_source_data,
        /// [in] 置き換える文字列の要素数。
        typename base_type::size_type const in_source_size)
    {
        return this->replace(
            in_target_offset,
            in_target_count,
            typename base_type::view(in_source_data, in_source_size));
    }

    /// @brief 文字列を一部を置換する。
    /// @return *this
    public: this_type& replace(
        ///[in] 置き換えられる文字列の開始オフセット位置。
        typename base_type::size_type const in_target_offset,
        /// [in] 置き換えられる文字列の、開始オフセット位置からの要素数。
        typename base_type::size_type const in_target_count,
        /// [in] 置き換える要素の数。
        typename base_type::size_type const in_source_count,
        /// [in] 置き換える要素。
        typename base_type::value_type const in_source_char)
    {
        auto const local_target_begin(this->begin() + in_target_offset);
        return this->replace(
            local_target_begin,
            local_target_begin + in_target_count,
            in_source_count,
            in_source_char);
    }

    /// @brief 文字列を一部を置換する。
    /// @return *this
    public: this_type& replace(
        /// [in] 置き換えられる文字列の開始位置を指す反復子。
        typename base_type::const_iterator const in_target_begin,
        /// [in] 置き換えられる文字列の末尾位置を指す反復子。
        typename base_type::const_iterator const in_target_end,
        /// [in] 置き換える文字列。
        typename base_type::view const& in_source_string)
    {
        return this->replace(
            in_target_begin,
            in_target_end,
            in_source_string.begin(),
            in_source_string.end());
    }

    /// @brief 文字列を一部を置換する。
    /// @return *this
    public: this_type& replace(
        /// [in] 置き換えられる文字列の開始位置を指す反復子。
        typename base_type::const_iterator const in_target_begin,
        /// [in] 置き換えられる文字列の末尾位置を指す反復子。
        typename base_type::const_iterator const in_target_end,
        /// [in] 置き換える文字列の先頭位置。
        typename base_type::const_pointer const in_source_data,
        /// [in] 置き換える文字列の要素数。
        typename base_type::size_type const in_source_size)
    {
        return this->replace(
            in_target_begin,
            in_target_end,
            typename base_type::view(in_source_data, in_source_size));
    }

    /// @brief 文字列を一部を置換する。
    /// @return *this
    public: this_type& replace(
        /// [in] 置き換えられる文字列の開始位置を指す反復子。
        typename base_type::const_iterator const in_target_begin,
        /// [in] 置き換えられる文字列の末尾位置を指す反復子。
        typename base_type::const_iterator const in_target_end,
        /// [in] 置き換える要素の数。
        typename base_type::size_type const in_source_count,
        /// [in] 置き換える要素。
        typename base_type::value_type const in_source_char)
    {
        auto local_target_begin(this->adjust_iterator(in_target_begin));
        auto local_target_end(this->adjust_iterator(in_target_end));
        if (local_target_end < local_target_begin)
        {
            PSYQ_ASSERT_THROW(false, std::invalid_argument);
            std::swap(local_target_begin, local_target_end);
        }
        auto const local_target_size(local_target_end - local_target_begin);
        typename this_type::iterator local_copy_end;
        if (local_target_size < in_source_count)
        {
            // 置換する文字列のほうが大きいので、あふれる文字列を挿入する。
            local_copy_end = this->insert(
                local_target_end, 
                in_source_count - local_target_size, 
                in_source_char);
        }
        else if (in_source_count < local_target_size)
        {
            // 置換される文字列のほうが大きいので、不要な文字列を削除する。
            local_copy_end = this->erase(
                local_target_end, 
                local_target_end + local_target_size - in_source_count);
        }
        else
        {
            local_copy_end = local_target_end;
        }
        base_type::traits_type::assign(
            local_target_begin,
            local_copy_end - local_target_begin,
            in_source_char);
        return *this;
    }

    /// @brief 文字列を一部を置換する。
    /// @return *this
    public: template<typename template_iterator>
    this_type& replace(
        /// [in] 置き換えられる文字列の開始位置を指す反復子。
        typename base_type::const_iterator const in_target_begin,
        /// [in] 置き換えられる文字列の末尾位置を指す反復子。
        typename base_type::const_iterator const in_target_end,
        /// [in] 置き換える文字列の開始位置を指す反復子。
        template_iterator const in_source_begin,
        /// [in] 置き換える文字列の末尾位置を指す反復子。
        template_iterator const in_source_end)
    {
        auto local_target_begin(this->adjust_iterator(in_target_begin));
        auto local_target_end(this->adjust_iterator(in_target_end));
        if (local_target_end < local_target_begin)
        {
            PSYQ_ASSERT_THROW(false, std::invalid_argument);
            std::swap(local_target_begin, local_target_end);
        }
        auto const local_target_size(local_target_end - local_target_begin);
        auto const local_source_size(
            std::distance(in_source_begin, in_source_end));
        typename this_type::iterator local_copy_end;
        if (local_target_size < local_source_size)
        {
            // 置換する文字列のほうが大きいので、あふれる文字列を挿入する。
            auto const local_post_begin(
                std::next(in_source_begin, local_target_size));
            local_copy_end = this->insert(
                local_target_end,
                local_post_begin,
                std::next(
                    local_post_begin, local_source_size - local_target_size));
        }
        else if (local_source_size < local_target_size)
        {
            // 置換される文字列のほうが大きいので、不要な文字列を削除する。
            local_copy_end = this->erase(
                local_target_end,
                local_target_end + local_target_size - local_source_size);
        }
        else
        {
            local_copy_end = local_target_end;
        }
        auto local_source(in_source_begin);
        for (
            auto i(local_target_begin);
            i != local_copy_end;
            ++i, ++local_source)
        {
            *i = *local_source;
        }
        return *this;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 文字列の構築
    /// @{

    /// @brief 文字列をコピー構築する。
    protected: interface_mutable(
        /// [in] コピー元となる文字列。
        this_type const& in_string):
    base_type(in_string)
    {}

    /// @brief 文字列をムーブ構築する。
    protected: interface_mutable(
        /// [in,out] ムーブ元となる文字列。
        this_type&& io_string)
    PSYQ_NOEXCEPT: base_type(std::move(io_string))
    {}

    /// @brief 文字列をコピー構築する。
    protected: explicit interface_mutable(
        /// [in] コピー元となる文字列。
        typename base_type::base_type const& in_string):
    base_type(in_string)
    {}

    /// @brief 文字列をムーブ構築する。
    protected: explicit interface_mutable(
        /// [in,out] ムーブ元となる文字列。
        typename base_type::base_type&& io_string)
    PSYQ_NOEXCEPT: base_type(std::move(io_string))
    {}
    /// @}
    //-------------------------------------------------------------------------
    /// @brief 反復子が範囲内になるよう調整する。
    /// @return 調整された反復子。
    private: typename this_type::iterator adjust_iterator(
        /// [in] 調整する反復子。
        typename base_type::const_iterator const in_iterator)
    {
        if (in_iterator < this->begin())
        {
            PSYQ_ASSERT_THROW(false, std::out_of_range);
            return this->begin();
        }
        else if (this->end() < in_iterator)
        {
            PSYQ_ASSERT_THROW(false, std::out_of_range);
            return this->end();
        }
        else
        {
            return const_cast<this_type::iterator>(in_iterator);
        }
    }

}; // class psyq::string::_private::interface_mutable

#endif // !defined(PSYQ_STRING_INTERFACE_MUTABLE_HPP_)
// vim: set expandtab:
