/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief 任意の幾何ベクトル型を扱うユーティリティ関数群。
    @defgroup psyq_geometry 幾何学的な処理の実装
    @defgroup psyq_geometry_vector 幾何ベクトルの処理
    @ingroup psyq_geometry
 */
#ifndef PSYQ_GEOMETRY_VECTOR_HPP_
#define PSYQ_GEOMETRY_VECTOR_HPP_

#include <functional>
#include "../assert.hpp"

#ifndef PSYQ_GEOMETRY_NEARLY_SCALAR_EPSILON_MAG_DEFAULT
#define PSYQ_GEOMETRY_NEARLY_SCALAR_EPSILON_MAG_DEFAULT 3
#endif // !defined(PSYQ_GEOMETRY_NEARLY_SCALAR_EPSILON_MAG_DEFAULT)

namespace psyq
{
    /// ベクトルや空間内での衝突判定など、幾何学的な処理の実装。
    namespace geometry
    {
        /// この名前空間を psyq::geometry 管理者以外が直接アクセスするのは禁止。
        namespace _private
        {
            /// @cond
            template<typename, unsigned> class vector_maker;
            /// @endcond
        } // namespace _private
    } // namespace geometry
} // psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_vector>
class psyq::geometry::_private::vector_maker<template_vector, 2>
{
    public: template<typename template_element>
    static template_vector make(
        template_element const in_element_0,
        template_element const in_element_1)
    {
        return template_vector(in_element_0, in_element_1);
    }

}; // struct psyq::geometry::_private::vector_maker

template<typename template_vector>
class psyq::geometry::_private::vector_maker<template_vector, 3>
{
    public: template<typename template_element>
    static template_vector make(
        template_element const in_element_0,
        template_element const in_element_1,
        template_element const in_element_2 = 0)
    {
        return template_vector(
            in_element_0, in_element_1, in_element_2);
    }

}; // struct psyq::geometry::_private::vector_maker

template<typename template_vector>
class psyq::geometry::_private::vector_maker<template_vector, 4>
{
    public: template<typename template_element>
    static template_vector make(
        template_element const in_element_0,
        template_element const in_element_1,
        template_element const in_element_2 = 0,
        template_element const in_element_3 = 0)
    {
        return template_vector(
            in_element_0, in_element_1, in_element_2, in_element_3);
    }

}; // struct psyq::geometry::_private::vector_maker

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
namespace geometry
{
/** @brief 幾何ベクトルの処理。
    @note
        ここで実装している幾何ベクトル処理は、汎用的な手法で実装している。
        幾何ベクトル処理に特化した適切な実装が可能なら、
        psyq::geometry::vector で定義されている関数を
        オーバーロードするかテンプレート特殊化し、
        別の適切な実装をユーザーが用意すること。
 */
namespace vector
{
/** @brief 幾何ベクトルの型特性の宣言。

    ここでは宣言のみを行い、実装は用意しない。
    ユーザーが使うベクトル型にテンプレート特殊化した実装を、
    ユーザーが用意する必要がある。その際、以下の条件を満たすこと。
    - psyq::geometry::vector::traits::type に、
      template_vector型が定義されている。
    - psyq::geometry::vector::traits::element に、
      template_vectorが持つ成分の型が定義されている。
    - psyq::geometry::vector::traits::SIZE に、
      template_vectorが持つ成分の数がunsigned型で定義されている。

    @code
    // 幾何ベクトル型特性の実装例。
    template<> class psyq::geometry::vector::traits<D3DXVECTOR3>
    {
        /// 幾何ベクトルの型。
        public: typedef D3DXVECTOR3 type;
        /// 幾何ベクトルが持つ成分の型。
        public: typedef FLOAT element; 
        /// 幾何ベクトルが持つ成分の数。
        public: enum: unsigned { SIZE = 3 };
    };
    @endcode

    @tparam template_vector 型特性を定義する幾何ベクトルの型。
    @ingroup psyq_geometry_vector
 */
template<typename template_vector> class traits;

//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの成分
//@{
/** @brief 幾何ベクトルの成分を参照する。
    @return 幾何ベクトルの成分への参照。
    @param[in,out] io_vector 成分を参照する幾何ベクトル。
    @param[in]     in_index  参照する成分のインデックス番号。
    @ingroup psyq_geometry_vector
 */
template<typename template_vector>
typename psyq::geometry::vector::traits<template_vector>::element& at(
    template_vector& io_vector,
    unsigned const in_index)
{
    /** @note
        この実装を実際に使う場合は、以下の条件を満たす必要がある。
        条件を満たさない場合は、
        テンプレート特殊化した実装をユーザーが用意すること。
        - 幾何ベクトルの成分は、連続したメモリに配置されている。
        - 幾何ベクトルの最初の成分のメモリ配置位置は、
          幾何ベクトルの先頭位置と一致する。
     */
    typedef psyq::geometry::vector::traits<template_vector> vector_traits;
    static_assert(
        std::is_standard_layout<typename vector_traits::type>::value,
        "'template_vector' is not standard layout type.");
    static_assert(
        vector_traits::SIZE * sizeof(typename vector_traits::element) <=
            sizeof(typename vector_traits::type),
        "");
    auto const local_elements(
        reinterpret_cast<typename vector_traits::element*>(&io_vector));
    PSYQ_ASSERT(in_index < vector_traits::SIZE);
    return *(local_elements + in_index);
}

/** @brief 幾何ベクトルの成分を参照する。
    @return 幾何ベクトルの成分への参照。
    @param[in] in_vector 成分を参照する幾何ベクトル。
    @param[in] in_index  参照する成分のインデックス番号。
    @ingroup psyq_geometry_vector
 */
template<typename template_vector>
typename psyq::geometry::vector::traits<template_vector>::element const&
const_at(
    template_vector const& in_vector,
    unsigned const in_index)
{
    return psyq::geometry::vector::at(
        const_cast<template_vector&>(in_vector), in_index);
}
//@}
//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの演算
//@{
/** @brief 2つの幾何ベクトルの内積を算出する。
    @return 2つの幾何ベクトルの内積。
    @param[in] in_left  内積の左辺値となる幾何ベクトル。
    @param[in] in_right 内積の右辺値となる幾何ベクトル。
    @ingroup psyq_geometry_vector
 */
template<typename template_vector>
typename psyq::geometry::vector::traits<template_vector>::element dot(
    template_vector const& in_left,
    template_vector const& in_right)
{
    typedef psyq::geometry::vector::traits<template_vector> vector_traits;
    auto local_dot(
        psyq::geometry::vector::const_at(in_left, 0) *
        psyq::geometry::vector::const_at(in_right, 0));
    for (unsigned i(1); i < vector_traits::SIZE; ++i)
    {
        local_dot +=
            psyq::geometry::vector::const_at(in_left, i) *
            psyq::geometry::vector::const_at(in_right, i);
    }
    return local_dot;
}

/** @brief 2次元での外積を算出する。
    @return 2次元での外積。
    @param[in] in_left  外積の左辺となる幾何ベクトル。
    @param[in] in_right 外積の右辺となる幾何ベクトル。
    @ingroup psyq_geometry_vector
 */
template<typename template_vector>
typename psyq::geometry::vector::traits<template_vector>::element cross_2d(
    template_vector const& in_left,
    template_vector const& in_right)
{
    typedef psyq::geometry::vector::traits<template_vector> vector_traits;
    static_assert(
        2 <= vector_traits::SIZE, "'vector_traits::SIZE' is less than 2.");
    auto const local_lx(psyq::geometry::vector::const_at(in_left, 0));
    auto const local_ly(psyq::geometry::vector::const_at(in_left, 1));
    auto const local_rx(psyq::geometry::vector::const_at(in_right, 0));
    auto const local_ry(psyq::geometry::vector::const_at(in_right, 1));
    return local_lx * local_ry - local_ly * local_rx;
}

/** @brief 3次元での外積を算出する。
    @return 3次元での外積。
    @param[in] in_left  外積の左辺となる幾何ベクトル。
    @param[in] in_right 外積の右辺となる幾何ベクトル。
    @ingroup psyq_geometry_vector
 */
template<typename template_vector>
template_vector cross_3d(
    template_vector const& in_left,
    template_vector const& in_right)
{
    typedef psyq::geometry::vector::traits<template_vector> vector_traits;
    static_assert(
        3 <= vector_traits::SIZE, "'vector_traits::SIZE' is less than 3.");
    auto const local_lx(psyq::geometry::vector::const_at(in_left, 0));
    auto const local_ly(psyq::geometry::vector::const_at(in_left, 1));
    auto const local_lz(psyq::geometry::vector::const_at(in_left, 2));
    auto const local_rx(psyq::geometry::vector::const_at(in_right, 0));
    auto const local_ry(psyq::geometry::vector::const_at(in_right, 1));
    auto const local_rz(psyq::geometry::vector::const_at(in_right, 2));
    return psyq::geometry::_private::vector_maker
        <template_vector, vector_traits::SIZE>
            ::make(
                local_ly * local_rz - local_lz * local_ry,
                local_lz * local_rx - local_lx * local_rz,
                local_lx * local_ry - local_ly * local_rx);
}

/** @brief 4次元での外積を算出する。
    @return 4次元での外積。
    @param[in] in_left   外積の左辺となる幾何ベクトル。
    @param[in] in_middle 外積の中辺となる幾何ベクトル。
    @param[in] in_right  外積の右辺となる幾何ベクトル。
    @ingroup psyq_geometry_vector
 */
template<typename template_vector>
template_vector cross_4d(
    template_vector const& in_left,
    template_vector const& in_middle,
    template_vector const& in_right)
{
    /** @note
        D3DXVec4Cross() と同じアルゴリズムのはず。
        下記のウェブページを参考に実装。
        http://www.gamedev.net/topic/298066-vector-cross-product-question
     */
    typedef psyq::geometry::vector::traits<template_vector> vector_traits;
    static_assert(
        4 <= vector_traits::SIZE, "'vector_traits::SIZE' is less than 4.");
    auto const local_mx(psyq::geometry::vector::const_at(in_middle, 0));
    auto const local_my(psyq::geometry::vector::const_at(in_middle, 1));
    auto const local_mz(psyq::geometry::vector::const_at(in_middle, 2));
    auto const local_mw(psyq::geometry::vector::const_at(in_middle, 3));
    auto const local_rx(psyq::geometry::vector::const_at(in_right, 0));
    auto const local_ry(psyq::geometry::vector::const_at(in_right, 1));
    auto const local_rz(psyq::geometry::vector::const_at(in_right, 2));
    auto const local_rw(psyq::geometry::vector::const_at(in_right, 3));

    auto const local_a(local_mx * local_ry - local_my * local_rx);
    auto const local_b(local_mx * local_rz - local_mz * local_rx);
    auto const local_c(local_mx * local_rw - local_mw * local_rx);
    auto const local_d(local_my * local_rz - local_mz * local_ry);
    auto const local_e(local_my * local_rw - local_mw * local_ry);
    auto const local_f(local_mz * local_rw - local_mw * local_rz);

    auto const local_lx(psyq::geometry::vector::const_at(in_left, 0));
    auto const local_ly(psyq::geometry::vector::const_at(in_left, 1));
    auto const local_lz(psyq::geometry::vector::const_at(in_left, 2));
    auto const local_lw(psyq::geometry::vector::const_at(in_left, 2));
    return psyq::geometry::_private::vector_maker
        <template_vector, vector_traits::SIZE>
            ::make(
                local_f * local_ly - local_e * local_lz + local_d * local_lw,
                local_f * local_lx + local_c * local_lz - local_b * local_lw,
                local_e * local_lx - local_c * local_ly + local_a * local_lw,
                local_d * local_lx + local_b * local_ly + local_a * local_lz);
}
//@}
//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの比較
//@{
/** @brief 2つのスカラ値がほぼ等値か比較する。
    @retval true  ほぼ等値だった。
    @retval false 等値ではなかった。
    @param[in] in_left_scalar  比較するスカラ値の左辺値。
    @param[in] in_right_scalar 比較するスカラ値の右辺値。
    @param[in] in_epsilon_mag  誤差の範囲に使うエプシロン値の倍率。
    @ingroup psyq_geometry_vector
 */
template<typename template_element>
bool nearly_scalar(
    template_element const in_left_scalar,
    template_element const in_right_scalar,
    unsigned const in_epsilon_mag =
        PSYQ_GEOMETRY_NEARLY_SCALAR_EPSILON_MAG_DEFAULT)
{
    auto const local_epsilon(
        std::numeric_limits<template_element>::epsilon() * in_epsilon_mag);
    auto const local_diff(in_left_scalar - in_right_scalar);
    return -local_epsilon <= local_diff && local_diff <= local_epsilon;
}

/** @brief 表す幾何ベクトルの大きさを比較する。
    @retval true  in_vector の大きさと in_length は、ほぼ等しい。
    @retval false in_vector の大きさと in_length は、等しくない。
    @param[in] in_vector      判定する幾何ベクトル。
    @param[in] in_length      判定する大きさ。
    @param[in] in_epsilon_mag 誤差の範囲に使うエプシロン値の倍率。
    @ingroup psyq_geometry_vector
 */
template<typename template_vector, typename template_element>
bool nearly_length(
    template_vector const& in_vector,
    template_element const in_length,
    unsigned const in_epsilon_mag =
        PSYQ_GEOMETRY_NEARLY_SCALAR_EPSILON_MAG_DEFAULT)
{
    typedef psyq::geometry::vector::traits<template_vector> vector_traits;
    return psyq::geometry::vector::nearly_scalar(
        psyq::geometry::vector::dot(in_vector, in_vector),
        static_cast<typename vector_traits::element>(in_length * in_length),
        in_epsilon_mag);
}

/** @brief 幾何ベクトルのすべての成分を比較する。
    @return
        幾何ベクトルのすべての成分の比較結果。
        各成分のインデックス番号のビット位置の値が…
        - 1なら、比較条件を満たしていた。
        - 0なら、比較条件を満たさなかった。
    @retval false 幾何ベクトルのいずれかの成分が比較条件を満たしてなかった。
    @param[in] in_left  比較の左辺値となる幾何ベクトル。
    @param[in] in_right 比較の右辺値となる幾何ベクトル。
    @param[in] in_compare
        成分を比較する関数オブジェクト。
        - 比較の左辺値と右辺値の2つの引数を受け取る。
        - 戻り値は…
          - trueなら、比較条件を満たしていた。
          - falseなら、比較条件を満たさなかった。
    @param[in] in_mask
        比較条件を必ず満たす成分は、
        成分インデックス番号のビット位置の値を1にしておく。
    @ingroup psyq_geometry_vector
 */
template<typename template_vector, typename template_compare>
unsigned compare_all(
    template_vector const& in_left,
    template_vector const& in_right,
    template_compare const& in_compare,
    unsigned const in_mask = 0)
{
    typedef psyq::geometry::vector::traits<template_vector> vector_traits;
    static_assert(vector_traits::SIZE <= sizeof(unsigned) * 8, "");
    unsigned local_result_bits(0);
    for (unsigned i(0); i < vector_traits::SIZE; ++i)
    {
        bool const local_compare(
            ((in_mask >> i) & 1) != 0
            || in_compare(
                psyq::geometry::vector::const_at(in_left, i),
                psyq::geometry::vector::const_at(in_right, i)));
        local_result_bits |= (local_compare << i);
    }
    return local_result_bits;
}

/** @brief 幾何ベクトルのすべての成分が「左辺値 < 右辺値」か判定する。
    @param[in] in_left  比較の左辺値となる幾何ベクトル。
    @param[in] in_right 比較の右辺値となる幾何ベクトル。
    @ingroup psyq_geometry_vector
 */
template<typename template_vector>
bool less_than(
    template_vector const& in_left,
    template_vector const& in_right)
{
    typedef psyq::geometry::vector::traits<template_vector> vector_traits;
    auto const local_compare(
        psyq::geometry::vector::compare_all(
            in_left,
            in_right,
            std::greater_equal<typename vector_traits::element>()));
    return local_compare == 0;
}

/** @brief 幾何ベクトルのすべての成分が「左辺値 <= 右辺値」か判定する。
    @param[in] in_left  比較の左辺値となる幾何ベクトル。
    @param[in] in_right 比較の右辺値となる幾何ベクトル。
    @ingroup psyq_geometry_vector
 */
template<typename template_vector>
bool less_than_equal(
    template_vector const& in_left,
    template_vector const& in_right)
{
    typedef psyq::geometry::vector::traits<template_vector> vector_traits;
    auto const local_compare(
        psyq::geometry::vector::compare_all(
            in_left,
            in_right,
            std::greater<typename vector_traits::element>()));
    return local_compare == 0;
}
//@}
//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの大きさ
//@{
/** @brief 幾何ベクトルの大きさを算出する。
    @return 幾何ベクトルの大きさ。
    @param[in] in_vector 大きさを算出する幾何ベクトル。
    @ingroup psyq_geometry_vector
 */
template<typename template_vector>
typename psyq::geometry::vector::traits<template_vector>::element length(
    template_vector const& in_vector)
{
    return std::sqrt(psyq::geometry::vector::dot(in_vector, in_vector));
}

/** @brief 正規化した幾何ベクトルを算出する。
    @return 正規化した幾何ベクトル。
    @param[in] in_vector 元となる幾何ベクトル。
    @ingroup psyq_geometry_vector
 */
template<typename template_vector>
template_vector normalize(template_vector const& in_vector)
{
    auto const local_square_length(
        psyq::geometry::vector::dot(in_vector, in_vector));
    if (0 < local_square_length)
    {
        return in_vector / std::sqrt(local_square_length);
    }
    else
    {
        auto local_vector(in_vector);
        psyq::geometry::vector::at(local_vector, 0) = 1;
        return local_vector;
    }
}
//@}

} // namespace vector
} // namespace geometry
} // namespace psyq

#endif // !defined(PSYQ_GEOMETRY_VECTOR_HPP_)
// vim: set expandtab:
