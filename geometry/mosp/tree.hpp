/// @file
/// @brief モートン順序による空間分割木を使った衝突判定の実装。
/// @details
///   以下のウェブページを参考にして実装した。
///   http://marupeke296.com/COL_2D_No8_QuadTree.html
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_GEOMETRY_MOSP_TREE_HPP_
#define PSYQ_GEOMETRY_MOSP_TREE_HPP_

#include <unordered_map>
#include "../../memory_arena.hpp"
#include "../../hash/primitive_bits.hpp"
#include "./node.hpp"

/// @brief psyq::geometry::mosp::tree で使う、デフォルトのメモリ割当子の型。
#ifndef PSYQ_GEOMETRY_MOSP_TREE_ALLOCATOR_DEFAULT
#define PSYQ_GEOMETRY_MOSP_TREE_ALLOCATOR_DEFAULT\
    psyq::memory_arena::allocator<\
        void*, psyq::memory_arena::fixed_pool<std::allocator<void*>>>
#endif // !defined(PSYQ_MOSP_ALLOCATOR_DEFAULT)

/// @cond
namespace psyq
{
    namespace geometry
    {
        namespace mosp
        {
            template<typename, typename, typename> class tree;
        } // namespace mosp
    } // namespace geometry
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief モートン順序による空間分割木。
/// @par 使い方の概要
/// -# psyq::geometry::mosp::tree インスタンスを構築し、
///    衝突判定を行う領域を空間分割木に設定する。
/// -# psyq::geometry::mosp::tree::node::attach_tree を呼び出し、
///    psyq::geometry::mosp::tree::node インスタンスを
///    psyq::geometry::mosp::tree インスタンスへ取りつける。
/// -# psyq::geometry::mosp::tree::detect_collision で、
///    psyq::geometry::mosp::tree インスタンスに取りつけられているすべての
///    psyq::geometry::mosp::tree::node インスタンスで衝突判定を行う。
///    - 2つの psyq::geometry::mosp::tree::node インスタンスの、
///      それぞれが所属している分割空間が衝突していると、それら2つの
///      psyq::geometry::mosp::tree::node::argument_ を引数に、
///      衝突関数が呼び出される。
/// @tparam template_argument  @copydoc psyq::geometry::mosp::node::argument
/// @tparam template_space     @copydoc tree::space
/// @tparam template_allocator @copydoc tree::allocator_type
/// @ingroup psyq_geometry_mosp
/// @note
///   psyq::geometry::mosp::tree::node_map
///   に、任意の辞書コンテナを指定できるようにしたい。
template<
    typename template_argument,
    typename template_space,
    typename template_allocator = PSYQ_GEOMETRY_MOSP_TREE_ALLOCATOR_DEFAULT>
class psyq::geometry::mosp::tree
{
    /// @copydoc psyq::string::view::this_type
    private: typedef tree this_type;

    //-------------------------------------------------------------------------
    /// @brief 空間分割木で衝突判定を行う空間。
    /// @details
    ///   psyq::geometry::mosp::space_2d か psyq::geometry::mosp::space_3d
    ///   と互換のインターフェイスを持つこと。
    public: typedef template_space space;
    /// @copydoc psyq::geometry::mosp::node::argument
    public: typedef template_argument argument;
    /// @brief 使用するメモリ割当子。
    public: typedef template_allocator allocator_type;
    /// @brief this_type に取りつけるノード。
    public: typedef
        psyq::geometry::mosp::node<
            template_argument, typename this_type::space::order>
        node;
    /// @cond
    friend node;
    /// @endcond
    /// @brief モートン順序をキーとする、分割空間の辞書。
    public: typedef
        std::unordered_multimap<
            typename this_type::space::order,
            typename this_type::node*,
            psyq::hash::primitive_bits<
                typename this_type::space::order, std::size_t>,
            std::equal_to<typename this_type::space::order>,
            template_allocator>
        node_map;
    /// @brief 対応できる空間分割の限界深度。
    public: static std::uint8_t const LEVEL_LIMIT =
        (CHAR_BIT * sizeof(typename this_type::space::order) - 1)
        / this_type::space::DIMENSION;

    //-------------------------------------------------------------------------
    /// @name 構築と解体
    /// @{

    /// @brief 衝突判定を行う領域を設定し、空間分割木を構築する。
    public: tree(
        /// [in] 衝突判定を行う領域の全体を包む、絶対座標系AABB。
        typename this_type::space::coordinate::aabb const& in_aabb,
        /// [in] 分割空間辞書のバケット数の初期値。
        std::size_t const in_bucket_count,
        /// [in] メモリ割当子の初期値。
        typename this_type::allocator_type const& in_allocator,
        /// [in] 空間分割の最深レベル。
        std::uint8_t const in_level_cap = this_type::LEVEL_LIMIT):
    space_(in_aabb, in_level_cap),
    node_map_(
        in_bucket_count,
        typename this_type::node_map::hasher(),
        typename this_type::node_map::key_equal(),
        in_allocator),
    level_cap_(in_level_cap),
    detect_collision_(false)
    {
        // 限界レベルより深い分割空間は作れない。
        if (this_type::LEVEL_LIMIT < in_level_cap)
        {
            PSYQ_ASSERT(false);
            this->level_cap_ = this_type::LEVEL_LIMIT;
        }
    }

    /// @brief 空間分割木をムーブ構築する。
    public: tree(
        /// [in,out] ムーブ元となる空間分割木。
        this_type&& io_source):
    space_(io_source.space_),
    node_map_(std::move(io_source.node_map_)),
    level_cap_(io_source.level_cap_),
    detect_collision_(false)
    {
        io_source.node_map_.clear();
        if (io_source.detect_collision_)
        {
            // 衝突判定中はムーブできない。
            PSYQ_ASSERT(false);
            this->node_map_.swap(io_source.node_map_);
        }
    }

    /// @brief
    ///   *thisに取りつけられてる this_type::node
    ///   をすべて取り外し、空間分割木を解体する。
    public: ~tree()
    {
        // 衝突判定中は、解体できない。
        PSYQ_ASSERT(!this->detect_collision_);

        // 分割空間ノードをすべて切り離す。
        for (auto& local_value: this->node_map_)
        {
            auto const local_node(local_value.second);
            if (local_node != nullptr)
            {
                local_node->detach_tree();
            }
        }
    }

    /// @brief 空間分割木をムーブ代入する。
    public: this_type& operator=(
        /// [in,out] ムーブ元となる空間分割木。
        this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->~this_type();
            new(this) this_type(std::move(io_source));
        }
        return *this;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 衝突判定
    /// @{

    /// @brief
    ///   空間分割木の衝突判定を開始し、
    ///   this_type::detect_collision を使える状態にする。
    /// @retval true  成功。あとで this_type::end_detection を呼び出すこと。
    /// @retval false 失敗。すでに this_type::begin_detection が呼び出されていた。
    public: bool begin_detection()
    {
        // ひとつのインスタンスで多重に detect_collision できない。
        if (this->detect_collision_)
        {
            return false;
        }
        this->detect_collision_ = true;

        // 空になった分割空間を削除する。
        for (auto i(this->node_map_.begin()); i != this->node_map_.end();)
        {
            auto const local_node(i->second);
            if (local_node != nullptr)
            {
                ++i;
            }
            else
            {
                i = this->node_map_.erase(i);
            }
        }
        return true;
    }

    /// @brief
    ///   空間分割木の衝突判定を終了し、
    ///   this_type::detect_collision を使えない状態にする。
    /// @details this_type::begin_detection と対になるように呼び出すこと。
    public: void end_detection()
    {
        PSYQ_ASSERT(this->detect_collision_);
        this->detect_collision_ = false;
    }

    /// @brief 分割空間辞書の要素同士で衝突しているか判定する。
    /// @details
    ///   this_type::begin_detection と this_type::end_detection
    ///   の間で呼び出すこと。
    public: template<typename template_collide_callback>
    void detect_collision(
        /// [in] 衝突関数オブジェクト。
        /// - 2つの this_type::node の分割空間が衝突するとき、呼び出される。
        /// - 引数として、2つの this_type::node::argument を受け取ること。
        /// - 戻り値はなくてよい。
        template_collide_callback const& in_collide_callback,
        /// [in] 衝突判定を開始する分割空間辞書の要素の開始オフセット値。
        std::size_t const in_offset,
        /// [in] 衝突判定をする分割空間辞書の要素の間隔。
        std::size_t const in_step)
    const
    {
        PSYQ_ASSERT(this->detect_collision_);
        PSYQ_ASSERT(0 < in_step);
        PSYQ_ASSERT(in_offset < in_step);
        auto local_count(in_offset);
        auto const local_map_end(this->node_map_.end());
        for (auto i(this->node_map_.begin()); i != local_map_end; ++i)
        {
            if (0 < local_count)
            {
                --local_count;
            }
            else
            {
                this_type::detect_collision_map(
                    in_collide_callback, i, this->node_map_);
                PSYQ_ASSERT(local_map_end == this->node_map_.end());
                local_count = in_step - 1;
            }
        }
    }

    /// @brief *thisに取りつけられたすべての this_type::node の衝突判定を行う。
    /// @details
    ///   - this_type::node::attach_tree によってthisに取りつけられた
    ///     this_type::node のうち、モートン空間で重なる2つの this_type::node
    ///     が持つそれぞれの this_type::node::argument
    ///     を引数に、衝突コールバック関数を呼び出す。
    ///   - スレッドなどで衝突判定処理を分割したい場合は、この関数ではなく
    ///     this_type::detect_collision を使う。
    /// @retval ture 成功。衝突判定を行った。
    /// @retval false
    ///     失敗。衝突判定を行わなかった。
    ///     原因は、すでに衝突判定を行なってる最中だから。
    public: template<typename template_collide_callback>
    bool detect_collision_batch(
        /// [in] 衝突関数オブジェクト。
        /// - 2つの this_type::node の分割空間が衝突するとき、呼び出される。
        /// - 引数として、2つの this_type::node::argument を受け取ること。
        /// - 戻り値はなくてよい。
        template_collide_callback const& in_collide_callback)
    {
        // 衝突判定を開始する。
        if (!this->begin_detection())
        {
            return false;
        }

        // 衝突判定を行う。
        this->detect_collision(in_collide_callback, 0, 1);

        // 衝突判定を終了する。
        this->end_detection();
        return true;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @brief コピー構築子は使用禁止。
    private: tree(this_type const&);
    /// @brief コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    //-------------------------------------------------------------------------
    /// @brief 分割空間ノードに分割空間辞書を衝突させる。
    /// @details
    ///   this_type::begin_detection と this_type::end_detection
    ///   の間で呼び出すこと。
    private: template<typename template_collide_callback>
    static void detect_collision_map(
        /// [in] 衝突関数オブジェクト。
        /// - 2つの this_type::node の分割空間が衝突するとき、呼び出される。
        /// - 引数として、2つの this_type::node::argument を受け取ること。
        /// - 戻り値はなくてよい。
        template_collide_callback const& in_collide_callback,
        /// [in] 衝突させる分割空間を指す反復子。
        /// in_node_map が持つ値を指していること。
        typename this_type::node_map::const_iterator const& in_node_handle,
        /// [in] 衝突させる分割空間辞書。
        typename this_type::node_map const& in_node_map)
    {
        // 対象となる分割空間ノードを、同じモートン順序の分割空間ノードに衝突させる。
        auto const local_node_map_end(in_node_map.end());
        PSYQ_ASSERT(in_node_handle != local_node_map_end);
        auto const& local_node_handle(*in_node_handle);
        auto const local_next_handle(std::next(in_node_handle));
        if (local_next_handle != local_node_map_end
            && local_node_handle.first == local_next_handle->first)
        {
            this_type::detect_collision_container(
                in_collide_callback,
                local_node_handle,
                local_next_handle,
                local_node_map_end);
            if (local_node_handle.second == nullptr)
            {
                return;
            }
        }

        // 対象となる分割空間ノードを、上位の分割空間ノードに衝突させる。
        for (
            auto local_super_order(in_node_handle->first);
            0 < local_super_order;)
        {
            // 上位の分割空間ノードを取得する。
            local_super_order =
                (local_super_order - 1) >> this_type::space::DIMENSION;
            auto const local_super_iterator(
                in_node_map.find(local_super_order));
            if (local_super_iterator != local_node_map_end)
            {
                // 上位の分割空間ノードに衝突させる。
                this_type::detect_collision_container(
                    in_collide_callback,
                    local_node_handle,
                    local_super_iterator,
                    local_node_map_end);
                if (local_node_handle.second == nullptr)
                {
                    return;
                }
            }
        }
    }

    /// @brief 分割空間ノードに分割空間コンテナを衝突させる。
    private: template<typename template_collide_callback>
    static void detect_collision_container(
        /// [in] 衝突関数オブジェクト。
        /// - 2つの this_type::node の分割空間が衝突するとき、呼び出される。
        /// - 引数として、2つの this_type::node::argument を受け取ること。
        /// - 戻り値はなくてよい。
        template_collide_callback const& in_collide_callback,
        /// [in] 衝突させる分割空間ノードのハンドル。
        typename this_type::node_map::value_type const& in_node_handle,
        /// [in] 衝突させる分割空間コンテナの先頭位置。
        typename this_type::node_map::const_iterator const& in_container_begin,
        /// [in] 衝突させる分割空間コンテナの終端位置。
        typename this_type::node_map::const_iterator const& in_container_end)
    {
        PSYQ_ASSERT(in_container_begin != in_container_end);
        auto const local_container_order(in_container_begin->first);
        for (
            auto i(in_container_begin);
            i != in_container_end && local_container_order == i->first;
            ++i)
        {
            auto const local_container_node(i->second);
            if (local_container_node != nullptr)
            {
                auto const local_handle_node(in_node_handle.second);
                if (local_handle_node == nullptr)
                {
                    return;
                }

                // 衝突関数を呼び出す。
                in_collide_callback(
                    local_handle_node->argument_,
                    local_container_node->argument_);
            }
        }
    }

    //-------------------------------------------------------------------------
    /// @brief AABBを包む最小の分割空間のハンドルを構築する。
    /// @retval !=nullptr AABBを包む最小の分割空間のハンドル。
    /// @retval ==nullptr 失敗。
    private: typename this_type::node_map::value_type* make_handle(
        /// [in] 空間分割木に取りつける分割空間ノード。
        typename this_type::node& in_node,
        /// [in] 衝突判定オブジェクトの絶対座標系AABB。
        typename this_type::space::coordinate::aabb const& in_aabb)
    {
        if (this->detect_collision_)
        {
            // detect_collision の実行中は分割空間ハンドルを作れない。
            PSYQ_ASSERT(false);
            return nullptr;
        }

        // モートン順序に対応する分割空間のハンドルを用意する。
        return &*this->node_map_.emplace(
            this_type::compute_order(this->level_cap_, this->space_, in_aabb),
            &in_node);
    }

    /// @brief AABBを包む最小の分割空間のモートン順序を算出する。
    /// @return AABBを包む最小の分割空間のモートン順序。
    private: static typename this_type::space::order compute_order(
        /// [in] 空間分割の最深レベル。
        unsigned const in_level_cap,
        /// [in] 使用するモートン空間。
        typename this_type::space const& in_space,
        /// [in] 絶対座標系AABB。
        typename this_type::space::coordinate::aabb const& in_aabb)
    {
        if (in_level_cap <= 0)
        {
            return 0;
        }

        // 衝突物体のAABBを包む、最小の分割空間のモートン順序を算出する。
        auto const local_order_max(
            psyq::shift_left_bitwise_fast<unsigned>(1, in_level_cap) - 1);
        auto const local_morton_max(
            in_space.compute_order(in_aabb.get_max(), local_order_max));
        auto const local_level(
            this_type::compute_level(
                in_space.compute_order(in_aabb.get_min(), local_order_max),
                local_morton_max));
        auto const local_partition_count(
            psyq::shift_left_bitwise_fast<unsigned>(
                1, (in_level_cap - local_level) * this_type::space::DIMENSION));
        enum: unsigned {PARTITION_BASE = (1 << this_type::space::DIMENSION) - 1};
        return (local_partition_count - 1) / PARTITION_BASE
            + psyq::shift_right_bitwise_fast(
                local_morton_max, local_level * this_type::space::DIMENSION);
    }

    private: static unsigned compute_level(
        typename this_type::space::order const in_min,
        typename this_type::space::order const in_max)
    {
        enum: unsigned {BIT_SIZE = sizeof(in_min) * CHAR_BIT - 1};
        return in_min != in_max?
            1 + (BIT_SIZE - psyq::count_leading_0bits(in_min ^ in_max))
                / this_type::space::DIMENSION:
            0;
    }

    //-------------------------------------------------------------------------
    /// @copydoc this_type::space
    private: typename this_type::space space_;
    /// @copydoc this_type::node_map
    private: typename this_type::node_map node_map_;
    /// @brief 空間分割の最大深度。
    private: std::uint8_t level_cap_;
    /// @brief this_type::detect_collision を実行中かどうか。
    private: bool detect_collision_;

}; // class psyq::geometry::mosp::tree

#endif // !defined(PSYQ_GEOMETRY_MOSP_TREE_HPP_)
// vim: set expandtab:
