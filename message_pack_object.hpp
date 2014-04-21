/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief MessagePackオブジェクト。
 */
#ifndef PSYQ_MESSAGE_PACK_OBJECT_HPP_
#define PSYQ_MESSAGE_PACK_OBJECT_HPP_

namespace psyq
{
    namespace message_pack
    {
        /// @cond
        template<typename> struct object_container;
        struct object_map;
        struct object;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief MessagePackオブジェクトコンテナの基底型。
    @tparam template_value_type @copydoc value_type
 */
template<typename template_value_type>
struct psyq::message_pack::object_container
{
    /// thisが指す値の型。
    private: typedef object_container<template_value_type> self;

    //-------------------------------------------------------------------------
    /// 要素の型。
    public: typedef template_value_type value_type;

    /// 要素数の型。
    public: typedef std::size_t size_type;

    /// 反復子の差を表す型。
    public: typedef std::ptrdiff_t difference_type;

    /// 要素へのpointer。
    public: typedef typename self::value_type const* const_pointer;

    /// 要素へのpointer。
    public: typedef typename self::value_type* pointer;

    /// 要素への参照。
    public: typedef typename self::value_type const& const_reference;

    /// 要素への参照。
    public: typedef typename self::value_type& reference;

    /// 要素を指す反復子。
    public: typedef typename self::const_pointer const_iterator;

    /// 要素を指す反復子。
    public: typedef typename self::pointer iterator;

    /// 要素を指す逆反復子。
    public: typedef std::reverse_iterator<const_iterator>
        const_reverse_iterator;

    /// 要素を指す逆反復子。
    public: typedef std::reverse_iterator<iterator>
        reverse_iterator;

    //-------------------------------------------------------------------------
    /// @name コンテナの要素を参照
    //@{
    /** @brief コンテナの先頭要素へのpointerを取得する。
        @return コンテナの先頭要素へのpointer。
     */
    public: typename self::const_pointer data()
    const PSYQ_NOEXCEPT
    {
        return this->data_;
    }

    /** @brief コンテナが持つ要素を参照する。
        @param[in] in_index 要素のindex番号。
        @return 要素への参照。
     */
    public: typename self::const_reference at(
        typename self::size_type const in_index)
    const
    {
        if (this->size() <= in_index)
        {
            PSYQ_ASSERT(false);
            //throw std::out_of_range; // 例外は使いたくない。
        }
        return *(this->data() + in_index);
    }

    /** @brief コンテナが持つ要素を参照する。
        @param[in] in_index 要素のindex番号。
        @return 要素への参照。
     */
    public: typename self::const_reference operator[](
        typename self::size_type const in_index)
    const
    {
        PSYQ_ASSERT(in_index < this->size());
        return *(this->data() + in_index);
    }

    /** @brief コンテナの最初の要素を参照する。
        @return コンテナの最初の要素への参照。
     */
    public: typename self::const_reference front() const
    {
        return (*this)[0];
    }

    /** @brief コンテナの最後の要素を参照する。
        @return コンテナの最後の要素への参照。
     */
    public: typename self::const_reference back() const
    {
        return (*this)[this->size() - 1];
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name iteratorの取得
    //@{
    /** @brief コンテナの先頭位置を取得する。
        @return コンテナの先頭位置への反復子。
     */
    public: typename self::const_iterator begin() const PSYQ_NOEXCEPT
    {
        return this->data();
    }

    /** @brief コンテナの末尾位置を取得する。
        @return コンテナの末尾位置への反復子。
     */
    public: typename self::const_iterator end() const PSYQ_NOEXCEPT
    {
        return this->begin() + this->size();
    }

    /** @brief コンテナの先頭位置を取得する。
        @return コンテナの先頭位置への反復子。
     */
    public: typename self::const_iterator cbegin() const PSYQ_NOEXCEPT
    {
        return this->begin();
    }

    /** @brief コンテナの末尾位置を取得する。
        @return コンテナの末尾位置への反復子。
     */
    public: typename self::const_iterator cend() const PSYQ_NOEXCEPT
    {
        return this->end();
    }

    /** @brief コンテナの末尾位置を取得する。
        @return コンテナの末尾位置への逆反復子。
     */
    public: typename self::const_reverse_iterator rbegin() const PSYQ_NOEXCEPT
    {
        return typename self::const_reverse_iterator(this->end());
    }

    /** @brief コンテナの先頭位置を取得する。
        @return コンテナの先頭位置への逆反復子。
     */
    public: typename self::const_reverse_iterator rend() const PSYQ_NOEXCEPT
    {
        return typename self::const_reverse_iterator(this->begin());
    }

    /** @brief コンテナの末尾位置を取得する。
        @return コンテナの末尾位置への逆反復子。
     */
    public: typename self::const_reverse_iterator crbegin() const PSYQ_NOEXCEPT
    {
        return this->rbegin();
    }

    /** @brief コンテナの先頭位置を取得する。
        @return コンテナの先頭位置への逆反復子。
     */
    public: typename self::const_reverse_iterator crend() const PSYQ_NOEXCEPT
    {
        return this->rend();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name コンテナの容量
    //@{
    /** @brief 空のコンテナか判定する。
        @retval true  空のコンテナ。
        @retval false 空のコンテナではない。
     */
    public: bool empty() const PSYQ_NOEXCEPT
    {
        return this->size() <= 0;
    }

    /** @brief コンテナの要素数を取得する。
        @return コンテナの要素数。
     */
    public: typename self::size_type size() const PSYQ_NOEXCEPT
    {
        return this->size_;
    }

    /** @brief コンテナの最大要素数を取得する。
        @return コンテナの最大要素数。
     */
    public: PSYQ_CONSTEXPR std::size_t max_size() const PSYQ_NOEXCEPT
    {
        return (std::numeric_limits<std::size_t>::max)();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name コンテナの編集
    //@{
    public: void reset(
        typename self::pointer const in_data,
        std::size_t const in_size)
    {
        this->data_ = in_data;
        this->size_ = in_size;
    }

    public: void push_back(typename self::const_reference in_element)
    {
        this->data_[this->size_] = in_element;
        ++this->size_;
    }
    //@}
    //-------------------------------------------------------------------------
    private: typename self::pointer data_;   ///< コンテナの先頭位置。
    private: typename self::size_type size_; ///< コンテナの要素数。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// MessagePackオブジェクトの連想配列。
struct psyq::message_pack::object_map:
    public psyq::message_pack::object_container<
        std::pair<psyq::message_pack::object, psyq::message_pack::object>>
{
    /// thisが指す値の型。
    private: typedef object_map self;

    /// thisの上位型。
    public: typedef psyq::message_pack::object_container<
        std::pair<psyq::message_pack::object, psyq::message_pack::object>>
            super;

    /// 連想配列のキー。
    public: typedef psyq::message_pack::object key_type;
    /// 連想配列のマップ値。
    public: typedef psyq::message_pack::object mapped_type;

    //-------------------------------------------------------------------------
    public: void push_back(super::value_type const& in_value)
    {
        this->super::push_back(in_value);
    }

    public: super::const_iterator find(
        self::key_type const& in_key)
    const
    {
        /// @todo 未実装。
        return this->cend();
    }

    private: super::const_reference at(super::size_type const) const;
    private: super::const_reference operator[](super::size_type const) const;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// MessagePackオブジェクト。
struct psyq::message_pack::object
{
    private: typedef object self;

    /** @brief MessagePackオブジェクトに格納されてる値の種別。
        @sa self::get_kind()
     */
    public: enum kind
    {
        kind_NIL,              ///< 空
        kind_BOOLEAN,          ///< 真偽値
        kind_POSITIVE_INTEGER, ///< 正の整数
        kind_NEGATIVE_INTEGER, ///< 負の整数
        kind_FLOATING_POINT,   ///< 浮動小数点実数
        kind_RAW,              ///< RAWバイト列
        kind_ARRAY,            ///< MessagePackオブジェクトの配列
        kind_MAP,              ///< MessagePackオブジェクトの連想配列
    };

    /// @copydoc self::kind_RAW
    public: typedef psyq::message_pack::object_container<char const> raw;
    /// @copydoc self::kind_ARRAY
    public: typedef psyq::message_pack::object_container<self> array;
    /// @copydoc self::kind_MAP
    public: typedef psyq::message_pack::object_map map;

    //-------------------------------------------------------------------------
    /// 空のオブジェクトを構築する。
    public: object(): kind_(self::kind_NIL) {}

    /// オブジェクトを空にする。
    public: void set_nil() PSYQ_NOEXCEPT
    {
        this->kind_ = psyq::message_pack::object::kind_NIL;
    }

    /** @brief オブジェクトに格納されてる値の種別を取得する。
        @return @copydoc self::kind
     */
    public: self::kind get_kind() const PSYQ_NOEXCEPT
    {
        return this->kind_;
    }

    //-------------------------------------------------------------------------
    /// @name 真偽値の操作
    //@{
    /** @brief オブジェクトに格納されてる真偽値を取得する。
        @retval !=nullptr オブジェクトに格納されてる真偽値へのポインタ。
        @retval ==nullptr オブジェクトは真偽値を持ってない。
     */
    public: bool const* get_boolean() const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind_BOOLEAN?
            &this->boolean_: nullptr;
    }

    /** @brief オブジェクトに真偽値を格納する。
        @param[in] in_boolean オブジェクトに格納する真偽値。
        @return オブジェクトに格納した真偽値。
     */
    public: bool set_boolean(bool const in_boolean) PSYQ_NOEXCEPT
    {
        this->boolean_ = in_boolean;
        this->kind_ = self::kind_BOOLEAN;
        return in_boolean;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 整数の操作
    //@{
    /** @brief オブジェクトに格納されてる正の整数を取得する。
        @retval !=nullptr オブジェクトに格納されてる正の整数へのポインタ。
        @retval ==nullptr オブジェクトは正の整数を持ってない。
     */
    public: std::uint64_t const* get_positive_integer() const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind_POSITIVE_INTEGER?
            &this->positive_integer_: nullptr;
    }

    /** @brief オブジェクトに格納されてる負の整数を取得する。
        @retval !=nullptr オブジェクトに格納されてる負の整数へのポインタ。
        @retval ==nullptr オブジェクトは負の整数を持ってない。
     */
    public: std::int64_t const* get_negative_integer() const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind_NEGATIVE_INTEGER?
            &this->negative_integer_: nullptr;
    }

    /** @brief オブジェクトに整数を格納する。
        @param[in] in_integer オブジェクトに格納する整数。
        @return オブジェクトに格納した整数。
     */
    public: template<typename template_integer_type>
    template_integer_type set_integer(
        template_integer_type const in_integer)
    PSYQ_NOEXCEPT
    {
        if (in_integer < 0)
        {
            this->kind_ = psyq::message_pack::object::kind_NEGATIVE_INTEGER;
            this->negative_integer_ = in_integer;
        }
        else
        {
            this->kind_ = psyq::message_pack::object::kind_POSITIVE_INTEGER;
            this->positive_integer_ = in_integer;
        }
        return in_integer;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 浮動小数点実数の操作
    //@{
    /** @brief オブジェクトに格納されてる浮動小数点実数を取得する。
        @retval !=nullptr オブジェクトに格納されてる浮動小数点実数へのポインタ。
        @retval ==nullptr オブジェクトは浮動小数点実数を持ってない。
     */
    public: double const* get_floating_point() const PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind_FLOATING_POINT?
            &this->floating_point_: nullptr;
    }

    /** @brief オブジェクトに浮動小数点実数を格納する。
        @param[in] in_floating_point オブジェクトに格納する浮動小数点実数。
        @return オブジェクトに格納した浮動小数点実数。
     */
    public: double set_floating_point(double const in_floating_point)
    PSYQ_NOEXCEPT
    {
        this->kind_ = psyq::message_pack::object::kind_FLOATING_POINT;
        this->floating_point_ = in_floating_point;
        return in_floating_point;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name RAWバイト列の操作
    //@{
    /** @brief オブジェクトに格納されてるRAWバイト列を取得する。
        @retval !=nullptr オブジェクトに格納されてるRAWバイト列へのポインタ。
        @retval ==nullptr オブジェクトはRAWバイト列を持ってない。
     */
    public: self::raw const* get_raw() const PSYQ_NOEXCEPT
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
    /// @name 配列の操作
    //@{
    /** @brief オブジェクトに格納されてる配列を取得する。
        @retval !=nullptr オブジェクトに格納されてる配列へのポインタ。
        @retval ==nullptr オブジェクトは配列を持ってない。
     */
    public: self::array* get_array() PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind_ARRAY? &this->array_: nullptr;
    }

    /// @copydoc get_array()
    public: self::array const* get_array() const PSYQ_NOEXCEPT
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
    /// @name 連想配列の操作
    //@{
    /** @brief オブジェクトに格納されてる連想配列を取得する。
        @retval !=nullptr オブジェクトに格納されてる連想配列へのポインタ。
        @retval ==nullptr オブジェクトは連想配列を持ってない。
     */
    public: self::map* get_map() PSYQ_NOEXCEPT
    {
        return this->get_kind() == self::kind_MAP? &this->map_: nullptr;
    }

    /// @copydoc get_map()
    public: self::map const* get_map() const PSYQ_NOEXCEPT
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
    private: union
    {
        bool boolean_;                   ///< 真偽値。
        std::uint64_t positive_integer_; ///< 0以上の整数。
        std::int64_t negative_integer_;  ///< 0未満の整数。
        double floating_point_;          ///< 浮動小数点実数。
        self::raw raw_;                  ///< @copydoc self::raw
        self::array array_;              ///< @copydoc self::array
        self::map map_;                  ///< @copydoc self::map
    };
    private: self::kind kind_; ///< @copydoc self::kind
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//void msgpack_object_print(FILE* out, msgpack_object o);

//bool msgpack_object_equal(const msgpack_object x, const msgpack_object y);

#endif // PSYQ_MESSAGE_PACK_OBJECT_HPP_
