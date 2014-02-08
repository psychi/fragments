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
    @brief @copybrief psyq::geometric_vector
 */
#ifndef PSYQ_GEOMETIC_VECTOR_HPP_
#define PSYQ_GEOMETIC_VECTOR_HPP_

#ifndef PSYQ_NEARLY_EQUAL_EPSILON_MAG_DEFAULT
#define PSYQ_NEARLY_EQUAL_EPSILON_MAG_DEFAULT 1
#endif

namespace psyq
{
    /// @cond
    template<typename> struct geometric_vector;
    /// @endcond

    /** @brief 2つの浮動小数点値がほぼ等値か比較する。
        @param[in] in_left_value  比較する浮動小数点値の左辺値。
        @param[in] in_right_value 比較する浮動小数点値の右辺値。
        @param[in] in_epsilon_mag 誤差の範囲に使うエプシロン値の倍率。
        @retval true  ほぼ等値だった。
        @retval false 等値ではなかった。
     */
    template<typename template_value>
    bool is_nearly_equal(
        template_value const in_left_value,
        template_value const in_right_value,
        unsigned const       in_epsilon_mag
            = PSYQ_NEARLY_EQUAL_EPSILON_MAG_DEFAULT)
    {
        auto const local_epsilon(
            std::numeric_limits<template_value>::epsilon() * in_epsilon_mag);
        auto const local_diff(in_left_value - in_right_value);
        return -local_epsilon <= local_diff && local_diff <= local_epsilon;
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 幾何ベクトルの型特性。
    @tparam template_vector @copydoc psyq::geometric_vector::type
 */
template<typename template_vector>
struct psyq::geometric_vector
{
    /** @brief 操作する幾何ベクトルの型。

        以下の条件を満たしていること。
        - 仮想関数を持たないこと。
        - メンバ変数として、3つ以上の要素を持つこと。
        - 要素は、すべてfloat型であること。
        - 要素は、連続したメモリに配置されていること。
        - 最初の要素の配置位置は、template_vectorの先頭と一致すること。

        上記をいずれかひとつでも満たさない場合は、
        以下の実装をテンプレート特殊化して用意すること。
        - psyq::geometric_vector
        - psyq::geometric_vector_element()
        - psyq::geometric_vector_cross()

        以下の条件は、必ず満たしていること。
        - copyコンストラクタとcopy代入演算子が使える。
        - 以下に相当するコンストラクタを持つ。
          - template_vector::template_vector(element, element, element)
        - 以下に相当する二項演算子が使える。
          - template_vector operator+(template_vector, template_vector)
          - template_vector operator-(template_vector, template_vector)
          - template_vector operator*(template_vector, template_vector)
          - template_vector operator*(template_vector, element)
          - template_vector operator/(template_vector, template_vector)
          - template_vector operator/(template_vector, element)
     */
    typedef template_vector type;

    /// 幾何ベクトルの要素の型。
    typedef float element;

    enum: unsigned
    {
        SIZE = 3 ///< 幾何ベクトルが持つ要素の数。
    };

    /// 幾何ベクトルを作る。
    static template_vector make(element const in_element)
    {
        return template_vector(in_element, in_element, in_element);
    }

    /// 幾何ベクトルを作る。
    static template_vector make(
        element const in_element0,
        element const in_element1,
        element const in_element2)
    {
        return template_vector(in_element0, in_element1, in_element2);
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    /** @brief 幾何ベクトルの要素から値を取得する。
        @tparam template_vector @copydoc psyq::geometric_vector::type
        @param[in] in_vector        要素の値を取得する幾何ベクトル。
        @param[in] in_element_index 取得する要素のインデックス番号。
        @return template_vector の要素の値。
     */
    template<typename template_vector>
    typename psyq::geometric_vector<template_vector>::element
    geometric_vector_element(
        template_vector const& in_vector,
        unsigned const         in_element_index)
    {
        PSYQ_ASSERT(
            in_element_index < psyq::geometric_vector<template_vector>::SIZE);
        typedef typename psyq::geometric_vector<template_vector>::element const
            vector_element;
        auto const local_element(
            reinterpret_cast<vector_element*>(&in_vector) + in_element_index);
        return *local_element;
    }

    /** @brief 幾何ベクトルの要素に値を設定する。
        @tparam template_vector @copydoc psyq::geometric_vector::type
        @param[in,out] io_vector        要素に値を設定する幾何ベクトル。
        @param[in]     in_element_index 設定する要素のインデックス番号。
        @param[in]     in_element_value 設定する要素の値。
        @return template_vector の要素に設定した値。
     */
    template<typename template_vector>
    typename psyq::geometric_vector<template_vector>::element
    geometric_vector_element(
        template_vector& io_vector,
        unsigned const   in_element_index,
        typename psyq::geometric_vector<template_vector>::element const
            in_element_value)
    {
        PSYQ_ASSERT(
            in_element_index < psyq::geometric_vector<template_vector>::SIZE);
        typedef typename psyq::geometric_vector<template_vector>::element
            vector_element;
        auto const local_element(
            reinterpret_cast<vector_element*>(&io_vector) + in_element_index);
        *local_element = in_element_value;
        return in_element_value;
    }

    /** @brief 幾何ベクトルの長さの2乗を算出する。
        @tparam template_vector @copydoc psyq::geometric_vector::type
        @param[in] in_vector 長さの2乗を算出する幾何ベクトル。
        @return 幾何ベクトルの長さの2乗。
     */
    template<typename template_vector>
    typename psyq::geometric_vector<template_vector>::element
    geometric_vector_squared_length(template_vector const& in_vector)
    {
        auto const local_element(
            psyq::geometric_vector_element(in_vector, 0));
        auto local_squared_length(local_element * local_element);
        for (
            unsigned i(1);
            i < psyq::geometric_vector<template_vector>::SIZE;
            ++i)
        {
            auto const local_element(
                psyq::geometric_vector_element(in_vector, i));
            local_squared_length += local_element * local_element;
        }
        return local_squared_length;
    }

    /** @brief 幾何ベクトルの長さを算出する。
        @tparam template_vector @copydoc psyq::geometric_vector::type
        @param[in] in_vector 長さを算出する幾何ベクトル。
        @return 幾何ベクトルの長さ。
     */
    template<typename template_vector>
    typename psyq::geometric_vector<template_vector>::element
    geometric_vector_length(template_vector const& in_vector)
    {
        return std::sqrt(psyq::geometric_vector_squared_length(in_vector));
    }

    /** @brief 幾何ベクトルが正規化されているか判定する。
        @tparam template_vector @copydoc psyq::geometric_vector::type
        @param[in] in_vector 判定する幾何ベクトル。
        @retval true  幾何ベクトルは正規化されている。
        @retval false 幾何ベクトルは正規化されていない。
     */
    template<typename template_vector>
    bool geometric_vector_is_normalized(template_vector const& in_vector)
    {
        return psyq::is_nearly_equal(
            psyq::geometric_vector_squared_length(in_vector),
            typename psyq::geometric_vector<template_vector>::element(1));
    }

    /** @brief 2つの幾何ベクトルの内積を算出する。
        @tparam template_vector @copydoc psyq::geometric_vector::type
        @param[in] in_left  左辺の幾何ベクトル。
        @param[in] in_right 右辺の幾何ベクトル。
        @return 2つの幾何ベクトルの内積。
     */
    template<typename template_vector>
    typename psyq::geometric_vector<template_vector>::element
    geometric_vector_dot(
        template_vector const& in_left,
        template_vector const& in_right)
    {
        auto local_dot(
            psyq::geometric_vector_element(in_left, 0)
            * psyq::geometric_vector_element(in_right, 0));
        for (
            unsigned i(1);
            i < psyq::geometric_vector<template_vector>::SIZE;
            ++i)
        {
            local_dot +=
                psyq::geometric_vector_element(in_left, i)
                * psyq::geometric_vector_element(in_right, i);
        }
        return local_dot;
    }

    /** @brief 2つの幾何ベクトルの外積を算出する。
        @tparam template_vector @copydoc psyq::geometric_vector::type
        @param[in] in_left  左辺の幾何ベクトル。
        @param[in] in_right 右辺の幾何ベクトル。
        @return 2つの幾何ベクトルの外積。
     */
    template<typename template_vector>
    template_vector geometric_vector_cross(
        template_vector const& in_left,
        template_vector const& in_right)
    {
        auto const local_left0(psyq::geometric_vector_element(in_left, 0));
        auto const local_left1(psyq::geometric_vector_element(in_left, 1));
        auto const local_left2(psyq::geometric_vector_element(in_left, 2));
        auto const local_right0(psyq::geometric_vector_element(in_right, 0));
        auto const local_right1(psyq::geometric_vector_element(in_right, 1));
        auto const local_right2(psyq::geometric_vector_element(in_right, 2));
        return psyq::geometric_vector<template_vector>::make(
            local_left1 * local_right2 - local_left2 * local_right1,
            local_left2 * local_right0 - local_left0 * local_right2,
            local_left0 * local_right1 - local_left1 * local_right0);
    }
};

#endif // !defined(PSYQ_GEOMETIC_VECTOR_HPP_)
