/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief
 */
#ifndef PSYQ_MESSAGE_PACK_SERIALIZER_HPP_
#define PSYQ_MESSAGE_PACK_SERIALIZER_HPP_

#include <array>
#include <sstream>
//#include "psyq/message_pack_object.hpp"

namespace psyq
{
    namespace message_pack
    {
        /// @cond
        template<typename> class serializer;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_out_stream = std::ostringstream>
class psyq::message_pack::serializer
{
    private: typedef serializer<template_out_stream> self;

    public: typedef template_out_stream stream;

    /// 次に直列化するオブジェクトの種類。
    public: enum next_kind
    {
        next_kind_VALUE,
        next_kind_ARRAY_ITEM, ///< 配列の要素。
        next_kind_MAP_KEY,    ///< 連想配列の要素のキー。
        next_kind_MAP_VALUE,  ///< 連想配列の要素の値。
    };

    private: struct stack
    {
        std::size_t rest_size;         ///< コンテナ要素の残数。
        typename self::next_kind kind; ///< @copydoc self::next_kind
    };

    //-------------------------------------------------------------------------
    public: explicit serializer(typename self::stream& in_stream)
    PSYQ_NOEXCEPT:
        stream_(in_stream),
        stack_size_(0)
    {}

    public: ~serializer()
    {
        this->fill_container_stack();
    }

    public: typename self::next_kind get_next_kind() const
    {
        return 0 < this->get_container_rest_size()?
            this->stack_.at(this->get_container_rest_size() - 1).kind:
            typename self::next_kind_VALUE;
    }

    /** @brief 現在直列化途中のコンテナの残り要素数を取得する。
        @return 現在のコンテナスタックの残り要素数。
     */
    public: std::size_t get_container_rest_size() const
    {
        return 0 < this->get_container_rest_size()?
            this->stack_.at(this->get_container_rest_size() - 1).rest_size: 0;
    }

    /** @brief 直列化途中のコンテナの数を取得する。
        @return 直列化途中のコンテナの数。
     */
    public: std::size_t get_container_stack_size() const
    {
        return this->stack_size_;
    }

    //-------------------------------------------------------------------------
    /// @name 値の直列化
    //@{
    /** @brief nilを直列化する。
     */
    public: void selialize_nil()
    {
        this->stream_ << std::uint8_t(0xc0);
        this->update_stack();
    }

    /** @brief 真偽値を直列化する。
        @param[in] in_boolean 直列化する真偽値。
     */
    public: void selialize(bool const in_boolean)
    {
        this->stream_ << std::uint8_t(in_boolean? 0xc3: 0xc2);
        this->update_stack();
    }

    /** @brief 無符号整数を直列化する。
        @param[in] in_integer 直列化する整数。
     */
    public: void serialize(std::uint8_t const in_integer)
    {
        if (in_integer <= 0x7f)
        {
            this->stream_ << static_cast<std::uint8_t>(in_integer);
        }
        else
        {
            self::serialize_integer(this->stream_, 0xcc, in_integer);
        }
        this->update_stack();
    }
    /// @copydoc self::serialize(std::uint8_t const)
    public: void serialize(std::uint16_t const in_integer)
    {
        if (in_integer <= (std::numeric_limits<std::uint8_t>::max)())
        {
            this->serialize(static_cast<std::uint8_t>(in_integer));
        }
        else
        {
            self::serialize_integer(this->stream_, 0xcd, in_integer);
            this->update_stack();
        }
    }
    /// @copydoc self::serialize(std::uint8_t const)
    public: void serialize(std::uint32_t const in_integer)
    {
        if (in_integer <= (std::numeric_limits<std::uint16_t>::max)())
        {
            this->serialize(static_cast<std::uint16_t>(in_integer));
        }
        else
        {
            self::serialize_integer(this->stream_, 0xce, in_integer);
            this->update_stack();
        }
    }
    /// @copydoc self::serialize(std::uint8_t const)
    public: void serialize(std::uint64_t const in_integer)
    {
        if (in_integer <= (std::numeric_limits<std::uint32_t>::max)())
        {
            this->serialize(static_cast<std::uint32_t>(in_integer));
        }
        else
        {
            self::serialize_integer(this->stream_, 0xcf, in_integer);
            this->update_stack();
        }
    }

    /** @brief 有符号整数を直列化する。
        @param[in] in_integer 直列化する整数。
     */
    public: void serialize(std::int8_t const in_integer)
    {
        if (0 <= in_integer)
        {
            this->serialize(static_cast<std::uint8_t>(in_integer));
            return;
        }

        if (-0x20 <= in_integer)
        {
            this->stream_ << static_cast<std::uint8_t>(in_integer);
        }
        else
        {
            self::serialize_integer(
                this->stream_, 0xd0, static_cast<std::uint8_t>(in_integer));
        }
        this->update_stack();
    }
    /// @copydoc self::serialize(std::int8_t const)
    public: void serialize(std::int16_t const in_integer)
    {
        if (0 <= in_integer)
        {
            this->serialize(static_cast<std::uint16_t>(in_integer));
        }
        else if ((std::numeric_limits<std::int8_t>::min()) <= in_integer)
        {
            this->serialize(static_cast<std::int8_t>(in_integer));
        }
        else
        {
            self::serialize_integer(
                this->stream_, 0xd1, static_cast<std::uint16_t>(in_integer));
            this->update_stack();
        }
    }
    /// @copydoc self::serialize(std::int8_t const)
    public: void serialize(std::int32_t const in_integer)
    {
        if (0 <= in_integer)
        {
            this->serialize(static_cast<std::uint32_t>(in_integer));
        }
        else if ((std::numeric_limits<std::int16_t>::min()) <= in_integer)
        {
            this->serialize(static_cast<std::int16_t>(in_integer));
        }
        else
        {
            self::serialize_integer(
                this->stream_, 0xd2, static_cast<std::uint32_t>(in_integer));
            this->update_stack();
        }
    }
    /// @copydoc self::serialize(std::int8_t const)
    public: void serialize(std::int64_t const in_integer)
    {
        if (0 <= in_integer)
        {
            this->serialize(static_cast<std::uint64_t>(in_integer));
        }
        else if ((std::numeric_limits<std::int32_t>::min()) <= in_integer)
        {
            this->serialize(static_cast<std::int32_t>(in_integer));
        }
        else
        {
            self::serialize_integer(
                this->stream_, 0xd3, static_cast<std::uint64_t>(in_integer));
            this->update_stack();
        }
    }

    /** @brief 浮動小数点実数を直列化する。
        @param[in] in_float 直列化する浮動小数点実数。
     */
    public: void serialize(float const in_float)
    {
        union {std::uint32_t integer; float real;} local_value;
        local_value.real = in_float;
        self::serialize_integer(this->stream_, 0xca, local_value.integer);
        this->update_stack();
    }
    /// @copydoc self::serialize(float const)
    public: void serialize(double const in_float)
    {
        union {std::uint64_t integer; double real;} local_value;
        local_value.real = in_float;
        self::serialize_integer(this->stream_, 0xcb, local_value.integer);
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
            self::serialize_integer(
                this->stream_, 0xd9, static_cast<std::uint8_t>(local_size));
        }
        else if (local_size <= (std::numeric_limits<std::uint16_t>::max)())
        {
            self::serialize_integer(
                this->stream_, 0xda, static_cast<std::uint16_t>(local_size));
        }
        else if (local_size <= (std::numeric_limits<std::uint32_t>::max)())
        {
            self::serialize_integer(
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
        @param[in] in_size 直列化する配列の大きさ。
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
                self::serialize_integer(
                    this->stream_, 0xdc, static_cast<std::uint16_t>(in_size));
            }
            else if (in_size <= (std::numeric_limits<std::uint32_t>::max)())
            {
                self::serialize_integer(
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
        @param[in] in_size 直列化する連想配列の大きさ。
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
                self::serialize_integer(
                    this->stream_, 0xde, static_cast<std::uint16_t>(in_size));
            }
            else if (in_size <= (std::numeric_limits<std::uint32_t>::max)())
            {
                self::serialize_integer(
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
    private: static void serialize_integer(
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
    private: static void serialize_integer(
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
    private: static void serialize_integer(
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
    private: static void serialize_integer(
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
    private: typename self::stream& stream_;
    private: std::array<typename self::stack, MSGPACK_EMBED_STACK_SIZE> stack_;
    private: std::size_t stack_size_; ///< スタックの数。
};

#endif // !defined(PSYQ_MESSAGE_PACK_SERIALIZER_HPP_)
