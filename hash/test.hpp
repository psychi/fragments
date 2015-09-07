/// @file
/// @brief
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_HASH_TEST_HPP_
#define PSYQ_HASH_TEST_HPP_

#include <cstdio>
#include "../bit_algorithm.hpp"
#include "./fnv.hpp"
#include "./murmur3.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    template<typename template_hash, typename template_seed>
    class bytes_hash_proxy
    {
        public: typedef
            typename std::result_of<
                template_hash(
                    unsigned char const* const,
                    std::size_t const,
                    template_seed const&)>
            ::type
            result_type;

        public: result_type operator()(
            unsigned char const* const in_bytes,
            std::size_t const in_length,
            std::size_t const in_seed)
        PSYQ_NOEXCEPT
        {
            return this->hasher_(
                in_bytes, in_length, static_cast<template_seed>(in_seed));
        }

        public: template_hash hasher_;

    }; // struct bytes_hash_proxy

    template<typename template_hash>
    class bytes_hash_proxy<template_hash, void>
    {
        public: typedef
            typename std::result_of<
                template_hash(unsigned char const* const, std::size_t const)>
            ::type
            result_type;

        public: result_type operator()(
            unsigned char const* const in_bytes,
            std::size_t const in_length,
            std::size_t)
        PSYQ_NOEXCEPT
        {
            return this->hasher_(in_bytes, in_length);
        }

        public: template_hash hasher_;

    }; // struct bytes_hash_proxy

    //-------------------------------------------------------------------------
    // This should hopefully be a thorough and uambiguous test of whether a hash
    // is correctly implemented on a given platform.
    template<typename template_hash, typename template_seed>
    bool hash_verification(
        std::uint32_t const in_expected,
        bool const in_verbose)
    {
        typedef bytes_hash_proxy<template_hash, template_seed> hash_proxy;
        hash_proxy local_hash;
        typename hash_proxy::result_type local_hashes[256] = {};
        unsigned char local_key[256] = {};

        // Hash keys of the form {0}, {0,1}, {0,1,2}...
        // up to N=255,using 256-N as the seed.
        for(unsigned i(0); i < 256; ++i)
        {
            local_key[i] = static_cast<unsigned char>(i);
            local_hashes[i] = local_hash(local_key, i, 256 - i);
        }

        // Then hash the result array.
        auto const local_final(
            local_hash(
                reinterpret_cast<unsigned char const*>(&local_hashes[0]),
                sizeof(local_hashes),
                0));

        //----------
        auto const local_verification(static_cast<std::uint32_t>(local_final));
        if (in_expected != local_verification)
        {
            PSYQ_ASSERT(false);
            if (in_verbose)
            {
                printf(
                    "Verification value 0x%08X : Failed! (Expected 0x%08x)\n",
                    local_verification,
                    in_expected);
            }
            return false;
        }
        else
        {
            if (in_verbose)
            {
                printf(
                    "Verification value 0x%08X : Passed!\n",
                    local_verification);
            }
            return true;
        }
    }

    inline void hash()
    {
        hash_verification<psyq::hash::_private::murmur3_hash32, std::uint32_t>(
            PSYQ_BIG_ENDIAN_4BYTES(std::uint32_t, 0xE3, 0x7E, 0xF5, 0xB0),
            false);
        hash_verification<psyq::hash::_private::murmur3_hash128, std::uint32_t>(
            PSYQ_BIG_ENDIAN_4BYTES(std::uint32_t, 0x2A, 0xE6, 0xEC, 0xB3),
            false);
        hash_verification<psyq::hash::_private::murmur3_hash128, std::uint64_t>(
            PSYQ_BIG_ENDIAN_4BYTES(std::uint32_t, 0x69, 0xBA, 0x84, 0x63),
            false);
    }

} // namespace psyq_test

#endif // !defined(PSYQ_HASH_TEST_HPP_)
// vim: set expandtab:
