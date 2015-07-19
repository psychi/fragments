/** @file
    @brief @copybrief psyq::member_comparison
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_MEMBER_COMPARISON_HPP_
#define PSYQ_MEMBER_COMPARISON_HPP_

/// @cond
namespace psyq
{
    template<typename, typename> class member_comparison;
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief オブジェクトの任意のメンバ変数を比較する。
    @tparam template_object @copydoc member_comparison::object
    @tparam template_key    @copydoc member_comparison::key
 */
template<typename template_object, typename template_key>
class psyq::member_comparison
{
    private: typedef member_comparison this_type;

    /// @brief メンバ変数を持つオブジェクトの型。
    public: typedef template_object object;
    /// @brief 比較するメンバ変数の型。
    public: typedef template_key key;

    //-------------------------------------------------------------------------
    /** @brief オブジェクトのメンバ変数を比較する関数オブジェクト。
        @tparam template_key_fetch_function   @copydoc function::key_fetch_function_
        @tparam template_key_compare_function @copydoc function::key_compare_function_
     */
    public: template<
        typename template_key_fetch_function,
        typename template_key_compare_function>
    class function
    {
        /** @brief オブジェクトのメンバ変数を比較する関数オブジェクトを構築する。
            @param[in] in_key_fetch_function   @copydoc function::key_fetch_function_
            @param[in] in_key_compare_function @copydoc function::key_compare_function_
         */
        public: function(
            template_key_fetch_function in_key_fetch_function,
            template_key_compare_function in_key_compare_function):
        key_fetch_function_(std::move(in_key_fetch_function)),
        key_compare_function_(std::move(in_key_compare_function))
        {}

        /** @brief オブジェクトを比較する。
            @param[in] in_left  左辺となるオブジェクトまたはキー。
            @param[in] in_right 右辺となるオブジェクトまたはキー。
            @return 比較結果。
         */
        public: template<typename template_left, typename template_right>
        bool operator()(
            template_left const& in_left,
            template_right const& in_right)
        const
        {
            return this->key_compare_function_(
                this->fetch_key(in_left), this->fetch_key(in_right));
        }

        public: typename member_comparison::key const& fetch_key(
            typename member_comparison::key const& in_key)
        const
        {
            return in_key;
        }

        public: typename member_comparison::key fetch_key(
            typename member_comparison::object const& in_value)
        const
        {
            return this->key_fetch_function_(in_value);
        }

        /// @brief 比較するメンバ変数をオブジェクトから取得する関数オブジェクト。
        private: template_key_fetch_function key_fetch_function_;
        /// @brief メンバ変数を比較する関数オブジェクト。
        private: template_key_compare_function key_compare_function_;

    }; // class function

    //-------------------------------------------------------------------------
    /** @brief オブジェクトのメンバ変数を比較する関数オブジェクトを構築する。
        @param[in] in_key_fetch_function   @copydoc function::key_fetch_function_
        @param[in] in_key_compare_function @copydoc function::key_compare_function_
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
    /** @brief コンテナから値を検索する。
        @param[in] in_begin   検索するコンテナの先頭位置。
        @param[in] in_end     検索するコンテナの末尾位置。
        @param[in] in_key     検索する値の識別値。
        @param[in] in_compare 値を比較する関数オブジェクト。
        @retval !=in_container.end() in_key に対応する値を指す反復子。
        @retval ==in_container.end() in_key に対応する値が見つからなかった。
     */
    public: template<typename template_iterator, typename template_compare>
    static typename template_iterator find_iterator(
        template_iterator const& in_begin,
        template_iterator const& in_end,
        typename this_type::key const& in_key,
        template_compare const& in_compare)
    {
        auto const local_lower_bound(
            std::lower_bound(in_begin, in_end, in_key, in_compare));
        return local_lower_bound != in_end
            && in_key == in_compare.fetch_key(*local_lower_bound)?
                local_lower_bound: in_end;
    }

    /** @brief コンテナから値を検索する。
        @param[in] in_container 検索するコンテナ。
        @param[in] in_key       検索する値の識別値。
        @param[in] in_compare   値を比較する関数オブジェクト。
        @retval !=in_container.end() in_key に対応する値を指す反復子。
        @retval ==in_container.end() in_key に対応する値が見つからなかった。
     */
    public: template<typename template_container, typename template_compare>
    static auto find_iterator(
        template_container& in_container,
        typename this_type::key const& in_key,
        template_compare const& in_compare)
    ->decltype(std::begin(in_container))
    {
        return this_type::find_iterator(
            std::begin(in_container), std::end(in_container), in_key, in_compare);
    }

    /** @brief コンテナから値を検索する。
        @param[in] in_begin   検索するコンテナの先頭位置。
        @param[in] in_end     検索するコンテナの末尾位置。
        @param[in] in_key     検索する値の識別値。
        @param[in] in_compare 値を比較する関数オブジェクト。
        @retval !=nullptr in_key に対応する値を指すポインタ。
        @retval ==nullptr in_key に対応する値が見つからなかった。
     */
    public: template<typename template_iterator, typename template_compare>
    static auto find_pointer(
        template_iterator const& in_begin,
        template_iterator const& in_end,
        typename this_type::key const& in_key,
        template_compare const& in_compare)
    ->decltype(&(*in_begin))
    {
        auto const local_lower_bound(
            std::lower_bound(in_begin, in_end, in_key, in_compare));
        return local_lower_bound != in_end
            && in_key == in_compare.fetch_key(*local_lower_bound)?
                &(*local_lower_bound): nullptr;
    }

    /** @brief コンテナから値を検索する。
        @param[in] in_container 検索するコンテナ。
        @param[in] in_key       検索する値の識別値。
        @param[in] in_compare   値を比較する関数オブジェクト。
        @retval !=nullptr in_key に対応する値を指すポインタ。
        @retval ==nullptr in_key に対応する値が見つからなかった。
     */
    public: template<typename template_container, typename template_compare>
    static auto find_pointer(
        template_container& in_container,
        typename this_type::key const& in_key,
        template_compare const& in_compare)
    ->decltype(&(*std::begin(in_container)))
    {
        return this_type::find_pointer(
            std::begin(in_container), std::end(in_container), in_key, in_compare);
    }

}; // struct psyq::::member_comparison

#endif // defined(PSYQ_MEMBER_COMPARISON_HPP_)
// vim: set expandtab:
