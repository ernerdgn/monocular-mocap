#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// using std cpp
#define MSGPACK_NO_BOOST
#include <msgpack.hpp>

namespace msgpack
{
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
{
namespace adaptor
{

//glm::vec2
template<>
struct pack<glm::vec2> {
    template <typename Stream>
    packer<Stream>& operator()(msgpack::packer<Stream>& o, const glm::vec2& v) const
    {
        o.pack_array(2);
        o.pack(v.x);
        o.pack(v.y);
        return o;
    }
};

template<>
struct convert<glm::vec2> {
    msgpack::object const& operator()(msgpack::object const& o, glm::vec2& v) const
    {
        if (o.type != msgpack::type::ARRAY || o.via.array.size != 2)
        {
            throw msgpack::type_error();
        }
        v.x = o.via.array.ptr[0].as<float>();
        v.y = o.via.array.ptr[1].as<float>();
        return o;
    }
};

//glm::vec3
template<>
struct pack<glm::vec3> {
    template <typename Stream>
    packer<Stream>& operator()(msgpack::packer<Stream>& o, const glm::vec3& v) const
    {
        o.pack_array(3);
        o.pack(v.x);
        o.pack(v.y);
        o.pack(v.z);
        return o;
    }
};

template<>
struct convert<glm::vec3> {
    msgpack::object const& operator()(msgpack::object const& o, glm::vec3& v) const
    {
        if (o.type != msgpack::type::ARRAY || o.via.array.size != 3)
        {
            throw msgpack::type_error();
        }
        v.x = o.via.array.ptr[0].as<float>();
        v.y = o.via.array.ptr[1].as<float>();
        v.z = o.via.array.ptr[2].as<float>();
        return o;
    }
};

//glm::quat
//xyzw
template<>
struct pack<glm::quat> {
    template <typename Stream>
    packer<Stream>& operator()(msgpack::packer<Stream>& o, const glm::quat& q) const
    {
        o.pack_array(4);
        o.pack(q.x);
        o.pack(q.y);
        o.pack(q.z);
        o.pack(q.w);
        return o;
    }
};

template<>
struct convert<glm::quat> {
    msgpack::object const& operator()(msgpack::object const& o, glm::quat& q) const
    {
        if (o.type != msgpack::type::ARRAY || o.via.array.size != 4)
        {
            throw msgpack::type_error();
        }
        q.x = o.via.array.ptr[0].as<float>();
        q.y = o.via.array.ptr[1].as<float>();
        q.z = o.via.array.ptr[2].as<float>();
        q.w = o.via.array.ptr[3].as<float>();
        return o;
    }
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack