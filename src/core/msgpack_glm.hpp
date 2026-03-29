#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <msgpack.hpp>

namespace msgpack
{
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
{
    namespace adaptor
    {

    // glm::vec3
    template <>
    struct pack<glm::vec3>
    {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, glm::vec3 const& v) const
        {
            o.pack_array(3);
            o.pack(v.x);
            o.pack(v.y);
            o.pack(v.z);
            return o;
        }
    };

    template <>
    struct convert<glm::vec3>
    {
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

    // glm::quat
    template <>
    struct pack<glm::quat>
    {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, glm::quat const& q) const
        {
            o.pack_array(4);
            o.pack(q.w);
            o.pack(q.x);
            o.pack(q.y);
            o.pack(q.z);
            return o;
        }
    };

    template <>
    struct convert<glm::quat>
    {
        msgpack::object const& operator()(msgpack::object const& o, glm::quat& q) const
        {
            if (o.type != msgpack::type::ARRAY || o.via.array.size != 4)
            {
                throw msgpack::type_error();
            }
            q.w = o.via.array.ptr[0].as<float>();
            q.x = o.via.array.ptr[1].as<float>();
            q.y = o.via.array.ptr[2].as<float>();
            q.z = o.via.array.ptr[3].as<float>();
            return o;
        }
    };

    } // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack