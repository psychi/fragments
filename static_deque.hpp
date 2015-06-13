/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @copybrief psyq::static_deque
 */
#ifndef PSYQ_STATIC_DEQUE_HPP_
#define PSYQ_STATIC_DEQUE_HPP_

#include <initializer_list>
//#include "./assert.hpp"

#if defined(NDEBUG) && !defined(PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW)
#define PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW
#endif // !defined(NDEBUG) && !defined(PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW)

#ifndef PSYQ_ALIGNOF
#   if defined(_MSC_VER)
#       define PSYQ_ALIGNOF __alignof
#   else
#       define PSYQ_ALIGNOF alignof
#   endif
#endif // !defined(PSYQ_ALIGNOF)

/// @cond
namespace psyq
{
    template<typename, std::size_t> class static_deque;
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 動的メモリ割当を行わない、二重終端キューコンテナ。

    std::deque を模したインターフェイスを持つ。

    @tparam template_value    @copydoc static_deque::value_type
    @tparam template_max_size @copydoc static_deque::MAX_SIZE
 */
template<typename template_value, std::size_t template_max_size>
class psyq::static_deque
{
    private: typedef static_deque this_type;

    /// @brief コンテナに格納する要素を表す型。
    public: typedef template_value value_type;
    /// @brief 要素の数を表す型。
    public: typedef std::size_t size_type;
    /// @brief 要素を指すポインタの差分を表す型。
    public: typedef std::ptrdiff_t difference_type;
    /// @brief 要素を指すポインタを表す型。
    public: typedef typename this_type::value_type* pointer;
    /// @brief 読み取り専用要素を指すポインタを表す型。
    public: typedef typename this_type::value_type const* const_pointer;
    /// @brief 要素への参照を表す型。
    public: typedef typename this_type::value_type& reference;
    /// @brief 読み取り専用要素への参照を表す型。
    public: typedef typename this_type::value_type const& const_reference;

    /// @brief コンテナに格納できる要素の最大数。
    public: static typename this_type::size_type const MAX_SIZE =
        template_max_size;
    static_assert(0 < template_max_size, "'template_max_size' is empty.");

    //-------------------------------------------------------------------------
    private: typedef typename std::aligned_storage<
        sizeof(template_value) * template_max_size,
        PSYQ_ALIGNOF(template_value)>
            ::type
                storage;

    private: typedef template_value array_view[template_max_size];

    //-------------------------------------------------------------------------
    private: struct iterator_base
    {
        typedef iterator_base this_type;

        iterator_base(
            static_deque* in_deque,
            typename static_deque::pointer const in_pointer)
        PSYQ_NOEXCEPT:
        deque_(in_deque),
        pointer_(in_pointer)
        {}

        static_deque* deque_;
        typename static_deque::pointer pointer_;
    };

    /// @brief 要素を指す反復子を表す型。
    public: typedef typename this_type::iterator_base iterator;
    /// @brief 読み取り専用要素を指す反復子を表す型。
    public: typedef typename this_type::iterator_base const_iterator;
    /// @brief 要素を指す逆反復子を表す型。
    public: typedef std::reverse_iterator<typename this_type::iterator>
        reverse_iterator;
    /// @brief 読み取り専用要素を指す逆反復子を表す型。
    public: typedef std::reverse_iterator<typename this_type::const_iterator>
        reverse_const_iterator;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 空のコンテナを構築する。
     */
    public: static_deque()
    PSYQ_NOEXCEPT:
    begin_(this->get_pointer(0)),
    end_(this->get_pointer(0))
#ifndef PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW
    , array_view_(
        *reinterpret_cast<typename this_type::array_view*>(&this->storage_))
#endif // !defined(PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW)
    {}

    /** @brief コンテナを構築する。
        @param[in] in_count コンテナの要素数。
     */
    public: explicit static_deque(
        typename this_type::size_type const in_count)
#ifndef PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW
    : array_view_(
        *reinterpret_cast<typename this_type::array_view*>(&this->storage_))
#endif // !defined(PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW)
    {
        auto const local_end(this->initialize_range(in_count));
        for (auto i(this->begin_); i != local_end; ++i)
        {
            new(i) typename this_type::value_type;
        }
    }

    /** @brief コンテナを構築する。
        @param[in] in_count コンテナの要素数。
        @param[in] in_value 全要素の初期値。
     */
    public: explicit static_deque(
        typename this_type::size_type const in_count,
        typename this_type::value_type const& in_value)
#ifndef PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW
    : array_view_(
        *reinterpret_cast<typename this_type::array_view*>(&this->storage_))
#endif // !defined(PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW)
    {
        auto const local_end(this->initialize_range(in_count));
        for (auto i(this->begin_); i != local_end; ++i)
        {
            new(i) typename this_type::value_type(in_value);
        }
    }

    public: template<typename template_iterator>
    static_deque(
        template_iterator const& in_front,
        template_iterator const& in_last)
#ifndef PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW
    : array_view_(
        *reinterpret_cast<typename this_type::array_view*>(&this->storage_))
#endif // !defined(PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW)
    {
        this->construct_deque(
            in_front, in_last, std::distance(in_front, in_last));
    }

    /** @brief コンテナを構築する。
        @param[in] in_source コンテナの初期化子リスト。
     */
    public: static_deque(
        std::initializer_list<typename this_type::value_type> const& in_source)
#ifndef PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW
    : array_view_(
        *reinterpret_cast<typename this_type::array_view*>(&this->storage_))
#endif // !defined(PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW)
    {
        this->construct_deque(
            in_source.begin(), in_source.end(), in_source.size());
    }

    /** @brief コンテナをコピー構築する。
        @param[in] in_source コピー元となるコンテナ。
     */
    public: static_deque(this_type const& in_source):
    begin_(this->get_pointer(0)),
    end_(this->get_pointer(0))
#ifndef PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW
    , array_view_(
        *reinterpret_cast<typename this_type::array_view*>(&this->storage_))
#endif // !defined(PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW)
    {
        this->copy_construct_deque(in_source);
    }

    /** @brief コンテナをムーブ構築する。
        @param[in,out] io_source ムーブ元となるコンテナ。
     */
    public: static_deque(this_type&& io_source):
    begin_(this->get_pointer(0)),
    end_(this->get_pointer(0))
#ifndef PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW
    , array_view_(
        *reinterpret_cast<typename this_type::array_view*>(&this->storage_))
#endif // !defined(PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW)
    {
        this->move_construct_deque(io_source);
    }

    /** @brief コンテナにコピー代入する。
        @param[in] in_source コピー元となるコンテナ。
        @return *this
     */
    public: this_type& operator=(this_type const& in_source)
    {
        if (this != &in_source)
        {
            this->~this_type();
            new(this) this_type(in_source);
        }
        return *this;
    }

    /** @brief コンテナにムーブ代入する。
        @param[in,out] io_source ムーブ元となるコンテナ。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->~this_type();
            new(this) this_type(std::move(io_source));
        }
        return *this;
    }

    /** @brief コンテナにコピー代入する。
        @param[in] in_source コピー元となるコンテナ。
        @return *this
     */
    public: this_type& operator=(
        std::initializer_list<typename this_type::value_type> const& in_source)
    {
        if (this != &in_source)
        {
            this->~this_type();
            new(this) this_type(in_source);
        }
        return *this;
    }

    /// @brief コンテナを交換する。
    public: void swap(this_type& io_target)
    {
        if (this != &io_target)
        {
            auto local_target(std::move(io_target));
            io_target.move_construct_deque(*this);
            this->move_construct_deque(local_target);
        }
    }

    /// @brief コンテナを解体する。
    public: ~static_deque()
    {
        this->clear();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name コンテナの大きさ
    //@{
    /** @brief コンテナが満杯か判定する。
        @retval true  コンテナは満杯。
        @retval false コンテナは満杯ではない。
     */
    public: bool full() const PSYQ_NOEXCEPT
    {
        return this->end_ == nullptr;
    }

    /** @brief コンテナが空か判定する。
        @retval true  コンテナは空。
        @retval false コンテナは空ではない。
     */
    public: bool empty() const PSYQ_NOEXCEPT
    {
        return this->end_ == this->begin_;
    }

    /** @brief コンテナに格納されている要素の数を取得する。
        @return コンテナに格納されている要素の数。
     */
    public: typename this_type::size_type size() const PSYQ_NOEXCEPT
    {
        return this->compute_distance(this->end_);
    }

    /** @brief コンテナに格納できる要素の最大数を取得する。
        @return コンテナに格納できる要素の最大数。
     */
    public: typename this_type::size_type max_size() const PSYQ_NOEXCEPT
    {
        return this_type::MAX_SIZE;
    }

    public: void resize(
        typename this_type::size_type const in_size,
        typename this_type::value_type const& in_value = value_type());
    //@}
    //-------------------------------------------------------------------------
    /// @name 要素の参照
    //@{
    /** @brief インデクス番号に対応する要素を参照する。

        インデクス番号に対応する要素がない場合はassertする。

        @param[in] in_index 参照する要素のインデクス番号。
        @return インデクス番号に対応する要素への参照。
     */
    public: typename this_type::reference operator[](
        typename this_type::size_type const in_index)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_index < this->size());
        auto const local_element(this->begin_ + in_index);
        return *(
            local_element - this_type::MAX_SIZE * (
                this->get_pointer(this_type::MAX_SIZE) <= local_element));
    }

    /// @copydoc operator[]
    public: typename this_type::const_reference operator[](
        typename this_type::size_type const in_index)
    const PSYQ_NOEXCEPT
    {
        return const_cast<this_type*>(this)->operator[](in_index);
    }

    /// @copydoc operator[]
    public: typename this_type::reference at(
        typename this_type::size_type const in_index)
    PSYQ_NOEXCEPT
    {
        /// @note 例外を使いたくないので、範囲外でも例外を出さない。
        return this->operator[](in_index);
    }

    /// @copydoc at
    public: typename this_type::const_reference at(
        typename this_type::size_type const in_index)
    const PSYQ_NOEXCEPT
    {
        return const_cast<this_type*>(this)->at(in_index);
    }

    /** @brief コンテナの先頭要素を参照する。

        コンテナが空だった場合はassertする。

        @return 先頭要素への参照。
     */
    public: typename this_type::reference front() PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(!this->empty());
        return *this->begin_;
    }

    /// @copydoc front
    public: typename this_type::const_reference front() const PSYQ_NOEXCEPT
    {
        return const_cast<this_type*>(this)->front();
    }

    /** @brief コンテナの末尾要素を参照する。

        コンテナが空だった場合はassertする。

        @return 末尾要素への参照。
     */
    public: typename this_type::reference back() PSYQ_NOEXCEPT
    {
        return *this->get_back();
    }

    /// @copydoc back
    public: typename this_type::const_reference back() const PSYQ_NOEXCEPT
    {
        return const_cast<this_type*>(this)->back();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 反復子の取得
    //@{
    /** @brief コンテナの先頭位置を指す反復子を取得する。
        @return コンテナの先頭位置を指す反復子。
     */
    public: typename this_type::iterator begin() PSYQ_NOEXCEPT
    {
        return this_type::cbegin();
    }

    /// @copydoc begin
    public: typename this_type::const_iterator begin() const PSYQ_NOEXCEPT
    {
        return this_type::cbegin();
    }

    /// @copydoc begin
    public: typename this_type::const_iterator cbegin() const PSYQ_NOEXCEPT
    {
        return this_type::iterator_base(this, this->begin_);
    }

    /** @brief コンテナの末尾位置を指す反復子を取得する。
        @return コンテナの末尾位置を指す反復子。
     */
    public: typename this_type::iterator end() PSYQ_NOEXCEPT
    {
        return this_type::cend();
    }

    /// @copydoc end
    public: typename this_type::const_iterator end() const PSYQ_NOEXCEPT
    {
        return this_type::cend();
    }

    /// @copydoc end
    public: typename this_type::const_iterator cend() const PSYQ_NOEXCEPT
    {
        return this_type::iterator_base(this, nullptr);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 逆反復子の取得
    //@{
    /** @brief コンテナの末尾位置を指す逆反復子を取得する。
        @return コンテナの末尾位置を指す逆反復子。
     */
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

    /** @brief コンテナの先頭位置を指す逆反復子を取得する。
        @return コンテナの先頭位置を指す逆反復子。
     */
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
    //@}
    //-------------------------------------------------------------------------
    /// @name 要素の挿入
    //@{
    /** @brief コンテナの先頭位置に要素を挿入する。
        @param[in] in_value 挿入する要素の初期値。
        @retval !=nullptr コンテナの先頭要素を指すポインタ。
        @retval ==nullptr 失敗。要素を挿入できなかった。
     */
    public: typename this_type::pointer push_front(
        typename this_type::value_type in_value)
    {
        auto const local_storage(this->allocate_front(1));
        return local_storage != nullptr?
            new(local_storage)
                typename this_type::value_type(std::move(in_value)):
            nullptr;
    }

    /** @brief コンテナの末尾位置に要素を挿入する。
        @param[in] in_value 挿入する要素の初期値。
        @retval !=nullptr コンテナの末尾要素を指すポインタ。
        @retval ==nullptr 失敗。要素を挿入できなかった。
     */
    public: typename this_type::pointer push_back(
        typename this_type::value_type in_value)
    {
        auto const local_storage(this->allocate_back(1));
        return local_storage != nullptr?
            new(local_storage)
                typename this_type::value_type(std::move(in_value)):
            nullptr;
    }

    public: typename this_type::iterator insert(
        typename this_type::const_iterator const& in_position,
        typename this_type::value_type in_value);

    public: typename this_type::iterator insert(
        typename this_type::const_iterator const& in_position,
        typename this_type::size_type const in_count,
        typename this_type::value_type const& in_value);

    public: template<typename template_iterator>
    typename this_type::iterator insert(
        typename this_type::const_iterator const& in_position,
        template_iterator const& in_first,
        template_iterator const& in_last);
    //@}
    //-------------------------------------------------------------------------
    /// @name 要素の削除
    //@{
    /** @brief コンテナの先頭要素を削除する。
        @retval true  コンテナの先頭要素を削除した。
        @retval false 失敗。コンテナが空なので、削除できなかった。
     */
    public: bool pop_front()
    {
        // すでに空なら失敗する。
        if (this->empty())
        {
            PSYQ_ASSERT(false);
            return false;
        }

        // 先頭位置を更新し、直前の先頭要素を破棄する。
        this->begin_->~value_type();
        this->begin_ = this->begin_ + 1 - this_type::MAX_SIZE * (
            this->get_pointer(this_type::MAX_SIZE - 1) <= this->begin_);
        return true;
    }

    /** @brief コンテナの末尾要素を削除する。
        @retval true  コンテナの末尾要素を削除した。
        @retval false 失敗。コンテナが空なので、削除できなかった。
     */
    public: bool pop_back()
    {
        // すでに空なら失敗する。
        if (this->empty())
        {
            PSYQ_ASSERT(false);
            return false;
        }

        // 末尾位置を更新し、直前の末尾要素を破棄する。
        auto const local_end(this->get_end_pointer());
        this->end_ = local_end - 1 +
            this_type::MAX_SIZE * (local_end <= this->get_pointer(0));
        this->end_->~value_type();
        return true;
    }

    public: typename this_type::iterator erase(
        typename this_type::const_iterator const& in_position);

    /** @brief コンテナから要素を削除する。
        @param[in] in_first 削除する範囲の先頭位置。
        @param[in] in_last  削除する範囲の末尾位置。
     */
    public: typename this_type::iterator erase(
        typename this_type::const_iterator const& in_first,
        typename this_type::const_iterator const& in_last)
    {
        auto const local_size(this->size());
        auto const local_first_index(this->compute_index(in_first.pointer_));
        auto const local_last_index(this->compute_index(in_last.pointer_));
        if (local_last_index <= local_first_index
            || local_size <= local_first_index)
        {
            PSYQ_ASSERT(
                local_first_index <= local_last_index
                && local_last_index <= local_size);
            return this->end();
        }
        else if (in_last.pointer_ == nullptr)
        {
            // 末尾位置に接する範囲を削除する。
            if (in_first.pointer_ != this->begin_)
            {
                this->destruct_element(
                    in_first.pointer_, this->get_end_pointer());
                this->end_ = in_first.pointer_;
            }
            else
            {
                this->clear();
            }
            return in_last;
        }
        else if (in_first.pointer_ == this->begin_)
        {
            // 先頭位置に接する範囲を削除する。
            this->destruct_element(this->begin_, in_last.pointer_);
            if (this->end_ == nullptr)
            {
                this->end_ = this->begin_;
            }
            this->begin_ = in_last.pointer_;
            return in_last;
        }

        this->destruct_element(in_first.pointer_, in_last.pointer_);
        if (local_first_index < local_size - local_last_index)
        {
            this->slide_element();
            this->begin_ = ;
            return in_last;
        }
        else
        {
            this->slide_element();
            this->end_ = ;
            return in_front;
        }
    }

    /** @brief コンテナの要素を全て削除する。
     */
    public: void clear()
    {
        if (!this->empty())
        {
            this->destruct_element(this->begin_, this->get_end_pointer());
            this->begin_ = this->get_pointer(0);
            this->end_ = this->begin_;
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief コンテナ先頭位置からの距離を算出する。
        @param[in] in_pointer 距離を算出する要素を指すポインタ。
        @return コンテナ先頭位置からの距離。
     */
    private: typename this_type::size_type compute_distance(
        typename this_type::const_pointer const in_pointer)
    const PSYQ_NOEXCEPT
    {
        return in_pointer != nullptr?
            static_cast<typename this_type::size_type>(
                in_pointer - this->begin_
                + this_type::MAX_SIZE * (in_pointer < this->begin_)):
            this_type::MAX_SIZE;
    }

    /** @brief コンテナ先頭位置からのインデクス番号を算出する。
        @param[in] in_pointer インデクス番号を算出する要素を指すポインタ。
        @return 要素のインデクス番号。
     */
    private: typename this_type::size_type compute_index(
        typename this_type::const_pointer const in_pointer)
    const PSYQ_NOEXCEPT
    {
        return in_pointer != nullptr?
            this->compute_distance(in_pointer): this->size();
    }

    private: typename this_type::pointer get_pointer(
        typename this_type::size_type const in_index)
    PSYQ_NOEXCEPT
    {
        return reinterpret_cast<this_type::pointer>(&this->storage_) + in_index;
    }

    private: typename this_type::const_pointer get_pointer(
        typename this_type::size_type const in_index)
    const PSYQ_NOEXCEPT
    {
        return const_cast<this_type*>(this)->get_pointer(in_index);
    }

    private: typename this_type::pointer get_end_pointer() const PSYQ_NOEXCEPT
    {
        return this->full()? this->begin_: this->end_;
    }

    /** @brief 末尾要素を指すポインタを取得する。
        @retval !=nullptr 末尾要素を指すポインタ。
        @retval ==nullptr コンテナが空だったので、末尾要素がない。
     */
    private: typename this_type::pointer get_back()
    {
        if (this->empty())
        {
            return nullptr;
        }
        auto const local_end(this->get_end_pointer());
        return local_end - 1 +
            this_type::MAX_SIZE * (local_end <= this->get_pointer(0));
    }

    private: typename this_type::const_pointer initialize_range(
        typename this_type::size_type const in_count)
    {
        auto local_end(this->get_pointer(0));
        this->begin_ = local_end;
        if (in_count < this_type::MAX_SIZE)
        {
            local_end += in_count;
            this->end_ = local_end;
        }
        else
        {
            PSYQ_ASSERT(in_count <= this_type::MAX_SIZE);
            local_end += this_type::MAX_SIZE;
            this->end_ = nullptr;
        }
        return local_end;
    }

    //-------------------------------------------------------------------------
    /** @brief コンテナの先頭に空要素を挿入する。
        @param[in] in_size 挿入する空要素の数。
        @retval !=nullptr 挿入した空要素の先頭を指すポインタ。
        @retval ==nullptr 失敗。空要素を挿入できなかった。
     */
    private: void* allocate_front(typename this_type::size_type const in_size)
    PSYQ_NOEXCEPT
    {
        // 残り容量がなければ、失敗する。
        if (this->full() || this_type::MAX_SIZE < in_size)
        {
            return nullptr;
        }

        // 新たな先頭位置を決定する。
        auto local_new_begin(this->begin_ - in_size);
        bool const local_circulate(local_new_begin < this->get_pointer(0));
        if (local_circulate)
        {
            local_new_begin += this_type::MAX_SIZE;
        }
        if (local_new_begin < this->end_
            && (local_circulate || this->end_ < this->begin_))
        {
            // 新たな先頭位置が末尾位置を追い越していたら、失敗する。
            return nullptr;
        }

        // 先頭位置を更新する。
        this->begin_ = local_new_begin;
        if (this->end_ == local_new_begin)
        {
            this->end_ = nullptr;
        }
        return local_new_begin;
    }

    /** @brief コンテナの末尾に空要素を挿入する。
        @param[in] in_size 挿入する空要素の数。
        @retval !=nullptr 挿入した空要素の先頭を指すポインタ。
        @retval ==nullptr 失敗。空要素を挿入できなかった。
     */
    private: void* allocate_back(typename this_type::size_type const in_size)
    PSYQ_NOEXCEPT
    {
        // 残り容量がなければ、失敗する。
        if (this->full() || this_type::MAX_SIZE < in_size)
        {
            return nullptr;
        }

        // 新たな末尾位置を決定する。
        auto local_new_end(this->end_ + in_size);
        bool const local_circulate(
            this->get_pointer(this_type::MAX_SIZE) <= local_new_end);
        if (local_circulate)
        {
            local_new_end -= this_type::MAX_SIZE;
        }
        if (this->begin_ < local_new_end
            && (local_circulate || this->end_ < this->begin_))
        {
            // 新たな末尾位置が先頭位置を追い越していたら、失敗する。
            return nullptr;
        }

        // 末尾位置を更新する。
        auto const local_last_end(this->end_);
        this->end_ = local_new_end != this->begin_? local_new_end: nullptr;
        return local_last_end;
    }

    private: void destruct_element(
        typename this_type::pointer const in_front,
        typename this_type::pointer const in_last)
    {
        PSYQ_ASSERT(
            this->get_pointer(0) <= in_front
            && in_front < this->get_pointer(this_type::MAX_SIZE)
            && this->get_pointer(0) <= in_last
            && in_last < this->get_pointer(this_type::MAX_SIZE));

        // 末尾から逆順に要素を解体する。
        auto local_element(in_last);
        for (;;)
        {
            local_element = local_element - 1 +
                this_type::MAX_SIZE * (local_element <= this->get_pointer(0));
            local_element->~value_type();
            if (in_front == local_element)
            {
                return;
            }
        }
    }

    //-------------------------------------------------------------------------
    private: template<typename template_iterator>
    void construct_deque(
        template_iterator const& in_front,
        template_iterator const& in_last,
        typename this_type::size_type const in_size)
    {
        auto const local_end(this->initialize_range(in_size));
        auto local_source(in_front);
        for (auto i(this->begin_); i != local_end; ++i, ++local_source)
        {
            new(i) typename this_type::value_type(*local_source);
        }
    }

    private: template<typename template_deque, typename template_constructor>
    void construct_deque(
        template_deque& in_source,
        template_constructor const& in_constructor)
    {
        PSYQ_ASSERT(this->empty());
        if (in_source.empty())
        {
            return;
        }
        auto const local_source_end(in_source.get_end_pointer());
        auto local_source_element(in_source.begin_);
        auto local_target_element(this->get_pointer(0));
        this->begin_ = local_target_element;
        for (;;)
        {
            in_constructor(local_target_element, *local_source_element);
            ++local_target_element;
            local_source_element = local_source_element + 1 -
                this_type::MAX_SIZE * (
                    in_source.get_pointer(this_type::MAX_SIZE - 1)
                    <= local_source_element);
            if (local_source_end == local_source_element)
            {
                break;
            }
        }
        this->end_ =
            local_target_element < this->get_pointer(this_type::MAX_SIZE)?
                local_target_element: nullptr;
    }

    private: void copy_construct_deque(this_type const& in_source)
    {
        this->construct_deque(
            in_source,
            [](
                typename this_type::value_type* const in_target_element,
                typename this_type::value_type const& in_source_element)
            {
                new(in_target_element) value_type(in_source_element);
            });
    }

    private: void move_construct_deque(this_type& io_source)
    {
        this->construct_deque(
            io_source,
            [](
                typename this_type::value_type* const in_target_element,
                typename this_type::value_type& io_source_element)
            {
                new(in_target_element) value_type(std::move(io_source_element));
            });
        io_source.clear();
    }

    //-------------------------------------------------------------------------
    /// @brief 要素を格納するメモリ領域。
    private: typename this_type::storage storage_;
    /// @brief コンテナの先頭位置。
    private: typename this_type::pointer begin_;
    /// @brief コンテナの末尾位置。
    private: typename this_type::pointer end_;

#ifndef PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW
    /// @brief デバッグ時にメモリ領域の内容を見るために使う。
    private: typename this_type::array_view& array_view_;
#endif // !defined(PSYQ_STATIC_DEQUE_NO_ARRAY_VIEW)

}; // class static_deque

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    void static_deque()
    {
        typedef psyq::static_deque<float, 16> float_deque;
        float_deque local_deque_a;
        float_deque local_deque_b(5);
        float_deque local_deque_c(float_deque::MAX_SIZE, 0.5f);
        float_deque local_deque_d({1, 2, 3, 4});
        float_deque local_deque_e(std::move(local_deque_c));

        local_deque_a = local_deque_d;
        local_deque_a.push_front(10);
        local_deque_a.push_back(20);
        local_deque_a.pop_front();
        local_deque_a.pop_back();
    }
} // namespace psyq_test

#endif // !defined(PSYQ_STATIC_DEQUE_HPP_)
// vim: set expandtab:
