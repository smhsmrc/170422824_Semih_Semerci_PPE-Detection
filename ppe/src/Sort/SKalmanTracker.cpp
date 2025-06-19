///////////////////////////////////////////////////////////////////////////////
// KalmanTracker.cpp: KalmanTracker Class Implementation Declaration

#include "SKalmanTracker.h"
#include "SHungarian.h"
typedef struct TrackingBox
{
    int frame;
    int id;
    Rect_<float> box;
    float confidence;
    float cls;
}TrackingBox;


int KalmanTracker::kf_count = 0;


// initialize Kalman filter
void KalmanTracker::init_kf(StateType stateMat)
{
	int stateNum = 7;
	int measureNum = 4;
	kf = KalmanFilter(stateNum, measureNum, 0);

	measurement = Mat::zeros(measureNum, 1, CV_32F);

	kf.transitionMatrix = (Mat_<float>(stateNum, stateNum) <<
		1, 0, 0, 0, 1, 0, 0,
		0, 1, 0, 0, 0, 1, 0,
		0, 0, 1, 0, 0, 0, 1,
		0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0,
		0, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 1);

	setIdentity(kf.measurementMatrix);
	setIdentity(kf.processNoiseCov, Scalar::all(1e-2));
	setIdentity(kf.measurementNoiseCov, Scalar::all(1e-1));
	setIdentity(kf.errorCovPost, Scalar::all(1));

	// initialize state vector with bounding box in [cx,cy,s,r] style
	kf.statePost.at<float>(0, 0) = stateMat.x + stateMat.width / 2;
	kf.statePost.at<float>(1, 0) = stateMat.y + stateMat.height / 2;
	kf.statePost.at<float>(2, 0) = stateMat.area();
	kf.statePost.at<float>(3, 0) = stateMat.width / stateMat.height;
}


// Predict the estimated bounding box.
StateType KalmanTracker::predict()
{
	// predict
	Mat p = kf.predict();
	m_age += 1;

	if (m_time_since_update > 0)
		m_hit_streak = 0;
	m_time_since_update += 1;

	StateType predictBox = get_rect_xysr(p.at<float>(0, 0), p.at<float>(1, 0), p.at<float>(2, 0), p.at<float>(3, 0));

	m_history.push_back(predictBox);
	return m_history.back();
}


// Update the state vector with observed bounding box.
void KalmanTracker::updateKF(StateType stateMat)
{
	m_time_since_update = 0;
	m_history.clear();
	m_hits += 1;
	m_hit_streak += 1;

	// measurement
	measurement.at<float>(0, 0) = stateMat.x + stateMat.width / 2;
	measurement.at<float>(1, 0) = stateMat.y + stateMat.height / 2;
	measurement.at<float>(2, 0) = stateMat.area();
	measurement.at<float>(3, 0) = stateMat.width / stateMat.height;

	// update
	kf.correct(measurement);
}
double GetIOU(Rect_<float> bb_test, Rect_<float> bb_gt)
{
    float in = (bb_test & bb_gt).area();
    float un = bb_test.area() + bb_gt.area() - in;

    if (un < DBL_EPSILON)
        return 0;

    return (double)(in / un);
}
int frame_count = 0;
// ✅ HUMAN-VEST KARIŞIKLIĞI İÇİN OPTİMİZE EDİLDİ
int max_age = 15;           // 30→15: Yanlış track'ler daha hızlı silinir
int min_hits = 2;           // 3→2: Daha hızlı track oluşumu (daha responsive)  
double iouThreshold = 0.25; // 0.3→0.25: Daha strict overlap kontrolü (karışıklığı azaltır)
double confidenceThreshold = 0.6; // Yeni tracker oluşturma için minimum confidence

vector<KalmanTracker> trackers;
 // tracking id relies on this, so we have to reset it in each seq.
void KalmanTracker::predictDS() {}
std::vector<float> KalmanTracker::update(std::vector<inference::Detection>& output, const cv::Mat& frame)
{   
    
    std::vector<inference::Detection>& detFrameData = output;
    KalmanTracker tracker;
    #define CNUM 20
    int total_frames = 0;
    double total_time = 0.0;
    RNG rng(0xFFFFFFFF);
    Scalar_<int> randColor[CNUM];
    for (int i = 0; i < CNUM; i++)
        rng.fill(randColor[i], RNG::UNIFORM, 0, 256);


    // 3. update across frames
    
    // std::cout << "SORT: " <<output.size() <<std::endl;
    
    // variables used in the for-loop
    vector<Rect_<float>> predictedBoxes;
    vector<vector<double>> iouMatrix;
    vector<int> assignment;
    set<int> unmatchedDetections;
    set<int> unmatchedTrajectories;
    set<int> allItems;
    set<int> matchedItems;
    vector<cv::Point> matchedPairs;
    vector<TrackingBox> frameTrackingResult;
    unsigned int trkNum = 0;
    unsigned int detNum = 0;

    double cycle_time = 0.0;
    int64 start_time = 0;



    if (trackers.size() == 0) // the first frame met
    {
        // initialize kalman trackers using first detections.
        for (unsigned int i = 0; i < detFrameData.size(); i++)
        {
            KalmanTracker trk = KalmanTracker(detFrameData[i].box, detFrameData[i].confidence, detFrameData[i].class_id);
            trackers.push_back(trk);
            //this->cls = detFrameData[i].class_id;
            //this->confidence = detFrameData[i].confidence;
        }
        // output the first frame detections
        for (unsigned int id = 0; id < detFrameData.size(); id++)
        {
            inference::Detection tb = detFrameData[id];
        }
        
    }

    ///////////////////////////////////////
    // 3.1. get predicted locations from existing trackers.
    predictedBoxes.clear();

    for (auto it = trackers.begin(); it != trackers.end();)
    {
        Rect_<float> pBox = (*it).predict();
        if (pBox.x >= 0 && pBox.y >= 0)
        {
            predictedBoxes.push_back(pBox);
            it++;
        }
        else
        {
            it = trackers.erase(it);
            //cerr << "Box invalid at frame: " << frame_count << endl;
        }
    }

    ///////////////////////////////////////
    // 3.2. associate detections to tracked object (both represented as bounding boxes)
    // dets : detFrameData[fi]
    trkNum = predictedBoxes.size();
    detNum = detFrameData.size();

    iouMatrix.clear();
    iouMatrix.resize(trkNum, vector<double>(detNum, 0));

    for (unsigned int i = 0; i < trkNum; i++) // compute iou matrix as a distance matrix
    {
        for (unsigned int j = 0; j < detNum; j++)
        {
            // use 1-iou because the hungarian algorithm computes a minimum-cost assignment.
            iouMatrix[i][j] = 1 - GetIOU(predictedBoxes[i], detFrameData[j].box);
        }
    }
   
    // solve the assignment problem using hungarian algorithm.
    // the resulting assignment is [track(prediction) : detection], with len=preNum
     
    HungarianAlgorithm HungAlgo;
    assignment.clear();       

    HungAlgo.Solve(iouMatrix, assignment);

    // find matches, unmatched_detections and unmatched_predictions
    unmatchedTrajectories.clear();
    unmatchedDetections.clear();
    allItems.clear();
    matchedItems.clear();
      
    if (detNum > trkNum) //	there are unmatched detections
    {
        for (unsigned int n = 0; n < detNum; n++)
            allItems.insert(n);

        for (unsigned int i = 0; i < trkNum; ++i)
            matchedItems.insert(assignment[i]);

        set_difference(allItems.begin(), allItems.end(),
            matchedItems.begin(), matchedItems.end(),
            insert_iterator<set<int>>(unmatchedDetections, unmatchedDetections.begin()));
    }
    else if (detNum < trkNum) // there are unmatched trajectory/predictions
    {
        for (unsigned int i = 0; i < trkNum; ++i)
            if (assignment[i] == -1) // unassigned label will be set as -1 in the assignment algorithm
                unmatchedTrajectories.insert(i);
    }
    else
        ;

    // filter out matched with low IOU and class mismatch
    matchedPairs.clear();
    for (unsigned int i = 0; i < trkNum; ++i)
    {
        if (assignment[i] == -1) // pass over invalid values
            continue;
            
        bool iouMatch = (1 - iouMatrix[i][assignment[i]]) >= iouThreshold;
        
        // ✅ CLASS CONSISTENCY KONTROLÜ - Human-Vest karışıklığını önle
        bool classMatch = (trackers[i].cls == detFrameData[assignment[i]].class_id);
        
        // ✅ Özel durum: Human (class 2) ve diğer KKD'ler arasında strict kontrol
        bool strictClassCheck = true;
        if (trackers[i].cls == 2 || detFrameData[assignment[i]].class_id == 2) {
            // Human ile diğer class'lar arasında assignment'a izin verme
            strictClassCheck = classMatch;
        }
        
        if (!iouMatch || !strictClassCheck)
        {
            unmatchedTrajectories.insert(i);
            unmatchedDetections.insert(assignment[i]);
            
            // ✅ Debug: Class karışıklığı tespiti
            if (iouMatch && !classMatch) {
                std::cout << "🚨 CLASS MISMATCH TESPİT EDİLDİ: Tracker[" << trackers[i].cls 
                         << "] → Detection[" << detFrameData[assignment[i]].class_id << "] (Track ID: " 
                         << trackers[i].m_id << ")" << std::endl;
            }
        }
        else
        {
            matchedPairs.push_back(cv::Point(i, assignment[i]));
        }
    }

    ///////////////////////////////////////
    // 3.3. updating trackers
   
    // update matched trackers with assigned detections.
    // each prediction is corresponding to a tracker
    int detIdx, trkIdx;
    for (unsigned int i = 0; i < matchedPairs.size(); i++)
    {
        trkIdx = matchedPairs[i].x;
        detIdx = matchedPairs[i].y;
        
        // ✅ Class consistency'yi koru - confidence'ı da güncelle
        trackers[trkIdx].updateKF(detFrameData[detIdx].box);
        trackers[trkIdx].confidence = detFrameData[detIdx].confidence; // Confidence güncelle
        // Class'ı güncelleme - sadece aynı class'tan gelirse
        if (trackers[trkIdx].cls == detFrameData[detIdx].class_id) {
            trackers[trkIdx].cls = detFrameData[detIdx].class_id;
        }
    }

    // create and initialise new trackers for unmatched detections
    for (auto umd : unmatchedDetections)
    {
        // ✅ Sadece yüksek confidence'lı detection'lar için yeni tracker oluştur
        if (detFrameData[umd].confidence > confidenceThreshold) { // Minimum confidence threshold
            KalmanTracker tracker = KalmanTracker(detFrameData[umd].box, detFrameData[umd].confidence, detFrameData[umd].class_id);
            trackers.push_back(tracker);
        }
    }

    // get trackers' output
    frameTrackingResult.clear();
    for (auto it = trackers.begin(); it != trackers.end();)
    {
        if (((*it).m_time_since_update < 1) &&
            ((*it).m_hit_streak >= min_hits || frame_count <= min_hits))
        {
            TrackingBox res;
            res.box = (*it).get_state();
            res.id = (*it).m_id + 1;
            res.frame = frame_count;
            res.cls = (*it).cls;
            res.confidence = (*it).confidence;
            frameTrackingResult.push_back(res);
            
            it++;
        }
        else
            it++;

        // remove dead tracklet
        if (it != trackers.end() && (*it).m_time_since_update > max_age)
            it = trackers.erase(it);

    }

    cycle_time = (double)(getTickCount() - start_time);
    total_time += cycle_time / getTickFrequency();
    std::vector<float> fine;
   
    for (auto tb : frameTrackingResult)
    {
        /*putText(frame, format("%d", tb.id), Point(tb.box.x, tb.box.y - 5),
            0, 0.6, Scalar(0, 0, 255), 2, LINE_AA);
        cv::rectangle(frame, tb.box, randColor[tb.id % CNUM], 2, 8, 0);*/
        fine.push_back(tb.box.x);
        fine.push_back(tb.box.y);
        fine.push_back(tb.box.width);
        fine.push_back(tb.box.height);
        fine.push_back(tb.confidence);
        fine.push_back(tb.cls);
        fine.push_back(tb.id);
        
        
    }


    
    /*cv::imshow("sort", frame);*/
    return fine;
}


// Return the current state vector
StateType KalmanTracker::get_state()
{
	Mat s = kf.statePost;
	return get_rect_xysr(s.at<float>(0, 0), s.at<float>(1, 0), s.at<float>(2, 0), s.at<float>(3, 0));
}


// Convert bounding box from [cx,cy,s,r] to [x,y,w,h] style.
StateType KalmanTracker::get_rect_xysr(float cx, float cy, float s, float r)
{
	float w = sqrt(s * r);
	float h = s / w;
	float x = (cx - w / 2);
	float y = (cy - h / 2);

	if (x < 0 && cx > 0)
		x = 0;
	if (y < 0 && cy > 0)
		y = 0;

	return StateType(x, y, w, h);
}



/*
// --------------------------------------------------------------------
// Kalman Filter Demonstrating, a 2-d ball demo
// --------------------------------------------------------------------

const int winHeight = 600;
const int winWidth = 800;

Point mousePosition = Point(winWidth >> 1, winHeight >> 1);

// mouse event callback
void mouseEvent(int event, int x, int y, int flags, void *param)
{
	if (event == CV_EVENT_MOUSEMOVE) {
		mousePosition = Point(x, y);
	}
}

void TestKF();

void main()
{
	TestKF();
}


void TestKF()
{
	int stateNum = 4;
	int measureNum = 2;
	KalmanFilter kf = KalmanFilter(stateNum, measureNum, 0);

	// initialization
	Mat processNoise(stateNum, 1, CV_32F);
	Mat measurement = Mat::zeros(measureNum, 1, CV_32F);

	kf.transitionMatrix = *(Mat_<float>(stateNum, stateNum) <<
		1, 0, 1, 0,
		0, 1, 0, 1,
		0, 0, 1, 0,
		0, 0, 0, 1);

	setIdentity(kf.measurementMatrix);
	setIdentity(kf.processNoiseCov, Scalar::all(1e-2));
	setIdentity(kf.measurementNoiseCov, Scalar::all(1e-1));
	setIdentity(kf.errorCovPost, Scalar::all(1));

	randn(kf.statePost, Scalar::all(0), Scalar::all(winHeight));

	namedWindow("Kalman");
	setMouseCallback("Kalman", mouseEvent);
	Mat img(winHeight, winWidth, CV_8UC3);

	while (1)
	{
		// predict
		Mat prediction = kf.predict();
		Point predictPt = Point(prediction.at<float>(0, 0), prediction.at<float>(1, 0));

		// generate measurement
		Point statePt = mousePosition;
		measurement.at<float>(0, 0) = statePt.x;
		measurement.at<float>(1, 0) = statePt.y;

		// update
		kf.correct(measurement);

		// visualization
		img.setTo(Scalar(255, 255, 255));
		circle(img, predictPt, 8, CV_RGB(0, 255, 0), -1); // predicted point as green
		circle(img, statePt, 8, CV_RGB(255, 0, 0), -1); // current position as red

		imshow("Kalman", img);
		char code = (char)waitKey(100);
		if (code == 27 || code == 'q' || code == 'Q')
			break;
	}
	destroyWindow("Kalman");
}
*/