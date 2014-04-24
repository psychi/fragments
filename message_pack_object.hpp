/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::message_pack::object
 */
#ifndef PSYQ_MESSAGE_PACK_OBJECT_HPP_
#define PSYQ_MESSAGE_PACK_OBJECT_HPP_

//#include "psyq/message_pack_value.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// MessagePackオブジェクト。
struct psyq::message_pack::object
{
    private: typedef object self; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// @copydoc self::value_
    private: typedef psyq::internal::message_pack_value value;

    /// @copydoc self::value::kind
    public: typedef self::value::kind kind;

    /// @name MessagePackオブジェクトが持つ値の型。
    //@{
    /// @copydoc self::value::float32
    public: typedef self::value::float32 float32;
    /// @copydoc self::value::float64
    public: typedef self::value::float64 float64;
    /// @copydoc self::value::raw
    public: typedef self::value::raw raw;
    /// @copydoc self::value::array
    public: typedef self::value::array array;
    /// @copydoc self::value::map
    public: typedef self::value::map map;
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトの構築
    //@{
    /// 空のMessagePackオブジェクトを構築する。
    public: PSYQ_CONSTEXPR object(): kind_(self::kind::NIL) {}

    /** @brief MessagePackオブジェクトに真偽値を格納する。
        @param[in] in_boolean MessagePackオブジェクトに格納する真偽値。
     */
    public: explicit PSYQ_CONSTEXPR object(bool const in_boolean)
    PSYQ_NOEXCEPT:
        value_(in_boolean),
        kind_(self::kind::BOOLEAN)
    {}

    /** @brief MessagePackオブジェクトに無符号整数を格納する。
        @param[in] in_integer MessagePackオブジェクトに格納する無符号整数。
     */
    public: explicit PSYQ_CONSTEXPR object(std::uint64_t const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        kind_(self::kind::POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: explicit PSYQ_CONSTEXPR object(unsigned long const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        kind_(self::kind::POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: explicit PSYQ_CONSTEXPR object(unsigned int const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        kind_(self::kind::POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: explicit PSYQ_CONSTEXPR object(unsigned short const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        kind_(self::kind::POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: explicit PSYQ_CONSTEXPR object(unsigned char const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        kind_(self::kind::POSITIVE_INTEGER)
    {}

    /** @brief MessagePackオブジェクトに整数を格納する。
        @param[in] in_integer MessagePackオブジェクトに格納する整数。
     */
    public: explicit PSYQ_CONSTEXPR object(std::int64_t const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        kind_(self::tell_signed_integer_kind(in_integer))
    {}
    /// @copydoc object(std::int64_t const)
    public: explicit PSYQ_CONSTEXPR object(long const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        kind_(self::tell_signed_integer_kind(in_integer))
    {}
    /// @copydoc object(std::int64_t const)
    public: explicit PSYQ_CONSTEXPR object(int const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        kind_(self::tell_signed_integer_kind(in_integer))
    {}
    /// @copydoc object(std::int64_t const)
    public: explicit PSYQ_CONSTEXPR object(short const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        kind_(self::tell_signed_integer_kind(in_integer))
    {}
    /// @copydoc object(std::int64_t const)
    public: explicit PSYQ_CONSTEXPR object(char const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        kind_(self::tell_signed_integer_kind(in_integer))
    {}

    /** @brief MessagePackオブジェクトに浮動小数点実数を格納する。
        @param[in] in_float MessagePackオブジェクトに格納する浮動小数点実数。
     */
    public: explicit PSYQ_CONSTEXPR object(self::float64 const in_float)
    PSYQ_NOEXCEPT:
        value_(in_float),
        kind_(self::kind::FLOAT64)
    {}
    /// @copydoc object(self::float64 const)
    public: explicit PSYQ_CONSTEXPR object(self::float32 const in_float)
    PSYQ_NOEXCEPT:
        value_(in_float),
        kind_(self::kind::FLOAT32)
    {}

    /** @brief MessagePackオブジェクトにRAWバイト列を格納する。
        @param[in] in_raw MessagePackオブジェクトに格納するRAWバイト列。
     */
    public: explicit PSYQ_CONSTEXPR object(self::raw const& in_raw)
    PSYQ_NOEXCEPT:
        value_(in_raw),
        kind_(self::kind::RAW)
    {}

    /** @brief MessagePackオブジェクトに配列を格納する。
        @param[in] in_array MessagePackオブジェクトに格納する配列。
     */
    public: explicit PSYQ_CONSTEXPR object(self::array const& in_array)
    PSYQ_NOEXCEPT:
        value_(in_array),
        kind_(self::kind::ARRAY)
    {}

    /** @brief MessagePackオブジェクトに連想配列を格納する。
        @param[in] in_map MessagePackオブジェクトに格納する連想配列。
     */
    public: explicit PSYQ_CONSTEXPR object(self::map const& in_map)
    PSYQ_NOEXCEPT:
        value_(in_map),
        kind_(self::kind::MAP)
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトの代入
    //@{
    /** @brief MessagePackオブジェクトに真偽値を格納する。
        @param[in] in_boolean MessagePackオブジェクトに格納する真偽値。
        @return *this
     */
    public: PSYQ_CONSTEXPR self& operator=(bool const in_boolean)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_boolean);
    }

    /** @brief MessagePackオブジェクトに無符号整数を格納する。
        @param[in] in_integer MessagePackオブジェクトに格納する無符号整数。
        @return *this
     */
    public: PSYQ_CONSTEXPR self& operator=(std::uint64_t const in_integer)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_integer);
    }
    /// @copydoc operator=(std::uint64_t const)
    public: PSYQ_CONSTEXPR self& operator=(unsigned long const in_integer)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_integer);
    }
    /// @copydoc operator=(std::uint64_t const)
    public: PSYQ_CONSTEXPR self& operator=(unsigned int const in_integer)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_integer);
    }
    /// @copydoc operator=(std::uint64_t const)
    public: PSYQ_CONSTEXPR self& operator=(unsigned short const in_integer)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_integer);
    }
    /// @copydoc operator=(std::uint64_t const)
    public: PSYQ_CONSTEXPR self& operator=(unsigned char const in_integer)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_integer);
    }

    /** @brief MessagePackオブジェクトに整数を格納する。
        @param[in] in_integer MessagePackオブジェクトに格納する整数。
        @return *this
     */
    public: PSYQ_CONSTEXPR self& operator=(std::int64_t const in_integer)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_integer);
    }
    /// @copydoc operator=(std::int64_t const)
    public: PSYQ_CONSTEXPR self& operator=(long const in_integer)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_integer);
    }
    /// @copydoc operator=(std::int64_t const)
    public: PSYQ_CONSTEXPR self& operator=(int const in_integer)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_integer);
    }
    /// @copydoc operator=(std::int64_t const)
    public: PSYQ_CONSTEXPR self& operator=(short const in_integer)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_integer);
    }
    /// @copydoc operator=(std::int64_t const)
    public: PSYQ_CONSTEXPR self& operator=(char const in_integer)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_integer);
    }

    /** @brief MessagePackオブジェクトに浮動小数点実数を格納する。
        @param[in] in_float
            MessagePackオブジェクトに格納する浮動小数点実数。
        @return *this
     */
    public: PSYQ_CONSTEXPR self& operator=(self::float64 const in_float)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_float);
    }
    /// @copydoc operator=(self::float64 const)
    public: PSYQ_CONSTEXPR self& operator=(self::float32 const in_float)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_float);
    }

    /** @brief MessagePackオブジェクトにRAWバイト列を格納する。
        @param[in] in_raw MessagePackオブジェクトに格納するRAWバイト列。
        @return *this
     */
    public: PSYQ_CONSTEXPR self& operator=(self::raw const& in_raw)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_raw);
    }

    /** @brief MessagePackオブジェクトに配列を格納する。
        @param[in] in_array MessagePackオブジェクトに格納する配列。
        @return *this
     */
    public: PSYQ_CONSTEXPR self& operator=(self::array const& in_array)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_array);
    }

    /** @brief MessagePackオブジェクトに連想配列を格納する。
        @param[in] in_map MessagePackオブジェクトに格納する連想配列。
        @return *this
     */
    public: PSYQ_CONSTEXPR self& operator=(self::map const& in_map)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_map);
    }

    /// @brief MessagePackオブジェクトを空にする。
    public: void reset() PSYQ_NOEXCEPT
    {
        new(this) self();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトに格納されてる値の操作
    //@{
    /** @brief MessagePackオブジェクトに格納されてる値の種別を取得する。
        @return @copydoc self::value::kind
     */
    public: PSYQ_CONSTEXPR self::value::kind get_kind() const PSYQ_NOEXCEPT
    {
        return this->kind_;
    }

    /** @brief MessagePackオブジェクトに格納されてる真偽値を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる真偽値へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは真偽値ではない。
     */
    public: PSYQ_CONSTEXPR bool const* get_boolean() const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind::BOOLEAN?
            &this->value_.boolean_: nullptr;
    }

    /** @brief MessagePackオブジェクトに格納されてる正の整数を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる正の整数へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは正の整数ではない。
     */
    public: PSYQ_CONSTEXPR std::uint64_t const* get_positive_integer()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind::POSITIVE_INTEGER?
            &this->value_.positive_integer_: nullptr;
    }

    /** @brief MessagePackオブジェクトに格納されてる負の整数を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる負の整数へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは負の整数ではない。
     */
    public: PSYQ_CONSTEXPR std::int64_t const* get_negative_integer()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind::NEGATIVE_INTEGER?
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
        switch (this->get_kind())
        {
        case self::kind::POSITIVE_INTEGER:
            if ((std::numeric_limits<template_integer_type>::max)()
                < this->value_.positive_integer_)
            {
                return false; // 範囲外なので失敗。
            }
            break;
        case self::kind::NEGATIVE_INTEGER:
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
            this->value_.positive_integer_);
        return true;
    }

    /** @brief MessagePackオブジェクトに格納されてる浮動小数点実数を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる浮動小数点実数へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは浮動小数点実数ではない。
     */
    public: PSYQ_CONSTEXPR self::float64 const* get_float64()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind::FLOAT64?
            &this->value_.float64_: nullptr;
    }
    /// @copydoc self::get_float64()
    public: PSYQ_CONSTEXPR self::float32 const* get_float32()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind::FLOAT32?
            &this->value_.float32_: nullptr;
    }

    /** @brief MessagePackオブジェクトに格納されてる浮動小数点実数を取得する。
        @param[out] out_float 取得した浮動小数点実数が格納される。
        @retval true  取得に成功した。
        @retval false 取得に失敗した。
     */
    public: template<typename template_float_type>
    bool get_floating_point(template_float_type& out_float) const PSYQ_NOEXCEPT
    {
        static_assert(
            std::is_floating_point<template_float_type>::value,
            "template_float_type is not floating point type.");
        switch (this->get_kind())
        {
        case self::kind::FLOAT32:
            out_float = static_cast<template_float_type>(
                this->value_.float32_);
            return true;
        case self::kind::FLOAT64:
            out_float = static_cast<template_float_type>(
                this->value_.float64_);
            return true;
        default:
            return false; // 整数以外が格納されていたので失敗。
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトに格納されてるRAWバイト列の操作
    //@{
    /** @brief MessagePackオブジェクトに格納されてるRAWバイト列を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてるRAWバイト列へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのはRAWバイト列ではない。
        @sa self::set_raw()
     */
    public: PSYQ_CONSTEXPR self::raw const* get_raw() const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind::RAW?
            &this->value_.raw_: nullptr;
    }

    /** @brief MessagePackオブジェクトにRAWバイト列を格納する。
        @param[in] in_data RAWバイト列の先頭位置。
        @param[in] in_size RAWバイト列のバイト数。
        @return MessagePackオブジェクトに格納したRAWバイト列。
     */
    public: self::raw const& set_raw(
        self::raw::pointer const in_data,
        self::raw::size_type const in_size)
    PSYQ_NOEXCEPT
    {
        this->kind_ = self::kind::RAW;
        this->value_.raw_.reset(in_data, in_size);
        return this->value_.raw_;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトに格納されてる配列の操作
    //@{
    /** @brief MessagePackオブジェクトに格納されてる配列を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる配列へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは配列ではない。
        @sa self::get_map()
     */
    public: PSYQ_CONSTEXPR self::array* get_array() PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind::ARRAY? &this->value_.array_: nullptr;
    }
    /// @copydoc get_array()
    public: PSYQ_CONSTEXPR self::array const* get_array() const PSYQ_NOEXCEPT
    {
        return const_cast<self*>(this)->get_array();
    }

    /** @brief MessagePackオブジェクトに配列を格納する。
        @param[in] in_data 配列の先頭位置。
        @param[in] in_size 配列の要素数。
        @return MessagePackオブジェクトに格納されてる配列。
     */
    public: self::array const& set_array(
        self::array::pointer const in_data,
        self::array::size_type const in_size)
    PSYQ_NOEXCEPT
    {
        this->kind_ = self::kind::ARRAY;
        this->value_.array_.reset(in_data, in_size);
        return this->value_.array_;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトに格納されてる連想配列の操作
    //@{
    /** @brief MessagePackオブジェクトに格納されてる連想配列を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる連想配列へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは連想配列ではない。
        @sa self::set_map()
     */
    public: PSYQ_CONSTEXPR self::map* get_map() PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind::MAP? &this->value_.map_: nullptr;
    }
    /// @copydoc get_map()
    public: PSYQ_CONSTEXPR self::map const* get_map() const PSYQ_NOEXCEPT
    {
        return const_cast<self*>(this)->get_map();
    }

    /** @brief MessagePackオブジェクトに配列を格納する。
        @param[in] in_data 連想配列の先頭位置。
        @param[in] in_size 連想配列の要素数。
        @return MessagePackオブジェクトに格納されてる連想配列。
     */
    public: self::map const& set_map(
        self::map::pointer const in_data,
        self::map::size_type const in_size)
    PSYQ_NOEXCEPT
    {
        this->kind_ = self::kind::MAP;
        this->value_.map_.reset(in_data, in_size);
        return this->value_.map_;
    }
    //@}
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
        return self::value::equal_value(
            this->value_,
            this->get_kind(),
            in_right.value_,
            in_right.get_kind());
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

    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと比較する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @return 左辺 < 右辺
     */
    public: bool operator<(self const& in_right) const PSYQ_NOEXCEPT
    {
        return this->compare(in_right) < 0;
    }

    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと比較する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @return 左辺 <= 右辺
     */
    public: bool operator<=(self const& in_right) const PSYQ_NOEXCEPT
    {
        return this->compare(in_right) <= 0;
    }

    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと比較する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @return 左辺 > 右辺
     */
    public: bool operator>(self const& in_right) const PSYQ_NOEXCEPT
    {
        return 0 < this->compare(in_right);
    }

    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと比較する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @return 左辺 >= 右辺
     */
    public: bool operator>=(self const& in_right) const PSYQ_NOEXCEPT
    {
        return 0 <= this->compare(in_right);
    }

    /** @brief *thisとMessagePackオブジェクトを比較する。
        @param[in] in_object 比較するMessagePackオブジェクト。
        @retval 正 *thisのほうが大きい。
        @retval 0  等値。
        @retval 負 *thisのほうが小さい。
     */
    public: int compare(self const& in_object) const PSYQ_NOEXCEPT
    {
        return self::value::compare_value(
            this->value_,
            this->get_kind(),
            in_object.value_,
            in_object.get_kind());
    }
    //@}
    //-------------------------------------------------------------------------
    public: bool serialize(std::basic_ostream<char>& out_stream) const
    {
        switch (this->get_kind())
        {
        case self::kind::NIL:
            out_stream << std::uint8_t(0xc0);
            return true;
        case self::kind::BOOLEAN:
            out_stream << std::uint8_t(this->value_.boolean_? 0xc3: 0xc2);
            return true;
        case self::kind::POSITIVE_INTEGER:
            self::serialize_positive_integer(
                out_stream, this->value_.positive_integer_);
            return true;
        case self::kind::NEGATIVE_INTEGER:
            self::serialize_negative_integer(
                out_stream, this->value_.negative_integer_);
            return true;
        case self::kind::FLOAT32:
            self::serialize_uint(out_stream, 0xca, this->value_.uint32_);
            return true;
        case self::kind::FLOAT64:
            self::serialize_uint(out_stream, 0xcb, this->value_.positive_integer_);
            return true;
        case self::kind::RAW:
            self::serialize_raw(out_stream, this->value_.raw_);
            return true;
        case self::kind::ARRAY:
            return self::serialize_array(out_stream, this->value_.array_);
        case self::kind::MAP:
            return self::serialize_map(out_stream, this->value_.map_);
        default:
            PSYQ_ASSERT(false);
            return false;
        }
    }

    private: template<typename template_out_stream>
    static void serialize_positive_integer(
        template_out_stream& out_stream,
        std::uint64_t const in_integer)
    {
        if (in_integer <= 0x7f)
        {
            out_stream << static_cast<std::uint8_t>(in_integer);
        }
        else if (in_integer <= (std::numeric_limits<std::uint8_t>::min)())
        {
            self::serialize_uint(
                out_stream, 0xcc, static_cast<std::uint8_t>(in_integer));
        }
        else if (in_integer <= (std::numeric_limits<std::uint16_t>::min)())
        {
            self::serialize_uint(
                out_stream, 0xcd, static_cast<std::uint16_t>(in_integer));
        }
        else if (in_integer <= (std::numeric_limits<std::uint32_t>::min)())
        {
            self::serialize_uint(
                out_stream, 0xce, static_cast<std::uint32_t>(in_integer));
        }
        else
        {
            self::serialize_uint(out_stream, 0xcf, in_integer);
        }
    }

    private: template<typename template_out_stream>
    static void serialize_negative_integer(
        template_out_stream& out_stream,
        std::int64_t const in_integer)
    {
        PSYQ_ASSERT(in_integer < 0);
        if (-0x20 <= in_integer)
        {
            out_stream << static_cast<std::uint8_t>(in_integer);
        }
        else if ((std::numeric_limits<std::int8_t>::min)() <= in_integer)
        {
            self::serialize_uint(
                out_stream, 0xd0, static_cast<std::uint8_t>(in_integer));
        }
        else if ((std::numeric_limits<std::int16_t>::min)() <= in_integer)
        {
            self::serialize_uint(
                out_stream, 0xd1, static_cast<std::uint16_t>(in_integer));
        }
        else if ((std::numeric_limits<std::int32_t>::min)() <= in_integer)
        {
            self::serialize_uint(
                out_stream, 0xd2, static_cast<std::uint32_t>(in_integer));
        }
        else
        {
            self::serialize_uint(
                out_stream, 0xd3, static_cast<std::uint64_t>(in_integer));
        }
    }

    private: template<typename template_out_stream>
    static void serialize_raw(
        template_out_stream& out_stream,
        self::raw const& in_raw)
    {
        // RAWバイト列の大きさを書き出す。
        std::uint32_t const local_size(in_raw.size());
        if (local_size <= 0x1f)
        {
            out_stream << std::uint8_t(0xa0 + (local_size & 0x1f));
        }
        else if (local_size <= 0xff)
        {
            self::serialize_uint(
                out_stream, 0xd9, static_cast<std::uint8_t>(local_size));
        }
        else if (local_size <= 0xffff)
        {
            self::serialize_uint(
                out_stream, 0xda, static_cast<std::uint16_t>(local_size));
        }
        else
        {
            PSYQ_ASSERT(in_raw.size() <= 0xffffffff);
            self::serialize_uint(out_stream, 0xdb, local_size);
        }

        // RAWバイト列を書き出す。
        typedef typename template_out_stream::char_type const* const_pointer;
        out_stream.write(
            reinterpret_cast<const_pointer>(in_raw.data()), local_size);
    }

    private: template<typename template_out_stream>
    static bool serialize_array(
        template_out_stream& out_stream,
        self::array const& in_array)
    {
        // 配列の大きさを書き出す。
        std::uint32_t const local_size(in_array.size());
        if (local_size <= 0xf)
        {
            out_stream << std::uint8_t(0x90 + (local_size & 0xf));
        }
        else if (local_size <= 0xffff)
        {
            self::serialize_uint(
                out_stream, 0xdc, static_cast<std::uint16_t>(local_size));
        }
        else
        {
            PSYQ_ASSERT(in_array.size() <= 0xffffffff);
            self::serialize_uint(out_stream, 0xdd, local_size);
        }

        // 配列を書き出す。
        for (auto& local_object: in_array)
        {
            if (!local_object.serialize(out_stream))
            {
                return false;
            }
        }
        return true;
    }

    private: template<typename template_out_stream>
    static bool serialize_map(
        template_out_stream& out_stream,
        self::map const& in_map)
    {
        // 配列の大きさを書き出す。
        std::uint32_t const local_size(in_map.size());
        if (local_size <= 0xf)
        {
            out_stream << std::uint8_t(0x80 + (local_size & 0xf));
        }
        else if (local_size <= 0xffff)
        {
            self::serialize_uint(
                out_stream, 0xde, static_cast<std::uint16_t>(local_size));
        }
        else
        {
            PSYQ_ASSERT(in_map.size() <= 0xffffffff);
            self::serialize_uint(out_stream, 0xdf, local_size);
        }

        // 配列を書き出す。
        for (auto& local_element: in_map)
        {
            if (!local_element.first.serialize(out_stream)
                || !local_element.second.serialize(out_stream))
            {
                return false;
            }
        }
        return true;
    }

    private: template<typename template_out_stream>
    static void serialize_uint(
        template_out_stream& out_stream,
        std::uint8_t const in_header,
        std::uint8_t const in_integer)
    {
        out_stream << in_header << in_integer;
    }

    private: template<typename template_out_stream>
    static void serialize_uint(
        template_out_stream& out_stream,
        std::uint8_t const in_header,
        std::uint16_t const in_integer)
    {
        out_stream << in_header
            << static_cast<std::uint8_t>(in_integer >> 8)
            << static_cast<std::uint8_t>(in_integer);
    }

    private: template<typename template_out_stream>
    static void serialize_uint(
        template_out_stream& out_stream,
        std::uint8_t const in_header,
        std::uint32_t const in_integer)
    {
        out_stream << in_header
            << static_cast<std::uint8_t>(in_integer >> 24)
            << static_cast<std::uint8_t>(in_integer >> 16)
            << static_cast<std::uint8_t>(in_integer >>  8)
            << static_cast<std::uint8_t>(in_integer);
    }

    private: template<typename template_out_stream>
    static void serialize_uint(
        template_out_stream& out_stream,
        std::uint8_t const in_header,
        std::uint64_t const in_integer)
    {
        out_stream << in_header
            << static_cast<std::uint8_t>(in_integer >> 56)
            << static_cast<std::uint8_t>(in_integer >> 48)
            << static_cast<std::uint8_t>(in_integer >> 40)
            << static_cast<std::uint8_t>(in_integer >> 32)
            << static_cast<std::uint8_t>(in_integer >> 24)
            << static_cast<std::uint8_t>(in_integer >> 16)
            << static_cast<std::uint8_t>(in_integer >>  8)
            << static_cast<std::uint8_t>(in_integer);
    }

    /** @brief 有符号整数値のMessagePackオブジェクト種別を判定する。
        @param[in] in_integer 判定する整数値。
        @retval self::kind_NEGATIVE_INTEGER 0未満の整数だった。
        @retval self::kind_POSITIVE_INTEGER 0以上の整数だった。
     */
    private: template<typename template_signed_type>
    PSYQ_CONSTEXPR static self::kind tell_signed_integer_kind(
        template_signed_type const in_integer)
    PSYQ_NOEXCEPT
    {
        static_assert(
            !std::is_unsigned<template_signed_type>::value,
            "template_signed_type is not signed integer type.");
        return in_integer < 0?
            self::kind::NEGATIVE_INTEGER: self::kind::POSITIVE_INTEGER;
    }
    //-------------------------------------------------------------------------
    private: self::value value_;      ///< MessagePackオブジェクトの値。
    private: self::value::kind kind_; ///< @copydoc self::value::kind
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
