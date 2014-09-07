/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief モートン空間分割木に取りつける、衝突判定オブジェクトの実装。
 */
#ifndef PSYQ_GEOMETRY_MOSP_NUT_HPP_
#define PSYQ_GEOMETRY_MOSP_NUT_HPP_

//#include "psyq/geometry/shape.hpp"

namespace psyq
{
namespace geometry
{
/** @brief モートン順序を用いた空間分割木による衝突判定の実装。

    使い方の概要。
    -# psyq::geometry::mosp::nut インスタンスを用意する。
       - psyq::geometry::mosp::nut は抽象型なので、実際には
         psyq::geometry::mosp::nut::ball などの
         具象型のインスタンスを用意することになる。
    -# psyq::mosp_tree インスタンスを用意する。
       - psyq::mosp_tree::argument には、
         psyq::geometry::mosp::nut* を適用する。
       - psyq::mosp_tree::space には、
         psyq::geometry::mosp::nut::space を適用する。
       - psyq::mosp_tree::allocator_type には、
         std::allocator 互換の任意のメモリ割当子を適用する。
         - 高速なメモリ管理を求めるので、
           psyq::memory_arena::fixed_pool を適用した
           psyq::memory_arena::allocator を推奨する。
    -# psyq::geometry::mosp::nut::attach_tree で、
       psyq::geometry::mosp::nut インスタンス を
       psyq::mosp_tree インスタンスに取りつける。
    -# psyq::mosp_tree::detect_collision で、
       psyq::mosp_tree インスタンスに取りつけられているすべての
       psyq::geometry::mosp::nut インスタンスで衝突判定を行う。
       - 2つの psyq::geometry::mosp::nut インスタンスの、
         それぞれが所属している分割空間が衝突していると、それら2つの
         psyq::geometry::mosp::nut* を引数に、衝突関数が呼び出される。
 */
namespace mosp
{
    /// @cond
    template<typename> class nut;
    /// @endcond
} // namespace mosp
} // namespace geometry
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief  モートン空間分割木に取りつける、衝突判定オブジェクトの基底型。
    @tparam template_space @copydoc psyq::geometry::mosp::nut::space
    @ingroup psyq_geometry_mosp
 */
template<typename template_space>
class psyq::geometry::mosp::nut
{
    /// thisが指す値の型。
    private: typedef nut this_type;

    //-------------------------------------------------------------------------
    /** @brief this_type で使うモートン空間の型。

        psyq::geometry::mosp::space_2d か psyq::geometry::mosp::space_3d
        と互換性のあるインターフェイスを持っている必要がある。
     */
    public: typedef template_space space;

    /// モートン空間分割木に取りつけるノードの型。
    public: typedef psyq::mosp_node<
        this_type*, typename this_type::space::order>
            node;

    /// @cond
    public: template<typename template_shape> class concrete;
    /// @endcond
    /// モートン空間分割木に取付可能な、球の衝突判定オブジェクト。
    public: typedef concrete<
        psyq::geometry::ball<typename this_type::space::coordinate>>
            ball;
    /// モートン空間分割木に取付可能な、線分の衝突判定オブジェクト。
    public: typedef concrete<
        psyq::geometry::segment<typename this_type::space::coordinate>>
            segment;
    /// モートン空間分割木に取付可能な、半直線の衝突判定オブジェクト。
    public: typedef concrete<
        psyq::geometry::ray<typename this_type::space::coordinate>>
            ray;
    /// モートン空間分割木に取付可能な、直方体の衝突判定オブジェクト。
    public: typedef concrete<
        psyq::geometry::box<typename this_type::space::coordinate>>
            box;

    //-------------------------------------------------------------------------
    protected: nut():
    node_(this),
    aabb_(
        typename this_type::space::coordinate::aabb(
            this_type::space::coordinate::make_filled(0),
            this_type::space::coordinate::make_filled(0)))
    {}

    /// *thisをモートン空間分割木から取り外す。
    public: virtual ~nut() {}

    //-------------------------------------------------------------------------
    /** @brief モートン空間分割木に取りつける。

        現在取りつけられているモートン空間分割木から*thisを取り外し、
        新しいモートン空間分割木に*thisを取りつける。

        @tparam template_tree
            *thisを取りつけるモートン空間分割木の型。
            psyq::mosp_tree 互換のインターフェイスを持っている必要がある。
        @param[in,out] io_tree *thisを取りつけるモートン空間分割木。
        @sa detach_tree()
        @sa is_attached()
     */
    public: template<typename template_tree>
    void attach_tree(template_tree& io_tree)
    {
        // AABBを更新してから取りつける。
        if (!this->node_.is_attached())
        {
            this->update_aabb();
        }
        this->node_.attach_tree(io_tree, this->get_aabb());
    }

    /// @copydoc this_type::node::detach_tree()
    public: void detach_tree()
    {
        this->node_.detach_tree();
    }

    /// @copydoc this_type::node::is_attached()
    protected: bool is_attached() const PSYQ_NOEXCEPT
    {
        return this->node_.is_attached();
    }

    /** @brief *thisが持つAABBを取得する。
     */
    public: typename this_type::space::coordinate::aabb const& get_aabb()
    const PSYQ_NOEXCEPT
    {
        return this->aabb_;
    }

    /** @brief *thisが持つAABBを更新する。
     */
    protected: virtual void update_aabb() = 0;

    //-------------------------------------------------------------------------
    /// 衝突判定オブジェクトに対応する分割空間ノード。
    private: typename this_type::node node_;
    /// 衝突判定オブジェクトの絶対座標系AABB。
    protected: typename this_type::space::coordinate::aabb aabb_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief  モートン空間分割木に取りつける、衝突判定オブジェクトの具象型。
    @tparam template_space @copydoc psyq::geometry::mosp::nut::space
    @tparam template_shape @copydoc psyq::geometry::mosp::nut::concrete::shape
 */
template<typename template_space>
template<typename template_shape>
class psyq::geometry::mosp::nut<template_space>::concrete:
public psyq::geometry::mosp::nut<template_space>
{
    /// thisが指す値の型。
    private: typedef concrete this_type;
    /// this_type の基底型。
    public: typedef psyq::geometry::mosp::nut<template_space> base_type;

    /// 衝突判定オブジェクトの幾何形状の型。
    public: typedef template_shape shape;

    //-------------------------------------------------------------------------
    /** @brief 衝突判定に使う形状を構築する。
        @param[in] in_shape 衝突判定に使う形状の初期値。
     */
    public: explicit concrete(typename this_type::shape const& in_shape):
    shape_(in_shape)
    {}

    //-------------------------------------------------------------------------
    /** @brief 衝突判定に使う形状を取得する。
        @return 衝突判定に使う形状。
     */
    public: typename this_type::shape const& get_shape() const PSYQ_NOEXCEPT
    {
        return this->shape_;
    }

    /** @brief モートン空間分割木から取り外し、衝突判定に使う形状を取得する。

        衝突判定に使う幾何形状を更新したい場合は、
        この関数の戻り値が参照する幾何形状を書き換えた後、
        base_type::attach_tree() でモートン空間分割木に取りつける。

        @return 衝突判定に使う形状。
     */
    public: typename this_type::shape& get_mutable_shape()
    {
        if (this->is_attached())
        {
            this->detach_tree();
        }
        return this->shape_;
    }

    //-------------------------------------------------------------------------
    protected: void update_aabb() override
    {
        this->base_type::aabb_ = psyq::geometry::make_aabb(this->get_shape());
    };

    //-------------------------------------------------------------------------
    /// 衝突判定オブジェクトの形状。
    protected: typename this_type::shape shape_;

}; // class psyq::geometry::mosp::nut<template_space>::concrete


//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @cond
namespace psyq
{
    namespace test
    {
        template<typename template_mosp_space>
        void geometry_mosp()
        {
            typedef template_mosp_space psyq_mosp_space;
            typedef psyq::geometry::mosp::nut<psyq_mosp_space> psyq_mosp_nut;
            typedef psyq::mosp_tree<psyq_mosp_nut*, template_mosp_space>
                psyq_mosp_tree;
            typename psyq_mosp_tree::node_map::allocator_type::arena::shared_ptr
                local_mosp_arena(
                    new typename psyq_mosp_tree::node_map::allocator_type::arena(16));
            psyq_mosp_tree local_mosp_tree(
                typename psyq_mosp_space::coordinate::aabb(
                    psyq_mosp_space::coordinate::make_filled(-65536),
                    psyq_mosp_space::coordinate::make_filled( 65536)),
                1024,
                typename psyq_mosp_tree::allocator_type(local_mosp_arena));
            typename psyq_mosp_nut::ball local_mosp_ball(
                psyq_mosp_nut::ball::shape::make(
                    psyq_mosp_space::coordinate::make(2, 3, 4), 1));
            local_mosp_ball.attach_tree(local_mosp_tree);
            typename psyq_mosp_nut::ray local_mosp_ray(
                psyq_mosp_nut::ray::shape::make(
                    psyq_mosp_space::coordinate::make(1, 2, 3),
                    psyq_mosp_space::coordinate::make(4, 5, 6)));
            local_mosp_ray.attach_tree(local_mosp_tree);
            local_mosp_tree.detect_collision_batch(
                [](
                    psyq_mosp_nut* const in_nut_0,
                    psyq_mosp_nut* const in_nut_1)
                {
                    PSYQ_ASSERT(in_nut_0 != nullptr);
                    PSYQ_ASSERT(in_nut_1 != nullptr);
                    PSYQ_ASSERT(in_nut_0 != in_nut_1);

                    // AABBが衝突しているか判定する。
                    bool const local_aabb_collision(
                        in_nut_0->get_aabb().detect_collision(in_nut_1->get_aabb()));
                    if (local_aabb_collision)
                    {
                    }
                });
            local_mosp_ray.attach_tree(local_mosp_tree);
        }
    }
}
/// @endcond

#endif // !defined(PSYQ_GEOMETRY_MOSP_NUT_HPP_)
