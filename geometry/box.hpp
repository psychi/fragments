/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copydoc psyq::geometry::box
    @ingroup psyq_geometry psyq::geometry
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

#endif // !defined(PSYQ_GEOMETRY_BOX_HPP_)
