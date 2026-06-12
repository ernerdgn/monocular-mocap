#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "smoothing/temporal_smoother.hpp"
#include <vector>

using namespace mocap;

TEST_CASE("TemporalSmoother Occlusion and Interpolation", "[smoothing]")
{
    //1000 hz cutoff: tau approaches 0, alpha becomes 1.0 and the filter passes raw data
    //TemporalSmoother smoother(1000.0f, 0.05f);
    //TemporalSmoother smoother(1.0f, .05f);

    SECTION("Normal confident frames pass through immediately")
    {
        TemporalSmoother smoother(1000.0f, 0.05f);
        PoseFrame f1;
        f1.timestamp = 0.0f;
        f1.overall_confidence = 0.9f;
        
        auto result = smoother.process(f1);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].is_interpolated == false);
    }

    SECTION("Low confidence frames are buffered and interpolated on recovery")
    {
        TemporalSmoother smoother(1000.0f, 0.05f);
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

    SECTION("Noisy signal is smoothed (Mean Absolute Difference reduction)")
    {
        TemporalSmoother normal_smoother(1.0f, 0.05f);
        
        float input_diff_sum = 0.0f;
        float output_diff_sum = 0.0f;
        float prev_input = 0.0f;
        float prev_output = 0.0f;

        for (int i = 0; i < 10; ++i)
        {
            PoseFrame f;
            f.timestamp = i * 0.033f; // 30 FPS
            f.overall_confidence = 0.9f;
            
            float noisy_val = (i % 2 == 0) ? 1.0f : -1.0f; 
            f.root_translation = glm::vec3(noisy_val, 0.0f, 0.0f);

            auto res = normal_smoother.process(f);
            REQUIRE(res.size() == 1);

            if (i > 0)
            {
                input_diff_sum += std::abs(noisy_val - prev_input);
                output_diff_sum += std::abs(res[0].root_translation.x - prev_output);
            }
            prev_input = noisy_val;
            prev_output = res[0].root_translation.x;
        }

        REQUIRE(output_diff_sum < input_diff_sum); 
    }

    SECTION("Occlusion exceeding max gap triggers frozen fallback")
    {
        TemporalSmoother normal_smoother(1.0f, 0.05f);
        
        PoseFrame f1;
        f1.timestamp = 0.0f;
        f1.overall_confidence = 0.9f;
        f1.root_translation = glm::vec3(5.0f, 0.0f, 0.0f);
        normal_smoother.process(f1);

        PoseFrame f2;
        f2.timestamp = 1.5f;
        f2.overall_confidence = 0.2f;
        f2.root_translation = glm::vec3(99.0f, 99.0f, 99.0f); 

        auto res = normal_smoother.process(f2);
        
        REQUIRE(res.size() == 1);
        REQUIRE(res[0].is_interpolated == true);
        REQUIRE(res[0].root_translation.x == Catch::Approx(5.0f).margin(0.01f)); // Frozen at f1's position
    }
}