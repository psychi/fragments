/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copydoc psyq::message_pack::serializer
 */
#ifndef PSYQ_MESSAGE_PACK_SERIALIZER_HPP_
#define PSYQ_MESSAGE_PACK_SERIALIZER_HPP_

#ifndef PSYQ_MESSAGE_PACK_SERIALIZER_STACK_SIZE_DEFAULT
/// psyq::message_pack::serializer のスタック限界数のデフォルト値。
#define PSYQ_MESSAGE_PACK_SERIALIZER_STACK_SIZE_DEFAULT 32
#endif // !defined(PSYQ_MESSAGE_PACK_SERIALIZER_STACK_SIZE_DEFAULT)

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
//#include "psyq/string/string_view_interface.hpp"

namespace psyq
{
    namespace message_pack
    {
        /// @cond
        template<typename, std::size_t> class serializer;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief MessagePack形式で直列化したバイナリを
           std::basic_ostream 互換のオブジェクトに書き込むためのアダプタ。
    @tparam template_out_stream @copydoc self::stream
    @tparam template_stack_size @copydoc self::stack_size
 */
template<
    typename template_out_stream = std::ostringstream,
    std::size_t template_stack_size
        = PSYQ_MESSAGE_PACK_SERIALIZER_STACK_SIZE_DEFAULT>
class psyq::message_pack::serializer
{
    /// thisが指す値の型。
    private: typedef serializer<template_out_stream, template_stack_size> self;

    /** 直列化したMessagePack形式のバイナリを書き込む
        std::basic_ostream 互換の出力ストリーム。
     */
    public: typedef template_out_stream stream;

    /** 直列化途中のコンテナのスタックの限界数。
     */
    public: static std::size_t const stack_size = template_stack_size;

    /// 次に直列化するオブジェクトの種類。
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
    public: serializer(): stack_size_(0) {}

    public: explicit serializer(typename self::stream in_stream):
        stream_(std::move(in_stream)),
        stack_size_(0)
    {}

    public: ~serializer()
    {
        this->fill_container_stack();
    }

    //-------------------------------------------------------------------------
    /// @name 値の直列化
    //@{
    /** @brief nilを直列化する。
     */
    public: void serialize_nil()
    {
        this->stream_ << std::uint8_t(0xc0);
        this->update_stack();
    }

    /** @brief 真偽値を直列化する。
        @param[in] in_boolean 直列化する真偽値。
     */
    public: void serialize_boolean(bool const in_boolean)
    {
        this->stream_ << std::uint8_t(in_boolean? 0xc3: 0xc2);
        this->update_stack();
    }

    /** @brief 無符号整数を直列化する。
        @param[in] in_integer 直列化する整数。
     */
    public: template<typename template_integer_type>
    void serialize_unsigned_integer(template_integer_type const in_integer)
    {
        static_assert(
            std::is_unsigned<template_integer_type>::value,
            "template_integer_type is not unsigned integer type.");
        if (in_integer <= 0x7f)
        {
            this->stream_ << static_cast<std::uint8_t>(in_integer);
        }
        else if (in_integer <= (std::numeric_limits<std::uint8_t>::max)())
        {
            self::serialize_8bits(
                this->stream_, 0xcc, static_cast<std::uint8_t>(in_integer));
        }
        else if (in_integer <= (std::numeric_limits<std::uint16_t>::max)())
        {
            self::serialize_16bits(
                this->stream_, 0xcd, static_cast<std::uint16_t>(in_integer));
        }
        else if (in_integer <= (std::numeric_limits<std::uint32_t>::max)())
        {
            self::serialize_32bits(
                this->stream_, 0xce, static_cast<std::uint32_t>(in_integer));
        }
        else
        {
            PSYQ_ASSERT(
                in_integer <= (std::numeric_limits<std::uint64_t>::max()));
            self::serialize_64bits(
                this->stream_, 0xcf, static_cast<std::uint64_t>(in_integer));
        }
        this->update_stack();
    }

    /** @brief 整数を直列化する。
        @param[in] in_integer 直列化する整数。
     */
    public: template<typename template_integer_type>
    void serialize_integer(template_integer_type const in_integer)
    {
        static_assert(
            std::is_integral<template_integer_type>::value,
            "template_integer_type is not integer type.");
        if (0 <= in_integer)
        {
            // 0以上の整数を直列化する。
            typedef typename std::make_unsigned<template_integer_type>::type
                unsigned_integer;
            this->serialize_unsigned_integer(
                static_cast<unsigned_integer>(in_integer));
            return;
        }

        // 0未満の整数を直列化する。
        if (-0x20 <= in_integer)
        {
            this->stream_ << static_cast<std::uint8_t>(in_integer);
        }
        else if ((std::numeric_limits<std::int8_t>::min()) <= in_integer)
        {
            self::serialize_8bits(
                this->stream_, 0xd0, static_cast<std::uint8_t>(in_integer));
        }
        else if ((std::numeric_limits<std::int16_t>::min()) <= in_integer)
        {
            self::serialize_16bits(
                this->stream_, 0xd1, static_cast<std::uint16_t>(in_integer));
        }
        else if ((std::numeric_limits<std::int32_t>::min()) <= in_integer)
        {
            self::serialize_32bits(
                this->stream_, 0xd2, static_cast<std::uint32_t>(in_integer));
        }
        else
        {
            PSYQ_ASSERT(
                (std::numeric_limits<std::int64_t>::min()) <= in_integer);
            self::serialize_64bits(
                this->stream_, 0xd3, static_cast<std::uint64_t>(in_integer));
        }
        this->update_stack();
    }

    /** @brief 浮動小数点実数を直列化する。
        @param[in] in_float 直列化する浮動小数点実数。
     */
    public: void serialize_floating_point(float const in_float)
    {
        union {std::uint32_t integer; float real;} local_value;
        local_value.real = in_float;
        self::serialize_32bits(this->stream_, 0xca, local_value.integer);
        this->update_stack();
    }
    /// @copydoc self::serialize_floating_point(float const)
    public: void serialize_floating_point(double const in_float)
    {
        union {std::uint64_t integer; double real;} local_value;
        local_value.real = in_float;
        self::serialize_64bits(this->stream_, 0xcb, local_value.integer);
        this->update_stack();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name コンテナの直列化
    //@{
    /** @brief 組み込み配列をRAWバイト列として直列化する。
        @param[in] in_data 直列化する組み込み配列の先頭位置。
        @param[in] in_size 組み込み配列の要素数。
     */
    public: template<typename template_value_type>
    void serialize_raw(
        template_value_type const* const in_data,
        std::size_t const in_size)
    {
        // RAWバイト列の大きさを直列化する。
        auto const local_size(in_size * sizeof(template_value_type));
        if (local_size <= 0x1f)
        {
            this->stream_ << std::uint8_t(0xa0 + (local_size & 0x1f));
        }
        else if (local_size <= (std::numeric_limits<std::uint8_t>::max)())
        {
            self::serialize_8bits(
                this->stream_, 0xd9, static_cast<std::uint8_t>(local_size));
        }
        else if (local_size <= (std::numeric_limits<std::uint16_t>::max)())
        {
            self::serialize_16bits(
                this->stream_, 0xda, static_cast<std::uint16_t>(local_size));
        }
        else if (local_size <= (std::numeric_limits<std::uint32_t>::max)())
        {
            self::serialize_32bits(
                this->stream_, 0xdb, static_cast<std::uint32_t>(local_size));
        }
        else
        {
            PSYQ_ASSERT(false);
            return;
        }

        // RAWバイト列を直列化する。
        static_assert(
            sizeof(typename self::stream::char_type) == 1,
            "sizeof(self::stream::char_type) is not 1.");
        this->stream_.write(
            reinterpret_cast<typename self::stream::char_type const*>(in_data),
            local_size);
        this->update_stack();
    }

    /** @brief MessagePackオブジェクト配列の直列化を開始する。
        @param[in] in_size 直列化する配列の要素数。
        @sa self::fill_container_rest() self::fill_container_stack()
     */
    public: void serialize_array(std::size_t const in_size)
    {
        if (in_size <= 0)
        {
            // 空の配列を直列化する。
            this->stream_ << std::uint8_t(0x90);
            this->update_stack();
        }
        else if (this->get_container_stack_size() < MSGPACK_EMBED_STACK_SIZE)
        {
            // 配列の要素数を直列化する。
            if (in_size <= 0xf)
            {
                this->stream_ << std::uint8_t(0x90 + in_size);
            }
            else if (in_size <= (std::numeric_limits<std::uint16_t>::max)())
            {
                self::serialize_16bits(
                    this->stream_, 0xdc, static_cast<std::uint16_t>(in_size));
            }
            else if (in_size <= (std::numeric_limits<std::uint32_t>::max)())
            {
                self::serialize_32bits(
                    this->stream_, 0xdd, static_cast<std::uint32_t>(in_size));
            }
            else
            {
                // 配列の限界要素数を超えたので失敗。
                PSYQ_ASSERT(false);
                return;
            }

            // 配列をスタックに積む。
            auto& local_stack(
                this->stack_.at(this->get_container_stack_size()));
            local_stack.kind = self::next_kind_ARRAY_ITEM;
            local_stack.rest_size = in_size;
            ++this->stack_size_;
        }
        else
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
        }
    }

    /** @brief MessagePackオブジェクト連想配列の直列化を開始する。
        @param[in] in_size 直列化する連想配列の要素数。
        @sa self::fill_container_rest() self::fill_container_stack()
     */
    public: void serialize_map(std::size_t const in_size)
    {
        if (in_size <= 0)
        {
            // 空の連想配列を直列化する。
            this->stream_ << std::uint8_t(0x80);
            this->update_stack();
        }
        else if (this->get_container_stack_size() < MSGPACK_EMBED_STACK_SIZE)
        {
            // 連想配列の要素数を直列化する。
            if (in_size <= 0xf)
            {
                this->stream_ << std::uint8_t(0x80 + in_size);
            }
            else if (in_size <= (std::numeric_limits<std::uint16_t>::max)())
            {
                self::serialize_16bits(
                    this->stream_, 0xde, static_cast<std::uint16_t>(in_size));
            }
            else if (in_size <= (std::numeric_limits<std::uint32_t>::max)())
            {
                self::serialize_32bits(
                    this->stream_, 0xdf, static_cast<std::uint32_t>(in_size));
            }
            else
            {
                // 配列の限界要素数を超えたので失敗。
                PSYQ_ASSERT(false);
                return;
            }

            // 連想配列をスタックに積む。
            auto& local_stack(
                this->stack_.at(this->get_container_stack_size()));
            local_stack.kind = self::next_kind_MAP_KEY;
            local_stack.rest_size = in_size;
            ++this->stack_size_;
        }
        else
        {
            // スタック限界を超えたので失敗。
            PSYQ_ASSERT(false);
        }
    }

    /** @brief 現在直列化途中のコンテナの残り要素をnilで埋める。
        @sa self::serialize_array() self::serialize_map()
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
            this->stream_ << std::uint8_t(0xc0);
        }
        --this->stack_size_;
        this->update_stack();
    }

    /** @brief 現在直列化途中のオブジェクトの残りをnillで埋める。
        @sa self::serialize_array() self::serialize_map()
     */
    public: void fill_container_stack()
    {
        while (0 < this->get_container_stack_size())
        {
            this->fill_container_rest();
        }
    }
    //@}
    //-------------------------------------------------------------------------
    private: void update_stack()
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
                this->update_stack();
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
                this->update_stack();
            }
            break;

        default:
            PSYQ_ASSERT(false);
            break;
        }
    }

    //-------------------------------------------------------------------------
    /** @brief 8bit整数をMessagePack形式で直列化する。
        @param[out] out_stream 直列化した整数を出力するストリーム。
        @param[in]  in_header  直列化する整数のヘッダ。
        @param[in]  in_integer 直列化する8bit整数。
     */
    private: static void serialize_8bits(
        typename self::stream& out_stream,
        std::uint8_t const in_header,
        std::uint8_t const in_integer)
    {
        out_stream << in_header << in_integer;
    }

    /** @brief 16bit整数をMessagePack形式で直列化する。
        @param[out] out_stream 直列化した整数を出力するストリーム。
        @param[in]  in_header  直列化する整数のヘッダ。
        @param[in]  in_integer 直列化する16bit整数。
     */
    private: static void serialize_16bits(
        typename self::stream& out_stream,
        std::uint8_t const in_header,
        std::uint16_t const in_integer)
    {
        out_stream << in_header
            << static_cast<std::uint8_t>(in_integer >> 8)
            << static_cast<std::uint8_t>(in_integer);
    }

    /** @brief 32bit整数をMessagePack形式で直列化する。
        @param[out] out_stream 直列化した整数を出力するストリーム。
        @param[in]  in_header  直列化する整数のヘッダ。
        @param[in]  in_integer 直列化する32bit整数。
     */
    private: static void serialize_32bits(
        typename self::stream& out_stream,
        std::uint8_t const in_header,
        std::uint32_t const in_integer)
    {
        out_stream << in_header
            << static_cast<std::uint8_t>(in_integer >> 24)
            << static_cast<std::uint8_t>(in_integer >> 16)
            << static_cast<std::uint8_t>(in_integer >>  8)
            << static_cast<std::uint8_t>(in_integer);
    }

    /** @brief 64bit整数をMessagePack形式で直列化する。
        @param[out] out_stream 直列化した整数を出力するストリーム。
        @param[in]  in_header  直列化する整数のヘッダ。
        @param[in]  in_integer 直列化する64bit整数。
     */
    private: static void serialize_64bits(
        typename self::stream& out_stream,
        std::uint8_t const in_header,
        std::uint64_t const in_integer)
    {
        out_stream << in_header
            << static_cast<std::uint8_t>(in_integer >> 56)
            << static_cast<std::uint8_t>(in_integer >> 48)
            << static_cast<std::uint8_t>(in_integer >> 40)
            << static_cast<std::uint8_t>(in_integer >> 32)
            << static_cast<std::uint8_t>(in_integer >> 24)
            << static_cast<std::uint8_t>(in_integer >> 16)
            << static_cast<std::uint8_t>(in_integer >>  8)
            << static_cast<std::uint8_t>(in_integer);
    }

    //-------------------------------------------------------------------------
    public: typename self::stream const& get_stream() const PSYQ_NOEXCEPT
    {
        return this->stream_;
    }

    public: typename self::next_kind get_next_kind() const PSYQ_NOEXCEPT
    {
        return 0 < this->get_container_rest_size()?
            this->stack_.at(this->get_container_rest_size() - 1).kind:
            typename self::next_kind_VALUE;
    }

    /** @brief 現在直列化途中のコンテナの残り要素数を取得する。
        @return 現在のコンテナスタックの残り要素数。
     */
    public: std::size_t get_container_rest_size() const PSYQ_NOEXCEPT
    {
        return 0 < this->get_container_rest_size()?
            this->stack_.at(this->get_container_rest_size() - 1).rest_size: 0;
    }

    /** @brief 直列化途中のコンテナの数を取得する。
        @return 直列化途中のコンテナの数。
     */
    public: std::size_t get_container_stack_size() const PSYQ_NOEXCEPT
    {
        return this->stack_size_;
    }

    //-------------------------------------------------------------------------
    /// @copydoc self::stream
    private: typename self::stream stream_;
    /// 直列化途中のコンテナのスタック。
    private: std::array<typename self::stack, template_stack_size> stack_;
    /// 直列化途中のコンテナのスタック数。
    private: std::size_t stack_size_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace internal
    {
        template<
            std::size_t template_index,
            typename template_out_stream,
            typename template_tuple>
        struct message_pack_tuple_serializer
        {
            static void serialize(
                psyq::message_pack::serializer<template_out_stream>&
                    out_stream,
                template_tuple const& in_tuple)
            {
                static_assert(0 < template_index, "");
                psyq::internal::message_pack_tuple_serializer<
                    template_index - 1, template_out_stream, template_tuple>
                        ::serialize(out_stream, in_tuple);
                out_stream << std::get<template_index>(in_tuple);
            }
        };
        template<typename template_out_stream, typename template_tuple>
        struct message_pack_tuple_serializer<
            0, template_out_stream, template_tuple>
        {
            static void serialize(
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
        /** @brief タプルをMessagePack形式で直列化する。
            @tparam template_tuple std::tuple 互換のタプル型。
            @param[out] out_stream 出力するストリーム。
            @param[in]  in_tuple   直列化するタプル。
         */
        template<typename template_out_stream, typename template_tuple>
        void serialize_tuple(
            psyq::message_pack::serializer<template_out_stream>& out_stream,
            template_tuple const& in_tuple)
        {
            out_stream.serialize_array(std::tuple_size<template_tuple>::value);
            if (0 < std::tuple_size<template_tuple>::value)
            {
                psyq::internal::message_pack_tuple_serializer<
                    std::tuple_size<template_tuple>::value - 1,
                    template_out_stream,
                    template_tuple>
                        ::serialize(out_stream, in_tuple);
            }
        }

        /** @brief 配列をMessagePack形式で直列化する。
            @param[out] out_stream  出力するストリーム。
            @param[in]  in_iterator 直列化する配列の先頭位置。
            @param[in]  in_size     直列化する配列の要素数。
         */
        template<typename template_out_stream, typename template_iterator>
        void serialize_array(
            psyq::message_pack::serializer<template_out_stream>& out_stream,
            template_iterator in_iterator,
            std::size_t const in_size)
        {
            out_stream.serialize_array(in_size);
            for (std::size_t i(0); i < in_size; ++i, ++in_iterator)
            {
                out_stream << *in_iterator;
            }
        }
        /** @brief 配列をMessagePack形式で直列化する。
            @param[out] out_stream 出力するストリーム。
            @param[in]  in_array   直列化する配列。
         */
        template<typename template_out_stream, typename template_array>
        void serialize_array(
            psyq::message_pack::serializer<template_out_stream>& out_stream,
            template_array const& in_array)
        {
            psyq::message_pack::serialize_array(
                out_stream, in_array.begin(), in_array.size());
        }

        /** @brief 連想配列をMessagePack形式で直列化する。
            @param[out] out_stream 出力するストリーム。
            @param[in]  in_set     直列化する連想配列。
         */
        template<typename template_out_stream, typename template_set>
        void serialize_set(
            psyq::message_pack::serializer<template_out_stream>& out_stream,
            template_set const& in_set)
        {
            // マップ値が空の辞書として直列化する。
            out_stream.serialize_map(in_set.size());
            for (auto& local_value: in_set)
            {
                out_stream << local_value;
                out_stream.serialize_nil();
            }
        }

        /** @brief 連想配列をMessagePack形式で直列化する。
            @param[out] out_stream 出力するストリーム。
            @param[in]  in_map     直列化する連想配列。
         */
        template<typename template_out_stream, typename template_map>
        void serialize_map(
            psyq::message_pack::serializer<template_out_stream>& out_stream,
            template_map const& in_map)
        {
            out_stream.serialize_map(in_map.size());
            for (auto& local_value: in_map)
            {
                out_stream << local_value.first << local_value.second;
            }
        }
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//-----------------------------------------------------------------------------
/// @name 真偽値の直列化
//@{
/** @brief 真偽値をMessagePack形式で直列化する。
    @param[out] out_stream 出力するストリーム。
    @param[in]  in_boolean 直列化する真偽値。
 */
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    bool const in_boolean)
{
    out_stream.serialize_boolean(in_boolean);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 無符号整数の直列化
//@{
/** @brief 無符号整数をMessagePack形式で直列化する。
    @param[out] out_stream 出力するストリーム。
    @param[in]  in_integer 直列化する無符号整数。
 */
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    unsigned char const in_integer)
{
    out_stream.serialize_unsigned_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, unsigned char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    unsigned short const in_integer)
{
    out_stream.serialize_unsigned_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, unsigned char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    unsigned int const in_integer)
{
    out_stream.serialize_unsigned_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, unsigned char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    unsigned long const in_integer)
{
    out_stream.serialize_unsigned_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, unsigned char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    unsigned long long const in_integer)
{
    out_stream.serialize_unsigned_integer(in_integer);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 有符号整数の直列化
//@{
/** @brief 有符号整数をMessagePack形式で直列化する。
    @param[out] out_stream 出力するストリーム。
    @param[in]  in_integer 直列化する有符号整数。
 */
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    char const in_integer)
{
    out_stream.serialize_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    short const in_integer)
{
    out_stream.serialize_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    int const in_integer)
{
    out_stream.serialize_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    long const in_integer)
{
    out_stream.serialize_integer(in_integer);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, char const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    long long const in_integer)
{
    out_stream.serialize_integer(in_integer);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 浮動小数点実数の直列化
//@{
/** @brief 浮動小数点実数をMessagePack形式で直列化する。
    @param[out] out_stream 出力するストリーム。
    @param[in]  in_float   直列化する浮動小数点実数。
 */
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    float const in_float)
{
    out_stream.serialize_floating_point(in_float);
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, float const)
template<typename template_out_stream>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    double const in_float)
{
    out_stream.serialize_floating_point(in_float);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name RAWバイト列の直列化
//@{
/** @brief 文字列をMessagePack形式のRAWバイト列として直列化する。
    @param[out] out_stream 出力するストリーム。
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
    out_stream.serialize_raw(in_string.data(), in_string.length());
    return out_stream;
}
/// @copydoc operator<<(psyq::message_pack::serializer<template_out_stream>&, std::basic_string<template_char, template_traits, template_allocator> const&)
template<
    typename template_out_stream,
    typename template_string>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    psyq::internal::string_view_interface<template_string> const& in_string)
{
    out_stream.serialize_raw(in_string.data(), in_string.length());
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 配列の直列化
//@{
/** @brief std::tuple をMessagePack形式の配列として直列化する。
    @param[out] out_stream 出力するストリーム。
    @param[in]  in_tuple   直列化するタプル。
 */
template<typename template_out_stream, typename template_item0>
psyq::message_pack::serializer<template_out_stream>& operator<<(
    psyq::message_pack::serializer<template_out_stream>& out_stream,
    std::tuple<template_item0> const& in_tuple)
{
    psyq::message_pack::serialize_tuple(out_stream, in_tuple);
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
    psyq::message_pack::serialize_tuple(out_stream, in_tuple);
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
    psyq::message_pack::serialize_tuple(out_stream, in_tuple);
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
    psyq::message_pack::serialize_tuple(out_stream, in_tuple);
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
    psyq::message_pack::serialize_tuple(out_stream, in_tuple);
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
    psyq::message_pack::serialize_tuple(out_stream, in_tuple);
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
    psyq::message_pack::serialize_tuple(out_stream, in_tuple);
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
    psyq::message_pack::serialize_tuple(out_stream, in_tuple);
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
    psyq::message_pack::serialize_tuple(out_stream, in_tuple);
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
    psyq::message_pack::serialize_tuple(out_stream, in_tuple);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 配列の直列化
//@{
/** @brief std::array をMessagePack形式の配列として直列化する。
    @param[out] out_stream 出力するストリーム。
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
    psyq::message_pack::serialize_array(out_stream, in_array);
    return out_stream;
}

/** @brief std::vector をMessagePack形式の配列として直列化する。
    @param[out] out_stream 出力するストリーム。
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
    psyq::message_pack::serialize_array(out_stream, in_vector);
    return out_stream;
}

/** @brief std::deque をMessagePack形式の配列として直列化する。
    @param[out] out_stream 出力するストリーム。
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
    psyq::message_pack::serialize_array(out_stream, in_deque);
    return out_stream;
}

/** @brief std::list をMessagePack形式の配列として直列化する。
    @param[out] out_stream 出力するストリーム。
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
    psyq::message_pack::serialize_array(out_stream, in_list);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 連想配列の直列化
//@{
/** @brief std::set をMessagePack形式の連想配列として直列化する。
    @param[out] out_stream 出力するストリーム。
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
    psyq::message_pack::serialize_set(out_stream, in_set);
    return out_stream;
}

/** @brief std::multiset をMessagePack形式の連想配列として直列化する。
    @param[out] out_stream 出力するストリーム。
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
    psyq::message_pack::serialize_set(out_stream, in_set);
    return out_stream;
}

/** @brief std::unordered_set をMessagePack形式の連想配列として直列化する。
    @param[out] out_stream 出力するストリーム。
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
    psyq::message_pack::serialize_set(out_stream, in_set);
    return out_stream;
}

/** @brief std::unordered_multiset をMessagePack形式の連想配列として直列化する。
    @param[out] out_stream 出力するストリーム。
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
    psyq::message_pack::serialize_set(out_stream, in_set);
    return out_stream;
}
//@}
//-----------------------------------------------------------------------------
/// @name 連想配列の直列化
//@{
/** @brief std::map をMessagePack形式の連想配列として直列化する。
    @param[out] out_stream 出力するストリーム。
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
    psyq::message_pack::serialize_map(out_stream, in_map);
    return out_stream;
}

/** @brief std::multimap をMessagePack形式の連想配列として直列化する。
    @param[out] out_stream 出力するストリーム。
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
    psyq::message_pack::serialize_map(out_stream, in_map);
    return out_stream;
}

/** @brief std::unordered_map をMessagePack形式の連想配列として直列化する。
    @param[out] out_stream 出力するストリーム。
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
    psyq::message_pack::serialize_map(out_stream, in_map);
    return out_stream;
}

/** @brief std::unordered_multimap をMessagePack形式の連想配列として直列化する。
    @param[out] out_stream 出力するストリーム。
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
    psyq::message_pack::serialize_map(out_stream, in_map);
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
                    std::string("std::string"),
                    psyq::string_view("psyq::string_view"))
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

            local_serializer.serialize_nil();
            local_serializer.get_stream().str().length();
        }
    }
}

#endif // !defined(PSYQ_MESSAGE_PACK_SERIALIZER_HPP_)
