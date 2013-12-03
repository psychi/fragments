/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief 型ごとにuniqueな識別値を管理する。
 */
#ifndef PSYQ_TYPE_HASH_HPP_
#define PSYQ_TYPE_HASH_HPP_
//#include "psyq/atomic_count.hpp"
#ifndef PSYQ_TYPE_HASH_RESERVED_COUNT
/// 予約済みの型の識別値の数。
#define PSYQ_TYPE_HASH_RESERVED_COUNT 1024
#endif // !defined(PSYQ_TYPE_HASH_RESERVED_COUNT)

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    typedef std::size_t type_hash; ///< 型の識別値。

    namespace internal
    {
        /** @brief 型の識別値を追加する。
            @return 追加された型の識別値。
         */
        inline psyq::type_hash add_type_hash()
        {
            static psyq::atomic_count
                static_type(PSYQ_TYPE_HASH_RESERVED_COUNT);
            auto const local_type(static_type.add());
            PSYQ_ASSERT(0 < local_type);
            return local_type;
        }

        /** @brief 型の識別値を登録する。

            一度登録された型の識別値は変更されない。

            @return 登録されている型の識別値。
         */
        template<typename template_type>
        psyq::type_hash register_type_hash(psyq::type_hash const in_type)
        {
            static auto const static_type(
                in_type != PSYQ_TYPE_HASH_RESERVED_COUNT?
                    in_type: add_type_hash());
            return static_type;
        }
    }

    /** @brief 型の識別値を取得する。
        @tparam template_type 識別値を取得したい型。
        @return 型の識別値。
     */
    template<typename template_type>
    psyq::type_hash get_type_hash()
    {
        return psyq::internal::register_type_hash<template_type>(
            get_type_hash<void>());
    }

    /** @brief void型の識別値を取得する。
        @return void型の識別値。
     */
    template<> psyq::type_hash get_type_hash<void>()
    {
        return PSYQ_TYPE_HASH_RESERVED_COUNT;
    }

    /** @brief 型の識別値を設定する。

        型の識別値は、一度設定すると変更できない。

        @tparam template_type 識別値を設定したい型。
        @retval !=psyq::get_type_hash<void>() 型の識別値。
        @retval ==psyq::get_type_hash<void>() 失敗。すでに設定されていた。
     */
    template<typename template_type>
    psyq::type_hash set_type_hash(psyq::type_hash const in_type)
    {
        if (psyq::get_type_hash<void>() < in_type
            && in_type == psyq::internal::register_type_hash(in_type))
        {
            return in_type;
        }
        return psyq::get_type_hash<void>();
    }
}

#endif // !defined(PSYQ_TYPE_HASH_HPP_)
