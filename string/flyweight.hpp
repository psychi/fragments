﻿/*
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

    private: struct item
    {
        typename flyweight_factory::view::const_pointer data() const
        {
            return reinterpret_cast<
                typename flyweight_factory::view::const_pointer>(this + 1);
        }

        /// 文字列の要素数。
        typename flyweight_factory::view::size_type size;
        /// 文字列のハッシュ値。
        typename flyweight_factory::hash::value_type hash;
    };

    private: typedef std::vector<
        typename this_type::item*,
        typename this_type::allocator_type::template
            rebind<typename this_type::item*>::other>
                item_array;

    public: class item_base;

    //-------------------------------------------------------------------------
    /** @brief 文字列のハッシュ値を算出する。
        @param[in] in_string ハッシュ値を算出する文字列。
        @return 文字列のハッシュ値。
     */
    public: static typename this_type::hash::value_type compute_hash(
        typename this_type::view const& in_string)
    PSYQ_NOEXCEPT
    {
        return this_type::hash::compute(
            in_string.data(), in_string.data() + in_string.size());
    }

    /** @brief 文字列を辞書に登録する。

        登録する文字列と等価な文字列がすでに存在するなら、
        登録済の文字列を再利用する。

        @param[in] in_string 登録する文字列。
     */
    private: typename this_type::item const& equip_item(
        typename this_type::view const& in_string)
    {
        // 等価な文字列を、辞書から検索する。
        auto const local_hash(this_type::compute_hash(in_string));
        auto const local_position(
            std::lower_bound(
                this->items_.begin(),
                this->items_.end(),
                local_hash));
        if (local_position != this->items_.end())
        {
            auto const& local_item(**local_position);
            if (local_hash == local_item.hash
                && in_string.size() == local_item.size)
            {
                typename this_type::view const local_string(
                    local_item.data(), local_item.size);
                if (in_string == local_string)
                {
                    return local_item;
                }
            }
        }

        // 等価な文字列が辞書になかったので、新たに登録する。
        auto const& local_item(this->make_item(in_string, local_hash));
        this->items_.insert(local_position, &local_item);
        return local_item;
    }

    private: typename this_type::item const& make_item(
        typename this_type::view const& in_string,
        typename this_type::hash const in_hash);

    //-------------------------------------------------------------------------
    /// ハッシュ値をキーにソートされている、文字列の辞書。
    private: typename this_type::item_array items_;

}; // class psyq::string::flyweight_factory

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
class psyq::string::flyweight_factory<
    template_char_type, template_char_traits, template_allocator_type>
        ::item_base
{
    /// thisが指す値の型。
    private: typedef item_base this_type;

    public: typedef template_char_traits traits_type; ///< 文字特性の型。

    public: typedef psyq::string::flyweight_factory<
        template_char_type, template_char_traits, template_allocator_type>
            factory;

    //-------------------------------------------------------------------------
    protected: item_base(this_type const& in_source) PSYQ_NOEXCEPT:
    factory_(io_source.factory_),
    item_(io_source.item_)
    {}

    protected: item_base(this_type&& io_source) PSYQ_NOEXCEPT:
    factory_(std::move(io_source.factory_)),
    item_(std::move(io_source.item_))
    {
        io_source.clear();
    }

    private: item_base(
        typename this_type::factory::weak_ptr in_factory,
        typename this_type::factory::item const* const in_item)
    PSYQ_NOEXCEPT:
    factory_(std::move(in_factory)),
    item_(in_item)
    {}

    protected: static this_type make() PSYQ_NOEXCEPT
    {
        return this_type(typename this_type::factory::weak_ptr(), nullptr);
    }

    protected: static this_type make(
        typename this_type::factory::view const& in_string,
        typename this_type::factory::shared_ptr const& in_factory)
    PSYQ_NOEXCEPT
    {
        return in_factory.get() != nullptr?
            this_type(in_factory, &in_factory->equip_item(in_string)):
            this_type::make();
    }

    /// @copydoc psyq::string::view::clear
    public: void clear() PSYQ_NOEXCEPT
    {
        this->factory_.reset();
        this->item_ = nullptr;
    }

    /// @copydoc psyq::string::view::data
    public: PSYQ_CONSTEXPR typename this_type::traits_type::char_type const* data()
    const PSYQ_NOEXCEPT
    {
        return this->factory_.expired()? nullptr: this->item_->data();
    }

    /// @copydoc psyq::string::view::size
    public: PSYQ_CONSTEXPR typename this_type::factory::view::size_type size()
    const PSYQ_NOEXCEPT
    {
        return this->factory_.expired()? 0: this->item_->size;
    }

    /// @copydoc psyq::string::view::max_size
    public: PSYQ_CONSTEXPR std::size_t max_size() const PSYQ_NOEXCEPT
    {
        return (std::numeric_limits<std::size_t>::max)();
    }

    public: typename this_type::factory::hash get_hash() const PSYQ_NOEXCEPT
    {
        return this->factory_.expired()? 0: this->item_.hash;
    }

    public: typename this_type::factory::weak_ptr const& get_factory()
    const PSYQ_NOEXCEPT
    {
        return this->factory_;
    }

    //-------------------------------------------------------------------------
    /// 文字列の本体を所有する flyweight_factory インスタンス。
    private: typename this_type::factory::weak_ptr factory_;
    /// 文字列の本体。
    private: typename this_type::factory::item const* item_;

}; // class psyq::string::flyweight_factory::item_base

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
class psyq::string::flyweight_view:
public psyq::string::_private::view_interface<
    typename psyq::string::flyweight_factory<
        template_char_type,
        template_char_traits,
        template_allocator_type>
            ::item_base>
{
    /// thisが指す値の型。
    private: typedef flyweight_view this_type;

    /// this_type の基底型。
    public: typedef psyq::string::_private::view_interface<
        typename psyq::string::flyweight_factory<
            template_char_type,
            template_char_traits,
            template_allocator_type>
                ::item_base>
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
