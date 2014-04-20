/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief
 */
#ifndef PSYQ_MESSAGE_PACK_CONTEXT_HPP_
#define PSYQ_MESSAGE_PACK_CONTEXT_HPP_

#include <array>

//#include "psyq/message_pack_define.hpp"
//#include "psyq/message_pack_object.hpp"

#define push_variable_value(func, base, pos, len)\
    if(msgpack_unpack_callback(func)(\
        local_user, (const char*)base, (const char*)pos, len, &local_object) < 0)\
    {goto TEMPLATE_EXECUTE_failed;}\
    goto TEMPLATE_EXECUTE_push

#define again_fixed_trail_if_zero(_cs, trail_len, ifzero)\
    local_trail = trail_len; \
    if(local_trail == 0) { goto ifzero; } \
    local_cs = _cs; \
    goto _fixed_trail_again

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class deserialize_context
{
    private: typedef deserialize_context self;

    private: struct stack
    {
        msgpack_object obj;
        msgpack_object map_key;
        std::size_t count;
        unsigned int ct;
    };

    private: struct user
    {
        msgpack_zone* zone;
        bool referenced;
    };

    private: enum deserialize_result
    {
        deserialize_result_FAILED,
        deserialize_result_FINISH,
        deserialize_result_ABORT,
        deserialize_result_CONTINUE,
    };

    //-------------------------------------------------------------------------
    public: void initialize(msgpack_zone& io_zone)
    {
        this->cs_ = CS_HEADER;
        this->trail_ = 0;
        this->top_ = 0;
        this->stack_[0].obj = self::make_stack_root();
        this->user_.zone = &io_zone;
        this->user_.referenced = false;
    }

    /** @brief MessagePackをデシリアライズする。
        @param[in]     in_data   デシリアライズするMessagePackの先頭位置。
        @param[in]     in_length デシリアライズするMessagePackのバイト数。
        @param[in,out] io_offset
     */
    public: int deserialize(
        void const* const in_data,
        std::size_t const in_length,
        std::size_t& io_offset)
    {
        if (in_length < io_offset)
        {
            PSYQ_ASSERT(false);
            return -1;
        }
        auto const local_data_begin(static_cast<std::uint8_t const*>(in_data));
        auto const local_data_end(local_data_begin + in_length);
        auto local_data_iterator(local_data_begin + io_offset);
        auto const local_result(
            local_data_iterator < local_data_end?
                this->deserialize_loop(local_data_iterator, local_data_end):
                0);
        io_offset = local_data_iterator - local_data_begin;
        return local_result;
    }

    public: msgpack_object get_root_object() const
    {
        return this->stack_[0].obj;
    }

    //-------------------------------------------------------------------------
    private: int deserialize_loop(
        std::uint8_t const*& io_data_iterator,
        std::uint8_t const* const in_data_end)
    {
        msgpack_object local_object;
        for (;;)
        {
            auto const local_deserialize_result(
                this->cs_ == CS_HEADER?
                    this->deserialize_header(
                        local_object, io_data_iterator, in_data_end):
                    this->deserialize_value(
                        local_object, io_data_iterator, in_data_end));
            switch (local_deserialize_result)
            {
            case self::deserialize_result_FINISH:
                this->stack_[0].obj = local_object;
                ++io_data_iterator;
                return 1;

            case self::deserialize_result_CONTINUE:
                this->cs_ = CS_HEADER;
                ++io_data_iterator;
                if (io_data_iterator < in_data_end)
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
    private: self::deserialize_result deserialize_header(
        msgpack_object& out_object,
        std::uint8_t const*& io_data_iterator,
        std::uint8_t const* const in_data_end)
    {
        unsigned const local_header(*io_data_iterator);
        if (local_header <= 0x7f)
        {
            // [0x00, 0x7f]: Positive Fixnum
            self::deserialize_integer<std::uint8_t>(
                local_header, out_object);
            return this->deserialize_pop(out_object);
        }
        else if (local_header <= 0x8f)
        {
            // [0x80, 0x8f]: FixMap
            return this->deserialize_container(
                out_object, local_header & 0x0f, CT_MAP_KEY);
        }
        else if (local_header <= 0x9f)
        {
            // [0x90, 0x9f]: FixArray
            return this->deserialize_container(
                out_object, local_header & 0x0f, CT_ARRAY_ITEM);
        }
        else if (local_header <= 0xbf)
        {
            // [0xa0, 0xbf]: FixRaw
            ////again_fixed_trail_if_zero(ACS_RAW_VALUE, ((unsigned int)local_header & 0x1f), _raw_zero);
            this->trail_ = (local_header & 0x1f);
            if (this->trail_ <= 0)
            {
                ////push_variable_value(_raw, in_data, local_n, this->trail_);
                //goto _raw_zero;
                self::deserialize_raw(this->user_, nullptr, 0, out_object);
                return this->deserialize_pop(out_object);
            }
            this->cs_ = ACS_RAW_VALUE;
        }
        else if (local_header == 0xc0)
        {
            // 0xc0: nil
            out_object.type = MSGPACK_OBJECT_NIL;
            return this->deserialize_pop(out_object);
        }
        else if (local_header == 0xc1)
        {
            // 0xc1: string
            //again_terminal_trail(self::next_cs(local_header), io_data_iterator+1);
            PSYQ_ASSERT(false);
            return self::deserialize_result_FAILED;
        }
        else if (local_header == 0xc2)
        {
            // 0xc2: false
            out_object.type = MSGPACK_OBJECT_BOOLEAN;
            out_object.via.boolean = false;
            return this->deserialize_pop(out_object);
        }
        else if (local_header == 0xc3)
        {
            // 0xc3: true
            out_object.type = MSGPACK_OBJECT_BOOLEAN;
            out_object.via.boolean = true;
            return this->deserialize_pop(out_object);
        }
        else if (local_header <= 0xc9)
        {
            // 0xc4:
            // 0xc5:
            // 0xc6:
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
            this->cs_ = self::next_cs(local_header);
        }
        else if (local_header <= 0xd9)
        {
            // 0xd4:
            // 0xd5:
            // 0xd6: big integer 16
            // 0xd7: big integer 32
            // 0xd8: big float 16
            // 0xd9: big float 32
            PSYQ_ASSERT(false);
            return self::deserialize_result_FAILED;
        }
        else if (local_header <= 0xdf)
        {
            // 0xda: raw 16
            // 0xdb: raw 32
            // 0xdc: array 16
            // 0xdd: array 32
            // 0xde: map 16
            // 0xdf: map 32
            this->trail_ = 2 << (local_header & 0x1);
            this->cs_ = self::next_cs(local_header);
        }
        else
        {
            // [0xe0, 0xff]: Negative Fixnum
            PSYQ_ASSERT(local_header <= 0xff);
            self::deserialize_integer<std::int8_t>(local_header, out_object);
            return this->deserialize_pop(out_object);
        }

        ++io_data_iterator;
        return this->deserialize_value(
            out_object, io_data_iterator, in_data_end);
    }

    //-------------------------------------------------------------------------
    private: self::deserialize_result deserialize_value(
        msgpack_object& out_object,
        std::uint8_t const*& io_data_iterator,
        std::uint8_t const* const in_data_end)
    {
        if (in_data_end < io_data_iterator + this->trail_)
        {
            return self::deserialize_result_ABORT;
        }
        void const* const local_n(io_data_iterator);
        io_data_iterator += this->trail_ - 1;
        switch (this->cs_)
        {
        case CS_UINT_8:
            self::deserialize_integer(
                *static_cast<std::uint8_t const*>(local_n),
                out_object);
            break;
        case CS_UINT_16:
            self::deserialize_integer(
                self::load_bytes<std::uint16_t>(local_n),
                out_object);
            break;
        case CS_UINT_32:
            self::deserialize_integer(
                self::load_bytes<std::uint32_t>(local_n),
                out_object);
            break;
        case CS_UINT_64:
            self::deserialize_integer(
                self::load_bytes<std::uint64_t>(local_n),
                out_object);
            break;

        case CS_INT_8:
            self::deserialize_integer(
                *static_cast<std::int8_t const*>(local_n),
                out_object);
            break;
        case CS_INT_16:
            self::deserialize_integer(
                self::load_bytes<std::int16_t>(local_n),
                out_object);
            break;
        case CS_INT_32:
            self::deserialize_integer(
                self::load_bytes<std::int32_t>(local_n),
                out_object);
            break;
        case CS_INT_64:
            self::deserialize_integer(
                self::load_bytes<std::int64_t>(local_n),
                out_object);
            break;

        case CS_FLOAT:
        {
            union {std::uint32_t integer; float real;} local_value;
            local_value.integer
                = self::load_bytes<std::uint32_t>(local_n);
            self::deserialize_real(
                local_value.real, out_object);
            break;
        }
        case CS_DOUBLE:
        {
            union {std::uint64_t integer; double real;} local_value;
            local_value.integer
                = self::load_bytes<std::uint64_t>(local_n);
            self::deserialize_real(
                local_value.real, out_object);
            break;
        }
#if 0
		//case CS_BIG_INT_16:
		//	again_fixed_trail_if_zero(ACS_BIG_INT_VALUE, _msgpack_load16(uint16_t,local_n), _big_int_zero);
		//case CS_BIG_INT_32:
		//	again_fixed_trail_if_zero(ACS_BIG_INT_VALUE, _msgpack_load32(uint32_t,local_n), _big_int_zero);
		//case ACS_BIG_INT_VALUE:
		//_big_int_zero:
		//	// FIXME
		//	push_variable_value(_big_int, in_data, local_n, this->trail_);

		//case CS_BIG_FLOAT_16:
		//	again_fixed_trail_if_zero(ACS_BIG_FLOAT_VALUE, _msgpack_load16(uint16_t,local_n), _big_float_zero);
		//case CS_BIG_FLOAT_32:
		//	again_fixed_trail_if_zero(ACS_BIG_FLOAT_VALUE, _msgpack_load32(uint32_t,local_n), _big_float_zero);
		//case ACS_BIG_FLOAT_VALUE:
		//_big_float_zero:
		//	// FIXME
		//	push_variable_value(_big_float, in_data, local_n, this->trail_);

		case CS_RAW_16:
			again_fixed_trail_if_zero(ACS_RAW_VALUE, _msgpack_load16(uint16_t,local_n), _raw_zero);
		case CS_RAW_32:
			again_fixed_trail_if_zero(ACS_RAW_VALUE, _msgpack_load32(uint32_t,local_n), _raw_zero);
#endif // 0
        case CS_ARRAY_16:
            return this->deserialize_container(
                out_object,
                self::load_bytes<std::int16_t>(local_n),
                CT_ARRAY_ITEM);
        case CS_ARRAY_32:
            // FIXME security guard
            return this->deserialize_container(
                out_object,
                self::load_bytes<std::int32_t>(local_n),
                CT_ARRAY_ITEM);

        case CS_MAP_16:
            return this->deserialize_container(
                out_object,
                self::load_bytes<std::int16_t>(local_n),
                CT_MAP_KEY);
        case CS_MAP_32:
            // FIXME security guard
            return this->deserialize_container(
                out_object,
                self::load_bytes<std::int32_t>(local_n),
                CT_MAP_KEY);

        case ACS_RAW_VALUE:
        _raw_zero:
            self::deserialize_raw(
                this->user_, local_n, this->trail_, out_object);
            break;

        default:
            PSYQ_ASSERT(false);
            return self::deserialize_result_FAILED;
        }
        return this->deserialize_pop(out_object);
    }

    //-------------------------------------------------------------------------
    private: self::deserialize_result deserialize_container(
        msgpack_object& out_object,
        std::size_t const in_count,
        unsigned const in_ct)
    {
        PSYQ_ASSERT(in_ct == CT_ARRAY_ITEM || in_ct == CT_MAP_KEY);
        if (MSGPACK_EMBED_STACK_SIZE <= this->top_)
        {
            PSYQ_ASSERT(false);
            return self::deserialize_result_FAILED; // FIXME
        }
        auto& local_stack_top(this->stack_[this->top_]);
        int local_deserialize_container(
            in_ct == CT_ARRAY_ITEM?
                self::deserialize_array(
                    this->user_, in_count, local_stack_top.obj):
                in_ct == CT_MAP_KEY?
                    self::deserialize_map(
                        this->user_, in_count, local_stack_top.obj):
                    -1);
        if (local_deserialize_container < 0)
        {
            PSYQ_ASSERT(false);
            return self::deserialize_result_FAILED;
        }

        if (0 < in_count)
        {
            // オブジェクトをスタックに積む。
            local_stack_top.ct = in_ct;
            local_stack_top.count = in_count;
            ++this->top_;
            return self::deserialize_result_CONTINUE;
        }
        out_object = local_stack_top.obj;
        return this->deserialize_pop(out_object);
    }

    //-------------------------------------------------------------------------
    /** @brief スタックからMessagePackオブジェクトを取り出す。
        @param[out] out_object スタックから取り出したオブジェクトを格納する。
     */
    private: self::deserialize_result deserialize_pop(
        msgpack_object& out_object)
    {
        while (0 < this->top_)
        {
            self::stack& local_stack_top(this->stack_[this->top_ - 1]);
            switch (local_stack_top.ct)
            {
            case CT_ARRAY_ITEM:
                self::deserialize_array_item(local_stack_top.obj, out_object);
                --local_stack_top.count;
                if (0 < local_stack_top.count)
                {
                    return self::deserialize_result_CONTINUE;
                }
                break;

            case CT_MAP_KEY:
                local_stack_top.map_key = out_object;
                local_stack_top.ct = CT_MAP_VALUE;
                return self::deserialize_result_CONTINUE;

            case CT_MAP_VALUE:
                self::deserialize_map_item(
                    local_stack_top.obj,
                    local_stack_top.map_key,
                    out_object);
                --local_stack_top.count;
                if (0 < local_stack_top.count)
                {
                    local_stack_top.ct = CT_MAP_KEY;
                    return self::deserialize_result_CONTINUE;
                }
                break;

            default:
                PSYQ_ASSERT(false);
                return self::deserialize_result_FAILED;
            }

            // オブジェクトをスタックから取り出す。
            out_object = local_stack_top.obj;
            --this->top_;
        }
        return self::deserialize_result_FINISH;
    }

    //-------------------------------------------------------------------------
    /** @brief MessagePackオブジェクトに整数を格納する。
        @param[in] in_integer  格納する整数値。
        @param[out] out_object 整数を格納するMessagePackオブジェクト。
        @return 必ず0。
     */
    private: template<typename template_value_type>
    static int deserialize_integer(
        template_value_type const in_integer,
        msgpack_object& out_object)
    {
        if (0 <= in_integer)
        {
            out_object.type = MSGPACK_OBJECT_POSITIVE_INTEGER;
            out_object.via.positive_integer = in_integer;
        }
        else
        {
            out_object.type = MSGPACK_OBJECT_NEGATIVE_INTEGER;
            out_object.via.negative_integer = in_integer;
        }
        return 0;
    }

    /** @brief MessagePackオブジェクトに浮動小数点実数を格納する。
        @param[in] in_integer  格納する実数値。
        @param[out] out_object 実数を格納するMessagePackオブジェクト。
        @return 必ず0。
     */
    private: template<typename template_value_type>
    static int deserialize_real(
        template_value_type const in_real,
        msgpack_object& out_object)
    {
        out_object.type = MSGPACK_OBJECT_DOUBLE;
        out_object.via.floating_point = in_real;
        return 0;
    }

    /** @brief MessagePackオブジェクトにRAW配列を格納する。
        @param[in]  in_size    RAW配列の容量。
        @param[out] out_object RAW配列を格納するMessagePackオブジェクト。
        @return 必ず0。
     */
    private: static int deserialize_raw(
        self::user& out_user,
        void const* const in_pointer,
        std::size_t const in_size,
        msgpack_object& out_object)
    {
        out_object.type = MSGPACK_OBJECT_RAW;
        out_object.via.raw.data = static_cast<char const*>(in_pointer);
        out_object.via.raw.size = in_size;
        out_user.referenced = true;
        return 0;
    }

    /** @brief MessagePackオブジェクトに配列を格納する。
        @param[in,out] io_user
        @param[in]     in_size    配列の容量。
        @param[out]    out_object 配列を格納するMessagePackオブジェクト。
        @retval 0以上 成功。
        @retval 0未満 失敗。
     */
    private: static int deserialize_array(
        self::user const& io_user,
        std::size_t const in_size,
        msgpack_object& out_object)
    {
        out_object.type = MSGPACK_OBJECT_ARRAY;
        out_object.via.array.size = 0;
        out_object.via.array.data = static_cast<msgpack_object*>(
            msgpack_zone_malloc(
                io_user.zone, in_size * sizeof(msgpack_object)));
        return out_object.via.array.data != nullptr? 0: -1;
    }

    /** @brief オブジェクト配列に要素を追加する。
        @param[in,out] io_object 要素を追加するオブジェクト配列。
        @param[in]     in_item   追加する要素。
        @return 必ず0。
     */
    private: static int deserialize_array_item(
        msgpack_object& io_object,
        msgpack_object const& in_item)
    {
        io_object.via.array.data[io_object.via.array.size] = in_item;
        ++io_object.via.array.size;
        return 0;
    }

    /** @brief MessagePackオブジェクトに連想配列を格納する。
        @param[in,out] io_user
        @param[in]     in_size    連想配列の容量。
        @param[out]    out_object 連想配列を格納するMessagePackオブジェクト。
        @retval 0以上 成功。
        @retval 0未満 失敗。
     */
    private: static int deserialize_map(
        self::user const& io_user,
        std::size_t const in_size,
        msgpack_object& out_object)
    {
        out_object.type = MSGPACK_OBJECT_MAP;
        out_object.via.map.size = 0;
        out_object.via.map.data = static_cast<msgpack_object_kv*>(
            msgpack_zone_malloc(
                io_user.zone, in_size * sizeof(msgpack_object_kv)));
        return out_object.via.map.data != nullptr? 0: -1;
    }

    /** @brief オブジェクト連想配列に要素を追加する。
        @param[in,out] io_object 要素を追加する連想配列。
        @param[in]     in_key    追加する要素のキー。
        @param[in]     in_mapped 追加する要素の値。
        @return 必ず0。
     */
    private: static int deserialize_map_item(
        msgpack_object& io_object,
        msgpack_object const& in_key,
        msgpack_object const& in_mapped)
    {
        auto& local_map_item(
            io_object.via.map.data[io_object.via.map.size]);
        local_map_item.key = in_key;
        local_map_item.val = in_mapped;
        ++io_object.via.map.size;
        return 0;
    }

    //-------------------------------------------------------------------------
    private: static msgpack_object make_stack_root()
    {
        msgpack_object local_object = {};
        return local_object;
    }

    private: static unsigned next_cs(unsigned const in_data)
    {
        return in_data & 0x1f;
    }

    private: template<typename template_value_type>
    static template_value_type load_bytes(void const* const in_value)
    {
#if 0 // PSYQ_PLATFORM_LITTLE_ENDIAN
        // MessagePackのネットワークバイトオーダーはビッグエンディアンなので、
        // リトルエンディアンに変換する。
        return static_cast<template_value_type>(
            _msgpack_be16(*static_cast<template_value_type const*>(in_value)));
#else
        PSYQ_ASSERT(false); /// @todo エンディアンをちゃんとチェックすること。
        return *static_cast<template_value_type const*>(in_value);
#endif
    }

    //-------------------------------------------------------------------------
    private: std::array<self::stack, MSGPACK_EMBED_STACK_SIZE> stack_;
    private: self::user user_;
    private: std::size_t cs_;
    private: std::size_t trail_;
    private: std::size_t top_;
};

#undef push_variable_value
#undef again_fixed_trail_if_zero

#endif // !defined(PSYQ_MESSAGE_PACK_CONTEXT_HPP_)
