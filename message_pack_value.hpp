/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copydoc psyq::internal::message_pack_value
 */
#ifndef PSYQ_MESSAGE_PACK_VALUE_HPP_
#define PSYQ_MESSAGE_PACK_VALUE_HPP_

//#include "psyq/message_pack_container.hpp"

/// 倍精度浮動小数点実数で許容する誤差の最大値。
#ifndef PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON
#define PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON\
    (std::numeric_limits<self::float32>::epsilon() * 4)
#endif // !defined(PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON)

/// 単精度浮動小数点実数で許容する誤差の最大値。
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
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief MessagePackオブジェクトの値。
    @sa psyq::message_pack::object
 */
union psyq::internal::message_pack_value
{
    private: typedef message_pack_value self; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /** @brief MessagePackオブジェクトに格納されてる値の種別。
     */
    public: enum class kind: std::uint8_t
    {
        NIL,              ///< 空。
        BOOLEAN,          ///< @copydoc self::boolean_
        POSITIVE_INTEGER, ///< @copydoc self::positive_integer_
        NEGATIVE_INTEGER, ///< @copydoc self::negative_integer_
        FLOAT32,          ///< @copydoc self::float32_
        FLOAT64,          ///< @copydoc self::float64_
        RAW,              ///< @copydoc self::raw_
        ARRAY,            ///< @copydoc self::array_
        MAP,              ///< @copydoc self::map_
    };

    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトに格納する型
    //@{
    /// @copydoc self::float32_
    public: typedef float float32;
    /// @copydoc self::float64_
    public: typedef double float64;
    /// @copydoc self::raw_
    public: typedef psyq::internal::message_pack_container<char const>
        raw;
    /// @copydoc self::array_
    public: typedef psyq::internal::message_pack_container<
        psyq::message_pack::object>
            array;
    /// @copydoc self::map_
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

    /** @brief MessagePackオブジェクトに浮動小数点実数を格納する。
        @param[in] in_float MessagePackオブジェクトに格納する浮動小数点実数。
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

    /** @brief MessagePackオブジェクトにRAWバイト列を格納する。
        @param[in] in_raw MessagePackオブジェクトに格納するRAWバイト列。
     */
    public: explicit PSYQ_CONSTEXPR message_pack_value(
        self::raw const& in_raw)
    PSYQ_NOEXCEPT:
        raw_(in_raw)
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
        self::kind const in_left_kind,
        self const& in_right_value,
        self::kind const in_right_kind)
    PSYQ_NOEXCEPT
    {
        if (in_left_kind != in_right_kind)
        {
            return false;
        }
        switch (in_left_kind)
        {
        case self::kind::NIL:
            return true;
        case self::kind::BOOLEAN:
            return in_left_value.boolean_ == in_right_value.boolean_;
        case self::kind::POSITIVE_INTEGER:
            return in_left_value.positive_integer_
                == in_right_value.positive_integer_;
        case self::kind::NEGATIVE_INTEGER:
            return in_left_value.negative_integer_
                == in_right_value.negative_integer_;
        case self::kind::FLOAT32:
            return 0 == self::compare_floating_point(
                in_left_value.float32_,
                in_right_value.float32_,
                PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON);
        case self::kind::FLOAT64:
            return 0 == self::compare_floating_point(
                in_left_value.float64_,
                in_right_value.float64_,
                PSYQ_MESSAGE_PACK_VALUE_FLOAT64_EPSILON);
        case self::kind::RAW:
            return in_left_value.raw_ == in_right_value.raw_;
        case self::kind::ARRAY:
            return in_left_value.array_ == in_right_value.array_;
        case self::kind::MAP:
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
        self::kind const in_left_kind,
        self const& in_right_value,
        self::kind const in_right_kind)
    PSYQ_NOEXCEPT
    {
        switch (in_right_kind)
        {
        case self::kind::NIL:
            return in_left_kind != self::kind::NIL? 1: 0;
        case self::kind::BOOLEAN:
            return self::compare_boolean(
                in_left_value, in_left_kind, in_right_value.boolean_);
        case self::kind::POSITIVE_INTEGER:
            return self::compare_unsigned_integer(
                in_left_value, in_left_kind, in_right_value.positive_integer_);
        case self::kind::NEGATIVE_INTEGER:
            return self::compare_signed_integer(
                in_left_value, in_left_kind, in_right_value.negative_integer_);
        case self::kind::FLOAT32:
            return self::compare_floating_point(
                in_left_value, in_left_kind, in_right_value.float32_);
        case self::kind::FLOAT64:
            return self::compare_floating_point(
                in_left_value, in_left_kind, in_right_value.float64_);
        case self::kind::RAW:
            return self::compare_raw(
                in_left_value, in_left_kind, in_right_value.raw_);
        case self::kind::ARRAY:
            return self::compare_array(
                in_left_value, in_left_kind, in_right_value.array_);
        case self::kind::MAP:
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
    private: static bool is_valid_kind(self::kind const in_kind)
    {
        return in_kind <= self::kind::MAP;
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
        self::kind const in_left_kind,
        self::map const& in_right_map)
    PSYQ_NOEXCEPT
    {
        switch (in_left_kind)
        {
        case self::kind::NIL:
            return 1;
        case self::kind::BOOLEAN:
            return -1;//-self::compare_map(in_right_map, this->boolean_);
        case self::kind::POSITIVE_INTEGER:
            return -1;//-self::compare_map(in_right_map, this->positive_integer_);
        case self::kind::NEGATIVE_INTEGER:
            return -1;//-self::compare_map(in_right_map, this->negative_integer_);
        case self::kind::FLOAT32:
            return -1;//-self::compare_map(in_right_map, this->float32_);
        case self::kind::FLOAT64:
            return -1;//-self::compare_map(in_right_map, this->float64_);
        case self::kind::RAW:
            return -1;//-self::compare_map(in_right_map, this->raw_);
        case self::kind::ARRAY:
            return -1;//-self::compare_map(in_right_map, this->array_);
        case self::kind::MAP:
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
        self::kind const in_left_kind,
        self::array const& in_right_array)
    PSYQ_NOEXCEPT
    {
        switch (in_left_kind)
        {
        case self::kind::NIL:
            return 1;
        case self::kind::BOOLEAN:
            return -1;//-self::compare_array(in_right_array, in_left_value.boolean_);
        case self::kind::POSITIVE_INTEGER:
            return -1;//-self::compare_array(in_right_array, in_left_value.positive_integer_);
        case self::kind::NEGATIVE_INTEGER:
            return -1;//-self::compare_array(in_right_array, in_left_value.negative_integer_);
        case self::kind::FLOAT32:
            return -1;//-self::compare_array(in_right_array, in_left_value.float32_);
        case self::kind::FLOAT64:
            return -1;//-self::compare_array(in_right_array, in_left_value.float64_);
        case self::kind::RAW:
            return -1;//-self::compare_array(in_right_array, in_left_value.raw_);
        case self::kind::ARRAY:
            return in_left_value.array_.compare(in_right_array);
        case self::kind::MAP:
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
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: static int compare_raw(
        self const& in_left_value,
        self::kind const in_left_kind,
        self::raw const& in_right_raw)
    PSYQ_NOEXCEPT
    {
        switch (in_left_kind)
        {
        case self::kind::NIL:
            return 1;
        case self::kind::BOOLEAN:
            return -1;//-self::compare_raw(in_right_raw, in_left_value.boolean_);
        case self::kind::POSITIVE_INTEGER:
            return -1;//-self::compare_raw(in_right_raw, in_left_value.positive_integer_);
        case self::kind::NEGATIVE_INTEGER:
            return -1;//-self::compare_raw(in_right_raw, in_left_value.negative_integer_);
        case self::kind::FLOAT32:
            return -1;//-self::compare_raw(in_right_raw, in_left_value.float32_);
        case self::kind::FLOAT64:
            return -1;//-self::compare_raw(in_right_raw, in_left_value.float64_);
        case self::kind::RAW:
            return in_left_value.raw_.compare(in_right_raw);
        case self::kind::ARRAY:
            return 1;//self::compare_array(in_left_value.array_, in_right_raw);
        case self::kind::MAP:
            return 1;//self::compare_map(in_left_value.map_, in_right_raw);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 浮動小数点実数との比較
    //@{
    /** @brief MessagePackオブジェクト値と浮動小数点実数を比較する。
        @param[in] in_left_value  左辺のMessagePackオブジェクト値。
        @param[in] in_left_kind   左辺のMessagePackオブジェクト値の種別。
        @param[in] in_right_float 右辺の浮動小数点実数。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    private: template<typename template_float_type>
    static int compare_floating_point(
        self const& in_left_value,
        self::kind const in_left_kind,
        template_float_type const in_right_float)
    PSYQ_NOEXCEPT
    {
        switch (in_left_kind)
        {
        case self::kind::NIL:
            return 1;
        case self::kind::BOOLEAN:
            return -self::compare_floating_point(
                in_right_float,
                in_left_value.boolean_,
                self::get_epsilon(template_float_type(0)));
        case self::kind::POSITIVE_INTEGER:
            return -self::compare_floating_point(
                in_right_float,
                in_left_value.positive_integer_,
                self::get_epsilon(template_float_type(0)));
        case self::kind::NEGATIVE_INTEGER:
            return -self::compare_floating_point(
                in_right_float,
                in_left_value.negative_integer_,
                self::get_epsilon(template_float_type(0)));
        case self::kind::FLOAT32:
            return self::compare_floating_point<template_float_type>(
                in_left_value.float32_,
                in_right_float,
                PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON);
        case self::kind::FLOAT64:
            return self::compare_floating_point<self::float64>(
                in_left_value.float64_,
                in_right_float,
                self::get_epsilon(template_float_type(0)));
        case self::kind::RAW:
            return 1;//self::compare_raw(in_left_value.raw_, in_right_float);
        case self::kind::ARRAY:
            return 1;//self::compare_array(in_left_value.array_, in_right_float);
        case self::kind::MAP:
            return 1;//self::compare_map(in_left_value.map_, in_right_float);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief 浮動小数点実数を比較する。
        @param[in] in_left    左辺の浮動小数点実数。
        @param[in] in_right   右辺の浮動小数点実数。
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

    /** @brief 浮動小数点実数と有符号整数を比較する。
        @param[in] in_left    左辺の浮動小数点実数。
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

    /** @brief 浮動小数点実数と無符号整数を比較する。
        @param[in] in_left    左辺の浮動小数点実数。
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

    /** @brief 浮動小数点実数と真偽値を比較する。
        @param[in] in_left    左辺の浮動小数点実数。
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

    /** @brief 倍精度浮動小数点実数で許容する誤差の最大値を取得する。
        @return 倍精度浮動小数点実数で許容する誤差の最大値。
     */
    private: static self::float64 get_epsilon(self::float64 const)
    {
        return PSYQ_MESSAGE_PACK_VALUE_FLOAT64_EPSILON;
    }
    /** @brief 単精度浮動小数点実数で許容する誤差の最大値を取得する。
        @return 単精度浮動小数点実数で許容する誤差の最大値。
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
        self::kind const in_left_kind,
        template_signed_type const in_right_integer)
    PSYQ_NOEXCEPT
    {
        static_assert(
            !std::is_unsigned<template_signed_type>::value,
            "template_signed_type is not signed integer type.");
        switch (in_left_kind)
        {
        case self::kind::NIL:
            return 1;
        case self::kind::BOOLEAN:
            return -self::compare_signed_integer(
                in_right_integer, in_left_value.boolean_);
        case self::kind::POSITIVE_INTEGER:
            return -self::compare_signed_integer(
                in_right_integer, in_left_value.positive_integer_);
        case self::kind::NEGATIVE_INTEGER:
            return -self::compare_signed_integer(
                in_right_integer, in_left_value.negative_integer_);
        case self::kind::FLOAT32:
            return self::compare_floating_point<self::float64>(
                in_left_value.float32_,
                static_cast<self::float64>(in_right_integer),
                PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON);
        case self::kind::FLOAT64:
            return self::compare_floating_point<self::float64>(
                in_left_value.float64_,
                static_cast<self::float64>(in_right_integer),
                PSYQ_MESSAGE_PACK_VALUE_FLOAT64_EPSILON);
        case self::kind::RAW:
            return 1;//self::compare_raw(in_left_value.raw_, in_right_integer);
        case self::kind::ARRAY:
            return 1;//self::compare_array(in_left_value.array_, in_right_integer);
        case self::kind::MAP:
            return 1;//self::compare_map(in_left_value.map_, in_right_integer);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief 有符号整数と浮動小数点実数を比較する。
        @param[in] in_left    左辺の有符号整数。
        @param[in] in_right   右辺の浮動小数点実数。
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
        self::kind const in_left_kind,
        template_unsigned_type const in_right_integer)
    PSYQ_NOEXCEPT
    {
        static_assert(
            std::is_unsigned<template_unsigned_type>::value,
            "template_unsigned_type is not unsigned integer type.");
        switch (in_left_kind)
        {
        case self::kind::NIL:
            return 1;
        case self::kind::BOOLEAN:
            return -self::compare_unsigned_integer(
                in_right_integer, in_left_value.boolean_);
        case self::kind::POSITIVE_INTEGER:
            return -self::compare_unsigned_integer(
                in_right_integer, in_left_value.positive_integer_);
        case self::kind::NEGATIVE_INTEGER:
            return -1;
        case self::kind::FLOAT32:
            return -self::compare_unsigned_integer(
                in_right_integer,
                static_cast<self::float64>(in_left_value.float32_),
                static_cast<self::float64>(
                    PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON));
        case self::kind::FLOAT64:
            return -self::compare_unsigned_integer(
                in_right_integer,
                in_left_value.float64_,
                PSYQ_MESSAGE_PACK_VALUE_FLOAT64_EPSILON);
        case self::kind::RAW:
            return 1;//self::compare_raw(in_left_value.raw_, in_right_integer);
        case self::kind::ARRAY:
            return 1;//self::compare_array(in_left_value.array_, in_right_integer);
        case self::kind::MAP:
            return 1;//self::compare_map(in_left_value.map_, in_right_integer);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief 無符号整数と浮動小数点実数を比較する。
        @param[in] in_left    左辺の無符号整数。
        @param[in] in_right   右辺の浮動小数点実数。
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
        self::kind const in_left_kind,
        bool const in_right_boolean)
    PSYQ_NOEXCEPT
    {
        switch (in_left_kind)
        {
        case self::kind::NIL:
            return 1;
        case self::kind::BOOLEAN:
            return self::compare_boolean(
                in_left_value.boolean_, in_right_boolean);
        case self::kind::POSITIVE_INTEGER:
            return self::compare_unsigned_integer(
                in_left_value.positive_integer_, in_right_boolean);
        case self::kind::NEGATIVE_INTEGER:
            return -1;
        case self::kind::FLOAT32:
            return self::compare_floating_point(
                in_left_value.float32_,
                in_right_boolean,
                PSYQ_MESSAGE_PACK_VALUE_FLOAT32_EPSILON);
        case self::kind::FLOAT64:
            return self::compare_floating_point(
                in_left_value.float64_,
                in_right_boolean,
                PSYQ_MESSAGE_PACK_VALUE_FLOAT64_EPSILON);
        case self::kind::RAW:
            return 1;//self::compare_raw(in_left_value.raw_, in_right_boolean);
        case self::kind::ARRAY:
            return 1;//self::compare_array(in_left_value.array_, in_right_boolean);
        case self::kind::MAP:
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
    /// 真偽値。
    public: bool boolean_;
    /// 0以上の整数。
    public: std::uint64_t positive_integer_;
    /// 0未満の整数。
    public: std::int64_t negative_integer_;
    /// 単精度浮動小数点数実数。
    public: self::float32 float32_;
    /// 倍精度浮動小数点実数。
    public: self::float64 float64_;
    /// RAWバイト列。
    public: self::raw raw_;
    /// MessagePackオブジェクトの配列。
    public: self::array array_;
    /// MessagePackオブジェクトの連想配列。
    public: self::map map_;
    /// self::float32_ をビット列として取り出すための32bit整数。
    public: std::uint32_t uint32_;
};

#endif // !defined(PSYQ_MESSAGE_PACK_VALUE_HPP_)
