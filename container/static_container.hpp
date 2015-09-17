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
/// @copybrief psyq::container::static_vector
#ifndef PSYQ_CONTAINER_STATIC_CONTAINER_HPP_
#define PSYQ_CONTAINER_STATIC_CONTAINER_HPP_

#include "../assert.hpp"

#if defined(NDEBUG) && !defined(PSYQ_CONTAINER_STATIC_CONTAINER_NO_ARRAY_VIEW)
#define PSYQ_CONTAINER_STATIC_CONTAINER_NO_ARRAY_VIEW
#endif // !defined(NDEBUG) && !defined(PSYQ_CONTAINER_STATIC_CONTAINER_NO_ARRAY_VIEW)

namespace psyq
{
    /// @brief C++標準ライブラリを模したコンテナ群。
    namespace container
    {
        /// @brief psyq::container 管理者以外がこの名前空間に直接アクセスするのは禁止。
        namespace _private
        {
            /// @cond
            template<typename, std::size_t, typename> class static_container;
            /// @endcond
        } // namespace _private
    } // namespace container
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 動的メモリ割当を行わないコンテナの基底型。
/// @tparam template_value    @copydoc static_container::value_type
/// @tparam template_max_size @copydoc static_container::MAX_SIZE
/// @tparam template_size     @copydoc static_container::size_type
template<
    typename template_value,
    std::size_t template_max_size,
    typename template_size>
class psyq::container::_private::static_container
{
    /// @copydoc psyq::string::view::this_type
    private: typedef static_container this_type;

    //-------------------------------------------------------------------------
    /// @brief コンテナに格納する要素を表す型。
    public: typedef template_value value_type;
    /// @brief 要素を指すポインタを表す型。
    public: typedef typename this_type::value_type* pointer;
    /// @brief 読み取り専用の要素を指すポインタを表す型。
    public: typedef typename this_type::value_type const* const_pointer;
    /// @brief 要素への参照を表す型。
    public: typedef typename this_type::value_type& reference;
    /// @brief 読み取り専用の要素への参照を表す型。
    public: typedef typename this_type::value_type const& const_reference;

    //-------------------------------------------------------------------------
    /// @brief 要素の数を表す型。
    public: typedef template_size size_type;
    static_assert(
        std::is_integral<template_size>::value
        && std::is_unsigned<template_size>::value,
        "'template_size' is not unsigned integer type");
    /// @brief 要素を指すポインタの差分を表す型。
    public: typedef
        typename std::make_signed<typename this_type::size_type>::type
        difference_type;
    public: enum: typename this_type::size_type
    {
        /// コンテナに格納できる要素の最大数。
        MAX_SIZE = template_max_size,
    };
    static_assert(0 < template_max_size, "'template_max_size' is empty.");
    static_assert(
        template_max_size <= ~(~0ull << (sizeof(template_size) * CHAR_BIT - 1)),
        "'template_max_size' is over flow.");

    //-------------------------------------------------------------------------
    /// @brief コンテナとして使う固定長のメモリ領域を表す型。
    private: typedef
        typename std::aligned_storage<
            sizeof(typename this_type::value_type) * this_type::MAX_SIZE,
            PSYQ_ALIGNOF(typename this_type::value_type)>
        ::type
        storage;
    /// @brief デバッグ時に、コンテナの内容を見るために使う。
    private: typedef template_value array_view[this_type::MAX_SIZE];

    //-------------------------------------------------------------------------
    protected: static_container() PSYQ_NOEXCEPT
#ifndef PSYQ_CONTAINER_STATIC_CONTAINER_NO_ARRAY_VIEW
    : array_view_(
        *reinterpret_cast<typename this_type::array_view*>(&this->storage_))
#endif // !defined(PSYQ_CONTAINER_STATIC_CONTAINER_NO_ARRAY_VIEW)
    {}

    //-------------------------------------------------------------------------
    protected: typename this_type::pointer get_element(
        typename this_type::size_type const in_offset)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_offset <= this_type::MAX_SIZE);
        return reinterpret_cast<typename this_type::pointer>(&this->storage_)
            + in_offset;
    }

    protected: typename this_type::const_pointer get_element(
        typename this_type::size_type const in_offset)
    const PSYQ_NOEXCEPT
    {
        return const_cast<this_type>(this)->get_element(in_offset);
    }

    protected: typename this_type::size_type get_offset(
        typename this_type::const_pointer const in_pointer)
    const PSYQ_NOEXCEPT
    {
        return in_pointer - this->get_element(0);
    }

    /// @brief コンテナに含まれている値か判定する。
    /// @retval true  コンテナに含まれてる値だった。
    /// @retval false コンテナに含まれてない値だった。
    protected: bool is_contained(
        /// [in] 判定する値。
        typename this_type::const_reference in_value)
    const PSYQ_NOEXCEPT
    {
        return this->get_offset(&in_value) < this_type::MAX_SIZE;
    }

    /// @brief コンテナに含まれているポインタか判定する。
    /// @retval true  コンテナに含まれてるポインタだった。
    /// @retval false コンテナに含まれてないポインタだった。
    protected: bool is_contained(
        /// [in] 判定する値。
        typename this_type::const_pointer const in_pointer)
    const PSYQ_NOEXCEPT
    {
        return this->get_offset(in_pointer) <= this_type::MAX_SIZE;
    }

    //-------------------------------------------------------------------------
    protected: void default_construct(
        typename this_type::size_type const in_count,
        typename this_type::pointer const in_target)
    {
        PSYQ_ASSERT(
            this->is_contained(in_target + in_count)
            && this->is_contained(in_target));
        auto const local_target_end(in_target + in_count);
        for (auto i(in_target); i < local_target_end; ++i)
        {
            new(i) typename this_type::value_type;
        }
    }

    protected: void copy_construct(
        typename this_type::const_reference in_source,
        typename this_type::size_type const in_count,
        typename this_type::pointer const in_target)
    {
        PSYQ_ASSERT(
            this->is_contained(in_target + in_count)
            && this->is_contained(in_target));
        auto const local_target_end(in_target + in_count);
        for (auto i(in_target); i < local_target_end; ++i)
        {
            new(i) typename this_type::value_type(in_source);
        }
    }

    protected: template<typename template_iterator>
    void copy_construct(
        template_iterator const& in_source,
        typename this_type::size_type const in_count,
        typename this_type::pointer const in_target)
    {
        PSYQ_ASSERT(
            this->is_contained(in_target + in_count)
            && this->is_contained(in_target));
        auto local_source(in_source);
        auto const local_target_end(in_target + in_count);
        for (auto i(in_target); i < local_target_end; ++i, ++local_source)
        {
            new(i) typename this_type::value_type(*local_source);
        }
    }

    //-------------------------------------------------------------------------
    /// @brief コンテナ外部の値からムーブ構築する。
    protected: template<typename template_iterator>
    void move_construct_import(
        /// [in] ムーブ元となる範囲の先頭を指す反復子。
        template_iterator const& in_source,
        /// [in] 構築する要素の数。
        typename this_type::size_type const in_count,
        /// [in] ムーブ先となる範囲の先頭を指すポインタ。
        typename this_type::pointer const in_target)
    {
        PSYQ_ASSERT(
            this->is_contained(in_target + in_count)
            && this->is_contained(in_target));
        auto local_source(in_source);
        auto const local_target_end(in_target + in_count);
        for (auto i(in_target); i < local_target_end; ++i, ++local_source)
        {
            new(i) typename this_type::value_type(std::move(*local_source));
        }
    }

    /// @brief コンテナ内部の値を先頭からムーブ構築する。
    protected: void move_construct_forward(
        /// [in] ムーブ元となる範囲の先頭を指すポインタ。
        typename this_type::pointer in_source,
        /// [in] 構築する要素の数。
        typename this_type::size_type const in_count,
        /// [in] ムーブ先となる範囲の先頭を指すポインタ。
        typename this_type::pointer in_target)
    {
        PSYQ_ASSERT(
            this->is_contained(in_target + in_count)
            && this->is_contained(in_source)
            && in_source <= in_target);
        if (in_source < in_target)
        {
            auto const local_source_end(in_source + in_count);
            while (in_source < local_source_end)
            {
                new(in_target) this_type::value_type(std::move(*in_source));
                in_source->~value_type();
                ++in_target;
                ++in_source;
            }
        }
    }

    /// @brief コンテナ内部の値を末尾からムーブ構築する。
    protected: void move_construct_backward(
        /// [in] ムーブ元となる範囲の先頭を指すポインタ。
        typename this_type::pointer const in_source,
        /// [in] 構築する要素の数。
        typename this_type::size_type const in_count,
        /// [in] ムーブ先となる範囲の先頭を指すポインタ。
        typename this_type::pointer const in_target)
    {
        PSYQ_ASSERT(
            this->is_contained(in_source + in_count)
            && this->is_contained(in_target)
            && in_target <= in_source);
        if (in_target < in_source)
        {
            auto local_target(in_target + in_count);
            auto local_source(in_source + in_count);
            while (in_source < local_source)
            {
                --local_target;
                --local_source;
                new(local_target) this_type::value_type(std::move(*local_source));
                local_source->~value_type();
            }
        }
    }

    protected: void destruct(
        typename this_type::pointer const in_position,
        typename this_type::size_type const in_count)
    {
        PSYQ_ASSERT(
            this->is_contained(in_position + in_count)
            && this->is_contained(in_position));
        for (auto i(in_position + in_count - 1); in_position <= i; --i)
        {
            i->~value_type();
        }
    }

    //-------------------------------------------------------------------------
    private: static_container(this_type const&);
    private: static_container(this_type&&);
    private: this_type& operator=(this_type const&);
    private: this_type& operator=(this_type&&);

    //-------------------------------------------------------------------------
    /// @brief コンテナの要素を格納する領域。
    private: typename this_type::storage storage_;
#ifndef PSYQ_CONTAINER_STATIC_CONTAINER_NO_ARRAY_VIEW
    /// @copydoc array_view
    private: typename this_type::array_view& array_view_;
#endif // !defined(PSYQ_CONTAINER_STATIC_CONTAINER_NO_ARRAY_VIEW)

}; // class psyq::container::_private::static_container

#endif // !defined(PSYQ_CONTAINER_STATIC_CONTAINER_HPP_)
// vim: set expandtab:
