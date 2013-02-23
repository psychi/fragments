#ifndef PSYQ_HASH_STRING_HPP_
#define PSYQ_HASH_STRING_HPP_

namespace psyq
{
    /// @cond
    template<typename, typename> class hash_string;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief hash値を使って比較を行う文字列。
    @tparam template_string_type @copydoc hash_string::super
    @tparam template_hash_type   @copydoc hash_string::hasher
 */
template<
    typename template_string_type,
    typename template_hash_type>
class psyq::hash_string:
    public template_string_type
{
    /// thisが指す値の型。
    public: typedef psyq::hash_string<
        template_string_type, template_hash_type >
            self;

    /// self の上位型。 std::basic_string とinterfaceが互換。
    public: typedef template_string_type super;

    /// hash値を生成するhash生成子の型。
    public: typedef template_hash_type hasher;

    //-------------------------------------------------------------------------
    /** @brief default-constructor
        @param[in] in_allocator  memory割当子の初期値。
        @param[in] in_hasher hash生成子の初期値。
     */
    public: explicit hash_string(
        typename super::allocator_type const& in_allocator =
            super::allocator_type(),
        template_hash_type const&             in_hasher =
            template_hash_type())
    :
        super(in_allocator),
        hash_(template_hash_type::EMPTY),
        hasher_(in_hasher)
    {
        // pass
    }

    /** @brief 変換constructor
        @param[in] in_string     初期値に使う文字列。
        @param[in] in_offset     文字列の開始位置。
        @param[in] in_count      文字数。
        @param[in] in_allocator  memory割当子の初期値。
        @param[in] in_hasher hash生成子の初期値。
     */
    public: template<typename template_other_string_type>
    hash_string(
        template_other_string_type const&     in_string,
        typename super::size_type const       in_offset = 0,
        typename super::size_type const       in_count = super::npos,
        typename super::allocator_type const& in_allocator =
            typename super::allocator_type(),
        template_hash_type const&             in_hasher =
            template_hash_type())
    :
        super(in_string, in_offset, in_count, in_allocator),
        hash_(template_hash_type::EMPTY),
        hasher_(in_hasher)
    {
        // pass
    }

    /** @param[in] in_string     初期値に使う文字列の先頭位置。NULL文字で終わる。
        @param[in] in_allocator  memory割当子の初期値。
        @param[in] in_hasher hash生成子の初期値。
     */
    public: explicit hash_string(
        typename super::const_pointer const   in_string,
        typename super::allocator_type const& in_allocator =
            super::allocator_type(),
        template_hash_type const&             in_hasher =
            template_hash_type())
    :
        super(in_string, in_allocator),
        hash_(template_hash_type::EMPTY),
        hasher_(in_hasher)
    {
        // pass
    }

    /** @param[in] in_string     初期値に使う文字列の先頭位置。
        @param[in] in_size       初期値に使う文字列の長さ。
        @param[in] in_allocator  memory割当子の初期値。
        @param[in] in_hasher hash生成子の初期値。
     */
    public: hash_string(
        typename super::const_pointer const   in_string,
        typename super::size_type const       in_size,
        typename super::allocator_type const& in_allocator =
            typename super::allocator_type(),
        template_hash_type const&             in_hasher =
            template_hash_type())
    :
        super(in_string, in_size, in_allocator),
        hash_(template_hash_type::EMPTY),
        hasher_(in_hasher)
    {
        // pass
    }

    /** @brief 文字列literalから構築。
        @param[in] in_offset     文字列literalの開始offset位置。
        @param[in] in_count      文字数。
        @param[in] in_string     初期値に使う文字列literal。
        @param[in] in_allocator  memory割当子の初期値。
        @param[in] in_hasher hash生成子の初期値。
     */
    public: template <std::size_t template_size>
    hash_string(
        typename self::size_type              in_offset,
        typename self::size_type              in_count,
        typename super::value_type const      (&in_string)[template_size],
        typename super::allocator_type const& in_allocator =
            typename super::allocator_type(),
        template_hash_type const&             in_hasher =
            template_hash_type())
    :
        super(&in_string[in_offset], in_count, in_allocator)
        hash_(template_hash_type::EMPTY),
        hasher_(in_hasher)
    {
        // pass
    }

    //-------------------------------------------------------------------------
    /** copy演算子。
        @param[in] in_right copy元となる値。
     */
    public: self& operator=(
        super const& in_right)
    {
        if (this != &in_right)
        {
            self(in_right).swap(*this);
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    public: self& assign(
        typename super::size_type const  in_count,
        typename super::value_type const in_char);

    //-------------------------------------------------------------------------
    public: typename super::reference at(
        typename super::size_type const in_index)
    {
        this->reset_hash();
        return this->super::at(in_index);
    }

    public: typename super::const_reference at(
        typename super::size_type const in_index)
    const
    {
        return this->super::at(in_index);
    }

    public: typename super::reference operator[](
        typename super::size_type const in_index)
    {
        this->reset_hash();
        return this->super::operator[](in_index);
    }

    public: typename super::const_reference operator[](
        typename super::size_type const in_index)
    const
    {
        return this->super::operator[](in_index);
    }

    //-------------------------------------------------------------------------
    public: typename super::reference front()
    {
        this->reset_hash();
        return this->super::front();
    }

    public: typename super::const_reference front() const
    {
        return this->super::front();
    }

    public: typename super::reference back()
    {
        this->reset_hash();
        return this->super::back();
    }

    public: typename super::const_reference back() const
    {
        return this->super::back();
    }

    //-------------------------------------------------------------------------
    public: typename super::iterator begin()
    {
        this->reset_hash();
        return this->super::begin();
    }

    public: typename super::const_iterator begin() const
    {
        return this->super::begin();
    }

    public: typename super::iterator end()
    {
        this->reset_hash();
        return this->super::end();
    }

    public: typename super::const_iterator end() const
    {
        return this->super::end();
    }

    //-------------------------------------------------------------------------
    public: typename super::reverse_iterator rbegin()
    {
        this->reset_hash();
        return this->super::rbegin();
    }

    public: typename super::const_reverse_iterator rbegin() const
    {
        return this->super::rbegin();
    }

    public: typename super::reverse_iterator rend()
    {
        this->reset_hash();
        return this->super::rend();
    }

    public: typename super::const_reverse_iterator rend() const
    {
        return this->super::rend();
    }

    //-------------------------------------------------------------------------
    public: void clear()
    {
        this->reset_hash();
        return this->super::clear();
    }

    //-------------------------------------------------------------------------
    public: self& insert(
        typename super::size_type const  in_index,
        typename super::size_type const  in_count,
        typename super::value_type const in_char)
    {
        this->reset_hash();
        this->super::insert(in_index, in_count, in_char);
        return *this;
    }

    //-------------------------------------------------------------------------
    public: self& erase(
        typename super::size_type const in_index = 0,
        typename super::size_type const in_count = super::npos)
    {
        this->reset_hash();
        this->super::erase(in_index, in_count);
        return *this;
    }

    //-------------------------------------------------------------------------
    public: void push_back(
        typename self::value_type const in_char)
    {
        this->reset_hash();
        this->super::push_back(in_char);
    }

    public: void pop_back()
    {
        this->reset_hash();
        this->super::pop_back();
    }

    //-------------------------------------------------------------------------
    public: self& append(
        typename super::size_type const  in_count,
        typename super::value_type const in_char)
    {
        this->reset_hash();
        this->super::append(in_count, in_char);
        return *this;
    }

    public: self& operator+=(
        self const& in_string)
    {
        this->reset_hash();
        this->super::operator+=(in_string);
        return *this;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列を比較。
        @param[in] in_right 右辺の文字列。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: int compare(
        self const& in_right)
    const
    {
        return self::compare_string(
            this->data(),
            this->size(),
            this->hash(),
            in_right.data(),
            in_right.size(),
            in_right.hash());
    }

    /** @brief 文字列を比較。
        @param[in] in_left_offset 左辺の文字列の開始位置。
        @param[in] in_left_count  左辺の文字列の文字数。
        @param[in] in_right       右辺の文字列。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: int compare(
        typename self::size_type const in_left_offset,
        typename self::size_type const in_left_count,
        self const&                    in_right)
    const
    {
        return self::compare_sub_string(
            *this,
            in_left_offset,
            in_left_count,
            in_right.data(),
            in_right.size(),
            in_right.hash());
    }

    /** @brief 文字列を比較。
        @param[in] in_left_offset 左辺の文字列の開始位置。
        @param[in] in_left_count  左辺の文字列の文字数。
        @param[in] in_right_begin 右辺の文字列の先頭位置。
        @param[in] in_right_size  右辺の文字列の長さ。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: int compare(
        typename self::size_type const     in_left_offset,
        typename self::size_type const     in_left_count,
        typename self::const_pointer const in_right_begin,
        typename self::size_type const     in_right_size)
    const
    {
        return self::compare_sub_string(
            *this,
            in_left_offset,
            in_left_count,
            in_right_begin,
            in_right_size,
            this->make_hash(in_right_begin, in_right_size));
    }

    /** @brief 文字列を比較。
        @tparam template_other_string_type 文字列型。
        @param[in] in_left_offset  左辺の文字列の開始位置。
        @param[in] in_left_count   左辺の文字列の文字数。
        @param[in] in_right        右辺の文字列。
        @param[in] in_right_offset 左辺の文字列の開始位置。
        @param[in] in_right_count  右辺の文字列の文字数。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: int compare(
        typename self::size_type const in_left_offset,
        typename self::size_type const in_left_count,
        super const&                   in_right,
        typename self::size_type const in_right_offset,
        typename self::size_type const in_right_count)
    const
    {
        return this->compare(
            in_left_offset,
            in_left_count,
            in_right.data() + in_right_offset,
            self::trim_size(in_right, in_right_offset, in_right_count));
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列を比較。
        @param[in] in_right 右辺の文字列。
        @return *this == 右辺の文字列
     */
    public: bool operator==(
        self const& in_right)
    const
    {
        return this->hash() == in_right.hash() &&
            static_cast<super const&>(*this) ==
                static_cast<super const&>(in_right);
    }

    /** @brief 文字列を比較。
        @param[in] in_right 右辺の文字列。
        @return *this != 右辺の文字列
     */
    public: bool operator!=(
        self const& in_right)
    const
    {
        return !this->operator==(in_right);
    }

    /** @brief 文字列を比較。
        @param[in] in_right 右辺の文字列。
        @return *this < 右辺の文字列
     */
    public: bool operator<(
        self const& in_right)
    const
    {
        return this->compare(in_right) < 0;
    }

    /** @brief 文字列を比較。
        @param[in] in_right 右辺の文字列。
        @return *this <= 右辺の文字列
     */
    public: bool operator<=(
        self const& in_right)
    const
    {
        return this->compare(in_right) <= 0;
    }

    /** @brief 文字列を比較。
        @param[in] in_right 右辺の文字列。
        @return *this > 右辺の文字列
     */
    public: bool operator>(
        self const& in_right)
    const
    {
        return in_right < *this;
    }

    /** @brief 文字列を比較。
        @param[in] in_right 右辺の文字列。
        @return *this >= 右辺の文字列
     */
    public: bool operator>=(
        self const& in_right)
    const
    {
        return in_right <= *this;
    }

    //-------------------------------------------------------------------------
    public: self& replace(
        typename super::size_type const in_position,
        typename super::size_type const in_count,
        self const&                     in_string)
    {
        this->reset_hash();
        this->super::replace(in_position, in_count, in_string);
        return *this;
    }

    //-------------------------------------------------------------------------
    public: void resize(
        typename super::size_type const in_count)
    {
        this->reset_hash();
        this->super::resize(in_count);
    }

    public: void resize(
        typename super::size_type const  in_count,
        typename super::value_type const in_char)
    {
        this->reset_hash();
        this->super::resize(in_count, in_char);
    }

    //-------------------------------------------------------------------------
    /** @brief 値を交換。
        @param[in,out] io_target 値を交換する対象。
     */
    public: void swap(
        self& io_target)
    {
        this->super::swap(io_target);
        std::swap(this->hash_, io_target.hash_);
        std::swap(this->hasher_, io_target.hasher_);
    };

    //-------------------------------------------------------------------------
    /** @brief 文字列を比較。
        @param[in] in_left        左辺の文字列。
        @param[in] in_left_offset 左辺の文字列の開始位置。
        @param[in] in_left_count  左辺の文字列の文字数。
        @param[in] in_right_begin 右辺の文字列の先頭位置。
        @param[in] in_right_size  右辺の文字列の大きさ。
        @param[in] in_right_hash  右辺の文字列のhash値。
        @retval 負 右辺の文字列のほうが大きい。
        @retval 正 左辺の文字列のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    private: template<typename template_left_type>
    static int compare_sub_string(
        template_left_type const&                    in_left,
        typename template_left_type::size_type const in_left_offset,
        typename template_left_type::size_type const in_left_count,
        typename super::const_pointer const          in_right_begin,
        typename super::size_type const              in_right_size,
        typename template_hash_type::value const&    in_right_hash)
    {
        typename template_left_type::size_type const local_left_size(
            self::trim_size(in_left, in_left_offset, in_left_count));
        typename template_left_type::const_pointer const local_left_begin(
            in_left.data() + in_left_offset);
        return self::compare_string(
            local_left_begin,
            local_left_size,
            this->make_hash(local_left_begin, local_left_size),
            in_right_begin,
            in_right_size,
            in_right_hash);
    }

    /** @brief 文字列を比較。
        @param[in] in_left_begin  左辺の文字列の先頭位置。
        @param[in] in_left_size   左辺文字列の大きさ。
        @param[in] in_left_hash   左辺の文字列のhash値。
        @param[in] in_right_begin 右辺の文字列の先頭位置。
        @param[in] in_right_size  右辺の文字列の大きさ。
        @param[in] in_right_hash  右辺の文字列のhash値。
        @retval 負 右辺の文字列のほうが大きい。
        @retval 正 左辺の文字列のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    private: static int compare_string(
        typename super::const_pointer const       in_left_begin,
        typename super::size_type const           in_left_size,
        typename template_hash_type::value const& in_left_hash,
        typename super::const_pointer const       in_right_begin,
        typename super::size_type const           in_right_size,
        typename template_hash_type::value const& in_right_hash)
    {
        // hash値を比較。
        int const local_compare_hash(
            self::compare_value(in_left_hash, in_right_hash));
        if (local_compare_hash != 0)
        {
            return local_compare_hash;
        }

        // 大きさを比較。
        int const local_compare_size(
            self::compare_value(in_left_size, in_right_size));
        if (local_compare_size != 0)
        {
            return local_compare_size;
        }

        // 文字列を比較。
        int const local_compare_string(
            super::traits_type::compare(
                in_left_begin, in_right_begin, in_right_size));
        if (0 != local_compare_string)
        {
            return local_compare_string;
        }
        return 0;
    }

    /** @brief 大きさを比較。
        @param[in] in_left  左辺値。
        @param[in] in_right 右辺値。
        @retval 負  右辺の値のほうが大きい。
        @retval 正  左辺の文字列のほうが大きい。
        @retval 0 左辺と右辺は同じ値。
     */
    private: template<typename template_value_type>
    static int compare_value(
        template_value_type const& in_left,
        template_value_type const& in_right)
    {
        if (in_left < in_right)
        {
            return -1;
        }
        if (in_right < in_left)
        {
            return 1;
        }
        return 0;
    }

    private: template<typename template_other_string_type>
    static typename template_other_string_type::size_type trim_size(
        template_other_string_type const&                    in_string,
        typename template_other_string_type::size_type const in_offset,
        typename template_other_string_type::size_type const in_count)
    {
        if (in_string.size() < in_offset)
        {
            PSYQ_ASSERT(false);
            return 0;
        }
        typename template_other_string_type::size_type const a_limit(
            in_string.size() - in_offset);
        return in_count < a_limit? in_count: a_limit;
    }

    //-------------------------------------------------------------------------
    /** @brief hash生成子を取得。
        @return hash生成子。
     */
    public: typename template_hash_type const& hash_function() const
    {
        return this->hasher_;
    }

    /** @brief 文字列のhash値を取得。
        @return 文字列のhash値。
     */
    public: typename template_hash_type::value const& hash() const
    {
        if (!this->empty() && this->hash_ == template_hash_type::EMPTY)
        {
            this->hash_ = this->make_hash(*this);
        }
        return this->hash_;
    }

    /** @brief 文字列のhash値を取得。
        @tparam template_other_string_type
            文字列の型。

            文字列の先頭位置を取得するため、以下に相当する関数が使えること。
            @code
            template_other_string_type::const_pointer
                template_other_string_type::data() const
            @endcode

            文字列の大きさを取得するため、以下に相当する関数が使えること。
            @code
            template_other_string_type::size_type
                template_other_string_type::size() const
            @endcode
        @param[in] in_string hash値を決定する文字列。
        @return 文字列のhash値。
     */
    public: template<typename template_other_string_type>
    typename template_hash_type::value make_hash(
        template_other_string_type const& in_string)
    const
    {
        return this->make_hash(in_string.data(), in_string.size());
    }

    /** @brief 配列のhash値を取得。
        @tparam template_value_type 配列の要素の型。
        @param[in] in_data hash値を決定する配列の先頭位置。
        @param[in] in_size 配列の大きさ。
        @return 配列のhash値。
     */
    public: template<typename template_value_type>
    typename template_hash_type::value make_hash(
        template_value_type const* const in_data,
        typename self::size_type const   in_size)
    const
    {
        return this->make_hash(in_data, in_data + in_size);
    }

    /** @brief 配列のhash値を取得。
        @param[in] in_begin hash値を決定する配列の先頭位置。
        @param[in] in_end   hash値を決定する配列の末尾位置。
        @return 配列のhash値。
     */
    public: typename template_hash_type::value make_hash(
        void const* const in_begin,
        void const* const in_end)
    const
    {
        typename template_hash_type::value const local_hash(
            this->hasher_.make(in_begin, in_end));
        if (local_hash == template_hash_type::EMPTY)
        {
            // 空配列と同じhash値だったので変更。
            return local_hash + std::distance(
                static_cast<char const*>(in_begin),
                static_cast<char const*>(in_end));
        }
        return local_hash;
    }

    private: void reset_hash()
    {
        this->hash_ = template_hash_type::EMPTY;
    }

    //-------------------------------------------------------------------------
    /// 文字列のhash値。
    private: mutable typename template_hash_type::value hash_;

    /// hash値を発行するinstance。
    private: mutable template_hash_type hasher_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 文字列の比較。
    @tparam template_string_type @copydoc psyq::hash_string::super
    @tparam template_hash_type   @copydoc psyq::hash_string::hasher
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 == 右辺
 */
template<
    typename template_string_type,
    typename template_hash_type>
bool operator==(
    template_string_type const& in_left,
    psyq::hash_string<template_string_type, template_hash_type> const&
        in_right)
{
    return in_right.operator==(in_left);
}

/** @brief 文字列の比較。
    @tparam template_string_type @copydoc psyq::hash_string::super
    @tparam template_hash_type   @copydoc psyq::hash_string::hasher
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 != 右辺
 */
template<
    typename template_string_type,
    typename template_hash_type>
bool operator!=(
    template_string_type const& in_left,
    psyq::hash_string<template_string_type, template_hash_type> const&
        in_right)
{
    return in_right.operator!=(in_left);
}

/** @brief 文字列の比較。
    @tparam template_string_type @copydoc psyq::hash_string::super
    @tparam template_hash_type   @copydoc psyq::hash_string::hasher
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 < 右辺
 */
template<
    typename template_string_type,
    typename template_hash_type>
bool operator<(
    template_string_type const& in_left,
    psyq::hash_string<template_string_type, template_hash_type> const&
        in_right)
{
    return in_right.operator>(in_left);
}

/** @brief 文字列の比較。
    @tparam template_string_type @copydoc psyq::hash_string::super
    @tparam template_hash_type   @copydoc psyq::hash_string::hasher
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 <= 右辺
 */
template<
    typename template_string_type,
    typename template_hash_type>
bool operator<=(
    template_string_type const& in_left,
    psyq::hash_string<template_string_type, template_hash_type> const&
        in_right)
{
    return in_right.operator>=(in_left);
}

/** @brief 文字列の比較。
    @tparam template_string_type @copydoc psyq::hash_string::super
    @tparam template_hash_type   @copydoc psyq::hash_string::hasher
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 > 右辺
 */
template<
    typename template_string_type,
    typename template_hash_type>
bool operator>(
    template_string_type const& in_left,
    psyq::hash_string<template_string_type, template_hash_type> const&
        in_right)
{
    return in_right.operator<(in_left);
}

/** @brief 文字列の比較。
    @tparam template_string_type @copydoc psyq::hash_string::super
    @tparam template_hash_type   @copydoc psyq::hash_string::hasher
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 >= 右辺
 */
template<
    typename template_string_type,
    typename template_hash_type>
bool operator>=(
    template_string_type const& in_left,
    psyq::hash_string<template_string_type, template_hash_type> const&
        in_right)
{
    return in_right.operator<=(in_left);
}

namespace std
{
    /** @brief 値を交換。
        @tparam template_string_type @copydoc psyq::hash_string::super
        @tparam template_hash_type   @copydoc psyq::hash_string::hasher
        @param[in,out] io_left  交換する値。
        @param[in,out] io_right 交換する値。
     */
    template<
        typename template_string_type,
        typename template_hash_type>
    void swap(
        psyq::hash_string<
            template_string_type, template_hash_type> const&
                i_left,
        psyq::hash_string<
            template_string_type, template_hash_type> const&
                i_right)
    {
        io_left.swap(io_right);
    }
} // namespace std

#endif // PSYQ_HASH_STRING_HPP_
