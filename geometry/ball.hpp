/** @file
    @brief @copybrief psyq::geometry::ball
    @author Hillco Psychi (https://twitter.com/psychi)
    @ingroup psyq_geometry_shape
 */
#ifndef PSYQ_GEOMETRY_BALL_HPP_
#define PSYQ_GEOMETRY_BALL_HPP_

#include "./point.hpp"

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename> class ball;
    } // namespace geometry
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 球。
    @tparam template_coordinate @copydoc ball::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::ball
{
    /// @brief thisが指す値の型。
    private: typedef ball this_type;

    /// @brief @copydoc psyq::geometry::point::coordinate
    public: typedef template_coordinate coordinate;

    /// @brief @copydoc psyq::geometry::point
    public: typedef psyq::geometry::point<template_coordinate> point;

    //-------------------------------------------------------------------------
    /** @brief 球を構築する。
        @param[in] in_center 球の中心点。
        @param[in] in_radius 球の半径。0以上であること。
     */
    public: ball(
        typename this_type::point const& in_center,
        typename this_type::coordinate::element const in_radius)
    :
    center_(in_center),
    radius_((PSYQ_ASSERT(0 <= in_radius), in_radius))
    {}

    /** @brief 球を構築する。
        @param[in] in_center 球の中心点。
        @param[in] in_radius 球の半径。0未満の場合は0になる。
     */
    public: static this_type make(
        typename this_type::coordinate::vector const& in_center,
        typename this_type::coordinate::element const in_radius)
    {
        return this_type(
            this_type::point::make(in_center),
            this_type::arrange_radius(in_radius));
    }

    /** @brief 球の半径を取得する。
        @return 球の半径。
     */
    public: typename this_type::coordinate::element get_radius()
    const PSYQ_NOEXCEPT
    {
        return this->radius_;
    }

    /** @brief 球の半径を設定する。
        @param[in] in_radius 新たに設定する球の半径。0未満の場合は0になる。
     */
    public: void set_radius(
        typename this_type::coordinate::element const in_radius)
    {
        this->radius_ = this_type::arrange_radius(in_radius);
    }

    private: static typename this_type::coordinate::element arrange_radius(
        typename this_type::coordinate::element const in_radius)
    {
        return 0 < in_radius? in_radius: 0;
    }

    //-------------------------------------------------------------------------
    /// @brief 球の中心点。
    public: typename this_type::point center_;
    /// @brief 球の半径。
    private: typename this_type::coordinate::element radius_;

}; // class psyq::geometry::ball

#endif // PSYQ_GEOMETRY_BALL_HPP_
// vim: set expandtab:
