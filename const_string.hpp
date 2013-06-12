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

    - constructorで文字pointerを渡すとmemory割り当てを行い、文字列をcopyする。
    - 上記以外では、memory割り当てを一切行わない。
    - 文字列定数なので、文字列を書き換えるinterfaceは持たない。

    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @tparam template_allocator_type memory割当子の型。

    @note スレッドセーフではないので、後日対応したい。
 */
template<
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
class psyq::basic_const_string:
    public psyq::basic_reference_string<
        template_char_type, template_char_traits>
{
    /// thisが指す値の型。
    public: typedef psyq::basic_const_string<
        template_char_type, template_char_traits, template_allocator_type>
            self;

    /// self の上位型。
    public: typedef psyq::basic_reference_string<
        template_char_type, template_char_traits>
            super;

    /// memory割当子の型。
    public: typedef template_allocator_type allocator_type;

    //-------------------------------------------------------------------------
    /** @brief 空文字列を構築する。memory割り当ては行わない。
     */
    public: explicit basic_const_string(
        typename self::allocator_type const& in_allocator =
            self::allocator_type())
    :
        super(),
        holder_(NULL),
        allocator_(in_allocator)
    {
        // pass
    }

    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_source copy元の文字列。
     */
    public: basic_const_string(
        self const& in_source)
    :
        super(in_source),
        holder_(in_source.holder_),
        allocator_(in_source.allocator_)
    {
        self::hold_string(in_source.holder_);
    }

    /** @brief 文字列を移動する。memory割り当ては行わない。
        @param[in,out] io_source move元の文字列。
     */
    public: basic_const_string(
        self&& io_source)
    {
        this->swap(io_source);
    }

    /** @brief 文字列literalを参照する。memory割り当ては行わない。
        @tparam template_size 参照する文字列literalの要素数。
        @param[in] in_string    参照する文字列literal。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: template <std::size_t template_size>
    basic_const_string(
        typename self::value_type const (&in_string)[template_size],
        typename self::allocator_type const& in_allocator =
            self::allocator_type())
    :
        super(in_string),
        holder_(NULL),
        allocator_(in_allocator)
    {
        // pass
    }

    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_string 参照する文字列。
        @param[in] in_offset 参照する文字列の開始offset位置。
        @param[in] in_count  参照する文字数。
     */
    public: basic_const_string(
        self const&                    in_string,
        typename self::size_type const in_offset,
        typename self::size_type const in_count = super::npos)
    :
        super(
            in_string.data() + in_offset,
            super::trim_count(in_string, in_offset, in_count)),
        holder_(in_string.holder_),
        allocator_(in_string.allocator_)
    {
        self::hold_string(in_string.holder_);
    }

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_string    copy元の文字列の先頭位置。
        @param[in] in_size      copy元の文字列の長さ。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: basic_const_string(
        typename self::const_pointer const   in_string,
        typename self::size_type const       in_size,
        typename self::allocator_type const& in_allocator =
            self::allocator_type())
    :
        super(),
        holder_(NULL),
        allocator_(in_allocator)
    {
        if (in_size <= 0)
        {
            return;
        }

        // 文字列bufferの確保。
        typename self::holder_allocator local_allocator(this->allocator_);
        typename self::holder* const local_holder(
            local_allocator.allocate(self::count_allocate_size(in_size)));
        if (local_holder == NULL)
        {
            PSYQ_ASSERT(false);
            return;
        }

        // 文字列bufferを初期化。
        typename self::value_type* const local_string(
            reinterpret_cast<self::value_type*>(local_holder + 1));
        new(this) super(local_string, in_size);
        this->holder_ = local_holder;
        local_holder->refrence_count_ = 1;
        local_holder->string_length_ = in_size;
        PSYQ_ASSERT(in_string != NULL);
        super::traits_type::copy(local_string, in_string, in_size);
        local_string[in_size] = 0;
    }

    /// destructor
    public: ~basic_const_string()
    {
        typename self::holder* const local_holder(this->holder_);
        if (local_holder != NULL)
        {
            //bool const local_destroy(
            //    1 == std::atomic_fetch_sub_explicit(
            //        &local_holder->refrence_count_, 1, std::memory_order_release));
            //if (local_destroy)
            if ((--local_holder->refrence_count_) == 0)
            {
                //std::atomic_thread_fence(std::memory_order_acquire);
                local_holder->~holder();
                typename self::holder_allocator(this->allocator_).deallocate(
                    local_holder, self::count_allocate_size(this->size()));
            }
        }
    }

    //-------------------------------------------------------------------------
    public: self& operator=(self const& in_right)
    {
        if (this != &in_right)
        {
            self(in_right).swap(*this);
        }
        return *this;
    }

    public: self& operator=(self&& io_right)
    {
        this->swap(io_right);
        return *this;
    }

    public: typename self::allocator_type const& get_allocator() const
    {
        return this->allocator_;
    }

    /** @brief 文字列を交換する。
        @param[in,out] io_target 交換する文字列。
     */
    public: void swap(self& io_right)
    {
        this->super::swap(io_right);
        std::swap(this->holder_, io_right.holder_);
        std::swap(this->allocator_, io_right.allocator_);
    }

    //-------------------------------------------------------------------------
    /// 文字列保持子。
    private: struct holder
    {
        std::size_t refrence_count_; ///< 文字列の被参照数。
        std::size_t string_length_;  ///< 参照してる文字列の文字数。
    };

    /// 文字列保持子のmemory割当子。
    private: typedef typename self::allocator_type::template
        rebind<typename self::holder>::other
            holder_allocator;

    private: static void hold_string(
        typename self::holder* const io_holder)
    {
        if (io_holder != NULL)
        {
            //std::atomic_fetch_add_explicit(
            //    &io_holder->refrence_count_, 1, std::memory_order_relaxed);
            ++io_holder->refrence_count_;
        }
    }

    private:
    static typename self::holder_allocator::size_type count_allocate_size(
        typename self::size_type const in_string_length)
    {
        std::size_t const local_holder_size(sizeof(typename self::holder));
        std::size_t const local_string_size(
            sizeof(typename self::value_type) * (in_string_length + 1) +
            local_holder_size - 1);
        return 1 + local_string_size / local_holder_size;
    }

    //-------------------------------------------------------------------------
    private: typename self::holder*        holder_;    ///< 文字列保持子。
    private: typename self::allocator_type allocator_; ///< 使用するmemory割当子。
};

//-----------------------------------------------------------------------------
namespace std
{
    /** @brief 文字列を交換する。
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

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief CSV形式の文字列を解析し、STL互換コンテナに格納する。
    @param[in,out] io_csv_rows         解析した結果を格納するSTL互換コンテナ。
    @param[in]     in_csv_string       解析するCSV形式のSTL互換文字列。
    @param[in]     in_field_separator  field区切り文字。
    @param[in]     in_record_separator record区切り文字。
    @param[in]     in_quote_begin      引用符の開始文字。
    @param[in]     in_quote_end        引用符の終了文字。
    @param[in]     in_quote_escape     引用符のescape文字。
 */
template<typename template_csv>
bool parse_csv(
    template_csv& io_csv_rows,
    typename template_csv::value_type::value_type const& in_csv_string,
    typename template_csv::value_type::value_type::value_type const
        in_field_separator = ',',
    typename template_csv::value_type::value_type::value_type const
        in_record_separator = '\n',
    typename template_csv::value_type::value_type::value_type const
        in_quote_begin = '"',
    typename template_csv::value_type::value_type::value_type const
        in_quote_end = '"',
    typename template_csv::value_type::value_type::value_type const
        in_quote_escape = '"')
{
    PSYQ_ASSERT(in_quote_escape != 0);

    io_csv_rows.push_back(typename template_csv::value_type());

    bool local_quote(false);
    template_csv::value_type::value_type local_field;
    template_csv::value_type::value_type::value_type local_last_char(0);
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
                        local_field.push_back(*i);
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
                local_field.push_back(*i);
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
                local_field.push_back(local_last_char);
                local_field.push_back(*i);
                local_last_char = *i;
            }
        }
        else if (*i == in_quote_begin)
        {
            // 引用符の開始。
            local_quote = true;
        }
        else if (*i == in_field_separator)
        {
            // fieldの区切り。
            io_csv_rows.last().push_back(std::move(local_field));
            local_field.clear();
        }
        else if (*i == in_record_separator)
        {
            // recordの区切り。
            if (!local_field.empty() || !io_csv_rows.last().empty())
            {
                io_csv_rows.last().push_back(std::move(local_field));
            }
            local_field.clear();
            io_csv_rows.push_back(typename template_csv::value_type());
        }
        else
        {
            local_field.push_back(*i);
        }
    }

    // 最終fieldの処理。
    if (!local_field.empty())
    {
        if (local_quote)
        {
            //local_field.insert(local_field.begin(), in_quote_begin);
        }
        io_csv_rows.last().push_back(std::move(local_field));
    }
    else if (io_csv_rows.last().empty())
    {
        io_csv_rows.pop_back();
    }
}

#endif // PSYQ_CONST_STRING_HPP_
