/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copydoc psyq::internal::message_pack_value
 */
#ifndef PSYQ_MESSAGE_PACK_VALUE_HPP_
#define PSYQ_MESSAGE_PACK_VALUE_HPP_

//#include "psyq/message_pack_container.hpp"

/// 倍精度浮動小数点数で許容する誤差の最大値。
#ifndef PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON
#define PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON\
    (std::numeric_limits<self::float32>::epsilon() * 4)
#endif // !defined(PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON)

/// 単精度浮動小数点数で許容する誤差の最大値。
#ifndef PSYQ_MESSAGE_PACK_VALUE_FLOAT64_EPSILON
#define PSYQ_MESSAGE_PACK_VALUE_FLOAT64_EPSILON\
    (std::numeric_limits<self::float64>::epsilon() * 4)
#endif // !defined(PSYQ_MESSAGE_PACK_VALUE_FLOAT64_EPSILON)

namespace psyq
{
    namespace internal
    {
        /// @cond
        union message_pack_value;
        /// @endcond
    }
    namespace message_pack
    {
        /// @cond
        struct object;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief MessagePackオブジェクトの値。
    @sa psyq::message_pack::object
 */
union psyq::internal::message_pack_value
{
    private: typedef message_pack_value self; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    public: struct type
    {
        /** @brief MessagePackオブジェクトに格納する値の種別。
         */
        enum value: std::uint8_t
        {
            NIL,              ///< 空。
            BOOLEAN,          ///< 真偽値。
            POSITIVE_INTEGER, ///< 0以上の整数。
            NEGATIVE_INTEGER, ///< 0未満の整数。
            FLOAT32,          ///< 単精度浮動小数点数。
            FLOAT64,          ///< 倍精度浮動小数点数。
            STRING,           ///< 文字列。
            BINARY,           ///< バイナリ。
            EXTENDED_BINARY,  ///< 拡張バイナリ。
            ARRAY,            ///< MessagePack配列。
            MAP,              ///< MessagePack連想配列。
        };
    };

    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトに格納する値の型
    //@{
    /// @copydoc self::type::FLOAT32
    public: typedef float float32;
    /// @copydoc self::type::FLOAT64
    public: typedef double float64;
    /// 文字列を保持するRAWバイト列。
    public: typedef psyq::internal::message_pack_container<std::int8_t const>
        string;
    /// バイナリを保持するRAWバイト列。
    public: typedef psyq::internal::message_pack_extended_binary::super
        binary;
    /// 拡張バイナリを保持するRAWバイト列。
    public: typedef psyq::internal::message_pack_extended_binary
        extended_binary;
    /// @copydoc self::type::ARRAY
    public: typedef psyq::internal::message_pack_container<
        psyq::message_pack::object>
            array;
    /// @copydoc self::type::MAP
    public: typedef psyq::internal::message_pack_map<
        psyq::message_pack::object>
            map;
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクト値の構築
    //@{
    /// 空のMessagePackオブジェクト値を構築する。
    public: PSYQ_CONSTEXPR message_pack_value() PSYQ_NOEXCEPT {}

    /** @brief MessagePackオブジェクトに真偽値を格納する。
        @param[in] in_boolean MessagePackオブジェクトに格納する真偽値。
     */
    public: explicit PSYQ_CONSTEXPR message_pack_value(
        bool const in_boolean)
    PSYQ_NOEXCEPT:
        boolean_(in_boolean)
    {}

    /** @brief MessagePackオブジェクトに整数を格納する。
        @param[in] in_integer MessagePackオブジェクトに格納する整数。
     */
    public: explicit PSYQ_CONSTEXPR message_pack_value(
        std::int64_t const in_integer)
    PSYQ_NOEXCEPT:
        negative_integer_(in_integer)
    {}

    /** @brief MessagePackオブジェクトに浮動小数点数を格納する。
        @param[in] in_float MessagePackオブジェクトに格納する浮動小数点数。
     */
    public: explicit PSYQ_CONSTEXPR message_pack_value(
        self::float64 const in_float)
    PSYQ_NOEXCEPT:
        float64_(in_float)
    {}
    /// @copydoc message_pack_value(self::float64 const)
    public: explicit PSYQ_CONSTEXPR message_pack_value(
        self::float32 const in_float)
    PSYQ_NOEXCEPT:
        float32_(in_float)
    {}

    /** @brief MessagePackオブジェクトに文字列を格納する。
        @param[in] in_string MessagePackオブジェクトに格納する文字列。
     */
    public: explicit PSYQ_CONSTEXPR message_pack_value(
        self::string const& in_string)
    PSYQ_NOEXCEPT:
        string_(in_string)
    {}
    /** @brief MessagePackオブジェクトにバイナリを格納する。
        @param[in] in_string MessagePackオブジェクトに格納するバイナリ。
     */
    public: explicit PSYQ_CONSTEXPR message_pack_value(
        self::binary const& in_binary)
    PSYQ_NOEXCEPT:
        binary_(in_binary)
    {}
    /** @brief MessagePackオブジェクトに文字列を格納する。
        @param[in] in_string MessagePackオブジェクトに格納する文字列。
     */
    public: explicit PSYQ_CONSTEXPR message_pack_value(
        self::extended_binary const& in_extended_binary)
    PSYQ_NOEXCEPT:
        extended_binary_(in_extended_binary)
    {}

    /** @brief MessagePackオブジェクトに配列を格納する。
        @param[in] in_array MessagePackオブジェクトに格納する配列。
     */
    public: explicit PSYQ_CONSTEXPR message_pack_value(
        self::array const& in_array)
    PSYQ_NOEXCEPT:
        array_(in_array)
    {}

    /** @brief MessagePackオブジェクトに連想配列を格納する。
        @param[in] in_map MessagePackオブジェクトに格納する連想配列。
     */
    public: explicit PSYQ_CONSTEXPR message_pack_value(
        self::map const& in_map)
    PSYQ_NOEXCEPT:
        map_(in_map)
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクト値の比較
    //@{
    /** @brief MessagePackオブジェクト値が等値か比較する。
        @param[in] in_left_value  左辺のMessagePackオブジェクト値。
        @param[in] in_left_kind   左辺のMessagePackオブジェクト値の種別。
        @param[in] in_right_value 右辺のMessagePackオブジェクト値。
        @param[in] in_right_kind  右辺のMessagePackオブジェクト値の種別。
        @retval true  左辺と右辺は等値。
        @retval false 左辺と右辺は非等値。
     */
    public: static bool equal(
        self const& in_left_value,
        self::type::value const in_left_kind,
        self const& in_right_value,
        self::type::value const in_right_kind)
    PSYQ_NOEXCEPT
    {
        if (in_left_kind != in_right_kind)
        {
            return false;
        }
        switch (in_left_kind)
        {
        case self::type::NIL:
            return true;
        case self::type::BOOLEAN:
            return in_left_value.boolean_ == in_right_value.boolean_;
        case self::type::POSITIVE_INTEGER:
            return in_left_value.positive_integer_
                == in_right_value.positive_integer_;
        case self::type::NEGATIVE_INTEGER:
            return in_left_value.negative_integer_
                == in_right_value.negative_integer_;
        case self::type::FLOAT32:
            return 0 == self::compare_floating_point(
                in_left_value.float32_,
                in_right_value.float32_,
                PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON);
        case self::type::FLOAT64:
            return 0 == self::compare_floating_point(
                in_left_value.float64_,
                in_right_value.float64_,
                PSYQ_MESSAGE_PACK_VALUE_FLOAT64_EPSILON);
        case self::type::STRING:
            return in_left_value.string_ == in_right_value.string_;
        case self::type::BINARY:
            return in_left_value.binary_ == in_right_value.binary_;
        case self::type::EXTENDED_BINARY:
            return in_left_value.extended_binary_
                == in_right_value.extended_binary_;
        case self::type::ARRAY:
            return in_left_value.array_ == in_right_value.array_;
        case self::type::MAP:
            return in_left_value.map_ == in_right_value.map_;
        default:
            PSYQ_ASSERT(false);
            return false;
        }
    }

    /** @brief MessagePackオブジェクト値を比較する。
        @param[in] in_left_value  左辺のMessagePackオブジェクト値。
        @param[in] in_left_kind   左辺のMessagePackオブジェクト値の種別。
        @param[in] in_right_value 右辺のMessagePackオブジェクト値。
        @param[in] in_right_kind  右辺のMessagePackオブジェクト値の種別。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    public: static int compare(
        self const& in_left_value,
        self::type::value const in_left_kind,
        self const& in_right_value,
        self::type::value const in_right_kind)
    PSYQ_NOEXCEPT
    {
        switch (in_right_kind)
        {
        case self::type::NIL:
            return in_left_kind != self::type::NIL? 1: 0;
        case self::type::BOOLEAN:
            return self::compare_boolean(
                in_left_value, in_left_kind, in_right_value.boolean_);
        case self::type::POSITIVE_INTEGER:
            return self::compare_unsigned_integer(
                in_left_value, in_left_kind, in_right_value.positive_integer_);
        case self::type::NEGATIVE_INTEGER:
            return self::compare_signed_integer(
                in_left_value, in_left_kind, in_right_value.negative_integer_);
        case self::type::FLOAT32:
            return self::compare_floating_point(
                in_left_value, in_left_kind, in_right_value.float32_);
        case self::type::FLOAT64:
            return self::compare_floating_point(
                in_left_value, in_left_kind, in_right_value.float64_);
        case self::type::STRING:
        case self::type::BINARY:
        case self::type::EXTENDED_BINARY:
            return self::compare_raw(
                in_left_value, in_left_kind, in_right_value, in_right_kind);
        case self::type::ARRAY:
            return self::compare_array(
                in_left_value, in_left_kind, in_right_value.array_);
        case self::type::MAP:
            return self::compare_map(
                in_left_value, in_left_kind, in_right_value.map_);
        default:
            PSYQ_ASSERT(false);
            return self::is_valid_kind(in_left_kind)? -1: 0;
        }
    }
    //@}

    /** @brief 正規の種別か判定する。
        @param[in] in_kind 判定する種別。
        @retval true  正規の種別だった。
        @retval false 不正な種別だった。
     */
    private: static bool is_valid_kind(self::type::value const in_kind)
    {
        return in_kind <= self::type::MAP;
    }
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトコンテナとの比較
    //@{
    /** @brief MessagePackオブジェクト値と連想配列を比較する。
        @param[in] in_left_value 左辺のMessagePackオブジェクト値。
        @param[in] in_left_kind  左辺のMessagePackオブジェクト値の種別。
        @param[in] in_right_map  右辺のMessagePackオブジェクト連想配列。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: static int compare_map(
        self const& in_left_value,
        self::type::value const in_left_kind,
        self::map const& in_right_map)
    PSYQ_NOEXCEPT
    {
        switch (in_left_kind)
        {
        case self::type::NIL:
            return 1;
        case self::type::BOOLEAN:
            return -1;//-self::compare_map(in_right_map, this->boolean_);
        case self::type::POSITIVE_INTEGER:
            return -1;//-self::compare_map(in_right_map, this->positive_integer_);
        case self::type::NEGATIVE_INTEGER:
            return -1;//-self::compare_map(in_right_map, this->negative_integer_);
        case self::type::FLOAT32:
            return -1;//-self::compare_map(in_right_map, this->float32_);
        case self::type::FLOAT64:
            return -1;//-self::compare_map(in_right_map, this->float64_);
        case self::type::STRING:
        case self::type::BINARY:
        case self::type::EXTENDED_BINARY:
            return -1;//-self::compare_map(in_right_map, this->raw_);
        case self::type::ARRAY:
            return -1;//-self::compare_map(in_right_map, this->array_);
        case self::type::MAP:
            return in_left_value.map_.compare(in_right_map);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief MessagePackオブジェクト値と配列を比較する。
        @param[in] in_left_value  左辺のMessagePackオブジェクト値。
        @param[in] in_left_kind   左辺のMessagePackオブジェクト値の種別。
        @param[in] in_right_array 右辺のMessagePackオブジェクト配列。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: static int compare_array(
        self const& in_left_value,
        self::type::value const in_left_kind,
        self::array const& in_right_array)
    PSYQ_NOEXCEPT
    {
        switch (in_left_kind)
        {
        case self::type::NIL:
            return 1;
        case self::type::BOOLEAN:
            return -1;//-self::compare_array(in_right_array, in_left_value.boolean_);
        case self::type::POSITIVE_INTEGER:
            return -1;//-self::compare_array(in_right_array, in_left_value.positive_integer_);
        case self::type::NEGATIVE_INTEGER:
            return -1;//-self::compare_array(in_right_array, in_left_value.negative_integer_);
        case self::type::FLOAT32:
            return -1;//-self::compare_array(in_right_array, in_left_value.float32_);
        case self::type::FLOAT64:
            return -1;//-self::compare_array(in_right_array, in_left_value.float64_);
        case self::type::STRING:
        case self::type::BINARY:
        case self::type::EXTENDED_BINARY:
            return -1;//-self::compare_array(in_right_array, in_left_value.raw_);
        case self::type::ARRAY:
            return in_left_value.array_.compare(in_right_array);
        case self::type::MAP:
            return 1;//self::compare_map(in_left_value.map_, in_right_array);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief MessagePackオブジェクト値とRAWバイト列を比較する。
        @param[in] in_left_value 左辺のMessagePackオブジェクト値。
        @param[in] in_left_kind  左辺のMessagePackオブジェクト種別。
        @param[in] in_right_raw  右辺のRAWバイト列。
        @param[in] in_right_kind 右辺のMessagePackオブジェクト種別。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: static int compare_raw(
        self const& in_left_value,
        self::type::value const in_left_kind,
        self const& in_right_value,
        self::type::value const in_right_kind)
    PSYQ_NOEXCEPT
    {
        if (in_left_kind != in_right_kind)
        {
            return in_left_kind < in_right_kind? -1: 1;
        }
        switch (in_left_kind)
        {
        case self::type::NIL:
            return 1;
        case self::type::BOOLEAN:
            return -1;//-self::compare_raw(in_right_raw, in_left_value.boolean_);
        case self::type::POSITIVE_INTEGER:
            return -1;//-self::compare_raw(in_right_raw, in_left_value.positive_integer_);
        case self::type::NEGATIVE_INTEGER:
            return -1;//-self::compare_raw(in_right_raw, in_left_value.negative_integer_);
        case self::type::FLOAT32:
            return -1;//-self::compare_raw(in_right_raw, in_left_value.float32_);
        case self::type::FLOAT64:
            return -1;//-self::compare_raw(in_right_raw, in_left_value.float64_);
        case self::type::STRING:
            return in_left_value.string_.compare(in_right_value.string_);
        case self::type::BINARY:
            return in_left_value.binary_.compare(in_right_value.binary_);
        case self::type::EXTENDED_BINARY:
            return in_left_value.extended_binary_.compare(
                in_right_value.extended_binary_);
        case self::type::ARRAY:
            return 1;//self::compare_array(in_left_value.array_, in_right_raw);
        case self::type::MAP:
            return 1;//self::compare_map(in_left_value.map_, in_right_raw);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 浮動小数点数との比較
    //@{
    /** @brief MessagePackオブジェクト値と浮動小数点数を比較する。
        @param[in] in_left_value  左辺のMessagePackオブジェクト値。
        @param[in] in_left_kind   左辺のMessagePackオブジェクト値の種別。
        @param[in] in_right_float 右辺の浮動小数点数。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<typename template_float_type>
    static int compare_floating_point(
        self const& in_left_value,
        self::type::value const in_left_kind,
        template_float_type const in_right_float)
    PSYQ_NOEXCEPT
    {
        switch (in_left_kind)
        {
        case self::type::NIL:
            return 1;
        case self::type::BOOLEAN:
            return -self::compare_floating_point(
                in_right_float,
                in_left_value.boolean_,
                self::get_epsilon(template_float_type(0)));
        case self::type::POSITIVE_INTEGER:
            return -self::compare_floating_point(
                in_right_float,
                in_left_value.positive_integer_,
                self::get_epsilon(template_float_type(0)));
        case self::type::NEGATIVE_INTEGER:
            return -self::compare_floating_point(
                in_right_float,
                in_left_value.negative_integer_,
                self::get_epsilon(template_float_type(0)));
        case self::type::FLOAT32:
            return self::compare_floating_point<template_float_type>(
                in_left_value.float32_,
                in_right_float,
                PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON);
        case self::type::FLOAT64:
            return self::compare_floating_point<self::float64>(
                in_left_value.float64_,
                in_right_float,
                self::get_epsilon(template_float_type(0)));
        case self::type::STRING:
        case self::type::BINARY:
        case self::type::EXTENDED_BINARY:
            return 1;//self::compare_raw(in_left_value.raw_, in_right_float);
        case self::type::ARRAY:
            return 1;//self::compare_array(in_left_value.array_, in_right_float);
        case self::type::MAP:
            return 1;//self::compare_map(in_left_value.map_, in_right_float);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief 浮動小数点数を比較する。
        @param[in] in_left    左辺の浮動小数点数。
        @param[in] in_right   右辺の浮動小数点数。
        @param[in] in_epsilon 許容する誤差の最大値。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<typename template_float_type>
    static int compare_floating_point(
        template_float_type const in_left,
        template_float_type const in_right,
        template_float_type const in_epsilon)
    PSYQ_NOEXCEPT
    {
        auto const local_diff(in_left - in_right);
        return local_diff < -in_epsilon? -1: (in_epsilon < local_diff? 1: 0);
    }

    /** @brief 浮動小数点数と有符号整数を比較する。
        @param[in] in_left    左辺の浮動小数点数。
        @param[in] in_right   右辺の有符号整数。
        @param[in] in_epsilon 許容する誤差の最大値。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<typename template_float_type>
    static int compare_floating_point(
        template_float_type const in_left,
        std::int64_t const in_right,
        template_float_type const in_epsilon)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_right < 0);
        return in_left < -in_epsilon?
            self::compare_floating_point<self::float64>(
                in_left, static_cast<self::float64>(in_right), in_epsilon):
            1;
    }

    /** @brief 浮動小数点数と無符号整数を比較する。
        @param[in] in_left    左辺の浮動小数点数。
        @param[in] in_right   右辺の無符号整数。
        @param[in] in_epsilon 許容する誤差の最大値。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<typename template_float_type>
    static int compare_floating_point(
        template_float_type const in_left,
        std::uint64_t const in_right,
        template_float_type const in_epsilon)
    PSYQ_NOEXCEPT
    {
        return in_left < -in_epsilon?
            -1:
            self::compare_floating_point(
                in_left,
                static_cast<template_float_type>(in_right),
                in_epsilon);
    }

    /** @brief 浮動小数点数と真偽値を比較する。
        @param[in] in_left    左辺の浮動小数点数。
        @param[in] in_right   右辺の真偽値。
        @param[in] in_epsilon 許容する誤差の最大値。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<typename template_float_type>
    static int compare_floating_point(
        template_float_type const in_left,
        bool const in_right,
        template_float_type const in_epsilon)
    PSYQ_NOEXCEPT
    {
        // C++の仕様に基づき、trueは1、falseは0に変換して判定する。
        return in_left < -in_epsilon?
            -1:
            self::compare_floating_point(
                in_left,
                static_cast<template_float_type>(in_right? 1: 0),
                in_epsilon);
    }

    /** @brief 倍精度浮動小数点数で許容する誤差の最大値を取得する。
        @return 倍精度浮動小数点数で許容する誤差の最大値。
     */
    private: static self::float64 get_epsilon(self::float64 const)
    {
        return PSYQ_MESSAGE_PACK_VALUE_FLOAT64_EPSILON;
    }
    /** @brief 単精度浮動小数点数で許容する誤差の最大値を取得する。
        @return 単精度浮動小数点数で許容する誤差の最大値。
     */
    private: static self::float32 get_epsilon(self::float32 const)
    {
        return PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 有符号整数との比較
    //@{
    /** @brief MessagePackオブジェクト値と有符号整数を比較する。
        @param[in] in_left_value    左辺のMessagePackオブジェクト値。
        @param[in] in_left_kind     左辺のMessagePackオブジェクト値の種別。
        @param[in] in_right_integer 右辺の有符号整数。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<typename template_signed_type>
    static int compare_signed_integer(
        self const& in_left_value,
        self::type::value const in_left_kind,
        template_signed_type const in_right_integer)
    PSYQ_NOEXCEPT
    {
        static_assert(
            !std::is_unsigned<template_signed_type>::value,
            "template_signed_type is not signed integer type.");
        switch (in_left_kind)
        {
        case self::type::NIL:
            return 1;
        case self::type::BOOLEAN:
            return -self::compare_signed_integer(
                in_right_integer, in_left_value.boolean_);
        case self::type::POSITIVE_INTEGER:
            return -self::compare_signed_integer(
                in_right_integer, in_left_value.positive_integer_);
        case self::type::NEGATIVE_INTEGER:
            return -self::compare_signed_integer(
                in_right_integer, in_left_value.negative_integer_);
        case self::type::FLOAT32:
            return self::compare_floating_point<self::float64>(
                in_left_value.float32_,
                static_cast<self::float64>(in_right_integer),
                PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON);
        case self::type::FLOAT64:
            return self::compare_floating_point<self::float64>(
                in_left_value.float64_,
                static_cast<self::float64>(in_right_integer),
                PSYQ_MESSAGE_PACK_VALUE_FLOAT64_EPSILON);
        case self::type::STRING:
        case self::type::BINARY:
        case self::type::EXTENDED_BINARY:
            return 1;//self::compare_raw(in_left_value.raw_, in_right_integer);
        case self::type::ARRAY:
            return 1;//self::compare_array(in_left_value.array_, in_right_integer);
        case self::type::MAP:
            return 1;//self::compare_map(in_left_value.map_, in_right_integer);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief 有符号整数と浮動小数点数を比較する。
        @param[in] in_left    左辺の有符号整数。
        @param[in] in_right   右辺の浮動小数点数。
        @param[in] in_epsilon 許容する誤差の最大値。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<
        typename template_signed_type,
        typename template_float_type>
    static int compare_signed_integer(
        template_signed_type const in_left,
        template_float_type const in_right,
        template_float_type const in_epsilon)
    PSYQ_NOEXCEPT
    {
        return self::compare_floating_point(
            static_cast<template_float_type>(in_left), in_right, in_epsilon);
    }

    /** @brief 有符号整数を比較する。
        @param[in] in_left  左辺の有符号整数。
        @param[in] in_right 右辺の有符号整数。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<typename template_signed_type>
    static int compare_signed_integer(
        template_signed_type const in_left,
        std::int64_t const in_right)
    PSYQ_NOEXCEPT
    {
        return in_left < in_right? -1: (in_right < in_left? 1: 0);
    }

    /** @brief 有符号整数と無符号整数を比較する。
        @param[in] in_left  左辺の有符号整数。
        @param[in] in_right 右辺の無符号整数。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<typename template_signed_type>
    static int compare_signed_integer(
        template_signed_type const in_left,
        std::uint64_t const in_right)
    PSYQ_NOEXCEPT
    {
        typedef typename std::make_unsigned<template_signed_type>::type
            unsigned_type;
        return in_left < 0?
            -1:
            self::compare_unsigned_integer(
                static_cast<unsigned_type>(in_left), in_right);
    }

    /** @brief 有符号整数と真偽値を比較する。
        @param[in] in_left  左辺の有符号整数。
        @param[in] in_right 右辺の真偽値。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<typename template_signed_type>
    static int compare_signed_integer(
        template_signed_type const in_left,
        bool const in_right)
    PSYQ_NOEXCEPT
    {
        typedef typename std::make_unsigned<template_signed_type>::type
            unsigned_type;
        return in_left < 0?
            -1:
            self::compare_unsigned_integer(
                static_cast<unsigned_type>(in_left), in_right);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 無符号整数との比較
    //@{
    /** @brief MessagePackオブジェクト値と無符号整数を比較する。
        @param[in] in_left_value    左辺のMessagePackオブジェクト値。
        @param[in] in_left_kind     左辺のMessagePackオブジェクト値の種別。
        @param[in] in_right_integer 右辺の無符号整数。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<typename template_unsigned_type>
    static int compare_unsigned_integer(
        self const& in_left_value,
        self::type::value const in_left_kind,
        template_unsigned_type const in_right_integer)
    PSYQ_NOEXCEPT
    {
        static_assert(
            std::is_unsigned<template_unsigned_type>::value,
            "template_unsigned_type is not unsigned integer type.");
        switch (in_left_kind)
        {
        case self::type::NIL:
            return 1;
        case self::type::BOOLEAN:
            return -self::compare_unsigned_integer(
                in_right_integer, in_left_value.boolean_);
        case self::type::POSITIVE_INTEGER:
            return -self::compare_unsigned_integer(
                in_right_integer, in_left_value.positive_integer_);
        case self::type::NEGATIVE_INTEGER:
            return -1;
        case self::type::FLOAT32:
            return -self::compare_unsigned_integer(
                in_right_integer,
                static_cast<self::float64>(in_left_value.float32_),
                static_cast<self::float64>(
                    PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON));
        case self::type::FLOAT64:
            return -self::compare_unsigned_integer(
                in_right_integer,
                in_left_value.float64_,
                PSYQ_MESSAGE_PACK_VALUE_FLOAT64_EPSILON);
        case self::type::STRING:
        case self::type::BINARY:
        case self::type::EXTENDED_BINARY:
            return 1;//self::compare_raw(in_left_value.raw_, in_right_integer);
        case self::type::ARRAY:
            return 1;//self::compare_array(in_left_value.array_, in_right_integer);
        case self::type::MAP:
            return 1;//self::compare_map(in_left_value.map_, in_right_integer);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief 無符号整数と浮動小数点数を比較する。
        @param[in] in_left    左辺の無符号整数。
        @param[in] in_right   右辺の浮動小数点数。
        @param[in] in_epsilon 許容する誤差の最大値。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<
        typename template_unsigned_type,
        typename template_float_type>
    static int compare_unsigned_integer(
        template_unsigned_type const in_left,
        template_float_type const in_right,
        template_float_type const in_epsilon)
    PSYQ_NOEXCEPT
    {
        static_assert(
            std::is_unsigned<template_unsigned_type>::value,
            "template_unsigned_type is not unsigned integer type.");
        return in_right < -in_epsilon?
            1:
            self::compare_floating_point(
                static_cast<template_float_type>(in_left),
                in_right,
                in_epsilon);
    }

    /** @brief 無符号整数を比較する。
        @param[in] in_left  左辺の無符号整数。
        @param[in] in_right 右辺の無符号整数。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<typename template_unsigned_type>
    static int compare_unsigned_integer(
        template_unsigned_type const in_left,
        std::uint64_t const in_right)
    PSYQ_NOEXCEPT
    {
        return in_left < in_right? -1: (in_right < in_left? 1: 0);
    }

    /** @brief 無符号整数と真偽値を比較する。
        @param[in] in_left  左辺の無符号整数。
        @param[in] in_right 右辺の真偽値。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<typename template_unsigned_type>
    static int compare_unsigned_integer(
        template_unsigned_type const in_left,
        bool const in_right)
    PSYQ_NOEXCEPT
    {
        // C++の仕様に基づき、trueは1、falseは0に変換して判定する。
        return in_right?
            (1 < in_left? 1: (in_left < 1? -1: 0)):
            (0 < in_left? 1: 0);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 真偽値の比較
    //@{
    /** @brief MessagePackオブジェクト値と真偽値を比較する。
        @param[in] in_left_value    左辺のMessagePackオブジェクト値。
        @param[in] in_left_kind     左辺のMessagePackオブジェクト値の種別。
        @param[in] in_right_boolean 右辺の真偽値。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: static int compare_boolean(
        self const& in_left_value,
        self::type::value const in_left_kind,
        bool const in_right_boolean)
    PSYQ_NOEXCEPT
    {
        switch (in_left_kind)
        {
        case self::type::NIL:
            return 1;
        case self::type::BOOLEAN:
            return self::compare_boolean(
                in_left_value.boolean_, in_right_boolean);
        case self::type::POSITIVE_INTEGER:
            return self::compare_unsigned_integer(
                in_left_value.positive_integer_, in_right_boolean);
        case self::type::NEGATIVE_INTEGER:
            return -1;
        case self::type::FLOAT32:
            return self::compare_floating_point(
                in_left_value.float32_,
                in_right_boolean,
                PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON);
        case self::type::FLOAT64:
            return self::compare_floating_point(
                in_left_value.float64_,
                in_right_boolean,
                PSYQ_MESSAGE_PACK_VALUE_FLOAT64_EPSILON);
        case self::type::STRING:
        case self::type::BINARY:
        case self::type::EXTENDED_BINARY:
            return 1;//self::compare_raw(in_left_value.raw_, in_right_boolean);
        case self::type::ARRAY:
            return 1;//self::compare_array(in_left_value.array_, in_right_boolean);
        case self::type::MAP:
            return 1;//self::compare_map(in_left_value.map_, in_right_boolean);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief 真偽値を比較する。
        @param[in] in_left  左辺の真偽値。
        @param[in] in_right 右辺の真偽値。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: static int compare_boolean(
        bool const in_left,
        bool const in_right)
    PSYQ_NOEXCEPT
    {
        return in_left == in_right? 0: (in_right? 1: -1);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @copydoc self::type::BOOLEAN
    public: bool boolean_;
    /// @copydoc self::type::POSITIVE_INTEGER
    public: std::uint64_t positive_integer_;
    /// @copydoc self::type::NEGATIVE_INTEGER
    public: std::int64_t negative_integer_;
    /// self::float32_ をビット列として取り出すための32bit整数。
    public: std::uint32_t uint32_;
    /// @copydoc self::type::FLOAT32
    public: self::float32 float32_;
    /// @copydoc self::type::FLOAT64
    public: self::float64 float64_;
    /// @copydoc self::string
    public: self::string string_;
    /// @copydoc self::binary
    public: self::binary binary_;
    /// @copydoc self::extended_binary
    public: self::extended_binary extended_binary_;
    /// @copydoc self::type::ARRAY
    public: self::array array_;
    /// @copydoc self::type::MAP
    public: self::map map_;
};

#endif // !defined(PSYQ_MESSAGE_PACK_VALUE_HPP_)
