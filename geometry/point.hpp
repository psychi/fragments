/** @file
    @brief @copybrief psyq::geometry::point
    @author Hillco Psychi (https://twitter.com/psychi)
    @ingroup psyq_geometry psyq::geometry
 */
#ifndef PSYQ_GEOMETRY_POINT_HPP_
#define PSYQ_GEOMETRY_POINT_HPP_

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename> class point;
    } // namespace geometry
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 点。
    @tparam template_coordinate @copydoc point::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::point
{
    /// thisが指す値の型。
    private: typedef point this_type;

    /// @copydoc psyq::geometry::direction::coordinate
    public: typedef template_coordinate coordinate;

    /// @cond
    public: class point_collision;
    /// @endcond

    //-------------------------------------------------------------------------
    /** @brief 点を構築する。
        @param[in] in_position 点の位置ベクトル。
     */
    public: explicit point(
        typename this_type::coordinate::vector const& in_position)
    :
    position_((
        PSYQ_ASSERT(this_type::coordinate::validate(in_position)),
        in_position))
    {}

    /** @brief 点を構築する。
        @param[in] in_position 点の位置ベクトル。
        @return 点。
     */
    public: static this_type make(
        typename this_type::coordinate::vector const& in_position)
    {
        return this_type(this_type::coordinate::make(in_position));
    }

    /** @brief 点の位置ベクトルを取得する。
        @return @copydoc this_type::position_
     */
    public: typename this_type::coordinate::vector const& get_position()
    const PSYQ_NOEXCEPT
    {
        return this->position_;
    }

    /** @brief 点の位置ベクトルを設定する。
        @param[in] in_position 新たに設定する点の位置ベクトル。
     */
    public: void set_position(
        typename this_type::coordinate::vector const& in_position)
    {
        *this = this_type::make(in_position);
    }

    //-------------------------------------------------------------------------
    /// 点の位置ベクトル。
    private: typename this_type::coordinate::vector position_;

}; // class psyq::geometry::point

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 点と点の衝突判定。
    @tparam template_coordinate @copydoc psyq::geometry::point::coordinate
 */
template<typename template_coordinate>
class psyq::geometry::point<template_coordinate>::point_collision
{
    /// thisが指す値の型。
    private: typedef point_collision this_type;

    /// @copydoc psyq::geometry::point::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief 点と点の衝突判定を構築する。
        @param[in] in_source 衝突判定の左辺となる点の位置。
        @param[in] in_target 衝突判定の右辺となる点の位置。
     */
    public: static this_type make(
        typename this_type::coordinate::vector const& in_source,
        typename this_type::coordinate::vector const& in_target)
    {
        return this_type(this_type::coordinate::make(in_target - in_source));
    }

    /** @brief 点と点が衝突していたか判定する。
        @param[in] in_range 衝突と判定する距離。
        @retval true  衝突している。
        @retval false 衝突していない。
     */
    public: bool detect(typename this_type::coordinate::element in_range)
    const PSYQ_NOEXCEPT
    {
        return this->get_square_distance() <= in_range * in_range;
    }

    /** @brief 点と点の差分を取得する。
        @return @copydoc this_type::difference_
     */
    public: typename this_type::coordinate::vector const& get_difference()
    const PSYQ_NOEXCEPT
    {
        return this->difference_;
    }

    /** @brief 点と点の距離の自乗を取得する。
        @return @copydoc this_type::square_distance_
     */
    public: typename this_type::coordinate::element get_square_distance()
    const PSYQ_NOEXCEPT
    {
        return this->square_distance_;
    }

    //-------------------------------------------------------------------------
    /** @brief 点と点の衝突判定を構築する。
        @param[in] in_difference 点と点の差分。
     */
    protected: explicit point_collision(
        typename this_type::coordinate::vector const& in_difference)
    :
    difference_(in_difference),
    square_distance_(
        psyq::geometry::vector::dot(in_difference, in_difference))
    {}

    //-------------------------------------------------------------------------
    /// 点と点の差分。
    private: typename this_type::coordinate::vector difference_;

    /// 点と点の距離の自乗。
    private: typename this_type::coordinate::element square_distance_;

}; // psyq::geometry::point::point_collision

#endif // PSYQ_GEOMETRY_POINT_HPP_
// vim: set expandtab:
