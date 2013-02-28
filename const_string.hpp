/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

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
 */
/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_CONST_STRING_HPP_
#define PSYQ_CONST_STRING_HPP_

//#include "reference_string.hpp"

#ifndef PSYQ_BASIC_CONST_STRING_ALLOCATOR_DEFAULT
#define PSYQ_BASIC_CONST_STRING_ALLOCATOR_DEFAULT\
    std::allocator<typename template_char_traits::char_type>
#endif // !PSYQ_BASIC_CONST_STRING_ALLOCATOR_DEFAULT

namespace psyq
{
    /// @cond
    template<
        typename template_char_type,
        typename template_char_traits = PSYQ_BASIC_REFERENCE_STRING_DEFAULT,
        typename template_allocator_type =
            PSYQ_BASIC_CONST_STRING_ALLOCATOR_DEFAULT>
                class basic_const_string;
    /// @endcond

    /// char型の文字を扱う basic_const_string
    typedef psyq::basic_const_string<char> const_string;

    /// wchar_t型の文字を扱う basic_const_string
    typedef psyq::basic_const_string<wchar_t> const_wstring;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string のinterfaceを模した、文字列定数。

    文字列定数なので、文字列を書き換えるinterfaceは持たない。

    constructorで文字列literalを渡すと、memory割り当てを行わない。

    constructorで文字列literal以外の文字列を渡すと、
    memory割り当てを行い、文字列をcopyする。

    constructor以外では、memory割り当てを一切行わない。

    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @tparam template_allocator_type memory割当子の型。
 */
template<
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
class psyq::basic_const_string:
    private psyq::basic_reference_string<
        template_char_type, template_char_traits>
{
    /// thisが指す値の型。
    public: typedef psyq::basic_const_string<
        template_char_type, template_char_traits, template_allocator_type>
            self;

    /// self の上位型。
    private: typedef psyq::basic_reference_string<
        template_char_type, template_char_traits>
            super;

    /// memory割当子の型。
    public: typedef template_allocator_type allocator_type;

    // publicにする上位型の内部型。
    public: using super::traits_type;
    public: using super::value_type;
    public: using super::size_type;
    public: using super::difference_type;
    public: using super::pointer;
    public: using super::const_pointer;
    public: using super::reference;
    public: using super::const_reference;
    public: using super::iterator;
    public: using super::const_iterator;
    public: using super::reverse_iterator;
    public: using super::const_reverse_iterator;

    // publicにする上位型のmember関数。
    public: using super::data;
    public: using super::begin;
    public: using super::end;
    public: using super::cbegin;
    public: using super::cend;
    public: using super::rbegin;
    public: using super::rend;
    public: using super::crbegin;
    public: using super::crend;
    public: using super::front;
    public: using super::back;
    public: using super::length;
    public: using super::size;
    public: using super::max_size;
    public: using super::capacity;
    public: using super::empty;
    public: using super::at;
    public: using super::operator[];
    public: using super::compare;
    public: using super::find;
    public: using super::rfind;
    public: using super::find_first_of;
    public: using super::find_last_of;
    public: using super::find_first_not_of;
    public: using super::find_last_not_of;

    // publicにする上位型のmember定数。
    public: using super::npos;

    //-------------------------------------------------------------------------
    /** @brief memory割当子を使わず、空文字列を構築。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: explicit basic_const_string(
        typename self::allocator_type const& in_allocator =
            self::allocator_type())
    :
        super(),
        is_allocated_(false),
        allocator_(in_allocator)
    {
        // pass
    }

    /** @brief memory割当子を使わず、文字列literalを直接割り当てて構築。
        @tparam template_size 割り当てる文字列literalの要素数。
        @param[in] in_string    割り当てる文字列literal。
        @param[in] in_allocator memory割当子の初期値。
        @note
            この関数を実装するため、
            basic_const_string(self::const_pointer) はomitした。
     */
    public: template <std::size_t template_size>
    basic_const_string(
        typename self::value_type const      (&in_string)[template_size],
        typename self::allocator_type const& in_allocator =
            self::allocator_type())
    :
        super(in_string),
        is_allocated_(false),
        allocator_(in_allocator)
    {
        PSYQ_ASSERT(0 < template_size);
    }

    /** @brief memory割り当てを行い、文字をcopyして構築。
        @param[in] in_count     文字数。
        @param[in] in_char      copyする文字。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: basic_const_string(
        typename self::size_type const      in_count,
        typename self::value_type const     in_char,
        typename self::allocator_type const in_allocator =
            self::allocator_type())
    :
        super(),
        is_allocated_(false),
        allocator_(in_allocator)
    {
        self::value_type* const local_pointer(this->allocate_string(in_count));
        if (local_pointer != NULL)
        {
            for (typename self::size_type i(0); i < in_count; ++i)
            {
                local_pointer[i] = in_char;
            }
            local_pointer[in_count] = 0;
        }
    }

    /** @brief memory割り当てを行い、文字列をcopyして構築。
        @param[in] in_string    copyする文字列の先頭位置。
        @param[in] in_size      copyする文字列の長さ。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: basic_const_string(
        typename self::const_pointer const  in_string,
        typename self::size_type const      in_size,
        typename self::allocator_type const in_allocator =
            self::allocator_type())
    :
        super(),
        is_allocated_(false),
        allocator_(in_allocator)
    {
        this->create_string(in_string, in_size);
    }

    /** @brief memory割り当てを行い、任意型の文字列をcopyして構築。
        @tparam template_string_type @copydoc string_interface
        @param[in] in_string    copyする文字列。
        @param[in] in_offset    copyする文字列の開始offset位置。
        @param[in] in_count     copyする文字数。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: template<typename template_string_type>
    basic_const_string(
        template_string_type const&                    in_string,
        typename template_string_type::size_type const in_offset,
        typename template_string_type::size_type const in_count =
            template_string_type::npos,
        typename self::allocator_type const            in_allocator =
            self::allocator_type())
    :
        super(),
        is_allocated_(false),
        allocator_(in_allocator)
    {
        this->create_string(
            in_string.data() + in_offset,
            super::trim_count(in_string, in_offset, in_count));
    }

    /// destructor
    public: ~basic_const_string()
    {
        if (this->is_allocated())
        {
            this->allocator_.deallocate(
                const_cast<typename self::value_type*>(this->data()),
                this->size() + 1);
        }
    }

    //-------------------------------------------------------------------------
    /** @brief memory割り当てを行ったかどうか。
     */
    public: bool is_allocated() const
    {
        return this->is_allocated_;
    }

    /** @brief 保持しているmemory割当子を取得。
     */
    public: typename self::allocator_type const& get_allocator() const
    {
        return this->allocator_;
    }

    /** @brief NULL文字で終わる文字列を取得。
        @return NULL文字で終わる文字列の先頭位置。
     */
    public: typename self::const_pointer c_str() const
    {
        return this->super::data();
    }

    /** @brief 文字列を交換。
        @param[in,out] io_target 交換する文字列。
     */
    public: void swap(self& io_target)
    {
        this->super::swap(io_target);
        std::swap(this->is_allocated_, io_target.is_allocated_);
        std::swap(this->allocator_, io_target.allocator_);
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @param[in] in_right 右辺の文字列。
        @return 左辺 == 右辺
     */
    public: bool operator==(
        self const& in_right)
    const
    {
        return this->super::is_equal(in_right.data(), in_right.size());
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @tparam template_string_traits @copydoc string_interface
        @param[in] in_right 右辺の文字列。
        @return 左辺 == 右辺
     */
    public: template<typename template_string_type>
    bool operator==(
        template_string_type const& in_right)
    const
    {
        return this->super::is_equal(in_right.data(), in_right.size());
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @tparam template_string_traits @copydoc string_interface
        @param[in] in_right 右辺の文字列。
        @return 左辺 == 右辺
     */
    public: template<std::size_t template_size>
    bool operator==(
        typename self::value_type const (&in_right)[template_size])
    const
    {
        return this->super::is_equal(&in_right[0], template_size - 1);
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @param[in] in_right 右辺の文字列。
        @return 左辺 != 右辺
     */
    public: bool operator!=(
        self const& in_right)
    const
    {
        return !this->super::is_equal(in_right.data(), in_right.size());
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @tparam template_string_traits @copydoc string_interface
        @param[in] in_right 右辺の文字列。
        @return 左辺 != 右辺
     */
    public: template<typename template_string_type>
    bool operator!=(
        template_string_type const& in_right)
    const
    {
        return !this->super::is_equal(in_right.data(), in_right.size());
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @tparam template_string_traits @copydoc string_interface
        @param[in] in_right 右辺の文字列。
        @return 左辺 != 右辺
     */
    public: template<std::size_t template_size>
    bool operator!=(
        typename self::value_type const (&in_right)[template_size])
    const
    {
        return !this->super::is_equal(&in_right[0], template_size - 1);
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @param[in] in_right 右辺の文字列。
        @return 左辺 < 右辺
     */
    public: bool operator<(
        self const& in_right)
    const
    {
        return this->super::compare(in_right) < 0;
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @tparam template_string_traits @copydoc string_interface
        @param[in] in_right 右辺の文字列。
        @return 左辺 < 右辺
     */
    public: template<typename template_string_type>
    bool operator<(
        template_string_type const& in_right)
    const
    {
        return this->super::compare(in_right) < 0;
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @tparam template_string_traits @copydoc string_interface
        @param[in] in_right 右辺の文字列。
        @return 左辺 < 右辺
     */
    public: template<std::size_t template_size>
    bool operator<(
        typename self::value_type const (&in_right)[template_size])
    const
    {
        return this->super::compare(super(in_right)) < 0;
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @param[in] in_right 右辺の文字列。
        @return 左辺 <= 右辺
     */
    public: bool operator<=(
        self const& in_right)
    const
    {
        return this->super::compare(in_right) <= 0;
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @tparam template_string_traits @copydoc string_interface
        @param[in] in_right 右辺の文字列。
        @return 左辺 <= 右辺
     */
    public: template<typename template_string_type>
    bool operator<=(
        template_string_type const& in_right)
    const
    {
        return this->super::compare(in_right) <= 0;
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @tparam template_string_traits @copydoc string_interface
        @param[in] in_right 右辺の文字列。
        @return 左辺 < 右辺
     */
    public: template<std::size_t template_size>
    bool operator<=(
        typename self::value_type const (&in_right)[template_size])
    const
    {
        return this->super::compare(super(in_right)) <= 0;
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @param[in] in_right 右辺の文字列。
        @return 左辺 > 右辺
     */
    public: bool operator>(
        self const& in_right)
    const
    {
        return 0 < this->super::compare(in_right);
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @tparam template_string_traits @copydoc string_interface
        @param[in] in_right 右辺の文字列。
        @return 左辺 > 右辺
     */
    public: template<typename template_string_type>
    bool operator>(
        template_string_type const& in_right)
    const
    {
        return 0 < this->super::compare(in_right);
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @tparam template_string_traits @copydoc string_interface
        @param[in] in_right 右辺の文字列。
        @return 左辺 < 右辺
     */
    public: template<std::size_t template_size>
    bool operator>(
        typename self::value_type const (&in_right)[template_size])
    const
    {
        return 0 < this->super::compare(super(in_right));
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @param[in] in_right 右辺の文字列。
        @return 左辺 >= 右辺
     */
    public: bool operator>=(
        self const& in_right)
    const
    {
        return 0 <= this->super::compare(in_right);
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @tparam template_string_traits @copydoc string_interface
        @param[in] in_right 右辺の文字列。
        @return 左辺 >= 右辺
     */
    public: template<typename template_string_type>
    bool operator>=(
        template_string_type const& in_right)
    const
    {
        return 0 <= this->super::compare(in_right);
    }

    /** @brief 文字列の比較。

        *thisを左辺として、右辺の文字列と比較。

        @tparam template_string_traits @copydoc string_interface
        @param[in] in_right 右辺の文字列。
        @return 左辺 < 右辺
     */
    public: template<std::size_t template_size>
    bool operator>=(
        typename self::value_type const (&in_right)[template_size])
    const
    {
        return 0 <= this->super::compare(super(in_right));
    }

    //-------------------------------------------------------------------------
    /** @brief
        memory割り当てを抑制するため、copy-constructorとcopy演算子は使用禁止。

        copy-constructorやcopy演算子を使えると便利だが、
        copy処理に伴う重みを実感しにくい。
        copy処理を行う場合は、それに伴う重みを実感しやすいように、
        典型的な関数呼び出しの形式で記述するのを推奨する。

        @note
        copy-constructorやcopy演算子がどうしても必要になった場合は、
        psyq::basic_reference_string をsmart-pointerで参照する実装を
        考えること。
     */
    private: basic_const_string(
        self const& in_source)
    :
        super(),
        is_allocated_(in_source.is_allocated_),
        allocator_(in_source.allocator_)
    {
        if (in_source.is_allocated())
        {
            // 文字列literalでなければ、memory割り当てを行い、文字列をcopy。
            this->create_string(in_source.data(), in_source.size());
        }
        else
        {
            // 文字列literalなら、文字列literalを直接割り当てる。
            static_cast<super&>(*this) = in_source;
        }
    }

    /// @copydoc basic_const_string(self const&)
    private: self& operator=(self const&);

    private: typename self::value_type* allocate_string(
        typename self::size_type const in_size)
    {
        if (in_size <= 0)
        {
            return NULL;
        }
        typename self::size_type const local_capacity(in_size + 1);
        typename self::value_type* const local_pointer(
            this->allocator_.allocate(local_capacity));
        if (local_pointer == NULL)
        {
            PSYQ_ASSERT(false);
            return NULL;
        }
        this->is_allocated_ = true;
        new(this) super(local_pointer, in_size);
        return local_pointer;
    }

    private: void create_string(
        typename self::const_pointer const  in_string,
        typename self::size_type const      in_size)
    {
        if (in_string == NULL)
        {
            PSYQ_ASSERT(in_size <= 0);
            return;
        }
        self::value_type* const local_pointer(this->allocate_string(in_size));
        if (local_pointer != NULL)
        {
            self::traits_type::copy(local_pointer, in_string, in_size);
            local_pointer[in_size] = 0;
        }
    }

    //-------------------------------------------------------------------------
    /// memory割り当てを行ったかどうか。
    private: bool is_allocated_;

    /// 使っているmemory割当子。
    private: typename self::allocator_type allocator_;
};

//-----------------------------------------------------------------------------
/** @brief 文字列の比較。
    @tparam template_string_type @copydoc string_interface
    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @tparam template_allocator_type memory割当子の型。
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 == 右辺
 */
template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
bool operator==(
    template_string_type const& in_left,
    psyq::basic_const_string<
        template_char_type,
        template_char_traits,
        template_allocator_type> const&
            in_right)
{
    return in_right.operator==(in_left);
}

/** @brief 文字列の比較。
    @tparam template_string_type @copydoc string_interface
    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @tparam template_allocator_type memory割当子の型。
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 != 右辺
 */
template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
bool operator!=(
    template_string_type const& in_left,
    psyq::basic_const_string<
        template_char_type,
        template_char_traits,
        template_allocator_type> const&
            in_right)
{
    return in_right.operator!=(in_left);
}

/** @brief 文字列の比較。
    @tparam template_string_type @copydoc string_interface
    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @tparam template_allocator_type memory割当子の型。
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 < 右辺
 */
template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
bool operator<(
    template_string_type const& in_left,
    psyq::basic_const_string<
        template_char_type,
        template_char_traits,
        template_allocator_type> const&
            in_right)
{
    return in_right.operator>(in_left);
}

/** @brief 文字列の比較。
    @tparam template_string_type @copydoc string_interface
    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @tparam template_allocator_type memory割当子の型。
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 <= 右辺
 */
template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
bool operator<=(
    template_string_type const& in_left,
    psyq::basic_const_string<
        template_char_type,
        template_char_traits,
        template_allocator_type> const&
            in_right)
{
    return in_right.operator>=(in_left);
}

/** @brief 文字列の比較。
    @tparam template_string_type @copydoc string_interface
    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @tparam template_allocator_type memory割当子の型。
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 > 右辺
 */
template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
bool operator>(
    template_string_type const& in_left,
    psyq::basic_const_string<
        template_char_type,
        template_char_traits,
        template_allocator_type> const&
            in_right)
{
    return in_right.operator<(in_left);
}

/** @brief 文字列の比較。
    @tparam template_string_type @copydoc string_interface
    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @tparam template_allocator_type memory割当子の型。
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 >= 右辺
 */
template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
bool operator>=(
    template_string_type const& in_left,
    psyq::basic_const_string<
        template_char_type,
        template_char_traits,
        template_allocator_type> const&
            in_right)
{
    return in_right.operator<=(in_left);
}

//-----------------------------------------------------------------------------
namespace std
{
    /** @brief 文字列の交換。
        @tparam template_char_type
            @copydoc psyq::basic_reference_string::value_type
        @tparam template_char_traits
            @copydoc psyq::basic_reference_string::traits_type
        @tparam template_allocator_type memory割当子の型。
        @param[in] in_left  交換する文字列。
        @param[in] in_right 交換する文字列。
     */
    template<
        typename template_char_type,
        typename template_char_traits,
        typename template_allocator_type>
    void swap(
        psyq::basic_const_string<
            template_char_type, template_char_traits, template_allocator_type>&
                io_left,
        psyq::basic_const_string<
            template_char_type, template_char_traits, template_allocator_type>&
                io_right)
    {
        io_left.swap(io_right);
    }
};

#endif // PSYQ_CONST_STRING_HPP_
