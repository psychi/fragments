/*
Copyright (c) 2015, Hillco Psychi, All rights reserved.

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
/// @file
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_CONTAINER_SORTED_CONTAINER_HPP_
#define PSYQ_CONTAINER_SORTED_CONTAINER_HPP_

#ifndef PSYQ_NO_STD_INITIALIZER_LIST
#include <initializer_list>
#endif // PSYQ_NO_STD_INITIALIZER_LIST

#include <type_traits>
#include "../assert.hpp"

namespace psyq
{
    namespace container
    {
        /// @cond
        template<typename, typename> struct map_value_compare;
        template<typename, typename> class sorted_map;
        template<typename, typename> class sorted_multimap;
        template<typename, typename> class sorted_set;
        template<typename, typename> class sorted_multiset;
        /// @endcond

        namespace _private
        {
            /// @cond
            template<bool> class container_sorter;
            template<typename, typename, bool> class sorted_container;
            /// @endcond
        } // namespace _private
    } // namespace container
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief std::pair::first を比較する関数オブジェクト。
/// @tparam template_pair    std::pair 型。
/// @tparam template_compare std::pair::first_type を比較する関数オブジェクト。
template<
    typename template_pair,
    typename template_compare = std::less<typename template_pair::first_type>>
struct psyq::container::map_value_compare: public template_compare
{
    /// @copydoc psyq::string::view::this_type
    private: typedef map_value_compare this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef template_compare base_type;

    //-------------------------------------------------------------------------
    public: typedef template_pair first_argument_type;
    public: typedef template_pair second_argument_type;

    //-------------------------------------------------------------------------
    public: bool operator()(
        typename this_type::first_argument_type const& in_left,
        typename this_type::second_argument_type const& in_right)
    const
    {
        return (*this)(in_left.first, in_right.first);
    }

    public: bool operator()(
        typename this_type::first_argument_type const& in_left,
        typename this_type::second_argument_type::first_type const& in_right)
    const
    {
        return (*this)(in_left.first, in_right);
    }

    public: bool operator()(
        typename this_type::first_argument_type::first_type const& in_left,
        typename this_type::second_argument_type const& in_right)
    const
    {
        return (*this)(in_left, in_right.first);
    }

    public: bool operator()(
        typename this_type::first_argument_type::first_type const& in_left,
        typename this_type::second_argument_type::first_type const& in_right)
    const
    {
        return static_cast<base_type const&>(*this)(in_left, in_right);
    }

}; // struct psyq::container::map_value_compare

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief ソート済コンテナを用いた、 std::map を模した辞書。
/// @tparam template_container @copydoc _private::sorted_container::base_container
/// @tparam template_compare   @copydoc _private::sorted_container::value_compare
template<
    typename template_container,
    typename template_compare
        = psyq::container::map_value_compare<
            typename template_container::value_type>>
class psyq::container::sorted_map:
public psyq::container::_private::sorted_container<
    template_container, template_compare, false>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef sorted_map this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::container::_private::sorted_container<
            template_container, template_compare, false>
        base_type;

    //-------------------------------------------------------------------------
    /// @brief キーの型。
    public: typedef typename base_type::value_type::first_type key_type;
    /// @brief キーに対応する値の型。
    public: typedef typename base_type::value_type::second_type mapped_type;

    //-------------------------------------------------------------------------
    /// @name 構築
    /// @{

    /// @brief ソート済辞書を構築する。
    public: explicit sorted_map(
        /// [in] 元となるコンテナ。
        typename base_type::base_container in_source = template_container(),
        /// [in] コンテナ要素を比較する関数オブジェクト。
        typename base_type::value_compare in_compare = template_compare()):
    base_type(std::move(in_source), std::move(in_compare))
    {}

    /// @brief ソート済辞書をコピー構築する。
    public: sorted_map(
        /// [in] コピー元となるソート済辞書。
        this_type const& in_source):
    base_type(in_source)
    {}

    /// @brief ソート済辞書をムーブ構築する。
    public: sorted_map(
        /// [in] ムーブ元となるソート済辞書。
        this_type&& io_source):
    base_type(std::move(io_source))
    {}
    /// @}
    //-------------------------------------------------------------------------
    /// @name 要素の参照
    /// @{

    /// @brief キーに対応する要素を参照する。
    /// @details in_key に対応する要素がない場合は std::out_of_range を投げる。
    /// @return in_key に対応する要素。
    public: typename base_type::value_type const& at(
        /// [in] 要素に対応するキー。
        typename this_type::key_type const& in_key)
    const
    {
        return *this_type::at(*this, in_key);
    }

    /// @brief キーに対応する要素を参照する。
    /// @details in_key に対応する要素がない場合は std::out_of_range を投げる。
    /// @return in_key に対応する要素。
    public: typename base_type::value_type& at(
        /// [in] 要素に対応するキー。
        typename this_type::key_type const& in_key)
    {
        return *this_type::at(*this, in_key);
    }

    /// @brief キーに対応する要素を参照する。
    /// @details
    ///   in_key に対応する要素がない場合は、
    ///   対応する要素を構築して挿入する。
    /// @return in_key に対応する要素。
    public: typename base_type::value_type& operator[](
        /// [in] 要素に対応するキー。
        typename this_type::key_type const& in_key)
    {
        auto const local_lower_bound(this->lower_bound(in_key));
        return std::end(*this) == local_lower_bound
            && this->compare_(*local_lower_bound, in_key)?
                *this->insert(
                    local_lower_bound,
                    typename base_type::value_type(
                        in_key, typename this_type::mapped_type())):
                *local_lower_bound;
    }
    /// @}
    //-------------------------------------------------------------------------
    private: template<typename template_sorted_container>
    static auto at(
        template_sorted_container& in_container,
        typename this_type::key_type const& in_key)
    ->decltype(*std::begin(in_container))
    {
        auto const local_lower_bound(in_container.lower_bound(in_key));
        PSYQ_ASSERT_THROW(
            std::end(in_container) != local_lower_bound
            && !in_container.compare_(*local_lower_bound, in_key),
            std::out_of_range);
        return *local_lower_bound;
    }

}; // class psyq::container::sorted_map:

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief ソート済コンテナを用いた、 std::multimap を模した辞書。
/// @tparam template_container @copydoc _private::sorted_container::base_container
/// @tparam template_compare   @copydoc _private::sorted_container::value_compare
template<
    typename template_container,
    typename template_compare
        = psyq::container::map_value_compare<
            typename template_container::value_type>>
class psyq::container::sorted_multimap:
public psyq::container::_private::sorted_container<
    template_container, template_compare, true>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef sorted_multimap this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::container::_private::sorted_container<
            template_container, template_compare, true>
        base_type;

    //-------------------------------------------------------------------------
    /// @copydoc sorted_map::key_type
    public: typedef typename base_type::value_type::first_type key_type;
    /// @copydoc sorted_map::mapped_type
    public: typedef typename base_type::value_type::second_type mapped_type;

    //-------------------------------------------------------------------------
    /// @name 構築
    /// @{

    /// @brief ソート済辞書を構築する。
    public: explicit sorted_multimap(
        /// [in] 元となるコンテナ。
        typename base_type::base_container in_source = template_container(),
        /// [in] コンテナ要素を比較する関数オブジェクト。
        typename base_type::value_compare in_compare = template_compare()):
    base_type(std::move(in_source), std::move(in_compare))
    {}

    /// @brief ソート済辞書をコピー構築する。
    public: sorted_multimap(
        /// [in] コピー元となるソート済辞書。
        this_type const& in_source):
    base_type(in_source)
    {}

    /// @brief ソート済辞書をムーブ構築する。
    public: sorted_multimap(
        /// [in] ムーブ元となるソート済辞書。
        this_type&& io_source):
    base_type(std::move(io_source))
    {}
    /// @}
}; // class psyq::container::sorted_multimap:

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief ソート済コンテナを用いた、 std::set を模した集合。
/// @tparam template_container @copydoc _private::sorted_container::base_container
/// @tparam template_compare   @copydoc _private::sorted_container::value_compare
template<
    typename template_container,
    typename template_compare
        = std::less<typename template_container::value_type>>
class psyq::container::sorted_set:
public psyq::container::_private::sorted_container<
    template_container, template_compare, false>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef sorted_set this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::container::_private::sorted_container<
            template_container, template_compare, false>
        base_type;

    //-------------------------------------------------------------------------
    /// @name 構築
    /// @{

    /// @brief ソート済集合を構築する。
    public: explicit sorted_set(
        /// [in] 元となるコンテナ。
        typename base_type::base_container in_source = template_container(),
        /// [in] コンテナ要素を比較する関数オブジェクト。
        typename base_type::value_compare in_compare = template_compare()):
    base_type(std::move(in_source), std::move(in_compare))
    {}

    /// @brief ソート済集合をコピー構築する。
    public: sorted_set(
        /// [in] コピー元となるソート済集合。
        this_type const& in_source):
    base_type(in_source)
    {}

    /// @brief ソート済集合をムーブ構築する。
    public: sorted_set(
        /// [in] ムーブ元となるソート済集合。
        this_type&& io_source):
    base_type(std::move(io_source))
    {}
    /// @}
}; // class psyq::container::sorted_set:

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief ソート済コンテナを用いた、 std::multiset を模した集合。
/// @tparam template_container @copydoc _private::sorted_container::base_container
/// @tparam template_compare   @copydoc _private::sorted_container::value_compare
template<
    typename template_container,
    typename template_compare
        = std::less<typename template_container::value_type>>
class psyq::container::sorted_multiset:
public psyq::container::_private::sorted_container<
    template_container, template_compare, true>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef sorted_multiset this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::container::_private::sorted_container<
            template_container, template_compare, true>
        base_type;

    //-------------------------------------------------------------------------
    /// @name 構築
    /// @{

    /// @brief ソート済集合を構築する。
    public: explicit sorted_multiset(
        /// [in] 元となるコンテナ。
        typename base_type::base_container in_source = template_container(),
        /// [in] コンテナ要素を比較する関数オブジェクト。
        typename base_type::value_compare in_compare = template_compare()):
    base_type(std::move(in_source), std::move(in_compare))
    {}

    /// @brief ソート済集合をコピー構築する。
    public: sorted_multiset(
        /// [in] コピー元となるソート済集合。
        this_type const& in_source):
    base_type(in_source)
    {}

    /// @brief ソート済集合をムーブ構築する。
    public: sorted_multiset(
        /// [in] ムーブ元となるソート済集合。
        this_type&& io_source):
    base_type(std::move(io_source))
    {}
    /// @}
}; // class psyq::container::sorted_multiset:

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 要素の重複を許可しないコンテナのソート処理。
template<> class psyq::container::_private::container_sorter<false>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef container_sorter this_type;

    /// @copydoc sorted_container::is_insert_position
    public: template<typename template_container, typename template_compare>
    static bool is_insert_position(
        template_container const& in_container,
        template_compare const& in_compare,
        typename template_container::const_iterator const& in_position,
        typename template_container::value_type const& in_value)
    {
        return (in_position == std::begin(in_container)
                || in_compare(*std::prev(in_position), in_value))
            && (in_position == std::end(in_container)
                || in_compare(in_value, *in_position));
    }

    /// @copydoc sorted_container::is_sorted
    public: template<typename template_container, typename template_compare>
    static bool is_sorted(
        template_container const& in_container,
        template_compare const& in_compare)
    {
        auto local_prev(std::begin(in_container));
        auto const local_end(std::end(in_container));
        if (local_prev != local_end)
        {
            for (auto i(std::next(local_prev)); i != local_end; ++i)
            {
                if (!in_compare(*local_prev, *i))
                {
                    return false;
                }
                local_prev = i;
            }
        }
        return true;
    }

    /// @copydoc sorted_container::sort
    public: template<typename template_container, typename template_compare>
    static bool sort(
        template_container& io_container,
        template_compare const& in_compare)
    {
        if (this_type::is_sorted(io_container, in_compare))
        {
            return false;
        }
        std::sort(std::begin(io_container), std::end(io_container), in_compare);

        // 重複する要素を削除する。
        auto local_next(std::prev(std::end(io_container)));
        for (auto i(std::prev(local_next)); i != std::begin(io_container);)
        {
            if (in_compare(*i, *local_next))
            {
                local_next = i;
                --i;
            }
            else
            {
                local_next = io_container.erase(local_next);
                i = std::prev(local_next);
            }
        }
        return true;
    }

    /// @brief コンテナに要素を挿入する。
    /// @return 要素の位置と挿入したかどうかのペア。
    public: template<typename template_container, typename template_compare>
    static std::pair<typename template_container::iterator, bool> insert(
        /// [in] 要素を挿入するコンテナ。
        template_container& io_container,
        /// [in] 要素を比較する関数オブジェクト。
        template_compare const& in_compare,
        /// [in] 要素を挿入する位置。
        typename template_container::iterator in_position,
        /// [in] 挿入する要素の初期値。
        typename template_container::value_type in_value)
    {
        PSYQ_ASSERT(
             std::begin(io_container) == in_position
             || in_compare(*std::prev(in_position), in_value));
        PSYQ_ASSERT(
             std::end(io_container) == in_position
             || !in_compare(*in_position, in_value));
        auto const local_insert(
            this_type::is_insert_position(
                io_container, in_compare, in_position, in_value));
        return std::make_pair(
            local_insert?
                io_container.insert(
                    std::move(in_position), std::move(in_value)):
                in_position,
            local_insert);
    }

}; // class psyq::container::_private::container_sorter<false>

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 要素の重複を許可するコンテナのソート処理。
template<> class psyq::container::_private::container_sorter<true>
{
    private: typedef container_sorter this_type;

    /// @copydoc sorted_container::is_insert_position
    public: template<typename template_container, typename template_compare>
    static bool is_insert_position(
        template_container const& in_container,
        template_compare const& in_compare,
        typename template_container::const_iterator const& in_position,
        typename template_container::value_type const& in_value)
    {
        return (in_position == std::begin(in_container)
                || !in_compare(in_value, *std::prev(in_position)))
            && (in_position == std::end(in_container)
                || !in_compare(*in_position, in_value));
    }

    /// @copydoc sorted_container::is_sorted
    public: template<typename template_container, typename template_compare>
    static bool is_sorted(
        template_container const& in_container,
        template_compare const& in_compare)
    {
        return std::is_sorted(
            std::begin(in_container), std::end(in_container), in_compare);
    }

    /// @copydoc sorted_container::sort
    public: template<typename template_container, typename template_compare>
    static bool sort(
        template_container& io_container,
        template_compare const& in_compare)
    {
        if (this_type::is_sorted(io_container, in_compare))
        {
            return false;
        }
        std::sort(std::begin(io_container), std::end(io_container), in_compare);
        return true;
    }

    /// @brief コンテナに要素を挿入する。
    /// @return 挿入した要素を指す反復子。
    public: template<typename template_container, typename template_compare>
    static typename template_container::iterator insert(
        /// [in] 要素を挿入するコンテナ。
        template_container& io_container,
        /// [in] 要素を比較する関数オブジェクト。
        template_compare const& in_compare,
        /// [in] 要素を挿入する位置。
        typename template_container::const_iterator in_position,
        /// [in] 挿入する要素の初期値。
        typename template_container::value_type in_value)
    {
        PSYQ_ASSERT(
            this_type::is_insert_position(
                io_container, in_compare, in_position, in_value));
        return io_container.insert(std::move(in_position), std::move(in_value));
    }

}; // class psyq::container::_private::container_sorter<true>

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief ソート済コンテナの基底型。
/// @tparam template_container @copydoc sorted_container::base_container
/// @tparam template_compare   @copydoc sorted_container::value_compare
/// @tparam template_multi     重複する要素を許可するかどうか。
template<
    typename template_container,
    typename template_compare,
    bool template_multi>
class psyq::container::_private::sorted_container: private template_container
{
    /// @copydoc psyq::string::view::this_type
    private: typedef sorted_container this_type;
    /// @copydoc psyq::string::view::base_type
    private: typedef template_container base_type;

    //-------------------------------------------------------------------------
    /// @brief ソートするコンテナの型。
    public: typedef template_container base_container;
    /// @brief 値を比較する関数オブジェクト。
    public: typedef template_compare value_compare;
    /// @brief キーを比較する関数オブジェクト。
    public: typedef template_compare key_compare;
    /// @brief キーの型。
    public: typedef typename base_type::value_type key_type;

    /// @brief 使用するソート処理。
    private: typedef
         psyq::container::_private::container_sorter<template_multi>
         sorter;

    //-------------------------------------------------------------------------
    public: using typename base_type::value_type;
    public: using typename base_type::size_type;
    public: using typename base_type::difference_type;

    public: using typename base_type::reference;
    public: using typename base_type::pointer;
    public: using typename base_type::iterator;
    public: using typename base_type::reverse_iterator;

    public: using typename base_type::const_reference;
    public: using typename base_type::const_pointer;
    public: using typename base_type::const_iterator;
    public: using typename base_type::const_reverse_iterator;

    public: using base_type::begin;
    public: using base_type::rbegin;
    public: using base_type::cbegin;
    public: using base_type::crbegin;

    public: using base_type::end;
    public: using base_type::rend;
    public: using base_type::cend;
    public: using base_type::crend;

    public: using base_type::size;
    public: using base_type::max_size;
    public: using base_type::empty;
    public: using base_type::clear;

    //-------------------------------------------------------------------------
    /// @name 構築
    /// @{

    /// @brief コンテナをソートし、ソート済コンテナを構築する。
    public: explicit sorted_container(
        /// [in] ソートするコンテナ。
        typename this_type::base_container in_source,
        /// [in] 値を比較する関数オブジェクト。
        typename this_type::value_compare in_compare = value_compare()):
    base_type(std::move(in_source)),
    compare_(std::move(in_compare))
    {
        this->sort();
    }

    /// @brief ソート済コンテナをコピー構築する。
    public: sorted_container(
        /// [in] コピー元となるソート済コンテナ。
        this_type const& in_source):
    base_type((PSYQ_ASSERT(in_source.is_sorted()), in_source)),
    compare_(in_source.value_comp())
    {}

    /// @brief ソート済コンテナをムーブ構築する。
    public: sorted_container(
        /// [in] ムーブ元となるソート済コンテナ。
        this_type&& io_source):
    base_type((PSYQ_ASSERT(io_source.is_sorted()), std::move(io_source))),
    compare_(std::move(io_source.compare_))
    {}
    /// @}
    //-------------------------------------------------------------------------
    /// @name 代入
    /// @{

    /// @brief ソート済コンテナをコピー代入する。
    /// @return *this
    public: this_type& operator=(
        /// [in] コピー元となるソート済コンテナ。
        this_type const& in_source)
    {
        this->assign(in_source);
        return *this;
    }

    /// @brief ソート済コンテナをムーブ代入する。
    /// @return *this
    public: this_type& operator=(
        /// [in] ムーブ元となるソート済コンテナ。
        this_type&& io_source)
    {
        this->assign(std::move(io_source));
        return *this;
    }

    /// @brief ソート済コンテナを代入する。
    public: void assign(
        /// @brief 代入するソート済コンテナ。
        this_type in_source)
    {
        PSYQ_ASSERT(in_source.is_sorted());
        this->compare_ = std::move(in_source.compare_);
        this->get_mutable_container() = std::move(in_source);
    }

    /// @brief コンテナを代入してソートする。
    public: void assign(
        /// @brief 代入するコンテナ。
        typename this_type::base_container in_source)
    {
        this->get_mutable_container() = std::move(in_source);
        this->sort();
    }

    /// @brief コンテナを代入してソートする。
    public: template<typename template_other_container>
    void assign(
        /// @brief 代入するコンテナ。
        template_other_container const& in_source)
    {
        return this->assign(std::begin(in_source), std::end(in_source));
    }

    /// @brief コンテナを代入してソートする。
    public: template<typename template_iterator>
    void assign(
        /// @brief 代入するコンテナの先頭位置。
        template_iterator in_begin,
        /// @brief 代入するコンテナの末尾位置。
        template_iterator in_end)
    {
        this->base_type::assign(std::move(in_begin), std::move(in_end));
        this->sort();
    }

    /// @brief ソート済コンテナを交換する。
    public: void swap(
        /// [in,out] 交換するソート済コンテナ。
        this_type& io_target)
    {
        PSYQ_ASSERT(this->is_sorted() && io_target.is_sorted());
        this->base_type::swap(io_target);
        std::swap(this->compare_, io_target.compare_);
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 比較
    /// @{

    /// @brief 等価なコンテナか判定する。
    /// @return *this == in_right
    public: bool operator==(
        /// [in] 比較演算子の右辺となるコンテナ。
        this_type const& in_right) const
    {
        return this->get_container() == in_right.get_container();
    }

    /// @brief 不等価なコンテナか判定する。
    /// @return *this != in_right
    public: bool operator!=(
        /// [in] 比較演算子の右辺となるコンテナ。
        this_type const& in_right) const
    {
        return this->get_container() != in_right.get_container();
    }

    /// @brief より大きいコンテナか判定する。
    /// @return *this < in_right
    public: bool operator<(
        /// [in] 比較演算子の右辺となるコンテナ。
        this_type const& in_right) const
    {
        return this->get_container() < in_right.get_container();
    }

    /// @brief より大きい、または等価なコンテナか判定する。
    /// @return *this <= in_right
    public: bool operator<=(
        /// [in] 比較演算子の右辺となるコンテナ。
        this_type const& in_right) const
    {
        return this->get_container() <= in_right.get_container();
    }

    /// @brief より小さいコンテナか判定する。
    /// @return *this > in_right
    public: bool operator>(
        /// [in] 比較演算子の右辺となるコンテナ。
        this_type const& in_right) const
    {
        return this->get_container() > in_right.get_container();
    }

    /// @brief より小さい、または等価なコンテナか判定する。
    /// @return *this >= in_right
    public: bool operator>=(
        /// [in] 比較演算子の右辺となるコンテナ。
        this_type const& in_right) const
    {
        return this->get_container() >= in_right.get_container();
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name コンテナのプロパティ
    /// @{

    /// @brief キーを比較する関数オブジェクトを取得する。
    /// @return キーを比較する関数オブジェクト。
    public: typename this_type::key_compare const& key_comp()
    const PSYQ_NOEXCEPT
    {
        return this->value_comp();
    }

    /// @brief 値を比較する関数オブジェクトを取得する。
    /// @return 値を比較する関数オブジェクト。
    public: typename this_type::value_compare const& value_comp()
    const PSYQ_NOEXCEPT
    {
        return this->compare_;
    }

    /// @brief コンテナを取得する。
    /// @return *this が使用しているコンテナ。
    public: typename this_type::base_container const& get_container()
    const PSYQ_NOEXCEPT
    {
        return *this;
    }

    /// @brief コンテナを取り出し、 *this を空にする。
    /// @return *this が使用していたコンテナ。
    public: typename this_type::base_container remove_container()
    {
        auto const local_container(std::move(this->container_));
        this->clear();
        return local_container;
    }

    /// @brief 値を挿入するのに適切な位置か判定する。
    /// @return 値を挿入するのに適切な位置かどうか。
    public: bool is_insert_position(
        /// [in] 挿入する位置。
        typename base_type::const_iterator const& in_position,
        /// [in] 挿入する値。
        typename base_type::value_type const& in_value)
    const
    {
        return this_type::sorter::is_insert_position(
            this->get_container(), this->value_comp(), in_position, in_value);
    }

    /// @brief コンテナがソートされているか判定する。
    /// @retval true  コンテナはソートされていない。
    /// @retval false コンテナはにソートされている。
    public: bool is_sorted() const
    {
        return this_type::sorter::is_sorted(
            this->get_container(), this->value_comp());
    }

    /// @brief コンテナをソートする。
    /// @retval true  コンテナがソートされてなかったので、ソートした。
    /// @retval false コンテナはすでにソートされていたので、何もしなかった。
    public: bool sort()
    {
        return this_type::sorter::sort(
            this->get_mutable_container(), this->value_comp());
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 要素の検索
    /// @{

    /// @brief 指定されたキーの位置を取得する。
    /// @return in_key に対応する要素が最初に現れる位置。
    public: template<typename template_key>
    typename base_type::iterator find(
        /// [in] 基準となるキー。
        template_key const& in_key)
    {
        return this_type::find(*this, in_key);
    }

    /// @copydoc find
    public: template<typename template_key>
    typename base_type::const_iterator find(
        /// [in] 基準となるキー。
        template_key const& in_key)
    const
    {
        return this_type::find(*this, in_key);
    }

    /// @brief 指定されたキー以上の要素の位置を取得する。
    /// @return in_key 以上の要素が最初に現れる位置。
    public: template<typename template_key>
    typename base_type::iterator lower_bound(
        /// [in] 基準となるキー。
        template_key const& in_key)
    {
        return this_type::lower_bound(*this, in_key);
    }

    /// @copydoc lower_bound
    public: template<typename template_key>
    typename base_type::const_iterator lower_bound(
        /// [in] 基準となるキー。
        template_key const& in_key)
    const
    {
        return this_type::lower_bound(*this, in_key);
    }

    /// @brief 指定されたキーより大きい要素の位置を取得する。
    /// @return in_key より大きい要素が最初に現れる位置。
    public: template<typename template_key>
    typename base_type::iterator upper_bound(
        /// [in] 基準となるキー。
        template_key const& in_key)
    {
        return this_type::upper_bound(*this, in_key);
    }

    /// @copydoc upper_bound
    public: template<typename template_key>
    typename base_type::const_iterator upper_bound(
        /// [in] 基準となるキー。
        template_key const& in_key)
    const
    {
        return this_type::upper_bound(*this, in_key);
    }

    /// @brief 指定されたキーと等価な範囲を取得する。
    /// @return in_key と等価な範囲。
    public: template<typename template_key>
    std::pair<typename base_type::iterator, typename base_type::iterator>
    equal_range(
        /// [in] 基準となるキー。
        template_key const& in_key)
    {
        return this_type::equal_range(*this, in_key);
    }

    /// @copydoc equal_range
    public: template<typename template_key>
    std::pair<
        typename base_type::const_iterator, typename base_type::const_iterator>
    equal_range(
        /// [in] 基準となるキー。
        template_key const& in_key)
    const
    {
        return this_type::equal_range(*this, in_key);
    }

    /// @brief キーと合致する要素を数える。
    /// @return in_key と合致する要素の数。
    public: template<typename template_key>
    typename base_type::size_type count(
        /// [in] 基準となるキー。
        template_key const& in_key)
    const
    {
        auto const local_range(this->equal_range(in_key));
        return std::distance(local_range.first, local_range.second);
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 要素の挿入
    /// @{

    /// @brief 要素を挿入する。
    /// @return 挿入した要素を指す反復子。
    public: auto insert(
        /// [in] 挿入する要素の初期値。
        typename base_type::value_type in_value)
    ->decltype(
        this_type::sorter::insert(
            *static_cast<base_type*>(nullptr),
            *static_cast<typename this_type::value_compare*>(nullptr),
            *static_cast<typename base_type::iterator*>(nullptr),
            *static_cast<typename base_type::value_type*>(nullptr)))
    {
        return this_type::sorter::insert(
            this->get_mutable_container(),
            this->value_comp(),
            this->find_insert_position(in_value),
            std::move(in_value));
    }

    /// @brief 要素を挿入する。
    /// @return 挿入した要素を指す反復子。
    public: typename base_type::iterator insert(
        /// [in] 要素を挿入する位置。
        typename base_type::const_iterator in_position,
        /// [in] 挿入する要素の初期値。
        typename base_type::value_type in_value)
    {
        if (!this->is_insert_position(in_position, in_value))
        {
            in_position = this->find_insert_position(in_value);
        }
        return this->base_type::insert(
            std::move(in_position), std::move(in_value));
    }

    /// @brief 要素を挿入する。
    public: template<typename template_iterator>
    void insert(
        /// [in] 挿入する要素の初期値のコンテナの先頭位置。
        template_iterator const& in_begin,
        /// [in] 挿入する要素の初期値のコンテナの末尾位置。
        template_iterator const& in_end)
    {
        for (auto i(in_begin); i != in_end; ++i)
        {
            this->insert(*i);
        }
    }

#ifndef PSYQ_NO_STD_INITIALIZER_LIST
    /// @brief 要素を挿入する。
    public: void insert(
        /// [in] 挿入する要素の初期値のコンテナ。
        std::initializer_list<typename base_type::value_type> const& in_list)
    {
        this->insert(std::begin(in_list), std::end(in_list));
    }
#endif // !defined(PSYQ_NO_STD_INITIALIZER_LIST)
    /// @}
    //-------------------------------------------------------------------------
    /// @name 要素の削除
    /// @{

    /// @brief 要素を削除する。
    /// @return 削除した要素の次の要素を指す反復子。
    public: typename base_type::iterator erase(
        /// [in] 削除する要素を指す反復子。
        typename base_type::const_iterator in_position)
    {
        return this->base_type::erase(std::move(in_position));
    }

    /// @brief 要素を削除する。
    /// @return 削除した範囲の次の要素を指す反復子。
    public: typename base_type::iterator erase(
        /// [in] 削除する範囲の先頭位置。
        typename base_type::const_iterator in_begin,
        /// [in] 削除する範囲の末尾位置。
        typename base_type::const_iterator in_end)
    {
        return this->base_type::erase(std::move(in_begin), std::move(in_end));
    }

    /// @brief キーに対応する要素をすべて削除する。
    /// @return 削除した要素の数。
    public: template<typename template_key>
    typename base_type::size_type erase(
        /// [in] 基準となるキー。
        template_key const& in_key)
    {
        auto local_range(this->equal_range(in_key));
        auto const local_count(
            std::distance(local_range.first, local_range.second));
        this->erase(
            std::move(local_range.first), std::move(local_range.second));
        return local_count;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @copydoc get_container
    private: typename this_type::base_container& get_mutable_container()
    PSYQ_NOEXCEPT
    {
        return *this;
    }

    private: typename base_type::iterator find_insert_position(
        /// [in] 挿入する値。
        typename base_type::value_type const& in_value)
    {
        return template_multi?
            this->upper_bound(in_value): this->lower_bound(in_value);
    }

    /// @copydoc find
    private: template<typename template_sorted_container, typename template_key>
    static auto find(
        /// [in] 要素を検索するソート済コンテナ。
        template_sorted_container& in_container,
        /// [in] 基準となるキー。
        template_key const& in_key)
    ->decltype(std::begin(in_container))
    {
        auto const local_end(std::end(in_container));
        auto const local_lower_bound(this_type::lower_bound(in_container, in_key));
        return local_lower_bound != local_end
            && !in_container.compare_(in_key, *local_lower_bound)?
                local_lower_bound: local_end;
    }

    /// @copydoc lower_bound
    private: template<typename template_sorted_container, typename template_key>
    static auto lower_bound(
        /// [in] 要素を検索するソート済コンテナ。
        template_sorted_container& in_container,
        /// [in] 基準となるキー。
        template_key const& in_key)
    ->decltype(std::begin(in_container))
    {
        PSYQ_ASSERT(in_container.is_sorted());
        return std::lower_bound(
            std::begin(in_container),
            std::end(in_container),
            in_key,
            in_container.value_comp());
    }

    /// @copydoc upper_bound
    private: template<typename template_sorted_container, typename template_key>
    static auto upper_bound(
        /// [in] 要素を検索するソート済コンテナ。
        template_sorted_container& in_container,
        /// [in] 基準となるキー。
        template_key const& in_key)
    ->decltype(std::begin(in_container))
    {
        PSYQ_ASSERT(in_container.is_sorted());
        return std::upper_bound(
            std::begin(in_container),
            std::end(in_container),
            in_key,
            in_container.value_comp());
    }

    /// @copydoc equal_range
    private: template<typename template_sorted_container, typename template_key>
    static auto equal_range(
        /// [in] 要素を検索するソート済コンテナ。
        template_sorted_container& in_container,
        /// [in] 基準となるキー。
        template_key const& in_key)
    ->std::pair<
        decltype(std::begin(in_container)),
        decltype(std::begin(in_container))>
    {
        PSYQ_ASSERT(in_container.is_sorted());
        if (template_multi)
        {
            return std::equal_range(
                std::begin(in_container),
                std::end(in_container),
                in_key,
                in_container.value_comp());
        }
        else
        {
            auto const local_lower_bound(in_container.lower_bound(in_key));
            return std::make_pair(
                local_lower_bound,
                local_lower_bound == std::end(in_container)
                || in_container.compare_(in_key, *local_lower_bound)?
                    local_lower_bound: std::next(local_lower_bound));
        }
    }

    //-------------------------------------------------------------------------
    /// @brief 要素を比較する関数オブジェクト。
    private: typename this_type::value_compare compare_;

}; // class psyq::container::sorted_container

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    inline void sorted_container()
    {
        typedef psyq::container::sorted_set<std::vector<int>> set;
        set local_set;
        PSYQ_ASSERT(local_set.insert(30).second);
        PSYQ_ASSERT(local_set.insert(10).second);
        PSYQ_ASSERT(local_set.insert(20).second);
        PSYQ_ASSERT(!local_set.insert(10).second);
        PSYQ_ASSERT(local_set.count(10) == 1);
        PSYQ_ASSERT(local_set.count(15) == 0);
        local_set.size();
    }
}

#endif // !defined(PSYQ_CONTAINER_SORTED_CONTAINER_HPP_)
// vim: set expandtab:
