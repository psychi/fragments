/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::message_pack::object
 */
#ifndef PSYQ_MESSAGE_PACK_OBJECT_HPP_
#define PSYQ_MESSAGE_PACK_OBJECT_HPP_

//#include "psyq/message_pack/storage.hpp"
//#include "psyq/message_pack/pool.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// 数値／文字列／バイト列／コンテナを格納するMessagePackオブジェクト。
class psyq::message_pack::object
{
    private: typedef object this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// MessagePackオブジェクトの値。
    private: typedef psyq::message_pack::_private::storage storage;

    /// @copydoc psyq::message_pack::_private::storage::type::value
    public: typedef psyq::message_pack::_private::storage::type::value type;

    /// @name MessagePackオブジェクトが持つ値の型。
    //@{
    /// @copydoc psyq::message_pack::_private::storage::type::UNSIGNED_INTEGER
    public: typedef psyq::message_pack::_private::storage::unsigned_integer
        unsigned_integer;
    /// @copydoc psyq::message_pack::_private::storage::type::NEGATIVE_INTEGER
    public: typedef psyq::message_pack::_private::storage::negative_integer
        negative_integer;
    /// @copydoc psyq::message_pack::_private::storage::type::FLOATING_POINT_32
    public: typedef psyq::message_pack::_private::storage::floating_point_32
        floating_point_32;
    /// @copydoc psyq::message_pack::_private::storage::type::FLOATING_POINT_64
    public: typedef psyq::message_pack::_private::storage::floating_point_64
        floating_point_64;
    /// @copydoc psyq::message_pack::_private::storage::type::STRING
    public: typedef psyq::message_pack::_private::storage::string string;
    /// @copydoc psyq::message_pack::_private::storage::type::BINARY
    public: typedef psyq::message_pack::_private::storage::binary binary;
    /// @copydoc psyq::message_pack::_private::storage::type::EXTENDED
    public: typedef psyq::message_pack::_private::storage::extended
        extended;
    /// @copydoc psyq::message_pack::_private::storage::type::ARRAY
    public: typedef psyq::message_pack::_private::storage::array array;
    /// @copydoc psyq::message_pack::_private::storage::type::UNORDERED_MAP
    public: typedef psyq::message_pack::_private::storage::unordered_map
        unordered_map;
    /// @copydoc psyq::message_pack::_private::storage::type::MAP
    public: typedef psyq::message_pack::_private::storage::map map;
    //@}

    /// @cond
    public: template<typename template_pool> class root;
    /// @endcond

    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトの構築
    //@{
    /// 空のMessagePackオブジェクトを構築する。
    public: PSYQ_CONSTEXPR object()
    PSYQ_NOEXCEPT: type_(this_type::type::NIL)
    {}

    /** @brief MessagePackオブジェクトに真偽値を格納する。
        @param[in] in_boolean MessagePackオブジェクトに格納する真偽値。
     */
    public: PSYQ_CONSTEXPR object(bool const in_boolean)
    PSYQ_NOEXCEPT:
        storage_(in_boolean),
        type_(this_type::type::BOOLEAN)
    {}

    /** @brief MessagePackオブジェクトに無符号整数を格納する。
        @param[in] in_integer MessagePackオブジェクトに格納する無符号整数。
     */
    public: PSYQ_CONSTEXPR object(unsigned long long const in_integer)
    PSYQ_NOEXCEPT:
        storage_(static_cast<std::int64_t>(in_integer)),
        type_(this_type::type::UNSIGNED_INTEGER)
    {}
    /// @copydoc object(unsigned long long const)
    public: PSYQ_CONSTEXPR object(unsigned long const in_integer)
    PSYQ_NOEXCEPT:
        storage_(static_cast<std::int64_t>(in_integer)),
        type_(this_type::type::UNSIGNED_INTEGER)
    {}
    /// @copydoc object(unsigned long long const)
    public: PSYQ_CONSTEXPR object(unsigned int const in_integer)
    PSYQ_NOEXCEPT:
        storage_(static_cast<std::int64_t>(in_integer)),
        type_(this_type::type::UNSIGNED_INTEGER)
    {}
    /// @copydoc object(unsigned long long const)
    public: PSYQ_CONSTEXPR object(unsigned short const in_integer)
    PSYQ_NOEXCEPT:
        storage_(static_cast<std::int64_t>(in_integer)),
        type_(this_type::type::UNSIGNED_INTEGER)
    {}
    /// @copydoc object(unsigned long long const)
    public: PSYQ_CONSTEXPR object(unsigned char const in_integer)
    PSYQ_NOEXCEPT:
        storage_(static_cast<std::int64_t>(in_integer)),
        type_(this_type::type::UNSIGNED_INTEGER)
    {}

    /** @brief MessagePackオブジェクトに有符号整数を格納する。
        @param[in] in_integer MessagePackオブジェクトに格納する有符号整数。
     */
    public: PSYQ_CONSTEXPR object(signed long long const in_integer)
    PSYQ_NOEXCEPT:
        storage_(static_cast<std::int64_t>(in_integer)),
        type_(this_type::tell_signed_integer_type(in_integer))
    {}
    /// @copydoc object(signed long long const)
    public: PSYQ_CONSTEXPR object(signed long const in_integer)
    PSYQ_NOEXCEPT:
        storage_(static_cast<std::int64_t>(in_integer)),
        type_(this_type::tell_signed_integer_type(in_integer))
    {}
    /// @copydoc object(signed long long const)
    public: PSYQ_CONSTEXPR object(signed int const in_integer)
    PSYQ_NOEXCEPT:
        storage_(static_cast<std::int64_t>(in_integer)),
        type_(this_type::tell_signed_integer_type(in_integer))
    {}
    /// @copydoc object(signed long long const)
    public: PSYQ_CONSTEXPR object(signed short const in_integer)
    PSYQ_NOEXCEPT:
        storage_(static_cast<std::int64_t>(in_integer)),
        type_(this_type::tell_signed_integer_type(in_integer))
    {}
    /// @copydoc object(signed long long const)
    public: PSYQ_CONSTEXPR object(signed char const in_integer)
    PSYQ_NOEXCEPT:
        storage_(static_cast<std::int64_t>(in_integer)),
        type_(this_type::tell_signed_integer_type(in_integer))
    {}

    /** @brief MessagePackオブジェクトに浮動小数点数を格納する。
        @param[in] in_float MessagePackオブジェクトに格納する浮動小数点数。
     */
    public: PSYQ_CONSTEXPR object(this_type::floating_point_64 const in_float)
    PSYQ_NOEXCEPT:
        storage_(in_float),
        type_(this_type::type::FLOATING_POINT_64)
    {}
    /// @copydoc object(this_type::floating_point_64 const)
    public: PSYQ_CONSTEXPR object(this_type::floating_point_32 const in_float)
    PSYQ_NOEXCEPT:
        storage_(in_float),
        type_(this_type::type::FLOATING_POINT_32)
    {}

    /** @brief MessagePackオブジェクトに文字列を格納する。
        @param[in] in_string MessagePackオブジェクトに格納する文字列。
     */
    public: PSYQ_CONSTEXPR object(this_type::string const& in_string)
    PSYQ_NOEXCEPT:
        storage_(in_string),
        type_(this_type::type::STRING)
    {}

    /** @brief MessagePackオブジェクトにバイナリを格納する。
        @param[in] in_binary MessagePackオブジェクトに格納するバイナリ。
     */
    public: PSYQ_CONSTEXPR object(this_type::binary const& in_binary)
    PSYQ_NOEXCEPT:
        storage_(in_binary),
        type_(this_type::type::BINARY)
    {}
    /** @brief MessagePackオブジェクトに拡張バイナリを格納する。
        @param[in] in_binary MessagePackオブジェクトに格納する拡張バイナリ。
     */
    public: PSYQ_CONSTEXPR object(this_type::extended const& in_binary)
    PSYQ_NOEXCEPT:
        storage_(in_binary),
        type_(this_type::type::EXTENDED)
    {}

    /** @brief MessagePackオブジェクトに配列を格納する。
        @param[in] in_array MessagePackオブジェクトに格納する配列。
     */
    public: PSYQ_CONSTEXPR object(this_type::array const& in_array)
    PSYQ_NOEXCEPT:
        storage_(in_array),
        type_(this_type::type::ARRAY)
    {}

    /** @brief MessagePackオブジェクトに連想配列を格納する。
        @param[in] in_map MessagePackオブジェクトに格納する連想配列。
     */
    public: PSYQ_CONSTEXPR object(this_type::unordered_map const& in_map)
    PSYQ_NOEXCEPT:
        storage_(in_map),
        type_(this_type::type::UNORDERED_MAP)
    {}

    /// @brief MessagePackオブジェクトを空にする。
    public: void reset() PSYQ_NOEXCEPT
    {
        new(this) this_type();
    }
    //@}
    /** @brief MessagePackオブジェクトに連想配列を格納する。
        @param[in] in_map MessagePackオブジェクトに格納する連想配列。
     */
    private: explicit PSYQ_CONSTEXPR object(this_type::map const& in_map)
    PSYQ_NOEXCEPT:
        storage_(in_map),
        type_(this_type::type::MAP)
    {}

    /** @brief 文字列を参照するMessagePackオブジェクトを構築する。
        @param[in] in_string 参照する文字列。
     */
    public: template<typename template_string>
    static this_type make_string(template_string const& in_string) PSYQ_NOEXCEPT
    {
        this_type::string local_string;
        local_string.reset(in_string.data(), in_string.length());
        return this_type(local_string);
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
    static this_type make_string(template_char const (&in_literal)[template_size])
    PSYQ_NOEXCEPT
    {
        static_assert(0 < template_size, "");
        PSYQ_ASSERT(in_literal[template_size - 1] == 0);
        this_type::string local_string;
        local_string.reset(&in_literal[0], template_size - 1);
        return this_type(local_string);
    }

    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトの比較
    //@{
    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと等値か判定する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @retval true  等値だった 。
        @retval false 非等値だった。
     */
    public: bool operator==(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return this_type::storage::equal(
            this->storage_,
            this->get_type(),
            in_right.storage_,
            in_right.get_type());
    }

    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと非等値か判定する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @retval true  非等値だった 。
        @retval false 等値だった。
     */
    public: bool operator!=(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return !(*this == in_right);
    }

    /** @brief *thisとMessagePackオブジェクトを比較する。
        @param[in] in_object 比較するMessagePackオブジェクト。
        @retval 正 *thisのほうが大きい。
        @retval 0  等値。
        @retval 負 *thisのほうが小さい。
     */
    public: int compare(this_type const& in_object) const PSYQ_NOEXCEPT
    {
        return this_type::storage::compare(
            this->storage_,
            this->get_type(),
            in_object.storage_,
            in_object.get_type());
    }
    //@}
    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと比較する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @return 左辺 < 右辺
     */
    private: bool operator<(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return this->compare(in_right) < 0;
    }

    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと比較する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @return 左辺 <= 右辺
     */
    private: bool operator<=(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return this->compare(in_right) <= 0;
    }

    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと比較する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @return 左辺 > 右辺
     */
    private: bool operator>(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return 0 < this->compare(in_right);
    }

    /** @brief thisを左辺として、右辺のMessagePackオブジェクトと比較する。
        @param[in] in_right 右辺のMessagePackオブジェクト。
        @return 左辺 >= 右辺
     */
    private: bool operator>=(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return 0 <= this->compare(in_right);
    }

    //-------------------------------------------------------------------------
    /// @name MessagePackオブジェクトに格納されてる値の取得
    //@{
    /** @brief MessagePackオブジェクトに格納されてる値の種別を取得する。
        @return @copydoc this_type::type
     */
    public: PSYQ_CONSTEXPR this_type::type get_type() const PSYQ_NOEXCEPT
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
        return this->get_type() == this_type::type::BOOLEAN?
            &this->storage_.boolean_: nullptr;
    }

    /** @brief MessagePackオブジェクトに格納されてる0以上の整数を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる正の整数へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは正の整数ではない。
     */
    public: PSYQ_CONSTEXPR this_type::unsigned_integer const* get_unsigned_integer()
    const PSYQ_NOEXCEPT
    {
        return this->get_type() == this_type::type::UNSIGNED_INTEGER?
            &this->storage_.unsigned_integer_: nullptr;
    }
    /** @brief MessagePackオブジェクトに格納されてる0未満の整数を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる負の整数へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは負の整数ではない。
     */
    public: PSYQ_CONSTEXPR this_type::negative_integer const* get_negative_integer()
    const PSYQ_NOEXCEPT
    {
        return this->get_type() == this_type::type::NEGATIVE_INTEGER?
            &this->storage_.negative_integer_: nullptr;
    }

    /** @brief MessagePackオブジェクトに格納されてる浮動小数点数を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる浮動小数点数へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは浮動小数点数ではない。
     */
    public: PSYQ_CONSTEXPR this_type::floating_point_64 const* get_floating_point_64()
    const PSYQ_NOEXCEPT
    {
        return this->get_type() == this_type::type::FLOATING_POINT_64?
            &this->storage_.floating_point_64_: nullptr;
    }
    /// @copydoc this_type::get_floating_point_64()
    public: PSYQ_CONSTEXPR this_type::floating_point_32 const* get_floating_point_32()
    const PSYQ_NOEXCEPT
    {
        return this->get_type() == this_type::type::FLOATING_POINT_32?
            &this->storage_.floating_point_32_: nullptr;
    }

    /** @brief MessagePackオブジェクトに格納されてる数値を取得する。

        MessagePackオブジェクトに格納されている数値をキャストして取得するので、
        格納されている値と取得した値が一致するとは限らない。

        @param[out] out_value 取得した数値が格納される。
        @retval true  MessagePackオブジェクト格納値と out_value は等値。
        @retval false MessagePackオブジェクト格納値と out_value は不等値。
     */
    public: template<typename template_value_type>
    bool get_numeric_value(template_value_type& out_value) const PSYQ_NOEXCEPT
    {
        switch (this->get_type())
        {
        case this_type::type::UNSIGNED_INTEGER:
            out_value = static_cast<template_value_type>(this->storage_.unsigned_integer_);
            return 0 <= out_value
                && static_cast<this_type::unsigned_integer>(out_value) == this->storage_.unsigned_integer_;
        case this_type::type::NEGATIVE_INTEGER:
            out_value = static_cast<template_value_type>(this->storage_.negative_integer_);
            return out_value < 0
                && static_cast<this_type::negative_integer>(out_value) == this->storage_.negative_integer_;
        case this_type::type::FLOATING_POINT_32:
            out_value = static_cast<template_value_type>(this->storage_.floating_point_32_);
            return static_cast<this_type::floating_point_32>(out_value) == this->storage_.floating_point_32_;
        case this_type::type::FLOATING_POINT_64:
            out_value = static_cast<template_value_type>(this->storage_.floating_point_64_);
            return static_cast<this_type::floating_point_64>(out_value) == this->storage_.floating_point_64_;
        default:
            //out_value = template_value_type();
            return false; // 数値以外が格納されていたので、必ず不等値となる。
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
    public: PSYQ_CONSTEXPR this_type::string const* get_string() const PSYQ_NOEXCEPT
    {
        return this->get_type() == this_type::type::STRING?
            &this->storage_.string_: nullptr;
    }
    /** @brief MessagePackオブジェクトに格納されてるバイナリを取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてるバイナリへのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのはバイナリではない。
     */
    public: PSYQ_CONSTEXPR this_type::binary const* get_binary() const PSYQ_NOEXCEPT
    {
        return this->get_type() == this_type::type::BINARY?
            &this->storage_.binary_: nullptr;
    }
    /** @brief MessagePackオブジェクトに格納されてる拡張バイナリを取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる拡張バイナリへのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは拡張バイナリではない。
     */
    public: PSYQ_CONSTEXPR this_type::extended const* get_extended()
    const PSYQ_NOEXCEPT
    {
        return this->get_type() == this_type::type::EXTENDED?
            &this->storage_.extended_: nullptr;
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
    public: PSYQ_CONSTEXPR this_type::array* get_array() PSYQ_NOEXCEPT
    {
        return this->get_type() == this_type::type::ARRAY?
            &this->storage_.array_: nullptr;
    }
    /// @copydoc get_array()
    public: PSYQ_CONSTEXPR this_type::array const* get_array() const PSYQ_NOEXCEPT
    {
        return const_cast<this_type*>(this)->get_array();
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
    public: PSYQ_CONSTEXPR this_type::unordered_map* get_unordered_map() PSYQ_NOEXCEPT
    {
        return this->get_type() == this_type::type::UNORDERED_MAP
            || this->get_type() == this_type::type::MAP?
                &this->storage_.map_: nullptr;
    }
    /// @copydoc get_map()
    public: PSYQ_CONSTEXPR this_type::unordered_map const* get_unordered_map()
    const PSYQ_NOEXCEPT
    {
        return const_cast<this_type*>(this)->get_unordered_map();
    }

    /** @brief MessagePackオブジェクトに格納されてる連想配列を取得する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる連想配列へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは連想配列ではない。
     */
    public: PSYQ_CONSTEXPR this_type::map* get_map() PSYQ_NOEXCEPT
    {
        return this->get_type() == this_type::type::MAP?
            &this->storage_.map_: nullptr;
    }
    /// @copydoc get_map()
    public: PSYQ_CONSTEXPR this_type::map const* get_map() const PSYQ_NOEXCEPT
    {
        return const_cast<this_type*>(this)->get_map();
    }

    /** @brief MessagePackオブジェクトに格納されてる unordered_map を並び替え、
               map に変換する。
        @retval !=nullptr
            MessagePackオブジェクトに格納されてる連想配列へのポインタ。
        @retval ==nullptr
            MessagePackオブジェクトに格納されてるのは連想配列ではない。
     */
    public: this_type::map const* sort_map()
    {
        switch (this->get_type())
        {
        case this_type::type::UNORDERED_MAP:
            this->storage_.map_.sort();
            this->type_ = this_type::type::MAP;
            return &this->storage_.map_;
        case this_type::type::MAP:
            return &this->storage_.map_;
        default:
            return nullptr;
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief 有符号整数値のMessagePackオブジェクト種別を判定する。
        @param[in] in_integer 判定する整数値。
        @retval this_type::type_NEGATIVE_INTEGER 0未満の整数だった。
        @retval this_type::type_UNSIGNED_INTEGER 0以上の整数だった。
     */
    private: template<typename template_signed_type>
    PSYQ_CONSTEXPR static this_type::type tell_signed_integer_type(
        template_signed_type const in_integer)
    PSYQ_NOEXCEPT
    {
        static_assert(
            !std::is_unsigned<template_signed_type>::value,
            "template_signed_type is not signed integer type.");
        return in_integer < 0?
            this_type::type::NEGATIVE_INTEGER: this_type::type::UNSIGNED_INTEGER;
    }

    //-------------------------------------------------------------------------
    private: this_type::storage storage_; ///< @copydoc this_type::storage
    private: this_type::type type_;       ///< @copydoc this_type::type
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 直列化復元の際に、最上位となるMessagePackオブジェクト。
    @tparam template_pool @copydoc psyq::message_pack::object::root::pool
    @sa psyq::message_pack::deserializer
 */
template<typename template_pool>
class psyq::message_pack::object::root: public psyq::message_pack::object
{
    private: typedef root this_type; ///< thisが指す値の型。
    public: typedef psyq::message_pack::object base_type; ///< this_typeの基底型。

    /// psyq::message_pack::pool 互換のメモリ割当子。下位オブジェクトを保持する。
    public: typedef template_pool pool;

    //-------------------------------------------------------------------------
    /// @name 構築
    //@{
    /// @brief 空のMessagePackオブジェクトを構築する。
    public: root() {}

    /** @brief 最上位のMessagePackオブジェクトを構築する。
        @param[in] in_root 最上位のMessagePackオブジェクト。
        @param[in] in_pool 下位オブジェクトを保持するメモリ割当子。
     */
    public: root(base_type const& in_root, typename this_type::pool in_pool):
        base_type(in_root),
        pool_(std::move(in_pool))
    {}

    /** @brief move構築子。
        @param[in,out] io_source 移動元。
     */
    public: root(this_type&& io_source):
        base_type(io_source),
        pool_(std::move(io_source.reset()))
    {}

    /** @brief move代入演算子。
        @param[in,out] io_source 移動元。
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->base_type::operator=(io_source);
            this->pool_ = std::move(io_source.reset());
        }
        return *this;
    }
    //@}
    /// copy構築子は使用禁止。
    private: root(this_type const&);// = delete;
    /// copy代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);// = delete;

    //-------------------------------------------------------------------------
    /// @name インスタンス変数の操作
    //@{
    /** @brief 空にする。
        @return 下位オブジェクトを保持するのに使っていたメモリ割当子。
     */
    public: typename this_type::pool reset()
    {
        this->base_type::reset();
        auto local_pool(std::move(this->pool_));
        return local_pool;
    }

    /** @brief 下位オブジェクトを保持するメモリ割当子を取得する。
        @return 下位オブジェクトを保持するメモリ割当子。
     */
    public: typename this_type::pool const& get_pool() const
    {
        return this->pool_;
    }
    //@}
    //-------------------------------------------------------------------------
    private: typename this_type::pool pool_; ///< @copydoc pool
}; // class psyq::message_pack::object::root

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace message_pack
    {
        namespace _private
        {
            /// @cond
            template<>
            inline int compare_value(
                psyq::message_pack::object const& in_left,
                psyq::message_pack::object const& in_right)
            {
                return in_left.compare(in_right);
            }
            template<>
            inline int compare_value(
                psyq::message_pack::object::map::value_type const& in_left,
                psyq::message_pack::object::map::value_type const& in_right)
            {
                return in_left.first.compare(in_right.first);
            }
            template<>
            inline bool equal_value(
                psyq::message_pack::object::map::value_type const& in_left,
                psyq::message_pack::object::map::value_type const& in_right)
            {
                return in_left.first == in_right.second;
            }
            /// @endcond
        } // namespace _private
    } // namespace message_pack
} // namespace psyq

#endif // PSYQ_MESSAGE_PACK_OBJECT_HPP_
