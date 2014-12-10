/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief 独自のバイナリデータ交換形式。
 */
#ifndef PSYQ_BINARC_HPP_
#define PSYQ_BINARC_HPP_

#include <memory>

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
    public: this_type::unit const* get_body(std::size_t const in_index) const
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
        kind_NIL,           ///< 空。
        kind_UNSIGNED,      ///< 符号なし整数。
        kind_BOOLEAN = 4,   ///< 真偽値。
        kind_NEGATIVE,      ///< 負の整数。
        kind_STRING = 8,    ///< 文字列。
        kind_EXTENDED,      ///< 拡張バイナリ。
        kind_ARRAY,         ///< 配列。
        kind_MAP,           ///< 辞書。
        kind_FLOATING = 14, ///< IEEE754浮動小数点数。
    };

    /// ノードが指す数値の種別。
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

    private: static const unsigned TAG_KIND_BITS_SIZE = 4;
    private: static const unsigned TAG_KIND_BITS_MAX
        = (1 << this_type::TAG_KIND_BITS_SIZE) - 1;
    private: static const unsigned TAG_IMMEDIATE_BITS_SIZE
        = sizeof(binarc_archive::unit) * 4 - this_type::TAG_KIND_BITS_SIZE;
    private: static const unsigned TAG_IMMEDIATE_BITS_MASK
        = ~(this_type::TAG_KIND_BITS_MAX << this_type::TAG_IMMEDIATE_BITS_SIZE);

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
            this->tag_ = local_archive->get_body(1);
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
        auto const local_tag_kind(this->get_tag_kind());
        switch (local_tag_kind)
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
            return static_cast<this_type::kind>(local_tag_kind);
        }
    }

    private: std::uint8_t get_tag_kind() const
    {
        return this->tag_ != nullptr?
            static_cast<std::uint8_t>(
                *(this->tag_) >> this_type::TAG_IMMEDIATE_BITS_SIZE):
            this_type::kind_NIL;
    }

    //-------------------------------------------------------------------------
    /** @brief ノードが指す数値を取得する。
        @param[in] in_default 数値の取得に失敗した場合に返す値。
        @return
            ノードが指す数値。
            ただし数値が取得できなかった場合は、 in_default を返す。
     */
    public: template<typename template_numerics>
    template_numerics make_numerics(template_numerics const in_default) const
    {
        template_numerics local_numerics;
        return this->read_numerics(local_numerics)? local_numerics: in_default;
    }

    /** @brief ノードが指す数値を取得する。
        @param[out] out_numerics 読み取った数値を格納する。
        @retval true  成功。 out_numerics に読み取った数値を格納した。
        @retval false
            失敗。ノードが数値を指してないか、
            out_numerics に数値を格納すると、異なる値となってしまう。
            out_numerics は変化しない。
     */
    public: template<typename template_numerics>
    bool read_numerics(template_numerics& out_numerics) const
    {
        switch (this->get_tag_kind())
        {
        case this_type::numerics_UNSIGNED_IMMEDIATE:
            return this->read_immediate_numerics<
                template_numerics,
                std::make_unsigned<binarc_archive::unit>::type>(
                    out_numerics);
        case this_type::numerics_UNSIGNED_32:
            return this->read_body_numerics<template_numerics, std::uint32_t>(out_numerics)
        case this_type::numerics_UNSIGNED_64:
            return this->read_body_numerics<template_numerics, std::uint64_t>(out_numerics)
        case this_type::numerics_NEGATIVE_IMMEDIATE:
            return this->read_immediate_numerics<
                template_numerics,
                std::make_signed<binarc_archive::unit>::type>(
                    out_numerics);
        case this_type::numerics_NEGATIVE_32:
            return this->read_body_numerics<template_numerics, std::int32_t>(out_numerics)
        case this_type::numerics_NEGATIVE_64:
            return this->read_body_numerics<template_numerics, std::int64_t>(out_numerics)
        case this_type::numerics_FLOATING_32:
            return this->read_body_numerics<template_numerics, float>(out_numerics)
        case this_type::numerics_FLOATING_64:
            return this->read_body_numerics<template_numerics, double>(out_numerics)
        default:
            return false;
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
            (this_type::TAG_KIND_BITS_MAX << this_type::TAG_IMMEDIATE_BITS_SIZE)
            * std::is_signed<template_read>::value);
        return this_type::write_numerics(
            out_numerics,
            static_cast<template_read>(local_immediate | local_sign));
    }

    private: template<typename template_write, typename template_read>
    void read_body_numerics(template_write& out_numerics) const
    {
        PSYQ_ASSERT(this->tag_ != nullptr && this->archive_.get() != nullptr);
        auto const local_body(
            this->archive_->get_body(
                *this->tag_ & this_type::TAG_IMMEDIATE_BITS_MASK));
        PSYQ_ASSERT(local_body != nullptr);
        PSYQ_ASSERT(reinterpret_cast<std::size_t>(local_body) % sizeof(template_read) == 0);
        return this_type::write_numerics(
            out_numerics, *reinterpret_cast<template_read const*>(local_body));
    }

    private: template<typename template_write, typename template_read>
    static bool write_numerics(
        template_write& out_numerics,
        template_read const in_numerics)
    {
        auto const local_numerics(static_cast<template_write>(in_numerics));
        if (in_numerics != local_numerics
            || ((in_numerics < 0) ^ (local_numerics < 0)) != 0)
        {
            // 読み込み値と書き込み値のビット配置が異なるか、
            // 符号が異なる場合は、正しい値を取得できない。
            return false;
        }
        out_numerics = local_numerics;
        return true;
    }

    //-------------------------------------------------------------------------
    /** @brief ノードが指す真偽値を取得する。
        @retval 正 ノードはtrueを指している。
        @retval 0  ノードはfalseを指している。
        @retval 負 ノードは真偽値を指してない。
     */
    public: int get_boolean_state() const
    {
        return this->get_tag_kind() == this_type::kind_BOOLEAN?
            (*this->tag_ & this_type::TAG_IMMEDIATE_BITS_MASK) != 0: -1;
    }

    /** @brief ノードが指す真偽値を取得する。
        @param[out] out_boolean 読み取った真偽値を格納する。
        @retval true 成功。 out_boolean に読み取った真偽値を格納した。
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

    /** @brief ノードが指す文字列の要素数を取得する。
        @return
            ノードが指す文字列の要素数。
            ただし、ノードが文字列を指してない場合は、0を返す。
     */
    public: std::size_t get_string_size() const
    {
        auto const local_body(this->get_body(this_type::kind_STRING));
        return local_body != nullptr? *local_body: 0;
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
            reinterpret_cast<char const*>(local_body + 1): nullptr;
    }

    /** @brief ノードが指す拡張バイナリの種別を取得する。
        @return
            ノードが指す拡張バイナリの種別。
            ただし、ノードが拡張バイナリを指してない場合は、0を返す。
     */
    public: psyq::binarc_archive::unit get_extended_kind() const
    {
        auto const local_body(this->get_body(this_type::kind_EXTENDED));
        return local_body != nullptr? *local_body: 0;
    }

    /** @brief ノードが指す拡張バイナリのバイト数を取得する。
        @return
            ノードが指す拡張バイナリのバイト数。
            ただし、ノードが拡張バイナリを指してない場合は、0を返す。
     */
    public: std::size_t get_extended_size() const
    {
        auto const local_body(this->get_body(this_type::kind_EXTENDED));
        return local_body != nullptr? *(local_body + 1): 0;
    }

    /** @brief ノードが指す拡張バイナリの先頭位置を取得する。
        @return
            ノードが指す拡張バイナリの先頭位置。
            ただし、ノードが拡張バイナリを指してない場合は、nullptrを返す。
     */
    public: void const* get_extended_data() const
    {
        auto const local_body(this->get_body(this_type::kind_EXTENDED));
        return local_body != nullptr && 0 < *(local_body + 1)?
            local_body + 2: nullptr;
    }

    /** @brief ノードが指す配列の要素数を取得する。
        @return
            ノードが指す配列の要素数。
            ただし、ノードが配列を指してない場合は、0を返す。
     */
    public: std::size_t get_array_size() const
    {
        auto const local_body(this->get_body(this_type::kind_ARRAY));
        return local_body != nullptr? *local_body: 0;
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
            this_type(local_body + 1 + in_index, this->archive_): this_type();
    }

    /** @brief ノードが指す辞書の要素数を取得する。
        @return
            ノードが指す辞書の要素数。
            ただし、ノードが辞書を指してない場合は、0を返す。
     */
    public: std::size_t get_map_size() const
    {
        auto const local_body(this->get_body(this_type::kind_MAP));
        return local_body != nullptr? *local_body: 0;
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
            this_type(local_body + 1 + in_index * 2, this->archive_):
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
            this_type(local_body + 2 + in_index * 2, this->archive_):
            this_type();
    }

    //-------------------------------------------------------------------------
    private: psyq::binarc_archive::unit const* get_body(
        psyq::binarc_archive::unit const in_kind)
    const
    {
        if (this->tag_ != nullptr)
        {
            auto const local_tag(*this->tag_);
            if ((local_tag >> this_type::TAG_IMMEDIATE_BITS_SIZE) == in_kind)
            {
                auto const local_archive(this->archive_.get());
                if (local_archive != nullptr)
                {
                    auto const local_body(
                        local_archive->get_body(
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

#endif // PSYQ_BINARC_HPP_
