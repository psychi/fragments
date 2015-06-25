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
 */
#ifndef PSYQ_STRING_FLYWEIGHT_HPP_
#define PSYQ_STRING_FLYWEIGHT_HPP_

#include "./view.hpp"
#include "./flyweight_factory.hpp"

/// @brief フライ級文字列生成器に適用するデフォルトのメモリ割当子の型
#ifndef PSYQ_STRING_FLYWEIGHT_ALLOCATOR_DEFAULT
#define PSYQ_STRING_FLYWEIGHT_ALLOCATOR_DEFAULT std::allocator<void*>
#endif // PSYQ_STRING_FLYWEIGHT_ALLOCATOR_DEFAULT

/// @brief フライ級文字列生成器の文字列チャンクのデフォルト容量。
#ifndef PSYQ_STRING_FLYWEIGHT_CHUNK_SIZE_DEFAULT
#define PSYQ_STRING_FLYWEIGHT_CHUNK_SIZE_DEFAULT 4096
#endif // !defined(PSYQ_STRING_FLYWEIGHT_CHUNK_SIZE_DEFAULT)

/// @cond
namespace psyq
{
    namespace string
    {
        template<typename, typename, typename> class flyweight;
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string_view を模した、flyweightパターンの文字列。

    base_type::base_type::factory を介して文字列を管理する。

    @tparam template_char_type   @copydoc psyq::string::view::value_type
    @tparam template_char_traits @copydoc psyq::string::view::traits_type
    @tparam template_allocator   @copydoc psyq::string::_private::flyweight_factory::allocator_type
 */
template<
    typename template_char_type,
    typename template_char_traits = PSYQ_STRING_VIEW_TRAITS_DEFAULT,
    typename template_allocator = PSYQ_STRING_FLYWEIGHT_ALLOCATOR_DEFAULT>
class psyq::string::flyweight:
public psyq::string::_private::interface_immutable<
    typename psyq::string::_private::flyweight_factory<
        psyq::string::view<template_char_type, template_char_traits>,
        psyq::fnv1a_hash32,
        template_allocator>
            ::_private_client>
{
    /// @brief thisが指す値の型。
    private: typedef flyweight this_type;

    /// @brief this_type の基底型。
    public: typedef
        psyq::string::_private::interface_immutable<
            typename psyq::string::_private::flyweight_factory<
                psyq::string::view<template_char_type, template_char_traits>,
                psyq::fnv1a_hash32,
                template_allocator>
                    ::_private_client>
        base_type;

    /** @brief 空の文字列を構築する。メモリ確保は行わない。
     */
    public: flyweight()
    PSYQ_NOEXCEPT: base_type(base_type::base_type::make())
    {}

    /** @brief 文字列をコピー構築する。メモリ確保は行わない。
        @param[in] in_source コピー元となる文字列。
     */
    public: flyweight(this_type const& in_source)
    PSYQ_NOEXCEPT: base_type(in_source)
    {}

    /** @brief 文字列をムーブ構築する。メモリ確保は行わない。
        @param[in,out] io_source ムーブ元となる文字列。
     */
    public: flyweight(this_type&& io_source)
    PSYQ_NOEXCEPT: base_type(std::move(io_source))
    {}

    /** @brief 文字列を構築する。メモリ確保を行う場合ある。
        @param[in] in_string  コピー元となる文字列。
        @param[in] in_factory 文字列生成器。
        @param[in] in_chunk_size
            文字列チャンクを生成する場合の、デフォルトのチャンク容量。
     */
    public: flyweight(
        typename base_type::view const& in_string,
        typename base_type::base_type::factory::shared_ptr in_factory,
        std::size_t const in_chunk_size =
            PSYQ_STRING_FLYWEIGHT_CHUNK_SIZE_DEFAULT)
    :
    base_type(
        base_type::base_type::make(
            std::move(in_factory), in_string, in_chunk_size))
    {}

    /** @brief 文字列をコピー代入する。メモリ確保は行わない。
        @param[in] in_source コピー元となる文字列。
        @return *this
     */
    public: this_type& operator=(this_type const& in_source)
    {
        this->base_type::base_type::operator=(in_source);
        return *this;
    }

    /** @brief 文字列をムーブ代入する。メモリ確保は行わない。
        @param[in,out] io_source ムーブ元となる文字列。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->base_type::base_type::operator=(std::move(io_source));
        return *this;
    }

}; // class psyq::string::flyweight

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    inline void flyweight_string()
    {
        typedef psyq::string::flyweight<char> string;
        string::factory::shared_ptr const local_factory(new string::factory);
        string local_string("stringstringstring", local_factory, 0);
        PSYQ_ASSERT(local_factory->count_hash(local_string.get_hash()) == 1);
        local_string.data();
        local_string = string("abcdefg", local_factory);
        local_string.data();
        local_string = string("string", local_factory);
        local_string.data();
        local_string.clear();
        local_factory->collect_garbage();
    }
}

#endif // !PSYQ_STRING_FLYWEIGHT_HPP_
// vim: set expandtab:
