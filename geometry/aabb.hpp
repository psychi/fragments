/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::geometry::aabb
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
        for (unsigned i(0); i < this_type::coordinate::DIMENSION; ++i)
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

    /** @brief 2つのAABBが衝突しているか判定する。
        @retval true  衝突している。
        @retval false 衝突してない。
        @param[in] in_aabb_0 AABB#0
        @param[in] in_aabb_1 AABB#1
     */
    public: static bool detect_collision(
        this_type const& in_aabb_0,
        this_type const& in_aabb_1)
    {
        auto const local_diff_0(in_aabb_0.get_min() - in_aabb_1.get_max());
        auto const local_diff_1(in_aabb_1.get_min() - in_aabb_0.get_max());
        for (unsigned i(0); i < this_type::coordinate::DIMENSION; ++i)
        {
            if (0 <= this_type::coordinate::get_element(local_diff_0, i) ||
                0 <= this_type::coordinate::get_element(local_diff_1, i))
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
        typedef std::array<
            typename this_type::coordinate::element, 
            this_type::coordinate::DIMENSION>
                element_container;
        element_container local_min;
        element_container local_max;
        for (unsigned i(0); i < this_type::coordinate::DIMENSION; ++i)
        {
            auto const local_element(
                std::minmax(
                    this_type::coordinate::get_element(in_point_a, i),
                    this_type::coordinate::get_element(in_point_b, i)));
            local_min[i] = local_element.first;
            local_max[i] = local_element.second;
        }
        return this_type(
            this_type::coordinate::make_vector(local_min),
            this_type::coordinate::make_vector(local_max));
    }

    //-------------------------------------------------------------------------
    private: typename this_type::coordinate::vector min_; ///< AABBの最小座標。
    private: typename this_type::coordinate::vector max_; ///< AABBの最大座標。
};

namespace psyq
{
    namespace geometry
    {
        /** @brief AABBのAABBを構築する。
            @param[in] in_aabb AABBを構築するAABB。
            @return AABBのAABB。
         */
        template<typename template_coordinate>
        psyq::geometry::aabb<template_coordinate> make_aabb(
            psyq::geometry::aabb<template_coordinate> const& in_aabb)
        {
            return in_aabb
        }
    }
}

#endif // !defined(PSYQ_GEOMETRY_AABB_HPP_)
