#pragma once
#pragma once
/*
Created by pratheek on 2019-11-27.
*/

#ifndef CENTROIDTRACKER_H
#define CENTROIDTRACKER_H

#endif //C___CENTROIDTRACKER_H

#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <set>
#include <algorithm>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "inference.h"
#include "ITracker.h"

using namespace inference;

class CentroidTracker : public ITracker {
public:
    explicit CentroidTracker(int maxDisappeared);

    void register_Object(int cX, int cY, float confidence, float class_id);

    std::vector<float> update(std::vector<inference::Detection>& output, const cv::Mat& frame) override ;
    void predictDS() override;
    // <ID, centroids>
    std::vector<std::pair<int, std::pair<int, int>>> objects;
    std::map<int, float> classIdMap; // Maps object IDs to class_id
    std::map<int, float> confidenceMap; // Maps object IDs to confidence
    //make buffer for path tracking

    std::map<int, std::vector<std::pair<int, int>>> path_keeper;
private:
    int maxDisappeared;

    int nextObjectID;

    static double calcDistance(double x1, double y1, double x2, double y2);

    // <ID, count>
    std::map<int, int> disappeared;
};
