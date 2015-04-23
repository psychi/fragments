/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief モートン空間分割木が使う空間の実装。
    @defgroup psyq_geometry_mosp モートン順序による衝突判定
    @ingroup psyq_geometry
 */
#ifndef PSYQ_GEOMETRY_MOSP_SPACE_HPP_
#define PSYQ_GEOMETRY_MOSP_SPACE_HPP_

/// @cond
namespace psyq
{
    namespace geometry
    {
        namespace mosp
        {
            template<typename> class space;
            template<typename, unsigned, unsigned> class space_2d;
            template<typename, unsigned, unsigned, unsigned> class space_3d;
        } // namespace mosp
    } // namespace geometry
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief モートン空間分割木が使う空間の基底型。
    @tparam template_coordinate @copydoc psyq::geometry::mosp::space::coordinate
    @ingroup psyq_geometry_mosp
 */
template<typename template_coordinate>
class psyq::geometry::mosp::space
{
    /// thisが指す値の型。
    private: typedef space this_type;

    /** @brief モートン空間の座標系の型特性。

        psyq::geometry::coordinate_2d か psyq::geometry::coordinate_3d
        と互換性のあるインターフェイスを持っている必要がある。
     */
    public: typedef template_coordinate coordinate;

    /** @brief モートン順序の型。
        @note
            64ビット整数だと、座標の要素の型が32ビット浮動小数点だった場合、
            精度が足りない。 psyq::geometry::mosp::space_2d::separate_bits
            も32ビット整数に特化したものしか実装してない。
     */
    public: typedef std::uint32_t order;

    //-------------------------------------------------------------------------
    /** @brief 衝突判定を行うモートン空間の範囲を定義する。
        @param[in] in_aabb      モートン空間の全体を包む、絶対座標系AABB。
        @param[in] in_level_cap 空間分割の最深レベル。
     */
    protected: space(
        typename this_type::coordinate::aabb const& in_aabb,
        unsigned const in_level_cap)
    :
    aabb_(in_aabb),
    scale_(this_type::compute_mosp_scale(in_aabb, in_level_cap))
    {}

    /** @brief 衝突判定を行う範囲の全体を包む、絶対座標系AABBを取得する。
        @return 衝突判定を行う範囲の全体を包む、絶対座標系AABB。
     */
    public: typename this_type::coordinate::aabb const& get_aabb() const
    {
        return this->aabb_;
    }

    /** @brief 絶対座標系からモートン空間の座標系への変換スケールを取得する。
        @return 絶対座標系からモートン空間の座標系への、全軸の変換スケール。
     */
    public: typename this_type::coordinate::vector const& get_scale() const
    {
        return this->scale_;
    }

    /** @brief 絶対座標系ベクトルの成分を、モートン空間の座標へ変換する。
        @return モートン空間での座標成分の値。
        @param[in] in_vector        変換する絶対座標系ベクトル。
        @param[in] in_element_index 変換する成分のインデックス番号。
     */
    protected: typename this_type::coordinate::element transform_element(
        typename this_type::coordinate::vector const& in_vector,
        unsigned const in_element_index)
    const
    {
        auto local_element(
            psyq::geometry::vector::const_at(in_vector, in_element_index));
        auto const local_min(
            psyq::geometry::vector::const_at(
                this->get_aabb().get_min(), in_element_index));
        if (local_element < local_min)
        {
            return 0;
        }
        auto const local_max(
            psyq::geometry::vector::const_at(
                this->get_aabb().get_max(), in_element_index));
        if (local_max < local_element)
        {
            local_element = local_max;
        }
        return (local_element - local_min) *
            psyq::geometry::vector::const_at(
                this->get_scale(), in_element_index);
    }

    /** @brief モートン空間に収まるように、座標値を丸める。
        @param[in] in_element 丸めるモートン空間の座標成分。
        @param[in] in_max     モートン空間の座標成分の最大値。
     */
    protected: static typename this_type::order clamp_axis_order(
        typename this_type::coordinate::element const in_element,
        typename this_type::order const in_max)
    {
        return in_element < 1?
            0:
            (std::min)(static_cast<typename this_type::order>(in_element), in_max);
    }

    /** @brief 絶対座標系からモートン空間の座標への変換スケールを算出する。
        @param[in] in_aabb      モートン空間の全体を包む、絶対座標系AABB。
        @param[in] in_level_cap 空間分割の最大深度。
     */
    private: static typename this_type::coordinate::vector compute_mosp_scale(
        typename this_type::coordinate::aabb const& in_aabb,
        unsigned const in_level_cap)
    {
        typename this_type::coordinate::element_array local_elements;
        auto const local_size(in_aabb.get_max() - in_aabb.get_min());
        auto const local_unit(
             static_cast<typename this_type::coordinate::element>(
                 1 << in_level_cap));
        for (unsigned i(0); i < this_type::coordinate::DIMENSION; ++i)
        {
            local_elements[i] = this_type::compute_mosp_scale(
                local_unit, psyq::geometry::vector::const_at(local_size, i));
        }
        return this_type::coordinate::make(local_elements);
    }

    /** @brief 絶対座標系からモートン空間の座標系への変換スケールを算出する。
        @param[in] in_morton_size モートン空間の座標の最大値。
        @param[in] in_world_size  絶対座標系でのモートン空間の大きさ。
     */
    private: static typename this_type::coordinate::element compute_mosp_scale(
        typename this_type::coordinate::element const in_morton_size,
        typename this_type::coordinate::element const in_world_size)
    {
        return in_world_size < std::numeric_limits<typename this_type::coordinate::element>::epsilon()?
            0: in_morton_size / in_world_size;
    }

    //-------------------------------------------------------------------------
    /// 衝突判定を行うモートン空間の全体を包む、絶対座標系AABB。
    private: typename this_type::coordinate::aabb aabb_;

    /// 最小となる分割空間の、絶対座標系での大きさの逆数。
    private: typename this_type::coordinate::vector scale_;

}; // class psyq::geometry::mosp::space

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 2次元のモートン空間分割木が使う空間。

    psyq::mosp_tree のテンプレート引数に使う。

    @tparam template_coordinate @copydoc psyq::geometry::mosp::space::coordinate
    @tparam template_element_0  @copydoc psyq::geometry::mosp::space_2d::ELEMENT_INDEX_0
    @tparam template_element_1  @copydoc psyq::geometry::mosp::space_2d::ELEMENT_INDEX_1
    @ingroup psyq_geometry_mosp
 */
template<
    typename template_coordinate,
    unsigned template_element_0,
    unsigned template_element_1>
class psyq::geometry::mosp::space_2d:
public psyq::geometry::mosp::space<template_coordinate>
{
    /// thisが指す値の型。
    private: typedef space_2d this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::mosp::space<template_coordinate> base_type;

    public: enum: unsigned
    {
        /// モートン空間の座標成分#0のインデックス番号。
        ELEMENT_INDEX_0 = template_element_0,
        /// モートン空間の座標成分#1のインデックス番号。
        ELEMENT_INDEX_1 = template_element_1,
        /// モートン空間の座標成分の数。
        DIMENSION = 2,
    };

    //-------------------------------------------------------------------------
    /// @copydoc psyq::geometry::mosp::space::space()
    public: space_2d(
        typename base_type::coordinate::aabb const& in_aabb,
        unsigned const in_level_cap)
    :
    base_type(in_aabb, in_level_cap)
    {}

    /** @brief 絶対座標系の点から、線形4分木のモートン順序を算出する。
        @return 2次元のモートン空間に対応するモートン順序。
        @param[in] in_point 絶対座標系の点。
        @param[in] in_max   モートン空間座標の最大値。
     */
    public: typename base_type::order calc_order(
        typename base_type::coordinate::vector const& in_point,
        typename base_type::order const in_max)
    const
    {
        auto const local_element_0(
            this->transform_element(in_point, this_type::ELEMENT_INDEX_0));
        auto const local_element_1(
            this->transform_element(in_point, this_type::ELEMENT_INDEX_1));
        return (this_type::separate_bits(local_element_0, in_max) << 0)
            |  (this_type::separate_bits(local_element_1, in_max) << 1);
    }

    /** @brief モートン座標の成分を、軸ごとのビットに分割する。
        @param[in] in_element モートン空間の座標の成分値。
        @param[in] in_max     モートン空間の座標の最大値。
        @note 64ビット精度の実装も用意したい。
     */
    private: static typename base_type::order separate_bits(
        typename base_type::coordinate::element const in_element,
        typename base_type::order const in_max)
    {
        auto local_bits(base_type::clamp_axis_order(in_element, in_max));
        local_bits = (local_bits | (local_bits << 8)) & 0x00ff00ff;
        local_bits = (local_bits | (local_bits << 4)) & 0x0f0f0f0f;
        local_bits = (local_bits | (local_bits << 2)) & 0x33333333;
        local_bits = (local_bits | (local_bits << 1)) & 0x55555555;
        return local_bits;
    }

}; // class psyq::geometry::mosp::space_2d

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 3次元のモートン空間分割木が使う空間。

    psyq::mosp_tree のテンプレート引数に使う。

    @tparam template_coordinate @copydoc psyq::geometry::mosp::space::coordinate
    @tparam template_element_0  @copydoc space_3d::ELEMENT_INDEX_0
    @tparam template_element_1  @copydoc space_3d::ELEMENT_INDEX_1
    @tparam template_element_2  @copydoc space_3d::ELEMENT_INDEX_2
    @ingroup psyq_geometry_mosp
 */
template<
    typename template_coordinate,
    unsigned template_element_0,
    unsigned template_element_1,
    unsigned template_element_2>
class psyq::geometry::mosp::space_3d:
public psyq::geometry::mosp::space<template_coordinate>
{
    /// thisが指す値の型。
    private: typedef space_3d this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::mosp::space<template_coordinate> base_type;

    public: enum: unsigned
    {
        /// @copydoc psyq::geometry::mosp::space_2d::ELEMENT_INDEX_0
        ELEMENT_INDEX_0 = template_element_0,
        /// @copydoc psyq::geometry::mosp::space_2d::ELEMENT_INDEX_1
        ELEMENT_INDEX_1 = template_element_1,
        /// モートン空間の座標成分#2のインデックス番号。
        ELEMENT_INDEX_2 = template_element_2,
        /// @copydoc psyq::geometry::mosp::space_2d::DIMENSION
        DIMENSION = 3,
    };

    //-------------------------------------------------------------------------
    /// @copydoc psyq::geometry::mosp::space::space()
    public: space_3d(
        typename base_type::coordinate::aabb const& in_aabb,
        unsigned const in_level_cap)
    :
    base_type(in_aabb, in_level_cap)
    {}

    /** @brief 絶対座標系の点から、線形8分木のモートン順序を算出する。
        @return 3次元のモートン空間に対応するモートン順序。
        @param[in] in_point 絶対座標系の点。
        @param[in] in_max   モートン空間座標の最大値。
     */
    public: typename base_type::order calc_order(
        typename base_type::coordinate::vector const& in_point,
        typename this_type::order const in_max)
    const
    {
        auto const local_element_0(
            this->transform_element(in_point, this_type::ELEMENT_INDEX_0));
        auto const local_element_1(
            this->transform_element(in_point, this_type::ELEMENT_INDEX_1));
        auto const local_element_2(
            this->transform_element(in_point, this_type::ELEMENT_INDEX_2));
        return this_type::separate_bits(local_element_0, in_max) << 0
            |  this_type::separate_bits(local_element_1, in_max) << 1
            |  this_type::separate_bits(local_element_2, in_max) << 2;
    }

    /// @copydoc psyq::geometry::mosp::space_2d::separate_bits
    private: static typename base_type::order separate_bits(
        typename base_type::coordinate::element const in_element,
        typename this_type::order const in_max)
    {
        auto local_bits(base_type::clamp_axis_order(in_element, in_max));
        local_bits = (local_bits | (local_bits << 8)) & 0x0000f00f;
        local_bits = (local_bits | (local_bits << 4)) & 0x000c30c3;
        local_bits = (local_bits | (local_bits << 2)) & 0x00249249;
        return local_bits;
    }

}; // class psyq::geometry::mosp::space_3d

#endif // !defined(PSYQ_GEOMETRY_MOSP_SPACE_HPP_)
// vim: set expandtab:
