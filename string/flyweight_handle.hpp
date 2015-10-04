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
/// @brief @copybrief psyq::string::_private::flyweight_handle
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_STRING_FLYWEIGHT_HANDLE_HPP_
#define PSYQ_STRING_FLYWEIGHT_HANDLE_HPP_

#include "./flyweight_factory.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief フライ級文字列の参照子の基底型。
/// @tparam template_hasher    @copydoc flyweight_factory::hasher
/// @tparam template_allocator @copydoc flyweight_factory::allocator_type
/// @tparam template_weak      弱参照かどうか。
template<typename template_hasher, typename template_allocator, bool template_weak>
class psyq::string::_private::flyweight_handle
{
    /// @copydoc psyq::string::view::this_type
    private: typedef flyweight_handle this_type;

    //-------------------------------------------------------------------------
    /// @brief フライ級文字列の生成器を表す型。
    public: typedef
        psyq::string::_private::flyweight_factory<template_hasher, template_allocator>
        factory;
    /// @brief フライ級文字列で使う文字列参照の型。
    public: typedef typename this_type::factory::hasher::argument_type view;
    /// @brief 文字特性の型。
    public: typedef typename this_type::view::traits_type traits_type;
    /// @copydoc psyq::string::_private::flyweight_factory::hasher
    public: typedef typename this_type::factory::hasher hasher;
    /// @brief メモリ割当子の型。
    public: typedef typename this_type::factory::allocator_type allocator_type;

    //-------------------------------------------------------------------------
    /// @brief フライ級文字列の生成器を指すスマートポインタ。
    protected: typedef
         typename std::conditional<
             template_weak,
             typename this_type::factory::weak_ptr,
             typename this_type::factory::shared_ptr>
         ::type
         factory_smart_ptr;

    //-------------------------------------------------------------------------
    /// @name 代入と解体
    /// @{

    /// @brief フライ級文字列の参照権を破棄する。
    public: ~flyweight_handle()
    {
        this->release_string();
    }

    /// @brief コピー代入演算子。
    /// @return *this
    public: this_type& operator=(
        /// [in] コピー元となるインスタンス。
        this_type const& in_source)
    {
        if (&in_source != this)
        {
            this->release_string();
            this->factory_ = in_source.get_factory();
            this->string_ = this_type::factory::hold_string(
                in_source.get_factory(), in_source.string_);
        }
        return *this;
    }

    /// @brief ムーブ代入演算子。
    /// @return *this
    public: this_type& operator=(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source)
    {
        if (&io_source != this)
        {
            this->release_string();
            // C++11では weak_ptr のムーブ代入がないので swap で対処。
            this->factory_.reset();
            this->factory_.swap(io_source.factory_);
            this->string_ = io_source.string_;
        }
        return *this;
    }

    /// @brief フライ級文字列の参照権を破棄し、参照子を空にする。
    public: void clear() PSYQ_NOEXCEPT
    {
        this->release_string();
        this->factory_.reset();
        this->string_ = nullptr;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 文字列のプロパティ
    /// @{

    /// @copydoc psyq::string::view::data
    public: typename this_type::traits_type::char_type const* data()
    const PSYQ_NOEXCEPT
    {
        return this->empty()? nullptr: this->string_->data();
    }

    /// @copydoc psyq::string::view::size
    public: typename this_type::view::size_type size() const PSYQ_NOEXCEPT
    {
        return this->empty()? 0: this->string_->size_;
    }

    /// @copydoc psyq::string::view::max_size
    public: std::size_t max_size() const PSYQ_NOEXCEPT
    {
        return (std::numeric_limits<std::size_t>::max)();
    }

    /// @copydoc psyq::string::view::empty
    public: bool empty() const
    {
        return this_type::factory::is_empty(this->get_factory(), this->string_);
    }

    /// @brief 参照しているフライ級文字列のハッシュ値を取得する。
    /// @return 参照しているフライ級文字列のハッシュ値。
    public: typename this_type::hasher::result_type get_hash()
    const PSYQ_NOEXCEPT
    {
        return this->empty()?
            this_type::factory::string_placeholder::get_empty_hash():
            this->string_->hash_;
    }

    /// @brief 参照しているフライ級文字列の生成器を取得する。
    /// @return 参照しているフライ級文字列の生成器。
    public: typename this_type::factory_smart_ptr const& get_factory()
    const PSYQ_NOEXCEPT
    {
        return this->factory_;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 文字列の比較
    /// @{

    /// @brief ハッシュ値を基準に文字列を比較する。
    /// @retval 負 右辺のほうが大きい。
    /// @retval 正 左辺のほうが大きい。
    /// @retval 0  左辺と右辺は等価。
    public: int compare_fast(
        /// [in] 右辺の文字列。
        this_type const& in_right)
    const PSYQ_NOEXCEPT
    {
        return this_type::_compare_fast(
            *this, this->get_hash(), in_right, in_right.get_hash());
    }

    /// @copydoc compare_fast
    public: int compare_fast(
        /// [in] 右辺の文字列。
        typename this_type::view const& in_right)
    const PSYQ_NOEXCEPT
    {
        return this_type::_compare_fast(
            *this,
            this->get_hash(),
            in_right,
            this_type::factory::_compute_hash(in_right));
    }

    /// @brief ハッシュ値を基準に文字列を比較する。
    /// @warning psyq::string 管理者以外は、この関数は使用禁止。
    /// @retval 負 右辺のほうが大きい。
    /// @retval 正 左辺のほうが大きい。
    /// @retval 0  左辺と右辺は等価。
    public: int _compare_fast(
        /// [in] 右辺の文字列。
        typename this_type::view const& in_right_string,
        /// [in] 右辺の文字列のハッシュ値。
        typename this_type::hasher::result_type const in_right_hash)
    const PSYQ_NOEXCEPT
    {
        return this_type::_compare_fast(
            *this, this->get_hash(), in_right_string, in_right_hash);
    }
    /// @}

    /// @copydoc _compare_fast
    public: static int _compare_fast(
        /// [in] 左辺の文字列。
        typename this_type::view const& in_left,
        /// [in] 右辺の文字列。
        typename this_type::view const& in_right)
    PSYQ_NOEXCEPT
    {
        return this_type::_compare_fast(
            in_left,
            this_type::factory::_compute_hash(in_left),
            in_right,
            this_type::factory::_compute_hash(in_right));
    }

    /// @copydoc _compare_fast
    public: static int _compare_fast(
        /// [in] 左辺の文字列。
        typename this_type::view const& in_left_string,
        /// [in] 左辺の文字列のハッシュ値。
        typename this_type::hasher::result_type const in_left_hash,
        /// [in] 右辺の文字列。
        typename this_type::view const& in_right_string,
        /// [in] 右辺の文字列のハッシュ値。
        typename this_type::hasher::result_type const in_right_hash)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(
            in_right_hash == this_type::factory::_compute_hash(in_right_string)
            && in_left_hash == this_type::factory::_compute_hash(in_left_string));
        if (in_left_hash != in_right_hash)
        {
            return in_left_hash < in_right_hash? -1: 1;
        }
        auto const local_left_size(in_left_string.size());
        auto const local_right_size(in_right_string.size());
        if (local_left_size != local_right_size)
        {
            return local_left_size < local_right_size? -1: 1;
        }
        return this_type::view::traits_type::compare(
            in_left_string.data(), in_right_string.data(), local_right_size);
    }

    //-------------------------------------------------------------------------
    /// @brief 空の参照子を構築する。
    protected: flyweight_handle() PSYQ_NOEXCEPT: string_(nullptr) {}

    /// @brief コピー構築子。
    protected: flyweight_handle(
        /// [in] コピー元となるインスタンス。
        this_type const& in_source)
    PSYQ_NOEXCEPT:
    factory_(in_source.get_factory()),
    string_(
        this_type::factory::hold_string(
            in_source.get_factory(), in_source.string_))
    {}

    /// @brief ムーブ構築子。
    protected: flyweight_handle(
        /// [in] ムーブ元となるインスタンス。
        this_type&& io_source)
    PSYQ_NOEXCEPT: string_(std::move(io_source.string_))
    {
        // C++11では weak_ptr のムーブ構築がないので swap で対処。
        this->factory_.swap(io_source.factory_);
    }

    /// @brief 空の参照子を作る。
    protected: static this_type make() PSYQ_NOEXCEPT
    {
        return this_type();
    }

    /// @brief フライ級文字列を用意して参照する。
    protected: static this_type make(
        /// [in] フライ級文字列の生成器。
        typename this_type::factory_smart_ptr const& in_factory,
        /// [in] 参照する文字列と等価な文字列。
        typename this_type::view const& in_string,
        /// [in] 文字列チャンクを生成する場合の、デフォルトのチャンク容量。
        std::size_t const in_chunk_size)
    {
        auto const& local_factory(
            this_type::factory::get_shared_ptr(in_factory));
        return in_string.empty() || local_factory.get() == nullptr?
            this_type():
            this_type(
                in_factory,
                this_type::factory::hold_string(
                    local_factory,
                    &local_factory->equip_string(in_string, in_chunk_size)));
    }

    //-------------------------------------------------------------------------
    /// @brief フライ級文字列を参照する。
    private: flyweight_handle(
        /// [in] フライ級文字列を保持している生成器。
        typename this_type::factory_smart_ptr const& in_factory,
        /// [in] 参照するフライ級文字列。
        typename this_type::factory::string_placeholder* const in_string)
    PSYQ_NOEXCEPT:
    factory_(in_factory),
    string_(in_string)
    {}

    /// @brief フライ級文字列の参照権を破棄する。
    private: void release_string()
    {
        this_type::factory::release_string(this->get_factory(), this->string_);
    }

    //-------------------------------------------------------------------------
    /// @brief 参照しているフライ級文字列の生成器。
    private: typename this_type::factory_smart_ptr factory_;
    /// @brief 参照しているフライ級文字列を指すポインタ。
    private: typename this_type::factory::string_placeholder* string_;

}; // class psyq::string::_private::flyweight_handle

#endif // !defined(PSYQ_STRING_FLYWEIGHT_HANDLE_HPP_)
// vim: set expandtab:
