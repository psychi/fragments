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
        template<
            typename,
            typename = psyq::message_pack::pool<>,
            std::size_t = PSYQ_MESSAGE_PACK_DESERIALIZER_STACK_CAPACITY_DEFAULT>
                class deserializer;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
    typename template_stream,
    typename template_pool,
    std::size_t template_stack_capacity>
class psyq::message_pack::deserializer
{
    /// thisが指す値の型。
    private: typedef deserializer self;

    /// MessagePackを読み込むための、 std::basic_istream 互換の入力ストリーム。
    public: typedef template_stream stream;
    // 今のところ、1byte単位のストリームしか許容しない。
    static_assert(sizeof(typename self::stream::char_type) == 1, "");

    /// psyq::message_pack::pool 互換のメモリ割当子。
    public: typedef template_pool pool;

    /// 最上位のMessagePackオブジェクト。
    public: typedef psyq::message_pack::root_object<typename self::pool>
        root_object;

    /// 直列化途中のコンテナのスタック限界数。
    public: static std::size_t const stack_capacity = template_stack_capacity;

    private: enum read_result
    {
        read_result_ABORT    = -2,
        read_result_FAILED   = -1,
        read_result_CONTINUE =  0,
        read_result_FINISH   =  1,
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
    public: explicit deserializer(
        typename self::stream in_stream,
        typename self::pool in_pool = self::pool())
    :
        stream_(std::move(in_stream)),
        pool_(std::move(in_pool)),
        stack_size_(0),
        allocate_raw_(true),
        sort_map_(true)
    {}

    public: deserializer(self&& io_source):
        stream_(std::move(io_source.stream_)),
        pool_(std::move(io_source.pool_)),
        stack_(std::move(io_source.stack_)),
        stack_size_(std::move(io_source.stack_size_)),
        allocate_raw_(std::move(io_source.allocate_raw_)),
        sort_map_(std::move(io_source.sort_map_))
    {
        io_source.stack_size_ = 0;
    }

    public: self& operator=(self&& io_source)
    {
        this->stream_ = std::move(io_source.stream_);
        this->pool_ = std::move(io_source.pool_);
        this->stack_.front().object.reset();
        this->stack_size_ = 0;
        this->allocate_raw_ = true;
        this->sort_map_ = true;
        return *this;
    }

    private: deserializer(self const&);
    private: self& operator=(self const&);

    public: bool swap_stream(typename self::stream& io_stream)
    {
        if (0 < this->stack_size_)
        {
            // 復元途中はできない。
            return false;
        }
        std::swap(this->stream_, io_stream);
        return true;
    }

    public: bool swap_pool(typename self::pool& io_pool)
    {
        if (0 < this->stack_size_)
        {
            // 復元途中はできない。
            return false;
        }
        std::swap(this->pool_, io_pool);
        return true;
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、MessagePackオブジェクトを復元する。
        @param[out] out_object
            復元したMessagePackオブジェクトの格納先。
            復元完了しなかった場合は、何もしない。
        @return
            - 正なら、MessagePackオブジェクトの復元完了。
            - 0 なら、MessagePackオブジェクトの復元途中で終了。
            - 負なら、復元に失敗。
     */
    public: int read_object(typename self::root_object& out_object)
    {
        // 復元途中のオブジェクトではないか判定する。
        if (0 < this->stack_size_ && this->stack_.front().object != out_object)
        {
            PSYQ_ASSERT(false);
            return -1;
        }

        // ストリームからMessagePackを読み込む。
        psyq::message_pack::object local_object;
        for (;;)
        {
            auto const local_pre_position(this->stream_.tellg());
            switch (this->read_unit(local_object))
            {
            case self::read_result_FINISH:
                out_object = typename self::root_object(
                    this->stack_.front().object, std::move(this->pool_));
                return 1;

            case self::read_result_CONTINUE:
                if (this->stream_.eof())
                {
                    return 0;
                }
                break;

            case self::read_result_ABORT:
                if (this->stream_.fail())
                {
                    return -1;
                }
                this->stream_.seekg(local_pre_position);
                return 0;

            default:
                PSYQ_ASSERT(false);
                return -1;
            }
        }
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、MessagePackオブジェクトを復元する。
        @param[out] out_object 復元したMessagePackオブジェクトの格納先。
     */
    private: typename self::read_result read_unit(
        psyq::message_pack::object& out_object)
    {
        // ストリームからMessagePack直列化形式を読み込む。
        if (!this->stream_.good())
        {
            return self::read_result_ABORT;
        }
        auto const local_header(static_cast<unsigned>(this->stream_.get()));
 
        // MessagePackの直列化形式によって、復元処理を分岐する。
        std::size_t local_read(0);
        if (local_header <= psyq::message_pack::header_FIX_INT_MAX)
        {
            // [0x00, 0x7f]: positive fixnum
            out_object = local_header;
        }
        else if (local_header <= psyq::message_pack::header_FIX_MAP_MAX)
        {
            // [0x80, 0x8f]: fix map
            return this->make_container<psyq::message_pack::object::unordered_map>(
                out_object, local_header & 0x0f);
        }
        else if (local_header <= psyq::message_pack::header_FIX_ARRAY_MAX)
        {
            // [0x90, 0x9f]: fix array
            return this->make_container<psyq::message_pack::object::array>(
                out_object, local_header & 0x0f);
        }
        else if (local_header <= psyq::message_pack::header_FIX_STR_MAX)
        {
            // [0xa0, 0xbf]: fix str
            return this->read_raw<psyq::message_pack::object::string>(
                out_object, local_header & 0x1f);
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
            local_read = 1 << (local_header - psyq::message_pack::header_BIN8);
        }
        else if (local_header <= psyq::message_pack::header_EXT32)
        {
            // 0xc7: ext 8
            // 0xc8: ext 16
            // 0xc9: ext 32
            local_read = 1 << (local_header - psyq::message_pack::header_EXT8);
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
            local_read = 1 << (local_header & 0x3);
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
                1 << (local_header - psyq::message_pack::header_FIX_EXT1));
        }
        else if (local_header <= psyq::message_pack::header_STR32)
        {
            // 0xd9: str 8
            // 0xda: str 16
            // 0xdb: str 32
            local_read = 1 << (local_header - psyq::message_pack::header_STR8);
        }
        else if (local_header <= psyq::message_pack::header_MAP32)
        {
            // 0xdc: array 16
            // 0xdd: array 32
            // 0xde: map 16
            // 0xdf: map 32
            local_read = 2 << (local_header & 0x1);
        }
        else
        {
            // [0xe0, 0xff]: negative fixnum
            PSYQ_ASSERT(local_header <= 0xff);
            out_object = static_cast<std::int8_t>(local_header);
        }

        // 次。
        return 0 < local_read?
            this->read_value(out_object, local_header):
            this->update_container_stack(out_object);
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、MessagePack値を復元する。
        @param[out] out_object 復元した値の格納先。
        @param[in]  in_header  復元するMessagePack値のヘッダ。
     */
    private: typename self::read_result read_value(
        psyq::message_pack::object& out_object,
        unsigned const in_header)
    {
        switch (in_header)
        {
        // 無符号整数
        case psyq::message_pack::header_UINT8:
            return this->read_big_endian<std::uint8_t>(out_object);
        case psyq::message_pack::header_UINT16:
            return this->read_big_endian<std::uint16_t>(out_object);
        case psyq::message_pack::header_UINT32:
            return this->read_big_endian<std::uint32_t>(out_object);
        case psyq::message_pack::header_UINT64:
            return this->read_big_endian<std::uint64_t>(out_object);

        // 有符号整数
        case psyq::message_pack::header_INT8:
            return this->read_big_endian<std::int8_t>(out_object);
        case psyq::message_pack::header_INT16:
            return this->read_big_endian<std::int16_t>(out_object);
        case psyq::message_pack::header_INT32:
            return this->read_big_endian<std::int32_t>(out_object);
        case psyq::message_pack::header_INT64:
            return this->read_big_endian<std::int64_t>(out_object);

        // 浮動小数点数
        case psyq::message_pack::header_FLOAT32:
            return this->read_big_endian<psyq::message_pack::object::float32>(out_object);
        case psyq::message_pack::header_FLOAT64:
            return this->read_big_endian<psyq::message_pack::object::float64>(out_object);

        // 文字列
        case psyq::message_pack::header_STR8:
            return this->read_raw<std::uint8_t,  psyq::message_pack::object::string>(out_object);
        case psyq::message_pack::header_STR16:
            return this->read_raw<std::uint16_t, psyq::message_pack::object::string>(out_object);
        case psyq::message_pack::header_STR32:
            return this->read_raw<std::uint32_t, psyq::message_pack::object::string>(out_object);

        // バイナリ
        case psyq::message_pack::header_BIN8:
            return this->read_raw<std::uint8_t,  psyq::message_pack::object::binary>(out_object);
        case psyq::message_pack::header_BIN16:
            return this->read_raw<std::uint16_t, psyq::message_pack::object::binary>(out_object);
        case psyq::message_pack::header_BIN32:
            return this->read_raw<std::uint32_t, psyq::message_pack::object::binary>(out_object);

        // 拡張バイナリ
        case psyq::message_pack::header_EXT8:
            return this->read_raw<std::uint8_t,  psyq::message_pack::object::extended>(out_object);
        case psyq::message_pack::header_EXT16:
            return this->read_raw<std::uint16_t, psyq::message_pack::object::extended>(out_object);
        case psyq::message_pack::header_EXT32:
            return this->read_raw<std::uint32_t, psyq::message_pack::object::extended>(out_object);

        // 配列
        case psyq::message_pack::header_ARRAY16:
            return this->read_container<psyq::message_pack::object::array, std::uint16_t>(out_object);
        case psyq::message_pack::header_ARRAY32:
            return this->read_container<psyq::message_pack::object::array, std::uint32_t>(out_object);

        // 連想配列
        case psyq::message_pack::header_MAP16:
            return this->read_container<psyq::message_pack::object::unordered_map, std::uint16_t>(out_object);
        case psyq::message_pack::header_MAP32:
            return this->read_container<psyq::message_pack::object::unordered_map, std::uint32_t>(out_object);

        default:
            PSYQ_ASSERT(false);
            return self::read_result_FAILED;
        }
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、MessagePackコンテナを復元する。
        @param[out] out_object 復元したMessagePackコンテナの格納先。
     */
    private: template<typename template_container, typename template_length>
    typename self::read_result read_container(
        psyq::message_pack::object& out_object)
    {
        template_length local_length;
        return self::read_big_endian(local_length, this->stream_)?
            this->make_container<template_container>(out_object, local_length):
            self::read_result_ABORT;
    }

    /** @brief 空のMessagePackコンテナを生成する。
        @param[out] out_object  生成したMessagePackコンテナの格納先。
        @param[in]  in_capacity 生成するコンテナの容量。
     */
    private: template<typename template_container>
    typename self::read_result make_container(
        psyq::message_pack::object& out_object,
        std::size_t const in_capacity)
    {
        static_assert(
            std::is_same<template_container, psyq::message_pack::object::array>::value
            || std::is_same<template_container, psyq::message_pack::object::unordered_map>::value,
            "template_container is not psyq::message_pack::object::array"
            "or psyq::message_pack::object::unordered_map.");
        if (this->stack_.size() <= this->stack_size_)
        {
            PSYQ_ASSERT(false);
            return self::read_result_FAILED;
        }

        // コンテナを構築する。
        auto& local_stack_top(this->stack_[this->stack_size_]);
        bool local_make_container(
            self::make_container<template_container>(
                local_stack_top.object,
                this->pool_,
                in_capacity,
                sizeof(std::int64_t)));
        if (!local_make_container)
        {
            PSYQ_ASSERT(false);
            return self::read_result_FAILED;
        }
        else if (0 < in_capacity)
        {
            // コンテナをスタックに積む。
            local_stack_top.kind =
                std::is_same<
                    template_container, psyq::message_pack::object::array>
                        ::value?
                            self::stack_kind_ARRAY_ELEMENT:
                            self::stack_kind_MAP_KEY;
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
    /** @brief 空のMessagePackコンテナを生成する。
        @tparam template_container 生成するMessagePackコンテナの型。
        @param[out]    out_object   生成したMessagePackコンテナの格納先。
        @param[in,out] io_pool      psyq::message_pack::pool 互換のメモリ割当子。
        @param[in]     in_capacity  コンテナの容量。
        @param[in]     in_alignment メモリ境界単位。
        @retval true  成功。
        @retval false 失敗。
     */
    private: template<typename template_container>
    static bool make_container(
        psyq::message_pack::object& out_object,
        typename self::pool& io_pool,
        std::size_t const in_capacity,
        std::size_t const in_alignment)
    {
        typename template_container::pointer local_storage;
        if (0 < in_capacity)
        {
            local_storage = static_cast<typename template_container::pointer>(
                io_pool.allocate(
                    in_capacity
                        * sizeof(typename template_container::value_type),
                    in_alignment));
            if (local_storage == nullptr)
            {
                PSYQ_ASSERT(false);
                return false;
            }
        }
        else
        {
            local_storage = nullptr;
        }
        template_container local_container;
        local_container.reset(local_storage, 0);
        out_object = local_container;
        return true;
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、MessagePackRAWバイト列を復元する。
        @tparam template_length MessagePackRAWバイト列のバイト長の型。
        @tparam template_raw    MessagePackRAWバイト列の型。
        @param[out] out_object RAWバイト列を格納するMessagePackオブジェクト。
     */
    private: template<typename template_length, typename template_raw>
    typename self::read_result read_raw(psyq::message_pack::object& out_object)
    {
        template_length local_size;
        return self::read_big_endian(local_size, this->stream_)?
            this->read_raw<template_raw>(out_object, local_size):
            self::read_result_ABORT;
    }
    /** @brief ストリームを読み込み、MessagePackRAWバイト列を復元する。
        @tparam template_raw MessagePackRAWバイト列の型。
        @param[out] out_object RAWバイト列を格納するMessagePackオブジェクト。
        @param[in]  in_size    RAWバイト列のバイト数。
     */
    private: template<typename template_raw>
    typename self::read_result read_raw(
        psyq::message_pack::object& out_object,
        std::size_t in_size)
    {
        if (std::is_same<template_raw, psyq::message_pack::object::extended>::value)
        {
            ++in_size;
        }
        auto const local_bytes(
            static_cast<typename template_raw::pointer>(
                self::read_bytes(
                    this->stream_, this->pool_, in_size, this->allocate_raw_)));
        if (local_bytes == nullptr && 0 < in_size)
        {
            return self::read_result_ABORT;
        }
        template_raw local_raw;
        local_raw.reset(local_bytes, in_size);
        out_object = local_raw;
        return this->update_container_stack(out_object);
    }

    /** @brief ストリームを読み込み、RAWバイト列を復元する。
        @param[in,out] io_istream RAWバイト列を読み込むストリーム。
        @param[in,out] io_pool    psyq::message_pack::pool 互換のメモリ割当子。
        @param[in]     in_size    RAWバイト列のバイト数。
     */
    private: static void* read_bytes(
        typename self::stream& io_istream,
        typename self::pool& io_pool,
        std::size_t const in_size,
        bool const in_allocate)
    {
        typedef typename self::stream::char_type char_type;
        auto const local_allocate_size(
            ((in_size + sizeof(char_type) - 1) / sizeof(char_type))
                * sizeof(char_type));
        if (local_allocate_size <= 0)
        {
            return nullptr;
        }
        else if (in_allocate)
        {
            auto const local_bytes(
                io_pool.allocate(local_allocate_size, sizeof(char_type)));
            if (local_bytes == nullptr)
            {
                PSYQ_ASSERT(false);
                return nullptr;
            }
            auto const local_read_size(
                psyq::internal::message_pack_read_bytes(
                    local_bytes, io_istream, local_allocate_size));
            if (local_read_size != local_allocate_size)
            {
                PSYQ_ASSERT(io_istream.eof());
                //io_pool.deallocate(local_bytes, local_allocate_size);
                return nullptr;
            }
            return local_bytes;
        }
        else
        {
            /** @todo 2014.05.15 未実装。
                メモリ割当てを行わず、
                ストリームのバッファを参照するような処理を実装すること。
             */
            PSYQ_ASSERT(false);
            return nullptr;
        }
    }

    //-------------------------------------------------------------------------
    private: template<typename template_value>
    typename self::read_result read_big_endian(
        psyq::message_pack::object& out_object)
    {
        template_value local_value;
        if (!self::read_big_endian(local_value, this->stream_))
        {
            return self::read_result_ABORT;
        }
        out_object = local_value;
        return this->update_container_stack(out_object);
    }
    private: template<typename template_value>
    static bool read_big_endian(
        template_value& out_value,
        typename self::stream& io_istream)
    {
        typedef psyq::message_pack::raw_bytes<template_value> raw_bytes;
        typename raw_bytes::pack local_bytes;
        auto const local_read_size(
            psyq::internal::message_pack_read_bytes(
                &local_bytes, io_istream, sizeof(local_bytes)));
        if (local_read_size != sizeof(local_bytes))
        {
            return false;
        }
        out_value = raw_bytes::convert_bytes_endianness(
            local_bytes, psyq::message_pack::big_endian);
        return true;
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
                auto const local_map(local_stack_top.object.get_unordered_map());
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
                if (this->sort_map_)
                {
                    local_stack_top.object.sort_map();
                }
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
    private: typename self::stream stream_;
    private: typename self::pool pool_;
    private: std::array<typename self::stack, template_stack_capacity> stack_;
    private: std::size_t stack_size_; ///< スタックの要素数。
    private: bool allocate_raw_;
    private: bool sort_map_;
};

#endif // !defined(PSYQ_MESSAGE_PACK_DESIRIALIZE_HPP_)
