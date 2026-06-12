#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "smoothing/temporal_smoother.hpp"
#include <vector>

using namespace mocap;

TEST_CASE("TemporalSmoother Occlusion and Interpolation", "[smoothing]")
{
    //1000 hz cutoff: tau approaches 0, alpha becomes 1.0 and the filter passes raw data
    TemporalSmoother smoother(1000.0f, 0.05f);

    SECTION("Normal confident frames pass through immediately")
    {
        PoseFrame f1;
        f1.timestamp = 0.0f;
        f1.overall_confidence = 0.9f;
        
        auto result = smoother.process(f1);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].is_interpolated == false);
    }

    SECTION("Low confidence frames are buffered and interpolated on recovery")
    {
        // init confident frame (T = 0.0s, X = 0.0)
        PoseFrame f1;
        f1.timestamp = 0.0f;
        f1.overall_confidence = 0.9f;
        f1.root_translation = glm::vec3(0.0f, 0.0f, 0.0f);
        smoother.process(f1);

        // occlusion frame (T = 0.5s, garbage data)
        PoseFrame f2;
        f2.timestamp = 0.5f;
        f2.overall_confidence = 0.2f; // below thold
        f2.root_translation = glm::vec3(99.0f, 99.0f, 99.0f); // ai hallucination (rooms man... mushrooms)
        
        auto buffer_result = smoother.process(f2);
        REQUIRE(buffer_result.empty());

        // recovery frame (T = 1.0s, X = 2.0)
        PoseFrame f3;
        f3.timestamp = 1.0f;
        f3.overall_confidence = 0.9f;
        f3.root_translation = glm::vec3(2.0f, 0.0f, 0.0f);

        auto recovery_result = smoother.process(f3);
        
        REQUIRE(recovery_result.size() == 2); 

        // f2 should be interpolated
        // T=.5, X=1.0
        REQUIRE(recovery_result[0].is_interpolated == true);
        REQUIRE(recovery_result[0].root_translation.x == Catch::Approx(1.0f).margin(0.05f));
        
        REQUIRE(recovery_result[1].is_interpolated == false);
        REQUIRE(recovery_result[1].root_translation.x == Catch::Approx(2.0f).margin(0.05f));
    }
}