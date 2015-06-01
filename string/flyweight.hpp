/*
Copyright (c) 2013, Hillco Psychi, All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 
ソースコード形式かバイナリ形式か、変更するかしないかを問わず、
以下の条件を満たす場合に限り、再頒布および使用が許可されます。

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer. 
   ソースコードを再頒布する場合、上記の著作権表示、本条件一覧、
   および下記の免責条項を含めること。
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 
   バイナリ形式で再頒布する場合、頒布物に付属のドキュメント等の資料に、
   上記の著作権表示、本条件一覧、および下記の免責条項を含めること。

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
本ソフトウェアは、著作権者およびコントリビューターによって
「現状のまま」提供されており、明示黙示を問わず、商業的な使用可能性、
および特定の目的に対する適合性に関する暗黙の保証も含め、
またそれに限定されない、いかなる保証もありません。
著作権者もコントリビューターも、事由のいかんを問わず、
損害発生の原因いかんを問わず、かつ責任の根拠が契約であるか厳格責任であるか
（過失その他の）不法行為であるかを問わず、
仮にそのような損害が発生する可能性を知らされていたとしても、
本ソフトウェアの使用によって発生した（代替品または代用サービスの調達、
使用の喪失、データの喪失、利益の喪失、業務の中断も含め、
またそれに限定されない）直接損害、間接損害、偶発的な損害、特別損害、
懲罰的損害、または結果損害について、一切責任を負わないものとします。
 */
/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_STRING_FLYWEIGHT_HPP_
#define PSYQ_STRING_FLYWEIGHT_HPP_

//#include "../atomic_count.hpp"
//#include "string/view.hpp"

#ifndef PSYQ_STRING_FLYWEIGHT_FACTORY_CAPACITY_DEFAULT
#define PSYQ_STRING_FLYWEIGHT_FACTORY_CAPACITY_DEFAULT 256
#endif // !defined(PSYQ_STRING_FLYWEIGHT_FACTORY_CAPACITY_DEFAULT)

#ifndef PSYQ_STRING_FLYWEIGHT_CHUNK_SIZE_DEFAULT
#define PSYQ_STRING_FLYWEIGHT_CHUNK_SIZE_DEFAULT 4096
#endif // !defined(PSYQ_STRING_FLYWEIGHT_CHUNK_SIZE_DEFAULT)

/// @cond
namespace psyq
{
    namespace string
    {
        template<
            typename template_char_type,
            typename = PSYQ_STRING_VIEW_TRAITS_DEFAULT,
            typename = std::allocator<void*>>
                class flyweight_view;
        template<
            typename template_char_type,
            typename = PSYQ_STRING_VIEW_TRAITS_DEFAULT,
            typename = std::allocator<void*>>
                class flyweight_factory;
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief flyweightパターンを使った文字列の、factory型。
    @tparam template_char_type      @copydoc psyq::string::view::value_type
    @tparam template_char_traits    @copydoc psyq::string::view::traits_type
    @tparam template_allocator_type @copydoc flyweight_factory::allocator_type
 */
template<
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
class psyq::string::flyweight_factory
{
    /// thisが指す値の型。
    private: typedef flyweight_factory this_type;

    /// メモリ割当子の型。
    public: typedef template_allocator_type allocator_type;

    public: typedef std::shared_ptr<this_type> shared_ptr;
    public: typedef std::weak_ptr<this_type> weak_ptr;

    /// 文字列のハッシュ計算機。
    public: typedef psyq::fnv1_hash32 hash;

    public: typedef psyq::string::view<
        template_char_type, template_char_traits>
            view;

    /// @brief 文字列項目。
    private: struct item
    {
        item(
            typename flyweight_factory::view::size_type const in_size,
            typename flyweight_factory::hash::value_type const in_hash)
        PSYQ_NOEXCEPT:
        reference_count(0),
        size(in_size),
        hash(in_hash)
        {}

        /** @brief 文字列の先頭位置を取得する。
            @return 文字列の先頭位置。
         */
        typename flyweight_factory::view::const_pointer data()
        const PSYQ_NOEXCEPT
        {
            return reinterpret_cast<
                typename flyweight_factory::view::const_pointer>(this + 1);
        }

        /// @brief 文字列の被参照数。
        psyq::atomic_count reference_count;
        /// @brief 文字列の要素数。
        typename flyweight_factory::view::size_type size;
        /// @brief 文字列のハッシュ値。
        typename flyweight_factory::hash::value_type hash;

    }; // struct item

    private: typedef std::vector<
        typename this_type::item*,
        typename this_type::allocator_type::template
            rebind<typename this_type::item*>::other>
                item_vector;

    /// @brief 文字列を比較する関数オブジェクト。
    private: struct compare_item
    {
        bool operator()(
            typename flyweight_factory::item const* const in_left,
            typename flyweight_factory::item const* const in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left->hash != in_right->hash?
                in_left->hash < in_right->hash: in_left->size < in_right->size;
        }

        bool operator()(
            typename flyweight_factory::item const* const in_left,
            typename flyweight_factory::item const& in_right)
        const PSYQ_NOEXCEPT
        {
            return this->operator()(in_left, &in_right);
        }

        bool operator()(
            typename flyweight_factory::item const& in_left,
            typename flyweight_factory::item const* const in_right)
        const PSYQ_NOEXCEPT
        {
            return this->operator()(&in_left, in_right);
        }

    }; // struct compare_item

    /// @brief 文字列チャンク。
    private: struct chunk_header
    {
        chunk_header(
            chunk_header* const in_next_chunk,
            std::size_t const in_capacity)
        PSYQ_NOEXCEPT:
        next_chunk(in_next_chunk),
        capacity((
            PSYQ_ASSERT(sizeof(chunk_header) < in_capacity), in_capacity)),
        first_item(
            (in_capacity - sizeof(chunk_header)) /
                sizeof(typename flyweight_factory::view::value_type),
            flyweight_factory::hash::traits_type::EMPTY)
        {}

        /// @brief 次の文字列チャンクを指すポインタ。
        typename flyweight_factory::chunk_header* next_chunk;
        /// @brief チャンクの大きさ。
        std::size_t capacity;
        /// @brief この文字列チャンクでの最初の文字列。
        typename flyweight_factory::item first_item;

    }; // struct chunk_header

    /** @brief 文字列参照の基底型。
        @note psyq::string::flyweight_factory 管理者以外は、使用禁止。
     */
    public: class _private_view;

    //-------------------------------------------------------------------------
    public: explicit flyweight_factory(
        std::size_t const in_reserved_items =
            PSYQ_STRING_FLYWEIGHT_FACTORY_CAPACITY_DEFAULT,
        typename this_type::allocator_type const& in_allocator =
            allocator_type())
    :
    items_(in_allocator),
    chunk_(nullptr)
    {
        this->items_.reserve(in_reserved_items);
    }

    /// @brief コピー構築子は使用禁止。
    private: flyweight_factory(this_type const&);
    /// @brief コピー演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    public: ~flyweight_factory()
    {
        // 文字列項目をすべて解体する。
        for (auto i(this->items_.begin()); i != this->items_.end(); ++i)
        {
            PSYQ_ASSERT(*i != nullptr);
            (**i).~item();
        }
        // 文字列チャンクをすべて解体する。
        typename this_type::item_vector::allocator_type::template
            rebind<typename this_type::chunk_header>::other
                local_allocator(this->items_.get_allocator());
        auto local_chunk(this->chunk_);
        while (local_chunk != nullptr)
        {
            auto const local_last_chunk(local_chunk);
            auto const local_chunk_count(
                local_chunk->capacity
                / sizeof(typename this_type::chunk_header));
            local_chunk = local_chunk->next_chunk;
            local_last_chunk->~chunk_header();
            local_allocator.deallocate(local_last_chunk, local_chunk_count);
        }
    }

    /** @brief 文字列のハッシュ値を算出する。
        @param[in] in_string ハッシュ値を算出する文字列。
        @return 文字列のハッシュ値。
     */
    public: static typename this_type::hash::value_type compute_hash(
        typename this_type::view const& in_string)
    PSYQ_NOEXCEPT
    {
        auto const local_data(in_string.data());
        auto const local_size(in_string.size());
        return this_type::hash::compute(local_data, local_data + local_size);
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列を辞書に登録する。

        登録する文字列と等価な文字列がすでに存在するなら、
        登録済の文字列を再利用する。

        @param[in] in_string     辞書に登録されている文字列。
        @param[in] in_chunk_size チャンクを用意する場合の、デフォルトのチャンク容量。
     */
    private: typename this_type::item& equip_item(
        typename this_type::view const& in_string,
        std::size_t const in_chunk_size)
    {
        PSYQ_ASSERT(!in_string.empty());

        // in_string と等価な文字列を、辞書から検索する。
        auto const local_hash(this_type::compute_hash(in_string));
        auto const local_size(in_string.size());
        for (
            auto i(
                std::lower_bound(
                    this->items_.begin(),
                    this->items_.end(),
                    typename this_type::item(local_size, local_hash),
                    typename this_type::compare_item()));
            i != this->items_.end();
            ++i)
        {
            auto& local_existing_item(**i);
            if (local_hash != local_existing_item.hash)
            {
                break;
            }
            typename this_type::view const local_string(
                local_existing_item.data(), local_existing_item.size);
            if (in_string == local_string)
            {
                // 登録済の文字列を再利用する。
                return local_existing_item;
            }
        }

        // 等価な文字列が辞書になかったので、新たな文字列を登録する。
        auto& local_item(
            this->equip_empty_item(local_size + 1, in_chunk_size));
        local_item.size = local_size;
        local_item.hash = local_hash;
        // 文字列をコピーする。
        auto const local_data(
            const_cast<typename this_type::view::value_type*>(
                local_item.data()));
        this_type::view::traits_type::copy(
            local_data, in_string.data(), local_size);
        local_data[local_size] = 0;
        // 辞書をソートする。
        std::sort(
            this->items_.begin(),
            this->items_.end(),
            typename this_type::compare_item());
        return local_item;
    }

    /** @brief 空の文字列項目を用意する。
        @param[in] in_string_size 文字列の要素数。
        @param[in] in_chunk_size  チャンクを用意する場合の、デフォルトのチャンク容量。
        @retval !=nullptr 文字列項目に使う領域の先頭位置。
     */
    private: typename this_type::item& equip_empty_item(
        typename this_type::view::size_type const in_string_size,
        std::size_t const in_chunk_size)
    {
        // in_string_size が収まる空文字列を検索する。
        auto const local_necessary_size(
            this_type::align_string_size(in_string_size));
        auto const local_just_position(
            std::lower_bound(
                this->items_.begin(),
                this->items_.end(),
                typename this_type::item(
                    local_necessary_size, this_type::hash::traits_type::EMPTY),
                typename this_type::compare_item()));
        if (local_just_position != this->items_.end())
        {
            auto& local_just_item(**local_just_position);
            if (local_just_item.hash == this_type::hash::traits_type::EMPTY
                && local_just_item.reference_count.load() == 0)
            {
                if (local_just_item.size == local_necessary_size)
                {
                    return local_just_item;
                }

                // 分割できる空文字列を探して、分割する。
                auto const local_empty_end(
                    std::lower_bound(
                        local_just_position,
                        this->items_.end(),
                        typename this_type::item(
                            0, this_type::hash::traits_type::EMPTY + 1),
                        typename this_type::compare_item()));
                auto& local_empty_item(**(local_empty_end - 1));
                PSYQ_ASSERT(
                    local_empty_item.hash
                    == this_type::hash::traits_type::EMPTY);
                auto const local_divide_item(
                    this->divide_item(local_empty_item, local_necessary_size));
                if (local_divide_item != nullptr)
                {
                    return local_empty_item;
                }
            }
        }

        // 分割できる空文字列がなかったので、新たにチャンクを用意する。
        this->allocate_chunk(in_string_size, in_chunk_size);
        this->divide_item(this->chunk_->first_item, local_necessary_size);
        return this->chunk_->first_item;
    }

    /** 文字列を2つに分割し、後方を空文字列として辞書に追加する。
        @param[in] in_item 分割する文字列。
        @param[in] in_size 前方の文字列の要素数。
        @retval !=nullptr 分割した後方の空文字列。
        @retval ==nullptr 失敗。分割できなかった。
     */
    private: typename this_type::item* divide_item(
        typename this_type::item const& in_item,
        typename this_type::view::size_type const in_size)
    {
        static_assert(
            0 == sizeof(typename this_type::item)
                % sizeof(typename this_type::view::value_type),
            "");
        auto const local_divide_size(
            in_size + sizeof(typename this_type::item)
                / sizeof(typename this_type::view::value_type));
        if (in_item.size < local_divide_size)
        {
            return nullptr;
        }
        auto const local_empty_block(
            const_cast<typename this_type::view::value_type*>(
                in_item.data() + in_size));
        auto const local_empty_item(
            new(local_empty_block) typename this_type::item(
                in_item.size - local_divide_size,
                this_type::hash::traits_type::EMPTY));
        this->items_.push_back(local_empty_item);
        return local_empty_item;
    }

    /** @brief 文字列を保存する文字列チャンクを用意する。
        @param[in] in_string_size 保存する文字列の要素数。
        @param[in] in_chunk_size  デフォルトのチャンク容量。
     */
    private: void allocate_chunk(
        typename this_type::view::size_type const in_string_size,
        std::size_t const in_chunk_size)
    {
        // チャンクの容量を決定する。
        auto const local_chunk_size(
            (std::max)(
                in_string_size * sizeof(typename this_type::view::value_type)
                + sizeof(typename this_type::chunk_header),
                in_chunk_size));
        auto const local_chunk_capacity(
            (local_chunk_size + sizeof(typename this_type::chunk_header) - 1)
            / sizeof(typename this_type::chunk_header));

        // チャンクを構築する。
        typename this_type::item_vector::allocator_type::template
            rebind<typename this_type::chunk_header>::other
                local_allocator(this->items_.get_allocator());
        this->chunk_ = new(local_allocator.allocate(local_chunk_capacity))
            typename this_type::chunk_header(
                this->chunk_,
                local_chunk_capacity *
                    sizeof(typename this_type::chunk_header));
        PSYQ_ASSERT(this->chunk_ != nullptr);
        this->items_.push_back(&this->chunk_->first_item);
    }

    /** @brief this_type::item のメモリ境界に接するように、文字列の要素数を調整する。
        @param[in] in_string_size 調整前の文字列の要素数。
        @return 調整後の文字列の要素数。
     */
    private: static typename this_type::view::size_type align_string_size(
        typename this_type::view::size_type const in_string_size)
    PSYQ_NOEXCEPT
    {
        auto const local_size(sizeof(typename this_type::view::value_type));
        auto const local_align(
#ifdef _MSC_VER
            __alignof(typename this_type::item)
#else
            alignof(typename this_type::item)
#endif // defined(_MSC_VER)
            );
        auto const local_count(
            (in_string_size * local_size + local_align - 1) / local_align);
        return (local_count * local_align + local_size - 1) / local_size;
    }

    //-------------------------------------------------------------------------
    /// @brief ハッシュ値をキーにソートされている、文字列項目の辞書。
    private: typename this_type::item_vector items_;
    /// @brief 先頭の文字列チャンク。
    private: typename this_type::chunk_header* chunk_;

}; // class psyq::string::flyweight_factory

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief flyweightパターンを使った文字列の、client型の基底型。
    @tparam template_char_type      @copydoc psyq::string::view::value_type
    @tparam template_char_traits    @copydoc psyq::string::view::traits_type
    @tparam template_allocator_type @copydoc psyq::string::flyweight_factory::allocator_type
 */
template<
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
class psyq::string::flyweight_factory<
    template_char_type, template_char_traits, template_allocator_type>
        ::_private_view
{
    /// @brief thisが指す値の型。
    private: typedef _private_view this_type;

    public: typedef template_char_traits traits_type; ///< 文字特性の型。

    public: typedef psyq::string::flyweight_factory<
        template_char_type, template_char_traits, template_allocator_type>
            factory;

    //-------------------------------------------------------------------------
    private: _private_view() PSYQ_NOEXCEPT: item_(nullptr) {}

    protected: _private_view(this_type const& in_source)
    PSYQ_NOEXCEPT: factory_(in_source.factory_)
    {
        if (this->factory_.get() != nullptr)
        {
            PSYQ_ASSERT(in_source.item_ != nullptr);
            in_source.item_->reference_count.add(1);
            this->item_ = in_source.item_;
        }
        else
        {
            this->item_ = nullptr;
        }
    }

    protected: _private_view(this_type&& io_source)
    PSYQ_NOEXCEPT:
    factory_(std::move(io_source.factory_)),
    item_(std::move(io_source.item_))
    {
        io_source.factory_.reset();
        io_source.item_ = nullptr;
    }

    private: _private_view(
        typename this_type::factory::shared_ptr const& in_factory,
        typename this_type::factory::item& in_item)
    PSYQ_NOEXCEPT: factory_(in_factory)
    {
        if (this->factory_.get() != nullptr)
        {
            in_item.reference_count.add(1);
            this->item_ = &in_item;
        }
        else
        {
            this->item_ = nullptr;
        }
    }

    public: ~_private_view()
    {
        if (this->factory_.get() != nullptr)
        {
            PSYQ_ASSERT(this->item_ != nullptr);
            this->item_->reference_count.sub(1);
        }
    }

    public: this_type& operator=(this_type const& in_source)
    {
        if (this->item_ != in_source.item_)
        {
            if (this->factory_.get() != nullptr)
            {
                PSYQ_ASSERT(this->item_ != nullptr);
                this->item_->reference_count.sub(1);
            }
            this->factory_ = in_source.factory_;
            if (in_source.factory_.get() != nullptr)
            {
                PSYQ_ASSERT(in_source.item_ != nullptr);
                in_source.item_->reference_count.add(1);
                this->item_ = in_source.item_;
            }
            else
            {
                this->item_ = nullptr;
            }
        }
        return *this;
    }

    public: this_type& operator=(this_type&& io_source)
    {
        if (this->item_ != io_source.item_)
        {
            if (this->factory_.get() != nullptr)
            {
                PSYQ_ASSERT(this->item_ != nullptr);
                this->item_->reference_count.sub(1);
            }
            this->factory_ = std::move(io_source.factory_);
            this->item_ = std::move(io_source.item_);
            io_source.factory_.reset();
            io_source.item_ = nullptr;
        }
        return *this;
    }

    protected: static this_type make() PSYQ_NOEXCEPT
    {
        return this_type();
    }

    protected: static this_type make(
        typename this_type::factory::shared_ptr const& in_factory,
        typename this_type::factory::view const& in_string,
        std::size_t const in_chunk_size)
    PSYQ_NOEXCEPT
    {
        auto const local_factory(in_factory.get());
        return local_factory == nullptr || in_string.empty()?
            this_type():
            this_type(
                in_factory,
                local_factory->equip_item(in_string, in_chunk_size));
    }

    /// @copydoc psyq::string::view::clear
    public: void clear() PSYQ_NOEXCEPT
    {
        if (this->factory_.get() != nullptr)
        {
            PSYQ_ASSERT(this->item_ != nullptr);
            this->item_->reference_count.sub(1);
            this->factory_.reset();
        }
        this->item_ = nullptr;
    }

    /// @copydoc psyq::string::view::data
    public: typename this_type::traits_type::char_type const* data()
    const PSYQ_NOEXCEPT
    {
        return this->factory_.get() != nullptr? 
            (PSYQ_ASSERT(this->item_ != nullptr), this->item_->data()):
            nullptr;
    }

    /// @copydoc psyq::string::view::size
    public: typename this_type::factory::view::size_type size()
    const PSYQ_NOEXCEPT
    {
        return this->factory_.get() != nullptr?
            (PSYQ_ASSERT(this->item_ != nullptr), this->item_->size): 0;
    }

    /// @copydoc psyq::string::view::max_size
    public: std::size_t max_size() const PSYQ_NOEXCEPT
    {
        return (std::numeric_limits<std::size_t>::max)();
    }

    /** @brief 文字列のハッシュ値を取得する。
        @return flyweight文字列辞書で使われているハッシュ値。
     */
    public: typename this_type::factory::hash::value_type get_hash()
    const PSYQ_NOEXCEPT
    {
        return this->factory_.get() != nullptr?
            (PSYQ_ASSERT(this->item_ != nullptr), this->item_->hash):
            this_type::factory::hash::traits_type::EMPTY;
    }

    /** @brief 文字列が所属するflyweight文字列辞書を取得する。
        @return 文字列が所属するflyweight文字列辞書。
     */
    public: typename this_type::factory::shared_ptr const& get_factory()
    const PSYQ_NOEXCEPT
    {
        return this->factory_;
    }

    //-------------------------------------------------------------------------
    /// 文字列を所有するflyweight文字列辞書。
    private: typename this_type::factory::shared_ptr factory_;
    /// flyweight文字列。
    private: typename this_type::factory::item* item_;

}; // class psyq::string::flyweight_factory::_private_view

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief flyweightパターンを使った文字列の、client型。
    @tparam template_char_type      @copydoc psyq::string::view::value_type
    @tparam template_char_traits    @copydoc psyq::string::view::traits_type
    @tparam template_allocator_type @copydoc psyq::string::flyweight_factory::allocator_type
 */
template<
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
class psyq::string::flyweight_view:
public psyq::string::_private::interface_immutable<
    typename psyq::string::flyweight_factory<
        template_char_type,
        template_char_traits,
        template_allocator_type>
            ::_private_view>
{
    /// thisが指す値の型。
    private: typedef flyweight_view this_type;

    /// this_type の基底型。
    public: typedef psyq::string::_private::interface_immutable<
        typename psyq::string::flyweight_factory<
            template_char_type,
            template_char_traits,
            template_allocator_type>
                ::_private_view>
                    base_type;

    /** @brief 空の文字列を構築する。メモリ確保は行わない。
     */
    public: flyweight_view()
    PSYQ_NOEXCEPT: base_type(base_type::base_type::make())
    {}

    /** @brief 文字列をコピー構築する。メモリ確保は行わない。
        @param[in] in_source コピー元となる文字列。
     */
    public: flyweight_view(this_type const& in_source)
    PSYQ_NOEXCEPT: base_type(in_source)
    {}

    /** @brief 文字列をムーブ構築する。メモリ確保は行わない。
        @param[in,out] io_source ムーブ元となる文字列。
     */
    public: flyweight_view(this_type && io_source)
    PSYQ_NOEXCEPT: base_type(std::move(io_source))
    {}

    /** @brief 文字列をコピー構築する。メモリ確保を行う場合ある。
        @param[in] in_factory    保持する文字列工場。
        @param[in] in_string     コピー元となる文字列。
        @param[in] in_chunk_size チャンクを用意する場合の、デフォルトのチャンク容量。
     */
    public: flyweight_view(
        typename base_type::factory::view const& in_string,
        typename base_type::factory::shared_ptr const& in_factory,
        std::size_t const in_chunk_size =
            PSYQ_STRING_FLYWEIGHT_CHUNK_SIZE_DEFAULT)
    PSYQ_NOEXCEPT:
    base_type(base_type::base_type::make(in_factory, in_string, in_chunk_size))
    {}

    /** @brief 文字列をコピー代入する。メモリ確保は行わない。
        @param[in] in_source コピー元となる文字列。
        @return *this
     */
    public: this_type& operator=(this_type const& in_source)
    {
        this->base_type::base_type::operator=(in_source);
        return *this;
    }

    /** @brief 文字列をムーブ代入する。メモリ確保は行わない。
        @param[in,out] io_source ムーブ元となる文字列。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->base_type::base_type::operator=(std::move(io_source));
        return *this;
    }

}; // class psyq::string::flyweight_view

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    inline void string_flyweight()
    {
        typedef psyq::string::flyweight_view<char> flyweight_string;
        flyweight_string::factory::shared_ptr const local_factory(
            new flyweight_string::factory);
        flyweight_string local_string("stringstringstring", local_factory, 0);
        local_string.size();
        local_string = flyweight_string("abcdefg", local_factory);
        local_string = flyweight_string("string", local_factory);
    }
}

#endif // !PSYQ_STRING_FLYWEIGHT_HPP_
// vim: set expandtab:
