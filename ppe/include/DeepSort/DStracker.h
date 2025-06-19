#pragma once
#ifndef TRACKER_H
#define TRACKER_H
#include <vector>

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "DSkalmanfilter.h"
#include "DStrack.h"
#include "DSmodel.h"
#include "inference.h"
#include "DSFeatureTensor.h"
#include "ITracker.h"
class NearNeighborDisMetric;

class tracker : public ITracker
{
public:
    NearNeighborDisMetric* metric;
    float max_iou_distance;
    int max_age;
    int n_init;

    KalmanFilter* kf;

    int _next_idx;
public:
    std::vector<Track> tracks;
    tracker(/*NearNeighborDisMetric* metric,*/
        float max_cosine_distance, int nn_budget,
        float max_iou_distance = 0.7,
        int max_age = 30, int n_init = 3);
    void predictDS() override;
    std::vector<float>update(std::vector<Detection>& results,const cv::Mat& frame) override;
    typedef DYNAMICM(tracker::* GATED_METRIC_FUNC)(
        std::vector<Track>& tracks,
        const DETECTIONS& dets,
        const std::vector<int>& track_indices,
        const std::vector<int>& detection_indices);
private:
    void _match(const DETECTIONS& detections, TRACHER_MATCHD& res);
    void _initiate_track(const DETECTION_ROW& detection);
public:
    DYNAMICM gated_matric(
        std::vector<Track>& tracks,
        const DETECTIONS& dets,
        const std::vector<int>& track_indices,
        const std::vector<int>& detection_indices);
    DYNAMICM iou_cost(
        std::vector<Track>& tracks,
        const DETECTIONS& dets,
        const std::vector<int>& track_indices,
        const std::vector<int>& detection_indices);
    Eigen::VectorXf iou(DETECTBOX& bbox,
        DETECTBOXSS& candidates);
};

#endif // TRACKER_H