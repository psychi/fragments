/** @file
    @brief @copybrief psyq::message_pack::serializer
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_MESSAGE_PACK_SERIALIZER_HPP_
#define PSYQ_MESSAGE_PACK_SERIALIZER_HPP_

#include <array>
#include <tuple>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

//#include "psyq/message_pack/endianness.hpp"
//#include "psyq/message_pack/object.hpp"

/// psyq::message_pack::serializer のスタック限界数のデフォルト値。
#ifndef PSYQ_MESSAGE_PACK_SERIALIZER_STACK_CAPACITY_DEFAULT
#define PSYQ_MESSAGE_PACK_SERIALIZER_STACK_CAPACITY_DEFAULT 32
#endif // !defined(PSYQ_MESSAGE_PACK_SERIALIZER_STACK_CAPACITY_DEFAULT)

namespace psyq
{
    namespace message_pack
    {
        /// @cond
        template<
            typename,
            std::size_t = PSYQ_MESSAGE_PACK_SERIALIZER_STACK_CAPACITY_DEFAULT>
                class serializer;
        /// @endcond
    } // namespace message_pack

    /// この名前空間をuserが直接accessするのは禁止。
    namespace internal
    {
        //---------------------------------------------------------------------
        /** @brief MessagePackに格納されている値の形式。

            以下のウェブページにある仕様を参照した。
            https://github.com/msgpack/msgpack/blob/d257d3c143c9fa21ba22afc666fe50cc66943ece/spec.md#formats
         */
        enum message_pack_format
        {
            message_pack_format_FIX_INTEGER_MIN     =-0x20, ///< 最小の固定値整数。
            message_pack_format_FIX_INTEGER_MAX     = 0x7f, ///< 最大の固定値整数。
            message_pack_format_FIX_MAP_MIN         = 0x80, ///< 最小長の固定長連想配列。
            message_pack_format_FIX_MAP_MAX         = 0x8f, ///< 最大長の固定長連想配列。
            message_pack_format_FIX_ARRAY_MIN       = 0x90, ///< 最小長の固定長配列。
            message_pack_format_FIX_ARRAY_MAX       = 0x9f, ///< 最大長の固定長配列。
            message_pack_format_FIX_STRING_MIN      = 0xa0, ///< 最小長の固定長文字列。
            message_pack_format_FIX_STRING_MAX      = 0xbf, ///< 最大長の固定長文字列。
            message_pack_format_NIL                 = 0xc0, ///< 空値。
            message_pack_format_NEVER_USED          = 0xc1, ///< 未使用。
            message_pack_format_FALSE               = 0xc2, ///< false
            message_pack_format_TRUE                = 0xc3, ///< true
            message_pack_format_BINARY_8            = 0xc4, ///< 長さが8bit以下のバイナリ。
            message_pack_format_BINARY_16           = 0xc5, ///< 長さが16bit以下のバイナリ。
            message_pack_format_BINARY_32           = 0xc6, ///< 長さが32bit以下のバイナリ。
            message_pack_format_EXTENDED_8          = 0xc7, ///< 長さが8bit以下の拡張バイナリ。
            message_pack_format_EXTENDED_16         = 0xc8, ///< 長さが16bit以下の拡張バイナリ。
            message_pack_format_EXTENDED_32         = 0xc9, ///< 長さが32bit以下の拡張バイナリ。
            message_pack_format_FLOATING_POINT_32   = 0xca, ///< IEEE754単精度浮動小数点数。
            message_pack_format_FLOATING_POINT_64   = 0xcb, ///< IEEE754倍精度浮動小数点数。
            message_pack_format_UNSIGNED_INTEGER_8  = 0xcc, ///< 0以上の8bit整数。
            message_pack_format_UNSIGNED_INTEGER_16 = 0xcd, ///< 0以上の16bit整数。
            message_pack_format_UNSIGNED_INTEGER_32 = 0xce, ///< 0以上の32bit整数。
            message_pack_format_UNSIGNED_INTEGER_64 = 0xcf, ///< 0以上の64bit整数。
            message_pack_foramt_NEGATIVE_INTEGER_8  = 0xd0, ///< 0未満の8bit整数。
            message_pack_foramt_NEGATIVE_INTEGER_16 = 0xd1, ///< 0未満の16bit整数。
            message_pack_foramt_NEGATIVE_INTEGER_32 = 0xd2, ///< 0未満の32bit整数。
            message_pack_foramt_NEGATIVE_INTEGER_64 = 0xd3, ///< 0未満の64bit整数。
            message_pack_format_FIX_EXTENDED_1      = 0xd4, ///< 長さが1の拡張バイナリ。
            message_pack_format_FIX_EXTENDED_2      = 0xd5, ///< 長さが2の拡張バイナリ。
            message_pack_format_FIX_EXTENDED_4      = 0xd6, ///< 長さが4の拡張バイナリ。
            message_pack_format_FIX_EXTENDED_8      = 0xd7, ///< 長さが8の拡張バイナリ。
            message_pack_format_FIX_EXTENDED_16     = 0xd8, ///< 長さが16の拡張バイナリ。
            message_pack_format_STRING_8            = 0xd9, ///< 長さが8bit以下の文字列。
            message_pack_format_STRING_16           = 0xda, ///< 長さが16bit以下の文字列。
            message_pack_format_STRING_32           = 0xdb, ///< 長さが32bit以下の文字列。
            message_pack_format_ARRAY_16            = 0xdc, ///< 長さが16bit以下の配列。
            message_pack_format_ARRAY_32            = 0xdd, ///< 長さが32bit以下の配列。
            message_pack_format_MAP_16              = 0xde, ///< 長さが16bit以下の連想配列。
            message_pack_format_MAP_32              = 0xdf, ///< 長さが32bit以下の連想配列。
        };

        //---------------------------------------------------------------------
        /** @brief タプルをストリームへ出力する。
            @tparam template_size 出力するタプルの要素数。
         */
        template<std::size_t template_size>
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
                    ::message_pack_tuple_serializer<template_size - 1>
                        ::write(out_stream, in_tuple);
                out_stream << std::get<template_size - 1>(in_tuple);
            }
        };
        /// @copydoc message_pack_tuple_serializer
        template<> struct message_pack_tuple_serializer<0>
        {
            /// タプルの要素数が0なので、何もしない。
            template<typename template_stream, typename template_tuple>
            static void write(template_stream&, template_tuple const&)
            PSYQ_NOEXCEPT
            {}
        };
    } // namespace internal
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief MessagePack形式で値を直列化し、
           std::basic_ostream 互換のストリームへ出力するアダプタ。

    使用例
    @code
    // 文字列出力ストリームを構築する。
    psyq::message_pack::serializer<std::ostringstream> local_serializer;
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

    @tparam template_stream @copydoc psyq::message_pack::serializer::stream
    @tparam template_stack_capacity @copydoc psyq::message_pack::serializer::stack_capacity
 */
template<typename template_stream, std::size_t template_stack_capacity>
class psyq::message_pack::serializer
{
    /// thisが指す値の型。
    private: typedef serializer this_type;

    /// 直列化したMessagePack値を出力する、std::basic_ostream 互換のストリーム。
    public: typedef template_stream stream;
    static_assert(
        sizeof(typename this_type::stream::char_type) == 1,
        "sizeof(this_type::stream::char_type) is not 1.");

    /// 直列化途中のMessagePackコンテナのスタック限界数。
    public: static std::size_t const stack_capacity = template_stack_capacity;

    /// 次に直列化する値の種類。
    public: enum value_kind
    {
        value_kind_ROOT,          ///< 最上位のMessagePackオブジェクト。
        value_kind_RAW_BYTES,     ///< RAWバイト列。
        value_kind_ARRAY_ELEMENT, ///< 配列の要素。
        value_kind_MAP_KEY,       ///< 連想配列の要素のキー。
        value_kind_MAP_VALUE,     ///< 連想配列の要素の値。
    };

    /// @brief 直列化途中のコンテナのスタック。
    private: struct container_stack
    {
        std::size_t rest_count;         ///< MessagePackコンテナ要素の残数。
        typename this_type::value_kind kind; ///< @copydoc value_kind
    };

    //-------------------------------------------------------------------------
    /// @name 構築と破壊
    //@{
    /** @brief 出力ストリームを構築する。
        @param[in] in_ostream MessagePackを書き出すストリームの初期値。
     */
    public: explicit serializer(typename this_type::stream in_ostream):
        stream_(std::move(in_ostream)),
        stack_size_(0)
    {}

    /** @brief move構築子。
        @param[in,out] io_source 移動元インスタンス。
     */
    public: serializer(this_type&& io_source):
        stream_(std::move(io_source.stream_)),
        container_stack_(std::move(io_source.container_stack_)),
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
    public: this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->~this_type();
            return *new(this) this_type(std::move(io_source));
        }
        return *this;
    }
    //@}
    /// copy構築子は使用禁止。
    private: serializer(this_type const&);// = delete;
    /// copy代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);// = delete;

    //-------------------------------------------------------------------------
    /// @name 出力ストリームの操作
    //@{
    /** @brief 出力ストリームを再構築する。
        @param[in] in_ostream 新たに使う出力ストリーム。
        @return これまで使っていた出力ストリーム。
     */
    public: typename this_type::stream reset(typename this_type::stream in_ostream)
    {
        this->stream_.swap(in_ostream);
        this->stack_size_ = 0;
        return in_ostream;
    }

    /** @brief 出力ストリームを取得する。
        @return 出力ストリーム。
     */
    public: typename this_type::stream const& get_stream() const PSYQ_NOEXCEPT
    {
        return this->stream_;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 状態の取得
    //@{
    /** @brief 次に直列化するMessagePack値の種別を取得する。
        @return 次に直列化するMessagePack値の種別。
        @sa this_type::make_serial_string()
        @sa this_type::make_serial_binary()
        @sa this_type::make_serial_extended()
        @sa this_type::make_serial_array()
        @sa this_type::make_serial_map()
     */
    public: typename this_type::value_kind get_next_value_kind() const PSYQ_NOEXCEPT
    {
        return 0 < this->get_rest_container_count()?
            this->container_stack_.at(this->get_rest_container_count() - 1).kind:
            typename this_type::value_kind_ROOT;
    }

    /** @brief 直前に直列化を開始したMessagePackコンテナの残り要素数を取得する。
        @return
            直前に直列化を開始したMessagePackコンテナが…
            - 文字列／バイナリ／拡張バイナリなら、残りバイト数。
            - 配列／連想配列なら、残り要素数。
        @sa this_type::make_serial_string()
        @sa this_type::make_serial_binary()
        @sa this_type::make_serial_extended()
        @sa this_type::make_serial_array()
        @sa this_type::make_serial_map()
     */
    public: std::size_t get_rest_element_count() const PSYQ_NOEXCEPT
    {
        return 0 < this->get_rest_container_count()?
            this->container_stack_.at(this->get_rest_container_count() - 1).rest_count:
            0;
    }

    /** @brief 直列化途中のMessagePackコンテナの数を取得する。
        @return 直列化途中のMessagePackコンテナの数。
        @sa this_type::make_serial_string()
        @sa this_type::make_serial_binary()
        @sa this_type::make_serial_extended()
        @sa this_type::make_serial_array()
        @sa this_type::make_serial_map()
     */
    public: std::size_t get_rest_container_count() const PSYQ_NOEXCEPT
    {
        return this->stack_size_;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 空値の直列化
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
        else if (this->write_big_endian<std::uint8_t>(psyq::internal::message_pack_format_NIL))
        {
            this->update_container_stack();
            return true;
        }
        return false;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 真偽値の直列化
    //@{
    /** @brief 真偽値をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_boolean 直列化する真偽値。
        @return *this
     */
    public: this_type& operator<<(bool const in_boolean)
    {
        if (!this->write_boolean(in_boolean))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }

    /** @brief 真偽値をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_boolean 直列化する真偽値。
        @retval true  成功。
        @retval false 失敗。
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
                    psyq::internal::message_pack_format_TRUE:
                    psyq::internal::message_pack_format_FALSE))
        {
            this->update_container_stack();
            return true;
        }
        return false;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 無符号整数の直列化
    //@{
    /** @brief 無符号整数をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_integer 直列化する無符号整数。
        @return *this
     */
    public: this_type& operator<<(unsigned char const in_integer)
    {
        if (!this->write_unsigned_integer(in_integer))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(unsigned char const)
    public: this_type& operator<<(unsigned short const in_integer)
    {
        if (!this->write_unsigned_integer(in_integer))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(unsigned char const)
    public: this_type& operator<<(unsigned int const in_integer)
    {
        if (!this->write_unsigned_integer(in_integer))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(unsigned char const)
    public: this_type& operator<<(unsigned long const in_integer)
    {
        if (!this->write_unsigned_integer(in_integer))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(unsigned char const)
    public: this_type& operator<<(unsigned long long const in_integer)
    {
        if (!this->write_unsigned_integer(in_integer))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }

    /** @brief 無符号整数をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_integer 直列化する整数。
        @retval true  成功。
        @retval false 失敗。
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
            if (psyq::internal::message_pack_format_FIX_INTEGER_MAX < in_integer
                && !this->write_big_endian<std::uint8_t>(psyq::internal::message_pack_format_UNSIGNED_INTEGER_8))
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
            if (!this->write_big_endian<std::uint8_t>(psyq::internal::message_pack_format_UNSIGNED_INTEGER_16)
                || !this->write_big_endian(static_cast<std::uint16_t>(in_integer)))
            {
                return false;
            }
        }
        else if (in_integer <= (std::numeric_limits<std::uint32_t>::max)())
        {
            if (!this->write_big_endian<std::uint8_t>(psyq::internal::message_pack_format_UNSIGNED_INTEGER_32)
                || !this->write_big_endian(static_cast<std::uint32_t>(in_integer)))
            {
                return false;
            }
        }
        else
        {
            PSYQ_ASSERT(
                in_integer <= (std::numeric_limits<std::uint64_t>::max)());
            if (!this->write_big_endian<std::uint8_t>(psyq::internal::message_pack_format_UNSIGNED_INTEGER_64)
                || !this->write_big_endian(static_cast<std::uint64_t>(in_integer)))
            {
                return false;
            }
        }
        this->update_container_stack();
        return true;
    }
    //@}
    /** @brief 数値を直列化し、ストリームへ出力する。
        @param[in] in_value 直列化する数値。
        @retval true  成功。
        @retval false 失敗。
     */
    private: template<typename template_value>
    bool write_big_endian(template_value const in_value)
    {
        return psyq::message_pack::endianness_converter<template_value>
            ::write_value(this->stream_, in_value, psyq::message_pack::big_endian);
    }

    //-------------------------------------------------------------------------
    /// @name 有符号整数の直列化
    //@{
    /** @brief 有符号整数をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_integer 直列化する有符号整数。
        @return *this
     */
    public: this_type& operator<<(signed char const in_integer)
    {
        if (!this->write_signed_integer(in_integer))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(signed char const)
    public: this_type& operator<<(signed short const in_integer)
    {
        if (!this->write_signed_integer(in_integer))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(signed char const)
    public: this_type& operator<<(signed int const in_integer)
    {
        if (!this->write_signed_integer(in_integer))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(signed char const)
    public: this_type& operator<<(signed long const in_integer)
    {
        if (!this->write_signed_integer(in_integer))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(signed char const)
    public: this_type& operator<<(signed long long const in_integer)
    {
        if (!this->write_signed_integer(in_integer))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }

    /** @brief 有符号整数をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_integer 直列化する整数。
        @retval true  成功。
        @retval false 失敗。
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
            if (in_integer < psyq::internal::message_pack_format_FIX_INTEGER_MIN
                && !this->write_big_endian<std::uint8_t>(psyq::internal::message_pack_foramt_NEGATIVE_INTEGER_8))
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
            if (!this->write_big_endian<std::uint8_t>(psyq::internal::message_pack_foramt_NEGATIVE_INTEGER_16)
                || !this->write_big_endian(static_cast<std::int16_t>(in_integer)))
            {
                return false;
            }
        }
        else if ((std::numeric_limits<std::int32_t>::min)() <= in_integer)
        {
            if (!this->write_big_endian<std::uint8_t>(psyq::internal::message_pack_foramt_NEGATIVE_INTEGER_32)
                || !this->write_big_endian(static_cast<std::int32_t>(in_integer)))
            {
                return false;
            }
        }
        else
        {
            PSYQ_ASSERT(
                (std::numeric_limits<std::int64_t>::min)() <= in_integer);
            if (!this->write_big_endian<std::uint8_t>(psyq::internal::message_pack_foramt_NEGATIVE_INTEGER_64)
                || !this->write_big_endian(static_cast<std::int64_t>(in_integer)))
            {
                return false;
            }
        }
        this->update_container_stack();
        return true;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 浮動小数点数の直列化
    //@{
    /** @brief 浮動小数点数をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_float 直列化する浮動小数点数。
        @return *this
     */
    public: this_type& operator<<(float const in_float)
    {
        if (!this->write_floating_point(in_float))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(float const)
    public: this_type& operator<<(double const in_float)
    {
        if (!this->write_floating_point(in_float))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }

    /** @brief 浮動小数点数をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_float 直列化する浮動小数点数。
        @retval true  成功。
        @retval false 失敗。
     */
    public: bool write_floating_point(float const in_float)
    {
        return this->write_floating_point<psyq::internal::message_pack_format_FLOATING_POINT_32>(in_float);
    }
    /// @copydoc write_floating_point(float const)
    public: bool write_floating_point(double const in_float)
    {
        return this->write_floating_point<psyq::internal::message_pack_format_FLOATING_POINT_64>(in_float);
    }
    //@}
    /// @copydoc write_floating_point(float const)
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

    //-------------------------------------------------------------------------
    /// @name MessagePack文字列への直列化
    //@{
    /** @brief 文字列literalをMessagePack形式の文字列として直列化し、
               ストリームへ出力する。
        @tparam template_size 文字列literalの要素数。終端文字も含む。
        @param[in] in_string 直列化する文字列literal。
        @return *this
        @warning 文字列literal以外の文字列を引数に渡すのは禁止。
        @note
            引数が文字列literalであることを保証するため、
            user定義literalを経由して呼び出すようにしたい。
     */
    public: template <typename template_char, std::size_t template_size>
    this_type& operator<<(template_char const (&in_string)[template_size])
    {
        if (!this->write_raw_string(in_string))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /** @brief std::basic_string をMessagePack形式の文字列として直列化し、
               ストリームへ出力する。
        @param[in] in_string  直列化する文字列。
        @return *this
     */
    public: template<
        typename template_char,
        typename template_traits,
        typename template_allocator>
    this_type& operator<<(
        std::basic_string<template_char, template_traits, template_allocator>
            const& in_string)
    {
        if (!this->write_raw_string(in_string))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
#ifdef PSYQ_STRING_VIEW_BASE_HPP_
    /** @brief 文字列をMessagePack形式の文字列として直列化し、
               ストリームへ出力する。
        @param[in] in_string 直列化する文字列。
        @return *this
     */
    public: template<typename template_char_traits>
    this_type& operator<<(
        psyq::internal::string_view_base<template_char_traits> const& in_string)
    {
        if (!this->write_raw_string(in_string))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
#endif // defined(PSYQ_STRING_VIEW_BASE_HPP_)

    /** @brief 文字列literalをMessagePack形式の文字列として直列化し、
               ストリームへ出力する。
        @tparam template_size 文字列literalの要素数。終端文字も含む。
        @param[in] in_string 直列化する文字列literal。
        @retval true  成功。
        @retval false 失敗。
        @warning 文字列literal以外の文字列を引数に渡すのは禁止。
        @note
            引数が文字列literalであることを保証するため、
            user定義literalを経由して呼び出すようにしたい。
     */
    public: template <typename template_char, std::size_t template_size>
    bool write_raw_string(template_char const (&in_string)[template_size])
    {
        static_assert(0 < template_size, "");
        PSYQ_ASSERT(in_string[template_size - 1] == 0);
        return this->write_raw_string(&in_string[0], template_size - 1);
    }
    /** @brief 連続するメモリ領域にある文字列を、
               MessagePack形式の文字列として直列化し、ストリームへ出力する。
        @param[in] in_string
            直列化する文字列。以下の関数を使える必要がある。
            @code
            // 文字列の先頭位置を取得する。
            in_string.data();
            // 文字列の要素数を取得する。
            in_string.size();
            @endcode
        @retval true  成功。
        @retval false 失敗。
     */
    public: template<typename template_string>
    bool write_raw_string(template_string const& in_string)
    {
        return this->write_raw_string(in_string.data(), in_string.size());
    }
    /** @brief 連続するメモリ領域にある文字列を、
               MessagePack形式の文字列として直列化し、ストリームへ出力する。
        @param[in] in_data 直列化する文字列の先頭位置。
        @param[in] in_size 直列化する文字列の要素数。
        @retval true  成功。
        @retval false 失敗。
     */
    public: template<typename template_char>
    bool write_raw_string(
        template_char const* const in_data,
        std::size_t const in_size)
    {
        static_assert(
            // MessagePack文字列はUTF-8なので、文字は1バイト単位となる。
            sizeof(template_char) == 1, "MessagePack string is only UTF-8.");
        auto const local_size(in_size * sizeof(template_char));
        return this->write_string_header(local_size)
            && this->write_raw_data(in_data, local_size);
    }

    /** @brief 標準コンテナをMessagePack形式の文字列として直列化し、
               ストリームへ出力する。
        @param[in] in_container 直列化する標準コンテナ。
        @retval true  成功。
        @retval false 失敗。
     */
    public: template<typename template_container>
    bool write_container_string(template_container const& in_container)
    {
        return this->write_container_string(
            in_container.begin(), in_container.size());
    }
    /** @brief 標準コンテナをMessagePack形式の文字列として直列化し、
               ストリームへ出力する。
        @param[in] in_begin  直列化する標準コンテナの先頭位置。
        @param[in] in_length 直列化する標準コンテナの要素数。
        @retval true  成功。
        @retval false 失敗。
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
        return this->make_serial_string(in_length)
            && this->fill_container_raw(in_begin, in_length, psyq::message_pack::big_endian) == 0;
    }

    /** @brief MessagePack文字列の直列化を開始する。

        以後、 in_size バイトの文字を、 this_type::fill_container_raw() で直列化できる。

        @param[in] in_size 直列化するMessagePack文字列のバイト数。
        @sa this_type::fill_rest_elements()
     */
    public: bool make_serial_string(std::size_t const in_size)
    {
        if (in_size <= 0)
        {
            // 空の文字列を直列化する。
            if (this->write_big_endian<std::uint8_t>(psyq::internal::message_pack_format_FIX_STRING_MIN))
            {
                this->update_container_stack();
                return true;
            }
        }
        else if (this_type::stack_capacity <= this->get_rest_container_count())
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
        }
        else if (this->write_string_header(in_size))
        {
            // RAWバイト列をスタックに積む。
            auto& local_stack(
                this->container_stack_.at(this->get_rest_container_count()));
            local_stack.kind = this_type::value_kind_RAW_BYTES;
            local_stack.rest_count = in_size;
            ++this->stack_size_;
            return true;
        }
        return false;
    }
    //@}
    /** @brief 文字列のバイト数を書き込む。
        @param[in] in_size 文字列のバイト数。
        @retval true  成功。
        @retval false 失敗。
     */
    private: bool write_string_header(std::size_t const in_size)
    {
        unsigned const local_fix_size(
            psyq::internal::message_pack_format_FIX_STRING_MAX
                - psyq::internal::message_pack_format_FIX_STRING_MIN);
        if (local_fix_size < in_size)
        {
            return this->write_raw_header<psyq::internal::message_pack_format_STRING_8>(in_size);
        }
        else if (this->get_stack_top_raw() == nullptr)
        {
            return this->write_big_endian(
                static_cast<std::uint8_t>(
                    psyq::internal::message_pack_format_FIX_STRING_MIN + in_size));
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
        @retval true  成功。
        @retval false 失敗。
     */
    public: template<typename template_element>
    bool write_raw_binary(
        template_element const* const in_begin,
        std::size_t const in_length)
    {
        auto const local_size(in_length * sizeof(template_element));
        return this->write_raw_header<psyq::internal::message_pack_format_BINARY_8>(local_size)
            && this->write_raw_data(in_begin, local_size);
    }

    /** @brief 標準コンテナをMessagePack形式のバイナリとして直列化し、
               ストリームへ出力する。
        @param[in] in_container  直列化する標準コンテナ。
        @param[in] in_endianness コンテナ要素を直列化する際のエンディアン性。
        @retval true  成功。
        @retval false 失敗。
     */
    public: template<typename template_container>
    bool write_container_binary(
        template_container const& in_container,
        psyq::message_pack::endianness const in_endianness
            = psyq::message_pack::big_endian)
    {
        return this->write_container_binary(
            in_container.begin(), in_container.size(), in_endianness);
    }
    /** @brief 標準コンテナをMessagePack形式のバイナリとして直列化し、
               ストリームへ出力する。
        @param[in] in_begin      直列化する標準コンテナの先頭位置。
        @param[in] in_length     直列化する標準コンテナの要素数。
        @param[in] in_endianness 要素を直列化する際のエンディアン性。
        @retval true  成功。
        @retval false 失敗。
     */
    public: template<typename template_iterator>
    bool write_container_binary(
        template_iterator const& in_begin,
        std::size_t const in_length,
        psyq::message_pack::endianness const in_endianness
            = psyq::message_pack::big_endian)
    {
        typedef typename std::iterator_traits<template_iterator>::value_type
            element;
        return this->make_serial_binary<element>(in_length)
            && this->fill_container_raw(in_begin, in_length, in_endianness) == 0;
    }

    /** @brief MessagePackバイナリの直列化を開始する。

        以後 in_length 個の要素を、 this_type::fill_container_raw() で直列化できる。

        @tparam template_element 直列化する標準コンテナの要素の型。
        @param[in] in_length 直列化する標準コンテナの要素数。
        @sa this_type::fill_rest_elements()
     */
    public: template<typename template_element>
    bool make_serial_binary(std::size_t const in_length)
    {
        auto const local_size(in_length * sizeof(template_element));
        if (local_size <= 0)
        {
            if (this->write_raw_header<psyq::internal::message_pack_format_BINARY_8>(0))
            {
                this->update_container_stack();
                return true;
            }
        }
        else if (this_type::stack_capacity <= this->get_rest_container_count())
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
        }
        else if (this->write_raw_header<psyq::internal::message_pack_format_BINARY_8>(local_size))
        {
            // RAWバイト列をスタックに積む。
            auto& local_stack(
                this->container_stack_.at(this->get_rest_container_count()));
            local_stack.kind = this_type::value_kind_RAW_BYTES;
            local_stack.rest_count = local_size;
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
        @param[in] in_type       直列化する値の拡張型識別値。
        @param[in] in_value      直列化する値。
        @param[in] in_endianness 値を直列化する際のエンディアン性。
        @retval true  成功。
        @retval false 失敗。
     */
    public: template<typename template_value>
    bool write_extended(
        std::int8_t const in_type,
        template_value const& in_value,
        psyq::message_pack::endianness const in_endianness
            = psyq::message_pack::big_endian)
    {
        return this->write_extended_header(in_type, sizeof(template_value))
            && psyq::message_pack::endianness_converter<template_value>
                ::write_value(this->stream_, in_value, in_endianness);
    }
    /** @brief 長さが0の拡張バイナリを直列化し、ストリームへ出力する。
        @param[in] in_type 直列化する拡張型識別値。
        @retval true  成功。
        @retval false 失敗。
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

        以後 in_size バイトを、 this_type::fill_container_raw() で直列化できる。

        @param[in] in_type 直列化する拡張型識別値。
        @param[in] in_size 直列化する拡張バイナリのバイト数。
        @sa this_type::fill_rest_elements()
     */
    public: bool make_serial_extended(
        std::int8_t const in_type,
        std::size_t const in_size)
    {
        if (in_size <= 0)
        {
            return this->write_extended(in_type);
        }
        else if (this_type::stack_capacity <= this->get_rest_container_count())
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
        }
        else if (this->write_extended_header(in_type, in_size))
        {
            // RAWバイト列をスタックに積む。
            auto& local_stack(
                this->container_stack_.at(this->get_rest_container_count()));
            local_stack.kind = this_type::value_kind_RAW_BYTES;
            local_stack.rest_count = in_size;
            ++this->stack_size_;
            return true;
        }
        return false;
    }
    //@}
    /** @brief 拡張バイナリのバイト数と型識別値を書き込む。
        @param[in] in_size 拡張バイナリのバイト数。
        @param[in] in_type 拡張型識別値。
        @retval true  成功。
        @retval false 失敗。
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
            local_header = psyq::internal::message_pack_format_FIX_EXTENDED_1;
            goto PSYQ_MESSAGE_PACK_SERIALIZER_WRITE_EXTENDED_BINARY_SIZE;
        case 2:
            local_header = psyq::internal::message_pack_format_FIX_EXTENDED_2;
            goto PSYQ_MESSAGE_PACK_SERIALIZER_WRITE_EXTENDED_BINARY_SIZE;
        case 4:
            local_header = psyq::internal::message_pack_format_FIX_EXTENDED_4;
            goto PSYQ_MESSAGE_PACK_SERIALIZER_WRITE_EXTENDED_BINARY_SIZE;
        case 8:
            local_header = psyq::internal::message_pack_format_FIX_EXTENDED_8;
            goto PSYQ_MESSAGE_PACK_SERIALIZER_WRITE_EXTENDED_BINARY_SIZE;
        case 16:
            local_header = psyq::internal::message_pack_format_FIX_EXTENDED_16;
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
            local_write = this->write_raw_header<psyq::internal::message_pack_format_EXTENDED_8>(in_size);
            break;
        }

        // 拡張型識別値を直列化する。
        return local_write? this->write_big_endian(in_type): false;
    }

    //-------------------------------------------------------------------------
    /// @name タプルの直列化
    //@{
    /** @brief std::tuple をMessagePack形式の配列として直列化し、
               ストリームへ出力する。
        @param[in] in_tuple 直列化するタプル。
        @return *this
        @note 2014.05.24
              可変長テンプレート引数が使えるようになったら、対応したい。
     */
    public: template<typename template_element0>
    this_type& operator<<(std::tuple<template_element0> const& in_tuple)
    {
        if (!this->write_tuple(in_tuple))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(std::tuple<template_element0> const&)
    public: template<typename template_element0, typename template_element1>
    this_type& operator<<(
        std::tuple<template_element0, template_element1> const& in_tuple)
    {
        this->write_tuple(in_tuple);
        return *this;
    }
    /// @copydoc operator<<(std::tuple<template_element0> const&)
    public: template<
        typename template_element0,
        typename template_element1,
        typename template_element2>
    this_type& operator<<(
        std::tuple<template_element0, template_element1, template_element2>
            const& in_tuple)
    {
        if (!this->write_tuple(in_tuple))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(std::tuple<template_element0> const&)
    public: template<
        typename template_element0,
        typename template_element1,
        typename template_element2,
        typename template_element3>
    this_type& operator<<(
        std::tuple<
            template_element0,
            template_element1,
            template_element2,
            template_element3>
                const& in_tuple)
    {
        if (!this->write_tuple(in_tuple))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(std::tuple<template_element0> const&)
    public: template<
        typename template_element0,
        typename template_element1,
        typename template_element2,
        typename template_element3,
        typename template_element4>
    this_type& operator<<(
        std::tuple<
            template_element0,
            template_element1,
            template_element2,
            template_element3,
            template_element4>
                const& in_tuple)
    {
        if (!this->write_tuple(in_tuple))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(std::tuple<template_element0> const&)
    public: template<
        typename template_element0,
        typename template_element1,
        typename template_element2,
        typename template_element3,
        typename template_element4,
        typename template_element5>
    this_type& operator<<(
        std::tuple<
            template_element0,
            template_element1,
            template_element2,
            template_element3,
            template_element4,
            template_element5>
                const& in_tuple)
    {
        if (!this->write_tuple(in_tuple))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(std::tuple<template_element0> const&)
    public: template<
        typename template_element0,
        typename template_element1,
        typename template_element2,
        typename template_element3,
        typename template_element4,
        typename template_element5,
        typename template_element6>
    this_type& operator<<(
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
        if (!this->write_tuple(in_tuple))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(std::tuple<template_element0> const&)
    public: template<
        typename template_element0,
        typename template_element1,
        typename template_element2,
        typename template_element3,
        typename template_element4,
        typename template_element5,
        typename template_element6,
        typename template_element7>
    this_type& operator<<(
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
        if (!this->write_tuple(in_tuple))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(std::tuple<template_element0> const&)
    public: template<
        typename template_element0,
        typename template_element1,
        typename template_element2,
        typename template_element3,
        typename template_element4,
        typename template_element5,
        typename template_element6,
        typename template_element7,
        typename template_element8>
    this_type& operator<<(
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
        if (!this->write_tuple(in_tuple))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /// @copydoc operator<<(std::tuple<template_element0> const&)
    public: template<
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
    this_type& operator<<(
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
        if (!this->write_tuple(in_tuple))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }

    /** @brief タプルをMessagePack形式の配列として直列化し、
               ストリームへ出力する。
        @tparam template_tuple std::tuple 互換のタプル型。
        @param[in] in_tuple 直列化するタプル。
        @retval true  成功。
        @retval false 失敗。
     */
    public: template<typename template_tuple>
    bool write_tuple(template_tuple const& in_tuple)
    {
        this->make_serial_array(std::tuple_size<template_tuple>::value);
        psyq::internal::message_pack_tuple_serializer
            <std::tuple_size<template_tuple>::value>
                ::write(*this, in_tuple);
        return !this->stream_.fail();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePack配列への直列化
    //@{
    /** @brief std::array をMessagePack形式の配列として直列化し、
               ストリームへ出力する。
        @param[in] in_array 直列化するコンテナ。
        @return *this
     */
    public: template<typename template_value, std::size_t template_size>
    this_type& operator<<(std::array<template_value, template_size> const& in_array)
    {
        this->write_array(in_array);
        return *this;
    }

    /** @brief std::vector をMessagePack形式の配列として直列化し、
              ストリームへ出力する。
        @param[in] in_vector 直列化するコンテナ。
        @return *this
     */
    public: template<typename template_value, typename template_allocator>
    this_type& operator<<(
        std::vector<template_value, template_allocator> const& in_vector)
    {
        this->write_array(in_vector);
        return *this;
    }

    /** @brief std::deque をMessagePack形式の配列として直列化し、
               ストリームへ出力する。
        @param[in] in_deque 直列化するコンテナ。
        @return *this
     */
    public: template<typename template_value, typename template_allocator>
    this_type& operator<<(
        std::deque<template_value, template_allocator> const& in_deque)
    {
        this->write_array(in_deque);
        return *this;
    }

    /** @brief std::list をMessagePack形式の配列として直列化し、
               ストリームへ出力する。
        @param[in] in_list 直列化するコンテナ。
        @return *this
     */
    public: template<typename template_value, typename template_allocator>
    this_type& operator<<(
        std::list<template_value, template_allocator> const& in_list)
    {
        this->write_array(in_list);
        return *this;
    }

    /** @brief 標準コンテナをMessagePack形式の配列として直列化し、
               ストリームへ出力する。
        @param[in] in_container 直列化する標準コンテナ。
        @retval true  成功。
        @retval false 失敗。
     */
    public: template<typename template_container>
    bool write_array(template_container const& in_container)
    {
        return this->write_array(in_container.begin(), in_container.size());
    }
    /** @brief 標準コンテナをMessagePack形式の配列として直列化し、
               ストリームへ出力する。
        @param[in] in_iterator 直列化する標準コンテナの先頭位置。
        @param[in] in_length   直列化する標準コンテナの要素数。
        @retval true  成功。
        @retval false 失敗。
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
        return !this->stream_.fail();
    }

    /** @brief MessagePack形式の配列の直列化を開始する。

        以後 in_length 個のMessagePack値を、配列の要素として直列化できる。

        以下の関数を1回呼び出す毎に、MessagePack値を1つ直列化する。
        - this_type::write_nil()
        - this_type::write_boolean()
        - this_type::write_unsigned_integer()
        - this_type::write_signed_integer()
        - this_type::write_floating_point()
        - this_type::write_raw_string()
        - this_type::write_raw_binary()
        - this_type::write_container_string()
        - this_type::write_container_binary()
        - this_type::write_extended()
        - this_type::write_tuple()
        - this_type::write_array()
        - this_type::write_set()
        - this_type::write_map()
        - this_type::make_serial_string()
        - this_type::make_serial_binary()
        - this_type::make_serial_extended()
        - this_type::make_serial_array()
        - this_type::make_serial_map()

        @param[in] in_length 直列化する配列の要素数。
        @sa this_type::fill_rest_elements()
     */
    public: bool make_serial_array(std::size_t const in_length)
    {
        return this->make_serial_container<
            this_type::value_kind_ARRAY_ELEMENT,
            psyq::internal::message_pack_format_ARRAY_16,
            psyq::internal::message_pack_format_FIX_ARRAY_MIN,
            psyq::internal::message_pack_format_FIX_ARRAY_MAX>(
                in_length);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePack連想配列への直列化
    //@{
    /** @brief std::set をMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @param[in] in_set 直列化するコンテナ。
        @return *this
     */
    public: template<
        typename template_key,
        typename template_compare,
        typename template_allocator>
    this_type& operator<<(
        std::set<template_key, template_compare, template_allocator>
            const& in_set)
    {
        if (!this->write_set(in_set))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /** @brief std::multiset をMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @param[in] in_set 直列化するコンテナ。
        @return *this
     */
    public: template<
        typename template_key,
        typename template_compare,
        typename template_allocator>
    this_type& operator<<(
        std::multiset<template_key, template_compare, template_allocator>
            const& in_set)
    {
        if (!this->write_set(in_set))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /** @brief std::unordered_set をMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @param[in] in_set 直列化するコンテナ。
        @return *this
     */
    public: template<
        typename template_key,
        typename template_hash,
        typename template_compare,
        typename template_allocator>
    this_type& operator<<(
        std::unordered_set
            <template_key, template_hash, template_compare, template_allocator>
                const& in_set)
    {
        if (!this->write_set(in_set))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /** @brief std::unordered_multiset をMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @param[in] in_set 直列化するコンテナ。
        @return *this
     */
    public: template<
        typename template_key,
        typename template_hash,
        typename template_compare,
        typename template_allocator>
    this_type& operator<<(
        std::unordered_multiset
            <template_key, template_hash, template_compare, template_allocator>
                const& in_set)
    {
        if (!this->write_set(in_set))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /** @brief std::map をMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @param[in] in_map 直列化するコンテナ。
        @return *this
     */
    public: template<
        typename template_key,
        typename template_mapped,
        typename template_compare,
        typename template_allocator>
    this_type& operator<<(
        std::map<template_key, template_mapped, template_compare, template_allocator>
            const& in_map)
    {
        if (!this->write_map(in_map))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /** @brief std::multimap をMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @param[in] in_map 直列化するコンテナ。
        @return *this
     */
    public: template<
        typename template_key,
        typename template_mapped,
        typename template_compare,
        typename template_allocator>
    this_type& operator<<(
        std::multimap
            <template_key, template_mapped, template_compare, template_allocator>
                const& in_map)
    {
        if (!this->write_map(in_map))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /** @brief std::unordered_map をMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @param[in] in_map 直列化するコンテナ。
        @return *this
     */
    public: template<
        typename template_key,
        typename template_mapped,
        typename template_hash,
        typename template_compare,
        typename template_allocator>
    this_type& operator<<(
        std::unordered_map<
            template_key,
            template_mapped,
            template_hash,
            template_compare,
            template_allocator>
                const& in_map)
    {
        if (!this->write_map(in_map))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /** @brief std::unordered_multimap をMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @param[in] in_map 直列化するコンテナ。
        @return *this
     */
    template<
        typename template_key,
        typename template_mapped,
        typename template_hash,
        typename template_compare,
        typename template_allocator>
    this_type& operator<<(
        std::unordered_multimap<
            template_key,
            template_mapped,
            template_hash,
            template_compare,
            template_allocator>
                const& in_map)
    {
        if (!this->write_map(in_map))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }

    /** @brief 標準コンテナをMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @param[in] in_set 直列化する標準コンテナ。
        @retval true  成功。
        @retval false 失敗。
     */
    public: template<typename template_set>
    bool write_set(template_set const& in_set)
    {
        return this->write_set(in_set.begin(), in_set.size());
    }
    /** @brief 標準コンテナをMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @param[in] in_iterator 直列化する標準コンテナの先頭位置。
        @param[in] in_length   直列化する標準コンテナの要素数。
        @retval true  成功。
        @retval false 失敗。
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
        return !this->stream_.fail();
    }

    /** @brief pairコンテナをMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @tparam template_map std::pair 互換の要素を持つ標準コンテナの型。
        @param[in] in_map 直列化するpairコンテナ。
        @retval true  成功。
        @retval false 失敗。
     */
    public: template<typename template_map>
    bool write_map(template_map const& in_map)
    {
        return this->write_map(in_map.begin(), in_map.size());
    }
    /** @brief pairコンテナをMessagePack形式の連想配列として直列化し、
               ストリームへ出力する。
        @tparam template_iterator std::pair 互換の要素を指す反復子の型。
        @param[in] in_iterator 直列化するpairコンテナの先頭位置。
        @param[in] in_length   直列化するpairコンテナの要素数。
        @retval true  成功。
        @retval false 失敗。
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
            auto const& local_pair(*in_iterator);
            *this << local_pair.first << local_pair.second;
        }
        return !this->stream_.fail();
    }

    /** @brief MessagePack形式の連想配列の直列化を開始する。

        以後 in_length*2 個のMessagePack値を、連想配列の要素として直列化できる。
        2つのMessagePack値で、連想配列の要素1つとなる。
        最初のMessagePack値を連想配列のキー、
        次のMessagePack値を連想配列のマップ値として直列化する。

        以下の関数を1回呼び出す毎に、MessagePack値を1つ直列化する。
        - this_type::write_nil()
        - this_type::write_boolean()
        - this_type::write_unsigned_integer()
        - this_type::write_signed_integer()
        - this_type::write_floating_point()
        - this_type::write_raw_string()
        - this_type::write_raw_binary()
        - this_type::write_container_string()
        - this_type::write_container_binary()
        - this_type::write_extended()
        - this_type::write_tuple()
        - this_type::write_array()
        - this_type::write_set()
        - this_type::write_map()
        - this_type::make_serial_string()
        - this_type::make_serial_binary()
        - this_type::make_serial_extended()
        - this_type::make_serial_array()
        - this_type::make_serial_map()

        @param[in] in_length 直列化する連想配列の要素数。
        @sa this_type::fill_rest_elements()
     */
    public: bool make_serial_map(std::size_t const in_length)
    {
        return this->make_serial_container<
            this_type::value_kind_MAP_KEY,
            psyq::internal::message_pack_format_MAP_16,
            psyq::internal::message_pack_format_FIX_MAP_MIN,
            psyq::internal::message_pack_format_FIX_MAP_MAX>(
                in_length);
    }
    //@}
#if 0
    /** @brief std::pair をMessagePack形式で直列化し、ストリームへ出力する。
        @param[in] in_pair 直列化する std::pair インスタンス。
        @return *this
     */
    private: template<typename template_first, typename template_second>
    this_type& operator<<(std::pair<template_first, template_second> const& in_pair)
    {
        return *this << in_pair.first << in_pair.second;
    }
#endif

    //-------------------------------------------------------------------------
    /// @name MessagePackコンテナ要素への直列化
    //@{
    /** @brief 直前に直列化を開始した文字列／バイナリ／拡張バイナリの残り要素に、
               MessagePack形式のRAWバイト列として値を直列化し、
               ストリームへ出力する。
        @param[in] in_value      直列化する値。
        @param[in] in_endianness コンテナ要素を直列化する際のエンディアン性。
        @sa this_type::make_serial_string()
        @sa this_type::make_serial_binary()
        @sa this_type::make_serial_extended()
     */
    public: template<typename template_value>
    std::size_t fill_value_raw(
        template_value const& in_value,
        psyq::message_pack::endianness const in_endianness
            = psyq::message_pack::big_endian)
    {
        return this->fill_container_raw(&in_value, 1, in_endianness);
    }

    /** @brief 直前に直列化を開始した文字列／バイナリ／拡張バイナリの残り要素に
               標準コンテナの要素をMessagePack形式のRAWバイト列として直列化し、
               ストリームへ出力する。
        @param[in] in_container  直列化する標準コンテナ。
        @param[in] in_endianness コンテナ要素を直列化する際のエンディアン性。
        @return 残り要素の数。
        @sa this_type::make_serial_string()
        @sa this_type::make_serial_binary()
        @sa this_type::make_serial_extended()
     */
    public: template<typename template_container>
    std::size_t fill_container_raw(
        template_container const& in_container,
        psyq::message_pack::endianness const in_endianness
            = psyq::message_pack::big_endian)
    {
        return this->fill_container_raw(
            in_container.begin(), in_container.size(), in_endianness);
    }

    /** @brief 直前に直列化を開始した文字列／バイナリ／拡張バイナリの残り要素に
               標準コンテナの要素をMessagePack形式のRAWバイト列として直列化し、
               ストリームへ出力する。
        @param[in] in_iterator   直列化する標準コンテナの先頭位置。
        @param[in] in_length     直列化する標準コンテナの要素数。
        @param[in] in_endianness コンテナ要素を直列化する際のエンディアン性。
        @return 残りバイト数。
        @sa this_type::make_serial_string()
        @sa this_type::make_serial_binary()
        @sa this_type::make_serial_extended()
     */
    public: template<typename template_iterator>
    std::size_t fill_container_raw(
        template_iterator in_iterator,
        std::size_t in_length,
        psyq::message_pack::endianness const in_endianness
            = psyq::message_pack::big_endian)
    {
        auto const local_stack(this->get_stack_top_raw());
        if (local_stack == nullptr)
        {
            PSYQ_ASSERT(false);
            return 0;
        }
        typedef typename std::iterator_traits<template_iterator>::value_type
            element;
        PSYQ_ASSERT(sizeof(element) * in_length <= local_stack->rest_count);
        for (;; --in_length, ++in_iterator)
        {
            if (local_stack->rest_count < sizeof(element))
            {
                PSYQ_ASSERT(local_stack->rest_count == 0);
                return this->fill_rest_elements();
            }
            else if (
                in_length <= 0
                || !psyq::message_pack::endianness_converter<element>
                    ::write_value(this->stream_, *in_iterator, in_endianness))
            {
                return local_stack->rest_count;
            }
            local_stack->rest_count -= sizeof(element);
        }
    }

    /** @brief 直前に直列化を開始したMessagePackコンテナの残り要素を、
               空値で埋める。
        @return 残りバイト数。0以外の場合は、途中で失敗している。
        @sa this_type::make_serial_string()
        @sa this_type::make_serial_binary()
        @sa this_type::make_serial_extended()
        @sa this_type::make_serial_array()
        @sa this_type::make_serial_map()
        @sa this_type::fill_rest_containers()
     */
    public: std::size_t fill_rest_elements()
    {
        if (this->get_rest_container_count() <= 0)
        {
            return 0;
        }

        // 残りの要素数を決定する。
        auto& local_stack(
            this->container_stack_.at(this->get_rest_container_count() - 1));
        std::size_t local_empty_count;
        std::uint8_t local_empty_value;
        switch (local_stack.kind)
        {
        case this_type::value_kind_RAW_BYTES:
            local_empty_value = 0;
            local_empty_count = local_stack.rest_count;
            break;

        case this_type::value_kind_ARRAY_ELEMENT:
            local_empty_value = psyq::internal::message_pack_format_NIL;
            local_empty_count = local_stack.rest_count;
            break;

        case this_type::value_kind_MAP_KEY:
            local_empty_value = psyq::internal::message_pack_format_NIL;
            local_empty_count = local_stack.rest_count * 2;
            break;

        case this_type::value_kind_MAP_VALUE:
            local_empty_value = psyq::internal::message_pack_format_NIL;
            local_empty_count = local_stack.rest_count * 2 + 1;
            break;

        default:
            PSYQ_ASSERT(false);
            return local_stack.rest_count;
        }

        // コンテナの残り要素を埋める。
        for (; 0 < local_empty_count; --local_empty_count)
        {
            if (!this->write_big_endian(local_empty_value))
            {
                PSYQ_ASSERT(false);
                return local_empty_count;
            }
        }
        --this->stack_size_;
        this->update_container_stack();
        return 0;
    }

    /** @brief 直列化を終了してないすべてのMessagePackコンテナの残り要素を、
               空値で埋める。
        @sa this_type::make_serial_string()
        @sa this_type::make_serial_binary()
        @sa this_type::make_serial_extended()
        @sa this_type::make_serial_array()
        @sa this_type::make_serial_map()
        @sa this_type::fill_rest_elements()
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
    /// @name MessagePackオブジェクトの直列化
    //@{
    /** @brief MessagePackオブジェクトを直列化し、ストリームへ出力する。
        @param[in] in_object 直列化するMessagePackオブジェクト。
        @return *this
     */
    public: this_type& operator<<(psyq::message_pack::object const& in_object)
    {
        if (!this->write_object(in_object))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /** @brief MessagePackオブジェクトを直列化し、ストリームへ出力する。
        @param[in] in_object 直列化するMessagePackオブジェクト。
        @retval true  成功。
        @retval false 失敗。
     */
    public: bool write_object(psyq::message_pack::object const& in_object)
    {
        switch(in_object.get_type())
        {
        case psyq::message_pack::object::type::NIL:
        {
            // 空値をストリームへ出力する。
            return this->write_nil();
        }
        case psyq::message_pack::object::type::BOOLEAN:
        {
            // 真偽値をストリームへ出力する。
            auto const local_boolean(in_object.get_boolean());
            return local_boolean != nullptr
                && this->write_boolean(*local_boolean);
        }
        case psyq::message_pack::object::type::UNSIGNED_INTEGER:
        {
            // 0以上の整数をストリームへ出力する。
            auto const local_integer(in_object.get_unsigned_integer());
            return local_integer != nullptr
                && this->write_unsigned_integer(*local_integer);
        }
        case psyq::message_pack::object::type::NEGATIVE_INTEGER:
        {
            // 0未満の整数をストリームへ出力する。
            auto const local_integer(in_object.get_negative_integer());
            return local_integer != nullptr
                && this->write_signed_integer(*local_integer);
        }
        case psyq::message_pack::object::type::FLOATING_POINT_32:
        {
            // 32bit浮動小数点数をストリームへ出力する。
            auto const local_float(in_object.get_floating_point_32());
            return local_float != nullptr
                && this->write_floating_point(*local_float);
        }
        case psyq::message_pack::object::type::FLOATING_POINT_64:
        {
            // 64bit浮動小数点数をストリームへ出力する。
            auto const local_float(in_object.get_floating_point_64());
            return local_float != nullptr
                && this->write_floating_point(*local_float);
        }
        case psyq::message_pack::object::type::STRING:
        {
            // 文字列をストリームへ出力する。
            auto const local_string(in_object.get_string());
            return local_string != nullptr
                && this->write_raw_string(
                    local_string->data(), local_string->size());
        }
        case psyq::message_pack::object::type::BINARY:
        {
            // バイナリをストリームへ出力する。
            auto const local_binary(in_object.get_binary());
            return local_binary != nullptr
                && this->write_raw_binary(
                    local_binary->data(), local_binary->size());
        }
        case psyq::message_pack::object::type::EXTENDED:
        {
            // 拡張バイナリをストリームへ出力する。
            auto const local_extended(in_object.get_extended());
            return local_extended != nullptr
                && this->make_serial_extended(
                    local_extended->type(), local_extended->size())
                && this->fill_container_raw(*local_extended) == 0;
        }
        case psyq::message_pack::object::type::ARRAY:
        {
            // 配列をストリームへ出力する。
            auto const local_array(in_object.get_array());
            return local_array != nullptr && this->write_array(*local_array);
        }
        case psyq::message_pack::object::type::UNORDERED_MAP:
        case psyq::message_pack::object::type::MAP:
        {
            // 連想配列をストリームへ出力する。
            auto const local_map(in_object.get_unordered_map());
            return local_map != nullptr && this->write_map(*local_map);
        }
        default:
            PSYQ_ASSERT(false);
            return false;
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief RAWバイト列のバイト数を書き込む。
        @tparam template_header 書き込むヘッダーの基準値。
        @param[in] in_size RAWバイト列のバイト数。
        @retval true  成功。
        @retval false 失敗。
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
        @retval true  成功。
        @retval false 失敗。
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
                static_cast<typename this_type::stream::char_type const*>(in_begin),
                in_size);
            if (this->stream_.fail())
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
    private: typename this_type::container_stack* get_stack_top_raw()
    {
        if (0 < this->get_rest_container_count())
        {
            auto& local_stack(
                this->container_stack_.at(this->get_rest_container_count() - 1));
            if (local_stack.kind == this_type::value_kind_RAW_BYTES)
            {
                return &local_stack;
            }
        }
        return nullptr;
    }

    /** @brief コンテナの直列化を開始する。
        @tparam template_value_kind     スタックの種別。
        @tparam template_format_begin   コンテナ形式。
        @tparam template_format_fix_min 固定長コンテナ形式の最小値。
        @tparam template_format_fix_max 固定長コンテナ形式の最大値。
        @param[in] in_length 直列化するコンテナの要素数。
     */
    private: template<
        typename this_type::value_kind template_value_kind,
        std::uint8_t template_format_begin,
        std::uint8_t template_format_fix_min,
        std::uint8_t template_format_fix_max>
    bool make_serial_container(std::size_t const in_length)
    {
        static_assert(template_format_fix_min <= template_format_fix_max, "");
        if (in_length <= 0)
        {
            // 空コンテナを直列化する。
            if (this->write_big_endian<std::uint8_t>(template_format_fix_min))
            {
                this->update_container_stack();
                return true;
            }
            return false;
        }
        else if (this_type::stack_capacity <= this->get_rest_container_count())
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
            return false;
        }
        else if (in_length <= template_format_fix_max - template_format_fix_min)
        {
            // 固定長コンテナのヘッダを直列化する。
            std::uint8_t const local_header(
                template_format_fix_min + in_length);
            if (!this->write_big_endian(local_header))
            {
                return false;
            }
        }
        else if (in_length <= (std::numeric_limits<std::uint16_t>::max)())
        {
            // 16bit長コンテナのヘッダと要素数を直列化する。
            std::uint8_t const local_header(template_format_begin);
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
            std::uint8_t const local_header(template_format_begin + 1);
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
        auto& local_stack(this->container_stack_.at(this->get_rest_container_count()));
        local_stack.kind = template_value_kind;
        local_stack.rest_count = in_length;
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
            this->container_stack_.at(this->get_rest_container_count() - 1));
        switch (local_stack.kind)
        {
        case this_type::value_kind_ARRAY_ELEMENT:
            if (1 < local_stack.rest_count)
            {
                --local_stack.rest_count;
            }
            else
            {
                --this->stack_size_;
                this->update_container_stack();
            }
            break;

        case this_type::value_kind_MAP_KEY:
            local_stack.kind = this_type::value_kind_MAP_VALUE;
            break;

        case this_type::value_kind_MAP_VALUE:
            if (1 < local_stack.rest_count)
            {
                local_stack.kind = this_type::value_kind_MAP_KEY;
                --local_stack.rest_count;
            }
            else
            {
                --this->stack_size_;
                this->update_container_stack();
            }
            break;

        case this_type::value_kind_RAW_BYTES:
        default:
            PSYQ_ASSERT(false);
            break;
        }
    }

    //-------------------------------------------------------------------------
    /// @copydoc stream
    private: typename this_type::stream stream_;
    /// @copydoc container_stack
    private: std::array<typename this_type::container_stack, template_stack_capacity>
        container_stack_;
    /// 直列化途中のコンテナのスタック階層数。
    private: std::size_t stack_size_;
}; // class psyq::message_pack::serializer

#endif // !defined(PSYQ_MESSAGE_PACK_SERIALIZER_HPP_)
