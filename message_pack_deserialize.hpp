/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief
 */
#ifndef PSYQ_MESSAGE_PACK_DESIRIALIZE_HPP_
#define PSYQ_MESSAGE_PACK_DESIRIALIZE_HPP_

#include <array>
//#include "psyq/message_pack_define.hpp"
//#include "psyq/message_pack_object.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class deserialize_context
{
    private: typedef deserialize_context self;

    private: enum deserialize_result
    {
        deserialize_result_FAILED,
        deserialize_result_FINISH,
        deserialize_result_ABORT,
        deserialize_result_CONTINUE,
    };

    /// 復元中のオブジェクトの種別。
    private: enum stack_kind
    {
        stack_kind_ARRAY_ITEM, ///< 配列の要素。
        stack_kind_MAP_KEY,    ///< 連想配列の要素のキー。
        stack_kind_MAP_VALUE,  ///< 連想配列の要素の値。
    };

    private: struct stack
    {
        psyq::message_pack::object object;  ///< 復元中のオブジェクト。
        psyq::message_pack::object map_key; ///< 直前に復元した連想配列キー。
        std::size_t count;                  ///< コンテナ要素の残数。
        self::stack_kind kind;              ///< 復元中のオブジェクトの種別。
    };

    private: struct user
    {
        msgpack_zone* zone;
        bool referenced;
    };

    //-------------------------------------------------------------------------
    public: void initialize(msgpack_zone& io_zone)
    {
        this->deserialize_kind_ = CS_HEADER;
        this->trail_ = 0;
        this->stack_size_ = 0;
        this->stack_[0].object.reset();
        this->user_.zone = &io_zone;
        this->user_.referenced = false;
    }

    /** @brief MessagePackを復元する。
        @param[in]     in_data   復元するMessagePackの先頭位置。
        @param[in]     in_size   復元するMessagePackのバイト数。
        @param[in,out] io_offset
            復元を開始するオフセット位置。
            復元処理の後、処理したバイト数が格納される。
        @retval 正 MessagePackの最後まで到達して、復元を終了。
        @retval 0  MessagePackの最後まで到達せず、復元を終了。
        @retval 負 復元に失敗。
     */
    public: int deserialize(
        void const* const in_data,
        std::size_t const in_size,
        std::size_t& io_offset)
    {
        if (in_size < io_offset)
        {
            PSYQ_ASSERT(false);
            return -1;
        }
        auto const local_data_begin(static_cast<std::uint8_t const*>(in_data));
        auto const local_data_end(local_data_begin + in_size);
        this->deserialize_iterator_ = local_data_begin + io_offset;
        auto const local_result(
            this->deserialize_iterator_ < local_data_end?
                this->deserialize_loop(local_data_end): 0);
        io_offset = this->deserialize_iterator_ - local_data_begin;
        return local_result;
    }

    public: psyq::message_pack::object const& get_root_object()
    const PSYQ_NOEXCEPT
    {
        return this->stack_[0].object;
    }

    //-------------------------------------------------------------------------
    /** @brief MessagePackを復元する。
        @param[in] in_end 復元するMessagePackの末尾位置。
        @retval 正 MessagePackの最後まで到達して、復元を終了。
        @retval 0  MessagePackの最後まで到達せず、復元を終了。
        @retval 負 復元に失敗。
     */
    private: int deserialize_loop(std::uint8_t const* const in_end)
    {
        psyq::message_pack::object local_object;
        for (;;)
        {
            switch (
                this->deserialize_kind_ == CS_HEADER?
                    this->deserialize_object(local_object, in_end):
                    this->deserialize_value(local_object, in_end))
            {
            case self::deserialize_result_FINISH:
                this->stack_[0].object = local_object;
                ++this->deserialize_iterator_;
                return 1;

            case self::deserialize_result_CONTINUE:
                this->deserialize_kind_ = CS_HEADER;
                ++this->deserialize_iterator_;
                if (this->deserialize_iterator_ < in_end)
                {
                    break;
                }
                return 0;

            case self::deserialize_result_ABORT:
                return 0;

            case self::deserialize_result_FAILED:
                return -1;

            default:
                PSYQ_ASSERT(false);
                return -1;
            }
        }
    }

    //-------------------------------------------------------------------------
    /** @brief MessagePackオブジェクトを復元する。
        @param[out] out_object 復元したオブジェクトが格納される。
        @param[in]  in_end     MessagePackの末尾位置。
     */
    private: self::deserialize_result deserialize_object(
        psyq::message_pack::object& out_object,
        std::uint8_t const* const in_end)
    {
        // MessagePackの直列化形式によって、復元処理を分岐する。
        auto const local_header(*this->deserialize_iterator_);
        if (local_header <= 0x7f)
        {
            // [0x00, 0x7f]: positive fixnum
            out_object = local_header;
            return this->deserialize_stack(out_object);
        }
        else if (local_header <= 0x8f)
        {
            // [0x80, 0x8f]: fix map
            return this->deserialize_container(
                out_object,
                local_header & 0x0f,
                self::stack_kind_MAP_KEY);
        }
        else if (local_header <= 0x9f)
        {
            // [0x90, 0x9f]: fix array
            return this->deserialize_container(
                out_object,
                local_header & 0x0f,
                self::stack_kind_ARRAY_ITEM);
        }
        else if (local_header == 0xa0)
        {
            // [0xa0, 0xbf]: fix raw
            self::deserialize_raw(out_object, this->user_, nullptr, 0);
            return this->deserialize_stack(out_object);
        }
        else if (local_header <= 0xbf)
        {
            // [0xa0, 0xbf]: fix raw
            this->trail_ = local_header & 0x1f;
            this->deserialize_kind_ = ACS_RAW_VALUE;
        }
        else if (local_header == 0xc0)
        {
            // 0xc0: nil
            out_object.reset();
            return this->deserialize_stack(out_object);
        }
        else if (local_header == 0xc1)
        {
            // 0xc1: string?
            PSYQ_ASSERT(false);
            return self::deserialize_result_FAILED;
        }
        else if (local_header == 0xc2)
        {
            // 0xc2: false
            out_object = false;
            return this->deserialize_stack(out_object);
        }
        else if (local_header == 0xc3)
        {
            // 0xc3: true
            out_object = true;
            return this->deserialize_stack(out_object);
        }
        else if (local_header <= 0xc6)
        {
            // 0xc4: bin 8
            // 0xc5: bin 16
            // 0xc6: bin 32
            this->trail_ = 1 << (local_header & 0x3);
            this->deserialize_kind_ = local_header & 0x1f;
        }
        else if (local_header <= 0xc9)
        {
            // 0xc7:
            // 0xc8:
            // 0xc9:
            PSYQ_ASSERT(false);
            return self::deserialize_result_FAILED;
        }
        else if (local_header <= 0xd3)
        {
            // 0xca: float
            // 0xcb: double
            // 0xcc: unsigned int  8
            // 0xcd: unsigned int 16
            // 0xce: unsigned int 32
            // 0xcf: unsigned int 64
            // 0xd0: signed int  8
            // 0xd1: signed int 16
            // 0xd2: signed int 32
            // 0xd3: signed int 64
            this->trail_ = 1 << (local_header & 0x3);
            this->deserialize_kind_ = local_header & 0x1f;
        }
        else if (local_header <= 0xd8)
        {
            // 0xd4:
            // 0xd5:
            // 0xd6: big integer 16?
            // 0xd7: big integer 32?
            // 0xd8: big float 16?
            PSYQ_ASSERT(false);
            return self::deserialize_result_FAILED;
        }
        else if (local_header <= 0xdb)
        {
            // 0xd9: raw 8
            // 0xda: raw 16
            // 0xdb: raw 32
            this->trail_ = 1 << ((local_header & 0x3) - 1);
            this->deserialize_kind_ = local_header & 0x1f;
        }
        else if (local_header <= 0xdf)
        {
            // 0xdc: array 16
            // 0xdd: array 32
            // 0xde: map 16
            // 0xdf: map 32
            this->trail_ = 2 << (local_header & 0x1);
            this->deserialize_kind_ = local_header & 0x1f;
        }
        else
        {
            // [0xe0, 0xff]: negative fixnum
            PSYQ_ASSERT(local_header <= 0xff);
            out_object = static_cast<std::int8_t>(local_header);
            return this->deserialize_stack(out_object);
        }

        // MessagePackオブジェクトの値を復元する。
        ++this->deserialize_iterator_;
        return this->deserialize_value(out_object, in_end);
    }

    //-------------------------------------------------------------------------
    /** @brief MessagePackオブジェクトの値を復元する。
        @param[out]out_object 復元した値を格納するオブジェクト。
        @param[in] in_end     MessagePackの末尾位置。
     */
    private: self::deserialize_result deserialize_value(
        psyq::message_pack::object& out_object,
        std::uint8_t const* const in_end)
    {
        if (in_end < this->deserialize_iterator_ + this->trail_)
        {
            return self::deserialize_result_ABORT;
        }
        auto const local_data(this->deserialize_iterator_);
        this->deserialize_iterator_ += this->trail_ - 1;
        switch (this->deserialize_kind_)
        {
        // 無符号整数
        case CS_UINT_8:
            out_object = *local_data;
            break;
        case CS_UINT_16:
            out_object
                = self::load_big_endian_integer<std::uint16_t>(local_data);
            break;
        case CS_UINT_32:
            out_object
                = self::load_big_endian_integer<std::uint32_t>(local_data);
            break;
        case CS_UINT_64:
            out_object
                = self::load_big_endian_integer<std::uint64_t>(local_data);
            break;

        // 有符号整数
        case CS_INT_8:
            out_object = static_cast<std::int8_t>(*local_data);
            break;
        case CS_INT_16:
            out_object
                = self::load_big_endian_integer<std::int16_t>(local_data);
            break;
        case CS_INT_32:
            out_object
                = self::load_big_endian_integer<std::int32_t>(local_data);
            break;
        case CS_INT_64:
            out_object
                = self::load_big_endian_integer<std::int64_t>(local_data);
            break;

        // 浮動小数点実数
        case CS_FLOAT:
        {
            union {std::uint32_t integer; float real;} local_value;
            local_value.integer
                = self::load_big_endian_integer<std::uint32_t>(local_data);
            out_object = local_value.real;
            break;
        }
        case CS_DOUBLE:
        {
            union {std::uint64_t integer; double real;} local_value;
            local_value.integer
                = self::load_big_endian_integer<std::uint64_t>(local_data);
            out_object = local_value.real;
            break;
        }

        // RAWバイト列
        case CS_BIN_8:
        case CS_RAW_8:
            this->trail_ = *local_data;
            goto PSYQ_MESSAGE_PACK_DESERIALIZE_ROW;
        case CS_BIN_16:
        case CS_RAW_16:
            this->trail_
                = self::load_big_endian_integer<std::uint16_t>(local_data);
            goto PSYQ_MESSAGE_PACK_DESERIALIZE_ROW;
        case CS_BIN_32:
        case CS_RAW_32:
            this->trail_
                = self::load_big_endian_integer<std::uint32_t>(local_data);
            goto PSYQ_MESSAGE_PACK_DESERIALIZE_ROW;

        PSYQ_MESSAGE_PACK_DESERIALIZE_ROW:
            if (0 < this->trail_)
            {
                this->deserialize_kind_ = ACS_RAW_VALUE;
                ++this->deserialize_iterator_;
            }
            else
            {
                self::deserialize_raw(out_object, this->user_, nullptr, 0);
            }
            break;

        case ACS_RAW_VALUE:
            self::deserialize_raw(
                out_object, this->user_, local_data, this->trail_);
            break;

        // 配列
        case CS_ARRAY_16:
            return this->deserialize_container(
                out_object,
                self::load_big_endian_integer<std::uint16_t>(local_data),
                self::stack_kind_ARRAY_ITEM);
        case CS_ARRAY_32:
            return this->deserialize_container(
                out_object,
                self::load_big_endian_integer<std::uint32_t>(local_data),
                self::stack_kind_ARRAY_ITEM);

        // 連想配列
        case CS_MAP_16:
            return this->deserialize_container(
                out_object,
                self::load_big_endian_integer<std::uint16_t>(local_data),
                self::stack_kind_MAP_KEY);
        case CS_MAP_32:
            return this->deserialize_container(
                out_object,
                self::load_big_endian_integer<std::uint32_t>(local_data),
                self::stack_kind_MAP_KEY);

        default:
            PSYQ_ASSERT(false);
            return self::deserialize_result_FAILED;
        }
        return this->deserialize_stack(out_object);
    }

    //-------------------------------------------------------------------------
    /** @brief MessagePackオブジェクトコンテナを復元する。
        @param[out]out_object  復元したコンテナを格納するオブジェクト。
        @param[in] in_capacity 復元するコンテナの容量。
        @param[in] in_kind     復元するコンテナの種別。
     */
    private: self::deserialize_result deserialize_container(
        psyq::message_pack::object& out_object,
        std::size_t const in_capacity,
        self::stack_kind const in_kind)
    {
        PSYQ_ASSERT(
            in_kind == self::stack_kind_ARRAY_ITEM
            || in_kind == self::stack_kind_MAP_KEY);
        if (MSGPACK_EMBED_STACK_SIZE <= this->stack_size_)
        {
            PSYQ_ASSERT(false);
            return self::deserialize_result_FAILED; // FIXME
        }
        auto& local_stack_top(this->stack_[this->stack_size_]);
        bool const local_deserialize_container(
            in_kind == self::stack_kind_ARRAY_ITEM?
                self::deserialize_array(
                    local_stack_top.object, this->user_, in_capacity):
                in_kind == self::stack_kind_MAP_KEY?
                    self::deserialize_map(
                        local_stack_top.object, this->user_, in_capacity):
                    false);
        if (!local_deserialize_container)
        {
            PSYQ_ASSERT(false);
            return self::deserialize_result_FAILED;
        }

        if (0 < in_capacity)
        {
            // コンテナをスタックに積む。
            local_stack_top.kind = in_kind;
            local_stack_top.count = in_capacity;
            ++this->stack_size_;
            return self::deserialize_result_CONTINUE;
        }
        out_object = local_stack_top.object;
        return this->deserialize_stack(out_object);
    }

    //-------------------------------------------------------------------------
    /** @brief スタックからMessagePackオブジェクトを取り出す。
        @param[out] out_object スタックから取り出したオブジェクトが格納される。
     */
    private: self::deserialize_result deserialize_stack(
        psyq::message_pack::object& out_object)
    PSYQ_NOEXCEPT
    {
        while (0 < this->stack_size_)
        {
            auto& local_stack_top(this->stack_[this->stack_size_ - 1]);
            switch (local_stack_top.kind)
            {
            case self::stack_kind_ARRAY_ITEM:
                // 配列に要素を追加する。
                self::deserialize_array_item(
                    local_stack_top.object, out_object);

                // 残り要素数を更新する。
                --local_stack_top.count;
                if (0 < local_stack_top.count)
                {
                    return self::deserialize_result_CONTINUE;
                }
                break;

            case self::stack_kind_MAP_KEY:
                local_stack_top.map_key = out_object;
                local_stack_top.kind = self::stack_kind_MAP_VALUE;
                return self::deserialize_result_CONTINUE;

            case self::stack_kind_MAP_VALUE:
            {
                // 連想配列に要素を追加する。
                auto const local_map(
                    self::deserialize_map_item(
                        local_stack_top.object,
                        local_stack_top.map_key,
                        out_object));
                if (local_map == nullptr)
                {
                    PSYQ_ASSERT(false);
                    return self::deserialize_result_FAILED;
                }

                // 残り要素数を更新する。
                --local_stack_top.count;
                if (0 < local_stack_top.count)
                {
                    local_stack_top.kind = self::stack_kind_MAP_KEY;
                    return self::deserialize_result_CONTINUE;
                }

                // 連想配列の全要素が揃ったので、ソートする。
                local_map->sort();
                break;
            }

            default:
                PSYQ_ASSERT(false);
                return self::deserialize_result_FAILED;
            }

            // オブジェクトをスタックから取り出す。
            out_object = local_stack_top.object;
            --this->stack_size_;
        }
        return self::deserialize_result_FINISH;
    }

    //-------------------------------------------------------------------------
    /** @brief MessagePackオブジェクトにRAWバイト列を格納する。
        @param[out] out_object RAWバイト列を格納するMessagePackオブジェクト。
        @param[in]  in_data    RAWバイト列の先頭位置。
        @param[in]  in_size    RAWバイト列のバイト数。
     */
    private: static void deserialize_raw(
        psyq::message_pack::object& out_object,
        self::user& out_user,
        void const* const in_data,
        std::size_t const in_size)
    PSYQ_NOEXCEPT
    {
        out_object.set_raw(static_cast<char const*>(in_data), in_size);
        out_user.referenced = true;
    }

    //-------------------------------------------------------------------------
    /** @brief MessagePackオブジェクトに配列を格納する。
        @param[out]    out_object  配列を格納するMessagePackオブジェクト。
        @param[in,out] io_user
        @param[in]     in_capacity 配列の容量。
        @retval true  成功。
        @retval false 失敗。
     */
    private: static bool deserialize_array(
        psyq::message_pack::object& out_object,
        self::user const& io_user,
        std::size_t const in_capacity)
    {
        auto& local_array(
            out_object.set_array(
                0 < in_capacity?
                    static_cast<psyq::message_pack::object*>(
                        msgpack_zone_malloc(
                            io_user.zone,
                            in_capacity * sizeof(psyq::message_pack::object))):
                    nullptr,
                0));
        return in_capacity <= 0 || local_array.data() != nullptr;
    }

    /** @brief 配列に要素を追加する。
        @param[in,out] io_object 要素を追加する配列。
        @param[in]     in_item   追加する要素。
     */
    private: static void deserialize_array_item(
        psyq::message_pack::object& io_object,
        psyq::message_pack::object const& in_item)
    PSYQ_NOEXCEPT
    {
        auto const local_array(io_object.get_array());
        if (local_array != nullptr)
        {
            local_array->push_back(in_item);
        }
        else
        {
            PSYQ_ASSERT(false);
        }
    }

    //-------------------------------------------------------------------------
    /** @brief MessagePackオブジェクトに連想配列を格納する。
        @param[out]    out_object  連想配列を格納するMessagePackオブジェクト。
        @param[in,out] io_user
        @param[in]     in_capacity 連想配列の容量。
        @retval true  成功。
        @retval false 失敗。
     */
    private: static bool deserialize_map(
        psyq::message_pack::object& out_object,
        self::user const& io_user,
        std::size_t const in_capacity)
    {
        auto& local_map(
            out_object.set_map(
                0 < in_capacity?
                    static_cast<psyq::message_pack::object::map::pointer>(
                        msgpack_zone_malloc(
                            io_user.zone,
                            in_capacity * sizeof(
                                psyq::message_pack::object::map::value_type))):
                    nullptr,
                0));
        return in_capacity <= 0 || local_map.data() != nullptr;
    }

    /** @brief 連想配列に要素を追加する。
        @param[in,out] io_object 要素を追加する連想配列。
        @param[in]     in_key    追加する要素のキー。
        @param[in]     in_mapped 追加する要素の値。
     */
    private: static psyq::message_pack::object::map* deserialize_map_item(
        psyq::message_pack::object& io_object,
        psyq::message_pack::object const& in_key,
        psyq::message_pack::object const& in_mapped)
    PSYQ_NOEXCEPT
    {
        auto const local_map(io_object.get_map());
        if (local_map != nullptr)
        {
            local_map->push_back(std::make_pair(in_key, in_mapped));
        }
        else
        {
            PSYQ_ASSERT(false);
        }
        return local_map;
    }

    //-------------------------------------------------------------------------
    /** @brief RAWバイト列から、ビッグエンディアンの整数値を取り出す。
        @param[in] in_bytes 整数値を取り出すRAWバイト列。
        @param[in] in_index 取り出す整数値のインデックス番号。
        @return 取り出した整数値。
     */
    private: template<typename template_value_type>
    static template_value_type load_big_endian_integer(
        void const* const in_bytes,
        std::size_t const in_index = 0)
    PSYQ_NOEXCEPT
    {
        static_assert(
            std::is_integral<template_value_type>::value,
            "template_value_type is not integral type.");
        static_assert(
            sizeof(template_value_type) == 1 ||
            sizeof(template_value_type) == 2 ||
            sizeof(template_value_type) == 4 ||
            sizeof(template_value_type) == 8,
            "sizeof(template_value_type) is invalid.");
        PSYQ_ASSERT(in_bytes != nullptr);

        // 整数を取り出す位置を決定する。
        auto const local_bytes(
            static_cast<std::uint8_t const*>(in_bytes)
                + in_index * sizeof(template_value_type));

        // 整数型の大きさによって、取り出し処理を分岐する。
        switch (sizeof(template_value_type))
        {
        case 1:
            return static_cast<template_value_type>(*local_bytes);
        case 2:
        {
            return static_cast<template_value_type>(
                (static_cast<std::uint16_t>(local_bytes[0]) << 8) |
                (static_cast<std::uint8_t >(local_bytes[1]) << 0));
        }
        case 4:
        {
            return static_cast<template_value_type>(
                (static_cast<std::uint32_t>(local_bytes[0]) << 24) |
                (static_cast<std::uint32_t>(local_bytes[1]) << 16) |
                (static_cast<std::uint16_t>(local_bytes[2]) <<  8) |
                (static_cast<std::uint8_t >(local_bytes[3]) <<  0));
        }
        case 8:
        {
            return static_cast<template_value_type>(
                (static_cast<std::uint64_t>(local_bytes[0]) << 54) |
                (static_cast<std::uint64_t>(local_bytes[1]) << 48) |
                (static_cast<std::uint64_t>(local_bytes[2]) << 40) |
                (static_cast<std::uint64_t>(local_bytes[3]) << 32) |
                (static_cast<std::uint32_t>(local_bytes[4]) << 24) |
                (static_cast<std::uint32_t>(local_bytes[5]) << 16) |
                (static_cast<std::uint16_t>(local_bytes[6]) <<  8) |
                (static_cast<std::uint8_t >(local_bytes[7]) <<  0));
        }
        default:
            PSYQ_ASSERT(false);
            return 0;
        }
    }

    //-------------------------------------------------------------------------
    private: std::array<self::stack, MSGPACK_EMBED_STACK_SIZE> stack_;
    private: self::user user_;
    private: std::uint8_t const* deserialize_iterator_;
    private: std::size_t deserialize_kind_; ///< 復元するオブジェクトの種別。
    private: std::size_t trail_;
    private: std::size_t stack_size_; ///< スタックの要素数。
};

#endif // !defined(PSYQ_MESSAGE_PACK_DESIRIALIZE_HPP_)
