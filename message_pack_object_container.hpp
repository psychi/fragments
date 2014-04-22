/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::internal::message_pack_object_container
 */
#ifndef PSYQ_MESSAGE_PACK_OBJECT_CONTAINER_HPP_
#define PSYQ_MESSAGE_PACK_OBJECT_CONTAINER_HPP_

namespace psyq
{
    namespace internal
    {
        /// @cond
        template<typename> struct message_pack_object_container;
        template<typename> struct message_pack_object_map;
        /// @endcond
    }
    namespace message_pack
    {
        /// @cond
        struct object;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief MessagePackオブジェクトで使うコンテナ。
    @tparam template_value_compare @copydoc value_compare
    @sa psyq::message_pack::object
 */
template<typename template_value_compare>
struct psyq::internal::message_pack_object_container
{
    /// thisが指す値の型。
    private: typedef message_pack_object_container<template_value_compare>
        self;

    /** @brief 要素の比較関数オブジェクト。

        - 要素として、以下の型を使えること。
          @code
          template_value_compare::value_type
          @endcode
        - 要素の比較のために、以下に相当するクラス関数が使えること。
          @code
          static int template_value_compare::compare(value_type const& in_left, value_type const& in_right)
          @endcode
          - 戻り値が正なら、in_left > in_right。
          - 戻り値が0なら、in_left == in_right。
          - 戻り値が負なら、in_left < in_right。
        - 要素の等値比較のために、以下に相当するクラス関数が使えること。
          @code
          static bool template_value_compare::equal(value_type const& in_left, value_type const& in_right)
          @endcode
          - 戻り値がtrueなら、in_left == in_right。
          - 戻り値がtrueなら、in_left != in_right。
     */
    public: typedef template_value_compare value_compare;

    //-------------------------------------------------------------------------
    /// 要素の型。
    public: typedef typename self::value_compare::value_type value_type;

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
                    self::value_compare::equal(this->at(i), in_right.at(i)));
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
        if (this->data() == in_right.data())
        {
            return local_compare_size;
        }
        for (typename self::size_type i(0); i < local_size; ++i)
        {
            int const local_compare_element(
                self::value_compare::compare(this->at(i), in_right.at(i)));
            if (local_compare_element != 0)
            {
                return local_compare_element;
            }
        }
        return local_compare_size;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name コンテナの編集
    //@{
    public: void reset(
        typename self::pointer const in_data,
        typename self::size_type const in_size)
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
/** @brief MessagePackオブジェクトで使う連想配列。
    @tparam template_value_compare @copydoc super::value_compare
    @sa psyq::message_pack::object
 */
template<typename template_value_compare>
struct psyq::internal::message_pack_object_map:
    public psyq::internal::message_pack_object_container<
        template_value_compare>
{
    /// thisが指す値の型。
    private: typedef message_pack_object_map<template_value_compare> self;

    /// thisの上位型。
    public: typedef psyq::internal
        ::message_pack_object_container<template_value_compare>
            super;

    /// 連想配列のキー。
    public: typedef psyq::message_pack::object key_type;
    //public: typedef typename super::value_type::first_type key_type;
    /// 連想配列のマップ値。
    public: typedef psyq::message_pack::object mapped_type;
    //public: typedef typename super::value_type::second_type mapped_type;

    //-------------------------------------------------------------------------
    //private: super::const_reference at(super::size_type const) const;
    private: typename super::const_reference operator[](
        typename super::size_type const)
    const;

    public: typename super::const_iterator find(
        typename self::key_type const& in_key)
    const
    {
        /// @todo 未実装。
        return this->cend();
    }

    public: typename super::const_iterator lower_bound(
        typename self::key_type const& in_key)
    const
    {
        return std::lower_bound(
            this->cbegin(),
            this->cend(),
            typename self::value_type(in_key, self::mapped_type()),
            [](
                typename self::value_type const& in_left,
                typename self::value_type const& in_right)
            {
                return self::value_compare::compare(in_left, in_right) < 0;
            });
    }

    public: typename super::const_iterator upper_bound(
        typename self::key_type const& in_key)
    const
    {
        return std::upper_bound(
            this->begin(),
            this->end(),
            typename self::value_type(in_key, self::mapped_type()),
            [](
                typename self::value_type const& in_left,
                typename self::value_type const& in_right)
            {
                return self::value_compare::compare(in_left, in_right) < 0;
            });
    }

    public: void push_back(typename super::value_type const& in_value)
    {
        this->super::push_back(in_value);
    }

    public: void sort()
    {
        std::sort(
            const_cast<typename super::iterator>(this->begin()),
            const_cast<typename super::iterator>(this->end()),
            [](
                typename self::value_type const& in_left,
                typename self::value_type const& in_right)
            {
                return self::value_compare::compare(in_left, in_right) < 0;
            });
    }
};

#endif // PSYQ_MESSAGE_PACK_OBJECT_CONTAINER_HPP_
