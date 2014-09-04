/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief 任意の幾何ベクトル型を扱うユーティリティ関数群。
    @defgroup psyq_geometry ベクトルや図形など、幾何学的な処理
    @defgroup psyq_geometry_vector_traits 幾何ベクトルの型特性
    @ingroup psyq_geometry
 */
#ifndef PSYQ_GEOMETRY_VECTOR_HPP_
#define PSYQ_GEOMETRY_VECTOR_HPP_

#ifndef PSYQ_GEOMETRY_NEARLY_EQUAL_EPSILON_MAG_DEFAULT
#define PSYQ_GEOMETRY_NEARLY_EQUAL_EPSILON_MAG_DEFAULT 3
#endif // !defined(PSYQ_GEOMETRY_NEARLY_EQUAL_EPSILON_MAG_DEFAULT)

namespace psyq
{
    /// ベクトルや図形など、幾何学的な処理を行う。
    namespace geometry
    {
        /** @brief 幾何ベクトルの型特性の宣言。

            ここでは宣言のみを行い、実装は用意しない。
            ユーザーが使うベクトル型にテンプレート特殊化した実装を、
            ユーザーが用意する必要がある。その際、以下の条件を満たすこと。
            - psyq::geometry::vector_traits::type に、
              template_vector型が定義されている。
            - psyq::geometry::vector_traits::element に、
              template_vectorが持つ成分の型が定義されている。
            - psyq::geometry::vector_traits::size に、
              template_vectorが持つ成分の数がunsigned型で定義されている。
            @code
            // 幾何ベクトル型特性の実装例。
            template<> class psyq::geometry::vector_traits<D3DXVECTOR3>
            {
                /// 幾何ベクトルの型。
                public: typedef D3DXVECTOR3 type;
                /// 幾何ベクトルが持つ成分の型。
                public: typedef FLOAT element; 
                /// 幾何ベクトルが持つ成分の数。
                public: enum: unsigned { size = 3 };
            };
            @endcode

            @tparam template_vector 型特性を定義する幾何ベクトルの型。
            @ingroup psyq_geometry_vector_traits
         */
        template<typename template_vector> class vector_traits;

        /** @brief 2つの浮動小数点値がほぼ等値か比較する。
            @retval true  ほぼ等値だった。
            @retval false 等値ではなかった。
            @param[in] in_left_value  比較する浮動小数点値の左辺値。
            @param[in] in_right_value 比較する浮動小数点値の右辺値。
            @param[in] in_epsilon_mag 誤差の範囲に使うエプシロン値の倍率。
         */
        template<typename template_value>
        bool is_nearly_equal(
            template_value const in_left_value,
            template_value const in_right_value,
            unsigned const in_epsilon_mag =
                PSYQ_GEOMETRY_NEARLY_EQUAL_EPSILON_MAG_DEFAULT)
        {
            auto const local_epsilon(
                std::numeric_limits<template_value>::epsilon() * in_epsilon_mag);
            auto const local_diff(in_left_value - in_right_value);
            return -local_epsilon <= local_diff && local_diff <= local_epsilon;
        }

        /** @brief 幾何ベクトルの長さを比較する。
            @retval true  in_vector の長さと in_length は、ほぼ等しい。
            @retval false in_vector の長さと in_length は、等しくない。
            @tparam template_coordinate
                使用する座標系の型。
                psyq::geometry::coordinate と互換性があること。
            @param[in] in_vector      判定する幾何ベクトル。
            @param[in] in_length      判定する長さ。
            @param[in] in_epsilon_mag 誤差の範囲に使うエプシロン値の倍率。
         */
        template<typename template_coordinate>
        bool is_nearly_length(
            typename template_coordinate::vector const& in_vector,
            typename template_coordinate::element const in_length,
            unsigned const in_epsilon_mag =
                PSYQ_GEOMETRY_NEARLY_EQUAL_EPSILON_MAG_DEFAULT)
        {
            return psyq::geometry::is_nearly_equal(
                template_coordinate::dot_product(in_vector, in_vector),
                in_length * in_length,
                in_epsilon_mag);
        }

        /// この名前空間をユーザーが直接アクセスするのは禁止。
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

#endif // !defined(PSYQ_GEOMETRY_VECTOR_HPP_)
