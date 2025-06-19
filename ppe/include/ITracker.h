#ifndef ITRACKER_H
#define ITRACKER_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "inference.h"
// Assuming Detection is defined elsewhere

class ITracker {
public:
    //virtual std::vector<float> update(std::vector<Detection>& detFrameData, const cv::Mat& frame);
    virtual std::vector<float> update(std::vector<inference::Detection>& detFrameData, const cv::Mat& frame) = 0;
    virtual void predictDS()=0;
    virtual ~ITracker() {}
};

#endif // ITRACKER_H
