/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief 任意の幾何ベクトル型を扱うユーティリティ関数群。
 */
#ifndef PSYQ_GEOMETRY_VECTOR_HPP_
#define PSYQ_GEOMETRY_VECTOR_HPP_

#include <type_traits>

#ifndef PSYQ_GEOMETRY_NEARLY_EQUAL_EPSILON_MAG_DEFAULT
#define PSYQ_GEOMETRY_NEARLY_EQUAL_EPSILON_MAG_DEFAULT 3
#endif // !defined(PSYQ_GEOMETRY_NEARLY_EQUAL_EPSILON_MAG_DEFAULT)

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    /// ベクトルや図形など、幾何学的な処理を行う。
    namespace geometry
    {
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

        //---------------------------------------------------------------------
        /** @brief 2次元ベクトルを構築する。

            他の適切な手法で実装できるなら、
            テンプレート特殊化した実装を用意すること。

            @return 構築した2次元ベクトル。
            @param[in] in_element_0 要素#0の初期値。
            @param[in] in_element_1 要素#1の初期値。
         */
        template<typename template_vector, typename template_element>
        template_vector make_vector(
            template_element const in_element_0,
            template_element const in_element_1)
        {
            return template_vector(in_element_0, in_element_1);
        }

        /** @brief 3次元ベクトルを構築する。

            他の適切な手法で実装できるなら、
            テンプレート特殊化した実装を用意すること。

            @return 構築した3次元ベクトル。
            @param[in] in_element_0 要素#0の初期値。
            @param[in] in_element_1 要素#1の初期値。
            @param[in] in_element_2 要素#2の初期値。
         */
        template<typename template_vector, typename template_element>
        template_vector make_vector(
            template_element const in_element_0,
            template_element const in_element_1,
            template_element const in_element_2)
        {
            return template_vector(in_element_0, in_element_1, in_element_2);
        }

        //---------------------------------------------------------------------
        /** @copydoc psyq::geometry::coordinate::get_element()

            @tparam template_dimension @copydoc psyq::geometry::coordinate::DIMENSION
            @tparam template_element   @copydoc psyq::geometry::coordinate::element
            @tparam template_vector    @copydoc psyq::geometry::coordinate::vector

            下記のいずれかを満たさないか、他の適切な手法で実装可能なら、
            テンプレート特殊化した実装を用意すること。
            - template_coordinate::vector
              の要素は、連続したメモリに配置されていること。
            - 最初の要素の配置位置は、 template_coordinate::vector
              インスタンスの先頭位置と一致すること。
         */
        template<
            unsigned template_dimension,
            typename template_element,
            typename template_vector>
        template_element get_vector_element(
            template_vector const& in_vector,
            unsigned const in_index)
        {
            static_assert(
                std::is_standard_layout<template_vector>::value,
                "'template_vector' is not standard layout type.");
            static_assert(
                template_dimension * sizeof(template_element) <= sizeof(template_vector),
                "");
            auto const local_elements(
                reinterpret_cast<template_element const*>(&in_vector));
            PSYQ_ASSERT(in_index < template_dimension);
            return *(local_elements + in_index);
        }

        /** @copydoc psyq::geometry::coordinate::set_element()

            @tparam template_dimension @copydoc psyq::geometry::coordinate::DIMENSION
            @tparam template_element   @copydoc psyq::geometry::coordinate::element
            @tparam template_vector    @copydoc psyq::geometry::coordinate::vector

            下記のいずれかを満たさないか、他の適切な手法で実装可能なら、
            テンプレート特殊化した実装を用意すること。
            - template_coordinate::vector
              の要素は、連続したメモリに配置されていること。
            - 最初の要素の配置位置は、 template_coordinate::vector
              インスタンスの先頭位置と一致すること。
         */
        template<
            unsigned template_dimension,
            typename template_element,
            typename template_vector>
        template_element set_vector_element(
            template_vector& io_vector,
            unsigned const in_index,
            template_element const in_value)
        {
            static_assert(
                std::is_standard_layout<template_vector>::value,
                "'template_coordinate::vector' is not standard layout type.");
            static_assert(
                template_dimension * sizeof(template_element) <= sizeof(template_vector),
                "");
            auto const local_elements(
                reinterpret_cast<template_element*>(&io_vector));
            PSYQ_ASSERT(in_index < template_dimension);
            *(local_elements + in_index) = in_value;
            return in_value;
        }

        //---------------------------------------------------------------------
        /** @copydoc psyq::geometry::coordinate::dot_product()

            他の適切な手法で実装できるなら、
            テンプレート特殊化した実装を用意すること。
         */
        template<
            unsigned template_dimension,
            typename template_element,
            typename template_vector>
        template_element dot_product_vector(
            template_vector const& in_left,
            template_vector const& in_right)
        {
            auto local_dot(
                psyq::geometry::get_vector_element<template_dimension, template_element>(in_left, 0) * 
                psyq::geometry::get_vector_element<template_dimension, template_element>(in_right, 0));
            for (unsigned i(1); i < template_dimension; ++i)
            {
                local_dot +=
                    psyq::geometry::get_vector_element<template_dimension, template_element>(in_left, i) * 
                    psyq::geometry::get_vector_element<template_dimension, template_element>(in_right, i);
            }
            return local_dot;
        }

        /** @copydoc psyq::geometry::coordinate::cross_product()

            他の適切な手法で実装できるなら、
            テンプレート特殊化した実装を用意すること。
         */
        template<typename template_element, typename template_vector>
        template_vector cross_product_vector(
            template_vector const& in_left,
            template_vector const& in_right)
        {
            auto const local_left_0(
                psyq::geometry::get_vector_element<3, template_element>(in_left, 0));
            auto const local_left_1(
                psyq::geometry::get_vector_element<3, template_element>(in_left, 1));
            auto const local_left_2(
                psyq::geometry::get_vector_element<3, template_element>(in_left, 2));
            auto const local_right_0(
                psyq::geometry::get_vector_element<3, template_element>(in_right, 0));
            auto const local_right_1(
                psyq::geometry::get_vector_element<3, template_element>(in_right, 1));
            auto const local_right_2(
                psyq::geometry::get_vector_element<3, template_element>(in_right, 2));
            return psyq::get_vector_element::make_vector<template_vector>(
                local_left_1 * local_right_2 - local_left_2 * local_right_1,
                local_left_2 * local_right_0 - local_left_0 * local_right_2,
                local_left_0 * local_right_1 - local_left_1 * local_right_0);
        }

        //---------------------------------------------------------------------
        /** @copydoc psyq::geometry::coordinate::square_length()

            他の適切な手法で実装できるなら、
            テンプレート特殊化した実装を用意すること。
         */
        template<
            unsigned template_dimension,
            typename template_element,
            typename template_vector>
        template_element square_vector_length(
            template_vector const& in_vector)
        {
            return psyq::geometry::dot_product_vector
                <template_dimension, template_element>(in_vector, in_vector);
        }

        /** @copydoc psyq::geometry::coordinate::compute_length()

            他の適切な手法で実装できるなら、
            テンプレート特殊化した実装を用意すること。
         */
        template<
            unsigned template_dimension,
            typename template_element,
            typename template_vector>
        template_element compute_vector_length(template_vector const& in_vector)
        {
            return std::sqrt(
                psyq::geometry::square_vector_length
                <template_dimension, template_element>(in_vector));
        }

        /** @copydoc psyq::geometry::coordinate::normalize()

            他の適切な手法で実装できるなら、
            テンプレート特殊化した実装を用意すること。
         */
        template<
            unsigned template_dimension,
            typename template_element,
            typename template_vector>
        template_vector normalize_vector(template_vector const& in_vector)
        {
            auto const local_square_length(
                psyq::geometry::square_vector_length
                <template_dimension, template_element>(in_vector));
            auto local_vector(in_vector);
            if (0 < local_square_length)
            {
                auto const local_invert_length(1 / std::sqrt(local_square_length));
                for (unsigned i(0); i < template_dimension; ++i)
                {
                    psyq::geometry::set_vector_element<template_dimension>(
                        local_vector,
                        i,
                        local_invert_length *
                            psyq::geometry::get_vector_element
                                <template_dimension, template_element>
                                    (in_vector, i));
                }
            }
            else
            {
                psyq::geometry::set_vector_element
                    <template_dimension>(local_vector, 0, 1);
            }
            return local_vector;
        }

    } // namespace geometry
} // namespace psyq

#endif // !defined(PSYQ_GEOMETRY_VECTOR_HPP_)
