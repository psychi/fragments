/** @file
    @brief @copybrief psyq::member_comparison
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_MEMBER_COMPARISON_HPP_
#define PSYQ_MEMBER_COMPARISON_HPP_

/// @cond
namespace psyq
{
    template<typename, typename> struct member_comparison;
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 比較値から比較キーを抽出して比較する。

    ### 使い方の概略
    -# member_comparison::make_function
       で、比較値から比較キーを抽出して比較する関数オブジェクトを構築する。
    -# 比較値から比較キーを抽出して比較する関数オブジェクトを引数として
       member_comparison::find_iterator や member_comparison::find_pointer
       を呼び出し、ソート済コンテナから要素を二分探索する。

    @tparam template_value @copydoc member_comparison::value_type
    @tparam template_key   @copydoc member_comparison::key_type
 */
template<typename template_value, typename template_key>
struct psyq::member_comparison
{
    /// @brief thisが指す値の型。
    private: typedef member_comparison this_type;

    /// @brief this_type::key_type を抽出する比較値の型。
    public: typedef template_value value_type;

    /// @brief 比較するキーの型。
    public: typedef template_key key_type;

    //-------------------------------------------------------------------------
    /** @brief 比較値から比較キーを抽出して比較する関数オブジェクト。
        @tparam template_key_fetch_function   @copydoc function::key_fetch_function_
        @tparam template_key_compare_function @copydoc function::key_compare_function_
     */
    public: template<
        typename template_key_fetch_function,
        typename template_key_compare_function>
    class function
    {
        /// @brief thisが指す値の型。
        private: typedef function this_type;

        /// @copydoc member_comparison::make_function
        public: function(
            template_key_fetch_function in_key_fetch_function,
            template_key_compare_function in_key_compare_function):
        key_fetch_function_(std::move(in_key_fetch_function)),
        key_compare_function_(std::move(in_key_compare_function))
        {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
        /** @brief ムーブ構築子。
            @param[in,out] io_source ムーブ元となるインスタンス。
         */
        public: function(this_type&& io_source):
        key_fetch_function_(std::move(io_source.key_fetch_function_)),
        key_compare_function_(std::move(io_source.key_compare_function_))
        {}

        /** @brief ムーブ代入演算子。
            @param[in,out] io_source ムーブ元となるインスタンス。
            @return *this
         */
        public: this_type& operator=(this_type&& io_source)
        {
            this->key_fetch_function_= std::move(io_source.key_fetch_function_);
            this->key_compare_function_ = std::move(io_source.key_compare_function_);
            return *this;
        }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

        /** @brief 比較値から比較キーを抽出して比較する。
            @param[in] in_left  左辺値となる比較値か比較キー。
            @param[in] in_right 右辺値となる比較値か比較キー。
            @return 比較結果。
         */
        public: template<typename template_left, typename template_right>
        bool operator()(
            template_left const& in_left,
            template_right const& in_right)
        {
            return this->key_compare_function_(
                this->fetch_key(in_left), this->fetch_key(in_right));
        }

        /** @brief 比較値から比較キーを抽出する。
            @param[in] in_value 比較キーを抽出する比較値。
            @return in_value から抽出した比較キー。
         */
        public: typename member_comparison::key_type fetch_key(
            typename member_comparison::value_type const& in_value)
        {
            return this->key_fetch_function_(in_value);
        }

        /** @brief 比較キーをそのまま使う。
            @param[in] in_key そのまま使う比較キー。
            @return in_key
         */
        public: typename member_comparison::key_type const& fetch_key(
            typename member_comparison::key_type const& in_key)
        const PSYQ_NOEXCEPT
        {
            return in_key;
        }

        /** @brief 比較値から比較キーを抽出する関数オブジェクト。

            以下の形式で呼び出せること。
            @code
            // @param[in] in_value 比較キーを抽出する比較値。
            // @return 比較値から抽出した比較キー。
            member_comparison::key_type template_key_fetch_function(member_comparison::value_type const& in_value);
            @endcode
         */
        private: template_key_fetch_function key_fetch_function_;

        /** @brief 比較キーを比較する関数オブジェクト。

            以下の形式で呼び出せること。
            @code
            // @param[in] in_left  左辺値に使う比較キー。
            // @param[in] in_right 右辺値に使う比較キー。
            // @return 比較した結果。
            bool template_key_compare_function(member_comparison::key_type const& in_left, member_comparison::key_type const& in_right);
            @endcode
         */
        private: template_key_compare_function key_compare_function_;

    }; // class function

    //-------------------------------------------------------------------------
    /** @brief 比較値から比較キーを抽出して比較する関数オブジェクトを構築する。
        @param[in] in_key_fetch_function   @copydoc function::key_fetch_function_
        @param[in] in_key_compare_function @copydoc function::key_compare_function_
        @return 比較値から比較キーを抽出して比較する関数オブジェクト。
     */
    public: template<
        typename template_key_fetch_function,
        typename template_key_compare_function>
    static typename this_type::function<
        template_key_fetch_function, template_key_compare_function>
    make_function(
        template_key_fetch_function in_key_fetch_function,
        template_key_compare_function in_key_compare_function)
    {
        return typename this_type::function<
            template_key_fetch_function, template_key_compare_function>(
                std::move(in_key_fetch_function),
                std::move(in_key_compare_function));
    }

    //-------------------------------------------------------------------------
    /** @brief ソート済コンテナから要素を二分探索する。
        @param[in] in_begin   検索するソート済コンテナの先頭位置。
        @param[in] in_end     検索するソート済コンテナの末尾位置。
        @param[in] in_key     検索する要素の比較キー。
        @param[in] in_compare
            比較値から比較キーを抽出して比較する関数オブジェクト。
        @retval !=in_end in_key に対応する要素を指す反復子。
        @retval ==in_end in_key に対応する要素が見つからなかった。
     */
    public: template<typename template_iterator, typename template_compare>
    static template_iterator find_iterator(
        template_iterator const& in_begin,
        template_iterator const& in_end,
        typename this_type::key_type const& in_key,
        template_compare in_compare)
    {
        auto const local_lower_bound(
            std::lower_bound(in_begin, in_end, in_key, std::move(in_compare)));
        return local_lower_bound != in_end
            && in_key == in_compare.fetch_key(*local_lower_bound)?
                local_lower_bound: in_end;
    }

    /** @brief ソート済コンテナから要素を二分探索する。
        @param[in] in_container 検索するソート済コンテナ。
        @param[in] in_key       検索する要素の比較キー。
        @param[in] in_compare
            比較値から比較キーを抽出して比較する関数オブジェクト。
        @retval !=in_container.end() in_key に対応する要素を指す反復子。
        @retval ==in_container.end() in_key に対応する要素が見つからなかった。
     */
    public: template<typename template_container, typename template_compare>
    static auto find_iterator(
        template_container& in_container,
        typename this_type::key_type const& in_key,
        template_compare in_compare)
    ->decltype(std::begin(in_container))
    {
        return this_type::find_iterator(
            std::begin(in_container),
            std::end(in_container),
            in_key,
            std::move(in_compare));
    }

    /** @brief ソート済コンテナから要素を二分探索する。
        @param[in] in_begin   検索するソート済コンテナの先頭位置。
        @param[in] in_end     検索するソート済コンテナの末尾位置。
        @param[in] in_key     検索する要素の比較キー。
        @param[in] in_compare
            比較値から比較キーを抽出して比較する関数オブジェクト。
        @retval !=nullptr in_key に対応する要素を指すポインタ。
        @retval ==nullptr in_key に対応する要素が見つからなかった。
     */
    public: template<typename template_iterator, typename template_compare>
    static auto find_pointer(
        template_iterator const& in_begin,
        template_iterator const& in_end,
        typename this_type::key_type const& in_key,
        template_compare in_compare)
    ->decltype(&(*in_begin))
    {
        auto const local_lower_bound(
            std::lower_bound(in_begin, in_end, in_key, std::move(in_compare)));
        return local_lower_bound != in_end
            && in_key == in_compare.fetch_key(*local_lower_bound)?
                &(*local_lower_bound): nullptr;
    }

    /** @brief ソート済コンテナから要素を二分探索する。
        @param[in] in_container 検索するソート済コンテナ。
        @param[in] in_key       検索する要素の比較キー。
        @param[in] in_compare
            比較値から比較キーを抽出して比較する関数オブジェクト。
        @retval !=nullptr in_key に対応する要素を指すポインタ。
        @retval ==nullptr in_key に対応する要素が見つからなかった。
     */
    public: template<typename template_container, typename template_compare>
    static auto find_pointer(
        template_container& in_container,
        typename this_type::key_type const& in_key,
        template_compare in_compare)
    ->decltype(&(*std::begin(in_container)))
    {
        return this_type::find_pointer(
            std::begin(in_container),
            std::end(in_container),
            in_key,
            std::move(in_compare));
    }

}; // struct psyq::member_comparison

#endif // defined(PSYQ_MEMBER_COMPARISON_HPP_)
// vim: set expandtab:
