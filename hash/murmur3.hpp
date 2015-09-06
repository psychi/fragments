/// @file
/// @brief MurmurHash3形式のハッシュ関数。
/// @details
/// 以下のウェブページを参考に実装。
/// https://code.google.com/p/smhasher/
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_HASH_MURMUR3_HPP_
#define PSYQ_HASH_MURMUR3_HPP_

#include "./proxy.hpp"
#include <cstdint>

/// @brief MurmurHash3形式の32ビットハッシュ関数のデフォルトのシード値。
#ifndef PSYQ_HASH_MURMUR3_HASHER32_SEED_DEFAULT
#define PSYQ_HASH_MURMUR3_HASHER32_SEED_DEFAULT 0xB0F57EE3
#endif // !defined(PSYQ_HASH_MURMUR3_HASHER32_SEED_DEFAULT)

/// @brief 32ビットで処理するMurmurHash3形式の128ビットハッシュ関数のデフォルトのシード値。
#ifndef PSYQ_HASH_MURMUR3_HASHER128_PROCESSING32_SEED_DEFAULT
#define PSYQ_HASH_MURMUR3_HASHER128_PROCESSING32_SEED_DEFAULT 0xB3ECE62A
#endif // !defined(PSYQ_HASH_MURMUR3_HASHER128_PROCESSING32_SEED_DEFAULT)

/// @brief 64ビットで処理するMurmurHash3形式の128ビットハッシュ関数のデフォルトのシード値。
#ifndef PSYQ_HASH_MURMUR3_HASHER128_PROCESSING64_SEED_DEFAULT
#define PSYQ_HASH_MURMUR3_HASHER128_PROCESSING64_SEED_DEFAULT 0x6384BA69ull
#endif // !defined(PSYQ_HASH_MURMUR3_HASHER128_PROCESSING64_SEED_DEFAULT)

namespace psyq
{
    namespace hash
    {
        template<typename> class murmur3_string_hash32;
        template<typename> class murmur3_string_hash64;
        template<typename> class murmur3_string_hash64_processing32;
        template<typename> class murmur3_string_hash64_processing64;

        namespace _private
        {
            class murmur3_hash32;
            class murmur3_hash64;
            class murmur3_hash128;

            /// @brief MurmurHash3形式の32ビットハッシュ関数代理オブジェクト。
            typedef
                runtime_seeding_bytes_hash_proxy<murmur3_hash32, std::uint32_t>
                murmur3_hash32_proxy;
            /// @brief MurmurHash3形式の64ビットハッシュ関数代理オブジェクト。
            typedef
                runtime_seeding_bytes_hash_proxy<murmur3_hash64, std::size_t>
                murmur3_hash64_proxy;
            /// @brief 32ビットで処理するMurmurHash3形式の64ビットハッシュ関数代理オブジェクト。
            typedef
                runtime_seeding_bytes_hash_proxy<murmur3_hash64, std::uint32_t>
                murmur3_hash64_proxy32;
            /// @brief 64ビットで処理するMurmurHash3形式の64ビットハッシュ関数代理オブジェクト。
            typedef
                runtime_seeding_bytes_hash_proxy<murmur3_hash64, std::uint64_t>
                murmur3_hash64_proxy64;

        } // namespace _private
    } // namespace hash
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief MurmurHash3形式の文字列32ビットハッシュ関数オブジェクト。
/// @tparam template_string @copydoc _private::string_hash_proxy::argument_type
template<typename template_string>
class psyq::hash::murmur3_string_hash32:
public psyq::hash::_private::string_hash_proxy<
    psyq::hash::_private::murmur3_hash32_proxy, template_string>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef murmur3_string_hash32 this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::hash::_private::string_hash_proxy<
            psyq::hash::_private::murmur3_hash32_proxy, template_string>
        base_type;

    /// @brief 文字列ハッシュ関数オブジェクトを構築する。
    public: explicit murmur3_string_hash32(
        /// [in] in_seed ハッシュ関数のシード値。
        typename base_type::hasher::seed in_seed =
            PSYQ_HASH_MURMUR3_HASHER32_SEED_DEFAULT):
    base_type(
        typename base_type::hasher(
            typename base_type::hasher::hasher(), std::move(in_seed)))
    {}

}; // class psyq::hash::murmur3_string_hash32

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief MurmurHash3形式の文字列64ビットハッシュ関数オブジェクト。
/// @tparam template_string @copydoc _private::string_hash_proxy::argument_type
template<typename template_string>
class psyq::hash::murmur3_string_hash64:
public psyq::hash::_private::string_hash_proxy<
    psyq::hash::_private::murmur3_hash64_proxy, template_string>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef murmur3_string_hash64 this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::hash::_private::string_hash_proxy<
            psyq::hash::_private::murmur3_hash64_proxy, template_string>
        base_type;

    /// @brief 文字列ハッシュ関数オブジェクトを構築する。
    public: explicit murmur3_string_hash64(
        /// [in] in_seed ハッシュ関数のシード値。
        typename base_type::hasher::seed in_seed = std::integral_constant<
            typename base_type::hasher::seed,
            sizeof(typename base_type::hasher::seed) < sizeof(std::uint64_t)?
                PSYQ_HASH_MURMUR3_HASHER128_PROCESSING32_SEED_DEFAULT:
                PSYQ_HASH_MURMUR3_HASHER128_PROCESSING64_SEED_DEFAULT>::value):
    base_type(
        typename base_type::hasher(
            typename base_type::hasher::hasher(), std::move(in_seed)))
    {}

}; // class psyq::hash::murmur3_string_hash64

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 32ビットで処理するMurmurHash3形式の、文字列64ビットハッシュ関数オブジェクト。
/// @tparam template_string @copydoc _private::string_hash_proxy::argument_type
template<typename template_string>
class psyq::hash::murmur3_string_hash64_processing32:
public psyq::hash::_private::string_hash_proxy<
    psyq::hash::_private::murmur3_hash64_proxy32, template_string>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef murmur3_string_hash64_processing32 this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::hash::_private::string_hash_proxy<
            psyq::hash::_private::murmur3_hash64_proxy32, template_string>
        base_type;

    /// @brief 文字列ハッシュ関数オブジェクトを構築する。
    public: explicit murmur3_string_hash64_processing32(
        /// [in] in_seed ハッシュ関数のシード値。
        typename base_type::hasher::seed in_seed =
            PSYQ_HASH_MURMUR3_HASHER128_PROCESSING32_SEED_DEFAULT):
    base_type(
        typename base_type::hasher(
            typename base_type::hasher::hasher(), std::move(in_seed)))
    {}

}; // class psyq::hash::murmur3_string_hash64_processing32

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 64ビットで処理するMurmurHash3形式の、文字列64ビットハッシュ関数オブジェクト。
/// @tparam template_string @copydoc _private::string_hash_proxy::argument_type
template<typename template_string>
class psyq::hash::murmur3_string_hash64_processing64:
public psyq::hash::_private::string_hash_proxy<
    psyq::hash::_private::murmur3_hash64_proxy64, template_string>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef murmur3_string_hash64_processing64 this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::hash::_private::string_hash_proxy<
            psyq::hash::_private::murmur3_hash64_proxy64, template_string>
        base_type;

    /// @brief 文字列ハッシュ関数オブジェクトを構築する。
    public: explicit murmur3_string_hash64_processing64(
        /// [in] in_seed ハッシュ関数のシード値。
        typename base_type::hasher::seed in_seed =
            PSYQ_HASH_MURMUR3_HASHER128_PROCESSING64_SEED_DEFAULT):
    base_type(
        typename base_type::hasher(
            typename base_type::hasher::hasher(), std::move(in_seed)))
    {}

}; // class psyq::hash::murmur3_string_hash64_processing64

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief MurmurHash3形式の32bitハッシュ関数オブジェクト。
class psyq::hash::_private::murmur3_hash32
{
    /// @copydoc psyq::string::view::this_type
    private: typedef murmur3_hash32 this_type;

    //-------------------------------------------------------------------------
    /// @copydoc this_type::hash
    public: std::uint32_t operator()(
        /// [in] バイト列の先頭要素を指すポインタ。
        unsigned char const* const in_bytes,
        /// [in] バイト列のバイト数。
        std::size_t const in_length,
        /// [in] ハッシュ関数のシード値。
        std::uint32_t const& in_seed)
    const
    {
        return this_type::hash(in_bytes, in_length, in_seed);
    }

    /// @brief バイト列のハッシュ値を算出する。
    /// @return バイト列のハッシュ値。
    public: static std::uint32_t hash(
        /// [in] バイト列の先頭要素を指すポインタ。
        unsigned char const* const in_bytes,
        /// [in] バイト列のバイト数。
        std::size_t const in_length,
        /// [in] ハッシュ関数のシード値。
        std::uint32_t const in_seed)
    {
        static_assert(sizeof(std::uint32_t) == 4, "");
        std::uint32_t const local_c0(0xCC9E2D51);
        std::uint32_t const local_c1(0x1B873593);
        auto const local_block_count(in_length / 4);
        auto const local_tail(in_bytes + local_block_count * 4);
        auto local_hash(in_seed);

        //----------
        // body
        for (auto i(in_bytes); i < local_tail; i += 4)
        {
            auto const local_k(this_type::get_block(i));
            local_hash ^= local_c1 * this_type::rotl(local_c0 * local_k, 15);
            local_hash = 0xE6546B64 + 5 * this_type::rotl(local_hash, 13);
        }

        //----------
        // tail
        std::uint32_t local_k(0);
        switch (in_length & 3)
        {
            case 3: local_k ^= local_tail[2] << 16;
            case 2: local_k ^= local_tail[1] <<  8;
            case 1: local_k ^= local_tail[0];
            local_hash ^= local_c1 * this_type::rotl(local_c0 * local_k, 15);

            default: break;
        };

        //----------
        // finalization
        return this_type::final_mix(local_hash ^ in_length);
    } 

    //-------------------------------------------------------------------------
    protected: static std::uint32_t get_block(unsigned char const* const in_bytes)
    {
        return in_bytes[0]
            | (in_bytes[1] <<  8)
            | (in_bytes[2] << 16)
            | (in_bytes[3] << 24);
    }

    protected: static std::uint32_t rotl(
         std::uint32_t const in_value,
         unsigned char const in_shift)
    {
#if defined(_MSC_VER)
        return _rotl(in_value, in_shift);
#else
        return (in_value << in_shift) | (in_value >> (32 - in_shift));
#endif // defined(_MSC_VER)
    }

    // Finalization mix - force all bits of a hash block to avalanche
    protected: static std::uint32_t final_mix(std::uint32_t in_hash)
    {
        in_hash ^= in_hash >> 16;
        in_hash *= 0x85EBCA6B;
        in_hash ^= in_hash >> 13;
        in_hash *= 0xC2B2AE35;
        in_hash ^= in_hash >> 16;
        return in_hash;
    }

}; // class psyq::string_hash::murmur3_hash32

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief MurmurHash3形式の128bitハッシュ関数オブジェクト。
class psyq::hash::_private::murmur3_hash128:
protected psyq::hash::_private::murmur3_hash32
{
    /// @copydoc psyq::string::view::this_type
    private: typedef murmur3_hash128 this_type;
    /// @copydoc psyq::string::view::base_type
    protected: typedef psyq::hash::_private::murmur3_hash32 base_type;

    //-------------------------------------------------------------------------
    /// @brief ハッシュ関数の戻り値。
    public: class result_type
    {
        public: result_type(
            std::uint64_t const in_uint64_0,
            std::uint64_t const in_uint64_1)
        {
            this->uint64_0_ = in_uint64_0;
            this->uint64_1_ = in_uint64_1;
        }

        public: result_type(
            std::uint32_t const in_uint32_0,
            std::uint32_t const in_uint32_1,
            std::uint32_t const in_uint32_2,
            std::uint32_t const in_uint32_3)
        {
            this->uint32_0_ = in_uint32_0;
            this->uint32_1_ = in_uint32_1;
            this->uint32_2_ = in_uint32_2;
            this->uint32_3_ = in_uint32_3;
        }

        public: union
        {
#ifdef PSYQ_HASH_MURMUR3_BIG_ENDIAN
            struct
            {
                std::uint64_t uint64_1_;
                std::uint64_t uint64_0_;
            };
            struct
            {
                std::uint32_t uint32_3_;
                std::uint32_t uint32_2_;
                std::uint32_t uint32_1_;
                std::uint32_t uint32_0_;
            };
#else
            struct
            {
                std::uint64_t uint64_0_;
                std::uint64_t uint64_1_;
            };
            struct
            {
                std::uint32_t uint32_0_;
                std::uint32_t uint32_1_;
                std::uint32_t uint32_2_;
                std::uint32_t uint32_3_;
            };
#endif // defined(PSYQ_HASH_MURMUR3_BIG_ENDIAN)
        };
    };

    //-------------------------------------------------------------------------
    /// @brief バイト列のハッシュ値を32ビット処理で算出する。
    public: static this_type::result_type hash(
        /// [in] バイト列の先頭要素を指すポインタ。
        unsigned char const* const in_bytes,
        /// [in] バイト列のバイト数。
        std::size_t const in_length,
        /// [in] ハッシュ関数のシード値。
        std::uint32_t in_seed)
    {
        auto const local_block_count(in_length / 16);
        auto const local_tail(in_bytes + local_block_count * 16);
        std::uint32_t const local_c0(0x239B961B);
        std::uint32_t const local_c1(0xAB0E9789);
        std::uint32_t const local_c2(0x38B34AE5); 
        std::uint32_t const local_c3(0xA1E38B93);
        auto local_hash0(in_seed);
        auto local_hash1(in_seed);
        auto local_hash2(in_seed);
        auto local_hash3(in_seed);

        //----------
        // body
        for (auto i(in_bytes); i < local_tail; i += 16)
        {
            auto const local_k0(base_type::get_block(i));
            local_hash0 ^= local_c1 * base_type::rotl(local_c0 * local_k0, 15);
            local_hash0 = 0x561CCD1B + 5 * (local_hash1 + base_type::rotl(local_hash0, 19));

            auto const local_k1(base_type::get_block(i + 4));
            local_hash1 ^= local_c2 * base_type::rotl(local_c1 * local_k1, 16);
            local_hash1 = 0x0BCAA747 + 5 * (local_hash2 + base_type::rotl(local_hash1, 17));

            auto const local_k2(base_type::get_block(i + 8));
            local_hash2 ^= local_c3 * base_type::rotl(local_c2 * local_k2, 17);
            local_hash2 = 0x96CD1C35 + 5 * (local_hash3 + base_type::rotl(local_hash2, 15));

            auto const local_k3(base_type::get_block(i + 12));
            local_hash3 ^= local_c0 * base_type::rotl(local_c3 * local_k3, 18);
            local_hash3 = 0x32AC3B17 + 5 * (local_hash0 + base_type::rotl(local_hash3, 13));
        }

        //----------
        // tail
        std::uint32_t local_k0(0);
        std::uint32_t local_k1(0);
        std::uint32_t local_k2(0);
        std::uint32_t local_k3(0);
        switch(in_length & 15)
        {
            case 15: local_k3 ^= local_tail[14] << 16;
            case 14: local_k3 ^= local_tail[13] <<  8;
            case 13: local_k3 ^= local_tail[12] <<  0;
            local_hash3 ^= local_c0 * base_type::rotl(local_c3 * local_k3, 18);

            case 12: local_k2 ^= local_tail[11] << 24;
            case 11: local_k2 ^= local_tail[10] << 16;
            case 10: local_k2 ^= local_tail[ 9] <<  8;
            case  9: local_k2 ^= local_tail[ 8] <<  0;
            local_hash2 ^= local_c3 * base_type::rotl(local_c2 * local_k2, 17);

            case  8: local_k1 ^= local_tail[ 7] << 24;
            case  7: local_k1 ^= local_tail[ 6] << 16;
            case  6: local_k1 ^= local_tail[ 5] <<  8;
            case  5: local_k1 ^= local_tail[ 4] <<  0;
            local_hash1 ^= local_c2 * base_type::rotl(local_c1 * local_k1, 16);

            case  4: local_k0 ^= local_tail[ 3] << 24;
            case  3: local_k0 ^= local_tail[ 2] << 16;
            case  2: local_k0 ^= local_tail[ 1] <<  8;
            case  1: local_k0 ^= local_tail[ 0] <<  0;
            local_hash0 ^= local_c1 * base_type::rotl(local_c0 * local_k0, 15);

            default: break;
        };

        //----------
        // finalization
        local_hash0 ^= static_cast<std::uint32_t>(in_length);
        local_hash1 ^= static_cast<std::uint32_t>(in_length);
        local_hash2 ^= static_cast<std::uint32_t>(in_length);
        local_hash3 ^= static_cast<std::uint32_t>(in_length);

        local_hash0 += local_hash1;
        local_hash0 += local_hash2;
        local_hash0 += local_hash3;

        local_hash1 += local_hash0;
        local_hash2 += local_hash0;
        local_hash3 += local_hash0;

        local_hash0 = base_type::final_mix(local_hash0);
        local_hash1 = base_type::final_mix(local_hash1);
        local_hash2 = base_type::final_mix(local_hash2);
        local_hash3 = base_type::final_mix(local_hash3);

        local_hash0 += local_hash1;
        local_hash0 += local_hash2;
        local_hash0 += local_hash3;

        return this_type::result_type(
            local_hash0,
            local_hash0 + local_hash1,
            local_hash0 + local_hash2,
            local_hash0 + local_hash3);
    }

    //-------------------------------------------------------------------------
    /// @brief バイト列のハッシュ値を64ビット処理で算出する。
    public: static this_type::result_type hash(
        /// [in] バイト列の先頭要素を指すポインタ。
        unsigned char const* const in_bytes,
        /// [in] バイト列のバイト数。
        std::size_t const in_length,
        /// [in] ハッシュ関数のシード値。
        std::uint64_t const in_seed)
    {
        static_assert(sizeof(std::uint64_t) == 8, "");
        auto const local_block_count(in_length / 16);
        auto const local_tail(in_bytes + local_block_count * 16);
        std::uint64_t const local_c0(0x87C37B91114253D5ull);
        std::uint64_t const local_c1(0x4CF5AD432745937Full);
        auto local_hash0(in_seed);
        auto local_hash1(in_seed);

        //----------
        // body
        for (auto i(in_bytes); i < local_tail; i += 16)
        {
            auto const local_k0(this_type::get_block(i));
            local_hash0 ^= local_c1 * this_type::rotl(local_k0 * local_c0, 31);
            local_hash0 = 0x52DCE729 + 5 * (local_hash1 + this_type::rotl(local_hash0, 27));

            auto const local_k1(this_type::get_block(i + 8));
            local_hash1 ^= local_c0 * this_type::rotl(local_k1 * local_c1, 33);
            local_hash1 = 0x38495AB5 + 5 * (local_hash0 + this_type::rotl(local_hash1, 31));
        }

        //----------
        // tail
        std::uint64_t local_k0(0);
        std::uint64_t local_k1(0);
        switch(in_length & 15)
        {
            case 15: local_k1 ^= static_cast<std::uint64_t>(local_tail[14]) << 48;
            case 14: local_k1 ^= static_cast<std::uint64_t>(local_tail[13]) << 40;
            case 13: local_k1 ^= static_cast<std::uint64_t>(local_tail[12]) << 32;
            case 12: local_k1 ^= static_cast<std::uint64_t>(local_tail[11]) << 24;
            case 11: local_k1 ^= static_cast<std::uint64_t>(local_tail[10]) << 16;
            case 10: local_k1 ^= static_cast<std::uint64_t>(local_tail[ 9]) <<  8;
            case  9: local_k1 ^= static_cast<std::uint64_t>(local_tail[ 8]) <<  0;
            local_hash1 ^= local_c0 * this_type::rotl(local_k1 * local_c1, 33);

            case  8: local_k0 ^= static_cast<std::uint64_t>(local_tail[ 7]) << 56;
            case  7: local_k0 ^= static_cast<std::uint64_t>(local_tail[ 6]) << 48;
            case  6: local_k0 ^= static_cast<std::uint64_t>(local_tail[ 5]) << 40;
            case  5: local_k0 ^= static_cast<std::uint64_t>(local_tail[ 4]) << 32;
            case  4: local_k0 ^= static_cast<std::uint64_t>(local_tail[ 3]) << 24;
            case  3: local_k0 ^= static_cast<std::uint64_t>(local_tail[ 2]) << 16;
            case  2: local_k0 ^= static_cast<std::uint64_t>(local_tail[ 1]) <<  8;
            case  1: local_k0 ^= static_cast<std::uint64_t>(local_tail[ 0]) <<  0;
            local_hash0 ^= local_c1 * this_type::rotl(local_k0 * local_c0, 31);

            default: break;
        };

        //----------
        // finalization
        local_hash0 ^= in_length;
        local_hash1 ^= in_length;

        local_hash0 += local_hash1;
        local_hash1 += local_hash0;

        local_hash0 = this_type::final_mix(local_hash0);
        local_hash1 = this_type::final_mix(local_hash1);

        local_hash0 += local_hash1;
        return this_type::result_type(local_hash0, local_hash0 + local_hash1);
    }

    //-------------------------------------------------------------------------
    protected: static std::uint64_t get_block(unsigned char const* const in_bytes)
    {
        return base_type::get_block(in_bytes)
            | (static_cast<std::uint64_t>(base_type::get_block(in_bytes + 4)) << 32);
    }

    protected: static std::uint64_t rotl(
         std::uint64_t const in_value,
         unsigned char const in_shift)
    {
#if defined(_MSC_VER)
        return _rotl64(in_value, in_shift);
#else
        return (in_value << in_shift) | (in_value >> (64 - in_shift));
#endif // defined(_MSC_VER)
    }

    protected: static std::uint64_t final_mix(std::uint64_t in_hash)
    {
        in_hash ^= in_hash >> 33;
        in_hash *= 0xFF51AFD7ED558CCDull;
        in_hash ^= in_hash >> 33;
        in_hash *= 0xC4CEB9FE1A85EC53ull;
        in_hash ^= in_hash >> 33;
        return in_hash;
    }

}; // class psyq::string_hash::_private::murmur3_hash128

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief MurmurHash3形式の64bitハッシュ関数オブジェクト。
class psyq::hash::_private::murmur3_hash64:
protected psyq::hash::_private::murmur3_hash128
{
    /// @copydoc psyq::string::view::this_type
    private: typedef murmur3_hash64 this_type;
    /// @copydoc psyq::string::view::base_type
    protected: typedef psyq::hash::_private::murmur3_hash128 base_type;

    //-------------------------------------------------------------------------
    /// @copydoc base_type::hash(unsigned char const*, std::size_t, std::uint32_t);
    public: std::uint64_t operator()(
        /// [in] バイト列の先頭要素を指すポインタ。
        unsigned char const* const in_bytes,
        /// [in] バイト列のバイト数。
        std::size_t const in_length,
        /// [in] ハッシュ関数のシード値。
        std::uint32_t const& in_seed)
    const
    {
        return this_type::hash(in_bytes, in_length, in_seed);
    }

    /// @copydoc base_type::hash(unsigned char const*, std::size_t, std::uint64_t);
    public: std::uint64_t operator()(
        /// [in] バイト列の先頭要素を指すポインタ。
        unsigned char const* const in_bytes,
        /// [in] バイト列のバイト数。
        std::size_t const in_length,
        /// [in] ハッシュ関数のシード値。
        std::uint64_t const& in_seed)
    const
    {
        return this_type::hash(in_bytes, in_length, in_seed);
    }

    /// @copydoc base_type::hash(unsigned char const*, std::size_t, std::uint32_t);
    public: static std::uint64_t hash(
        /// [in] バイト列の先頭要素を指すポインタ。
        unsigned char const* const in_bytes,
        /// [in] バイト列のバイト数。
        std::size_t const in_length,
        /// [in] ハッシュ関数のシード値。
        std::uint32_t const in_seed)
    {
        return base_type::hash(in_bytes, in_length, in_seed).uint64_0_;
    }

    /// @copydoc base_type::hash(unsigned char const*, std::size_t, std::uint64_t);
    public: static std::uint64_t hash(
        /// [in] バイト列の先頭要素を指すポインタ。
        unsigned char const* const in_bytes,
        /// [in] バイト列のバイト数。
        std::size_t const in_length,
        /// [in] ハッシュ関数のシード値。
        std::uint64_t const in_seed)
    {
        return base_type::hash(in_bytes, in_length, in_seed).uint64_0_;
    }

}; // class psyq::hash::_private::murmur3_hash64;

#endif // !defined(PSYQ_HASH_MURMUR3_HPP_)
// vim: set expandtab:
