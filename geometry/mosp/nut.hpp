/// @file
/// @brief モートン空間分割木に取りつける、衝突判定オブジェクトの実装。
/// @author Hillco Psychi (https://twitter.com/psychi)
/// @ingroup psyq_geometry_mosp
#ifndef PSYQ_GEOMETRY_MOSP_NUT_HPP_
#define PSYQ_GEOMETRY_MOSP_NUT_HPP_

#include <bitset>
#include "../make_aabb.hpp"
#include "./tree.hpp"

namespace psyq
{
namespace geometry
{
/// @brief モートン順序を用いた空間分割木による衝突判定の実装。
/// @par 使い方の概要
/// -# 使用する幾何ベクトル型でテンプレート特殊化した
///    psyq::geometry::vector::traits の実装を用意する。
/// -# psyq::geometry::mosp::nut インスタンスを用意する。
///    - psyq::geometry::mosp::nut::space には、
///      モートン順序による空間分割木を使った衝突判定を行う空間を適用する。
///      - 空間分割木が2次元（4分木）なら、
///        psyq::geometry::mosp::space_2d を適用する。
///      - 空間分割木が3次元（8分木）なら、
///        psyq::geometry::mosp::space_3d を適用する。
///    - psyq::geometry::mosp::nut は抽象型なので、実際には
///      psyq::geometry::mosp::nut::ball などの
///      具象型のインスタンスを用意することになる。
/// -# psyq::geometry::mosp::tree インスタンスを用意する。
///    - psyq::geometry::mosp::tree::argument には、
///      psyq::geometry::mosp::nut* を適用する。
///    - psyq::geometry::mosp::tree::space には、
///      psyq::geometry::mosp::nut::space を適用する。
///    - psyq::geometry::mosp::tree::allocator_type には、
///      std::allocator 互換の任意のメモリ割当子を適用する。
///      - 高速なメモリ管理を求めるので、
///        psyq::memory_arena::fixed_pool を適用した
///        psyq::memory_arena::allocator を推奨する。
/// -# psyq::geometry::mosp::nut::attach_tree で、
///    psyq::geometry::mosp::nut インスタンス を
///    psyq::geometry::mosp::tree インスタンスに取りつける。
/// -# psyq::geometry::mosp::tree::detect_collision で、
///    psyq::geometry::mosp::tree インスタンスに取りつけられているすべての
///    psyq::geometry::mosp::nut インスタンスで衝突判定を行う。
///    - 2つの psyq::geometry::mosp::nut インスタンスの、
///      それぞれが所属している分割空間が衝突していると、それら2つの
///      psyq::geometry::mosp::nut* を引数に、衝突関数が呼び出される。
/// @ingroup psyq_geometry_mosp
namespace mosp
{
    /// @cond
    template<typename> class nut;
    /// @endcond
} // namespace mosp
} // namespace geometry
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 衝突判定ナット。モートン空間分割木に取りつける、衝突判定形状の基底型。
/// @copydetails psyq::geometry::mosp
/// @tparam template_space @copydoc nut::space
/// @ingroup psyq_geometry_mosp
template<typename template_space>
class psyq::geometry::mosp::nut
{
    /// @copydoc psyq::string::view::this_type
    private: typedef nut this_type;

    //-------------------------------------------------------------------------
    public: template<typename template_allocator> class tree;
    /// @brief this_type で使うモートン空間の型。
    /// @details
    ///   psyq::geometry::mosp::space_2d か psyq::geometry::mosp::space_3d
    ///   と互換性のあるインターフェイスを持っている必要がある。
    public: typedef template_space space;
    /// @brief モートン空間分割木に取りつけるノードの型。
    public: typedef
        psyq::geometry::mosp::node<
            this_type*, typename this_type::space::order>
        node;
    /// @brief 衝突判定ナットが所属する衝突判定のグループを表す型。
    public: typedef void const* group;
    /// @brief 衝突判定ナットが所属する衝突判定の位相を表す型。
    public: typedef std::bitset<32> topology;
    /// @brief this_type を強参照するスマートポインタ。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// @brief this_type を弱参照するスマートポインタ。
    public: typedef std::weak_ptr<this_type> weak_ptr;

    //-------------------------------------------------------------------------
    /// @name 衝突判定形状
    /// @{
    public: template<typename template_shape> class concrete;
    /// @brief モートン空間分割木に取付可能な、半直線の衝突判定ナット。
    public: typedef
        concrete<psyq::geometry::ray<typename this_type::space::coordinate>>
        ray;
    /// @brief モートン空間分割木に取付可能な、線分の衝突判定ナット。
    public: typedef
        concrete<
            psyq::geometry::line_segment<
                typename this_type::space::coordinate>>
        line_segment;
    /// @brief モートン空間分割木に取付可能な、球の衝突判定ナット。
    public: typedef
        concrete<psyq::geometry::ball<typename this_type::space::coordinate>>
        ball;
    /// @brief モートン空間分割木に取付可能な、直方体の衝突判定ナット。
    public: typedef
        concrete<psyq::geometry::box<typename this_type::space::coordinate>>
        box;
    /// @}
    //-------------------------------------------------------------------------
    /// @brief 衝突判定ナットを構築する。
    protected: explicit nut(
        /// [in] 所属する衝突判定グループ。
        typename this_type::group const in_group,
        /// [in] 所属する衝突判定位相。
        typename this_type::topology const& in_topology,
        /// [in] 衝突する衝突判定位相。
        typename this_type::topology const& in_target_topology):
    aabb_(
        typename this_type::space::coordinate::aabb(
            this_type::space::coordinate::make_filled(0),
            this_type::space::coordinate::make_filled(0))),
    node_(this),
    group_(in_group),
    topology_(in_topology),
    target_topology_(in_target_topology)
    {}

    //-------------------------------------------------------------------------
    /// @name 空間分割木との連結
    /// @{

    /// @brief モートン空間分割木に取りつける。
    /// @details
    ///   現在取りつけられているモートン空間分割木から*thisを取り外し、
    ///   新しいモートン空間分割木に*thisを取りつける。
    /// @tparam template_tree
    ///   *thisを取りつけるモートン空間分割木の型。
    ///   psyq::geometry::mosp::tree 互換のインターフェイスを持っている必要がある。
    /// @sa this_type::detach_tree
    public: template<typename template_tree>
    void attach_tree(
        /// [in,out] *thisを取りつけるモートン空間分割木。
        template_tree& io_tree)
    {
        // AABBを更新してから取りつける。
        if (!this->get_node().is_attached())
        {
            this->update_aabb();
        }
        this->node_.attach_tree(io_tree, this->get_aabb());
    }

    /// @copydoc this_type::node::detach_tree
    public: void detach_tree()
    {
        this->node_.detach_tree();
    }

    /// @brief *thisの this_type::node を取得する。
    /// @return @copydoc this_type::node_
    public: typename this_type::node const& get_node() const PSYQ_NOEXCEPT
    {
        return this->node_;
    }

    /// @brief *thisのAABBを取得する。
    /// @return @copydoc this_type::aabb_
    public: typename this_type::space::coordinate::aabb const& get_aabb()
    const PSYQ_NOEXCEPT
    {
        return this->aabb_;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 衝突判定の位相
    /// @{

    /// @brief 所属する衝突判定グループを取得する。
    /// @return 所属する衝突判定グループの識別値。
    public: typename this_type::group get_group() const PSYQ_NOEXCEPT
    {
        return this->group_;
    }

    /// @brief 所属する衝突判定の位相を取得する。
    /// @return 所属する衝突判定の位相。
    public: typename this_type::topology const& get_topology()
    const PSYQ_NOEXCEPT
    {
        return this->topology_;
    }

    /// @brief 衝突する衝突判定の位相を取得する。
    /// @return 衝突する衝突判定の位相。
    public: typename this_type::topology const& get_target_topology()
    const PSYQ_NOEXCEPT
    {
        return this->target_topology_;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @brief *thisが持つAABBを更新する。
    protected: virtual void update_aabb() = 0;

    //-------------------------------------------------------------------------
    private: nut(this_type const&);
    private: this_type& operator=(this_type const&);

    //-------------------------------------------------------------------------
    /// @brief 衝突判定形状の絶対座標系AABB。
    protected: typename this_type::space::coordinate::aabb aabb_;
    /// @brief 衝突判定形状に対応する分割空間ノード。
    private: typename this_type::node node_;
    /// @brief 衝突判定グループの識別値。同じグループとは衝突しない。
    private: typename this_type::group group_;
    /// @brief 所属する衝突判定の位相。
    private: typename this_type::topology topology_;
    /// @brief 衝突する衝突判定の位相。
    private: typename this_type::topology target_topology_;

}; // class psyq::geometry::mosp::nut

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 衝突判定ナットを取りつける空間分割木。
/// @tparam template_space     @copydoc nut::space
/// @tparam template_allocator @copydoc tree::allocator_type
/// @note 実装途中
template<typename template_space>
template<typename template_allocator>
class psyq::geometry::mosp::nut<template_space>::tree:
public psyq::geometry::mosp::tree<
    psyq::geometry::mosp::nut<template_space>*,
    template_space,
    template_allocator>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef tree this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::geometry::mosp::tree<
            psyq::geometry::mosp::nut<template_space>*,
            template_space,
            template_allocator>
        base_type;

    //-------------------------------------------------------------------------
    /// @brief 扱う衝突判定ナットの基底型。
    private: typedef psyq::geometry::mosp::nut<template_space> nut;
    /// @brief 衝突判定ナットの所有権なしスマートポインタのコンテナ。
    private: typedef
        std::vector<typename this_type::nut::weak_ptr, template_allocator>
        nut_vector;

    //-------------------------------------------------------------------------
    /// @brief 空間分割木に衝突判定ナットを取りつける。
    /// @retval true  成功。空間分割木に衝突判定ナットを登録した。
    /// @retval false 失敗。
    public: bool register_nut(
        /// [in] 登録する衝突判定ナットを指すスマートポインタ。
        typename this_type::nut::shared_ptr const& in_nut)
    {
        auto const local_nut(in_nut.get());
        if (local_nut == nullptr ||
            this->find_nut_index(*local_nut) < this->nuts_.size())
        {
            return false;
        }
        this->nuts_.push_back(in_nut);
        return true;
    }

    /// @brief 空間分割木から衝突判定ナットを取りはずす。
    /// @retval true  成功。空間分割木から衝突判定ナットを取り外した。
    /// @retval false 失敗。
    public: bool unregister_nut(
        /// [in] 取り外す衝突判定ナットを指すポインタ。
        typename this_type::nut const* const in_nut)
    {
        if (in_nut != nullptr)
        {
            auto const local_nut_index(this->find_nut_index(*in_nut));
            if (local_nut_index < this->nuts_.size())
            {
                this->nuts_.at(local_nut_index).reset();
                return true;
            }
        }
        return false;
    }

    //-------------------------------------------------------------------------
    public: bool begin_detection(
        typename this_type::nut::topology const& in_target_topology)
    {
        for (auto i(this->nuts_.begin()); i != this->nuts_.end();)
        {
            auto const local_nut_holder(i->lock());
            auto const local_nut(local_nut_holder.get());
            if (local_nut != nullptr)
            {
                // 衝突判定ナットが衝突位相と重なる場合にのみ、
                // 衝突判定ナットを空間分割木へ取りつける。
                if ((in_target_topology & local_nut->get_topology()).any())
                {
                    local_nut->attach_tree(*this);
                }
                else
                {
                    local_nut->detach();
                }
                ++i;
            }
            else
            {
                i = this->nuts_.erase(i);
            }
        }
        return this->base_type::begin_detection();
    }

    /// @brief 分割空間辞書の要素同士で衝突しているか判定する。
    /// @details this_type::begin_detection と base_type::end_detection の間で呼び出すこと。
    /// @param[in] in_offset 衝突判定を開始する分割空間辞書の要素の開始オフセット値。
    /// @param[in] in_step   衝突判定をする分割空間辞書の要素の間隔。
    public: void detect_collision(
        std::size_t const in_offset,
        std::size_t const in_step)
    const
    {
        this->base_type::detect_collision(
            [&](typename base_type::node::argument const in_argument_0,
                typename base_type::node::argument const in_argument_1)
            {
                this_type::detect_aabb_collision(*in_argument_0, *in_argument_1);
            },
            in_offset,
            in_step);
    }

    public: bool detect_collision_batch(
        typename this_type::nut::topology const& in_target_topology)
    {
        // 衝突判定を開始する。
        if (!this->begin_detection(in_target_topology))
        {
            return false;
        }

        // 衝突判定を行う。
        this->detect_collision(0, 1);

        // 衝突判定を終了する。
        this->end_detection();
        return true;
    }

    //-------------------------------------------------------------------------
    private: std::size_t find_nut_index(typename this_type::nut const& in_nut)
    const PSYQ_NOEXCEPT
    {
        for (std::size_t i(0); i < this->nuts_.size(); ++i)
        {
            if (this->nuts_.at(i).lock().get() == &in_nut)
            {
                return i;
            }
        }
        return this->nuts_.size();
    };

    /// @brief 衝突判定ナットのAABBが衝突しているか判定する。
    /// @return
    ///   - in_nut_0 が in_nut_1 に衝突していたら、ビット#0が1となる。
    ///   - in_nut_1 が in_nut_0 に衝突していたら、ビット#1が1となる。
    private: static void detect_aabb_collision(
        /// [in] 衝突判定ナット#0。
        typename this_type::nut const& in_nut_0,
        /// [in] 衝突判定ナット#1。
        typename this_type::nut const& in_nut_1)
    {
        if (in_nut_0.get_group() != in_nut_1.get_group())
        {
            auto const local_collision_0(
                (in_nut_0.get_target_topology() & in_nut_1.get_topology()).any());
            auto const local_collision_1(
                (in_nut_1.get_target_topology() & in_nut_0.get_topology()).any());
            if (local_collision_0 | local_collision_1)
            {
                typedef
                    typename this_type::space::coordinate::aabb::aabb_collision
                    collision;
                if (collision::detect(in_nut_0.get_aabb(), in_nut_1.get_aabb()))
                {
                    if (local_collision_0)
                    {
                        /// @todo 衝突コンテナに追加する。
                    }
                    if (local_collision_1)
                    {
                        /// @todo 衝突コンテナに追加する。
                    }
                }
            }
        }
    }

    //-------------------------------------------------------------------------
    private: typename this_type::nut_vector nuts_;

}; // class psyq::geometry::mosp::nut::tree

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief  モートン空間分割木に取りつける、衝突判定オブジェクトの具象型。
/// @tparam template_space @copydoc nut::space
/// @tparam template_shape @copydoc nut::concrete::shape
template<typename template_space>
template<typename template_shape>
class psyq::geometry::mosp::nut<template_space>::concrete:
public psyq::geometry::mosp::nut<template_space>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef concrete this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef psyq::geometry::mosp::nut<template_space> base_type;

    //-------------------------------------------------------------------------
    /// @brief 衝突判定オブジェクトの幾何形状の型。
    public: typedef template_shape shape;

    //-------------------------------------------------------------------------
    /// @brief 衝突判定に使う形状を構築する。
    public: concrete(
        /// [in] 所属する衝突判定位相。
        typename base_type::topology const& in_topology,
        /// [in] 衝突する衝突判定位相。
        typename base_type::topology const& in_target_topology,
        /// [in] 衝突判定に使う形状の初期値。
        typename this_type::shape const& in_shape):
    base_type(this, in_topology, in_target_topology),
    shape_(in_shape)
    {}

    /// @brief 衝突判定に使う形状を構築する。
    public: concrete(
        /// [in] 所属する衝突判定グループ。
        typename base_type::group const in_group,
        /// [in] 所属する衝突判定位相。
        typename base_type::topology const& in_topology,
        /// [in] 衝突する衝突判定位相。
        typename base_type::topology const& in_target_topology,
        /// [in] 衝突判定に使う形状の初期値。
        typename this_type::shape const& in_shape):
    base_type(in_group, in_topology, in_target_topology),
    shape_(in_shape)
    {}

    //-------------------------------------------------------------------------
    /// @brief 衝突判定に使う形状を取得する。
    /// @return 衝突判定に使う形状。
    public: typename this_type::shape const& get_shape() const PSYQ_NOEXCEPT
    {
        return this->shape_;
    }

    /// @brief モートン空間分割木から取り外し、衝突判定に使う形状を取得する。
    /// @details
    ///   衝突判定に使う幾何形状を更新したい場合は、
    ///   この関数の戻り値が参照する幾何形状を書き換えた後、
    ///   base_type::attach_tree でモートン空間分割木に取りつける。
    /// @return 衝突判定に使う形状。
    public: typename this_type::shape& fetch_shape()
    {
        this->detach_tree();
        return this->shape_;
    }

    //-------------------------------------------------------------------------
    protected: void update_aabb() override
    {
        this->base_type::aabb_ = psyq::geometry::make_aabb(this->get_shape());
    };

    //-------------------------------------------------------------------------
    /// @brief 衝突判定の形状。
    protected: typename this_type::shape shape_;

}; // class psyq::geometry::mosp::nut::concrete

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @cond
namespace psyq_test
{
    template<typename template_mosp_space>
    void geometry_mosp()
    {
        typedef template_mosp_space psyq_mosp_space;
        typedef psyq::geometry::mosp::nut<psyq_mosp_space> psyq_mosp_nut;
        typedef
            psyq::geometry::mosp::tree<psyq_mosp_nut*, template_mosp_space>
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
        enum: unsigned
        {
            TOPOLOGY_ENABLE  = 1 << 0,
            TOPOLOGY_PLAYER_COLLISION = 1 << 1,
        };
        typename psyq_mosp_nut::ball local_mosp_ball(
            TOPOLOGY_ENABLE | TOPOLOGY_PLAYER_COLLISION,
            0,
            psyq_mosp_nut::ball::shape::make(
                psyq_mosp_space::coordinate::make(2, 3, 4), 5));
        auto const local_ball_collision(
            psyq::geometry::point<typename psyq_mosp_space::coordinate>::point_collision::make(
                local_mosp_ball.get_shape().center_.get_position(),
                local_mosp_ball.get_shape().center_.get_position()));
        local_mosp_ball.attach_tree(local_mosp_tree);
        typename psyq_mosp_nut::ray local_mosp_ray(
            TOPOLOGY_ENABLE,
            TOPOLOGY_PLAYER_COLLISION,
            psyq_mosp_nut::ray::shape(
                psyq_mosp_nut::ray::shape::point::make(
                    psyq_mosp_space::coordinate::make(1, 2, 3)),
                psyq_mosp_nut::ray::shape::direction::make(
                    psyq_mosp_space::coordinate::make(4, 5, 6))));
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
                    psyq::geometry::aabb<typename psyq_mosp_space::coordinate>::
                        aabb_collision::detect(
                            in_nut_0->get_aabb(), in_nut_1->get_aabb()));
                if (local_aabb_collision)
                {
                }
            });
        local_mosp_ray.attach_tree(local_mosp_tree);
    }
}
/// @endcond

#endif // !defined(PSYQ_GEOMETRY_MOSP_NUT_HPP_)
// vim: set expandtab:
