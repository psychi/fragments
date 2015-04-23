/** @file
    @brief モートン空間分割木に取りつける、衝突判定オブジェクトの実装。
    @author Hillco Psychi (https://twitter.com/psychi)
    @ingroup psyq_geometry_mosp
 */
#ifndef PSYQ_GEOMETRY_MOSP_NUT_HPP_
#define PSYQ_GEOMETRY_MOSP_NUT_HPP_

#include <bitset>
//#include "../make_aabb.hpp"
//#include "./node.hpp"

namespace psyq
{
namespace geometry
{
/** @brief モートン順序を用いた空間分割木による衝突判定の実装。

    ### 使い方の概要
    -# 使用する幾何ベクトル型でテンプレート特殊化した
       psyq::geometry::vector::traits の実装を用意する。
    -# psyq::geometry::mosp::nut インスタンスを用意する。
       - psyq::geometry::mosp::nut::space には、
         モートン順序による空間分割木を使った衝突判定を行う空間を適用する。
         - 空間分割木が2次元（4分木）なら、
           psyq::geometry::mosp::space_2d を適用する。
         - 空間分割木が3次元（8分木）なら、
           psyq::geometry::mosp::space_3d を適用する。
       - psyq::geometry::mosp::nut は抽象型なので、実際には
         psyq::geometry::mosp::nut::ball などの
         具象型のインスタンスを用意することになる。
    -# psyq::geometry::mosp::tree インスタンスを用意する。
       - psyq::geometry::mosp::tree::argument には、
         psyq::geometry::mosp::nut* を適用する。
       - psyq::geometry::mosp::tree::space には、
         psyq::geometry::mosp::nut::space を適用する。
       - psyq::geometry::mosp::tree::allocator_type には、
         std::allocator 互換の任意のメモリ割当子を適用する。
         - 高速なメモリ管理を求めるので、
           psyq::memory_arena::fixed_pool を適用した
           psyq::memory_arena::allocator を推奨する。
    -# psyq::geometry::mosp::nut::attach_tree で、
       psyq::geometry::mosp::nut インスタンス を
       psyq::geometry::mosp::tree インスタンスに取りつける。
    -# psyq::geometry::mosp::tree::detect_collision で、
       psyq::geometry::mosp::tree インスタンスに取りつけられているすべての
       psyq::geometry::mosp::nut インスタンスで衝突判定を行う。
       - 2つの psyq::geometry::mosp::nut インスタンスの、
         それぞれが所属している分割空間が衝突していると、それら2つの
         psyq::geometry::mosp::nut* を引数に、衝突関数が呼び出される。

    @ingroup psyq_geometry_mosp
 */
namespace mosp
{
    /// @cond
    template<typename, typename> class nut;
    /// @endcond
} // namespace mosp
} // namespace geometry
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 衝突判定ナット。
           モートン空間分割木に取りつける、衝突判定形状の基底型。
    @copydetails psyq::geometry::mosp

    @tparam template_space     @copydoc nut::space
    @tparam template_allocator @copydoc nut::allocator_type
    @ingroup psyq_geometry_mosp
 */
template<typename template_space, typename template_allocator>
class psyq::geometry::mosp::nut
{
    /// @brief thisが指す値の型。
    private: typedef nut this_type;

    //-------------------------------------------------------------------------
    /** @brief this_type で使うモートン空間の型。

        psyq::geometry::mosp::space_2d か psyq::geometry::mosp::space_3d
        と互換性のあるインターフェイスを持っている必要がある。
     */
    public: typedef template_space space;

    /// @brief コンテナに用いるメモリ割当子の型。
    public: typedef template_allocator allocator_type;

    /// @brief モートン空間分割木に取りつけるノードの型。
    public: typedef psyq::geometry::mosp::node<
        this_type*, typename this_type::space::order>
            node;

    /// @brief 衝突判定ナットが所属する衝突判定の位相のフラグ。
    public: typedef std::bitset<32> topology;

    public: class cluster;
    public: template<typename template_shape> class concrete;

    /// @brief 衝突判定ナットが所属するクラスタのコンテナ。
    private: typedef std::vector<
        typename this_type::cluster const*, template_allocator>
            cluster_container;

    //-------------------------------------------------------------------------
    /// @name スマートポインタ
    //@{
    /// @brief this_type の所有権ありスマートポインタ。
    public: typedef std::shared_ptr<this_type> shared_ptr;

    /// @brief this_type の所有権なしスマートポインタ。
    public: typedef std::weak_ptr<this_type> weak_ptr;

    /// @brief this_type の所有権なしスマートポインタのコンテナ。
    public: typedef std::vector<
        typename this_type::weak_ptr, typename this_type::allocator_type>
            weak_ptr_vector;
    //@}
    //-------------------------------------------------------------------------
    /// @name 衝突判定形状
    //@{
    /// @brief モートン空間分割木に取付可能な、半直線の衝突判定ナット。
    public: typedef concrete<
        psyq::geometry::ray<typename this_type::space::coordinate>>
            ray;
    /// @brief モートン空間分割木に取付可能な、線分の衝突判定ナット。
    public: typedef concrete<
        psyq::geometry::line_segment<typename this_type::space::coordinate>>
            line_segment;
    /// @brief モートン空間分割木に取付可能な、球の衝突判定ナット。
    public: typedef concrete<
        psyq::geometry::ball<typename this_type::space::coordinate>>
            ball;
    /// @brief モートン空間分割木に取付可能な、直方体の衝突判定ナット。
    public: typedef concrete<
        psyq::geometry::box<typename this_type::space::coordinate>>
            box;
    //@}
    //-------------------------------------------------------------------------
    /** @brief 衝突判定ナットを構築する。
        @param[in] in_cluster_capacity 衝突判定クラスタのコンテナの予約容量。
     */
    protected: explicit nut(std::size_t const in_cluster_capacity):
    aabb_(
        typename this_type::space::coordinate::aabb(
            this_type::space::coordinate::make_filled(0),
            this_type::space::coordinate::make_filled(0))),
    node_(this)
    {
        this->clusters_.reserve(in_cluster_capacity);
    }

    /// @brief *thisをモートン空間分割木から取り外す。
    public: virtual ~nut()
    {
        PSYQ_ASSERT(this->clusters_.empty());
    }

    //-------------------------------------------------------------------------
    /** @brief モートン空間分割木に取りつける。

        現在取りつけられているモートン空間分割木から*thisを取り外し、
        新しいモートン空間分割木に*thisを取りつける。

        @tparam template_tree
            *thisを取りつけるモートン空間分割木の型。
            psyq::geometry::mosp::tree 互換のインターフェイスを持っている必要がある。
        @param[in,out] io_tree *thisを取りつけるモートン空間分割木。
        @sa this_type::detach_tree
     */
    public: template<typename template_tree>
    void attach_tree(template_tree& io_tree)
    {
        // AABBを更新してから取りつける。
        if (!this->get_node().is_attached())
        {
            this->update_aabb();
        }
        this->node_.attach_tree(io_tree, this->get_aabb());
    }

    public: template<typename template_tree>
    static void attach_tree(
        template_tree& io_tree,
        typename this_type::weak_ptr_vector& io_nuts)
    {
        // 衝突判定ナットを空間分割木に取りつける。
        for (auto i(io_nuts.begin()); i != io_nuts.end();)
        {
            auto const local_holder(i->lock());
            auto const local_nut(local_holder.get());
            if (local_nut != nullptr)
            {
                local_nut->attach_tree(io_tree);
                ++i;
            }
            else
            {
                i = io_nuts.erase(i);
            }
        }
    }

    /// @copydoc this_type::node::detach_tree
    public: void detach_tree()
    {
        this->node_.detach_tree();
    }

    /** @brief *thisの this_type::node を取得する。
        @return @copydoc this_type::node_
     */
    public: typename this_type::node const& get_node() const PSYQ_NOEXCEPT
    {
        return this->node_;
    }

    /** @brief *thisのAABBを取得する。
        @return @copydoc this_type::aabb_
     */
    public: typename this_type::space::coordinate::aabb const& get_aabb()
    const PSYQ_NOEXCEPT
    {
        return this->aabb_;
    }

    /** @brief *thisが持つAABBを更新する。
     */
    protected: virtual void update_aabb() = 0;

    /** @brief 衝突判定ナットのAABBが衝突しているか判定する。
        @param[in] in_target 衝突対象となるナット。
        @return
            - this が in_target に衝突していたら、ビット#0が1となる。
            - in_target が this に衝突していたら、ビット#1が1となる。
     */
    protected: unsigned detect_aabb_collision(this_type& in_target)
    {
        this->update_cluster_topology();
        in_target.update_cluster_topology();
        auto const local_topology_0(
            this->target_topology_ & in_target.self_topology_);
        auto const local_topology_1(
            in_target.target_topology_ & this->self_topology_);
        auto const local_collision(
            (static_cast<unsigned>(local_topology_1.any()) * 2)
            | static_cast<unsigned>(local_topology_0.any()));
        typedef typename this_type::space::coordinate::aabb::aabb_collision
            collision;
        return local_collision != 0
            && collision::detect(this->get_aabb(), in_target.get_aabb())?
                local_collision: 0;
    }

    //-------------------------------------------------------------------------
    /// @brief 衝突判定ナットが所属するすべての衝突判定クラスタの位相を合成する。
    private: void update_cluster_topology() PSYQ_NOEXCEPT
    {
        if (this->target_topology_.none())
        {
            this->self_topology_.reset();
            for (auto local_cluster: this->clusters_)
            {
                PSYQ_ASSERT(local_cluster != nullptr);
                this->mix_cluster_topology(*local_cluster);
            }
        }
    }

    /** @brief 衝突判定クラスタの位相を合成する。
        @param[in] in_cluster 位相を合成する衝突判定クラスタ。
     */
    private: void mix_cluster_topology(
        typename this_type::cluster const& in_cluster)
    PSYQ_NOEXCEPT
    {
        this->self_topology_ |= in_cluster.get_self_topology();
        this->target_topology_ |= in_cluster.get_target_topology();
    }

    private: bool add_cluster(typename this_type::cluster const& in_cluster)
    {
        auto const local_end(this->clusters_.cend());
        auto const local_iterator(
            std::find(this->clusters_.cbegin(), local_end, &in_cluster));
        if (local_end != local_iterator)
        {
            return false;
        }
        this->clusters_.push_back(&in_cluster);
        this->mix_cluster_topology(in_cluster);
        return true;
    }

    private: bool remove_cluster(typename this_type::cluster const& in_cluster)
    {
        auto const local_end(this->clusters_.end());
        auto const local_iterator(
            std::find(this->clusters_.begin(), local_end, &in_cluster));
        if (local_iterator == local_end)
        {
            return false;
        }
        *local_iterator = std::move(this->clusters_.back());
        this->clusters_.pop_back();
        this->target_topology_.reset();
        return true;
    }

    //-------------------------------------------------------------------------
    /// @brief 衝突判定形状の絶対座標系AABB。
    protected: typename this_type::space::coordinate::aabb aabb_;
    /// @brief 衝突判定形状に対応する分割空間ノード。
    private: typename this_type::node node_;
    /// @brief *thisが属している衝突判定クラスタのコンテナ。
    private: typename this_type::cluster_container clusters_;
    /// @brief *thisが属する衝突判定の位相。
    private: typename this_type::topology self_topology_;
    /// @brief 衝突対象が属する衝突判定の位相。
    private: typename this_type::topology target_topology_;

}; // class psyq::geometry::mosp::nut

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 衝突判定クラスタ。
           衝突判定ナットの集合を、1つの衝突判定オブジェクトとして扱う。

    @tparam template_space     @copydoc nut::space
    @tparam template_allocator @copydoc nut::allocator_type
 */
template<typename template_space, typename template_allocator>
class psyq::geometry::mosp::nut<template_space, template_allocator>::cluster
{
    /// @brief thisが指す値の型。
    private: typedef cluster this_type;

    //-------------------------------------------------------------------------
    /// @brief 扱う衝突判定ナットの基底型。
    private: typedef
        psyq::geometry::mosp::nut<template_space, template_allocator> nut;

    /// @brief 衝突判定ナットのコンテナ型。
    public: typedef std::vector<typename this_type::nut::shared_ptr>
        nut_container;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 衝突判定クラスタを構築する。
        @param[in] in_nuts            衝突判定クラスタに所属する衝突判定ナットのコンテナ。
        @param[in] in_self_topology   衝突判定クラスタが所属する衝突判定の位相。
        @param[in] in_target_topology 衝突判定クラスタが衝突する衝突判定の位相。
     */
    public: cluster(
        typename this_type::nut_container in_nuts,
        typename this_type::nut::topology const in_self_topology,
        typename this_type::nut::topology const in_target_topology)
    :
    nuts_(std::move(in_nuts)),
    self_topology_(std::move(in_self_topology)),
    target_topology_(std::move(in_target_topology))
    {
        this->initialize_nuts();
    }

    /** @brief コピー構築子。
        @param[in] in_source コピー元となるインスタンス。
     */
    public: cluster(this_type const& in_source):
    nuts_(in_source.nuts_),
    self_topology_(in_source.self_topology_),
    target_topology_(in_source.target_topology_)
    {
        this->initialize_nuts();
    }

    public: ~cluster()
    {
        this->remove_nuts();
    }

    /** @brief コピー代入演算子。
        @param[in] in_source コピー元となるインスタンス。
     */
    public: this_type& operator=(this_type const& in_source)
    {
        if (this != &in_source)
        {
            this->~this_type();
            new(this) this_type(in_source);
        }
        return *this;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 衝突判定の位相
    //@{
    /** @brief クラスタが所属する衝突判定の位相を取得する。
        @return クラスタが所属する衝突判定の位相。
     */
    public: typename this_type::nut::topology const& get_self_topology()
    const PSYQ_NOEXCEPT
    {
        return this->self_topology_;
    }

    /** @brief クラスタが衝突する衝突判定の位相を取得する。
        @return クラスタが衝突する衝突判定の位相。
     */
    public: typename this_type::nut::topology const& get_target_topology()
    const PSYQ_NOEXCEPT
    {
        return this->target_topology_;
    }

    /** @brief 衝突判定の位相を設定する。
        @param[in] in_self_topology   クラスタが所属する衝突判定の位相。
        @param[in] in_target_topology クラスタが衝突する衝突判定の位相。
     */
    public: void set_topology(
        typename this_type::nut::topology const& in_self_topology,
        typename this_type::nut::topology const& in_target_topology)
    PSYQ_NOEXCEPT
    {
        for (auto& local_nut: this->nuts_)
        {
            local_nut->target_topology_.reset();
        }
        this->self_topology_ = in_self_topology;
        this->target_topology_ = in_target_topology;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 衝突判定ナット
    //@{
    /** @brief 衝突判定クラスタに衝突判定ナットを追加する。
        @param[in] in_nut 追加する衝突判定ナットを所有するスマートポインタ。
        @retval true 成功。衝突判定クラスタに衝突判定ナットを追加した。
        @retval false
            失敗。衝突判定ナットのスマートポインタが空か、
            すでに追加されている衝突判定ナットだった。
     */
    public: bool add_nut(typename this_type::nut::shared_ptr in_nut)
    {
        auto const local_nut(in_nut.get());
        if (local_nut == nullptr || !local_nut->add_cluster(*this))
        {
            return false;
        }
        this->nuts_.push_back(std::move(in_nut));
        return true;
    }

    /** @brief 衝突判定クラスタから衝突判定ナットを削除する。
        @param[in] in_nut 削除する衝突判定ナット。
        @return
            削除した衝突判定ナットを所有するスマートポインタ。
            ただし in_nut が衝突判定クラスタにない場合は空となる。
     */
    public: typename this_type::nut::shared_ptr remove_nut(
        typename this_type::nut* const in_nut)
    {
        if (in_nut != nullptr)
        {
            for (auto& local_nut: this->nuts_)
            {
                if (local_nut.get() == in_nut)
                {
                    in_nut->remove_cluster(*this);
                    auto const local_remove_nut(std::move(local_nut));
                    local_nut = std::move(this->nuts_.back());
                    this->nuts_.pop_back();
                    return local_remove_nut;
                }
            }
        }
        return typename this_type::nut::shared_ptr();
    }

    /** @brief 衝突判定クラスタから衝突判定ナットをすべて削除する。
        @return 削除した衝突判定ナット保持子のコンテナ。
     */
    public: typename this_type::nut_container remove_nuts()
    {
        auto const local_nuts(std::move(this->nuts_));
        this->nuts_.clear();
        for (auto& local_nut: local_nuts)
        {
            local_nut->remove_cluster(*this);
        }
        return local_nuts;
    }

    /** @brief 衝突判定クラスタから衝突判定ナットを検索する。
        @param[in] in_nut 検索する衝突判定ナット。
        @return
            発見した衝突判定ナットを所有するスマートポインタ。
            ただし in_nut が衝突判定クラスタにない場合は空となる。
     */
    public: typename this_type::nut::shared_ptr find_nut(
        typename this_type::nut const* const in_nut)
    const
    {
        if (in_nut != nullptr)
        {
            for (auto& local_nut: this->nuts_)
            {
                if (local_nut.get() == in_nut)
                {
                    return local_nut;
                }
            }
        }
        return typename this_type::nut::shared_ptr();
    }
    //@}
    private: void initialize_nuts()
    {
        for (auto i(this->nuts_.begin()); i != this->nuts_.end();)
        {
            auto const local_nut(i->get());
            if (local_nut != nullptr)
            {
                local_nut->add_cluster(*this);
                ++i;
            }
            else
            {
                i = this->nuts_.erase(i);
            }
        }
    }

    //-------------------------------------------------------------------------
    /** @brief 衝突したクラスタを検出する。
        @todo 実装中。
     */
    public: static void update_collision_list(
        typename this_type::nut const& in_nut_0,
        typename this_type::nut const& in_nut_1)
    {
        // AABBが衝突しているか判定する。
        if(in_nut_0.detect_aabb_collision(in_nut_1) != 0)
        {
            // 衝突したクラスタを検知する。
            for (auto local_clustor_0: in_nut_0.clusters_)
            {
                for (auto local_clustor_1: in_nut_1.clusters_)
                {
                    // 異なるクラスタなら衝突する。
                    if (local_clustor_0 != local_clustor_1)
                    {
                        auto const local_topology_0(
                            local_clustor_0->get_target_topology()
                            & local_clustor_1->get_self_topology());
                        if (local_topology_0.any())
                        {
                            // クラスタ#0がクラスタ#1と衝突した。
                        }
                        auto const local_topology_1(
                            local_clustor_1->get_target_topology()
                            & local_clustor_0->get_self_topology());
                        if (local_topology_1.any())
                        {
                            // クラスタ#1がクラスタ#0と衝突した。
                        }
                    }
                }
            }
        }
    }

    //-------------------------------------------------------------------------
    /// @brief 衝突判定クラスタに所属する衝突判定ナットのコンテナ。
    private: typename this_type::nut_container nuts_;
    /// @brief 衝突判定クラスタが所属する衝突判定の位相。
    private: typename this_type::nut::topology self_topology_;
    /// @brief 衝突判定クラスタが衝突する衝突判定の位相。
    private: typename this_type::nut::topology target_topology_;

}; // class psyq::geometry::mosp::nut::cluster

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief  モートン空間分割木に取りつける、衝突判定オブジェクトの具象型。
    @tparam template_space @copydoc psyq::geometry::mosp::nut::space
    @tparam template_shape @copydoc psyq::geometry::mosp::nut::concrete::shape
 */
template<typename template_space, typename template_allocator>
template<typename template_shape>
class psyq::geometry::mosp::nut<template_space, template_allocator>::concrete:
public psyq::geometry::mosp::nut<template_space, template_allocator>
{
    /// @brief thisが指す値の型。
    private: typedef concrete this_type;

    /// @brief this_type の基底型。
    public: typedef
        psyq::geometry::mosp::nut<template_space, template_allocator>
            base_type;

    /// @brief 衝突判定オブジェクトの幾何形状の型。
    public: typedef template_shape shape;

    //-------------------------------------------------------------------------
    /** @brief 衝突判定に使う形状を構築する。
        @param[in] in_shape            衝突判定に使う形状の初期値。
        @param[in] in_cluster_capacity 衝突判定クラスタのコンテナの予約容量。
     */
    public: explicit concrete(
        typename this_type::shape const& in_shape,
        std::size_t const in_cluster_capacity = 0):
    base_type(in_cluster_capacity),
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
        base_type::attach_tree でモートン空間分割木に取りつける。

        @return 衝突判定に使う形状。
     */
    public: typename this_type::shape& get_mutable_shape()
    {
        if (this->get_node().is_attached())
        {
            this->detach_tree();
        }
        return this->shape_;
    }

    /** @brief 衝突判定ナットの形状が衝突しているか判定する。
        @param[in] in_target 衝突対象となるナット。
     */
    public: template<typename template_target_shape>
    unsigned detect_collision(
        typename base_type::concrete<template_target_shape> const& in_target)
    const
    {
        auto const local_collision(this->detect_aabb_collision(in_target));
        if (local_collision != 0
            /// @todo 形状の衝突判定が未実装。
            && true)//collision_detector<>(this->get_shape(), in_target.get_shape())
        {
            return local_collision;
        }
        return false;
    }

    //-------------------------------------------------------------------------
    protected: void update_aabb() override
    {
        this->base_type::aabb_ = psyq::geometry::make_aabb(this->get_shape());
    };

    //-------------------------------------------------------------------------
    /// @brief 衝突判定オブジェクトの形状。
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
        typedef psyq::geometry::mosp::nut<psyq_mosp_space, std::allocator<void*>>
            psyq_mosp_nut;
        typedef psyq::geometry::mosp::tree<psyq_mosp_nut*, template_mosp_space>
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
        auto const local_ball_collision(
            psyq::geometry::point<typename psyq_mosp_space::coordinate>::point_collision::make(
                local_mosp_ball.get_shape().center_.get_position(),
                local_mosp_ball.get_shape().center_.get_position()));
        local_mosp_ball.attach_tree(local_mosp_tree);
        typename psyq_mosp_nut::ray local_mosp_ray(
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
