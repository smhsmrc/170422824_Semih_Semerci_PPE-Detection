#ifndef DETECTION_H
#define DETECTION_H
#include <torch/script.h>
#include <opencv2/opencv.hpp>

#include <vector>
#include <string>
#include <unordered_map>
#include "../include/person.h" // veya Person struct'ının bulunduğu dosya


class DetectionModule {
public:
    // Detection yapısını tanımlayın
    struct Detection {
        int class_id;
        std::string className;
        float confidence;
        cv::Rect box;
        cv::Scalar color;
    };

    struct OCRResult {
        std::string text;
        float confidence;
        int frame_id;
        std::string track_id;
    };  

    struct Vest{
        std::string track_id;
        cv::Rect box;
        cv::Mat cropped_vest;
        int id_card;
    };

    struct IdCard {
        cv::Rect box;
        cv::Mat cropped_id_card;
    };
    
    // Constructor ve Destructor
    DetectionModule();
    ~DetectionModule();
    
    // KKD durum bilgilerini almak için getter
    static Person getCurrentPerson();

    // Sonuç değişkenleri
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

    // Genel detection işlemini başlatan fonksiyon (frame döndürür)
    cv::Mat runDetection(
        const cv::Mat &frame,
        int frame_id,
        torch::jit::script::Module &model,
        const std::vector<std::string> &class_names,
        bool enable_ocr = true  // FPS optimizasyonu için OCR'ı kapatma seçeneği
    );

    // ID Card detection işlemini başlatan fonksiyon (void)
    void runIdCardDetection(
        const cv::Mat &frame,
        torch::jit::script::Module &model,
        const std::vector<std::string> &class_names
    );

    // ROI (Region of Interest) çizim fonksiyonu
    void drawROI(const cv::Mat &frame);

    // Genel ROI kontrol fonksiyonu (class_id parametreli)
    void checkROI(const cv::Mat &frame, int x_center, int y_center, 
                  std::string trackIdStr, std::string confidenceStr, 
                  cv::Rect detected_roi, int class_id);

    // ID Card ROI kontrol fonksiyonu (class_id parametresiz)
    void checkIdCardROI(const cv::Mat &frame, int x_center, int y_center, 
                        std::string trackIdStr, std::string confidenceStr, 
                        cv::Rect detected_roi);

private:
    torch::jit::script::Module model;
    torch::jit::script::Module modelLoaded;
    static std::unordered_map<std::string, std::vector<OCRResult>> trackOCRResults;
    static std::unordered_map<std::string, int> vestFirstDetectedFrame;
    static std::unordered_map<std::string, cv::Mat> vestCroppedImages;

    void performOCRAndStore(const cv::Mat& idCardImage, const std::string& trackId, int frameId);
    void updateBestOCRResult(const std::string& trackId, Person& personInFrame);
    void selectBestOCRResult(const std::string& trackId, Person& personInFrame);
    bool validateIDCard(const cv::Rect& idCardBox, const cv::Mat& vestImage, float confidence);
    

    torch::jit::script::Module id_card_model;
    // Detection için kullanılan map'ler
    std::unordered_map<std::string, bool> vestCroppedMap;
    std::unordered_map<std::string, bool> humanCroppedMap;
};

#endif // DETECTION_H
