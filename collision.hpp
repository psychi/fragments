#ifndef PSYQ_COLLISION_HPP_
#define PSYQ_COLLISION_HPP_

#include <glm/glm.hpp>

namespace psyq_collision
{
/// 衝突判定に使う半直線
struct half_line
{
    glm::vec3 origin;    ///< 半直線の開始位置
    glm::vec3 direction; ///< 半直線の方向。長さは1。
    float     length;    ///< 半直線の長さ。
};

/// 半直線との衝突判定に使う三角形。
struct triangle
{
    /** @brief 初期化。
        @param[in] in_vertex0 三角形の頂点#0
        @param[in] in_vertex1 三角形の頂点#1
        @param[in] in_vertex2 三角形の頂点#2
        @param[in] in_epsilon 計算機epsilon値。
        @retval true  成功。
        @retval false 失敗。
     */
    bool initialize(
        glm::vec3 const& in_vertex0,
        glm::vec3 const& in_vertex1,
        glm::vec3 const& in_vertex2,
        float const      in_epsilon =
            std::numeric_limits<float>::epsilon())
    {
        glm::vec3 local_edge1(in_vertex1 - in_vertex0);
        glm::vec3 local_edge2(in_vertex2 - in_vertex0);
        this->normal = glm::cross(local_edge1, local_edge2);
        glm::vec3 const local_nx(glm::cross(local_edge2, this->normal));
        glm::vec3 const local_ny(glm::cross(local_edge1, this->normal));
        this->binormal_u = local_nx / glm::dot(local_edge1, local_nx);
        this->binormal_v = local_ny / glm::dot(local_edge2, local_ny);
        return true;
        /*
        glm::vec3 const local_e2(glm::cross(in_vertex0, in_vertex1));
        float const local_d(glm::dot(local_e2, in_vertex2));
        if (local_d < in_epsilon)
        {
            return false;
        }
        this->origin = in_vertex0;
        this->normal = glm::cross(
            in_vertex1 - in_vertex0, in_vertex2 - in_vertex0);
        float const local_invert_d(1 / local_d);
        glm::vec3 const local_e1(glm::cross(in_vertex2, in_vertex0));
        this->binormal_u = local_e1 * local_invert_d;
        this->binormal_v = local_e2 * local_invert_d;
        return true;
         */
    }

    /** @brief 半直線との衝突判定。

        以下の web page を参考にしました。
        http://d.hatena.ne.jp/ototoi/20050320/p1

        衝突した場合、衝突位置は以下の式で算出できる。
        in_half_line.origin + in_half_line.direction * out_intersection.x

        また頂点毎に法線がある場合、以下の式で衝突位置の法線が算出できる。
        normal0 * (1 - out_intersection.y - out_intersection.z) +
        normal1 * out_intersection.y +
        normal2 * out_intersection.z

        @param[out] out_intersection 衝突時の[t,u,v]値。
        @param[in]  in_ray_origin    判定する半直線の始点。
        @param[in]  in_ray_direction 半直線の方向。長さは1。
        @param[in]  in_ray_length    半直線の長さ。
        @param[in]  in_epsilon       計算機epsilon値。
        @retval true  半直線と衝突した。
        @retval false 半直線と衝突しなかった。
     */
    bool intersect(
        glm::vec3&       out_intersection,
        glm::vec3 const& in_ray_origin,
        glm::vec3 const& in_ray_direction,
        float const      in_ray_length,
        float const      in_epsilon = 0)
    const
    {
        float const local_nv(-glm::dot(in_ray_direction, this->normal));
        if (local_nv < in_epsilon)
        {
            return false;
        }

        glm::vec3 const local_origin_diff(in_ray_origin - this->origin);
        float const local_t(
            glm::dot(local_origin_diff, this->normal) / local_nv);
        if (local_t < 0 || in_ray_length < local_t)
        {
            return false;
        }

        glm::vec3 const local_position(
            in_ray_direction * local_t + local_origin_diff);
        float const local_u(glm::dot(local_position, this->binormal_u));
        if (local_u < in_epsilon)
        {
            return false;
        }
        float const local_v(glm::dot(local_position, this->binormal_v));
        if (local_v < in_epsilon || 1 - in_epsilon < local_u + local_v)
        {
            return false;
        }

        out_intersection = glm::vec3(local_t, local_u, local_v);
        return true;
    }

    glm::vec3 origin;     ///< 三角形の開始位置
    glm::vec3 normal;     ///< 三角形の法線
    glm::vec3 binormal_u; ///< 三角形の重心座標U
    glm::vec3 binormal_v; ///< 三角形の重心座標V 
};

} // namespace psyq_collision

#endif // PSYQ_COLLISION_HPP_
