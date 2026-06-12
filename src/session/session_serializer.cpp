#include "session/session_serializer.hpp"
#include "core/json_glm.hpp"
#define MSGPACK_NO_BOOST
#include "core/msgpack_glm.hpp"
#include "core/logger.hpp"

#include <fstream>
#include <vector>

// json intrusive bindings
namespace mocap
{
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JointPose, rotation, position)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FaceParams, expression_blendshapes, jaw_pose)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PoseFrame, timestamp, frameIndex, root_translation, body_joints, hand_joints, face, overall_confidence, is_interpolated)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CaptureSession, frames, captureFrameRate, sessionId, captureDateTime)
}

// msgpack adaptors
namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<> struct pack<mocap::JointPose>
{
    template <typename Stream> packer<Stream>& operator()(msgpack::packer<Stream>& o, const mocap::JointPose& v) const
    {
        o.pack_array(2); 
        o.pack(v.rotation); 
        o.pack(v.position); 
        return o;
    }
};
template<> struct convert<mocap::JointPose>
{
    msgpack::object const& operator()(msgpack::object const& o, mocap::JointPose& v) const
    {
        if (o.type != msgpack::type::ARRAY || o.via.array.size != 2) throw msgpack::type_error();
        v.rotation = o.via.array.ptr[0].as<glm::quat>();
        v.position = o.via.array.ptr[1].as<glm::vec3>();
        return o;
    }
};

template<> struct pack<mocap::FaceParams>
{
    template <typename Stream> packer<Stream>& operator()(msgpack::packer<Stream>& o, const mocap::FaceParams& v) const
    {
        o.pack_array(2); 
        o.pack(v.expression_blendshapes); 
        o.pack(v.jaw_pose); 
        return o;
    }
};

template<> struct convert<mocap::FaceParams>
{
    msgpack::object const& operator()(msgpack::object const& o, mocap::FaceParams& v) const
    {
        if (o.type != msgpack::type::ARRAY || o.via.array.size != 2) throw msgpack::type_error();
        v.expression_blendshapes = o.via.array.ptr[0].as<std::vector<float>>();
        v.jaw_pose = o.via.array.ptr[1].as<glm::quat>();
        return o;
    }
};

template<> struct pack<mocap::PoseFrame>
{
    template <typename Stream> packer<Stream>& operator()(msgpack::packer<Stream>& o, const mocap::PoseFrame& v) const
    {
        o.pack_array(8);
        o.pack(v.timestamp); 
        o.pack(v.frameIndex); 
        o.pack(v.root_translation); 
        o.pack(v.body_joints);
        o.pack(v.hand_joints); 
        o.pack(v.face); 
        o.pack(v.overall_confidence); 
        o.pack(v.is_interpolated);
        return o;
    }
};
template<> struct convert<mocap::PoseFrame>
{
    msgpack::object const& operator()(msgpack::object const& o, mocap::PoseFrame& v) const
    {
        if (o.type != msgpack::type::ARRAY || o.via.array.size != 8) throw msgpack::type_error();
        v.timestamp = o.via.array.ptr[0].as<double>();
        v.frameIndex = o.via.array.ptr[1].as<int>();
        v.root_translation = o.via.array.ptr[2].as<glm::vec3>();
        v.body_joints = o.via.array.ptr[3].as<std::vector<mocap::JointPose>>();
        v.hand_joints = o.via.array.ptr[4].as<std::vector<mocap::JointPose>>();
        v.face = o.via.array.ptr[5].as<mocap::FaceParams>();
        v.overall_confidence = o.via.array.ptr[6].as<float>();
        v.is_interpolated = o.via.array.ptr[7].as<bool>();
        return o;
    }
};

template<> struct pack<mocap::CaptureSession>
{
    template <typename Stream> packer<Stream>& operator()(msgpack::packer<Stream>& o, const mocap::CaptureSession& v) const
    {
        o.pack_array(4);
        o.pack(v.frames); o.pack(v.captureFrameRate); o.pack(v.sessionId); o.pack(v.captureDateTime);
        return o;
    }
};
template<> struct convert<mocap::CaptureSession>
{
    msgpack::object const& operator()(msgpack::object const& o, mocap::CaptureSession& v) const
    {
        if (o.type != msgpack::type::ARRAY || o.via.array.size != 4) throw msgpack::type_error();
        v.frames = o.via.array.ptr[0].as<std::vector<mocap::PoseFrame>>();
        v.captureFrameRate = o.via.array.ptr[1].as<float>();
        v.sessionId = o.via.array.ptr[2].as<std::string>();
        v.captureDateTime = o.via.array.ptr[3].as<std::string>();
        return o;
    }
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE
} // namespace msgpack

// mcap file engine
namespace mocap {

Result<void> SessionSerializer::Save(const CaptureSession& session, const std::string& filepath, SessionFormat format)
{
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) return Result<void>::err("Failed to open file for writing: " + filepath);

    // macp security header
    const char magic[4] = {'M', 'C', 'A', 'P'};
    file.write(magic, 4);
    
    char version = 0x01;
    file.write(&version, 1);
    
    char encoding = (format == SessionFormat::JSON) ? 0x01 : 0x02;
    file.write(&encoding, 1);

    // write serialized payload
    try
    {
        if (format == SessionFormat::JSON)
        {
            nlohmann::json j = session;
            std::string s = j.dump();
            file.write(s.data(), s.size());
        }
        
        else
        {
            msgpack::sbuffer sbuf;
            msgpack::pack(sbuf, session);
            file.write(sbuf.data(), sbuf.size());
        }
    }
    
    catch (const std::exception& e)
    {
        return Result<void>::err(std::string("Serialization failed: ") + e.what());
    }

    if (file.fail()) return Result<void>::err("File stream error occurred during save.");

    MOCAP_INFO("Session saved successfully to {}", filepath);
    return Result<void>::ok();
}

Result<CaptureSession> SessionSerializer::Load(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return Result<CaptureSession>::err("Failed to open file for reading: " + filepath);

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size < 6) return Result<CaptureSession>::err("File is too small to be a valid MCAP file.");

    // mcap header validation
    char magic[4];
    file.read(magic, 4);
    if (magic[0] != 'M' || magic[1] != 'C' || magic[2] != 'A' || magic[3] != 'P')
    {
        return Result<CaptureSession>::err("Invalid file format: MCAP magic identifier missing.");
    }

    char version;
    file.read(&version, 1);
    if (version != 0x01) return Result<CaptureSession>::err("Unsupported MCAP version.");

    char encoding;
    file.read(&encoding, 1);
    if (encoding != 0x01 && encoding != 0x02) return Result<CaptureSession>::err("Unknown MCAP encoding format.");

    // load to memory
    std::streamsize payload_size = size - 6;
    std::vector<char> buffer(payload_size);
    if (payload_size > 0) file.read(buffer.data(), payload_size);

    // decode based on header flag
    try
    {
        CaptureSession session;
        if (encoding == 0x01) // json
        {
            nlohmann::json j = nlohmann::json::parse(buffer.begin(), buffer.end());
            session = j.get<CaptureSession>();
        }
        
        else // msgpack
        {
            msgpack::object_handle oh = msgpack::unpack(buffer.data(), buffer.size());
            msgpack::object obj = oh.get();
            obj.convert(session);
        }
        
        MOCAP_INFO("Session loaded successfully from {} (auto-detected: {})", filepath, encoding == 0x01 ? "JSON" : "MSGPACK");
        return Result<CaptureSession>::ok(session);
        
    }
    
    catch (const std::exception& e)
    {
        return Result<CaptureSession>::err(std::string("Deserialization failed: ") + e.what());
    }
}

}