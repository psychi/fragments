﻿/*
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
 */
#ifndef PSYQ_STRING_FLYWEIGHT_STRING_HPP_
#define PSYQ_STRING_FLYWEIGHT_STRING_HPP_

//#include "../atomic_count.hpp"

/// @cond
namespace psyq
{
    namespace string
    {
        namespace _private
        {
            template<typename, typename> class flyweight_string;
        } // namespace
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief psyq::string::_private::flyweight_factory が生成するフライ級文字列。
    @tparam template_string_view @copydoc flyweight_string::view
    @tparam template_hash_value  文字列のハッシュ値の型。
 */
template<typename template_string_view, typename template_hash_value>
class psyq::string::_private::flyweight_string
{
    /// @brief thisが指す値の型。
    private: typedef flyweight_string this_type;

    /// @brief フライ級文字列で使う文字列参照の型。
    public: typedef template_string_view view;

    /// @brief フライ級文字列を比較する関数オブジェクト。
    public: struct less
    {
        bool operator()(
            flyweight_string const& in_left,
            flyweight_string const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.hash != in_right.hash?
                in_left.hash < in_right.hash: in_left.size < in_right.size;
        }

        bool operator()(
            flyweight_string const* const in_left,
            flyweight_string const& in_right)
        const PSYQ_NOEXCEPT
        {
            return this->operator()(*in_left, in_right);
        }

        bool operator()(
            flyweight_string const& in_left,
            flyweight_string const* const in_right)
        const PSYQ_NOEXCEPT
        {
            return this->operator()(in_left, *in_right);
        }

        bool operator()(
            flyweight_string const* const in_left,
            flyweight_string const* const in_right)
        const PSYQ_NOEXCEPT
        {
            return this->operator()(*in_left, *in_right);
        }

    }; // struct less

    //-------------------------------------------------------------------------
    /** @brief 文字列を構築する。
        @param[in] in_size this_type::size の初期値。
        @param[in] in_hash this_type::hash の初期値。
     */
    public: flyweight_string(
        typename this_type::view::size_type const in_size,
        template_hash_value const in_hash)
    PSYQ_NOEXCEPT:
    reference_count(0),
    size(in_size),
    hash(in_hash)
    {}

    /** @brief 文字列の先頭位置を取得する。
        @return 文字列の先頭位置。
     */
    public: typename this_type::view::const_pointer data() const PSYQ_NOEXCEPT
    {
        return reinterpret_cast<typename this_type::view::const_pointer>(this + 1);
    }

    //-------------------------------------------------------------------------
    /// @brief 文字列の被参照数。
    public: psyq::atomic_count reference_count;
    /// @brief 文字列の要素数。
    public: typename this_type::view::size_type size;
    /// @brief 文字列のハッシュ値。
    public: template_hash_value hash;

}; // class psyq::string::_private::flyweight_string

#endif // !PSYQ_STRING_FLYWEIGHT_STRING_HPP_
// vim: set expandtab: