/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::message_pack::object
 */
#ifndef PSYQ_MESSAGE_PACK_OBJECT_HPP_
#define PSYQ_MESSAGE_PACK_OBJECT_HPP_

//#include "psyq/message_pack_object.hpp"

#ifndef PSYQ_MESSAGE_PACK_OBJECT_FLOAT32_EPSILON
#define PSYQ_MESSAGE_PACK_OBJECT_FLOAT32_EPSILON\
    std::numeric_limits<self::float32>::epsilon() * 4
#endif // !defined(PSYQ_MESSAGE_PACK_OBJECT_FLOAT32_EPSILON)

#ifndef PSYQ_MESSAGE_PACK_OBJECT_FLOAT64_EPSILON
#define PSYQ_MESSAGE_PACK_OBJECT_FLOAT64_EPSILON\
    std::numeric_limits<self::float64>::epsilon() * 4
#endif // !defined(PSYQ_MESSAGE_PACK_OBJECT_FLOAT64_EPSILON)

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// MessagePackオブジェクト。
struct psyq::message_pack::object
{
    private: typedef object self; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /** @brief MessagePackオブジェクトに格納されてる値の種別。
        @sa self::get_kind()
     */
    public: enum kind: std::uint8_t
    {
        kind_NIL,              ///< 空。
        kind_BOOLEAN,          ///< 真偽値。
        kind_POSITIVE_INTEGER, ///< 0以上の整数。
        kind_NEGATIVE_INTEGER, ///< 0未満の整数。
        kind_FLOAT32,          ///< IEEE754単精度浮動小数点実数。
        kind_FLOAT64,          ///< IEEE754倍精度浮動小数点実数。
        kind_RAW,              ///< RAWバイト列。
        kind_ARRAY,            ///< MessagePackオブジェクトの配列。
        kind_MAP,              ///< MessagePackオブジェクトの連想配列。
    };

    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトに格納する型
    //@{
    /// @copydoc self::kind_FLOAT32
    public: typedef float float32;
    /// @copydoc self::kind_FLOAT64
    public: typedef double float64;
    /// @copydoc self::kind_RAW
    public: typedef psyq::internal::message_pack_object_container<char const>
        raw;
    /// @copydoc self::kind_ARRAY
    public: typedef psyq::internal::message_pack_object_container<self> array;
    /// @copydoc self::kind_MAP
    public: typedef psyq::internal
        ::message_pack_object_map<std::pair<self, self>>
            map;
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトの構築と破棄
    //@{
    /// 空のMessagePackオブジェクトを構築する。
    public: PSYQ_CONSTEXPR object(): kind_(self::kind_NIL) {}

    /** @brief MessagePackオブジェクトに真偽値を格納する。
        @param[in] in_boolean MessagePackオブジェクトに格納する真偽値。
     */
    public: explicit PSYQ_CONSTEXPR object(bool const in_boolean)
    PSYQ_NOEXCEPT:
        boolean_(in_boolean),
        kind_(self::kind_BOOLEAN)
    {}

    /** @brief MessagePackオブジェクトに無符号整数を格納する。
        @param[in] in_integer MessagePackオブジェクトに格納する無符号整数。
     */
    public: explicit PSYQ_CONSTEXPR object(std::uint64_t const in_integer)
    PSYQ_NOEXCEPT:
        positive_integer_(in_integer),
        kind_(self::kind_POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: explicit PSYQ_CONSTEXPR object(unsigned long const in_integer)
    PSYQ_NOEXCEPT:
        positive_integer_(in_integer),
        kind_(self::kind_POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: explicit PSYQ_CONSTEXPR object(unsigned int const in_integer)
    PSYQ_NOEXCEPT:
        positive_integer_(in_integer),
        kind_(self::kind_POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: explicit PSYQ_CONSTEXPR object(unsigned short const in_integer)
    PSYQ_NOEXCEPT:
        positive_integer_(in_integer),
        kind_(self::kind_POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: explicit PSYQ_CONSTEXPR object(unsigned char const in_integer)
    PSYQ_NOEXCEPT:
        positive_integer_(in_integer),
        kind_(self::kind_POSITIVE_INTEGER)
    {}

    /** @brief MessagePackオブジェクトに整数を格納する。
        @param[in] in_integer MessagePackオブジェクトに格納する整数。
     */
    public: explicit PSYQ_CONSTEXPR object(std::int64_t const in_integer)
    PSYQ_NOEXCEPT:
        negative_integer_(in_integer),
        kind_(self::tell_signed_integer_kind(in_integer))
    {}
    /// @copydoc object(std::int64_t const)
    public: explicit PSYQ_CONSTEXPR object(long const in_integer)
    PSYQ_NOEXCEPT:
        negative_integer_(in_integer),
        kind_(self::tell_signed_integer_kind(in_integer))
    {}
    /// @copydoc object(std::int64_t const)
    public: explicit PSYQ_CONSTEXPR object(int const in_integer)
    PSYQ_NOEXCEPT:
        negative_integer_(in_integer),
        kind_(self::tell_signed_integer_kind(in_integer))
    {}
    /// @copydoc object(std::int64_t const)
    public: explicit PSYQ_CONSTEXPR object(short const in_integer)
    PSYQ_NOEXCEPT:
        negative_integer_(in_integer),
        kind_(self::tell_signed_integer_kind(in_integer))
    {}
    /// @copydoc object(std::int64_t const)
    public: explicit PSYQ_CONSTEXPR object(char const in_integer)
    PSYQ_NOEXCEPT:
        negative_integer_(in_integer),
        kind_(self::tell_signed_integer_kind(in_integer))
    {}

    /** @brief MessagePackオブジェクトに浮動小数点実数を格納する。
        @param[in] in_float MessagePackオブジェクトに格納する浮動小数点実数。
     */
    public: explicit PSYQ_CONSTEXPR object(self::float64 const in_float)
    PSYQ_NOEXCEPT:
        float64_(in_float),
        kind_(self::kind_FLOAT64)
    {}
    /// @copydoc object(self::float64 const)
    public: explicit PSYQ_CONSTEXPR object(self::float32 const in_float)
    PSYQ_NOEXCEPT:
        float32_(in_float),
        kind_(self::kind_FLOAT32)
    {}

    /** @brief MessagePackオブジェクトにRAWバイト列を格納する。
        @param[in] in_raw MessagePackオブジェクトに格納するRAWバイト列。
     */
    public: explicit PSYQ_CONSTEXPR object(self::raw const& in_raw)
    PSYQ_NOEXCEPT:
        raw_(in_raw),
        kind_(self::kind_RAW)
    {}

    /** @brief MessagePackオブジェクトに配列を格納する。
        @param[in] in_array MessagePackオブジェクトに格納する配列。
     */
    public: explicit PSYQ_CONSTEXPR object(self::array const& in_array)
    PSYQ_NOEXCEPT:
        array_(in_array),
        kind_(self::kind_ARRAY)
    {}

    /** @brief MessagePackオブジェクトに連想配列を格納する。
        @param[in] in_map MessagePackオブジェクトに格納する連想配列。
     */
    public: explicit PSYQ_CONSTEXPR object(self::map const& in_map)
    PSYQ_NOEXCEPT:
        map_(in_map),
        kind_(self::kind_MAP)
    {}

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
        @return @copydoc self::kind
     */
    public: PSYQ_CONSTEXPR self::kind get_kind() const PSYQ_NOEXCEPT
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
        return this->get_kind() == self::kind_BOOLEAN?
            &this->boolean_: nullptr;
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
        return this->get_kind() == self::kind_POSITIVE_INTEGER?
            &this->positive_integer_: nullptr;
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
        return this->get_kind() == self::kind_NEGATIVE_INTEGER?
            &this->negative_integer_: nullptr;
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
        case self::kind_POSITIVE_INTEGER:
            if ((std::numeric_limits<template_integer_type>::max)()
                < this->positive_integer_)
            {
                return false; // 範囲外なので失敗。
            }
            break;
        case self::kind_NEGATIVE_INTEGER:
            if (this->negative_integer_
                < (std::numeric_limits<template_integer_type>::min)())
            {
                return false; // 範囲外なので失敗。
            }
            break;
        default:
            return false; // 整数以外が格納されていたので失敗。
        }
        out_integer
            = static_cast<template_integer_type>(this->positive_integer_);
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
        return this->get_kind() == self::kind_FLOAT64?
            &this->float64_: nullptr;
    }
    /// @copydoc self::get_float64()
    public: PSYQ_CONSTEXPR self::float32 const* get_float32()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind_FLOAT32?
            &this->float32_: nullptr;
    }

    /** @brief MessagePackオブジェクトに格納されてる浮動小数点実数を取得する。
        @param[out] out_integer 取得した整数が格納される。
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
        case self::kind_FLOAT32:
            out_float = static_cast<template_float_type>(this->float32_);
            return true;
        case self::kind_FLOAT64:
            out_float = static_cast<template_float_type>(this->float64_);
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
        return this->get_kind() == self::kind_RAW? &this->raw_: nullptr;
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
        this->kind_ = psyq::message_pack::object::kind_RAW;
        this->raw_.reset(in_data, in_size);
        return this->raw_;
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
        return this->get_kind() == self::kind_ARRAY? &this->array_: nullptr;
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
        this->kind_ = psyq::message_pack::object::kind_ARRAY;
        this->array_.reset(in_data, in_size);
        return this->array_;
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
        return this->get_kind() == self::kind_MAP? &this->map_: nullptr;
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
        this->kind_ = psyq::message_pack::object::kind_MAP;
        this->map_.reset(in_data, in_size);
        return this->map_;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトの比較
    //@{
    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと等値か判定する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @retval true  等値だった。
        @retval false 非等値だった 。
     */
    public: bool operator==(self const& in_right) const PSYQ_NOEXCEPT
    {
        if (this->get_kind() != in_right.get_kind())
        {
            return false;
        }
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return true;
        case self::kind_BOOLEAN:
            return this->boolean_ == in_right.boolean_;
        case self::kind_POSITIVE_INTEGER:
            return this->positive_integer_ == in_right.positive_integer_;
        case self::kind_NEGATIVE_INTEGER:
            return this->negative_integer_ == in_right.negative_integer_;
        case self::kind_FLOAT32:
            return 0 == self::compare_floating_point(
                this->float32_,
                in_right.float32_,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT32_EPSILON);
        case self::kind_FLOAT64:
            return 0 == self::compare_floating_point(
                this->float64_,
                in_right.float64_,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT64_EPSILON);
        case self::kind_RAW:
            return this->raw_ == in_right.raw_;
        case self::kind_ARRAY:
            return this->array_ == in_right.array_;
        case self::kind_MAP:
            return this->map_ == in_right.map_;
        default:
            PSYQ_ASSERT(false);
            return false;
        }
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
        switch (in_object.get_kind())
        {
        case self::kind_NIL:
            return this->get_kind() != self::kind_NIL? 1: 0;
        case self::kind_BOOLEAN:
            return this->compare(in_object.boolean_);
        case self::kind_POSITIVE_INTEGER:
            return this->compare(in_object.positive_integer_);
        case self::kind_NEGATIVE_INTEGER:
            return this->compare(in_object.negative_integer_);
        case self::kind_FLOAT32:
            return this->compare(in_object.float32_);
        case self::kind_FLOAT64:
            return this->compare(in_object.float64_);
        case self::kind_RAW:
            return this->compare(in_object.raw_);
        case self::kind_ARRAY:
            return this->compare(in_object.array_);
        case self::kind_MAP:
            return this->compare(in_object.map_);
        default:
            PSYQ_ASSERT(false);
            return self::is_valid_kind(this->get_kind())? -1: 0;
        }
    }

    /** @brief *thisとMessagePackオブジェクト連想配列を比較する。
        @param[in] in_map 比較するMessagePackオブジェクト連想配列。
        @retval 正 *thisのほうが大きい。
        @retval 0  等値。
        @retval 負 *thisのほうが小さい。
     */
    public: int compare(self::map const& in_map) const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return -1;//-self::compare_map(in_map, this->boolean_);
        case self::kind_POSITIVE_INTEGER:
            return -1;//-self::compare_map(in_map, this->positive_integer_);
        case self::kind_NEGATIVE_INTEGER:
            return -1;//-self::compare_map(in_map, this->negative_integer_);
        case self::kind_FLOAT32:
            return -1;//-self::compare_map(in_map, this->float32_);
        case self::kind_FLOAT64:
            return -1;//-self::compare_map(in_map, this->float64_);
        case self::kind_RAW:
            return -1;//-self::compare_map(in_map, this->raw_);
        case self::kind_ARRAY:
            return -1;//-self::compare_map(in_map, this->array_);
        case self::kind_MAP:
            return this->map_.compare(in_map);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief *thisとMessagePackオブジェクト配列を比較する。
        @param[in] in_array 比較するMessagePackオブジェクト配列。
        @retval 正 *thisのほうが大きい。
        @retval 0  等値。
        @retval 負 *thisのほうが小さい。
     */
    public: int compare(self::array const& in_array) const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return -1;//-self::compare_array(in_array, this->boolean_);
        case self::kind_POSITIVE_INTEGER:
            return -1;//-self::compare_array(in_array, this->positive_integer_);
        case self::kind_NEGATIVE_INTEGER:
            return -1;//-self::compare_array(in_array, this->negative_integer_);
        case self::kind_FLOAT32:
            return -1;//-self::compare_array(in_array, this->float32_);
        case self::kind_FLOAT64:
            return -1;//-self::compare_array(in_array, this->float64_);
        case self::kind_RAW:
            return -1;//-self::compare_array(in_array, this->raw_);
        case self::kind_ARRAY:
            return this->array_.compare(in_array);
        case self::kind_MAP:
            return 1;//self::compare_map(this->map_, in_array);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief *thisとRAWバイト列を比較する。
        @param[in] in_raw 比較するRAWバイト列。
        @retval 正 *thisのほうが大きい。
        @retval 0  等値。
        @retval 負 *thisのほうが小さい。
     */
    public: int compare(self::raw const& in_raw) const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return -1;//-self::compare_raw(in_raw, this->boolean_);
        case self::kind_POSITIVE_INTEGER:
            return -1;//-self::compare_raw(in_raw, this->positive_integer_);
        case self::kind_NEGATIVE_INTEGER:
            return -1;//-self::compare_raw(in_raw, this->negative_integer_);
        case self::kind_FLOAT32:
            return -1;//-self::compare_raw(in_raw, this->float32_);
        case self::kind_FLOAT64:
            return -1;//-self::compare_raw(in_raw, this->float64_);
        case self::kind_RAW:
            return this->raw_.compare(in_raw);
        case self::kind_ARRAY:
            return 1;//self::compare_array(this->array_, in_raw);
        case self::kind_MAP:
            return 1;//self::compare_map(this->map_, in_raw);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief *thisと浮動小数点実数を比較する。
        @param[in] in_float 比較する浮動小数点実数。
        @retval 正 *thisのほうが大きい。
        @retval 0  等値。
        @retval 負 *thisのほうが小さい。
     */
    public: int compare(self::float64 const in_float)
    const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return -self::compare_floating_point(
                in_float,
                this->boolean_,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT64_EPSILON);
        case self::kind_POSITIVE_INTEGER:
            return -self::compare_floating_point(
                in_float,
                this->positive_integer_,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT64_EPSILON);
        case self::kind_NEGATIVE_INTEGER:
            return -self::compare_floating_point(
                in_float,
                this->negative_integer_,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT64_EPSILON);
        case self::kind_FLOAT32:
            return self::compare_floating_point<self::float64>(
                this->float32_,
                in_float,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT32_EPSILON);
        case self::kind_FLOAT64:
            return self::compare_floating_point(
                this->float64_,
                in_float,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT64_EPSILON);
        case self::kind_RAW:
            return 1;//self::compare_raw(this->raw_, in_float);
        case self::kind_ARRAY:
            return 1;//self::compare_array(this->array_, in_float);
        case self::kind_MAP:
            return 1;//self::compare_map(this->map_, in_float);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }
    /// @copydoc self::compare(self::float32 const)
    public: int compare(self::float32 const in_float)
    const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return -self::compare_floating_point(
                in_float,
                this->boolean_,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT32_EPSILON);
        case self::kind_POSITIVE_INTEGER:
            return -self::compare_floating_point(
                in_float,
                this->positive_integer_,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT32_EPSILON);
        case self::kind_NEGATIVE_INTEGER:
            return -self::compare_floating_point(
                in_float,
                this->negative_integer_,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT32_EPSILON);
        case self::kind_FLOAT32:
            return self::compare_floating_point(
                this->float32_,
                in_float,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT32_EPSILON);
        case self::kind_FLOAT64:
            return self::compare_floating_point<self::float64>(
                this->float64_,
                in_float,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT32_EPSILON);
        case self::kind_RAW:
            return 1;//self::compare_raw(this->raw_, in_float);
        case self::kind_ARRAY:
            return 1;//self::compare_array(this->array_, in_float);
        case self::kind_MAP:
            return 1;//self::compare_map(this->map_, in_float);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief *thisと有符号整数を比較する。
        @param[in] in_signed_integer 比較する有符号整数。
        @retval 正 *thisのほうが大きい。
        @retval 0  等値。
        @retval 負 *thisのほうが小さい。
     */
    public: int compare(std::int64_t const in_signed_integer)
    const PSYQ_NOEXCEPT
    {
        return this->compare_signed_integer(in_signed_integer);
    }
    // @copydoc self::compare(std::int64_t const)
    public: int compare(long const in_signed_integer) const PSYQ_NOEXCEPT
    {
        return this->compare_signed_integer(in_signed_integer);
    }
    // @copydoc self::compare(std::int64_t const)
    public: int compare(int const in_signed_integer) const PSYQ_NOEXCEPT
    {
        return this->compare_signed_integer(in_signed_integer);
    }
    // @copydoc self::compare(std::int64_t const)
    public: int compare(short const in_signed_integer) const PSYQ_NOEXCEPT
    {
        return this->compare_signed_integer(in_signed_integer);
    }
    // @copydoc self::compare(std::int64_t const)
    public: int compare(char const in_signed_integer) const PSYQ_NOEXCEPT
    {
        return this->compare_signed_integer(in_signed_integer);
    }

    /** @brief *thisと無符号整数を比較する。
        @param[in] in_unsigned_integer 比較する無符号整数。
        @retval 正 *thisのほうが大きい。
        @retval 0  等値。
        @retval 負 *thisのほうが小さい。
     */
    public: int compare(std::uint64_t const in_unsigned_integer)
    const PSYQ_NOEXCEPT
    {
        return this->compare_unsigned_integer(in_unsigned_integer);
    }
    // @copydoc self::compare(std::uint64_t const)
    public: int compare(unsigned long const in_unsigned_integer)
    const PSYQ_NOEXCEPT
    {
        return this->compare_unsigned_integer(in_unsigned_integer);
    }
    // @copydoc self::compare(std::uint64_t const)
    public: int compare(unsigned int const in_unsigned_integer)
    const PSYQ_NOEXCEPT
    {
        return this->compare_unsigned_integer(in_unsigned_integer);
    }
    // @copydoc self::compare(std::uint64_t const)
    public: int compare(unsigned short const in_unsigned_integer)
    const PSYQ_NOEXCEPT
    {
        return this->compare_unsigned_integer(in_unsigned_integer);
    }
    // @copydoc self::compare(std::uint64_t const)
    public: int compare(unsigned char const in_unsigned_integer)
    const PSYQ_NOEXCEPT
    {
        return this->compare_unsigned_integer(in_unsigned_integer);
    }

    /** @brief *thisと真偽値を比較する。
        @param[in] in_boolean 比較する真偽値。
        @retval 正 *thisのほうが大きい。
        @retval 0  等値。
        @retval 負 *thisのほうが小さい。
     */
    public: int compare(bool const in_boolean) const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return self::compare_boolean(this->boolean_, in_boolean);
        case self::kind_POSITIVE_INTEGER:
            return self::compare_unsigned_integer(
                this->positive_integer_, in_boolean);
        case self::kind_NEGATIVE_INTEGER:
            return -1;
        case self::kind_FLOAT32:
            return self::compare_floating_point(
                this->float32_,
                in_boolean,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT32_EPSILON);
        case self::kind_FLOAT64:
            return self::compare_floating_point(
                this->float64_,
                in_boolean,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT64_EPSILON);
        case self::kind_RAW:
            return 1;//self::compare_raw(this->raw_, in_boolean);
        case self::kind_ARRAY:
            return 1;//self::compare_array(this->array_, in_boolean);
        case self::kind_MAP:
            return 1;//self::compare_map(this->map_, in_boolean);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 浮動小数点実数との比較
    //@{
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
    //@}
    //-------------------------------------------------------------------------
    /// @name 有符号整数との比較
    //@{
    private: template<typename template_signed_type>
    int compare_signed_integer(template_signed_type const in_signed_integer)
    const PSYQ_NOEXCEPT
    {
        static_assert(
            !std::is_unsigned<template_signed_type>::value,
            "template_signed_type is not signed integer type.");
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return -self::compare_signed_integer(
                in_signed_integer, this->boolean_);
        case self::kind_POSITIVE_INTEGER:
            return -self::compare_signed_integer(
                in_signed_integer, this->positive_integer_);
        case self::kind_NEGATIVE_INTEGER:
            return -self::compare_signed_integer(
                in_signed_integer, this->negative_integer_);
        case self::kind_FLOAT32:
            return self::compare_floating_point<self::float64>(
                this->float32_,
                static_cast<self::float64>(in_signed_integer),
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT32_EPSILON);
        case self::kind_FLOAT64:
            return self::compare_floating_point<self::float64>(
                this->float64_,
                static_cast<self::float64>(in_signed_integer),
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT64_EPSILON);
        case self::kind_RAW:
            return 1;//self::compare_raw(this->raw_, in_signed_integer);
        case self::kind_ARRAY:
            return 1;//self::compare_array(this->array_, in_signed_integer);
        case self::kind_MAP:
            return 1;//self::compare_map(this->map_, in_signed_integer);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

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

    private: template<typename template_signed_type>
    static int compare_signed_integer(
        template_signed_type const in_left,
        std::int64_t const in_right)
    PSYQ_NOEXCEPT
    {
        return in_left < in_right? -1: (in_right < in_left? 1: 0);
    }

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
            self::kind_NEGATIVE_INTEGER: self::kind_POSITIVE_INTEGER;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 無符号整数との比較
    //@{
    private: template<typename template_unsigned_type>
    int compare_unsigned_integer(
        template_unsigned_type const in_unsigned_integer)
    const PSYQ_NOEXCEPT
    {
        static_assert(
            std::is_unsigned<template_unsigned_type>::value,
            "template_unsigned_type is not unsigned integer type.");
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return -self::compare_unsigned_integer(
                in_unsigned_integer, this->boolean_);
        case self::kind_POSITIVE_INTEGER:
            return -self::compare_unsigned_integer(
                in_unsigned_integer, this->positive_integer_);
        case self::kind_NEGATIVE_INTEGER:
            return -1;
        case self::kind_FLOAT32:
            return -self::compare_unsigned_integer(
                in_unsigned_integer,
                static_cast<self::float64>(this->float32_),
                static_cast<self::float64>(
                    PSYQ_MESSAGE_PACK_OBJECT_FLOAT32_EPSILON));
        case self::kind_FLOAT64:
            return -self::compare_unsigned_integer(
                in_unsigned_integer,
                this->float64_,
                PSYQ_MESSAGE_PACK_OBJECT_FLOAT64_EPSILON);
        case self::kind_RAW:
            return 1;//self::compare_raw(this->raw_, in_unsigned_integer);
        case self::kind_ARRAY:
            return 1;//self::compare_array(this->array_, in_unsigned_integer);
        case self::kind_MAP:
            return 1;//self::compare_map(this->map_, in_unsigned_integer);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

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

    private: template<typename template_unsigned_type>
    static int compare_unsigned_integer(
        template_unsigned_type const in_left,
        std::uint64_t const in_right)
    PSYQ_NOEXCEPT
    {
        return in_left < in_right? -1: (in_right < in_left? 1: 0);
    }

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
    private: static int compare_boolean(
        bool const in_left,
        bool const in_right)
    PSYQ_NOEXCEPT
    {
        return in_left == in_right? 0: (in_right? 1: -1);
    }
    //@}
    //-------------------------------------------------------------------------
    private: static bool is_valid_kind(self::kind const in_kind)
    {
        return in_kind <= self::kind_MAP;
    }

    //-------------------------------------------------------------------------
    public: bool serialize(std::basic_ostream<char>& out_stream) const
    {
        switch (this->get_kind())
        {
        case self::kind_NIL:
            out_stream << std::uint8_t(0xc0);
            return true;
        case self::kind_BOOLEAN:
            out_stream << std::uint8_t(this->boolean_? 0xc3: 0xc2);
            return true;
        case self::kind_POSITIVE_INTEGER:
            self::serialize_positive_integer(
                out_stream, this->positive_integer_);
            return true;
        case self::kind_NEGATIVE_INTEGER:
            self::serialize_negative_integer(
                out_stream, this->negative_integer_);
            return true;
        case self::kind_FLOAT32:
            self::serialize_uint(out_stream, 0xca, this->uint32_);
            return true;
        case self::kind_FLOAT64:
            self::serialize_uint(out_stream, 0xcb, this->positive_integer_);
            return true;
        case self::kind_RAW:
            self::serialize_raw(out_stream, this->raw_);
            return true;
        case self::kind_ARRAY:
            return self::serialize_array(out_stream, this->array_);
        case self::kind_MAP:
            return self::serialize_map(out_stream, this->map_);
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

    //-------------------------------------------------------------------------
    private: union
    {
        /// @copydoc self::kind_BOOLEAN
        bool boolean_;
        /// @copydoc self::kind_POSITIVE_INTEGER
        std::uint64_t positive_integer_;
        /// @copydoc self::kind_NEGATIVE_INTEGER
        std::int64_t negative_integer_;
        std::uint32_t uint32_;
        /// @copydoc self::kind_FLOAT32
        self::float32 float32_;
        /// @copydoc self::kind_FLOAT64
        self::float64 float64_;
        /// @copydoc self::kind_RAW
        self::raw raw_;
        /// @copydoc self::kind_ARRAY
        self::array array_;
        /// @copydoc self::kind_MAP
        self::map map_;
    };
    private: self::kind kind_; ///< @copydoc self::kind
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
