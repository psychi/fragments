/** @file
    @brief @copybrief psyq::geometry::aabb
    @author Hillco Psychi (https://twitter.com/psychi)
    @ingroup psyq_geometry psyq::geometry
 */
#ifndef PSYQ_GEOMETRY_AABB_HPP_
#define PSYQ_GEOMETRY_AABB_HPP_

//#include "psyq/geometry/line.hpp"

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
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::aabb
{
    /// thisが指す値の型。
    private: typedef aabb this_type;

    /// @copydoc psyq::geometry::ball::coordinate
    public: typedef template_coordinate coordinate;

    public: class point_collision;
    public: class line_collision;
    public: class aabb_collision;

    //-------------------------------------------------------------------------
    /** @brief AABBを構築する。
        @param[in] in_min AABBの最小座標。
        @param[in] in_max AABBの最大座標。
     */
    public: aabb(
        typename this_type::coordinate::vector const& in_min,
        typename this_type::coordinate::vector const& in_max)
    :
    min_((
        PSYQ_ASSERT(psyq::geometry::vector::less_than_equal(in_min, in_max)),
        PSYQ_ASSERT(this_type::coordinate::validate(in_min)),
        in_min)),
    max_((
        PSYQ_ASSERT(this_type::coordinate::validate(in_min)),
        in_max))
    {}

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

    /** @brief 2つの点を含む最小のAABBを構築する。
        @return 点Aと点Bを包むAABB。
        @param[in] in_point_a 点Aの座標。
        @param[in] in_point_b 点Bの座標。
     */
    public: static this_type make(
        typename this_type::coordinate::vector const& in_point_a,
        typename this_type::coordinate::vector const& in_point_b)
    {
        auto local_min(this_type::coordinate::make(in_point_a));
        auto local_max(this_type::coordinate::make(in_point_b));
        for (unsigned i(0); i < this_type::coordinate::dimension; ++i)
        {
            auto const local_element_a(
                psyq::geometry::vector::const_at(in_point_a, i));
            auto const local_element_b(
                psyq::geometry::vector::const_at(in_point_b, i));
            if (local_element_b < local_element_a)
            {
                psyq::geometry::vector::at(local_min, i) = local_element_b;
                psyq::geometry::vector::at(local_max, i) = local_element_a;
            }
        }
        return this_type(local_min, local_max);
    }

    //-------------------------------------------------------------------------
    private: typename this_type::coordinate::vector min_; ///< AABBの最小座標。
    private: typename this_type::coordinate::vector max_; ///< AABBの最大座標。

}; // class psyq::geometry::aabb

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief AABBと点の衝突判定。
    @tparam template_coordinate @copydoc psyq::geometry::aabb::coordinate
 */
template<typename template_coordinate>
class psyq::geometry::aabb<template_coordinate>::point_collision
{
    /// thisが指す値の型。
    private: typedef point_collision this_type;

    /// @copydoc psyq::geometry::aabb::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief AABBと点が衝突しているか判定する。
        @retval true  衝突している。
        @retval false 衝突してない。
        @param[in] in_aabb  衝突判定の左辺となるAABB。
        @param[in] in_point 衝突判定の右辺となる点。
     */
    public: static bool detect(
        psyq::geometry::aabb<template_coordinate> const& in_aabb,
        typename this_type::coordinate::vector const& in_point)
    {
        PSYQ_ASSERT(this_type::coordinate::validate(in_point));
        return psyq::geometry::vector::less_than_equal(in_aabb.get_min(), in_point)
            && psyq::geometry::vector::less_than_equal(in_point, in_aabb.get_max());
    }

}; // class psyq::geometry::aabb::point_collision

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief AABBと直線の衝突判定。
    @tparam template_coordinate @copydoc psyq::geometry::aabb::coordinate
 */
template<typename template_coordinate>
class psyq::geometry::aabb<template_coordinate>::line_collision
{
    /// thisが指す値の型。
    private: typedef line_collision this_type;

    /// @copydoc psyq::geometry::aabb::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief AABBと直線が衝突しているか判定する。

        以下のウェブページを参考にして実装した。
        http://marupeke296.com/COL_3D_No18_LineAndAABB.html

        @param[in] in_aabb        衝突判定を行うAABB。
        @param[in] in_line        衝突判定を行う直線。
        @param[in] in_epsilon_mag 誤差の範囲に使うエプシロン値の倍率。
     */
    public: line_collision(
        psyq::geometry::aabb<template_coordinate> const& in_aabb,
        psyq::geometry::line<template_coordinate> const& in_line,
        unsigned const in_epsilon_mag =
            PSYQ_GEOMETRY_NEARLY_SCALAR_EPSILON_MAG_DEFAULT)
    :
    t_min_(-(std::numeric_limits<typename this_type::coordinate::element>::max)()),
    t_max_( (std::numeric_limits<typename this_type::coordinate::element>::max)()),
    face_min_(0),
    face_max_(0)
    {
        typename this_type::coordinate::element_array local_odd_elements;
        auto const local_epsilon(
            std::numeric_limits<typename this_type::coordinate::element>::epsilon()
            * in_epsilon_mag);
        for (unsigned i(0); i < this_type::coordinate::dimension; ++i)
        {
            auto const local_element(
                psyq::geometry::vector::const_at(in_line.direction_.get_unit(), i));
            local_odd_elements[i] = local_epsilon < std::abs(local_element)?
                1 / local_element: 0;
        }
        auto const local_odd(this_type::coordinate::make(local_odd_elements));
        auto const local_diff_max(
            (in_aabb.get_max() - in_line.origin_.get_position()) * local_odd);
        auto const local_diff_min(
            (in_aabb.get_min() - in_line.origin_.get_position()) * local_odd);
        for (unsigned i(0); i < this_type::coordinate::dimension; ++i)
        {
            if (psyq::geometry::vector::const_at(local_odd, i) != 0)
            {
                // スラブとの距離を算出する。
                auto local_t_far(
                    psyq::geometry::vector::const_at(local_diff_max, i));
                auto local_t_near(
                    psyq::geometry::vector::const_at(local_diff_min, i));
                auto local_face_far(static_cast<std::int8_t>(i + 1));
                auto local_face_near(-local_face_far);
                if (local_t_far < local_t_near)
                {
                    std::swap(local_t_near, local_t_far);
                    std::swap(local_face_near, local_face_far);
                }
                if (this->t_min_ < local_t_near)
                {
                    this->t_min_ = local_t_near;
                    this->face_min_ = local_face_near;
                }
                if (local_t_far < this->t_max_)
                {
                    this->t_max_ = local_t_far;
                    this->face_max_ = local_face_far;
                }

                // スラブ交差判定。
                if (!this->is_collide())
                {
                    return;
                }
            }
            else
            {
                // 直線方向と軸が平行だった。
                auto const local_line_origin(
                    psyq::geometry::vector::const_at(in_line.origin_.get_position(), i));
                auto const local_aabb_min(
                    psyq::geometry::vector::const_at(in_aabb.get_min(), i));
                auto const local_aabb_max(
                    psyq::geometry::vector::const_at(in_aabb.get_max(), i));
                if (local_line_origin < local_aabb_min || local_aabb_max < local_line_origin)
                {
                    return;
                }
            }
        }
    }

    public: bool detect() const
    {
        return this->get_t_min() <= this->get_t_max();
    }

    /** @brief 直線の始点位置から直線上の衝突区間の開始位置までの距離を取得する。
        @return @copydoc t_min_
     */
    public: typename this_type::coordinate::element get_t_min() const
    {
        return this->t_min_;
    }

    /** @brief 直線の始点位置から直線上の衝突区間の終了位置までの距離を取得する。
        @return @copydoc t_max_
     */
    public: typename this_type::coordinate::element get_t_max() const
    {
        return this->t_max_;
    }

    /** @brief 衝突区間の開始位置の座標軸番号を取得する。
        @return @copydoc face_min_
     */
    public: std::int8_t get_face_min() const
    {
        return this->face_min_;
    }

    /** @brief 衝突区間の終了位置の座標軸番号を取得する。
        @return @copydoc face_max_
     */
    public: std::int8_t get_face_max() const
    {
        return this->face_max_;
    }

    //-------------------------------------------------------------------------
    /** @brief 直線の始点位置から直線上の衝突区間の開始位置までの距離。

        - AABBと直線が衝突してる場合は、 t_max_ 以下の値となる。
        - AABBと直線が衝突してない場合は、t_max_ より大きい値となる。
     */
    private: typename this_type::coordinate::element t_min_;

    /** @brief 直線の始点位置から直線上の衝突区間の終了位置までの距離。

        - AABBと直線が衝突してる場合は、 t_min_ 以上の値となる。
        - AABBと直線が衝突してない場合は、 t_min_ より小さい値となる。
     */
    private: typename this_type::coordinate::element t_max_;

    /** @brief 衝突区間の開始位置の座標軸番号。

        - 0なら、直線上の衝突区間の開始位置を検知しなかった。
        - 負値なら、直線上の衝突区間の開始位置がAABBの最小面にある。
          (1 - face_min_)が、座標軸のインデックス番号。
        - 正値なら、直線上の衝突区間の開始位置がAABBの最大面にある。
          (face_min_ - 1)が、座標軸のインデックス番号。
     */
    private: std::int8_t face_min_;

    /** @brief 衝突区間の終了位置の座標軸番号。

        - 0なら、直線上の衝突区間の終了位置を検知しなかった。
        - 負値なら、直線上の衝突区間の終了位置がAABBの最小面にある。
          (1 - face_max_)が、座標軸のインデックス番号。
        - 正値なら、直線上の衝突区間の終了位置がAABBの最大面にある。
          (face_max_ - 1)が、座標軸のインデックス番号。
     */
    private: std::int8_t face_max_;

}; // class psyq::geometry::aabb::line_collision

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief AABBとAABBの衝突判定。
    @tparam template_coordinate @copydoc psyq::geometry::aabb::coordinate
 */
template<typename template_coordinate>
class psyq::geometry::aabb<template_coordinate>::aabb_collision
{
    /// thisが指す値の型。
    private: typedef aabb_collision this_type;

    /// @copydoc psyq::geometry::aabb::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief AABBとAABBが衝突しているか判定する。
        @retval true  衝突している。
        @retval false 衝突してない。
        @param[in] in_source 衝突判定の左辺となるAABB。
        @param[in] in_target 衝突判定の右辺となるAABB。
     */
    public: static bool detect(
        psyq::geometry::aabb<template_coordinate> const& in_source,
        psyq::geometry::aabb<template_coordinate> const& in_target)
     {
        return psyq::geometry::vector::less_than_equal(in_source.get_min(), in_target.get_max())
            && psyq::geometry::vector::less_than_equal(in_target.get_min(), in_source.get_max());
    }

}; // class psyq::geometry::aabb::aabb_collision

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
            psyq::geometry::line<template_coordinate> const& in_segment)
        {
            typename template_coordinate::element_array local_min;
            typename template_coordinate::element_array local_max;
            auto const local_end(
                in_segment.origin_.get_position() + in_segment.direction_.get_unit());
            for (unsigned i(0); i < template_coordinate::dimension; ++i)
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
            typename template_coordinate::element_array local_min;
            typename template_coordinate::element_array local_max;
            for (unsigned i(0); i < template_coordinate::dimension; ++i)
            {
                auto const local_direction(
                    psyq::geometry::vector::const_at(in_ray.direction_.get_unit(), i));
                auto const local_origin(
                    psyq::geometry::vector::const_at(in_ray.origin_.get_position(), i));
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
            return typename template_coordinate::aabb(
                template_coordinate::make(local_min),
                template_coordinate::make(local_max));
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
            for (unsigned i(0); i < template_coordinate::dimension; ++i)
            {
                auto const local_axis(
                    in_box.get_axes()[i] *
                        psyq::geometry::vector::const_at(
                            in_box.get_extent(), i));
                for (unsigned j(0); j < template_coordinate::dimension; ++j)
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
            return in_aabb
        }

    } // namespace geometry
} // namespace psyq

#endif // !defined(PSYQ_GEOMETRY_AABB_HPP_)
// vim: set expandtab:
