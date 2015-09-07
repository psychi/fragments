/// @file
/// @brief ハッシュ関数オブジェクトの代理オブジェクト。
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_HASH_PROXY_HPP_
#define PSYQ_HASH_PROXY_HPP_

#include <type_traits>
#include "../assert.hpp"

namespace psyq
{
    /// @brief std::hash 互換インタフェイスを持つハッシュ関数オブジェクト。
    namespace hash
    {
        /// @brief psyq::hash 管理者以外がこの名前空間に直接アクセスするのは禁止。
        namespace _private
        {
            template<typename, typename> class basic_bytes_hash_proxy;
            template<typename> class seedless_bytes_hash_proxy;
            template<typename, typename template_seed, template_seed>
                class seeding_bytes_hash_proxy;
            template<typename, typename>
                class runtime_seeding_bytes_hash_proxy;
            template<typename, typename, typename> class string_hash_proxy;

        } // namespace _private
    } // namespace hash
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief バイト列ハッシュ関数代理オブジェクトの基底型。
/// @tparam template_bytes_hasher @copydoc basic_bytes_hash_proxy::hasher
/// @tparam template_result       @copydoc basic_bytes_hash_proxy::result_type
template<typename template_bytes_hasher, typename template_result>
class psyq::hash::_private::basic_bytes_hash_proxy
{
    /// @copydoc psyq::string::view::this_type
    private: typedef basic_bytes_hash_proxy this_type;

    //-------------------------------------------------------------------------
    /// @brief 委譲先となるバイト列ハッシュ関数オブジェクト。
    public: typedef template_bytes_hasher hasher;
    /// @brief バイト列ハッシュ関数の戻り値。
    public: typedef template_result result_type;

    //-------------------------------------------------------------------------
    /// @brief 委譲先のバイト列ハッシュ関数オブジェクトを参照する。
    /// @return 委譲先のバイト列ハッシュ関数オブジェクトへの参照。
    public: typename this_type::hasher const& get_hasher() const PSYQ_NOEXCEPT
    {
        return this->hasher_;
    }

    //-------------------------------------------------------------------------
    /// バイト列ハッシュ関数オブジェクトを構築する。
    protected: explicit basic_bytes_hash_proxy(
        /// @[in] 委譲先となるバイト列ハッシュ関数オブジェクト。
        typename this_type::hasher in_hasher):
    hasher_(std::move(in_hasher))
    {}

    //-------------------------------------------------------------------------
    /// @brief @copybrief hasher
    protected: typename this_type::hasher hasher_;

}; // class psyq::hash::_private::basic_bytes_hash_proxy

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief バイト列ハッシュ関数代理オブジェクト。
/// @tparam template_bytes_hasher
/// 委譲先となるバイト列ハッシュ関数オブジェクト。
/// 以下に相当するメンバ関数が実装されていること。
/// @code
/// // brief バイト列のハッシュ値を算出する。
/// // param[in] in_data バイト列の先頭位置。
/// // param[in] in_size バイト列のバイト数。
/// // return バイト列のハッシュ値。
/// result_type template_bytes_hasher::operator()(unsigned char const* const in_data, std::size_t const in_size) const noexcept;
/// @endcode
template<typename template_bytes_hasher>
class psyq::hash::_private::seedless_bytes_hash_proxy:
public psyq::hash::_private::basic_bytes_hash_proxy<
    template_bytes_hasher,
    typename std::result_of<
        template_bytes_hasher(unsigned char const* const, std::size_t const)>
    ::type>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef seedless_bytes_hash_proxy this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef 
        psyq::hash::_private::basic_bytes_hash_proxy<
            template_bytes_hasher,
            typename std::result_of<
                template_bytes_hasher(unsigned char const* const, std::size_t const)>
            ::type>
        base_type;

    //-------------------------------------------------------------------------
    /// バイト列ハッシュ関数オブジェクトを構築する。
    public: explicit seedless_bytes_hash_proxy(
        /// @[in] 委譲先となるバイト列ハッシュ関数オブジェクト。
        typename base_type::hasher in_hasher):
    base_type(std::move(in_hasher))
    {}

    /// @brief バイト列のハッシュ値を算出する。
    /// @return バイト列のハッシュ値。
    public: typename base_type::result_type operator()(
        /// [in] バイト列の先頭位置。
        void const* const in_begin,
        /// [in] バイト列の末尾位置。
        void const* const in_end)
    const PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_begin <= in_end);
        auto const local_begin(static_cast<unsigned char const*>(in_begin));
        return (*this)(
            local_begin,
            static_cast<unsigned char const*>(in_end) - local_begin);
    }

    /// @brief バイト列のハッシュ値を算出する。
    /// @return バイト列のハッシュ値。
    public: typename base_type::result_type operator()(
        /// [in] バイト列の先頭位置。
        unsigned char const* const in_bytes,
        /// [in] バイト列のバイト数。
        std::size_t const in_size)
    const PSYQ_NOEXCEPT
    {
        return this->hasher_(in_bytes, in_size);
    }

}; // class psyq::hash::_private::seedless_bytes_hash_proxy

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief シード値つきバイト列ハッシュ関数代理オブジェクト。
/// @tparam template_bytes_hasher
/// 委譲先となるバイト列ハッシュ関数オブジェクト。
/// 以下に相当するメンバ関数が実装されていること。
/// @code
/// // brief バイト列のハッシュ値を算出する。
/// // param[in] in_data バイト列の先頭位置。
/// // param[in] in_size バイト列のバイト数。
/// // param[in] in_seed ハッシュ関数のシード値。
/// // return バイト列のハッシュ値。
/// result_type template_bytes_hasher::operator()(unsigned char const* const in_data, std::size_t const in_size, template_seed const& in_seed) const noexcept;
/// @endcode
/// @tparam template_seed       @copydoc seeding_bytes_hash_proxy::seed
/// @tparam template_seed_value ハッシュ関数のシード値。
template<
    typename template_bytes_hasher,
    typename template_seed,
    template_seed template_seed_value>
class psyq::hash::_private::seeding_bytes_hash_proxy:
public psyq::hash::_private::basic_bytes_hash_proxy<
    template_bytes_hasher,
    typename std::result_of<
        template_bytes_hasher(
            unsigned char const* const,
            std::size_t const,
            template_seed const&)>
    ::type>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef seeding_bytes_hash_proxy this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef 
        psyq::hash::_private::basic_bytes_hash_proxy<
            template_bytes_hasher,
            typename std::result_of<
                template_bytes_hasher(
                    unsigned char const* const,
                    std::size_t const,
                    template_seed const&)>
            ::type>
        base_type;
    /// @brief ハッシュ関数のシード値の型。
    public: typedef template_seed seed;

    //-------------------------------------------------------------------------
    /// バイト列ハッシュ関数オブジェクトを構築する。
    public: explicit seeding_bytes_hash_proxy(
        /// @[in] 委譲先となるバイト列ハッシュ関数オブジェクト。
        typename base_type::hasher in_hasher):
    base_type(std::move(in_hasher))
    {}

    /// @brief バイト列のハッシュ値を算出する。
    /// @return バイト列のハッシュ値。
    public: typename base_type::result_type operator()(
        /// [in] バイト列の先頭位置。
        void const* const in_begin,
        /// [in] バイト列の末尾位置。
        void const* const in_end)
    const PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_begin <= in_end);
        auto const local_begin(static_cast<unsigned char const*>(in_begin));
        return (*this)(
            local_begin,
            static_cast<unsigned char const*>(in_end) - local_begin);
    }

    /// @brief バイト列のハッシュ値を算出する。
    /// @return バイト列のハッシュ値。
    public: typename base_type::result_type operator()(
        /// [in] バイト列の先頭位置。
        unsigned char const* const in_bytes,
        /// [in] バイト列のバイト数。
        std::size_t const in_size)
    const PSYQ_NOEXCEPT
    {
        return this->hasher_(in_bytes, in_size, this_type::get_seed());
    }

    /// @brief ハッシュ関数のシード値を参照する。
    /// @return ハッシュ関数のシード値への参照。
    public: static typename this_type::seed const& get_seed()
    {
        static template_seed const static_seed(template_seed_value);
        return static_seed;
    }

}; // class psyq::hash::_private::seeding_bytes_hash_proxy

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 実行時にシード値を決める、バイト列ハッシュ関数代理オブジェクト。
/// @tparam template_bytes_hasher @copydoc seeding_bytes_hash_proxy::hasher
/// @tparam template_seed         @copydoc seeding_bytes_hash_proxy::seed
template<typename template_bytes_hasher, typename template_seed>
class psyq::hash::_private::runtime_seeding_bytes_hash_proxy:
public psyq::hash::_private::basic_bytes_hash_proxy<
    template_bytes_hasher,
    typename std::result_of<
        template_bytes_hasher(
            unsigned char const* const,
            std::size_t const,
            template_seed const&)>
    ::type>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef runtime_seeding_bytes_hash_proxy this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::hash::_private::basic_bytes_hash_proxy<
            template_bytes_hasher,
            typename std::result_of<
                template_bytes_hasher(
                    unsigned char const* const,
                    std::size_t const,
                    template_seed const&)>
            ::type>
        base_type;
    /// @copydoc seeding_bytes_hash_proxy::seed
    public: typedef template_seed seed;

    //-------------------------------------------------------------------------
    /// バイト列ハッシュ関数オブジェクトを構築する。
    public: runtime_seeding_bytes_hash_proxy(
        /// [in] 委譲先となるバイト列ハッシュ関数オブジェクト。
        typename base_type::hasher in_hasher,
        /// [in] ハッシュ関数のシード値。
        typename this_type::seed in_seed):
    base_type(std::move(in_hasher)),
    seed_(std::move(in_seed))
    {}

    /// @brief バイト列のハッシュ値を算出する。
    /// @return バイト列のハッシュ値。
    public: typename base_type::result_type operator()(
        /// [in] バイト列の先頭位置。
        void const* const in_begin,
        /// [in] バイト列の末尾位置。
        void const* const in_end)
    const PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_begin <= in_end);
        auto const local_begin(static_cast<unsigned char const*>(in_begin));
        return (*this)(
            local_begin,
            static_cast<unsigned char const*>(in_end) - local_begin);
    }

    /// @brief バイト列のハッシュ値を算出する。
    /// @return バイト列のハッシュ値。
    public: typename base_type::result_type operator()(
        /// [in] バイト列の先頭位置。
        unsigned char const* const in_bytes,
        /// [in] バイト列のバイト数。
        std::size_t const in_size)
    const PSYQ_NOEXCEPT
    {
        return this->hasher_(in_bytes, in_size, this->get_seed());
    }

    /// @brief ハッシュ関数のシード値を参照する。
    /// @return @copydoc this_type::seed_
    public: typename this_type::seed const& get_seed() const PSYQ_NOEXCEPT
    {
        return this->seed_;
    }

    //-------------------------------------------------------------------------
    /// @brief ハッシュ関数のシード値。
    private: typename this_type::seed seed_;

}; // class psyq::hash::_private::runtime_seeding_bytes_hash_proxy

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief std::hash 互換の、文字列ハッシュ関数代理オブジェクト。
/// @tparam template_bytes_hasher @copydoc string_hash_proxy::hasher
/// @tparam template_string       @copydoc string_hash_proxy::argument_type
/// @tparam template_result       @copydoc string_hash_proxy::result_type
template<
    typename template_bytes_hasher,
    typename template_string,
    typename template_result>
class psyq::hash::_private::string_hash_proxy
{
    /// @copydoc psyq::string::view::this_type
    private: typedef string_hash_proxy this_type;

    /// @brief ハッシュ値を算出する文字列。
    /// @details
    /// 以下に相当するメンバ関数が実装されていること。
    /// @code
    /// // brief 文字列の先頭位置を取得する。
    /// // return 文字列の先頭要素を指すポインタ。
    /// template_string::value_type const* template_string::data() const;
    /// // brief 文字列の要素数を取得する。
    /// // return 文字列の要素数。
    /// std::size_t template_string::size() const;
    /// @endcode
    public: typedef template_string argument_type;
    /// @copydoc psyq::hash::_private::basic_bytes_hash_proxy::hasher
    public: typedef template_bytes_hasher hasher;
    /// @copydoc psyq::hash::numeric_hash::result_type
    public: typedef typename template_result result_type;

    //-------------------------------------------------------------------------
    /// 文字列ハッシュ関数オブジェクトを構築する。
    public: explicit string_hash_proxy(
        /// [in] 委譲先となるバイト列ハッシュ関数オブジェクト。
        typename this_type::hasher in_hasher):
    hasher_(std::move(in_hasher))
    {}

    /// @brief 文字列のハッシュ値を算出する。
    /// @return 文字列のハッシュ値。
    public: typename this_type::result_type operator()(
        /// [in] ハッシュ値を算出する文字列。
        typename this_type::argument_type const& in_string)
    const PSYQ_NOEXCEPT
    {
        auto const local_data(in_string.data());
        return (*this)(local_data, local_data + in_string.size());
    }

    /// @brief バイト列のハッシュ値を算出する。
    /// @return バイト列のハッシュ値。
    public: typename this_type::result_type operator()(
        /// [in] バイト列の先頭位置。
        void const* const in_begin,
        /// [in] バイト列の末尾位置。
        void const* const in_end)
    const PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_begin <= in_end);
        auto const local_begin(static_cast<unsigned char const*>(in_begin));
        return (*this)(
            local_begin,
            static_cast<unsigned char const*>(in_end) - local_begin);
    }

    /// @brief バイト列のハッシュ値を算出する。
    /// @return バイト列のハッシュ値。
    public: typename this_type::result_type operator()(
        /// [in] バイト列の先頭位置。
        unsigned char const* const in_bytes,
        /// [in] バイト列のバイト数。
        std::size_t const in_size)
    const PSYQ_NOEXCEPT
    {
        return static_cast<typename this_type::result_type>(
            this->hasher_(in_bytes, in_size));
    }

    /// @brief 委譲先のバイト列ハッシュ関数オブジェクトを参照する。
    /// @return 委譲先のバイト列ハッシュ関数オブジェクトへの参照。
    public: typename this_type::hasher const& get_hasher() const PSYQ_NOEXCEPT
    {
        return this->hasher_;
    }

    //-------------------------------------------------------------------------
    /// @brief @copybrief hasher
    protected: typename this_type::hasher hasher_;

}; // class psyq::hash::_private::string_hash_proxy

#endif // !defined(PSYQ_HASH_PROXY_HPP_)
// vim: set expandtab:
