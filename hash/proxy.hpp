/// @file
/// @brief ハッシュ関数の代理オブジェクト。
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
            /// @cond
            template<typename, typename> class array_proxy_base;
            template<typename> class array_seedless_proxy;
            template<typename, typename template_seed, template_seed>
                class array_seeding_proxy;
            template<typename, typename> class array_runtime_seeding_proxy;
            template<typename, typename, typename> class string_proxy;
            /// @endcond

        } // namespace _private
    } // namespace hash
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 配列ハッシュ関数オブジェクトの基底型。
/// @tparam template_bytes_hasher @copydoc array_proxy_base::hasher
/// @tparam template_result       @copydoc array_proxy_base::result_type
template<typename template_bytes_hasher, typename template_result>
class psyq::hash::_private::array_proxy_base
{
    /// @copydoc psyq::string::view::this_type
    private: typedef array_proxy_base this_type;

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
    /// 配列ハッシュ関数オブジェクトを構築する。
    protected: explicit array_proxy_base(
        /// @[in] 委譲先となるバイト列ハッシュ関数オブジェクト。
        typename this_type::hasher in_hasher):
    hasher_(std::move(in_hasher))
    {}

    //-------------------------------------------------------------------------
    /// @brief @copybrief hasher
    private: typename this_type::hasher hasher_;

}; // class psyq::hash::_private::array_proxy_base

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief シード値のない、配列ハッシュ関数オブジェクト。
/// @tparam template_bytes_hasher
/// 委譲先となるバイト列ハッシュ関数オブジェクト。
/// 以下に相当するメンバ関数が実装されていること。
/// @code
/// // brief バイト列のハッシュ値を算出する。
/// // return バイト列のハッシュ値。
/// template_bytes_hasher::result_type template_bytes_hasher::operator()(
///     // [in] バイト列の先頭位置。
///     unsigned char const* const in_data,
///     // [in] バイト列のバイト数。
///     std::size_t const in_size)
/// const noexcept;
/// @endcode
template<typename template_bytes_hasher>
class psyq::hash::_private::array_seedless_proxy:
public psyq::hash::_private::array_proxy_base<
    template_bytes_hasher,
    typename std::result_of<
        template_bytes_hasher(unsigned char const* const, std::size_t const)>
    ::type>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef array_seedless_proxy this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef 
        psyq::hash::_private::array_proxy_base<
            template_bytes_hasher,
            typename std::result_of<
                template_bytes_hasher(unsigned char const* const, std::size_t const)>
            ::type>
        base_type;

    //-------------------------------------------------------------------------
    /// 配列ハッシュ関数オブジェクトを構築する。
    public: explicit array_seedless_proxy(
        /// [in] 委譲先となるバイト列ハッシュ関数オブジェクト。
        typename base_type::hasher in_hasher):
    base_type(std::move(in_hasher))
    {}

    /// @brief 配列のハッシュ値を算出する。
    /// @return 配列のハッシュ値。
    public: template<typename template_value>
    typename base_type::result_type operator()(
        /// [in] キーとなる配列の先頭要素を指すポインタ。
        template_value const* const in_data,
        /// [in] キーとなる配列の要素数。
        std::size_t const in_size)
    const PSYQ_NOEXCEPT
    {
        return this->get_hasher()(
            reinterpret_cast<unsigned char const*>(in_data),
            in_size * sizeof(template_value));
    }

}; // class psyq::hash::_private::array_seedless_proxy

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief シード値のある、配列ハッシュ関数オブジェクト。
/// @tparam template_bytes_hasher
/// 委譲先となるバイト列ハッシュ関数オブジェクト。
/// 以下に相当するメンバ関数が実装されていること。
/// @code
/// // brief バイト列のハッシュ値を算出する。
/// // return バイト列のハッシュ値。
/// template_bytes_hasher::result_type template_bytes_hasher::operator()(
///     // [in] バイト列の先頭位置。
///     unsigned char const* const in_data,
///     // [in] バイト列のバイト数。
///     std::size_t const in_size,
///     // [in] ハッシュ関数のシード値。
///     template_seed const& in_seed)
/// const noexcept;
/// @endcode
/// @tparam template_seed       @copydoc array_seeding_proxy::seed
/// @tparam template_seed_value ハッシュ関数のシード値。
template<
    typename template_bytes_hasher,
    typename template_seed,
    template_seed template_seed_value>
class psyq::hash::_private::array_seeding_proxy:
public psyq::hash::_private::array_proxy_base<
    template_bytes_hasher,
    typename std::result_of<
        template_bytes_hasher(
            unsigned char const* const,
            std::size_t const,
            template_seed const&)>
    ::type>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef array_seeding_proxy this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef 
        psyq::hash::_private::array_proxy_base<
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
    /// 配列ハッシュ関数オブジェクトを構築する。
    public: explicit array_seeding_proxy(
        /// @[in] 委譲先となるバイト列ハッシュ関数オブジェクト。
        typename base_type::hasher in_hasher):
    base_type(std::move(in_hasher))
    {}

    /// @brief 配列のハッシュ値を算出する。
    /// @return 配列のハッシュ値。
    public: template<typename template_value>
    typename base_type::result_type operator()(
        /// [in] キーとなる配列の先頭要素を指すポインタ。
        template_value const* const in_data,
        /// [in] キーとなる配列の要素数。
        std::size_t const in_size)
    const PSYQ_NOEXCEPT
    {
        return this->get_hasher()(
            reinterpret_cast<unsigned char const*>(in_data),
            in_size * sizeof(template_value),
            this_type::get_seed());
    }

    /// @brief ハッシュ関数のシード値を参照する。
    /// @return ハッシュ関数のシード値への参照。
    public: static typename this_type::seed const& get_seed()
    {
        static template_seed const static_seed(template_seed_value);
        return static_seed;
    }

}; // class psyq::hash::_private::array_seeding_proxy

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 実行時にシード値を決める、配列ハッシュ関数オブジェクト。
/// @tparam template_bytes_hasher
/// 委譲先となるバイト列ハッシュ関数オブジェクト。
/// 以下に相当するメンバ関数が実装されていること。
/// @code
/// // brief バイト列のハッシュ値を算出する。
/// // return バイト列のハッシュ値。
/// template_bytes_hasher::result_type template_bytes_hasher::operator()(
///     // [in] バイト列の先頭位置。
///     unsigned char const* const in_data,
///     // [in] バイト列のバイト数。
///     std::size_t const in_size,
///     // [in] ハッシュ関数のシード値。
///     template_seed const& in_seed)
/// const noexcept;
/// @endcode
/// @tparam template_seed @copydoc array_seeding_proxy::seed
template<typename template_bytes_hasher, typename template_seed>
class psyq::hash::_private::array_runtime_seeding_proxy:
public psyq::hash::_private::array_proxy_base<
    template_bytes_hasher,
    typename std::result_of<
        template_bytes_hasher(
            unsigned char const* const,
            std::size_t const,
            template_seed const&)>
    ::type>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef array_runtime_seeding_proxy this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::hash::_private::array_proxy_base<
            template_bytes_hasher,
            typename std::result_of<
                template_bytes_hasher(
                    unsigned char const* const,
                    std::size_t const,
                    template_seed const&)>
            ::type>
        base_type;
    /// @copydoc array_seeding_proxy::seed
    public: typedef template_seed seed;

    //-------------------------------------------------------------------------
    /// 配列ハッシュ関数オブジェクトを構築する。
    public: array_runtime_seeding_proxy(
        /// [in] 委譲先となるバイト列ハッシュ関数オブジェクト。
        typename base_type::hasher in_hasher,
        /// [in] ハッシュ関数のシード値。
        typename this_type::seed in_seed):
    base_type(std::move(in_hasher)),
    seed_(std::move(in_seed))
    {}

    /// @brief 配列のハッシュ値を算出する。
    /// @return 配列のハッシュ値。
    public: template<typename template_value>
    typename base_type::result_type operator()(
        /// [in] キーとなる配列の先頭要素を指すポインタ。
        template_value const* const in_data,
        /// [in] キーとなる配列の要素数。
        std::size_t const in_size)
    const PSYQ_NOEXCEPT
    {
        return this->get_hasher()(
            reinterpret_cast<unsigned char const*>(in_data),
            in_size * sizeof(template_value),
            this->get_seed());
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

}; // class psyq::hash::_private::array_runtime_seeding_proxy

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 文字列ハッシュ関数オブジェクト。
/// @tparam template_array_hasher @copydoc string_proxy::hasher
/// @tparam template_result       @copydoc string_proxy::result_type
/// @tparam template_string       @copydoc string_proxy::argument_type
template<
    typename template_array_hasher,
    typename template_result,
    typename template_string>
class psyq::hash::_private::string_proxy
{
    /// @copydoc psyq::string::view::this_type
    private: typedef string_proxy this_type;

    /// @brief 委譲先となる配列ハッシュ関数オブジェクト。
    /// @details 以下に相当するメンバ関数が実装されていること。
    /// @code
    /// // brief 配列のハッシュ値を算出する。
    /// // return 配列のハッシュ値。
    /// template_array_hasher::result_type template_array_hasher::operator()(
    ///     // [in] キーとなる配列の先頭要素を指すポインタ。
    ///     template_string::value_type const* const in_data,
    ///     // [in] キーとなる配列の要素数。
    ///     std::size_t const in_size)
    /// const noexcept;
    /// @endcode
    /// template_array_hasher::operator() の戻り値から
    /// this_type::result_type へ static_cast できること。
    public: typedef template_array_hasher hasher;
    /// @brief 文字列ハッシュ関数の戻り値。
    /// @details
    /// template_array_hasher::operator() の戻り値から
    /// this_type::result_type へ static_cast できること。
    public: typedef template_result result_type;
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

    //-------------------------------------------------------------------------
    /// 文字列ハッシュ関数オブジェクトを構築する。
    public: explicit string_proxy(
        /// [in] 委譲先となる配列ハッシュ関数オブジェクト。
        typename this_type::hasher in_hasher):
    hasher_(std::move(in_hasher))
    {}

    /// @brief 文字列のハッシュ値を算出する。
    /// @return in_string のハッシュ値。
    public: typename this_type::result_type operator()(
        /// [in] キーとなる文字列。
        typename this_type::argument_type const& in_string)
    const PSYQ_NOEXCEPT
    {
        return static_cast<typename this_type::result_type>(
            this->get_hasher()(in_string.data(), in_string.size()));
    }

    /// @brief 委譲先の配列ハッシュ関数オブジェクトを参照する。
    /// @return 委譲先の配列ハッシュ関数オブジェクトへの参照。
    public: typename this_type::hasher const& get_hasher() const PSYQ_NOEXCEPT
    {
        return this->hasher_;
    }

    //-------------------------------------------------------------------------
    /// @brief @copybrief this_type::hasher
    private: typename this_type::hasher hasher_;

}; // class psyq::hash::_private::string_proxy

#endif // !defined(PSYQ_HASH_PROXY_HPP_)
// vim: set expandtab:
