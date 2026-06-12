#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace glm
{

//glm::vec2
inline void to_json(nlohmann::json& j, const vec2& v)
{
    j = nlohmann::json::array({v.x, v.y});
}

inline void from_json(const nlohmann::json& j, vec2& v)
{
    v.x = j.at(0).get<float>();
    v.y = j.at(1).get<float>();
}

//glm::vec3
inline void to_json(nlohmann::json& j, const vec3& v)
{
    j = nlohmann::json::array({v.x, v.y, v.z});
}

inline void from_json(const nlohmann::json& j, vec3& v)
{
    v.x = j.at(0).get<float>();
    v.y = j.at(1).get<float>();
    v.z = j.at(2).get<float>();
}

//glm::quat
inline void to_json(nlohmann::json& j, const quat& q)
{
    j = nlohmann::json::array({q.x, q.y, q.z, q.w});
}

inline void from_json(const nlohmann::json& j, quat& q)
{
    q.x = j.at(0).get<float>();
    q.y = j.at(1).get<float>();
    q.z = j.at(2).get<float>();
    q.w = j.at(3).get<float>();
}

}