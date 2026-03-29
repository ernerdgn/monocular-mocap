#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace glm
{

// glm::vec3
inline void to_json(nlohmann::json& j, const vec3& v)
{
    j = nlohmann::json{v.x, v.y, v.z};
}

inline void from_json(const nlohmann::json& j, vec3& v)
{
    j.at(0).get_to(v.x);
    j.at(1).get_to(v.y);
    j.at(2).get_to(v.z);
}

// glm::quat
inline void to_json(nlohmann::json& j, const quat& q)
{
    j = nlohmann::json{q.w, q.x, q.y, q.z};
}

inline void from_json(const nlohmann::json& j, quat& q)
{
    j.at(0).get_to(q.w);
    j.at(1).get_to(q.x);
    j.at(2).get_to(q.y);
    j.at(3).get_to(q.z);
}

// glm::mat4
inline void to_json(nlohmann::json& j, const mat4& m)
{
    j = nlohmann::json::array();
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            j.push_back(m[col][row]);
        }
    }
}

inline void from_json(const nlohmann::json& j, mat4& m)
{
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            j.at(col * 4 + row).get_to(m[col][row]);
        }
    }
}

} // namespace glm