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
    /// MessagePackオブジェクトの値。
    private: typedef psyq::internal::message_pack_value value;

    /// @copydoc psyq::internal::message_pack_value::type::value
    public: typedef psyq::internal::message_pack_value::type::value type;

    /// @name MessagePackオブジェクトが持つ値の型。
    //@{
    /// @copydoc self::value::float32
    public: typedef psyq::internal::message_pack_value::float32 float32;
    /// @copydoc self::value::float64
    public: typedef psyq::internal::message_pack_value::float64 float64;
    /// @copydoc self::value::string
    public: typedef psyq::internal::message_pack_value::string string;
    /// @copydoc self::value::binary
    public: typedef psyq::internal::message_pack_value::binary binary;
    /// @copydoc self::value::extended_binary
    public: typedef psyq::internal::message_pack_value::extended_binary
        extended_binary;
    /// @copydoc self::value::array
    public: typedef psyq::internal::message_pack_value::array array;
    /// @copydoc self::value::map
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
    public: explicit PSYQ_CONSTEXPR object(bool const in_boolean)
    PSYQ_NOEXCEPT:
        value_(in_boolean),
        type_(self::type::BOOLEAN)
    {}

    /** @brief MessagePackオブジェクトに無符号整数を格納する。
        @param[in] in_integer MessagePackオブジェクトに格納する無符号整数。
     */
    public: explicit PSYQ_CONSTEXPR object(std::uint64_t const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::type::POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: explicit PSYQ_CONSTEXPR object(unsigned long const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::type::POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: explicit PSYQ_CONSTEXPR object(unsigned int const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::type::POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: explicit PSYQ_CONSTEXPR object(unsigned short const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::type::POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: explicit PSYQ_CONSTEXPR object(unsigned char const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::type::POSITIVE_INTEGER)
    {}

    /** @brief MessagePackオブジェクトに有符号整数を格納する。
        @param[in] in_integer MessagePackオブジェクトに格納する有符号整数。
     */
    public: explicit PSYQ_CONSTEXPR object(std::int64_t const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::tell_signed_integer_type(in_integer))
    {}
    /// @copydoc object(std::int64_t const)
    public: explicit PSYQ_CONSTEXPR object(long const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::tell_signed_integer_type(in_integer))
    {}
    /// @copydoc object(std::int64_t const)
    public: explicit PSYQ_CONSTEXPR object(int const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::tell_signed_integer_type(in_integer))
    {}
    /// @copydoc object(std::int64_t const)
    public: explicit PSYQ_CONSTEXPR object(short const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::tell_signed_integer_type(in_integer))
    {}
    /// @copydoc object(std::int64_t const)
    public: explicit PSYQ_CONSTEXPR object(char const in_integer)
    PSYQ_NOEXCEPT:
        value_(static_cast<std::int64_t>(in_integer)),
        type_(self::tell_signed_integer_type(in_integer))
    {}

    /** @brief MessagePackオブジェクトに浮動小数点数を格納する。
        @param[in] in_float MessagePackオブジェクトに格納する浮動小数点数。
     */
    public: explicit PSYQ_CONSTEXPR object(self::float64 const in_float)
    PSYQ_NOEXCEPT:
        value_(in_float),
        type_(self::type::FLOAT64)
    {}
    /// @copydoc object(self::float64 const)
    public: explicit PSYQ_CONSTEXPR object(self::float32 const in_float)
    PSYQ_NOEXCEPT:
        value_(in_float),
        type_(self::type::FLOAT32)
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
    public: PSYQ_CONSTEXPR object(self::extended_binary const& in_binary)
    PSYQ_NOEXCEPT:
        value_(in_binary),
        type_(self::type::EXTENDED_BINARY)
    {}

    /** @brief MessagePackオブジェクトに配列を格納する。
        @param[in] in_array MessagePackオブジェクトに格納する配列。
     */
    public: explicit PSYQ_CONSTEXPR object(self::array const& in_array)
    PSYQ_NOEXCEPT:
        value_(in_array),
        type_(self::type::ARRAY)
    {}

    /** @brief MessagePackオブジェクトに連想配列を格納する。
        @param[in] in_map MessagePackオブジェクトに格納する連想配列。
     */
    public: explicit PSYQ_CONSTEXPR object(self::map const& in_map)
    PSYQ_NOEXCEPT:
        value_(in_map),
        type_(self::type::MAP)
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトへの値の格納
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

    /** @brief MessagePackオブジェクトに有符号整数を格納する。
        @param[in] in_integer MessagePackオブジェクトに格納する有符号整数。
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

    /** @brief MessagePackオブジェクトに浮動小数点数を格納する。
        @param[in] in_float MessagePackオブジェクトに格納する浮動小数点数。
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

    /** @brief MessagePackオブジェクトに文字列を格納する。
        @param[in] in_raw MessagePackオブジェクトに格納する文字列。
        @return *this
     */
    public: PSYQ_CONSTEXPR self& operator=(self::string const& in_string)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_string);
    }
    /** @brief MessagePackオブジェクトにバイナリを格納する。
        @param[in] in_binary MessagePackオブジェクトに格納するバイナリ。
        @return *this
     */
    public: PSYQ_CONSTEXPR self& operator=(self::binary const& in_binary)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_binary);
    }
    /** @brief MessagePackオブジェクトに拡張バイナリを格納する。
        @param[in] in_binary MessagePackオブジェクトに格納する拡張バイナリ。
        @return *this
     */
    public: PSYQ_CONSTEXPR self& operator=(
        self::extended_binary const& in_binary)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_binary);
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
    /// @name MessagePackオブジェクトに格納されてる値の操作
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

    /** @brief MessagePackオブジェクトに格納されてる正の整数を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる正の整数へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは正の整数ではない。
     */
    public: PSYQ_CONSTEXPR std::uint64_t const* get_positive_integer()
    const PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::POSITIVE_INTEGER?
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
        case self::type::POSITIVE_INTEGER:
            if ((std::numeric_limits<template_integer_type>::max)()
                < this->value_.positive_integer_)
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
            this->value_.positive_integer_);
        return true;
    }

    /** @brief MessagePackオブジェクトに格納されてる浮動小数点数を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる浮動小数点数へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは浮動小数点数ではない。
     */
    public: PSYQ_CONSTEXPR self::float64 const* get_float64()
    const PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::FLOAT64?
            &this->value_.float64_: nullptr;
    }
    /// @copydoc self::get_float64()
    public: PSYQ_CONSTEXPR self::float32 const* get_float32()
    const PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::FLOAT32?
            &this->value_.float32_: nullptr;
    }

    /** @brief MessagePackオブジェクトに格納されてる浮動小数点数を取得する。
        @param[out] out_float 取得した浮動小数点数が格納される。
        @retval true  取得に成功した。
        @retval false 取得に失敗した。
     */
    public: template<typename template_float_type>
    bool get_floating_point(template_float_type& out_float) const PSYQ_NOEXCEPT
    {
        static_assert(
            std::is_floating_point<template_float_type>::value,
            "template_float_type is not floating point type.");
        switch (this->get_type())
        {
        case self::type::FLOAT32:
            out_float = static_cast<template_float_type>(
                this->value_.float32_);
            return true;
        case self::type::FLOAT64:
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
    /** @brief MessagePackオブジェクトに格納されてる文字列を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる文字列へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは文字列ではない。
        @sa self::set_string()
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
        @sa self::set_binary()
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
        @sa self::set_extended_binary()
     */
    public: PSYQ_CONSTEXPR self::extended_binary const* get_extended_binary()
    const PSYQ_NOEXCEPT
    {
        return this->get_type() == self::type::EXTENDED_BINARY?
            &this->value_.extended_binary_: nullptr;
    }

    /** @brief MessagePackオブジェクトに文字列を格納する。
        @param[in] in_data 文字列の先頭位置。
        @param[in] in_size 文字列のバイト数。
        @return MessagePackオブジェクトに格納したRAWバイト列。
     */
    public: self::string const& set_string(
        self::string::pointer const in_data,
        self::string::size_type const in_size)
    PSYQ_NOEXCEPT
    {
        this->type_ = self::type::STRING;
        this->value_.string_.reset(in_data, in_size);
        return this->value_.string_;
    }
    /** @brief MessagePackオブジェクトにバイナリを格納する。
        @param[in] in_data バイナリの先頭位置。
        @param[in] in_size バイナリのバイト数。
        @return MessagePackオブジェクトに格納したRAWバイト列。
     */
    public: self::binary const& set_binary(
        self::binary::pointer const in_data,
        self::binary::size_type const in_size)
    PSYQ_NOEXCEPT
    {
        this->type_ = self::type::BINARY;
        this->value_.binary_.reset(in_data, in_size);
        return this->value_.binary_;
    }
    /** @brief MessagePackオブジェクトに拡張バイナリを格納する。
        @param[in] in_data
            拡張バイナリの先頭位置。先頭1バイトに型識別値が格納され、
            以後、バイナリが格納されていること。
        @param[in] in_size
            拡張バイナリのバイト数。
            型識別値の1バイトと、バイナリのバイト数を合わせたもの。
        @return MessagePackオブジェクトに格納した拡張バイナリ。
     */
    public: self::extended_binary const& set_extended_binary(
        self::extended_binary::pointer const in_data,
        self::extended_binary::size_type const in_size)
    PSYQ_NOEXCEPT
    {
        this->type_ = self::type::EXTENDED_BINARY;
        this->value_.extended_binary_.reset(in_data, in_size);
        return this->value_.extended_binary_;
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
        @sa self::set_array()
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
        this->type_ = self::type::ARRAY;
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
        return this->get_type() == self::type::MAP?
            &this->value_.map_: nullptr;
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
        this->type_ = self::type::MAP;
        this->value_.map_.reset(in_data, in_size);
        return this->value_.map_;
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief 有符号整数値のMessagePackオブジェクト種別を判定する。
        @param[in] in_integer 判定する整数値。
        @retval self::type_NEGATIVE_INTEGER 0未満の整数だった。
        @retval self::type_POSITIVE_INTEGER 0以上の整数だった。
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
            self::type::NEGATIVE_INTEGER: self::type::POSITIVE_INTEGER;
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
