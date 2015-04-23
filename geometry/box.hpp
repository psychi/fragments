/** @file
    @brief @copybrief psyq::geometry::box
    @author Hillco Psychi (https://twitter.com/psychi)
    @ingroup psyq_geometry_shape
 */
#ifndef PSYQ_GEOMETRY_BOX_HPP_
#define PSYQ_GEOMETRY_BOX_HPP_

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename> class box;
    } // namespace geometry
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 直方体。
    @tparam template_coordinate @copydoc box::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::box
{
    /// @brief thisが指す値の型。
    private: typedef box this_type;

    /// @brief @copydoc psyq::geometry::ball::coordinate
    public: typedef template_coordinate coordinate;

    /// @brief 直方体の軸方向の単位ベクトルの配列。
    public: typedef std::array<
        typename this_type::coordinate::vector,
        this_type::coordinate::DIMENSION>
            axis_array;

    //-------------------------------------------------------------------------
    /** @brief 直方体を構築する。
        @param[in] in_center 直方体の中心位置。
        @param[in] in_extent 直方体の大きさの1/2。すべての要素が0以上であること。
        @param[in] in_axes   直方体の各軸方向の単位ベクトルの配列。
     */
    public: box(
        typename this_type::coordinate::vector const& in_center,
        typename this_type::coordinate::vector const& in_extent,
        typename this_type::axis_array const& in_axes)
    :
    center_((
        PSYQ_ASSERT(this_type::coordinate::validate(in_center)), in_center)),
    extent_((
        PSYQ_ASSERT(this_type::coordinate::validate(in_extent)), in_extent)),
    axes_(in_axes)
    {
        for (unsigned i(0); i < this_type::coordinate::DIMENSION; ++i)
        {
            PSYQ_ASSERT(0 <= psyq::geometry::vector::const_at(in_extent, i));
            PSYQ_ASSERT(this_type::coordinate::validate(in_axes[i]));
            PSYQ_ASSERT(psyq::geometry::vector::nearly_length(in_axes[i], 1));
        }
    }

    /** @brief 直方体の中心位置を取得する。
        @return 直方体の中心位置。
     */
    public: typename this_type::coordinate::vector const& get_center()
    const PSYQ_NOEXCEPT
    {
        return this->center_;
    }

    /** @brief 直方体の大きさの1/2を取得する。
        @return 直方体の大きさの1/2。
     */
    public: typename this_type::coordinate::vector const& get_extent()
    const PSYQ_NOEXCEPT
    {
        return this->extent_;
    }

    /** @brief 直方体の軸方向ベクトルの配列を取得する。
        @return 直方体の軸方向の単位ベクトルの配列。
     */
    public: typename this_type::axis_array const& get_axes()
    const PSYQ_NOEXCEPT
    {
        return this->axes_;
    }

    /** @brief 直方体を作る。
        @param[in] in_center   直方体の中心位置。
        @param[in] in_extent   直方体の大きさの1/2。
        @param[in] in_rotation 直方体の回転ラジアン。
        @param[in] in_axis     直方体の回転軸。
     */
    public: static this_type make_cuboid(
        typename this_type::coordinate::vector const& in_center,
        typename this_type::coordinate::vector const& in_extent,
        typename this_type::coordinate::element const in_rotation,
        typename this_type::coordinate::vector const& in_axis)
    {
        static_assert(
            3 <= this_type::coordinate::DIMENSION,
            "'this_type::coordinate::DIMENSION' is less than 3.");

        // 回転軸と回転角度から四元数を算出する。
        auto const local_half_rotation(in_rotation / 2);
        auto const local_half_sin(std::sin(local_half_rotation));
        auto const local_axis(
            psyq::geometry::vector::normalize(
                this_type::coordinate::make(in_axis)));
        auto const local_qx(
            local_half_sin * psyq::geometry::vector::const_at(local_axis, 0));
        auto const local_qy(
            local_half_sin * psyq::geometry::vector::const_at(local_axis, 1));
        auto const local_qz(
            local_half_sin * psyq::geometry::vector::const_at(local_axis, 2));
        auto const local_qw(std::cos(local_half_rotation));

        // 四元数から軸ベクトルを算出する。
        auto const local_xx(local_qx * local_qx);
        auto const local_xy(local_qx * local_qy);
        auto const local_xz(local_qx * local_qz);
        auto const local_xw(local_qx * local_qw);
        auto const local_yy(local_qy * local_qy);
        auto const local_yz(local_qy * local_qz);
        auto const local_yw(local_qy * local_qw);
        auto const local_zz(local_qz * local_qz);
        auto const local_zw(local_qz * local_qw);
        typename this_type::axis_array local_axes;
        local_axes[0] = this_type::coordinate::make(
            (local_yy + local_zz) * -2 + 1,
            (local_xy + local_zw) * 2,
            (local_xz - local_yw) * 2);
        local_axes[1] = this_type::coordinate::make(
            (local_xy - local_zw) * 2,
            (local_xx + local_zz) * -2 + 1,
            (local_yz + local_xw) * 2);
        local_axes[2] = this_type::coordinate::make(
            (local_xz + local_yw) * 2,
            (local_yz - local_xw) * 2,
            (local_xx + local_yy) * -2 + 1);

        // 大きさを正規化する。
        auto local_extent(this_type::coordinate::make(in_extent));
        for (unsigned i(0); i < this_type::coordinate::DIMENSION; ++i)
        {
            auto const local_element(
                psyq::geometry::vector::const_at(in_extent, i));
            if (local_element < 0)
            {
                psyq::geometry::vector::at(local_extent, i) = -local_element;
            }
        }
        return this_type(
            this_type::coordinate::make(in_center), local_extent, local_axes);
    }

    //-------------------------------------------------------------------------
    /// @brief 直方体の中心位置。
    private: typename this_type::coordinate::vector center_;
    /// @brief 直方体の大きさの1/2。
    private: typename this_type::coordinate::vector extent_;
    /// @brief 直方体の軸方向の単位ベクトルの配列。
    private: typename this_type::axis_array axes_;

}; // class psyq::geometry::box

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

#endif // !defined(PSYQ_GEOMETRY_BOX_HPP_)
// vim: set expandtab:
