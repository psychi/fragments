#ifndef PSYQ_STRING_HASH_HPP_
#define PSYQ_STRING_HASH_HPP_

namespace psyq
{
    /// @cond
    template<typename, typename> class string_hash;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 文字列と文字列のhash値を一緒に保持する。

    比較演算子や compare() で比較を行うときは、
    最初にhash値の比較をした後で、文字列を比較する。

    @tparam template_string_type @copydoc string_hash::string
    @tparam template_hash_type   @copydoc string_hash::hasher
 */
template<
    typename template_string_type,
    typename template_hash_type>
class psyq::string_hash
{
    /// thisが指す値の型。
    typedef psyq::string_hash<template_string_type, template_hash_type> self;

    /** @brief 文字列の型。

        文字列の長さを得るため、以下に相当する関数が使えること。
        @code
        self::string::size_type self::string::size() const
        @endcode

        文字列を比較するため、以下に相当する関数が使えること。
        @code
        int self::string::compare(self::string const&) const
        @endcode

        文字列を交換するため、以下に相当する関数が使えること。
        @code
        std::swap(self::string&, self::string&)
        @endcode
     */
    public: typedef template_string_type string;

    /** hash生成関数objectの型。

        hash値を生成するため、以下に相当する関数が使えること。
        @code
        self::hasher::result_type self::hasher::operator()(self::string const&);
        @endcode
     */
    public: typedef template_hash_type hasher;

    //-------------------------------------------------------------------------
    /** @brief default-constructor
        @param[in] in_string 文字列の初期値。
        @param[in] in_hasher hash生成関数objectの初期値。
     */
    public: string_hash(
        typename self::string const& in_string = self::string(),
        typename self::hasher const& in_hasher = self::hasher()):
    string_(in_string),
    hasher_(in_hasher)
    {
        this->rehash();
    }

    //-------------------------------------------------------------------------
    /** @brief 値を交換。
        @param[in,out] io_target 交換する値。
     */
    public: void swap(
        self& io_target)
    {
        std::swap(this->string_, io_target.string_);
        std::swap(this->hash_, io_target.hash_);
        std::swap(this->hasher_, io_target.hasher_);
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列を取得。
        @return 保持している文字列。
     */
    public: typename self::string const& get_string() const
    {
        return this->string_;
    }

    /** @brief 文字列をcopy。

        保持している文字列にcopyする。
        @param[in] in_source copy元の文字列。
     */
    public: void copy_string(
        typename self::string const& in_source)
    {
        this->string_ = in_source;
        this->rehash();
    }

    /** @brief 文字列を交換。

        保持している文字列と交換する。
        @param[in,out] io_target 交換する文字列。
     */
    public: void swap_string(
        typename self::string& io_target)
    {
        this->string_.swap(io_target);
        this->rehash();
    }

    /** @brief 文字列のhash値を取得。
        @return 保持している文字列のhash値。
     */
    public: typename typename self::hasher::result_type const& get_hash() const
    {
        return this->hash_;
    }

    /** @brief hash生成関数objectを取得。
        @return 保持しているhash生成関数object。
     */
    public: typename self::hasher const& get_hasher() const
    {
        return this->hasher_;
    }

    //-------------------------------------------------------------------------
    /** @brief 値を比較。

        *thisを左辺値として、値を比較する。

        @param[in] in_right 右辺値。
        @return *this == 右辺
     */
    public: bool operator==(
        self const& in_right)
    const
    {
        return
            this->get_hash() == in_right.get_hash() &&
            this->get_string() == in_right.get_string();
    }

    /** @brief 値を比較。

        *thisを左辺値として、値を比較する。

        @param[in] in_right 右辺値。
        @return *this != 右辺
     */
    public: bool operator!=(
        self const& in_right)
    const
    {
        return !this->operator==(in_right);
    }

    /** @brief 値を比較。

        *thisを左辺値として、値を比較する。

        @param[in] in_right 右辺値。
        @return *this < 右辺
     */
    public: bool operator<(
        self const& in_right)
    const
    {
        return this->compare(in_right) < 0;
    }

    /** @brief 値を比較。

        *thisを左辺値として、値を比較する。

        @param[in] in_right 右辺値。
        @return *this <= 右辺
     */
    public: bool operator<=(
        self const& in_right)
    const
    {
        return this->compare(in_right) <= 0;
    }

    /** @brief 値を比較。

        *thisを左辺値として、値を比較する。

        @param[in] in_right 右辺値。
        @return *this > 右辺
     */
    public: bool operator>(
        self const& in_right)
    const
    {
        return 0 < this->compare(in_right);
    }

    /** @brief 値を比較。

        *thisを左辺値として、値を比較する。

        @param[in] in_right 右辺値。
        @return *this >= 右辺
     */
    public: bool operator>=(
        self const& in_right)
    const
    {
        return 0 <= this->compare(in_right);
    }

    //-------------------------------------------------------------------------
    /** @brief 値を比較。

        *thisを左辺値として、値を比較する。

        @param[in] in_right 右辺値。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: int compare(
        self const& in_right)
    const
    {
        // hash値を比較。
        if (this->get_hash() < in_right.get_hash())
        {
            return -1;
        }
        if (in_right.get_hash() < this->get_hash())
        {
            return 1;
        }

        // 文字列を比較。
        return this->get_string().compare(in_right.get_string());
    }

    //-------------------------------------------------------------------------
    private: void rehash()
    {
        this->hash_ = this->hasher_.make(
            this->get_string().data(),
            this->get_string().data() + this->string_.size());
    }

    //-------------------------------------------------------------------------
    private: typename self::string              string_; ///< 文字列。
    private: typename self::hasher::result_type hash_;   ///< 文字列のhash値。
    private: typename self::hasher              hasher_; ///< hash生成関数object。
};

namespace std
{
    /** @brief 値を交換。
        @tparam template_string_type @copydoc psyq::strin_hash::string
        @tparam template_hash_type   @copydoc psyq::string_hash::hasher
        @param[in,out] io_left  交換する値。
        @param[in,out] io_right 交換する値。
     */
    template<
        typename template_string_type,
        typename template_hash_type>
    void swap(
        psyq::string_hash<
            template_string_type, template_hash_type> const&
                i_left,
        psyq::string_hash<
            template_string_type, template_hash_type> const&
                i_right)
    {
        io_left.swap(io_right);
    }
} // namespace std

#endif // PSYQ_STRING_HASH_HPP_
