/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief 独自のバイナリデータ交換形式。
 */
#ifndef PSYQ_BINARC_HPP_
#define PSYQ_BINARC_HPP_

#include <memory>
#include <algorithm>

#ifndef PSYQ_ASSERT
#include <assert.h>
#define PSYQ_ASSERT assert
#endif // PSYQ_ASSERT

namespace psyq
{
    class binarc_archive;
    class binarc_node;
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::binarc_archive
{
    /// thisが指す値の型。
    private: typedef binarc_archive this_type;

    public: typedef std::shared_ptr<this_type> shared_ptr;
    public: typedef std::weak_ptr<this_type> weak_ptr;

    public: typedef std::uint32_t unit;

    public: binarc_archive(
        void const* const in_front,
        std::size_t const in_size):
    unit_front_(static_cast<this_type::unit const*>(in_front)),
    unit_count_(in_size / sizeof(this_type::unit))
    {
        PSYQ_ASSERT(
            reinterpret_cast<std::size_t>(in_front) % sizeof(std::uint64_t) == 0
            && in_size % sizeof(this_type::unit) == 0);
        PSYQ_ASSERT(in_front != nullptr || in_size == 0);
    }

    /** @brief binarc書庫内のメモリ位置を取得する。
        @param[in] in_index メモリ単位のインデックス番号。
     */
    public: this_type::unit const* get_unit(std::size_t const in_index) const
    {
        return in_index < this->unit_count_?
            this->unit_front_ + in_index: nullptr;
    }

    private: this_type::unit const* unit_front_;
    private: std::size_t unit_count_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::binarc_node
{
    /// thisが指す値の型。
    private: typedef binarc_node this_type;

    /// ノードが指す値の種別。
    public: enum kind: std::uint8_t
    {
        kind_NIL,      ///< 空。
        kind_BOOLEAN,  ///< 真偽値。
        kind_STRING,   ///< 文字列。
        kind_EXTENDED, ///< 拡張型バイト列。
        kind_ARRAY,    ///< 配列。
        kind_MAP,      ///< 辞書。
        kind_UNSIGNED, ///< 符号なし整数。
        kind_NEGATIVE = kind_UNSIGNED + 3, ///< 負の整数。
        kind_FLOATING = kind_NEGATIVE + 3, ///< IEEE754浮動小数点数。
    };

    /// ノードが指す数値の格納形式。
    private: enum numerics: std::uint8_t
    {
        numerics_UNSIGNED_IMMEDIATE = this_type::kind_UNSIGNED,
        numerics_UNSIGNED_32,
        numerics_UNSIGNED_64,
        numerics_NEGATIVE_IMMEDIATE = this_type::kind_NEGATIVE,
        numerics_NEGATIVE_32,
        numerics_NEGATIVE_64,
        numerics_FLOATING_32 = this_type::kind_FLOATING,
        numerics_FLOATING_64,
    };

    public: static std::size_t const MAP_INDEX_NONE = 0 - std::size_t(1);

    private: static unsigned const TAG_FORMAT_BITS_SIZE = 4;
    private: static unsigned const TAG_FORMAT_BITS_MAX
        = (1 << this_type::TAG_FORMAT_BITS_SIZE) - 1;
    private: static unsigned const TAG_FORMAT_BITS_POSITION
        = sizeof(binarc_archive::unit) * 4 - this_type::TAG_FORMAT_BITS_SIZE;
    private: static unsigned const TAG_IMMEDIATE_BITS_MASK
        = ~(this_type::TAG_FORMAT_BITS_MAX << this_type::TAG_FORMAT_BITS_POSITION);
    private: static unsigned const CONTAINER_HASH = 0;
    private: static unsigned const CONTAINER_SIZE = this_type::CONTAINER_HASH + 1;
    private: static unsigned const CONTAINER_FRONT = this_type::CONTAINER_SIZE + 1;
    private: static unsigned const EXTENDED_KIND = this_type::CONTAINER_HASH + 1;
    private: static unsigned const EXTENDED_FRONT = this_type::EXTENDED_KIND + 1;
    private: static unsigned const UNIT_COUNT_PER_MAP_ELEMENT = 2;

    //-------------------------------------------------------------------------
    /** @brief 空のノードを構築する。
     */
    public: binarc_node(): tag_(nullptr) {}

    /** @brief binarc書庫の最上位ノードを構築する。
        @param[in] in_archive 保持するbinarc書庫。
     */
    public: explicit binarc_node(psyq::binarc_archive::shared_ptr in_archive):
    archive_(std::move(in_archive))
    {
        auto const local_archive(this->archive_.get());
        if (local_archive != nullptr)
        {
            this->tag_ = local_archive->get_unit(1);
            if (this->tag_ == nullptr)
            {
                this->archive_.reset();
            }
        }
        else
        {
            this->tag_ = nullptr;
        }
    }

    private: binarc_node(
        psyq::binarc_archive::unit const* const in_tag,
        psyq::binarc_archive::shared_ptr const& in_archive):
    tag_(in_tag),
    archive_(in_archive)
    {}

    //-------------------------------------------------------------------------
    public: binarc_archive::shared_ptr const& get_archive() const
    {
        return this->archive_;
    }

    public: bool is_empty() const
    {
        return this->get_archive().get() == nullptr;
    }

    public: this_type::kind get_kind() const
    {
        return this_type::make_kind(this->get_format());
    }

    private: unsigned get_format() const
    {
        return this->tag_ != nullptr?
            *(this->tag_) >> this_type::TAG_FORMAT_BITS_POSITION:
            this_type::kind_NIL;
    }

    private: static this_type::kind make_kind(unsigned const in_format)
    {
        switch (in_format)
        {
        case this_type::numerics_UNSIGNED_32:
        case this_type::numerics_UNSIGNED_64:
            return this_type::kind_UNSIGNED;
        case this_type::numerics_NEGATIVE_32:
        case this_type::numerics_NEGATIVE_64:
            return this_type::kind_NEGATIVE;
        case this_type::numerics_FLOATING_64:
            return this_type::kind_FLOATING;
        default:
            return in_format <= this_type::numerics_FLOATING_64?
                static_cast<this_type::kind>(in_format): this_type::kind_NIL;
        }
    }

    //-------------------------------------------------------------------------
    public: bool is_numerics() const
    {
        return this_type::kind_UNSIGNED <= this->get_format();
    }

    /** @brief ノードが指す数値を取得する。
        @param[in] in_default 数値の取得に失敗した場合に返す値。
        @return
            ノードが指す値を template_numerics にキャストした値。
            ただし、ノードが数値を指してない場合は、 in_default を返す。
     */
    public: template<typename template_numerics>
    template_numerics make_numerics(template_numerics const in_default) const
    {
        template_numerics local_numerics;
        return this->read_numerics(local_numerics) < 0?
            in_default: local_numerics;
    }

    /** @brief ノードが指す数値を取得する。
        @param[out] out_numerics ノードから取得した数値が代入される。
        @retval 正 ノードが指す値の等値を out_numerics へ代入。
        @retval 0  ノードが指す値を template_numerics にキャストして out_numerics へ代入。
        @retval 負 失敗。ノードは数値を指してない。 out_numerics は変わらない。
     */
    public: template<typename template_numerics>
    int read_numerics(template_numerics& out_numerics) const
    {
        switch (this->get_format())
        {
        case this_type::numerics_UNSIGNED_IMMEDIATE:
            return this->read_immediate_numerics<
                template_numerics,
                std::make_unsigned<binarc_archive::unit>::type>(
                    out_numerics);
        case this_type::numerics_UNSIGNED_32:
            return this->read_body_numerics<template_numerics, std::uint32_t>(out_numerics);
        case this_type::numerics_UNSIGNED_64:
            return this->read_body_numerics<template_numerics, std::uint64_t>(out_numerics);
        case this_type::numerics_NEGATIVE_IMMEDIATE:
            return this->read_immediate_numerics<
                template_numerics,
                std::make_signed<binarc_archive::unit>::type>(
                    out_numerics);
        case this_type::numerics_NEGATIVE_32:
            return this->read_body_numerics<template_numerics, std::int32_t>(out_numerics);
        case this_type::numerics_NEGATIVE_64:
            return this->read_body_numerics<template_numerics, std::int64_t>(out_numerics);
        case this_type::numerics_FLOATING_32:
            return this->read_body_numerics<template_numerics, float>(out_numerics);
        case this_type::numerics_FLOATING_64:
            return this->read_body_numerics<template_numerics, double>(out_numerics);
        default:
            return -1;
        }
    }

    private: template<typename template_write, typename template_read>
    bool read_immediate_numerics(template_write& out_numerics) const
    {
        static_assert(std::is_integral<template_read>::value, "");
        PSYQ_ASSERT(this->tag_ != nullptr);
        auto const local_immediate(
            *this->tag_ & this_type::TAG_IMMEDIATE_BITS_MASK);
        auto const local_sign(
            (this_type::TAG_FORMAT_BITS_MAX << this_type::TAG_FORMAT_BITS_POSITION)
            * std::is_signed<template_read>::value);
        return this_type::read_argument_numerics(
            out_numerics,
            static_cast<template_read>(local_immediate | local_sign));
    }

    private: template<typename template_write, typename template_read>
    bool read_body_numerics(template_write& out_numerics) const
    {
        PSYQ_ASSERT(this->tag_ != nullptr && this->archive_.get() != nullptr);
        auto const local_body(
            this->archive_->get_unit(
                *this->tag_ & this_type::TAG_IMMEDIATE_BITS_MASK));
        PSYQ_ASSERT(local_body != nullptr);
        PSYQ_ASSERT(reinterpret_cast<std::size_t>(local_body) % sizeof(template_read) == 0);
        return this_type::read_argument_numerics(
            out_numerics, *reinterpret_cast<template_read const*>(local_body));
    }

    private: template<typename template_write, typename template_read>
    static bool read_argument_numerics(
        template_write& out_numerics,
        template_read const in_numerics)
    {
        // 読み込み値と書き込み値のビット配置が異なるか、
        // 符号が異なる場合は、等値が代入できてない。
        out_numerics = static_cast<template_write>(in_numerics);
        return in_numerics == out_numerics
            && ((in_numerics < 0) ^ (out_numerics < 0)) == 0;
    }

    //-------------------------------------------------------------------------
    /** @brief ノードが指す真偽値を取得する。
        @retval 正 ノードはtrueを指している。
        @retval 0  ノードはfalseを指している。
        @retval 負 ノードは真偽値を指してない。
     */
    public: int get_boolean_state() const
    {
        return this->get_format() == this_type::kind_BOOLEAN?
            (*this->tag_ & this_type::TAG_IMMEDIATE_BITS_MASK) != 0: -1;
    }

    /** @brief ノードが指す真偽値を取得する。
        @param[out] out_boolean 取得した真偽値が代入される。
        @retval true 成功。 out_boolean に取得した真偽値を代入。
        @retval false
            失敗。ノードが真偽値を指してない。 out_boolean は変化しない。
     */
    public: bool read_boolean(bool& out_boolean) const
    {
        auto const local_state(this->get_boolean_state());
        if (local_state < 0)
        {
            return false;
        }
        out_boolean = (0 < local_state);
        return true;
    }

    /** @brief ノードが指す文字列のバイト数を取得する。
        @return
            ノードが指す文字列のバイト数。
            ただし、ノードが文字列を指してない場合は、0を返す。
     */
    public: std::size_t get_string_size() const
    {
        auto const local_body(this->get_body(this_type::kind_STRING));
        return local_body != nullptr? local_body[this_type::CONTAINER_SIZE]: 0;
    }

    /** @brief ノードが指す文字列の先頭位置を取得する。
        @return
            ノードが指す文字列の先頭位置。
            ただし、ノードが文字列を指してない場合は、nullptrを返す。
     */
    public: char const* get_string_data() const
    {
        auto const local_body(this->get_body(this_type::kind_STRING));
        return local_body != nullptr?
            reinterpret_cast<char const*>(local_body + this_type::CONTAINER_FRONT):
            nullptr;
    }

    /** @brief ノードが指す拡張バイナリのバイト数を取得する。
        @return
            ノードが指す拡張バイナリのバイト数。
            ただし、ノードが拡張バイナリを指してない場合は、0を返す。
     */
    public: std::size_t get_extended_size() const
    {
        auto const local_body(this->get_body(this_type::kind_EXTENDED));
        return local_body != nullptr? local_body[this_type::CONTAINER_SIZE]: 0;
    }

    /** @brief ノードが指す拡張バイナリの種別を取得する。
        @return
            ノードが指す拡張バイナリの種別。
            ただし、ノードが拡張バイナリを指してない場合は、0を返す。
     */
    public: psyq::binarc_archive::unit get_extended_kind() const
    {
        auto const local_body(this->get_body(this_type::kind_EXTENDED));
        return local_body != nullptr? local_body[this_type::EXTENDED_KIND]: 0;
    }

    /** @brief ノードが指す拡張バイナリの先頭位置を取得する。
        @return
            ノードが指す拡張バイナリの先頭位置。
            ただし、ノードが拡張バイナリを指してない場合は、nullptrを返す。
     */
    public: void const* get_extended_data() const
    {
        auto const local_body(this->get_body(this_type::kind_EXTENDED));
        return local_body != nullptr && 0 < local_body[this_type::CONTAINER_SIZE]?
            local_body + this_type::EXTENDED_FRONT: nullptr;
    }

    /** @brief ノードが指す配列の要素数を取得する。
        @return
            ノードが指す配列の要素数。
            ただし、ノードが配列を指してない場合は、0を返す。
     */
    public: std::size_t get_array_size() const
    {
        auto const local_body(this->get_body(this_type::kind_ARRAY));
        return local_body != nullptr? local_body[this_type::CONTAINER_SIZE]: 0;
    }

    /** @brief ノードが指す配列の要素を取得する。
        @param[in] in_index 取得する配列要素のインデックス番号。
        @return
            配列の要素を指すノード。
            ただし、該当する配列要素が存在しない場合は、空ノードを返す。
     */
    public: this_type make_array_element(std::size_t const in_index) const
    {
        auto const local_body(this->get_body(this_type::kind_ARRAY));
        return local_body != nullptr && in_index < *local_body?
            this_type(local_body + this_type::CONTAINER_FRONT + in_index, this->archive_):
            this_type();
    }

    /** @brief ノードが指す辞書の要素数を取得する。
        @return
            ノードが指す辞書の要素数。
            ただし、ノードが辞書を指してない場合は、0を返す。
     */
    public: std::size_t get_map_size() const
    {
        auto const local_body(this->get_body(this_type::kind_MAP));
        return local_body != nullptr? local_body[this_type::CONTAINER_SIZE]: 0;
    }

    /** @brief ノードが指す辞書のキーを取得する。
        @param[in] in_index 取得するキーを持つ辞書要素のインデックス番号。
        @return
            辞書要素のキーを指すノード。
            ただし、該当する辞書要素が存在しない場合は、空ノードを返す。
     */
    public: this_type make_map_key(std::size_t const in_index) const
    {
        auto const local_body(this->get_body(this_type::kind_MAP));
        return local_body != nullptr && in_index < *local_body?
            this_type(
                local_body + this_type::CONTAINER_FRONT
                + in_index * this_type::UNIT_COUNT_PER_MAP_ELEMENT,
                this->archive_):
            this_type();
    }

    /** @brief ノードが指す辞書の値を取得する。
        @param[in] in_index 取得する値を持つ辞書要素のインデックス番号。
        @return
            辞書要素の値を指すノード。
            ただし、該当する辞書要素が存在しない場合は、空ノードを返す。
     */
    public: this_type make_map_value(std::size_t const in_index) const
    {
        auto const local_body(this->get_body(this_type::kind_MAP));
        return local_body != nullptr && in_index < *local_body?
            this_type(
                local_body + this_type::CONTAINER_FRONT + 1
                + in_index * this_type::UNIT_COUNT_PER_MAP_ELEMENT,
                this->archive_):
            this_type();
    }

    //-------------------------------------------------------------------------
    /// 辞書の検索に使うキー。
    private: struct map_key
    {
        void initialize(std::uint64_t const in_key)
        {
            if ((std::numeric_limits<std::uint32_t>::max)() < in_key)
            {
                // 64ビット無符号整数としてハッシュ化。
                this->bits_64 = in_key;
                this->hash = static_cast<std::uint32_t>(in_key)
                    ^ static_cast<std::uint32_t>(in_key >> 32);
                this->format = psyq::binarc_node::numerics_UNSIGNED_64;
            }
            else
            {
                // 無符号整数としてハッシュ化。
                auto const local_key(static_cast<std::uint32_t>(in_key));
                this->bits_32[0] = local_key;
                this->hash = local_key;
                this->format = psyq::binarc_node::TAG_IMMEDIATE_BITS_MASK < in_key?
                    psyq::binarc_node::numerics_UNSIGNED_32:
                    psyq::binarc_node::numerics_UNSIGNED_IMMEDIATE;
            }
        }

        void initialize(std::int64_t const in_key)
        {
            if (0 <= in_key)
            {
                // 無符号整数としてハッシュ化。
                this->initialize(static_cast<std::uint64_t>(in_key));
            }
            else if (in_key < (std::numeric_limits<std::int32_t>::min)())
            {
                // 64ビットで負の整数としてハッシュ化。
                this->bits_64 = static_cast<std::uint64_t>(in_key);
                this->hash = this->bits_32[0] ^ this->bits_32[1];
                this->format = psyq::binarc_node::numerics_NEGATIVE_64;
            }
            else
            {
                auto const local_key(static_cast<std::uint32_t>(in_key));
                this->bits_32[0] = local_key;
                if (local_key < (this_type::TAG_FORMAT_BITS_MAX << this_type::TAG_FORMAT_BITS_POSITION))
                {
                    // 32ビットで負の整数としてハッシュ化。
                    this->hash = local_key;
                    this->format = psyq::binarc_node::numerics_NEGATIVE_32;
                }
                else
                {
                    // 負の整数としてハッシュ化。
                    this->hash = local_key & psyq::binarc_node::TAG_IMMEDIATE_BITS_MASK;
                    this->format = psyq::binarc_node::numerics_NEGATIVE_IMMEDIATE;
                }
            }
        }

        void initialize(double const in_key)
        {
            auto const local_integer_key(static_cast<std::int64_t>(in_key));
            if (in_key == local_integer_key)
            {
                // 整数としてハッシュ化。
                this->initialize(local_integer_key);
                return;
            }
            this->floating_32 = static_cast<float>(in_key);
            if (in_key == this->floating_32)
            {
                // 単精度浮動小数点数としてハッシュ化。
                this->hash = this->bits_32[0];
                this->format = psyq::binarc_node::numerics_FLOATING_32;
            }
            else
            {
                // 倍精度浮動小数点数としてハッシュ化。
                this->floating_64 = in_key;
                this->hash = this->bits_32[0] ^ this->bits_32[1];
                this->format = psyq::binarc_node::numerics_FLOATING_64;
            }
        }

        int compare_node_value(
            psyq::binarc_archive const& in_archive,
            psyq::binarc_archive::unit const in_node_tag)
        const
        {
            auto const local_node_format(
                in_node_tag >> psyq::binarc_node::TAG_FORMAT_BITS_POSITION);
            if (local_node_format != this->format)
            {
                return -1;
            }
            auto const local_node_immediate(
                in_node_tag & psyq::binarc_node::TAG_IMMEDIATE_BITS_MASK);
            switch (local_node_format)
            {
            case this_type::kind_BOOLEAN:
            case this_type::numerics_UNSIGNED_IMMEDIATE:
            case this_type::numerics_NEGATIVE_IMMEDIATE:
                return local_node_immediate == this->bits_32[0]? 1: -1;
            case this_type::numerics_UNSIGNED_32:
            case this_type::numerics_NEGATIVE_32:
            case this_type::numerics_FLOATING_32:
            {
                auto const local_body(
                    in_archive.get_unit(local_node_immediate));
                if (local_body != nullptr)
                {
                    return *local_body == this->bits_32[0]? 1: -1;
                }
                break;
            }
            case this_type::numerics_UNSIGNED_64:
            case this_type::numerics_NEGATIVE_64:
            case this_type::numerics_FLOATING_64:
            {
                auto const local_body(
                    reinterpret_cast<std::uint64_t const*>(
                        in_archive.get_unit(local_node_immediate)));
                if (local_body != nullptr)
                {
                    return *local_body == this->bits_64? 1: -1;
                }
                break;
            }
            case this_type::kind_STRING:
            case this_type::kind_EXTENDED:
            {
                auto const local_body(
                    in_archive.get_unit(local_node_immediate));
                if (local_body != nullptr)
                {
                    if (this->hash != local_body[psyq::binarc_node::CONTAINER_HASH])
                    {
                        return -1;
                    }
                    if (this->raw.size != local_body[psyq::binarc_node::CONTAINER_SIZE])
                    {
                        return 0;
                    }
                    auto local_node_string(local_body);
                    if (local_node_format == this_type::kind_STRING)
                    {
                        local_node_string += psyq::binarc_node::CONTAINER_FRONT;
                    }
                    else if (this->raw.kind == local_body[psyq::binarc_node::EXTENDED_KIND])
                    {
                        local_node_string += psyq::binarc_node::EXTENDED_FRONT;
                    }
                    else
                    {
                        return 0;
                    }
                    return std::memcmp(local_node_string, this->raw.data, this->raw.size) == 0? 1: -1;
                }
                break;
            }
            case this_type::kind_ARRAY:
            case this_type::kind_MAP:
            default:
                break;
            }
            return 0;
        }

        psyq::binarc_archive::unit hash;
        psyq::binarc_archive::unit format;
        union
        {
            double floating_64;
            float floating_32;
            std::uint64_t bits_64;
            std::uint32_t bits_32[2];
            struct
            {
                void const* data;
                std::size_t size;
                psyq::binarc_archive::unit kind;
            } raw;
        };
    };

    /// 辞書の要素。
    private: typedef psyq::binarc_archive::unit map_element[2];

    /// 辞書の要素を比較する関数オブジェクト。
    private: struct less_map
    {
        less_map(psyq::binarc_archive const& in_archive):
        archive(in_archive)
        {}

        bool operator()(
            psyq::binarc_node::map_key const& in_left_key,
            psyq::binarc_node::map_element const& in_right_element)
        const
        {
            auto const local_compare(
                psyq::binarc_node::compare_node_hash(
                    this->archive, in_right_element[0], in_left_key));
            return 0 < local_compare;
        }

        bool operator()(
            psyq::binarc_node::map_element const& in_left_element,
            psyq::binarc_node::map_key const& in_right_key)
        const
        {
            auto const local_compare(
                psyq::binarc_node::compare_node_hash(
                    this->archive, in_left_element[0], in_right_key));
            return local_compare < 0;
        }

        psyq::binarc_archive const& archive;
    };

    public: std::size_t find_map_index(double const in_key) const
    {
        this_type::map_key local_map_key;
        local_map_key.initialize(in_key);

        auto const local_body(this->get_body(this_type::kind_MAP));
        if (local_body != nullptr)
        {
            auto const local_archive(this->archive_.get());
            PSYQ_ASSERT(local_archive != nullptr);

            auto const local_begin(
                reinterpret_cast<this_type::map_element const*>(
                    local_body + this_type::CONTAINER_FRONT));
            auto const local_end(
                local_begin + local_body[this_type::CONTAINER_SIZE]);
            auto const local_position(
                std::lower_bound(
                    local_begin,
                    local_end,
                    local_map_key,
                    this_type::less_map(*this->archive_)));
            for (auto i(local_position); i < local_end; ++i)
            {
                auto const local_compare(
                    local_map_key.compare_node_value(*local_archive, (*i)[0]));
                if (local_compare < 0)
                {
                    break;
                }
                if (0 < local_compare)
                {
                    return i - local_begin;
                }
            }
        }
        return this_type::MAP_INDEX_NONE;
    }

    /** @brief ノードのハッシュ値を比較する。
        @param[in] in_archive    左辺ノードを含む書庫。
        @param[in] in_left_tag   左辺ノードのタグ値。
        @param[in] in_right_key  右辺キー。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
        @retval 負 左辺のほうが小さい。
     */
    private: static int compare_node_hash(
        psyq::binarc_archive const& in_archive,
        psyq::binarc_archive::unit const in_left_tag,
        this_type::map_key const& in_right_key)
    {
        auto const local_left_format(
            in_left_tag >> this_type::TAG_FORMAT_BITS_POSITION);
        auto const local_right_format(in_right_key.format);
        if (local_left_format < local_right_format)
        {
            return -1;
        }
        if (local_right_format < local_left_format)
        {
            return 1;
        }
        auto const local_left_hash(this_type::get_node_hash(in_archive, in_left_tag));
        auto const local_right_hash(in_right_key.hash);
        if (local_left_hash < local_right_hash)
        {
            return -1;
        }
        if (local_right_hash < local_left_hash)
        {
            return 1;
        }
        return 0;
    }

    /** @brief ノードが指す値のハッシュ値を取得する。
        @param[in] in_archive  ノードを含む書庫。
        @param[in] in_node_tag ノードのタグ値。
        @return ノードが指す値のハッシュ値。
     */
    private: static psyq::binarc_archive::unit get_node_hash(
        psyq::binarc_archive const& in_archive,
        psyq::binarc_archive::unit const in_node_tag)
    {
        auto const local_format(
            in_node_tag >> this_type::TAG_FORMAT_BITS_POSITION);
        auto const local_immediate(
            in_node_tag & this_type::TAG_IMMEDIATE_BITS_MASK);
        switch (local_format)
        {
        case this_type::kind_BOOLEAN:
        case this_type::numerics_UNSIGNED_IMMEDIATE:
        case this_type::numerics_NEGATIVE_IMMEDIATE:
            return local_immediate;
        case this_type::numerics_UNSIGNED_32:
        case this_type::numerics_NEGATIVE_32:
        case this_type::numerics_FLOATING_32:
        case this_type::kind_STRING:
        case this_type::kind_EXTENDED:
        case this_type::kind_ARRAY:
        case this_type::kind_MAP:
        {
            auto const local_body(in_archive.get_unit(local_immediate));
            if (local_body != nullptr)
            {
                static_assert(this_type::CONTAINER_HASH == 0, "");
                return *local_body;
            }
            break;
        }
        case this_type::numerics_UNSIGNED_64:
        case this_type::numerics_NEGATIVE_64:
        case this_type::numerics_FLOATING_64:
        {
            auto const local_body(in_archive.get_unit(local_immediate));
            if (local_body != nullptr)
            {
                return local_body[0] ^ local_body[1];
            }
            break;
        }
        default:
            break;
        }
        PSYQ_ASSERT(false);
        return 0 - psyq::binarc_archive::unit(1);
    }

    //-------------------------------------------------------------------------
    private: psyq::binarc_archive::unit const* get_body(unsigned const in_format)
    const
    {
        if (this->tag_ != nullptr)
        {
            auto const local_tag(*this->tag_);
            if ((local_tag >> this_type::TAG_FORMAT_BITS_POSITION) == in_format)
            {
                auto const local_archive(this->archive_.get());
                if (local_archive != nullptr)
                {
                    auto const local_body(
                        local_archive->get_unit(
                            local_tag & this_type::TAG_IMMEDIATE_BITS_MASK));
                    PSYQ_ASSERT(local_body != nullptr);
                    return local_body;
                }
                PSYQ_ASSERT(false);
            }
        }
        return nullptr;
    }

    //-------------------------------------------------------------------------
    private: psyq::binarc_archive::unit const* tag_;
    private: psyq::binarc_archive::shared_ptr archive_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class binarc_to_block_yaml
{
    /// thisが指す値の型。
    private: typedef binarc_to_block_yaml this_type;

    //-------------------------------------------------------------------------
    public: static void convert(
        std::ostringstream& out_stream,
        psyq::binarc_archive::shared_ptr const& in_archive)
    {
        psyq::binarc_node const local_node(in_archive);
        if (!local_node.is_empty())
        {
            this_type::convert_node(out_stream, local_node);
        }
    }

    //-------------------------------------------------------------------------
    private: static void convert_node(
        std::ostringstream& out_stream,
        psyq::binarc_node const& in_node)
    {
        switch (in_node.get_kind())
        {
        case psyq::binarc_node::kind_UNSIGNED:
            out_stream << in_node.make_numerics<std::uint64_t>(0);
            break;
        case psyq::binarc_node::kind_NEGATIVE:
            out_stream << in_node.make_numerics<std::int64_t>(0);
            break;
        case psyq::binarc_node::kind_FLOATING:
            out_stream << in_node.make_numerics<double>(0);
            break;
        case psyq::binarc_node::kind_STRING:
            this_type::convert_string(out_stream, in_node);
            break;
        case psyq::binarc_node::kind_ARRAY:
            this_type::convert_sequence(out_stream, in_node);
            break;
        case psyq::binarc_node::kind_MAP:
            this_type::convert_mapping(out_stream, in_node);
            break;
        default:
            PSYQ_ASSERT(false);
            break;
        }
    }

    private: static void convert_string(
        std::ostringstream& out_stream,
        psyq::binarc_node const& in_node)
    {
        auto const local_data(in_node.get_string_data());
        assert(local_data != nullptr);
        out_stream << '\'';
        out_stream.write(local_data, in_node.get_string_size());
        out_stream << '\'';
    }

    private: static void convert_sequence(
        std::ostringstream& out_stream,
        psyq::binarc_node const& in_node)
    {
        out_stream << '[';
        auto const local_size(in_node.get_array_size());
        for (unsigned i(0); i < local_size; ++i)
        {
            if (0 < i)
            {
                out_stream << ',';
            }
            this_type::convert_node(out_stream, in_node.make_array_element(i));
        }
        out_stream << ']';
    }

    private: static void convert_mapping(
        std::ostringstream& out_stream,
        psyq::binarc_node const& in_node)
    {
        out_stream << '{';
        auto const local_size(in_node.get_map_size());
        for (unsigned i(0); i < local_size; ++i)
        {
            if (0 < i)
            {
                out_stream << ',';
            }
            this_type::convert_node(out_stream, in_node.make_map_key(i));
            out_stream << ':';
            this_type::convert_node(out_stream, in_node.make_map_value(i));
        }
        out_stream << '}';
    }
};

#endif // PSYQ_BINARC_HPP_
