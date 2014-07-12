/** @file
    @brief @copybrief psyq::message_pack::endianness_converter
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_MESSAGE_PACK_ENDIANNESS_HPP_
#define PSYQ_MESSAGE_PACK_ENDIANNESS_HPP_

#ifndef PSYQ_ASSERT
#include <assert.h>
#define PSYQ_ASSERT assert
#endif // !defined(PSYQ_ASSERT)

#ifndef PSYQ_NOEXCEPT
#define PSYQ_NOEXCEPT
#endif // !defined(PSYQ_NOEXCEPT)

namespace psyq
{
    /// MessagePackの直列化／直列化復元。
    namespace message_pack
    {
        /// @cond
        template<typename> struct endianness_converter;
        /// @endcond

        //---------------------------------------------------------------------
        /// 値を直列化／直列化復元する際のエンディアン性。
        enum endianness
        {
            little_endian = false, ///< リトルエンディアン。
            big_endian = true,     ///< ビッグエンディアン。
            //native_endian = (__BYTE_ORDER == __BIG_ENDIAN), ///< ネイティブエンディアン。
        };

        /** @brief 稼働環境でのエンディアン性を取得する。
            @return 稼働環境でのエンディアン性。
         */
        inline psyq::message_pack::endianness get_native_endian() PSYQ_NOEXCEPT
        {
            static const union {int integer; std::uint8_t endianness;}
                static_native = {psyq::message_pack::big_endian};
            return static_cast<psyq::message_pack::endianness>(
                psyq::message_pack::big_endian - static_native.endianness);
        }

        //---------------------------------------------------------------------
        /** @brief 1バイト整数のエンディアン性を切り替える。
            @param[in] in_value 元になる1バイト整数。
            @return エンディアン性を切り替えた1バイト整数。
         */
        inline std::uint8_t swap_endianness(std::uint8_t in_value) PSYQ_NOEXCEPT
        {
            return in_value;
        }
        /** @brief 2バイト整数のエンディアン性を切り替える。
            @param[in] in_value 元になる2バイト整数。
            @return エンディアン性を切り替えた2バイト整数。
         */
        inline std::uint16_t swap_endianness(std::uint16_t in_value) PSYQ_NOEXCEPT
        {
            return static_cast<std::uint16_t>((in_value << 8) | (in_value >> 8));
        }
        /** @brief 4バイト整数のエンディアン性を切り替える。
            @param[in] in_value 元になる4バイト整数。
            @return エンディアン性を切り替えた4バイト整数。
         */
        inline std::uint32_t swap_endianness(std::uint32_t in_value) PSYQ_NOEXCEPT
        {
            in_value = static_cast<std::uint32_t>(
                ((in_value << 8) & 0xff00ff00) |
                ((in_value >> 8) & 0x00ff00ff));
            return static_cast<std::uint32_t>((in_value << 16) | (in_value >> 16));
        }
        /** @brief 8バイト整数のエンディアン性を切り替える。
            @param[in] in_value 元になる8バイト整数。
            @return エンディアン性を切り替えた8バイト整数。
         */
        inline std::uint64_t swap_endianness(std::uint64_t in_value) PSYQ_NOEXCEPT
        {
            in_value = static_cast<std::uint64_t>(
                ((in_value << 8) & 0xff00ff00ff00ff00) |
                ((in_value >> 8) & 0x00ff00ff00ff00ff));
            in_value = static_cast<std::uint64_t>(
                ((in_value << 16) & 0xffff0000ffff0000) |
                ((in_value >> 16) & 0x0000ffff0000ffff));
            return static_cast<std::uint64_t>((in_value << 32) | (in_value >> 32));
        }
    } // namespace message_pack

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

        //---------------------------------------------------------------------
        /// @brief 整数値のエンディアン性を変換する。
        template<bool template_integral>
        struct message_pack_endianness_converter
        {
            /** @brief 値のエンディアン性を変換する。
                @tparam template_target 変換先の値の型。
                @tparam template_source 変換元の値の型。
                @param[in] in_source     変換元の値。ネイティブエンディアン。
                @param[in] in_endianness 変換先のエンディアン性。
                @return エンディアン性を変換した値。
             */
            template<typename template_target, typename template_source>
            static template_target convert(
                template_source const in_source,
                psyq::message_pack::endianness const in_endianness)
            PSYQ_NOEXCEPT
            {
                static_assert(
                    std::is_integral<template_source>::value,
                    "template_source is not integer type.");
                typedef typename psyq::internal
                    ::message_pack_bytes<sizeof(template_source)>::type
                        bytes_type;
                return static_cast<template_target>(
                    in_endianness != psyq::message_pack::get_native_endian()?
                        psyq::message_pack::swap_endianness(
                            static_cast<bytes_type>(in_source)):
                        in_source);
            }
        };
        /// @brief 値のエンディアン性を変換する。
        template<> struct message_pack_endianness_converter<false>
        {
            /// @copydoc message_pack_endianness_converter::convert()
            template<typename template_target, typename template_source>
            static template_target convert(
                template_source const in_source,
                psyq::message_pack::endianness const in_endianness)
            PSYQ_NOEXCEPT
            {
                typedef typename psyq::internal
                    ::message_pack_bytes<sizeof(template_source)>::type
                        bytes_type;
                /** @note 2014.05.13
                    strict-aliasingの対応のためにunionを使う手法は、
                    C++標準では許容されておらず、リスクを伴う。
                    でも、ほとんどのコンパイラでは問題はないはず。
                    http://d.hatena.ne.jp/yohhoy/20120220/p1
                 */
                union
                {
                    template_source source;
                    bytes_type bytes;
                    template_target target;
                } local_union = {in_source};
                if (in_endianness != psyq::message_pack::get_native_endian())
                {
                    local_union.bytes = psyq::message_pack::swap_endianness(
                        local_union.bytes);
                }
                return local_union.target;
            }
        };
    } // namespace internal
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の値のエンディアン性を変換する。
    @tparam template_value @copydoc psyq::message_pack::endianness_converter::value_type
 */
template<typename template_value>
struct psyq::message_pack::endianness_converter
{
    private: typedef endianness_converter this_type;

    /** @brief エンディアン性を変換する値の型。

        この実装では、整数型か浮動小数点数型にのみに対応している。
        これらの型以外に対応するには、テンプレートの特殊化をした
        psyq::message_pack::endianness_converter を実装し、
        staticメンバ関数として this_type::write_value() を実装すること。
     */
    public: typedef template_value value_type;
    static_assert(
        std::is_integral<template_value>::value
        || std::is_floating_point<template_value>::value,
        "template_value is not integer or floating point type.");

    /// this_type::value_type のRAWバイト列の型。
    public: typedef typename psyq::internal
        ::message_pack_bytes<sizeof(template_value)>::type
            bytes;

    //-------------------------------------------------------------------------
    /** @brief 値からRAWバイト列へ直列化し、ストリームへ書き出す。

        in_endianness とnative-endianが一致するなら先頭から末尾へ、
        異なるなら末尾から先頭の順に、値のRAWバイト列をストリームへ出力する。

        @param[in,out] io_ostream    RAWバイト列を書き出す出力ストリーム。
        @param[in]     in_value      直列化する値。
        @param[in]     in_endianness 値を直列化する際のエンディアン性。
     */
    public: template<typename template_stream>
    static bool write_value(
        template_stream& io_ostream,
        template_value const in_value,
        psyq::message_pack::endianness const in_endianness)
    {
        auto const local_bytes(this_type::pack_bytes(in_value, in_endianness));
        auto const local_pre_offset(io_ostream.tellp());
        io_ostream.write(
            reinterpret_cast<typename template_stream::char_type const*>(&local_bytes),
            sizeof(template_value));
        if (io_ostream.fail())
        {
            PSYQ_ASSERT(false);
            io_ostream.seekp(local_pre_offset);
            return false;
        }
        return true;
    }

    //---------------------------------------------------------------------
    /** @brief 値をRAWバイト列に変換する。
        @param[in] in_value      変換する値。
        @param[in] in_endianness RAWバイト列のエンディアン性。
        @return 値から変換されたRAWバイト列。
     */
    public: static typename this_type::bytes pack_bytes(
        template_value const in_value,
        psyq::message_pack::endianness const in_endianness)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::message_pack_endianness_converter
            <std::is_integral<template_value>::value>
                ::template convert<typename this_type::bytes>(in_value, in_endianness);
    }

    /** @brief RAWバイト列を値に変換する。
        @param[in] in_bytes      変換するRAWバイト列。
        @param[in] in_endianness RAWバイト列のエンディアン性。
        @return バイト列から変換された値。
     */
    public: static template_value unpack_bytes(
        typename this_type::bytes const in_bytes,
        psyq::message_pack::endianness const in_endianness)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::message_pack_endianness_converter
            <std::is_integral<template_value>::value>
                ::template convert<template_value>(in_bytes, in_endianness);
    }
    /** @brief RAWバイト列を値に変換する。
        @param[in] in_bytes      変換するRAWバイト列の先頭位置。
        @param[in] in_endianness RAWバイト列のエンディアン性。
        @return RAWバイト列から変換された値。
     */
    public: static template_value unpack_bytes(
        void const* const in_bytes,
        psyq::message_pack::endianness const in_endianness)
    PSYQ_NOEXCEPT
    {
        typename this_type::bytes local_bytes;
        std::memcpy(&local_bytes, in_bytes, sizeof(template_value));
        return this_type::unpack_bytes(local_bytes, in_endianness);
    }
}; // struct psyq::message_pack::endianness_converter;

#endif // !defined(PSYQ_MESSAGE_PACK_ENDIANNESS_HPP_)
