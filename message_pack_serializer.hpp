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
        /// @endcond

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
            header_NIL           = 0xc0, ///< nil値。
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
        /// バイト列として使う型。
        template<std::size_t> struct message_pack_bytes;
        /// 1バイトのバイト列として使う型。
        template<> struct message_pack_bytes<1> {typedef std::uint8_t  type;};
        /// 2バイトのバイト列として使う型。
        template<> struct message_pack_bytes<2> {typedef std::uint16_t type;};
        /// 4バイトのバイト列として使う型。
        template<> struct message_pack_bytes<4> {typedef std::uint32_t type;};
        /// 8バイトのバイト列として使う型。
        template<> struct message_pack_bytes<8> {typedef std::uint64_t type;};

        /** @brief バイト列をストリームへ出力する。
            @tparam template_size バイト列のバイト数。
         */
        template<std::size_t template_size>
        struct message_pack_bytes_serializer
        {
            static_assert(0 < template_size, "");
            static_assert(
                static_cast<int>(true) == 1 && static_cast<int>(false) == 0,
                "");

            /** @brief バイト列をストリームへ出力する。
                @param[out] out_stream 出力先ストリーム。
                @param[in]  in_bytes  出力するバイト列。
                @param[in]  in_big_endian
                    trueならbig-endianで、falseならlittle-endianで出力する。
             */
            template<typename template_stream, typename template_bytes>
            static bool write(
                template_stream& out_stream,
                template_bytes const in_bytes,
                bool const in_big_endian)
            {
                static_assert(sizeof(template_bytes) == template_size, "");
                static_assert(std::is_integral<template_bytes>::value, "");

                auto const local_1st_shift(
                    (8 * template_size / 2) * in_big_endian);
                auto const local_2nd_shift(
                    (8 * template_size / 2) - local_1st_shift);

                typedef psyq::internal
                    ::message_pack_bytes_serializer<template_size / 2>
                        half_serializer;
                typedef typename psyq::internal
                    ::message_pack_bytes<template_size / 2>::type
                        half_bytes;
                return half_serializer::write(
                        out_stream,
                        static_cast<half_bytes>(in_bytes >> local_1st_shift),
                        in_big_endian)
                    && half_serializer::write(
                        out_stream,
                        static_cast<half_bytes>(in_bytes >> local_2nd_shift),
                        in_big_endian);
            }
        };
        /// 1バイトのバイト列をストリームへ出力する。
        template<> struct message_pack_bytes_serializer<1>
        {
            template<typename template_stream>
            static bool write(
                template_stream& out_stream,
                std::uint8_t const in_bytes,
                bool const)
            {
                out_stream.put(
                    static_cast<typename template_stream::char_type>(in_bytes));
                auto const local_good(out_stream.good());
                PSYQ_ASSERT(local_good);
                return local_good;
            }
        };

        //---------------------------------------------------------------------
        /// static_castで値をバイト列に変換し、ストリームへ出力する。
        template<bool> struct message_pack_cast_value_serializer
        {
            /** @brief static_castで値をバイト列に変換し、ストリームへ出力する。
                @param[out] out_stream 出力先ストリーム。
                @param[in]  in_value   バイト列に変換する値。
                @param[in]  in_big_endian
                    trueならbig-endianで、falseならlittle-endianで出力する。
             */
            template<typename template_stream, typename template_value>
            static bool write(
                template_stream& out_stream,
                template_value const in_value,
                bool const in_big_endian)
            {
                typedef typename psyq::internal
                    ::message_pack_bytes<sizeof(template_value)>::type
                        bytes;
                typedef typename psyq::internal
                    ::message_pack_bytes_serializer<sizeof(template_value)>
                        bytes_serializer;

                // 値をバイト列にキャストして直列化する。
                return bytes_serializer::write(
                    out_stream, static_cast<bytes>(in_value), in_big_endian);
            }
        };
        /// unionで値をバイト列に変換し、ストリームへ出力する。
        template<> struct message_pack_cast_value_serializer<false>
        {
            /** @brief unionで値をバイト列に変換し、ストリームへ出力する。
                @param[out] out_stream 出力先ストリーム。
                @param[in]  in_value   バイト列に変換する値。
                @param[in]  in_big_endian
                    trueならbig-endianで、falseならlittle-endianで出力する。
             */
            template<typename template_stream, typename template_value>
            static bool write(
                template_stream& out_stream,
                template_value in_value,
                bool const in_big_endian)
            {
                typedef typename psyq::internal
                    ::message_pack_bytes<sizeof(template_value)>::type
                        bytes_type;
                typedef typename psyq::internal
                    ::message_pack_bytes_serializer<sizeof(template_value)>
                        bytes_serializer;

                // 値をバイト列として扱えないので、
                // 同じ大きさのunionを経由して直列化する。
                union {template_value value; bytes_type bytes;} local_union;
                local_union.value = std::move(in_value);
                return bytes_serializer::write(
                    out_stream, local_union.bytes, in_big_endian);
            }
        };
        /// 値を直列化し、ストリームへ出力する。
        template<typename template_value>
        struct message_pack_value_serializer
        {
            typedef template_value value_type;

            /** @brief 値を直列化し、ストリームへ出力する。
                @param[out] out_stream 出力先ストリーム。
                @param[in]  in_value   直列化する値。
                @param[in]  in_big_endian
                    trueならbig-endianで、falseならlittle-endianで出力する。
             */
            template<typename template_stream>
            static bool write(
                template_stream& out_stream,
                template_value in_value,
                bool const in_big_endian)
            {
                /** @note 2014.05.10
                    C++1y対応コンパイラでないとstatic_ifが使えないので、
                    テンプレート特殊化で実装しておく。
                 */
                return psyq::internal::message_pack_cast_value_serializer<
                    std::is_integral<template_value>::value>::write(
                        out_stream, std::move(in_value), in_big_endian);
            }
        };

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
            static void write(
                template_stream& out_stream,
                template_tuple const& in_tuple)
            {
                psyq::internal
                    ::message_pack_tuple_serializer<template_length - 1>
                        ::write(out_stream, in_tuple);
                out_stream << std::get<template_length - 1>(in_tuple);
            }
        };
        /// @copydoc message_pack_tuple_serializer
        template<> struct message_pack_tuple_serializer<0>
        {
            /// タプルの要素数が0なので、何もしない。
            template<typename template_stream, typename template_tuple>
            static void write(template_stream&, template_tuple const&) {}
        };
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief MessagePack形式で直列化したバイナリを、 std::basic_ostream
           互換のストリームへ出力するアダプタ。
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

    /** @brief 直列化途中のコンテナのスタック限界数。
     */
    public: static std::size_t const stack_capacity = template_stack_capacity;

    /// 値を直列化する場合のエンディアン性。
    public: enum endianess: bool
    {
        little_endian = false, ///< リトルエンディアン。
        big_endian = true,     ///< ビッグエンディアン。
        //native_endian = ,    ///< ネイティブエンディアン。
    };

    /** @brief 次に直列化する値の種類。
     */
    public: enum next_type
    {
        next_type_VALUE,
        next_type_RAW_ELEMENT,   ///< RAWバイト列の要素。
        next_type_ARRAY_ELEMENT, ///< 配列の要素。
        next_type_MAP_KEY,       ///< 連想配列の要素のキー。
        next_type_MAP_VALUE,     ///< 連想配列の要素の値。
    };

    /// @copydoc self::stack_
    private: struct stack
    {
        std::size_t rest_size;         ///< コンテナ要素の残数。
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
        this->fill_container_stack();
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
    /** @brief nil値をMessagePack形式で直列化し、ストリームへ出力する。
     */
    public: void write_nil()
    {
        if (this->get_stack_top_raw() != nullptr)
        {
            PSYQ_ASSERT(false);
            return;
        }
        this->write_big_endian<std::uint8_t>(psyq::message_pack::header_NIL);
        this->update_container_stack();
    }

    /** @brief 真偽値をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_boolean 直列化する真偽値。
     */
    public: void write_boolean(bool const in_boolean)
    {
        if (this->get_stack_top_raw() != nullptr)
        {
            PSYQ_ASSERT(false);
            return;
        }
        this->write_big_endian<std::uint8_t>(
            in_boolean?
                psyq::message_pack::header_TRUE:
                psyq::message_pack::header_FALSE);
        this->update_container_stack();
    }

    /** @brief 無符号整数をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_integer 直列化する整数。
     */
    public: template<typename template_integer_type>
    void write_unsigned_integer(template_integer_type const in_integer)
    {
        if (this->get_stack_top_raw() != nullptr)
        {
            PSYQ_ASSERT(false);
            return;
        }

        static_assert(
            std::is_unsigned<template_integer_type>::value,
            "template_integer_type is not unsigned integer type.");
        if (in_integer <= (std::numeric_limits<std::uint8_t>::max)())
        {
            if (psyq::message_pack::header_FIX_INT_MAX < in_integer)
            {
                this->write_big_endian<std::uint8_t>(
                    psyq::message_pack::header_UINT8);
            }
            this->write_big_endian(static_cast<std::uint8_t>(in_integer));
        }
        else if (in_integer <= (std::numeric_limits<std::uint16_t>::max)())
        {
            this->write_big_endian<std::uint8_t>(
                psyq::message_pack::header_UINT16);
            this->write_big_endian(static_cast<std::uint16_t>(in_integer));
        }
        else if (in_integer <= (std::numeric_limits<std::uint32_t>::max)())
        {
            this->write_big_endian<std::uint8_t>(
                psyq::message_pack::header_UINT32);
            this->write_big_endian(static_cast<std::uint32_t>(in_integer));
        }
        else
        {
            PSYQ_ASSERT(
                in_integer <= (std::numeric_limits<std::uint64_t>::max)());
            this->write_big_endian<std::uint8_t>(
                psyq::message_pack::header_UINT64);
            this->write_big_endian(static_cast<std::uint64_t>(in_integer));
        }
        this->update_container_stack();
    }

    /** @brief 有符号整数をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_integer 直列化する整数。
     */
    public: template<typename template_integer_type>
    void write_signed_integer(template_integer_type const in_integer)
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
            this->write_unsigned_integer(
                static_cast<unsigned_integer>(in_integer));
            return;
        }
        if (this->get_stack_top_raw() != nullptr)
        {
            PSYQ_ASSERT(false);
            return;
        }

        // 0未満の整数を直列化する。
        if ((std::numeric_limits<std::int8_t>::min)() <= in_integer)
        {
            if (in_integer < psyq::message_pack::header_FIX_INT_MIN)
            {
                this->write_big_endian<std::uint8_t>(
                    psyq::message_pack::header_INT8);
            }
            this->write_big_endian(static_cast<std::int8_t>(in_integer));
        }
        else if ((std::numeric_limits<std::int16_t>::min)() <= in_integer)
        {
            this->write_big_endian<std::uint8_t>(
                psyq::message_pack::header_INT16);
            this->write_big_endian(static_cast<std::int16_t>(in_integer));
        }
        else if ((std::numeric_limits<std::int32_t>::min)() <= in_integer)
        {
            this->write_big_endian<std::uint8_t>(
                psyq::message_pack::header_INT32);
            this->write_big_endian(static_cast<std::int32_t>(in_integer));
        }
        else
        {
            PSYQ_ASSERT(
                (std::numeric_limits<std::int64_t>::min)() <= in_integer);
            this->write_big_endian<std::uint8_t>(
                psyq::message_pack::header_INT64);
            this->write_big_endian(static_cast<std::int64_t>(in_integer));
        }
        this->update_container_stack();
    }

    /** @brief 浮動小数点数をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_float 直列化する浮動小数点数。
     */
    public: void write_floating_point(float const in_float)
    {
        if (this->get_stack_top_raw() != nullptr)
        {
            PSYQ_ASSERT(false);
            return;
        }
        this->write_big_endian<std::uint8_t>(
            psyq::message_pack::header_FLOAT32);
        this->write_big_endian(in_float);
        this->update_container_stack();
    }
    /// @copydoc self::write_floating_point(float const)
    public: void write_floating_point(double const in_float)
    {
        if (this->get_stack_top_raw() != nullptr)
        {
            PSYQ_ASSERT(false);
            return;
        }
        this->write_big_endian<std::uint8_t>(
            psyq::message_pack::header_FLOAT64);
        this->write_big_endian(in_float);
        this->update_container_stack();
    }
    //@}
    /** @brief 数値を直列化し、ストリームへ出力する。
     */
    private: template<typename template_value>
    bool write_big_endian(template_value const in_value)
    {
        return psyq::internal::message_pack_value_serializer<template_value>
            ::write(this->stream_, in_value, self::big_endian);
    }

    //-------------------------------------------------------------------------
    /// @name MessagePack RAWバイト列への直列化
    //@{
    /** @brief 連続するメモリ領域にある文字列を、
               MessagePack形式の文字列として直列化し、ストリームへ出力する。
        @param[in] in_begin  直列化する文字列の先頭位置。
        @param[in] in_length 直列化する文字列の要素数。
     */
    public: template<typename template_char>
    void write_contiguous_string(
        template_char const* const in_begin,
        std::size_t const in_length)
    {
        // MessagePack文字列はUTF-8なので、文字は1バイト単位となる。
        static_assert(
            sizeof(template_char) == 1, "MessagePack string is only UTF-8.");

        // 文字列の大きさを直列化する。
        auto const local_size(in_length * sizeof(template_char));
        if (this->write_string_header(local_size))
        {
            // 文字列を直列化する。
            this->write_raw_data(in_begin, local_size);
        }
    }
    /** @brief 連続するメモリ領域にある配列を、
               MessagePack形式のバイナリとして直列化し、ストリームへ出力する。
        @param[in] in_begin  直列化する配列の先頭位置。
        @param[in] in_length 直列化する配列の要素数。
     */
    public: template<typename template_element_type>
    void write_contiguous_binary(
        template_element_type const* const in_begin,
        std::size_t const in_length)
    {
        // バイナリの大きさを直列化する。
        auto const local_size(in_length * sizeof(template_element_type));
        if (this->write_raw_header(local_size, psy::message_pack::header_BIN8))
        {
            // バイナリを直列化する。
            this->write_raw_data(in_begin, local_size);
        }
    }
    /** @brief 値をMessagePack形式の拡張バイナリとして直列化し、
               ストリームへ出力する。
        @param[in] in_value 直列化する値。
        @param[in] in_type  直列化する値の拡張型識別値。
     */
    public: template<typename template_element_type>
    void write_contiguous_extended_binary(
        template_element_type const& in_value,
        std::int8_t const in_type)
    {
        this->write_contiguous_extended_binary(&in_value, 1, in_type);
    }
    /** @brief 連続するメモリ上にある配列を、MessagePack形式の
               拡張バイナリとして直列化し、ストリームへ出力する。
        @param[in] in_begin  直列化する配列の先頭位置。
        @param[in] in_length 直列化する配列の要素数。
        @param[in] in_type   直列化する配列の拡張型識別値。
     */
    public: template<typename template_element_type>
    void write_contiguous_extended_binary(
        template_element_type const* const in_begin,
        std::size_t const in_length,
        std::int8_t const in_type)
    {
        // 拡張バイナリの大きさと拡張型識別値を直列化する。
        auto const local_size(in_length * sizeof(template_element_type));
        if (this->write_extended_binary_header(local_size, in_type))
        {
            // 拡張バイナリを直列化する。
            this->write_raw_data(in_begin, local_size);
        }
    }

    /** @brief コンテナをMessagePack形式の文字列として直列化し、
               ストリームへ出力する。
        @param[in] in_begin  直列化するコンテナの先頭位置。
        @param[in] in_length 直列化するコンテナの要素数。
     */
    public: template<typename template_iterator>
    void write_container_string(
        template_iterator const& in_begin,
        std::size_t const in_length)
    {
        typedef typename std::iterator_traits<template_iterator>::value_type
            element;
        // MessagePack文字列はUTF-8なので、文字は1バイト単位となる。
        static_assert(
            sizeof(element) == 1, "MessagePack string is only UTF-8.");
        this->make_serial_string<element>(in_length);
        this->write_serial_raw(self::big_endian, in_begin, in_length);
    }
    /** @brief コンテナをMessagePack形式の文字列として直列化し、
               ストリームへ出力する。
        @param[in] in_container 直列化するコンテナ。
     */
    public: template<typename template_container>
    void write_container_string(template_container const& in_container)
    {
        this->write_container_string(
            in_container.begin(), in_container.size());
    }

    /** @brief コンテナをMessagePack形式のバイナリとして直列化し、
               ストリームへ出力する。
        @param[in] in_endianess コンテナ要素を直列化するときの self::endianess
        @param[in] in_begin     直列化するコンテナの先頭位置。
        @param[in] in_length    直列化するコンテナの要素数。
     */
    public: template<typename template_iterator>
    void write_container_binary(
        bool const in_endianess,
        template_iterator const& in_begin,
        std::size_t const in_length)
    {
        typedef typename std::iterator_traits<template_iterator>::value_type
            element;
        this->make_serial_binary<element>(in_length);
        this->write_serial_raw(in_endianess, in_begin, in_length);
    }
    /** @brief コンテナをMessagePack形式のバイナリとして直列化し、
               ストリームへ出力する。
        @param[in] in_endianess コンテナ要素を直列化するときの self::endianess
        @param[in] in_container 直列化するコンテナ。
     */
    public: template<typename template_container>
    void write_container_binary(
        bool const in_endianess,
        template_container const& in_container)
    {
        this->write_container_binary(
            in_endianess, in_container.begin(), in_container.size());
    }

    /** @brief コンテナをMessagePack形式の拡張バイナリとして直列化し、
               ストリームへ出力する。
        @param[in] in_endianess コンテナ要素を直列化するときの self::endianess
        @param[in] in_begin     直列化するコンテナの先頭位置。
        @param[in] in_length    直列化するコンテナの要素数。
        @param[in] in_type      直列化するコンテナの拡張型識別値。
     */
    public: template<typename template_iterator>
    void write_container_extended_binary(
        bool const in_endianess,
        template_iterator const& in_begin,
        std::size_t const in_length,
        std::int8_t const in_type)
    {
        typedef typename std::iterator_traits<template_iterator>::value_type
            element;
        this->make_serial_extended_binary<element>(in_length, in_type);
        this->write_serial_raw(in_endianess, in_begin, in_length);
    }
    /** @brief コンテナをMessagePack形式の拡張バイナリとして直列化し、
               ストリームへ出力する。
        @param[in] in_endianess コンテナ要素を直列化するときの self::endianess
        @param[in] in_container 直列化するコンテナ。
        @param[in] in_type      直列化するコンテナの拡張型識別値。
     */
    public: template<typename template_container>
    void write_container_extended_binary(
        bool const in_endianess,
        template_container const& in_container,
        std::int8_t const in_type)
    {
        this->write_container_extended_binary(
            in_endianess, in_container.begin(), in_container.size(), in_type);
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列コンテナの直列化を開始する。

        以後、 in_length 個の要素を、 write_serial_raw() で直列化できる。

        @param[in] in_length 直列化する文字列コンテナの要素数。
        @sa self::write_serial_raw() self::fill_container_rest()
     */
    public: void make_serial_string(std::size_t const in_length)
    {
        if (in_length <= 0)
        {
            // 空の文字列を直列化する。
            this->write_big_endian<std::uint8_t>(
                psyq::message_pack::header_FIX_STR_MIN);
            this->update_container_stack();
        }
        else if (self::stack_capacity <= this->get_container_stack_size())
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
        }
        // 文字列のバイト数を直列化する。
        else if (this->write_string_header(in_length))
        {
            // RAWバイト列をスタックに積む。
            auto& local_stack(
                this->stack_.at(this->get_container_stack_size()));
            local_stack.type = self::next_type_RAW_ELEMENT;
            local_stack.rest_size = in_length;
            ++this->stack_size_;
        }
    }
    /** @brief バイナリコンテナの直列化を開始する。

        以後 in_length 個の要素を、 write_serial_raw() で直列化できる。

        @tparam template_element バイナリコンテナの要素の型。
        @param[in] in_length 直列化するバイナリコンテナの要素数。
        @sa self::write_serial_raw() self::fill_container_rest()
     */
    public: template<typename template_element>
    void make_serial_binary(std::size_t const in_length)
    {
        auto const local_size(in_length * sizeof(template_element));
        if (self::stack_capacity <= this->get_container_stack_size())
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
        }
        // バイナリのバイト数を直列化する。
        else if (
            this->write_raw_header(local_size, psyq::message_pack::header_BIN8)
            && 0 < local_size)
        {
            // RAWバイト列をスタックに積む。
            auto& local_stack(
                this->stack_.at(this->get_container_stack_size()));
            local_stack.type = self::next_type_RAW_ELEMENT;
            local_stack.rest_size = local_size;
            ++this->stack_size_;
        }
    }
    /** @brief 拡張バイナリの直列化を開始する。

        以後 in_length 個の要素を、 write_serial_raw() で直列化できる。

        @tparam template_element 直列化するコンテナの要素の型。
        @param[in] in_length 直列化するコンテナの要素数。
        @param[in] in_type   直列化するコンテナの拡張型識別値。
        @sa self::write_serial_raw() self::fill_container_rest()
     */
    public: template<typename template_element>
    void make_serial_extended_binary(
        std::size_t const in_length,
        std::int8_t const in_type)
    {
        auto const local_size(in_length * sizeof(template_element));
        if (self::stack_capacity <= this->get_container_stack_size())
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
        }
        else if (
            // 拡張バイナリのバイト数を直列化する。
            this->write_extended_binary_header(local_size, in_type)
            && 0 < local_size)
        {
            // RAWバイト列をスタックに積む。
            auto& local_stack(
                this->stack_.at(this->get_container_stack_size()));
            local_stack.type = self::next_type_RAW_ELEMENT;
            local_stack.rest_size = local_size;
            ++this->stack_size_;
        }
    }
    /** @brief コンテナの要素をMessagePack形式のRAWバイト列として直列化し、
               ストリームへ出力する。
        @param[in] in_endianess コンテナ要素を直列化するときの self::endianess
        @param[in] in_iterator  直列化するコンテナの先頭位置。
        @param[in] in_length    直列化するコンテナの要素数。
        @sa self::make_serial_string()
            self::make_serial_binary()
            self::make_serial_extended_binary()
     */
    public: template<typename template_iterator>
    void write_serial_raw(
        bool const in_endianess,
        template_iterator in_iterator,
        std::size_t in_length)
    {
        auto const local_stack(this->get_stack_top_raw());
        if (local_stack == nullptr)
        {
            PSYQ_ASSERT(false);
            return;
        }
        typedef typename std::iterator_traits<template_iterator>::value_type
            element;
        typedef psyq::internal::message_pack_value_serializer<element>
            value_serializer;
        for (;; --in_length, ++in_iterator)
        {
            if (local_stack->rest_size < sizeof(element))
            {
                this->fill_container_rest();
                break;
            }
            else if (in_length <= 0)
            {
                break;
            }
            local_stack->rest_size -= sizeof(element);
            value_serializer::write(this->stream_, *in_iterator, in_endianess);
        }
    }
    //@}
    /** @brief 最上段スタックがRAWバイト列なら、それを取得する。
     */
    private: typename self::stack* get_stack_top_raw()
    {
        if (0 < this->get_container_stack_size())
        {
            auto& local_stack(
                this->stack_.at(this->get_container_stack_size() - 1));
            if (local_stack.type == self::next_type_RAW_ELEMENT)
            {
                return &local_stack;
            }
        }
        return nullptr;
    }

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
            return this->write_raw_header(
                in_size, psyq::message_pack::header_STR8);
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
        }
        return false;
    }

    /** @brief 拡張バイナリのバイト数と型識別値を書き込む。
        @param[in] in_size 拡張バイナリのバイト数。
        @param[in] in_type 拡張型識別値。
     */
    private: bool write_extended_binary_header(
        std::size_t const in_size,
        std::int8_t const in_type)
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
            local_write = this->write_raw_header(
                in_size, psyq::message_pack::header_EXT8);
            break;
        }
        if (!local_write)
        {
            return false;
        }

        // 拡張型識別値を直列化する。
        return this->write_big_endian(in_type);
    }

    /** @brief RAWバイト列のバイト数を書き込む。
        @param[in] in_size         RAWバイト列のバイト数。
        @param[in] in_header_begin RAWバイト列のヘッダ。
     */
    private: bool write_raw_header(
        std::size_t const in_size,
        std::uint8_t const in_header_begin)
    {
        if (this->get_stack_top_raw() != nullptr)
        {
            PSYQ_ASSERT(false);
        }
        else if (in_size <= (std::numeric_limits<std::uint8_t>::max)())
        {
            return this->write_big_endian(in_header_begin)
                && this->write_big_endian(static_cast<std::uint8_t>(in_size));
        }
        else if (in_size <= (std::numeric_limits<std::uint16_t>::max)())
        {
            return this->write_big_endian(
                    static_cast<std::uint8_t>(in_header_begin + 1))
                && this->write_big_endian(static_cast<std::uint16_t>(in_size));
        }
        else if (in_size <= (std::numeric_limits<std::uint32_t>::max)())
        {
            return this->write_big_endian(
                    static_cast<std::uint8_t>(in_header_begin + 2))
                && this->write_big_endian(static_cast<std::uint32_t>(in_size));
        }
        else
        {
            PSYQ_ASSERT(false);
        }
        return false;
    }

    /** @brief RAWバイト列を書き込む。
        @param[in] in_data RAWバイト列の先頭位置。
        @param[in] in_size RAWバイト列のバイト数。
     */
    private: bool write_raw_data(
        void const* const in_data,
        std::size_t const in_size)
    {
        this->stream_.write(
            static_cast<typename self::stream::char_type const*>(in_data),
            in_size);
        if (!this->stream_.good())
        {
            PSYQ_ASSERT(false);
            return false;
        }
        this->update_container_stack();
        return true;
    }

    //-------------------------------------------------------------------------
    /// @name MessagePackコンテナへの直列化
    //@{
    /** @brief タプルをMessagePack形式の配列として直列化し、
               ストリームへ出力する。
        @tparam template_tuple std::tuple 互換のタプル型。
        @param[in] in_tuple 直列化するタプル。
     */
    public: template<typename template_tuple>
    void write_tuple(template_tuple const& in_tuple)
    {
        this->make_serial_array(std::tuple_size<template_tuple>::value);
        psyq::internal::message_pack_tuple_serializer<
            std::tuple_size<template_tuple>::value>
                ::write(*this, in_tuple);
    }

    /** @brief コンテナをMessagePack形式の配列として直列化し、
               ストリームへ出力する。
        @param[in] in_iterator 直列化するコンテナの先頭位置。
        @param[in] in_length   直列化するコンテナの要素数。
     */
    public: template<typename template_iterator>
    void write_array(template_iterator in_iterator, std::size_t in_length)
    {
        this->make_serial_array(in_length);
        for (; 0 < in_length; --in_length, ++in_iterator)
        {
            *this << *in_iterator;
        }
    }

    /** @brief コンテナをMessagePack形式の配列として直列化し、
               ストリームへ出力する。
        @param[in] in_container 直列化するコンテナ。
     */
    public: template<typename template_container>
    void write_array(template_container const& in_container)
    {
        this->write_array(in_container.begin(), in_container.size());
    }

    /** @brief コンテナをMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @param[in] in_iterator 直列化するコンテナの先頭位置。
        @param[in] in_length   直列化するコンテナの要素数。
     */
    public: template<typename template_iterator>
    void write_set(template_iterator in_iterator, std::size_t in_length)
    {
        // マップ値が空の連想配列としてコンテナを直列化する。
        this->make_serial_map(in_length);
        for (; 0 < in_length; --in_length, ++in_iterator)
        {
            *this << *in_iterator;
            this->write_nil();
        }
    }
    /** @brief コンテナをMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @param[in] in_set 直列化するコンテナ。
     */
    public: template<typename template_set>
    void write_set(template_set const& in_set)
    {
        this->write_set(in_set.begin(), in_set.size());
    }

    /** @brief pairコンテナをMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @tparam template_iterator std::pair 互換の要素を指す反復子の型。
        @param[in] in_iterator 直列化する連想配列コンテナの先頭位置。
        @param[in] in_length   直列化する連想配列コンテナの要素数。
     */
    public: template<typename template_iterator>
    void write_map(template_iterator in_iterator, std::size_t in_length)
    {
        this->make_serial_map(in_length);
        for (; 0 < in_length; --in_length, ++in_iterator)
        {
            auto& local_value(*in_iterator);
            *this << local_value.first << local_value.second;
        }
    }
    /** @brief pairコンテナをMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @tparam template_map std::pair 互換の要素を持つコンテナ型。
        @param[in] in_map 直列化する連想配列コンテナ。
     */
    public: template<typename template_map>
    void write_map(template_map const& in_map)
    {
        this->write_map(in_map.begin(), in_map.size());
    }

    //-------------------------------------------------------------------------
    /** @brief MessagePack形式の配列の直列化を開始する。

        以後 in_length 個のMessagePack値を、配列の要素として直列化できる。

        以下の関数を1回呼び出す毎に、MessagePack値を1つ直列化する。
        - self::write_nil()
        - self::write_boolean()
        - self::write_unsigned_integer()
        - self::write_signed_integer()
        - self::write_floating_point()
        - self::write_contiguous_string()
        - self::write_contiguous_binary()
        - self::write_contiguous_extended_binary()
        - self::write_container_string()
        - self::write_container_binary()
        - self::write_container_extended_binary()
        - self::write_tuple()
        - self::write_array()
        - self::write_set()
        - self::write_map()
        - self::make_serial_string()
        - self::make_serial_binary()
        - self::make_serial_extended_binary()
        - self::make_serial_array()
        - self::make_serial_map()

        @param[in] in_length 直列化する配列の要素数。
        @sa self::fill_container_rest() self::fill_container_stack()
     */
    public: void make_serial_array(std::size_t const in_length)
    {
        this->make_serial_container<
            self::next_type_ARRAY_ELEMENT,
            psyq::message_pack::header_ARRAY16,
            psyq::message_pack::header_FIX_ARRAY_MIN,
            psyq::message_pack::header_FIX_ARRAY_MAX>(
                in_length);
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
        - self::write_contiguous_string()
        - self::write_contiguous_binary()
        - self::write_contiguous_extended_binary()
        - self::write_container_string()
        - self::write_container_binary()
        - self::write_container_extended_binary()
        - self::write_tuple()
        - self::write_array()
        - self::write_set()
        - self::write_map()
        - self::make_serial_string()
        - self::make_serial_binary()
        - self::make_serial_extended_binary()
        - self::make_serial_array()
        - self::make_serial_map()

        @param[in] in_length 直列化する連想配列の要素数。
        @sa self::fill_container_rest() self::fill_container_stack()
     */
    public: void make_serial_map(std::size_t const in_length)
    {
        this->make_serial_container<
            self::next_type_MAP_KEY,
            psyq::message_pack::header_MAP16,
            psyq::message_pack::header_FIX_MAP_MIN,
            psyq::message_pack::header_FIX_MAP_MAX>(
                in_length);
    }

    /** @brief 直前に直列化を開始したコンテナの残り要素をnil値で埋める。
        @sa self::make_serial_array() self::make_serial_map()
     */
    public: void fill_container_rest()
    {
        if (this->get_container_stack_size() <= 0)
        {
            return;
        }

        // 残りの要素数を決定する。
        auto& local_stack(
            this->stack_.at(this->get_container_stack_size() - 1));
        std::size_t local_empty_count;
        std::uint8_t local_empty_value(psyq::message_pack::header_NIL);
        switch (local_stack.type)
        {
        case self::next_type_RAW_ELEMENT:
            local_empty_value = 0;
            local_empty_count = local_stack.rest_size;
            break;

        case self::next_type_ARRAY_ELEMENT:
            local_empty_count = local_stack.rest_size;
            break;

        case self::next_type_MAP_KEY:
            local_empty_count = local_stack.rest_size * 2;
            break;

        case self::next_type_MAP_VALUE:
            local_empty_count = local_stack.rest_size * 2 + 1;
            break;

        default:
            PSYQ_ASSERT(false);
            return;
        }

        // コンテナの残り要素を埋める。
        for (; 0 < local_empty_count; --local_empty_count)
        {
            this->write_big_endian<std::uint8_t>(local_empty_value);
        }
        --this->stack_size_;
        this->update_container_stack();
    }

    /** @brief 現在直列化途中のオブジェクトの残りをnil値で埋める。
        @sa self::make_serial_array() self::make_serial_map()
     */
    public: void fill_container_stack()
    {
        while (0 < this->get_container_stack_size())
        {
            this->fill_container_rest();
        }
    }
    //@}
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
            // 空の連想配列を直列化する。
            if (this->write_big_endian<std::uint8_t>(template_header_fix_min))
            {
                this->update_container_stack();
                return true;
            }
            return false;
        }
        else if (self::stack_capacity <= this->get_container_stack_size())
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
        auto& local_stack(
            this->stack_.at(this->get_container_stack_size()));
        local_stack.type = template_next_type;
        local_stack.rest_size = in_length;
        ++this->stack_size_;
        return true;
    }

    /** @brief コンテナスタックを更新する。
     */
    private: void update_container_stack()
    {
        if (this->get_container_stack_size() <= 0)
        {
            return;
        }

        auto& local_stack(
            this->stack_.at(this->get_container_stack_size() - 1));
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

        case self::next_type_RAW_ELEMENT:
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
        return 0 < this->get_container_stack_size()?
            this->stack_.at(this->get_container_stack_size() - 1).type:
            typename self::next_type_VALUE;
    }

    /** @brief 現在直列化途中のコンテナの残り要素数を取得する。
        @return 現在のコンテナスタックの残り要素数。
        @sa self::make_serial_array() self::make_serial_map()
     */
    public: std::size_t get_container_rest_size() const PSYQ_NOEXCEPT
    {
        return 0 < this->get_container_stack_size()?
            this->stack_.at(this->get_container_stack_size() - 1).rest_size: 0;
    }

    /** @brief 直列化途中のコンテナの数を取得する。
        @return 直列化途中のコンテナの数。
        @sa self::make_serial_array() self::make_serial_map()
     */
    public: std::size_t get_container_stack_size() const PSYQ_NOEXCEPT
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

    private: void swap_stream(typename self::stream& io_stream)
    {
        this->fill_container_stack();
        this->stream_.swap(io_stream);
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
            out_stream,
    std::basic_string<template_char, template_traits, template_allocator>
        const& in_string)
{
    static_assert(
        sizeof(template_char) == 1, "sizeof(template_char) is not 1.");
    out_stream.write_contiguous_string(in_string.data(), in_string.length());
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
            out_stream,
    std::unordered_set<
        template_key, template_hash, template_compare, template_allocator>
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
            out_stream,
    std::unordered_multiset<
        template_key, template_hash, template_compare, template_allocator>
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
            out_stream,
    std::map<
        template_key, template_mapped, template_compare, template_allocator>
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
            out_stream,
    std::multimap<
        template_key, template_mapped, template_compare, template_allocator>
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
    psyq::message_pack::serializer<
        template_out_stream, template_stack_capacity>&
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
