/** @file
    @brief @copybrief psyq::geometry::line
    @author Hillco Psychi (https://twitter.com/psychi)
    @ingroup psyq_geometry_shape
 */
#ifndef PSYQ_GEOMETRY_LINE_HPP_
#define PSYQ_GEOMETRY_LINE_HPP_

//#include "./direction.hpp"
//#include "./point.hpp"

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename> class line;
        template<typename> class line_segment;
    } // namespace geometry
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 直線。
    @tparam template_coordinate @copydoc line::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::line
{
    /// @brief thisが指す値の型。
    private: typedef line this_type;

    /// @brief @copydoc psyq::geometry::direction::coordinate
    public: typedef template_coordinate coordinate;

    /// @brief 直線上の点。
    public: typedef psyq::geometry::point<template_coordinate> point;

    /// @brief 直線の方向。
    public: typedef psyq::geometry::direction<template_coordinate> direction;

    public: class aabb_collision;
    public: class point_collision;
    public: class line_collision;

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
/** @brief 線分。
    @tparam template_coordinate @copydoc line_segment::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::line_segment:
public psyq::geometry::line<template_coordinate>
{
    /// @brief thisが指す値の型。
    private: typedef line_segment this_type;

    /// @brief this_type の基底型。
    public: typedef psyq::geometry::line<template_coordinate> base_type;

    //-------------------------------------------------------------------------
    /** @brief 線分を構築する。
        @param[in] in_origin    線分の原点。
        @param[in] in_direction 線分の方向。
        @param[in] in_length    線分の長さ。
     */
    public: line_segment(
        typename base_type::point const& in_origin,
        typename base_type::direction const& in_direction,
        typename base_type::coordinate::element const in_length)
    PSYQ_NOEXCEPT:
    base_type(in_origin, in_direction),
    length_(in_length)
    {}

    /** @brief 線分を構築する。
        @param[in] in_line   線分として用いる直線。
        @param[in] in_length 線分の長さ。
     */
    public: line_segment(
        base_type const& in_line,
        typename base_type::coordinate::element const in_length)
    PSYQ_NOEXCEPT:
    base_type(in_line),
    length_(in_length)
    {}

    /** @brief 線分を構築する。
        @param[in] in_begin 線分の始点。
        @param[in] in_end   線分の終点。
        @return 線分。
     */
    public: static this_type make(
        typename base_type::point const& in_begin,
        typename base_type::point const& in_end)
    {
        return this_type::make(
            in_begin, in_end.get_position() - in_begin.get_position());
    }

    /** @brief 線分を構築する。
        @param[in] in_origin 線分の原点。
        @param[in] in_vector 線分のベクトル。
        @return 線分。
     */
    public: static this_type make(
        typename base_type::point const& in_origin,
        typename base_type::coordinate::vector const& in_vector)
    {
        auto const local_length(
            psyq::geometry::vector::length(
                this_type::coordinate::make(in_vector)));
        PSYQ_ASSERT(0 < local_length);
        return this_type(
            in_origin,
            typename base_type::direction(in_vector / local_length),
            local_length);
    }

    /// @brief 線分の長さ。
    public: typename base_type::coordinate::element length_;

}; // namespace psyq::geometry::line_segment

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 直線とAABBの衝突判定。
    @tparam template_coordinate @copydoc aabb_collision::coordinate
 */
template<typename template_coordinate>
class psyq::geometry::line<template_coordinate>::aabb_collision
{
    /// @brief thisが指す値の型。
    private: typedef aabb_collision this_type;

    /// @brief @copydoc psyq::geometry::line::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief 直線とAABBが衝突しているか判定する。

        以下のウェブページを参考にして実装した。
        http://marupeke296.com/COL_3D_No18_LineAndAABB.html

        @param[in] in_line        衝突判定を行う直線。
        @param[in] in_aabb        衝突判定を行うAABB。
        @param[in] in_epsilon_mag 誤差の範囲に使うエプシロン値の倍率。
     */
    public: aabb_collision(
        psyq::geometry::line<template_coordinate> const& in_line,
        psyq::geometry::aabb<template_coordinate> const& in_aabb,
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
                std::int8_t local_face_near(-local_face_far);
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

    /** @brief 直線とAABBが衝突しているか判定する。
        @retval true  衝突している。
        @retval false 衝突してない。
     */
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

}; // class psyq::geometry::line::aabb_collision

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 直線と点の衝突判定。
    @tparam template_coordinate @copydoc point_collision::coordinate
 */
template<typename template_coordinate>
class psyq::geometry::line<template_coordinate>::point_collision
{
    /// @brief thisが指す値の型。
    private: typedef point_collision this_type;

    /// @brief @copydoc psyq::geometry::line::coordinate
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
            this_type::coordinate::make(
                in_point - in_line.origin_.get_position()));
        auto const local_cross_position(
            psyq::geometry::vector::dot(
                in_line.direction_.get_unit(), local_difference));
        auto const local_perpendicular(
            in_line.direction_.get_unit() * local_cross_position
            - local_difference);
        return this_type(
            local_difference,
            local_perpendicular,
            local_cross_position,
            psyq::geometry::vector::dot(local_perpendicular, local_perpendicular));
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
            this_type::coordinate::make(
                in_point - in_line.origin_.get_position()));
        auto const local_cross_position(
            psyq::geometry::vector::dot(
                in_line.direction_.get_unit(), local_difference));
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
                in_line.direction_.get_unit() * local_cross_position
                - local_difference);
            return this_type(
                local_difference,
                local_perpendicular,
                local_cross_position,
                psyq::geometry::vector::dot(
                    local_perpendicular, local_perpendicular));
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
    /// @brief 直線の始点から点へのベクトル。
    private: typename this_type::coordinate::vector difference_;
    /// @brief 点から直線への垂線。
    private: typename this_type::coordinate::vector perpendicular_;
    /// @brief 直線と、点から直線への垂線が交わる、直線上の位置。
    private: typename this_type::coordinate::element cross_position_;
    /// @brief 直線と点の距離の自乗。
    private: typename this_type::coordinate::element square_distance_;

}; // class psyq::geometry::line::point_collision

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 直線と直線の衝突判定。
    @tparam template_coordinate @copydoc line_collision::coordinate
 */
template<typename template_coordinate>
class psyq::geometry::line<template_coordinate>::line_collision
{
    /// @brief thisが指す値の型。
    private: typedef line_collision this_type;

    /// @brief @copydoc psyq::geometry::line::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief 直線と直線の衝突判定を行う。
        @param[in] in_source  衝突判定の左辺となる直線。
        @param[in] in_target  衝突判定の右辺となる直線。
        @param[in] in_epsilon 誤差の範囲。0以上であること。
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
            psyq::geometry::vector::dot(
                in_source.direction_.get_unit(), in_target.direction_.get_unit()));
        auto const local_denominator(
            1 - local_direction_dot * local_direction_dot);
        if (local_denominator < -in_epsilon || in_epsilon < local_denominator)
        {
            auto const local_source_dot(
                psyq::geometry::vector::dot(
                    local_origin_difference, in_source.direction_.get_unit()));
            auto const local_target_dot(
                psyq::geometry::vector::dot(
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
                psyq::geometry::vector::dot(
                    local_point_difference, local_point_difference));
        }
        else
        {
            // 左辺と右辺の直線が平行だったので、
            // 左辺の直線と右辺の原点の衝突判定を構築する。
            auto const local_cross_position(
                psyq::geometry::vector::dot(
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

#endif // PSYQ_GEOMETRY_LINE_HPP_
// vim: set expandtab:
