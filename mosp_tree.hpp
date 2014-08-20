/** @file
    @brief モートン順序を用いた空間分割木による衝突判定。

    使い方は、 psyq::mosp_cell と psyq::mosp_tree
    の詳細説明を参照すること。

    以下のweb-pageを参考にして実装した。
    http://marupeke296.com/COL_2D_No8_QuadTree.html
 */
#ifndef PSYQ_MOSP_TREE_HPP_
#define PSYQ_MOSP_TREE_HPP_
//#include "psyq/bit_algorithm.hpp"
//#include "psyq/geometric_aabb.hpp"
//#include "psyq/memory_arena.hpp"

/// psyq::mosp_coordinates で使う、defaultのvector型。
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
    template<typename, unsigned, unsigned, unsigned> class mosp_coordinates;
    template<typename> class mosp_space;
    template<typename = mosp_coordinates<PSYQ_MOSP_VECTOR_DEFAULT, 0, 2, 1>>
        class mosp_space_2d;
    template<typename = mosp_coordinates<PSYQ_MOSP_VECTOR_DEFAULT, 0, 1, 2>>
        class mosp_space_3d;
    template<typename, typename> class mosp_cell;
    template<typename = PSYQ_MOSP_SPACE_DEFAULT> class mosp_node;
    template<
        typename = mosp_node<PSYQ_MOSP_SPACE_DEFAULT>*,
        typename = PSYQ_MOSP_SPACE_DEFAULT,
        typename = PSYQ_MOSP_ALLOCATOR_DEFAULT>
            class mosp_tree;
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief モートン座標の型特性。 psyq::mosp_space のtemplate引数に使う。
    @tparam template_vector   @copydoc psyq::mosp_coordinates::vector
    @tparam template_element0 @copydoc psyq::mosp_coordinates::ELEMENT0_INDEX
    @tparam template_element1 @copydoc psyq::mosp_coordinates::ELEMENT1_INDEX
    @tparam template_element2 @copydoc psyq::mosp_coordinates::ELEMENT2_INDEX
 */
template<
    typename template_vector,
    unsigned template_element0,
    unsigned template_element1,
    unsigned template_element2>
class psyq::mosp_coordinates
{
    /// thisが指す値の型。
    private: typedef mosp_coordinates this_type;

    public: enum: unsigned
    {
        /// モートン座標の成分#0のindex番号。
        ELEMENT0_INDEX = template_element0,
        /// モートン座標の成分#1のindex番号。
        ELEMENT1_INDEX = template_element1,
        /// モートン座標の成分#2のindex番号。
        ELEMENT2_INDEX = template_element2,
    };

    /// モートン座標を表すvectorの型。
    public: typedef template_vector vector;

    /// モートン座標を表すvectorの成分の型。
    public: typedef typename psyq::geometric_vector<template_vector>::element
        element;

    /// モートン座標のAABB。
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
    @tparam template_coordinates @copydoc psyq::mosp_space::coordinates
 */
template<typename template_coordinates>
class psyq::mosp_space
{
    /// thisが指す値の型。
    private: typedef mosp_space this_type;

    /// 衝突判定に使う psyq::mosp_coordinates の型。
    public: typedef template_coordinates coordinates;

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
        typename this_type::coordinates::aabb const& in_aabb,
        unsigned const                               in_level_cap)
    :
        aabb_(in_aabb),
        scale_(this_type::coordinates::calc_scale(in_aabb, in_level_cap))
    {}

    /** @brief 衝突判定を行う領域の全体を包む、絶対座標系AABBを取得する。
        @return 衝突判定を行う領域の全体を包む、絶対座標系AABB。
     */
    public: typename this_type::coordinates::aabb const& get_aabb() const
    {
        return this->aabb_;
    }

    /** @brief 絶対座標系ベクトルの要素を、モートン座標に変換する。
        @param[in] in_vector        変換する絶対座標系ベクトル。
        @param[in] in_element_index 変換する要素のインデックス番号。
        @return モートン座標でのベクトル要素の値。
     */
    protected: typename this_type::coordinates::element transform_element(
        typename this_type::coordinates::vector const& in_vector,
        unsigned const                                 in_element_index)
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
        return (local_element - local_min)
            * psyq::geometric_vector_element(this->scale_, in_element_index);
    }

    /** @brief モートン空間に収まるように、モートン座標値をclampする。
        @param[in] in_element clampするモートン座標値。
        @param[in] in_max     モートン座標の最大値。
     */
    protected: static typename this_type::order clamp_axis_order(
        typename this_type::coordinates::element const in_element,
        typename this_type::order const                in_max)
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
    private: typename this_type::coordinates::aabb aabb_;

    /// 最小となる分割空間の、絶対座標系での大きさの逆数。
    private: typename this_type::coordinates::vector scale_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 2次元座標で衝突判定を行うモートン空間。

    psyq::mosp_tree のtemplate引数に使う。

    @tparam template_coordinates @copydoc psyq::mosp_space::coordinates
 */
template<typename template_coordinates>
class psyq::mosp_space_2d: public psyq::mosp_space<template_coordinates>
{
    /// thisが指す値の型。
    private: typedef mosp_space_2d this_type;
    /// this_type の基底型。
    public: typedef mosp_space<template_coordinates> base_type;

    public: enum: unsigned
    {
        DIMENSION = 2, ///< 衝突判定に使う座標の成分の数。
    };

    //-------------------------------------------------------------------------
    /// @copydoc mosp_space::mosp_space
    public: mosp_space_2d(
        typename base_type::coordinates::aabb const& in_aabb,
        unsigned const in_level_cap)
    :
        base_type(in_aabb, in_level_cap)
    {}

    /** @brief 2次元座標上の点から、線形4分木のモートン順序を算出する。
        @param[in] in_point 2次元座標上の点。
        @param[in] in_max   モートン座標の最大値。
        @return 2次元座標に対応するモートン順序。
     */
    public: typename base_type::order calc_order(
        typename base_type::coordinates::vector const& in_point,
        typename base_type::order const in_max)
    const
    {
        auto const local_element0(
            this->transform_element(
                in_point, base_type::coordinates::ELEMENT0_INDEX));
        auto const local_element1(
            this->transform_element(
                in_point, base_type::coordinates::ELEMENT1_INDEX));
        return this_type::separate_bits(local_element0, in_max) << 0
            |  this_type::separate_bits(local_element1, in_max) << 1;
    }

    /** @brief モートン座標を、軸ごとのbitに分割する。
        @param[in] in_element モートン座標の成分値。
        @param[in] in_max     モートン座標の最大値。
     */
    private: static typename base_type::order separate_bits(
        typename base_type::coordinates::element const in_element,
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

    @tparam template_coordinates @copydoc psyq::mosp_space::coordinates
 */
template<typename template_coordinates>
class psyq::mosp_space_3d: public psyq::mosp_space<template_coordinates>
{
    /// thisが指す値の型。
    private: typedef mosp_space_3d this_type;
    /// this_type の基底型。
    public: typedef mosp_space<template_coordinates> base_type;

    public: enum: unsigned
    {
        DIMENSION = 3, ///< @copydoc mosp_space_2d::DIMENSION
    };

    //-------------------------------------------------------------------------
    /// @copydoc mosp_space::mosp_space
    public: mosp_space_3d(
        typename base_type::coordinates::aabb const& in_aabb,
        unsigned const in_level_cap)
    :
        base_type(in_aabb, in_level_cap)
    {}

    /** @brief 3次元座標上の点から、線形8分木のモートン順序を算出する。
        @param[in] in_point 3次元座標上の点。
        @param[in] in_max   モートン順序の最大値。
        @return 3次元座標に対応するモートン順序。
     */
    public: typename base_type::order calc_order(
        typename base_type::coordinates::vector const& in_point,
        typename this_type::order const in_max)
    const
    {
        auto const local_element0(
            this->transform_element(
                in_point, base_type::coordinates::ELEMENT0_INDEX));
        auto const local_element1(
            this->transform_element(
                in_point, base_type::coordinates::ELEMENT1_INDEX));
        auto const local_element2(
            this->transform_element(
                in_point, base_type::coordinates::ELEMENT2_INDEX));
        return this_type::separate_bits(local_element0, in_max) << 0
            |  this_type::separate_bits(local_element1, in_max) << 1
            |  this_type::separate_bits(local_element2, in_max) << 2;
    }

    /// @copydoc mosp_space_2d::separate_bits
    private: static typename base_type::order separate_bits(
        typename base_type::coordinates::element const in_element,
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
/** @brief モートン順序による空間分割木に取りつける分割空間。

    使い方の概要。
    -# 衝突判定オブジェクトの識別子を、 mosp_cell::nucleus_ に代入する。
    -# mosp_cell::attach_tree() で、 mosp_cell を mosp_tree に取りつける。
    -# mosp_tree::detect_collision() で、 mosp_tree に取りつけられている
       mosp_cell の衝突判定を行う。ある mosp_cell が別の mosp_cell に衝突すると、
       ぞれぞれの mosp_cell が内包する衝突判定オブジェクト識別子を引数に、
       衝突コールバック関数が呼び出される。

    @tparam template_nucleus      @copydoc mosp_cell::nucleus
    @tparam template_morton_order @copydoc mosp_cell::order
 */
template<typename template_nucleus, typename template_morton_order>
class psyq::mosp_cell
{
    /// thisが指す値の型。
    private: typedef mosp_cell this_type;

    /// 分割空間が内包する、衝突判定オブジェクトの識別子。
    public: typedef template_nucleus nucleus;
    /// @copydoc psyq::mosp_space::order
    public: typedef template_morton_order order;

    //-------------------------------------------------------------------------
    /** @brief 衝突判定ハンドルを構築する。
        @param[in] in_object *thisに対応する、衝突判定オブジェクトの識別子。
     */
    public: explicit mosp_cell(typename this_type::nucleus in_object):
        map_link_(nullptr),
        nucleus_(std::move(in_object))
    {}

    /// copy-constructorは使用禁止。
    private: mosp_cell(this_type const&);

    /** @brief move-constructor。
        @param[in,out] io_source 移動元となるinstance。
     */
    public: mosp_cell(this_type&& io_source):
        map_link_(io_source.map_link_),
        nucleus_(std::move(io_source.nucleus_))
    {
        if (io_source.map_link_ != nullptr)
        {
            io_source.map_link_ = nullptr;
            this->map_link_->second = this;
        }
    }

    /// @brief *thisを mosp_tree 空間分割木から取り外す。
    public: ~mosp_cell()
    {
        this->detach_tree();
    }

    /// copy代入演算子は使用禁止。
    private: this_type& operator=(const this_type&);

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
        typename template_mosp_tree::space::coordinates::aabb const& in_aabb)
    {
        // 新たな分割空間を用意する。
        auto const local_link(io_tree.make_cell(in_aabb, this));
        if (local_link == nullptr)
        {
            return false;
        }

        // 古い分割空間から新たな分割空間へ切り替える。
        if (this->map_link_ != nullptr)
        {
            PSYQ_ASSERT(this == this->map_link_->second);
            this->map_link_->second = nullptr;
        }
        this->map_link_ = local_link;
        return true;
    }

    /** @brief *thisを mosp_tree 空間分割木から取り外す。

        現在取りつけられている mosp_tree から、*thisを切り離す。

        @sa attach_tree() is_attached()
     */
    public: void detach_tree()
    {
        if (this->map_link_ != nullptr)
        {
            PSYQ_ASSERT(this == this->map_link_->second);
            this->map_link_->second = nullptr;
            this->map_link_ = nullptr;
        }
    }

    /** @brief *thisが mosp_tree 空間分割木に取りつけられているか判定する。
        @retval true  *thisは mosp_tree に取りつけられている。
        @retval false *thisは mosp_tree に取りつけられていない。
        @sa attach_tree() detach_tree()
     */
    public: bool is_attached() const
    {
        return this->map_link_ != nullptr;
    }

    //-------------------------------------------------------------------------
    /// *thisに対応する分割空間の辞書へのリンク。
    private: std::pair<template_morton_order const, this_type*>* map_link_;

    /** @brief *thisに対応する、衝突判定オブジェクトの識別子。

        mosp_tree::detect_collision() で他の分割空間と重なったとき、
        この値を引数として衝突コールバック関数が呼び出される。
     */
    public: typename this_type::nucleus nucleus_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief モートン順序による空間分割木。

    使い方の概要は、以下の通り。
    -# mosp_tree::mosp_tree() を呼び出し、
       衝突判定を行う領域を空間分割木に設定する。
    -# mosp_cell::attach_tree() を呼び出し、
       mosp_cell を mosp_tree に取りつける。
    -# mosp_tree::detect_collision() で、 mosp_tree に取りつけられている
       mosp_cell の衝突判定を行う。ある mosp_cell が別の mosp_cell に衝突すると、
       ぞれぞれの mosp_cell が内包する衝突判定オブジェクト識別子を引数に、
       衝突コールバック関数が呼び出される。

    @tparam template_nucleus   @copydoc mosp_cell::nucleus
    @tparam template_space     @copydoc mosp_tree::space
    @tparam template_allocator @copydoc mosp_tree::allocator_type

    @note mosp_tree::cell_map に、任意の辞書コンテナを指定できるようにしたい。
 */
template<
    typename template_nucleus,
    typename template_space,
    typename template_allocator>
class psyq::mosp_tree
{
    private: typedef mosp_tree this_type; ///< *thisの型。

    /// mosp_tree で使うモートン空間。 morton_space_2d や morton_space_3d を使う。
    public: typedef template_space space;

    /// 使用するメモリ割当子。
    public: typedef template_allocator allocator_type;

    /// mosp_tree で使うAABBの型。
    public: typedef typename this_type::space::coordinates::aabb aabb;

    /// mosp_tree で使う幾何ベクトルの型。
    public: typedef typename this_type::space::coordinates::vector vector;

    /// 空間分割木に取りつける mosp_cell 。
    public: typedef psyq::mosp_cell<
        template_nucleus, typename template_space::order>
            cell;
    /// @cond
    friend cell;
    /// @endcond

    /// @copydoc psyq::mosp_node
    public: typedef psyq::mosp_node<typename this_type::space> node;

    private: struct order_hash
    {
        std::size_t operator()(typename this_type::space::order in_order) const
        {
            return in_order;
        }
    };

    /// 分割空間の辞書。
    public: typedef std::unordered_multimap<
        typename this_type::space::order,
        typename this_type::cell*,
        typename this_type::order_hash,
        std::equal_to<typename this_type::space::order>,
        template_allocator>
            cell_map;

    public: enum: unsigned
    {
        /// 対応できる空間分割の限界深度。
        LEVEL_LIMIT = (8 * sizeof(typename this_type::space::order) - 1)
            / this_type::space::DIMENSION
    };

    //-------------------------------------------------------------------------
    /** @brief 衝突判定を行う領域を設定する。
        @param[in] in_aabb         衝突判定を行う領域の全体を包む、絶対座標系AABB。
        @param[in] in_bucket_count 分割空間の辞書のバケット数の初期値。
        @param[in] in_allocator    メモリ割当子の初期値。
        @param[in] in_level_cap    空間分割の最深レベル。
     */
    public: explicit mosp_tree(
        typename this_type::space::coordinates::aabb const& in_aabb,
        std::size_t const in_bucket_count,
        typename this_type::allocator_type const& in_allocator,
        unsigned const in_level_cap = this_type::LEVEL_LIMIT)
    :
        space_(in_aabb, in_level_cap),
        cell_map_(
            in_bucket_count,
            typename this_type::cell_map::hasher(),
            typename this_type::cell_map::key_equal(),
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

    /// copy-constructorは使用禁止。
    private: mosp_tree(this_type const&);

    /** @brief move-constructor。
        @param[in,out] io_source 移動元となるinstance。
     */
    public: mosp_tree(this_type&& io_source):
        space_(io_source.space_),
        cell_map_(std::move(io_source.cell_map_)),
        level_cap_(io_source.level_cap_),
        detect_collision_(false)
    {
        io_source.cell_map_.clear();
        if (io_source.detect_collision_)
        {
            // 衝突判定中はmoveできない。
            PSYQ_ASSERT(false);
            this->cell_map_.swap(io_source.cell_map_);
        }
    }

    /** @brief 取りつけられてる this_type::cell をすべて取り外し、
               空間分割木を破棄する。
     */
    public: ~mosp_tree()
    {
        // 衝突判定中は、破棄できない。
        PSYQ_ASSERT(!this->detect_collision_);

        // ハンドルをすべて切り離す。
        for (auto& local_value: this->cell_map_)
        {
            auto const local_cell(local_value.second);
            if (local_cell != nullptr)
            {
                local_cell->detach_tree();
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
    /** @brief thisに取りつけられたすべての this_type::cell の衝突判定を行う。

        - this_type::cell::attach_tree() によってthisに取りつけられた
          this_type::cell のうち、モートン空間で重なる2つの this_type::cell
          が持つそれぞれの this_type::cell::nucleus
          を引数に、衝突コールバック関数を呼び出す。
        - スレッドなどで衝突判定処理を分割したい場合は、この関数ではなく
          this_type::detect_collision() を使う。

        @param[in] in_collide_callback
            衝突コールバック関数オブジェクト。
            - 2つの this_type::cell がモートン空間で衝突した時、呼び出される。
            - 引数として、2つの this_type::cell::nucleus を受け取ること。
            - 戻り値はなくてよい。

        @retval ture 成功。衝突判定を行った。
        @retval false
            失敗。衝突判定を行わなかった。
            原因は、すでに衝突判定を行なってる最中だから。
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

    //-------------------------------------------------------------------------
    /** @brief 空間分割木の衝突判定を開始する。

        end_detection() と対になるように呼び出すこと。

        @retval true  成功。
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
        for (auto i(this->cell_map_.begin()); i != this->cell_map_.end();)
        {
            auto const local_cell(i->second);
            if (local_cell != nullptr)
            {
                ++i;
            }
            else
            {
                i = this->cell_map_.erase(i);
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

    /** @brief 分割空間の辞書の要素同士で衝突しているか判定する。

        begin_detection() と end_detection() の間で呼び出すこと。

        @param[in] in_collide_callback
            衝突コールバック関数オブジェクト。
            - 2つの this_type::cell がモートン空間で衝突した時、呼び出される。
            - 引数として、2つの this_type::cell::nucleus を受け取ること。
            - 戻り値はなくてよい。
        @param[in] in_offset 衝突判定を開始する辞書要素の開始オフセット値。
        @param[in] in_step   衝突判定をする辞書要素の間隔。
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
        for (auto i(this->cell_map_.begin()); i != this->cell_map_.end(); ++i)
        {
            if (0 < local_count)
            {
                --local_count;
            }
            else
            {
                this_type::detect_collision_map(
                    in_collide_callback, i, this->cell_map_);
                local_count = in_step - 1;
            }
        }
    }

    /** @brief 分割空間に分割空間の辞書を衝突させる。

        begin_detection() と end_detection() の間で呼び出すこと。

        @param[in] in_collide_callback
            衝突コールバック関数オブジェクト。
            - 2つの this_type::cell が衝突した時、呼び出される。
            - 引数として、2つの this_type::cell::nucleus を受け取ること。
            - 戻り値はなくてよい。
        @param[in] in_target_link
            衝突させる分割空間の反復子。
            in_cell_map が持つ値を指していること。
        @param[in] in_cell_map 衝突させる分割空間の辞書。
     */
    private: template<typename template_collide_callback>
    static void detect_collision_map(
        template_collide_callback const& in_collide_callback,
        typename this_type::cell_map::const_iterator const& in_target_link,
        typename this_type::cell_map const& in_cell_map)
    {
        // 対象となる分割空間を、同じモートン順序の分割空間に衝突させる。
        auto const local_cell_map_end(in_cell_map.end());
        PSYQ_ASSERT(in_target_link != local_cell_map_end);
        auto const& local_target_link(*in_target_link);
        auto const local_next_node(std::next(in_target_link));
        if (local_next_node != local_cell_map_end
            && in_target_link->first == local_next_node->first)
        {
            auto const local_target_cell(
                this_type::detect_collision_container(
                    in_collide_callback,
                    local_target_link,
                    local_next_node,
                    local_cell_map_end));
            if (local_target_cell == nullptr)
            {
                return;
            }
        }

        // 対象となる分割空間を、上位の分割空間に衝突させる。
        for (
            auto local_super_order(in_target_link->first);
            0 < local_super_order;)
        {
            // 上位の分割空間を取得する。
            local_super_order =
                (local_super_order - 1) >> this_type::space::DIMENSION;
            auto const local_super_iterator(
                in_cell_map.find(local_super_order));
            if (local_super_iterator != local_cell_map_end)
            {
                // 上位の分割空間に衝突させる。
                auto const local_target_cell(
                    this_type::detect_collision_container(
                        in_collide_callback,
                        local_target_link,
                        local_super_iterator,
                        local_cell_map_end));
                if (local_target_cell == nullptr)
                {
                    return;
                }
            }
        }
    }

    /** @brief 分割空間に分割空間コンテナを衝突させる。
        @param[in] in_collide_callback
            衝突コールバック関数オブジェクト。
            - 2つの this_type::cell が衝突した時、呼び出される。
            - 引数として、2つの this_type::cell::nucleus を受け取ること。
            - 戻り値はなくてよい。
        @param[in] in_target_link     衝突させる分割空間。
        @param[in] in_container_begin 衝突させる分割空間コンテナの先頭位置。
        @param[in] in_container_end   衝突させる分割空間コンテナの終端位置。
     */
    private: template<typename template_collide_callback>
    static typename this_type::cell const* detect_collision_container(
        template_collide_callback const& in_collide_callback,
        typename this_type::cell_map::value_type const& in_target_link,
        typename this_type::cell_map::const_iterator const& in_container_begin,
        typename this_type::cell_map::const_iterator const& in_container_end)
    {
        PSYQ_ASSERT(in_container_begin != in_container_end);
        auto const local_container_order(in_container_begin->first);
        for (
            auto local_container_node(in_container_begin);
            local_container_node != in_container_end
            && local_container_order == local_container_node->first;
            ++local_container_node)
        {
            auto const local_container_cell(local_container_node->second);
            if (local_container_cell != nullptr)
            {
                auto const local_target_cell(in_target_link.second);
                if (local_target_cell == nullptr)
                {
                    return nullptr;
                }

                // 衝突コールバック関数を呼び出す。
                in_collide_callback(
                    local_target_cell->nucleus_,
                    local_container_cell->nucleus_);
            }
        }
        return in_target_link.second;
    }

    //-------------------------------------------------------------------------
    /** @brief AABBを包む最小の分割空間を構築する。
        @param[in] in_aabb 衝突判定ハンドルの絶対座標系AABB。
        @param[in] in_cell 空間分割木に取りつける分割空間。
        @retval !=nullptr AABBを包む最小の分割空間。
        @retval ==nullptr 失敗。
     */
    private: typename this_type::cell_map::value_type* make_cell(
        typename this_type::space::coordinates::aabb const& in_aabb,
        typename this_type::cell* const in_cell)
    {
        if (this->detect_collision_)
        {
            // detect_collision() の実行中は分割空間を作れない。
            PSYQ_ASSERT(false);
            return nullptr;
        }

        // モートン順序に対応する分割空間を用意する。
        auto const local_morton_order(
            this_type::calc_order(this->level_cap_, this->space_, in_aabb));
        auto const local_map_iterator(
            this->cell_map_.emplace(local_morton_order, in_cell));
        return &(*local_map_iterator);
    }

    /** @brief AABBを包む最小の分割空間のモートン順序を算出する。
        @param[in] in_level_cap 空間分割の最深レベル。
        @param[in] in_space     使用するモートン空間。
        @param[in] in_aabb      絶対座標系AABB。
        @return AABBを包む最小の分割空間のモートン順序。
     */
    private: static typename this_type::space::order calc_order(
        unsigned const in_level_cap,
        typename this_type::space const& in_space,
        typename this_type::space::coordinates::aabb const& in_aabb)
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
        auto const local_node_count(
            psyq::bitwise_shift_left_fast<unsigned>(
                1, (in_level_cap - local_level) * this_type::space::DIMENSION));
        auto const local_order(
            (local_node_count - 1) / ((1 << this_type::space::DIMENSION) - 1));
        return local_order + psyq::bitwise_shift_right_fast(
            local_max_morton, local_level * this_type::space::DIMENSION);
    }

    public: typename this_type::cell_map const& get_cell_map()
    const PSYQ_NOEXCEPT
    {
        return this->cell_map_;
    }

    //-------------------------------------------------------------------------
    /// @copydoc space
    private: typename this_type::space space_;
    /// @copydoc cell_map
    private: typename this_type::cell_map cell_map_;
    /// 空間分割の最大深度。
    private: std::uint8_t level_cap_;
    /// detect_collision() を実行中かどうか。
    private: bool detect_collision_;
};

#endif // !defined(PSYQ_MOSP_TREE_HPP_)
