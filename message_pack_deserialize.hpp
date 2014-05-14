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
        template<typename = std::int8_t const*> class istream;
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
    public: typedef typename
        std::iterator_traits<template_iterator>::value_type char_type;
    public: typedef std::char_traits<typename self::char_type> traits_type;
    public: typedef typename self::traits_type::int_type int_type;
    //public: typedef typename self::traits_type::pos_type pos_type;
    //public: typedef typename self::traits_type::off_type off_type;
    public: typedef std::size_t pos_type;
    public: typedef int off_type;

    public: istream(
        typename self::iterator const in_begin,
        typename self::iterator const in_end)
    :
        current_(in_begin),
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
        return this->current_;
    }

    public: bool eof() const
    {
        return this->end() <= this->current();
    }
    public: bool fail() const
    {
        return false;
    }

    public: typename self::char_type get()
    {
        if (this->eof())
        {
            return 0;
        }
        auto const local_char(*this->current());
        ++this->current_;
        return local_char;
    }

    public: self& read(
        typename self::char_type* const out_buffer,
        std::size_t const in_length)
    {
        auto const local_length(
            std::min<std::size_t>(in_length, this->end() - this->current()));
        std::memcpy(
            out_buffer,
            this->current(),
            sizeof(typename self::char_type) * local_length);
        this->current_ += local_length;
        return *this;
    }

    public: typename self::pos_type tellg() const
    {
        return this->current() - this->begin();
    }

    public: self& seekg(typename self::pos_type const in_offset)
    {
        this->current_ = in_offset < this->size_?
            std::next(this->begin_, in_offset): this->end_;
        return *this;
    }
    public: self& seekg(
        typename self::off_type const in_offset,
        std::ios::seek_dir const in_direction)
    {
        switch (in_direction)
        {
        case std::ios::beg:
            if (0 < in_offset)
            {
                return this->seekg(
                    static_cast<typename self::pos_type>(in_offset));
            }
            this->current_ = this->begin_;
            break;
        case std::ios::end:
            if (0 <= in_offset)
            {
                this->current_ = this->end_;
            }
            else if (this->size_ <= unsigned(-in_offset))
            {
                this->current_ = this->begin_;
            }
            else
            {
                this->current_ = std::prev(this->begin_, -in_offset);
            }
            break;
        case std::ios::cur:
            if (in_offset < 0)
            {
                auto const local_distance(
                    std::distance(this->begin_, this->current_));
                this->current_ = -in_offset < local_distance?
                    std::prev(this->current_, -in_offset): this->begin_;
            }
            else if (0 < in_offset)
            {
                auto const local_distance(
                    std::distance(this->current_, this->end_));
                this->current_ = in_offset < local_distance?
                    std::next(this->current_, in_offset): this->end_;
            }
            break;
        }
        return *this;
    }

    private: typename self::iterator current_;
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

    private: enum read_result
    {
        read_result_FAILED,
        read_result_FINISH,
        read_result_ABORT,
        read_result_CONTINUE,
    };

    /// 復元中のオブジェクトの種別。
    private: enum stack_kind
    {
        stack_kind_ARRAY_ELEMENT, ///< 配列の要素。
        stack_kind_MAP_KEY,       ///< 連想配列の要素のキー。
        stack_kind_MAP_VALUE,     ///< 連想配列の要素の値。
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
        auto const local_data_begin(static_cast<std::uint8_t const*>(in_data));
        psyq::message_pack::istream<std::uint8_t const*> local_istream(
            local_data_begin, local_data_begin + in_size);
        psyq::message_pack::object local_object;
        auto const local_result(
            local_istream.current() < local_istream.end()?
                this->read_object(local_object, local_istream): 0);
        io_offset = local_istream.current() - local_istream.begin();
        return local_result;
    }

    public: psyq::message_pack::object const& get_root_object()
    const PSYQ_NOEXCEPT
    {
        return this->stack_.front().object;
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、最上位のMessagePackオブジェクトを復元する。
        @param[out]    out_object 復元したMessagePackオブジェクトの格納先。
        @param[in,out] io_istream MessagePackを読み込む入力ストリーム。
        @retval 正 MessagePackオブジェクトの復元を終了。
        @retval 0  MessagePackオブジェクトの復元途中で終了。
        @retval 負 復元に失敗。
     */
    public: template<typename template_stream>
    int read_object(
        psyq::message_pack::object& out_object,
        template_stream& io_istream)
    {
        this->stack_.front().object.reset();
        this->stack_size_ = 0;
        this->allocate_raw_ = true;// !this->pool_.is_from(in_data);

        for (;;)
        {
            switch (this->read_sub_object(out_object, io_istream))
            {
            case self::read_result_FINISH:
                this->stack_.front().object = out_object;
                return 1;

            case self::read_result_CONTINUE:
                if (io_istream.eof())
                {
                    out_object.reset();
                    return 0;
                }
                break;

            case self::read_result_ABORT:
                out_object.reset();
                return 0;

            case self::read_result_FAILED:
                out_object.reset();
                return -1;

            default:
                PSYQ_ASSERT(false);
                out_object.reset();
                return -1;
            }
        }
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、MessagePackオブジェクトを復元する。
        @param[out]    out_object 復元したMessagePackオブジェクトの格納先。
        @param[in,out] io_istream MessagePackを読み込む入力ストリーム。
     */
    private: template<typename template_stream>
    typename self::read_result read_sub_object(
        psyq::message_pack::object& out_object,
        template_stream& io_istream)
    {
        // ストリームからMessagePack直列化形式を読み込む。
        auto const local_header(static_cast<unsigned>(io_istream.get()));
        if (io_istream.fail())
        {
            PSYQ_ASSERT(false);
            return self::read_result_FAILED;
        }

        // MessagePackの直列化形式によって、復元処理を分岐する。
        std::size_t local_trail(0);
        if (local_header <= psyq::message_pack::header_FIX_INT_MAX)
        {
            // [0x00, 0x7f]: positive fixnum
            out_object = local_header;
        }
        else if (local_header <= psyq::message_pack::header_FIX_MAP_MAX)
        {
            // [0x80, 0x8f]: fix map
            return this->read_container<self::stack_kind_MAP_KEY>(
                out_object, local_header & 0x0f);
        }
        else if (local_header <= psyq::message_pack::header_FIX_ARRAY_MAX)
        {
            // [0x90, 0x9f]: fix array
            return this->read_container<self::stack_kind_ARRAY_ELEMENT>(
                out_object, local_header & 0x0f);
        }
        else if (local_header <= psyq::message_pack::header_FIX_STR_MAX)
        {
            // [0xa0, 0xbf]: fix str
            return this->read_raw<psyq::message_pack::object::string>(
                out_object, io_istream, local_header & 0x1f);
        }
        else if (local_header == psyq::message_pack::header_NIL)
        {
            // 0xc0: nil
            out_object.reset();
        }
        else if (local_header == psyq::message_pack::header_NEVER_USED)
        {
            // 0xc1: never used
            PSYQ_ASSERT(false);
            return self::read_result_FAILED;
        }
        else if (local_header == psyq::message_pack::header_FALSE)
        {
            // 0xc2: false
            out_object = false;
        }
        else if (local_header == psyq::message_pack::header_TRUE)
        {
            // 0xc3: true
            out_object = true;
        }
        else if (local_header <= psyq::message_pack::header_BIN32)
        {
            // 0xc4: bin 8
            // 0xc5: bin 16
            // 0xc6: bin 32
            local_trail = 1 << (local_header - psyq::message_pack::header_BIN8);
        }
        else if (local_header <= psyq::message_pack::header_EXT32)
        {
            // 0xc7: ext 8
            // 0xc8: ext 16
            // 0xc9: ext 32
            local_trail = 1 << (local_header - psyq::message_pack::header_EXT8);
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
            local_trail = 1 << (local_header & 0x3);
        }
        else if (local_header <= psyq::message_pack::header_FIX_EXT16)
        {
            // 0xd4: fix ext 1
            // 0xd5: fix ext 2
            // 0xd6: fix ext 4
            // 0xd7: fix ext 8
            // 0xd8: fix ext 16
            return this->read_raw<psyq::message_pack::object::extended>(
                out_object,
                io_istream,
                1 << (local_header - psyq::message_pack::header_FIX_EXT1));
        }
        else if (local_header <= psyq::message_pack::header_STR32)
        {
            // 0xd9: str 8
            // 0xda: str 16
            // 0xdb: str 32
            local_trail = 1 << (local_header - psyq::message_pack::header_STR8);
        }
        else if (local_header <= psyq::message_pack::header_MAP32)
        {
            // 0xdc: array 16
            // 0xdd: array 32
            // 0xde: map 16
            // 0xdf: map 32
            local_trail = 2 << (local_header & 0x1);
        }
        else
        {
            // [0xe0, 0xff]: negative fixnum
            PSYQ_ASSERT(local_header <= 0xff);
            out_object = static_cast<std::int8_t>(local_header);
        }

        // 次。
        if (io_istream.end() < io_istream.current() + local_trail)
        {
            PSYQ_ASSERT(false);
            return self::read_result_ABORT;
        }
        return 0 < local_trail?
            this->read_value(out_object, io_istream, local_header):
            this->update_container_stack(out_object);
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、MessagePack値を復元する。
        @param[out]    out_object 復元した値の格納先。
        @param[in,out] io_istream MessagePackを読み込む入力ストリーム。
        @param[in]     in_header  復元するMessagePack値のヘッダ。
     */
    private: template<typename template_stream>
    typename self::read_result read_value(
        psyq::message_pack::object& out_object,
        template_stream& io_istream,
        unsigned const in_header)
    {
        switch (in_header)
        {
        // 無符号整数
        case psyq::message_pack::header_UINT8:
            return this->read_big_endian<std::uint8_t>(out_object, io_istream);
        case psyq::message_pack::header_UINT16:
            return this->read_big_endian<std::uint16_t>(out_object, io_istream);
        case psyq::message_pack::header_UINT32:
            return this->read_big_endian<std::uint32_t>(out_object, io_istream);
        case psyq::message_pack::header_UINT64:
            return this->read_big_endian<std::uint64_t>(out_object, io_istream);

        // 有符号整数
        case psyq::message_pack::header_INT8:
            return this->read_big_endian<std::int8_t>(out_object, io_istream);
        case psyq::message_pack::header_INT16:
            return this->read_big_endian<std::int16_t>(out_object, io_istream);
        case psyq::message_pack::header_INT32:
            return this->read_big_endian<std::int32_t>(out_object, io_istream);
        case psyq::message_pack::header_INT64:
            return this->read_big_endian<std::int64_t>(out_object, io_istream);

        // 浮動小数点数
        case psyq::message_pack::header_FLOAT32:
            return this->read_big_endian<psyq::message_pack::object::float32>(
                out_object, io_istream);
        case psyq::message_pack::header_FLOAT64:
            return this->read_big_endian<psyq::message_pack::object::float64>(
                out_object, io_istream);

        // 文字列
        case psyq::message_pack::header_STR8:
            return this->read_raw<
                std::uint8_t, psyq::message_pack::object::string>(
                    out_object, io_istream);
        case psyq::message_pack::header_STR16:
            return this->read_raw<
                std::uint16_t, psyq::message_pack::object::string>(
                    out_object, io_istream);
        case psyq::message_pack::header_STR32:
            return this->read_raw<
                std::uint32_t, psyq::message_pack::object::string>(
                    out_object, io_istream);

        // バイナリ
        case psyq::message_pack::header_BIN8:
            return this->read_raw<
                std::uint8_t, psyq::message_pack::object::binary>(
                    out_object, io_istream);
        case psyq::message_pack::header_BIN16:
            return this->read_raw<
                std::uint16_t, psyq::message_pack::object::binary>(
                    out_object, io_istream);
        case psyq::message_pack::header_BIN32:
            return this->read_raw<
                std::uint32_t, psyq::message_pack::object::binary>(
                    out_object, io_istream);

        // 拡張バイナリ
        case psyq::message_pack::header_EXT8:
            return this->read_raw<
                std::uint8_t, psyq::message_pack::object::extended>(
                    out_object, io_istream);
        case psyq::message_pack::header_EXT16:
            return this->read_raw<
                std::uint16_t, psyq::message_pack::object::extended>(
                    out_object, io_istream);
        case psyq::message_pack::header_EXT32:
            return this->read_raw<
                std::uint32_t, psyq::message_pack::object::extended>(
                    out_object, io_istream);

        // 配列
        case psyq::message_pack::header_ARRAY16:
            return this->read_container<
                self::stack_kind_ARRAY_ELEMENT, std::uint16_t>(
                    out_object, io_istream);
        case psyq::message_pack::header_ARRAY32:
            return this->read_container<
                self::stack_kind_ARRAY_ELEMENT, std::uint32_t>(
                    out_object, io_istream);

        // 連想配列
        case psyq::message_pack::header_MAP16:
            return this->read_container<
                self::stack_kind_MAP_KEY, std::uint16_t>(
                    out_object, io_istream);
        case psyq::message_pack::header_MAP32:
            return this->read_container<
                self::stack_kind_MAP_KEY, std::uint32_t>(
                    out_object, io_istream);

        default:
            PSYQ_ASSERT(false);
            return self::read_result_FAILED;
        }
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、MessagePackコンテナを復元する。
        @param[out]    out_object 復元したMessagePackコンテナの格納先。
        @param[in,out] io_istream MessagePackを読み込む入力ストリーム。
     */
    private: template<
        typename self::stack_kind template_kind,
        typename template_length,
        typename template_stream>
    typename self::read_result read_container(
        psyq::message_pack::object& out_object,
        template_stream& io_istream)
    {
        template_length local_length;
        if (!self::read_big_endian(local_length, io_istream))
        {
            PSYQ_ASSERT(false);
            local_length = 0;
        }
        return this->read_container<template_kind>(
            out_object, local_length);
    }
    /** @brief ストリームを読み込み、MessagePackコンテナを復元する。
        @param[out] out_object  復元したMessagePackコンテナの格納先。
        @param[in]  in_capacity 復元するコンテナの容量。
     */
    private: template<typename self::stack_kind template_kind>
    typename self::read_result read_container(
        psyq::message_pack::object& out_object,
        std::size_t const in_capacity)
    {
        if (this->stack_.size() <= this->stack_size_)
        {
            PSYQ_ASSERT(false);
            return self::read_result_FAILED;
        }
        auto& local_stack_top(this->stack_[this->stack_size_]);
        bool local_read_container;
        switch (template_kind)
        {
        case self::stack_kind_ARRAY_ELEMENT:
            local_read_container = self::read_array(
                local_stack_top.object, this->pool_, in_capacity);
            break;
        case self::stack_kind_MAP_KEY:
            local_read_container = self::read_map(
                local_stack_top.object, this->pool_, in_capacity);
            break;
        default:
            PSYQ_ASSERT(false);
            local_read_container = false;
            break;
        }

        if (!local_read_container)
        {
            PSYQ_ASSERT(false);
            return self::read_result_FAILED;
        }
        else if (0 < in_capacity)
        {
            // コンテナをスタックに積む。
            local_stack_top.kind = template_kind;
            local_stack_top.rest_size = in_capacity;
            ++this->stack_size_;
            return self::read_result_CONTINUE;
        }
        else
        {
            out_object = local_stack_top.object;
            return this->update_container_stack(out_object);
        }
    }

    //-------------------------------------------------------------------------
    /** @brief スタックからMessagePackオブジェクトを取り出す。
        @param[in,out] io_object スタックから取り出したオブジェクトが格納される。
     */
    private: typename self::read_result update_container_stack(
        psyq::message_pack::object& io_object)
    PSYQ_NOEXCEPT
    {
        while (0 < this->stack_size_)
        {
            auto& local_stack_top(this->stack_[this->stack_size_ - 1]);
            switch (local_stack_top.kind)
            {
            case self::stack_kind_ARRAY_ELEMENT:
            {
                // 配列に要素を追加する。
                auto const local_array(local_stack_top.object.get_array());
                if (local_array != nullptr)
                {
                    local_array->push_back(io_object);
                }
                else
                {
                    PSYQ_ASSERT(false);
                }

                // 残り要素数を更新する。
                --local_stack_top.rest_size;
                if (0 < local_stack_top.rest_size)
                {
                    return self::read_result_CONTINUE;
                }
                break;
            }

            case self::stack_kind_MAP_KEY:
                local_stack_top.map_key = io_object;
                local_stack_top.kind = self::stack_kind_MAP_VALUE;
                return self::read_result_CONTINUE;

            case self::stack_kind_MAP_VALUE:
            {
                // 連想配列に要素を追加する。
                auto const local_map(local_stack_top.object.get_map());
                if (local_map != nullptr)
                {
                    local_map->push_back(
                        std::make_pair(local_stack_top.map_key, io_object));
                }
                else
                {
                    PSYQ_ASSERT(false);
                    return self::read_result_FAILED;
                }

                // 残り要素数を更新する。
                --local_stack_top.rest_size;
                if (0 < local_stack_top.rest_size)
                {
                    local_stack_top.kind = self::stack_kind_MAP_KEY;
                    return self::read_result_CONTINUE;
                }

                // 連想配列の全要素が揃ったので、ソートする。
                local_map->sort();
                break;
            }

            default:
                PSYQ_ASSERT(false);
                return self::read_result_FAILED;
            }

            // オブジェクトをスタックから取り出す。
            io_object = local_stack_top.object;
            --this->stack_size_;
        }
        return self::read_result_FINISH;
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、MessagePackRAWバイト列を復元する。
        @tparam template_length RAWバイト列のバイト長の型。
        @tparam template_raw    RAWバイト列の型。
        @param[out]    out_object RAWバイト列を格納するMessagePackオブジェクト。
        @param[in,out] io_istream RAWバイト列を読み込むストリーム。
     */
    private: template<
        typename template_length,
        typename template_raw,
        typename template_stream>
    typename self::read_result read_raw(
        psyq::message_pack::object& out_object,
        template_stream& io_istream)
    {
        template_length local_size;
        if (!self::read_big_endian(local_size, io_istream))
        {
            PSYQ_ASSERT(false);
            local_size = 0;
        }
        return this->read_raw<template_raw>(out_object, io_istream, local_size);
    }
    /** @brief ストリームを読み込み、MessagePackRAWバイト列を復元する。
        @tparam template_raw RAWバイト列の型。
        @param[out]    out_object RAWバイト列を格納するMessagePackオブジェクト。
        @param[in,out] io_istream RAWバイト列を読み込むストリーム。
        @param[in]     in_size    RAWバイト列のバイト数。
     */
    private: template<typename template_raw, typename template_stream>
    typename self::read_result read_raw(
        psyq::message_pack::object& out_object,
        template_stream& io_istream,
        std::size_t in_size)
    {
        if (std::is_same<template_raw, psyq::message_pack::object::extended>::value)
        {
            ++in_size;
        }
        auto const local_data(
            static_cast<typename template_raw::pointer>(
                self::make_raw(this->pool_, io_istream, in_size)));
        if (local_data == nullptr)
        {
            in_size = 0;
        }
        template_raw local_raw;
        local_raw.reset(local_data, in_size);
        out_object = local_raw;
        return this->update_container_stack(out_object);
    }

    private: template<typename template_stream>
    static void* make_raw(
        typename self::pool& io_pool,
        template_stream& io_istream,
        std::size_t const in_size)
    {
        auto const local_length(
            in_size / sizeof(typename template_stream::char_type));
        if (local_length <= 0)
        {
            return nullptr;
        }
        typedef psyq::message_pack::object::string::value_type element;
        auto const local_raw(io_pool.allocate(local_length));
        if (local_raw == nullptr)
        {
            PSYQ_ASSERT(false);
            return nullptr;
        }
        io_istream.read(
            static_cast<typename template_stream::char_type*>(local_raw),
            local_length);
        if (io_istream.fail())
        {
            PSYQ_ASSERT(false);
            //io_pool.deallocate(local_raw, local_length);
            return nullptr;
        }
        return local_raw;
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、MessagePack配列を復元する。
        @param[out]    out_object  復元したMessagePack配列の格納先。
        @param[in,out] io_pool     メモリ割当子。
        @param[in]     in_capacity 配列の容量。
        @retval true  成功。
        @retval false 失敗。
     */
    private: static bool read_array(
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

    //-------------------------------------------------------------------------
    /** @brief MessagePackオブジェクトに連想配列を格納する。
        @param[out]    out_object  連想配列を格納するMessagePackオブジェクト。
        @param[in,out] io_user
        @param[in]     in_capacity 連想配列の容量。
        @retval true  成功。
        @retval false 失敗。
     */
    private: static bool read_map(
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

    //-------------------------------------------------------------------------
    private: template<typename template_value, typename template_stream>
    typename self::read_result read_big_endian(
        psyq::message_pack::object& out_object,
        template_stream& io_istream)
    {
        template_value local_value;
        if (!self::read_big_endian(local_value, io_istream))
        {
            return self::read_result_FAILED;
        }
        out_object = local_value;
        return this->update_container_stack(out_object);
    }
    private: template<typename template_value, typename template_stream>
    static bool read_big_endian(
        template_value& out_value,
        template_stream& io_istream)
    {
        return psyq::message_pack::raw_bytes<template_value>::read_stream(
            out_value, io_istream, psyq::message_pack::big_endian);
    }

    //-------------------------------------------------------------------------
    private: typename self::pool pool_;
    private: std::array<typename self::stack, template_stack_capacity> stack_;
    private: std::size_t stack_size_; ///< スタックの要素数。
    private: bool allocate_raw_;
};

#endif // !defined(PSYQ_MESSAGE_PACK_DESIRIALIZE_HPP_)
