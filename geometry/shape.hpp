/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief 幾何形状オブジェクト群。
    @defgroup psyq_geometry_shape 幾何形状オブジェクト
    @ingroup psyq_geometry psyq::geometry
 */
#ifndef PSYQ_GEOMETRY_SHAPE_HPP_
#define PSYQ_GEOMETRY_SHAPE_HPP_

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename> class direction;
        template<typename> class point;
        template<typename> class ball;
        template<typename> class line;
        template<typename> class ray;
        template<typename> class plane;
        template<typename> class box;
    } // namespace geometry
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 方向。
    @tparam template_coordinate @copydoc psyq::geometry::direction::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::direction
{
    /// thisが指す値の型。
    private: typedef direction this_type;

    /** @brief 座標系の型特性。

        psyq::geometry::coordinate 互換のインターフェイスを持っていること。
     */
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief 方向を構築する。
        @param[in] in_unit 方向の単位ベクトル。
     */
    public: explicit direction(
        typename this_type::coordinate::vector const& in_unit)
    :
    unit_((
        PSYQ_ASSERT(
            this_type::coordinate::validate(in_unit) &&
            psyq::geometry::vector::nearly_length(in_unit, 1)),
        in_unit))
    {}

    /** @brief 方向を構築する。
        @param[in] in_direction
            方向ベクトル。正規化されてなくともよい。
            ただし0ベクトルの場合は、任意の単位ベクトルに変換する。
        @return 方向。
     */
   public: static this_type make(
        typename this_type::coordinate::vector const& in_direction)
    {
        return this_type(
            psyq::geometry::vector::normalize(
                this_type::coordinate::make(in_direction)));
    }

    /** @brief 方向の単位ベクトルを取得する。
        @return @copydoc this_type::unit_
     */
    public: typename this_type::coordinate::vector const& get_unit()
    const PSYQ_NOEXCEPT
    {
        return this->unit_;
    }

    /** @brief 方向の単位ベクトルを設定する。
        @param[in] in_direction 新たに設定する方向ベクトル。
     */
    public: void set_unit(
        typename this_type::coordinate::vector const& in_direction)
    {
        *this = this_type::make(in_direction);
    }

    //-------------------------------------------------------------------------
    /// 方向の単位ベクトル。
    private: typename this_type::coordinate::vector unit_;

}; // class psyq::geometry::direction

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 点。
    @tparam template_coordinate @copydoc this_type::coordinate
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
    @ingroup psyq_geometry_shape
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

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 球。
    @tparam template_coordinate @copydoc this_type::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::ball
{
    /// thisが指す値の型。
    private: typedef ball this_type;

    /// @copydoc psyq::geometry::point::coordinate
    public: typedef template_coordinate coordinate;

    /// @copydoc psyq::geometry::point
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
    /// 球の中心点。
    public: typename this_type::point center_;
    /// 球の半径。
    private: typename this_type::coordinate::element radius_;

}; // class psyq::geometry::ball

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 直線。
    @tparam template_coordinate @copydoc this_type::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::line
{
    /// thisが指す値の型。
    private: typedef line this_type;

    /// @copydoc psyq::geometry::direction::coordinate
    public: typedef template_coordinate coordinate;

    /// 直線上の点。
    public: typedef psyq::geometry::point<template_coordinate> point;

    /// 直線の方向。
    public: typedef psyq::geometry::direction<template_coordinate> direction;

    /// @cond
    public: class point_collision;
    public: class line_collision;
    /// @endcond

    //-------------------------------------------------------------------------
    /** @brief 直線を構築する。
        @param[in] in_origin    直線の原点。
        @param[in] in_direction 直線の方向。
     */
    public: line(
        typename this_type::point const& in_origin,
        typename this_type::direction const& in_direction)
    PSYQ_NOEXCEPT:
    origin_(in_origin),
    direction_(in_direction)
    {}

    /** @brief 直線上の点を構築する。
        @param[in] in_position 直線の原点からの位置。
        @return 直線上の点。
     */
    public: typename this_type::point make_point(
        typename this_type::coordinate::element in_position)
    const
    {
        return this_type::point(
            this->origin_.get_position()
            + this->direction_.get_unit() * in_position);
    }

    //-------------------------------------------------------------------------
    /// 直線の原点。
    public: typename this_type::point origin_;
    /// 直線の方向。
    public: typename this_type::direction direction_;

}; // namespace psyq::geometry::line

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 直線と点の衝突判定。
    @tparam template_coordinate @copydoc psyq::geometry::line::coordinate
 */
template<typename template_coordinate>
class psyq::geometry::line<template_coordinate>::point_collision
{
    /// thisが指す値の型。
    private: typedef point_collision this_type;

    /// @copydoc psyq::geometry::line::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief 直線と点の衝突判定を構築する。
        @param[in] in_line  衝突判定の左辺となる直線。
        @param[in] in_point 衝突判定の右辺となる点。
     */
    public: static this_type make(
        psyq::geometry::line<template_coordinate> const& in_line,
        typename this_type::coordinate::vector const& in_point)
    {
        auto const local_difference(
            this_type::coordinate::make(in_point - in_line.origin_.get_position()));
        auto const local_cross_position(
            psyq::geometry::dot(in_line.direction_.get_unit(), local_difference));
        auto const local_perpendicular(
            in_line.direction_.get_unit() * local_cross_position - local_difference);
        return this_type(
            local_difference,
            local_perpendicular,
            local_cross_position,
            psyq::geometry::dot(local_perpendicular, local_perpendicular));
    }

    /** @brief 直線と点の衝突判定を構築する。
        @param[in] in_line       衝突判定の左辺となる直線。
        @param[in] in_line_begin 直線上にある衝突区間の始点の位置。
        @param[in] in_line_end   直線上にある衝突区間の終点の位置。
        @param[in] in_point      衝突判定の右辺となる点。
     */
    public: static this_type make(
        psyq::geometry::line<template_coordinate> const& in_line,
        typename this_type::coordinate::element const in_line_begin,
        typename this_type::coordinate::element const in_line_end,
        typename this_type::coordinate::vector const& in_point)
    {
        PSYQ_ASSERT(in_line_begin <= in_line_end);
        auto const local_difference(
            this_type::coordinate::make(in_point - in_line.origin_.get_position()));
        auto const local_cross_position(
            psyq::geometry::dot(in_line.direction_.get_unit(), local_difference));
        if (local_cross_position < in_line_begin || in_line_end < local_cross_position)
        {
            // 衝突区間の外で交わっている。
            return this_type(
                local_difference,
                local_difference,
                local_cross_position,
                (std::numeric_limits<typename this_type::coordinate::element>::max)());
        }
        else
        {
            // 衝突区間の内で交わっている。
            auto const local_perpendicular(
                in_line.direction_.get_unit() * local_cross_position - local_difference);
            return this_type(
                local_difference,
                local_perpendicular,
                local_cross_position,
                psyq::geometry::dot(local_perpendicular, local_perpendicular));
        }
    }

    /** @brief 直線と点が衝突しているか判定する。
        @param[in] in_range 衝突と判定する距離。
        @retval true  衝突している。
        @retval false 衝突していない。
     */
    public: bool detect(typename this_type::coordinate::element in_range)
    const PSYQ_NOEXCEPT
    {
        return this->get_square_distance() <= in_range * in_range;
    }

    /** @brief 直線の始点から点へのベクトルを取得する。
        @return @copydoc this_type::difference_
     */
    public: typename this_type::coordinate::vector const& get_difference()
    const PSYQ_NOEXCEPT
    {
        return this->difference_;
    }

    /** @brief 点から直線への垂線を取得する。
        @return @copydoc this_type::perpendicular_
     */
    public: typename this_type::coordinate::vector const& get_perpendicular()
    const PSYQ_NOEXCEPT
    {
        return this->perpendicular_;
    }

    /** @brief 直線と、点から直線への垂線が交わる、直線上の位置を取得する。
        @return @copydoc this_type::cross_position_
     */
    public: typename this_type::coordinate::element get_cross_position()
    const PSYQ_NOEXCEPT
    {
        return this->cross_position_;
    }

    /** @brief 直線と点の距離の自乗を取得する。
        @return @copydoc this_type::square_distance_
     */
    public: typename this_type::coordinate::element get_square_distance()
    const PSYQ_NOEXCEPT
    {
        return this->square_distance_;
    }

    //-------------------------------------------------------------------------
    protected: point_collision(
        typename this_type::coordinate::vector const& in_difference,
        typename this_type::coordinate::vector const& in_perpendicular,
        typename this_type::coordinate::element const in_cross_position,
        typename this_type::coordinate::element const in_square_distance)
    :
    difference_(in_difference),
    perpendicular_(in_perpendicular),
    cross_position_(in_cross_position),
    square_distance_(in_square_distance)
    {}

    //-------------------------------------------------------------------------
    /// 直線の始点から点へのベクトル。
    private: typename this_type::coordinate::vector difference_;
    /// 点から直線への垂線。
    private: typename this_type::coordinate::vector perpendicular_;
    /// 直線と、点から直線への垂線が交わる、直線上の位置。
    private: typename this_type::coordinate::element cross_position_;
    /// 直線と点の距離の自乗。
    private: typename this_type::coordinate::element square_distance_;

}; // class psyq::geometry::line::point_collision

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 直線と直線の衝突判定。
    @tparam template_coordinate @copydoc psyq::geometry::line::coordinate
 */
template<typename template_coordinate>
class psyq::geometry::line<template_coordinate>::line_collision
{
    /// thisが指す値の型。
    private: typedef line_collision this_type;

    /// @copydoc psyq::geometry::line::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief 直線と直線の衝突判定を行う。
        @param[in] in_source 衝突判定の左辺となる直線。
        @param[in] in_target 衝突判定の右辺となる直線。
     */
    public: static this_type make(
        psyq::geometry::line<template_coordinate> const& in_source,
        psyq::geometry::line<template_coordinate> const& in_target,
        typename this_type::coordinate::element const in_epsilon)
    {
        PSYQ_ASSERT(0 <= in_epsilon);
        auto const local_origin_difference(
            in_target.origin_.get_position() - in_source.origin_.get_position());
        auto const local_direction_dot(
            psyq::geometry::dot(
                in_source.direction_.get_unit(), in_target.direction_.get_unit()));
        auto const local_denominator(
            1 - local_direction_dot * local_direction_dot);
        if (local_denominator < -in_epsilon || in_epsilon < local_denominator)
        {
            auto const local_source_dot(
                psyq::geometry::dot(
                    local_origin_difference, in_source.direction_.get_unit()));
            auto const local_target_dot(
                psyq::geometry::dot(
                    local_origin_difference, in_target.direction_.get_unit()));
            auto const local_inverse_denominator(1 / local_denominator);
            auto const local_source_position(
                (local_source_dot - local_target_dot * local_direction_dot)
                * local_inverse_denominator);
            auto const local_target_position(
                (local_target_dot - local_source_dot * local_direction_dot)
                * -local_inverse_denominator);
            auto const local_source_point(
                in_source.make_point(local_source_position).get_position());
            auto const local_target_point(
                in_target.make_point(local_target_position).get_position());
            auto const local_point_difference(
                local_target_point - local_source_point);
            return this_type(
                local_source_point,
                local_source_position,
                local_target_point,
                local_target_position,
                local_origin_difference,
                local_direction_dot,
                local_point_difference,
                psyq::geometry::dot(
                    local_point_difference, local_point_difference));
        }
        else
        {
            // 左辺と右辺の直線が平行だったので、
            // 左辺の直線と右辺の原点の衝突判定を構築する。
            auto const local_cross_position(
                psyq::geometry::dot(
                    local_origin_difference, in_source.direction_.get_unit()));
            auto const local_perpendicular(
                local_origin_difference
                - in_source.direction_.get_unit() * local_cross_position);
            return this_type(
                in_target.origin_.get_position() - local_perpendicular,
                local_cross_position,
                in_target.origin_.get_position(),
                0,
                local_origin_difference,
                1,
                local_perpendicular,
                psyq::geometry::vector::dot(
                    local_perpendicular, local_perpendicular));
        }
    }

    /** @brief 左辺となる直線上にある最短点を取得する。
        @return @copydoc this_type::source_point_
     */
    public: typename this_type::coordinate::vector const& get_source_point()
    const PSYQ_NOEXCEPT
    {
        return this->source_point_;
    }

    /** @brief 左辺となる直線上の、最短点の位置を取得する。
        @return @copydoc this_type::source_position_
     */
    public: typename this_type::coordinate::element const get_source_position()
    const PSYQ_NOEXCEPT
    {
        return this->source_position_;
    }

    /** @brief 右辺となる直線上にある最短点を取得する。
        @return @copydoc this_type::target_point_
     */
    public: typename this_type::coordinate::vector const& get_target_point()
    const PSYQ_NOEXCEPT
    {
        return this->target_point_;
    }

    /** @brief 右辺となる直線上の、最短点の位置を取得する。
        @return @copydoc this_type::target_position_
     */
    public: typename this_type::coordinate::element const get_target_position()
    const PSYQ_NOEXCEPT
    {
        return this->target_position_;
    }

    //-------------------------------------------------------------------------
    protected: line_collision(
        typename this_type::coordinate::vector const& in_source_point,
        typename this_type::coordinate::element const in_source_position,
        typename this_type::coordinate::vector const& in_target_point,
        typename this_type::coordinate::element const in_target_position,
        typename this_type::coordinate::vector const& in_origin_difference,
        typename this_type::coordinate::element const in_direction_dot,
        typename this_type::coordinate::vector const& in_point_difference,
        typename this_type::coordinate::element const in_square_distance)
    :
    source_point_(in_source_point),
    target_point_(in_target_point),
    point_difference_(in_point_difference),
    origin_difference_(in_origin_difference),
    source_position_(in_source_position),
    target_position_(in_target_position),
    square_distance_(in_square_distance),
    direction_dot_(in_direction_dot)
    {}

    //-------------------------------------------------------------------------
    /// 左辺となる直線上にある最短点。
    private: typename this_type::coordinate::vector source_point_;
    /// 右辺となる直線上にある最短点。
    private: typename this_type::coordinate::vector target_point_;
    /// 左辺の最短点から右辺の最短点へのベクトル。
    private: typename this_type::coordinate::vector point_difference_;
    /// 左辺の原点から右辺の原点へのベクトル。
    private: typename this_type::coordinate::vector origin_difference_;
    /// 左辺となる直線上の、最短点の位置。
    private: typename this_type::coordinate::element source_position_;
    /// 右辺となる直線上の、最短点の位置。
    private: typename this_type::coordinate::element target_position_;
    /// 直線と直線の最短距離の自乗。
    private: typename this_type::coordinate::element square_distance_;
    /// 左辺の方向と右辺の方向の内積。
    private: typename this_type::coordinate::element direction_dot_;

}; // class psyq::geometry::line::line_collision

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 半直線。
    @tparam template_coordinate @copydoc this_type::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::ray: public psyq::geometry::line<template_coordinate>
{
    /// thisが指す値の型。
    private: typedef ray this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::line<template_coordinate> base_type;

    /// @cond
    public: class triangle_3d;
    /// @endcond

    //-------------------------------------------------------------------------
    /** @brief 半直線を構築する。
        @param[in] in_origin    半直線の原点。
        @param[in] in_direction 半直線の方向。
     */
    public: ray(
        typename base_type::point const& in_origin,
        typename base_type::direction const& in_direction)
    PSYQ_NOEXCEPT:
    base_type(in_origin, in_direction)
    {}

    /** @brief 半直線を構築する。
        @param[in] in_line 半直線として用いる直線。
     */
    public: explicit ray(base_type const& in_line): base_type(in_line) {}

}; // namespace psyq::geometry::ray

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 平面。
    @tparam template_coordinate @copydoc this_type::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::plane: public psyq::geometry::ray<template_coordinate>
{
    /// thisが指す値の型。
    private: typedef plane this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::ray<template_coordinate> base_type;

    /// @cond
    public: class point_collision;
    public: class line_collision;
    public: class plane_collision;
    /// @endcond

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
    @tparam template_coordinate @copydoc psyq::geometry::plane::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::plane<template_coordinate>::point_collision
{
    /// thisが指す値の型。
    private: typedef point_collision this_type;

    /// @copydoc psyq::geometry::point::coordinate
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
    /// 平面の原点から点までのベクトル。
    private: typename this_type::coordinate::vector difference_;
    /// 平面から点までの相対位置。
    private: typename this_type::coordinate::element distance_;

}; // psyq::geometry::plane::point_collision

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 平面と直線の衝突判定。
    @tparam template_coordinate @copydoc psyq::geometry::plane::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::plane<template_coordinate>::line_collision
{
    /// thisが指す値の型。
    private: typedef line_collision this_type;

    /// @copydoc psyq::geometry::point::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief 平面と直線の衝突判定を構築する。
        @param[in] in_plane 衝突判定の左辺となる平面。
        @param[in] in_line 衝突判定の右辺となる直線。
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
                local_begin_dot / (local_begin_dot + local_end_dot))
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
     */
    public: typename this_type::coordinate::element const* get_cross_position()
    const PSYQ_NOEXCEPT
    {
        return this->get_direction_dot() != 0 || this->distance_ == 0?
            &this->distance_: nullptr;
    }

    /** @brief 平面と直線が平行していた場合の、平面から直線の位置を取得する。
     */
    public: typename this_type::coordinate::element const* get_parralel_position()
    {
        return this->get_direction_dot() != 0? nullptr: &this->distance_;
    }

    //-------------------------------------------------------------------------
    /** @brief 平面と点の衝突判定を構築する。
        @param[in] in_difference @copydoc this_type::difference_
        @param[in] in_distance   @copydoc this_type::distance_
     */
    protected: line_collision(
        typename this_type::coordinate::element const in_direction_dot,
        typename this_type::coordinate::element const in_distance)
    :
    direction_dot_(in_direction_dot),
    distance_(in_distance)
    {}

    //-------------------------------------------------------------------------
    /// 平面の法線と直線の方向の内積。
    private: typename this_type::coordinate::element direction_dot_;
    private: typename this_type::coordinate::element distance_;

}; // psyq::geometry::plane::line_collision

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 半直線と衝突する三角形。
    @tparam template_coordinate @copydoc this_type::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::ray<template_coordinate>::triangle_3d
{
    /// thisが指す値の型。
    private: typedef triangle_3d this_type;

    /// @copydoc psyq::geometry::ball::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief 初期化。
        @param[in] in_vertex0 三角形の頂点#0
        @param[in] in_vertex1 三角形の頂点#1
        @param[in] in_vertex2 三角形の頂点#2
     */
    public: triangle_3d(
        typename this_type::coordinate::vector const& in_vertex0,
        typename this_type::coordinate::vector const& in_vertex1,
        typename this_type::coordinate::vector const& in_vertex2)
    :
    origin_(in_vertex0)
    {
        PSYQ_ASSERT(base_type::coordinate::validate(in_vertex0));
        PSYQ_ASSERT(base_type::coordinate::validate(in_vertex1));
        PSYQ_ASSERT(base_type::coordinate::validate(in_vertex2));
#if 1
        auto local_edge1(in_vertex1 - in_vertex0);
        auto local_edge2(in_vertex2 - in_vertex0);
        this->normal_ = psyq::geometry::vector::cross_3d(
            local_edge1, local_edge2);
        auto const local_nx(
            psyq::geometry::vector::cross_3d(local_edge2, this->normal_));
        auto const local_ny(
            psyq::geometry::vector::cross_3d(local_edge1, this->normal_));
        this->binormal_u_ = local_nx /
            psyq::geometry::vector::dot(local_edge1, local_nx);
        this->binormal_v_ = local_ny /
            psyq::geometry::vector::dot(local_edge2, local_ny);
#else
        auto const local_e2(
            psyq::geometry::vector::cross_3d(in_vertex0, in_vertex1));
        auto const local_d(
            psyq::geometry::vector::dot(local_e2, in_vertex2));
        if (local_d <= 0)
        {
            PSYQ_ASSERT(false);
            return;
        }
        auto const local_e1(
            psyq::geometry::vector::cross_3d(in_vertex2, in_vertex0));
        this->binormal_u_ = local_e1 / local_d;
        this->binormal_v_ = local_e2 / local_d;
        this->normal_ = psyq::geometry::vector::cross_3d(
            in_vertex1 - in_vertex0, in_vertex2 - in_vertex0);
#endif
    }

    /** @brief 半直線との衝突判定。

        以下の web page を参考にした。
        http://d.hatena.ne.jp/ototoi/20050320/p1

        衝突した場合、衝突位置は以下の式で算出できる。
        in_half_line.origin + in_half_line.direction * out_intersection.x

        また頂点毎に法線がある場合、以下の式で衝突位置の法線が算出できる。
        normal0 * (1 - out_intersection.y - out_intersection.z) +
        normal1 * out_intersection.y +
        normal2 * out_intersection.z

        @param[out] out_tuv       衝突時の[t,u,v]値。
        @param[in]  in_ray        判定する半直線。
        @param[in]  in_ray_length 半直線の長さ。
        @param[in]  in_epsilon    計算機epsilon値。
        @retval true  半直線と衝突した。
        @retval false 半直線と衝突しなかった。
     */
    public: bool intersect(
        typename this_type::coordinate::vector& out_tuv,
        psyq::geometry::ray<template_coordinate> const& in_ray,
        typename this_type::coordinate::element const in_ray_length,
        typename this_type::coordinate::element const in_epsilon = 0)
    const
    {
        auto const local_nv(
            -psyq::geometry::vector::dot(
                in_ray.direction_.get_unit(), this->get_normal()));
        if (local_nv <= in_epsilon)
        {
            return false;
        }

        auto const local_origin_diff(in_ray.origin_.get_position() - this->origin_.get_position());
        auto const local_t(
            psyq::geometry::vector::dot(
                local_origin_diff, this->get_normal()) / local_nv);
        if (local_t < 0 || in_ray_length < local_t)
        {
            return false;
        }

        auto const local_position(
            in_ray.direction_.get_unit() * local_t + local_origin_diff);
        auto const local_u(
            psyq::geometry::vector::dot(
                local_position, this->binormal_u_));
        if (local_u < in_epsilon)
        {
            return false;
        }
        auto const local_v(
            psyq::geometry::vector::dot(local_position, this->binormal_v_));
        if (local_v < in_epsilon || 1 - in_epsilon < local_u + local_v)
        {
            return false;
        }

        out_tuv = this_type::coordinate::make(local_t, local_u, local_v);
        return true;
    }

    public: typename this_type::coordinate::vector const& get_origin() const
    {
        return this->origin_;
    }

    public: typename this_type::coordinate::vector const& get_normal() const
    {
        return this->normal_;
    }

    //-------------------------------------------------------------------------
    /// 三角形の開始位置
    private: typename this_type::coordinate::vector origin_;
    /// 三角形の法線
    private: typename this_type::coordinate::vector normal_;
    /// 三角形の重心座標U
    private: typename this_type::coordinate::vector binormal_u_;
    /// 三角形の重心座標V 
    private: typename this_type::coordinate::vector binormal_v_;

}; // class psyq::geometry::ray::triangle_3d

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 直方体。
    @tparam template_coordinate @copydoc this_type::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::box
{
    /// thisが指す値の型。
    private: typedef box this_type;

    /// @copydoc psyq::geometry::ball::coordinate
    public: typedef template_coordinate coordinate;

    /// 直方体の軸方向の単位ベクトルの配列。
    public: typedef std::array<
        typename this_type::coordinate::vector,
        this_type::coordinate::dimension>
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
        for (unsigned i(0); i < this_type::coordinate::dimension; ++i)
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
            3 <= this_type::coordinate::dimension,
            "'this_type::coordinate::dimension' is less than 3.");

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
        for (unsigned i(0); i < this_type::coordinate::dimension; ++i)
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
    /// 直方体の中心位置。
    private: typename this_type::coordinate::vector center_;
    /// 直方体の大きさの1/2。
    private: typename this_type::coordinate::vector extent_;
    /// 直方体の軸方向の単位ベクトルの配列。
    private: typename this_type::axis_array axes_;

}; // class psyq::geometry::box

#endif // !defined(PSYQ_GEOMETRY_SHAPE_HPP_)
