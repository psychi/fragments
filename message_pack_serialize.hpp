/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief
 */
#ifndef PSYQ_MESSAGE_PACK_SERIALIZE_HPP_
#define PSYQ_MESSAGE_PACK_SERIALIZE_HPP_

#include <array>
#include <sstream>
//#include "psyq/message_pack_define.hpp"
//#include "psyq/message_pack_object.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class message_pack_serialize_context
{
    private: typedef message_pack_serialize_context self;

    public: typedef std::ostringstream stream;

    /// 直列化してるオブジェクトの種別。
    private: enum stack_kind
    {
        stack_kind_ARRAY_ITEM, ///< 配列の要素。
        stack_kind_MAP_KEY,    ///< 連想配列の要素のキー。
        stack_kind_MAP_VALUE,  ///< 連想配列の要素の値。
    };

    private: struct stack
    {
        std::size_t rest_size; ///< コンテナ要素の残数。
        self::stack_kind kind; ///< 復元中のオブジェクトの種別。
    };

    //-------------------------------------------------------------------------
    public: explicit message_pack_serialize_context(
        self::stream& in_stream)
    PSYQ_NOEXCEPT:
        stream_(in_stream),
        stack_size_(0)
    {}

    ~message_pack_serialize_context()
    {
        PSYQ_ASSERT(this->stack_size_ == 0);
    }

    //-------------------------------------------------------------------------
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
    public: void serialize(double const in_float)
    {
        union {std::uint64_t integer; double real;} local_value;
        local_value.real = in_float;
        self::serialize_integer(this->stream_, 0xcb, local_value.integer);
        this->update_stack();
    }

    public: void serialize_raw(
        void const* const in_data,
        std::size_t const in_size)
    {
        // RAWバイト列の大きさを直列化する。
        if (in_size <= 0x1f)
        {
            this->stream_ << std::uint8_t(0xa0 + (in_size & 0x1f));
        }
        else if (in_size <= (std::numeric_limits<std::uint8_t>::max)())
        {
            self::serialize_integer(
                this->stream_, 0xd9, static_cast<std::uint8_t>(in_size));
        }
        else if (in_size <= (std::numeric_limits<std::uint16_t>::max)())
        {
            self::serialize_integer(
                this->stream_, 0xda, static_cast<std::uint16_t>(in_size));
        }
        else
        {
            PSYQ_ASSERT(
                in_size <= (std::numeric_limits<std::uint32_t>::max)());
            self::serialize_integer(
                this->stream_, 0xdb, static_cast<std::uint32_t>(in_size));
        }

        // RAWバイト列を直列化する。
        static_assert(
            sizeof(self::stream::char_type) == 1,
            "sizeof(self::stream::char_type) is not 1.");
        this->stream_.write(
            static_cast<self::stream::char_type const*>(in_data), in_size);
        this->update_stack();
    }

    //-------------------------------------------------------------------------
    private: void update_stack()
    {
        if (this->stack_size_ <= 0)
        {
            return;
        }

        auto& local_stack(this->stack_.at(this->stack_size_ - 1));
        switch (local_stack.kind)
        {
        case self::stack_kind_ARRAY_ITEM:
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

        case self::stack_kind_MAP_KEY:
            local_stack.kind = self::stack_kind_MAP_VALUE;
            break;

        case self::stack_kind_MAP_VALUE:
            if (1 < local_stack.rest_size)
            {
                local_stack.kind = self::stack_kind_MAP_KEY;
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

    public: void push_array(std::size_t const in_size)
    {
        if (0 < in_size)
        {
            PSYQ_ASSERT(this->stack_size_ < MSGPACK_EMBED_STACK_SIZE);
            auto& local_stack(this->stack_.at(this->stack_size_));
            local_stack.kind = self::stack_kind_ARRAY_ITEM;
            local_stack.rest_size = in_size;
            ++this->stack_size_;
        }
        else
        {
            this->stream_ << std::uint8_t(0x80);
        }
    }

    public: void push_map(std::size_t const in_size)
    {
    }

    public: void pop_container()
    {
    }

    //-------------------------------------------------------------------------
    /** @brief 8bit整数をMessagePack形式で直列化する。
        @tparam template_out_stream std::basic_ostream 互換の出力ストリーム型。
        @param[out] out_stream 直列化した整数を出力するストリーム。
        @param[in]  in_integer 直列化する8bit整数。
     */
    private: static void serialize_integer(
        self::stream& out_stream,
        std::uint8_t const in_header,
        std::uint8_t const in_integer)
    {
        out_stream << in_header << in_integer;
    }

    /** @brief 16bit整数をMessagePack形式で直列化する。
        @tparam template_out_stream std::basic_ostream 互換の出力ストリーム型。
        @param[out] out_stream 直列化した整数を出力するストリーム。
        @param[in]  in_integer 直列化する16bit整数。
     */
    private: static void serialize_integer(
        self::stream& out_stream,
        std::uint8_t const in_header,
        std::uint16_t const in_integer)
    {
        out_stream << in_header
            << static_cast<std::uint8_t>(in_integer >> 8)
            << static_cast<std::uint8_t>(in_integer);
    }

    /** @brief 32bit整数をMessagePack形式で直列化する。
        @tparam template_out_stream std::basic_ostream 互換の出力ストリーム型。
        @param[out] out_stream 直列化した整数を出力するストリーム。
        @param[in]  in_integer 直列化する32bit整数。
     */
    private: static void serialize_integer(
        self::stream& out_stream,
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
        @tparam template_out_stream std::basic_ostream 互換の出力ストリーム型。
        @param[out] out_stream 直列化した整数を出力するストリーム。
        @param[in]  in_integer 直列化する64bit整数。
     */
    private: static void serialize_integer(
        self::stream& out_stream,
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
    private: self::stream& stream_;
    private: std::array<self::stack, MSGPACK_EMBED_STACK_SIZE> stack_;
    private: std::size_t stack_size_; ///< スタックの要素数。
    private: std::size_t serialize_kind_; ///< 復元するオブジェクトの種別。
    private: std::size_t trail_;
};

#endif // !defined(PSYQ_MESSAGE_PACK_SERIALIZE_HPP_)
