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
        template<typename, std::size_t> class serializer;
        /// @endcond

        /** @brief MessagePackに格納されている値の種別。

            以下のウェブページにある仕様を参照した。
            https://github.com/msgpack/msgpack/blob/d257d3c143c9fa21ba22afc666fe50cc66943ece/spec.md#formats
         */
        enum header
        {
            header_FIX_INT_MIN   = -0x20,
            header_FIX_INT_MAX   = 0x7f,
            header_FIX_MAP_MIN   = 0x80,
            header_FIX_MAP_MAX   = 0x8f,
            header_FIX_ARRAY_MIN = 0x90,
            header_FIX_ARRAY_MAX = 0x9f,
            header_FIX_STR_MIN   = 0xa0,
            header_FIX_STR_MAX   = 0xbf,
            header_NIL           = 0xc0,
            header_NEVER_USED    = 0xc1,
            header_FALSE         = 0xc2,
            header_TRUE          = 0xc3,
            header_BIN8          = 0xc4,
            header_BIN16         = 0xc5,
            header_BIN32         = 0xc6,
            header_EXT8          = 0xc7,
            header_EXT16         = 0xc8,
            header_EXT32         = 0xc9,
            header_FLOAT32       = 0xca,
            header_FLOAT64       = 0xcb,
            header_UINT8         = 0xcc,
            header_UINT16        = 0xcd,
            header_UINT32        = 0xce,
            header_UINT64        = 0xcf,
            header_INT8          = 0xd0,
            header_INT16         = 0xd1,
            header_INT32         = 0xd2,
            header_INT64         = 0xd3,
            header_FIX_EXT1      = 0xd4,
            header_FIX_EXT2      = 0xd5,
            header_FIX_EXT4      = 0xd6,
            header_FIX_EXT8      = 0xd7,
            header_FIX_EXT16     = 0xd8,
            header_STR8          = 0xd9,
            header_STR16         = 0xda,
            header_STR32         = 0xdb,
            header_ARRAY16       = 0xdc,
            header_ARRAY32       = 0xdd,
            header_MAP16         = 0xde,
            header_MAP32         = 0xdf,
        };
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief MessagePack形式で直列化したバイナリを、 std::basic_ostream
           互換の出力ストリームオブジェクトに書き込むアダプタ。
    @tparam template_out_stream @copydoc self::stream
    @tparam template_stack_capacity @copydoc self::stack_capacity
 */
template<
    typename template_out_stream = std::ostringstream,
    std::size_t template_stack_capacity
        = PSYQ_MESSAGE_PACK_SERIALIZER_STACK_CAPACITY_DEFAULT>
class psyq::message_pack::serializer
{
    /// thisが指す値の型。
    private: typedef serializer<template_out_stream, template_stack_capacity>
        self;

    /** @brief 直列化したMessagePack形式のバイナリを書き込む
               std::basic_ostream 互換の出力ストリーム。
     */
    public: typedef template_out_stream stream;
    static_assert(
        sizeof(typename self::stream::char_type) == 1,
        "sizeof(self::stream::char_type) is not 1.");

    /** @brief 直列化途中のコンテナのスタック限界数。
     */
    public: static std::size_t const stack_capacity = template_stack_capacity;

    /** @brief 次に直列化するオブジェクトの種類。
     */
    public: enum next_kind
    {
        next_kind_VALUE,
        next_kind_ARRAY_ITEM, ///< 配列の要素。
        next_kind_MAP_KEY,    ///< 連想配列の要素のキー。
        next_kind_MAP_VALUE,  ///< 連想配列の要素の値。
    };

    /// @copydoc self::stack_
    private: struct stack
    {
        std::size_t rest_size;         ///< コンテナ要素の残数。
        typename self::next_kind kind; ///< @copydoc self::next_kind
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

    /// 出力ストリームを破壊する。
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
    /// @name 値の直列化
    //@{
    /** @brief nilを直列化し、出力ストリームへ書き込む。
     */
    public: void write_nil()
    {
        this->put(psyq::message_pack::header_NIL);
        this->update_container_stack();
    }

    /** @brief 真偽値を直列化し、出力ストリームへ書き込む。
        @param[in] in_boolean 直列化する真偽値。
     */
    public: void write_boolean(bool const in_boolean)
    {
        this->put(
            in_boolean?
                psyq::message_pack::header_TRUE:
                psyq::message_pack::header_FALSE);
        this->update_container_stack();
    }

    /** @brief 無符号整数を直列化し、出力ストリームへ書き込む。
        @param[in] in_integer 直列化する整数。
     */
    public: template<typename template_integer_type>
    void write_unsigned_integer(template_integer_type const in_integer)
    {
        static_assert(
            std::is_unsigned<template_integer_type>::value,
            "template_integer_type is not unsigned integer type.");
        if (in_integer <= (std::numeric_limits<std::uint8_t>::max)())
        {
            if (psyq::message_pack::header_FIX_INT_MAX < in_integer)
            {
                this->put(psyq::message_pack::header_UINT8);
            }
            this->put(static_cast<std::uint8_t>(in_integer));
        }
        else if (in_integer <= (std::numeric_limits<std::uint16_t>::max)())
        {
            this->put(psyq::message_pack::header_UINT16);
            this->put_16bits(static_cast<std::uint16_t>(in_integer));
        }
        else if (in_integer <= (std::numeric_limits<std::uint32_t>::max)())
        {
            this->put(psyq::message_pack::header_UINT32);
            this->put_32bits(static_cast<std::uint32_t>(in_integer));
        }
        else
        {
            PSYQ_ASSERT(
                in_integer <= (std::numeric_limits<std::uint64_t>::max)());
            this->put(psyq::message_pack::header_UINT64);
            this->put_64bits(static_cast<std::uint64_t>(in_integer));
        }
        this->update_container_stack();
    }

    /** @brief 有符号整数を直列化し、出力ストリームへ書き込む。
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

        // 0未満の整数を直列化する。
        if ((std::numeric_limits<std::int8_t>::min)() <= in_integer)
        {
            if (in_integer < psyq::message_pack::header_FIX_INT_MIN)
            {
                this->put(psyq::message_pack::header_INT8);
            }
            this->put(static_cast<std::uint8_t>(in_integer));
        }
        else if ((std::numeric_limits<std::int16_t>::min)() <= in_integer)
        {
            this->put(psyq::message_pack::header_INT16);
            this->put_16bits(static_cast<std::uint16_t>(in_integer));
        }
        else if ((std::numeric_limits<std::int32_t>::min)() <= in_integer)
        {
            this->put(psyq::message_pack::header_INT32);
            this->put_32bits(static_cast<std::uint32_t>(in_integer));
        }
        else
        {
            PSYQ_ASSERT(
                (std::numeric_limits<std::int64_t>::min)() <= in_integer);
            this->put(psyq::message_pack::header_INT64);
            this->put_64bits(static_cast<std::uint64_t>(in_integer));
        }
        this->update_container_stack();
    }

    /** @brief 浮動小数点数を直列化し、出力ストリームへ書き込む。
        @param[in] in_float 直列化する浮動小数点数。
     */
    public: void write_floating_point(float const in_float)
    {
        union {std::uint32_t integer; float real;} local_value;
        local_value.real = in_float;
        this->put(psyq::message_pack::header_FLOAT32);
        this->put_32bits(local_value.integer);
        this->update_container_stack();
    }
    /// @copydoc self::write_floating_point(float const)
    public: void write_floating_point(double const in_float)
    {
        union {std::uint64_t integer; double real;} local_value;
        local_value.real = in_float;
        this->put(psyq::message_pack::header_FLOAT64);
        this->put_64bits(local_value.integer);
        this->update_container_stack();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name RAWバイト列の直列化
    //@{
    /** @brief 文字列を直列化し、出力ストリームへ書き込む。
        @param[in] in_data   直列化する文字列の先頭位置。
        @param[in] in_length 文字列の文字数。
     */
    public: template<typename template_char_type>
    void write_string(
        template_char_type const* const in_data,
        std::size_t const in_length)
    {
        // 文字列の大きさを直列化する。
        auto const local_size(in_length * sizeof(template_char_type));
        unsigned const local_fix_size(
            psyq::message_pack::header_FIX_STR_MAX
                - psyq::message_pack::header_FIX_STR_MIN);
        if (local_size <= local_fix_size)
        {
            this->put(
                static_cast<std::uint8_t>(
                    psyq::message_pack::header_FIX_STR_MIN + local_size));
        }
        else if (
            !this->write_raw_size(
                local_size, psyq::message_pack::header_STR8))
        {
            return;
        }

        // 文字列を直列化する。
        this->write_raw_data(in_data, local_size);
    }

    /** @brief バイナリを直列化し、出力ストリームへ書き込む。
        @param[in] in_data   直列化するバイナリの先頭位置。
        @param[in] in_length 直列化するバイナリの要素数。
     */
    public: template<typename template_value_type>
    void write_binary(
        template_value_type const* const in_data,
        std::size_t const in_length)
    {
        // バイナリの大きさを直列化する。
        auto const local_size(in_length * sizeof(template_value_type));
        if (this->write_raw_size(local_size, psy::message_pack::header_BIN8))
        {
            // バイナリを直列化する。
            this->write_raw_data(in_data, local_size);
        }
    }

    /** @brief 拡張バイナリを直列化し、出力ストリームへ書き込む。
        @param[in] in_type 直列化する拡張バイナリ型の識別番号。
        @param[in] in_data 直列化する拡張バイナリ。
     */
    public: template<typename template_value_type>
    void write_extended_binary(
        std::int8_t const in_type,
        template_value_type const& in_data)
    {
        this->write_extended_binary(in_type, &in_data, 1);
    }

    /** @brief 拡張バイナリを直列化し、出力ストリームへ書き込む。
        @param[in] in_type   直列化する拡張バイナリ型の識別番号。
        @param[in] in_data   直列化する拡張バイナリの先頭位置。
        @param[in] in_length 直列化する拡張バイナリの要素数。
     */
    public: template<typename template_value_type>
    void write_extended_binary(
        std::int8_t const in_type,
        template_value_type const* const in_data,
        std::size_t const in_length)
    {
        // 拡張バイナリの大きさを直列化する。
        auto const local_size(in_length * sizeof(template_value_type));
        switch (local_size)
        {
        case 1:
            this->put(psyq::message_pack::header_FIX_EXT1);
            break;
        case 2:
            this->put(psyq::message_pack::header_FIX_EXT2);
            break;
        case 4:
            this->put(psyq::message_pack::header_FIX_EXT4);
            break;
        case 8:
            this->put(psyq::message_pack::header_FIX_EXT8);
            break;
        case 16:
            this->put(psyq::message_pack::header_FIX_EXT16);
            break;
        default:
            if (
                this->write_raw_size(
                    local_size, psyq::message_pack::header_EXT8))
            {
                break;
            }
            return;
        }

        // 拡張バイナリの型とバイナリを直列化する。
        this->put(static_cast<std::int8_t>(in_type));
        this->write_raw_data(in_data, local_size);
    }
    //@}
    /** @brief RAWバイト列のバイト数を書き込む。
        @param[in] in_size         RAWバイト列のバイト数。
        @param[in] in_header_begin RAWバイト列のヘッダ。
     */
    private: bool write_raw_size(
        std::size_t const in_size,
        std::uint8_t const in_header_begin)
    {
        if (in_size <= (std::numeric_limits<std::uint8_t>::max)())
        {
            this->put(in_header_begin);
            this->put(static_cast<std::uint8_t>(in_size));
        }
        else if (in_size <= (std::numeric_limits<std::uint16_t>::max)())
        {
            this->put(in_header_begin + 1);
            this->put_16bits(static_cast<std::uint16_t>(in_size));
        }
        else if (in_size <= (std::numeric_limits<std::uint32_t>::max)())
        {
            this->put(in_header_begin + 2);
            this->put_32bits(static_cast<std::uint32_t>(in_size));
        }
        else
        {
            PSYQ_ASSERT(false);
            return false;
        }
        return true;
    }

    /** @brief RAWバイト列を書き込む。
        @param[in] in_data RAWバイト列の先頭位置。
        @param[in] in_size RAWバイト列のバイト数。
     */
    private: void write_raw_data(
        void const* const in_data,
        std::size_t const in_size)
    {
        this->stream_.write(
            static_cast<typename self::stream::char_type const*>(in_data),
            in_size);
        PSYQ_ASSERT(this->stream_.good());
        this->update_container_stack();
    }

    //-------------------------------------------------------------------------
    /// @name コンテナの直列化
    //@{
    /** @brief MessagePackオブジェクト配列の直列化を開始する。
        @param[in] in_size 直列化する配列の要素数。
        @sa self::fill_container_rest() self::fill_container_stack()
     */
    public: void make_array(std::size_t const in_size)
    {
        if (in_size <= 0)
        {
            // 空の配列を直列化する。
            this->put(psyq::message_pack::header_FIX_ARRAY_MIN);
            this->update_container_stack();
        }
        else if (self::stack_capacity <= this->get_container_stack_size())
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
        }
        // 配列の要素数を直列化する。
        else if (
            this->write_container_size(
                in_size,
                psyq::message_pack::header_ARRAY16,
                psyq::message_pack::header_FIX_ARRAY_MIN,
                psyq::message_pack::header_FIX_ARRAY_MAX))
        {
            // 配列をスタックに積む。
            auto& local_stack(
                this->stack_.at(this->get_container_stack_size()));
            local_stack.kind = self::next_kind_ARRAY_ITEM;
            local_stack.rest_size = in_size;
            ++this->stack_size_;
        }
    }

    /** @brief MessagePackオブジェクト連想配列の直列化を開始する。
        @param[in] in_size 直列化する連想配列の要素数。
        @sa self::fill_container_rest() self::fill_container_stack()
     */
    public: void make_map(std::size_t const in_size)
    {
        if (in_size <= 0)
        {
            // 空の連想配列を直列化する。
            this->put(psyq::message_pack::header_FIX_MAP_MIN);
            this->update_container_stack();
        }
        else if (self::stack_capacity <= this->get_container_stack_size())
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
        }
        // 連想配列の要素数を直列化する。
        else if (
            this->write_container_size(
                in_size,
                psyq::message_pack::header_MAP16,
                psyq::message_pack::header_FIX_MAP_MIN,
                psyq::message_pack::header_FIX_MAP_MAX))
        {
            // 連想配列をスタックに積む。
            auto& local_stack(
                this->stack_.at(this->get_container_stack_size()));
            local_stack.kind = self::next_kind_MAP_KEY;
            local_stack.rest_size = in_size;
            ++this->stack_size_;
        }
    }

    /** @brief 直前に直列化を開始したコンテナの残り要素をnilで埋める。
        @sa self::make_array() self::make_map()
     */
    public: void fill_container_rest()
    {
        if (this->get_container_stack_size() <= 0)
        {
            return;
        }

        // 直列化するnilの数を決定する。
        auto& local_stack(
            this->stack_.at(this->get_container_stack_size() - 1));
        std::size_t local_nil_count;
        switch (local_stack.kind)
        {
        case self::next_kind_ARRAY_ITEM:
            local_nil_count = local_stack.rest_size;
            break;

        case self::next_kind_MAP_KEY:
            local_nil_count = local_stack.rest_size * 2;
            break;

        case self::next_kind_MAP_VALUE:
            local_nil_count = local_stack.rest_size * 2 + 1;
            break;

        default:
            PSYQ_ASSERT(false);
            return;
        }

        // nilを直列化し、コンテナの残り要素を埋める。
        for (; 0 < local_nil_count; --local_nil_count)
        {
            this->put(0xc0);
        }
        --this->stack_size_;
        this->update_container_stack();
    }

    /** @brief 現在直列化途中のオブジェクトの残りをnillで埋める。
        @sa self::make_array() self::make_map()
     */
    public: void fill_container_stack()
    {
        while (0 < this->get_container_stack_size())
        {
            this->fill_container_rest();
        }
    }
    //@}
    /** @brief コンテナの要素数を直列化する。
        @param[in] in_size           コンテナの要素数。
        @param[in] in_header_begin   コンテナのヘッダ。
        @param[in] in_fix_header_min 固定長ヘッダの最小値。
        @param[in] in_fix_header_max 固定長ヘッダの最大値。
     */
    private: bool write_container_size(
        std::size_t const in_size,
        std::uint8_t const in_header_begin,
        std::uint8_t const in_fix_header_min,
        std::uint8_t const in_fix_header_max)
    {
        PSYQ_ASSERT(in_fix_header_min <= in_fix_header_max);
        if (in_size <= unsigned(in_fix_header_max - in_fix_header_min))
        {
            this->put(std::uint8_t(in_fix_header_min + in_size));
        }
        else if (in_size <= (std::numeric_limits<std::uint16_t>::max)())
        {
            this->put(in_header_begin);
            this->put_16bits(static_cast<std::uint16_t>(in_size));
        }
        else if (in_size <= (std::numeric_limits<std::uint32_t>::max)())
        {
            this->put(in_header_begin + 1);
            this->put_32bits(static_cast<std::uint32_t>(in_size));
        }
        else
        {
            // コンテナの限界要素数を超えたので失敗。
            PSYQ_ASSERT(false);
            return false;
        }
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
        switch (local_stack.kind)
        {
        case self::next_kind_ARRAY_ITEM:
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

        case self::next_kind_MAP_KEY:
            local_stack.kind = self::next_kind_MAP_VALUE;
            break;

        case self::next_kind_MAP_VALUE:
            if (1 < local_stack.rest_size)
            {
                local_stack.kind = self::next_kind_MAP_KEY;
                --local_stack.rest_size;
            }
            else
            {
                --this->stack_size_;
                this->update_container_stack();
            }
            break;

        default:
            PSYQ_ASSERT(false);
            break;
        }
    }

    //-------------------------------------------------------------------------
    /** @brief 文字を出力ストリームへ書き込む。
        @param[in] in_char 書き込む文字。
     */
    private: void put(std::uint8_t const in_char)
    {
        this->stream_.put(
            static_cast<typename self::stream::char_type>(in_char));
        PSYQ_ASSERT(this->stream_.good());
    }

    /** @brief 16bit整数をMessagePack形式で直列化し、出力ストリームへ書き込む。
        @param[in] in_integer 直列化する16bit整数。
     */
    private: void put_16bits(std::uint16_t const in_integer)
    {
        this->put(static_cast<std::uint8_t>(in_integer >> 8));
        this->put(static_cast<std::uint8_t>(in_integer));
    }

    /** @brief 32bit整数をMessagePack形式で直列化し、出力ストリームへ書き込む。
        @param[in] in_integer 直列化する32bit整数。
     */
    private: void put_32bits(std::uint32_t const in_integer)
    {
        this->put_16bits(static_cast<std::uint16_t>(in_integer >> 16));
        this->put_16bits(static_cast<std::uint16_t>(in_integer));
    }

    /** @brief 64bit整数をMessagePack形式で直列化し、出力ストリームへ書き込む。
        @param[in] in_integer 直列化する64bit整数。
     */
    private: void put_64bits(std::uint64_t const in_integer)
    {
        this->put_32bits(static_cast<std::uint32_t>(in_integer >> 32));
        this->put_32bits(static_cast<std::uint32_t>(in_integer));
    }

    //-------------------------------------------------------------------------
    /// @name 状態の取得
    //@{
    public: typename self::next_kind get_next_kind() const PSYQ_NOEXCEPT
    {
        return 0 < this->get_container_rest_size()?
            this->stack_.at(this->get_container_rest_size() - 1).kind:
            typename self::next_kind_VALUE;
    }

    /** @brief 現在直列化途中のコンテナの残り要素数を取得する。
        @return 現在のコンテナスタックの残り要素数。
        @sa self::wmake_array() self::make_map()
     */
    public: std::size_t get_container_rest_size() const PSYQ_NOEXCEPT
    {
        return 0 < this->get_container_rest_size()?
            this->stack_.at(this->get_container_rest_size() - 1).rest_size: 0;
    }

    /** @brief 直列化途中のコンテナの数を取得する。
        @return 直列化途中のコンテナの数。
        @sa self::make_array() self::make_map()
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
namespace psyq
{
    /// この名前空間をuserが直接accessするのは禁止。
    namespace internal
    {
        template<
            std::size_t template_index,
            typename template_out_stream,
            typename template_tuple>
        struct message_pack_tuple_serializer
        {
            static void write(
                psyq::message_pack::serializer<template_out_stream>&
                    out_stream,
                template_tuple const& in_tuple)
            {
                static_assert(0 < template_index, "template_tuple is empty.");
                psyq::internal::message_pack_tuple_serializer<
                    template_index - 1, template_out_stream, template_tuple>
                        ::write(out_stream, in_tuple);
                out_stream << std::get<template_index>(in_tuple);
            }
        };
        template<typename template_out_stream, typename template_tuple>
        struct message_pack_tuple_serializer<
            0, template_out_stream, template_tuple>
        {
            static void write(
                psyq::message_pack::serializer<template_out_stream>&
                    out_stream,
                template_tuple const& in_tuple)
            {
                out_stream << std::get<0>(in_tuple);
            }
        };
    }

    namespace message_pack
    {
        /** @brief タプルをMessagePack形式で直列化し、出力ストリームへ書き込む。
            @tparam template_tuple std::tuple 互換のタプル型。
            @param[out] out_stream 書き込む出力ストリーム。
            @param[in]  in_tuple   直列化するタプル。
         */
        template<typename template_out_stream, typename template_tuple>
        void write_tuple(
            psyq::message_pack::serializer<template_out_stream>& out_stream,
            template_tuple const& in_tuple)
        {
            out_stream.make_array(std::tuple_size<template_tuple>::value);
            if (0 < std::tuple_size<template_tuple>::value)
            {
                psyq::internal::message_pack_tuple_serializer<
                    std::tuple_size<template_tuple>::value - 1,
                    template_out_stream,
                    template_tuple>
                        ::write(out_stream, in_tuple);
            }
        }

        /** @brief 配列をMessagePack形式で直列化し、出力ストリームへ書き込む。
            @param[out] out_stream 書き込む出力ストリーム。
            @param[in]  in_iterator 直列化する配列の先頭位置。
            @param[in]  in_size     直列化する配列の要素数。
         */
        template<typename template_out_stream, typename template_iterator>
        void write_array(
            psyq::message_pack::serializer<template_out_stream>& out_stream,
            template_iterator in_iterator,
            std::size_t in_size)
        {
            out_stream.make_array(in_size);
            for (; 0 < in_size; --in_size, ++in_iterator)
            {
                out_stream << *in_iterator;
            }
        }
        /** @brief 配列をMessagePack形式で直列化し、出力ストリームへ書き込む。
            @param[out] out_stream 出力するストリーム。
            @param[in]  in_array   直列化する配列。
         */
        template<typename template_out_stream, typename template_array>
        void write_array(
            psyq::message_pack::serializer<template_out_stream>& out_stream,
            template_array const& in_array)
        {
            psyq::message_pack::write_array(
                out_stream, in_array.begin(), in_array.size());
        }

        /** @brief 集合をMessagePack形式で直列化し、出力ストリームへ書き込む。
            @param[out] out_stream 書き込む出力ストリーム。
            @param[in]  in_set     直列化する連想配列。
         */
        template<typename template_out_stream, typename template_set>
        void write_set(
            psyq::message_pack::serializer<template_out_stream>& out_stream,
            template_set const& in_set)
        {
            // マップ値が空の辞書として集合を直列化する。
            out_stream.make_map(in_set.size());
            for (auto& local_value: in_set)
            {
                out_stream << local_value;
                out_stream.write_nil();
            }
        }

        /** @brief 連想配列をMessagePack形式で直列化し、出力ストリームへ書き込む。
            @param[out] out_stream 書き込む出力ストリーム。
            @param[in]  in_map     直列化する連想配列。
         */
        template<typename template_out_stream, typename template_map>
        void write_map(
            psyq::message_pack::serializer<template_out_stream>& out_stream,
            template_map const& in_map)
        {
            out_stream.make_map(in_map.size());
            for (auto& local_value: in_map)
            {
                out_stream << local_value;
            }
        }
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//----------------------------------------------------------------------------
/// @name 真偽値の直列化
//@{
/** @brief 真偽値をMessagePack形式で直列化し、出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_boolean 直列化する真偽値。
 */
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    bool const in_boolean)
{
    out_stream.write_boolean(in_boolean);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 無符号整数の直列化
//@{
/** @brief 無符号整数をMessagePack形式で直列化し、出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_integer 直列化する無符号整数。
 */
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    unsigned char const in_integer)
{
    out_stream.write_unsigned_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, unsigned char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    unsigned short const in_integer)
{
    out_stream.write_unsigned_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, unsigned char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    unsigned int const in_integer)
{
    out_stream.write_unsigned_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, unsigned char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    unsigned long const in_integer)
{
    out_stream.write_unsigned_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, unsigned char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    unsigned long long const in_integer)
{
    out_stream.write_unsigned_integer(in_integer);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 有符号整数の直列化
//@{
/** @brief 有符号整数をMessagePack形式で直列化し、出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_integer 直列化する有符号整数。
 */
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    char const in_integer)
{
    out_stream.write_signed_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    short const in_integer)
{
    out_stream.write_signed_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    int const in_integer)
{
    out_stream.write_signed_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    long const in_integer)
{
    out_stream.write_signed_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    long long const in_integer)
{
    out_stream.write_signed_integer(in_integer);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 浮動小数点数の直列化
//@{
/** @brief 浮動小数点数をMessagePack形式で直列化し、出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_float   直列化する浮動小数点数。
 */
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    float const in_float)
{
    out_stream.write_floating_point(in_float);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, float const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
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
           出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_string  直列化する文字列。
 */
template<
    typename template_out_stream,
    typename template_char,
    typename template_traits,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::basic_string<template_char, template_traits, template_allocator>
        const& in_string)
{
    out_stream.write_string(in_string.data(), in_string.length());
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name タプルの直列化
//@{
/** @brief std::pair をMessagePack形式で直列化し、出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_pair    直列化するペアタプル。
 */
template<
    typename template_out_stream,
    typename template_first,
    typename template_second>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::pair<template_first, template_second> const& in_pair)
{
    out_stream << in_pair.first << in_pair.second;
    return out_stream;
}

/** @brief std::tuple をMessagePack形式の配列として直列化し、
           出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_tuple   直列化するタプル。
 */
template<typename template_out_stream, typename template_item0>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::tuple<template_item0> const& in_tuple)
{
    psyq::message_pack::write_tuple(out_stream, in_tuple);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, std::tuple<template_item0> const&)
template<
    typename template_out_stream,
    typename template_item0,
    typename template_item1>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::tuple<template_item0, template_item1> const& in_tuple)
{
    psyq::message_pack::write_tuple(out_stream, in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, std::tuple<template_item0> const&)
template<
    typename template_out_stream,
    typename template_item0,
    typename template_item1,
    typename template_item2>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::tuple<template_item0, template_item1, template_item2> const& in_tuple)
{
    psyq::message_pack::write_tuple(out_stream, in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, std::tuple<template_item0> const&)
template<
    typename template_out_stream,
    typename template_item0,
    typename template_item1,
    typename template_item2,
    typename template_item3>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::tuple<template_item0, template_item1, template_item2, template_item3>
        const& in_tuple)
{
    psyq::message_pack::write_tuple(out_stream, in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, std::tuple<template_item0> const&)
template<
    typename template_out_stream,
    typename template_item0,
    typename template_item1,
    typename template_item2,
    typename template_item3,
    typename template_item4>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::tuple<
        template_item0,
        template_item1,
        template_item2,
        template_item3,
        template_item4>
            const& in_tuple)
{
    psyq::message_pack::write_tuple(out_stream, in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, std::tuple<template_item0> const&)
template<
    typename template_out_stream,
    typename template_item0,
    typename template_item1,
    typename template_item2,
    typename template_item3,
    typename template_item4,
    typename template_item5>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::tuple<
        template_item0,
        template_item1,
        template_item2,
        template_item3,
        template_item4,
        template_item5>
            const& in_tuple)
{
    psyq::message_pack::write_tuple(out_stream, in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, std::tuple<template_item0> const&)
template<
    typename template_out_stream,
    typename template_item0,
    typename template_item1,
    typename template_item2,
    typename template_item3,
    typename template_item4,
    typename template_item5,
    typename template_item6>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::tuple<
        template_item0,
        template_item1,
        template_item2,
        template_item3,
        template_item4,
        template_item5,
        template_item6>
            const& in_tuple)
{
    psyq::message_pack::write_tuple(out_stream, in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, std::tuple<template_item0> const&)
template<
    typename template_out_stream,
    typename template_item0,
    typename template_item1,
    typename template_item2,
    typename template_item3,
    typename template_item4,
    typename template_item5,
    typename template_item6,
    typename template_item7>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::tuple<
        template_item0,
        template_item1,
        template_item2,
        template_item3,
        template_item4,
        template_item5,
        template_item6,
        template_item7>
            const& in_tuple)
{
    psyq::message_pack::write_tuple(out_stream, in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, std::tuple<template_item0> const&)
template<
    typename template_out_stream,
    typename template_item0,
    typename template_item1,
    typename template_item2,
    typename template_item3,
    typename template_item4,
    typename template_item5,
    typename template_item6,
    typename template_item7,
    typename template_item8>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::tuple<
        template_item0,
        template_item1,
        template_item2,
        template_item3,
        template_item4,
        template_item5,
        template_item6,
        template_item7,
        template_item8>
            const& in_tuple)
{
    psyq::message_pack::write_tuple(out_stream, in_tuple);
    return out_stream;
}

/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, std::tuple<template_item0> const&)
template<
    typename template_out_stream,
    typename template_item0,
    typename template_item1,
    typename template_item2,
    typename template_item3,
    typename template_item4,
    typename template_item5,
    typename template_item6,
    typename template_item7,
    typename template_item8,
    typename template_item9>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::tuple<
        template_item0,
        template_item1,
        template_item2,
        template_item3,
        template_item4,
        template_item5,
        template_item6,
        template_item7,
        template_item8,
        template_item9>
            const& in_tuple)
{
    psyq::message_pack::write_tuple(out_stream, in_tuple);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 配列の直列化
//@{
/** @brief std::array をMessagePack形式の配列として直列化し、
           出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_array   直列化するコンテナ。
 */
template<
    typename template_out_stream,
    typename template_value,
    std::size_t template_size>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::array<template_value, template_size> const& in_array)
{
    psyq::message_pack::write_array(out_stream, in_array);
    return out_stream;
}

/** @brief std::vector をMessagePack形式の配列として直列化し、
           出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_vector  直列化するコンテナ。
 */
template<
    typename template_out_stream,
    typename template_value,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::vector<template_value, template_allocator> const& in_vector)
{
    psyq::message_pack::write_array(out_stream, in_vector);
    return out_stream;
}

/** @brief std::deque をMessagePack形式の配列として直列化し、
           出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_deque   直列化するコンテナ。
 */
template<
    typename template_out_stream,
    typename template_value,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::deque<template_value, template_allocator> const& in_deque)
{
    psyq::message_pack::write_array(out_stream, in_deque);
    return out_stream;
}

/** @brief std::list をMessagePack形式の配列として直列化し、
           出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_list    直列化するコンテナ。
 */
template<
    typename template_out_stream,
    typename template_value,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::list<template_value, template_allocator> const& in_list)
{
    psyq::message_pack::write_array(out_stream, in_list);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 集合の直列化
//@{
/** @brief std::set をMessagePack形式の連想配列として直列化し、
           出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_set     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    typename template_key,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::set<template_key, template_compare, template_allocator>
        const& in_set)
{
    psyq::message_pack::write_set(out_stream, in_set);
    return out_stream;
}

/** @brief std::multiset をMessagePack形式の連想配列として直列化し、
           出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_set     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    typename template_key,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::multiset<template_key, template_compare, template_allocator>
        const& in_set)
{
    psyq::message_pack::write_set(out_stream, in_set);
    return out_stream;
}

/** @brief std::unordered_set をMessagePack形式の連想配列として直列化し、
           出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_set     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    typename template_key,
    typename template_hash,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::unordered_set<
        template_key, template_hash, template_compare, template_allocator>
            const& in_set)
{
    psyq::message_pack::write_set(out_stream, in_set);
    return out_stream;
}

/** @brief std::unordered_multiset をMessagePack形式の連想配列として直列化し、
           出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_set     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    typename template_key,
    typename template_hash,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::unordered_multiset<
        template_key, template_hash, template_compare, template_allocator>
            const& in_set)
{
    psyq::message_pack::write_set(out_stream, in_set);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 連想配列の直列化
//@{
/** @brief std::map をMessagePack形式の連想配列として直列化し、
           出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_map     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    typename template_key,
    typename template_mapped,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::map<
        template_key, template_mapped, template_compare, template_allocator>
            const& in_map)
{
    psyq::message_pack::write_map(out_stream, in_map);
    return out_stream;
}

/** @brief std::multimap をMessagePack形式の連想配列として直列化し、
           出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_map     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    typename template_key,
    typename template_mapped,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::multimap<
        template_key, template_mapped, template_compare, template_allocator>
            const& in_map)
{
    psyq::message_pack::write_map(out_stream, in_map);
    return out_stream;
}

/** @brief std::unordered_map をMessagePack形式の連想配列として直列化し、
           出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_map     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    typename template_key,
    typename template_mapped,
    typename template_hash,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::unordered_map<
        template_key,
        template_mapped,
        template_hash,
        template_compare,
        template_allocator>
            const& in_map)
{
    psyq::message_pack::write_map(out_stream, in_map);
    return out_stream;
}

/** @brief std::unordered_multimap をMessagePack形式の連想配列として直列化し、
           出力ストリームへ書き込む。
    @param[out] out_stream 書き込む出力ストリーム。
    @param[in]  in_map     直列化するコンテナ。
 */
template<
    typename template_out_stream,
    typename template_key,
    typename template_mapped,
    typename template_hash,
    typename template_compare,
    typename template_allocator>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::unordered_multimap<
        template_key,
        template_mapped,
        template_hash,
        template_compare,
        template_allocator>
            const& in_map)
{
    psyq::message_pack::write_map(out_stream, in_map);
    return out_stream;
}
//@}
//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        inline void message_pack_serializer()
        {
            psyq::message_pack::serializer<> local_serializer;

            local_serializer
                << std::make_tuple(
                    0.0f,
                    0.0,
                    false,
                    true,
                    std::string("std::string"))
                << (std::numeric_limits<std::uint64_t>::max)();
            local_serializer.get_stream().str().length();

            std::vector<std::int64_t> local_vector;
            local_vector.push_back((std::numeric_limits<std::int64_t>::min)());
            local_vector.push_back((std::numeric_limits<std::int32_t>::min)());
            local_vector.push_back((std::numeric_limits<std::int16_t>::min)());
            local_vector.push_back((std::numeric_limits<std::int8_t>::min)());
            local_vector.push_back(-1);
            local_vector.push_back(0);
            local_vector.push_back((std::numeric_limits<std::uint8_t>::max)());
            local_vector.push_back((std::numeric_limits<std::uint16_t>::max)());
            local_vector.push_back((std::numeric_limits<std::uint32_t>::max)());
            local_vector.push_back((std::numeric_limits<std::int64_t>::max)());
            local_serializer << local_vector;
            local_serializer.get_stream().str().length();

            std::list<std::int64_t> local_list(
                local_vector.begin(), local_vector.end());
            local_serializer << local_list;
            local_serializer.get_stream().str().length();

            std::multiset<std::int64_t> local_set(
                local_vector.begin(), local_vector.end());
            local_serializer << local_set;
            local_serializer.get_stream().str().length();

            std::map<std::string, std::uint64_t> local_map;
            local_map["0x12"] = 0x12;
            local_map["0x1234"] = 0x1234;
            local_map["0x12345678"] = 0x12345678;
            local_map["0x123456789abcdef"] = 0x123456789abcdefL;
            local_serializer << local_map;
            local_serializer.get_stream().str().length();

            local_serializer.write_nil();
            local_serializer.get_stream().str().length();
        }
    }
}

#endif // !defined(PSYQ_MESSAGE_PACK_SERIALIZER_HPP_)
