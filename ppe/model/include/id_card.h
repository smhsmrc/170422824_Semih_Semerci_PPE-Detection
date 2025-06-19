#ifndef ID_CARD_DETECTOR_H
#define ID_CARD_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <torch/script.h>
#include <vector>
#include <string>

class IdCardDetector {
public:
    // Detection yapısını tanımlayın
    struct Detection {
        int class_id;
        std::string className;
        float confidence;
        cv::Rect box;
        cv::Scalar color;
    };
    
    // Constructor ve Destructor
    IdCardDetector();
    ~IdCardDetector();

    std::string resultDetection;
    bool resultStatus = false;

    // Getter fonksiyonları
    std::string getResulDetection() const {
        return resultDetection;
    }

    bool getResulStatus() const {
        return resultStatus;
    }

    // Model yükleme fonksiyonu
    torch::jit::script::Module loadModel(const std::string &model_path);

    // Detection işlemini başlatan fonksiyon
    void runDetection(
        const cv::Mat &frame,
        torch::jit::script::Module &model,
        const std::vector<std::string> &class_names
    );

    // ROI (Region of Interest) çizim fonksiyonu
    void drawROI(const cv::Mat &frame);

    // ROI kontrol fonksiyonu
    void checkROI(const cv::Mat &frame, int x_center, int y_center, 
                  std::string trackIdStr, std::string confidenceStr, 
                  cv::Rect detected_roi);

private:
    torch::jit::script::Module model;
    torch::jit::script::Module modelLoaded;
};

#endif // ID_CARD_DETECTOR_H
