#ifndef PSYQ_HASH_STRING_HPP_
#define PSYQ_HASH_STRING_HPP_

/// @cond
namespace psyq
{
    template<typename, typename> class const_hash_string;
    template<typename, typename> class hash_string;
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief hash値を使って比較を行う定数文字列。
    @tparam template_string_type @copydoc const_hash_string::base_type
    @tparam template_hash_type   @copydoc const_hash_string::maker_type
 */
template<
    typename template_string_type,
    typename template_hash_type>
class psyq::const_hash_string:
    public template_string_type
{
    /// thisが指す値の型。
    public: typedef psyq::const_hash_string<
        template_string_type, template_hash_type >
            self;

    /// self の上位型。
    public: typedef template_string_type super;

    /// 基底文字列型。 std::basic_string とinterfaceが互換。
    public: typedef super base_type;

    /// hash値を発行する型。
    public: typedef template_hash_type maker_type;

    /** @brief default-constructor
        @param[in] in_hash_maker hash生成instanceの初期値。
     */
    public: explicit const_hash_string(
        template_hash_type const& in_hash_maker = template_hash_type())
    :
        super(),
        hash_(template_hash_type::EMPTY),
        hash_maker_(in_hash_maker)
    {
        // pass
    }

    public: explicit const_hash_string(
        template_string_type const& in_source,
        template_hash_type const&   in_hash_maker = template_hash_type())
    :
        super(in_source),
        hash_(template_hash_type::EMPTY),
        hash_maker_(in_hash_maker)
    {
        // pass
    }

    /** copy演算子。
        @param[in] in_right copy元となる値。
     */
    public: self& operator=(
        super const& in_right)
    {
        self(in_right).swap(*this);
        return *this;
    }

    /** @brief 文字列を比較。
        @param[in] in_right 右辺の文字列。
        @return *this == 右辺の文字列
     */
    public: bool operator==(
        self const& in_right)
    const
    {
        return in_left.hash() == in_left.hash() &&
            static_cast<super const&>(*this) == in_right;
    }

    /** @brief 文字列を比較。
        @param[in] in_right 右辺の文字列。
        @return *this == 右辺の文字列
     */
    public: bool operator==(
        super const& in_right)
    const
    {
        return static_cast<super const&>(*this) == in_right;
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
        @return *this != 右辺の文字列
     */
    public: bool operator!=(
        super const& in_right)
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
        return self::less(
            *this, this->hash(), in_right, in_right.hash());
    }

    /** @brief 文字列を比較。
        @param[in] in_right 右辺の文字列。
        @return *this < 右辺の文字列
     */
    public: bool operator<(
        super const& in_right)
    const
    {
        template_hash_type local_hash_maker
        return self::less(
            *this, this->hash(), in_right, this->make_hash(in_right));
    }

    /** @brief 文字列を比較。
        @param[in] in_right 右辺の文字列。
        @return *this <= 右辺の文字列
     */
    public: bool operator<=(
        self const& in_right)
    const
    {
        return self::less_equal(
            *this, this->hash(), in_right, in_right.hash());
    }

    /** @brief 文字列を比較。
        @param[in] in_right 右辺の文字列。
        @return *this <= 右辺の文字列
     */
    public: bool operator<=(
        super const& in_right)
    const
    {
        return self::less_equal(
            *this, this->hash(), in_right, this->make_hash(in_right));
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
        @return *this > 右辺の文字列
     */
    public: bool operator>(
        super const& in_right)
    const
    {
        return self::less(
            in_right, this->make_hash(in_right), *this, this->hash());
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

    /** @brief 文字列を比較。
        @param[in] in_right 右辺の文字列。
        @return *this >= 右辺の文字列
     */
    public: bool operator>=(
        super const& in_right)
    const
    {
        return self::less_equal(
            in_right, this->make_hash(in_right), *this, this->hash());
    }

    /** @brief 値を交換。
        @param[in,out] io_target 値を交換する対象。
     */
    public: void swap(
        self& io_target)
    {
        this->super::swap(io_target);
        std::swap(this->hash_, io_target.hash_);
        std::swap(this->hash_maker_, io_target.hash_maker_);
    };

    public: typename template_hash_type const& hash_maker() const
    {
        return this->hash_maker_;
    }

    /** @brief 文字列のhash値を取得。
        @return 文字列のhash値。
     */
    public: typename template_hash_type::value const& hash() const
    {
        if (this->hash_ == template_hash_type::EMPTY)
        {
            this->hash_ = this->make_hash(*this);
        }
        return this->hash_;
    }

    protected: typename template_hash_type::value make_hash(
        template_string_type const& in_string) const
    {
        typename super::const_pointer const local_data(in_string.data());
        return this->hash_maker_.make(
            local_data, local_data + in_string.size());
    }

    protected: void reset_hash() const
    {
        this->hash_ = template_hash_type::EMPTY;
    }

    /** @brief 文字列を比較。
        @param[in] in_left       左辺の文字列。
        @param[in] in_left_hash  左辺の文字列のhash値。
        @param[in] in_right      右辺の文字列。
        @param[in] in_right_hash 右辺の文字列のhash値。
        @return 左辺の文字列 < 右辺の文字列
     */
    private: static bool less(
        super const&                              in_left,
        typename template_hash_type::value const& in_left_hash,
        super const&                              in_right,
        typename template_hash_type::value const& in_right_hash)
    {
        const int local_compare_hash(
            self::compare(in_left_hash, in_right_hash));
        if (local_compare_hash != 0)
        {
            return local_compare_hash < 0;
        }
        const int local_compare_size(self::compare_size(in_left, in_right));
        if (local_compare_size != 0)
        {
            return local_compare_size < 0;
        }
        return in_left < in_right;
    }

    /** @brief 文字列を比較。
        @param[in] in_left       左辺の文字列。
        @param[in] in_left_hash  左辺の文字列のhash値。
        @param[in] in_right      右辺の文字列。
        @param[in] in_right_hash 右辺の文字列のhash値。
        @return 左辺の文字列 <= 右辺の文字列
     */
    private: static bool less_equal(
        super const&                              in_left,
        typename template_hash_type::value const& in_left_hash,
        super const&                              in_right,
        typename template_hash_type::value const& in_right_hash)
    {
        const int local_compare_hash(
            self::compare(in_left_hash, in_right_hash));
        if (local_compare_hash != 0)
        {
            return local_compare_hash < 0;
        }
        const int local_compare_size(self::compare_size(in_left in_right));
        if (local_compare_size != 0)
        {
            return local_compare_size < 0;
        }
        return in_left <= in_right;
    }

    /** @brief 文字列の大きさを比較。
        @param[in] in_left  左辺の文字列。
        @param[in] in_right 右辺の文字列。
        @retval <0  右辺の文字列のほうが文字数が多い。
        @retval ==0 左辺と右辺は同じ文字数。
        @retval >0  左辺の文字列のほうが文字数が多い。
     */
    private: static int compare_size(
        super const& in_left,
        super const& in_right)
    {
        return self::compare(in_left.size(), in_right.size());
    }

    /** @brief 大きさを比較。
        @param[in] in_left  左辺値。
        @param[in] in_right 右辺値。
        @retval <0  右辺の値のほうが大きい。
        @retval ==0 左辺と右辺は同じ値。
        @retval >0  左辺の文字列のほうが大きい。
     */
    private: template< typename template_value_type >
    static int compare(
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
        else
        {
            return 0;
        }
    }

    /// 文字列のhash値。
    private: mutable typename template_hash_type::value hash_;

    /// hash値を発行するinstance。
    private: mutable template_hash_type hash_maker_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
#if 0
template<
    typename template_string_type,
    typename template_hash_type>
class psyq::hash_string:
    public psyq::const_hash_string<template_string_type, template_hash_type>
{
    /// thisが指す値の型。
    public: typedef psyq::hash_string<
        template_string_type, template_hash_type>
            self;

    /// selfの上位型。
    public: typedef psyq::const_hash_string<
        template_string_type, template_hash_type>
            super;

    public: typename super::reference operator[](
        typename super::size_type const in_index)
    {
        this->dirty_ = true;
        return this->super::operator[](in_index);
    }

    public: typename super::const_reference operator[](
        typename super::size_type const in_index)
    const
    {
        return this->super::operator[](in_index);
    }

    public: typename super::reference at(
        typename super::size_type const in_index)
    {
        this->dirty_ = true;
        return this->super::at(in_index);
    }

    public: typename super::const_reference at(
        typename super::size_type const in_index)
    const
    {
        return this->super::at(in_index);
    }

    public: typename super::reference front()
    {
        this->dirty_ = true;
        return this->super::front();
    }

    public: typename super::const_reference front() const
    {
        return this->super::front();
    }

    public: typename super::reference back()
    {
        this->dirty_ = true;
        return this->super::back();
    }

    public: typename super::const_reference back() const
    {
        return this->super::back();
    }

    public: typename super::pointer data()
    {
        this->dirty_ = true;
        return this->super::data();
    }

    public: typename super::const_pointer data() const
    {
        return this->super::data();
    }

    public: typename super::pointer c_str()
    {
        this->dirty_ = true;
        return this->super::c_str();
    }

    public: typename super::const_pointer c_str() const
    {
        return this->super::c_str();
    }

    public: typename super::iterator begin()
    {
        this->dirty_ = true;
        return this->super::begin();
    }

    public: typename super::const_iterator begin() const
    {
        return this->super::begin();
    }

    public: typename super::iterator end()
    {
        this->dirty_ = true;
        return this->super::end();
    }

    public: typename super::const_iterator end() const
    {
        return this->super::end();
    }

    public: typename super::reverse_iterator rbegin()
    {
        this->dirty_ = true;
        return this->super::rbegin();
    }

    public: typename super::const_reverse_iterator rbegin() const
    {
        return this->super::rbegin();
    }

    public: typename super::reverse_iterator rend()
    {
        this->dirty_ = true;
        return this->super::rend();
    }

    public: typename super::const_reverse_iterator rend() const
    {
        return this->super::rend();
    }

    public: void clear()
    {
        this->dirty_ = true;
        this->super::clear();
    }

    /** @brief 文字列のhash値を取得。
        @return 文字列のhash値。
     */
    public: typename template_hash_type::value const& hash() const
    {
        if (this->dirty_)
        {
            this->dirty_ = false;
            this->make_hash();
        }
        return this->hash_value_;
    }

    /// hash発行要求。
    private: mutable bool dirty_;
};
#endif //0

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
    typename template_string_type,
    typename template_hash_type>
bool operator==(
    template_string_type const& in_left,
    psyq::const_hash_string<template_string_type, template_hash_type> const&
        in_right)
{
    return in_right.operator==(in_left);
}

template<
    typename template_string_type,
    typename template_hash_type>
bool operator!=(
    template_string_type const& in_left,
    psyq::const_hash_string<template_string_type, template_hash_type> const&
        in_right)
{
    return in_right.operator!=(in_left);
}

namespace std
{
    /** @brief 値を交換。
        @param[in,out] io_left  交換する値の左辺値。
        @param[in,out] io_right 交換する値の右辺値。
     */
    template<
        typename template_string_type,
        typename template_hash_type>
    void swap(
        psyq::const_hash_string<
            template_string_type, template_hash_type> const&
                i_left,
        psyq::const_hash_string<
            template_string_type, template_hash_type> const&
                i_right)
    {
        io_left.swap(io_right);
    }
} // namespace std

#endif // PSYQ_HASH_STRING_HPP_
