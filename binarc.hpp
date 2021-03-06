﻿/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief 独自のバイナリデータ交換形式。
 */
#ifndef PSYQ_BINARC_HPP_
#define PSYQ_BINARC_HPP_

#include <memory>
#include <algorithm>
#include <cstdint>

#ifndef PSYQ_ASSERT
#include <assert.h>
#define PSYQ_ASSERT assert
#endif // PSYQ_ASSERT

namespace psyq
{
    namespace binarc
    {
        class archive;

    } // namespace binarc
} // namespace psyq


//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// binarc形式の書庫。
class psyq::binarc::archive
{
    /// thisが指す値の型。
    private: typedef archive this_type;

    /// binarc書庫の中にある値を指す反復子。
    public: typedef void const* iterator;

    /// binarc書庫の単位ワード。
    public: typedef std::uint32_t word;

    /// 書庫が持つ値の種別。
    public: enum kind: std::uint8_t
    {
        kind_NIL,      ///< 空値。
        kind_BOOLEAN,  ///< 真偽値。
        kind_STRING,   ///< 文字列。
        kind_EXTENDED, ///< 拡張型バイト列。
        kind_ARRAY,    ///< 配列コンテナ。
        kind_MAP,      ///< 辞書コンテナ。
        kind_UNSIGNED, ///< 符号なし整数。
        kind_NEGATIVE = kind_UNSIGNED + 3, ///< 負の整数。
        kind_FLOATING = kind_NEGATIVE + 3, ///< 浮動小数点数。

    }; // enum kind

    /// コンテナの先頭位置と終端位置のペア。
    public: typedef std::pair<this_type::iterator, this_type::iterator>
        container_slice;

    /// コンテナの範囲外を示すインデックス番号。
    public: static std::size_t const CONTAINER_INDEX_NONE = ~std::size_t(0);

    //-------------------------------------------------------------------------
    private: struct container_header
    {
        this_type::word size;
    };

    private: struct string_header: public this_type::container_header
    {
        this_type::word hash;
    };

    private: struct extended_header: public this_type::string_header
    {
        this_type::word kind;
    };

    /// 値タグが指す数値の格納形式。
    private: enum numerics: std::uint8_t
    {
        /// イミディエイト符号なし整数。
        numerics_UNSIGNED_IMMEDIATE = this_type::kind_UNSIGNED,
        numerics_UNSIGNED_32, ///< 32ビット符号なし整数。
        numerics_UNSIGNED_64, ///< 64ビット符号なし整数。
        /// イミディエイト負の整数。
        numerics_NEGATIVE_IMMEDIATE = this_type::kind_NEGATIVE,
        numerics_NEGATIVE_32, ///< 32ビット負の整数。
        numerics_NEGATIVE_64, ///< 64ビット負の整数。
        /// IEEE754単精度浮動小数点数
        numerics_FLOATING_32 = this_type::kind_FLOATING,
        numerics_FLOATING_64, ///< IEEE754倍精度浮動小数点数

    }; // enum numerics

    private: enum: unsigned
    {
        TAG_FORMAT_BITS_SIZE = 4,
        TAG_FORMAT_BITS_MAX = (1 << this_type::TAG_FORMAT_BITS_SIZE) - 1,
        TAG_FORMAT_BITS_POSITION = sizeof(this_type::word) * 8
            - this_type::TAG_FORMAT_BITS_SIZE,
        TAG_IMMEDIATE_BITS_MASK
            = (1 << this_type::TAG_FORMAT_BITS_POSITION) - 1,
        ROOT_UNIT_POSITION = 1,
    };

    //-------------------------------------------------------------------------
    /// binarc辞書の検索に使うキー。
    public: class map_key
    {
        private: typedef map_key this_type;
        friend archive;

        //.....................................................................
        /** @brief 数値で、辞書検索キーを初期化する。
            @param[in] in_key 辞書検索キーの初期値となる数値。
         */
        public: template<typename template_key>
        map_key(template_key const in_key)
        {
            static_assert(
                std::is_floating_point<template_key>::value
                || std::is_integral<template_key>::value,
                "");
            if (std::is_floating_point<template_key>::value)
            {
                new(this) this_type(static_cast<double>(in_key));
            }
            else if (std::is_signed<template_key>::value)
            {
                new(this) this_type(static_cast<std::int64_t>(in_key));
            }
            else //if (std::is_unsigned<template_numerics>::value)
            {
                new(this) this_type(static_cast<std::uint64_t>(in_key));
            }
        }

        /** @brief 真偽値で、辞書検索キーを初期化する。
            @param[in] in_key 辞書検索キーの初期値となる真偽値。
         */
        public: map_key(bool const in_key)
        {
            this->bits_32_[0] = in_key;
            this->hash_ = psyq::binarc::archive::make_hash(this->bits_32_[0]);
            this->format_ = psyq::binarc::archive::kind_BOOLEAN;
        }

        /** @brief 無符号整数で、辞書検索キーを初期化する。
            @param[in] in_key 辞書検索キーの初期値となる無符号整数。
         */
        public: map_key(std::uint64_t const in_key)
        {
            if ((std::numeric_limits<std::uint32_t>::max)() < in_key)
            {
                // 64ビット無符号整数キーとして初期化。
                this->bits_64_ = in_key;
                this->hash_ = psyq::binarc::archive::make_hash(in_key);
                this->format_ = psyq::binarc::archive::numerics_UNSIGNED_64;
            }
            else
            {
                // 無符号整数キーとして初期化。
                auto const local_key(static_cast<std::uint32_t>(in_key));
                this->bits_32_[0] = local_key;
                this->hash_ = psyq::binarc::archive::make_hash(local_key);
                this->format_ = psyq::binarc::archive::TAG_IMMEDIATE_BITS_MASK < local_key?
                    psyq::binarc::archive::numerics_UNSIGNED_32:
                    psyq::binarc::archive::numerics_UNSIGNED_IMMEDIATE;
            }
        }

        /** @brief 有符号整数で、辞書検索キーを初期化する。
            @param[in] in_key 辞書検索キーの初期値となる有符号整数。
         */
        public: map_key(std::int64_t const in_key)
        {
            if (0 <= in_key)
            {
                // 無符号整数キーとして初期化
                new(this) this_type(static_cast<std::uint64_t>(in_key));
            }
            else if (in_key < (std::numeric_limits<std::int32_t>::min)())
            {
                // 64ビットで負の整数のキーとして初期化。
                this->bits_64_ = static_cast<std::uint64_t>(in_key);
                this->hash_ = psyq::binarc::archive::make_hash(
                    static_cast<std::uint64_t>(in_key));
                this->format_ = psyq::binarc::archive::numerics_NEGATIVE_64;
            }
            else
            {
                // 負の整数のキーとして初期化。
                auto const local_key(static_cast<std::uint32_t>(in_key));
                this->bits_32_[0] = local_key;
                auto const local_immediate_limits(
                    psyq::binarc::archive::TAG_FORMAT_BITS_MAX
                    << psyq::binarc::archive::TAG_FORMAT_BITS_POSITION);
                this->hash_ = psyq::binarc::archive::make_hash(local_key);
                this->format_ = local_key < local_immediate_limits?
                    psyq::binarc::archive::numerics_NEGATIVE_32:
                    psyq::binarc::archive::numerics_NEGATIVE_IMMEDIATE;
            }
        }

        /** @brief 浮動小数点数で、辞書検索キーを初期化する。
            @param[in] in_key 辞書検索キーの初期値となる浮動小数点数。
         */
        public: map_key(double const in_key)
        {
            auto const local_integer_key(static_cast<std::int64_t>(in_key));
            if (in_key == local_integer_key)
            {
                // 整数キーとして初期化。
                new(this) this_type(local_integer_key);
                return;
            }
            this->floating_32_ = static_cast<float>(in_key);
            if (in_key == this->floating_32_)
            {
                // 単精度浮動小数点数キーとして初期化。
                this->hash_ = psyq::binarc::archive::make_hash(this->bits_32_[0]);
                this->format_ = psyq::binarc::archive::numerics_FLOATING_32;
            }
            else
            {
                // 倍精度浮動小数点数キーとして初期化。
                this->floating_64_ = in_key;
                this->hash_ = psyq::binarc::archive::make_hash(this->bits_64_);
                this->format_ = psyq::binarc::archive::numerics_FLOATING_64;
            }
        }

        /** @brief 文字列で、辞書検索キーを初期化する。
            @param[in] in_string_data 文字列の先頭位置。
            @param[in] in_string_size 文字列のバイト数。
         */
        public: map_key(
            void const* const in_string_data,
            std::size_t const in_string_size)
        {
            PSYQ_ASSERT(in_string_data != nullptr || in_string_data <= 0);
            this->raw_.data = in_string_data;
            this->raw_.size = in_string_size;
            this->hash_ = psyq::binarc::archive::make_hash(
                in_string_data, in_string_size);
            this->format_ = psyq::binarc::archive::kind_STRING;
        }

        /** @brief 拡張型バイト列で、辞書検索キーを初期化する。
            @param[in] in_extended_data 拡張型バイト列の先頭位置。
            @param[in] in_extended_size 拡張型バイト列のバイト数。
            @param[in] in_extended_kind 拡張型バイト列の種別。
         */
        public: map_key(
            void const* const in_extended_data,
            std::size_t const in_extended_size,
            psyq::binarc::archive::word const in_extended_kind)
        {
            PSYQ_ASSERT(in_extended_data != nullptr || in_extended_data <= 0);
            this->raw_.data = in_extended_data;
            this->raw_.size = in_extended_size;
            this->raw_.kind = in_extended_kind;
            this->hash_ = psyq::binarc::archive::make_hash(
                in_extended_data, in_extended_size);
            this->format_ = psyq::binarc::archive::kind_EXTENDED;
        }

        //.....................................................................
        private: union
        {
            double floating_64_;
            float floating_32_;
            std::uint64_t bits_64_;
            std::uint32_t bits_32_[2];
            struct
            {
                void const* data;
                std::size_t size;
                psyq::binarc::archive::word kind;
            } raw_;
        };
        private: std::uint32_t hash_;
        private: std::uint32_t format_;

    }; // class map_key

    //-------------------------------------------------------------------------
    /// binarc辞書の要素を比較する関数オブジェクト。
    private: class less_map
    {
        private: typedef less_map this_type;

        public: less_map(psyq::binarc::archive const& in_archive):
            archive_(in_archive)
        {}

        public: bool operator()(
            psyq::binarc::archive::map_key const& in_left_key,
            psyq::binarc::archive::word const& in_right_element)
        const
        {
            auto const local_compare(
                this->archive_.compare_hash(in_left_key, in_right_element));
            return local_compare < 0;
        }

        public: bool operator()(
            psyq::binarc::archive::word const& in_left_element,
            psyq::binarc::archive::map_key const& in_right_key)
        const
        {
            auto const local_compare(
                this->archive_.compare_hash(in_right_key, in_left_element));
            return 0 < local_compare;
        }

        private: psyq::binarc::archive const& archive_;

    }; // class less_map

    //-------------------------------------------------------------------------
    public: archive(
        void const* const in_front,
        std::size_t const in_size)
    :
        unit_front_(static_cast<this_type::word const*>(in_front)),
        unit_count_(in_size / sizeof(this_type::word))
    {
        PSYQ_ASSERT(
            reinterpret_cast<std::size_t>(in_front) % sizeof(std::uint64_t) == 0
            && in_size % sizeof(this_type::word) == 0);
        PSYQ_ASSERT(in_front != nullptr || in_size == 0);
    }

    /** @brief binarc書庫の最上位の値を指す反復子を取得する。
        @return *thisの最上位の値を指す反復子。
     */
    public: this_type::iterator get_root() const
    {
        return this->get_word(this_type::ROOT_UNIT_POSITION);
    }

    /** @brief binarc書庫内のメモリ位置を取得する。
        @param[in] in_index メモリ単位のインデックス番号。
     */
    private: this_type::word const* get_word(
        std::size_t const in_index)
    const
    {
        return in_index < this->unit_count_?
            this->unit_front_ + in_index: nullptr;
    }

    private: this_type::word const* get_word(
        this_type::iterator const in_iterator)
    const
    {
        return this->unit_front_ <= in_iterator
            && in_iterator < this->unit_front_ + this->unit_count_?
                static_cast<this_type::word const*>(in_iterator):
                nullptr;
    }

    //-------------------------------------------------------------------------
    /** @brief 反復子が指す値の種別を取得する。
        @param[in] in_iterator 値の種別を取得する反復子。
        @return 反復子が指す値の種別。
     */
    public: this_type::kind get_kind(this_type::iterator const in_iterator)
    const
    {
        return this_type::make_kind(this->get_format(in_iterator));
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

    /** @brief 反復子が指す値の、格納形式を取得する。
        @param[in] in_iterator 格納形式を取得する反復子。
        @return in_iterator が指す値の格納形式。
     */
    private: unsigned get_format(this_type::iterator const in_iterator) const
    {
        auto const local_unit(this->get_word(in_iterator));
        return local_unit != nullptr?
            this_type::get_format(*local_unit): this_type::kind_NIL;
    }

    /** @brief タグが指す値の、格納形式を取得する。
        @param[in] in_tag 格納形式を取得するタグ。
        @return in_tag が指す値の格納形式。
     */
    private: static unsigned get_format(this_type::word const in_tag)
    {
        return in_tag >> this_type::TAG_FORMAT_BITS_POSITION;
    }

    private: this_type::word get_tag(
        this_type::iterator const in_iterator)
    const
    {
        auto const local_unit(this->get_word(in_iterator));
        return local_unit != nullptr? *local_unit: 0;
    }

    private: template<typename template_body>
    template_body const* get_body(
        this_type::iterator const in_iterator,
        unsigned const in_format)
    const
    {
        auto const local_tag(this->get_tag(in_iterator));
        if (this_type::get_format(local_tag) != in_format)
        {
            return nullptr;
        }
        auto const local_body(
            this->get_word(
                local_tag & this_type::TAG_IMMEDIATE_BITS_MASK));
        PSYQ_ASSERT(local_body != nullptr);
        return reinterpret_cast<template_body const*>(local_body);
    }

    //-------------------------------------------------------------------------
    /** @brief 32ビット整数のハッシュ値を算出する。
        @param[in] in_value ハッシュ値を求める32ビット整数。
        @return 32ビット整数のハッシュ値。
     */
    private: static std::uint32_t make_hash(std::uint32_t const in_value)
    {
        // 32ビット整数をそのままハッシュ値として使う。
        return in_value;
    }

    /** @brief 64ビット整数のハッシュ値を算出する。
        @param[in] in_value ハッシュ値を求める64ビット整数。
        @return 64ビット整数のハッシュ値。
     */
    private: static std::uint32_t make_hash(std::uint64_t const in_value)
    {
        return static_cast<std::uint32_t>(in_value)
            ^  static_cast<std::uint32_t>(in_value >> 32);
    }

    /** @brief バイト列のハッシュ値を算出する。
        @param[in] in_raw_data バイト列の先頭位置。
        @param[in] in_raw_size バイト列のバイト数。
        @return バイト列のハッシュ値。
     */
    private: static std::uint32_t make_hash(
        void const* const in_raw_data,
        std::size_t const in_raw_size)
    {
        // 32ビットのFNV-1でハッシュ化する。
        std::uint32_t const FNV_OFFSET_BASIS_32(0x811c9dc5U);
        std::uint32_t const FNV_PRIME_32(0x1000193U);
        auto const local_begin(static_cast<std::uint8_t const*>(in_raw_data));
        auto const local_end(local_begin + in_raw_size);
        auto local_hash(FNV_OFFSET_BASIS_32);
        for (auto i(local_begin); i < local_end ; ++i)
        {
            local_hash = (FNV_PRIME_32 * local_hash) ^ *i;
        }
        return local_hash;
    }

    //-------------------------------------------------------------------------
    /// @name 数値
    //@{
    /** @brief 反復子が数値を指すか判定する。
        @param[in] in_iterator 判定する反復子。
        @retval true  in_iterator は数値を指している。
        @retval false in_iterator は数値を指していない。
     */
    public: bool is_numerics(this_type::iterator const in_iterator) const
    {
        return this_type::kind_UNSIGNED <= this->get_format(in_iterator);
    }

    /** @brief 反復子が指す数値を取得する。
        @param[in] in_iterator 数値を指す反復子。
        @param[in] in_default  数値の取得に失敗した場合に返す値。
        @return
            in_iterator が指す値を template_numerics にキャストした値。
            ただし in_iterator が数値を指してない場合は、 in_default を返す。
     */
    public: template<typename template_numerics>
    template_numerics get_numerics(
        this_type::iterator const in_iterator,
        template_numerics const in_default)
    const
    {
        template_numerics local_numerics;
        return this->read_numerics(in_iterator, local_numerics) < 0?
            in_default: local_numerics;
    }

    /** @brief 反復子が指す数値を取得する。
        @param[in]  in_iterator  数値を指す反復子。
        @param[out] out_numerics in_iterator から取得した数値が代入される。
        @retval 正 in_iterator が指す値の等値を out_numerics へ代入。
        @retval 0  in_iterator が指す値を template_numerics にキャストして out_numerics へ代入。
        @retval 負 失敗。 in_iterator は数値を指してない。 out_numerics は変わらない。
     */
    public: template<typename template_numerics>
    int read_numerics(
        this_type::iterator const in_iterator,
        template_numerics& out_numerics)
    const
    {
        auto const local_tag(this->get_tag(in_iterator));
        switch (this_type::get_format(local_tag))
        {
        case this_type::numerics_UNSIGNED_IMMEDIATE:
            return this_type::read_immediate_numerics<template_numerics, std::make_unsigned<this_type::word>::type>(
                local_tag, out_numerics);
        case this_type::numerics_UNSIGNED_32:
            return this->read_body_numerics<template_numerics, std::uint32_t>(
                local_tag, out_numerics);
        case this_type::numerics_UNSIGNED_64:
            return this->read_body_numerics<template_numerics, std::uint64_t>(
                local_tag, out_numerics);
        case this_type::numerics_NEGATIVE_IMMEDIATE:
            return this_type::read_immediate_numerics<template_numerics, std::make_signed<this_type::word>::type>(
                local_tag, out_numerics);
        case this_type::numerics_NEGATIVE_32:
            return this->read_body_numerics<template_numerics, std::int32_t>(
                local_tag, out_numerics);
        case this_type::numerics_NEGATIVE_64:
            return this->read_body_numerics<template_numerics, std::int64_t>(
                local_tag, out_numerics);
        case this_type::numerics_FLOATING_32:
            /** @todo
                格納されている数値がNANやINFだった場合に、
                template_numerics型へのキャストはどうする？
             */
            return this->read_body_numerics<template_numerics, float>(
                local_tag, out_numerics);
        case this_type::numerics_FLOATING_64:
            /** @todo
                格納されている数値がNANやINFだった場合に、
                template_numerics型へのキャストはどうする？
             */
            return this->read_body_numerics<template_numerics, double>(
                local_tag, out_numerics);
        default:
            return -1;
        }
    }
    //@}
    private: template<typename template_write, typename template_read>
    static bool read_immediate_numerics(
        this_type::word const in_tag,
        template_write& out_numerics)
    {
        static_assert(std::is_integral<template_read>::value, "");
        auto const local_immediate(
            in_tag & this_type::TAG_IMMEDIATE_BITS_MASK);
        auto const local_sign(
            (this_type::TAG_FORMAT_BITS_MAX << this_type::TAG_FORMAT_BITS_POSITION)
            * std::is_signed<template_read>::value);
        return this_type::read_argument_numerics(
            out_numerics,
            static_cast<template_read>(local_immediate | local_sign));
    }

    private: template<typename template_write, typename template_read>
    bool read_body_numerics(
        this_type::word const in_tag,
        template_write& out_numerics)
    const
    {
        auto const local_body(
            this->get_word(in_tag & this_type::TAG_IMMEDIATE_BITS_MASK));
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
    /// @name 真偽値
    //@{
    /** @brief 反復子が指す真偽値を取得する。
        @param[in] in_iterator この反復子から値を取得する。
        @param[in] in_default  真偽値の取得に失敗した場合に返す値。
        @return
            in_iterator が指す真偽値。
            ただし in_iterator が真偽値を指してない場合は、 in_default を返す。
     */
    public: bool get_boolean(
        this_type::iterator const in_iterator,
        bool const in_default)
    const
    {
        auto const local_state(this->get_boolean_state(in_iterator));
        return local_state < 0? in_default: (0 < local_state);
    }

    /** @brief 反復子が指す真偽値の状態を取得する。
        @param[in] in_iterator この反復子から値を取得する。
        @retval 正 in_iterator はtrueを指している。
        @retval 0  in_iterator はfalseを指している。
        @retval 負 in_iterator は真偽値を指してない。
     */
    public: int get_boolean_state(this_type::iterator const in_iterator) const
    {
        auto const local_tag(this->get_tag(in_iterator));
        return this_type::get_format(local_tag) == this_type::kind_BOOLEAN?
            (local_tag & this_type::TAG_IMMEDIATE_BITS_MASK): -1;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列
    //@{
    /** @brief 反復子が指す文字列の、先頭位置を取得する。
        @param[in] in_iterator この反復子から値を取得する。
        @retval !=nullptr
            反復子が指す文字列の先頭位置。
            反復子が文字列を指す場合は、必ずnullptr以外となる。
        @retval ==nullptr 反復子が文字列を指してない。
     */
    public: char const* get_string_data(
        this_type::iterator const in_iterator)
    const
    {
        auto const local_string(this->get_string_header(in_iterator));
        return local_string != nullptr?
            reinterpret_cast<char const*>(local_string + 1): nullptr;
    }

    /** @brief 反復子が指す文字列の、バイト数を取得する。
        @param[in] in_iterator この反復子から値を取得する。
        @return
            反復子が指す文字列のバイト数。
            ただし、反復子が文字列を指してない場合は、0を返す。
     */
    public: std::size_t get_string_size(
        this_type::iterator const in_iterator)
    const
    {
        auto const local_string(this->get_string_header(in_iterator));
        return local_string != nullptr? local_string->size: 0;
    }
    //@}
    private: this_type::string_header const* get_string_header(
        this_type::iterator const in_iterator)
    const
    {
        return this->get_body<this_type::string_header>(
            in_iterator, this_type::kind_STRING);
    }

    //-------------------------------------------------------------------------
    /// @name 拡張型バイト列
    //@{
    /** @brief 反復子が指す拡張型バイト列の、先頭位置を取得する。
        @param[in] in_iterator この反復子から値を取得する。
        @retval !=nullptr
            反復子が指す拡張型バイト列の先頭位置。
            反復子が拡張型バイト列を指す場合は、必ずnullptr以外となる。
        @retval ==nullptr 反復子が拡張型バイト列を指してない。
     */
    public: void const* get_extended_data(
        this_type::iterator const in_iterator)
    const
    {
        auto const local_extended(this->get_extended_header(in_iterator));
        return local_extended != nullptr?
            reinterpret_cast<char const*>(local_extended + 1): nullptr;
    }

    /** @brief 反復子が指す拡張型バイト列の、バイト数を取得する。
        @param[in] in_iterator この反復子から値を取得する。
        @return
            反復子が指す拡張型バイト列のバイト数。
            ただし、反復子が拡張型バイト列を指してない場合は、0を返す。
     */
    public: std::size_t get_extended_size(
        this_type::iterator const in_iterator)
    const
    {
        auto const local_extended(this->get_extended_header(in_iterator));
        return local_extended != nullptr? local_extended->size: 0;
    }

    /** @brief 反復子が指す拡張型バイト列の、種別を取得する。
        @param[in] in_iterator この反復子から値を取得する。
        @return
            反復子が指す拡張型バイト列の種別。
            ただし、反復子が拡張型バイト列を指してない場合は、0を返す。
     */
    public: this_type::word get_extended_kind(
        this_type::iterator const in_iterator)
    const
    {
        auto const local_extended(this->get_extended_header(in_iterator));
        return local_extended != nullptr? local_extended->kind: 0;
    }
    //@}
    private: this_type::extended_header const* get_extended_header(
        this_type::iterator const in_iterator)
    const
    {
        return this->get_body<this_type::extended_header>(
            in_iterator, this_type::kind_EXTENDED);
    }

    //-------------------------------------------------------------------------
    /// @name コンテナ
    //@{
    /** @brief 反復子がコンテナを指すか判定する。
        @param[in] in_iterator この反復子から値を取得する。
        @retval true  in_iterator はコンテナを指している。
        @retval false in_iterator はコンテナを指していない。
     */
    public: bool is_container(this_type::iterator const in_iterator) const
    {
        switch (this->get_format(in_iterator))
        {
        case this_type::kind_ARRAY:
        case this_type::kind_MAP:
            return true;
        default:
            return false;
        }
    }

    /** @brief コンテナを指す反復子から、コンテナが持つ値の数を取得する。
        @param[in] in_container コンテナを指す反復子。
        @return
            in_container が指すコンテナが持つ値の数。
            ただし in_container がコンテナを指していない場合は、0を返す。
     */
    public: std::size_t get_container_size(
        this_type::iterator const in_container)
    const
    {
        auto const local_container(this->get_container_header(in_container));
        return local_container != nullptr? local_container->size: 0;
    }

    /** @brief コンテナが持つ値を指す反復子から、インデックス番号を取得する。
        @param[in] in_container 基準となるコンテナを指す反復子。
        @param[in] in_element   コンテナが持つ値を指す反復子。
        @retval !=CONTAINER_INDEX_NONE 値のインデックス番号。
        @retval ==CONTAINER_INDEX_NONE
            失敗。 in_container がコンテナではないか、
            in_element は in_container が持つ値ではない。
     */
    public: std::size_t get_container_index(
        this_type::iterator const in_container,
        this_type::iterator const in_element)
    const
    {
        auto const local_container(this->get_container_header(in_container));
        if (local_container != nullptr)
        {
            auto const local_begin(
                reinterpret_cast<this_type::word const*>(local_container + 1));
            auto const local_end(local_begin + local_container->size);
            if (local_begin <= in_element && in_element < local_end)
            {
                return static_cast<this_type::word const*>(in_element)
                    - local_begin;
            }
        }
        return this_type::CONTAINER_INDEX_NONE;
    }

    /** @brief コンテナを指す反復子から、コンテナの範囲を取得する。
        @param[in] in_container コンテナを指す反復子。
        @param[in] in_front_offset
            スライスする範囲の開始インデックス番号。
            0以上の場合はコンテナの先頭位置を基準とし、
            0未満の場合はコンテナの終端位置を基準とする。
        @param[in] in_back_offset
            スライスする範囲の終了インデックス番号。
            0より大きい場合はコンテナの先頭位置を基準とし、
            0以下の場合はコンテナの終端位置を基準とする。
        @return コンテナの範囲。
     */
    public: this_type::container_slice get_container_slice(
        this_type::iterator const in_container,
        int const in_front_offset = 0,
        int const in_back_offset = 0)
    const
    {
        auto const local_container(this->get_container_header(in_container));
        if (local_container != nullptr)
        {
            auto local_begin(
                reinterpret_cast<this_type::word const*>(local_container + 1));
            auto local_end(local_begin + in_back_offset);
            local_begin += in_front_offset;
            if (in_front_offset < 0)
            {
                local_begin += local_container->size;
            }
            if (in_back_offset <= 0)
            {
                local_end += local_container->size;
            }
            if (local_begin < local_end)
            {
                return this_type::container_slice(local_begin, local_end);
            }
        }
        return this_type::container_slice(nullptr, nullptr);
    }

    /** @brief コンテナを指す反復子から、コンテナが持つ値への反復子を取得する。
        @param[in] in_container コンテナを指す反復子。
        @param[in] in_index     取得する値のインデックス番号。
        @retval !=nullptr コンテナが持つ値への反復子。
        @retval ==nullptr
            失敗。 in_container がコンテナを指してないか、
            in_index はコンテナの範囲外。
     */
    public: this_type::iterator get_container_iterator(
        this_type::iterator const in_container,
        std::size_t const in_index)
    const
    {
        auto const local_tag(this->get_tag(in_container));
        switch (this_type::get_format(local_tag))
        {
        case this_type::kind_ARRAY:
        case this_type::kind_MAP:
            return this->get_container_node(local_tag, in_index);
        default:
            return nullptr;
        }
    }

    /** @brief コンテナの範囲から、コンテナが持つ値への反復子を取得する。
        @param[in] in_slice コンテナの範囲。
        @param[in] in_index 取得する値のインデックス番号。
        @retval !=nullptr コンテナが持つ値への反復子。
        @retval ==nullptr 失敗。 in_index はコンテナの範囲外。
     */
    public: this_type::iterator get_container_iterator(
        this_type::container_slice const& in_slice,
        std::size_t const in_index)
    const
    {
        if (in_slice.first < in_slice.second)
        {
            auto const local_begin(
                static_cast<this_type::word const*>(in_slice.first));
            auto const local_end(
                static_cast<this_type::word const*>(in_slice.second));
            if (in_index < static_cast<std::size_t>(local_end - local_begin))
            {
                return local_begin + in_index;
            }
        }
        return nullptr;
    }

    /** @brief コンテナが持つ値を指す反復子を進める。
        @param[in] in_slice    コンテナの範囲。
        @param[in] in_iterator コンテナが持つ値を指す反復子。
        @param[in] in_count    反復子を進める回数。負の場合は、戻す回数。
        @retval !=nullptr コンテナが持つ値への反復子。
        @retval ==nullptr 失敗。値を指す反復子がコンテナの範囲外となった。
     */
    public: this_type::iterator advance_container_iterator(
        this_type::container_slice const& in_slice,
        this_type::iterator const in_iterator,
        int const in_count)
    const
    {
        if (in_slice.first < in_slice.second)
        {
            auto const local_begin(
                static_cast<this_type::word const*>(in_slice.first));
            auto const local_end(
                static_cast<this_type::word const*>(in_slice.second));
            auto const local_iterator(
                static_cast<this_type::word const*>(in_iterator) + in_count);
            if (local_begin <= local_iterator && local_iterator < local_end)
            {
                return local_iterator;
            }
        }
        return nullptr;
    }
    //@}
    /** @brief 値タグが指すコンテナが持つ値への反復子を取得する。
        @param[in] in_tag   コンテナを指す値タグ。
        @param[in] in_index インデックス番号。
        @retval !=nullptr コンテナが持つへの反復子。
        @retval ==nullptr
            失敗。 in_index は in_tag が指すコンテナの範囲外。
     */
    private: this_type::iterator get_container_node(
        this_type::word const in_tag,
        std::size_t const in_index)
    const
    {
        auto const& local_container(this->get_container_header(in_tag));
        if (local_container.size <= in_index)
        {
            return nullptr;
        }
        auto const local_begin(
            reinterpret_cast<this_type::word const*>(&local_container + 1));
        return local_begin + in_index;
    }

    private: this_type::container_header const* get_container_header(
        this_type::iterator const in_container)
    const
    {
        auto const local_tag(this->get_tag(in_container));
        switch (this_type::get_format(local_tag))
        {
        case this_type::kind_ARRAY:
        case this_type::kind_MAP:
            return &(this->get_container_header(local_tag));
        default:
            return nullptr;
        }
    }

    private: this_type::container_header const& get_container_header(
        this_type::word const in_tag)
    const
    {
        PSYQ_ASSERT(
            this_type::get_format(in_tag) == this_type::kind_ARRAY
            || this_type::get_format(in_tag) == this_type::kind_MAP);
        auto const local_container(
            reinterpret_cast<this_type::container_header const*>(
                this->get_word(in_tag & this_type::TAG_IMMEDIATE_BITS_MASK)));
        PSYQ_ASSERT(local_container != nullptr);
        return *local_container;
    }

    //-------------------------------------------------------------------------
    /// @name 辞書
    //@{
    /** @brief 辞書を指す反復子から、辞書の値に対応するキーへの反復子を取得する。
        @param[in] in_map   辞書を指す反復子。
        @param[in] in_index 値のインデックス番号。
        @retval !=nullptr 辞書の値に対応するキーへの反復子。
        @retval ==nullptr
            失敗。 in_map が辞書を指してないか、
            in_index に対応する値が辞書に存在しない。
     */
    public: this_type::iterator get_map_key(
        this_type::iterator const in_map,
        std::size_t const in_index)
    const
    {
        auto const local_tag(this->get_tag(in_map));
        return this_type::get_format(local_tag) == this_type::kind_MAP?
            this->get_container_node(
                this_type::make_key_container_tag(
                    local_tag, this->get_container_header(local_tag)),
                in_index):
            nullptr;
    }

    /** @brief 辞書の値を指す反復子から、対応するキーを指す反復子を取得する。
        @param[in] in_map   辞書を指す反復子。
        @param[in] in_value 辞書の値を指す反復子。
        @retval !=nullptr in_value に対応するキーを指す反復子。
        @retval ==nullptr 失敗。 in_value は、辞書の値を指していない。
     */
    public: this_type::iterator get_map_key(
        this_type::iterator const in_map,
        this_type::iterator const in_value)
    const
    {
        auto const local_tag(this->get_tag(in_map));
        if (this_type::get_format(local_tag) == this_type::kind_MAP)
        {
            auto const& local_container(this->get_container_header(local_tag));
            auto const local_begin(
                reinterpret_cast<this_type::word const*>(
                    &local_container + 1));
            if (local_begin <= in_value)
            {
                return this->get_container_node(
                    this_type::make_key_container_tag(local_tag, local_container),
                    static_cast<this_type::word const*>(in_value) - local_begin);
            }
        }
        return nullptr;
    }

    /** @brief 辞書を指す反復子から、辞書が持つ値への反復子を取得する。
        @param[in] in_map 辞書を指す反復子。
        @param[in] in_key 値に対応するキー。
        @retval !=nullptr 辞書が持つ値への反復子。
        @retval ==nullptr
            失敗。 in_map が辞書を指してないか、
            in_key に対応する値が辞書に存在しない。
     */
    public: this_type::iterator get_map_value(
        this_type::iterator const in_map,
        this_type::map_key const& in_key)
    const
    {
        auto const local_tag(this->get_tag(in_map));
        if (this_type::get_format(local_tag) == this_type::kind_MAP)
        {
            // 辞書からキーコンテナを取得する。
            auto const& local_value_container(
                this->get_container_header(local_tag));
            auto const& local_key_container(
                this->get_container_header(
                    this_type::make_key_container_tag(
                        local_tag, local_value_container)));

            // バイナリ検索で、最も近いキーを見つける。
            auto const local_key_begin(
                reinterpret_cast<this_type::word const*>(
                    &local_key_container + 1));
            auto const local_key_end(
                local_key_begin + local_key_container.size);
            auto const local_key_position(
                std::lower_bound(
                    local_key_begin,
                    local_key_end,
                    in_key,
                    this_type::less_map(*this)));

            // 線形検索で、一致するキーを見つける。
            for (auto i(local_key_position); i < local_key_end; ++i)
            {
                auto const local_compare(this->compare_value(in_key, *i));
                if (0 <= local_compare)
                {
                    if (0 < local_compare)
                    {
                        break;
                    }
                    // キーのインデックス番号から、値を特定する。
                    auto const local_value_begin(
                        reinterpret_cast<this_type::word const*>(
                            &local_value_container + 1));
                    return local_value_begin + (i - local_key_begin);
                }
            }
        }
        return nullptr;
    }

    /** @brief 辞書を指す反復子から、辞書が持つ値への反復子を取得する。
        @param[in] in_map 辞書を指す反復子。
        @param[in] in_key
            値に対応するキーの反復子。*thisが持つ値を指していること。
        @retval !=nullptr 辞書が持つ値への反復子。
        @retval ==nullptr
            失敗。 in_map が辞書を指してないか、
            in_key に対応する値が辞書に存在しない。
     */
    public: this_type::iterator get_map_value(
        this_type::iterator const in_map,
        this_type::iterator const in_key)
    const
    {
        return this->get_map_value(in_map, in_key, *this);
    }

    /** @brief 辞書を指す反復子から、辞書が持つ値への反復子を取得する。
        @param[in] in_map 辞書を指す反復子。
        @param[in] in_key_iterator
            値に対応するキーの反復子。
            in_key_archive が持つ値を指していること。
        @param[in] in_key_archive キーとなる反復子が指す書庫。
        @retval !=nullptr 辞書が持つ値への反復子。
        @retval ==nullptr
            失敗。 in_map が辞書を指してないか、
            in_key_iterator に対応する値が辞書に存在しない。
     */
    public: this_type::iterator get_map_value(
        this_type::iterator const in_map,
        this_type::iterator const in_key_iterator,
        this_type const& in_key_archive)
    const
    {
        // キーを指す反復子から取り出した値を初期値として構築した
        // this_type::map_key で、辞書を検索する。
        auto const local_key_format(
            in_key_archive.get_format(in_key_iterator));
        switch (local_key_format)
        {
        case this_type::kind_BOOLEAN:
        {
            auto const local_boolean_state(
                in_key_archive.get_boolean_state(in_key_iterator));
            if (local_boolean_state < 0)
            {
                break;
            }
            return this->get_map_value(
                in_map, this_type::map_key(0 < local_boolean_state));
        }
        case this_type::numerics_UNSIGNED_IMMEDIATE:
        case this_type::numerics_UNSIGNED_32:
        case this_type::numerics_UNSIGNED_64:
            return this->get_numerics_map_value<std::uint64_t>(
                in_map, in_key_iterator, in_key_archive);
        case this_type::numerics_NEGATIVE_IMMEDIATE:
        case this_type::numerics_NEGATIVE_32:
        case this_type::numerics_NEGATIVE_64:
            return this->get_numerics_map_value<std::int64_t>(
                in_map, in_key_iterator, in_key_archive);
        case this_type::numerics_FLOATING_32:
        case this_type::numerics_FLOATING_64:
            return this->get_numerics_map_value<double>(
                in_map, in_key_iterator, in_key_archive);
        case this_type::kind_STRING:
        {
            auto const local_string_data(
                in_key_archive.get_string_data(in_key_iterator));
            PSYQ_ASSERT(local_string_data != nullptr);
            return this->get_map_value(
                in_map,
                this_type::map_key(
                    local_string_data,
                    in_key_archive.get_string_size(in_key_iterator)));
        }
        case this_type::kind_EXTENDED:
        {
            auto const local_extended_data(
                in_key_archive.get_extended_data(in_key_iterator));
            PSYQ_ASSERT(local_extended_data != nullptr);
            return this->get_map_value(
                in_map,
                this_type::map_key(
                    local_extended_data,
                    in_key_archive.get_extended_size(in_key_iterator),
                    in_key_archive.get_extended_kind(in_key_iterator)));
        }
        case this_type::kind_ARRAY:
        case this_type::kind_MAP:
            /// @note コンテナをキーとする検索は未実装。
            break;
        default:
            break;
        }
        return nullptr;
    }
    //@}
    private: template<typename template_numerics>
    this_type::iterator get_numerics_map_value(
        this_type::iterator const in_map,
        this_type::iterator const in_value_iterator,
        this_type const& in_value_archive)
    const
    {
        template_numerics local_numerics;
        return in_value_archive.read_numerics(in_value_iterator, local_numerics)?
            this->get_map_value(in_map, this_type::map_key(local_numerics)):
            nullptr;
    }

    private: static this_type::word make_key_container_tag(
        this_type::word const in_value_container_tag,
        this_type::container_header const& in_value_container_header)
    {
        return in_value_container_tag + in_value_container_header.size
            + sizeof(this_type::container_header) / sizeof(this_type::word);
    }

    //-------------------------------------------------------------------------
    /** @brief 辞書検索キーと値タグの指す値を比較する。
        @param[in] in_left_key  左辺となる辞書検索キー。
        @param[in] in_right_tag 右辺となる値を指すタグ値。
        @retval 正 辞書検索キーとノードは、ハッシュ値が異なる。
        @retval 0  辞書検索キーとノードが指す値は等値。
        @retval 負 辞書検索キーとノードは、ハッシュ値は等値だが、値が異なる。
     */
    private: int compare_value(
        this_type::map_key const& in_left_key,
        this_type::word const in_right_tag)
    const
    {
        auto const local_right_format(this_type::get_format(in_right_tag));
        if (local_right_format != in_left_key.format_)
        {
            return 1;
        }
        auto const local_right_immediate(
            in_right_tag & this_type::TAG_IMMEDIATE_BITS_MASK);
        switch (local_right_format)
        {
        case this_type::kind_BOOLEAN:
        case this_type::numerics_UNSIGNED_IMMEDIATE:
            return local_right_immediate != in_left_key.bits_32_[0];
        case this_type::numerics_NEGATIVE_IMMEDIATE:
        {
            auto const local_value(
                local_right_immediate | (
                    this_type::TAG_FORMAT_BITS_MAX
                    << this_type::TAG_FORMAT_BITS_POSITION));
            return local_value != in_left_key.bits_32_[0];
        }
        case this_type::numerics_UNSIGNED_32:
        case this_type::numerics_NEGATIVE_32:
        case this_type::numerics_FLOATING_32:
        {
            auto const local_body(this->get_word(local_right_immediate));
            if (local_body == nullptr)
            {
                break;
            }
            return *local_body != in_left_key.bits_32_[0];
        }
        case this_type::numerics_UNSIGNED_64:
        case this_type::numerics_NEGATIVE_64:
        case this_type::numerics_FLOATING_64:
        {
            auto const local_body(
                reinterpret_cast<std::uint64_t const*>(
                    this->get_word(local_right_immediate)));
            if (local_body == nullptr)
            {
                break;
            }
            if (in_left_key.hash_ != this_type::make_hash(*local_body))
            {
                return 1;
            }
            return 0 - (*local_body != in_left_key.bits_64_);
        }
        case this_type::kind_STRING:
        case this_type::kind_EXTENDED:
        {
            auto const local_raw(
                reinterpret_cast<this_type::extended_header const*>(
                    this->get_word(local_right_immediate)));
            if (local_raw == nullptr)
            {
                break;
            }
            if (in_left_key.hash_ != local_raw->hash)
            {
                return 1;
            }
            if (in_left_key.raw_.size != local_raw->size)
            {
                return -1;
            }
            void const* local_raw_data;
            if (local_right_format == this_type::kind_STRING)
            {
                local_raw_data = static_cast<this_type::string_header const*>(local_raw) + 1;
            }
            else if (in_left_key.raw_.kind == local_raw->kind)
            {
                local_raw_data = local_raw + 1;
            }
            else
            {
                return -1;
            }
            /** @note
                バイト列が完全に一致するかの確認は、
                高速化のため、assertで確認する。
             */
            PSYQ_ASSERT(std::memcmp(local_raw_data, in_left_key.raw_.data, in_left_key.raw_.size) == 0);
            return 0;
        }
        case this_type::kind_ARRAY:
        case this_type::kind_MAP:
        default:
            /// @note コンテナの比較は未実装。
            PSYQ_ASSERT(false);
            break;
        }
        return 1;
    }

    /** @brief 辞書検索キーと値タグの指す値のハッシュを比較する。
        @param[in] in_left_key  左辺となる辞書検索キー。
        @param[in] in_right_tag 右辺となる値を指す値タグ。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺の値ハッシュは同じ。
        @retval 負 左辺のほうが小さい。
     */
    private: int compare_hash(
        this_type::map_key const& in_left_key,
        this_type::word const in_right_tag)
    const
    {
        auto const local_left_hash(in_left_key.hash_);
        auto const local_right_hash(this->get_hash(in_right_tag));
        if (local_left_hash < local_right_hash)
        {
            return -1;
        }
        if (local_right_hash < local_left_hash)
        {
            return 1;
        }
        auto const local_left_format(in_left_key.format_);
        auto const local_right_format(this_type::get_format(in_right_tag));
        if (local_left_format < local_right_format)
        {
            return -1;
        }
        if (local_right_format < local_left_format)
        {
            return 1;
        }
        return 0;
    }

    /** @brief 値タグが指す値のハッシュ値を取得する。
        @param[in] in_tag 値タグ値。
        @return 値タグが指す値のハッシュ値。
     */
    private: this_type::word get_hash(this_type::word const in_tag) const
    {
        auto const local_format(
            in_tag >> this_type::TAG_FORMAT_BITS_POSITION);
        auto const local_immediate(
            in_tag & this_type::TAG_IMMEDIATE_BITS_MASK);
        switch (local_format)
        {
        case this_type::kind_BOOLEAN:
        case this_type::numerics_UNSIGNED_IMMEDIATE:
            return local_immediate;
        case this_type::numerics_NEGATIVE_IMMEDIATE:
            return local_immediate | (
                this_type::TAG_FORMAT_BITS_MAX
                << this_type::TAG_FORMAT_BITS_POSITION);
        case this_type::numerics_UNSIGNED_32:
        case this_type::numerics_NEGATIVE_32:
        case this_type::numerics_FLOATING_32:
        {
            auto const local_body(this->get_word(local_immediate));
            if (local_body == nullptr)
            {
                break;
            }
            return *local_body;
        }
        case this_type::kind_STRING:
        case this_type::kind_EXTENDED:
        {
            auto const local_header(
                reinterpret_cast<this_type::string_header const*>(
                    this->get_word(local_immediate)));
            if (local_header == nullptr)
            {
                break;
            }
            return local_header->hash;
        }
        case this_type::numerics_UNSIGNED_64:
        case this_type::numerics_NEGATIVE_64:
        case this_type::numerics_FLOATING_64:
        {
            auto const local_body(
                reinterpret_cast<std::uint64_t const*>(
                    this->get_word(local_immediate)));
            if (local_body == nullptr)
            {
                break;
            }
            return this_type::make_hash(*local_body);
        }
        case this_type::kind_ARRAY:
        case this_type::kind_MAP:
            break;
        default:
            PSYQ_ASSERT(false);
            break;
        }
        return 0xffffffff;
    }

    //-------------------------------------------------------------------------
    private: this_type::word const* unit_front_;
    private: std::size_t unit_count_;

}; // class psyq::binarc::archive

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class binarc_to_block_yaml
{
    /// thisが指す値の型。
    private: typedef binarc_to_block_yaml this_type;

    //-------------------------------------------------------------------------
    public: static void convert(std::string const& in_file_path)
    {
        // ファイルを開く。
        std::ifstream local_file_stream(
            in_file_path, std::ios::in | std::ios::binary);
        if (!local_file_stream)
        {
            PSYQ_ASSERT(false);
            return;
        }

        // ファイルの大きさを取得する。
        local_file_stream.seekg(0, std::ios::end);
        auto const local_file_size(
            static_cast<std::size_t>(local_file_stream.tellg()));

        // ファイルを、std::vectorへ読み込む。
        std::vector<psyq::binarc::archive::word> local_file_buffer(
            (local_file_size + sizeof(psyq::binarc::archive::word) - 1)
            / sizeof(psyq::binarc::archive::word));
        local_file_stream.seekg(0, std::ios::beg);
        local_file_stream.read(
            reinterpret_cast<char*>(&local_file_buffer.front()),
            local_file_size);

        // BINARCバイナリをYAML文字列に変換する。
        psyq::binarc::archive local_archive(
            &local_file_buffer.front(), local_file_size);
        std::ostringstream local_string_stream;
        convert(local_string_stream, local_archive);
        std::cout << local_string_stream.str();
    }

    //-------------------------------------------------------------------------
    public: static void convert(
        std::ostringstream& out_stream,
        psyq::binarc::archive const& in_archive)
    {
        auto local_iterator(in_archive.get_root());
        if (local_iterator != nullptr)
        {
            this_type::convert_node(out_stream, in_archive, local_iterator);
        }
    }

    //-------------------------------------------------------------------------
    private: static void convert_node(
        std::ostringstream& out_stream,
        psyq::binarc::archive const& in_archive,
        psyq::binarc::archive::iterator const in_iterator)
    {
        switch (in_archive.get_kind(in_iterator))
        {
        case psyq::binarc::archive::kind_UNSIGNED:
            out_stream << in_archive.get_numerics<std::uint64_t>(in_iterator, 0);
            break;
        case psyq::binarc::archive::kind_NEGATIVE:
            out_stream << in_archive.get_numerics<std::int64_t>(in_iterator, 0);
            break;
        case psyq::binarc::archive::kind_FLOATING:
            out_stream << in_archive.get_numerics<double>(in_iterator, 0);
            break;
        case psyq::binarc::archive::kind_STRING:
            this_type::convert_string(out_stream, in_archive, in_iterator);
            break;
        case psyq::binarc::archive::kind_ARRAY:
            this_type::convert_sequence(out_stream, in_archive, in_iterator);
            break;
        case psyq::binarc::archive::kind_MAP:
            this_type::convert_mapping(out_stream, in_archive, in_iterator);
            break;
        default:
            PSYQ_ASSERT(false);
            break;
        }
    }

    private: static void convert_string(
        std::ostringstream& out_stream,
        psyq::binarc::archive const& in_archive,
        psyq::binarc::archive::iterator const in_iterator)
    {
        auto const local_data(in_archive.get_string_data(in_iterator));
        assert(local_data != nullptr);
        out_stream << '\'';
        out_stream.write(local_data, in_archive.get_string_size(in_iterator));
        out_stream << '\'';
    }

    private: static void convert_sequence(
        std::ostringstream& out_stream,
        psyq::binarc::archive const& in_archive,
        psyq::binarc::archive::iterator const in_array)
    {
        out_stream << '[';
#if 1
        auto const local_range(in_archive.get_container_range(in_array));
        for (
            auto i(local_range.first);
            i != nullptr;
            i = in_archive.advance_container_iterator(local_range, i, 1))
        {
            if (i != local_range.first)
            {
                out_stream << ',';
            }
            this_type::convert_node(out_stream, in_archive, i);
        }
#else
        auto const local_size(in_archive.get_container_size(in_array));
        for (unsigned i(0); i < local_size; ++i)
        {
            if (0 < i)
            {
                out_stream << ',';
            }
            this_type::convert_node(
                out_stream,
                in_archive,
                in_archive.get_container_iterator(in_array, i));
        }
#endif // 1
        out_stream << ']';
    }

    private: static void convert_mapping(
        std::ostringstream& out_stream,
        psyq::binarc::archive const& in_archive,
        psyq::binarc::archive::iterator const in_map)
    {
        out_stream << '{';
#if 1
        auto const local_range(in_archive.get_container_range(in_map));
        for (
            auto i(local_range.first);
            i != nullptr;
            i = in_archive.advance_container_iterator(local_range, i, 1))
        {
            if (i != local_range.first)
            {
                out_stream << ',';
            }
            this_type::convert_node(
                out_stream, in_archive, in_archive.get_map_key(in_map, i));
            out_stream << ':';
            this_type::convert_node(out_stream, in_archive, i);
        }
#else
        auto const local_size(in_archive.get_container_size(in_map));
        for (unsigned i(0); i < local_size; ++i)
        {
            if (0 < i)
            {
                out_stream << ',';
            }
            auto const local_key_iterator(
                in_archive.get_map_key(in_map, i));
            this_type::convert_node(
                out_stream, in_archive, local_key_iterator);
            out_stream << ':';
            auto const local_value_iterator(
                in_archive.get_map_value(in_map, local_key_iterator));
                //in_archive.get_container_iterator(in_map, i));
            this_type::convert_node(
                out_stream, in_archive, local_value_iterator);
        }
#endif
        out_stream << '}';
    }

}; // class binarc_to_block_yaml


#endif // PSYQ_BINARC_HPP_
