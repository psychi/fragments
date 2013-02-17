/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/** @brief PBON/JSON is a library for translating between PBON and JSON.

    PBON/JSONは、PBONとJSONの相互変換を行うためのlibrary。
    PBONは、Packed-Binary-Object-Notationの略。

    - Call pbon::json::value constructor, import the JSON.
      pbon::json::value のconstructorを呼び出し、JSONから値を取り出す。

    @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PBON_JSON_HPP_
#define PBON_JSON_HPP_

namespace pbon
{
typedef std::int8_t  int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;
typedef int8  char8;
typedef int16 char16;
typedef int32 char32;
typedef float  float32;
typedef double float64;

enum type
{
    type_NULL,
    type_BOOL,
    type_ARRAY,
    type_OBJECT,
    type_CHAR8 = 1 << 4,
    type_CHAR16,
    type_CHAR32,
    type_INT8 = 2 << 4,
    type_INT16,
    type_INT32,
    type_INT64,
    type_FLOAT32 = (3 << 4) + 2,
    type_FLOAT64,
};

template< typename template_type > pbon::type get_type()
{
    return pbon::type_NULL;
}

template<> pbon::type get_type< pbon::char8 >()
{
    return pbon::type_CHAR8;
}

template<> pbon::type get_type< pbon::char16 >()
{
    return pbon::type_CHAR16;
}

template<> pbon::type get_type< pbon::int32 >()
{
    return pbon::type_INT32;
}

template<> pbon::type get_type< pbon::int64 >()
{
    return pbon::type_INT64;
}

template<> pbon::type get_type< pbon::float32 >()
{
    return pbon::type_FLOAT32;
}

template<> pbon::type get_type< pbon::float64 >()
{
    return pbon::type_FLOAT64;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief PBONの要素。
    @tparam template_attribute_type @copydoc pbon::value::attribute
 */
template< typename template_attribute_type >
class value
{
    /// thisが指す型。
    public: typedef value self;

    /// packed-binaryの属性の型。
    public: typedef template_attribute_type attribute;

    /// packed-binaryのheader情報。
    private: struct header
    {
        pbon::int32 endian_;
        pbon::int16 type_;
        pbon::int16 root_;
    };

    /** @brief packed-binaryの最上位要素を取得。
        @param[in] in_packed_binary packed-binaryの先頭位置。
        @retval !=NULL packed-binaryの最上位要素へのpointer。
        @retval ==NULL 扱えないpacked-binaryだった。
     */
    public: static const self* get_root(
        const void* const in_packed_binary)
    {
        if (in_packed_binary == NULL)
        {
            return NULL;
        }
        const self::header& local_header(
            *static_cast< const self::header* >(in_packed_binary));
        if (local_header.endian_ != 'pbon')
        {
            // endianが異なるので扱えない。
            return NULL;
        }
        if (local_header.type_ != pbon::get_type< self::attribute >())
        {
            // 属性の型が異なるので扱えない。
            return NULL;
        }
        return self::get_address< self >(in_packed_binary, local_header.root_);
    }

    /** @brief 上位要素を取得。
        @retval !=NULL 上位要素へのpointer。
        @retval ==NULL 上位要素がない。
     */
    public: const self* get_super() const
    {
        if (this->super_ == 0)
        {
            return NULL;
        }
        return self::get_address< self >(this, this->super_);
    }

    /** @brief 持っている値の数を取得。
     */
    public: std::size_t get_size() const
    {
        return this->size_;
    }

    /** @brief 持っている値の型を取得。
     */
    public: pbon::type get_type() const
    {
        return static_cast< pbon::type >(this->type_);
    }

    public: bool is_array() const
    {
        return this->get_type() == pbon::type_ARRAY;
    }

    public: bool is_object() const
    {
        return this->get_type() == pbon::type_OBJECT;
    }

    /** @brief 持っている値へのpointerを取得。
        @tparam template_valuetype 持っている値の型。
     */
    protected: template< typename template_valuetype >
    const template_valuetype* get_value() const
    {
        return self::get_address< template_valuetype >(this, this->value_);
    }

    /** @brief 相対位置からaddressを取得。
        @tparam template_value_type    pointerが指す値の型。
        @tparam template_position_type 相対位置の型。
        @param[in] in_base_address     基準位置となるpointer。
        @param[in] in_byte_position    基準位置からのbyte単位での相対位置。
     */
    private: template<
        typename template_value_type,
        typename template_position_type >
    static const template_value_type* get_address(
        const void* const            in_base_address,
        const template_position_type in_byte_position)
    {
        return reinterpret_cast< const template_value_type* >(
            static_cast< const char* >(in_base_address) + in_byte_position);
    }

    private: typename self::attribute value_; ///< 値。もしくは値への相対位置。
    private: typename self::attribute size_;  ///< 値の数。
    private: typename self::attribute type_;  ///< 値の型。
    private: typename self::attribute super_; ///< 上位要素への相対位置。
};
typedef pbon::value< pbon::int32 > value32;

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// pbon::value の配列。
template< typename template_attribute_type >
class array:
    private pbon::value< template_attribute_type >
{
    public: typedef pbon::array< template_attribute_type > self;
    private: typedef pbon::value< template_attribute_type > super;
    public: typedef super value;

    using super::attribute;
    using super::get_super;

    public: static const self* cast(
        const super* const in_value)
    {
        if (in_value == NULL || !in_value->is_array())
        {
            return NULL;
        }
        return static_cast< const self* >(in_value);
    }

    /** @brief 持っている値の数を取得。
     */
    public: std::size_t get_size() const
    {
        if (!this->is_array())
        {
            return 0;
        }
        return this->super::get_size();
    }

    public: const typename self::value* get_begin() const
    {
        if (!this->is_array())
        {
            return NULL;
        }
        return this->get_value< typename self::value >();
    }

    public: const typename self::value* get_end() const
    {
        if (!this->is_array())
        {
            return NULL;
        }
        return this->get_value< typename self::value >() + this->get_size();
    }

    public: const typename self::value* at(
        const std::size_t in_index)
    {
        if (!this->is_array() || this->get_size() <= in_index)
        {
            return NULL;
        }
        return this->get_value< self::value >() + in_index;
    }
};
typedef pbon::array< pbon::int32 > array32;

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// pbon::value の辞書。
template< typename template_attribute_type >
class object:
    private value< template_attribute_type >
{
    public: typedef pbon::object< template_attribute_type > self;
    private: typedef pbon::value< template_attribute_type > super;
    public: typedef std::pair< super, super > value;

    using super::attribute;
    using super::get_super;

    public: static const self* cast(
        const super* const in_value)
    {
        if (in_value == NULL || !in_value->is_object())
        {
            return NULL;
        }
        return static_cast< const self* >(in_value);
    }

    /** @brief 持っている値の数を取得。
     */
    public: std::size_t get_size() const
    {
        if (!this->is_object())
        {
            return 0;
        }
        return this->super::get_size() / 2;
    }

    public: const typename self::value* get_begin() const
    {
        if (!this->is_object())
        {
            return NULL;
        }
        return this->get_value< typename self::value >();
    }

    public: const typename self::value* get_end() const
    {
        if (!this->is_object())
        {
            return NULL;
        }
        return this->get_value< typename self::value >() + this->get_size();
    }

    public: template< typename template_key_type >
    const typename self::value* lower_bound(
        const template_key_type& in_key) const;

    public: template< typename template_key_type >
    const typename self::value* upper_bound(
        const template_key_type& in_key) const;

    public: template< typename template_key_type >
    const typename self::value* find(
        const template_key_type& in_key) const;
};
typedef pbon::object< pbon::int32 > object32;

namespace json
{

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief JSONから取り出した値を保持する。
 */
class value
{
    /// thisが指す値の型。
    public: typedef pbon::json::value self;

    /** @brief JSONの解析結果。

        - 成功した場合は[0, 0]。
        - 失敗した場合は、取り込みに失敗した文字位置の[行番号, 桁位置]。
     */
    public: typedef std::pair< unsigned, unsigned > parse_result;

    //-------------------------------------------------------------------------
    /// @brief 空の値を構築。
    public: value():
    holder_(NULL)
    {
        // pass
    }

    /** @brief copy constructor
        @param[in] in_source 代入元となる値。
     */
    public: value(
        const self& in_source):
    holder_(NULL)
    {
        if (in_source.holder_ != NULL)
        {
            this->holder_ = in_source.holder_->create_clone();
        }
    }

    /** @brief JSON形式の文字列を解析し、値を取り出す。
        @tparam template_string_type 解析する文字列の型。
        @param[in]  in_string  値を取り込むJSON形式の文字列。
        @param[out] out_result
            @copydoc pbon::json::value::parse_result
     */
    public: template< typename template_string_type >
    value(
        const template_string_type& in_string,
        self::parse_result&         out_result)
    {
        new(this) self(type_traits<>(), in_string, out_result);
    }

    /** @brief JSON形式の文字列を解析し、値を取り出す。
        @tparam template_traits_type
            pbon::json::type_traits に準拠した型特性の型。
        @tparam template_string_type 解析する文字列の型。
        @param[in]  in_type_traits pbon::json::type_traits に準拠した型特性。
        @param[in]  in_string      値を取り込むJSON形式の文字列。
        @param[out] out_result
            @copydoc pbon::json::value::parse_result
     */
    public: template<
        typename template_traits_type,
        typename template_string_type >
    value(
        const template_traits_type& in_type_traits,
        const template_string_type& in_string,
        self::parse_result&         out_result)
    {
        typename template_traits_type::allocator local_allocator;
        new(this) self(in_type_traits, in_string, local_allocator, out_result);
    }

    /** @brief JSON形式の文字列を解析し、値を取り出す。
        @tparam template_traits_type
            pbon::json::type_traits に準拠した型特性の型。
        @tparam template_string_type 解析する文字列の型。
        @tparam template_allocator_type
            @copydoc pbon::json::type_traits::allocator
        @param[in]     in_type_traits pbon::json::type_traits に準拠した型特性。
        @param[in]     in_string      値を取り込むJSON形式の文字列。
        @param[in,out] io_allocator   使用するmemory割当子。
        @param[out]    out_result
            @copydoc pbon::json::value::parse_result
     */
    public: template<
        typename template_traits_type,
        typename template_string_type,
        typename template_allocator_type >
    value(
        const template_traits_type& in_type_traits,
        const template_string_type& in_string,
        template_allocator_type&    io_allocator,
        self::parse_result&         out_result)
    {
        new(this) self(
            in_type_traits,
            in_string.begin(),
            in_string.end(),
            io_allocator,
            out_result);
    }

    /** @brief JSON形式の文字列を解析し、値を取り出す。
        @tparam template_traits_type
            pbon::json::type_traits に準拠した型特性の型。
        @tparam template_iterator_type @copydoc pbon::json::parser::iterator
        @tparam template_allocator_type
            @copydoc pbon::json::type_traits::allocator
        @param[in]     in_type_traits  型特性。
        @param[in]     in_string_begin 解析する文字列の先頭位置。
        @param[in]     in_string_end   解析する文字列の末尾位置。
        @param[in,out] io_allocator    使用するmemory割当子。
        @param[out]    out_result
            @copydoc pbon::json::value::parse_result
     */
    public: template<
        typename template_traits_type,
        typename template_iterator_type,
        typename template_allocator_type >
    value(
        const template_traits_type&   in_type_traits,
        const template_iterator_type& in_string_begin,
        const template_iterator_type& in_string_end,
        template_allocator_type&      io_allocator,
        self::parse_result&           out_result):
    holder_(NULL)
    {
        // 使わない引数。
        in_type_traits;

        // JSON文字列を解析して値を取り出す。
        self local_value;
        pbon::json::parser<
            template_iterator_type,
            typename template_traits_type::number,
            typename template_traits_type::string,
            typename template_traits_type::array,
            typename template_traits_type::object >
                local_parser(
                    in_string_begin, in_string_end, io_allocator, local_value);
        if (local_parser.get_line() == 0)
        {
            this->swap(local_value);
        }
        out_result = self::parse_result(
            local_parser.get_line(), local_parser.get_column());
    }

    /** @brief 任意型の値を構築。

        @tparam template_value_type     値の型。
        @tparam template_allocator_type memory割当子の型。
        @param[in] in_value     初期値。
        @param[in] io_allocator memory割当子の初期値。
     */
    public: template<
        typename template_value_type,
        typename template_allocator_type >
    value(
        const template_value_type&     in_value,
        const template_allocator_type& in_allocator)
    {
        typedef typename template_allocator_type::template
            rebind< template_value_type >::other
                allocator;
        this->holder_ = holder< allocator >::create(
            in_value, in_allocator);
    }

    /// @brief destructor
    ~value()
    {
        if (this->holder_ != NULL)
        {
            this->holder_->destroy_this();
        }
    }

    /** @brief 代入演算子。
        @param[in] in_source 代入元となる値。
     */
    public: self& operator=(const self& in_source)
    {
        if (this == &in_source)
        {
            return *this;
        }
        this->~value();
        return *new(this) self(in_source);
    }

    /** @brief 値を交換。
        @param[in,out] io_target 値を交換する対象。
     */
    public: void swap(self& io_target)
    {
        std::swap(this->holder_, io_target.holder_);
    }

    /** @brief 値が空か判定。
        @retval true  値が空だった。
        @retval false 値が空ではなかった。
     */
    public: bool empty() const
    {
        return this->holder_ == NULL;
    }

    //-------------------------------------------------------------------------
    private: class placeholder
    {
        public: typedef placeholder self;

        public: placeholder() {}

        /** @brief *thisのdeep-copyを作成。
         */
        public: virtual placeholder* create_clone() const = 0;

        /** @brief *thisを破棄。
         */
        public: virtual void destroy_this() = 0;

        /// copy-constructorは使用禁止。
        private: placeholder(const self&);

        /// 代入演算子は使用禁止。
        private: self& operator=(const self&);
    };

    private: template< typename template_allocator_type >
    class holder:
        public placeholder
    {
        /// this が指す値の型。
        public: typedef holder< template_allocator_type > self;

        /// self の上位型。
        public: typedef placeholder super;

        /// holder が持つmemory割当子の型。
        public: typedef typename template_allocator_type::template
            rebind< self >::other
                allocator;

        /// holder が持つ値の型。
        public: typedef typename template_allocator_type::value_type value;

        /** @param[in] holder が持つ値の初期値。
            @param[in] holder が持つmemory割当子の初期値。
         */
        private: holder(
            const typename self::value&     in_value,
            const typename self::allocator& in_allocator):
        super(),
        value_(in_value),
        allocator_(in_allocator)
        {
            // pass
        }

        /** @brief holder を構築。
            @param[in] holder が持つ値の初期値。
            @param[in] holder が持つmemory割当子の初期値。
         */
        public: template< typename template_other_allocator_type >
        static self* create(
            const typename self::value&          in_value,
            const template_other_allocator_type& in_allocator)
        {
            self::allocator local_allocator(in_allocator);
            self* const local_holder(local_allocator.allocate(1));
            if (local_holder != NULL)
            {
                new(local_holder) self(in_value, local_allocator);
            }
            return local_holder;
        }

        public: virtual super* create_clone() const
        {
            return self::create(this->value_, this->allocator_);
        }

        public: virtual void destroy_this()
        {
            typename self::allocator local_allocator(this->allocator_);
            this->~self();
            local_allocator.deallocate(this, 1);
        }

        private: typename self::value     value_;
        private: typename self::allocator allocator_;
    };

    //-------------------------------------------------------------------------
    private: self::placeholder* holder_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief pbon::json::value に持たせる値の型特性。
    @tparam template_number_type    @copydoc type_traits::number
    @tparam template_string_type    @copydoc type_traits::string
    @tparam template_allocator_type @copydoc type_traits::allocator
 */
template<
    typename template_number_type = double,
    typename template_string_type = std::string,
    typename template_allocator_type = std::allocator< void* > >
struct type_traits
{
    /** @brief pbon::json::value に持たせる数値の型。

        文字列から数値に変換するため、以下に相当する関数が使えること。
        @code
        operator>>(
            std::basic_istringstream<
                pbon::json::type_traits::string::value_type >&,
            pbon::json::type_traits::number&)
        @endcode
     */
    typedef template_number_type number;

    /** @brief pbon::json::value に持たせる文字列の型。

        末尾に文字を追加するため、以下に相当する関数が使えること。
        @code
        pbon::json::type_traits::string::push_back(
            const pbon::json::type_traits::string::value_type)
        @endcode
     */
    typedef template_string_type string;

    /** @brief pbon::json::value に持たせる配列の型。

        末尾に値を追加するため、以下に相当する関数が使えること。
        @code
        pbon::json::type_traits::array::push_back(const pbon::json::value&)
        @endcode

        値の数を取得するため、以下に相当する関数が使えること。
        @code
        pbon::json::type_traits::array::size_type
            pbon::json::type_traits::array::size()
        @endcode
     */
    typedef std::list< pbon::json::value, template_allocator_type > array;

    /** @brief pbon::json::value に持たせるobjectの型。

        値を挿入するため、以下に相当する関数が使えること。
        @code
        pbon::json::value& pbon::json::type_traits::object::operator[](
            const pbon::json::type_traits::string&)
        @endcode

        keyの数を取得するため、以下に相当する関数が使えること。
        @code
        pbon::json::type_traits::object::size_type
            pbon::json::type_traits::object::count(
                const pbon::json::type_traits::string&)
        @endcode
     */
    typedef std::map<
        template_string_type,
        pbon::json::value,
        std::less< template_string_type >,
        template_allocator_type >
            object;

    /** @brief std::allocator 互換のinterfaceを持つmemory割当子。
     */
    typedef template_allocator_type allocator;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief JSON形式の文字列を解析し、値を取り出す。
    @tparam template_iterator_type @copydoc pbon::json::parser::iterator
    @tparam template_number_type   @copydoc pbon::json::type_traits::number
    @tparam template_string_type   @copydoc pbon::json::type_traits::string
    @tparam template_array_type    @copydoc pbon::json::type_traits::array
    @tparam template_object_type   @copydoc pbon::json::type_traits::object
 */
template<
    typename template_iterator_type,
    typename template_number_type,
    typename template_string_type,
    typename template_array_type,
    typename template_object_type >
class parser
{
    /// thisが指す値の型。
    public: typedef parser<
        template_iterator_type,
        template_number_type,
        template_string_type,
        template_array_type,
        template_object_type >
            self;

    //-------------------------------------------------------------------------
    /// JSONの解析に使う反復子の型。
    public: typedef template_iterator_type iterator;

    /// @copydoc pbon::json::type_traits::number
    public: typedef template_number_type number;

    /// @copydoc pbon::json::type_traits::string
    public: typedef template_string_type string;

    /// @copydoc pbon::json::type_traits::array
    public: typedef template_array_type array;

    /// @copydoc pbon::json::type_traits::object
    public: typedef template_object_type object;

    //-------------------------------------------------------------------------
    /** @brief JSON形式の文字列を解析し、値を取り出す。
        @tparam template_allocator_type
            @copydoc pbon::json::type_traits::allocator
        @param[in]     in_begin     解析する文字列の先頭位置。
        @param[in]     in_end       解析する文字列の末尾位置。
        @param[in,out] io_allocator 使用するmemory割当子。
        @param[out]    out_value    JSONから取り出した値の出力先。
     */
    public: template< typename template_allocator_type >
    parser(
        const template_iterator_type& in_begin,
        const template_iterator_type& in_end,
        template_allocator_type&      io_allocator,
        pbon::json::value&            out_value):
    current_(in_begin),
    end_(in_end),
    last_char_(self::END_CHAR),
    line_(1),
    column_(1),
    undo_(false)
    {
        if (this->parse(io_allocator, out_value))
        {
            this->line_ = 0;
            this->column_ = 0;
        }
    }

    //-------------------------------------------------------------------------
    /** @brief 直前に解析した文字の行番号を取得。
        @retval >0  直前に解析した文字の行番号。
        @retval <=0 解析は成功して終了した。
     */
    public: unsigned get_line() const
    {
        return this->line_;
    }

    /** @brief 直前に解析した文字の桁位置を取得。
        @retval >0  直前に解析した文字の桁位置。
        @retval <=0 解析は成功して終了した。
     */
    public: unsigned get_column() const
    {
        return this->column_;
    }

    //-------------------------------------------------------------------------
    /** @brief JSONが持つ値を解析して取り出す。
        @tparam template_allocator_type
            @copydoc pbon::json::type_traits::allocator
        @param[in,out] io_allocator 使用するmemory割当子。
        @param[out]    out_value    JSONから取り出した値の出力先。
        @retval true  成功。
        @retval false 失敗。値は出力されない。
     */
    private: template< typename template_allocator_type >
    bool parse(
        template_allocator_type& io_allocator,
        pbon::json::value&       out_value)
    {
        this->skip_white_space();
        const int local_char(this->read_char());
        switch (local_char)
        {
            case '"':
            return this->parse_string(io_allocator, out_value);

            case '[':
            return this->parse_array(io_allocator, out_value);

            case '{':
            return this->parse_object(io_allocator, out_value);

            case 'n':
            if (this->match("ull"))
            {
                pbon::json::value().swap(out_value);
                return true;
            }
            return false;

            case 't':
            if (this->match("rue"))
            {
                pbon::json::value(true, io_allocator).swap(out_value);
                return true;
            }
            return false;

            case 'f':
            if (this->match("alse"))
            {
                pbon::json::value(false, io_allocator).swap(out_value);
                return true;
            }
            return false;

            case self::END_CHAR:
            return false;

            default:
            this->undo_char();
            if (('0' <= local_char && local_char <= '9') ||
                local_char == '-')
            {
                return this->parse_number(io_allocator, out_value);
            }
            return false;
        }
    }

    /** @brief JSONが持っている配列を解析して取り出す。
        @tparam template_allocator_type
            @copydoc pbon::json::type_traits::allocator
        @param[in,out] io_allocator 使用するmemory割当子。
        @param[out]    out_value    JSONから取り出した配列の出力先。
        @retval true  成功。
        @retval false 失敗。配列は出力されない。
     */
    private: template< typename template_allocator_type >
    bool parse_array(
        template_allocator_type& io_allocator,
        pbon::json::value&       out_value)
    {
        template_array_type local_array;
        if (!this->expect(']'))
        {
            for (;;)
            {
                const typename template_array_type::size_type local_size(
                    local_array.size());
                local_array.push_back(pbon::json::value());
                if (local_array.size() != local_size + 1 ||
                    !this->parse(io_allocator, local_array.back()))
                {
                    return false;
                }
                if (!this->expect(','))
                {
                    break;
                }
            }
        }
        pbon::json::value(local_array, io_allocator).swap(out_value);
        return this->expect(']');
    }

    /** @brief JSONが持っているobjectを解析して取り出す。
        @tparam template_allocator_type
            @copydoc pbon::json::type_traits::allocator
        @param[in,out] io_allocator 使用するmemory割当子。
        @param[out]    out_value    JSONから取り出したobjectの出力先。
        @retval true  成功。
        @retval false 失敗。objectは出力されない。
     */
    private: template< typename template_allocator_type >
    bool parse_object(
        template_allocator_type& io_allocator,
        pbon::json::value&       out_value)
    {
        if (this->expect('}'))
        {
            return true;
        }
        template_object_type local_object;
        for (;;)
        {
            template_string_type local_key;
            pbon::json::value local_value;
            if (!this->expect('"') ||
                !this->parse_string(local_key) ||
                !this->expect(':') ||
                !this->parse(io_allocator, local_value))
            {
                return false;
            }
            local_object[local_key].swap(local_value);
            if (local_object.count(local_key) <= 0)
            {
                return false;
            }
            if (!this->expect(','))
            {
                break;
            }
        }
        if (!this->expect('}'))
        {
            return false;
        }
        pbon::json::value(local_object, io_allocator).swap(out_value);
        return true;
    }

    /** @brief JSONが持っている数値を解析して取り出す。
        @tparam template_allocator_type
            @copydoc pbon::json::type_traits::allocator
        @param[in,out] io_allocator 使用するmemory割当子。
        @param[out]    out_value    JSONから取り出した数値の出力先。
        @retval true  成功。
        @retval false 失敗。数値は出力されない。
     */
    private: template< typename template_allocator_type >
    bool parse_number(
        template_allocator_type& io_allocator,
        pbon::json::value&       out_value)
    {
        // 数の文字列を取り出す。
        template_string_type local_string;
        for (;;)
        {
            const int local_char(this->read_char());
            if (('0' <= local_char && local_char <= '9') ||
                local_char == '+' ||
                local_char == '-' ||
                local_char == '.' ||
                local_char == 'e' ||
                local_char == 'E')
            {
                local_string.push_back(
                    static_cast< typename template_string_type::value_type >(
                        local_char));
            }
            else
            {
                this->undo_char();
                break;
            }
        }

        // 数の文字列を数値に変換。
        std::basic_istringstream< typename template_string_type::value_type >
            local_stream(local_string);
        template_number_type local_number;
        local_stream >> local_number;
        if (!local_stream.eof())
        {
            return false;
        }
        pbon::json::value(local_number, io_allocator).swap(out_value);
        return true;
    }

    /** @brief JSONが持っている文字列を解析して取り出す。
        @tparam template_allocator_type
            @copydoc pbon::json::type_traits::allocator
        @param[in,out] io_allocator 使用するmemory割当子。
        @param[out]    out_value    JSONから取り出した文字列の出力先。
        @retval true  成功。
        @retval false 失敗。文字列は出力されない。
     */
    private: template< typename template_allocator_type >
    bool parse_string(
        template_allocator_type& io_allocator,
        pbon::json::value&       out_value)
    {
        template_string_type local_string;
        if (!this->parse_string(local_string))
        {
            return false;
        }
        pbon::json::value(local_string, io_allocator).swap(out_value);
        return true;
    }

    /** @brief JSONが持っている文字列を解析して取り出す。
        @param[out] out_string JSONから取り出した文字列の出力先。
        @retval true  成功。
        @retval false 失敗。
     */
    private: bool parse_string(
        template_string_type& out_string)
    {
        for (;;)
        {
            int local_char(this->read_char());
            if (local_char < ' ')
            {
                this->undo_char();
                return false;
            }
            if (local_char == '"')
            {
                return true;
            }
            if (local_char == '\\')
            {
                local_char = this->read_char();
                switch (local_char)
                {
                    case '"':
                    break;

                    case '\\':
                    break;

                    case '/':
                    break;

                    case 'b':
                    local_char = '\b';
                    break;

                    case 'f':
                    local_char = '\f';
                    break;

                    case 'n':
                    local_char = '\n';
                    break;

                    case 'r':
                    local_char = '\r';
                    break;

                    case 't':
                    local_char = '\t';
                    break;

                    case 'u':
                    if (this->parse_code_point(out_string))
                    {
                        continue;
                    }
                    return false;

                    default:
                    return false;
                }
            }
            out_string.push_back(
                static_cast< typename template_string_type::value_type >(
                    local_char));
        }
    }

    /** @brief 文字列のcode-point表記を解析して取り出す。
        @param[out] out_string JSONから取り出した文字列の出力先。
        @retval true  成功。
        @retval false 失敗。
        @todo 今のところUTF-8専用。その他のUTFにも対応させたい。
     */
    private: bool parse_code_point(
        template_string_type& out_string)
    {
        int local_unicode_char(this->parse_quad_hex());
        if (local_unicode_char == self::END_CHAR)
        {
            return false;
        }
        if (0xd800 <= local_unicode_char && local_unicode_char <= 0xdfff)
        {
            // 16bit surrogate pairの後半か判定。
            if (0xdc00 <= local_unicode_char)
            {
                return false;
            }

            // 16bit surrogate pairの前半だった。
            if (this->read_char() != '\\' || this->read_char() != 'u')
            {
                this->undo_char();
                return false;
            }
            const int local_second(this->parse_quad_hex());
            if (local_second < 0xdc00 || 0xdfff < local_second)
            {
                return false;
            }
            local_unicode_char = 0x10000 + (
                ((local_unicode_char - 0xd800) << 10) |
                ((local_second - 0xdc00) & 0x3ff));
        }
        if (local_unicode_char < 0x80)
        {
            out_string.push_back(
                static_cast< template_string_type::value_type >(
                    local_unicode_char));
            return true;
        }

        // UTF-8に変換して格納。
        if (local_unicode_char < 0x800)
        {
            out_string.push_back(
                static_cast< template_string_type::value_type >(
                    0xc0 | (local_unicode_char >> 6)));
        }
        else
        {
            if (local_unicode_char < 0x10000)
            {
                out_string.push_back(
                    static_cast< template_string_type::value_type >(
                        0xe0 | (local_unicode_char >> 12)));
            }
            else
            {
                out_string.push_back(
                    static_cast< template_string_type::value_type >(
                        0xf0 | (local_unicode_char >> 18)));
                out_string.push_back(
                    static_cast< template_string_type::value_type >(
                        0x80 | ((local_unicode_char >> 12) & 0x3f)));
            }
            out_string.push_back(
                static_cast< template_string_type::value_type >(
                    0x80 | ((local_unicode_char >> 6) & 0x3f)));
        }
        out_string.push_back(
            static_cast< template_string_type::value_type >(
                0x80 | (local_unicode_char & 0x3f)));
        return true;
    }

    /** @brief 文字列の16進数表記4桁を解析して取り出す。
     */
    private: int parse_quad_hex()
    {
        int local_unicode_char(0);
        for (int i = 0; i < 4; i++)
        {
            int local_hex(this->read_char());
            if ('0' <= local_hex && local_hex <= '9')
            {
                local_hex -= '0';
            }
            else if ('A' <= local_hex && local_hex <= 'F')
            {
                local_hex -= 'A' - 0xa;
            }
            else if ('a' <= local_hex && local_hex <= 'f')
            {
                local_hex -= 'a' - 0xa;
            }
            else
            {
                if (local_hex != self::END_CHAR)
                {
                    this->undo_char();
                }
                return self::END_CHAR;
            }
            local_unicode_char = local_unicode_char * 16 + local_hex;
        }
        return local_unicode_char;
    }

    //-------------------------------------------------------------------------
    /** @brief 1文字読み込む。
        @return 読み込んだ文字。
     */
    private: int read_char()
    {
        if (this->undo_)
        {
            this->undo_ = false;
            return this->last_char_;
        }
        if (this->current_ == this->end_)
        {
            this->last_char_ = self::END_CHAR;
            return self::END_CHAR;
        }
        if (this->last_char_ == '\n')
        {
            this->column_ = 1;
            ++this->line_;
        }
        else
        {
            ++this->column_;
        }
        this->last_char_ = *this->current_;
        ++this->current_;
        return this->last_char_;
    }

    private: void undo_char()
    {
        if (this->last_char_ != self::END_CHAR)
        {
            //assert(!this->undo_);
            this->undo_ = true;
        }
    }

    /** @brief 空白文字をskipする。
     */
    private: void skip_white_space()
    {
        for (;;)
        {
            switch (this->read_char())
            {
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                break;

                default:
                this->undo_char();
                return;
            }
        }
    }

    private: bool expect(
        const int in_expect_char)
    {
        this->skip_white_space();
        if (this->read_char() != in_expect_char)
        {
            this->undo_char();
            return false;
        }
        return true;
    }

    private: bool match(
        const char* const in_begin)
    {
        for (const char* i = in_begin; *i != 0; ++i)
        {
            if (this->read_char() != *i)
            {
                this->undo_char();
                return false;
            }
        }
        this->skip_white_space();
        switch (this->last_char_)
        {
            case ',':
            case ']':
            case '}':
            case self::END_CHAR:
            return true;

            default:
            return false;
        }
    }

    private: enum
    {
        END_CHAR = -1 ///< 末尾位置を表す文字。
    };

    //-------------------------------------------------------------------------
    private: template_iterator_type current_;
    private: template_iterator_type end_;
    private: int      last_char_;
    private: unsigned line_;
    private: unsigned column_;
    private: bool     undo_;
};

} // namespace json
} // namespace pbon

#endif // PBON_JSON_HPP_
