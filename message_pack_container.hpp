/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::internal::message_pack_container
 */
#ifndef PSYQ_MESSAGE_PACK_CONTAINER_HPP_
#define PSYQ_MESSAGE_PACK_CONTAINER_HPP_

namespace psyq
{
    namespace internal
    {
        /// @cond
        template<typename> struct message_pack_container;
        struct message_pack_extended;
        template<typename> struct message_pack_map;
        /// @endcond

        /** @brief コンテナ要素を比較する。
            @param[in] in_left  左辺のコンテナ要素。
            @param[in] in_right 右辺のコンテナ要素。
            @retval 正 左辺のほうが大きい。
            @retval 0  等値。
            @retval 負 右辺のほうが小さい。
         */
        template<typename template_value_type>
        int message_pack_object_compare(
            template_value_type const& in_left,
            template_value_type const& in_right)
        {
            return in_left < in_right? -1: (in_right < in_left? 1: 0);
        }

        /** @brief コンテナ要素が等値か判定する。
            @param[in] in_left  左辺のコンテナ要素。
            @param[in] in_right 右辺のコンテナ要素。
            @retval true  左辺と右辺は等値。
            @retval false 左辺と右辺は非等値。
         */
        template<typename template_value_type>
        bool message_pack_object_equal(
            template_value_type const& in_left,
            template_value_type const& in_right)
        {
            return in_left == in_right;
        }
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief MessagePackオブジェクトで使うコンテナの基底型。
    @tparam template_value_type @copydoc value_type
    @sa psyq::message_pack::object::array
    @sa psyq::message_pack::object::map
 */
template<typename template_value_type>
struct psyq::internal::message_pack_container
{
    /// thisが指す値の型。
    private: typedef message_pack_container<template_value_type>
        self;

    //-------------------------------------------------------------------------
    /// コンテナ要素の型。
    public: typedef template_value_type value_type;

    /// コンテナ要素数の型。
    public: typedef std::uint32_t size_type;

    /// コンテナ反復子の差を表す型。
    public: typedef std::ptrdiff_t difference_type;

    /// コンテナ要素へのpointer。
    public: typedef typename self::value_type const* const_pointer;

    /// コンテナ要素へのpointer。
    public: typedef typename self::value_type* pointer;

    /// コンテナ要素への参照。
    public: typedef typename self::value_type const& const_reference;

    /// コンテナ要素への参照。
    public: typedef typename self::value_type& reference;

    /// コンテナ要素を指す反復子。
    public: typedef typename self::const_pointer const_iterator;

    /// コンテナ要素を指す反復子。
    public: typedef typename self::pointer iterator;

    /// コンテナ要素を指す逆反復子。
    public: typedef std::reverse_iterator<const_iterator>
        const_reverse_iterator;

    /// コンテナ要素を指す逆反復子。
    public: typedef std::reverse_iterator<iterator>
        reverse_iterator;

    //-------------------------------------------------------------------------
#ifndef _MSC_VER
    /** @brief 空のコンテナを構築する。
        @note 2014.04.25
        VisualStudio2013以前では、C++11の仕様であるunionの制限解除に未対応。
        そのため psyq::internal::message_pack_value でビルドエラーが発生する。
        対応時期が現時点では不明なので、VisualStudioでは無効にしておく。
     */
    public: PSYQ_CONSTEXPR message_pack_container() PSYQ_NOEXCEPT:
       data_(nullptr),
       size_(0)
    {}
#endif // !defined(_MSC_VER)

    //-------------------------------------------------------------------------
    /// @name コンテナの要素を参照
    //@{
    /** @brief コンテナの先頭要素へのpointerを取得する。
        @return コンテナの先頭要素へのpointer。
     */
    public: typename self::const_pointer data() const PSYQ_NOEXCEPT
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
    public: PSYQ_CONSTEXPR typename self::size_type max_size()
    const PSYQ_NOEXCEPT
    {
        return (std::numeric_limits<typename self::size_type>::max)();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name コンテナの比較
    //@{
    /** @brief thisを左辺として、右辺のコンテナと等値か判定する。
        @param[in] in_right 右辺のコンテナ。
        @retval true  等値だった。
        @retval false 非等値だった 。
     */
    public: bool operator==(self const& in_right) const PSYQ_NOEXCEPT
    {
        if (this->size() != in_right.size())
        {
            return false;
        }
        if (this->data() != in_right.data())
        {
            for (typename self::size_type i(0); i < this->size(); ++i)
            {
                bool const local_equal(
                    psyq::internal::message_pack_object_equal(
                        this->at(i), in_right.at(i)));
                if (!local_equal)
                {
                    return false;
                }
            }
        }
        return true;
    }

    /** @brief thisを左辺として、右辺のコンテナと非等値か判定する。
        @param[in] in_right 右辺のコンテナ。
        @retval true  非等値だった 。
        @retval false 等値だった。
     */
    public: bool operator!=(self const& in_right) const PSYQ_NOEXCEPT
    {
        return !(*this == in_right);
    }

    /** @brief thisを左辺として、右辺のコンテナと比較する。
        @param[in] in_right 右辺のコンテナ。
        @return 左辺 < 右辺
     */
    public: bool operator<(self const& in_right) const PSYQ_NOEXCEPT
    {
        return this->compare(in_right) < 0;
    }

    /** @brief thisを左辺として、右辺のコンテナと比較する。
        @param[in] in_right 右辺のコンテナ。
        @return 左辺 <= 右辺
     */
    public: bool operator<=(self const& in_right) const PSYQ_NOEXCEPT
    {
        return this->compare(in_right) <= 0;
    }

    /** @brief thisを左辺として、右辺のコンテナと比較する。
        @param[in] in_right 右辺のコンテナ。
        @return 左辺 > 右辺
     */
    public: bool operator>(self const& in_right) const PSYQ_NOEXCEPT
    {
        return 0 < this->compare(in_right);
    }

    /** @brief thisを左辺として、右辺のコンテナと比較する。
        @param[in] in_right 右辺のコンテナ。
        @return 左辺 >= 右辺
     */
    public: bool operator>=(self const& in_right) const PSYQ_NOEXCEPT
    {
        return 0 <= this->compare(in_right);
    }

    /** @brief コンテナを比較する。
        @param[in] in_right 右辺となる配列。
        @retval 正 左辺のほうが大きい。
        @retval 0  等値。
        @retval 負 左辺のほうが小さい。
     */
    public: int compare(self const& in_right) const PSYQ_NOEXCEPT
    {
        typename self::size_type local_size;
        int local_compare_size;
        if (this->size() < in_right.size())
        {
            local_size = this->size();
            local_compare_size = -1;
        }
        else
        {
            local_size = in_right.size();
            local_compare_size = this->size() < in_right.size()? 1: 0;
        }
        if (this->data() != in_right.data())
        {
            for (typename self::size_type i(0); i < local_size; ++i)
            {
                int const local_compare_element(
                    psyq::internal::message_pack_object_compare(
                        this->at(i), in_right.at(i)));
                if (local_compare_element != 0)
                {
                    return local_compare_element;
                }
            }
        }
        return local_compare_size;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name コンテナの編集
    //@{
    /** @brief コンテナを初期化する。
        @param[in] in_data コンテナの先頭位置。
        @param[in] in_size コンテナの要素数。
     */
    public: void reset(
        typename self::pointer const in_data,
        typename self::size_type const in_size)
    PSYQ_NOEXCEPT
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
/** @brief MessagePackオブジェクトで使う拡張バイナリ。

    RAWバイト列の先頭1バイトに、拡張型の識別値として符号つき整数が格納され、
    その後ろにバイナリが格納されている。

    @sa psyq::message_pack::object::extended
 */
struct psyq::internal::message_pack_extended:
    public psyq::internal::message_pack_container<std::uint8_t const>
{
    /// thisが指す値の型。
    private: typedef message_pack_extended self;

    /// thisの上位型。
    public: typedef psyq::internal::message_pack_container<std::uint8_t const>
        super;

    //-------------------------------------------------------------------------
    /// @name コンテナの要素を参照
    //@{
    /// @copydoc super::data()
    public: super::const_pointer data() const PSYQ_NOEXCEPT
    {
        return 1 < this->super::size()?
            this->super::data() + 1: this->super::data();
    }

    /// @copydoc super::at()
    public: super::const_reference at(super::size_type const in_index) const
    {
        if (this->size() <= in_index)
        {
            PSYQ_ASSERT(false);
            //throw std::out_of_range; // 例外は使いたくない。
        }
        return *(this->data() + in_index);
    }

    /// @copydoc super::operator[]()
    public: super::const_reference operator[](super::size_type const in_index)
    const
    {
        PSYQ_ASSERT(in_index < this->size());
        return *(this->data() + in_index);
    }

    /// @copydoc super::front()
    public: super::const_reference front() const
    {
        return (*this)[0];
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name iteratorの取得
    //@{
    /// @copydoc super::begin()
    public: super::const_iterator begin() const PSYQ_NOEXCEPT
    {
        return this->data();
    }

    /// @copydoc super::cbegin()
    public: super::const_iterator cbegin() const PSYQ_NOEXCEPT
    {
        return this->begin();
    }

    /// @copydoc super::rend()
    public: super::const_reverse_iterator rend() const PSYQ_NOEXCEPT
    {
        return super::const_reverse_iterator(this->begin());
    }

    /// @copydoc super::crend()
    public: super::const_reverse_iterator crend() const PSYQ_NOEXCEPT
    {
        return this->rend();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @copydoc super::size()
    public: super::size_type size() const PSYQ_NOEXCEPT
    {
        return 1 < this->super::size()? this->super::size() - 1: 0;
    }

    /** @brief 拡張バイナリの型の識別値を取得する。
        @retval !=nullptr 拡張バイナリの型の識別値が格納されているポインタ。
        @retval ==nullptr 拡張バイナリが空。
     */
    public: std::int8_t type() const PSYQ_NOEXCEPT
    {
        return 0 < this->super::size()?
            *reinterpret_cast<std::int8_t const*>(this->super::data()): 0;
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief MessagePackオブジェクトで使う連想配列。
    @tparam template_object キーとマップ値に使うMessagePackオブジェクト。
    @sa psyq::message_pack::object::map
 */
template<typename template_object>
struct psyq::internal::message_pack_map:
    public psyq::internal::message_pack_container<
        std::pair<template_object, template_object>>
{
    /// thisが指す値の型。
    private: typedef message_pack_map<template_object> self;

    /// thisの上位型。
    public: typedef psyq::internal::message_pack_container<
        std::pair<template_object, template_object>>
            super;

    /// 連想配列のキー。
    public: typedef template_object key_type;
    /// 連想配列のマップ値。
    public: typedef template_object mapped_type;

    //-------------------------------------------------------------------------
    /** @brief 連想配列の要素を並び替える。
     */
    public: void sort()
    {
        std::sort(
            const_cast<typename super::iterator>(this->begin()),
            const_cast<typename super::iterator>(this->end()),
            [](
                typename super::value_type const& in_left,
                typename super::value_type const& in_right)
            {
                return psyq::internal
                    ::message_pack_object_compare(in_left, in_right) < 0;
            });
    }

    //-------------------------------------------------------------------------
    /** @brief std::multimap::count() 相当の関数。
        @warning 事前に self::sort() されている必要がある。
     */
    public: typename super::size_type count(
        typename self::key_type const& in_key)
    const
    {
        auto const local_range(this->equal_range(in_key));
        return local_range.second - local_range.first;
    }

    /** @brief std::multimap::find() 相当の関数。
        @warning 事前に self::sort() されている必要がある。
     */
    public: typename super::const_iterator find(
        typename self::key_type const& in_key)
    const
    {
        auto const local_iterator(this->lower_bound(in_key));
        return local_iterator != this->end()
            && local_iterator->first == in_key?
                local_iterator: this->end();
    }

    /** @brief std::multimap::equal_range() 相当の関数。
        @warning 事前に self::sort() されている必要がある。
     */
    public: std::pair<
        typename super::const_iterator, typename super::const_iterator>
            equal_range(typename self::key_type const& in_key) const
    {
        return std::equal_range(
            this->begin(),
            this->end(),
            typename self::value_type(in_key, self::mapped_type()),
            [](
                typename super::value_type const& in_left,
                typename super::value_type const& in_right)
            {
                return psyq::internal
                    ::message_pack_object_compare(in_left, in_right) < 0;
            });
    }

    /** @brief std::multimap::lower_bound() 相当の関数。
        @warning 事前に self::sort() されている必要がある。
     */
    public: typename super::const_iterator lower_bound(
        typename self::key_type const& in_key)
    const
    {
        return std::lower_bound(
            this->begin(),
            this->end(),
            typename self::value_type(in_key, self::mapped_type()),
            [](
                typename super::value_type const& in_left,
                typename super::value_type const& in_right)
            {
                return psyq::internal
                    ::message_pack_object_compare(in_left, in_right) < 0;
            });
    }

    /** @brief std::multimap::upper_bound() 相当の関数。
        @warning 事前に self::sort() されている必要がある。
     */
    public: typename super::const_iterator upper_bound(
        typename self::key_type const& in_key)
    const
    {
        return std::upper_bound(
            this->begin(),
            this->end(),
            typename self::value_type(in_key, self::mapped_type()),
            [](
                typename super::value_type const& in_left,
                typename super::value_type const& in_right)
            {
                return psyq::internal
                    ::message_pack_object_compare(in_left, in_right) < 0;
            });
    }

    //-------------------------------------------------------------------------
    private: typename super::const_reference at(
        typename super::size_type const)
    const;

    private: typename super::const_reference operator[](
        typename super::size_type const)
    const;
};

#endif // PSYQ_MESSAGE_PACK_CONTAINER_HPP_
