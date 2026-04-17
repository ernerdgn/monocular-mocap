#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "flow/optical_flow_processor.hpp"
#include <opencv2/imgproc.hpp>

using namespace mocap;

CaptureFrame createSyntheticFrame(int offset, double timestamp)
{
    CaptureFrame frame;
    frame.timestamp = timestamp;
    frame.frameIndex = static_cast<int>(timestamp * 30.0);
    frame.image = cv::Mat::zeros(480, 640, CV_8UC3);
    
    cv::rectangle(frame.image, 
                  cv::Point(100 + offset, 100 + offset), 
                  cv::Point(200 + offset, 200 + offset), 
                  cv::Scalar(255, 255, 255), -1);
    return frame;
}

TEST_CASE("OpticalFlowProcessor detects synthetic motion", "[flow]")
{
    OpticalFlowProcessor processor;

    SECTION("Static frames produce zero magnitude")
    {
        CaptureFrame frame1 = createSyntheticFrame(0, 0.0);
        CaptureFrame frame2 = createSyntheticFrame(0, 0.033);

        auto result1 = processor.process(frame1);
        auto result2 = processor.process(frame2);

        REQUIRE(result1.motionMagnitude == 0.0f);
        REQUIRE(result2.motionMagnitude == Catch::Approx(0.0f).margin(0.001f));
    }

    SECTION("Shifted frames produce positive magnitude")
    {
        CaptureFrame frame1 = createSyntheticFrame(0, 0.0);
        CaptureFrame frame2 = createSyntheticFrame(10, 0.033); // Shifted by 10 pixels

        auto result1 = processor.process(frame1);
        auto result2 = processor.process(frame2);

        REQUIRE(result1.motionMagnitude == 0.0f);
        REQUIRE(result2.motionMagnitude > 0.01f);
    }

    SECTION("Reset method clears tracking history")
    {
        CaptureFrame frame1 = createSyntheticFrame(0, 0.0);
        CaptureFrame frame2 = createSyntheticFrame(10, 0.033); 

        processor.process(frame1);
        
        processor.reset();

        auto result2 = processor.process(frame2);

        REQUIRE(result2.motionMagnitude == 0.0f);
    }
}