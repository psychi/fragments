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
    @brief @copydoc psyq::message_pack::serializer

    使用例
    @code
    psyq::message_pack::serializer<> local_serializer;
    // 真偽値を直列化する。
    local_serializer << false << true;
    // 整数を直列化する。
    local_serializer << -0x12 << 0x1234 << -0x12345678L << 0x123456789abcdefLL;
    // 浮動小数点数を直列化する。
    local_serializer << -1.2f << 3.4;
    // コンテナを直列化する。
    local_serializer << std::make_tuple(
        std::vector<std::uint32_t>(4, 0x12345678),
        std::list<std::string>(3, std::string("std::string")));
    @endcode
 */
#ifndef PSYQ_MESSAGE_PACK_SERIALIZER_HPP_
#define PSYQ_MESSAGE_PACK_SERIALIZER_HPP_

#include <array>
#include <sstream>
#include <tuple>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

/// psyq::message_pack::serializer のスタック限界数のデフォルト値。
#ifndef PSYQ_MESSAGE_PACK_SERIALIZER_STACK_CAPACITY_DEFAULT
#define PSYQ_MESSAGE_PACK_SERIALIZER_STACK_CAPACITY_DEFAULT 32
#endif // !defined(PSYQ_MESSAGE_PACK_SERIALIZER_STACK_CAPACITY_DEFAULT)

#ifndef PSYQ_ASSERT
#include <assert.h>
#define PSYQ_ASSERT assert
#endif // !defined(PSYQ_ASSERT)

#ifndef PSYQ_NOEXCEPT
#define PSYQ_NOEXCEPT
#endif // !defined(PSYQ_NOEXCEPT)

namespace psyq
{
    namespace message_pack
    {
        /// @cond
        template<
            typename = std::ostringstream,
            std::size_t = PSYQ_MESSAGE_PACK_SERIALIZER_STACK_CAPACITY_DEFAULT>
                class serializer;
        template<typename> struct raw_stream;
        /// @endcond

        /// 値を直列化／直列化復元する際のエンディアン性。
        enum endianess
        {
            little_endian = 0, ///< リトルエンディアン。
            big_endian = 1,    ///< ビッグエンディアン。
            //native_endian = (__BYTE_ORDER == __BIG_ENDIAN), ///< ネイティブエンディアン。
        };
        static_assert(
            static_cast<int>(true) == 1 && static_cast<int>(false) == 0, "");

        //---------------------------------------------------------------------
        /** @brief MessagePackに格納されている値の種別。

            以下のウェブページにある仕様を参照した。
            https://github.com/msgpack/msgpack/blob/d257d3c143c9fa21ba22afc666fe50cc66943ece/spec.md#formats
         */
        enum header
        {
            header_FIX_INT_MIN   =-0x20, ///< 最小の固定値整数。
            header_FIX_INT_MAX   = 0x7f, ///< 最大の固定値整数。
            header_FIX_MAP_MIN   = 0x80, ///< 最小長の固定長連想配列。
            header_FIX_MAP_MAX   = 0x8f, ///< 最大長の固定長連想配列。
            header_FIX_ARRAY_MIN = 0x90, ///< 最小長の固定長配列。
            header_FIX_ARRAY_MAX = 0x9f, ///< 最大長の固定長配列。
            header_FIX_STR_MIN   = 0xa0, ///< 最小長の固定長文字列。
            header_FIX_STR_MAX   = 0xbf, ///< 最大長の固定長文字列。
            header_NIL           = 0xc0, ///< 空値。
            header_NEVER_USED    = 0xc1, ///< 未使用。
            header_FALSE         = 0xc2, ///< false
            header_TRUE          = 0xc3, ///< true
            header_BIN8          = 0xc4, ///< 長さが8bit以下のバイナリ。
            header_BIN16         = 0xc5, ///< 長さが16bit以下のバイナリ。
            header_BIN32         = 0xc6, ///< 長さが32bit以下のバイナリ。
            header_EXT8          = 0xc7, ///< 長さが8bit以下の拡張バイナリ。
            header_EXT16         = 0xc8, ///< 長さが16bit以下の拡張バイナリ。
            header_EXT32         = 0xc9, ///< 長さが32bit以下の拡張バイナリ。
            header_FLOAT32       = 0xca, ///< IEEE754単精度浮動小数点数。
            header_FLOAT64       = 0xcb, ///< IEEE754倍精度浮動小数点数。
            header_UINT8         = 0xcc, ///< 0以上の8bit整数。
            header_UINT16        = 0xcd, ///< 0以上の16bit整数。
            header_UINT32        = 0xce, ///< 0以上の32bit整数。
            header_UINT64        = 0xcf, ///< 0以上の64bit整数。
            header_INT8          = 0xd0, ///< 0未満の8bit整数。
            header_INT16         = 0xd1, ///< 0未満の16bit整数。
            header_INT32         = 0xd2, ///< 0未満の32bit整数。
            header_INT64         = 0xd3, ///< 0未満の64bit整数。
            header_FIX_EXT1      = 0xd4, ///< 長さが1の拡張バイナリ。
            header_FIX_EXT2      = 0xd5, ///< 長さが2の拡張バイナリ。
            header_FIX_EXT4      = 0xd6, ///< 長さが4の拡張バイナリ。
            header_FIX_EXT8      = 0xd7, ///< 長さが8の拡張バイナリ。
            header_FIX_EXT16     = 0xd8, ///< 長さが16の拡張バイナリ。
            header_STR8          = 0xd9, ///< 長さが8bit以下の文字列。
            header_STR16         = 0xda, ///< 長さが16bit以下の文字列。
            header_STR32         = 0xdb, ///< 長さが32bit以下の文字列。
            header_ARRAY16       = 0xdc, ///< 長さが16bit以下の配列。
            header_ARRAY32       = 0xdd, ///< 長さが32bit以下の配列。
            header_MAP16         = 0xde, ///< 長さが16bit以下の連想配列。
            header_MAP32         = 0xdf, ///< 長さが32bit以下の連想配列。
        };
    }

    //ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
    /// この名前空間をuserが直接accessするのは禁止。
    namespace internal
    {
        //---------------------------------------------------------------------
        /** @brief タプルをストリームへ出力する。
            @tparam template_length 出力するタプルの要素数。
         */
        template<std::size_t template_length>
        struct message_pack_tuple_serializer
        {
            /** @brief タプルをストリームへ出力する。
                @tparam template_stream 出力演算子が使えるストリーム型。
                @tparam template_tuple  std::get() で要素を取得できるタプル型。
                @param[out] out_stream 出力先ストリーム。
                @param[in]  in_tuple   出力するタプル。
             */
            template<typename template_stream, typename template_tuple>
            static bool write(
                template_stream& out_stream,
                template_tuple const& in_tuple)
            {
                psyq::internal
                    ::message_pack_tuple_serializer<template_length - 1>
                        ::write(out_stream, in_tuple);
                out_stream << std::get<template_length - 1>(in_tuple);
                return out_stream.good();
            }
        };
        /// @copydoc message_pack_tuple_serializer
        template<> struct message_pack_tuple_serializer<0>
        {
            /// タプルの要素数が0なので、何もしない。
            template<typename template_stream, typename template_tuple>
            static bool write(template_stream&, template_tuple const&)
            {
                return true;
            }
        };
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief MessagePack形式で直列化したバイナリを、
           std::basic_ostream 互換のストリームへ出力するアダプタ。
    @tparam template_out_stream @copydoc self::stream
    @tparam template_stack_capacity @copydoc self::stack_capacity
 */
template<typename template_out_stream, std::size_t template_stack_capacity>
class psyq::message_pack::serializer
{
    /// thisが指す値の型。
    private: typedef serializer<template_out_stream, template_stack_capacity>
        self;

    /** @brief 直列化したMessagePack値を出力する、
               std::basic_ostream 互換のストリーム。
     */
    public: typedef template_out_stream stream;
    static_assert(
        sizeof(typename self::stream::char_type) == 1,
        "sizeof(self::stream::char_type) is not 1.");

    /** @brief 直列化途中のMessagePackコンテナのスタック限界数。
     */
    public: static std::size_t const stack_capacity = template_stack_capacity;

    /** @brief 次に直列化する値の種類。
     */
    public: enum next_type
    {
        next_type_VALUE,
        next_type_RAW_BYTES,     ///< RAWバイト列。
        next_type_ARRAY_ELEMENT, ///< 配列の要素。
        next_type_MAP_KEY,       ///< 連想配列の要素のキー。
        next_type_MAP_VALUE,     ///< 連想配列の要素の値。
    };

    /// @copydoc self::stack_
    private: struct stack
    {
        std::size_t rest_size;         ///< MessagePackコンテナ要素の残数。
        typename self::next_type type; ///< @copydoc self::next_type
    };

    //-------------------------------------------------------------------------
    /// @name 構築と破壊
    //@{
    /** @brief 出力ストリームを構築する。
        @param[in] in_stream 出力ストリームの初期値。
     */
    public: explicit serializer(
        typename self::stream in_stream = self::stream())
    :
        stream_(std::move(in_stream)),
        stack_size_(0)
    {}

    /** @brief move構築子。
        @param[in,out] io_source 移動元インスタンス。
     */
    public: serializer(self&& io_source):
        stream_(std::move(io_source.stream_)),
        stack_(std::move(io_source.stack_)),
        stack_size_(std::move(io_source.stack_size_))
    {
        io_source.stack_size_ = 0;
    }

    /// 出力ストリームを破棄する。
    public: ~serializer()
    {
        this->fill_rest_containers();
    }

    /** @brief move代入演算子。
        @param[in,out] io_source 移動元インスタンス。
        @return *this
     */
    public: self& operator=(self&& io_source)
    {
        if (this != &io_source)
        {
            this->~self();
            return *new(this) self(std::move(io_source));
        }
        return *this;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePack値への直列化
    //@{
    /** @brief 空値をMessagePack形式で直列化し、ストリームへ出力する。
        @retval true  成功。
        @retval false 失敗。
     */
    public: bool write_nil()
    {
        if (this->get_stack_top_raw() != nullptr)
        {
            PSYQ_ASSERT(false);
        }
        else if (
            this->write_big_endian<std::uint8_t>(psyq::message_pack::header_NIL))
        {
            this->update_container_stack();
            return true;
        }
        return false;
    }

    /** @brief 真偽値をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_boolean 直列化する真偽値。
     */
    public: bool write_boolean(bool const in_boolean)
    {
        if (this->get_stack_top_raw() != nullptr)
        {
            PSYQ_ASSERT(false);
        }
        else if (
            this->write_big_endian<std::uint8_t>(
                in_boolean?
                    psyq::message_pack::header_TRUE:
                    psyq::message_pack::header_FALSE))
        {
            this->update_container_stack();
            return true;
        }
        return false;
    }

    /** @brief 無符号整数をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_integer 直列化する整数。
     */
    public: template<typename template_integer_type>
    bool write_unsigned_integer(template_integer_type const in_integer)
    {
        static_assert(
            std::is_unsigned<template_integer_type>::value,
            "template_integer_type is not unsigned integer type.");

        if (this->get_stack_top_raw() != nullptr)
        {
            PSYQ_ASSERT(false);
            return false;
        }
        else if (in_integer <= (std::numeric_limits<std::uint8_t>::max)())
        {
            if (psyq::message_pack::header_FIX_INT_MAX < in_integer
                && !this->write_big_endian<std::uint8_t>(psyq::message_pack::header_UINT8))
            {
                return false;
            }
            if (!this->write_big_endian(static_cast<std::uint8_t>(in_integer)))
            {
                return false;
            }
        }
        else if (in_integer <= (std::numeric_limits<std::uint16_t>::max)())
        {
            if (!this->write_big_endian<std::uint8_t>(psyq::message_pack::header_UINT16)
                || !this->write_big_endian(static_cast<std::uint16_t>(in_integer)))
            {
                return false;
            }
        }
        else if (in_integer <= (std::numeric_limits<std::uint32_t>::max)())
        {
            if (!this->write_big_endian<std::uint8_t>(psyq::message_pack::header_UINT32)
                || !this->write_big_endian(static_cast<std::uint32_t>(in_integer)))
            {
                return false;
            }
        }
        else
        {
            PSYQ_ASSERT(
                in_integer <= (std::numeric_limits<std::uint64_t>::max)());
            if (!this->write_big_endian<std::uint8_t>(psyq::message_pack::header_UINT64)
                || !this->write_big_endian(static_cast<std::uint64_t>(in_integer)))
            {
                return false;
            }
        }
        this->update_container_stack();
        return true;
    }

    /** @brief 有符号整数をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_integer 直列化する整数。
     */
    public: template<typename template_integer_type>
    bool write_signed_integer(template_integer_type const in_integer)
    {
        static_assert(
            std::is_integral<template_integer_type>::value
            && std::is_signed<template_integer_type>::value,
            "template_integer_type is not signed integer type.");
        if (0 <= in_integer)
        {
            // 0以上の整数を直列化する。
            typedef typename std::make_unsigned<template_integer_type>::type
                unsigned_integer;
            return this->write_unsigned_integer(
                static_cast<unsigned_integer>(in_integer));
        }
        if (this->get_stack_top_raw() != nullptr)
        {
            PSYQ_ASSERT(false);
            return false;
        }

        // 0未満の整数を直列化する。
        if ((std::numeric_limits<std::int8_t>::min)() <= in_integer)
        {
            if (in_integer < psyq::message_pack::header_FIX_INT_MIN
                && !this->write_big_endian<std::uint8_t>(psyq::message_pack::header_INT8))
            {
                return false;
            }
            if (!this->write_big_endian(static_cast<std::int8_t>(in_integer)))
            {
                return false;
            }
        }
        else if ((std::numeric_limits<std::int16_t>::min)() <= in_integer)
        {
            if (!this->write_big_endian<std::uint8_t>(psyq::message_pack::header_INT16)
                || !this->write_big_endian(static_cast<std::int16_t>(in_integer)))
            {
                return false;
            }
        }
        else if ((std::numeric_limits<std::int32_t>::min)() <= in_integer)
        {
            if (!this->write_big_endian<std::uint8_t>(psyq::message_pack::header_INT32)
                || !this->write_big_endian(static_cast<std::int32_t>(in_integer)))
            {
                return false;
            }
        }
        else
        {
            PSYQ_ASSERT(
                (std::numeric_limits<std::int64_t>::min)() <= in_integer);
            if (!this->write_big_endian<std::uint8_t>(psyq::message_pack::header_INT64)
                || !this->write_big_endian(static_cast<std::int64_t>(in_integer)))
            {
                return false;
            }
        }
        this->update_container_stack();
        return true;
    }

    /** @brief 浮動小数点数をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_float 直列化する浮動小数点数。
     */
    public: bool write_floating_point(float const in_float)
    {
        return this->write_floating_point<psyq::message_pack::header_FLOAT32>(in_float);
    }
    /// @copydoc self::write_floating_point(float const)
    public: bool write_floating_point(double const in_float)
    {
        return this->write_floating_point<psyq::message_pack::header_FLOAT64>(in_float);
    }
    //@}
    /// @copydoc self::write_floating_point(float const)
    private: template<std::uint8_t template_header, typename template_value>
    bool write_floating_point(template_value const in_float)
    {
        if (this->get_stack_top_raw() != nullptr)
        {
            PSYQ_ASSERT(false);
        }
        else if (
            this->write_big_endian<std::uint8_t>(template_header)
            && this->write_big_endian(in_float))
        {
            this->update_container_stack();
            return true;
        }
        return false;
    }

    /** @brief 数値を直列化し、ストリームへ出力する。
     */
    private: template<typename template_value>
    bool write_big_endian(template_value const in_value)
    {
        return psyq::message_pack::raw_stream<template_value>::write(
            this->stream_, in_value, psyq::message_pack::big_endian);
    }

    //-------------------------------------------------------------------------
    /// @name MessagePack文字列への直列化
    //@{
    /** @brief 連続するメモリ領域にある文字列を、
               MessagePack形式の文字列として直列化し、ストリームへ出力する。
        @param[in] in_begin 直列化する文字列の先頭位置。
        @param[in] in_size  直列化する文字列のバイト数。
     */
    public: template<typename template_char>
    bool write_raw_string(
        template_char const* const in_begin,
        std::size_t const in_size)
    {
        static_assert(
            // MessagePack文字列はUTF-8なので、文字は1バイト単位となる。
            sizeof(template_char) == 1, "MessagePack string is only UTF-8.");
        auto const local_size(in_size * sizeof(template_char));
        return this->write_string_header(local_size)
            && this->write_raw_data(in_begin, local_size);
    }

    /** @brief 標準コンテナをMessagePack形式の文字列として直列化し、
               ストリームへ出力する。
        @param[in] in_begin  直列化する標準コンテナの先頭位置。
        @param[in] in_length 直列化する標準コンテナの要素数。
     */
    public: template<typename template_iterator>
    bool write_container_string(
        template_iterator const& in_begin,
        std::size_t const in_length)
    {
        typedef typename std::iterator_traits<template_iterator>::value_type
            element;
        static_assert(
            // MessagePack文字列はUTF-8なので、文字は1バイト単位となる。
            sizeof(element) == 1, "MessagePack string is only UTF-8.");
        return this->make_serial_string<element>(in_length)
            && this->fill_serial_raw(in_begin, in_length, psyq::message_pack::big_endian) == 0;
    }
    /** @brief 標準コンテナをMessagePack形式の文字列として直列化し、
               ストリームへ出力する。
        @param[in] in_container 直列化する標準コンテナ。
     */
    public: template<typename template_container>
    bool write_container_string(template_container const& in_container)
    {
        return this->write_container_string(
            in_container.begin(), in_container.size());
    }

    /** @brief MessagePack文字列の直列化を開始する。

        以後、 in_size バイトの文字を、 self::fill_serial_raw() で直列化できる。

        @param[in] in_size 直列化するMessagePack文字列のバイト数。
        @sa self::fill_rest_elements()
     */
    public: bool make_serial_string(std::size_t const in_size)
    {
        if (in_size <= 0)
        {
            // 空の文字列を直列化する。
            if (this->write_big_endian<std::uint8_t>(psyq::message_pack::header_FIX_STR_MIN))
            {
                this->update_container_stack();
                return true;
            }
        }
        else if (self::stack_capacity <= this->get_rest_container_count())
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
        }
        else if (this->write_string_header(in_size))
        {
            // RAWバイト列をスタックに積む。
            auto& local_stack(
                this->stack_.at(this->get_rest_container_count()));
            local_stack.type = self::next_type_RAW_BYTES;
            local_stack.rest_size = in_size;
            ++this->stack_size_;
            return true;
        }
        return false;
    }
    //@}
    /** @brief 文字列のバイト数を書き込む。
        @param[in] in_size 文字列のバイト数。
     */
    private: bool write_string_header(std::size_t const in_size)
    {
        unsigned const local_fix_size(
            psyq::message_pack::header_FIX_STR_MAX
                - psyq::message_pack::header_FIX_STR_MIN);
        if (local_fix_size < in_size)
        {
            return this->write_raw_header<psyq::message_pack::header_STR8>(in_size);
        }
        else if (this->get_stack_top_raw() == nullptr)
        {
            return this->write_big_endian(
                static_cast<std::uint8_t>(
                    psyq::message_pack::header_FIX_STR_MIN + in_size));
        }
        else
        {
            PSYQ_ASSERT(false);
            return false;
        }
    }

    //-------------------------------------------------------------------------
    /// @name MessagePackバイナリへの直列化
    //@{
    /** @brief 連続するメモリ領域にある配列を、
               MessagePack形式のバイナリとして直列化し、ストリームへ出力する。
        @param[in] in_begin  直列化する配列の先頭位置。
        @param[in] in_length 直列化する配列の要素数。
     */
    public: template<typename template_element>
    bool write_raw_binary(
        template_element const* const in_begin,
        std::size_t const in_length)
    {
        auto const local_size(in_length * sizeof(template_element));
        return this->write_raw_header<psy::message_pack::header_BIN8>(local_size)
            && this->write_raw_data(in_begin, local_size);
    }

    /** @brief 標準コンテナをMessagePack形式のバイナリとして直列化し、
               ストリームへ出力する。
        @param[in] in_begin     直列化する標準コンテナの先頭位置。
        @param[in] in_length    直列化する標準コンテナの要素数。
        @param[in] in_endianess 要素を直列化する際のエンディアン性。
     */
    public: template<typename template_iterator>
    bool write_container_binary(
        template_iterator const& in_begin,
        std::size_t const in_length,
        psyq::message_pack::endianess const in_endianess
            = psyq::message_pack::big_endian)
    {
        typedef typename std::iterator_traits<template_iterator>::value_type
            element;
        return this->make_serial_binary<element>(in_length)
            && this->fill_serial_raw(in_begin, in_length, in_endianess) == 0;
    }
    /** @brief 標準コンテナをMessagePack形式のバイナリとして直列化し、
               ストリームへ出力する。
        @param[in] in_container 直列化する標準コンテナ。
        @param[in] in_endianess コンテナ要素を直列化する際のエンディアン性。
     */
    public: template<typename template_container>
    bool write_container_binary(
        template_container const& in_container,
        psyq::message_pack::endianess const in_endianess
            = psyq::message_pack::big_endian)
    {
        return this->write_container_binary(
            in_container.begin(), in_container.size(), in_endianess);
    }

    /** @brief MessagePackバイナリの直列化を開始する。

        以後 in_length 個の要素を、 self::fill_serial_raw() で直列化できる。

        @tparam template_element 直列化する標準コンテナの要素の型。
        @param[in] in_length 直列化する標準コンテナの要素数。
        @sa self::fill_rest_elements()
     */
    public: template<typename template_element>
    bool make_serial_binary(std::size_t const in_length)
    {
        auto const local_size(in_length * sizeof(template_element));
        if (local_size <= 0)
        {
            if (this->write_raw_header<psyq::message_pack::header_BIN8>(0))
            {
                this->update_container_stack();
                return true;
            }
        }
        else if (self::stack_capacity <= this->get_rest_container_count())
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
        }
        else if (this->write_raw_header<psyq::message_pack::header_BIN8>(local_size))
        {
            // RAWバイト列をスタックに積む。
            auto& local_stack(
                this->stack_.at(this->get_rest_container_count()));
            local_stack.type = self::next_type_RAW_BYTES;
            local_stack.rest_size = local_size;
            ++this->stack_size_;
            return true;
        }
        return false;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePack拡張バイナリへの直列化
    //@{
    /** @brief 値をMessagePack形式の拡張バイナリとして直列化し、
               ストリームへ出力する。
        @param[in] in_type      直列化する値の拡張型識別値。
        @param[in] in_value     直列化する値。
        @param[in] in_endianess 値を直列化する際のエンディアン性。
     */
    public: template<typename template_value>
    bool write_extended(
        std::int8_t const in_type,
        template_value const& in_value,
        psyq::message_pack::endianess const in_endianess
            = psyq::message_pack::big_endian)
    {
        return this->write_extended_header(in_type, sizeof(template_value))
            && psyq::message_pack::raw_stream<template_value>
                ::write(this->stream_, in_value, in_endianess);
    }
    /** @brief 長さが0の拡張バイナリを直列化し、ストリームへ出力する。
        @param[in] in_type 直列化する拡張型識別値。
     */
    public: bool write_extended(std::int8_t const in_type)
    {
        // 拡張バイナリの大きさと拡張型識別値を直列化する。
        if (this->write_extended_header(in_type, 0))
        {
            this->update_container_stack();
            return true;
        }
        return false;
    }

    /** @brief MessagePack拡張バイナリの直列化を開始する。

        以後 in_size バイトを、 self::fill_serial_raw() で直列化できる。

        @param[in] in_type 直列化する拡張型識別値。
        @param[in] in_size 直列化する拡張バイナリのバイト数。
        @sa self::fill_rest_elements()
     */
    public: bool make_serial_extended(
        std::int8_t const in_type,
        std::size_t const in_size)
    {
        if (in_size <= 0)
        {
            return this->write_extended(in_type);
        }
        else if (self::stack_capacity <= this->get_rest_container_count())
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
        }
        else if (this->write_extended_header(in_type, in_size))
        {
            // RAWバイト列をスタックに積む。
            auto& local_stack(
                this->stack_.at(this->get_rest_container_count()));
            local_stack.type = self::next_type_RAW_BYTES;
            local_stack.rest_size = in_size;
            ++this->stack_size_;
            return true;
        }
        return false;
    }
    //@}
    /** @brief 拡張バイナリのバイト数と型識別値を書き込む。
        @param[in] in_size 拡張バイナリのバイト数。
        @param[in] in_type 拡張型識別値。
     */
    private: bool write_extended_header(
        std::int8_t const in_type,
        std::size_t const in_size)
    {
        std::uint8_t local_header;
        bool local_write;
        switch (in_size)
        {
        case 1:
            local_header = psyq::message_pack::header_FIX_EXT1;
            goto PSYQ_MESSAGE_PACK_SERIALIZER_WRITE_EXTENDED_BINARY_SIZE;
        case 2:
            local_header = psyq::message_pack::header_FIX_EXT2;
            goto PSYQ_MESSAGE_PACK_SERIALIZER_WRITE_EXTENDED_BINARY_SIZE;
        case 4:
            local_header = psyq::message_pack::header_FIX_EXT4;
            goto PSYQ_MESSAGE_PACK_SERIALIZER_WRITE_EXTENDED_BINARY_SIZE;
        case 8:
            local_header = psyq::message_pack::header_FIX_EXT8;
            goto PSYQ_MESSAGE_PACK_SERIALIZER_WRITE_EXTENDED_BINARY_SIZE;
        case 16:
            local_header = psyq::message_pack::header_FIX_EXT16;
            goto PSYQ_MESSAGE_PACK_SERIALIZER_WRITE_EXTENDED_BINARY_SIZE;
        PSYQ_MESSAGE_PACK_SERIALIZER_WRITE_EXTENDED_BINARY_SIZE:
            if (this->get_stack_top_raw() != nullptr)
            {
                PSYQ_ASSERT(false);
                return false;
            }
            local_write = this->write_big_endian<std::uint8_t>(local_header);
            break;
        default:
            local_write = this->write_raw_header<psyq::message_pack::header_EXT8>(in_size);
            break;
        }

        // 拡張型識別値を直列化する。
        return local_write? this->write_big_endian(in_type): false;
    }

    //-------------------------------------------------------------------------
    /// @name MessagePack配列への直列化
    //@{
    /** @brief タプルをMessagePack形式の配列として直列化し、
               ストリームへ出力する。
        @tparam template_tuple std::tuple 互換のタプル型。
        @param[in] in_tuple 直列化するタプル。
     */
    public: template<typename template_tuple>
    bool write_tuple(template_tuple const& in_tuple)
    {
        this->make_serial_array(std::tuple_size<template_tuple>::value);
        return psyq::internal::message_pack_tuple_serializer<
            std::tuple_size<template_tuple>::value>
                ::write(*this, in_tuple);
    }

    /** @brief 標準コンテナをMessagePack形式の配列として直列化し、
               ストリームへ出力する。
        @param[in] in_iterator 直列化する標準コンテナの先頭位置。
        @param[in] in_length   直列化する標準コンテナの要素数。
     */
    public: template<typename template_iterator>
    bool write_array(template_iterator in_iterator, std::size_t in_length)
    {
        if (!this->make_serial_array(in_length))
        {
            return false;
        }
        for (; 0 < in_length; --in_length, ++in_iterator)
        {
            *this << *in_iterator;
        }
        return this->stream_.good();
    }

    /** @brief 標準コンテナをMessagePack形式の配列として直列化し、
               ストリームへ出力する。
        @param[in] in_container 直列化する標準コンテナ。
     */
    public: template<typename template_container>
    bool write_array(template_container const& in_container)
    {
        return this->write_array(in_container.begin(), in_container.size());
    }

    /** @brief MessagePack形式の配列の直列化を開始する。

        以後 in_length 個のMessagePack値を、配列の要素として直列化できる。

        以下の関数を1回呼び出す毎に、MessagePack値を1つ直列化する。
        - self::write_nil()
        - self::write_boolean()
        - self::write_unsigned_integer()
        - self::write_signed_integer()
        - self::write_floating_point()
        - self::write_raw_string()
        - self::write_raw_binary()
        - self::write_container_string()
        - self::write_container_binary()
        - self::write_extended()
        - self::write_tuple()
        - self::write_array()
        - self::write_set()
        - self::write_map()
        - self::make_serial_string()
        - self::make_serial_binary()
        - self::make_serial_extended()
        - self::make_serial_array()
        - self::make_serial_map()

        @param[in] in_length 直列化する配列の要素数。
        @sa self::fill_rest_elements()
     */
    public: bool make_serial_array(std::size_t const in_length)
    {
        return this->make_serial_container<
            self::next_type_ARRAY_ELEMENT,
            psyq::message_pack::header_ARRAY16,
            psyq::message_pack::header_FIX_ARRAY_MIN,
            psyq::message_pack::header_FIX_ARRAY_MAX>(
                in_length);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePack連想配列への直列化
    //@{
    /** @brief 標準コンテナをMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @param[in] in_iterator 直列化する標準コンテナの先頭位置。
        @param[in] in_length   直列化する標準コンテナの要素数。
     */
    public: template<typename template_iterator>
    bool write_set(template_iterator in_iterator, std::size_t in_length)
    {
        // マップ値が空の連想配列として標準コンテナを直列化する。
        if (!this->make_serial_map(in_length))
        {
            return false;
        }
        for (; 0 < in_length; --in_length, ++in_iterator)
        {
            *this << *in_iterator;
            this->write_nil();
        }
        return this->stream_.good();
    }
    /** @brief 標準コンテナをMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @param[in] in_set 直列化する標準コンテナ。
     */
    public: template<typename template_set>
    bool write_set(template_set const& in_set)
    {
        return this->write_set(in_set.begin(), in_set.size());
    }

    /** @brief pairコンテナをMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @tparam template_iterator std::pair 互換の要素を指す反復子の型。
        @param[in] in_iterator 直列化するpairコンテナの先頭位置。
        @param[in] in_length   直列化するpairコンテナの要素数。
     */
    public: template<typename template_iterator>
    bool write_map(template_iterator in_iterator, std::size_t in_length)
    {
        if (!this->make_serial_map(in_length))
        {
            return false;
        }
        for (; 0 < in_length; --in_length, ++in_iterator)
        {
            auto& local_value(*in_iterator);
            *this << local_value.first << local_value.second;
        }
        return this->stream_.good();
    }
    /** @brief pairコンテナをMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @tparam template_map std::pair 互換の要素を持つ標準コンテナの型。
        @param[in] in_map 直列化するpairコンテナ。
     */
    public: template<typename template_map>
    bool write_map(template_map const& in_map)
    {
        return this->write_map(in_map.begin(), in_map.size());
    }

    /** @brief MessagePack形式の連想配列の直列化を開始する。

        以後 in_length*2 個のMessagePack値を、連想配列の要素として直列化できる。
        2つのMessagePack値で、連想配列の要素1つとなる。
        最初のMessagePack値を連想配列のキー、
        次のMessagePack値を連想配列のマップ値として直列化する。

        以下の関数を1回呼び出す毎に、MessagePack値を1つ直列化する。
        - self::write_nil()
        - self::write_boolean()
        - self::write_unsigned_integer()
        - self::write_signed_integer()
        - self::write_floating_point()
        - self::write_raw_string()
        - self::write_raw_binary()
        - self::write_container_string()
        - self::write_container_binary()
        - self::write_extended()
        - self::write_tuple()
        - self::write_array()
        - self::write_set()
        - self::write_map()
        - self::make_serial_string()
        - self::make_serial_binary()
        - self::make_serial_extended()
        - self::make_serial_array()
        - self::make_serial_map()

        @param[in] in_length 直列化する連想配列の要素数。
        @sa self::fill_rest_elements()
     */
    public: bool make_serial_map(std::size_t const in_length)
    {
        return this->make_serial_container<
            self::next_type_MAP_KEY,
            psyq::message_pack::header_MAP16,
            psyq::message_pack::header_FIX_MAP_MIN,
            psyq::message_pack::header_FIX_MAP_MAX>(
                in_length);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackコンテナ要素への直列化
    //@{
    /** @brief 直前に直列化を開始した文字列／バイナリ／拡張バイナリの残り要素に
               値をMessagePack形式のRAWバイト列として直列化し、
               ストリームへ出力する。
        @param[in] in_value     直列化する値。
        @param[in] in_endianess コンテナ要素を直列化する際のエンディアン性。
        @sa self::make_serial_string()
        @sa self::make_serial_binary()
        @sa self::make_serial_extended()
     */
    public: template<typename template_value>
    std::size_t fill_serial_raw(
        template_value const& in_value,
        psyq::message_pack::endianess const in_endianess)
    {
        return this->fill_serial_raw(&in_value, 1, in_endianess);
    }

    /** @brief 直前に直列化を開始した文字列／バイナリ／拡張バイナリの残り要素に
               標準コンテナの要素をMessagePack形式のRAWバイト列として直列化し、
               ストリームへ出力する。
        @param[in] in_iterator  直列化する標準コンテナの先頭位置。
        @param[in] in_length    直列化する標準コンテナの要素数。
        @param[in] in_endianess コンテナ要素を直列化する際のエンディアン性。
        @sa self::make_serial_string()
        @sa self::make_serial_binary()
        @sa self::make_serial_extended()
     */
    public: template<typename template_iterator>
    std::size_t fill_serial_raw(
        template_iterator in_iterator,
        std::size_t in_length,
        psyq::message_pack::endianess const in_endianess)
    {
        auto const local_stack(this->get_stack_top_raw());
        if (local_stack == nullptr)
        {
            PSYQ_ASSERT(false);
            return 0;
        }
        typedef typename std::iterator_traits<template_iterator>::value_type
            element;
        PSYQ_ASSERT(sizeof(element) * in_length <= local_stack->rest_size);
        for (;; --in_length, ++in_iterator)
        {
            if (local_stack->rest_size < sizeof(element))
            {
                PSYQ_ASSERT(local_stack->rest_size == 0);
                return this->fill_rest_elements();
            }
            else if (
                in_length <= 0
                || !psyq::message_pack::raw_stream<element>::write(
                    this->stream_, *in_iterator, in_endianess))
            {
                return local_stack->rest_size;
            }
            local_stack->rest_size -= sizeof(element);
        }
    }

    /** @brief 直前に直列化を開始したMessagePackコンテナの残り要素を、
               空値で埋める。
        @sa self::make_serial_string()
        @sa self::make_serial_binary()
        @sa self::make_serial_extended()
        @sa self::make_serial_array()
        @sa self::make_serial_map()
        @sa self::fill_rest_containers()
     */
    public: std::size_t fill_rest_elements()
    {
        if (this->get_rest_container_count() <= 0)
        {
            return 0;
        }

        // 残りの要素数を決定する。
        auto& local_stack(
            this->stack_.at(this->get_rest_container_count() - 1));
        std::size_t local_empty_count;
        std::uint8_t local_empty_value;
        switch (local_stack.type)
        {
        case self::next_type_RAW_BYTES:
            local_empty_value = 0;
            local_empty_count = local_stack.rest_size;
            break;

        case self::next_type_ARRAY_ELEMENT:
            local_empty_value = psyq::message_pack::header_NIL;
            local_empty_count = local_stack.rest_size;
            break;

        case self::next_type_MAP_KEY:
            local_empty_value = psyq::message_pack::header_NIL;
            local_empty_count = local_stack.rest_size * 2;
            break;

        case self::next_type_MAP_VALUE:
            local_empty_value = psyq::message_pack::header_NIL;
            local_empty_count = local_stack.rest_size * 2 + 1;
            break;

        default:
            PSYQ_ASSERT(false);
            return local_stack.rest_size;
        }

        // コンテナの残り要素を埋める。
        for (; 0 < local_empty_count; --local_empty_count)
        {
            if (!this->write_big_endian<std::uint8_t>(local_empty_value))
            {
                return local_empty_count;
            }
        }
        --this->stack_size_;
        this->update_container_stack();
        return 0;
    }

    /** @brief 直列化を終了してないすべてのMessagePackコンテナの残り要素を、
               空値で埋める。
        @sa self::make_serial_string()
        @sa self::make_serial_binary()
        @sa self::make_serial_extended()
        @sa self::make_serial_array()
        @sa self::make_serial_map()
        @sa self::fill_rest_elements()
     */
    public: std::size_t fill_rest_containers()
    {
        while (
            0 < this->get_rest_container_count()
            && this->fill_rest_elements() <= 0)
        {}
        return this->get_rest_container_count();
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief RAWバイト列のバイト数を書き込む。
        @tparam template_header 書き込むヘッダーの基準値。
        @param[in] in_size RAWバイト列のバイト数。
     */
    private: template<std::uint8_t template_header>
    bool write_raw_header(std::size_t const in_size)
    {
        if (this->get_stack_top_raw() != nullptr)
        {
            PSYQ_ASSERT(false);
        }
        else if (in_size <= (std::numeric_limits<std::uint8_t>::max)())
        {
            return this->write_big_endian<std::uint8_t>(template_header)
                && this->write_big_endian(static_cast<std::uint8_t>(in_size));
        }
        else if (in_size <= (std::numeric_limits<std::uint16_t>::max)())
        {
            return this->write_big_endian<std::uint8_t>(template_header + 1)
                && this->write_big_endian(static_cast<std::uint16_t>(in_size));
        }
        else if (in_size <= (std::numeric_limits<std::uint32_t>::max)())
        {
            return this->write_big_endian<std::uint8_t>(template_header + 2)
                && this->write_big_endian(static_cast<std::uint32_t>(in_size));
        }
        else
        {
            PSYQ_ASSERT(false);
        }
        return false;
    }

    /** @brief RAWバイト列を書き込む。
        @param[in] in_begin RAWバイト列の先頭位置。
        @param[in] in_size  RAWバイト列のバイト数。
     */
    private: bool write_raw_data(
        void const* const in_begin,
        std::size_t const in_size)
    {
        if (0 < in_size)
        {
            if (in_begin == nullptr)
            {
                PSYQ_ASSERT(false);
                return false;
            }
            this->stream_.write(
                static_cast<typename self::stream::char_type const*>(in_begin),
                in_size);
            if (!this->stream_.good())
            {
                PSYQ_ASSERT(false);
                return false;
            }
        }
        this->update_container_stack();
        return true;
    }

    /** @brief 最上段スタックがRAWバイト列なら、それを取得する。
     */
    private: typename self::stack* get_stack_top_raw()
    {
        if (0 < this->get_rest_container_count())
        {
            auto& local_stack(
                this->stack_.at(this->get_rest_container_count() - 1));
            if (local_stack.type == self::next_type_RAW_BYTES)
            {
                return &local_stack;
            }
        }
        return nullptr;
    }

    /** @brief コンテナの直列化を開始する。
        @tparam template_next_type      スタックの種別。
        @tparam template_header_begin   コンテナのヘッダ。
        @tparam template_header_fix_min 固定長ヘッダの最小値。
        @tparam template_header_fix_max 固定長ヘッダの最大値。
        @param[in] in_length 直列化するコンテナの要素数。
     */
    private: template<
        typename self::next_type template_next_type,
        std::uint8_t template_header_begin,
        std::uint8_t template_header_fix_min,
        std::uint8_t template_header_fix_max>
    bool make_serial_container(std::size_t const in_length)
    {
        static_assert(template_header_fix_min <= template_header_fix_max, "");
        if (in_length <= 0)
        {
            // 空コンテナを直列化する。
            if (this->write_big_endian<std::uint8_t>(template_header_fix_min))
            {
                this->update_container_stack();
                return true;
            }
            return false;
        }
        else if (self::stack_capacity <= this->get_rest_container_count())
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
            return false;
        }
        else if (in_length <= template_header_fix_max - template_header_fix_min)
        {
            // 固定長コンテナのヘッダを直列化する。
            std::uint8_t const local_header(
                template_header_fix_min + in_length);
            if (!this->write_big_endian(local_header))
            {
                return false;
            }
        }
        else if (in_length <= (std::numeric_limits<std::uint16_t>::max)())
        {
            // 16bit長コンテナのヘッダと要素数を直列化する。
            std::uint8_t const local_header(template_header_begin);
            std::uint16_t const local_length(in_length);
            if (!this->write_big_endian(local_header) ||
                !this->write_big_endian(local_length))
            {
                return false;
            }
        }
        else if (in_length <= (std::numeric_limits<std::uint32_t>::max)())
        {
            // 32bit長コンテナのヘッダと要素数を直列化する。
            std::uint8_t const local_header(template_header_begin + 1);
            std::uint32_t const local_length(in_length);
            if (!this->write_big_endian(local_header) ||
                !this->write_big_endian(local_length))
            {
                return false;
            }
        }
        else
        {
            // コンテナの限界要素数を超えたので失敗。
            PSYQ_ASSERT(false);
            return false;
        }

        // コンテナをスタックに積む。
        auto& local_stack(this->stack_.at(this->get_rest_container_count()));
        local_stack.type = template_next_type;
        local_stack.rest_size = in_length;
        ++this->stack_size_;
        return true;
    }

    /** @brief コンテナスタックを更新する。
     */
    private: void update_container_stack()
    {
        if (this->get_rest_container_count() <= 0)
        {
            return;
        }

        auto& local_stack(
            this->stack_.at(this->get_rest_container_count() - 1));
        switch (local_stack.type)
        {
        case self::next_type_ARRAY_ELEMENT:
            if (1 < local_stack.rest_size)
            {
                --local_stack.rest_size;
            }
            else
            {
                --this->stack_size_;
                this->update_container_stack();
            }
            break;

        case self::next_type_MAP_KEY:
            local_stack.type = self::next_type_MAP_VALUE;
            break;

        case self::next_type_MAP_VALUE:
            if (1 < local_stack.rest_size)
            {
                local_stack.type = self::next_type_MAP_KEY;
                --local_stack.rest_size;
            }
            else
            {
                --this->stack_size_;
                this->update_container_stack();
            }
            break;

        case self::next_type_RAW_BYTES:
        default:
            PSYQ_ASSERT(false);
            break;
        }
    }

    //-------------------------------------------------------------------------
    /// @name 状態の取得
    //@{
    public: typename self::next_type get_next_type() const PSYQ_NOEXCEPT
    {
        return 0 < this->get_rest_container_count()?
            this->stack_.at(this->get_rest_container_count() - 1).type:
            typename self::next_type_VALUE;
    }

    /** @brief 直前に直列化を開始したMessagePackコンテナの残り要素数を取得する。
        @return
            直前に直列化を開始したMessagePackコンテナが…
            - 文字列／バイナリ／拡張バイナリなら、残りバイト数。
            - 配列／連想配列なら、残り要素数。
        @sa self::make_serial_string()
        @sa self::make_serial_binary()
        @sa self::make_serial_extended()
        @sa self::make_serial_array()
        @sa self::make_serial_map()
     */
    public: std::size_t get_rest_element_count() const PSYQ_NOEXCEPT
    {
        return 0 < this->get_rest_container_count()?
            this->stack_.at(this->get_rest_container_count() - 1).rest_size: 0;
    }

    /** @brief 直列化を終了してないMessagePackコンテナの数を取得する。
        @return 直列化を終了してないMessagePackコンテナの数。
        @sa self::make_serial_string()
        @sa self::make_serial_binary()
        @sa self::make_serial_extended()
        @sa self::make_serial_array()
        @sa self::make_serial_map()
     */
    public: std::size_t get_rest_container_count() const PSYQ_NOEXCEPT
    {
        return this->stack_size_;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name ストリームの操作
    //@{
    public: typename self::stream const& get_stream() const PSYQ_NOEXCEPT
    {
        return this->stream_;
    }

    private: bool swap_stream(typename self::stream& io_stream)
    {
        if (this->fill_rest_containers() != 0)
        {
            return false;
        }
        this->stream_.swap(io_stream);
        return true;
    }

    public: bool good() const
    {
        return this->stream_.good();
    }

    public: typename self::stream::pos_type tellp()
    {
        return this->stream_.tellp();
    }

    public: void flush()
    {
        this->stream_.flush();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @copydoc self::stream
    private: typename self::stream stream_;
    /// 直列化途中のコンテナのスタック。
    private: std::array<typename self::stack, template_stack_capacity> stack_;
    /// 直列化途中のコンテナのスタック数。
    private: std::size_t stack_size_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 値を直列化／直列化復元する。
    @tparam template_value @copydoc psyq::message_pack::raw_stream::value_type
 */
template<typename template_value>
struct psyq::message_pack::raw_stream
{
    /** @brief 直列化／直列化復元する値の型。

        この実装では、整数型か浮動小数点数型にのみに対応している。
        これらの型以外に対応するには、テンプレートの特殊化をした
        psyq::message_pack::raw_stream を実装し、
        staticメンバ関数として read() と write() を実装すること。
     */
    typedef template_value value_type;
    static_assert(
        std::is_integral<template_value>::value
        || std::is_floating_point<template_value>::value,
        "template_value is not integer or floating point type.");

    /** @brief 値を直列化し、ストリームへ出力する。

        in_endianess とnative-endianが一致するなら先頭から末尾へ、
        異なるなら末尾から先頭の順に、値のバイト列をストリームへ出力する。

        @param[out] out_stream   出力先ストリーム。
        @param[in]  in_value     直列化する値。
        @param[in]  in_endianess 値を直列化する際のエンディアン性。
     */
    template<typename template_stream>
    static bool write(
        template_stream& out_stream,
        template_value const& in_value,
        psyq::message_pack::endianess const in_endianess)
    {
        PSYQ_ASSERT(in_endianess == 0 || in_endianess == 1);
        static const union {int integer; std::uint8_t endianess;}
            static_native = {1};
        int const local_mask((static_native.endianess ^ in_endianess) - 1);
        int const local_step(1 - (2 & local_mask));
        auto const local_begin(
            reinterpret_cast<std::uint8_t const*>(&in_value)
                + ((sizeof(template_value) - 1) & local_mask));
        auto const local_end(local_begin + sizeof(template_value) * local_step);
        for (auto i(local_begin); i != local_end; i += local_step)
        {
            out_stream.put(static_cast<typename template_stream::char_type>(*i));
        }
        return out_stream.good();
    }

    /** @brief バイト列から値を直列化復元する。
        @note 未実装。
     */
    private: static bool read();
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//----------------------------------------------------------------------------
/// @name 真偽値の直列化
//@{
/** @brief 真偽値をMessagePack形式で直列化し、ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_boolean 直列化する真偽値。
 */
template<typename template_out_stream, std::size_t template_stack_capacity>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    bool const in_boolean)
{
    out_stream.write_boolean(in_boolean);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 無符号整数の直列化
//@{
/** @brief 無符号整数をMessagePack形式で直列化し、ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_integer 直列化する無符号整数。
 */
template<typename template_out_stream, std::size_t template_stack_capacity>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    unsigned char const in_integer)
{
    out_stream.write_unsigned_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, unsigned char const)
template<typename template_out_stream, std::size_t template_stack_capacity>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    unsigned short const in_integer)
{
    out_stream.write_unsigned_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, unsigned char const)
template<typename template_out_stream, std::size_t template_stack_capacity>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    unsigned int const in_integer)
{
    out_stream.write_unsigned_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, unsigned char const)
template<typename template_out_stream, std::size_t template_stack_capacity>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    unsigned long const in_integer)
{
    out_stream.write_unsigned_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, unsigned char const)
template<typename template_out_stream, std::size_t template_stack_capacity>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    unsigned long long const in_integer)
{
    out_stream.write_unsigned_integer(in_integer);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 有符号整数の直列化
//@{
/** @brief 有符号整数をMessagePack形式で直列化し、ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_integer 直列化する有符号整数。
 */
template<typename template_out_stream, std::size_t template_stack_capacity>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    char const in_integer)
{
    out_stream.write_signed_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, char const)
template<typename template_out_stream, std::size_t template_stack_capacity>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    short const in_integer)
{
    out_stream.write_signed_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, char const)
template<typename template_out_stream, std::size_t template_stack_capacity>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    int const in_integer)
{
    out_stream.write_signed_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, char const)
template<typename template_out_stream, std::size_t template_stack_capacity>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    long const in_integer)
{
    out_stream.write_signed_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, char const)
template<typename template_out_stream, std::size_t template_stack_capacity>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    long long const in_integer)
{
    out_stream.write_signed_integer(in_integer);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 浮動小数点数の直列化
//@{
/** @brief 浮動小数点数をMessagePack形式で直列化し、ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_float   直列化する浮動小数点数。
 */
template<typename template_out_stream, std::size_t template_stack_capacity>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    float const in_float)
{
    out_stream.write_floating_point(in_float);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, float const)
template<typename template_out_stream, std::size_t template_stack_capacity>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    double const in_float)
{
    out_stream.write_floating_point(in_float);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 文字列の直列化
//@{
/** @brief 文字列をMessagePack形式のRAWバイト列として直列化し、
           ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_string  直列化する文字列。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_char,
    typename template_traits,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::basic_string<template_char, template_traits, template_allocator>
        const& in_string)
{
    static_assert(
        sizeof(template_char) == 1, "sizeof(template_char) is not 1.");
    out_stream.write_raw_string(in_string.data(), in_string.length());
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name タプルの直列化
//@{
#if 0
/** @brief std::pair をMessagePack形式で直列化し、ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_pair    直列化するペアタプル。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_first,
    typename template_second>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::pair<template_first, template_second> const& in_pair)
{
    return out_stream << in_pair.first << in_pair.second;
}
#endif // 0

/** @brief std::tuple をMessagePack形式の配列として直列化し、
           ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_tuple   直列化するタプル。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_element0>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::tuple<template_element0> const& in_tuple)
{
    out_stream.write_tuple(in_tuple);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, std::tuple<template_element0> const&)
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_element0,
    typename template_element1>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::tuple<template_element0, template_element1> const& in_tuple)
{
    out_stream.write_tuple(in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, std::tuple<template_element0> const&)
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_element0,
    typename template_element1,
    typename template_element2>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::tuple<template_element0, template_element1, template_element2> const&
        in_tuple)
{
    out_stream.write_tuple(in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, std::tuple<template_element0> const&)
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_element0,
    typename template_element1,
    typename template_element2,
    typename template_element3>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::tuple<
        template_element0,
        template_element1,
        template_element2,
        template_element3>
            const& in_tuple)
{
    out_stream.write_tuple(in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, std::tuple<template_element0> const&)
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_element0,
    typename template_element1,
    typename template_element2,
    typename template_element3,
    typename template_element4>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::tuple<
        template_element0,
        template_element1,
        template_element2,
        template_element3,
        template_element4>
            const& in_tuple)
{
    out_stream.write_tuple(in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, std::tuple<template_element0> const&)
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_element0,
    typename template_element1,
    typename template_element2,
    typename template_element3,
    typename template_element4,
    typename template_element5>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::tuple<
        template_element0,
        template_element1,
        template_element2,
        template_element3,
        template_element4,
        template_element5>
            const& in_tuple)
{
    out_stream.write_tuple(in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, std::tuple<template_element0> const&)
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_element0,
    typename template_element1,
    typename template_element2,
    typename template_element3,
    typename template_element4,
    typename template_element5,
    typename template_element6>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::tuple<
        template_element0,
        template_element1,
        template_element2,
        template_element3,
        template_element4,
        template_element5,
        template_element6>
            const& in_tuple)
{
    out_stream.write_tuple(in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, std::tuple<template_element0> const&)
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_element0,
    typename template_element1,
    typename template_element2,
    typename template_element3,
    typename template_element4,
    typename template_element5,
    typename template_element6,
    typename template_element7>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::tuple<
        template_element0,
        template_element1,
        template_element2,
        template_element3,
        template_element4,
        template_element5,
        template_element6,
        template_element7>
            const& in_tuple)
{
    out_stream.write_tuple(in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, std::tuple<template_element0> const&)
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_element0,
    typename template_element1,
    typename template_element2,
    typename template_element3,
    typename template_element4,
    typename template_element5,
    typename template_element6,
    typename template_element7,
    typename template_element8>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::tuple<
        template_element0,
        template_element1,
        template_element2,
        template_element3,
        template_element4,
        template_element5,
        template_element6,
        template_element7,
        template_element8>
            const& in_tuple)
{
    out_stream.write_tuple(in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&, std::tuple<template_element0> const&)
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_element0,
    typename template_element1,
    typename template_element2,
    typename template_element3,
    typename template_element4,
    typename template_element5,
    typename template_element6,
    typename template_element7,
    typename template_element8,
    typename template_element9>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::tuple<
        template_element0,
        template_element1,
        template_element2,
        template_element3,
        template_element4,
        template_element5,
        template_element6,
        template_element7,
        template_element8,
        template_element9>
            const& in_tuple)
{
    out_stream.write_tuple(in_tuple);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name コンテナの直列化
//@{
/** @brief std::array をMessagePack形式の配列として直列化し、
           ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_array   直列化するコンテナ。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_value,
    std::size_t template_size>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::array<template_value, template_size> const& in_array)
{
    out_stream.write_array(in_array);
    return out_stream;
}

/** @brief std::vector をMessagePack形式の配列として直列化し、
           ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_vector  直列化するコンテナ。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_value,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::vector<template_value, template_allocator> const& in_vector)
{
    out_stream.write_array(in_vector);
    return out_stream;
}

/** @brief std::deque をMessagePack形式の配列として直列化し、
           ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_deque   直列化するコンテナ。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_value,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::deque<template_value, template_allocator> const& in_deque)
{
    out_stream.write_array(in_deque);
    return out_stream;
}

/** @brief std::list をMessagePack形式の配列として直列化し、
           ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_list    直列化するコンテナ。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_value,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::list<template_value, template_allocator> const& in_list)
{
    out_stream.write_array(in_list);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 集合コンテナの直列化
//@{
/** @brief std::set をMessagePack形式の連想配列として直列化し、
           ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_set     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_key,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::set<template_key, template_compare, template_allocator>
        const& in_set)
{
    out_stream.write_set(in_set);
    return out_stream;
}

/** @brief std::multiset をMessagePack形式の連想配列として直列化し、
           ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_set     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_key,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::multiset<template_key, template_compare, template_allocator>
        const& in_set)
{
    out_stream.write_set(in_set);
    return out_stream;
}

/** @brief std::unordered_set をMessagePack形式の連想配列として直列化し、
           ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_set     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_key,
    typename template_hash,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::unordered_set<template_key, template_hash, template_compare, template_allocator>
        const& in_set)
{
    out_stream.write_set(in_set);
    return out_stream;
}

/** @brief std::unordered_multiset をMessagePack形式の連想配列として直列化し、
           ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_set     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_key,
    typename template_hash,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::unordered_multiset<template_key, template_hash, template_compare, template_allocator>
        const& in_set)
{
    out_stream.write_set(in_set);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 連想配列コンテナの直列化
//@{
/** @brief std::map をMessagePack形式の連想配列として直列化し、
           ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_map     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_key,
    typename template_mapped,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::map<template_key, template_mapped, template_compare, template_allocator>
        const& in_map)
{
    out_stream.write_map(in_map);
    return out_stream;
}

/** @brief std::multimap をMessagePack形式の連想配列として直列化し、
           ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_map     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_key,
    typename template_mapped,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::multimap<template_key, template_mapped, template_compare, template_allocator>
        const& in_map)
{
    out_stream.write_map(in_map);
    return out_stream;
}

/** @brief std::unordered_map をMessagePack形式の連想配列として直列化し、
           ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_map     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_key,
    typename template_mapped,
    typename template_hash,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::unordered_map<
        template_key,
        template_mapped,
        template_hash,
        template_compare,
        template_allocator>
            const& in_map)
{
    out_stream.write_map(in_map);
    return out_stream;
}

/** @brief std::unordered_multimap をMessagePack形式の連想配列として直列化し、
           ストリームへ出力する。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_map     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    std::size_t template_stack_capacity,
    typename template_key,
    typename template_mapped,
    typename template_hash,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
operator<<(
    psyq::message_pack::serializer<template_out_stream, template_stack_capacity>&
        out_stream,
    std::unordered_multimap<
        template_key,
        template_mapped,
        template_hash,
        template_compare,
        template_allocator>
            const& in_map)
{
    out_stream.write_map(in_map);
    return out_stream;
}
//@}

#endif // !defined(PSYQ_MESSAGE_PACK_SERIALIZER_HPP_)
