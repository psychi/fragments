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
/// @brief @copybrief psyq::string::_private::flyweight_placeholder
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_STRING_FLYWEIGHT_PLACEHOLDER_HPP_
#define PSYQ_STRING_FLYWEIGHT_PLACEHOLDER_HPP_

#include "../atomic_count.hpp"

// @copydoc psyq::string::_private::flyweight_placeholder::data_
#if defined(NDEBUG) && !defined(PSYQ_STRING_FLYWEIGHT_PLACEHOLDER_NO_VIEW)
#define PSYQ_STRING_FLYWEIGHT_PLACEHOLDER_NO_VIEW
#endif // defined(NDEBUG) || !defined(PSYQ_STRING_FLYWEIGHT_PLACEHOLDER_HPP_)

namespace psyq
{
    namespace string
    {
        namespace _private
        {
            /// @cond
            template<typename> class flyweight_placeholder;
            /// @endcond
        } // namespace
    } // namespace string
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief フライ級文字列を格納する領域。
/// @tparam template_hasher @copydoc flyweight_factory::hasher
template<typename template_hasher>
class psyq::string::_private::flyweight_placeholder
{
    /// @brief thisが指す値の型。
    private: typedef flyweight_placeholder this_type;

    //-------------------------------------------------------------------------
    /// @brief フライ級文字列を比較する関数オブジェクト。
    public: struct less
    {
        bool operator()(
            flyweight_placeholder const& in_left,
            flyweight_placeholder const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.hash_ != in_right.hash_?
                in_left.hash_ < in_right.hash_: in_left.size_ < in_right.size_;
        }

        bool operator()(
            flyweight_placeholder const* const in_left,
            flyweight_placeholder const& in_right)
        const PSYQ_NOEXCEPT
        {
            return this->operator()(*in_left, in_right);
        }

        bool operator()(
            flyweight_placeholder const& in_left,
            flyweight_placeholder const* const in_right)
        const PSYQ_NOEXCEPT
        {
            return this->operator()(in_left, *in_right);
        }

        bool operator()(
            flyweight_placeholder const* const in_left,
            flyweight_placeholder const* const in_right)
        const PSYQ_NOEXCEPT
        {
            return this->operator()(*in_left, *in_right);
        }

    }; // struct less

    //-------------------------------------------------------------------------
    /// @brief 文字列チャンク連結リストのノードの型。
    public: class chunk
    {
        private: typedef chunk this_type;

        /// @brief 文字列チャンクを構築する。
        public: chunk(
            /// [in] リストで次の要素となる文字列チャンクを指すポインタ。
            this_type* const in_next_chunk,
            /// [in] 文字列チャンクの空き容量。
            std::size_t const in_capacity)
        PSYQ_NOEXCEPT:
        next_chunk_(in_next_chunk),
        capacity_((
            PSYQ_ASSERT(sizeof(this_type) < in_capacity), in_capacity)),
        front_string_(
            this_type::compute_string_capacity(in_capacity),
            flyweight_placeholder::get_empty_hash())
        {}

        /// @brief 空の文字列チャンクか判定する。
        public: bool is_empty() const PSYQ_NOEXCEPT
        {
            return this->front_string_.hash_
                   == flyweight_placeholder::get_empty_hash()
                && this->front_string_.size_
                   == this_type::compute_string_capacity(this->capacity_);
        }

        /// @brief 文字列チャンクの空き容量から、文字列の空き容量を算出する。
        private: static std::size_t compute_string_capacity(
            /// [in] 文字列チャンクの空き容量。
            std::size_t const in_chunk_capacity)
        PSYQ_NOEXCEPT
        {
            return (in_chunk_capacity - sizeof(this_type)) /
                sizeof(typename template_hasher::argument_type::value_type);
        }

        /// @brief 次の文字列チャンクを指すポインタ。
        public: this_type* next_chunk_;
        /// @brief チャンクの大きさ。
        public: std::size_t capacity_;
        /// @brief この文字列チャンクでの最初の文字列。
        public: flyweight_placeholder front_string_;

    }; // class chunk

    //-------------------------------------------------------------------------
    /// @brief 文字列を構築する。
    public: flyweight_placeholder(
        /// [in] this_type::size_ の初期値。
        typename template_hasher::argument_type::size_type const in_size,
        /// [in] this_type::hash_ の初期値。
        typename template_hasher::result_type const in_hash)
    PSYQ_NOEXCEPT:
    reference_count_(0),
    size_(in_size),
    hash_(in_hash)
#ifndef PSYQ_STRING_FLYWEIGHT_PLACEHOLDER_NO_VIEW
    , data_(this->data())
#endif // !defined(PSYQ_STRING_FLYWEIGHT_PLACEHOLDER_NO_VIEW)
    {}

    /// @brief 文字列の先頭位置を取得する。
    /// @return 文字列の先頭位置。
    public: typename template_hasher::argument_type::const_pointer data()
    const PSYQ_NOEXCEPT
    {
        return reinterpret_cast<
            typename template_hasher::argument_type::const_pointer>(this + 1);
    }

    /// @brief 空文字列のハッシュ値を取得する。
    /// @return 空文字列のハッシュ値。
    public: static typename template_hasher::result_type get_empty_hash()
    {
        static auto const static_empty_hash(
            template_hasher().operator()(
                typename template_hasher::argument_type()));
        return static_empty_hash;
    }

    //-------------------------------------------------------------------------
    /// @brief 文字列の被参照数。
    public: psyq::atomic_count reference_count_;
    /// @brief 文字列の要素数。
    public: typename template_hasher::argument_type::size_type size_;
    /// @brief 文字列のハッシュ値。
    public: typename template_hasher::result_type hash_;
#ifndef PSYQ_STRING_FLYWEIGHT_PLACEHOLDER_NO_VIEW
    /// @brief デバッグ時に文字列の内容を確認するのに使う。
    private: typename template_hasher::argument_type::const_pointer data_;
#endif // !defined(PSYQ_STRING_FLYWEIGHT_PLACEHOLDER_NO_VIEW)

}; // class psyq::string::_private::flyweight_placeholder

#endif // !defined(PSYQ_STRING_FLYWEIGHT_PLACEHOLDER_HPP_)
// vim: set expandtab:
