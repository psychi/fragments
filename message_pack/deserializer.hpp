﻿/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::message_pack::deserializer
 */
#ifndef PSYQ_MESSAGE_PACK_DESIRIALIZER_HPP_
#define PSYQ_MESSAGE_PACK_DESIRIALIZER_HPP_

//#include "psyq/message_pack/serializer.hpp"
//#include "psyq/message_pack/root_object.hpp"

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

        /// この名前空間をユーザーが直接アクセスするのは禁止。
        namespace _private
        {
            //---------------------------------------------------------------------
            /** @brief ストリームからRAWバイト列を読み込む。
                @param[out]    out_bytes    ストリームから読み込んだRAWバイト列を格納する。
                @param[in,out] io_istream   読み込むストリーム。
                @param[in]     in_read_size 読み込むバイト数。
                @return 読み込んだバイト数。
             */
            template<typename template_stream>
            std::size_t read_bytes(
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
        } // namespace _private
    } // namespace message_pack
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_istream 互換の入力ストリームを読み込み、
           MessagePackの直列化復元をするアダプタ。

    使用例
    @code
    // ファイルを読み込み、MessagePackオブジェクトを直列化復元する関数。
    // @param[in] in_file_path 読み込むファイルのパス名。
    // @return ファイルから直列化復元したMessagePackオブジェクト。
    psyq::message_pack::deserializer<std::ifstream>::root_object
    read_message_pack(std::string const& in_file_path)
    {
        // ファイル入力ストリームを構築する。
        psyq::message_pack::deserializer<std::ifstream> local_deserializer(
            std::ifstream(in_file_path, std::ios::in | std::ios::binary),
            psyq::message_pack::deserializer<std::ifstream>::pool());
        // 直列化復元した最上位オブジェクトを格納するインスタンスを用意する。
        psyq::message_pack::deserializer<std::ifstream>::root_object
            local_root_object;
        // 直列化復元する。
        local_deserializer >> local_root_object;
        return local_root_object;
    }
    @endcode

    @tparam template_stream         @copydoc psyq::message_pack::deserializer::stream
    @tparam template_pool           @copydoc psyq::message_pack::deserializer::pool
    @tparam template_stack_capacity @copydoc psyq::message_pack::deserializer::stack_capacity
 */
template<
    typename template_stream,
    typename template_pool,
    std::size_t template_stack_capacity>
class psyq::message_pack::deserializer
{
    /// thisが指す値の型。
    private: typedef deserializer this_type;

    //-------------------------------------------------------------------------
    /// 直列化復元に使う、 std::basic_istream 互換の入力ストリーム。
    public: typedef template_stream stream;
    static_assert(
        sizeof(typename this_type::stream::char_type) == 1,
        "sizeof(this_type::stream::char_type) is not 1.");

    /// 直列化復元に使う、 psyq::message_pack::pool 互換のメモリ割当子。
    public: typedef template_pool pool;

    /// 最上位のMessagePackオブジェクト。
    public: typedef psyq::message_pack::object::root<typename this_type::pool>
        root_object;

    /// 直列化復元途中のコンテナのスタック限界数。
    public: static std::size_t const stack_capacity = template_stack_capacity;

    /// 次に直列化復元する値の種類。
    public: enum value_kind
    {
        value_kind_ROOT,          ///< 最上位のMessagePackオブジェクト。
        value_kind_RAW_BYTES,     ///< RAWバイト列。
        value_kind_ARRAY_ELEMENT, ///< 配列の要素。
        value_kind_MAP_KEY,       ///< 連想配列の要素のキー。
        value_kind_MAP_VALUE,     ///< 連想配列の要素のマップ値。
    };

    /// 直列化復元途中のコンテナのスタック。
    private: struct container_stack
    {
        psyq::message_pack::object object;   ///< 復元中のオブジェクト。
        psyq::message_pack::object map_key;  ///< 直前に復元した連想配列のキー。
        std::size_t rest_count;              ///< コンテナ要素の残数。
        typename this_type::value_kind kind; ///< 次に直列化復元する値の種類。
    };

    private: enum read_result
    {
        read_result_ABORT    = -2,
        read_result_FAILED   = -1,
        read_result_CONTINUE =  0,
        read_result_FINISH   =  1,
    };

    //-------------------------------------------------------------------------
    /// @name 構築
    //@{
    /** @brief 入力ストリームを構築する。
        @param[in] in_istream MessagePackを読み込むストリームの初期値。
        @param[in] in_pool    直列化復元に使うメモリ割当子の初期値。
     */
    public: deserializer(
        typename this_type::stream in_istream,
        typename this_type::pool in_pool)
    :
        stream_(std::move(in_istream)),
        pool_(std::move(in_pool)),
        stack_size_(0),
        allocate_raw_(true),
        sort_map_(true)
    {}

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元インスタンス。
     */
    public: deserializer(this_type&& io_source):
        stream_(std::move(io_source.stream_)),
        pool_(std::move(io_source.pool_)),
        stack_(std::move(io_source.stack_)),
        stack_size_(std::move(io_source.stack_size_)),
        allocate_raw_(std::move(io_source.allocate_raw_)),
        sort_map_(std::move(io_source.sort_map_))
    {
        io_source.stack_size_ = 0;
    }

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元インスタンス。
     */
    public: this_type& operator=(this_type&& io_source)
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
    /// コピー構築子は使用禁止。
    private: deserializer(this_type const&);// = delete;
    /// コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);// = delete;

    //-------------------------------------------------------------------------
    /// @name インスタンス変数の操作
    //@{
    /** @brief 入力ストリームを再構築する。
        @param[in] in_istream 新たに設定する入力ストリーム。
        @param[in] in_pool    新たに設定するメモリ割当子。
        @return これまで使っていた入力ストリーム。
     */
    public: typename this_type::stream reset(
        typename this_type::stream in_istream,
        typename this_type::pool in_pool)
    {
        this->stream_.swap(in_istream);
        this->pool_ = std::move(in_pool);
        this->stack_size_ = 0;
        return in_istream;
    }

    /** @brief 直列化復元で読み込む入力ストリームを取得する。
        @return 直列化復元で読み込む入力ストリーム。
     */
    public: typename this_type::stream const& get_stream() const PSYQ_NOEXCEPT
    {
        return this->stream_;
    }

    /** @brief 直列化復元で使うメモリ割当子を取得する。
        @return 直列化復元で使うメモリ割当子
     */
    public: typename this_type::pool const& get_pool() const PSYQ_NOEXCEPT
    {
        return this->pool_;
    }

    /** @brief 連想配列の直列化復元で、ソートするかを取得する。
        @return ソートをするかどうか。
     */
    public: bool get_sort_map() const PSYQ_NOEXCEPT
    {
        return this->sort_map_;
    }

    /** @brief 連想配列の直列化復元で、ソートするかを設定する。
        @param[in] in_sort_map ソートをするかどうか。
     */
    public: void set_sort_map(bool const in_sort_map) PSYQ_NOEXCEPT
    {
        this->sort_map_ = in_sort_map;
    }

    /** @brief 次に直列化復元するMessagePack値の種別を取得する。
        @return 次に直列化復元するMessagePack値の種別。
        @sa this_type::read_object()
     */
    public: typename this_type::value_kind get_next_value_kind() const PSYQ_NOEXCEPT
    {
        return 0 < this->get_rest_container_count()?
            this->container_stack_.at(this->get_rest_container_count() - 1).kind:
            typename this_type::value_kind_ROOT;
    }

    /** @brief 直前に直列化復元を開始したMessagePackコンテナの残り要素数を取得する。
        @return
            直前に直列化復元を開始したMessagePackコンテナが…
            - 文字列／バイナリ／拡張バイナリなら、残りバイト数。
            - 配列／連想配列なら、残り要素数。
        @sa this_type::read_object()
     */
    public: std::size_t get_rest_element_count() const PSYQ_NOEXCEPT
    {
        return 0 < this->get_rest_container_count()?
            this->container_stack_.at(this->get_rest_container_count() - 1).rest_count:
            0;
    }

    /** @brief 直列化復元途中のMessagePackコンテナの数を取得する。
        @return 直列化復元途中のMessagePackコンテナの数。
        @sa this_type::read_object()
     */
    public: std::size_t get_rest_container_count() const PSYQ_NOEXCEPT
    {
        return this->stack_size_;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 直列化復元
    //@{
    /** @brief ストリームを読み込み、MessagePackオブジェクトを直列化復元する。
        @param[out] out_object
            - 直列化復元が完了したMessagePackオブジェクトを格納する。
            - 直列化復元が完了しなかった場合は、空値を格納する。
        @return *this
     */
    public: this_type& operator>>(typename this_type::root_object& out_object)
    {
        auto const local_result(this->read_object(out_object));
        if (local_result <= 0)
        {
            PSYQ_ASSERT(false);
            out_object.reset();
        }
        return *this;
    }

    /** @brief ストリームを読み込み、MessagePackオブジェクトを直列化復元する。
        @param[out] out_object
            - 直列化復元が完了したMessagePackオブジェクトを格納する。
            - 直列化復元が完了しなかった場合は、何もしない。
        @param[in] in_pool 直列化復元に使うメモリ割当子。
        @return
            - 正なら、MessagePackオブジェクトの直列化復元を完了。
            - 0 なら、MessagePackオブジェクトの直列化復元途中で中断。
              this_type::read_object(this_type::root_object&) で、直列化復元を続行できる。
            - 負なら、直列化復元に失敗。
        @sa this_type::get_rest_container_count()
     */
    public: int read_object(
        typename this_type::root_object& out_object,
        typename this_type::pool in_pool)
    {
        if (0 < this->get_rest_container_count())
        {
            // 復元途中だった。
            PSYQ_ASSERT(false);
            return -1;
        }
        this->pool_ = std::move(in_pool);
        return this->read_object(out_object);
    }
    /** @brief ストリームを読み込み、MessagePackオブジェクトの直列化復元を続行する。
        @param[out] out_object
            - 直列化復元が完了したMessagePackオブジェクトを格納する。
            - 直列化復元が完了しなかった場合は、何もしない。
        @return
            - 正なら、MessagePackオブジェクトの直列化復元を完了。
            - 0 なら、MessagePackオブジェクトの直列化復元途中で中断。
              this_type::read_object(this_type::root_object&) で、直列化復元を続行できる。
            - 負なら、直列化復元に失敗。
        @sa this_type::get_rest_container_count()
     */
    public: int read_object(typename this_type::root_object& out_object)
    {
        for (;;)
        {
            auto const local_pre_position(this->stream_.tellg());
            switch (this->read_value())
            {
            case this_type::read_result_FINISH:
                out_object = typename this_type::root_object(
                    this->container_stack_.front().object,
                    std::move(this->pool_));
                return 1;

            case this_type::read_result_CONTINUE:
                if (this->stream_.eof())
                {
                    return 0;
                }
                break;

            case this_type::read_result_ABORT:
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
    private: typename this_type::read_result read_value()
    {
        // ストリームからMessagePack直列化形式を読み込む。
        if (!this->stream_.good())
        {
            return this_type::read_result_ABORT;
        }
        auto const local_header(static_cast<unsigned>(this->stream_.get()));

        // MessagePackの直列化形式によって、復元処理を分岐する。
        if (local_header <= psyq::message_pack::_private::format_FIX_INTEGER_MAX)
        {
            // [0x00, 0x7f]: positive fixnum
            return this->add_container_element(psyq::message_pack::object(local_header));
        }
        else if (local_header <= psyq::message_pack::_private::format_FIX_MAP_MAX)
        {
            // [0x80, 0x8f]: fix map
            return this->reserve_container<psyq::message_pack::object::unordered_map>(local_header & 0x0f);
        }
        else if (local_header <= psyq::message_pack::_private::format_FIX_ARRAY_MAX)
        {
            // [0x90, 0x9f]: fix array
            return this->reserve_container<psyq::message_pack::object::array>(local_header & 0x0f);
        }
        else if (local_header <= psyq::message_pack::_private::format_FIX_STRING_MAX)
        {
            // [0xa0, 0xbf]: fix str
            return this->read_raw<psyq::message_pack::object::string>(local_header & 0x1f);
        }
        else if (local_header <= psyq::message_pack::_private::foramt_NEGATIVE_INTEGER_64)
        {
            // 0xc0: nil
            // 0xc1: never used
            // 0xc2: false
            // 0xc3: true
            // 0xc4: bin 8
            // 0xc5: bin 16
            // 0xc6: bin 32
            // 0xc7: ext 8
            // 0xc8: ext 16
            // 0xc9: ext 32
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
            return this->read_value(local_header);
        }
        else if (local_header <= psyq::message_pack::_private::format_FIX_EXTENDED_16)
        {
            // 0xd4: fix ext 1
            // 0xd5: fix ext 2
            // 0xd6: fix ext 4
            // 0xd7: fix ext 8
            // 0xd8: fix ext 16
            return this->read_raw<psyq::message_pack::object::extended>(
                1 << (local_header - psyq::message_pack::_private::format_FIX_EXTENDED_1));
        }
        else if (local_header <= psyq::message_pack::_private::format_MAP_32)
        {
            // 0xd9: str 8
            // 0xda: str 16
            // 0xdb: str 32
            // 0xdc: array 16
            // 0xdd: array 32
            // 0xde: map 16
            // 0xdf: map 32
            return this->read_value(local_header);
        }
        else
        {
            // [0xe0, 0xff]: negative fixnum
            PSYQ_ASSERT(local_header <= 0xff);
            return this->add_container_element(
                psyq::message_pack::object(
                    static_cast<std::int8_t>(local_header)));
        }
    }
    /** @brief ストリームを読み込み、MessagePack値を復元する。
        @param[in] in_header 復元するMessagePack値のヘッダ。
     */
    private: typename this_type::read_result read_value(unsigned const in_header)
    {
        switch (in_header)
        {
        // 空値
        case psyq::message_pack::_private::format_NIL:
            return this->add_container_element(psyq::message_pack::object());

        // 真偽値
        case psyq::message_pack::_private::format_FALSE:
            return this->add_container_element(psyq::message_pack::object(false));
        case psyq::message_pack::_private::format_TRUE:
            return this->add_container_element(psyq::message_pack::object(true));

        // 0以上の整数
        case psyq::message_pack::_private::format_UNSIGNED_INTEGER_8:
            return this->read_big_endian<std::uint8_t >();
        case psyq::message_pack::_private::format_UNSIGNED_INTEGER_16:
            return this->read_big_endian<std::uint16_t>();
        case psyq::message_pack::_private::format_UNSIGNED_INTEGER_32:
            return this->read_big_endian<std::uint32_t>();
        case psyq::message_pack::_private::format_UNSIGNED_INTEGER_64:
            return this->read_big_endian<std::uint64_t>();

        // 0未満の整数
        case psyq::message_pack::_private::foramt_NEGATIVE_INTEGER_8:
            return this->read_big_endian<std::int8_t >();
        case psyq::message_pack::_private::foramt_NEGATIVE_INTEGER_16:
            return this->read_big_endian<std::int16_t>();
        case psyq::message_pack::_private::foramt_NEGATIVE_INTEGER_32:
            return this->read_big_endian<std::int32_t>();
        case psyq::message_pack::_private::foramt_NEGATIVE_INTEGER_64:
            return this->read_big_endian<std::int64_t>();

        // 浮動小数点数
        case psyq::message_pack::_private::format_FLOATING_POINT_32:
            return this->read_big_endian<psyq::message_pack::object::floating_point_32>();
        case psyq::message_pack::_private::format_FLOATING_POINT_64:
            return this->read_big_endian<psyq::message_pack::object::floating_point_64>();

        // 文字列
        case psyq::message_pack::_private::format_STRING_8:
            return this->read_raw<psyq::message_pack::object::string, std::uint8_t >();
        case psyq::message_pack::_private::format_STRING_16:
            return this->read_raw<psyq::message_pack::object::string, std::uint16_t>();
        case psyq::message_pack::_private::format_STRING_32:
            return this->read_raw<psyq::message_pack::object::string, std::uint32_t>();

        // バイナリ
        case psyq::message_pack::_private::format_BINARY_8:
            return this->read_raw<psyq::message_pack::object::binary, std::uint8_t >();
        case psyq::message_pack::_private::format_BINARY_16:
            return this->read_raw<psyq::message_pack::object::binary, std::uint16_t>();
        case psyq::message_pack::_private::format_BINARY_32:
            return this->read_raw<psyq::message_pack::object::binary, std::uint32_t>();

        // 拡張バイナリ
        case psyq::message_pack::_private::format_EXTENDED_8:
            return this->read_raw<psyq::message_pack::object::extended, std::uint8_t >();
        case psyq::message_pack::_private::format_EXTENDED_16:
            return this->read_raw<psyq::message_pack::object::extended, std::uint16_t>();
        case psyq::message_pack::_private::format_EXTENDED_32:
            return this->read_raw<psyq::message_pack::object::extended, std::uint32_t>();

        // 配列
        case psyq::message_pack::_private::format_ARRAY_16:
            return this->reserve_container<psyq::message_pack::object::array, std::uint16_t>();
        case psyq::message_pack::_private::format_ARRAY_32:
            return this->reserve_container<psyq::message_pack::object::array, std::uint32_t>();

        // 連想配列
        case psyq::message_pack::_private::format_MAP_16:
            return this->reserve_container<psyq::message_pack::object::unordered_map, std::uint16_t>();
        case psyq::message_pack::_private::format_MAP_32:
            return this->reserve_container<psyq::message_pack::object::unordered_map, std::uint32_t>();

        default:
            PSYQ_ASSERT(false);
            return this_type::read_result_FAILED;
        }
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、big-endianで値を復元する。
        @tparam template_value 復元する値の型。
     */
    private: template<typename template_value>
    typename this_type::read_result read_big_endian()
    {
        template_value local_value;
        return this_type::read_big_endian(local_value, this->stream_)?
            this->add_container_element(psyq::message_pack::object(local_value)):
            this_type::read_result_ABORT;
    }
    /** @brief ストリームを読み込み、big-endianで値を復元する。
        @tparam template_value 復元する値の型。
        @param[out] out_value     復元した値を格納する。
        @param[in,out] io_istream 読み込むストリーム。
     */
    private: template<typename template_value>
    static bool read_big_endian(
        template_value& out_value,
        typename this_type::stream& io_istream)
    {
        typedef psyq::message_pack::endianness_converter<template_value>
            endianness_converter;
        typename endianness_converter::bytes local_bytes;
        auto const local_read_size(
            psyq::message_pack::_private::read_bytes(
                &local_bytes, io_istream, sizeof(local_bytes)));
        if (local_read_size != sizeof(local_bytes))
        {
            return false;
        }
        out_value = endianness_converter::unpack_bytes(
            local_bytes, psyq::message_pack::big_endian);
        return true;
    }

    //-------------------------------------------------------------------------
    /** @brief ストリームを読み込み、MessagePackRAWバイト列を復元する。
        @tparam template_length MessagePackRAWバイト列のバイト長の型。
        @tparam template_raw    MessagePackRAWバイト列の型。
     */
    private: template<typename template_raw, typename template_length>
    typename this_type::read_result read_raw()
    {
        template_length local_size;
        return this_type::read_big_endian(local_size, this->stream_)?
            this->read_raw<template_raw>(local_size):
            this_type::read_result_ABORT;
    }
    /** @brief ストリームを読み込み、MessagePackRAWバイト列を復元する。
        @tparam template_raw MessagePackRAWバイト列の型。
        @param[in] in_size RAWバイト列のバイト数。
     */
    private: template<typename template_raw>
    typename this_type::read_result read_raw(std::size_t in_size)
    {
        if (std::is_same<template_raw, psyq::message_pack::object::extended>::value)
        {
            ++in_size;
        }
        auto const local_bytes(
            static_cast<typename template_raw::pointer>(
                this_type::read_raw(
                    this->stream_, this->pool_, in_size, this->allocate_raw_)));
        if (local_bytes == nullptr && 0 < in_size)
        {
            return this_type::read_result_ABORT;
        }
        template_raw local_raw;
        local_raw.reset(local_bytes, in_size);
        return this->add_container_element(psyq::message_pack::object(local_raw));
    }
    /** @brief ストリームを読み込み、RAWバイト列を復元する。
        @param[in,out] io_istream  RAWバイト列を読み込むストリーム。
        @param[in,out] io_pool     psyq::message_pack::pool 互換のメモリ割当子。
        @param[in]     in_size     RAWバイト列のバイト数。
        @param[in]     in_allocate メモリ割当てをするかどうか。
     */
    private: static void* read_raw(
        typename this_type::stream& io_istream,
        typename this_type::pool& io_pool,
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
                psyq::message_pack::_private::read_bytes(
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
    private: template<typename template_container, typename template_length>
    typename this_type::read_result reserve_container()
    {
        // コンテナ容量をストリームから読み込み、コンテナを予約する。
        template_length local_length;
        return this_type::read_big_endian(local_length, this->stream_)?
            this->reserve_container<template_container>(local_length):
            this_type::read_result_ABORT;
    }
    /** @brief MessagePackコンテナを予約し、スタックに積む。
        @param[in] in_capacity 予約するコンテナの容量。
     */
    private: template<typename template_container>
    typename this_type::read_result reserve_container(std::size_t const in_capacity)
    {
        static_assert(
            std::is_same<template_container, psyq::message_pack::object::array>::value
            || std::is_same<template_container, psyq::message_pack::object::unordered_map>::value,
            "template_container is not psyq::message_pack::object::array"
            "or psyq::message_pack::object::unordered_map.");
        if (this->container_stack_.size() <= this->get_rest_container_count())
        {
            PSYQ_ASSERT(false);
            return this_type::read_result_FAILED;
        }

        // コンテナを構築する。
        template_container local_container;
        bool local_reserve_container(
            this_type::reserve_container(
                local_container,
                this->pool_,
                in_capacity,
                sizeof(std::int64_t)));
        if (!local_reserve_container)
        {
            PSYQ_ASSERT(false);
            return this_type::read_result_FAILED;
        }
        else if (in_capacity <= 0)
        {
            return this->add_container_element(
                psyq::message_pack::object(local_container));
        }

        // コンテナをスタックに積む。
        auto& local_stack_top(
            this->container_stack_[this->get_rest_container_count()]);
        local_stack_top.object = psyq::message_pack::object(local_container);
        local_stack_top.kind =
            std::is_same<template_container, psyq::message_pack::object::array>
                ::value?
                    this_type::value_kind_ARRAY_ELEMENT:
                    this_type::value_kind_MAP_KEY;
        local_stack_top.rest_count = in_capacity;
        ++this->stack_size_;
        return this_type::read_result_CONTINUE;
    }
    /** @brief MessagePackコンテナを予約する。
        @param[out]    out_container 予約したMessagePackコンテナの格納先。
        @param[in,out] io_pool       psyq::message_pack::pool 互換のメモリ割当子。
        @param[in]     in_capacity   予約するコンテナの容量。
        @param[in]     in_alignment  メモリ境界単位。
        @retval true  成功。
        @retval false 失敗。
     */
    private: template<typename template_container>
    static bool reserve_container(
        template_container& out_container,
        typename this_type::pool& io_pool,
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
        out_container.reset(local_storage, 0);
        return true;
    }

    //-------------------------------------------------------------------------
    /** @brief コンテナスタックを更新する。
        @param[in] in_object コンテナに追加するオブジェクト。
     */
    private: typename this_type::read_result add_container_element(
        psyq::message_pack::object in_object)
    PSYQ_NOEXCEPT
    {
        if (this->get_rest_container_count() <= 0)
        {
            this->container_stack_.front().object = in_object;
            return this_type::read_result_FINISH;
        }
        for (;;)
        {
            // スタックに積まれているコンテナに、オブジェクトを追加する。
            auto& local_stack_top(
                this->container_stack_[this->get_rest_container_count() - 1]);
            switch (local_stack_top.kind)
            {
            case this_type::value_kind_ARRAY_ELEMENT:
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
                --local_stack_top.rest_count;
                if (0 < local_stack_top.rest_count)
                {
                    return this_type::read_result_CONTINUE;
                }
                break;
            }

            case this_type::value_kind_MAP_KEY:
                // 連想配列のキーを保存する。
                local_stack_top.map_key = in_object;
                local_stack_top.kind = this_type::value_kind_MAP_VALUE;
                return this_type::read_result_CONTINUE;

            case this_type::value_kind_MAP_VALUE:
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
                    return this_type::read_result_FAILED;
                }

                // 残り要素数を更新する。
                --local_stack_top.rest_count;
                if (0 < local_stack_top.rest_count)
                {
                    local_stack_top.kind = this_type::value_kind_MAP_KEY;
                    return this_type::read_result_CONTINUE;
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
                return this_type::read_result_FAILED;
            }

            // オブジェクトをスタックから取り出す。
            in_object = local_stack_top.object;
            if (1 < this->get_rest_container_count())
            {
                --this->stack_size_;
            }
            else
            {
                this->stack_size_ = 0;
                return this_type::read_result_FINISH;
            }
        }
    }

    //-------------------------------------------------------------------------
    /// @copydoc stream
    private: typename this_type::stream stream_;
    /// @copydoc pool
    private: typename this_type::pool pool_;
    /// @copydoc container_stack
    private: std::array<typename this_type::container_stack, template_stack_capacity>
        container_stack_;
    /// 直列化復元している途中のコンテナのスタック階層数。
    private: std::size_t stack_size_;
    /// RAWバイト列の構築で、メモリ割当てをするかどうか。
    private: bool allocate_raw_;
    /// 連想配列の構築で、要素をソートするかどうか。
    private: bool sort_map_;

}; // class psyq::message_pack::deserializer

#endif // !defined(PSYQ_MESSAGE_PACK_DESIRIALIZER_HPP_)
