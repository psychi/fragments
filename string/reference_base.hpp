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
    @brief @copybrief psyq::string::_private::reference_base
    @defgroup psyq_string 動的メモリ確保を抑制した文字列オブジェクト
 */
#ifndef PSYQ_STRING_REFERENCE_BASE_HPP_
#define PSYQ_STRING_REFERENCE_BASE_HPP_

#include <iosfwd>
#include <iterator>
#include <algorithm>
#include <cctype>
#include "../assert.hpp"

namespace psyq
{
    /// 動的メモリ確保を抑制した文字列
    namespace string
    {
        /// psyq::string 管理者以外がこの名前空間に直接アクセスするのは禁止。
        namespace _private
        {
            /// @cond
            template<typename> class reference_base;
            /// @endcond
        } // namespace _private
    } // namespace string
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief immutableな文字列への参照の基底型。
    @tparam template_char_traits @copydoc psyq::string::_private::reference_base::traits_type
    @warning
        文字の配列を単純にconst参照しているので、
        参照してる文字の配列が変更／破壊されると、動作を保証できなくなる。
 */
template<typename template_char_traits>
class psyq::string::_private::reference_base
{
    /// @brief thisが指す値の型。
    private: typedef reference_base this_type;

    /// @brief 文字特性の型。
    public: typedef template_char_traits traits_type;

    //-------------------------------------------------------------------------
    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列。
     */
    protected: PSYQ_CONSTEXPR reference_base(this_type const& in_string)
    PSYQ_NOEXCEPT:
    size_(in_string.size()),
    data_(in_string.data())
    {}

    /** @brief 文字列を参照する。
        @param[in] in_data 参照する文字列の先頭位置。
        @param[in] in_size 参照する文字列の要素数。
     */
    private: PSYQ_CONSTEXPR reference_base(
        typename this_type::traits_type::char_type const* const in_data,
        std::size_t const in_size)
    PSYQ_NOEXCEPT:
    size_(in_size),
    data_(in_data)
    {}

    //-------------------------------------------------------------------------
    /// @name 文字列の変更
    //@{
    /// @brief 文字列を空にする。
    public: void clear() PSYQ_NOEXCEPT
    {
        this->size_ = 0;
    }

    /** @brief 先頭の要素を削除する。
        @param[in] in_remove_size 削除する要素数。
     */
    public: void remove_prefix(std::size_t const in_remove_size)
    {
        if (in_remove_size <= this->size())
        {
            this->size_ -= in_remove_size;
            this->data_ += in_remove_size;
        }
        else
        {
            PSYQ_ASSERT(false);
            this->data_ += this->size();
            this->clear();
        }
    }

    /** @brief 末尾の要素を削除する。
        @param[in] in_remove_size 削除する要素数。
     */
    public: void remove_suffix(std::size_t const in_remove_size)
    {
        if (in_remove_size <= this->size())
        {
            this->size_ -= in_remove_size;
        }
        else
        {
            PSYQ_ASSERT(false);
            this->clear();
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列のプロパティ
    //@{
    /** @brief 文字列の先頭文字へのpointerを取得する。
        @return 文字列の先頭文字へのpointer。
        @warning
            文字列の先頭文字から末尾文字までのmemory連続性は保証されているが、
            文字列の終端がnull文字となっている保証はない。
     */
    public: PSYQ_CONSTEXPR typename this_type::traits_type::char_type const* data()
    const PSYQ_NOEXCEPT
    {
        return this->data_;
    }

    /** @brief 文字列の要素数を取得する。
        @return 文字列の要素数。
     */
    public: PSYQ_CONSTEXPR std::size_t size() const PSYQ_NOEXCEPT
    {
        return this->size_;
    }

    /** @brief 文字列の最大要素数を取得する。
        @return 文字列の最大要素数。
     */
    public: PSYQ_CONSTEXPR std::size_t max_size() const PSYQ_NOEXCEPT
    {
        return (std::numeric_limits<std::size_t>::max)();
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief 文字列リテラルを参照して構築する。
        @tparam template_size 参照する文字列リテラルの要素数。終端文字も含む。
        @param[in] in_literal 参照する文字列リテラル。
        @return 構築した文字列。
        @warning 文字列リテラル以外の文字列を引数に渡すのは禁止。
        @note
            引数が文字列リテラルであることを保証するため、
            ユーザー定義リテラルを経由して呼び出すようにしたい。
     */
    protected: template <std::size_t template_size>
    static PSYQ_CONSTEXPR this_type make(
        typename this_type::traits_type::char_type const (&in_literal)[template_size])
    PSYQ_NOEXCEPT
    {
        static_assert(
            0 < template_size, "'template_size' is not greater than 0.");
        return this_type(
            (PSYQ_ASSERT(in_literal[template_size - 1] == 0), &in_literal[0]),
            template_size - 1);
    }

    /** @brief 文字列を参照して構築する。
        @param[in] in_data 文字列の先頭位置。
        @param[in] in_size 文字列の要素数。
        @return 構築した文字列。
     */
    protected: static PSYQ_CONSTEXPR this_type make(
        typename this_type::traits_type::char_type const* const in_data,
        std::size_t const in_size)
    {
        return this_type(
            (PSYQ_ASSERT(in_data != nullptr || in_size == 0), in_data),
            in_data != nullptr && 0 < in_size? in_size: 0);
    }

    /** @brief 部分文字列を参照して構築する。
        @param[in] in_string 参照する文字列。
        @param[in] in_offset 部分文字列の開始オフセット値。
        @param[in] in_count  部分文字列の開始オフセット値からの要素数。
        @return 構築した文字列。
     */
    protected: static this_type make(
        this_type const& in_string,
        std::size_t in_offset,
        std::size_t const in_count)
    {
        if (in_string.size() < in_offset)
        {
            PSYQ_ASSERT_THROW(false, std::out_of_range);
            in_offset = in_string.size();
        }
        return this_type(
            in_string.data() + in_offset,
            (std::min)(in_string.size() - in_offset, in_count));
    }

    //-------------------------------------------------------------------------
    /// @brief 文字列の要素数。
    private: std::size_t size_;
    /// @brief 文字列の先頭位置。
    private: typename this_type::traits_type::char_type const* data_;

}; // class psyq::string::_private::reference_base

#endif // !PSYQ_STRING_REFERENCE_BASE_HPP_
