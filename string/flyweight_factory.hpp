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
/// @brief @copybrief psyq::string::_private::flyweight_factory
/// @author Hillco Psychi (https://twitter.com/psychi)
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
            template<typename, typename> class flyweight_factory;
        } // namespace
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief フライ級文字列の生成器。
/// @tparam template_hasher    @copydoc flyweight_factory::hasher
/// @tparam template_allocator @copydoc flyweight_factory::allocator_type
template<typename template_hasher, typename template_allocator>
class psyq::string::_private::flyweight_factory
{
    /// @copydoc psyq::string::view::this_type
    protected: typedef flyweight_factory this_type;

    //-------------------------------------------------------------------------
    /// @brief 文字列からハッシュ値を生成する、ハッシュ関数オブジェクトの型。
    /// @details
    /// - std::hash 互換インターフェイスを持つこと。
    /// - hasher::argument_type
    ///   が文字列型で、以下に相当するメンバ関数を使えること。
    ///   @code
    ///   // 文字列の先頭位置を取得する。
    ///   hasher::argument_type::const_pointer hasher::argument_type::data() const;
    ///   // 文字列の要素数を取得する。
    ///   std::size_t hasher::argument_type::size() const;
    ///   @endcode
    private: typedef template_hasher hasher;
    /// @brief メモリ割当子の型。
    private: typedef template_allocator allocator_type;
    /// @brief this_type を指す、所有権ありスマートポインタ。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// @brief this_type を指す、所有権なしスマートポインタ。
    public: typedef std::weak_ptr<this_type> weak_ptr;

    //-------------------------------------------------------------------------
    /// @brief フライ級文字列の参照子の基底型。
    /// @warning psyq::string 管理者以外がこの型に直接アクセスするのは禁止。
    public: template<bool> class _string_handle;
    /// @brief 生成するフライ級文字列の型。
    private: typedef
        psyq::string::_private::flyweight_string<
            typename this_type::hasher::argument_type,
            typename this_type::hasher::result_type>
        string_placeholder;
    /// @brief フライ級文字列のコンテナの型。
    private: typedef
        std::vector<
            typename this_type::string_placeholder*,
            typename this_type::allocator_type::template
                rebind<typename this_type::string_placeholder*>::other>
        string_container;
    /// @brief 文字列チャンク連結リストのノードの型。
    private: class string_chunk
    {
        private: typedef string_chunk this_type;

        /// @brief 文字列チャンクを構築する。
        public: string_chunk(
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
            flyweight_factory::_get_empty_hash())
        {}

        /// @brief 空の文字列チャンクか判定する。
        public: bool is_empty() const PSYQ_NOEXCEPT
        {
            return this->front_string_.hash_
                   == flyweight_factory::_get_empty_hash()
                && this->front_string_.size_
                   == this_type::compute_string_capacity(this->capacity_);
        }

        /// @brief 文字列チャンクの空き容量から、文字列の空き容量を算出する。
        private: static std::size_t compute_string_capacity(
            /// [in] 文字列チャンクの空き容量。
            std::size_t const in_chunk_capacity)
        {
            return (in_chunk_capacity - sizeof(this_type)) /
                sizeof(typename flyweight_factory::string_placeholder::view::value_type);
        }

        /// @brief 次の文字列チャンクを指すポインタ。
        public: this_type* next_chunk_;
        /// @brief チャンクの大きさ。
        public: std::size_t capacity_;
        /// @brief この文字列チャンクでの最初の文字列。
        public: typename flyweight_factory::string_placeholder front_string_;

    }; // struct string_chunk
    private: typedef typename this_type::allocator_type::template
        rebind<typename this_type::string_chunk>::other
            chunk_allocator;

    //-------------------------------------------------------------------------
    /// @name 構築
    /// @{

    /// @brief フライ級文字列の生成器を構築する。
    public: explicit flyweight_factory(
        /// [in] フライ級文字列の予約数。
        std::size_t const in_reserved_strings =
            PSYQ_STRING_FLYWEIGHT_FACTORY_CAPACITY_DEFAULT,
        /// [in] メモリ割当子の初期値。
        typename this_type::allocator_type const& in_allocator =
            allocator_type()):
    strings_(in_allocator),
    chunk_(nullptr)
    {
        this->strings_.reserve(in_reserved_strings);
    }

    /// @brief *this を解体する。
    public: ~flyweight_factory()
    {
        // 文字列チャンク連結リストを走査し、すべて破棄する。
        typename this_type::chunk_allocator local_allocator(
            this->get_allocator());
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
                    local_string->~string_placeholder();
                }

                // 次の文字列へ移行する。
                auto const local_rest_size(
                    local_chunk_end
                    - static_cast<char const*>(local_string_end));
                PSYQ_ASSERT(0 <= local_rest_size);
                if (local_rest_size < sizeof(typename this_type::string_placeholder))
                {
                    break;
                }
                local_string = static_cast<typename this_type::string_placeholder*>(
                    const_cast<void*>(local_string_end));
            }
        }
    }

    /// @brief メモリ割当子を取得する。
    /// @return メモリ割当子。
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->strings_.get_allocator();
    }

    /// @brief ハッシュ関数を取得する。
    /// @return フライ級文字列生成器のハッシュ関数。
    public: typename this_type::hasher hash_function() const
    {
        return typename this_type::hasher();
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 文字列
    /// @{

    /// @brief 参照されてない文字列を削除する。
    public: void collect_garbage()
    {
        // 文字列チャンク連結リストを走査し、未参照の文字列を片づける。
        typename this_type::chunk_allocator local_allocator(
            this->get_allocator());
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
            typename this_type::string_placeholder::less());
    }

    /// @brief 同じハッシュ値を持つ文字列を数える。
    /// @return in_hash と同じハッシュ値を持つ文字列の数。
    public: std::size_t count_hash(
        /// [in] 数える対象となるハッシュ値。
        typename this_type::hasher::result_type const in_hash)
    const PSYQ_NOEXCEPT
    {
        std::size_t local_count(
            // 空文字列のハッシュ値は予約済なので、必ず1以上となる。
            in_hash == this_type::_get_empty_hash()? 1: 0);
        for (
            auto i(
                std::lower_bound(
                    this->strings_.begin(),
                    this->strings_.end(),
                    typename this_type::string_placeholder(0, in_hash),
                    typename this_type::string_placeholder::less()));
            i != this->strings_.end() && in_hash == (**i).hash_;
            ++i)
        {
            ++local_count;
        }
        return local_count;
    }
    /// @}

    /// @brief 文字列のハッシュ値を算出する。
    /// @warning psyq::string 管理者以外は、この関数は使用禁止。
    /// @return 文字列のハッシュ値。
    public: static typename this_type::hasher::result_type _compute_hash(
        /// [in] ハッシュ値を算出する文字列。
        typename this_type::hasher::argument_type const& in_string)
    PSYQ_NOEXCEPT
    {
        return typename this_type::hasher()(in_string);
    }

    /// @brief 空文字列のハッシュ値を算出する。
    /// @warning psyq::string 管理者以外は、この関数は使用禁止。
    /// @return 空文字列のハッシュ値。
    public: static typename this_type::hasher::result_type _get_empty_hash()
    {
        static auto const static_empty_hash(
            this_type::_compute_hash(
                typename this_type::hasher::argument_type()));
        return static_empty_hash;
    }

    //-------------------------------------------------------------------------
    /// @brief 文字列チャンクにある未参照の文字列を空にする。
    private: static void collect_chunk_garbage(
        /// [in,out] 未参照文字列を片づける文字列チャンク。
        typename this_type::string_chunk& io_chunk,
        /// [in,out] 未参照文字列を片づける文字列の辞書。
        typename this_type::string_container& io_strings)
    {
        auto local_string(&io_chunk.front_string_);
        auto const local_chunk_end(
            reinterpret_cast<char const*>(&io_chunk) + io_chunk.capacity_);
        typename this_type::string_placeholder* local_empty_string(nullptr);
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
                local_string->~string_placeholder();
                local_empty_string->size_ += local_aligned_size
                    + sizeof(typename this_type::string_placeholder) / sizeof(
                        typename this_type::string_placeholder::view::value_type);
            }
            else
            {
                // 未参照文字列を空文字列にする。
                local_string->size_ = local_aligned_size;
                local_string->hash_ = this_type::_get_empty_hash();
                local_empty_string = local_string;
            }

            // 次の文字列へ移行する。
            auto const local_rest_size(
                local_chunk_end - static_cast<char const*>(local_string_end));
            PSYQ_ASSERT(0 <= local_rest_size);
            if (local_rest_size < sizeof(typename this_type::string_placeholder))
            {
                if (local_empty_string != nullptr)
                {
                    local_empty_string->size_ += local_rest_size / sizeof(
                        typename this_type::string_placeholder::view::value_type);
                }
                break;
            }
            local_string = static_cast<typename this_type::string_placeholder*>(
                const_cast<void*>(local_string_end));
        }
    }

    /// @brief 文字列を辞書に用意する。
    /// @details
    /// - in_string と等価な文字列が辞書に存在するなら、
    ///   既存の文字列を再利用する。動的メモリ割当は行わない。
    /// - in_string と等価な文字列が辞書に存在しないなら、
    ///   新たな文字列を構築子辞書に追加する。動的メモリ割当を行う場合がある。
    private: typename this_type::string_placeholder& equip_string(
        /// [in] 辞書に用意する文字列。
        typename this_type::string_placeholder::view const& in_string,
        /// [in] 文字列チャンクを生成する場合の、デフォルトのチャンク容量。
        std::size_t const in_chunk_size)
    {
        // in_string と等価な文字列を、辞書から探す。
        PSYQ_ASSERT(!in_string.empty());
        auto const local_string_data(in_string.data());
        auto const local_string_size(in_string.size());
        auto const local_string_hash(this_type::_compute_hash(in_string));
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
        auto& local_new_string(
            this->make_idle_string(local_string_size, in_chunk_size));
        auto const local_new_data(
            const_cast<typename this_type::string_placeholder::view::value_type*>(
                local_new_string.data()));
        if (local_string_size < local_new_string.size_)
        {
            // 終端文字を入れる余裕があれば、入れておく。
            local_new_data[local_string_size] = 0;
        }
        local_new_string.size_ = local_string_size;
        local_new_string.hash_ = local_string_hash;
        this_type::string_placeholder::view::traits_type::copy(
            local_new_data, local_string_data, local_string_size);

        // 新たな文字列を辞書へ追加する。
        this_type::add_string(this->strings_, local_new_string);
        return local_new_string;
    }

    /// @brief 辞書から文字列を探す。
    /// @return 辞書にある該当文字列を指すポインタ。
    /// ただし該当文字列が辞書にない場合は、 nullptr を返す。
    private: static typename this_type::string_placeholder* find_string(
        /// [in] 文字列の辞書。
        typename this_type::string_container const& in_strings,
        /// [in] 探す文字列の先頭位置。
        typename this_type::string_placeholder::view::const_pointer const in_string_data,
        /// [in] 探す文字列の要素数。
        typename this_type::string_placeholder::view::size_type const in_string_size,
        /// [in] 探す文字列のハッシュ値。
        typename this_type::hasher::result_type const in_string_hash)
    PSYQ_NOEXCEPT
    {
        for (
            auto i(
                std::lower_bound(
                    in_strings.begin(),
                    in_strings.end(),
                    typename this_type::string_placeholder(in_string_size, in_string_hash),
                    typename this_type::string_placeholder::less()));
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
                this_type::string_placeholder::view::traits_type::compare(
                    local_string.data(), in_string_data, in_string_size));
            if (local_compare == 0)
            {
                return &local_string;
            }
        }
        return nullptr;
    }

    /// @brief 未使用の文字列を作る。
    /// @return 未使用文字列。
    private: typename this_type::string_placeholder& make_idle_string(
        /// [in] 未使用文字列の要素数。
        typename this_type::string_placeholder::view::size_type const in_string_size,
        /// [in] 文字列チャンクを生成する場合の、デフォルトのチャンク容量。
        std::size_t const in_chunk_size)
    {
        // in_string_size が収まる空文字列を、動的メモリ割当せず用意する。
        auto const local_required_size(
            this_type::align_string_size(in_string_size));
        auto const local_distributed_string(
            this_type::distribute_idle_string(
                this->strings_, local_required_size));
        if (local_distributed_string != nullptr)
        {
            return *local_distributed_string;
        }

        // 適切な空文字列がなかったので、
        // 文字列チャンクを新たに生成し、未使用文字列を構築する。
        auto& local_new_string(
            this->create_chunk(in_string_size, in_chunk_size));
        auto const local_empty_string(
            this_type::divide_string(local_new_string, local_required_size));
        if (local_empty_string != nullptr)
        {
            this_type::add_string(this->strings_, *local_empty_string);
        }
        return local_new_string;
    }

    /// @brief 動的メモリ割当てをせずに、未使用の文字列を作る。
    /// @return 未使用文字列を指すポインタ。
    /// 未使用文字列が見つからなかった場合は nullptr を返す。
    private: static typename this_type::string_placeholder* distribute_idle_string(
        /// [in,out] 文字列の辞書。
        typename this_type::string_container& io_strings,
        /// [in] 未使用文字列の要素数。
        typename this_type::string_placeholder::view::size_type const in_string_size)
    {
        // in_string_size が収まる空文字列を検索する。
        for (
            auto i(
                std::lower_bound(
                    io_strings.begin(),
                    io_strings.end(),
                    typename this_type::string_placeholder(
                        in_string_size, this_type::_get_empty_hash()),
                    typename this_type::string_placeholder::less()));
            i != io_strings.end();
            ++i)
        {
            auto& local_string(**i);
            if (local_string.hash_ != this_type::_get_empty_hash())
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

    /// @brief 空文字列を2つに分割する。
    /// @return 分割した後側の文字列を指すポインタ。
    /// 分割に失敗した場合は nullptr を返す。
    private: static typename this_type::string_placeholder* divide_string(
        /// [in,out] 分割する空文字列。
        typename this_type::string_placeholder& io_string,
        /// [in] 前側の空文字列に割り当てる要素数。
        typename this_type::string_placeholder::view::size_type const in_front_size)
    {
        // io_string の大きさが分割するのに足りてないか判定する。
        PSYQ_ASSERT(
            in_front_size == this_type::align_string_size(in_front_size)
            && io_string.hash_ == this_type::_get_empty_hash());
        static_assert(
            0 == sizeof(typename this_type::string_placeholder)
                % sizeof(typename this_type::string_placeholder::view::value_type),
            "");
        auto const local_divide_size(
            in_front_size + sizeof(typename this_type::string_placeholder)
                / sizeof(typename this_type::string_placeholder::view::value_type));
        if (io_string.size_ < local_divide_size)
        {
            return nullptr;
        }

        // io_string の後側を、空文字列として構築する。
        auto const local_empty_block(
            const_cast<typename this_type::string_placeholder::view::value_type*>(
                io_string.data() + in_front_size));
        auto const local_empty_string(
            new(local_empty_block) typename this_type::string_placeholder(
                io_string.size_ - local_divide_size,
                this_type::_get_empty_hash()));
        io_string.size_ = in_front_size;
        return local_empty_string;
    }

    /// @brief 文字列を辞書に追加する。
    private: static void add_string(
        /// [in,out] 文字列を追加する辞書。
        typename this_type::string_container& io_strings,
        /// [in] 辞書に追加する文字列。
        typename this_type::string_placeholder& in_string)
    {
        auto const local_lower_bound(
            std::lower_bound(
                io_strings.begin(),
                io_strings.end(),
                in_string,
                typename this_type::string_placeholder::less()));
        PSYQ_ASSERT(
            // 1つの文字列インスタンスを2つ以上追加できない。
            io_strings.end() == local_lower_bound
            || &in_string != *local_lower_bound);
        io_strings.insert(local_lower_bound, &in_string);
    }

    /// @brief 文字列を辞書から削除する。
    private: static void remove_string(
        /// [in,out] 文字列を削除する辞書。
        typename this_type::string_container& io_strings,
        /// [in] 辞書から削除する文字列。
        typename this_type::string_placeholder const& in_string)
    {
        auto const local_find_iterator(
            std::find(io_strings.begin(), io_strings.end(), &in_string));
        PSYQ_ASSERT(local_find_iterator != io_strings.end());
        io_strings.erase(local_find_iterator);
    }

    /// @brief 文字列チャンクを生成し、空文字列を構築する。
    /// @return 文字列チャンクの先頭の空文字列。
    private: typename this_type::string_placeholder& create_chunk(
        /// [in] 空文字列の容量。
        typename this_type::string_placeholder::view::size_type const in_string_size,
        /// [in] 文字列チャンクのデフォルトのチャンク容量。
        std::size_t const in_chunk_size)
    {
        // 文字列チャンクの容量を決定する。
        auto const local_header_size(sizeof(typename this_type::string_chunk));
        auto const local_chunk_size(
            (std::max)(
                local_header_size * 2 - 1 + in_string_size * sizeof(
                    typename this_type::string_placeholder::view::value_type),
                in_chunk_size));
        auto const local_chunk_capacity(local_chunk_size / local_header_size);

        // 文字列チャンクを生成する。
        typename this_type::chunk_allocator local_allocator(
            this->get_allocator());
        this->chunk_ = new(local_allocator.allocate(local_chunk_capacity))
            typename this_type::string_chunk(
                this->chunk_, local_chunk_capacity * local_header_size);
        PSYQ_ASSERT(this->chunk_ != nullptr);
        return this->chunk_->front_string_;
    }

    /// @brief 文字列チャンクを破棄する。
    /// @return 次の文字列チャンクを指すポインタ。
    private: static typename this_type::string_chunk* destroy_chunk(
        /// [in,out] 破棄する文字列チャンク。
        typename this_type::string_chunk& io_chunk,
        /// [in,out] 破棄に使うメモリ割当子。
        typename this_type::chunk_allocator& io_allocator)
    {
        auto const local_next_chunk(io_chunk.next_chunk_);
        auto const local_chunk_count(
            io_chunk.capacity_ / sizeof(typename this_type::string_chunk));
        io_chunk.~string_chunk();
        io_allocator.deallocate(&io_chunk, local_chunk_count);
        return local_next_chunk;
    }

    /** @brief this_type::string_placeholder のメモリ境界に接するように、文字列の要素数を調整する。
        @return 調整後の文字列の要素数。
     */
    private:
    static typename this_type::string_placeholder::view::size_type align_string_size(
        /// [in] 調整前の文字列の要素数。
        typename this_type::string_placeholder::view::size_type const in_string_size)
    PSYQ_NOEXCEPT
    {
        auto const local_size(
            sizeof(typename this_type::string_placeholder::view::value_type));
        auto const local_align(PSYQ_ALIGNOF(typename this_type::string_placeholder));
        auto const local_count(
            (in_string_size * local_size + local_align - 1) / local_align);
        return (local_count * local_align + local_size - 1) / local_size;
    }

    //-------------------------------------------------------------------------
    /// @brief 生成器を指す強参照スマートポインタを取得する。
    private: static typename this_type::shared_ptr const& get_shared_ptr(
        typename this_type::shared_ptr const& in_factory)
    {
        return in_factory;
    }

    /// @copydoc this_type::get_shared_ptr
    private: static typename this_type::shared_ptr get_shared_ptr(
        typename this_type::weak_ptr const& in_factory)
    {
        return in_factory.lock();
    }

    /// @brief 生成器を指すスマートポインタが空か判定する。
    private: static bool is_empty(
        /// [in] 判定するスマートポインタ。
        typename this_type::shared_ptr const& in_factory)
    PSYQ_NOEXCEPT
    {
        return in_factory.get() == nullptr;
    }

    /// @brief this_type::is_empty
    private: static bool is_empty(
        /// [in] 判定するスマートポインタ。
        typename this_type::weak_ptr const& in_factory)
    PSYQ_NOEXCEPT
    {
        return in_factory.expired();
    }

    /// @brief フライ級文字列の参照権がないか判定する。
    private: template<typename template_smart_ptr>
    static bool is_empty(
        /// @brief 判定するフライ級文字列の生成器を指すスマートポインタ。
        template_smart_ptr const& in_factory,
        /// @brief 判定するフライ級文字列を指すポインタ。
        typename this_type::string_placeholder const* const in_string)
    PSYQ_NOEXCEPT
    {
        return in_string == nullptr || this_type::is_empty(in_factory);
    }

    /// @brief フライ級文字列の参照権を取得する。
    /// @return
    ///   参照権を取得したフライ級文字列を指すポインタ。
    ///   参照権を取得できなかった場合は nullptr を返す。
    private: template<typename template_smart_ptr>
    static typename this_type::string_placeholder* hold_string(
        /// @brief 参照するフライ級文字列の生成器を指すスマートポインタ。
        template_smart_ptr const& in_factory,
        /// @brief 参照するフライ級文字列を指すポインタ。
        typename this_type::string_placeholder* const io_string)
    PSYQ_NOEXCEPT
    {
        auto const& local_lock(this_type::get_shared_ptr(in_factory));
        if (this_type::is_empty(in_factory, io_string))
        {
            return nullptr;
        }
        io_string->reference_count_.add(1);
        return io_string;
    }

    /// @brief フライ級文字列の参照権を破棄する。
    private: template<typename template_smart_ptr>
    static void release_string(
        /// @brief 参照権を破棄するフライ級文字列の生成器を指すスマートポインタ。
        template_smart_ptr const& in_factory,
        /// @brief 参照権を破棄するフライ級文字列を指すポインタ。
        typename this_type::string_placeholder* const io_string)
    PSYQ_NOEXCEPT
    {
        auto const& local_lock(this_type::get_shared_ptr(in_factory));
        if (!this_type::is_empty(in_factory, io_string))
        {
            io_string->reference_count_.sub(1);
        }
    }

    //-------------------------------------------------------------------------
    /// @brief コピー構築子は使用禁止。
    private: flyweight_factory(this_type const&);
    /// @brief コピー演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    //-------------------------------------------------------------------------
    /// @brief ハッシュ値をキーにソートされている、フライ級文字列の辞書。
    private: typename this_type::string_container strings_;
    /// @brief 文字列チャンク連結リストの先頭。
    private: typename this_type::string_chunk* chunk_;

}; // class psyq::string::_private::flyweight_factory

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief フライ級文字列の参照子。
/// @tparam template_hasher    @copydoc flyweight_factory::hasher
/// @tparam template_allocator @copydoc flyweight_factory::allocator_type
/// @tparam template_weak      弱参照かどうか。
template<typename template_hasher, typename template_allocator>
template<bool template_weak>
class psyq::string::_private::flyweight_factory<
    template_hasher, template_allocator>
::_string_handle
{
    /// @copydoc psyq::string::view::this_type
    protected: typedef _string_handle this_type;

    //-------------------------------------------------------------------------
    /// @brief フライ級文字列の生成器を表す型。
    public: typedef
        psyq::string::_private::flyweight_factory<
            template_hasher, template_allocator>
        factory;
    /// @brief フライ級文字列で使う文字列参照の型。
    public: typedef typename this_type::factory::string_placeholder::view view;
    /// @brief 文字特性の型。
    public: typedef typename this_type::view::traits_type traits_type;
    /// @copydoc this_type::factory::hasher
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
    public: ~_string_handle()
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
    public: typename this_type::factory::string_placeholder::view::size_type size()
    const PSYQ_NOEXCEPT
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
    public: typename this_type::factory::hasher::result_type get_hash()
    const PSYQ_NOEXCEPT
    {
        return this->empty()?
            this_type::factory::_get_empty_hash(): this->string_->hash_;
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
        typename this_type::factory::string_placeholder::view const& in_right)
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
        typename this_type::factory::string_placeholder::view const& in_right_string,
        /// [in] 右辺の文字列のハッシュ値。
        typename this_type::factory::hasher::result_type const in_right_hash)
    const PSYQ_NOEXCEPT
    {
        return this_type::_compare_fast(
            *this, this->get_hash(), in_right_string, in_right_hash);
    }
    /// @}

    /// @copydoc _compare_fast
    public: static int _compare_fast(
        /// [in] 左辺の文字列。
        typename this_type::factory::string_placeholder::view const& in_left,
        /// [in] 右辺の文字列。
        typename this_type::factory::string_placeholder::view const& in_right)
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
        typename this_type::factory::string_placeholder::view const& in_left_string,
        /// [in] 左辺の文字列のハッシュ値。
        typename this_type::factory::hasher::result_type const in_left_hash,
        /// [in] 右辺の文字列。
        typename this_type::factory::string_placeholder::view const& in_right_string,
        /// [in] 右辺の文字列のハッシュ値。
        typename this_type::factory::hasher::result_type const in_right_hash)
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
        return this_type::factory::string_placeholder::view::traits_type::compare(
            in_left_string.data(), in_right_string.data(), local_right_size);
    }

    //-------------------------------------------------------------------------
    /// @brief 空の参照子を構築する。
    protected: _string_handle() PSYQ_NOEXCEPT: string_(nullptr) {}

    /// @brief コピー構築子。
    protected: _string_handle(
        /// [in] コピー元となるインスタンス。
        this_type const& in_source)
    PSYQ_NOEXCEPT:
    factory_(in_source.get_factory()),
    string_(
        this_type::factory::hold_string(
            in_source.get_factory(), in_source.string_))
    {}

    /// @brief ムーブ構築子。
    protected: _string_handle(
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
        typename this_type::factory::string_placeholder::view const& in_string,
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
    private: _string_handle(
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

}; // class psyq::string::_private::flyweight_factory::_string_handle

#endif // !PSYQ_STRING_FLYWEIGHT_FACTORY_HPP_
// vim: set expandtab:
