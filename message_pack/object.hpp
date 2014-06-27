/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::message_pack::object
 */
#ifndef PSYQ_MESSAGE_PACK_OBJECT_HPP_
#define PSYQ_MESSAGE_PACK_OBJECT_HPP_

//#include "psyq/message_pack/value.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// MessagePackオブジェクト。
class psyq::message_pack::object
{
    private: typedef object self; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// MessagePackオブジェクトの値。
    private: typedef psyq::internal::message_pack_value value;

    /// @copydoc psyq::internal::message_pack_value::type::value
    public: typedef psyq::internal::message_pack_value::type::value type;

    /// @name MessagePackオブジェクトが持つ値の型。
    //@{
    /// @copydoc psyq::internal::message_pack_value::type::UNSIGNED_INTEGER
    public: typedef psyq::internal::message_pack_value::unsigned_integer unsigned_integer;
    /// @copydoc psyq::internal::message_pack_value::type::NEGATIVE_INTEGER
    public: typedef psyq::internal::message_pack_value::negative_integer negative_integer;
    /// @copydoc psyq::internal::message_pack_value::type::FLOATING_POINT_32
    public: typedef psyq::internal::message_pack_value::floating_point_32 floating_point_32;
    /// @copydoc psyq::internal::message_pack_value::type::FLOATING_POINT_64
    public: typedef psyq::internal::message_pack_value::floating_point_64 floating_point_64;
    /// @copydoc psyq::internal::message_pack_value::type::STRING
    public: typedef psyq::internal::message_pack_value::string string;
    /// @copydoc psyq::internal::message_pack_value::type::BINARY
    public: typedef psyq::internal::message_pack_value::binary binary;
    /// @copydoc psyq::internal::message_pack_value::type::EXTENDED
    public: typedef psyq::internal::message_pack_value::extended extended;
    /// @copydoc psyq::internal::message_pack_value::type::ARRAY
    public: typedef psyq::internal::message_pack_value::array array;
    /// @copydoc psyq::internal::message_pack_value::type::UNORDERED_MAP
    public: typedef psyq::internal::message_pack_value::unordered_map unordered_map;
    /// @copydoc psyq::internal::message_pack_value::type::MAP
    public: typedef psyq::internal::message_pack_value::map map;
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトの構築
    //@{
    /// 空のMessagePackオブジェクトを構築する。
    public: PSYQ_CONSTEXPR object(): type_(self::type::NIL) {}

    /** @brief MessagePackオブジェクトに真偽値を格納する。
        @param[in] in_boolean MessagePackオブジェクトに格納する真偽値。
     */
    public: PSYQ_CONSTEXPR object(bool const in_boolean)
    PSYQ_NOEXCEPT:
        value_(in_boolean),
        type_(self::type::BOOLEAN)
    {}

    /** @brief MessagePackオブジェクトに無符号整数を格納する。
        @param[in] in_integer MessagePackオブジェクトに格納する無符号整数。
     */
    public: PSYQ_CONSTEXPR object(unsigned long long const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::type::UNSIGNED_INTEGER)
    {}
    /// @copydoc object(unsigned long long const)
    public: PSYQ_CONSTEXPR object(unsigned long const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::type::UNSIGNED_INTEGER)
    {}
    /// @copydoc object(unsigned long long const)
    public: PSYQ_CONSTEXPR object(unsigned int const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::type::UNSIGNED_INTEGER)
    {}
    /// @copydoc object(unsigned long long const)
    public: PSYQ_CONSTEXPR object(unsigned short const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::type::UNSIGNED_INTEGER)
    {}
    /// @copydoc object(unsigned long long const)
    public: PSYQ_CONSTEXPR object(unsigned char const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::type::UNSIGNED_INTEGER)
    {}

    /** @brief MessagePackオブジェクトに有符号整数を格納する。
        @param[in] in_integer MessagePackオブジェクトに格納する有符号整数。
     */
    public: PSYQ_CONSTEXPR object(signed long long const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::tell_signed_integer_type(in_integer))
    {}
    /// @copydoc object(signed long long const)
    public: PSYQ_CONSTEXPR object(signed long const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::tell_signed_integer_type(in_integer))
    {}
    /// @copydoc object(signed long long const)
    public: PSYQ_CONSTEXPR object(signed int const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::tell_signed_integer_type(in_integer))
    {}
    /// @copydoc object(signed long long const)
    public: PSYQ_CONSTEXPR object(signed short const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::tell_signed_integer_type(in_integer))
    {}
    /// @copydoc object(signed long long const)
    public: PSYQ_CONSTEXPR object(signed char const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::tell_signed_integer_type(in_integer))
    {}

    /** @brief MessagePackオブジェクトに浮動小数点数を格納する。
        @param[in] in_float MessagePackオブジェクトに格納する浮動小数点数。
     */
    public: PSYQ_CONSTEXPR object(self::floating_point_64 const in_float)
    PSYQ_NOEXCEPT:
        value_(in_float),
        type_(self::type::FLOATING_POINT_64)
    {}
    /// @copydoc object(self::floating_point_64 const)
    public: PSYQ_CONSTEXPR object(self::floating_point_32 const in_float)
    PSYQ_NOEXCEPT:
        value_(in_float),
        type_(self::type::FLOATING_POINT_32)
    {}

    /** @brief MessagePackオブジェクトに文字列を格納する。
        @param[in] in_string MessagePackオブジェクトに格納する文字列。
     */
    public: PSYQ_CONSTEXPR object(self::string const& in_string)
    PSYQ_NOEXCEPT:
        value_(in_string),
        type_(self::type::STRING)
    {}

    /** @brief MessagePackオブジェクトにバイナリを格納する。
        @param[in] in_binary MessagePackオブジェクトに格納するバイナリ。
     */
    public: PSYQ_CONSTEXPR object(self::binary const& in_binary)
    PSYQ_NOEXCEPT:
        value_(in_binary),
        type_(self::type::BINARY)
    {}
    /** @brief MessagePackオブジェクトに拡張バイナリを格納する。
        @param[in] in_binary MessagePackオブジェクトに格納する拡張バイナリ。
     */
    public: PSYQ_CONSTEXPR object(self::extended const& in_binary)
    PSYQ_NOEXCEPT:
        value_(in_binary),
        type_(self::type::EXTENDED)
    {}

    /** @brief MessagePackオブジェクトに配列を格納する。
        @param[in] in_array MessagePackオブジェクトに格納する配列。
     */
    public: PSYQ_CONSTEXPR object(self::array const& in_array)
    PSYQ_NOEXCEPT:
        value_(in_array),
        type_(self::type::ARRAY)
    {}

    /** @brief MessagePackオブジェクトに連想配列を格納する。
        @param[in] in_map MessagePackオブジェクトに格納する連想配列。
     */
    public: PSYQ_CONSTEXPR object(self::unordered_map const& in_map)
    PSYQ_NOEXCEPT:
        value_(in_map),
        type_(self::type::UNORDERED_MAP)
    {}

    /// @brief MessagePackオブジェクトを空にする。
    public: void reset() PSYQ_NOEXCEPT
    {
        new(this) self();
    }
    //@}
    /** @brief MessagePackオブジェクトに連想配列を格納する。
        @param[in] in_map MessagePackオブジェクトに格納する連想配列。
     */
    private: explicit PSYQ_CONSTEXPR object(self::map const& in_map)
    PSYQ_NOEXCEPT:
        value_(in_map),
        type_(self::type::MAP)
    {}

    /** @brief 文字列を参照するMessagePackオブジェクトを構築する。
        @param[in] in_string 参照する文字列。
     */
    public: template<typename template_string>
    static self make_string(template_string const& in_string) PSYQ_NOEXCEPT
    {
        self::string local_string;
        local_string.reset(in_string.data(), in_string.length());
        return self(local_string);
    }
    /** @brief 文字列literalを参照するMessagePackオブジェクトを構築する。
        @tparam template_size 参照する文字列literalの要素数。終端文字も含む。
        @param[in] in_literal 参照する文字列literal。
        @warning 文字列literal以外の文字列を引数に渡すのは禁止。
        @note
            引数が文字列literalであることを保証するため、
            user定義literalを経由して呼び出すようにしたい。
     */
    public: template<typename template_char, std::size_t template_size>
    static self make_string(template_char const (&in_literal)[template_size])
    PSYQ_NOEXCEPT
    {
        static_assert(0 < template_size, "");
        PSYQ_ASSERT(in_literal[template_size - 1] == 0);
        self::string local_string;
        local_string.reset(&in_literal[0], template_size - 1);
        return self(local_string);
    }

    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトの比較
    //@{
    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと等値か判定する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @retval true  等値だった 。
        @retval false 非等値だった。
     */
    public: bool operator==(self const& in_right) const PSYQ_NOEXCEPT
    {
        return self::value::equal(
            this->value_,
            this->get_type(),
            in_right.value_,
            in_right.get_type());
    }

    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと非等値か判定する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @retval true  非等値だった 。
        @retval false 等値だった。
     */
    public: bool operator!=(self const& in_right) const PSYQ_NOEXCEPT
    {
        return !(*this == in_right);
    }

    /** @brief *thisとMessagePackオブジェクトを比較する。
        @param[in] in_object 比較するMessagePackオブジェクト。
        @retval 正 *thisのほうが大きい。
        @retval 0  等値。
        @retval 負 *thisのほうが小さい。
     */
    public: int compare(self const& in_object) const PSYQ_NOEXCEPT
    {
        return self::value::compare(
            this->value_,
            this->get_type(),
            in_object.value_,
            in_object.get_type());
    }
    //@}
    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと比較する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @return 左辺 < 右辺
     */
    private: bool operator<(self const& in_right) const PSYQ_NOEXCEPT
    {
        return this->compare(in_right) < 0;
    }

    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと比較する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @return 左辺 <= 右辺
     */
    private: bool operator<=(self const& in_right) const PSYQ_NOEXCEPT
    {
        return this->compare(in_right) <= 0;
    }

    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと比較する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @return 左辺 > 右辺
     */
    private: bool operator>(self const& in_right) const PSYQ_NOEXCEPT
    {
        return 0 < this->compare(in_right);
    }

    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと比較する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @return 左辺 >= 右辺
     */
    private: bool operator>=(self const& in_right) const PSYQ_NOEXCEPT
    {
        return 0 <= this->compare(in_right);
    }

    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトに格納されてる値の取得
    //@{
    /** @brief MessagePackオブジェクトに格納されてる値の種別を取得する。
        @return @copydoc self::type
     */
    public: PSYQ_CONSTEXPR self::type get_type() const PSYQ_NOEXCEPT
    {
        return this->type_;
    }

    /** @brief MessagePackオブジェクトに格納されてる真偽値を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる真偽値へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは真偽値ではない。
     */
    public: PSYQ_CONSTEXPR bool const* get_boolean() const PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::BOOLEAN?
            &this->value_.boolean_: nullptr;
    }

    /** @brief MessagePackオブジェクトに格納されてる0以上の整数を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる正の整数へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは正の整数ではない。
     */
    public: PSYQ_CONSTEXPR self::unsigned_integer const* get_unsigned_integer()
    const PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::UNSIGNED_INTEGER?
            &this->value_.unsigned_integer_: nullptr;
    }

    /** @brief MessagePackオブジェクトに格納されてる0未満の整数を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる負の整数へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは負の整数ではない。
     */
    public: PSYQ_CONSTEXPR self::negative_integer const* get_negative_integer()
    const PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::NEGATIVE_INTEGER?
            &this->value_.negative_integer_: nullptr;
    }

    /** @brief MessagePackオブジェクトに格納されてる整数を取得する。
        @param[out] out_integer 取得した整数が格納される。
        @retval true  取得に成功した。
        @retval false 取得に失敗した。
     */
    public: template<typename template_integer_type>
    bool get_integer(template_integer_type& out_integer) const PSYQ_NOEXCEPT
    {
        static_assert(
            std::is_integral<template_integer_type>::value,
            "template_integer_type is not integer type.");
        switch (this->get_type())
        {
        case self::type::UNSIGNED_INTEGER:
            if ((std::numeric_limits<template_integer_type>::max)()
                < this->value_.unsigned_integer_)
            {
                return false; // 範囲外なので失敗。
            }
            break;
        case self::type::NEGATIVE_INTEGER:
            if (this->value_.negative_integer_
                < (std::numeric_limits<template_integer_type>::min)())
            {
                return false; // 範囲外なので失敗。
            }
            break;
        default:
            return false; // 整数以外が格納されていたので失敗。
        }
        out_integer = static_cast<template_integer_type>(
            this->value_.unsigned_integer_);
        return true;
    }

    /** @brief MessagePackオブジェクトに格納されてる浮動小数点数を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる浮動小数点数へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは浮動小数点数ではない。
     */
    public: PSYQ_CONSTEXPR self::floating_point_64 const* get_floating_point_64()
    const PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::FLOATING_POINT_64?
            &this->value_.floating_point_64_: nullptr;
    }
    /// @copydoc self::get_floating_point_64()
    public: PSYQ_CONSTEXPR self::floating_point_32 const* get_floating_point_32()
    const PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::FLOATING_POINT_32?
            &this->value_.floating_point_32_: nullptr;
    }

    /** @brief MessagePackオブジェクトに格納されてる浮動小数点数を取得する。
        @param[out] out_value 取得した浮動小数点数が格納される。
        @retval true  取得に成功した。
        @retval false 取得に失敗した。
     */
    public: template<typename template_value_type>
    bool get_floating_point(template_value_type& out_value) const PSYQ_NOEXCEPT
    {
        switch (this->get_type())
        {
        case self::type::FLOATING_POINT_32:
            out_value = static_cast<template_value_type>(this->value_.floating_point_32_);
            return true;
        case self::type::FLOATING_POINT_64:
            out_value = static_cast<template_value_type>(this->value_.floating_point_64_);
            return true;
        default:
            return false; // 浮動小数点数以外が格納されていたので失敗。
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトに格納されてるRAWバイト列の取得
    //@{
    /** @brief MessagePackオブジェクトに格納されてる文字列を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる文字列へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは文字列ではない。
     */
    public: PSYQ_CONSTEXPR self::string const* get_string() const PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::STRING?
            &this->value_.string_: nullptr;
    }
    /** @brief MessagePackオブジェクトに格納されてるバイナリを取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてるバイナリへのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのはバイナリではない。
     */
    public: PSYQ_CONSTEXPR self::binary const* get_binary() const PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::BINARY?
            &this->value_.binary_: nullptr;
    }
    /** @brief MessagePackオブジェクトに格納されてる拡張バイナリを取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる拡張バイナリへのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは拡張バイナリではない。
     */
    public: PSYQ_CONSTEXPR self::extended const* get_extended()
    const PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::EXTENDED?
            &this->value_.extended_: nullptr;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトに格納されてる配列の取得
    //@{
    /** @brief MessagePackオブジェクトに格納されてる配列を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる配列へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは配列ではない。
     */
    public: PSYQ_CONSTEXPR self::array* get_array() PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::ARRAY?
            &this->value_.array_: nullptr;
    }
    /// @copydoc get_array()
    public: PSYQ_CONSTEXPR self::array const* get_array() const PSYQ_NOEXCEPT
    {
        return const_cast<self*>(this)->get_array();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトに格納されてる連想配列の取得
    //@{
    /** @brief MessagePackオブジェクトに格納されてる連想配列を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる連想配列へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは連想配列ではない。
     */
    public: PSYQ_CONSTEXPR self::unordered_map* get_unordered_map() PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::UNORDERED_MAP
            || this->get_type() == self::type::MAP?
                &this->value_.map_: nullptr;
    }
    /// @copydoc get_map()
    public: PSYQ_CONSTEXPR self::unordered_map const* get_unordered_map()
    const PSYQ_NOEXCEPT
    {
        return const_cast<self*>(this)->get_unordered_map();
    }

    /** @brief MessagePackオブジェクトに格納されてる連想配列を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる連想配列へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは連想配列ではない。
     */
    public: PSYQ_CONSTEXPR self::map* get_map() PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::MAP?
            &this->value_.map_: nullptr;
    }
    /// @copydoc get_map()
    public: PSYQ_CONSTEXPR self::map const* get_map() const PSYQ_NOEXCEPT
    {
        return const_cast<self*>(this)->get_map();
    }

    /** @brief MessagePackオブジェクトに格納されてる unordered_map を並び替え、
               map に変換する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる連想配列へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは連想配列ではない。
     */
    public: self::map const* sort_map()
    {
        switch (this->get_type())
        {
        case self::type::UNORDERED_MAP:
            this->value_.map_.sort();
            this->type_ = self::type::MAP;
            return &this->value_.map_;
        case self::type::MAP:
            return &this->value_.map_;
        default:
            return nullptr;
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief 有符号整数値のMessagePackオブジェクト種別を判定する。
        @param[in] in_integer 判定する整数値。
        @retval self::type_NEGATIVE_INTEGER 0未満の整数だった。
        @retval self::type_UNSIGNED_INTEGER 0以上の整数だった。
     */
    private: template<typename template_signed_type>
    PSYQ_CONSTEXPR static self::type tell_signed_integer_type(
        template_signed_type const in_integer)
    PSYQ_NOEXCEPT
    {
        static_assert(
            !std::is_unsigned<template_signed_type>::value,
            "template_signed_type is not signed integer type.");
        return in_integer < 0?
            self::type::NEGATIVE_INTEGER: self::type::UNSIGNED_INTEGER;
    }

    //-------------------------------------------------------------------------
    private: self::value value_; ///< @copydoc self::value
    private: self::type type_;   ///< @copydoc self::type
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace internal
    {
        /// @cond
        template<>
        inline int message_pack_object_compare(
            psyq::message_pack::object const& in_left,
            psyq::message_pack::object const& in_right)
        {
            return in_left.compare(in_right);
        }
        template<>
        inline int message_pack_object_compare(
            psyq::message_pack::object::map::value_type const& in_left,
            psyq::message_pack::object::map::value_type const& in_right)
        {
            return in_left.first.compare(in_right.first);
        }
        template<>
        inline bool message_pack_object_equal(
            psyq::message_pack::object::map::value_type const& in_left,
            psyq::message_pack::object::map::value_type const& in_right)
        {
            return in_left.first == in_right.second;
        }
        /// @endcond
    }
}

#endif // PSYQ_MESSAGE_PACK_OBJECT_HPP_
