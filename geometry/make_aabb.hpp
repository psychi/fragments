/** @file
    @brief 幾何形状からAABBを作る関数群。
    @author Hillco Psychi (https://twitter.com/psychi)
    @ingroup psyq_geometry_shape
 */
#ifndef PSYQ_GEOMETRY_MAKE_AABB_HPP_
#define PSYQ_GEOMETRY_MAKE_AABB_HPP_

//#include "./aabb.hpp"
//#include "./line.hpp"
//#include "./ray.hpp"
//#include "./ball.hpp"
//#include "./box.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace geometry
    {
        //---------------------------------------------------------------------
        /** @brief 球のAABBを構築する。
            @return 球のAABB。
            @tparam template_coordinate @copydoc psyq::geometry::aabb::coordinate
            @param[in] in_ball AABBを構築する球。
            @ingroup psyq_geometry_shape
         */
        template<typename template_coordinate>
        typename template_coordinate::aabb make_aabb(
            psyq::geometry::ball<template_coordinate> const& in_ball)
        {
            auto const local_extent(
                template_coordinate::make_filled(in_ball.get_radius()));
            return typename template_coordinate::aabb(
                in_ball.center_.get_position() - local_extent,
                in_ball.center_.get_position() + local_extent);
        }

        //---------------------------------------------------------------------
        /** @brief 線分のAABBを構築する。
            @return 線分のAABB。
            @tparam template_coordinate @copydoc psyq::geometry::aabb::coordinate
            @param[in] in_segment AABBを構築する線分。
            @ingroup psyq_geometry_shape
         */
        template<typename template_coordinate>
        typename template_coordinate::aabb make_aabb(
            typename psyq::geometry::line_segment<template_coordinate> const&
                in_segment)
        {
            typename template_coordinate::element_array local_min;
            typename template_coordinate::element_array local_max;
            auto const local_end(
                in_segment.origin_.get_position()
                + in_segment.direction_.get_unit() * in_segment.length_);
            for (unsigned i(0); i < template_coordinate::DIMENSION; ++i)
            {
                auto const local_direction(
                    psyq::geometry::vector::const_at(
                        in_segment.direction_.get_unit(), i));
                auto const local_origin(
                    psyq::geometry::vector::const_at(
                        in_segment.origin_.get_position(), i));
                auto const local_end_element(
                    psyq::geometry::vector::const_at(local_end, i));
                if (local_direction < 0)
                {
                    local_min[i] = local_end_element;
                    local_max[i] = local_origin;
                }
                else
                {
                    local_min[i] = local_origin;
                    local_max[i] = local_end_element;
                }
            }
            return typename template_coordinate::aabb(
                template_coordinate::make(local_min),
                template_coordinate::make(local_max));
        }

        //---------------------------------------------------------------------
        /** @brief 半直線のAABBを構築する。
            @return 半直線のAABB。
            @tparam template_coordinate @copydoc psyq::geometry::aabb::coordinate
            @param[in] in_ray AABBを構築する半直線。
            @ingroup psyq_geometry_shape
         */
        template<typename template_coordinate>
        typename template_coordinate::aabb make_aabb(
            psyq::geometry::ray<template_coordinate> const& in_ray)
        {
            // 極大の線分からAABBを決定する。
            typedef typename template_coordinate::element element;
            static_assert(1 <= std::numeric_limits<element>::digits, "");
            return make_aabb(
                psyq::geometry::line_segment<template_coordinate>(
                    in_ray,
                    static_cast<element>(
                        1ull << (std::numeric_limits<element>::digits - 1))));
        }

        //---------------------------------------------------------------------
        /** @brief 直方体のAABBを構築する。
            @return 直方体のAABB。
            @tparam template_coordinate @copydoc psyq::geometry::aabb::coordinate
            @param[in] in_box AABBを構築する直方体。
            @ingroup psyq_geometry_shape
         */
        template<typename template_coordinate>
        typename template_coordinate::aabb make_aabb(
            psyq::geometry::box<template_coordinate> const& in_box)
        {
            typename template_coordinate::element_array local_elements;
            auto local_half_diagonal(template_coordinate::make_filled(0));
            for (unsigned i(0); i < template_coordinate::DIMENSION; ++i)
            {
                auto const local_axis(
                    psyq::geometry::vector::const_at(in_box.get_extent(), i)
                    * in_box.get_axes()[i]);
                for (unsigned j(0); j < template_coordinate::DIMENSION; ++j)
                {
                    local_elements[j] = std::abs(
                        psyq::geometry::vector::const_at(local_axis, j));
                }
                local_half_diagonal = local_half_diagonal +
                    template_coordinate::make(local_elements);
            }
            return typename template_coordinate::aabb(
                in_box.get_center() - local_half_diagonal,
                in_box.get_center() + local_half_diagonal);
        }

        //---------------------------------------------------------------------
        /** @brief AABBのAABBを構築する。
            @return AABBのAABB。
            @tparam template_coordinate @copydoc psyq::geometry::aabb::coordinate
            @param[in] in_aabb AABBを構築するAABB。
            @ingroup psyq_geometry_shape
         */
        template<typename template_coordinate>
        typename template_coordinate::aabb make_aabb(
            psyq::geometry::aabb<template_coordinate> const& in_aabb)
        {
            return in_aabb;
        }

    } // namespace geometry
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @cond
namespace psyq_test
{
   template<typename template_coordinate>
   void geometry_coordinate()
   {
       typedef psyq::geometry::ball<template_coordinate> ball_type;
       auto const local_ball(
           ball_type::make(template_coordinate::make_filled(2), 10));
       auto const local_ball_aabb(
           psyq::geometry::make_aabb(local_ball));

       auto local_point(
           ball_type::point::make(template_coordinate::make_filled(2)));
       local_point = ball_type::point::make(template_coordinate::make_filled(3));

       typedef typename psyq::geometry::line_segment<template_coordinate>
           line_segment_type;
       auto const local_line(
           line_segment_type::make(
               local_ball.center_,
               template_coordinate::make(local_ball.get_radius(), -4, 3)));
       auto const local_line_aabb(psyq::geometry::make_aabb(local_line));

       typedef psyq::geometry::ray<template_coordinate> ray_type;
       ray_type const local_ray(local_line);
       auto const local_ray_aabb(
           psyq::geometry::make_aabb(local_ray));

       typedef psyq::geometry::box<template_coordinate> box_type;
       auto const local_box(
           box_type::make_cuboid(
               local_line.origin_.get_position(),
               local_line.direction_.get_unit(),
               60 * 3.1415926535f / 180,
               template_coordinate::make_filled(1)));
       auto const local_box_aabb(
           psyq::geometry::make_aabb(local_box));

       typedef psyq::geometry::barycentric_triangle<template_coordinate> triangle_type;
       auto const local_triangle(
           triangle_type::make(
               template_coordinate::make(0, 0, 0),
               template_coordinate::make(1, 0, 0),
               template_coordinate::make(0, 1, 0)));
   }
}
/// @endcond

#endif // !defined(PSYQ_GEOMETRY_MAKE_AABB_HPP_)
// vim: set expandtab:
