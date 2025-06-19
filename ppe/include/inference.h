#pragma once

// Cpp native
#include <fstream>
#include <vector>
#include <string>
#include <random>

// OpenCV / DNN / Inference
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

namespace inference {

    struct Detection
    {
        int class_id{ 0 };
        std::string className{};
        float confidence{ 0.0 };
        cv::Scalar color{};
        cv::Rect box{};
    };
}
