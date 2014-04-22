/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::message_pack::object
 */
#ifndef PSYQ_MESSAGE_PACK_OBJECT_HPP_
#define PSYQ_MESSAGE_PACK_OBJECT_HPP_

//#include "psyq/message_pack_object.hpp"

#ifndef PSYQ_MESSAGE_PACK_OBJECT_FLOATING_POINT_EPSILON
#define PSYQ_MESSAGE_PACK_OBJECT_FLOATING_POINT_EPSILON\
    std::numeric_limits<self::floating_point>::epsilon() * 4
#endif // !defined(PSYQ_MESSAGE_PACK_OBJECT_FLOATING_POINT_EPSILON)

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
        kind_FLOATING_POINT,   ///< 浮動小数点実数。
        kind_RAW,              ///< RAWバイト列。
        kind_ARRAY,            ///< MessagePackオブジェクトの配列。
        kind_MAP,              ///< MessagePackオブジェクトの連想配列。
    };

    //-------------------------------------------------------------------------
    /// 任意型を比較する関数オブジェクト。
    private: template<typename template_value_type>
    struct compare_value
    {
        typedef template_value_type value_type;

        static int compare(
            value_type const in_left,
            value_type const in_right)
        PSYQ_NOEXCEPT
        {
            return in_left < in_right? -1: (in_right < in_left? 1: 0);
        }

        static bool equal(
            value_type const in_left,
            value_type const in_right)
        PSYQ_NOEXCEPT
        {
            return in_left == in_right;
        }
    };

    /// MessagePackオブジェクトを比較する関数オブジェクト。
    private: struct compare_object
    {
        typedef object value_type;

        static int compare(
            value_type const& in_left,
            value_type const& in_right)
        PSYQ_NOEXCEPT
        {
            return in_left.compare(in_right);
        }

        static bool equal(
            value_type const& in_left,
            value_type const& in_right)
        PSYQ_NOEXCEPT
        {
            return in_left == in_right;
        }
    };

    /// 連想配列のキーを比較する関数オブジェクト。
    private: struct compare_map_key
    {
        typedef std::pair<object, object> value_type;

        static int compare(
            value_type const& in_left,
            value_type const& in_right)
        PSYQ_NOEXCEPT
        {
            return in_left.first.compare(in_right.first);
        }

        static bool equal(
            value_type const& in_left,
            value_type const& in_right)
        PSYQ_NOEXCEPT
        {
            return in_left.first == in_right.first;
        }
    };

    //-------------------------------------------------------------------------
    /// @name オブジェクトに格納できる型
    //@{
    /// @copydoc self::kind_FLOATING_POINT
    public: typedef double floating_point;
    /// @copydoc self::kind_RAW
    public: typedef psyq::internal
        ::message_pack_object_container<self::compare_value<char const>>
            raw;
    /// @copydoc self::kind_ARRAY
    public: typedef psyq::internal
        ::message_pack_object_container<self::compare_object>
            array;
    /// @copydoc self::kind_MAP
    public: typedef psyq::internal
        ::message_pack_object_map<self::compare_map_key>
            map;
    //@}
    //-------------------------------------------------------------------------
    /// @name オブジェクトの構築と破棄
    //@{
    /// 空のオブジェクトを構築する。
    public: PSYQ_CONSTEXPR object(): kind_(self::kind_NIL) {}

    /** @brief オブジェクトに真偽値を格納する。
        @param[in] in_boolean オブジェクトに格納する真偽値。
     */
    public: PSYQ_CONSTEXPR object(bool const in_boolean)
    PSYQ_NOEXCEPT:
        boolean_(in_boolean),
        kind_(self::kind_BOOLEAN)
    {}

    /** @brief オブジェクトに無符号整数を格納する。
        @param[in] in_integer オブジェクトに格納する無符号整数。
     */
    public: PSYQ_CONSTEXPR object(std::uint64_t const in_integer)
    PSYQ_NOEXCEPT:
        positive_integer_(in_integer),
        kind_(self::kind_POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: PSYQ_CONSTEXPR object(unsigned long const in_integer)
    PSYQ_NOEXCEPT:
        positive_integer_(in_integer),
        kind_(self::kind_POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: PSYQ_CONSTEXPR object(unsigned int const in_integer)
    PSYQ_NOEXCEPT:
        positive_integer_(in_integer),
        kind_(self::kind_POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: PSYQ_CONSTEXPR object(unsigned short const in_integer)
    PSYQ_NOEXCEPT:
        positive_integer_(in_integer),
        kind_(self::kind_POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::uint64_t const)
    public: PSYQ_CONSTEXPR object(unsigned char const in_integer)
    PSYQ_NOEXCEPT:
        positive_integer_(in_integer),
        kind_(self::kind_POSITIVE_INTEGER)
    {}

    /** @brief オブジェクトに整数を格納する。
        @param[in] in_integer オブジェクトに格納する整数。
     */
    public: PSYQ_CONSTEXPR object(std::int64_t const in_integer)
    PSYQ_NOEXCEPT:
        negative_integer_(in_integer),
        kind_(
            in_integer < 0?
                self::kind_NEGATIVE_INTEGER: self::kind_POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::int64_t const)
    public: PSYQ_CONSTEXPR object(long const in_integer)
    PSYQ_NOEXCEPT:
        negative_integer_(in_integer),
        kind_(
            in_integer < 0?
                self::kind_NEGATIVE_INTEGER: self::kind_POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::int64_t const)
    public: PSYQ_CONSTEXPR object(int const in_integer)
    PSYQ_NOEXCEPT:
        negative_integer_(in_integer),
        kind_(
            in_integer < 0?
                self::kind_NEGATIVE_INTEGER: self::kind_POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::int64_t const)
    public: PSYQ_CONSTEXPR object(short const in_integer)
    PSYQ_NOEXCEPT:
        negative_integer_(in_integer),
        kind_(
            in_integer < 0?
                self::kind_NEGATIVE_INTEGER: self::kind_POSITIVE_INTEGER)
    {}
    /// @copydoc object(std::int64_t const)
    public: PSYQ_CONSTEXPR object(char const in_integer)
    PSYQ_NOEXCEPT:
        negative_integer_(in_integer),
        kind_(
            in_integer < 0?
                self::kind_NEGATIVE_INTEGER: self::kind_POSITIVE_INTEGER)
    {}

    /** @brief オブジェクトに浮動小数点実数を格納する。
        @param[in] in_floating_point オブジェクトに格納する浮動小数点実数。
     */
    public: PSYQ_CONSTEXPR object(
        self::floating_point const in_floating_point)
    PSYQ_NOEXCEPT:
        floating_point_(in_floating_point),
        kind_(self::kind_FLOATING_POINT)
    {}

    /** @brief オブジェクトにRAWバイト列を格納する。
        @param[in] in_raw オブジェクトに格納するRAWバイト列。
     */
    public: PSYQ_CONSTEXPR object(self::raw const& in_raw)
    PSYQ_NOEXCEPT:
        raw_(in_raw),
        kind_(self::kind_RAW)
    {}

    /** @brief オブジェクトに配列を格納する。
        @param[in] in_array オブジェクトに格納する配列。
     */
    public: PSYQ_CONSTEXPR object(self::array const& in_array)
    PSYQ_NOEXCEPT:
        array_(in_array),
        kind_(self::kind_ARRAY)
    {}

    /** @brief オブジェクトに連想配列を格納する。
        @param[in] in_map オブジェクトに格納する連想配列。
     */
    public: PSYQ_CONSTEXPR object(self::map const& in_map)
    PSYQ_NOEXCEPT:
        map_(in_map),
        kind_(self::kind_MAP)
    {}

    /** @brief オブジェクトに真偽値を格納する。
        @param[in] in_boolean オブジェクトに格納する真偽値。
        @return *this
     */
    public: self& operator=(bool const in_boolean) PSYQ_NOEXCEPT
    {
        this->boolean_ = in_boolean;
        this->kind_ = self::kind_BOOLEAN;
        return *this;
    }

    /** @brief オブジェクトに無符号整数を格納する。
        @param[in] in_integer オブジェクトに格納する無符号整数。
        @return *this
     */
    public: self& operator=(std::uint64_t const in_integer) PSYQ_NOEXCEPT
    {
        this->positive_integer_ = in_integer;
        this->kind_ = self::kind_POSITIVE_INTEGER;
        return *this;
    }
    /// @copydoc operator=(std::uint64_t const)
    public: self& operator=(unsigned long const in_integer) PSYQ_NOEXCEPT
    {
        return *this = static_cast<std::uint64_t>(in_integer);
    }
    /// @copydoc operator=(std::uint64_t const)
    public: self& operator=(unsigned int const in_integer) PSYQ_NOEXCEPT
    {
        return *this = static_cast<std::uint64_t>(in_integer);
    }
    /// @copydoc operator=(std::uint64_t const)
    public: self& operator=(unsigned short const in_integer) PSYQ_NOEXCEPT
    {
        return *this = static_cast<std::uint64_t>(in_integer);
    }
    /// @copydoc operator=(std::uint64_t const)
    public: self& operator=(unsigned char const in_integer) PSYQ_NOEXCEPT
    {
        return *this = static_cast<std::uint64_t>(in_integer);
    }

    /** @brief オブジェクトに整数を格納する。
        @param[in] in_integer オブジェクトに格納する整数。
        @return *this
     */
    public: self& operator=(std::int64_t const in_integer) PSYQ_NOEXCEPT
    {
        this->negative_integer_ = in_integer;
        this->kind_ = in_integer < 0?
            self::kind_NEGATIVE_INTEGER: self::kind_POSITIVE_INTEGER;
        return *this;
    }
    /// @copydoc operator=(std::int64_t const)
    public: self& operator=(long const in_integer) PSYQ_NOEXCEPT
    {
        return *this = static_cast<std::int64_t>(in_integer);
    }
    /// @copydoc operator=(std::int64_t const)
    public: self& operator=(int const in_integer) PSYQ_NOEXCEPT
    {
        return *this = static_cast<std::int64_t>(in_integer);
    }
    /// @copydoc operator=(std::int64_t const)
    public: self& operator=(short const in_integer) PSYQ_NOEXCEPT
    {
        return *this = static_cast<std::int64_t>(in_integer);
    }
    /// @copydoc operator=(std::int64_t const)
    public: self& operator=(char const in_integer) PSYQ_NOEXCEPT
    {
        return *this = static_cast<std::int64_t>(in_integer);
    }

    /** @brief オブジェクトに浮動小数点実数を格納する。
        @param[in] in_floating_point オブジェクトに格納する浮動小数点実数。
        @return *this
     */
    public: self& operator=(self::floating_point const in_floating_point)
    PSYQ_NOEXCEPT
    {
        this->floating_point_ = in_floating_point;
        this->kind_ = self::kind_FLOATING_POINT;
        return *this;
    }

    /** @brief オブジェクトにRAWバイト列を格納する。
        @param[in] in_raw オブジェクトに格納するRAWバイト列。
        @return *this
     */
    public: self& operator=(self::raw const& in_raw) PSYQ_NOEXCEPT
    {
        this->raw_ = in_raw;
        this->kind_ = self::kind_RAW;
        return *this;
    }

    /** @brief オブジェクトに配列を格納する。
        @param[in] in_array オブジェクトに格納する配列。
        @return *this
     */
    public: self& operator=(self::array const& in_array) PSYQ_NOEXCEPT
    {
        this->array_ = in_array;
        this->kind_ = self::kind_ARRAY;
        return *this;
    }

    /** @brief オブジェクトに連想配列を格納する。
        @param[in] in_map オブジェクトに格納する連想配列。
        @return *this
     */
    public: self& operator=(self::map const& in_map) PSYQ_NOEXCEPT
    {
        this->map_ = in_map;
        this->kind_ = self::kind_MAP;
        return *this;
    }

    /// @brief オブジェクトを空にする。
    public: void reset() PSYQ_NOEXCEPT
    {
        this->kind_ = self::kind_NIL;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name オブジェクトに格納されてる値の操作
    //@{
    /** @brief オブジェクトに格納されてる値の種別を取得する。
        @return @copydoc self::kind
     */
    public: PSYQ_CONSTEXPR self::kind get_kind() const PSYQ_NOEXCEPT
    {
        return this->kind_;
    }

    /** @brief オブジェクトに格納されてる真偽値を取得する。
        @retval !=nullptr オブジェクトに格納されてる真偽値へのポインタ。
        @retval ==nullptr オブジェクトに格納されてるのは真偽値ではない。
     */
    public: PSYQ_CONSTEXPR bool const* get_boolean() const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind_BOOLEAN?
            &this->boolean_: nullptr;
    }

    /** @brief オブジェクトに格納されてる正の整数を取得する。
        @retval !=nullptr オブジェクトに格納されてる正の整数へのポインタ。
        @retval ==nullptr オブジェクトに格納されてるのは正の整数ではない。
     */
    public: PSYQ_CONSTEXPR std::uint64_t const* get_positive_integer()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind_POSITIVE_INTEGER?
            &this->positive_integer_: nullptr;
    }

    /** @brief オブジェクトに格納されてる負の整数を取得する。
        @retval !=nullptr オブジェクトに格納されてる負の整数へのポインタ。
        @retval ==nullptr オブジェクトに格納されてるのは負の整数ではない。
     */
    public: PSYQ_CONSTEXPR std::int64_t const* get_negative_integer()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind_NEGATIVE_INTEGER?
            &this->negative_integer_: nullptr;
    }

    /** @brief オブジェクトに格納されてる整数を取得する。
        @param[out] out_integer 取得した整数が格納される。
        @retval true  取得に成功した。
        @retval false 取得に失敗した。
     */
    public: template<typename template_integer_type>
    bool get_integer(template_integer_type& out_integer)
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

    /** @brief オブジェクトに格納されてる浮動小数点実数を取得する。
        @retval !=nullptr オブジェクトに格納されてる浮動小数点実数へのポインタ。
        @retval ==nullptr オブジェクトに格納されてるのは浮動小数点実数ではない。
     */
    public: PSYQ_CONSTEXPR self::floating_point const* get_floating_point()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind_FLOATING_POINT?
            &this->floating_point_: nullptr;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name オブジェクトに格納されてるRAWバイト列の操作
    //@{
    /** @brief オブジェクトに格納されてるRAWバイト列を取得する。
        @retval !=nullptr オブジェクトに格納されてるRAWバイト列へのポインタ。
        @retval ==nullptr オブジェクトに格納されてるのはRAWバイト列ではない。
        @sa self::set_raw()
     */
    public: PSYQ_CONSTEXPR self::raw const* get_raw() const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind_RAW? &this->raw_: nullptr;
    }

    /** @brief オブジェクトにRAWバイト列を格納する。
        @param[in] in_data RAWバイト列の先頭位置。
        @param[in] in_size RAWバイト列のバイト数。
        @return オブジェクトに格納したRAWバイト列。
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
    /// @name オブジェクトに格納されてる配列の操作
    //@{
    /** @brief オブジェクトに格納されてる配列を取得する。
        @retval !=nullptr オブジェクトに格納されてる配列へのポインタ。
        @retval ==nullptr オブジェクトに格納されてるのは配列ではない。
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

    /** @brief オブジェクトに配列を格納する。
        @param[in] in_data 配列の先頭位置。
        @param[in] in_size 配列の要素数。
        @return オブジェクトに格納されてる配列。
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
    /// @name オブジェクトに格納されてる連想配列の操作
    //@{
    /** @brief オブジェクトに格納されてる連想配列を取得する。
        @retval !=nullptr オブジェクトに格納されてる連想配列へのポインタ。
        @retval ==nullptr オブジェクトに格納されてるのは連想配列ではない。
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

    /** @brief オブジェクトに配列を格納する。
        @param[in] in_data 連想配列の先頭位置。
        @param[in] in_size 連想配列の要素数。
        @return オブジェクトに格納されてる連想配列。
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
    /// @name オブジェクトの比較
    //@{
    /** @brief thisを左辺として、右辺のオブジェクトと等値か判定する。
        @param[in] in_right 右辺のオブジェクト。
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
        case self::kind_FLOATING_POINT:
            return 0 == self::compare_floating_point(
                this->floating_point_, in_right.floating_point_);
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

    /** @brief thisを左辺として、右辺のオブジェクトと非等値か判定する。
        @param[in] in_right 右辺のオブジェクト。
        @retval true  非等値だった 。
        @retval false 等値だった。
     */
    public: bool operator!=(self const& in_right) const PSYQ_NOEXCEPT
    {
        return !(*this == in_right);
    }

    /** @brief thisを左辺として、右辺のオブジェクトと比較する。
        @param[in] in_right 右辺のオブジェクト。
        @return 左辺 < 右辺
     */
    public: bool operator<(self const& in_right) const PSYQ_NOEXCEPT
    {
        return this->compare(in_right) < 0;
    }

    /** @brief thisを左辺として、右辺のオブジェクトと比較する。
        @param[in] in_right 右辺のオブジェクト。
        @return 左辺 <= 右辺
     */
    public: bool operator<=(self const& in_right) const PSYQ_NOEXCEPT
    {
        return this->compare(in_right) <= 0;
    }

    /** @brief thisを左辺として、右辺のオブジェクトと比較する。
        @param[in] in_right 右辺のオブジェクト。
        @return 左辺 > 右辺
     */
    public: bool operator>(self const& in_right) const PSYQ_NOEXCEPT
    {
        return 0 < this->compare(in_right);
    }

    /** @brief thisを左辺として、右辺のオブジェクトと比較する。
        @param[in] in_right 右辺のオブジェクト。
        @return 左辺 >= 右辺
     */
    public: bool operator>=(self const& in_right) const PSYQ_NOEXCEPT
    {
        return 0 <= this->compare(in_right);
    }

    /** @brief thisを左辺として、右辺のオブジェクトと比較する。
        @param[in] in_right 右辺となるオブジェクト。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    public: int compare(self const& in_right) const PSYQ_NOEXCEPT
    {
        switch (in_right.get_kind())
        {
        case self::kind_NIL:
            return this->get_kind() != self::kind_NIL? 1: 0;
        case self::kind_BOOLEAN:
            return this->compare(in_right.boolean_);
        case self::kind_POSITIVE_INTEGER:
            return this->compare(in_right.positive_integer_);
        case self::kind_NEGATIVE_INTEGER:
            return this->compare(in_right.negative_integer_);
        case self::kind_FLOATING_POINT:
            return this->compare(in_right.floating_point_);
        case self::kind_RAW:
            return this->compare(in_right.raw_);
        case self::kind_ARRAY:
            return this->compare(in_right.array_);
        case self::kind_MAP:
            return this->compare(in_right.map_);
        default:
            PSYQ_ASSERT(false);
            return this->get_kind() <= self::kind_MAP? -1: 0;
        }
    }

    /** @brief thisを左辺として、右辺の連想配列と比較する。
        @param[in] in_right 右辺となる連想配列。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    public: int compare(self::map const& in_right) const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return -1;//-self::compare_map(in_right, this->boolean_);
        case self::kind_POSITIVE_INTEGER:
            return -1;//-self::compare_map(in_right, this->positive_integer_);
        case self::kind_NEGATIVE_INTEGER:
            return -1;//-self::compare_map(in_right, this->negative_integer_);
        case self::kind_FLOATING_POINT:
            return -1;//-self::compare_map(in_right, this->floating_point_);
        case self::kind_RAW:
            return -1;//-self::compare_map(in_right, this->raw_);
        case self::kind_ARRAY:
            return -1;//-self::compare_map(in_right, this->array_);
        case self::kind_MAP:
            return this->map_.compare(in_right);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief thisを左辺として、右辺の配列と比較する。
        @param[in] in_right 右辺となる配列。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    public: int compare(self::array const& in_right) const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return -1;//-self::compare_array(in_right, this->boolean_);
        case self::kind_POSITIVE_INTEGER:
            return -1;//-self::compare_array(in_right, this->positive_integer_);
        case self::kind_NEGATIVE_INTEGER:
            return -1;//-self::compare_array(in_right, this->negative_integer_);
        case self::kind_FLOATING_POINT:
            return -1;//-self::compare_array(in_right, this->floating_point_);
        case self::kind_RAW:
            return -1;//-self::compare_array(in_right, this->raw_);
        case self::kind_ARRAY:
            return this->array_.compare(in_right);
        case self::kind_MAP:
            return 1;//self::compare_map(this->map_, in_right);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief thisを左辺として、右辺のRAWバイト列と比較する。
        @param[in] in_right 右辺となるRAWバイト列。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    public: int compare(self::raw const& in_right) const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return -1;//-self::compare_raw(in_right, this->boolean_);
        case self::kind_POSITIVE_INTEGER:
            return -1;//-self::compare_raw(in_right, this->positive_integer_);
        case self::kind_NEGATIVE_INTEGER:
            return -1;//-self::compare_raw(in_right, this->negative_integer_);
        case self::kind_FLOATING_POINT:
            return -1;//-self::compare_raw(in_right, this->floating_point_);
        case self::kind_RAW:
            return this->raw_.compare(in_right);
        case self::kind_ARRAY:
            return 1;//self::compare_array(this->array_, in_right);
        case self::kind_MAP:
            return 1;//self::compare_map(this->map_, in_right);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief thisを左辺として、右辺の浮動小数点実数と比較する。
        @param[in] in_right 右辺となる浮動小数点実数。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    public: int compare(self::floating_point const in_right)
    const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return -self::compare_floating_point(in_right, this->boolean_);
        case self::kind_POSITIVE_INTEGER:
            return -self::compare_floating_point(
                in_right, this->positive_integer_);
        case self::kind_NEGATIVE_INTEGER:
            return -self::compare_floating_point(
                in_right, this->negative_integer_);
        case self::kind_FLOATING_POINT:
            return self::compare_floating_point(
                this->floating_point_, in_right);
        case self::kind_RAW:
            return 1;//self::compare_raw(this->raw_, in_right);
        case self::kind_ARRAY:
            return 1;//self::compare_array(this->array_, in_right);
        case self::kind_MAP:
            return 1;//self::compare_map(this->map_, in_right);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief thisを左辺として、右辺の有符号整数と比較する。
        @param[in] in_right 右辺となる有符号整数。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    public: int compare(std::int64_t const in_right) const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return -self::compare_signed_integer(in_right, this->boolean_);
        case self::kind_POSITIVE_INTEGER:
            return -self::compare_signed_integer(
                in_right, this->positive_integer_);
        case self::kind_NEGATIVE_INTEGER:
            return self::compare_signed_integer(
                this->negative_integer_, in_right);
        case self::kind_FLOATING_POINT:
            return -self::compare_signed_integer(
                in_right, this->floating_point_);
        case self::kind_RAW:
            return 1;//self::compare_raw(this->raw_, in_right);
        case self::kind_ARRAY:
            return 1;//self::compare_array(this->array_, in_right);
        case self::kind_MAP:
            return 1;//self::compare_map(this->map_, in_right);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief thisを左辺として、右辺の無符号整数と比較する。
        @param[in] in_right 右辺となる無符号整数。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    public: int compare(std::uint64_t const in_right) const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return -self::compare_unsigned_integer(in_right, this->boolean_);
        case self::kind_POSITIVE_INTEGER:
            return self::compare_unsigned_integer(
                this->positive_integer_, in_right);
        case self::kind_NEGATIVE_INTEGER:
            return -1;//-self::compare_unsigned_integer(in_right, this->negative_integer_);
        case self::kind_FLOATING_POINT:
            return self::compare_floating_point(
                this->floating_point_, in_right);
        case self::kind_RAW:
            return 1;//self::compare_raw(this->raw_, in_right);
        case self::kind_ARRAY:
            return 1;//self::compare_array(this->array_, in_right);
        case self::kind_MAP:
            return 1;//self::compare_map(this->map_, in_right);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }

    /** @brief thisを左辺として、右辺の真偽値と比較する。
        @param[in] in_right 右辺となる真偽値。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    public: int compare(bool const in_right) const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
        case self::kind_NIL:
            return 1;
        case self::kind_BOOLEAN:
            return self::compare_boolean(this->boolean_, in_right);
        case self::kind_POSITIVE_INTEGER:
            return self::compare_unsigned_integer(
                this->positive_integer_, in_right);
        case self::kind_NEGATIVE_INTEGER:
            return -1;//-self::compare_negative_integer(this->negative_integer_, in_right);
        case self::kind_FLOATING_POINT:
            return self::compare_floating_point(
                this->floating_point_, in_right);
        case self::kind_RAW:
            return 1;//self::compare_raw(this->raw_, in_right);
        case self::kind_ARRAY:
            return 1;//self::compare_array(this->array_, in_right);
        case self::kind_MAP:
            return 1;//self::compare_map(this->map_, in_right);
        default:
            PSYQ_ASSERT(false);
            return 1;
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 浮動小数点実数との比較
    //@{
    private: static int compare_floating_point(
        self::floating_point const in_left,
        self::floating_point const in_right)
    PSYQ_NOEXCEPT
    {
        auto const local_epsilon(
            PSYQ_MESSAGE_PACK_OBJECT_FLOATING_POINT_EPSILON);
        auto const local_diff(in_left - in_right);
        return local_diff < -local_epsilon?
            -1: (local_epsilon < local_diff? 1: 0);
    }

    private: static int compare_floating_point(
        self::floating_point const in_left,
        std::uint64_t const in_right)
    PSYQ_NOEXCEPT
    {
        return in_left < 0?
            -1:
            self::compare_floating_point(
                in_left, static_cast<self::floating_point>(in_right));
    }

    private: static int compare_floating_point(
        self::floating_point const in_left,
        std::int64_t const in_right)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_right < 0);
        return in_left < 0?
            self::compare_floating_point(
                in_left, static_cast<self::floating_point>(in_right)):
            1;
    }

    private: static int compare_floating_point(
        self::floating_point const in_left,
        bool const in_right)
    PSYQ_NOEXCEPT
    {
        return in_right?
            in_left < 1? -1: (1 < in_left? 1: 0):
            in_left < 0? -1: (0 < in_left? 1: 0);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 有符号整数との比較
    //@{
    private: static int compare_signed_integer(
        std::int64_t const in_left,
        self::floating_point const in_right)
    PSYQ_NOEXCEPT
    {
        return self::compare_floating_point(
            static_cast<self::floating_point>(in_left), in_right);
    }

    private: static int compare_signed_integer(
        std::int64_t const in_left,
        std::uint64_t const in_right)
    PSYQ_NOEXCEPT
    {
        return in_left < 0?
            -1:
            self::compare_unsigned_integer(
                static_cast<std::uint64_t>(in_left), in_right);
    }

    private: static int compare_signed_integer(
        std::int64_t const in_left,
        std::int64_t const in_right)
    PSYQ_NOEXCEPT
    {
        return in_left < in_right? -1: (in_right < in_left? 1: 0);
    }

    private: static int compare_signed_integer(
        std::int64_t const in_left,
        bool const in_right)
    PSYQ_NOEXCEPT
    {
        return in_left < 0?
            -1:
            self::compare_unsigned_integer(
                static_cast<std::uint64_t>(in_left), in_right);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 無符号整数との比較
    //@{
    private: static int compare_unsigned_integer(
        std::uint64_t const in_left,
        std::uint64_t const in_right)
    PSYQ_NOEXCEPT
    {
        return in_left < in_right? -1: (in_right < in_left? 1: 0);
    }

    private: static int compare_unsigned_integer(
        std::uint64_t const in_left,
        bool const in_right)
    PSYQ_NOEXCEPT
    {
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
    private: union
    {
        /// @copydoc self::kind_BOOLEAN
        bool boolean_;
        /// @copydoc self::kind_POSITIVE_INTEGER
        std::uint64_t positive_integer_;
        /// @copydoc self::kind_NEGATIVE_INTEGER
        std::int64_t negative_integer_;
        /// @copydoc self::kind_FLOATING_POINT
        self::floating_point floating_point_;
        /// @copydoc self::kind_RAW
        self::raw raw_;
        /// @copydoc self::kind_ARRAY
        self::array array_;
        /// @copydoc self::kind_MAP
        self::map map_;
    };
    private: self::kind kind_; ///< @copydoc self::kind
};

#endif // PSYQ_MESSAGE_PACK_OBJECT_HPP_
