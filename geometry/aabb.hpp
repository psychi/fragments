/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::geometry::aabb
 */
#ifndef PSYQ_GEOMETRY_AABB_HPP_
#define PSYQ_GEOMETRY_AABB_HPP_

//#include "psyq/geometry/shape.hpp"

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename> class aabb;
    }
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 座標軸に平行な直方体。Axis-Aligned-Bounding-Box.
    @tparam template_coordinate @copydoc this_type::coordinate
 */
template<typename template_coordinate>
class psyq::geometry::aabb
{
    /// thisが指す値の型。
    private: typedef aabb this_type;

    /// psyq::geometry::coordinate 互換の座標の型特性。
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief AABBを構築する。
        @param[in] in_min AABBの最小座標。
        @param[in] in_max AABBの最大座標。
     */
    public: aabb(
        typename this_type::coordinate::vector const& in_min,
        typename this_type::coordinate::vector const& in_max)
    :
        min_(in_min),
        max_(in_max)
    {
        for (unsigned i(0); i < this_type::coordinate::dimension; ++i)
        {
            PSYQ_ASSERT(
                this_type::coordinate::get_element(in_min, i) <=
                this_type::coordinate::get_element(in_max, i));
        }
    }

    /** @brief AABBの最小座標を取得する。
        @return AABBの最小座標。
     */
    public: typename this_type::coordinate::vector const& get_min()
    const PSYQ_NOEXCEPT
    {
        return this->min_;
    }

    /** @brief AABBの最大座標を取得する。
        @return AABBの最大座標。
     */
    public: typename this_type::coordinate::vector const& get_max()
    const PSYQ_NOEXCEPT
    {
        return this->max_;
    }

    /** @brief 他のAABBと衝突しているか判定する。
        @retval true  衝突している。
        @retval false 衝突してない。
        @param[in] in_target 判定対象となるAABB。
     */
    public: bool detect_collision(this_type const& in_target) const
    {
        auto const local_diff_a(this->get_min() - in_target.get_max());
        auto const local_diff_b(in_target.get_min() - this->get_max());
        for (unsigned i(0); i < this_type::coordinate::dimension; ++i)
        {
            if (0 <= this_type::coordinate::get_element(local_diff_a, i) ||
                0 <= this_type::coordinate::get_element(local_diff_b, i))
            {
                return false;
            }
        }
        return true;
    }

    /** @brief 2つの点を含む最小のAABBを構築する。
        @return 点Aと点Bを包むAABB。
        @param[in] in_point_a 点Aの座標。
        @param[in] in_point_b 点Bの座標。
     */
    public: static this_type make(
        typename template_coordinate::vector const& in_point_a,
        typename template_coordinate::vector const& in_point_b)
    {
        auto local_min(in_point_a);
        auto local_max(in_point_b);
        for (unsigned i(0); i < this_type::coordinate::dimension; ++i)
        {
            auto const local_element_a(
                this_type::coordinate::get_element(in_point_a, i));
            auto const local_element_b(
                this_type::coordinate::get_element(in_point_b, i));
            if (local_element_b < local_element_a)
            {
                this_type::coordinate::set_element(
                    local_min, i, local_element_b);
                this_type::coordinate::set_element(
                    local_max, i, local_element_a);
            }
        }
        return this_type(local_min, local_max);
    }

    //-------------------------------------------------------------------------
    private: typename this_type::coordinate::vector min_; ///< AABBの最小座標。
    private: typename this_type::coordinate::vector max_; ///< AABBの最大座標。

}; // class psyq::geometry::aabb

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace geometry
    {
        //---------------------------------------------------------------------
        /** @brief 球のAABBを構築する。
            @return 球のAABB。
            @param[in] in_ball AABBを構築する球。
         */
        template<typename template_coordinate>
        psyq::geometry::aabb<template_coordinate> make_aabb(
            psyq::geometry::ball<template_coordinate> const& in_ball)
        {
            auto const local_extent(
                template_coordinate::make(in_ball.get_radius()));
            return psyq::geometry::aabb<template_coordinate>(
                in_ball.get_center() - local_extent,
                in_ball.get_center() + local_extent);
        }

        //---------------------------------------------------------------------
        /** @brief 線分のAABBを構築する。
            @return 線分のAABB。
            @param[in] in_segment AABBを構築する線分。
         */
        template<typename template_coordinate>
        psyq::geometry::aabb<template_coordinate> make_aabb(
            psyq::geometry::segment<template_coordinate> const& in_segment)
        {
            typedef std::array<
                typename template_coordinate::element,
                template_coordinate::dimension>
                    element_array;
            element_array local_min;
            element_array local_max;
            auto const local_end(
                in_segment.get_origin() + in_segment.get_direction());
            for (unsigned i(0); i < template_coordinate::dimension; ++i)
            {
                auto const local_direction(
                    template_coordinate::get_element(
                        in_segment.get_direction(), i));
                auto const local_origin(
                    template_coordinate::get_element(
                        in_segment.get_origin(), i));
                auto const local_end_element(
                    template_coordinate::get_element(local_end, i));
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
            return psyq::geometry::aabb<template_coordinate>(
                template_coordinate::make(local_min),
                template_coordinate::make(local_max));
        }

        //---------------------------------------------------------------------
        /** @brief 半直線のAABBを構築する。
            @return 半直線のAABB。
            @param[in] in_ray AABBを構築する半直線。
         */
        template<typename template_coordinate>
        psyq::geometry::aabb<template_coordinate> make_aabb(
            psyq::geometry::ray<template_coordinate> const& in_ray)
        {
            typedef std::array<
                typename template_coordinate::element,
                template_coordinate::dimension>
                    element_array;
            element_array local_min;
            element_array local_max;
            for (unsigned i(0); i < template_coordinate::dimension; ++i)
            {
                auto const local_direction(
                    template_coordinate::get_element(in_ray.get_direction(), i));
                auto const local_origin(
                    template_coordinate::get_element(in_ray.get_origin(), i));
                if (local_direction < 0)
                {
                    local_min[i] = -(std::numeric_limits<typename template_coordinate::element>::max)();
                    local_max[i] = local_origin;
                }
                else if (0 < local_direction)
                {
                    local_min[i] = local_origin;
                    local_max[i] = (std::numeric_limits<typename template_coordinate::element>::max)();
                }
                else
                {
                    local_min[i] = local_origin;
                    local_max[i] = local_origin;
                }
            }
            return psyq::geometry::aabb<template_coordinate>(
                template_coordinate::make(local_min),
                template_coordinate::make(local_max));
        }

        //---------------------------------------------------------------------
        /** @brief 直方体のAABBを構築する。
            @return 直方体のAABB。
            @param[in] in_box AABBを構築する直方体。
         */
        template<typename template_coordinate>
        psyq::geometry::aabb<template_coordinate> make_aabb(
            psyq::geometry::box<template_coordinate> const& in_box)
        {
            typedef std::array<
                typename template_coordinate::element,
                template_coordinate::dimension>
                    element_array;
            element_array local_elements;
            auto local_half_diagonal(
                template_coordinate::make(
                    static_cast<typename template_coordinate::element>(0)));
            for (unsigned i(0); i < template_coordinate::dimension; ++i)
            {
                auto const local_axis(
                    in_box.get_axes()[i] *
                        template_coordinate::get_element(
                            in_box.get_extent(), i));
                for (unsigned j(0); j < template_coordinate::dimension; ++j)
                {
                    local_elements[j] = std::abs(
                        template_coordinate::get_element(local_axis, j));
                }
                local_half_diagonal = local_half_diagonal +
                    template_coordinate::make(local_elements);
            }
            return psyq::geometry::aabb<template_coordinate>(
                in_box.get_center() - local_half_diagonal,
                in_box.get_center() + local_half_diagonal);
        }

        //---------------------------------------------------------------------
        /** @brief AABBのAABBを構築する。
            @return AABBのAABB。
            @param[in] in_aabb AABBを構築するAABB。
         */
        template<typename template_coordinate>
        psyq::geometry::aabb<template_coordinate> make_aabb(
            psyq::geometry::aabb<template_coordinate> const& in_aabb)
        {
            return in_aabb
        }

    } // namespace geometry
} // namespace psyq

#endif // !defined(PSYQ_GEOMETRY_AABB_HPP_)
