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
#include "./suite.hpp"
#include "./receiver.hpp"

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
            template<typename, typename> class listener;
            template<typename, typename> class dispatcher;
            template<typename, typename> class transmitter;
        } // namespace message
    } // namespace any
} // namespace psyq
/// @endcond

#ifdef PSYQ_ANY_MESSAGE_DISPATCHER_HPP_
//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_base_suite, typename template_allocator>
class psyq::any::message::listener
{
    private: typedef listener this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    public: typedef std::shared_ptr<this_type> shared_ptr;
    public: typedef std::weak_ptr<this_type> weak_ptr;

    /// @brief メッセージ一式を保持するパケットの基底型。
    public: typedef psyq::any::message::packet<template_base_suite> packet;

    /// @brief this_type で使うメモリ割当子。
    public: typedef template_allocator allocator_type;

    /// @brief メッセージ受信関数の型。
    public: typedef std::function<void(typename this_type::packet const&)> function;

    /// @brief 呼び出す関数の識別値。
    public: typedef typename this_type::packet::suite::call function_key;

    /// @brief メッセージ受信器の識別値。
    public: typedef typename this_type::packet::suite::tag key;

    private: typedef
        std::unordered_map<
            typename this_type::function_key,
            typename this_type::function,
            std::hash<typename this_type::function_key>,
            std::equal_to<typename this_type::function_key>,
            typename this_type::allocator_type>
        function_map;

    //-------------------------------------------------------------------------
    public: listener(
        typename this_type::key in_key,
        typename this_type::allocator_type const& in_allocator):
    key_(std::move(in_key)),
    functions_(in_allocator)
    {}

    public: bool register_function(
        typename this_type::function_key in_function_key,
        typename this_type::function in_function)
    {
        return static_cast<bool>(in_function)
            && this->functions_.emplace(
                std::move(in_function_key), std::move(in_function)).second;
    }

    public: typename this_type::function unregister_function(
        typename this_type::function_key const& in_function_key)
    {
        auto const local_find(this->functions_.find(in_function_key));
        if (local_find == this->functions_.end())
        {
            return typename this_type::function();
        }
        auto const local_function(std::move(local_find->second));
        this->functions_.erase(local_find);
        return local_function;
    }

    public: bool call_function(
        typename this_type::function_key const in_function_key,
        typename this_type::packet const& in_packet)
    {
        auto const local_find(this->functions_.find(in_function_key));
        if (local_find == this->functions_.end())
        {
            return false;
        }
        local_find->second(in_packet);
        return true;
    }

    public: typename this_type::function const* find_function(
        typename this_type::function_key const in_function_key)
    const PSYQ_NOEXCEPT
    {
        auto const local_find(this->functions_.find(in_function_key));
        return local_find != this->functions_.end()?
            &local_find->second: nullptr;
    }

    //-------------------------------------------------------------------------
    private: typename this_type::key key_;
    private: typename this_type::function_map functions_;

}; // class psyq::any::message::listener

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_base_suite, typename template_allocator>
class psyq::any::message::dispatcher
{
    private: typedef dispatcher this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// @brief メッセージ一式を保持するパケットの基底型。
    public: typedef psyq::any::message::packet<template_base_suite> packet;

    /// @brief メッセージ受信先の識別値。
    public: typedef typename this_type::packet::suite::tag receiver_key;

    /// @brief メッセージ受信関数の型。
    public: typedef
        std::function<void(typename this_type::packet const&)>
        function;

    /// @brief メッセージ受信関数の所有権ありスマートポインタの型。
    public: typedef
        std::shared_ptr<typename this_type::function>
        function_shared_ptr;

    /// @brief メッセージ受信関数の所有権なしスマートポインタの型。
    public: typedef
        std::weak_ptr<typename this_type::function>
        function_weak_ptr;

    /// @brief メッセージ受信関数の識別値。
    public: typedef typename this_type::packet::suite::call function_key;

    public: typedef std::int32_t function_priority;

    //-------------------------------------------------------------------------
    public: bool register_function(
        typename this_type::receiver_key const in_receiver_key,
        typename this_type::function_key const in_function_key,
        typename this_type::function_shared_ptr const& in_function,
        typename this_type::function_priority const in_priority)
    {
        auto const local_receiver(in_receiver.get());
        if (local_receiver == nullptr || !this->agree_this_thread())
        {
            return false;
        }
        auto const local_iterator(
            this_type::find_receiver_iterator(
                this->receiver_map_, *local_receiver, in_method));
        if (local_iterator != this->receiver_map_.end()
            && local_iterator->first == in_method)
        {
            // 等価な組み合わせがすでに登録されていた。
            return false;
        }
        this->receiver_map_.emplace(in_method, in_receiver);
        return true;
    }

}; // class psyq::any::message::dispatcher
#endif // !defined(PSYQ_ANY_MESSAGE_DISPATCHER_HPP_)

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief スレッド別のRPCメッセージ伝送器。

    - psyq::any::message::zone::equip_transmitter() で、 this_type を用意する。
    - this_type::register_receiver() で、 this_type::receiver を登録する。
    - this_type::post_message() で、メッセージパケットを送信する。
    - psyq::any::message::zone::flush() で、
      this_type が持つメッセージパケットが集配される。
    - this_type::flush() で、 this_type が持つメッセージパケットを
      this_type::receiver へ配信する。

    @tparam template_base_suite @copydoc psyq::any::message::packet::suite
    @tparam template_allocator  @copydoc psyq::any::message::transmitter::allocator_type
 */
template<typename template_base_suite, typename template_allocator>
class psyq::any::message::transmitter
{
    private: typedef transmitter this_type; ///< thisが指す値の型。

    /// @cond
    friend psyq::any::message::zone<template_base_suite, template_allocator>;
    /// @endcond

    //-------------------------------------------------------------------------
    /// this_type の保持子。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// this_type の監視子。
    public: typedef std::weak_ptr<this_type> weak_ptr;
    /// this_type で使うメモリ割当子。
    public: typedef template_allocator allocator_type;
    /// this_type で使うメッセージ受信器。
    public: typedef psyq::any::message::receiver<template_base_suite> receiver;

    //-------------------------------------------------------------------------
    /// メッセージのメソッド種別からハッシュ値を生成する関数オブジェクト。
    private: struct method_hash
    {
        /** @brief メッセージのメソッド種別からハッシュ値を生成する。
            @return メソッド種別のハッシュ値。
            @param[in] in_method メッセージのメソッド種別。
         */
        PSYQ_CONSTEXPR std::size_t operator()(
            typename this_type::receiver::call::key const in_method)
        const PSYQ_NOEXCEPT
        {
            return static_cast<std::size_t>(in_method);
        }
    };

    /// @copydoc receiver_map_
    private: typedef
        std::unordered_multimap<
            typename this_type::receiver::call::key,
            typename this_type::receiver::weak_ptr,
            typename this_type::method_hash,
            std::equal_to<typename this_type::receiver::call::key>,
            typename this_type::allocator_type>
        receiver_map;

    /// this_type::receiver::packet 保持子のコンテナ。
    private: typedef
        std::vector<
            typename this_type::receiver::packet::shared_ptr,
            typename this_type::allocator_type>
        shared_packet_container;

    //-------------------------------------------------------------------------
    /** @name this_type::receiver の登録
        @{
     */
    /** @brief this_type::receiver を登録する。

        - 登録に成功した後は、
          メッセージ受信アドレスとメッセージ受信メソッド種別が合致する
          メッセージパケットを受信するたび、 this_type::receiver::get_functor()
          で取得できるメッセージ受信関数が実行される。
        - 登録した this_type::receiver は、 this_type::receiver::weak_ptr
          で監視しているだけで、 this_type では所有権を持たない。
          this_type::receiver の所有権は、ユーザーが管理すること。
        - 登録した this_type::receiver が破棄されると、登録から解放される。
          または this_type::unregister_receiver() でも、登録から解放できる。

        @retval true 成功。 in_receiver / in_method の組み合わせを登録した。
        @retval false
            失敗。 this_type::receiver を登録しなかった。
            - in_receiver が空だと失敗する。
            - this_type::get_thread_id() と合致しないスレッドで実行すると失敗する。
            - in_receiver / in_method と等価な組み合わせがすでに登録されていると失敗する。

        @param[in] in_receiver 登録する this_type::receiver 。
        @param[in] in_method   登録するメッセージ受信メソッドの種別。
     */
    public: bool register_receiver(
        typename this_type::receiver::shared_ptr const& in_receiver,
        typename this_type::receiver::call::key in_method)
    {
        auto const local_receiver(in_receiver.get());
        if (local_receiver == nullptr || !this->agree_this_thread())
        {
            return false;
        }
        auto const local_iterator(
            this_type::find_receiver_iterator(
                this->receiver_map_, *local_receiver, in_method));
        if (local_iterator != this->receiver_map_.end()
            && local_iterator->first == in_method)
        {
            // 等価な組み合わせがすでに登録されていた。
            return false;
        }
        this->receiver_map_.emplace(in_method, in_receiver);
        return true;
    }

    /** @brief this_type::receiver を登録から解放する。

        this_type::register_receiver() で登録した
        this_type::receiver を解放する。

        @return 登録から解放した this_type::receiver の監視子。
        @param[in] in_receiver 登録から解放する this_type::receiver 。
     */
    public: typename this_type::receiver::weak_ptr unregister_receiver(
        typename this_type::receiver const* const in_receiver)
    {
        typename this_type::receiver::weak_ptr local_receiver;
        if (in_receiver != nullptr)
        {
            // メッセージ受信器の辞書から削除する。
            for (auto& local_value: this->receiver_map_)
            {
                if (local_value.second.lock().get() == in_receiver)
                {
                    local_receiver = std::move(local_value.second);
                    //local_value.second.reset();
                }
            }
        }
        return local_receiver;
    }

    /** @brief this_type::receiver を登録から解放する。
        @copydetails unregister_receiver(typename this_type::receiver const* const)
        @param[in] in_method 解放する this_type::receiver のメソッド種別。
     */
    public: typename this_type::receiver::weak_ptr unregister_receiver(
        typename this_type::receiver const* const in_receiver,
        typename this_type::receiver::call::key const in_method)
    {
        typename this_type::receiver::weak_ptr local_receiver;
        if (in_receiver != nullptr)
        {
            // メッセージ受信器の辞書から削除する。
            auto const local_iterator(
                this_type::find_receiver_iterator(
                    this->receiver_map_, *in_receiver, in_method));
            if (local_iterator != this->receiver_map_.end()
                && local_iterator->first == in_method)
            {
                local_receiver = std::move(
                    const_cast<typename this_type::receiver::weak_ptr&>(
                        local_iterator->second));
                //const_cast<typename this_type::receiver::weak_ptr&>
                //    (local_iterator->second).reset();
            }
        }
        return local_receiver;
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name メッセージパケットの送受信
        @{
     */
    /** @interface interface_post_message
        @brief post_message() と post_zonal_message() に共通する説明。

        - this_type::get_thread_id() と合致しないスレッドで実行すると失敗する。
        - この関数では、メッセージパケットの送信の予約のみを行う。
          実際のメッセージパケット送信処理は、この関数の実行後、
          psyq::any::message::zone::flush() / this_type::flush()
          の順に実行することで行なわれる。
        - メッセージパケットの受信処理は、送信処理が行われた後、
          psyq::any::message::zone::flush() / this_type::flush()
          の順に実行することで行なわれる。
        - 同一スレッドで送信を予約したメッセージパケットの受信順序は、
          送信予約順序と同じになる。
     */
    /** @brief
        引数を持たないメッセージパケットを動的メモリ確保で生成し、
        メッセージゾーンの内と外への送信を予約する。

        @copydetails interface_post_message

        @sa メッセージゾーン内にだけメッセージパケットを送信するには、
            this_type::post_zonal_message() を使う。
        @sa *thisに登録されている this_type::receiver にだけメッセージパケットを
            送信するには、 this_type::send_local_message() を使う。

        @retval true  成功。メッセージパケットの送信を予約した。
        @retval false 失敗。メッセージパケットの送信を予約しなかった。

        @param[in] in_tag  送信するメッセージの荷札。
        @param[in] in_call 送信するメッセージの呼出状。
        @todo ゾーンの外へ送信する処理は未実装。
     */
    public: bool post_message(
        typename this_type::receiver::tag const& in_tag,
        typename this_type::receiver::call const& in_call)
    {
        return this->add_export_packet(
            this_type::create_external_packet(
                typename this_type::receiver::packet::suite(in_tag, in_call),
                this->get_allocator()));
    }

    /** @brief
        POD型の引数を持つメッセージパケットを動的メモリ確保で生成し、
        メッセージゾーンの内と外への送信を予約する。

        @copydetails post_message()

        @param[in] in_parameter 送信するメッセージの引数。必ずPOD型。
        @todo 未実装。
     */
    public: template<typename template_parameter>
    bool post_message(
        typename this_type::receiver::tag const& in_tag,
        typename this_type::receiver::call const& in_call,
        template_parameter in_parameter);

    /** @brief
        引数を持たないメッセージパケットを動的メモリ確保で生成し、
        メッセージゾーン内への送信を予約する。

        @copydetails interface_post_message

        @sa メッセージゾーンの内と外にメッセージパケットを送信するには、
            this_type::post_message() を使う。
        @sa *thisに登録されている this_type::receiver にだけメッセージパケットを
            送信するには、 this_type::send_local_message() を使う。

        @retval true  成功。メッセージ送信を予約した。
        @retval false 失敗。メッセージ送信を予約しなかった。

        @param[in] in_tag  送信するメッセージの荷札。
        @param[in] in_call 送信するメッセージの呼出状。
     */
    public: bool post_zonal_message(
        typename this_type::receiver::tag const& in_tag,
        typename this_type::receiver::call const& in_call)
    {
        return this->add_export_packet(
            this_type::create_zonal_packet(
                typename this_type::receiver::packet::suite(in_tag, in_call),
                this->get_allocator()));
    }

    /** @brief
        任意型の引数を持つメッセージパケットを動的メモリ確保で生成し、
        メッセージゾーン内への送信を予約する。

        @copydetails post_zonal_message()
        @param[in] in_parameter 送信するメッセージの引数。
     */
    public: template<typename template_parameter>
    bool post_zonal_message(
        typename this_type::receiver::tag const& in_tag,
        typename this_type::receiver::call const& in_call,
        template_parameter in_parameter)
    {
        typedef typename this_type::receiver::packet::suite::template
            parametric<template_parameter>
                suite;
        return this->add_export_packet(
            this_type::create_zonal_packet(
                suite(in_tag, in_call, std::move(in_parameter)),
                this->get_allocator()));
    }

    /** @interface interface_send_local_message
        @brief send_local_message() に共通する説明。

        - this_type::get_thread_id() と合致しないスレッドで実行すると失敗する。
        - メッセージパケットを送信し、受信関数の終了までブロックする。

        @sa this_type::receiver を*thisに登録するには、
            this_type::register_receiver() を使う。
        @sa メッセージゾーンの内と外にメッセージパケットを送信するには、
            this_type::post_message() を使う。
        @sa メッセージゾーン内にだけメッセージパケットを送信するには、
            this_type::post_zonal_message() を使う。

        @retval true  成功。メッセージパケットを送信した。
        @retval false 失敗。メッセージパケットを送信しなかった。
     */
    /** @brief
        メッセージパケットを、*thisに登録されている
        this_type::receiver にだけ送信する。

        @copydetails interface_send_local_message

        @param[in] in_packet 送信するメッセージパケット。
     */
    public: bool send_local_message(
        typename this_type::receiver::packet const& in_packet)
    {
        if (!this->agree_this_thread())
        {
            return false;
        }
        this_type::deliver_packet(this->receiver_map_, in_packet);
        return true;
    }

    /** @brief
        引数を持たないメッセージパケットを構築し、*thisに登録されている
        this_type::receiver にだけ送信する。

        @copydetails interface_send_local_message

        @param[in] in_tag  送信するメッセージの荷札。
        @param[in] in_call 送信するメッセージの呼出状。
     */
    public: bool send_local_message(
        typename this_type::receiver::tag const& in_tag,
        typename this_type::receiver::call const& in_call)
    {
        typedef typename this_type::receiver::packet::suite suite;
        return this->send_local_message(
            typename this_type::receiver::packet::template zonal<suite>(
                suite(in_tag, in_call)));
    }

    /** @brief
        任意型の引数を持つメッセージパケットを構築し、*thisに登録されている
        this_type::receiver にだけ送信する。

        @copydetails send_local_message(typename this_type::receiver::tag const&, typename this_type::receiver::call const&)

        @param[in] in_parameter 送信するメッセージの引数。
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
            typename this_type::receiver::packet::template zonal<suite>(
                suite(in_tag, in_call, std::move(in_parameter))));
    }

    /** @brief *thisに登録されている this_type::receiver に、メッセージパケットを配信する。

        - this_type::get_thread_id() と合致しないスレッドで実行すると失敗する。
        - psyq::any::message::zone::flush() とこの関数を定期的に実行し、
          メッセージパケットを循環させること。

        @sa this_type::receiver を*thisに登録するには、
            this_type::register_receiver() を使う。

        @retval true  成功。メッセージパケットを配信した。
        @retval false 失敗。メッセージパケットを配信しなかった。
     */
    public: bool flush()
    {
        if (!this->agree_this_thread())
        {
            return false;
        }

        // 配信するメッセージパケットを取得する。
        std::lock_guard<psyq::spinlock> local_lock(this->lock_);
        this->delivery_packets_.swap(this->import_packets_);
        this_type::remove_empty_receiver(this->receiver_map_);

        // メッセージ受信器へメッセージを配信する。
        this_type::deliver_packet(
            this->receiver_map_, this->delivery_packets_);
        this_type::clear_packet_container(
            this->delivery_packets_, this->delivery_packets_.size());
        return true;
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name this_type のプロパティ
        @{
     */
    /** @brief *thisが使うメモリ割当子を取得する。
        @return *thisが使うメモリ割当子。
     */
    public: PSYQ_CONSTEXPR typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->receiver_map_.get_allocator();
    }

    /** @brief *thisに合致するスレッドの識別子を取得する。
        @return *thisに合致するスレッドの識別子。
     */
    public: PSYQ_CONSTEXPR std::thread::id const& get_thread_id()
    const PSYQ_NOEXCEPT
    {
        return this->thread_id_;
    }

    /** @brief *thisのメッセージ送信アドレスを取得する。
        @return *thisのメッセージ送信アドレス。
     */
    public: PSYQ_CONSTEXPR typename this_type::receiver::tag::key const
    get_message_address() const PSYQ_NOEXCEPT
    {
        return this->message_address_;
    }

    /** @brief *thisから送信するメッセージの荷札を構築する。
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
    /// @}
    //-------------------------------------------------------------------------
    /** @brief this_type を構築する。
        @param[in] in_thread_id       *thisに対応するスレッドの識別子。
        @param[in] in_message_address *thisのメッセージ送受信アドレス。
        @param[in] in_allocator       *thisが使うメモリ割当子の初期値。
     */
    private: transmitter(
        std::thread::id in_thread_id,
        typename this_type::receiver::tag::key const in_message_address,
        typename this_type::allocator_type const& in_allocator):
    receiver_map_(in_allocator),
    export_packets_(in_allocator),
    import_packets_(in_allocator),
    delivery_packets_(in_allocator),
    thread_id_(std::move(in_thread_id)),
    message_address_(in_message_address)
    {}

    /// @brief コピー構築子は使用禁止。
    private: transmitter(this_type const&);
    /// @brief コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    //-------------------------------------------------------------------------
    /** @brief this_type::receiver を辞書から検索する。
        @return 検索した位置。
        @param[in] in_receiver_map 検索範囲となる this_type::receiver の辞書。
        @param[in] in_receiver     検索する this_type::receiver 。
        @param[in] in_method       検索する this_type::receiver のメソッド種別。
     */
    private: static typename this_type::receiver_map::const_iterator
    find_receiver_iterator(
        typename this_type::receiver_map const& in_receiver_map,
        typename this_type::receiver const& in_receiver,
        typename this_type::receiver::call::key const in_method)
    PSYQ_NOEXCEPT
    {
        auto local_iterator(in_receiver_map.find(in_method));
        while (
            local_iterator != in_receiver_map.end()
            && local_iterator->first == in_method
            && local_iterator->second.lock().get() != &in_receiver)
        {
            ++local_iterator;
        }
        return local_iterator;
    }

    /** @brief 解放された this_type::receiver を辞書から削除する。
        @param[in,out] io_receivers this_type::receiver が削除される辞書。
     */
    private: static void remove_empty_receiver(
        typename this_type::receiver_map& io_receivers)
    {
        for (auto i(io_receivers.begin()); i != io_receivers.end();)
        {
            if (i->second.expired())
            {
                i = io_receivers.erase(i);
            }
            else
            {
                ++i;
            }
        }
    }

    //-------------------------------------------------------------------------
    /** @brief メッセージパケットの送信を予約する。

        @copydetails interface_post_message

        @retval true  成功。メッセージパケットを送信した。
        @retval false 失敗。メッセージパケットを送信しなかった。

        @param[in] in_packet 送信するメッセージパケット。
     */
    private: bool add_export_packet(
        typename this_type::receiver::packet::shared_ptr in_packet)
    {
        if (in_packet.get() == nullptr || !this->agree_this_thread())
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

        // パケットを輸出する。
        io_export_packets.reserve(
            io_export_packets.size() + this->export_packets_.size());
        for (auto& local_packet_holder: this->export_packets_)
        {
            io_export_packets.emplace_back(std::move(local_packet_holder));
        }
        this_type::clear_packet_container(
            this->export_packets_, this->export_packets_.size());

        // パケットを輸入する。
        this->import_packets_.insert(
            this->import_packets_.end(),
            in_import_packets.begin(),
            in_import_packets.end());
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
        @return
            生成したメッセージパケットの保持子。
            生成に失敗した場合は、保持子は空となる。
        @param[in,out] io_suite     パケットに設定するメッセージ一式。
        @param[in]     in_allocator 使用するメモリ割当子。
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
        @return
            生成したメッセージパケットの保持子。
            生成に失敗した場合は、保持子は空となる。
        @param[in,out] io_suite     パケットに設定するメッセージ一式。
        @param[in]     in_allocator 使用するメモリ割当子。
     */
    private: template<typename template_suite>
    static typename this_type::receiver::packet::shared_ptr create_zonal_packet(
        template_suite&& io_suite,
        typename this_type::allocator_type const& in_allocator)
    {
        typedef typename this_type::receiver::packet::template
            zonal<template_suite>
                zonal_packet;
        return this_type::create_packet<zonal_packet>(
            std::move(io_suite), in_allocator);
    }

    /** @brief メッセージパケットを生成する。
        @return
            生成したメッセージパケットの保持子。
            生成に失敗した場合は、保持子は空となる。
        @param[in,out] io_suite     パケットに設定するメッセージ一式。
        @param[in]     in_allocator 使用するメモリ割当子。
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
                if (io_packet != nullptr)
                {
                    auto local_deallocator(local_allocator);
                    local_deallocator.destroy(io_packet);
                    local_deallocator.deallocate(io_packet, 1);
                }
            },
            local_allocator);
    }

    /** @brief this_type::receiver へメッセージパケットを配信する。
        @param[in] in_receivers this_type::receiver の辞書。
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

    /** @brief this_type::receiver へメッセージパケットを配信する。
        @param[in] in_receivers this_type::receiver の辞書。
        @param[in] in_packet    配信するメッセージパケット。
     */
    private: static void deliver_packet(
        typename this_type::receiver_map const& in_receivers,
        typename this_type::receiver::packet const& in_packet)
    {
        // this_type::receiver の辞書のうち、
        // メソッド種別が一致するものに、メッセージパケットを配信する。
        auto& local_suite(in_packet.get_suite());
        auto const local_method(local_suite.get_call().get_method());
        for (
            auto i(in_receivers.find(local_method));
            i != in_receivers.end() && i->first == local_method;
            ++i)
        {
            // this_type::receiver と合致するメッセージ受信アドレスか判定する。
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

    /** @brief 現在のスレッドが処理が許可されているスレッドか判定する。
        @retval true  現在のスレッドは、処理が許可されている。
        @retval false 現在のスレッドは、処理が許可されてない。
     */
    private: bool agree_this_thread() const PSYQ_NOEXCEPT
    {
        bool const local_agree(
            std::this_thread::get_id() == this->get_thread_id());
        PSYQ_ASSERT(local_agree);
        return local_agree;
    }

    //-------------------------------------------------------------------------
    /// メッセージパケットを配信する this_type::receiver の辞書。
    private: typename this_type::receiver_map receiver_map_;
    /// 外部から輸入したメッセージパケットのコンテナ。
    private: typename this_type::shared_packet_container import_packets_;
    /// 外部へ輸出するメッセージパケットのコンテナ。
    private: typename this_type::shared_packet_container export_packets_;
    /// this_type::receiver へ配信するメッセージパケット。
    private: typename this_type::shared_packet_container delivery_packets_;
    /// *thisに合致するスレッドの識別子。
    private: std::thread::id const thread_id_;
    /// 排他的処理に使うロックオブジェクト。
    private: psyq::spinlock lock_;
    /// *thisのメッセージ送受信アドレス。
    private: typename this_type::receiver::tag::key const message_address_;

}; // class psyq::any::message::transmitter

#endif // !defined(PSYQ_ANY_MESSAGE_TRANSMITTER_HPP_)
// vim: set expandtab:
