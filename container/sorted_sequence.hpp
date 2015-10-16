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
/// @brief ソート済シーケンスコンテナを用いた連想コンテナ。
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_CONTAINER_SORTED_SEQUENCE_HPP_
#define PSYQ_CONTAINER_SORTED_SEQUENCE_HPP_

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
        template<typename, typename> class sequence_map;
        template<typename, typename> class sequence_multimap;
        template<typename, typename> class sequence_set;
        template<typename, typename> class sequence_multiset;
        template<typename, typename> struct pair_first_compare;
        /// @endcond

        namespace _private
        {
            /// @cond
            template<bool> class sequence_sorter;
            template<typename, typename, bool, bool> class sorted_sequence;
            /// @endcond
        } // namespace _private
    } // namespace container
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief std::map を模した、ソート済シーケンスコンテナを用いた辞書。
/// @warning
///   コンテナの要素となっている値の template_container::value_type::first
///   を書き換えるのは禁止。ソート済でなくなる可能性があるため。
/// @tparam template_container
///   - base_type::container_type の条件を満たしていること。
///   - template_container::value_type が std::pair 互換の型であること。
/// @tparam template_compare
///   値を比較する関数オブジェクト。以下のメンバ関数が使えること。
///   - bool template_compare::operator()(
///       template_container::value_type const&,
///       template_container::value_type const&)
///     const
///   - bool template_compare::operator()(
///       template_container::value_type const&,
///       template_container::value_type::first_type const&)
///     const
///   - bool template_compare::operator()(
///       template_container::value_type::first_type const&,
///       template_container::value_type const&)
///     const
///   - bool template_compare::operator()(
///       template_container::value_type::first_type const&,
///       template_container::value_type::first_type const&)
///     const
template<
    typename template_container,
    typename template_compare
        = psyq::container::pair_first_compare<
            typename template_container::value_type,
            std::less<typename template_container::value_type::first_type>>>
class psyq::container::sequence_map:
public psyq::container::_private::sorted_sequence<
    template_container, template_compare, false, true>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef sequence_map this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::container::_private::sorted_sequence<
            template_container, template_compare, false, true>
        base_type;

    //-------------------------------------------------------------------------
    /// @brief キーの型。
    public: typedef typename base_type::value_type::first_type key_type;
    /// @brief キーに対応する値の型。
    public: typedef typename base_type::value_type::second_type mapped_type;

    //-------------------------------------------------------------------------
    /// @name 構築
    /// @{

    /// @brief 空のソート済シーケンス辞書を構築する。
    public: explicit sequence_map(
        /// [in] 値を比較する関数オブジェクトの初期値。
        typename base_type::value_compare in_compare = template_compare()):
    base_type(std::move(in_compare))
    {}

    /// @brief ソート済シーケンス辞書をコピー構築する。
    /// @warning
    ///   in_container がソートされてなかった場合 *this は空となり、
    ///   std::invalid_argument 例外を投げる。
    public: explicit sequence_map(
        /// [in] コピー元となるソート済シーケンスコンテナ。
        typename base_type::container_type const& in_container,
        /// [in] 値を比較する関数オブジェクトの初期値。
        typename base_type::value_compare const& in_compare
            = template_compare()):
    base_type(in_container, in_compare)
    {}

    /// @brief ソート済シーケンス辞書をムーブ構築する。
    /// @warning
    ///   io_container がソートされてなかった場合 *this は空となり、
    ///   std::invalid_argument 例外を投げる。
    public: explicit sequence_map(
        /// [in] ムーブ元となるソート済シーケンスコンテナ。
        typename base_type::container_type&& io_container,
        /// [in] 値を比較する関数オブジェクトの初期値。
        typename base_type::value_compare&& io_compare = template_compare()):
    base_type(std::move(io_container), std::move(io_compare))
    {}

    /// @brief ソート済シーケンス辞書をコピー構築する。
    public: sequence_map(
        /// [in] コピー元となるソート済シーケンス辞書。
        this_type const& in_source):
    base_type(in_source)
    {}

    /// @brief ソート済シーケンス辞書をムーブ構築する。
    public: sequence_map(
        /// [in] ムーブ元となるソート済シーケンス辞書。
        this_type&& io_source):
    base_type(std::move(io_source))
    {}
    /// @}
    //-------------------------------------------------------------------------
    /// @name 要素の参照
    /// @{

    /// @brief キーに対応する要素を参照する。
    /// @warning in_key に対応する要素がない場合 std::out_of_range 例外を投げる。
    /// @return in_key に対応する要素。
    public: template<typename template_key>
    typename this_type::mapped_type const& at(
        /// [in] 要素に対応するキー。
        template_key const& in_key)
    const
    {
        return this_type::at(*this, in_key);
    }

    /// @copydoc at
    public: template<typename template_key>
    typename this_type::mapped_type& at(
        /// [in] 要素に対応するキー。
        template_key const& in_key)
    {
        return this_type::at(*this, in_key);
    }

    /// @brief キーに対応する要素を参照する。
    /// @details in_key に対応する要素がない場合は、対応する要素を構築して挿入する。
    /// @return in_key に対応する要素。
    public: typename this_type::mapped_type& operator[](
        /// [in] 要素に対応するキー。
        typename base_type::key_type const& in_key)
    {
        auto const local_lower_bound(this->lower_bound(in_key));
        return std::end(*this) == local_lower_bound
            && this->compare_(*local_lower_bound, in_key)?
                this->insert(
                    local_lower_bound,
                    typename base_type::value_type(
                        in_key, typename this_type::mapped_type()))->second:
                local_lower_bound->second;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @copydoc at
    private: template<typename template_sequence_map, typename template_key>
    static auto at(
        /// [in] 参照する要素を持つ辞書。
        template_sequence_map& in_map,
        /// [in] 要素に対応するキー。
        template_key const& in_key)
    ->decltype((std::begin(in_map)->second))
    {
        auto const local_find(in_map.find(in_key));
        PSYQ_ASSERT_THROW(std::end(in_map) != local_find, std::out_of_range);
        return local_find->second;
    }

}; // class psyq::container::sequence_map:

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief std::multimap を模した、ソート済シーケンスコンテナを用いた辞書。
/// @copydetails psyq::container::sequence_map
template<
    typename template_container,
    typename template_compare
        = psyq::container::pair_first_compare<
            typename template_container::value_type,
            std::less<typename template_container::value_type::first_type>>>
class psyq::container::sequence_multimap:
public psyq::container::_private::sorted_sequence<
    template_container, template_compare, true, true>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef sequence_multimap this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::container::_private::sorted_sequence<
            template_container, template_compare, true, true>
        base_type;

    //-------------------------------------------------------------------------
    /// @copydoc sequence_map::key_type
    public: typedef typename base_type::value_type::first_type key_type;
    /// @copydoc sequence_map::mapped_type
    public: typedef typename base_type::value_type::second_type mapped_type;

    //-------------------------------------------------------------------------
    /// @name 構築
    /// @{

    /// @brief 空のソート済シーケンス辞書を構築する。
    public: explicit sequence_multimap(
        /// [in] 値を比較する関数オブジェクトの初期値。
        typename base_type::value_compare in_compare = template_compare()):
    base_type(std::move(in_compare))
    {}

    /// @brief ソート済シーケンス辞書をコピー構築する。
    /// @warning
    ///   in_container がソートされてなかった場合 *this は空となり、
    ///   std::invalid_argument 例外を投げる。
    public: explicit sequence_multimap(
        /// [in] コピー元となるソート済シーケンスコンテナ。
        typename base_type::container_type const& in_container,
        /// [in] 値を比較する関数オブジェクトの初期値。
        typename base_type::value_compare const& in_compare
            = template_compare()):
    base_type(in_container, in_compare)
    {}

    /// @brief ソート済シーケンス辞書をムーブ構築する。
    /// @warning
    ///   io_container がソートされてなかった場合 *this は空となり、
    ///   std::invalid_argument 例外を投げる。
    public: explicit sequence_multimap(
        /// [in] ムーブ元となるソート済シーケンスコンテナ。
        typename base_type::container_type&& io_container,
        /// [in] 値を比較する関数オブジェクトの初期値。
        typename base_type::value_compare&& io_compare = template_compare()):
    base_type(std::move(io_container), std::move(io_compare))
    {}

    /// @brief ソート済シーケンス辞書をコピー構築する。
    public: sequence_multimap(
        /// [in] コピー元となるソート済シーケンス辞書。
        this_type const& in_source):
    base_type(in_source)
    {}

    /// @brief ソート済シーケンス辞書をムーブ構築する。
    public: sequence_multimap(
        /// [in] ムーブ元となるソート済シーケンス辞書。
        this_type&& io_source):
    base_type(std::move(io_source))
    {}
    /// @}
}; // class psyq::container::sequence_multimap:

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief std::set を模した、ソート済シーケンスコンテナを用いた集合。
/// @tparam template_container @copydoc _private::sorted_sequence::container_type
/// @tparam template_compare   @copydoc _private::sorted_sequence::value_compare
template<
    typename template_container,
    typename template_compare
        = std::less<typename template_container::value_type>>
class psyq::container::sequence_set:
public psyq::container::_private::sorted_sequence<
    template_container, template_compare, false, false>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef sequence_set this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::container::_private::sorted_sequence<
            template_container, template_compare, false, false>
        base_type;

    //-------------------------------------------------------------------------
    /// @name 構築
    /// @{

    /// @brief 空のソート済シーケンス集合を構築する。
    public: explicit sequence_set(
        /// [in] 値を比較する関数オブジェクトの初期値。
        typename base_type::value_compare in_compare = template_compare()):
    base_type(std::move(in_compare))
    {}

    /// @brief ソート済シーケンス集合をコピー構築する。
    /// @warning
    ///   in_container がソートされてなかった場合 *this は空となり、
    ///   std::invalid_argument 例外を投げる。
    public: explicit sequence_set(
        /// [in] コピー元となるソート済シーケンスコンテナ。
        typename base_type::container_type const& in_container,
        /// [in] 値を比較する関数オブジェクトの初期値。
        typename base_type::value_compare const& in_compare
            = template_compare()):
    base_type(in_container, in_compare)
    {}

    /// @brief ソート済シーケンス集合をムーブ構築する。
    /// @warning
    ///   io_container がソートされてなかった場合 *this は空となり、
    ///   std::invalid_argument 例外を投げる。
    public: explicit sequence_set(
        /// [in] ムーブ元となるソート済シーケンスコンテナ。
        typename base_type::container_type&& io_container,
        /// [in] 値を比較する関数オブジェクトの初期値。
        typename base_type::value_compare&& io_compare = template_compare()):
    base_type(std::move(io_container), std::move(io_compare))
    {}

    /// @brief ソート済シーケンス集合をコピー構築する。
    public: sequence_set(
        /// [in] コピー元となるソート済シーケンス集合。
        this_type const& in_source):
    base_type(in_source)
    {}

    /// @brief ソート済シーケンス集合をムーブ構築する。
    public: sequence_set(
        /// [in] ムーブ元となるソート済シーケンス集合。
        this_type&& io_source):
    base_type(std::move(io_source))
    {}
    /// @}
}; // class psyq::container::sequence_set:

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief std::multiset を模した、ソート済シーケンスコンテナを用いた集合。
/// @copydetails psyq::container::sequence_set
template<
    typename template_container,
    typename template_compare
        = std::less<typename template_container::value_type>>
class psyq::container::sequence_multiset:
public psyq::container::_private::sorted_sequence<
    template_container, template_compare, true, false>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef sequence_multiset this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::container::_private::sorted_sequence<
            template_container, template_compare, true, false>
        base_type;

    //-------------------------------------------------------------------------
    /// @name 構築
    /// @{

    /// @brief 空のソート済シーケンス集合を構築する。
    public: explicit sequence_multiset(
        /// [in] 値を比較する関数オブジェクトの初期値。
        typename base_type::value_compare in_compare = template_compare()):
    base_type(std::move(in_compare))
    {}

    /// @brief ソート済シーケンス集合をコピー構築する。
    /// @warning
    ///   in_container がソートされてなかった場合 *this は空となり、
    ///   std::invalid_argument 例外を投げる。
    public: explicit sequence_multiset(
        /// [in] コピー元となるソート済シーケンスコンテナ。
        typename base_type::container_type const& in_container,
        /// [in] 値を比較する関数オブジェクトの初期値。
        typename base_type::value_compare const& in_compare
            = template_compare()):
    base_type(in_container, in_compare)
    {}

    /// @brief ソート済シーケンス集合をムーブ構築する。
    /// @warning
    ///   io_container がソートされてなかった場合 *this は空となり、
    ///   std::invalid_argument 例外を投げる。
    public: explicit sequence_multiset(
        /// [in] ムーブ元となるソート済シーケンスコンテナ。
        typename base_type::container_type&& io_container,
        /// [in] 値を比較する関数オブジェクトの初期値。
        typename base_type::value_compare&& io_compare = template_compare()):
    base_type(std::move(io_container), std::move(io_compare))
    {}

    /// @brief ソート済シーケンス集合をコピー構築する。
    public: sequence_multiset(
        /// [in] コピー元となるソート済シーケンス集合。
        this_type const& in_source):
    base_type(in_source)
    {}

    /// @brief ソート済シーケンス集合をムーブ構築する。
    public: sequence_multiset(
        /// [in] ムーブ元となるソート済シーケンス集合。
        this_type&& io_source):
    base_type(std::move(io_source))
    {}
    /// @}
}; // class psyq::container::sequence_multiset:

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief ペアの第1要素を比較する関数オブジェクト。
/// @tparam template_pair 比較するペアの型。 std::pair 互換であること。
/// @tparam template_compare
///   ペアの第1要素を比較する関数オブジェクト。以下のメンバ関数が使えること。
///   - bool template_compare::operator()(
///       template_pair::first_type const&,
///       template_pair::first_type const&)
///     const
template<
    typename template_pair,
    typename template_compare>
struct psyq::container::pair_first_compare: public template_compare
{
    /// @copydoc psyq::string::view::this_type
    private: typedef pair_first_compare this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef template_compare base_type;

    //-------------------------------------------------------------------------
    /// @brief 比較関数オブジェクトの第1引数となる要素を持つペアの型。
    public: typedef template_pair first_argument_type;
    /// @brief 比較関数オブジェクトの第2引数となる要素を持つペアの型。
    public: typedef template_pair second_argument_type;
    /// @brief 比較関数オブジェクトの戻り値の型。
    public: typedef bool result_type;

    //-------------------------------------------------------------------------
    /// @brief ペアを比較する。
    /// @return 比較の結果。
    public: bool operator()(
        /// [in] 比較関数オブジェクトの第1引数を持つペア。
        typename this_type::first_argument_type const& in_left,
        /// [in] 比較関数オブジェクトの第2引数を持つペア。
        typename this_type::second_argument_type const& in_right)
    const
    {
        return (*this)(in_left.first, in_right.first);
    }

    /// @brief ペアと要素を比較する。
    /// @return 比較の結果。
    public: bool operator()(
        /// [in] 比較関数オブジェクトの第1引数を持つペア。
        typename this_type::first_argument_type const& in_left,
        /// [in] 比較関数オブジェクトの第2引数。
        typename this_type::second_argument_type::first_type const& in_right)
    const
    {
        return (*this)(in_left.first, in_right);
    }

    /// @brief 要素とペアを比較する。
    /// @return 比較の結果。
    public: bool operator()(
        /// [in] 比較関数オブジェクトの第1引数。
        typename this_type::first_argument_type::first_type const& in_left,
        /// [in] 比較関数オブジェクトの第2引数を持つペア。
        typename this_type::second_argument_type const& in_right)
    const
    {
        return (*this)(in_left, in_right.first);
    }

    /// @brief 要素を比較する。
    /// @return 比較の結果。
    public: bool operator()(
        /// [in] 比較関数オブジェクトの第1引数。
        typename this_type::first_argument_type::first_type const& in_left,
        /// [in] 比較関数オブジェクトの第2引数。
        typename this_type::second_argument_type::first_type const& in_right)
    const
    {
        return static_cast<base_type const&>(*this)(in_left, in_right);
    }

}; // struct psyq::container::pair_first_compare

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 要素の重複を許可しないソート済シーケンスコンテナの選別器。
template<> class psyq::container::_private::sequence_sorter<false>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef sequence_sorter this_type;

    /// @copydoc sorted_sequence::is_sorted
    public: template<typename template_iterator, typename template_compare>
    static bool is_sorted(
        /// [in] コンテナの先頭位置。
        template_iterator const& in_begin,
        /// [in] コンテナの末尾位置。
        template_iterator const& in_end,
        /// [in] コンテナ要素を比較する関数オブジェクト。
        template_compare const& in_compare)
    {
        if (in_begin != in_end)
        {
            auto local_prev(in_begin);
            for (auto i(std::next(in_begin)); i != in_end; ++i)
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

    /// @copydoc sorted_sequence::is_insert_position
    public: template<typename template_container, typename template_compare>
    static bool is_insert_position(
        /// [in] 要素を挿入するコンテナ。
        template_container const& in_container,
        /// [in] 要素を比較する関数オブジェクト。
        template_compare const& in_compare,
        /// [in] 要素を挿入する位置。
        typename template_container::const_iterator const& in_position,
        /// [in] 挿入する要素の初期値。
        typename template_container::value_type const& in_value)
    {
        return (in_position == std::begin(in_container)
                || in_compare(*std::prev(in_position), in_value))
            && (in_position == std::end(in_container)
                || in_compare(in_value, *in_position));
    }

    /// @brief コンテナに要素を挿入する。
    /// @return 要素の位置と挿入したかどうかのペア。
    public: template<
        typename template_container,
        typename template_compare,
        typename template_iterator>
    static std::pair<template_iterator, bool> insert(
        /// [in] 要素を挿入するコンテナ。
        template_container& io_container,
        /// [in] 要素を比較する関数オブジェクト。
        template_compare const& in_compare,
        /// [in] 要素を挿入する位置。
        template_iterator in_position,
        /// [in] 挿入する要素の初期値。
        typename template_container::value_type const& in_value)
    {
        auto const local_insert(
            this_type::is_inserting_position(
                io_container, in_compare, in_position, in_value));
        return std::make_pair(
            local_insert?
                io_container.insert(std::move(in_position), in_value):
                in_position,
            local_insert);
    }

    /// @copydoc insert
    public: template<
        typename template_container,
        typename template_compare,
        typename template_iterator>
    static std::pair<template_iterator, bool> insert(
        /// [in] 要素を挿入するコンテナ。
        template_container& io_container,
        /// [in] 要素を比較する関数オブジェクト。
        template_compare const& in_compare,
        /// [in] 要素を挿入する位置。
        template_iterator in_position,
        /// [in] 挿入する要素の初期値。
        typename template_container::value_type&& io_value)
    {
        auto const local_insert(
            this_type::is_inserting_position(
                io_container, in_compare, in_position, io_value));
        return std::make_pair(
            local_insert?
                static_cast<template_iterator>(
                    io_container.insert(
                        std::move(in_position), std::move(io_value))):
                in_position,
            local_insert);
    }

    private: template<typename template_container, typename template_compare>
    static bool is_inserting_position(
        /// [in] 要素を挿入するコンテナ。
        template_container const& in_container,
        /// [in] 要素を比較する関数オブジェクト。
        template_compare const& in_compare,
        /// [in] 要素を挿入する位置。
        typename template_container::const_iterator const& in_position,
        /// [in] 挿入する要素の初期値。
        typename template_container::value_type const& in_value)
    {
        PSYQ_ASSERT(
             std::begin(in_container) == in_position
             || in_compare(*std::prev(in_position), in_value));
        PSYQ_ASSERT(
             std::end(in_container) == in_position
             || !in_compare(*in_position, in_value));
        return this_type::is_insert_position(
            in_container, in_compare, in_position, in_value);
    }

}; // class psyq::container::_private::sequence_sorter<false>

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 要素の重複を許可するソート済シーケンスコンテナの選別器。
template<> class psyq::container::_private::sequence_sorter<true>
{
    private: typedef sequence_sorter this_type;

    /// @copydoc sorted_sequence::is_sorted
    public: template<typename template_iterator, typename template_compare>
    static bool is_sorted(
        /// [in] コンテナの先頭位置。
        template_iterator const& in_begin,
        /// [in] コンテナの末尾位置。
        template_iterator const& in_end,
        /// [in] コンテナ要素を比較する関数オブジェクト。
        template_compare const& in_compare)
    {
        return std::is_sorted(in_begin, in_end, in_compare);
    }

    /// @copydoc sorted_sequence::is_insert_position
    public: template<typename template_container, typename template_compare>
    static bool is_insert_position(
        /// [in] 要素を挿入するコンテナ。
        template_container const& in_container,
        /// [in] 要素を比較する関数オブジェクト。
        template_compare const& in_compare,
        /// [in] 要素を挿入する位置。
        typename template_container::const_iterator const& in_position,
        /// [in] 挿入する要素の初期値。
        typename template_container::value_type const& in_value)
    {
        return (in_position == std::begin(in_container)
                || !in_compare(in_value, *std::prev(in_position)))
            && (in_position == std::end(in_container)
                || !in_compare(*in_position, in_value));
    }

    /// @brief コンテナに要素を挿入する。
    /// @return 挿入した要素を指す反復子。
    public: template<
        typename template_container,
        typename template_compare,
        typename template_iterator>
    static template_iterator insert(
        /// [in] 要素を挿入するコンテナ。
        template_container& io_container,
        /// [in] 要素を比較する関数オブジェクト。
        template_compare const& in_compare,
        /// [in] 要素を挿入する位置。
        template_iterator in_position,
        /// [in] 挿入する要素の初期値。
        typename template_container::value_type const& in_value)
    {
        PSYQ_ASSERT(
            this_type::is_insert_position(
                io_container, in_compare, in_position, in_value));
        return io_container.insert(std::move(in_position), in_value);
    }

    /// @copydoc insert
    public: template<
        typename template_container,
        typename template_compare,
        typename template_iterator>
    static template_iterator insert(
        /// [in] 要素を挿入するコンテナ。
        template_container& io_container,
        /// [in] 要素を比較する関数オブジェクト。
        template_compare const& in_compare,
        /// [in] 要素を挿入する位置。
        template_iterator in_position,
        /// [in] 挿入する要素の初期値。
        typename template_container::value_type&& io_value)
    {
        PSYQ_ASSERT(
            this_type::is_insert_position(
                io_container, in_compare, in_position, io_value));
        return io_container.insert(
            std::move(in_position), std::move(io_value));
    }

}; // class psyq::container::_private::sequence_sorter<true>

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief ソート済シーケンスコンテナを用いた連想コンテナの基底型。
/// @tparam template_container @copydoc sorted_sequence::container_type
/// @tparam template_compare   @copydoc sorted_sequence::value_compare
/// @tparam template_multi     重複する要素を許可するかの区別。
/// @tparam template_map       辞書か集合かの区別。
template<
    typename template_container,
    typename template_compare,
    bool template_multi,
    bool template_map>
class psyq::container::_private::sorted_sequence: private template_container
{
    /// @copydoc psyq::string::view::this_type
    private: typedef sorted_sequence this_type;
    /// @copydoc psyq::string::view::base_type
    private: typedef template_container base_type;

    //-------------------------------------------------------------------------
    /// @brief 扱うシーケンスコンテナの型。
    /// @details
    ///   以下のメンバ型が使えること。
    ///   - template_container::value_type
    ///   - template_container::size_type
    ///   - template_container::difference_type
    ///   - template_container::reference
    ///   - template_container::pointer
    ///   - template_container::iterator
    ///   - template_container::reverse_iterator
    ///   - template_container::const_reference
    ///   - template_container::const_pointer
    ///   - template_container::const_iterator
    ///   - template_container::const_reverse_iterator
    ///   .
    ///   また、以下に相当するメンバ関数が使えること。
    ///   - bool template_container::empty() const
    ///   - template_container::size_type template_container::size() const
    ///   - template_container::size_type template_container::max_size() const
    ///   - template_container::iterator template_container::begin()
    ///   - template_container::const_iterator template_container::begin() const
    ///   - template_container::iterator template_container::end()
    ///   - template_container::const_iterator template_container::end() const
    ///   - template_container::reverse_iterator template_container::rbegin()
    ///   - template_container::const_reverse_iterator template_container::rbegin() const
    ///   - template_container::reverse_iterator template_container::rend()
    ///   - template_container::const_reverse_iterator template_container::rend() const
    ///   - template_container::const_iterator template_container::cbegin() const
    ///   - template_container::const_iterator template_container::cend() const
    ///   - template_container::const_reverse_iterator template_container::crbegin() const
    ///   - template_container::const_reverse_iterator template_container::crend() const
    ///   - template_container::iterator template_container::insert(
    ///       template_container::const_iterator,
    ///       template_container::value_type)
    ///   - template_container::iterator template_container::erase(
    ///       template_container::const_iterator)
    ///   - void template_container::clear()
    public: typedef template_container container_type;
    /// @brief 値を比較する関数オブジェクト。
    /// @details
    ///   以下に相当するメンバ関数が使えること。
    ///   - bool template_compare::operator()(
    ///       template_container::value_type const&,
    ///       template_container::value_type const&)
    ///     const
    public: typedef template_compare value_compare;
    /// @brief コンテナの要素の型。
    public: typedef typename base_type::value_type value_type;
    /// @brief キーを比較する関数オブジェクト。
    public: typedef template_compare key_compare;
    /// @brief キーの型。
    public: typedef typename base_type::value_type key_type;
    /// @brief コンテナの要素を指す、反復子の型。
    public: typedef
        typename std::conditional<
            template_map,
            typename base_type::iterator,
            typename base_type::const_iterator>::type
        iterator;
    /// @brief コンテナの要素を指す、逆反復子の型。
    public: typedef
        typename std::conditional<
            template_map,
            typename base_type::reverse_iterator,
            typename base_type::const_reverse_iterator>::type
        reverse_iterator;

    //-------------------------------------------------------------------------
    /// @brief ソート済シーケンスコンテナの選別器。
    private: typedef
         psyq::container::_private::sequence_sorter<template_multi>
         sorter;

    /// @brief this_type::sorter::insert の戻り値の型。
    private: typedef
        decltype(
            this_type::sorter::insert(
                *static_cast<base_type*>(nullptr),
                *static_cast<typename this_type::value_compare*>(nullptr),
                *static_cast<typename this_type::iterator*>(nullptr),
                *static_cast<typename base_type::value_type*>(nullptr)))
        insert_result;

    //-------------------------------------------------------------------------
    public: using typename base_type::size_type;
    public: using typename base_type::difference_type;
    public: using typename base_type::reference;
    public: using typename base_type::pointer;
    public: using typename base_type::const_reference;
    public: using typename base_type::const_pointer;
    public: using typename base_type::const_iterator;
    public: using typename base_type::const_reverse_iterator;

    public: using base_type::cbegin;
    public: using base_type::cend;
    public: using base_type::crbegin;
    public: using base_type::crend;
    public: using base_type::size;
    public: using base_type::max_size;
    public: using base_type::empty;
    public: using base_type::clear;

    //-------------------------------------------------------------------------
    /// @name 代入
    /// @{

    /// @brief ソート済シーケンスコンテナをコピー代入する。
    /// @return *this
    public: this_type& operator=(
        /// [in] コピー元となるソート済シーケンスコンテナ。
        this_type const& in_source)
    {
        this->assign(in_source);
        return *this;
    }

    /// @brief ソート済シーケンスコンテナをムーブ代入する。
    /// @return *this
    public: this_type& operator=(
        /// [in] ムーブ元となるソート済シーケンスコンテナ。
        this_type&& io_source)
    {
        this->assign(std::move(io_source));
        return *this;
    }

    /// @brief ソート済シーケンスコンテナをコピー代入する。
    public: void assign(
        /// [in] コピー元となるソート済シーケンスコンテナ。
        this_type const& in_source)
    {
        PSYQ_ASSERT(in_source.is_sorted());
        this->compare_ = in_source.compare_;
        this->get_mutable_container() = in_source;
    }

    /// @brief ソート済シーケンスコンテナをムーブ代入する。
    public: void assign(
        /// [in] ムーブ元となるソート済シーケンスコンテナ。
        this_type&& io_source)
    {
        PSYQ_ASSERT(io_source.is_sorted());
        this->compare_ = std::move(io_source.compare_);
        this->get_mutable_container() = std::move(io_source);
    }

    /// @brief ソート済シーケンスコンテナをコピー代入する。
    /// @retval true  成功。 in_source を *this へ代入した。
    /// @retval false 失敗。 in_source がソートされてなかった。 *this は変化しない。
    public: bool assign(
        /// [in] コピー元となるソート済シーケンスコンテナ。
        typename this_type::container_type const& in_source)
    {
        auto const local_sorted(
            this_type::sorter::is_sorted(
                std::begin(in_source),
                std::end(in_source),
                this->value_comp()));
        if (local_sorted)
        {
            this->get_mutable_container() = in_source;
        }
        return local_sorted;
    }

    /// @brief ソート済シーケンスコンテナをムーブ代入する。
    /// @retval true  成功。 io_source を *this へ代入した。
    /// @retval false 失敗。 io_source がソートされてなかった。 *this は変化しない。
    public: bool assign(
        /// [in] ムーブ元となるソート済シーケンスコンテナ。
        typename this_type::container_type&& io_source)
    {
        auto const local_sorted(
            this_type::sorter::is_sorted(
                std::begin(io_source),
                std::end(io_source),
                this->value_comp()));
        if (local_sorted)
        {
            this->get_mutable_container() = std::move(io_source);
        }
        return local_sorted;
    }

    /// @copydoc assign(typename this_type::container_type const&)
    public: template<typename template_iterator>
    bool assign(
        /// [in] コピー元となるソート済シーケンスコンテナの先頭位置。
        template_iterator in_begin,
        /// [in] コピー元となるソート済シーケンスコンテナの末尾位置。
        template_iterator in_end)
    {
        auto const local_sorted(
            this_type::sorter::is_sorted(
                in_begin, in_end, this->value_comp()));
        if (local_sorted)
        {
            this->base_type::assign(std::move(in_begin), std::move(in_end));
        }
        return local_sorted;
    }

    /// @brief ソート済シーケンスコンテナを交換する。
    public: void swap(
        /// [in,out] 交換するソート済シーケンスコンテナ。
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
        this_type const& in_right)
    const
    {
        return this->get_container() == in_right.get_container();
    }

    /// @brief 不等価なコンテナか判定する。
    /// @return *this != in_right
    public: bool operator!=(
        /// [in] 比較演算子の右辺となるコンテナ。
        this_type const& in_right)
    const
    {
        return this->get_container() != in_right.get_container();
    }

    /// @brief より大きいコンテナか判定する。
    /// @return *this < in_right
    public: bool operator<(
        /// [in] 比較演算子の右辺となるコンテナ。
        this_type const& in_right)
    const
    {
        return this->get_container() < in_right.get_container();
    }

    /// @brief より大きい、または等価なコンテナか判定する。
    /// @return *this <= in_right
    public: bool operator<=(
        /// [in] 比較演算子の右辺となるコンテナ。
        this_type const& in_right)
    const
    {
        return this->get_container() <= in_right.get_container();
    }

    /// @brief より小さいコンテナか判定する。
    /// @return *this > in_right
    public: bool operator>(
        /// [in] 比較演算子の右辺となるコンテナ。
        this_type const& in_right)
    const
    {
        return this->get_container() > in_right.get_container();
    }

    /// @brief より小さい、または等価なコンテナか判定する。
    /// @return *this >= in_right
    public: bool operator>=(
        /// [in] 比較演算子の右辺となるコンテナ。
        this_type const& in_right)
    const
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

    /// @brief シーケンスコンテナを取得する。
    /// @return *this が使用しているシーケンスコンテナ。
    public: typename this_type::container_type const& get_container()
    const PSYQ_NOEXCEPT
    {
        return *this;
    }

    /// @brief シーケンスコンテナを取り出し、 *this を空にする。
    /// @return *this が使用していたシーケンスコンテナ。
    public: typename this_type::container_type remove_container()
    {
        auto const local_container(std::move(static_cast<base_type&>(*this)));
        this->clear();
        return local_container;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 反復子の取得
    /// @{

    /// @brief コンテナの先頭要素を指す反復子を取得する。
    /// @return *this の先頭要素を指す反復子。
    public: typename this_type::iterator begin()
    {
        return std::begin(this->get_mutable_container());
    }

    /// @copydoc begin
    public: typename base_type::const_iterator begin() const
    {
        return std::begin(this->get_container());
    }

    /// @brief コンテナの末尾要素の次を指す反復子を取得する。
    /// @return *this の末尾要素の次を指す反復子。
    public: typename this_type::iterator end()
    {
        return std::end(this->get_mutable_container());
    }

    /// @copydoc end
    public: typename base_type::const_iterator end() const
    {
        return std::end(this->get_container());
    }

    /// @brief コンテナの先頭要素の前を指す逆反復子を取得する。
    /// @return *this の先頭要素の前を指す逆反復子。
    public: typename this_type::reverse_iterator rbegin()
    {
        return this->base_type::rbegin();
    }

    /// @copydoc rbegin
    public: typename base_type::const_reverse_iterator rbegin() const
    {
        return this->base_type::rbegin();
    }

    /// @brief コンテナの末尾要素を指す逆反復子を取得する。
    /// @return *this の末尾要素を指す逆反復子。
    public: typename this_type::reverse_iterator rend()
    {
        return this->base_type::rend();
    }

    /// @copydoc rend
    public: typename base_type::const_reverse_iterator rend() const
    {
        return this->base_type::rend();
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 要素の検索
    /// @{

    /// @brief 指定されたキーの位置を取得する。
    /// @return in_key に対応する要素が最初に現れる位置。
    public: template<typename template_key>
    typename this_type::iterator find(
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
    typename this_type::iterator lower_bound(
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
    typename this_type::iterator upper_bound(
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
    std::pair<typename this_type::iterator, typename this_type::iterator>
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

    /// @brief コンテナがソートされているか判定する。
    /// @retval true  コンテナはソートされている。
    /// @retval false コンテナはソートされていない。
    public: template<typename template_iterator>
    static bool is_sorted(
        /// [in] コンテナの先頭位置。
        template_iterator const& in_begin,
        /// [in] コンテナの末尾位置。
        template_iterator const& in_end,
        /// [in] 値を比較する関数オブジェクト。
        typename this_type::value_compare const& in_compare = value_compare())
    {
        return this_type::sorter::is_sorted(in_begin, in_end, in_compare);
    }

    //-------------------------------------------------------------------------
    /// @name 要素の挿入
    /// @{

    /// @brief 要素を挿入する。
    /// @return 挿入した要素を指す反復子。
    public: typename this_type::insert_result insert(
        /// [in] 挿入する要素の初期値。
        typename base_type::value_type const& in_value)
    {
        return this_type::sorter::insert(
            this->get_mutable_container(),
            this->value_comp(),
            this->find_insert_position(in_value),
            in_value);
    }

    /// @brief 要素を挿入する。
    /// @return 挿入した要素を指す反復子。
    public: typename this_type::insert_result insert(
        /// [in] 挿入する要素の初期値。
        typename base_type::value_type&& io_value)
    {
        return this_type::sorter::insert(
            this->get_mutable_container(),
            this->value_comp(),
            this->find_insert_position(io_value),
            std::move(io_value));
    }

    /// @brief 要素を挿入する。
    /// @return 挿入した要素を指す反復子。
    public: typename this_type::iterator insert(
        /// [in] 要素を挿入する位置のヒント。
        typename base_type::const_iterator in_position,
        /// [in] 挿入する要素の初期値。
        typename base_type::value_type const& in_value)
    {
        return this->base_type::insert(
            this->is_insert_position(in_position, in_value)?
                std::move(in_position): this->find_insert_position(in_value),
            in_value);
    }

    /// @brief 要素を挿入する。
    /// @return 挿入した要素を指す反復子。
    public: typename this_type::iterator insert(
        /// [in] 要素を挿入する位置のヒント。
        typename base_type::const_iterator in_position,
        /// [in] 挿入する要素の初期値。
        typename base_type::value_type&& io_value)
    {
        return this->base_type::insert(
            this->is_insert_position(in_position, io_value)?
                std::move(in_position): this->find_insert_position(io_value),
            std::move(io_value));
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
    public: typename this_type::iterator erase(
        /// [in] 削除する要素を指す反復子。
        typename base_type::const_iterator in_position)
    {
        return this->base_type::erase(std::move(in_position));
    }

    /// @brief 要素を削除する。
    /// @return 削除した範囲の次の要素を指す反復子。
    public: typename this_type::iterator erase(
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
    /// @name 構築
    /// @{

    /// @brief 空のソート済シーケンスコンテナを構築する。
    protected: explicit sorted_sequence(
        /// [in] 値を比較する関数オブジェクト。
        typename this_type::value_compare&& io_compare):
    base_type(), compare_(std::move(io_compare))
    {}

    /// @brief ソート済シーケンスコンテナをコピー構築する。
    /// @warning
    ///   in_container がソートされてなかった場合 *this は空となり、
    ///   std::invalid_argument 例外を投げる。
    protected: sorted_sequence(
        /// [in] コピー元となるソート済シーケンスコンテナ。
        typename this_type::container_type const& in_container,
        /// [in] 値を比較する関数オブジェクト。
        typename this_type::value_compare const& in_compare):
    base_type(in_container),
    compare_(in_compare)
    {
        if (!this->is_sorted())
        {
            this->clear();
            PSYQ_ASSERT_THROW(false, std::invalid_argument);
        }
    }

    /// @brief ソート済シーケンスコンテナをムーブ構築する。
    /// @warning
    ///   io_container がソートされてなかった場合 *this は空となり、
    ///   std::invalid_argument 例外を投げる。
    protected: sorted_sequence(
        /// [in] ムーブ元となるソート済シーケンスコンテナ。
        typename this_type::container_type&& io_container,
        /// [in] 値を比較する関数オブジェクト。
        typename this_type::value_compare&& io_compare):
    base_type(std::move(io_container)),
    compare_(std::move(io_compare))
    {
        if (!this->is_sorted())
        {
            io_container = std::move(this->get_mutable_container());
            this->clear();
            PSYQ_ASSERT_THROW(false, std::invalid_argument);
        }
    }

    /// @brief ソート済シーケンスコンテナをコピー構築する。
    protected: sorted_sequence(
        /// [in] コピー元となるソート済シーケンスコンテナ。
        this_type const& in_source):
    base_type((PSYQ_ASSERT(in_source.is_sorted()), in_source)),
    compare_(in_source.value_comp())
    {}

    /// @brief ソート済シーケンスコンテナをムーブ構築する。
    protected: sorted_sequence(
        /// [in] ムーブ元となるソート済シーケンスコンテナ。
        this_type&& io_source):
    base_type((PSYQ_ASSERT(io_source.is_sorted()), std::move(io_source))),
    compare_(std::move(io_source.compare_))
    {}
    /// @}
    //-------------------------------------------------------------------------
    /// @copydoc get_container
    private: typename this_type::container_type& get_mutable_container()
    PSYQ_NOEXCEPT
    {
        return *this;
    }

    /// @copydoc is_sorted
    private: bool is_sorted() const
    {
        return this_type::is_sorted(
            std::begin(*this), std::end(*this), this->value_comp());
    }

    /// @brief 要素を挿入するのに適切な位置か判定する。
    /// @retval true  in_position は in_value を挿入するのに適切な位置。
    /// @retval false in_position は in_value を挿入するのに適切な位置ではない。
    private: bool is_insert_position(
        /// [in] 挿入する位置。
        typename base_type::const_iterator const& in_position,
        /// [in] 挿入する値。
        typename base_type::value_type const& in_value)
    const
    {
        return this_type::sorter::is_insert_position(
            this->get_container(), this->value_comp(), in_position, in_value);
    }

    private: typename this_type::iterator find_insert_position(
        /// [in] 挿入する値。
        typename base_type::value_type const& in_value)
    {
        return template_multi?
            this->upper_bound(in_value): this->lower_bound(in_value);
    }

    /// @copydoc find
    private: template<typename template_sorted_sequence, typename template_key>
    static auto find(
        /// [in] 要素を検索するソート済シーケンスコンテナ。
        template_sorted_sequence& in_container,
        /// [in] 基準となるキー。
        template_key const& in_key)
    ->decltype(std::begin(in_container))
    {
        auto const local_end(std::end(in_container));
        auto const local_lower_bound(
            this_type::lower_bound(in_container, in_key));
        return local_lower_bound != local_end
            && !in_container.compare_(in_key, *local_lower_bound)?
                local_lower_bound: local_end;
    }

    /// @copydoc lower_bound
    private: template<typename template_sorted_sequence, typename template_key>
    static auto lower_bound(
        /// [in] 要素を検索するソート済シーケンスコンテナ。
        template_sorted_sequence& in_container,
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
    private: template<typename template_sorted_sequence, typename template_key>
    static auto upper_bound(
        /// [in] 要素を検索するソート済シーケンスコンテナ。
        template_sorted_sequence& in_container,
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
    private: template<typename template_sorted_sequence, typename template_key>
    static auto equal_range(
        /// [in] 要素を検索するソート済シーケンスコンテナ。
        template_sorted_sequence& in_container,
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

}; // class psyq::container::sorted_sequence

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    inline void sorted_sequence()
    {
        typedef psyq::container::sequence_set<std::vector<int>> set;
        set::container_type local_sorted_container;
        local_sorted_container.reserve(32);
        set local_set(local_sorted_container);
        PSYQ_ASSERT(local_set.assign(std::move(local_sorted_container)));
        local_set.insert(local_set.begin(), 30);
        PSYQ_ASSERT(local_set.insert(10).second);
        PSYQ_ASSERT(local_set.insert(20).second);
        PSYQ_ASSERT(!local_set.insert(10).second);
        PSYQ_ASSERT(local_set.count(10) == 1);
        PSYQ_ASSERT(local_set.count(15) == 0);
        PSYQ_ASSERT(local_set.assign(local_set.get_container()));
        PSYQ_ASSERT(local_set.assign(local_set.remove_container()));
        local_set.size();
    }
}

#endif // !defined(PSYQ_CONTAINER_SORTED_SEQUENCE_HPP_)
// vim: set expandtab:
