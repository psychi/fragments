/** @file
    @brief @copybrief psyq::scenario_engine::_private::key_less
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_KEY_LESS_HPP_
#define PSYQ_SCENARIO_ENGINE_KEY_LESS_HPP_

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        namespace _private
        {
            template<typename, typename> struct key_less;
        } // namespace _private
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 識別値を比較する関数オブジェクト。
template<typename template_value, typename template_key>
struct psyq::scenario_engine::_private::key_less
{
    bool operator()(
        template_value const& in_left,
        template_value const& in_right)
    const PSYQ_NOEXCEPT
    {
        return in_left.key < in_right.key;
    }

    bool operator()(
        template_key const& in_left,
        template_value const& in_right)
    const PSYQ_NOEXCEPT
    {
        return in_left < in_right.key;
    }

    bool operator()(
        template_value const& in_left,
        template_key const& in_right)
    const PSYQ_NOEXCEPT
    {
        return in_left.key < in_right;
    }

    /** @brief コンテナから値を検索する。
        @param[in] in_container 検索するコンテナ。
        @param[in] in_key       検索する値の識別値。
        @retval !=in_container.end() in_key に対応する値を指す反復子。
        @retval ==in_container.end() in_key に対応する値が見つからなかった。
     */
    template<typename template_container>
    static typename template_container::const_iterator find_const_iterator(
        template_container const& in_container,
        template_key const& in_key)
    PSYQ_NOEXCEPT
    {
        return find_iterator(
            const_cast<template_container&>(in_container), in_key);
    }

    /// @copydoc find_const_iterator
    template<typename template_container>
    static typename template_container::iterator find_iterator(
        template_container& in_container,
        template_key const& in_key)
    PSYQ_NOEXCEPT
    {
        auto const local_end(in_container.end());
        auto const local_lower_bound(
            std::lower_bound(
                in_container.begin(), local_end, in_key, key_less()));
        return local_lower_bound != local_end
            && local_lower_bound->key == in_key?
                local_lower_bound: local_end;
    }

    /** @brief コンテナから値を検索する。
        @param[in] in_container 検索するコンテナ。
        @param[in] in_key       検索する値の識別値。
        @retval !=nullptr in_key に対応する値を指すポインタ。
        @retval ==nullptr in_key に対応する値が見つからなかった。
     */
    template<typename template_container>
    static typename template_container::value_type const* find_const_pointer(
        template_container const& in_container,
        template_key const& in_key)
    PSYQ_NOEXCEPT
    {
        auto const local_end(in_container.end());
        auto const local_lower_bound(
            std::lower_bound(
                in_container.begin(), local_end, in_key, key_less()));
        return local_lower_bound != local_end
            && local_lower_bound->key == in_key?
                &(*local_lower_bound): nullptr;
    }

    /// @copydoc find_const_pointer
    template<typename template_container>
    static typename template_container::value_type* find_pointer(
        template_container& in_container,
        template_key const& in_key)
    PSYQ_NOEXCEPT
    {
        return const_cast<typename template_container::value_type*>(
            find_const_pointer(in_container, in_key));
    }

}; // struct psyq::scenario_engine::_private::key_less

#endif // defined(PSYQ_SCENARIO_ENGINE_KEY_LESS_HPP_)
// vim: set expandtab:
