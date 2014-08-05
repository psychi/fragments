/** @file
    @brief RPCメッセージの送受信。
    @copydetails psyq::any::message::router
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ANY_MESSAGE_ROUTER_HPP_
#define PSYQ_ANY_MESSAGE_ROUTER_HPP_

//#include "psyq/any/message/hub.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージルータ。

    初期化の手順
    -# メッセージ送受信アドレスを指定し、 psyq::any::message::router
       インスタンスを構築する。
    -# psyq::any::message::router::equip_hub() で、スレッド毎に固有の
       psyq::any::message::hub インスタンスを用意する。
    -# psyq::any::message::hub::register_receiver() で、
       メッセージ受信関数を設定した
       psyq::any::message::receiver インスタンスを登録する。

    メッセージ送受信の手順
    -# それぞれのメッセージ中継器に対応するスレッドで
       psyq::any::message::hub::send_message() を実行し、メッセージを送信する。
    -# メインスレッドで psyq::any::message::router::flush()
       を実行し、メッセージ中継器にメッセージの集配／分配をする。
    -# それぞれのメッセージ中継器に対応するスレッドで
       psyq::any::message::hub::flush() を実行すると、
       メッセージ受信器にメッセージが分配され、メッセージに対応する
       psyq::any::message::receiver::get_functor()
       で取得できるメッセージ受信関数が呼び出される。

    @tparam template_base_suite @copydoc psyq::any::message::packet::suite
    @tparam template_allocator  @copydoc psyq::any::message::router::allocator_type
 */
template<typename template_base_suite, typename template_allocator>
class psyq::any::message::router
{
    private: typedef router this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// パケットに用いるメモリ割当子。
    public: typedef template_allocator allocator_type;
    /// メッセージ中継器。
    public: typedef psyq::any::message
        ::hub<template_base_suite, template_allocator>
            hub;
    /// メッセージ受信器。
    public: typedef typename this_type::hub::receiver receiver;
    /// メッセージ中継器のコンテナ。
    private: typedef std
        ::vector<typename this_type::hub::weak_ptr, template_allocator>
            weak_hub_container;

    //-------------------------------------------------------------------------
    /// @name メッセージルータの構築
    //@{
    /** @brief メッセージルータを構築する。
        @param[in] in_address   構築するインスタンスのメッセージ送受信アドレス。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: router(
        typename this_type::receiver::tag::key const in_address,
        typename this_type::allocator_type const& in_allocator)
    :
        hubs_(in_allocator),
        import_packets_(in_allocator),
        export_packets_(in_allocator),
        address_((
            PSYQ_ASSERT(in_address != this_type::receiver::tag::EMPTY_KEY),
            in_address))
    {}
    //@}
    /// コピー構築子は使用禁止。
    private: router(this_type const&);
    /// コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    //-------------------------------------------------------------------------
    /// @name メッセージ中継器の取得
    //@{
    /** @brief このスレッドに対応するメッセージ中継器を用意する。
        @return このスレッドに対応するメッセージ中継器。
     */
    public: typename this_type::hub::shared_ptr equip_hub()
    {
        return this->equip_hub(std::this_thread::get_id());
    }
    /** @brief スレッドに対応するメッセージ中継器を用意する。
        @param[in] in_thread_id スレッド識別子。
        @return スレッドに対応するメッセージ中継器。
     */
    public: typename this_type::hub::shared_ptr equip_hub(
        std::thread::id const& in_thread_id)
    {
        std::lock_guard<psyq::spinlock> local_lock(this->lock_);
        auto const local_hub(
            this_type::find_hub(this->hubs_, in_thread_id));
        if (local_hub.get() != nullptr)
        {
            return local_hub;
        }
        return this_type::make_hub(this->hubs_, in_thread_id);
    }
    //@}
    private: static typename this_type::hub::shared_ptr find_hub(
        typename this_type::weak_hub_container const& in_hubs,
        std::thread::id const& in_thread_id)
    {
        for (auto const& local_observer: in_hubs)
        {
            auto const local_holder(local_observer.lock());
            auto const local_hub(local_holder.get());
            if (local_hub != nullptr
                && local_hub->get_thread_id() == in_thread_id)
            {
                return local_holder;
            }
        }
        return typename this_type::hub::shared_ptr();
    }

    private: static typename this_type::hub::shared_ptr make_hub(
        typename this_type::weak_hub_container& io_hubs,
        std::thread::id const& in_thread_id)
    {
        typename this_type::hub::allocator_type::template
            rebind<typename this_type::hub>::other
                local_allocator(io_hubs.get_allocator());
        void* const local_storage(local_allocator.allocate(1));
        if (local_storage == nullptr)
        {
            PSYQ_ASSERT(false);
            return typename this_type::hub::shared_ptr();
        }
        typename this_type::hub::shared_ptr const local_hub(
            new(local_storage) typename this_type::hub(in_thread_id, local_allocator),
            [local_allocator](typename this_type::hub* const io_hub)
            {
                auto local_deallocator(local_allocator);
                local_deallocator.destroy(io_hub);
                local_deallocator.deallocate(io_hub, 1);
            },
            local_allocator);
        if (local_hub.get() != nullptr)
        {
            io_hubs.push_back(local_hub);
        }
        else
        {
            PSYQ_ASSERT(false);
        }
        return local_hub;
    }

    //-------------------------------------------------------------------------
    /// @name メッセージの送受信
    //@{
    /** @brief メッセージ中継器のメッセージを集配／分配する。
     */
    public: void flush()
    {
        std::lock_guard<psyq::spinlock> local_lock(this->lock_);
        this_type::trade_message(
            this->hubs_, this->export_packets_, this->import_packets_);
        this_type::swap_packet_container(
            this->export_packets_, this->import_packets_);
    }
    //@}
    private: static void trade_message(
        typename this_type::weak_hub_container& io_hubs,
        typename this_type::hub::shared_packet_container& io_export_packets,
        typename this_type::hub::shared_packet_container const& in_import_packets)
    {
        auto local_last_iterator(io_hubs.begin());
        for (auto i(local_last_iterator); i != io_hubs.end(); ++i)
        {
            auto const local_holder(i->lock());
            auto const local_hub(local_holder.get());
            if (local_hub != nullptr)
            {
                local_hub->trade_message(
                    io_export_packets, in_import_packets);
                i->swap(*local_last_iterator);
                ++local_last_iterator;
            }
        }
        io_hubs.erase(local_last_iterator, io_hubs.end());
    }

    private: static void swap_packet_container(
        typename this_type::hub::shared_packet_container& io_export_packets,
        typename this_type::hub::shared_packet_container& io_import_packets)
    {
        if (io_import_packets.capacity() < io_export_packets.size() * 2
            || io_export_packets.size() < 16)
        {
            io_import_packets.clear();
        }
        else
        {
            io_import_packets =
                typename this_type::hub::shared_packet_container();
            io_import_packets.reserve(io_export_packets.size() * 2);
        }
        io_import_packets.swap(io_export_packets);
    }

    //-------------------------------------------------------------------------
    /// @name メッセージルータのプロパティ
    //@{
    public: PSYQ_CONSTEXPR typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->hubs_.get_allocator();
    }

    /** @brief メッセージの送受信に使うアドレスを取得する。
        @return メッセージの送受信に使うアドレス。
     */
    public: PSYQ_CONSTEXPR typename this_type::receiver::tag::key const get_address()
    const PSYQ_NOEXCEPT
    {
        return this->address_;
    }

    /** @brief このルータから送信するメッセージの荷札を構築する。
        @param[in] in_receiver_address メッセージ受信アドレス。
        @param[in] in_receiver_mask    メッセージ受信マスク。
     */
    public: PSYQ_CONSTEXPR typename this_type::receiver::tag make_receiver_tag(
        typename this_type::receiver::tag::key const in_receiver_address,
        typename this_type::receiver::tag::key const in_receiver_mask =
            ~this_type::receiver::tag::EMPTY_KEY)
    const PSYQ_NOEXCEPT
    {
        return typename this_type::receiver::tag(
            this->get_address(), in_receiver_address, in_receiver_mask);
    }
    //@}
    //-------------------------------------------------------------------------
    /// スレッド毎のメッセージ中継器のコンテナ。
    private: typename this_type::weak_hub_container hubs_;
    /// メッセージ中継器から輸入したメッセージパケットのコンテナ。
    private: typename this_type::hub::shared_packet_container import_packets_;
    /// メッセージ中継器へ輸出するメッセージパケットのコンテナ。
    private: typename this_type::hub::shared_packet_container export_packets_;
    /// このインスタンスのメッセージアドレス。
    private: typename this_type::receiver::tag::key address_;
    private: psyq::spinlock lock_;

}; // psyq::any::message::hub::router

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        inline void any_message()
        {
            psyq::any::rtti::make<psyq::test::floating_wrapper>();

            typedef psyq::any::message::router<> message_router;
            message_router local_router(
                0x7f000001, message_router::allocator_type());
            auto const local_hub(local_router.equip_hub());
            enum: message_router::receiver::call::key
            {
                METHOD_PARAMETER_VOID = 1,
                METHOD_PARAMETER_DOUBLE,
            };
            message_router::receiver::shared_ptr const local_method_a(
                new message_router::receiver(
                    [=](message_router::receiver::packet const& in_packet)
                    {
                        PSYQ_ASSERT(
                            in_packet.get_parameter_rtti()
                            == psyq::any::rtti::find<void>());
                    },
                    local_router.get_address()));
            double const local_double(0.5);
            message_router::receiver::shared_ptr const local_method_b(
                new message_router::receiver(
                    [local_double]
                    (message_router::receiver::packet const& in_packet)
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
                    local_router.get_address()));

            local_hub->register_receiver(METHOD_PARAMETER_VOID, local_method_a);
            local_hub->register_receiver(METHOD_PARAMETER_DOUBLE, local_method_b);
            local_hub->send_message(
                local_router.make_receiver_tag(local_router.get_address()),
                message_router::receiver::call(METHOD_PARAMETER_VOID));
            local_hub->send_internal_message(
                local_router.make_receiver_tag(local_router.get_address()),
                message_router::receiver::call(METHOD_PARAMETER_DOUBLE),
                floating_wrapper(0.5));
            local_router.flush();
            local_hub->flush();
            local_router.flush();
            local_hub->flush();
            local_hub->unregister_receiver(nullptr);
            local_hub->unregister_receiver(0, nullptr);
        }
    }
}

#endif // !defined(PSYQ_ANY_MESSAGE_ROUTER_HPP_)
