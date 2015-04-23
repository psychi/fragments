/** @file
    @brief @copybrief psyq::geometry::aabb
    @author Hillco Psychi (https://twitter.com/psychi)
    @ingroup psyq_geometry_shape
 */
#ifndef PSYQ_GEOMETRY_AABB_HPP_
#define PSYQ_GEOMETRY_AABB_HPP_

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
    @tparam template_coordinate @copydoc aabb::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::aabb
{
    /// @brief thisが指す値の型。
    private: typedef aabb this_type;

    /// @brief @copydoc psyq::geometry::ball::coordinate
    public: typedef template_coordinate coordinate;

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
/** @brief AABBとAABBの衝突判定。
    @tparam template_coordinate @copydoc aabb_collision::coordinate
 */
template<typename template_coordinate>
class psyq::geometry::aabb<template_coordinate>::aabb_collision
{
    /// @brief thisが指す値の型。
    private: typedef aabb_collision this_type;

    /// @brief @copydoc psyq::geometry::aabb::coordinate
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
        return psyq::geometry::vector::less_than_equal(
                in_source.get_min(), in_target.get_max())
            && psyq::geometry::vector::less_than_equal(
                in_target.get_min(), in_source.get_max());
    }

}; // class psyq::geometry::aabb::aabb_collision

#endif // !defined(PSYQ_GEOMETRY_AABB_HPP_)
// vim: set expandtab:
