/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ANY_MESSAGE_TRANSMITTER_HPP_
#define PSYQ_ANY_MESSAGE_TRANSMITTER_HPP_

#ifdef _MSC_VER
#include <eh.h>
#endif // _MSC_VER
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
//#include "psyq/any/message/suite.hpp"
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
                    class zone;
            template<typename, typename> class transmitter;
        } // namespace message
    } // namespace any
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief スレッド別のメッセージ伝送器。

    @copydetails psyq::any::message::zone
 */
template<typename template_base_suite, typename template_allocator>
class psyq::any::message::transmitter
{
    private: typedef transmitter this_type; ///< thisが指す値の型。
    /// @cond
    friend psyq::any::message::zone<template_base_suite, template_allocator>;
    /// @endcond
    //-------------------------------------------------------------------------
    /// メッセージ伝送器の保持子。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// メッセージ伝送器の監視子。
    public: typedef std::weak_ptr<this_type> weak_ptr;
    /// 使用するメモリ割当子。
    public: typedef template_allocator allocator_type;
    /// メッセージ受信器。
    public: typedef psyq::any::message::receiver<template_base_suite> receiver;

    //-------------------------------------------------------------------------
    /// メッセージのメソッド種別からハッシュ値を生成する関数オブジェクト。
    private: struct method_hash
    {
        PSYQ_CONSTEXPR std::size_t operator()(
            typename this_type::receiver::call::key const in_method)
        const PSYQ_NOEXCEPT
        {
            return static_cast<std::size_t>(in_method);
        }
    };

    /// @copydoc receiver_map_
    private: typedef std::unordered_multimap<
        typename this_type::receiver::call::key,
        typename this_type::receiver::weak_ptr,
        typename this_type::method_hash,
        std::equal_to<typename this_type::receiver::call::key>,
        typename this_type::allocator_type>
            receiver_map;

    /// メッセージパケット保持子のコンテナ。
    private: typedef std::vector<
        typename this_type::receiver::packet::shared_ptr,
        typename this_type::allocator_type>
            shared_packet_container;

    //-------------------------------------------------------------------------
    /** @brief メッセージ伝送器を構築する。
        @param[in] in_thread_id       このメッセージ伝送器が稼働するスレッドの識別子。
        @param[in] in_message_address このメッセージ伝送器のメッセージ送受信アドレス。
        @param[in] in_allocator       このメッセージ伝送器が使うメモリ割当子の初期値。
     */
    private: transmitter(
        std::thread::id in_thread_id,
        typename this_type::receiver::tag::key const in_message_address,
        typename this_type::allocator_type const& in_allocator)
    :
        receiver_map_(in_allocator),
        export_packets_(in_allocator),
        import_packets_(in_allocator),
        delivery_packets_(in_allocator),
        thread_id_(std::move(in_thread_id)),
        message_address_(in_message_address)
    {}

    /// コピー構築子は使用禁止。
    private: transmitter(this_type const&);
    /// コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    //-------------------------------------------------------------------------
    /// @name メッセージ受信器の登録
    //@{
    /** @brief メッセージ受信器を登録する。

        - 登録したメッセージ受信器は this_type::receiver::weak_ptr
          で監視しているだけで、このメッセージ伝送器では所有権を持たない。
        - this_type::unregister_receiver() で登録を除去できる。

        @param[in] in_method   登録するメッセージ受信メソッドの種別。
        @param[in] in_receiver 登録するメッセージ受信器。
     */
    public: void register_receiver(
        typename this_type::receiver::call::key in_method,
        typename this_type::receiver::weak_ptr in_receiver)
    {
        std::lock_guard<psyq::spinlock> local_lock(this->lock_);
        this->receiver_map_.emplace(
            std::move(in_method), std::move(in_receiver));
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
                if (local_value.second.lock().get() == in_receiver)
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
        typename this_type::receiver::call::key const in_method,
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
    /** @brief メッセージ受信器を辞書から検索する。
        @param[in] in_receiver_map メッセージ受信器の辞書。
        @param[in] in_method       メッセージのメソッド番号。
        @param[in] in_receiver     メッセージ受信器。
     */
    private: static typename this_type::receiver_map::const_iterator
    find_receiver_iterator(
        typename this_type::receiver_map const& in_receiver_map,
        typename this_type::receiver::call::key const in_method,
        void const* const in_receiver)
    PSYQ_NOEXCEPT
    {
        auto local_iterator(in_receiver_map.find(in_method));
        while (
            local_iterator != in_receiver_map.end()
            && local_iterator->first == in_method
            && local_iterator->second.lock().get() != in_receiver)
        {
            ++local_iterator;
        }
        return local_iterator;
    }

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
    /** @brief 引数を持たないメッセージを構築し、メッセージゾーンの内と外への送信を予約する。

        - メッセージ送信の予約のみを行う。実際のメッセージ送受信処理は、
          psyq::any::message::zone::flush() と this_type::flush()
          で非同期で行われる。
        - メッセージゾーン内にのみメッセージを送信するには、
          this_type::post_zonal_message() を使う。
        - このメッセージ伝送器に登録されている受信器へのみメッセージを送信するには、
          this_type::send_local_message() を使う。

        @param[in] in_tag  送信するメッセージの荷札。
        @param[in] in_call 送信するメッセージの呼出状。
        @retval true  成功。メッセージ送信を予約した。
        @retval false 失敗。メッセージ送信を予約しなかった。
     */
    public: bool post_message(
        typename this_type::receiver::tag const& in_tag,
        typename this_type::receiver::call const& in_call)
    {
        return this->export_packet(
            this_type::create_external_packet(
                typename this_type::receiver::packet::suite(in_tag, in_call),
                this->get_allocator()));
    }

    /** @brief POD型の引数を持つメッセージを構築し、メッセージゾーンの内と外への送信を予約する。

        - メッセージ送信の予約のみを行う。実際のメッセージ送受信処理は、
          psyq::any::message::zone::flush() と this_type::flush()
          で非同期で行われる。
        - メッセージゾーン内にのみメッセージを送信するには、
          this_type::post_zonal_message() を使う。
        - このメッセージ伝送器に登録されている受信器へのみメッセージを送信するには、
          this_type::send_local_message() を使う。

        @param[in] in_tag       送信するメッセージの荷札。
        @param[in] in_call      送信するメッセージの呼出状。
        @param[in] in_parameter 送信するメッセージの引数。必ずPOD型。
        @retval true  成功。メッセージ送信を予約した。
        @retval false 失敗。メッセージ送信を予約しなかった。
     */
    public: template<typename template_parameter>
    bool post_message(
        typename this_type::receiver::tag const& in_tag,
        typename this_type::receiver::call const& in_call,
        template_parameter in_parameter);

    /** @brief 引数を持たないメッセージを構築し、メッセージゾーン内への送信を予約する。

        - メッセージ送信の予約のみを行う。実際のメッセージ送受信処理は、
          psyq::any::message::zone::flush() と this_type::flush()
          で非同期で行われる。
        - メッセージゾーンの内と外にメッセージを送信するには、
          this_type::post_message() を使う。
        - このメッセージ伝送器に登録されている受信器へのみメッセージを送信するには、
          this_type::send_local_message() を使う。

        @param[in] in_tag  送信するメッセージの荷札。
        @param[in] in_call 送信するメッセージの呼出状。
        @retval true  成功。メッセージ送信を予約した。
        @retval false 失敗。メッセージ送信を予約しなかった。
     */
    public: bool post_zonal_message(
        typename this_type::receiver::tag const& in_tag,
        typename this_type::receiver::call const& in_call)
    {
        return this->export_packet(
            this_type::create_internal_packet(
                typename this_type::suite(in_tag, in_call),
                this->get_allocator()));
    }

    /** @brief 任意型の引数を持つメッセージを構築し、メッセージゾーン内への送信を予約する。

        - メッセージ送信の予約のみを行う。実際のメッセージ送受信処理は、
          psyq::any::message::zone::flush() と this_type::flush()
          で非同期で行われる。
        - メッセージゾーンの内と外にメッセージを送信するには、
          this_type::post_message() を使う。
        - このメッセージ伝送器に登録されている受信器へのみメッセージを送信するには、
          this_type::send_local_message() を使う。

        @param[in] in_tag       送信するメッセージの荷札。
        @param[in] in_call      送信するメッセージの呼出状。
        @param[in] in_parameter 送信するメッセージの引数。
        @retval true  成功。メッセージ送信を予約した。
        @retval false 失敗。メッセージ送信を予約しなかった。
     */
    public: template<typename template_parameter>
    bool post_zonal_message(
        typename this_type::receiver::tag const& in_tag,
        typename this_type::receiver::call const& in_call,
        template_parameter in_parameter)
    {
        typedef typename this_type::receiver::packet::suite::template
            parametric<template_parameter>
                parametric_suite;
        return this->export_packet(
            this_type::create_internal_packet(
                parametric_suite(in_tag, in_call, std::move(in_parameter)),
                this->get_allocator()));
    }

    /** @brief このメッセージ伝送器に登録されている受信器へのみメッセージを送信する。

        - メッセージを送信し、メッセージ受信関数の終了までブロックする。
        - メッセージゾーンの内と外にメッセージを送信するには、
          this_type::post_message() を使う。
        - メッセージゾーン内にのみメッセージを送信するには、
          this_type::post_zonal_message() を使う。

        @param[in] in_packet 送信するメッセージパケット。
        @retval true  成功。メッセージを送信した。
        @retval false 失敗。メッセージを送信しなかった。
     */
    public: bool send_local_message(
        typename this_type::receiver::packet const& in_packet)
    {
        if (std::this_thread::get_id() != this->get_thread_id())
        {
            PSYQ_ASSERT(false);
            return false;
        }
        this_type::deliver_packet(this->receiver_map_, in_packet);
        return true;
    }

    /** @brief このメッセージ伝送器に登録されている受信器へのみメッセージを送信する。

        - メッセージを送信し、メッセージ受信関数の終了までブロックする。
        - メッセージゾーンの内と外にメッセージを送信するには、
          this_type::post_message() を使う。
        - メッセージゾーン内にのみメッセージを送信するには、
          this_type::post_zonal_message() を使う。

        @param[in] in_tag  送信するメッセージの荷札。
        @param[in] in_call 送信するメッセージの呼出状。
        @retval true  成功。メッセージを送信した。
        @retval false 失敗。メッセージを送信しなかった。
     */
    public: bool send_local_message(
        typename this_type::receiver::tag const& in_tag,
        typename this_type::receiver::call const& in_call)
    {
        typedef typename this_type::receiver::packet::suite suite;
        return this->send_local_message(
            typename this_type::receiver::packet::template
                internal<suite>(suite(in_tag, in_call)));
    }

    /** @brief このメッセージ伝送器に登録されている受信器へのみメッセージを送信する。

        - メッセージを送信し、メッセージ受信関数の終了までブロックする。
        - メッセージゾーンの内と外にメッセージを送信するには、
          this_type::post_message() を使う。
        - メッセージゾーン内にのみメッセージを送信するには、
          this_type::post_zonal_message() を使う。

        @param[in] in_tag       送信するメッセージの荷札。
        @param[in] in_call      送信するメッセージの呼出状。
        @param[in] in_parameter 送信するメッセージの引数。
        @retval true  成功。メッセージを送信した。
        @retval false 失敗。メッセージを送信しなかった。
     */
    public: template<typename template_parameter>
    bool send_local_message(
        typename this_type::receiver::tag const& in_tag,
        typename this_type::receiver::call const& in_call,
        template_parameter in_parameter)
    {
        typedef typename this_type::receiver::packet::suite::template
            parametric<template_parameter>
                suite;
        return this->send_local_message(
            typename this_type::receiver::packet::template
                internal<suite>(
                    suite(in_tag, in_call, std::move(in_parameter))));
    }

    /** @brief メッセージ受信器へメッセージを配信する。

        psyq::any::message::zone::flush() とこの関数を定期的に実行し、
        メッセージを循環させること。
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
            // メッセージ受信器とメッセージパケットを更新する。
            std::lock_guard<psyq::spinlock> local_lock(this->lock_);
            this_type::remove_empty_receiver(this->receiver_map_);
            this->delivery_packets_.swap(this->import_packets_);
        }

        // メッセージ受信器へメッセージを配信する。
        this_type::deliver_packet(
            this->receiver_map_, this->delivery_packets_);
        this_type::clear_packet_container(
            this->delivery_packets_, this->delivery_packets_.size());
    }
    //@}
    /** @brief メッセージの送信を予約する。
        @param[in] in_packet 送信するメッセージパケット。
        @retval true  成功。メッセージ送信を予約した。
        @retval false 失敗。メッセージ送信を予約しなかった。
     */
    private: bool export_packet(
        typename this_type::receiver::packet::shared_ptr in_packet)
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

    /** @brief 外部とメッセージパケットを交換する。
        @param[in,out] io_export_packets 輸出するメッセージパケットのコンテナ。
        @param[in]     in_import_packets 輸入するメッセージパケットのコンテナ。
     */
    private: void trade_packet_container(
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
        this_type::clear_packet_container(
            this->export_packets_, this->export_packets_.size());
    }

    private: static void clear_packet_container(
        typename this_type::shared_packet_container& io_container,
        std::size_t const in_last_size)
    {
        if (in_last_size < 16 || io_container.capacity() < in_last_size * 2)
        {
            io_container.clear();
        }
        else
        {
            io_container = typename this_type::shared_packet_container();
            io_container.reserve(in_last_size * 2);
        }
    }

    /** @brief メッセージゾーン外パケットを生成する。
        @param[in,out] io_suite     パケットに設定するメッセージスイート。
        @param[in]     in_allocator 使用するメモリ割当子。
        @return
            生成したメッセージパケットの保持子。
            生成に失敗した場合は、保持子は空となる。
     */
    private: template<typename template_suite>
    static typename this_type::receiver::packet::shared_ptr create_external_packet(
        template_suite&& io_suite,
        typename this_type::allocator_type const& in_allocator)
    {
        typedef typename this_type::receiver::packet::template
            external<template_suite>
                external_packet;
        return this_type::create_packet<external_packet>(
            std::move(io_suite), in_allocator);
    }

    /** @brief メッセージゾーン内パケットを生成する。
        @param[in,out] io_suite     パケットに設定するメッセージスイート。
        @param[in]     in_allocator 使用するメモリ割当子。
        @return
            生成したメッセージパケットの保持子。
            生成に失敗した場合は、保持子は空となる。
     */
    private: template<typename template_suite>
    static typename this_type::receiver::packet::shared_ptr create_internal_packet(
        template_suite&& io_suite,
        typename this_type::allocator_type const& in_allocator)
    {
        typedef typename this_type::receiver::packet::template
            internal<template_suite>
                internal_packet;
        return this_type::create_packet<internal_packet>(
            std::move(io_suite), in_allocator);
    }

    /** @brief メッセージパケットを生成する。
        @param[in,out] io_suite     パケットに設定するメッセージスイート。
        @param[in]     in_allocator 使用するメモリ割当子。
        @return
            生成したメッセージパケットの保持子。
            生成に失敗した場合は、保持子は空となる。
     */
    private: template<typename template_packet>
    static typename this_type::receiver::packet::shared_ptr create_packet(
        typename template_packet::suite&& io_suite,
        typename this_type::allocator_type const& in_allocator)
    {
        // メッセージパケットに割り当てるメモリ領域を確保する。
        typename this_type::allocator_type::template
            rebind<template_packet>::other
                local_allocator(in_allocator);
        auto const local_storage(local_allocator.allocate(1));
        if (local_storage == nullptr)
        {
            PSYQ_ASSERT(false);
            return typename this_type::receiver::packet::shared_ptr();
        }

        // メッセージパケット保持子を構築する。
        return typename this_type::receiver::packet::shared_ptr(
            new(local_storage) template_packet(std::move(io_suite)),
            [local_allocator](template_packet* const io_packet)
            {
                auto local_deallocator(local_allocator);
                local_deallocator.destroy(io_packet);
                local_deallocator.deallocate(io_packet, 1);
            },
            local_allocator);
    }

    /** @brief メッセージ受信器へメッセージを配信する。
        @param[in] in_receivers メッセージ受信器の辞書。
        @param[in] in_packets   配信するメッセージパケットのコンテナ。
     */
    private: static void deliver_packet(
        typename this_type::receiver_map const& in_receivers,
        typename this_type::shared_packet_container const& in_packets)
    {
        // メッセージパケットを走査し、メッセージ受信器の辞書へ中継する。
        for (auto& local_packet_holder: in_packets)
        {
            auto const local_packet_pointer(local_packet_holder.get());
            if (local_packet_pointer != nullptr)
            {
                this_type::deliver_packet(in_receivers, *local_packet_pointer);
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
    private: static void deliver_packet(
        typename this_type::receiver_map const& in_receivers,
        typename this_type::receiver::packet const& in_packet)
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
                && local_suite.get_tag().agree_receiver_address(
                    local_receiver->get_message_address()))
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
    /// @name メッセージ伝送器のプロパティ
    //@{
    /** @brief このメッセージ伝送器が使うメモリ割当子を取得する。
        @return このメッセージ伝送器が使うメモリ割当子。
     */
    public: PSYQ_CONSTEXPR typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->receiver_map_.get_allocator();
    }

    /** @brief このメッセージ伝送器が稼働するスレッドの識別子を取得する。
        @return このメッセージ伝送器が稼働するスレッドの識別子。
     */
    public: PSYQ_CONSTEXPR std::thread::id const& get_thread_id()
    const PSYQ_NOEXCEPT
    {
        return this->thread_id_;
    }

    /** @brief このメッセージ伝送器のメッセージ送信アドレスを取得する。
        @return このメッセージ伝送器のメッセージ送信アドレス。
     */
    public: PSYQ_CONSTEXPR typename this_type::receiver::tag::key const
    get_message_address() const PSYQ_NOEXCEPT
    {
        return this->message_address_;
    }

    /** @brief このメッセージ伝送器から送信するメッセージの荷札を構築する。
        @param[in] in_receiver_address メッセージ受信アドレス。
        @param[in] in_receiver_mask    メッセージ受信マスク。
     */
    public: PSYQ_CONSTEXPR typename this_type::receiver::tag make_receiver_tag(
        typename this_type::receiver::tag::key const in_receiver_address,
        typename this_type::receiver::tag::key const in_receiver_mask =
            ~this_type::receiver::tag::key(0))
    const PSYQ_NOEXCEPT
    {
        return typename this_type::receiver::tag(
            this->get_message_address(), in_receiver_address, in_receiver_mask);
    }
    //@}
    //-------------------------------------------------------------------------
    /// メッセージ受信器の辞書。
    private: typename this_type::receiver_map receiver_map_;
    /// 外部から輸入したメッセージパケットのコンテナ。
    private: typename this_type::shared_packet_container import_packets_;
    /// 外部へ輸出するメッセージパケットのコンテナ。
    private: typename this_type::shared_packet_container export_packets_;
    /// メッセージ受信器へ配信するパケット。
    private: typename this_type::shared_packet_container delivery_packets_;
    /// このメッセージ伝送器を稼働させるスレッドの識別子。
    private: std::thread::id const thread_id_;
    private: psyq::spinlock lock_;
    /// このメッセージ伝送器のメッセージ送受信アドレス。
    private: typename this_type::receiver::tag::key const message_address_;

}; // class psyq::any::message::transmitter

#endif // !defined(PSYQ_ANY_MESSAGE_TRANSMITTER_HPP_)
