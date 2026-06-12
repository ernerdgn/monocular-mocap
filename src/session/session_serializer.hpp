#pragma once
#include "core/types.hpp"
#include "core/result.hpp"
#include <string>

namespace mocap
{

enum class SessionFormat {
    JSON,
    MSGPACK
};

class SessionSerializer {
public:
    static Result<void> Save(const CaptureSession& session, const std::string& filepath, SessionFormat format);
    static Result<CaptureSession> Load(const std::string& filepath);
};

}