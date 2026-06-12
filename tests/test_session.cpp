#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "session/session_serializer.hpp"
#include <fstream>
#include <filesystem>

using namespace mocap;

TEST_CASE("MCAP Session Serialization Engine", "[session]")
{
    CaptureSession original;
    original.sessionId = "test_verify_01";
    original.captureFrameRate = 60.0f;
    original.captureDateTime = "2026-06-12T14:00:00Z";

    PoseFrame f1;
    f1.timestamp = 1.5;
    f1.frameIndex = 90;
    f1.root_translation = glm::vec3(1.1f, -2.2f, 3.3f);
    f1.overall_confidence = 0.95f;
    f1.is_interpolated = true;
    
    f1.face.jaw_pose = glm::quat(0.5f, 0.5f, 0.5f, 0.5f); 
    original.frames.push_back(f1);

    std::string test_file = "temp_test_session.mcap";

    SECTION("JSON Encoding Round-Trip Parity")
    {
        auto save_res = SessionSerializer::Save(original, test_file, SessionFormat::JSON);
        REQUIRE(save_res.is_ok());

        auto load_res = SessionSerializer::Load(test_file);
        REQUIRE(load_res.is_ok());

        CaptureSession loaded = load_res.value();
        REQUIRE(loaded.sessionId == "test_verify_01");
        REQUIRE(loaded.frames.size() == 1);
        REQUIRE(loaded.frames[0].timestamp == Catch::Approx(1.5));
        REQUIRE(loaded.frames[0].root_translation.y == Catch::Approx(-2.2f));
        REQUIRE(loaded.frames[0].is_interpolated == true);
        REQUIRE(loaded.frames[0].face.jaw_pose.w == Catch::Approx(0.5f));
    }

    SECTION("MessagePack Binary Round-Trip Parity")
    {
        auto save_res = SessionSerializer::Save(original, test_file, SessionFormat::MSGPACK);
        REQUIRE(save_res.is_ok());

        auto load_res = SessionSerializer::Load(test_file);
        REQUIRE(load_res.is_ok());

        CaptureSession loaded = load_res.value();
        REQUIRE(loaded.captureFrameRate == Catch::Approx(60.0f));
        REQUIRE(loaded.frames[0].root_translation.x == Catch::Approx(1.1f));
        REQUIRE(loaded.frames[0].overall_confidence == Catch::Approx(0.95f));
    }

    SECTION("Corrupt File / Invalid Header Rejection")
    {
        std::string bad_file = "temp_corrupt.mcap";

        std::ofstream out(bad_file, std::ios::binary);
        out.write("GARBAGE_DATA", 12);
        out.close();

        auto load_res = SessionSerializer::Load(bad_file);
        
        REQUIRE(load_res.is_err());
        REQUIRE(load_res.error().find("magic identifier missing") != std::string::npos);
        
        std::filesystem::remove(bad_file);
    }

    if (std::filesystem::exists(test_file)) std::filesystem::remove(test_file);
}