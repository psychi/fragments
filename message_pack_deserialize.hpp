/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copydoc psyq::message_pack::deserializer
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
    } // namespace message_pack
    namespace internal
    {
        //---------------------------------------------------------------------
        /** @brief ストリームからRAWバイト列を読み込む。
            @param[out]    out_bytes    ストリームから読み込んだRAWバイト列を格納する。
            @param[in,out] io_istream   読み込むストリーム。
            @param[in]     in_read_size 読み込むバイト数。
            @return 読み込んだバイト数。
         */
        template<typename template_stream>
        std::size_t message_pack_read_bytes(
            void* const out_bytes,
            template_stream& io_istream,
            std::size_t const in_read_size)
        {
            if (io_istream.fail())
            {
                return 0;
            }

            // ストリームを読み込む。
            auto const local_pre_position(io_istream.tellg());
            typedef typename template_stream::char_type char_type;
            io_istream.read(
                static_cast<char_type*>(out_bytes),
                in_read_size / sizeof(char_type));
            if (io_istream.fail())
            {
                PSYQ_ASSERT(false);
                io_istream.seekg(local_pre_position);
                return 0;
            }
            return static_cast<std::size_t>(io_istream.tellg() - local_pre_position)
                * sizeof(char_type);
        }
    } // namespace internal
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_istream 互換のストリームを読み込み、
           MessagePackの直列化復元をするアダプタ。
    @tparam template_pool @copydoc self::pool
    @tparam template_stack_capacity @copydoc self::stack_capacity
 */
template<
    typename template_stream,
    typename template_pool,
    std::size_t template_stack_capacity>
class psyq::message_pack::deserializer
{
    /// thisが指す値の型。
    private: typedef deserializer self;

    /** @brief 読み込みに使う、 std::basic_istream 互換のストリーム。
     */
    public: typedef template_stream stream;
    static_assert(
        sizeof(typename self::stream::char_type) == 1,
        "sizeof(self::stream::char_type) is not 1.");

    /// 直列化復元に使う、 psyq::message_pack::pool 互換のメモリ割当子。
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

    private: struct container_stack
    {
        psyq::message_pack::object object;  ///< 復元中のオブジェクト。
        psyq::message_pack::object map_key; ///< 直前に復元した連想配列キー。
        std::size_t rest_length;            ///< コンテナ要素の残数。
        typename self::stack_kind kind;     ///< 復元中のオブジェクトの種別。
    };

    //-------------------------------------------------------------------------
    /// @name 構築
    //@{
    /** @brief 初期化する。
        @param[in] in_stream MessagePackを読み込むストリーム。
        @param[in] in_pool   直列化復元に使うメモリ割当子。
     */
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

    /** @brief move構築子。
        @param[in,out] io_source 移動元。
     */
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

    /** @brief move代入演算子。
        @param[in,out] io_source 移動元。
     */
    public: self& operator=(self&& io_source)
    {
        this->stream_ = std::move(io_source.stream_);
        this->pool_ = std::move(io_source.pool_);
        this->container_stack_ = std::move(io_source.container_stack_);
        this->stack_size_ = std::move(io_source.stack_size_);
        this->allocate_raw_ = std::move(io_source.allocate_raw_);
        this->sort_map_ = std::move(io_source.sort_map_);
        io_source.stack_size_ = 0;
        return *this;
    }
    //@}
    private: deserializer(self const&);
    private: self& operator=(self const&);

    //-------------------------------------------------------------------------
    /// @name 属性の操作
    //@{
    /** @brief 直列化復元で読み込む入力ストリームを取得する。
        @return 直列化復元で読み込む入力ストリーム。
     */
    public: typename self::stream const& get_stream() const
    {
        return this->stream_;
    }

    public: typename self::stream::pos_type tellg()
    {
        return this->stream_.tellg();
    }

    /** @brief 直列化復元で使うメモリ割当子を取得する。
        @return 直列化復元で使うメモリ割当子
     */
    public: typename self::pool const& get_pool() const
    {
        return this->pool_;
    }

    /** @brief 直列化復元で使うメモリ割当子を設定する。

        ただし直列化復元の途中では、メモリ割当子を設定できない。

        @param[in] in_pool 直列化復元で使うメモリ割当子
     */
    public: bool set_pool(typename self::pool in_pool)
    {
        if (0 < this->stack_size_)
        {
            // 復元途中はできない。
            return false;
        }
        this->pool_ = std::move(in_pool);
        return true;
    }

    /** @brief 連想配列の直列化復元で、ソートするかを取得する。
        @return ソートをするかどうか。
     */
    public: bool get_sort_map() const
    {
        return this->sort_map_;
    }

    /** @brief 連想配列の直列化復元で、ソートするかを設定する。
        @param[in] in_sort_map ソートをするかどうか。
     */
    public: void set_sort_map(bool const in_sort_map)
    {
        this->sort_map_ = in_sort_map;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 直列化復元
    //@{
    /** @brief ストリームを読み込み、MessagePackオブジェクトを復元する。
        @param[out] out_object 復元したMessagePackオブジェクトを格納する。
        @return *this
     */
    public: self& operator>>(typename self::root_object& out_object)
    {
        auto const local_result(this->read_object(out_object));
        if (local_result <= 0)
        {
            PSYQ_ASSERT(false);
            out_object.reset();
        }
        return *this;
    }

    /** @brief ストリームを読み込み、MessagePackオブジェクトを復元する。
        @param[out] out_object
            - 復元が完了したMessagePackオブジェクトを格納する。
            - 復元が完了しなかった場合は、何もしない。
        @return
            - 正なら、MessagePackオブジェクトの復元を完了。
            - 0 なら、MessagePackオブジェクトの復元途中で中断。
              self::read_object_continue() で、復元を続行できる。
            - 負なら、復元に失敗。
     */
    public: int read_object(typename self::root_object& out_object)
    {
        if (0 < this->stack_size_)
        {
            PSYQ_ASSERT(false);
            return -1;
        }
        return this->read_object_continue(out_object);
    }
    /** @copydoc self::read_object()
        @param[in] in_pool 直列化復元に使うメモリ割当子。
     */
    public: int read_object(
        typename self::root_object& out_object,
        typename self::pool in_pool)
    {
        if (!this->set_pool(std::move(in_pool)))
        {
            PSYQ_ASSERT(false);
            return -1;
        }
        return this->read_object_continue(out_object);
    }

    /** @brief ストリームを読み込み、MessagePackオブジェクトの復元を続行する。
        @param[out] out_object
            - 復元が完了したMessagePackオブジェクトを格納する。
            - 復元が完了しなかった場合は、何もしない。
        @return
            - 正なら、MessagePackオブジェクトの復元を完了。
            - 0 なら、MessagePackオブジェクトの復元途中で中断。
              self::read_object_continue() で、復元を続行できる。
            - 負なら、復元に失敗。
        @sa self::read_object()
     */
    public: int read_object_continue(typename self::root_object& out_object)
    {
        for (;;)
        {
            auto const local_pre_position(this->stream_.tellg());
            switch (this->read_value())
            {
            case self::read_result_FINISH:
                out_object = typename self::root_object(
                    this->container_stack_.front().object,
                    std::move(this->pool_));
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
    //@}
    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、MessagePack値を復元する。
     */
    private: typename self::read_result read_value()
    {
        // ストリームからMessagePack直列化形式を読み込む。
        if (!this->stream_.good())
        {
            return self::read_result_ABORT;
        }
        auto const local_header(static_cast<unsigned>(this->stream_.get()));

        // MessagePackの直列化形式によって、復元処理を分岐する。
        unsigned local_read(0);
        if (local_header <= psyq::message_pack::header_FIX_INT_MAX)
        {
            // [0x00, 0x7f]: positive fixnum
            return this->add_container_value(
                psyq::message_pack::object(local_header));
        }
        else if (local_header <= psyq::message_pack::header_FIX_MAP_MAX)
        {
            // [0x80, 0x8f]: fix map
            return this->reserve_container<psyq::message_pack::object::unordered_map>(
                local_header & 0x0f);
        }
        else if (local_header <= psyq::message_pack::header_FIX_ARRAY_MAX)
        {
            // [0x90, 0x9f]: fix array
            return this->reserve_container<psyq::message_pack::object::array>(
                local_header & 0x0f);
        }
        else if (local_header <= psyq::message_pack::header_FIX_STR_MAX)
        {
            // [0xa0, 0xbf]: fix str
            return this->read_raw<psyq::message_pack::object::string>(
                local_header & 0x1f);
        }
        else if (local_header == psyq::message_pack::header_NIL)
        {
            // 0xc0: nil
            return this->add_container_value(psyq::message_pack::object());
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
            return this->add_container_value(psyq::message_pack::object(false));
        }
        else if (local_header == psyq::message_pack::header_TRUE)
        {
            // 0xc3: true
            return this->add_container_value(psyq::message_pack::object(true));
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
            return this->add_container_value(
                psyq::message_pack::object(
                    static_cast<std::int8_t>(local_header)));
        }

        // ヘッダをもとに、MessagePack値を復元する。
        return this->read_value(local_header);
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、MessagePack値を復元する。
        @param[in] in_header 復元するMessagePack値のヘッダ。
     */
    private: typename self::read_result read_value(unsigned const in_header)
    {
        switch (in_header)
        {
        // 無符号整数
        case psyq::message_pack::header_UINT8:
            return this->read_big_endian<std::uint8_t >();
        case psyq::message_pack::header_UINT16:
            return this->read_big_endian<std::uint16_t>();
        case psyq::message_pack::header_UINT32:
            return this->read_big_endian<std::uint32_t>();
        case psyq::message_pack::header_UINT64:
            return this->read_big_endian<std::uint64_t>();

        // 有符号整数
        case psyq::message_pack::header_INT8:
            return this->read_big_endian<std::int8_t >();
        case psyq::message_pack::header_INT16:
            return this->read_big_endian<std::int16_t>();
        case psyq::message_pack::header_INT32:
            return this->read_big_endian<std::int32_t>();
        case psyq::message_pack::header_INT64:
            return this->read_big_endian<std::int64_t>();

        // 浮動小数点数
        case psyq::message_pack::header_FLOAT32:
            return this->read_big_endian<psyq::message_pack::object::float32>();
        case psyq::message_pack::header_FLOAT64:
            return this->read_big_endian<psyq::message_pack::object::float64>();

        // 文字列
        case psyq::message_pack::header_STR8:
            return this->read_raw<psyq::message_pack::object::string, std::uint8_t >();
        case psyq::message_pack::header_STR16:
            return this->read_raw<psyq::message_pack::object::string, std::uint16_t>();
        case psyq::message_pack::header_STR32:
            return this->read_raw<psyq::message_pack::object::string, std::uint32_t>();

        // バイナリ
        case psyq::message_pack::header_BIN8:
            return this->read_raw<psyq::message_pack::object::binary, std::uint8_t >();
        case psyq::message_pack::header_BIN16:
            return this->read_raw<psyq::message_pack::object::binary, std::uint16_t>();
        case psyq::message_pack::header_BIN32:
            return this->read_raw<psyq::message_pack::object::binary, std::uint32_t>();

        // 拡張バイナリ
        case psyq::message_pack::header_EXT8:
            return this->read_raw<psyq::message_pack::object::extended, std::uint8_t >();
        case psyq::message_pack::header_EXT16:
            return this->read_raw<psyq::message_pack::object::extended, std::uint16_t>();
        case psyq::message_pack::header_EXT32:
            return this->read_raw<psyq::message_pack::object::extended, std::uint32_t>();

        // 配列
        case psyq::message_pack::header_ARRAY16:
            return this->reserve_container<psyq::message_pack::object::array, std::uint16_t>();
        case psyq::message_pack::header_ARRAY32:
            return this->reserve_container<psyq::message_pack::object::array, std::uint32_t>();

        // 連想配列
        case psyq::message_pack::header_MAP16:
            return this->reserve_container<psyq::message_pack::object::unordered_map, std::uint16_t>();
        case psyq::message_pack::header_MAP32:
            return this->reserve_container<psyq::message_pack::object::unordered_map, std::uint32_t>();

        default:
            PSYQ_ASSERT(false);
            return self::read_result_FAILED;
        }
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、big-endianで値を復元する。
        @tparam template_value 復元する値の型。
     */
    private: template<typename template_value>
    typename self::read_result read_big_endian()
    {
        template_value local_value;
        return self::read_big_endian(local_value, this->stream_)?
            this->add_container_value(psyq::message_pack::object(local_value)):
            self::read_result_ABORT;
    }
    /** @brief ストリームを読み込み、big-endianで値を復元する。
        @tparam template_value 復元する値の型。
        @param[out] out_value     復元した値を格納する。
        @param[in,out] io_istream 読み込むストリーム。
     */
    private: template<typename template_value>
    static bool read_big_endian(
        template_value& out_value,
        typename self::stream& io_istream)
    {
        typedef psyq::message_pack::bytes_serializer<template_value> bytes_serializer;
        typename bytes_serializer::pack local_bytes;
        auto const local_read_size(
            psyq::internal::message_pack_read_bytes(
                &local_bytes, io_istream, sizeof(local_bytes)));
        if (local_read_size != sizeof(local_bytes))
        {
            return false;
        }
        out_value = bytes_serializer::convert_bytes_endianness(
            local_bytes, psyq::message_pack::big_endian);
        return true;
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、MessagePackRAWバイト列を復元する。
        @tparam template_length MessagePackRAWバイト列のバイト長の型。
        @tparam template_raw    MessagePackRAWバイト列の型。
     */
    private: template<typename template_raw, typename template_length>
    typename self::read_result read_raw()
    {
        template_length local_size;
        return self::read_big_endian(local_size, this->stream_)?
            this->read_raw<template_raw>(local_size):
            self::read_result_ABORT;
    }
    /** @brief ストリームを読み込み、MessagePackRAWバイト列を復元する。
        @tparam template_raw MessagePackRAWバイト列の型。
        @param[in] in_size RAWバイト列のバイト数。
     */
    private: template<typename template_raw>
    typename self::read_result read_raw(std::size_t in_size)
    {
        if (std::is_same<template_raw, psyq::message_pack::object::extended>::value)
        {
            ++in_size;
        }
        auto const local_bytes(
            static_cast<typename template_raw::pointer>(
                self::read_raw(
                    this->stream_, this->pool_, in_size, this->allocate_raw_)));
        if (local_bytes == nullptr && 0 < in_size)
        {
            return self::read_result_ABORT;
        }
        template_raw local_raw;
        local_raw.reset(local_bytes, in_size);
        return this->add_container_value(psyq::message_pack::object(local_raw));
    }
    /** @brief ストリームを読み込み、RAWバイト列を復元する。
        @param[in,out] io_istream  RAWバイト列を読み込むストリーム。
        @param[in,out] io_pool     psyq::message_pack::pool 互換のメモリ割当子。
        @param[in]     in_size     RAWバイト列のバイト数。
        @param[in]     in_allocate メモリ割当てをするかどうか。
     */
    private: static void* read_raw(
        typename self::stream& io_istream,
        typename self::pool& io_pool,
        std::size_t const in_size,
        bool const in_allocate)
    {
        typedef typename template_stream::char_type char_type;
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
    /** @brief MessagePackコンテナを予約し、スタックに積む。
        @tparam template_container コンテナの型。
        @tparam template_length    コンテナ容量の型。
     */
    private: template<
        typename template_container,
        typename template_length>
    typename self::read_result reserve_container()
    {
        // コンテナ容量をストリームから読み込み、コンテナを予約する。
        template_length local_length;
        return self::read_big_endian(local_length, this->stream_)?
            this->reserve_container<template_container>(local_length):
            self::read_result_ABORT;
    }
    /** @brief MessagePackコンテナを予約し、スタックに積む。
        @param[in] in_capacity 予約するコンテナの容量。
     */
    private: template<typename template_container>
    typename self::read_result reserve_container(std::size_t const in_capacity)
    {
        static_assert(
            std::is_same<template_container, psyq::message_pack::object::array>::value
            || std::is_same<template_container, psyq::message_pack::object::unordered_map>::value,
            "template_container is not psyq::message_pack::object::array"
            "or psyq::message_pack::object::unordered_map.");
        if (this->container_stack_.size() <= this->stack_size_)
        {
            PSYQ_ASSERT(false);
            return self::read_result_FAILED;
        }

        // コンテナを構築する。
        auto& local_stack_top(this->container_stack_[this->stack_size_]);
        bool local_reserve_container(
            self::reserve_container<template_container>(
                local_stack_top.object,
                this->pool_,
                in_capacity,
                sizeof(std::int64_t)));
        if (!local_reserve_container)
        {
            PSYQ_ASSERT(false);
            return self::read_result_FAILED;
        }
        else if (in_capacity <= 0)
        {
            return this->add_container_value(local_stack_top.object);
        }

        // コンテナをスタックに積む。
        local_stack_top.kind =
            std::is_same<template_container, psyq::message_pack::object::array>
                ::value?
                    self::stack_kind_ARRAY_ELEMENT:
                    self::stack_kind_MAP_KEY;
        local_stack_top.rest_length = in_capacity;
        ++this->stack_size_;
        return self::read_result_CONTINUE;
    }
    /** @brief MessagePackコンテナを予約する。
        @tparam template_container 予約するMessagePackコンテナの型。
        @param[out]    out_object   予約したMessagePackコンテナの格納先。
        @param[in,out] io_pool      psyq::message_pack::pool 互換のメモリ割当子。
        @param[in]     in_capacity  予約するコンテナの容量。
        @param[in]     in_alignment メモリ境界単位。
        @retval true  成功。
        @retval false 失敗。
     */
    private: template<typename template_container>
    static bool reserve_container(
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
                    in_capacity * sizeof(typename template_container::value_type),
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
    /** @brief コンテナスタックを更新する。
        @param[in] in_object コンテナに追加するオブジェクト。
     */
    private: typename self::read_result add_container_value(
        psyq::message_pack::object in_object)
    PSYQ_NOEXCEPT
    {
        if (this->stack_size_ <= 0)
        {
            this->container_stack_.front().object = in_object;
            return self::read_result_FINISH;
        }
        for (;;)
        {
            // スタックに積まれているコンテナに、オブジェクトを追加する。
            auto& local_stack_top(this->container_stack_[this->stack_size_ - 1]);
            switch (local_stack_top.kind)
            {
            case self::stack_kind_ARRAY_ELEMENT:
            {
                // 配列に要素を追加する。
                auto const local_array(local_stack_top.object.get_array());
                if (local_array != nullptr)
                {
                    local_array->push_back(in_object);
                }
                else
                {
                    PSYQ_ASSERT(false);
                }

                // 残り要素数を更新する。
                --local_stack_top.rest_length;
                if (0 < local_stack_top.rest_length)
                {
                    return self::read_result_CONTINUE;
                }
                break;
            }

            case self::stack_kind_MAP_KEY:
                // 連想配列のキーを保存する。
                local_stack_top.map_key = in_object;
                local_stack_top.kind = self::stack_kind_MAP_VALUE;
                return self::read_result_CONTINUE;

            case self::stack_kind_MAP_VALUE:
            {
                // 連想配列に要素を追加する。
                auto const local_map(local_stack_top.object.get_unordered_map());
                if (local_map != nullptr)
                {
                    local_map->push_back(
                        std::make_pair(local_stack_top.map_key, in_object));
                }
                else
                {
                    PSYQ_ASSERT(false);
                    return self::read_result_FAILED;
                }

                // 残り要素数を更新する。
                --local_stack_top.rest_length;
                if (0 < local_stack_top.rest_length)
                {
                    local_stack_top.kind = self::stack_kind_MAP_KEY;
                    return self::read_result_CONTINUE;
                }

                // 連想配列の全要素が揃ったので、ソートする。
                if (this->get_sort_map())
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
            in_object = local_stack_top.object;
            if (1 < this->stack_size_)
            {
                --this->stack_size_;
            }
            else
            {
                this->stack_size_ = 0;
                return self::read_result_FINISH;
            }
        }
    }

    //-------------------------------------------------------------------------
    private: typename self::stream stream_;
    private: typename self::pool pool_;
    private: std::array<typename self::container_stack, template_stack_capacity> container_stack_;
    private: std::size_t stack_size_; ///< スタックの階層数。
    private: bool allocate_raw_;
    private: bool sort_map_;
};

#endif // !defined(PSYQ_MESSAGE_PACK_DESIRIALIZE_HPP_)
