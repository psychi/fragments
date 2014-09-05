/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief 任意の幾何ベクトル型を扱うユーティリティ関数群。
    @defgroup psyq_geometry 幾何学的な処理の実装
    @defgroup psyq_geometry_vector_traits 幾何ベクトルの型特性
    @ingroup psyq_geometry
 */
#ifndef PSYQ_GEOMETRY_VECTOR_HPP_
#define PSYQ_GEOMETRY_VECTOR_HPP_

namespace psyq
{
    /// ベクトルや空間内での衝突判定など、幾何学的な処理の実装。
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
