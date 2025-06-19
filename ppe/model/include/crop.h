#ifndef CROP_H
#define CROP_H

#include "detection.h"
#include <QObject>
#include <opencv2/opencv.hpp>
#include <string>
#include "person.h"


// Forward declaration - header'da torch include etmeyin
class DetectionModule;

class Crop : public QObject
{
    Q_OBJECT

public:
    explicit Crop(QObject *parent = nullptr);
    ~Crop();

    void triggerCrop(const cv::Mat &frame, cv::Rect roi, std::string trackId, Person &personInFrame);
    void triggerVestCrop(const cv::Mat &frame, cv::Rect roi, std::string trackId, Person &personInFrame, DetectionModule::Vest &vest);
    void triggerHumanCrop(const cv::Mat &frame, cv::Rect roi, std::string trackId, Person &personInFrame);
    void triggerIdCardCrop(const cv::Mat &frame, cv::Rect detected_roi, Person &personInFrame, DetectionModule::IdCard &idCard_obj);

private:
    DetectionModule* detectionModule;  // Pointer olarak tutun

public slots:
    void cropSlot(const cv::Mat &frame, cv::Rect detected_roi, std::string trackIdStr, Person &personInFrame);
    void vestCropSlot(const cv::Mat &frame, cv::Rect roi, std::string trackId, Person &personInFrame, DetectionModule::Vest &vest);
    void humanCropSlot(const cv::Mat &frame, cv::Rect roi, std::string trackId, Person &personInFrame);
    void idCardCropSlot(const cv::Mat &frame, cv::Rect roi, Person &personInFrame, DetectionModule::IdCard &idCard_obj);

signals:
    void cropSignal(const cv::Mat &frame, cv::Rect roi, std::string trackId, Person &personInFrame);
    void vestCropSignal(const cv::Mat &frame, cv::Rect roi, std::string trackId, Person &personInFrame, DetectionModule::Vest &vest);
    void humanCropSignal(const cv::Mat &frame, cv::Rect roi, std::string trackId, Person &personInFrame);
    void idCardCropSignal(const cv::Mat &frame, cv::Rect detected_roi, Person &personInFrame, DetectionModule::IdCard &idCard_obj);
};

#endif // CROP_H
