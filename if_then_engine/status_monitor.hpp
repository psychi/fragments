/** @file
    @brief @copybrief psyq::if_then_engine::_private::status_monitor
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_MONITOR_HPP_
#define PSYQ_IF_THEN_ENGINE_STATUS_MONITOR_HPP_

#include <vector>
#include "./key_less.hpp"

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            template<typename, typename, typename> class status_monitor;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態監視器。

    条件式の要素条件が参照する状態値を監視し、
    状態値が変化した際に、条件式の評価を更新するために使う。
 */
template<
    typename template_status_key,
    typename template_expression_key,
    typename template_allocator>
class psyq::if_then_engine::_private::status_monitor
{
    /// @brief thisが指す値の型。
    private: typedef status_monitor this_type;

    /// @brief 状態監視器のコンテナ。
    public: typedef std::vector<this_type, template_allocator> container;

    /// @brief 状態監視器を状態値の識別値の昇順で並び替えるのに使う、比較関数オブジェクト。
    public: typedef
        psyq::if_then_engine::_private::key_less<
            psyq::if_then_engine::_private::object_key_getter<
                this_type, template_status_key>>
        key_less;

    /// @brief 条件式の識別値のコンテナの型。
    public: typedef
        std::vector<template_expression_key, template_allocator>
        expression_key_container;

    //-------------------------------------------------------------------------
    /** @brief 状態監視器を構築する。
        @param[in] in_key       状態値の識別値。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: status_monitor(
        template_status_key in_key,
        template_allocator const& in_allocator):
    expression_keys_(in_allocator),
    key_(std::move(in_key)),
    last_existence_(false)
    {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: status_monitor(this_type&& io_source):
    expression_keys_(std::move(io_source.expression_keys_)),
    key_(std::move(io_source.key_)),
    last_existence_(std::move(io_source.last_existence_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->expression_keys_ = std::move(io_source.expression_keys_);
        this->key_ = std::move(io_source.key_);
        this->last_existence_ = std::move(io_source.last_existence_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /** @brief 状態変化を検知し、条件式監視器へ通知する。
        @param[in,out] io_expression_monitors
            状態変化を通知する条件式監視器のコンテナ。
        @param[in] in_reservoir 状態変化を把握している状態貯蔵器。
        @retval true  状態監視器が空になった。
        @retval false 状態監視器が空ではない。
     */
    public: template<typename template_container, typename template_reservoir>
    bool notify_transition(
        template_container& io_expression_monitors,
        template_reservoir const& in_reservoir)
    {
        // 状態変化を検知する。
        /** @todo
            reservoir::_get_transition は二分探索を行うが、
            監視しているすべての状態値に対し二分探索を毎回行うのは、
            計算量として問題にならないか気になる。計算量が問題なら、
            reservoir::status_container をハッシュ辞書にすべきか、要検討。
         */
        auto const local_transition(in_reservoir._get_transition(this->key_));
        auto const local_existence(0 <= local_transition);
        if (0 < local_transition || local_existence != this->last_existence_)
        {
            template_container::value_type::notify_status_transition(
                io_expression_monitors, this->expression_keys_, local_existence);
        }
        this->last_existence_ = local_existence;
        return this->expression_keys_.empty();
    }

    /** @brief 状態変化を通知する条件式を登録する。
        @param[in] in_register_key 状態変化を通知する条件式の識別値。
        @param[in] in_reserve_expressions
            状態監視器が持つ条件式識別値コンテナの予約容量。
        @retval true 条件式を登録した。
        @retval false
            同じ識別値の条件式がすでに登録されていたので、何もしなかった。
     */
    public: bool insert_expression_key(
        template_expression_key const& in_register_key,
        std::size_t const in_reserve_expressions)
    {
        return this_type::insert_expression_key(
            this->expression_keys_, in_register_key, in_reserve_expressions);
    }

    /** @brief 状態変化を通知する条件式を登録する。
        @param[in,out] io_expression_keys
            状態変化を通知する条件式の識別値を挿入するコンテナ。
        @param[in] in_register_key 状態変化を通知する条件式の識別値。
        @param[in] in_reserve_expressions
            状態監視器が持つ条件式識別値コンテナの予約容量。
        @retval true 条件式を登録した。
        @retval false
            同じ識別値の条件式がすでに登録されていたので、何もしなかった。
     */
    private: static bool insert_expression_key(
        typename this_type::expression_key_container& io_expression_keys,
        template_expression_key const& in_register_key,
        std::size_t const in_reserve_expressions)
    {
        io_expression_keys.reserve(in_reserve_expressions);
        auto const local_lower_bound(
            std::lower_bound(
                io_expression_keys.begin(),
                io_expression_keys.end(),
                in_register_key));
        if (local_lower_bound != io_expression_keys.end()
            && *local_lower_bound == in_register_key)
        {
            return false;
        }
        io_expression_keys.insert(local_lower_bound, in_register_key);
        return true;
    }

    /** @brief 条件式識別値コンテナを整理する。
        @param[in] in_expression_monitors 参照する条件式監視器のコンテナ。
        @retval true  条件式識別値コンテナが空になった。
        @retval false 条件式識別値コンテナは空になってない。
     */
    public: template<typename template_container>
    bool shrink_expression_keys(
        template_container const& in_expression_monitors)
    {
        return this_type::shrink_expression_keys(
            this->expression_keys_, in_expression_monitors);
    }

    /** @brief 条件式識別値コンテナを整理する。
        @param[in,out] io_expression_keys 整理する条件式識別値のコンテナ。
        @param[in] in_expression_monitors 参照する条件式監視器のコンテナ。
        @retval true  条件式識別値コンテナが空になった。
        @retval false 条件式識別値コンテナは空になってない。
     */
    private: template<typename template_container>
    static bool shrink_expression_keys(
         typename this_type::expression_key_container& io_expression_keys,
         template_container const& in_expression_monitors)
    {
        for (auto i(io_expression_keys.begin()); i != io_expression_keys.end();)
        {
            auto const local_expression_monitor(
                template_container::value_type::key_less::find_const_pointer(
                    in_expression_monitors, *i));
            if (local_expression_monitor == nullptr)
            {
                i = io_expression_keys.erase(i);
            }
            else
            {
                ++i;
            }
        }
        io_expression_keys.shrink_to_fit();
        return io_expression_keys.empty();
    }

    //-------------------------------------------------------------------------
    /// @brief 評価の更新を要求する条件式の識別値のコンテナ。
    private: typename this_type::expression_key_container expression_keys_;
    /// @brief 監視する状態値の識別値。
    public: template_status_key key_;
    /// @brief 前回の notify_transition で、状態値が存在したか。
    private: bool last_existence_;

}; // class psyq::if_then_engine::_private::status_monitor

#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_MONITOR_HPP_)
// vim: set expandtab:
