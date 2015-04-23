/** @file
    @brief @copybrief psyq::geometry::plane
    @author Hillco Psychi (https://twitter.com/psychi)
    @ingroup psyq_geometry_shape
 */
#ifndef PSYQ_GEOMETRY_PLANE_HPP_
#define PSYQ_GEOMETRY_PLANE_HPP_

//#include "./ray.hpp"

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename> class plane;
    } // namespace geometry
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 平面。
    @tparam template_coordinate @copydoc plane::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::plane: public psyq::geometry::ray<template_coordinate>
{
    /// @brief thisが指す値の型。
    private: typedef plane this_type;

    /// @brief this_type の基底型。
    public: typedef psyq::geometry::ray<template_coordinate> base_type;

    public: class point_collision;
    public: class line_collision;
    public: class plane_collision;

    //-------------------------------------------------------------------------
    /** @brief 平面を構築する。
        @param[in] in_origin    平面の原点。
        @param[in] in_direction 平面の法線。
     */
    public: plane(
        typename base_type::point const& in_origin,
        typename base_type::direction const& in_direction)
    PSYQ_NOEXCEPT:
    base_type(in_origin, in_direction)
    {}

    /** @brief 平面を構築する。
        @param[in] in_line 平面の法線として用いる直線。
     */
    public: explicit plane(typename base_type::base_type const& in_line):
    base_type(in_line)
    {}

}; // namespace psyq::geometry::plane

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 平面と点の衝突判定。
    @tparam template_coordinate @copydoc point_collision::coordinate
 */
template<typename template_coordinate>
class psyq::geometry::plane<template_coordinate>::point_collision
{
    /// @brief thisが指す値の型。
    private: typedef point_collision this_type;

    /// @brief @copydoc psyq::geometry::point::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief 平面と点の衝突判定を構築する。
        @param[in] in_plane 衝突判定の左辺となる平面。
        @param[in] in_point 衝突判定の右辺となる点の位置。
     */
    public: static this_type make(
        psyq::geometry::plane<template_coordinate> const& in_plane,
        typename this_type::coordinate::vector const& in_point)
    {
        auto const local_difference(in_point - in_plane.origin_.get_position());
        return this_type(
            local_difference,
            psyq::geometry::vector::dot(
                local_difference, in_plane.direction_.get_unit()));
    }

    //-------------------------------------------------------------------------
    /** @brief 平面と点の衝突判定を構築する。
        @param[in] in_difference @copydoc this_type::difference_
        @param[in] in_distance   @copydoc this_type::distance_
     */
    protected: point_collision(
        typename this_type::coordinate::vector const& in_difference,
        typename this_type::coordinate::element const in_distance)
    :
    difference_(in_difference),
    distance_(in_distance)
    {}

    //-------------------------------------------------------------------------
    /// @brief 平面の原点から点までのベクトル。
    private: typename this_type::coordinate::vector difference_;
    /// @brief 平面から点までの相対位置。
    private: typename this_type::coordinate::element distance_;

}; // psyq::geometry::plane::point_collision

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 平面と直線の衝突判定。
    @tparam template_coordinate @copydoc line_collision::coordinate
 */
template<typename template_coordinate>
class psyq::geometry::plane<template_coordinate>::line_collision
{
    /// @brief thisが指す値の型。
    private: typedef line_collision this_type;

    /// @brief @copydoc psyq::geometry::point::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief 平面と直線の衝突判定を構築する。
        @param[in] in_plane   衝突判定の左辺となる平面。
        @param[in] in_line    衝突判定の右辺となる直線。
        @param[in] in_epsilon 誤差の範囲。0以上であること。
     */
    public: static this_type make(
        psyq::geometry::plane<template_coordinate> const& in_plane,
        psyq::geometry::line<template_coordinate> const& in_line,
        typename this_type::coordinate::element const in_epsilon)
    {
        PSYQ_ASSERT(0 <= in_epsilon);
        auto const local_direction_dot(
            psyq::geometry::vector::dot(
                in_plane.direction_.get_unit(),
                in_line.direction_.get_unit()));
        if (local_direction_dot < -in_epsilon || in_epsilon < local_direction_dot)
        {
            auto const local_begin_dot(
                psyq::geometry::vector::dot(
                    in_plane.origin_.get_position(),
                    in_line.origin_.get_position()));
            auto const local_end_dot(
                psyq::geometry::vector::dot(
                    in_plane.origin_.get_position(),
                    in_line.origin_.get_position() + in_line.direction_.get_unit()));
            return this_type(
                local_direction_dot,
                local_begin_dot / (local_begin_dot + local_end_dot));
        }
        else
        {
            // 平面と直線が平行だった。
            return this_type(
                0,
                psyq::geometry::vector::dot(
                    in_line.origin_.get_position() - in_plane.origin_.get_position(),
                    in_plane.direction_.get_unit()));
        }
    }

    /** @brief 平面の法線と直線の方向の内積を取得する。
        @return this_type::direction_dot_
     */
    public: typename this_type::coordinate::element get_direction_dot()
    const PSYQ_NOEXCEPT
    {
        return this->direction_dot_;
    }

    /** @brief 平面と直線の交点の、直線上の位置を取得する。
        @retval !=nullptr 平面と直線の交点の、直線上の位置。
        @retval ==nullptr 平面と直線が交わらなかった。
     */
    public: typename this_type::coordinate::element const* get_cross_position()
    const PSYQ_NOEXCEPT
    {
        return this->get_direction_dot() != 0 || this->distance_ == 0?
            &this->distance_: nullptr;
    }

    /** @brief 平面と直線が平行していた場合の、平面から直線の位置を取得する。
        @retval !=nullptr 平行な平面と直線の、平面から直線の位置。
        @retval ==nullptr 平面と直線が平行ではなかった。
     */
    public: typename this_type::coordinate::element const* get_parralel_position()
    {
        return this->get_direction_dot() != 0? nullptr: &this->distance_;
    }

    //-------------------------------------------------------------------------
    /** @brief 平面と点の衝突判定を構築する。
        @param[in] in_direction_dot @copydoc this_type::direction_dot_
        @param[in] in_distance      @copydoc this_type::distance_
     */
    protected: line_collision(
        typename this_type::coordinate::element const in_direction_dot,
        typename this_type::coordinate::element const in_distance)
    :
    direction_dot_(in_direction_dot),
    distance_(in_distance)
    {}

    //-------------------------------------------------------------------------
    /// @brief 平面の法線と直線の方向の内積。
    private: typename this_type::coordinate::element direction_dot_;
    /// @brief 平面と直線の交点の、直線上の位置。
    private: typename this_type::coordinate::element distance_;

}; // psyq::geometry::plane::line_collision

#endif // PSYQ_GEOMETRY_PLANE_HPP_
// vim: set expandtab:
