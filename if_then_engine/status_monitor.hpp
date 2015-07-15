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

    //-------------------------------------------------------------------------
    /// @brief 評価の更新を要求する条件式の識別値のコンテナ。
    public: typename this_type::expression_key_container expression_keys_;
    /// @brief 監視する状態値の識別値。
    public: template_status_key key_;
    /// @brief 前回の notify_transition で、状態値が存在したか。
    private: bool last_existence_;

}; // class psyq::if_then_engine::_private::status_monitor

#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_MONITOR_HPP_)
// vim: set expandtab:
