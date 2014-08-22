/** @file
    @brief モートン順序を用いた空間分割木による衝突判定。

    使い方は、 psyq::mosp_node と psyq::mosp_tree
    の詳細説明を参照すること。

    以下のweb-pageを参考にして実装した。
    http://marupeke296.com/COL_2D_No8_QuadTree.html
 */
#ifndef PSYQ_MOSP_TREE_HPP_
#define PSYQ_MOSP_TREE_HPP_
//#include "psyq/bit_algorithm.hpp"
//#include "psyq/geometric_aabb.hpp"
//#include "psyq/memory_arena.hpp"

/// psyq::mosp_coordinate で使う、defaultのvector型。
#ifndef PSYQ_MOSP_VECTOR_DEFAULT
#include <glm/glm.hpp> // OpenGL Mathematics
#define PSYQ_MOSP_VECTOR_DEFAULT glm::vec3
#endif

#ifndef PSYQ_MOSP_SPACE_DEFAULT
#define PSYQ_MOSP_SPACE_DEFAULT psyq::mosp_space_2d<>
#endif // !defined(PSYQ_MOSP_SPACE_DEFAULT)

#ifndef PSYQ_MOSP_ALLOCATOR_DEFAULT
#define PSYQ_MOSP_ALLOCATOR_DEFAULT\
    psyq::memory_arena::allocator<\
        void*, psyq::memory_arena::fixed_pool<std::allocator<void*>>>
#endif // !defined(PSYQ_MOSP_ALLOCATOR_DEFAULT)

/// @cond
namespace psyq
{
    template<typename, unsigned, unsigned, unsigned> class mosp_coordinate;
    template<typename> class mosp_space;
    template<typename = mosp_coordinate<PSYQ_MOSP_VECTOR_DEFAULT, 0, 2, 1>>
        class mosp_space_2d;
    template<typename = mosp_coordinate<PSYQ_MOSP_VECTOR_DEFAULT, 0, 1, 2>>
        class mosp_space_3d;
    template<typename, typename> class mosp_node;
    template<
        typename,
        typename = PSYQ_MOSP_SPACE_DEFAULT,
        typename = PSYQ_MOSP_ALLOCATOR_DEFAULT>
            class mosp_tree;
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief モートン座標の型特性。 psyq::mosp_space のtemplate引数に使う。
    @tparam template_vector   @copydoc psyq::mosp_coordinate::vector
    @tparam template_element0 @copydoc psyq::mosp_coordinate::ELEMENT0_INDEX
    @tparam template_element1 @copydoc psyq::mosp_coordinate::ELEMENT1_INDEX
    @tparam template_element2 @copydoc psyq::mosp_coordinate::ELEMENT2_INDEX
 */
template<
    typename template_vector,
    unsigned template_element0,
    unsigned template_element1,
    unsigned template_element2>
class psyq::mosp_coordinate
{
    /// thisが指す値の型。
    private: typedef mosp_coordinate this_type;

    /// モートン座標の成分#0のindex番号。
    public: static unsigned const ELEMENT0_INDEX = template_element0;
    /// モートン座標の成分#1のindex番号。
    public: static unsigned const ELEMENT1_INDEX = template_element1;
    /// モートン座標の成分#2のindex番号。
    public: static unsigned const ELEMENT2_INDEX = template_element2;

    /// モートン座標を表す幾何ベクトルの型。
    public: typedef template_vector vector;

    /// モートン座標を表す幾何ベクトルの成分の型。
    public: typedef typename psyq::geometric_vector<template_vector>::element
        element;

    /// モートン座標の最小座標と最大座標を要素とするAABB。
    public: typedef psyq::geometric_aabb<template_vector> aabb;

    /** @brief 絶対座標系からモートン座標系への変換scaleを算出する。
        @param[in] in_morton_size モートン座標の最大値。
        @param[in] in_world_size  絶対座標系でのモートン空間の大きさ。
     */
    public: static typename this_type::element calc_scale(
        typename this_type::element const in_morton_size,
        typename this_type::element const in_world_size)
    {
        if (in_world_size < std::numeric_limits<typename this_type::element>::epsilon())
        {
            return 0;
        }
        return in_morton_size / in_world_size;
    }

    /** @brief 絶対座標系からモートン座標系への変換scaleを算出する。
        @param[in] in_aabb      衝突判定領域の全体を包む、絶対座標系AABB。
        @param[in] in_level_cap 空間分割の最大深度。
     */
    public: static typename this_type::vector calc_scale(
        typename this_type::aabb const& in_aabb,
        unsigned const in_level_cap)
    {
        auto const local_size(in_aabb.get_max() - in_aabb.get_min());
        auto const local_unit(
            static_cast<typename this_type::element>(1 << in_level_cap));
        return psyq::geometric_vector<this_type::vector>::make(
            this_type::calc_scale(
                local_unit, psyq::geometric_vector_element(local_size, 0)),
            this_type::calc_scale(
                local_unit, psyq::geometric_vector_element(local_size, 1)),
            this_type::calc_scale(
                local_unit, psyq::geometric_vector_element(local_size, 2)));
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief psyq::mosp_tree のtemplate引数に使う、モートン空間の基底型。
    @tparam template_coordinate @copydoc mosp_space::coordinate
 */
template<typename template_coordinate>
class psyq::mosp_space
{
    /// thisが指す値の型。
    private: typedef mosp_space this_type;

    /// 衝突判定に使う mosp_coordinate の型。
    public: typedef template_coordinate coordinate;

    /** @brief モートン順序の型。
        @note
            64ビット整数だと、座標の要素の型が32ビット浮動小数点だった場合に、
            精度が足らなくなる。
            separate_bits() も32ビット整数に特化したものしか用意してない。
     */
    public: typedef std::uint32_t order;

    //-------------------------------------------------------------------------
    /** @brief 衝突判定を行う領域を設定する。
        @param[in] in_aabb      衝突判定を行う領域の全体を包む、絶対座標系AABB。
        @param[in] in_level_cap 空間分割の最深レベル。
     */
    protected: mosp_space(
        typename this_type::coordinate::aabb const& in_aabb,
        unsigned const in_level_cap)
    :
        aabb_(in_aabb),
        scale_(this_type::coordinate::calc_scale(in_aabb, in_level_cap))
    {}

    /** @brief 衝突判定を行う領域の全体を包む、絶対座標系AABBを取得する。
        @return 衝突判定を行う領域の全体を包む、絶対座標系AABB。
     */
    public: typename this_type::coordinate::aabb const& get_aabb() const
    {
        return this->aabb_;
    }

    /** @brief 絶対座標系ベクトルの要素を、モートン座標に変換する。
        @return モートン座標でのベクトル要素の値。
        @param[in] in_vector        変換する絶対座標系ベクトル。
        @param[in] in_element_index 変換する要素のインデックス番号。
     */
    protected: typename this_type::coordinate::element transform_element(
        typename this_type::coordinate::vector const& in_vector,
        unsigned const in_element_index)
    const
    {
        auto local_element(
            psyq::geometric_vector_element(in_vector, in_element_index));
        auto const local_min(
            psyq::geometric_vector_element(
                this->aabb_.get_min(), in_element_index));
        if (local_element < local_min)
        {
            return 0;
        }
        auto const local_max(
            psyq::geometric_vector_element(
                this->aabb_.get_max(), in_element_index));
        if (local_max < local_element)
        {
            local_element = local_max;
        }
        return (local_element - local_min) * psyq::geometric_vector_element(
            this->scale_, in_element_index);
    }

    /** @brief モートン空間に収まるように、モートン座標値をclampする。
        @param[in] in_element clampするモートン座標値。
        @param[in] in_max     モートン座標の最大値。
     */
    protected: static typename this_type::order clamp_axis_order(
        typename this_type::coordinate::element const in_element,
        typename this_type::order const in_max)
    {
        if (in_element < 1)
        {
            return 0;
        }
        return (std::min)(
            static_cast<typename this_type::order>(in_element), in_max);
    }

    //-------------------------------------------------------------------------
    /// 衝突判定を行う領域全体の、絶対座標系AABB。
    private: typename this_type::coordinate::aabb aabb_;

    /// 最小となる分割空間の、絶対座標系での大きさの逆数。
    private: typename this_type::coordinate::vector scale_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 2次元座標で衝突判定を行うモートン空間。

    psyq::mosp_tree のtemplate引数に使う。

    @tparam template_coordinate @copydoc psyq::mosp_space::coordinate
 */
template<typename template_coordinate>
class psyq::mosp_space_2d: public psyq::mosp_space<template_coordinate>
{
    /// thisが指す値の型。
    private: typedef mosp_space_2d this_type;
    /// this_type の基底型。
    public: typedef mosp_space<template_coordinate> base_type;

    ///< 衝突判定に使う座標の成分の数。
    public: static unsigned const DIMENSION = 2;

    //-------------------------------------------------------------------------
    /// @copydoc mosp_space::mosp_space
    public: mosp_space_2d(
        typename base_type::coordinate::aabb const& in_aabb,
        unsigned const in_level_cap)
    :
        base_type(in_aabb, in_level_cap)
    {}

    /** @brief 2次元座標上の点から、線形4分木のモートン順序を算出する。
        @return 2次元座標に対応するモートン順序。
        @param[in] in_point 2次元座標上の点。
        @param[in] in_max   モートン座標の最大値。
     */
    public: typename base_type::order calc_order(
        typename base_type::coordinate::vector const& in_point,
        typename base_type::order const in_max)
    const
    {
        auto const local_element0(
            this->transform_element(
                in_point, base_type::coordinate::ELEMENT0_INDEX));
        auto const local_element1(
            this->transform_element(
                in_point, base_type::coordinate::ELEMENT1_INDEX));
        return this_type::separate_bits(local_element0, in_max) << 0
            |  this_type::separate_bits(local_element1, in_max) << 1;
    }

    /** @brief モートン座標を、軸ごとのbitに分割する。
        @param[in] in_element モートン座標の成分値。
        @param[in] in_max     モートン座標の最大値。
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
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 3次元座標で衝突判定を行うmoront空間。

    mosp_tree のtemplate引数に使う。

    @tparam template_coordinate @copydoc psyq::mosp_space::coordinate
 */
template<typename template_coordinate>
class psyq::mosp_space_3d: public psyq::mosp_space<template_coordinate>
{
    /// thisが指す値の型。
    private: typedef mosp_space_3d this_type;
    /// this_type の基底型。
    public: typedef mosp_space<template_coordinate> base_type;

    /// @copydoc mosp_space_2d::DIMENSION
    public: static unsigned const DIMENSION = 3;

    //-------------------------------------------------------------------------
    /// @copydoc mosp_space::mosp_space
    public: mosp_space_3d(
        typename base_type::coordinate::aabb const& in_aabb,
        unsigned const in_level_cap)
    :
        base_type(in_aabb, in_level_cap)
    {}

    /** @brief 3次元座標上の点から、線形8分木のモートン順序を算出する。
        @return 3次元座標に対応するモートン順序。
        @param[in] in_point 3次元座標上の点。
        @param[in] in_max   モートン順序の最大値。
     */
    public: typename base_type::order calc_order(
        typename base_type::coordinate::vector const& in_point,
        typename this_type::order const in_max)
    const
    {
        auto const local_element0(
            this->transform_element(
                in_point, base_type::coordinate::ELEMENT0_INDEX));
        auto const local_element1(
            this->transform_element(
                in_point, base_type::coordinate::ELEMENT1_INDEX));
        auto const local_element2(
            this->transform_element(
                in_point, base_type::coordinate::ELEMENT2_INDEX));
        return this_type::separate_bits(local_element0, in_max) << 0
            |  this_type::separate_bits(local_element1, in_max) << 1
            |  this_type::separate_bits(local_element2, in_max) << 2;
    }

    /// @copydoc mosp_space_2d::separate_bits
    private: static typename base_type::order separate_bits(
        typename base_type::coordinate::element const in_element,
        typename this_type::order const                 in_max)
    {
        auto local_bits(base_type::clamp_axis_order(in_element, in_max));
        local_bits = (local_bits | (local_bits << 8)) & 0x0000f00f;
        local_bits = (local_bits | (local_bits << 4)) & 0x000c30c3;
        local_bits = (local_bits | (local_bits << 2)) & 0x00249249;
        return local_bits;
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief モートン順序による空間分割木に取りつける分割空間ノード。

    使い方の概要。
    -# 衝突判定オブジェクトの識別子を、 mosp_node::argument_ に代入する。
    -# mosp_node::attach_tree() で、 mosp_node を mosp_tree に取りつける。
    -# mosp_tree::detect_collision() で、 mosp_tree に取りつけられている
       mosp_node の衝突判定を行う。ある mosp_node が別の mosp_node に衝突すると、
       ぞれぞれの mosp_node::argument_ を引数に、衝突関数が呼び出される。

    @tparam template_argument     @copydoc mosp_node::argument
    @tparam template_morton_order @copydoc mosp_node::order
 */
template<typename template_argument, typename template_morton_order>
class psyq::mosp_node
{
    /// thisが指す値の型。
    private: typedef mosp_node this_type;

    /// mosp_tree::detect_collision() から呼び出される衝突関数に渡す引数の型。
    public: typedef template_argument argument;
    /// @copydoc psyq::mosp_space::order
    public: typedef template_morton_order order;

    //-------------------------------------------------------------------------
    /** @brief 分割空間ノードを構築する。
        @param[in] in_argument 衝突コールバック関数に渡す引数。
     */
    public: explicit mosp_node(typename this_type::argument in_argument):
        handle_(nullptr),
        argument_(std::move(in_argument))
    {}

    /// コピー構築子は使用禁止。
    private: mosp_node(this_type const&);

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元インスタンス。
     */
    public: mosp_node(this_type&& io_source):
        handle_(io_source.handle_),
        argument_(std::move(io_source.argument_))
    {
        if (this->handle_->second != nullptr)
        {
            io_source.handle_ = nullptr;
            this->handle_->second = this;
        }
    }

    /// @brief *thisを mosp_tree 空間分割木から取り外す。
    public: ~mosp_node()
    {
        this->detach_tree();
    }

    /// コピー代入演算子は使用禁止。
    private: this_type& operator=(const this_type&);

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元インスタンス。
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->~this_type();
            new(this) this_type(std::move(io_source));
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    /** @brief *thisを mosp_tree 空間分割木に取りつける。

        現在取りつけられている mosp_tree から*thisを切り離し、
        新しい mosp_tree に*thisを取りつける。

        @param[in,out] io_tree *thisを取りつける mosp_tree 。
        @param[in]     in_aabb *thisに対応する衝突領域の、絶対座標系AABB。
        @sa detach_tree() is_attached()
     */
    public: template<typename template_mosp_tree>
    bool attach_tree(
        template_mosp_tree& io_tree,
        typename template_mosp_tree::space::coordinate::aabb const& in_aabb)
    {
        // 新たな分割空間ハンドルを用意する。
        auto const local_handle(io_tree.make_handle(*this, in_aabb));
        if (local_handle == nullptr)
        {
            return false;
        }

        // 古い分割空間ハンドルから新たな分割空間ハンドルへ切り替える。
        if (this->handle_ != nullptr)
        {
            PSYQ_ASSERT(this == this->handle_->second);
            this->handle_->second = nullptr;
        }
        this->handle_ = local_handle;
        return true;
    }

    /** @brief *thisを mosp_tree 空間分割木から取り外す。

        現在取りつけられている mosp_tree から、*thisを切り離す。

        @sa attach_tree() is_attached()
     */
    public: void detach_tree()
    {
        auto const local_handle(this->handle_);
        if (local_handle != nullptr)
        {
            PSYQ_ASSERT(this == local_handle->second);
            local_handle->second = nullptr;
            this->handle_ = nullptr;
        }
    }

    /** @brief *thisが mosp_tree 空間分割木に取りつけられているか判定する。
        @retval true  *thisは mosp_tree に取りつけられている。
        @retval false *thisは mosp_tree に取りつけられていない。
        @sa attach_tree() detach_tree()
     */
    public: bool is_attached() const
    {
        return this->handle_ != nullptr;
    }

    //-------------------------------------------------------------------------
    /// *thisと連結している mosp_tree::node_map の要素。
    private: std::pair<template_morton_order const, this_type*>* handle_;

    /** @brief *thisに対応する、衝突判定オブジェクトの識別子。

        mosp_tree::detect_collision() で他の分割空間ノードと重なったとき、
        この値を引数として衝突関数が呼び出される。
     */
    public: typename this_type::argument argument_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief モートン順序による空間分割木。

    使い方の概要は、以下の通り。
    -# mosp_tree::mosp_tree() を呼び出し、
       衝突判定を行う領域を空間分割木に設定する。
    -# mosp_node::attach_tree() を呼び出し、
       mosp_node を mosp_tree に取りつける。
    -# mosp_tree::detect_collision() で、 mosp_tree に取りつけられている
       mosp_node の衝突判定を行う。ある mosp_node が別の mosp_node に衝突すると、
       ぞれぞれの mosp_node が内包する衝突判定オブジェクト識別子を引数に、
       衝突関数が呼び出される。

    @tparam template_argument  @copydoc mosp_node::argument
    @tparam template_space     @copydoc mosp_tree::space
    @tparam template_allocator @copydoc mosp_tree::allocator_type

    @note mosp_tree::node_map に、任意の辞書コンテナを指定できるようにしたい。
 */
template<
    typename template_argument,
    typename template_space,
    typename template_allocator>
class psyq::mosp_tree
{
    private: typedef mosp_tree this_type; ///< *thisの型。

    /// mosp_tree で使うモートン空間。 morton_space_2d や morton_space_3d を使う。
    public: typedef template_space space;

    /// 使用するメモリ割当子。
    public: typedef template_allocator allocator_type;

    /// mosp_tree に取りつける mosp_node 。
    public: typedef psyq::mosp_node<
        template_argument, typename this_type::space::order>
            node;
    /// @cond
    friend node;
    /// @endcond

    private: struct order_hash
    {
        std::size_t operator()(typename this_type::space::order in_order) const
        {
            return static_cast<std::size_t>(in_order);
        }
    };

    /// 分割空間辞書。
    public: typedef std::unordered_multimap<
        typename this_type::space::order,
        typename this_type::node*,
        typename this_type::order_hash,
        std::equal_to<typename this_type::space::order>,
        template_allocator>
            node_map;

    /// 対応できる空間分割の限界深度。
    public: static unsigned const LEVEL_LIMIT =
        (8 * sizeof(typename this_type::space::order) - 1) / this_type::space::DIMENSION;

    //-------------------------------------------------------------------------
    /** @brief 衝突判定を行う領域を設定する。
        @param[in] in_aabb         衝突判定を行う領域の全体を包む、絶対座標系AABB。
        @param[in] in_bucket_count 分割空間辞書のバケット数の初期値。
        @param[in] in_allocator    メモリ割当子の初期値。
        @param[in] in_level_cap    空間分割の最深レベル。
     */
    public: explicit mosp_tree(
        typename this_type::space::coordinate::aabb const& in_aabb,
        std::size_t const in_bucket_count,
        typename this_type::allocator_type const& in_allocator,
        unsigned const in_level_cap = this_type::LEVEL_LIMIT)
    :
        space_(in_aabb, in_level_cap),
        node_map_(
            in_bucket_count,
            typename this_type::node_map::hasher(),
            typename this_type::node_map::key_equal(),
            in_allocator),
        level_cap_(static_cast<decltype(level_cap_)>(in_level_cap)),
        detect_collision_(false)
    {
        // 限界レベルより深い分割空間は作れない。
        if (this_type::LEVEL_LIMIT < in_level_cap)
        {
            PSYQ_ASSERT(false);
            this->level_cap_ = this_type::LEVEL_LIMIT;
        }
    }

    /// コピー構築子は使用禁止。
    private: mosp_tree(this_type const&);

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元インスタンス。
     */
    public: mosp_tree(this_type&& io_source):
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

    /** @brief 取りつけられてる this_type::node をすべて取り外し、
               空間分割木を破棄する。
     */
    public: ~mosp_tree()
    {
        // 衝突判定中は、破棄できない。
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

    /// copy代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    /** @brief move代入演算子。
        @param[in,out] io_source 移動元となるinstance。
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->~this_type();
            new(this) this_type(std::move(io_source));
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    /** @brief 空間分割木の衝突判定を開始する。
        @retval true  成功。あとで end_detection() を呼び出すこと。
        @retval false 失敗。すでに begin_detection() が呼び出されていた。
     */
    public: bool begin_detection()
    {
        // ひとつのインスタンスで多重に detect_collision() できない。
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

    /** @brief 空間分割木の衝突判定を終了する。

        begin_detection() と対になるように呼び出すこと。
     */
    public: void end_detection()
    {
        PSYQ_ASSERT(this->detect_collision_);
        this->detect_collision_ = false;
    }

    /** @brief 分割空間辞書の要素同士で衝突しているか判定する。

        begin_detection() と end_detection() の間で呼び出すこと。

        @param[in] in_collide_callback
            衝突関数オブジェクト。
            - 2つの this_type::node の分割空間が衝突するとき、呼び出される。
            - 引数として、2つの this_type::node::argument を受け取ること。
            - 戻り値はなくてよい。
        @param[in] in_offset 衝突判定を開始する分割空間辞書の要素の開始オフセット値。
        @param[in] in_step   衝突判定をする分割空間辞書の要素の間隔。
     */
    public: template<typename template_collide_callback>
    void detect_collision(
        template_collide_callback const& in_collide_callback,
        std::size_t const in_offset,
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

    /** @brief *thisに取りつけられたすべての this_type::node の衝突判定を行う。

        - this_type::node::attach_tree() によってthisに取りつけられた
          this_type::node のうち、モートン空間で重なる2つの this_type::node
          が持つそれぞれの this_type::node::argument
          を引数に、衝突コールバック関数を呼び出す。
        - スレッドなどで衝突判定処理を分割したい場合は、この関数ではなく
          this_type::detect_collision() を使う。

        @retval ture 成功。衝突判定を行った。
        @retval false
            失敗。衝突判定を行わなかった。
            原因は、すでに衝突判定を行なってる最中だから。

        @param[in] in_collide_callback
            衝突関数オブジェクト。
            - 2つの this_type::node の分割空間が衝突するとき、呼び出される。
            - 引数として、2つの this_type::node::argument を受け取ること。
            - 戻り値はなくてよい。
     */
    public: template<typename template_collide_callback>
    bool detect_collision_batch(
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

    /** @brief 分割空間ノードに分割空間辞書を衝突させる。

        begin_detection() と end_detection() の間で呼び出すこと。

        @param[in] in_collide_callback
            衝突関数オブジェクト。
            - 2つの this_type::node の分割空間が衝突するとき、呼び出される。
            - 引数として、2つの this_type::node::argument を受け取ること。
            - 戻り値はなくてよい。
        @param[in] in_node_handle
            衝突させる分割空間辞書の反復子。
            in_node_map が持つ値を指していること。
        @param[in] in_node_map 衝突させる分割空間辞書。
     */
    private: template<typename template_collide_callback>
    static void detect_collision_map(
        template_collide_callback const& in_collide_callback,
        typename this_type::node_map::const_iterator const& in_node_handle,
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

    /** @brief 分割空間ノードに分割空間コンテナを衝突させる。
        @param[in] in_collide_callback
            衝突関数オブジェクト。
            - 2つの this_type::node の分割空間が衝突するとき、呼び出される。
            - 引数として、2つの this_type::node::argument を受け取ること。
            - 戻り値はなくてよい。
        @param[in] in_node_handle     衝突させる分割空間ノードのハンドル。
        @param[in] in_container_begin 衝突させる分割空間コンテナの先頭位置。
        @param[in] in_container_end   衝突させる分割空間コンテナの終端位置。
     */
    private: template<typename template_collide_callback>
    static void detect_collision_container(
        template_collide_callback const& in_collide_callback,
        typename this_type::node_map::value_type const& in_node_handle,
        typename this_type::node_map::const_iterator const& in_container_begin,
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
    /** @brief AABBを包む最小の分割空間のハンドルを構築する。
        @retval !=nullptr AABBを包む最小の分割空間のハンドル。
        @retval ==nullptr 失敗。
        @param[in] in_node 空間分割木に取りつける分割空間ノード。
        @param[in] in_aabb 衝突判定オブジェクトの絶対座標系AABB。
     */
    private: typename this_type::node_map::value_type* make_handle(
        typename this_type::node& in_node,
        typename this_type::space::coordinate::aabb const& in_aabb)
    {
        if (this->detect_collision_)
        {
            // detect_collision() の実行中は分割空間ハンドルを作れない。
            PSYQ_ASSERT(false);
            return nullptr;
        }

        // モートン順序に対応する分割空間のハンドルを用意する。
        auto const local_morton_order(
            this_type::calc_order(this->level_cap_, this->space_, in_aabb));
        auto const local_map_iterator(
            this->node_map_.emplace(local_morton_order, &in_node));
        return &(*local_map_iterator);
    }

    /** @brief AABBを包む最小の分割空間のモートン順序を算出する。
        @return AABBを包む最小の分割空間のモートン順序。
        @param[in] in_level_cap 空間分割の最深レベル。
        @param[in] in_space     使用するモートン空間。
        @param[in] in_aabb      絶対座標系AABB。
     */
    private: static typename this_type::space::order calc_order(
        unsigned const in_level_cap,
        typename this_type::space const& in_space,
        typename this_type::space::coordinate::aabb const& in_aabb)
    {
        if (in_level_cap <= 0)
        {
            return 0;
        }

        // 衝突物体のAABBを包む、最小の分割空間のモートン順序を算出する。
        auto const local_axis_order_max(
            psyq::bitwise_shift_left_fast<unsigned>(1, in_level_cap) - 1);
        auto const local_min_morton(
            in_space.calc_order(in_aabb.get_min(), local_axis_order_max));
        auto const local_max_morton(
            in_space.calc_order(in_aabb.get_max(), local_axis_order_max));
        auto const local_morton_distance(local_max_morton ^ local_min_morton);
        unsigned local_level;
        if (local_morton_distance != 0)
        {
            auto const local_clz(
                psyq::count_leading_0bits(local_morton_distance));
            local_level = (sizeof(local_morton_distance) * 8 + 1 - local_clz)
                / this_type::space::DIMENSION;
        }
        else
        {
            local_level = 1;
        }
        auto const local_partition_count(
            psyq::bitwise_shift_left_fast<unsigned>(
                1, (in_level_cap - local_level) * this_type::space::DIMENSION));
        auto const local_order(
            (local_partition_count - 1) / ((1 << this_type::space::DIMENSION) - 1));
        return local_order + psyq::bitwise_shift_right_fast(
            local_max_morton, local_level * this_type::space::DIMENSION);
    }

    public: typename this_type::node_map const& get_node_map() const
    {
        return this->node_map_;
    }

    //-------------------------------------------------------------------------
    /// @copydoc space
    private: typename this_type::space space_;
    /// @copydoc node_map
    private: typename this_type::node_map node_map_;
    /// 空間分割の最大深度。
    private: std::uint8_t level_cap_;
    /// detect_collision() を実行中かどうか。
    private: bool detect_collision_;
};

#endif // !defined(PSYQ_MOSP_TREE_HPP_)
