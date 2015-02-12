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
    @brief @copybrief psyq::string::flyweight
 */
#ifndef PSYQ_STRING_FLYWEIGHT_HPP_
#define PSYQ_STRING_FLYWEIGHT_HPP_

//#include "string/view.hpp"

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

    private: enum: std::size_t
    {
        CHUNK_SIZE = 4096,
    };

    private: struct item
    {
        PSYQ_CONSTEXPR item(
            typename flyweight_factory::hash::value_type const in_hash,
            typename flyweight_factory::view::size_type const in_size)
        PSYQ_NOEXCEPT:
        hash(in_hash),
        size(in_size)
        {}

        typename PSYQ_CONSTEXPR flyweight_factory::view::const_pointer data()
        const PSYQ_NOEXCEPT
        {
            return reinterpret_cast<
                typename flyweight_factory::view::const_pointer>(this + 1);
        }

        /// 文字列のハッシュ値。
        typename flyweight_factory::hash::value_type hash;
        /// 文字列の要素数。
        typename flyweight_factory::view::size_type size;

    }; // struct item

    private: struct compare_item
    {
        PSYQ_CONSTEXPR bool operator()(
            typename flyweight_factory::item const* const in_left,
            typename flyweight_factory::item const* const in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left->hash != in_right->hash?
                in_left->hash < in_right->hash: in_left->size < in_right->size;
        }

        PSYQ_CONSTEXPR bool operator()(
            typename flyweight_factory::item const* const in_left,
            typename flyweight_factory::item const& in_right)
        const PSYQ_NOEXCEPT
        {
            return this->operator()(in_left, &in_right);
        }

        PSYQ_CONSTEXPR bool operator()(
            typename flyweight_factory::item const& in_left,
            typename flyweight_factory::item const* const in_right)
        const PSYQ_NOEXCEPT
        {
            return this->operator()(&in_left, in_right);
        }

    }; // struct compare_item

    private: struct chunk_header
    {
        typename flyweight_factory::chunk_header* next_chunk;
        typename flyweight_factory::item first_item;
    }; // struct chunk_header

    private: typedef std::vector<
        typename this_type::item*,
        typename this_type::allocator_type::template
            rebind<typename this_type::item*>::other>
                item_array;

    /** @brief 文字列参照の基底型。
        @note psyq::string::flyweight_factory 管理者以外は、使用禁止。
     */
    public: class _private_view;

    //-------------------------------------------------------------------------
    public: explicit flyweight_factory(
        typename this_type::allocator_type const& in_allocator)
    :
    items_(in_allocator),
    chunk_(nullptr)
    {}

    private: flyweight_factory(this_type const&);

    public: ~flyweight_factory()
    {
        // 文字列項目をすべて解体する。
        for (auto i(this->items_.begin()); i != this->items_.end(); ++i)
        {
            PSYQ_ASSERT(*i != nullptr);
            (**i).~item();
        }
        // チャンクをすべて破棄する。
        typename this_type::item_array::allocator_type::template
            rebind<typename this_type::chunk_header>::other
                local_allocator(this->items_.get_allocator());
        auto local_chunk(this->chunk_);
        while (local_chunk != nullptr)
        {
            auto const local_last_chunk(local_chunk);
            local_chunk = local_chunk->next_chunk;
            local_allocator.deallocate(
                local_last_chunk,
                this_type::compute_chunk_header_count(
                    local_last_chunk->first_item.size + 1));
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
        auto const local_hash(
            this_type::hash::compute(local_data, local_data + local_size));
        return local_hash == this_type::hash::EMPTY && 0 < local_size?
            this_type::EMPTY + 1: local_hash;
    }

    /** @brief 文字列を辞書に登録する。

        登録する文字列と等価な文字列がすでに存在するなら、
        登録済の文字列を再利用する。

        @param[in] in_string 辞書に登録されている文字列。
     */
    private: typename this_type::item const& equip_item(
        typename this_type::view const& in_string)
    {
        PSYQ_ASSERT(!in_string.empty());

        // in_string と等価な文字列を、辞書から検索する。
        auto const local_hash(this_type::compute_hash(in_string));
        auto const local_size(in_string.size());
        auto const local_existing_position(
            std::lower_bound(
                this->items_.begin(),
                this->items_.end(),
                typename this_type::item(local_hash, local_size)
                typename this_type::compare_item()));
        if (local_existing_position != this->items_.end())
        {
            auto const& local_existing_item(**local_existing_position);
            if (local_hash == local_existing_item.hash)
            {
                typename this_type::view const local_string(
                    local_existing_item.data(), local_existing_item.size);
                if (in_string == local_string)
                {
                    // 登録済の文字列を再利用する。
                    return local_existing_item;
                }
            }
        }

        // 等価な文字列が辞書になかったので、新たに登録する。
        auto const& local_new_item(
            *new(this->allocate_item(local_size + 1))
                typename this_type::item(local_hash, local_size));
        // 文字列をコピーする。
        auto const local_new_data(
            const_cast<typename this_type::view::pointer>(
                local_new_item.data()));
        this_type::view::traits_type::copy(
            local_new_data, in_string.data(), local_size);
        local_new_data[local_size] = 0;
        // 辞書をソートする。
        std::sort(
            this->items_.begin(),
            this->items_.end(),
            typename this_type::compare_item());
        return local_new_item;
    }

    private: void* allocate_item(
        typename this_type::view::size_type const in_string_size)
    {
        // in_string_size が収まる空文字列を検索する。
        auto const local_necessary_size(
            this_type::align_string_size(in_string_size));
        auto const local_just_position(
            std::lower_bound(
                this->items_.begin(),
                this->items_.end(),
                typename this_type::item(
                    this_type::hash::EMPTY, local_necessary_size),
                typename this_type::compare_item()));
        if (local_just_position == this->items.end())
        {
            // 空文字列がなかったので、新たにチャンクを用意する。
            return this->allocate_chunk(in_string_size);
        }
        auto& local_just_item(**local_just_position);
        if (local_just_item.hash != this_type::hash::EMPTY)
        {
            // 空文字列がなかったので、新たにチャンクを用意する。
            return this->allocate_chunk(in_string_size);
        }
        else if (local_just_item.size == local_necessary_size)
        {
            return &local_just_item;
        }

        // 分割可能な空文字列を検索する。
        auto const local_distribute_position(
            std::lower_bound(
                local_just_position,
                this->items_.end(),
                typename this_type::item(this_type::hash::EMPTY + 1, 0),
                typename this_type::compare_item()));
        auto& local_distribute_item(**(local_distribute_position - 1));
        PSYQ_ASSERT(local_distribute_item.hash == this_type::hash::EMPTY);
        static_assert(
            0 == sizeof(typename this_type::item)
                % sizeof(typename this_type::view::value_type));
        auto const local_distribute_size(
            local_necessary_size + sizeof(typename this_type::item) 
                / sizeof(typename this_type::view::value_type));
        if (local_distribute_item.size < local_distribute_size)
        {
            // 分割可能な空文字列がなかったので、新たにチャンクを用意する。
            return this->allocate_chunk(in_string_size);
        }
        // 空文字列を2つに分割し、後方を空文字列として辞書に追加する。
        auto const local_empty_block(
            const_cast<typename this_type::view::pointer>(
                local_distribute_item.data() + local_necessary_size));
        this->items_.push_back(
            new(local_empty_block) typename this_type::item(
                this_type::hash::EMPTY,
                local_distribute_item.size - local_distribute_size));
        return &local_distribute_item;
    }

    private: void* allocate_chunk(
        typename this_type::view::size_type const in_string_size)
    {
        typename this_type::item_array::allocator_type::template
            rebind<typename this_type::chunk_header>::other
                local_allocator(this->items_.get_allocator());
        auto const local_allocate_count(
            this_type::compute_chunk_header_count(in_string_size));
        auto const local_chunk(
            new(local_allocator.allocate(local_allocate_count))
                typename this_type::chunk_header());
        PSYQ_ASSERT(local_chunk != nullptr);
        this->items_.push_back(
            new() typename this_type::item(this_type::hash::EMPTY, ));
        return &local_chunk->first_item;
    }

    /** @brief this_type::item のメモリ境界に接するように、文字列の要素数を調整する。
        @param[in] in_string_size 調整前の文字列の要素数。
        @return 調整後の文字列の要素数。
     */
    private: static PSYQ_CONSTEXPR std::size_t align_string_size(
        std::size_t const in_string_size)
    PSYQ_NOEXCEPT
    {
        auto const local_size(sizeof(typename this_type::view::value_type));
        auto const local_align(alignof(typename this_type::item));
        auto const local_count(
            (in_string_size * local_size + local_align - 1) / local_align);
        return (local_count * local_align + local_size - 1) / local_size;
    }

    private: static PSYQ_CONSTEXPR std::size_t compute_chunk_header_count(
         typename this_type::view::size_type const in_string_size)
    PSYQ_NOEXCEPT
    {
        auto const local_header_size(sizeof(typename this_type::chunk_header));
        auto const local_chunk_size(
            (std::max)(
                this_type::CHUNK_SIZE,
                in_string_size * sizeof(typename this_type::view::value_type)
                + local_header_size));
        return (local_chunk_size + local_header_size - 1) / local_header_size;
    }

    //-------------------------------------------------------------------------
    /// ハッシュ値をキーにソートされている、文字列の辞書。
    private: typename this_type::item_array items_;
    /// 先頭のチャンク。
    private: typename this_type::chunk_header* chunk_;

}; // class psyq::string::flyweight_factory

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
class psyq::string::flyweight_factory<
    template_char_type, template_char_traits, template_allocator_type>
        ::_private_view
{
    /// thisが指す値の型。
    private: typedef _private_view this_type;

    public: typedef template_char_traits traits_type; ///< 文字特性の型。

    public: typedef psyq::string::flyweight_factory<
        template_char_type, template_char_traits, template_allocator_type>
            factory;

    //-------------------------------------------------------------------------
    protected: _private_view(this_type const& in_source) PSYQ_NOEXCEPT:
    factory_(io_source.factory_),
    item_(io_source.item_)
    {}

    protected: _private_view(this_type&& io_source) PSYQ_NOEXCEPT:
    factory_(std::move(io_source.factory_)),
    item_(std::move(io_source.item_))
    {
        io_source.clear();
    }

    private: _private_view(
        typename this_type::factory::weak_ptr in_factory,
        typename this_type::factory::item* const in_item)
    PSYQ_NOEXCEPT:
    factory_(std::move(in_factory)),
    item_(in_item)
    {}

    public: ~_private_view()
    {
        if (!this->get_factory().expired())
        {
            PSYQ_ASSERT(this->item_ != nullptr);
            /// @todo 文字列の参照数を更新すること。
            //this->item_->reference_count.sub(1);
        }
    }

    protected: static this_type make() PSYQ_NOEXCEPT
    {
        return this_type(typename this_type::factory::weak_ptr(), nullptr);
    }

    protected: static this_type make(
        typename this_type::factory::view const& in_string,
        typename this_type::factory::shared_ptr const& in_factory)
    PSYQ_NOEXCEPT
    {
        auto const local_factory(in_factory.get());
        return local_factory == nullptr || in_string.empty()?
            this_type::make():
            this_type(in_factory, &local_factory->equip_item(in_string));
    }

    /// @copydoc psyq::string::view::clear
    public: void clear() PSYQ_NOEXCEPT
    {
        this->factory_.reset();
        this->item_ = nullptr;
    }

    /// @copydoc psyq::string::view::data
    public: PSYQ_CONSTEXPR typename this_type::traits_type::char_type const*
    data() const PSYQ_NOEXCEPT
    {
        return this->factory_.expired()? 
            nullptr: PSYQ_ASSERT(this->item_ != nullptr), this->item_->data();
    }

    /// @copydoc psyq::string::view::size
    public: PSYQ_CONSTEXPR typename this_type::factory::view::size_type size()
    const PSYQ_NOEXCEPT
    {
        return this->factory_.expired()?
            0: PSYQ_ASSERT(this->item_ != nullptr), this->item_->size;
    }

    /// @copydoc psyq::string::view::max_size
    public: PSYQ_CONSTEXPR std::size_t max_size() const PSYQ_NOEXCEPT
    {
        return (std::numeric_limits<std::size_t>::max)();
    }

    /** @brief 文字列のハッシュ値を取得する。
        @return flyweight文字列辞書で使われているハッシュ値。
     */
    public: PSYQ_CONSTEXPR typename this_type::factory::hash::value_type
    get_hash() const PSYQ_NOEXCEPT
    {
        return this->factory_.expired()?
            this_type::factory::hash::EMPTY:
            PSYQ_ASSERT(this->item_ != nullptr), this->item_->hash;
    }

    /** @brief 文字列が所属するflyweight文字列辞書を取得する。
        @return 文字列が所属するflyweight文字列辞書。
     */
    public: PSYQ_CONSTEXPR typename this_type::factory::weak_ptr const&
    get_factory() const PSYQ_NOEXCEPT
    {
        return this->factory_;
    }

    //-------------------------------------------------------------------------
    /// 文字列を所有するflyweight文字列辞書。
    private: typename this_type::factory::weak_ptr factory_;
    /// flyweight文字列。
    private: typename this_type::factory::item* item_;

}; // class psyq::string::flyweight_factory::_private_view

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
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
    public: flyweight_view() PSYQ_NOEXCEPT:
    base_type(base_type::make())
    {}

    /** @brief 文字列をコピー構築する。メモリ確保は行わない。
        @param[in] in_source コピー元となる文字列。
     */
    public: flyweight_view(this_type const& in_source) PSYQ_NOEXCEPT:
    base_type(in_source)
    {}

    /** @brief 文字列をムーブ構築する。メモリ確保は行わない。
        @param[in,out] io_source ムーブ元となる文字列。
     */
    public: flyweight_view(this_type && io_source) PSYQ_NOEXCEPT:
    base_type(std::move(io_source))
    {}

    /** @brief 文字列をコピー構築する。メモリ確保を行う場合ある。
        @param[in] in_string  コピー元となる文字列。
        @param[in] in_factory 保持する文字列工場。
     */
    public: flyweight_view(
        typename base_type::factory::view const& in_string,
        typename base_type::factory::shared_ptr const& in_factory)
    PSYQ_NOEXCEPT:
    base_type(base_type::make(in_string, in_factory))
    {}

}; // class psyq::string::flyweight_view

#endif // !PSYQ_STRING_FLYWEIGHT_HPP_
