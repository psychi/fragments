﻿/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copydoc psyq::geometry::direction
    @defgroup psyq_geometry_shape 幾何形状オブジェクト
    @ingroup psyq_geometry psyq::geometry
 */
#ifndef PSYQ_GEOMETRY_DIRECTION_HPP_
#define PSYQ_GEOMETRY_DIRECTION_HPP_

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename> class direction;
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

#endif // PSYQ_GEOMETRY_DIRECTION_HPP_