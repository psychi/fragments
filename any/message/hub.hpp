/** @file
    @brief RPCメッセージの送受信。
    @copydetails psyq::any::message::router
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ANY_MESSAGE_HUB_HPP_
#define PSYQ_ANY_MESSAGE_HUB_HPP_

#include <memory>
#include <vector>
#include <list>
#include <unordered_map>
#include <mutex>
//#include "psyq/any/message/receiver.hpp"

/// @cond
namespace psyq
{
    namespace any
    {
        namespace message
        {
            template<
                typename = psyq::any::message::suite<std::uint32_t, std::uint32_t, std::uint32_t>,
                typename = std::allocator<void*>>
                    class hub;
        } // namespace message
    } // namespace any
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_base_suite, typename template_allocator>
class psyq::any::message::hub
{
    private: typedef hub this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// メッセージ分配器の保持子。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// メッセージ分配器の監視子。
    public: typedef std::weak_ptr<this_type> weak_ptr;

    /// パケットに用いるメモリ割当子。
    public: typedef template_allocator allocator_type;
    /// メッセージ受信器。
    public: typedef psyq::any::message::receiver<template_base_suite> receiver;
    /// @copydoc psyq::any::message::receiver::packet
    public: typedef typename this_type::receiver::packet packet;
    /// メッセージの荷札と呼出状と引数をひとまとめにしたスイート。
    public: typedef typename this_type::receiver::packet::suite suite;
    /// メッセージの呼出状。
    public: typedef typename this_type::receiver::packet::suite::call call;
    /// メッセージの荷札。
    public: typedef typename this_type::receiver::packet::suite::tag tag;
    /// メッセージパケット保持子のコンテナ。
    public: typedef std::vector<
        typename this_type::packet::shared_ptr,
        typename this_type::allocator_type>
            shared_packet_container;
    /// @cond
    public: class router;
    /// @endcond
    //-------------------------------------------------------------------------
    /// メッセージのメソッド種別からハッシュ値を生成する関数オブジェクト。
    private: struct method_hash
    {
        PSYQ_CONSTEXPR std::size_t operator()(
            typename this_type::call::key const in_method)
        const PSYQ_NOEXCEPT
        {
            return static_cast<std::size_t>(in_method);
        }
    };

    /// @copydoc receiver_map_
    private: typedef std::unordered_multimap<
        typename this_type::call::key,
        typename this_type::receiver::weak_ptr,
        typename this_type::method_hash,
        std::equal_to<typename this_type::call::key>,
        typename this_type::allocator_type>
            receiver_map;

    //-------------------------------------------------------------------------
    /// @name メッセージ分配器の構築
    //@{
    /** @brief メッセージ分配器を構築する。
        @param[in] in_thread_id このインスタンスが活動するスレッドの識別子。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: hub(
        std::thread::id in_thread_id,
        typename this_type::allocator_type const& in_allocator)
    :
        receiver_map_(in_allocator),
        export_packets_(in_allocator),
        import_packets_(in_allocator),
        distribution_packets_(in_allocator),
        thread_id_(std::move(in_thread_id))
    {}
    //@}
    /// コピー構築子は使用禁止。
    private: hub(this_type const&);
    /// コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    //-------------------------------------------------------------------------
    /// @name プロパティ
    //@{
    public: PSYQ_CONSTEXPR typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->receiver_map_.get_allocator();
    }

    public: PSYQ_CONSTEXPR std::thread::id const& get_thread_id()
    const PSYQ_NOEXCEPT
    {
        return this->thread_id_;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name メッセージ受信機の登録
    //@{
    /** @brief メッセージ受信器を登録する。

        this_type::unregister_receiver() で登録を除去できる。

        @param[in] in_method   登録するメッセージ受信メソッドの種別。
        @param[in] in_receiver 登録するメッセージ受信器。
     */
    public: void register_receiver(
        typename this_type::call::key const in_method,
        typename this_type::receiver::shared_ptr const& in_receiver)
    {
        std::lock_guard<psyq::spinlock> local_lock(this->lock_);
        this->receiver_map_.insert(
            typename this_type::receiver_map::value_type(in_method, in_receiver));
    }

    /** @brief メッセージ受信器の登録を除去する。

        this_type::register_receiver() で登録したメッセージ受信器を解放する。

        @param[in] in_receiver 登録を除去するメッセージ受信器。
     */
    public: void unregister_receiver(
        typename this_type::receiver const* const in_receiver)
    {
        if (in_receiver != nullptr)
        {
            // メッセージ受信器の辞書から削除する。
            for (auto& local_value: this->receiver_map_)
            {
                auto const local_receiver(local_value.second.lock().get());
                if (local_receiver == nullptr || local_receiver == in_receiver)
                {
                    local_value.second.reset();
                }
            }
        }
    }

    /** @brief メッセージ受信器の登録を除去する。

        this_type::register_receiver() で登録したメッセージ受信器を解放する。

        @param[in] in_method   登録を除去するメッセージ受信器のメソッド番号。
        @param[in] in_receiver 登録を除去するメッセージ受信器。
     */
    public: void unregister_receiver(
        typename this_type::call::key const in_method,
        typename this_type::receiver const* const in_receiver)
    {
        if (in_receiver != nullptr)
        {
            // メッセージ受信器の辞書から削除する。
            auto const local_iterator(
                this_type::find_receiver_iterator(
                    this->receiver_map_, in_method, in_receiver));
            if (local_iterator != this->receiver_map_.end()
                && local_iterator->first == in_method)
            {
                const_cast<typename this_type::receiver::weak_ptr&>
                    (local_iterator->second).reset();
            }
        }
    }
    //@}
    /** @brief 空になったメッセージ受信器を辞書から削除する。
        @param[in,out] io_receivers 空のメッセージ受信器を削除する辞書。
     */
    private: static void remove_empty_receiver(
        typename this_type::receiver_map& io_receivers)
    {
        for (auto i(io_receivers.begin()); i != io_receivers.end();)
        {
            if (i->second.lock().get() != nullptr)
            {
                ++i;
            }
            else
            {
                i = io_receivers.erase(i);
            }
        }
    }

    //-------------------------------------------------------------------------
    /// @name メッセージの送受信
    //@{
    public: bool send_message(
        typename this_type::packet::shared_ptr in_packet)
    {
        if (std::this_thread::get_id() != this->get_thread_id())
        {
            PSYQ_ASSERT(false);
            return false;
        }
        else if (in_packet.get() == nullptr)
        {
            return false;
        }
        this->export_packets_.emplace_back(std::move(in_packet));
        return true;
    }
#if 0
    /** @brief 引数を持たないメッセージを構築し、送信する。

        引数を持つメッセージを送信するには、以下の関数を使う。
        - this_type::send_internal_message()
        - this_type::send_external_message()

        @param[in] in_tag      送信するメッセージの荷札。
        @param[in] in_call     送信するメッセージの呼出状。
        @param[in] in_external プロセス外にもメッセージを送信するかどうか。
     */
    public: bool send_message(
        typename this_type::tag const& in_tag,
        typename this_type::call const& in_call,
        bool const in_external)
    {
        if (in_external)
        {
            return false;
        }
        else
        {
            auto const local_packet(
                this_type::create_packet(
                    typename this_type::suite(in_tag, in_call),
                    this->get_allocator()));
            return true;
        }
    }

    public: template<typename template_suite>
    static typename this_type::packet::shared_ptr create_packet(
        template_suite in_suite,
        typename this_type::allocator_type const& in_allocator)
    {
        typedef psyq::any::message::internal_packet<typename this_type::suite, template_suite>
            internal_packet;
        typedef typename template_allocator::template rebind<internal_packet>::other
            packet_allocator;
        packet_allocator local_allocator(in_allocator);
        auto const local_storage(local_allocator.allocate(1));
        if (local_storage == nullptr)
        {
            PSYQ_ASSERT(false);
            return typename this_type::packet::shared_ptr();
        }
        new(local_storage) internal_packet(std::move(in_suite));
        typename this_type::packet::shared_ptr local_packet(local_storage);
        /*
        return typename this_type::packet::shared_ptr(
            new(local_storage) internal_packet(std::move(in_suite)),
            [local_allocator](internal_packet* const io_packet)
            {
                const_cast<decltype(local_allocator)&>(local_allocator).destroy(io_packet);
                const_cast<decltype(local_allocator)&>(local_allocator).deallocate(io_packet, 1);
            },
            local_allocator);
         */
        return typename this_type::packet::shared_ptr();
    }

    /** @brief 任意型の引数を持つメッセージを構築し、プロセス内へ送信する。

        - 引数を持たないメッセージを送信するには、
          this_type::send_message() を使う。
        - 引数を持つメッセージをプロセス外にも送信するには、
          this_type::send_internal_message() を使う。

        @param[in] in_tag       送信するメッセージの荷札。
        @param[in] in_call      送信するメッセージの呼出状。
        @param[in] in_parameter 送信するメッセージの引数。
     */
    public: template<typename template_parameter>
    bool send_internal_message(
        typename this_type::tag const& in_tag,
        typename this_type::call const& in_call,
        template_parameter in_parameter)
    {
        /*
        typedef typename this_type::suite::template
            parametric<template_parameter>
                parametric_suite;
        return this->send_message(
            this_type::packet::template internal<parametric_suite>::create(
                parametric_suite(in_tag, in_call, std::move(in_parameter)),
                this->get_allocator()));
         */
        return false;
    }

    /** @brief POD型の引数を持つメッセージを構築し、プロセスの内と外へ送信する。

        - 引数を持たないメッセージを送信するには、
          this_type::send_message() を使う。
        - 引数を持つメッセージをプロセス内だけに送信するには、
          this_type::send_internal_message() を使う。

        @param[in] in_tag       送信するメッセージの荷札。
        @param[in] in_call      送信するメッセージの呼出状。
        @param[in] in_parameter 送信するメッセージの引数。必ずPOD型。
     */
    public: template<typename template_parameter>
    bool send_external_message(
        typename this_type::tag const& in_tag,
        typename this_type::call const& in_call,
        template_parameter in_parameter);
#endif
    /** @brief 外部とメッセージを交換する。
        @param[in,out] io_export_packets 輸出するメッセージパケットのコンテナ。
        @param[in]     in_import_packets 輸入するメッセージパケットのコンテナ。
     */
    public: void trade_message(
        typename this_type::shared_packet_container& io_export_packets,
        typename this_type::shared_packet_container const& in_import_packets)
    {
        std::lock_guard<psyq::spinlock> local_lock(this->lock_);
        this->import_packets_.insert(
            this->import_packets_.end(),
            in_import_packets.begin(),
            in_import_packets.end());
        io_export_packets.reserve(
            io_export_packets.size() + this->export_packets_.size());
        for (auto& local_packet_holder: this->export_packets_)
        {
            io_export_packets.emplace_back(std::move(local_packet_holder));
        }
        this->export_packets_.clear();
    }

    /** @brief メッセージ受信器へメッセージを配信する。
     */
    public: void flush()
    {
        if (std::this_thread::get_id() != this->get_thread_id())
        {
            PSYQ_ASSERT(false);
            return;
        }
        else
        {
            std::lock_guard<psyq::spinlock> local_lock(this->lock_);
            this_type::remove_empty_receiver(this->receiver_map_);
            this->distribution_packets_.swap(this->import_packets_);
        }
        this_type::distribute_message(
            this->receiver_map_, this->distribution_packets_);
        this->distribution_packets_.clear();
    }
    //@}
    private: void export_message(
        typename this_type::packet::shared_ptr in_packet)
    {
        if (std::this_thread::get_id() == this->get_thread_id())
        {
            this->export_packets_.emplace_back(std::move(in_packet));
        }
        else
        {
            PSYQ_ASSERT(false);
        }
    }

    /** @brief メッセージ受信器へメッセージを配信する。
        @param[in] in_receivers メッセージ受信器の辞書。
        @param[in] in_packets   配信するメッセージパケットのコンテナ。
     */
    private: static void distribute_message(
        typename this_type::receiver_map const& in_receivers,
        typename this_type::shared_packet_container const& in_packets)
    {
        // メッセージパケットを走査し、メッセージ受信器の辞書へ中継する。
        for (auto& local_packet_holder: in_packets)
        {
            auto const local_packet_pointer(local_packet_holder.get());
            if (local_packet_pointer != nullptr)
            {
                this_type::distribute_message(in_receivers, *local_packet_pointer);
            }
            else
            {
                PSYQ_ASSERT(false);
            }
        }
    }

    /** @brief メッセージ受信器へメッセージを配信する。
        @param[in] in_receivers メッセージ受信器の辞書。
        @param[in] in_packet    配信するメッセージパケット。
     */
    private: static void distribute_message(
        typename this_type::receiver_map const& in_receivers,
        typename this_type::packet const& in_packet)
    {
        // メッセージ受信器の辞書のうち、
        // メソッド番号が一致するものへメッセージを中継する。
        auto& local_suite(in_packet.get_suite());
        auto const local_method(local_suite.get_call().get_method());
        for (
            auto i(in_receivers.find(local_method));
            i != in_receivers.end() && i->first == local_method;
            ++i)
        {
            // メッセージ受信アドレスとメッセージ受信器が合致するか判定する。
            auto const local_holder(i->second.lock());
            auto const local_receiver(local_holder.get());
            if (local_receiver != nullptr
                && local_suite.get_tag().agree_receiver_address(local_receiver->get_address()))
            {
                // メッセージ受信関数を呼び出す。
                auto& local_functor(local_receiver->get_functor());
                if (bool(local_functor))
                {
                    local_functor(in_packet);
                }
            }
        }
    }

    //-------------------------------------------------------------------------
    /// メッセージ受信器の辞書。
    private: typename this_type::receiver_map receiver_map_;
    /// 外部から輸入したメッセージパケットのコンテナ。
    private: typename this_type::shared_packet_container import_packets_;
    /// 外部へ輸出するメッセージパケットのコンテナ。
    private: typename this_type::shared_packet_container export_packets_;
    /// メッセージ受信器へ配信するパケット。
    private: typename this_type::shared_packet_container distribution_packets_;
    private: std::thread::id thread_id_;
    private: psyq::spinlock lock_;

}; // psyq::any::message::hub

#endif // !defined(PSYQ_ANY_MESSAGE_HUB_HPP_)
