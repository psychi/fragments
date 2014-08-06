/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ANY_MESSAGE_ZONE_HPP_
#define PSYQ_ANY_MESSAGE_ZONE_HPP_

//#include "psyq/any/message/transmitter.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief ANYメッセージを送受信する範囲の単位。

    初期化の手順
    -# psyq::any::message::zone インスタンスを用意する。
    -# psyq::any::message::zone::equip_transmitter() で、スレッド毎に固有の
       psyq::any::message::transmitter インスタンスを保持しておく。
    -# メッセージ受信関数を設定した
       psyq::any::message::receiver インスタンスを用意し、
       psyq::any::message::transmitter::register_receiver() で登録する。

    メッセージ送受信の手順
    -# それぞれのメッセージ伝送器に対応するスレッドで
       psyq::any::message::transmitter::post_message() を実行し、
       メッセージを送信する。
    -# psyq::any::message::zone::flush() をメインスレッドで定期的に実行し、
       スレッド毎に固有のメッセージ伝送器にメッセージを集配する。
    -# それぞれのメッセージ伝送器に対応するスレッドで
       psyq::any::message::transmitter::flush() を定期的に実行すると、
       メッセージに対応するメッセージ受信器にメッセージが配信され、
       psyq::any::message::receiver::get_functor()
       で取得できるメッセージ受信関数が呼び出される。

    @tparam template_base_suite @copydoc psyq::any::message::packet::suite
    @tparam template_allocator  @copydoc psyq::any::message::zone::allocator_type
 */
template<typename template_base_suite, typename template_allocator>
class psyq::any::message::zone
{
    private: typedef zone this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// 使用するメモリ割当子。
    public: typedef template_allocator allocator_type;
    /// メッセージ伝送器。
    public: typedef psyq::any::message
        ::transmitter<template_base_suite, template_allocator>
            transmitter;
    /// メッセージ受信器。
    public: typedef typename this_type::transmitter::receiver receiver;
    /// メッセージ伝送器のコンテナ。
    private: typedef std
        ::vector<typename this_type::transmitter::weak_ptr, template_allocator>
            weak_transmitter_container;

    //-------------------------------------------------------------------------
    /// @name メッセージゾーンの構築
    //@{
    /** @brief メッセージゾーンを構築する。
        @param[in] in_allocator このメッセージゾーンが使うメモリ割当子の初期値。
     */
    public: explicit zone(
        typename this_type::allocator_type const& in_allocator =
            template_allocator())
    :
        transmitters_(in_allocator),
        import_packets_(in_allocator),
        export_packets_(in_allocator)
    {}
    //@}
    /// コピー構築子は使用禁止。
    private: zone(this_type const&);
    /// コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    //-------------------------------------------------------------------------
    /// @name メッセージ伝送器の取得
    //@{
    /** @brief このスレッドに対応するメッセージ伝送器を用意する。

        - 用意したメッセージ伝送器は this_type::transmitter::weak_ptr
          で監視しているだけで、このメッセージゾーンでは所有権を持たない。

        @return このスレッドに対応するメッセージ伝送器。
     */
    public: typename this_type::transmitter::shared_ptr equip_transmitter()
    {
        return this->equip_transmitter(std::this_thread::get_id());
    }
    /** @brief スレッドに対応するメッセージ伝送器を用意する。

        - 用意したメッセージ伝送器は this_type::transmitter::weak_ptr
          で監視しているだけで、このメッセージゾーンでは所有権を持たない。

        @param[in] in_thread_id スレッド識別子。
        @return スレッドに対応するメッセージ伝送器。
     */
    public: typename this_type::transmitter::shared_ptr equip_transmitter(
        std::thread::id const& in_thread_id)
    {
        std::lock_guard<psyq::spinlock> local_lock(this->lock_);
        auto const local_transmitter(
            this_type::find_transmitter(this->transmitters_, in_thread_id));
        if (local_transmitter.get() != nullptr)
        {
            return local_transmitter;
        }
        typename this_type::receiver::tag::key const local_message_address(0);
        return this_type::make_transmitter(
            this->transmitters_, in_thread_id, local_message_address);
    }
    //@}
    private: static typename this_type::transmitter::shared_ptr find_transmitter(
        typename this_type::weak_transmitter_container const& in_transmitters,
        std::thread::id const& in_thread_id)
    {
        for (auto const& local_observer: in_transmitters)
        {
            auto const local_holder(local_observer.lock());
            auto const local_transmitter(local_holder.get());
            if (local_transmitter != nullptr
                && local_transmitter->get_thread_id() == in_thread_id)
            {
                return local_holder;
            }
        }
        return typename this_type::transmitter::shared_ptr();
    }

    private: static typename this_type::transmitter::shared_ptr make_transmitter(
        typename this_type::weak_transmitter_container& io_transmitters,
        std::thread::id const& in_thread_id,
        typename this_type::receiver::tag::key const in_mesasge_address)
    {
        typename this_type::transmitter::allocator_type::template
            rebind<typename this_type::transmitter>::other
                local_allocator(io_transmitters.get_allocator());
        void* const local_storage(local_allocator.allocate(1));
        if (local_storage == nullptr)
        {
            PSYQ_ASSERT(false);
            return typename this_type::transmitter::shared_ptr();
        }
        typename this_type::transmitter::shared_ptr const local_transmitter(
            new(local_storage) typename this_type::transmitter(
                in_thread_id, in_mesasge_address, local_allocator),
            [local_allocator](typename this_type::transmitter* const io_transmitter)
            {
                auto local_deallocator(local_allocator);
                local_deallocator.destroy(io_transmitter);
                local_deallocator.deallocate(io_transmitter, 1);
            },
            local_allocator);
        if (local_transmitter.get() != nullptr)
        {
            io_transmitters.push_back(local_transmitter);
        }
        else
        {
            PSYQ_ASSERT(false);
        }
        return local_transmitter;
    }

    //-------------------------------------------------------------------------
    /// @name メッセージの送受信
    //@{
    /** @brief メッセージ伝送器のメッセージを集配する。

        psyq::any::message::transmitter::flush() とこの関数を定期的に実行し、
        メッセージを循環させること。
     */
    public: void flush()
    {
        std::lock_guard<psyq::spinlock> local_lock(this->lock_);
        this_type::trade_packet_container(
            this->transmitters_, this->export_packets_, this->import_packets_);
        this_type::swap_packet_container(
            this->export_packets_, this->import_packets_);
    }
    //@}
    private: static void trade_packet_container(
        typename this_type::weak_transmitter_container& io_transmitters,
        typename this_type::transmitter::shared_packet_container& io_export_packets,
        typename this_type::transmitter::shared_packet_container const& in_import_packets)
    {
        auto local_last_iterator(io_transmitters.begin());
        for (auto i(local_last_iterator); i != io_transmitters.end(); ++i)
        {
            auto const local_holder(i->lock());
            auto const local_transmitter(local_holder.get());
            if (local_transmitter != nullptr)
            {
                local_transmitter->trade_packet_container(
                    io_export_packets, in_import_packets);
                i->swap(*local_last_iterator);
                ++local_last_iterator;
            }
        }
        io_transmitters.erase(local_last_iterator, io_transmitters.end());
    }

    private: static void swap_packet_container(
        typename this_type::transmitter::shared_packet_container& io_export_packets,
        typename this_type::transmitter::shared_packet_container& io_import_packets)
    {
        io_export_packets.swap(io_import_packets);
        this_type::transmitter::clear_packet_container(
            io_export_packets, io_import_packets.size());
    }

    //-------------------------------------------------------------------------
    /// @name メッセージゾーンのプロパティ
    //@{
    public: PSYQ_CONSTEXPR typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->transmitters_.get_allocator();
    }
    //@}
    //-------------------------------------------------------------------------
    /// スレッド毎のメッセージ伝送器のコンテナ。
    private: typename this_type::weak_transmitter_container transmitters_;
    /// メッセージ伝送器から輸入したメッセージパケットのコンテナ。
    private: typename this_type::transmitter::shared_packet_container import_packets_;
    /// メッセージ伝送器へ輸出するメッセージパケットのコンテナ。
    private: typename this_type::transmitter::shared_packet_container export_packets_;
    private: psyq::spinlock lock_;

}; // class psyq::any::message::zone

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        inline void any_message()
        {
            psyq::any::rtti::make<psyq::test::floating_wrapper>();

            typedef psyq::any::message::zone<> message_zone;
            message_zone local_zone;
            auto const local_transmitter_holder(local_zone.equip_transmitter());
            auto& local_transmitter(*local_transmitter_holder);
            enum: message_zone::receiver::call::key
            {
                METHOD_PARAMETER_VOID = 1,
                METHOD_PARAMETER_DOUBLE,
            };
            message_zone::receiver::shared_ptr const local_method_a(
                new message_zone::receiver(
                    [](message_zone::receiver::packet const& in_packet)
                    {
                        PSYQ_ASSERT(
                            in_packet.get_parameter_rtti()
                            == psyq::any::rtti::find<void>());
                    },
                    local_transmitter.get_message_address()));
            double const local_double(0.5);
            message_zone::receiver::shared_ptr const local_method_b(
                new message_zone::receiver(
                    [local_double]
                    (message_zone::receiver::packet const& in_packet)
                    {
                        auto const local_parameter(
                            in_packet.get_parameter<psyq::test::floating_wrapper>());
                        if (local_parameter != nullptr)
                        {
                            PSYQ_ASSERT(local_parameter->value == local_double);
                        }
                        else
                        {
                            PSYQ_ASSERT(false);
                        }
                    },
                    local_transmitter.get_message_address()));

            local_transmitter.register_receiver(
                METHOD_PARAMETER_VOID, local_method_a);
            local_transmitter.register_receiver(
                METHOD_PARAMETER_DOUBLE, local_method_b);
            local_transmitter.send_local_message(
                local_transmitter.make_receiver_tag(
                    local_transmitter.get_message_address()),
                message_zone::receiver::call(METHOD_PARAMETER_VOID));
            local_transmitter.send_local_message(
                local_transmitter.make_receiver_tag(
                    local_transmitter.get_message_address()),
                message_zone::receiver::call(METHOD_PARAMETER_DOUBLE),
                floating_wrapper(0.5));
            local_transmitter.post_message(
                local_transmitter.make_receiver_tag(
                    local_transmitter.get_message_address()),
                message_zone::receiver::call(METHOD_PARAMETER_VOID));
            local_transmitter.post_zonal_message(
                local_transmitter.make_receiver_tag(
                    local_transmitter.get_message_address()),
                message_zone::receiver::call(METHOD_PARAMETER_DOUBLE),
                floating_wrapper(0.5));
            local_zone.flush();
            local_transmitter.flush();
            local_zone.flush();
            local_transmitter.flush();
            local_transmitter.unregister_receiver(nullptr);
            local_transmitter.unregister_receiver(0, nullptr);
        }
    }
}

#endif // !defined(PSYQ_ANY_MESSAGE_ZONE_HPP_)
