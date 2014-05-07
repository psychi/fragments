/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief
 */
#ifndef PSYQ_MESSAGE_PACK_DESIRIALIZE_HPP_
#define PSYQ_MESSAGE_PACK_DESIRIALIZE_HPP_

//#include "psyq/message_pack_serializer.hpp"
//#include "psyq/message_pack_pool.hpp"
//#include "psyq/message_pack_object.hpp"

/// psyq::message_pack::deserializer のスタック限界数のデフォルト値。
#ifndef PSYQ_MESSAGE_PACK_DESERIALIZER_STACK_CAPACITY_DEFAULT
#define PSYQ_MESSAGE_PACK_DESERIALIZER_STACK_CAPACITY_DEFAULT 32
#endif // !defined(PSYQ_MESSAGE_PACK_SERIALIZER_STACK_CAPACITY_DEFAULT)

namespace psyq
{
    namespace message_pack
    {
        /// @cond
        template<typename = std::uint8_t const*> class istream;
        template<
            typename = psyq::message_pack::pool<>,
            std::size_t
                = PSYQ_MESSAGE_PACK_DESERIALIZER_STACK_CAPACITY_DEFAULT>
                    class deserializer;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_iterator>
class psyq::message_pack::istream
{
    private: typedef istream<template_iterator> self;

    public: typedef template_iterator iterator;

    public: istream(
        typename self::iterator const in_begin,
        typename self::iterator const in_end)
    :
        iterator_(in_begin),
        begin_(in_begin),
        end_(in_end),
        size_(std::distance(in_begin, in_end))
    {}

    public: typename self::iterator const& begin() const
    {
        return this->begin_;
    }
    public: typename self::iterator const& end() const
    {
        return this->end_;
    }
    public: typename self::iterator const& current() const
    {
        return this->iterator_;
    }

    public: typename self::iterator tellg() const
    {
        return this->iterator_;
    }

    public: self& seekg(
        signed const in_offset,
        std::ios::seek_dir const in_direction)
    {
        switch (in_direction)
        {
        case std::ios::cur:
            if (in_offset < 0)
            {
                auto const local_distance(
                    std::distance(this->begin_, this->iterator_));
                this->iterator_ = -in_offset < local_distance?
                    std::prev(this->iterator_, -in_offset): this->begin_;
            }
            else if (0 < in_offset)
            {
                auto const local_distance(
                    std::distance(this->iterator_, this->end_));
                this->iterator_ = in_offset < local_distance?
                    std::next(this->iterator_, in_offset): this->end_;
            }
            break;
        case std::ios::beg:
            if (in_offset <= 0)
            {
                this->iterator_ = this->begin_;
            }
            else if (this->size_ <= unsigned(in_offset))
            {
                this->iterator_ = this->end_;
            }
            else
            {
                this->iterator_ = std::next(this->begin_, in_offset);
            }
            break;
        case std::ios::end:
            if (0 <= in_offset)
            {
                this->iterator_ = this->end_;
            }
            else if (this->size_ <= unsigned(-in_offset))
            {
                this->iterator_ = this->begin_;
            }
            else
            {
                this->iterator_ = std::prev(this->begin_, -in_offset);
            }
            break;
        }
        return *this;
    }

    private: typename self::iterator iterator_;
    private: typename self::iterator begin_;
    private: typename self::iterator end_;
    private: std::size_t size_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_pool, std::size_t template_stack_capacity>
class psyq::message_pack::deserializer
{
    /// thisが指す値の型。
    private: typedef deserializer<template_pool, template_stack_capacity> self;

    public: typedef template_pool pool;

    /** @brief 直列化途中のコンテナのスタック限界数。
     */
    public: static std::size_t const stack_capacity = template_stack_capacity;

    private: enum deserialize_result
    {
        deserialize_result_FAILED,
        deserialize_result_FINISH,
        deserialize_result_ABORT,
        deserialize_result_CONTINUE,
    };

    private: enum phase
    {
        phase_HEADER,
        phase_STRING,
        phase_BINARY,
        phase_EXTENDED_BINARY,
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
        std::size_t rest_size;              ///< コンテナ要素の残数。
        typename self::stack_kind kind;     ///< 復元中のオブジェクトの種別。
    };

    //-------------------------------------------------------------------------
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
        this->phase_ = self::phase_HEADER;
        this->trail_ = 0;
        this->stack_size_ = 0;
        this->stack_.front().object.reset();
        this->allocate_raw_ = true;// !this->pool_.is_from(in_data);

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
        return this->stack_.front().object;
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
                this->phase_ == self::phase_HEADER?
                    this->deserialize_header(local_object, in_end):
                    this->deserialize_value(local_object, in_end))
            {
            case self::deserialize_result_FINISH:
                this->stack_[0].object = local_object;
                ++this->deserialize_iterator_;
                return 1;

            case self::deserialize_result_CONTINUE:
                this->phase_ = self::phase_HEADER;
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
    private: typename self::deserialize_result deserialize_header(
        psyq::message_pack::object& out_object,
        std::uint8_t const* const in_end)
    {
        // MessagePackの直列化形式によって、復元処理を分岐する。
        auto const local_header(*this->deserialize_iterator_);
        if (local_header <= psyq::message_pack::header_FIX_INT_MAX)
        {
            // [0x00, 0x7f]: positive fixnum
            out_object = local_header;
            return this->deserialize_stack(out_object);
        }
        else if (local_header <= psyq::message_pack::header_FIX_MAP_MAX)
        {
            // [0x80, 0x8f]: fix map
            return this->deserialize_container(
                out_object,
                local_header & 0x0f,
                self::stack_kind_MAP_KEY);
        }
        else if (local_header <= psyq::message_pack::header_FIX_ARRAY_MAX)
        {
            // [0x90, 0x9f]: fix array
            return this->deserialize_container(
                out_object,
                local_header & 0x0f,
                self::stack_kind_ARRAY_ITEM);
        }
        else if (local_header == psyq::message_pack::header_FIX_STR_MIN)
        {
            // [0xa0, 0xbf]: fix str
            self::deserialize_string(
                out_object, this->pool_, nullptr, 0, false);
            return this->deserialize_stack(out_object);
        }
        else if (local_header <= psyq::message_pack::header_FIX_STR_MAX)
        {
            // [0xa0, 0xbf]: fix str
            this->trail_ = local_header & 0x1f;
            this->phase_ = self::phase_STRING;
        }
        else if (local_header == psyq::message_pack::header_NIL)
        {
            // 0xc0: nil
            out_object.reset();
            return this->deserialize_stack(out_object);
        }
        else if (local_header == psyq::message_pack::header_NEVER_USED)
        {
            // 0xc1: never used
            PSYQ_ASSERT(false);
            return self::deserialize_result_FAILED;
        }
        else if (local_header == psyq::message_pack::header_FALSE)
        {
            // 0xc2: false
            out_object = false;
            return this->deserialize_stack(out_object);
        }
        else if (local_header == psyq::message_pack::header_TRUE)
        {
            // 0xc3: true
            out_object = true;
            return this->deserialize_stack(out_object);
        }
        else if (local_header <= psyq::message_pack::header_BIN32)
        {
            // 0xc4: bin 8
            // 0xc5: bin 16
            // 0xc6: bin 32
            this->trail_
                = 1 << (local_header - psyq::message_pack::header_BIN8);
            this->phase_ = local_header;
        }
        else if (local_header <= psyq::message_pack::header_EXT32)
        {
            // 0xc7: ext 8
            // 0xc8: ext 16
            // 0xc9: ext 32
            this->trail_
                = 1 << (local_header - psyq::message_pack::header_EXT8);
            this->phase_ = local_header;
        }
        else if (local_header <= psyq::message_pack::header_INT64)
        {
            // 0xca: float 32
            // 0xcb: float 64
            // 0xcc: unsigned int 8
            // 0xcd: unsigned int 16
            // 0xce: unsigned int 32
            // 0xcf: unsigned int 64
            // 0xd0: signed int 8
            // 0xd1: signed int 16
            // 0xd2: signed int 32
            // 0xd3: signed int 64
            this->trail_ = 1 << (local_header & 0x3);
            this->phase_ = local_header;
        }
        else if (local_header <= psyq::message_pack::header_FIX_EXT16)
        {
            // 0xd4: fix ext 1
            // 0xd5: fix ext 2
            // 0xd6: fix ext 4
            // 0xd7: fix ext 8
            // 0xd8: fix ext 16
            this->trail_ = 1 + (
                1 << (local_header - psyq::message_pack::header_FIX_EXT1));
            this->phase_ = self::phase_EXTENDED_BINARY;
        }
        else if (local_header <= psyq::message_pack::header_STR32)
        {
            // 0xd9: str 8
            // 0xda: str 16
            // 0xdb: str 32
            this->trail_
                = 1 << (local_header - psyq::message_pack::header_STR8);
            this->phase_ = local_header;
        }
        else if (local_header <= psyq::message_pack::header_MAP32)
        {
            // 0xdc: array 16
            // 0xdd: array 32
            // 0xde: map 16
            // 0xdf: map 32
            this->trail_ = 2 << (local_header & 0x1);
            this->phase_ = local_header;
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
    private: typename self::deserialize_result deserialize_value(
        psyq::message_pack::object& out_object,
        std::uint8_t const* const in_end)
    {
        if (in_end < this->deserialize_iterator_ + this->trail_)
        {
            return self::deserialize_result_ABORT;
        }
        auto const local_data(this->deserialize_iterator_);
        PSYQ_ASSERT(0 < this->trail_);
        this->deserialize_iterator_ += this->trail_ - 1;
        switch (this->phase_)
        {
        // 無符号整数
        case psyq::message_pack::header_UINT8:
            out_object = *local_data;
            break;
        case psyq::message_pack::header_UINT16:
            out_object
                = self::load_big_endian_integer<std::uint16_t>(local_data);
            break;
        case psyq::message_pack::header_UINT32:
            out_object
                = self::load_big_endian_integer<std::uint32_t>(local_data);
            break;
        case psyq::message_pack::header_UINT64:
            out_object
                = self::load_big_endian_integer<std::uint64_t>(local_data);
            break;

        // 有符号整数
        case psyq::message_pack::header_INT8:
            out_object = static_cast<std::int8_t>(*local_data);
            break;
        case psyq::message_pack::header_INT16:
            out_object
                = self::load_big_endian_integer<std::int16_t>(local_data);
            break;
        case psyq::message_pack::header_INT32:
            out_object
                = self::load_big_endian_integer<std::int32_t>(local_data);
            break;
        case psyq::message_pack::header_INT64:
            out_object
                = self::load_big_endian_integer<std::int64_t>(local_data);
            break;

        // 浮動小数点数
        case psyq::message_pack::header_FLOAT32:
        {
            union {std::uint32_t integer; float real;} local_value;
            local_value.integer
                = self::load_big_endian_integer<std::uint32_t>(local_data);
            out_object = local_value.real;
            break;
        }
        case psyq::message_pack::header_FLOAT64:
        {
            union {std::uint64_t integer; double real;} local_value;
            local_value.integer
                = self::load_big_endian_integer<std::uint64_t>(local_data);
            out_object = local_value.real;
            break;
        }

        // 文字列
        case psyq::message_pack::header_STR8:
            this->trail_ = *local_data;
            goto PSYQ_MESSAGE_PACK_DESERIALIZE_STRING;
        case psyq::message_pack::header_STR16:
            this->trail_
                = self::load_big_endian_integer<std::uint16_t>(local_data);
            goto PSYQ_MESSAGE_PACK_DESERIALIZE_STRING;
        case psyq::message_pack::header_STR32:
            this->trail_
                = self::load_big_endian_integer<std::uint32_t>(local_data);
            goto PSYQ_MESSAGE_PACK_DESERIALIZE_STRING;
        PSYQ_MESSAGE_PACK_DESERIALIZE_STRING:
            if (0 < this->trail_)
            {
                this->phase_ = self::phase_STRING;
                ++this->deserialize_iterator_;
                return this->deserialize_value(out_object, in_end);
            }
            self::deserialize_string(
                out_object, this->pool_, nullptr, 0, false);
            break;
        case self::phase_STRING:
            self::deserialize_string(
                out_object,
                this->pool_,
                local_data,
                this->trail_,
                this->allocate_raw_);
            break;

        // バイナリ
        case psyq::message_pack::header_BIN8:
            this->trail_ = *local_data;
            goto PSYQ_MESSAGE_PACK_DESERIALIZE_BINARY;
        case psyq::message_pack::header_BIN16:
            this->trail_
                = self::load_big_endian_integer<std::uint16_t>(local_data);
            goto PSYQ_MESSAGE_PACK_DESERIALIZE_BINARY;
        case psyq::message_pack::header_BIN32:
            this->trail_
                = self::load_big_endian_integer<std::uint32_t>(local_data);
            goto PSYQ_MESSAGE_PACK_DESERIALIZE_BINARY;
        PSYQ_MESSAGE_PACK_DESERIALIZE_BINARY:
            if (0 < this->trail_)
            {
                this->phase_ = self::phase_BINARY;
                ++this->deserialize_iterator_;
                return this->deserialize_value(out_object, in_end);
            }
            self::deserialize_binary(
                out_object, this->pool_, nullptr, 0, false);
            break;
        case self::phase_BINARY:
            self::deserialize_binary(
                out_object,
                this->pool_,
                local_data,
                this->trail_,
                this->allocate_raw_);
            break;

        // 拡張バイナリ
        case psyq::message_pack::header_EXT8:
            this->trail_ = *local_data;
            goto PSYQ_MESSAGE_PACK_DESERIALIZE_EXTENDED_BINARY;
        case psyq::message_pack::header_EXT16:
            this->trail_
                = self::load_big_endian_integer<std::uint16_t>(local_data);
            goto PSYQ_MESSAGE_PACK_DESERIALIZE_EXTENDED_BINARY;
        case psyq::message_pack::header_EXT32:
            this->trail_
                = self::load_big_endian_integer<std::uint32_t>(local_data);
            goto PSYQ_MESSAGE_PACK_DESERIALIZE_EXTENDED_BINARY;
        PSYQ_MESSAGE_PACK_DESERIALIZE_EXTENDED_BINARY:
            ++this->deserialize_iterator_;
            if (0 < this->trail_)
            {
                this->phase_ = self::phase_EXTENDED_BINARY;
                ++this->trail_;
                return this->deserialize_value(out_object, in_end);
            }
            self::deserialize_extended_binary(
                out_object,
                this->pool_,
                this->deserialize_iterator_ - 1,
                1,
                this->allocate_raw_);
            break;
        case self::phase_EXTENDED_BINARY:
            PSYQ_ASSERT(0 < this->trail_);
            self::deserialize_extended_binary(
                out_object,
                this->pool_,
                local_data,
                this->trail_,
                this->allocate_raw_);
            break;

        // 配列
        case psyq::message_pack::header_ARRAY16:
            return this->deserialize_container(
                out_object,
                self::load_big_endian_integer<std::uint16_t>(local_data),
                self::stack_kind_ARRAY_ITEM);
        case psyq::message_pack::header_ARRAY32:
            return this->deserialize_container(
                out_object,
                self::load_big_endian_integer<std::uint32_t>(local_data),
                self::stack_kind_ARRAY_ITEM);

        // 連想配列
        case psyq::message_pack::header_MAP16:
            return this->deserialize_container(
                out_object,
                self::load_big_endian_integer<std::uint16_t>(local_data),
                self::stack_kind_MAP_KEY);
        case psyq::message_pack::header_MAP32:
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
    private: typename self::deserialize_result deserialize_container(
        psyq::message_pack::object& out_object,
        std::size_t const in_capacity,
        typename self::stack_kind const in_kind)
    {
        PSYQ_ASSERT(
            in_kind == self::stack_kind_ARRAY_ITEM
            || in_kind == self::stack_kind_MAP_KEY);
        if (this->stack_.size() <= this->stack_size_)
        {
            PSYQ_ASSERT(false);
            return self::deserialize_result_FAILED;
        }
        auto& local_stack_top(this->stack_[this->stack_size_]);
        bool const local_deserialize_container(
            in_kind == self::stack_kind_ARRAY_ITEM?
                self::deserialize_array(
                    local_stack_top.object, this->pool_, in_capacity):
                in_kind == self::stack_kind_MAP_KEY?
                    self::deserialize_map(
                        local_stack_top.object, this->pool_, in_capacity):
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
            local_stack_top.rest_size = in_capacity;
            ++this->stack_size_;
            return self::deserialize_result_CONTINUE;
        }
        else
        {
            out_object = local_stack_top.object;
            return this->deserialize_stack(out_object);
        }
    }

    //-------------------------------------------------------------------------
    /** @brief スタックからMessagePackオブジェクトを取り出す。
        @param[out] out_object スタックから取り出したオブジェクトが格納される。
     */
    private: typename self::deserialize_result deserialize_stack(
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
                --local_stack_top.rest_size;
                if (0 < local_stack_top.rest_size)
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
                --local_stack_top.rest_size;
                if (0 < local_stack_top.rest_size)
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
    /** @brief MessagePackオブジェクトに文字列を格納する。
        @param[out] out_object 文字列を格納するMessagePackオブジェクト。
        @param[in]  in_data    文字列の先頭位置。
        @param[in]  in_size    文字列のバイト数。
     */
    private: static void deserialize_string(
        psyq::message_pack::object& out_object,
        typename self::pool& io_pool,
        void const* const in_data,
        std::size_t const in_size,
        bool const in_allocate)
    PSYQ_NOEXCEPT
    {
        typedef psyq::message_pack::object::string::value_type element;
        out_object.set_string(
            self::make_raw<element>(io_pool, in_data, in_size, in_allocate),
            in_size);
    }

    /** @brief MessagePackオブジェクトにバイナリを格納する。
        @param[out] out_object バイナリを格納するMessagePackオブジェクト。
        @param[in]  in_data    バイナリの先頭位置。
        @param[in]  in_size    バイナリのバイト数。
     */
    private: static void deserialize_binary(
        psyq::message_pack::object& out_object,
        typename self::pool& io_pool,
        void const* const in_data,
        std::size_t const in_size,
        bool const in_allocate)
    PSYQ_NOEXCEPT
    {
        typedef psyq::message_pack::object::binary::value_type element;
        out_object.set_binary(
            self::make_raw<element>(io_pool, in_data, in_size, in_allocate),
            in_size);
    }

    /** @brief MessagePackオブジェクトに拡張バイナリを格納する。
        @param[out] out_object 拡張バイナリを格納するMessagePackオブジェクト。
        @param[in]  in_data    拡張バイナリの先頭位置。
        @param[in]  in_size    拡張バイナリのバイト数。
     */
    private: static void deserialize_extended_binary(
        psyq::message_pack::object& out_object,
        typename self::pool& io_pool,
        void const* const in_data,
        std::size_t const in_size,
        bool const in_allocate)
    PSYQ_NOEXCEPT
    {
        typedef psyq::message_pack::object::extended_binary::value_type
            element;
        out_object.set_extended_binary(
            self::make_raw<element>(io_pool, in_data, in_size, in_allocate),
            in_size);
    }

    private: template<typename template_value>
    static template_value* make_raw(
        typename self::pool& io_pool,
        void const* const in_data,
        std::size_t const in_size,
        bool const in_allocate)
    {
        if (!in_allocate || in_size <= 0)
        {
            return static_cast<template_value*>(in_data);
        }
        auto const local_data(io_pool.allocate(in_size, 1));
        if (local_data == nullptr)
        {
            PSYQ_ASSERT(false);
            return nullptr;
        }
        std::char_traits<std::int8_t>::copy(
            static_cast<std::int8_t*>(local_data),
            static_cast<std::int8_t const*>(in_data),
            in_size);
        return static_cast<template_value*>(local_data);
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
        typename self::pool& io_pool,
        std::size_t const in_capacity)
    {
        auto& local_array(
            out_object.set_array(
                0 < in_capacity?
                    static_cast<psyq::message_pack::object*>(
                        io_pool.allocate(
                            in_capacity * sizeof(psyq::message_pack::object),
                            sizeof(std::int64_t))):
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
        typename self::pool& io_pool,
        std::size_t const in_capacity)
    {
        auto& local_map(
            out_object.set_map(
                0 < in_capacity?
                    static_cast<psyq::message_pack::object::map::pointer>(
                        io_pool.allocate(
                            in_capacity * sizeof(
                                psyq::message_pack::object::map::value_type),
                            sizeof(std::int64_t))):
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
                (static_cast<std::uint8_t >(local_bytes[1])));
        }
        case 4:
        {
            return static_cast<template_value_type>(
                (static_cast<std::uint32_t>(local_bytes[0]) << 24) |
                (static_cast<std::uint32_t>(local_bytes[1]) << 16) |
                (static_cast<std::uint16_t>(local_bytes[2]) <<  8) |
                (static_cast<std::uint8_t >(local_bytes[3])));
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
                (static_cast<std::uint8_t >(local_bytes[7])));
        }
        default:
            PSYQ_ASSERT(false);
            return 0;
        }
    }

    //-------------------------------------------------------------------------
    private: std::array<typename self::stack, template_stack_capacity> stack_;
    private: typename self::pool pool_;
    private: std::uint8_t const* deserialize_iterator_;
    private: std::size_t phase_; ///< 復元するオブジェクトの種別。
    private: std::size_t trail_;
    private: std::size_t stack_size_; ///< スタックの要素数。
    private: bool allocate_raw_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        inline void message_pack_deserializer()
        {
            psyq::message_pack::serializer<> local_serializer;
            local_serializer.make_array(17);
            local_serializer << (std::numeric_limits<std::int64_t>::min)();
            local_serializer << (std::numeric_limits<std::int32_t>::min)();
            local_serializer << (std::numeric_limits<std::int16_t>::min)();
            local_serializer << (std::numeric_limits<std::int8_t >::min)();
            local_serializer << -0x20;
            local_serializer << false;
            local_serializer << true;
            local_serializer << 0x7f;
            local_serializer << (std::numeric_limits<std::uint8_t >::max)();
            local_serializer << (std::numeric_limits<std::uint16_t>::max)();
            local_serializer << (std::numeric_limits<std::uint32_t>::max)();
            local_serializer << (std::numeric_limits<std::uint64_t>::max)();
            local_serializer << std::string("0123456789ABCDEFGHIJKLMNOPQRSTU");
            local_serializer << std::string(0xff, 'x');
            local_serializer << std::string(0xffff, 'y');
            local_serializer << std::string(0x10000, 'z');
            local_serializer.write_nil();

            auto const local_message_string(local_serializer.get_stream().str());
            psyq::message_pack::deserializer<> local_deserializer;
            std::size_t local_message_offset(0);
            local_deserializer.deserialize(
                local_message_string.data(),
                local_message_string.size(),
                local_message_offset);
            auto const& local_root(local_deserializer.get_root_object());
            PSYQ_ASSERT(local_root.get_array() != nullptr);
            PSYQ_ASSERT(
                local_root.get_array()->at(0)
                    == psyq::message_pack::object(
                        (std::numeric_limits<std::int64_t>::min)()));
            PSYQ_ASSERT(
                local_root.get_array()->at(1)
                    == psyq::message_pack::object(
                        (std::numeric_limits<std::int32_t>::min)()));
            PSYQ_ASSERT(
                local_root.get_array()->at(2)
                    == psyq::message_pack::object(
                        (std::numeric_limits<std::int16_t>::min)()));
            PSYQ_ASSERT(
                local_root.get_array()->at(3)
                    == psyq::message_pack::object(
                        (std::numeric_limits<std::int8_t>::min)()));
        }
    }
}

#endif // !defined(PSYQ_MESSAGE_PACK_DESIRIALIZE_HPP_)
