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
    @brief @copybrief psyq::string::storage
 */
#ifndef PSYQ_STRING_STORAGE_HPP_
#define PSYQ_STRING_STORAGE_HPP_

#include<type_traits>
//#include "string/view.hpp"

/// psyq::string::storage で使う、defaultの最大文字数。
#ifndef PSYQ_STRING_STORAGE_MAX_SIZE_DEFAULT
#define PSYQ_STRING_STORAGE_MAX_SIZE_DEFAULT 160
#endif // !defined(PSYQ_STRING_STORAGE_MAX_SIZE_DEFAULT)

/// @cond
namespace psyq
{
    namespace string
    {
        template<
            typename template_char_type,
            std::size_t = PSYQ_STRING_STORAGE_MAX_SIZE_DEFAULT,
            typename = PSYQ_STRING_VIEW_TRAITS_DEFAULT>
                class storage;

        namespace _private
        {
            template<typename, std::size_t> class storage_base;
        } // namespace _private
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 文字の固定長領域を使った文字列の基底型。

    - memory割り当てを一切行わない。
    - 文字列の終端がnull文字となっている保証はない。

    @warning this_type::MAX_SIZE より多い文字数は保持できない。

    @tparam template_char_traits @copydoc this_type::traits_type
    @tparam template_max_size    @copydoc this_type::MAX_SIZE
 */
template<typename template_char_traits, std::size_t template_max_size>
class psyq::string::_private::storage_base
{
    private: typedef storage_base this_type; ///< thisが指す値の型。

    /// 文字特性の型。
    public: typedef template_char_traits traits_type;
    static_assert(
        std::is_pod<typename this_type::traits_type::char_type>::value,
        "this_type::traits_type::char_type is not POD type.");
    /// 文字列を格納する領域。
    private: typedef typename std::aligned_storage<
        sizeof(typename this_type::traits_type::char_type) * template_max_size,
        std::alignment_of<typename this_type::traits_type::char_type>::value>
            ::type storage_type;

    public: enum: std::size_t
    {
        MAX_SIZE = template_max_size, ///< 最大の要素数。
    };

    //-------------------------------------------------------------------------
    /// @brief 空の文字列を構築する。
    private: PSYQ_CONSTEXPR storage_base() PSYQ_NOEXCEPT: size_(0) {}

    /** @brief 文字列をコピーする。
        @param[in] in_string コピー元となる文字列。
     */
    protected: storage_base(this_type const& in_string) PSYQ_NOEXCEPT
    {
        this->copy_string(in_string.data(), in_string.size());
    }

    /** @copydoc storage_base(this_type const&)
        @return *this
     */
    protected: this_type& operator=(this_type const& in_string) PSYQ_NOEXCEPT
    {
        return *new(this) this_type(in_string);
    }

    /** @brief 空の文字列を構築する。
        @return 空の文字列。
     */
    protected: static PSYQ_CONSTEXPR this_type make() PSYQ_NOEXCEPT
    {
        return this_type();
    }

    //-------------------------------------------------------------------------
    /// @name 文字列のプロパティ
    //@{
    /// @copydoc psyq::string::view::data()
    public: PSYQ_CONSTEXPR typename this_type::traits_type::char_type const* data()
    const PSYQ_NOEXCEPT
    {
        return reinterpret_cast<typename this_type::traits_type::char_type const*>(
            &this->storage_);
    }

    /// @copydoc psyq::string::view::size()
    public: PSYQ_CONSTEXPR std::size_t size() const PSYQ_NOEXCEPT
    {
        return this->size_;
    }

    /// @copydoc psyq::string::view::max_size()
    public: PSYQ_CONSTEXPR std::size_t max_size() const PSYQ_NOEXCEPT
    {
        return this_type::MAX_SIZE;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の変更
    //@{
    /// @brief 文字列を空にする。
    public: void clear() PSYQ_NOEXCEPT
    {
        this->size_ = 0;
    }
    //@}
    /** @brief 文字列をコピーする。
        @param[in] in_data コピー元となる文字列の先頭位置。
        @param[in] in_size コピー元となる文字列の要素数。
     */
    protected: void copy_string(
        typename traits_type::char_type const* const in_data,
        std::size_t const in_size)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_size <= this_type::MAX_SIZE);
        auto const local_size(
            (std::min<std::size_t>)(in_size, this_type::MAX_SIZE));
        auto const local_data(
            const_cast<typename this_type::traits_type::char_type*>(this->data()));
        this->size_ = local_size;
        this_type::traits_type::copy(local_data, in_data, local_size);
    }

    //-------------------------------------------------------------------------
    /// 文字列を保存する領域。
    private: typename this_type::storage_type storage_;
    /// 文字列の要素数。
    private: std::size_t size_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string_view を模した、固定長領域を使った文字列。

    - memory割り当てを一切行わない。
    - 文字列の終端がnull文字となっている保証はない。

    @warning base_type::MAX_SIZE より多い文字数は保持できない。

    @note
        せっかくインスタンス毎に個別のメモリ領域を持っているので、
        std::stringと同じように変更できる文字列を扱えるようにしたい。

    @tparam template_char_type   @copydoc base_type::value_type
    @tparam template_max_size    @copydoc base_type::MAX_SIZE
    @tparam template_char_traits @copydoc base_type::traits_type
 */
template<
    typename template_char_type,
    std::size_t template_max_size,
    typename template_char_traits>
class psyq::string::storage:
    public psyq::string::_private::view_interface<
        psyq::string::_private::storage_base<
            template_char_traits, template_max_size>>
{
    /// thisが指す値の型。
    private: typedef storage this_type;
    /// this_type の基底型。
    public: typedef psyq::string::_private::view_interface<
        psyq::string::_private::storage_base<
            template_char_traits, template_max_size>>
                base_type;

    //-------------------------------------------------------------------------
    /// @name コンストラクタ
    //@{
    /** @brief 空文字列を構築する。
     */
    public: PSYQ_CONSTEXPR storage() PSYQ_NOEXCEPT:
        base_type(base_type::base_type::make())
    {}

    /** @brief 文字列をコピーする。
        @param[in] in_string コピー元の文字列。
     */
    public: storage(this_type const& in_string) PSYQ_NOEXCEPT:
        base_type(in_string)
    {}

    /** @brief 文字列をコピーする。
        @param[in] in_string コピー元の文字列。
     */
    public: storage(typename base_type::view const& in_string) PSYQ_NOEXCEPT:
        base_type(base_type::base_type::make())
    {
        this->copy_string(in_string.data(), in_string.size());
    }

    /** @brief 文字列をコピーする。
        @param[in] in_data コピー元の文字列の先頭位置。
        @param[in] in_size コピー元の文字列の要素数。
     */
    public: storage(
        typename base_type::const_pointer const in_data,
        typename base_type::size_type const in_size)
    PSYQ_NOEXCEPT:
        base_type(base_type::base_type::make())
    {
        this->copy_string(in_data, in_size);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の代入
    //@{
    /** @copydoc storage(this_type const&)
        @return *this
     */
    public: this_type& operator=(this_type const& in_string) PSYQ_NOEXCEPT
    {
        this->base_type::base_type::operator=(in_string);
        return *this;
    }

    /** @copydoc storage(typename base_type::base_type::view const&)
        @return *this
     */
    public: this_type& operator=(typename base_type::view const& in_string)
    PSYQ_NOEXCEPT
    {
        return *new(this) this_type(in_string);
    }
    //@}
};

#endif // !defined(PSYQ_STRING_STORAGE_HPP_)
