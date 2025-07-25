#include "DStracker.h"
#include "DSnn_matching.h"
#include "DSmodel.h"
#include "DSlinear_assignment.h"
using namespace std;
//#define MY_inner_DEBUG
#ifdef MY_inner_DEBUG
#include <string>
#include <iostream>
#endif

tracker::tracker(/*NearNeighborDisMetric *metric,*/
    float max_cosine_distance, int nn_budget,
    float max_iou_distance, int max_age, int n_init)
{
    this->metric = new NearNeighborDisMetric(
        NearNeighborDisMetric::METRIC_TYPE::cosine,
        max_cosine_distance, nn_budget);
    this->max_iou_distance = max_iou_distance;
    this->max_age = max_age;
    this->n_init = n_init;

    this->kf = new KalmanFilter();
    this->tracks.clear();
    this->_next_idx = 1;
}

void tracker::predictDS()
{
    for (Track& track : tracks)
    {
        track.predit(kf);
    }
}
void get_detections(DETECTBOX box, float confidence, DETECTIONS& d,float cls)
{
    
    DETECTION_ROW tmpRow;
    tmpRow.tlwh = box;
    tmpRow.cls = cls;
    tmpRow.confidence = confidence;
    // Push the DETECTION_ROW object containing the box, confidence, and cls into the vector
    d.push_back(tmpRow);
    
}
std::vector<float> tracker::update(std::vector<Detection>& results,const cv::Mat& frame)
{
    /////////////////////////
    std::vector<Detection> objects;
    DETECTIONS detections;
    std::vector<float> fine;
    for (Detection dr : results)
    {
        
        objects.push_back(dr);
        cv::rectangle(frame, dr.box, cv::Scalar(255, 0, 0), 2);
        get_detections(DETECTBOX(dr.box.x, dr.box.y, dr.box.width, dr.box.height), dr.confidence, detections,dr.class_id);
        
        
    }
    if (FeatureTensor::getInstance()->getRectsFeature(frame, detections))
    {
        // std::cout << "111111111111111111111111111111111111111" << std::endl;
        this->predictDS();
        // std::cout << "222222222222222222222" << std::endl;
        
        //////////////////////////////
        TRACHER_MATCHD res;
        // std::cout << detections.size() << std::endl;
        _match(detections, res);
        // std::cout << "3333333333333333333333333333" << std::endl;
        
        vector<MATCH_DATA>& matches = res.matches;
        // std::cout << "4444444444444444444444444444" << std::endl;

        //#ifdef MY_inner_DEBUG
        //    printf("res.matches size = %d:\n", matches.size());
        //#endif
        for (MATCH_DATA& data : matches)
        {
            int track_idx = data.first;
            int detection_idx = data.second;
            //#ifdef MY_inner_DEBUG
            //        printf("\t%d == %d;\n", track_idx, detection_idx);
            //#endif
            tracks[track_idx].update(this->kf, detections[detection_idx]);
            
            /*std::cout << "1A1A1A1A1A1A: " << tracks[track_idx]. << std::endl;*/

        }
        vector<int>& unmatched_tracks = res.unmatched_tracks;
        //#ifdef MY_inner_DEBUG
        //    printf("res.unmatched_tracks size = %d\n", unmatched_tracks.size());
        //#endif
        for (int& track_idx : unmatched_tracks)
        {
            this->tracks[track_idx].mark_missed();
        }
        vector<int>& unmatched_detections = res.unmatched_detections;
        //#ifdef MY_inner_DEBUG
        //    printf("res.unmatched_detections size = %d\n", unmatched_detections.size());
        //#endif
        for (int& detection_idx : unmatched_detections)
        {
            this->_initiate_track(detections[detection_idx]);
        }
        //#ifdef MY_inner_DEBUG
        //    int size = tracks.size();
        //    printf("now track size = %d\n", size);
        //#endif
        vector<Track>::iterator it;
        for (it = tracks.begin(); it != tracks.end();)
        {
            if ((*it).is_deleted())
                it = tracks.erase(it);
            else
                ++it;
        }

        vector<int> active_targets;
        vector<TRACKER_DATA> tid_features;
        for (Track& track : tracks)
        {
            
            if (track.is_confirmed() == false)
                continue;
            active_targets.push_back(track.track_id);
            tid_features.push_back(std::make_pair(track.track_id, track.features));
            FEATURESS t = FEATURESS(0, k_feature_dim);
            track.features = t;
        }
        this->metric->partial_fit(tid_features, active_targets);

        std::vector<float>ves;
        std::vector<RESULT_DATA> result;
        int ki = 0;
        for (Track& track : this->tracks) {
            
            if (!track.is_confirmed() || track.time_since_update > 1) continue;
            result.push_back(std::make_pair(track.track_id, track.to_tlwh()));
            ves.push_back(track.confidence); ves.push_back(track.cls);
            ki++;
        }
        // std::cout << "ID: " << ves.size() << std::endl;
        // std::cout << "ID: " << result.size() << std::endl;
        //for (unsigned int k = 0; k < detections.size(); k++)
        //{
        //    DETECTBOX tmpbox = detections[k].tlwh;
        //    cv::Rect rect(tmpbox(0), tmpbox(1), tmpbox(2), tmpbox(3));
        //    //cv::rectangle(frame, rect, cv::Scalar(0, 0, 255), 4);
        for (unsigned int k = 0; k < result.size(); k++)
        {
            DETECTBOX tmp = result[k].second;
            //cv::Rect rect = cv::Rect(tmp(0), tmp(1), tmp(2), tmp(3));
            //rectangle(frame, rect, cv::Scalar(255, 255, 0), 2);
            fine.push_back(tmp(0));
            fine.push_back(tmp(1));
            fine.push_back(tmp(2));
            fine.push_back(tmp(3));
            fine.push_back(ves[2*k]);
            fine.push_back(ves[2*k+1]);
            fine.push_back(result[k].first);
            
            // std::cout << "ID: " << result[k].first << std::endl;
            //std::string label = cv::format("%d", result[k].first);
            //cv::putText(frame, label, cv::Point(rect.x, rect.y), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 0), 2);
        }
            
        //}
    }
    
    return fine;
    
}

void tracker::_match(const DETECTIONS& detections, TRACHER_MATCHD& res)
{
    // std::cout << "55555555555555555" << std::endl;
    vector<int> confirmed_tracks;
    vector<int> unconfirmed_tracks;
    int idx = 0;
    for (Track& t : tracks)
    {
        if (t.is_confirmed())
            confirmed_tracks.push_back(idx);
        else
            unconfirmed_tracks.push_back(idx);
        idx++;
    }
    // std::cout << "6666666666666666666666" << std::endl;
    TRACHER_MATCHD matcha = linear_assignment::getInstance()->matching_cascade(
        this, &tracker::gated_matric,
        this->metric->mating_threshold,
        this->max_age,
        this->tracks,
        detections,
        confirmed_tracks);
    // std::cout << "777777777777777777777777" << std::endl;
    vector<int> iou_track_candidates;
    iou_track_candidates.assign(unconfirmed_tracks.begin(), unconfirmed_tracks.end());
    vector<int>::iterator it;
    // std::cout << "999999999999999999999999" << std::endl;
    for (it = matcha.unmatched_tracks.begin(); it != matcha.unmatched_tracks.end();)
    {
        int idx = *it;
        if (tracks[idx].time_since_update == 1)
        { // push into unconfirmed
            iou_track_candidates.push_back(idx);
            it = matcha.unmatched_tracks.erase(it);
            continue;
        }
        ++it;
    }
    // std::cout << "AAAAAAAAAAAAAAAAAAAAAAAA" << std::endl;
    TRACHER_MATCHD matchb = linear_assignment::getInstance()->min_cost_matching(
        this, &tracker::iou_cost,
        this->max_iou_distance,
        this->tracks,
        detections,
        iou_track_candidates,
        matcha.unmatched_detections);
    // get result:
    // std::cout << "BBBBBBBBBBBBBBBBBBBBBB" << std::endl;
    res.matches.assign(matcha.matches.begin(), matcha.matches.end());
    res.matches.insert(res.matches.end(), matchb.matches.begin(), matchb.matches.end());
    // unmatched_tracks;
    // std::cout << "CCCCCCCCCCCCCCCCCC" << std::endl;
    res.unmatched_tracks.assign(
        matcha.unmatched_tracks.begin(),
        matcha.unmatched_tracks.end());
    res.unmatched_tracks.insert(
        res.unmatched_tracks.end(),
        matchb.unmatched_tracks.begin(),
        matchb.unmatched_tracks.end());
    res.unmatched_detections.assign(
        matchb.unmatched_detections.begin(),
        matchb.unmatched_detections.end());
    // std::cout << "DDDDDDDDDDDDDDDD" << std::endl;
}

void tracker::_initiate_track(const DETECTION_ROW& detection)
{
    KAL_DATA data = kf->initiate(detection.to_xyah());
    KAL_MEAN mean = data.first;
    KAL_COVA covariance = data.second;

    this->tracks.push_back(Track(mean, covariance, this->_next_idx, this->n_init,
        this->max_age, detection.feature));
    _next_idx += 1;
}

DYNAMICM tracker::gated_matric(
    std::vector<Track>& tracks,
    const DETECTIONS& dets,
    const std::vector<int>& track_indices,
    const std::vector<int>& detection_indices)
{
    FEATURESS features(detection_indices.size(), k_feature_dim);
    int pos = 0;
    for (int i : detection_indices)
    {
        features.row(pos++) = dets[i].feature;
    }
    vector<int> targets;
    for (int i : track_indices)
    {
        targets.push_back(tracks[i].track_id);
    }
    DYNAMICM cost_matrix = this->metric->distance(features, targets);
    DYNAMICM res = linear_assignment::getInstance()->gate_cost_matrix(
        this->kf, cost_matrix, tracks, dets, track_indices,
        detection_indices);
    return res;
}

DYNAMICM
tracker::iou_cost(
    std::vector<Track>& tracks,
    const DETECTIONS& dets,
    const std::vector<int>& track_indices,
    const std::vector<int>& detection_indices)
{
    //!!!python diff: track_indices && detection_indices will never be None.
    //    if(track_indices.empty() == true) {
    //        for(size_t i = 0; i < tracks.size(); i++) {
    //            track_indices.push_back(i);
    //        }
    //    }
    //    if(detection_indices.empty() == true) {
    //        for(size_t i = 0; i < dets.size(); i++) {
    //            detection_indices.push_back(i);
    //        }
    //    }
    int rows = track_indices.size();
    int cols = detection_indices.size();
    DYNAMICM cost_matrix = Eigen::MatrixXf::Zero(rows, cols);
    for (int i = 0; i < rows; i++)
    {
        int track_idx = track_indices[i];
        if (tracks[track_idx].time_since_update > 1)
        {
            cost_matrix.row(i) = Eigen::RowVectorXf::Constant(cols, INFTY_COST);
            continue;
        }
        DETECTBOX bbox = tracks[track_idx].to_tlwh();
        int csize = detection_indices.size();
        DETECTBOXSS candidates(csize, 4);
        for (int k = 0; k < csize; k++)
            candidates.row(k) = dets[detection_indices[k]].tlwh;
        Eigen::RowVectorXf rowV = (1. - iou(bbox, candidates).array()).matrix().transpose();
        cost_matrix.row(i) = rowV;
    }
    return cost_matrix;
}

Eigen::VectorXf
tracker::iou(DETECTBOX& bbox, DETECTBOXSS& candidates)
{
    float bbox_tl_1 = bbox[0];
    float bbox_tl_2 = bbox[1];
    float bbox_br_1 = bbox[0] + bbox[2];
    float bbox_br_2 = bbox[1] + bbox[3];
    float area_bbox = bbox[2] * bbox[3];

    Eigen::Matrix<float, -1, 2> candidates_tl;
    Eigen::Matrix<float, -1, 2> candidates_br;
    candidates_tl = candidates.leftCols(2);
    candidates_br = candidates.rightCols(2) + candidates_tl;

    int size = int(candidates.rows());
    //    Eigen::VectorXf area_intersection(size);
    //    Eigen::VectorXf area_candidates(size);
    Eigen::VectorXf res(size);
    for (int i = 0; i < size; i++)
    {
        float tl_1 = std::max(bbox_tl_1, candidates_tl(i, 0));
        float tl_2 = std::max(bbox_tl_2, candidates_tl(i, 1));
        float br_1 = std::min(bbox_br_1, candidates_br(i, 0));
        float br_2 = std::min(bbox_br_2, candidates_br(i, 1));

        float w = br_1 - tl_1;
        w = (w < 0 ? 0 : w);
        float h = br_2 - tl_2;
        h = (h < 0 ? 0 : h);
        float area_intersection = w * h;
        float area_candidates = candidates(i, 2) * candidates(i, 3);
        res[i] = area_intersection / (area_bbox + area_candidates - area_intersection);
    }
    //#ifdef MY_inner_DEBUG
    //        std::cout << res << std::endl;
    //#endif
    return res;
}