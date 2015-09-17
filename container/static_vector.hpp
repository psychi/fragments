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
/// @copybrief psyq::container::static_vector
#ifndef PSYQ_STATIC_VECTOR_HPP_
#define PSYQ_STATIC_VECTOR_HPP_

#include "./static_container.hpp"

#ifndef PSYQ_NO_STD_INITIALIZER_LIST
#include <initializer_list>
#endif // PSYQ_NO_STD_INITIALIZER_LIST

namespace psyq
{
    namespace container
    {
        /// @cond
        template<typename, std::size_t, typename> class static_vector;
        /// @endcond
    } // namespace container
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 動的メモリ割当を行わない、 std::vector を模したコンテナ。
/// @tparam template_value    @copydoc base_type::value_type
/// @tparam template_max_size @copydoc base_type::MAX_SIZE
/// @tparam template_size     @copydoc base_type::size_type
template<
    typename template_value,
    std::size_t template_max_size,
    typename template_size = std::size_t>
class psyq::container::static_vector:
public psyq::container::_private::static_container<
    template_value, template_max_size, template_size>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef static_vector this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::container::_private::static_container<
            template_value, template_max_size, template_size>
        base_type;

    //-------------------------------------------------------------------------
    /// @brief 要素を指す反復子。
    public: typedef typename base_type::pointer iterator;
    /// @brief 読み取り専用の要素を指す反復子。
    public: typedef typename base_type::const_pointer const_iterator;
    /// @brief 要素を指す逆反復子。
    public: typedef
        std::reverse_iterator<typename this_type::iterator>
        reverse_iterator;
    /// @brief 読み取り専用の要素を指す逆反復子。
    public: typedef
        std::reverse_iterator<typename this_type::const_iterator>
        reverse_const_iterator;

    //-------------------------------------------------------------------------
    /// @name 構築と解体
    /// @{

    /// @brief 空のコンテナを構築する。
    public: static_vector() PSYQ_NOEXCEPT: size_(0) {}

    /// @brief コンテナをコピー構築する。
    public: static_vector(
        /// [in] コピー元となるコンテナ。
        this_type const& in_source):
    size_(in_source.size())
    {
        this->copy_construct(
            std::begin(in_source), this->size(), this->get_element(0));
    }

    /// @brief コンテナをムーブ構築する。
    public: static_vector(
        /// [in,out] ムーブ元となるコンテナ。
        this_type&& io_source):
    size_(io_source.size())
    {
        this->move_construct_import(
            std::begin(io_source), this->size(), this->get_element(0));
    }

#ifndef PSYQ_NO_STD_INITIALIZER_LIST
    /// @brief コンテナをコピー構築する。
    public: static_vector(
        /// [in] コピー元となるコンテナ。
        std::initializer_list<typename base_type::value_type> const& in_source):
    size_(
        (std::min)(
            static_cast<typename base_type::size_type>(in_source.size()),
            static_cast<typename base_type::size_type>(base_type::MAX_SIZE)))
    {
        PSYQ_ASSERT(this->size() == in_source.size());
        this->copy_construct(
            in_source.begin(), this->size(), this->get_element(0));
    }
#endif // !defined(PSYQ_NO_STD_INITIALIZER_LIST)

    /// @brief コンテナを構築する。
    public: explicit static_vector(
        /// [in] コンテナの要素数。
        typename base_type::size_type const in_count):
    size_(
        (std::min)(
            in_count,
            static_cast<typename base_type::size_type>(base_type::MAX_SIZE)))
    {
        PSYQ_ASSERT(this->size() == in_count);
        this->default_construct(this->size(), this->get_element(0));
    }

    /// @brief コンテナをコピー構築する。
    public: static_vector(
        /// [in] コピーする要素の数。
        typename base_type::size_type const in_count,
        /// [in] コピーする値。
        typename base_type::value_type const& in_value):
    size_(
        (std::min)(
            in_count,
            static_cast<typename base_type::size_type>(base_type::MAX_SIZE)))
    {
        PSYQ_ASSERT(this->size() == in_count);
        this->copy_construct(in_value, this->size(), this->get_element(0));
    }

    /// @brief コンテナをコピー構築する。
    public: template<typename template_iterator>
    static_vector(
        /// [in] コピー元コンテナの先頭位置を指す反復子。
        template_iterator const& in_begin,
        /// [in] コピー元コンテナの末尾位置を指す反復子。
        template_iterator const& in_end):
    size_(
        (std::min)(
            static_cast<typename base_type::size_type>(std::distance(in_begin, in_end)),
            static_cast<typename base_type::size_type>(base_type::MAX_SIZE)))
    {
        PSYQ_ASSERT(this->size() == std::distance(in_begin, in_end));
        this->copy_construct(in_begin, this->size(), this->get_element(0));
    }

    /// @brief コンテナを解体する。
    public: ~static_vector()
    {
        this->destruct(this->get_element(0), this->size());
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 代入
    /// @{

    /// @brief コンテナにコピー代入する。
    /// @return *this
    public: this_type& operator=(
        /// [in] コピー元となるコンテナ。
        this_type const& in_source)
    {
        if (this != &in_source)
        {
            this->copy_assign(in_source);
        }
        return *this;
    }

    /// @brief コンテナにムーブ代入する。
    /// @return *this
    public: this_type& operator=(
        /// [in,out] ムーブ元となるコンテナ。
        this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->move_assign(std::move(io_source));
        }
        return *this;
    }

#ifndef PSYQ_NO_STD_INITIALIZER_LIST
    /// @brief コンテナにコピー代入する。
    /// @return *this
    public: this_type& operator=(
        /// [in] コピー元となるコンテナ。
        std::initializer_list<typename base_type::value_type> const& in_source)
    {
        this->assign(in_source);
        return *this;
    }

    /// @brief コンテナにコピー代入する。
    public: void assign(
        /// [in] コピー元となるコンテナ。
        std::initializer_list<typename base_type::value_type> const& in_source)
    {
        this->~this_type();
        new(this) this_type(in_source);
    }
#endif // !defined(PSYQ_NO_STD_INITIALIZER_LIST)

    /// @brief コンテナにコピー代入する。
    public: template<typename template_iterator>
    void assign(
        /// [in] コピー元コンテナの先頭位置を指す反復子。
        template_iterator const& in_begin,
        /// [in] コピー元コンテナの末尾位置を指す反復子。
        template_iterator const& in_end)
    {
        if (this->is_contained(*in_begin))
        {
            this->move_assign(this_type(in_begin, in_end));
        }
        else
        {
            this->~this_type();
            new(this) this_type(in_begin, in_end);
        }
    }

    /// @brief コンテナにコピー代入する。
    public: void assign(
        /// [in] コピーする要素の数。
        typename base_type::size_type const in_count,
        /// [in] コピーする値。
        typename base_type::const_reference in_value)
    {
        this->~this_type();
        new(this) this_type(in_count, in_value);
    }

    /// @brief コンテナを交換する。
    public: void swap(this_type& io_target)
    {
        if (this != &io_target)
        {
            this_type local_target(std::move(io_target));
            io_target.move_assign(std::move(*this));
            this->move_assign(std::move(local_target));
        }
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name コンテナの大きさ
    /// @{

    /// @brief コンテナが空か判定する。
    /// @retval true  *this は空。
    /// @retval false *this は空ではない。
    public: bool empty() const PSYQ_NOEXCEPT
    {
        return this->size() <= 0;
    }

    /// @brief コンテナに格納されている要素の数を取得する。
    /// @return *this に格納されている要素の数。
    public: typename base_type::size_type size() const PSYQ_NOEXCEPT
    {
        return this->size_;
    }

    /// @brief コンテナに格納できる要素の最大数を取得する。
    /// @return *this に格納できる要素の最大数。
    public: typename base_type::size_type max_size() const PSYQ_NOEXCEPT
    {
        return base_type::MAX_SIZE;
    }

    /// @copydoc max_size
    public: typename base_type::size_type capacity() const PSYQ_NOEXCEPT
    {
        return this->max_size();
    }

    /// @brief コンテナの要素数を変更する。
    /// @retval true  成功。要素数を変更した。
    /// @retval false 失敗。 *this は変化しない。
    public: bool resize(
        /// [in] コンテナの新たな要素数。
        typename base_type::size_type const in_size)
    {
        if (base_type::MAX_SIZE < in_size)
        {
            PSYQ_ASSERT(false);
            return false;
        }
        auto const local_size(this->shrink_size(in_size));
        if (local_size < in_size)
        {
            this->default_construct(
                in_size - local_size,
                this->get_element(local_size));
            this->size_ = in_size;
        }
        return true;
    }

    /// @copydoc this_type::resize
    public: bool resize(
        /// [in] コンテナの新たな要素数。
        typename base_type::size_type const in_size,
        /// [in] コンテナの要素数を増やす場合の要素の初期値。
        typename base_type::const_reference in_value)
    {
        if (base_type::MAX_SIZE < in_size)
        {
            PSYQ_ASSERT(false);
            return false;
        }
        auto const local_size(this->shrink_size(in_size));
        if (local_size < in_size)
        {
            this->copy_construct(
                in_value,
                in_size - local_size,
                this->get_element(local_size));
            this->size_ = in_size;
        }
        return true;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 反復子の取得
    /// @{

    /// @brief コンテナの先頭位置を指す反復子を取得する。
    /// @return コンテナの先頭位置を指す反復子。
    public: typename this_type::iterator begin() PSYQ_NOEXCEPT
    {
        return const_cast<typename this_type::iterator>(this->cbegin());
    }

    /// @copydoc begin
    public: typename this_type::const_iterator begin() const PSYQ_NOEXCEPT
    {
        return this->cbegin();
    }

    /// @copydoc begin
    public: typename this_type::const_iterator cbegin() const PSYQ_NOEXCEPT
    {
        return this->data();
    }

    /// @brief コンテナの末尾位置を指す反復子を取得する。
    /// @return コンテナの末尾位置を指す反復子。
    public: typename this_type::iterator end() PSYQ_NOEXCEPT
    {
        return const_cast<typename this_type::iterator>(this->cend());
    }

    /// @copydoc end
    public: typename this_type::const_iterator end() const PSYQ_NOEXCEPT
    {
        return this->cend();
    }

    /// @copydoc end
    public: typename this_type::const_iterator cend() const PSYQ_NOEXCEPT
    {
        return this->begin() + this->size();
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 逆反復子の取得
    /// @{

    /// @brief コンテナの末尾位置を指す逆反復子を取得する。
    /// @return コンテナの末尾位置を指す逆反復子。
    public: typename this_type::reverse_iterator rbegin() PSYQ_NOEXCEPT
    {
        return typename this_type::reverse_iterator(this->end());
    }

    /// @copydoc rbegin
    public: typename this_type::reverse_const_iterator rbegin()
    const PSYQ_NOEXCEPT
    {
        return const_cast<this_type*>(this)->rbegin();
    }

    /// @copydoc rbegin
    public: typename this_type::reverse_const_iterator crbegin()
    const PSYQ_NOEXCEPT
    {
        return this->rbegin();
    }

    /// @brief コンテナの先頭位置を指す逆反復子を取得する。
    /// @return コンテナの先頭位置を指す逆反復子。
    public: typename this_type::reverse_iterator rend() PSYQ_NOEXCEPT
    {
        return typename this_type::reverse_iterator(this->begin());
    }

    /// @copydoc rend
    public: typename this_type::reverse_const_iterator rend()
    const PSYQ_NOEXCEPT
    {
        return const_cast<this_type*>(this)->rend();
    }

    /// @copydoc rend
    public: typename this_type::reverse_const_iterator crend()
    const PSYQ_NOEXCEPT
    {
        return this->rend();
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 要素の参照
    /// @{

    /// @brief コンテナの先頭を指すポインタを取得する。
    /// @return コンテナの先頭を指すポインタ。
    public: typename base_type::const_pointer data() const PSYQ_NOEXCEPT
    {
        return this->get_element(0);
    }

    /// @brief インデクス番号に対応する要素を参照する。
    /// @return in_index に対応する要素への参照。
    public: typename base_type::reference operator[](
        /// [in] 参照する要素のインデクス番号。
        typename base_type::size_type const in_index)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_index < base_type::MAX_SIZE);
        return *this->get_element(in_index);
    }

    /// @copydoc this_type::operator[]
    public: typename base_type::const_reference operator[](
        /// [in] 参照する要素のインデクス番号。
        typename base_type::size_type const in_index)
    const PSYQ_NOEXCEPT
    {
        return const_cast<this_type*>(this)->operator[](in_index);
    }

    /// @brief インデクス番号に対応する要素を参照する。
    /// @details in_index に対応する要素がない場合は例外を投げる。
    /// @return in_index に対応する要素への参照。
    public: typename base_type::reference at(
        /// [in] 参照する要素のインデクス番号。
        typename base_type::size_type const in_index)
    {
        PSYQ_ASSERT_THROW(in_index < this->size(), std::out_of_range);
        return this->operator[](in_index);
    }

    /// @copydoc this_type::at
    public: typename base_type::const_reference at(
        /// [in] 参照する要素のインデクス番号。
        typename base_type::size_type const in_index)
    const
    {
        return const_cast<this_type*>(this)->at(in_index);
    }

    /// @brief コンテナの先頭要素を参照する。
    /// @details コンテナが空の場合は例外を投げる。
    /// @return 先頭要素への参照。
    public: typename base_type::reference front()
    {
        return this->at(0);
    }

    /// @copydoc front
    public: typename base_type::const_reference front() const
    {
        return const_cast<this_type*>(this)->front();
    }

    /// @brief コンテナの末尾要素を参照する。
    /// @details コンテナが空の場合は例外を投げる。
    /// @return 末尾要素への参照。
    public: typename base_type::reference back()
    {
        return this->at(this->size() - 1);
    }

    /// @copydoc back
    public: typename base_type::const_reference back() const
    {
        return const_cast<this_type*>(this)->back();
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 要素の挿入
    /// @{

    /// @brief コンテナの末尾位置に要素を挿入する。
    /// @retval true  成功。コンテナの末尾位置に要素を挿入した。
    /// @retval false 失敗。要素を挿入できなかった。
    public: bool push_back(
        /// [in] 挿入する要素の初期値。
        typename base_type::value_type in_value)
    {
        auto const local_size(this->size());
        if (base_type::MAX_SIZE <= local_size)
        {
            PSYQ_ASSERT(false);
            return false;
        }
        this->size_ = local_size + 1;
        new(this->get_element(local_size))
            typename base_type::value_type(std::move(in_value));
        return true;
    }

    /// @brief コンテナに要素を挿入する。
    /// @retval !=this->end() 挿入した要素を指す反復子。
    /// @retval ==this->end() 挿入に失敗。 *this は変化しない。
    public: typename this_type::iterator insert(
        /// [in] 要素を挿入する位置。
        typename this_type::const_iterator const in_position,
        /// [in] 挿入する要素の初期値。
        typename base_type::const_reference in_value)
    {
        return this->insert(in_position, 1, in_value);
    }

#ifndef PSYQ_NO_STD_INITIALIZER_LIST
    /// @brief コンテナに要素を挿入する。
    /// @retval !=this->end() 挿入した要素の先頭を指す反復子。
    /// @retval ==this->end() 挿入に失敗。 *this は変化しない。
    public: typename this_type::iterator insert(
        /// [in] 要素を挿入する位置。
        typename this_type::const_iterator const in_position,
        /// [in] 挿入する要素のコンテナ。
        std::initializer_list<typename base_type::value_type> const& in_source)
    {
        return this->insert(
            in_position, std::begin(in_source), std::end(in_source));
    }
#endif // !defined(PSYQ_NO_STD_INITIALIZER_LIST)

    /// @brief コンテナに要素を挿入する。
    /// @retval !=this->end() 挿入した要素の先頭を指す反復子。
    /// @retval ==this->end() 挿入に失敗。 *this は変化しない。
    public: typename this_type::iterator insert(
        /// [in] 要素を挿入する位置。
        typename this_type::const_iterator const in_position,
        /// [in] 挿入する要素の数。
        typename base_type::size_type const in_count,
        /// [in] 挿入する要素の初期値。
        typename base_type::const_reference in_value)
    {
        auto const& local_value(
            this->is_contained(in_value)?
                typename base_type::value_type(in_value): in_value);

        // 挿入する場所を空けて、値をコピー構築する。
        auto const local_position(
            const_cast<typename base_type::pointer>(in_position));
        auto const local_count(
            this->insert_empty(
                local_position,
                (std::min)(base_type::MAX_SIZE - this->size(), in_count)));
        PSYQ_ASSERT(local_count == in_count);
        this->copy_construct(
            local_value, local_count, local_position);
        return local_position;
    }

    /// @brief コンテナにコンテナを挿入する。
    /// @retval !=this->end() 挿入したコンテナの先頭要素を指す反復子。
    /// @retval ==this->end() 挿入に失敗。 *this は変化しない。
    public: template<typename template_iterator>
    typename this_type::iterator insert(
        /// [in] コンテナを挿入する位置。
        typename this_type::const_iterator const in_position,
        /// [in] 挿入するコンテナの先頭位置を指す反復子。
        template_iterator const& in_begin,
        /// [in] 挿入するコンテナの末尾位置を指す反復子。
        template_iterator const& in_end)
    {
        if (in_begin == in_end)
        {
            return const_cast<typename this_type::iterator>(in_position);
        }
        else if (this->is_contained(*in_begin))
        {
            this_type const local_source(in_begin, in_end);
            return this->insert_copy(
                in_position, local_source.begin(), local_source.end());
        }
        else
        {
            return this->insert_copy(in_position, in_begin, in_end);
        }
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 要素の削除
    /// @{

    /// @brief コンテナの末尾要素を削除する。
    /// @retval true  コンテナの末尾要素を削除した。
    /// @retval false 失敗。コンテナが空なので、削除できなかった。
    public: bool pop_back()
    {
        // すでに空なら失敗する。
        if (this->empty())
        {
            PSYQ_ASSERT(false);
            return false;
        }

        // 末尾位置を更新し、直前の末尾要素を破棄する。
        this->back().~value_type();
        --this->size_;
        return true;
    }

    /// @brief コンテナから要素を削除する。
    /// @return 削除した要素の次を指す反復子。
    public: typename this_type::iterator erase(
        /// [in] 削除する要素を指す反復子。
        typename this_type::const_iterator const in_position)
    {
        auto const local_begin(this->get_element(0));
        auto const local_end(local_begin + this->size());
        auto const local_position(
            const_cast<typename base_type::pointer>(in_position));
        if (local_position < local_begin || local_end <= local_position)
        {
            // コンテナの範囲外なので削除しない。
            PSYQ_ASSERT(local_end == local_position);
            return local_end;
        }
        local_position->~value_type();
        auto const local_position_next(local_position + 1);
        if (local_position_next < local_end)
        {
            this->move_construct_forward(
                local_position_next,
                local_end - local_position_next,
                local_position);
        }
        --this->size_;
        return local_position;
    }

    /// @brief コンテナから要素を削除する。
    /// @return 削除した範囲の次を指す反復子。
    public: typename this_type::iterator erase(
        /// [in] 削除する範囲の先頭を指す反復子。
        typename this_type::const_iterator const in_erase_begin,
        /// [in] 削除する範囲の末尾を指す反復子。
        typename this_type::const_iterator const in_erase_end)
    {
        auto const local_begin(this->get_element(0));
        auto const local_end(local_begin + this->size());
        /// @todo 範囲の確認をすること。
        if (in_erase_end == local_end)
        {
            if (in_erase_begin == local_begin)
            {
                this->clear();
            }
            else
            {
                this->erase_back(local_end - local_begin);
            }
            return this->end();
        }

        auto const local_erase_begin(
            const_cast<typename base_type::pointer>(in_erase_begin));
        auto const local_erase_size(in_erase_end - in_erase_begin);
        this->destruct(local_erase_begin, local_erase_size);
        this->move_construct_forward(
            const_cast<typename base_type::pointer>(in_erase_end),
            local_end - in_erase_end,
            local_erase_begin);
        this->size_ -= local_erase_size;
        return local_erase_begin;
    }

    /// @brief コンテナの要素を全て削除する。
    public: void clear()
    {
        if (!this->empty())
        {
            this->~this_type();
            new(this) this_type;
        }
    }
    /// @}
    //-------------------------------------------------------------------------
    private: void copy_assign(this_type const& in_source)
    {
        this->~this_type();
        new(this) this_type(in_source);
    }

    private: void move_assign(this_type&& io_source)
    {
        this->~this_type();
        new(this) this_type(std::move(io_source));
    }

    /// @brief コンテナに空要素を挿入する。
    private: typename base_type::size_type insert_empty(
        /// [in] 空要素を挿入する位置。
        typename base_type::pointer const in_position,
        /// [in] 挿入する空要素の数。
        typename base_type::size_type const in_count)
    {
        auto const local_size(this->size());
        auto const local_begin(this->data());
        auto const local_end(local_begin + local_size);
        if (in_position < local_begin || local_end < in_position)
        {
            PSYQ_ASSERT(false);
            return 0;
        }
        /// @todo このあたりはなんか間違ってる。考えよう。
        auto const local_count(
            (std::min)(base_type::MAX_SIZE - local_size, in_count));
        PSYQ_ASSERT(local_count == in_count);
        this->size_ = local_size + local_count;
        this->move_construct_backward(
            in_position,
            local_end - in_position,
            in_position + local_count);
        return local_count;
    }

    private: template<typename template_iterator>
    typename base_type::pointer insert_copy(
        /// [in] コンテナを挿入する位置。
        typename base_type::const_pointer const in_position,
        /// [in] 挿入するコンテナの先頭位置を指す反復子。
        template_iterator const& in_begin,
        /// [in] 挿入するコンテナの末尾位置を指す反復子。
        template_iterator const& in_end)
    {
        auto const local_position(
            const_cast<typename base_type::pointer>(in_position));
        auto const local_distance(std::distance(in_begin, in_end));
        auto const local_count(
            this->insert_empty(
                local_position,
                (std::min<typename base_type::size_type>)(
                    base_type::MAX_SIZE - this->size(), local_distance)));
        PSYQ_ASSERT(local_count == local_distance);
        this->copy_construct(in_begin, local_count, local_position);
        return local_position;
    }

    /// @brief コンテナの要素数を縮小する。
    /// @return
    ///   縮小後のコンテナの要素数。
    ///   縮小に失敗した場合は、コンテナの要素数は変化しない。
    private: typename base_type::size_type shrink_size(
        /// [in] コンテナの新たな要素数。
        typename base_type::size_type const in_size)
    {
        auto const local_current_size(this->size());
        if (local_current_size < in_size)
        {
            return local_current_size;
        }
        this->erase_back(local_current_size - in_size);
        return in_size;
    }

    /// @brief コンテナ要素を末尾から削除する。
    private: void erase_back(
        /// [in] 削除する要素の数。
        typename base_type::size_type const in_count)
     {
        this->destruct(this->end() - in_count, in_count);
        this->size_ -= in_count;
     }

    //-------------------------------------------------------------------------
    /// @brief コンテナの要素数。
    private: typename base_type::size_type size_;

}; // class psyq::static_vector

#endif // !defined(PSYQ_STATIC_VECTOR_HPP_)
// vim: set expandtab:
