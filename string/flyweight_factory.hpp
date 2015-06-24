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

#include <memory>
#include <vector>
#include "./flyweight_string.hpp"

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
    private: typedef
        psyq::string::_private::flyweight_string<
            template_string_view, typename this_type::hash::value_type>
        string;

    /// @brief フライ級文字列のコンテナの型。
    private: typedef
        std::vector<
            typename this_type::string*,
            typename this_type::allocator_type::template
                rebind<typename this_type::string*>::other>
        string_container;

    /// @brief 文字列チャンク連結リストのノードの型。
    private: class string_chunk
    {
        private: typedef string_chunk this_type;

        public: string_chunk(
            this_type* const in_next_chunk,
            std::size_t const in_capacity)
        PSYQ_NOEXCEPT:
        next_chunk_(in_next_chunk),
        capacity_((
            PSYQ_ASSERT(sizeof(this_type) < in_capacity), in_capacity)),
        front_string_(
            this_type::compute_string_capacity(in_capacity),
            flyweight_factory::hash::traits_type::EMPTY)
        {}

        public: bool is_empty() const PSYQ_NOEXCEPT
        {
            return this->front_string_.hash_
                   == flyweight_factory::hash::traits_type::EMPTY
                && this->front_string_.size_
                   == this_type::compute_string_capacity(this->capacity_);
        }

        public: static std::size_t compute_string_capacity(
            std::size_t const in_chunk_capacity)
        {
            return (in_chunk_capacity - sizeof(this_type)) /
                sizeof(typename flyweight_factory::string::view::value_type);
        }

        /// @brief 次の文字列チャンクを指すポインタ。
        public: this_type* next_chunk_;
        /// @brief チャンクの大きさ。
        public: std::size_t capacity_;
        /// @brief この文字列チャンクでの最初の文字列。
        public: typename flyweight_factory::string front_string_;

    }; // struct string_chunk

    private: typedef typename this_type::allocator_type::template
        rebind<typename this_type::string_chunk>::other
            chunk_allocator;

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
        // 文字列チャンク連結リストを走査し、すべて破棄する。
        typename this_type::chunk_allocator local_allocator(
            this->strings_.get_allocator());
        for (
            auto local_chunk(this->chunk_);
            local_chunk != nullptr;
            local_chunk = this_type::destroy_chunk(
                *local_chunk, local_allocator))
        {
            // 文字列チャンクの中にある文字列を解体する。
            auto local_string(&local_chunk->front_string_);
            auto const local_chunk_end(
                reinterpret_cast<char const*>(local_chunk)
                + local_chunk->capacity_);
            for (;;)
            {
                // 文字列チャンクの先頭文字列以外を解体する。
                auto const local_aligned_size(
                    this_type::align_string_size(local_string->size_));
                void const* const local_string_end(
                    local_string->data() + local_aligned_size);
                if (local_string != &local_chunk->front_string_)
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
            i != this->strings_.end() && in_hash == (**i).hash_;
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

    /** @brief 未参照の文字列を空にする。
     */
    public: void collect_garbage()
    {
        // 文字列チャンク連結リストを走査し、未参照の文字列を片づける。
        typename this_type::chunk_allocator local_allocator(
            this->strings_.get_allocator());
        auto local_chunk_link(&this->chunk_);
        for (auto local_chunk(this->chunk_); local_chunk != nullptr;)
        {
            this_type::collect_chunk_garbage(*local_chunk, this->strings_);
            if (local_chunk->is_empty())
            {
                // 文字列チャンクが空になったので、破棄する。
                this_type::remove_string(
                    this->strings_, local_chunk->front_string_);
                local_chunk = this_type::destroy_chunk(
                    *local_chunk, local_allocator);
                *local_chunk_link = local_chunk;
            }
            else
            {
                local_chunk_link = &local_chunk->next_chunk_;
                local_chunk = local_chunk->next_chunk_;
            }
        }

        // 文字列辞書をソートする。
        std::sort(
            this->strings_.begin(),
            this->strings_.end(),
            typename this_type::string::less());
    }

    /** @brief 文字列チャンクにある未参照の文字列を空にする。
        @param[in,out] io_chunk   未参照文字列を片づける文字列チャンク。
        @param[in,out] io_strings 未参照文字列を片づける文字列の辞書。
     */
    private: static void collect_chunk_garbage(
        typename this_type::string_chunk& io_chunk,
        typename this_type::string_container& io_strings)
    {
        auto local_string(&io_chunk.front_string_);
        auto const local_chunk_end(
            reinterpret_cast<char const*>(&io_chunk) + io_chunk.capacity_);
        typename this_type::string* local_empty_string(nullptr);
        for (;;)
        {
            auto const local_aligned_size(
                this_type::align_string_size(local_string->size_));
            void const* const local_string_end(
                local_string->data() + local_aligned_size);

            // 未参照文字列を、空文字列として回収する。
            if (0 < local_string->reference_count_.load())
            {
                // 参照中の文字列なので何もしない。
                local_empty_string = nullptr;
            }
            else if (local_empty_string != nullptr)
            {
                // 未参照文字列を解体し、直前の空文字列に追加する。
                this_type::remove_string(io_strings, *local_string);
                local_string->~string();
                local_empty_string->size_ += local_aligned_size +
                    sizeof(typename this_type::string)
                    / sizeof(typename this_type::string::view::value_type);
            }
            else
            {
                // 未参照文字列を空文字列にする。
                local_string->size_ = local_aligned_size;
                local_string->hash_ = this_type::hash::traits_type::EMPTY;
                local_empty_string = local_string;
            }

            // 次の文字列へ移行する。
            auto const local_rest_size(
                local_chunk_end - static_cast<char const*>(local_string_end));
            PSYQ_ASSERT(0 <= local_rest_size);
            if (local_rest_size < sizeof(typename this_type::string))
            {
                if (local_empty_string != nullptr)
                {
                    local_empty_string->size_ += local_rest_size / sizeof(
                        typename this_type::string::view::value_type);
                }
                break;
            }
            local_string = static_cast<typename this_type::string*>(
                const_cast<void*>(local_string_end));
        }
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
        auto const local_string_data(in_string.data());
        auto const local_string_size(in_string.size());
        auto const local_string_hash(
            this_type::hash::compute(
                local_string_data, local_string_data + local_string_size));
        auto const local_existing_string(
            this_type::find_string(
                this->strings_,
                local_string_data,
                local_string_size,
                local_string_hash));
        if (local_existing_string != nullptr)
        {
            // 辞書にある文字列を再利用する。
            return *local_existing_string;
        }

        // 等価な文字列が辞書になかったので、新たな文字列を用意する。
        auto& local_idle_string(
            this->make_idle_string(local_string_size, in_chunk_size));
        auto const local_idle_data(
            const_cast<typename this_type::string::view::value_type*>(
                local_idle_string.data()));
        if (local_string_size < local_idle_string.size_)
        {
            // 終端文字を入れる余裕があれば、入れておく。
            local_idle_data[local_string_size] = 0;
        }
        local_idle_string.size_ = local_string_size;
        local_idle_string.hash_ = local_string_hash;
        this_type::string::view::traits_type::copy(
            local_idle_data, local_string_data, local_string_size);

        // 新たな文字列を辞書へ追加する。
        this_type::add_string(this->strings_, local_idle_string);
        return local_idle_string;
    }

    /** @brief 辞書から文字列を探す。
        @param[in] in_strings 文字列の辞書。
        @param[in] in_string_data 探す文字列の先頭位置。
        @param[in] in_string_size 探す文字列の要素数。
        @param[in] in_string_hash 探す文字列のハッシュ値。
        @retval !=nullptr 辞書にある文字列。
        @retval ==nullptr 等価な文字列が辞書になかった。
     */
    private: static typename this_type::string* find_string(
        typename this_type::string_container const& in_strings,
        typename this_type::string::view::const_pointer const in_string_data,
        typename this_type::string::view::size_type const in_string_size,
        typename this_type::hash::value_type const in_string_hash)
    PSYQ_NOEXCEPT
    {
        for (
            auto i(
                std::lower_bound(
                    in_strings.begin(),
                    in_strings.end(),
                    typename this_type::string(in_string_size, in_string_hash),
                    typename this_type::string::less()));
            i != in_strings.end();
            ++i)
        {
            auto& local_string(**i);
            if (in_string_size != local_string.size_
                || in_string_hash != local_string.hash_)
            {
                break;
            }
            auto const local_compare(
                this_type::string::view::traits_type::compare(
                    local_string.data(), in_string_data, in_string_size));
            if (local_compare == 0)
            {
                return &local_string;
            }
        }
        return nullptr;
    }

    /** @brief 未使用の文字列を作る。
        @param[in] in_string_size 未使用文字列の要素数。
        @param[in] in_chunk_size
            文字列チャンクを生成する場合の、デフォルトのチャンク容量。
        @return 未使用文字列。
     */
    private: typename this_type::string& make_idle_string(
        typename this_type::string::view::size_type const in_string_size,
        std::size_t const in_chunk_size)
    {
        // in_string_size が収まる空文字列を、動的メモリ割当てをせずに作る。
        auto const local_required_size(
            this_type::align_string_size(in_string_size));
        auto const local_bring_string(
            this_type::bring_idle_string(this->strings_, local_required_size));
        if (local_bring_string != nullptr)
        {
            return *local_bring_string;
        }

        // 適切な空文字列がなかったので、
        // 文字列チャンクを新たに生成し、未使用文字列を構築する。
        auto& local_idle_string(
            this->create_chunk(in_string_size, in_chunk_size));
        auto const local_empty_string(
            this_type::divide_string(local_idle_string, local_required_size));
        if (local_empty_string != nullptr)
        {
            this_type::add_string(this->strings_, *local_empty_string);
        }
        return local_idle_string;
    }

    /** @brief 動的メモリ割当てをせずに、未使用の文字列を作る。
        @param[in,out] io_strings 文字列の辞書。
        @param[in] in_string_size 未使用文字列の要素数。
        @retval !=nullptr 未使用文字列を指すポインタ。
        @retval ==nullptr 未使用文字列が見つからなかった。
     */
    private: static typename this_type::string* bring_idle_string(
        typename this_type::string_container& io_strings,
        typename this_type::string::view::size_type const in_string_size)
    {
        // in_string_size が収まる空文字列を検索する。
        for (
            auto i(
                std::lower_bound(
                    io_strings.begin(),
                    io_strings.end(),
                    typename this_type::string(
                        in_string_size, this_type::hash::traits_type::EMPTY),
                    typename this_type::string::less()));
            i != io_strings.end();
            ++i)
        {
            auto& local_string(**i);
            if (local_string.hash_ != this_type::hash::traits_type::EMPTY)
            {
                break;
            }
            if (0 < local_string.reference_count_.load())
            {
                continue;
            }

            // 同じ大きさの空文字列があるなら、
            // いったん辞書から削除してから利用する。
            if (in_string_size == local_string.size_)
            {
                io_strings.erase(i);
                return &local_string;
            }

            // 空文字列を分割できるなら、
            // 分割された文字列をいったん辞書から削除してから利用する。
            auto const local_empty_string(
                this_type::divide_string(local_string, in_string_size));
            if (local_empty_string != nullptr)
            {
                io_strings.erase(i);
                this_type::add_string(io_strings, *local_empty_string);
                return &local_string;
            }
        }
        return nullptr;
    }

    /** @brief 空文字列を2つに分割する。
        @param[in,out] io_string 分割する空文字列。
        @param[in] in_front_size 前側の空文字列に割り当てる要素数。
        @retval !=nullptr 分割した後側の文字列。
        @retval ==nullptr 失敗。分割できなかった。
     */
    private: static typename this_type::string* divide_string(
        typename this_type::string& io_string,
        typename this_type::string::view::size_type const in_front_size)
    {
        // io_string の大きさが分割するのに足りてないか判定する。
        PSYQ_ASSERT(
            in_front_size == this_type::align_string_size(in_front_size)
            && io_string.hash_ == this_type::hash::traits_type::EMPTY);
        static_assert(
            0 == sizeof(typename this_type::string)
                % sizeof(typename this_type::string::view::value_type),
            "");
        auto const local_divide_size(
            in_front_size + sizeof(typename this_type::string)
                / sizeof(typename this_type::string::view::value_type));
        if (io_string.size_ < local_divide_size)
        {
            return nullptr;
        }

        // io_string の後側を、空文字列として構築する。
        auto const local_empty_block(
            const_cast<typename this_type::string::view::value_type*>(
                io_string.data() + in_front_size));
        auto const local_empty_string(
            new(local_empty_block) typename this_type::string(
                io_string.size_ - local_divide_size,
                this_type::hash::traits_type::EMPTY));
        io_string.size_ = in_front_size;
        return local_empty_string;
    }

    /** @brief 文字列を辞書に追加する。
        @param[in,out] io_strings 文字列を追加する辞書。
        @param[in] in_string      辞書に追加する文字列。
     */
    private: static void add_string(
         typename this_type::string_container& io_strings,
         typename this_type::string& in_string)
    {
        auto const local_lower_bound(
            std::lower_bound(
                io_strings.begin(),
                io_strings.end(),
                in_string,
                typename this_type::string::less()));
        PSYQ_ASSERT(
            // 1つの文字列インスタンスを2つ以上追加できない。
            io_strings.end() == local_lower_bound
            || &in_string != *local_lower_bound);
        io_strings.insert(local_lower_bound, &in_string);
    }

    /** @brief 文字列を辞書から削除する。
        @param[in,out] io_strings 文字列を削除する辞書。
        @param[in] in_string      辞書から削除する文字列。
     */
    private: static void remove_string(
        typename this_type::string_container& io_strings,
        typename this_type::string const& in_string)
    {
        auto const local_find_iterator(
            std::find(io_strings.begin(), io_strings.end(), &in_string));
        PSYQ_ASSERT(local_find_iterator != io_strings.end());
        io_strings.erase(local_find_iterator);
    }

    /** @brief 文字列チャンクを生成し、空文字列を構築する。
        @param[in] in_string_size 空文字列の容量。
        @param[in] in_chunk_size  文字列チャンクのデフォルトのチャンク容量。
        @return 文字列チャンクの先頭の空文字列。
     */
    private: typename this_type::string& create_chunk(
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
        typename this_type::chunk_allocator local_allocator(
            this->strings_.get_allocator());
        this->chunk_ = new(local_allocator.allocate(local_chunk_capacity))
            typename this_type::string_chunk(
                this->chunk_, local_chunk_capacity * local_header_size);
        PSYQ_ASSERT(this->chunk_ != nullptr);
        return this->chunk_->front_string_;
    }

    /** @brief 文字列チャンクを破棄する。
        @param[in,out] io_chunk     破棄する文字列チャンク。
        @param[in,out] io_allocator 破棄に使うメモリ割当子。
        @return 次の文字列チャンクを指すポインタ。
     */
    private: static typename this_type::string_chunk* destroy_chunk(
        typename this_type::string_chunk& io_chunk,
        typename this_type::chunk_allocator& io_allocator)
    {
        auto const local_next_chunk(io_chunk.next_chunk_);
        auto const local_chunk_count(
            io_chunk.capacity_ / sizeof(typename this_type::string_chunk));
        io_chunk.~string_chunk();
        io_allocator.deallocate(&io_chunk, local_chunk_count);
        return local_next_chunk;
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
    private: typename this_type::string_container strings_;
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
    public: typedef
        psyq::string::_private::flyweight_factory<
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
            in_source.string_->reference_count_.add(1);
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
        typename this_type::factory::shared_ptr in_factory,
        typename this_type::factory::string& in_string)
    PSYQ_NOEXCEPT: factory_(std::move(in_factory))
    {
        if (this->get_factory().get() != nullptr)
        {
            // 文字列の参照数を増やす。
            in_string.reference_count_.add(1);
            this->string_ = &in_string;
        }
        else
        {
            PSYQ_ASSERT(false);
            this->string_ = nullptr;
        }
    }

    public: ~_private_client()
    {
        if (this->string_ != nullptr)
        {
            // 文字列の参照数を減じる。
            PSYQ_ASSERT(this->get_factory().get() != nullptr);
            this->string_->reference_count_.sub(1);
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
                this->string_->reference_count_.sub(1);
            }
            if (in_source.string_ != nullptr)
            {
                // 文字列の参照数を増やす。
                PSYQ_ASSERT(in_source.get_factory().get() != nullptr);
                in_source.string_->reference_count_.add(1);
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
                this->string_->reference_count_.sub(1);
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
            this->string_->reference_count_.sub(1);
            this->factory_.reset();
            this->string_ = nullptr;
        }
    }

    /// @copydoc psyq::string::view::data
    public: typename this_type::traits_type::char_type const* data()
    const PSYQ_NOEXCEPT
    {
        if (this->string_ == nullptr)
        {
            return nullptr;
        }
        PSYQ_ASSERT(this->get_factory().get() != nullptr);
        return this->string_->data();
    }

    /// @copydoc psyq::string::view::size
    public: typename this_type::factory::string::view::size_type size()
    const PSYQ_NOEXCEPT
    {
        if (this->string_ == nullptr)
        {
            return 0;
        }
        PSYQ_ASSERT(this->get_factory().get() != nullptr);
        return this->string_->size_;
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
        if (this->string_ == nullptr)
        {
            return this_type::factory::hash::traits_type::EMPTY;
        }
        PSYQ_ASSERT(this->get_factory().get() != nullptr);
        return this->string_->hash_;
    }

    /** @brief 参照しているフライ級文字列の生成器を取得する。
        @return 参照しているフライ級文字列の生成器。
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
        typename this_type::factory::shared_ptr in_factory,
        typename this_type::factory::string::view const& in_string,
        std::size_t const in_chunk_size)
    {
        auto const local_factory(in_factory.get());
        return in_string.empty() || local_factory == nullptr?
            this_type():
            this_type(
                std::move(in_factory),
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
