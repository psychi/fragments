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
#ifndef PSYQ_STRING_FLYWEIGHT_FACTORY_HPP_
#define PSYQ_STRING_FLYWEIGHT_FACTORY_HPP_

/// @brief フライ級文字列生成器の文字列予約数のデフォルト値。
#ifndef PSYQ_STRING_FLYWEIGHT_FACTORY_CAPACITY_DEFAULT
#define PSYQ_STRING_FLYWEIGHT_FACTORY_CAPACITY_DEFAULT 256
#endif // !defined(PSYQ_STRING_FLYWEIGHT_FACTORY_CAPACITY_DEFAULT)

/// @cond
namespace psyq
{
    namespace string
    {
        namespace _private
        {
            template<typename, typename, typename> class flyweight_factory;
        } // namespace
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief フライ級文字列の生成器。
    @tparam template_string_view    @copydoc flyweight_factory::string::view
    @tparam template_hasher         @copydoc flyweight_factory::hash
    @tparam template_allocator_type @copydoc flyweight_factory::allocator_type
 */
template<
    typename template_string_view,
    typename template_hash,
    typename template_allocator_type>
class psyq::string::_private::flyweight_factory
{
    /// @brief thisが指す値の型。
    private: typedef flyweight_factory this_type;

    /// @brief 文字列のハッシュ計算機。
    public: typedef template_hash hash;

    /// @brief メモリ割当子の型。
    public: typedef template_allocator_type allocator_type;

    public: typedef std::shared_ptr<this_type> shared_ptr;
    public: typedef std::weak_ptr<this_type> weak_ptr;

    //-------------------------------------------------------------------------
    /// @brief 生成するフライ級文字列の型。
    private: typedef psyq::string::_private::flyweight_string<
        template_string_view, typename this_type::hash::value_type>
            string;

    /// @brief フライ級文字列のコンテナの型。
    private: typedef std::vector<
        typename this_type::string*,
        typename this_type::allocator_type::template
            rebind<typename this_type::string*>::other>
                string_vector;

    /// @brief 文字列チャンク連結リストのノードの型。
    private: struct string_chunk
    {
        string_chunk(
            string_chunk* const in_next_chunk,
            std::size_t const in_capacity)
        PSYQ_NOEXCEPT:
        next_chunk(in_next_chunk),
        capacity((
            PSYQ_ASSERT(sizeof(string_chunk) < in_capacity), in_capacity)),
        front_string(
            (in_capacity - sizeof(string_chunk)) /
                sizeof(typename flyweight_factory::string::view::value_type),
            flyweight_factory::hash::traits_type::EMPTY)
        {}

        /// @brief 次の文字列チャンクを指すポインタ。
        typename flyweight_factory::string_chunk* next_chunk;
        /// @brief チャンクの大きさ。
        std::size_t capacity;
        /// @brief この文字列チャンクでの最初の文字列。
        typename flyweight_factory::string front_string;

    }; // struct string_chunk

    public: class _private_client;

    //-------------------------------------------------------------------------
    /** @brief フライ級文字列生成器を構築する。
        @param[in] in_reserved_strings フライ級文字列の予約数。
        @param[in] in_allocator        メモリ割当子の初期値。
     */
    public: explicit flyweight_factory(
        std::size_t const in_reserved_strings =
            PSYQ_STRING_FLYWEIGHT_FACTORY_CAPACITY_DEFAULT,
        typename this_type::allocator_type const& in_allocator =
            allocator_type())
    :
    strings_(in_allocator),
    chunk_(nullptr)
    {
        this->strings_.reserve(in_reserved_strings);
    }

    /// @brief コピー構築子は使用禁止。
    private: flyweight_factory(this_type const&);
    /// @brief コピー演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    /// @brief thisを解体する。
    public: ~flyweight_factory()
    {
        // 文字列チャンクをすべて破棄する。
        typename this_type::string_vector::allocator_type::template
            rebind<typename this_type::string_chunk>::other
                local_allocator(this->strings_.get_allocator());
        for (auto local_chunk(this->chunk_); local_chunk != nullptr;)
        {
            // 文字列チャンクの中にある文字列を解体する。
            auto local_string(&local_chunk->front_string);
            auto const local_chunk_end(
                reinterpret_cast<char const*>(local_chunk)
                + local_chunk->capacity);
            for (;;)
            {
                // 文字列チャンクの先頭文字列以外を解体する。
                auto const local_aligned_size(
                    this_type::align_string_size(local_string->size));
                void const* const local_string_end(
                    local_string->data() + local_aligned_size);
                if (local_string != &local_chunk->front_string)
                {
                    local_string->~string();
                }

                // 次の文字列へ移行する。
                auto const local_rest_size(
                    local_chunk_end
                    - static_cast<char const*>(local_string_end));
                PSYQ_ASSERT(0 <= local_rest_size);
                if (local_rest_size < sizeof(typename this_type::string))
                {
                    break;
                }
                local_string = static_cast<typename this_type::string*>(
                    const_cast<void*>(local_string_end));
            }

            // 文字列チャンクを破棄する。
            auto const local_last_chunk(local_chunk);
            auto const local_chunk_count(
                local_chunk->capacity
                / sizeof(typename this_type::string_chunk));
            local_chunk = local_chunk->next_chunk;
            local_last_chunk->~string_chunk();
            local_allocator.deallocate(local_last_chunk, local_chunk_count);
        }
    }

    //-------------------------------------------------------------------------
    /** @brief 同じハッシュ値を持つ文字列を数える。
        @param[in] in_hash 対象となるハッシュ値。
        @return in_hash と同じハッシュ値を持つ文字列の数。
     */
    public: std::size_t count_hash(
        typename this_type::hash::value_type const in_hash)
    const PSYQ_NOEXCEPT
    {
        std::size_t local_count(
            // 空文字列のハッシュ値は予約済なので、必ず1以上となる。
            in_hash == this_type::hash::traits_type::EMPTY);
        for (
            auto i(
                std::lower_bound(
                    this->strings_.begin(),
                    this->strings_.end(),
                    typename this_type::string(0, in_hash),
                    typename this_type::string::less()));
            i != this->strings_.end() && in_hash == (**i).hash;
            ++i)
        {
            ++local_count;
        }
        return local_count;
    }

    /** @brief 文字列のハッシュ値を算出する。
        @param[in] in_string ハッシュ値を算出する文字列。
        @return 文字列のハッシュ値。
     */
    public: static typename this_type::hash::value_type compute_hash(
        typename this_type::string::view const& in_string)
    PSYQ_NOEXCEPT
    {
        auto const local_data(in_string.data());
        return this_type::hash::compute(
            local_data, local_data + in_string.size());
    }

    /** @brief 参照されてない文字列を空にする。
     */
    public: void collect_garbage()
    {
        for (
            auto local_chunk(this->chunk_);
            local_chunk != nullptr;
            local_chunk = local_chunk->next_chunk)
        {
            typename this_type::string* local_empty_string(nullptr);
            auto local_string(&local_chunk->front_string);
            auto const local_chunk_end(
                reinterpret_cast<char const*>(local_chunk)
                + local_chunk->capacity);
            for (;;)
            {
                auto const local_aligned_size(
                    this_type::align_string_size(local_string->size));
                void const* const local_string_end(
                    local_string->data() + local_aligned_size);

                // 未参照の文字列なら、空文字列として回収する。
                if (0 < local_string->reference_count.load())
                {
                    // 参照中の文字列だった。
                    local_empty_string = nullptr;
                }
                else if (local_empty_string != nullptr)
                {
                    // 辞書から未参照の文字列を削除する。
                    auto const local_find_iterator(
                        std::find(
                            this->strings_.begin(),
                            this->strings_.end(),
                            local_string));
                    PSYQ_ASSERT(local_find_iterator != this->strings_.end());
                    this->strings_.erase(local_find_iterator);

                    // 未参照の文字列を解体し、直前の空文字列に追加する。
                    local_string->~string();
                    local_empty_string->size += local_aligned_size +
                        sizeof(typename this_type::string)
                        / sizeof(typename this_type::string::view::value_type);
                }
                else
                {
                    // 未参照の文字列を空文字列にする。
                    local_string->size = local_aligned_size;
                    local_string->hash = this_type::hash::traits_type::EMPTY;
                    local_empty_string = local_string;
                }

                // 次の文字列へ移行する。
                auto const local_rest_size(
                    local_chunk_end
                    - static_cast<char const*>(local_string_end));
                PSYQ_ASSERT(0 <= local_rest_size);
                if (local_rest_size < sizeof(typename this_type::string))
                {
                    if (local_empty_string != nullptr)
                    {
                        local_empty_string->size += local_rest_size / sizeof(
                            typename this_type::string::view::value_type);
                    }
                    break;
                }
                local_string = static_cast<typename this_type::string*>(
                    const_cast<void*>(local_string_end));
            }
        }

        // 辞書をソートする。
        std::sort(
            this->strings_.begin(),
            this->strings_.end(),
            typename this_type::string::less());
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列を辞書に用意する。

        用意する文字列と等価な文字列がすでに存在するなら、
        既存の文字列を再利用する。

        @param[in] in_string 辞書に用意する文字列。
        @param[in] in_chunk_size
            文字列チャンクを生成する場合の、デフォルトのチャンク容量。
     */
    private: typename this_type::string& equip_string(
        typename this_type::string::view const& in_string,
        std::size_t const in_chunk_size)
    {
        // in_string と等価な文字列を、辞書から探す。
        PSYQ_ASSERT(!in_string.empty());
        auto const local_hash(this_type::compute_hash(in_string));
        auto const local_existing_string(
            this_type::find_string(this->strings_, in_string, local_hash));
        if (local_existing_string != nullptr)
        {
            // 辞書にある文字列を再利用する。
            return *local_existing_string;
        }

        // 等価な文字列が辞書になかったので、新たな文字列を登録する。
        auto& local_new_string(
            this->equip_empty_string(in_string.size() + 1, in_chunk_size));
        local_new_string.size = in_string.size();
        local_new_string.hash = local_hash;

        // 文字列をコピーする。
        auto const local_data(
            const_cast<typename this_type::string::view::value_type*>(
                local_new_string.data()));
        this_type::string::view::traits_type::copy(
            local_data, in_string.data(), in_string.size());
        local_data[in_string.size()] = 0;

        // 辞書をソートする。
        std::sort(
            this->strings_.begin(),
            this->strings_.end(),
            typename this_type::string::less());
        return local_new_string;
    }

    /** @brief 辞書から文字列を探す。
        @param[in] in_strings 文字列の辞書。
        @param[in] in_string  探す文字列。
        @param[in] in_hash    探す文字列のハッシュ値。
        @retval !=nullptr 辞書にある文字列。
        @retval ==nullptr 等価な文字列が辞書になかった。
     */
    private: static typename this_type::string* find_string(
        typename this_type::string_vector const& in_strings,
        typename this_type::string::view const& in_string,
        typename this_type::hash::value_type const in_hash)
    PSYQ_NOEXCEPT
    {
        for (
            auto i(
                std::lower_bound(
                    in_strings.begin(),
                    in_strings.end(),
                    typename this_type::string(in_string.size(), in_hash),
                    typename this_type::string::less()));
            i != in_strings.end();
            ++i)
        {
            auto& local_string(**i);
            if (in_hash != local_string.hash)
            {
                break;
            }
            typename this_type::string::view const local_string_view(
                local_string.data(), local_string.size);
            if (in_string == local_string_view)
            {
                return &local_string;
            }
        }
        return nullptr;
    }

    /** @brief 空文字列を用意する。
        @param[in] in_empty_size 空文字列の容量。
        @param[in] in_chunk_size
            文字列チャンクを生成する場合の、デフォルトのチャンク容量。
        @return 用意した空文字列。
     */
    private: typename this_type::string& equip_empty_string(
        typename this_type::string::view::size_type const in_empty_size,
        std::size_t const in_chunk_size)
    {
        // in_empty_size が収まる空文字列を検索する。
        auto const local_necessary_size(
            this_type::align_string_size(in_empty_size));
        auto const local_empty_string(
            this->find_empty_string(local_necessary_size));
        if (local_empty_string != nullptr)
        {
            return *local_empty_string;
        }

        // 該当する空文字列がなかったので、新たに空き文字列を用意する。
        auto& local_new_string(
            this->create_empty_string(in_empty_size, in_chunk_size));
        this->divide_string(local_new_string, local_necessary_size);
        return local_new_string;
    }

    /** @brief 空文字列を探す。
        @param[in] in_empty_size 空文字列の容量。
        @retval !=nullptr 空文字列を指すポインタ。
        @retval ==nullptr 空文字列が見つからなかった。
     */
    private: typename this_type::string* find_empty_string(
        typename this_type::string::view::size_type const in_empty_size)
    {
        for (
            auto i(
                std::lower_bound(
                    this->strings_.begin(),
                    this->strings_.end(),
                    typename this_type::string(
                        in_empty_size, this_type::hash::traits_type::EMPTY),
                    typename this_type::string::less()));
            i != this->strings_.end();
            ++i)
        {
            auto& local_string(**i);
            if (local_string.hash != this_type::hash::traits_type::EMPTY)
            {
                break;
            }
            if (0 < local_string.reference_count.load())
            {
                continue;
            }
            if (local_string.size == in_empty_size
                || this->divide_string(local_string, in_empty_size) != nullptr)
            {
                return &local_string;
            }
        }
        return nullptr;
    }

    /** 文字列を2つに分割し、後方を空文字列として辞書に追加する。
        @param[in] in_string 分割する文字列。
        @param[in] in_size   前方の文字列の要素数。
        @retval !=nullptr 分割した後方の空文字列。
        @retval ==nullptr 失敗。分割できなかった。
     */
    private: typename this_type::string* divide_string(
        typename this_type::string const& in_string,
        typename this_type::string::view::size_type const in_size)
    {
        // in_string の大きさが分割するのに足りてないか判定する。
        static_assert(
            0 == sizeof(typename this_type::string)
                % sizeof(typename this_type::string::view::value_type),
            "");
        PSYQ_ASSERT(in_size == this_type::align_string_size(in_size));
        auto const local_divide_size(
            in_size + sizeof(typename this_type::string)
                / sizeof(typename this_type::string::view::value_type));
        if (in_string.size < local_divide_size)
        {
            return nullptr;
        }

        // in_string の領域の後方を、空文字列として登録する。
        auto const local_empty_block(
            const_cast<typename this_type::string::view::value_type*>(
                in_string.data() + in_size));
        auto const local_empty_string(
            new(local_empty_block) typename this_type::string(
                in_string.size - local_divide_size,
                this_type::hash::traits_type::EMPTY));
        this->strings_.push_back(local_empty_string);
        return local_empty_string;
    }

    /** @brief 空文字列を新たに生成する。
        @param[in] in_string_size 空文字列の容量。
        @param[in] in_chunk_size  文字列チャンクのデフォルトのチャンク容量。
     */
    private: typename this_type::string& create_empty_string(
        typename this_type::string::view::size_type const in_string_size,
        std::size_t const in_chunk_size)
    {
        // 文字列チャンクの容量を決定する。
        auto const local_header_size(sizeof(typename this_type::string_chunk));
        auto const local_chunk_size(
            (std::max)(
                local_header_size * 2 - 1 + in_string_size * sizeof(
                    typename this_type::string::view::value_type),
                in_chunk_size));
        auto const local_chunk_capacity(local_chunk_size / local_header_size);

        // 文字列チャンクを生成する。
        typename this_type::string_vector::allocator_type::template
            rebind<typename this_type::string_chunk>::other
                local_allocator(this->strings_.get_allocator());
        this->chunk_ = new(local_allocator.allocate(local_chunk_capacity))
            typename this_type::string_chunk(
                this->chunk_, local_chunk_capacity * local_header_size);
        PSYQ_ASSERT(this->chunk_ != nullptr);

        // 文字列チャンクの先頭文字列を空文字列として使う。
        auto& local_empty_string(this->chunk_->front_string);
        this->strings_.push_back(&local_empty_string);
        return local_empty_string;
    }

    /** @brief this_type::string のメモリ境界に接するように、文字列の要素数を調整する。
        @param[in] in_string_size 調整前の文字列の要素数。
        @return 調整後の文字列の要素数。
     */
    private:
    static typename this_type::string::view::size_type align_string_size(
        typename this_type::string::view::size_type const in_string_size)
    PSYQ_NOEXCEPT
    {
        auto const local_size(
            sizeof(typename this_type::string::view::value_type));
        auto const local_align(
#ifdef _MSC_VER
            __alignof(typename this_type::string)
#else
            alignof(typename this_type::string)
#endif // defined(_MSC_VER)
            );
        auto const local_count(
            (in_string_size * local_size + local_align - 1) / local_align);
        return (local_count * local_align + local_size - 1) / local_size;
    }

    //-------------------------------------------------------------------------
    /// @brief ハッシュ値をキーにソートされている、フライ級文字列の辞書。
    private: typename this_type::string_vector strings_;
    /// @brief 文字列チャンク連結リストの先頭。
    private: typename this_type::string_chunk* chunk_;

}; // class psyq::string::_private::flyweight_factory

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief フライ級文字列クライアントの基底型。
    @note psyq::string::_private::flyweight_factory 管理者以外は、使用禁止。
    @tparam template_string_view    @copydoc flyweight_factory::string::view
    @tparam template_hasher         @copydoc flyweight_factory::hash
    @tparam template_allocator_type @copydoc flyweight_factory::allocator_type
 */
template<
    typename template_string_view,
    typename template_hash,
    typename template_allocator_type>
class psyq::string::_private::flyweight_factory<
    template_string_view, template_hash, template_allocator_type>
        ::_private_client
{
    /// @brief thisが指す値の型。
    private: typedef _private_client this_type;

    /// @brief フライ級文字列で使う文字列参照の型。
    public: typedef template_string_view view;

    /// @brief 文字特性の型。
    public: typedef typename template_string_view::traits_type traits_type;

    /// @brief フライ級文字列の生成器を表す型。
    public: typedef psyq::string::_private::flyweight_factory<
        template_string_view, template_hash, template_allocator_type>
            factory;

    //-------------------------------------------------------------------------
    private: _private_client() PSYQ_NOEXCEPT: string_(nullptr) {}

    protected: _private_client(this_type const& in_source)
    PSYQ_NOEXCEPT:
    factory_(in_source.get_factory()),
    string_(in_source.string_)
    {
        if (in_source.string_ != nullptr)
        {
            // 文字列の参照数を増やす。
            PSYQ_ASSERT(in_source.get_factory().get() != nullptr);
            in_source.string_->reference_count.add(1);
        }
    }

    protected: _private_client(this_type&& io_source)
    PSYQ_NOEXCEPT:
    factory_(std::move(io_source.factory_)),
    string_(std::move(io_source.string_))
    {
        io_source.factory_.reset();
        io_source.string_ = nullptr;
    }

    private: _private_client(
        typename this_type::factory::shared_ptr const& in_factory,
        typename this_type::factory::string& in_string)
    PSYQ_NOEXCEPT: factory_(in_factory)
    {
        if (in_factory.get() != nullptr)
        {
            // 文字列の参照数を増やす。
            in_string.reference_count.add(1);
            this->string_ = &in_string;
        }
        else
        {
            this->string_ = nullptr;
        }
    }

    public: ~_private_client()
    {
        if (this->string_ != nullptr)
        {
            // 文字列の参照数を減じる。
            PSYQ_ASSERT(this->get_factory().get() != nullptr);
            this->string_->reference_count.sub(1);
        }
    }

    public: this_type& operator=(this_type const& in_source)
    {
        if (this->string_ != in_source.string_)
        {
            if (this->string_ != nullptr)
            {
                // 文字列の参照数を減じる。
                PSYQ_ASSERT(this->get_factory().get() != nullptr);
                this->string_->reference_count.sub(1);
            }
            if (in_source.string_ != nullptr)
            {
                // 文字列の参照数を増やす。
                PSYQ_ASSERT(in_source.get_factory().get() != nullptr);
                in_source.string_->reference_count.add(1);
                this->string_ = in_source.string_;
                this->factory_ = in_source.get_factory();
            }
            else
            {
                this->string_ = nullptr;
                this->factory_.reset();
            }
        }
        return *this;
    }

    public: this_type& operator=(this_type&& io_source)
    {
        if (this->string_ != io_source.string_)
        {
            if (this->string_ != nullptr)
            {
                // 文字列の参照数を減じる。
                PSYQ_ASSERT(this->get_factory().get() != nullptr);
                this->string_->reference_count.sub(1);
            }

            // 文字列を移動する。
            this->factory_ = std::move(io_source.factory_);
            this->string_ = std::move(io_source.string_);
            io_source.factory_.reset();
            io_source.string_ = nullptr;
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    /// @copydoc psyq::string::view::clear
    public: void clear() PSYQ_NOEXCEPT
    {
        if (this->string_ != nullptr)
        {
            // 文字列の参照数を減じる。
            PSYQ_ASSERT(this->get_factory().get() != nullptr);
            this->string_->reference_count.sub(1);
            this->factory_.reset();
        }
        this->string_ = nullptr;
    }

    /// @copydoc psyq::string::view::data
    public: typename this_type::traits_type::char_type const* data()
    const PSYQ_NOEXCEPT
    {
        return this->string_ != nullptr?
            (PSYQ_ASSERT(this->get_factory().get() != nullptr), this->string_->data()):
            nullptr;
    }

    /// @copydoc psyq::string::view::size
    public: typename this_type::factory::string::view::size_type size()
    const PSYQ_NOEXCEPT
    {
        return this->string_ != nullptr?
            (PSYQ_ASSERT(this->get_factory().get() != nullptr), this->string_->size):
            0;
    }

    /// @copydoc psyq::string::view::max_size
    public: std::size_t max_size() const PSYQ_NOEXCEPT
    {
        return (std::numeric_limits<std::size_t>::max)();
    }

    /** @brief 文字列のハッシュ値を取得する。
        @return フライ級文字列辞書で使われているハッシュ値。
     */
    public: typename this_type::factory::hash::value_type get_hash()
    const PSYQ_NOEXCEPT
    {
        return this->string_ != nullptr?
            (PSYQ_ASSERT(this->get_factory().get() != nullptr), this->string_->hash):
            this_type::factory::hash::traits_type::EMPTY;
    }

    /** @brief 文字列が所属するフライ級文字列の生成器を取得する。
        @return 文字列が所属するフライ級文字列の生成器。
     */
    public: typename this_type::factory::shared_ptr const& get_factory()
    const PSYQ_NOEXCEPT
    {
        return this->factory_;
    }

    //-------------------------------------------------------------------------
    protected: static this_type make() PSYQ_NOEXCEPT
    {
        return this_type();
    }

    protected: static this_type make(
        typename this_type::factory::shared_ptr const& in_factory,
        typename this_type::factory::string::view const& in_string,
        std::size_t const in_chunk_size)
    {
        auto const local_factory(in_factory.get());
        return in_string.empty() || local_factory == nullptr?
            this_type():
            this_type(
                in_factory,
                local_factory->equip_string(in_string, in_chunk_size));
    }

    //-------------------------------------------------------------------------
    /// @brief 文字列を所有するフライ級文字列生成器。
    private: typename this_type::factory::shared_ptr factory_;
    /// @brief フライ級文字列。
    private: typename this_type::factory::string* string_;

}; // class psyq::string::_private::flyweight_factory::_private_client

#endif // !PSYQ_STRING_FLYWEIGHT_FACTORY_HPP_
// vim: set expandtab:
