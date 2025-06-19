#include "../../cpp_infer/include/paddleocr.h"
// #include "../../cpp_infer/include/paddlestructure.h"
#include "../include/detection.h"
#include "../include/crop.h"
#include <torchvision/ops/nms.h>
#include "TrackerFactory.h"
#include "inference.h"
#include <ctime>
#include <map>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "../include/progressbar.h"
#include "../include/person.h"
#include <gflags/gflags.h>
#include "../../ui/pages/include/settings.h"
// #include "../../ocr/include/OCRProcessor.h"
// #include "../../cpp_infer/include/ocr_rec.h"

// ✅ GLOBAL FLAGS DEĞİŞKENLERİ (dosyanın en başında)

// Static değişkenlerin tanımlanması (DetectionModule.cpp dosyasının başında)
std::unordered_map<std::string, std::vector<DetectionModule::OCRResult>> DetectionModule::trackOCRResults;
std::unordered_map<std::string, int> DetectionModule::vestFirstDetectedFrame;
std::unordered_map<std::string, cv::Mat> DetectionModule::vestCroppedImages;

// Global Person objesi tanımlaması
Person currentPersonInFrame;

// Model paths
std::string FLAGS_det_model_dir = "/Users/semihsemerci/tez/ocr_model/ch_PP-OCRv3_det_infer";
std::string FLAGS_rec_model_dir = "/Users/semihsemerci/tez/ocr_model/en_PP-OCRv3_rec_infer";
std::string FLAGS_cls_model_dir = "";  // Boş bırakın
std::string FLAGS_rec_char_dict_path = "/Users/semihsemerci/tez/ocr_model/en_dict.txt";

// Module flags
bool FLAGS_det = true;
bool FLAGS_rec = true; 
bool FLAGS_cls = false;
bool FLAGS_use_angle_cls = false;

// System settings
bool FLAGS_use_gpu = false;
int FLAGS_gpu_id = 0;
int FLAGS_gpu_mem = 4000;
int FLAGS_cpu_threads = 10;
bool FLAGS_enable_mkldnn = true;
bool FLAGS_use_tensorrt = false;
std::string FLAGS_precision = "fp32";

// Detection settings - Daha düşük threshold ile daha hassas detection
double FLAGS_det_db_thresh = 0.3;  // Düşürüldü
double FLAGS_det_db_box_thresh = 0.5; // Düşürüldü
double FLAGS_det_db_unclip_ratio = 1.5;
bool FLAGS_det_db_score_mode = true;
bool FLAGS_use_dilation = false;
std::string FLAGS_limit_type = "max";
int FLAGS_limit_side_len = 960;

// Recognition settings  
int FLAGS_rec_batch_num = 6;
int FLAGS_rec_img_h = 32;
int FLAGS_rec_img_w = 320;

// Classification settings
double FLAGS_cls_thresh = 0.9;
int FLAGS_cls_batch_num = 1;

// ✅ EKSİK OLAN ÖNEMLI FLAGS'LER
std::string FLAGS_image_dir = "";
std::string FLAGS_output_dir = "./output/";
bool FLAGS_benchmark = false;
bool FLAGS_save_results = false;
bool FLAGS_use_mp = false;
int FLAGS_total_process_num = 1;
int FLAGS_process_id = 0;
bool FLAGS_merge_no_span_structure = true;
std::string FLAGS_table_model_dir = "";
std::string FLAGS_table_char_dict_path = "";
int FLAGS_table_batch_num = 1;
int FLAGS_table_max_len = 488;
bool FLAGS_visualize = true;

// Constructor ve Destructor
DetectionModule::DetectionModule() {}
DetectionModule::~DetectionModule() {}

// KKD durum bilgilerini UI için al
Person DetectionModule::getCurrentPerson() {
    return currentPersonInFrame;
}

// Global değişkenler
static std::unordered_map<std::string, time_t> trackTimeMap;
static std::unordered_map<std::string, bool> triggeredMap;

// Random color generator for consistent class coloring
cv::Scalar getClassColor(int class_id) {
    static std::map<int, cv::Scalar> color_map;
    if (color_map.find(class_id) == color_map.end()) {
        cv::RNG rng(class_id);
        color_map[class_id] = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
    }
    return color_map[class_id];
}

// Mat to Tensor dönüşüm fonksiyonu
static torch::Tensor matToTensor(const cv::Mat &image) {
    cv::Mat rgb_image;
    cv::cvtColor(image, rgb_image, cv::COLOR_BGR2RGB);
    torch::Tensor tensor_image = torch::from_blob(rgb_image.data, {1, rgb_image.rows, rgb_image.cols, 3}, torch::kByte);
    tensor_image = tensor_image.permute({0, 3, 1, 2});
    tensor_image = tensor_image.toType(torch::kFloat).div(255);
    return tensor_image;
}

// Model yükleme fonksiyonu
torch::jit::script::Module DetectionModule::loadModel(const std::string &model_path) {
    torch::jit::script::Module model;
    try {
        model = torch::jit::load(model_path);
        // std::cout << "✅ Model yüklendi: " << model_path << std::endl;
    } catch (const c10::Error &e) {
        std::cerr << "❌ Model yüklenemedi: " << e.what() << std::endl;
        throw;
    }
    return model;
}

// ROI çizim fonksiyonu
void DetectionModule::drawROI(const cv::Mat &frame) {
    cv::Rect roi = cv::Rect(500, 500, 500, 300); 
    // cv::rectangle(frame, roi, cv::Scalar(0, 255, 0), 2);
}

// Box eşleştirme fonksiyonu
std::string findMatchingTrackId(const cv::Rect& detectionBox, 
                               const std::vector<std::pair<cv::Rect, std::string>>& trackBoxes) {
    for (const auto& trackPair : trackBoxes) {
        const cv::Rect& trackBox = trackPair.first;
        const std::string& trackId = trackPair.second;
        
        // IoU hesapla veya basit overlap kontrolü yap
        cv::Rect intersection = detectionBox & trackBox;
        float intersectionArea = intersection.area();
        float unionArea = detectionBox.area() + trackBox.area() - intersectionArea;
        
        if (unionArea > 0) {
            float iou = intersectionArea / unionArea;
            if (iou > 0.3) { // %30 overlap threshold
                return trackId;
            }
        }
    }
    return "";
}

// Global değişkenler - ID Card yakınlık uyarısı için
static std::map<std::string, int> smallCardWarningCount;
static std::map<std::string, bool> showProximityWarning;

// OCR yapıp sonucu sakla
void DetectionModule::performOCRAndStore(const cv::Mat& idCardImage, const std::string& trackId, int frameId) {
    try {
        // std::cout << "🔍 OCR başlatılıyor - Track ID: " << trackId << ", Frame: " << frameId << std::endl;
        
        // 📸 OCR giriş görüntüsünü kaydet (devre dışı - performans için)
        // std::string ocr_input_filename = "/Users/semihsemerci/tez/ocr_crops/ocr_input_track" + trackId + "_frame" + std::to_string(frameId) + ".jpg";
        // cv::imwrite(ocr_input_filename, idCardImage);
        // std::cout << "📸 OCR giriş görüntüsü kaydedildi: " << ocr_input_filename << std::endl;
        
        // OCR objesi oluştur (static yaparak performans artırabilirsiniz)
        static PaddleOCR::PPOCR ocr(
            "/Users/semihsemerci/tez/ocr_model/ch_PP-OCRv3_det_infer",
            "/Users/semihsemerci/tez/ocr_model/en_PP-OCRv3_rec_infer",
            "/Users/semihsemerci/tez/ocr_model/en_dict.txt",
            false, 10, true, true, true, false
        );
        
        // std::cout << "🔄 OCR işlemi başlatılıyor... (Görüntü boyutu: " << idCardImage.cols << "x" << idCardImage.rows << ")" << std::endl;
        
        // OCR için basit görüntü ön işleme
        cv::Mat processed_image;
        
        // Önce orijinal görüntüyü deneyelim
        processed_image = idCardImage.clone();
        
        // Sadece hafif bir kontrast artırma
        cv::cvtColor(processed_image, processed_image, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(processed_image, processed_image); // Histogram eşitleme
        cv::cvtColor(processed_image, processed_image, cv::COLOR_GRAY2BGR);
        
        
        // Önce işlenmiş görüntüyü dene
        std::vector<PaddleOCR::OCRPredictResult> ocr_results = ocr.ocr(processed_image, FLAGS_det, FLAGS_rec, FLAGS_cls);
        
        // std::cout << "🔍 İşlenmiş görüntü OCR tamamlandı - Bulunan sonuç sayısı: " << ocr_results.size() << std::endl;
        
        // Eğer sonuç bulunamazsa orijinal görüntüyü dene
        if (ocr_results.empty()) {
            // std::cout << "🔄 İşlenmiş görüntüde sonuç bulunamadı, orijinal görüntü deneniyor..." << std::endl;
            ocr_results = ocr.ocr(idCardImage, FLAGS_det, FLAGS_rec, FLAGS_cls);
            // std::cout << "🔍 Orijinal görüntü OCR tamamlandı - Bulunan sonuç sayısı: " << ocr_results.size() << std::endl;
        }
        
        // OCR sonuçlarını görüntü üzerine çiz ve kaydet
        if (!ocr_results.empty()) {
            cv::Mat result_image = processed_image.clone();
            for (size_t i = 0; i < ocr_results.size(); i++) {
                const auto& result = ocr_results[i];
                if (!result.text.empty()) {
                    // OCR bounding box'ını çiz
                    std::vector<cv::Point> points;
                    for (const auto& point : result.box) {
                        points.push_back(cv::Point(static_cast<int>(point[0]), static_cast<int>(point[1])));
                    }
                    if (points.size() == 4) {
                        cv::polylines(result_image, points, true, cv::Scalar(0, 0, 255), 2);
                        cv::putText(result_image, result.text, points[0], cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
                    }
                }
            }
        }
        
        // Tüm OCR sonuçlarını terminalde göster
        if (ocr_results.empty()) {
            std::cout << "❌ OCR sonucu bulunamadı!" << std::endl;
        } else {
            std::cout << "=== OCR SONUÇLARI (Track ID: " << trackId << ", Frame: " << frameId << ") ===" << std::endl;
            for (size_t i = 0; i < ocr_results.size(); i++) {
                const auto& result = ocr_results[i];
                std::cout << "  " << (i+1) << ". Text: '" << result.text 
                         << "', Confidence: " << std::fixed << std::setprecision(3) << result.score << std::endl;
            }
            std::cout << "================================================================" << std::endl;
        }
        
        // Her OCR sonucunu kaydet - gelişmiş filtreleme ile
        for (const auto& result : ocr_results) {
            if (!result.text.empty() && result.score > 0.3) { // Confidence threshold düşürüldü
                // Sadece sayısal değerleri kabul et (ID card numaraları için)
                std::string cleaned_text = result.text;
                // Boşlukları ve özel karakterleri temizle
                cleaned_text.erase(std::remove_if(cleaned_text.begin(), cleaned_text.end(), 
                    [](char c) { return !std::isdigit(c); }), cleaned_text.end());
                
                // En az 2 haneli sayı olmalı
                if (cleaned_text.length() >= 2 && cleaned_text.length() <= 6) {
                    OCRResult ocrResult;
                    ocrResult.text = cleaned_text;
                    ocrResult.confidence = result.score;
                    ocrResult.frame_id = frameId;
                    ocrResult.track_id = trackId;
                    
                    trackOCRResults[trackId].push_back(ocrResult);
                    
                                                            // Console spam'i azalt - sadece önemli sonuçları göster
                                        if (result.score > 0.7) { // Yüksek confidence'lı sonuçları göster
                                            std::cout << "✅ OCR: '" << cleaned_text << "' (C:" << std::fixed << std::setprecision(2) << result.score << ")" << std::endl;
                                        }
                } else {
                    std::cout << "⚠️ Geçersiz format OCR sonucu atlandı - Text: '" << result.text 
                             << "', Temizlenmiş: '" << cleaned_text << "' (2-6 hane arası olmalı)" << std::endl;
                }
            } else if (!result.text.empty()) {
                std::cout << "⚠️ Düşük confidence OCR sonucu atlandı - Text: '" << result.text 
                         << "', Confidence: " << std::fixed << std::setprecision(3) << result.score 
                         << " (Minimum: 0.3)" << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ OCR hatası: " << e.what() << std::endl;
    }
}

// ID Card geçerlilik kontrolü
bool DetectionModule::validateIDCard(const cv::Rect& idCardBox, const cv::Mat& vestImage, float confidence) {
    // 1. Confidence kontrolü (zaten 0.5'ten yüksek olmalı)
    if (confidence < 0.5) {
        std::cout << "❌ ID Card geçersiz: Düşük confidence (" << std::fixed << std::setprecision(2) << confidence << " < 0.5)" << std::endl;
        return false;
    }
    
    // 2. Minimum boyut kontrolü - daha esnek
    if (idCardBox.width < 15 || idCardBox.height < 15) {
        std::cout << "❌ ID Card geçersiz: Çok küçük boyut (" << idCardBox.width << "x" << idCardBox.height << " < 15x15)" << std::endl;
        return false;
    }
    
    // 3. Maksimum boyut kontrolü (vest'in yarısından büyük olmamalı)
    if (idCardBox.width > vestImage.cols * 0.8 || idCardBox.height > vestImage.rows * 0.8) {
        std::cout << "❌ ID Card geçersiz: Çok büyük boyut (" << idCardBox.width << "x" << idCardBox.height 
                  << " > " << static_cast<int>(vestImage.cols * 0.8) << "x" << static_cast<int>(vestImage.rows * 0.8) << ")" << std::endl;
        return false;
    }
    
    // 4. Aspect ratio kontrolü (ID card genellikle dikdörtgen) - daha esnek
    float aspect_ratio = static_cast<float>(idCardBox.width) / static_cast<float>(idCardBox.height);
    if (aspect_ratio < 0.5 || aspect_ratio > 5.0) { // Daha geniş aralık
        std::cout << "❌ ID Card geçersiz: Yanlış aspect ratio (" << std::fixed << std::setprecision(2) << aspect_ratio 
                  << " - 0.5 ile 5.0 arası olmalı)" << std::endl;
        return false;
    }
    
    // 5. Vest içinde olma kontrolü
    if (idCardBox.x < 0 || idCardBox.y < 0 || 
        idCardBox.x + idCardBox.width > vestImage.cols || 
        idCardBox.y + idCardBox.height > vestImage.rows) {
        std::cout << "❌ ID Card geçersiz: Vest sınırları dışında (Box: " << idCardBox.x << "," << idCardBox.y 
                  << "," << idCardBox.width << "," << idCardBox.height << " - Vest: " << vestImage.cols << "x" << vestImage.rows << ")" << std::endl;
        return false;
    }
    
    // 6. Alan kontrolü (çok küçük alanları filtrele) - daha esnek
    int area = idCardBox.width * idCardBox.height;
    if (area < 200) { // 200 piksel minimum alan
        std::cout << "❌ ID Card geçersiz: Çok küçük alan (" << area << " < 200 piksel)" << std::endl;
        return false;
    }
    
    std::cout << "✅ ID Card geçerli: " << idCardBox.width << "x" << idCardBox.height 
              << ", Confidence: " << std::fixed << std::setprecision(2) << confidence 
              << ", Aspect ratio: " << std::fixed << std::setprecision(2) << aspect_ratio 
              << ", Alan: " << area << " piksel" << std::endl;
    return true;
}

// Anlık en iyi OCR sonucunu güncelle (her OCR sonrasında çağrılır)
void DetectionModule::updateBestOCRResult(const std::string& trackId, Person& personInFrame) {
    if (trackOCRResults.find(trackId) == trackOCRResults.end() || trackOCRResults[trackId].empty()) {
        return; // Henüz OCR sonucu yok
    }
    
    // En yüksek confidence'a sahip sonucu bul
    OCRResult bestResult;
    bestResult.confidence = 0.0f;
    
    for (const auto& result : trackOCRResults[trackId]) {
        if (result.confidence > bestResult.confidence) {
            bestResult = result;
        }
    }
    
    if (bestResult.confidence > 0.0f) {
        try {
            // Sadece sayısal değerleri kabul et
            if (std::all_of(bestResult.text.begin(), bestResult.text.end(), ::isdigit)) {
                int newCardId = std::stoi(bestResult.text);
                
                // Eğer yeni bir card ID bulunduysa veya daha yüksek confidence'lı bir sonuç varsa güncelle
                if (personInFrame.cardId != newCardId || personInFrame.cardId == -1) {
                    personInFrame.cardId = newCardId;
                    personInFrame.card_status = true;
                    
                    std::cout << "🎯 CARD ID: " << personInFrame.cardId << " (C:" << std::fixed << std::setprecision(2) << bestResult.confidence << ")" << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cout << "❌ Card ID parse hatası: '" << bestResult.text << "' - " << e.what() << std::endl;
        }
    }
}

// En iyi OCR sonucunu seç (100 frame sonunda çağrılır)
void DetectionModule::selectBestOCRResult(const std::string& trackId, Person& personInFrame) {
    if (trackOCRResults.find(trackId) == trackOCRResults.end() || trackOCRResults[trackId].empty()) {
        // Console spam'i azalt - sessiz başarısızlık
        // std::cout << "❌ Track ID " << trackId << " için OCR sonucu bulunamadı!" << std::endl;
        personInFrame.card_status = false;
        return;
    }
    
    std::cout << "\n=== OCR SONUÇLARI ANALİZİ ===" << std::endl;
    std::cout << "Track ID: " << trackId << std::endl;
    std::cout << "Toplam OCR denemesi: " << trackOCRResults[trackId].size() << std::endl;
    
    // Tüm sonuçları listele
    for (size_t i = 0; i < trackOCRResults[trackId].size(); i++) {
        const auto& result = trackOCRResults[trackId][i];
        std::cout << "  " << (i+1) << ". Text: '" << result.text 
                 << "', Confidence: " << std::fixed << std::setprecision(3) << result.confidence 
                 << ", Frame: " << result.frame_id << std::endl;
    }
    
    // En yüksek confidence'a sahip sonucu bul
    OCRResult bestResult;
    bestResult.confidence = 0.0f;
    
    for (const auto& result : trackOCRResults[trackId]) {
        if (result.confidence > bestResult.confidence) {
            bestResult = result;
        }
    }
    
    if (bestResult.confidence > 0.0f) {
        try {
            // Sadece sayısal değerleri kabul et
            if (std::all_of(bestResult.text.begin(), bestResult.text.end(), ::isdigit)) {
                personInFrame.cardId = std::stoi(bestResult.text);
                personInFrame.card_status = true;
                std::cout << "🏆 EN İYİ OCR SONUCU SEÇİLDİ:" << std::endl;
                std::cout << "  - Track ID: " << trackId << std::endl;
                std::cout << "  - Card ID: " << personInFrame.cardId << std::endl;
                std::cout << "  - Confidence: " << std::fixed << std::setprecision(3) << bestResult.confidence << std::endl;
                std::cout << "  - Frame: " << bestResult.frame_id << std::endl;
            } else {
                std::cout << "❌ En iyi sonuç sayısal değil: '" << bestResult.text << "'" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "❌ Card ID parse hatası: '" << bestResult.text << "' - " << e.what() << std::endl;
        }
    }
    
    std::cout << "==============================\n" << std::endl;
    
    // Belleği temizle
    trackOCRResults.erase(trackId);
    vestFirstDetectedFrame.erase(trackId);
    vestCroppedImages.erase(trackId);
}

// Genel detection fonksiyonu (frame döndürür)
cv::Mat DetectionModule::runDetection(
    const cv::Mat &frame,
    int frame_id,
    torch::jit::script::Module &model,
    const std::vector<std::string> &class_names,
    bool enable_ocr
) { 
    // QObject::connect(&crop_obj, &Crop::cropSignal, &crop_obj, &Crop::cropSlot);
    Crop crop_obj;
    QObject::connect(&crop_obj, &Crop::vestCropSignal, &crop_obj, &Crop::vestCropSlot);
    QObject::connect(&crop_obj, &Crop::humanCropSignal, &crop_obj, &Crop::humanCropSlot);
    QObject::connect(&crop_obj, &Crop::idCardCropSignal, &crop_obj, &Crop::idCardCropSlot);

    // static Person personInFrame; // Artık global kullanıyoruz
    Person& personInFrame = currentPersonInFrame; // Global referansı
    cv::Mat result_frame = frame.clone(); // Frame'i kopyala

    DetectionModule::Vest vest_obj;
    DetectionModule::IdCard idCard_obj;
    
    cv::Mat img_resized;
    cv::resize(result_frame, img_resized, cv::Size(640, 640));
    auto img_tensor = matToTensor(img_resized);
    auto out_tuple = model.forward({img_tensor}).toTuple();
    auto out_tensor = out_tuple->elements()[0].toTensor();

    torch::Tensor detections = out_tensor[0];
    std::vector<torch::Tensor> boxes;
    std::vector<float> confidences;
    std::vector<int> class_ids;

    for (int i = 0; i < detections.size(0); i++) {
        float confidence = detections[i][4].item<float>();
        if (confidence > 0.4) {
            float center_x = detections[i][0].item<float>();
            float center_y = detections[i][1].item<float>();
            float width = detections[i][2].item<float>();
            float height = detections[i][3].item<float>();

            int class_id = 0;
            float max_class_confidence = 0.0f;
            for (int j = 5; j < detections.size(1); j++) {
                float class_conf = detections[i][j].item<float>();
                if (class_conf > max_class_confidence) {
                    max_class_confidence = class_conf;
                    class_id = j - 5;
                }
            }
            
            // ✅ Class ID debug - yanlış sınıflandırmaları tespit et (yorum)
            // if (frame_id % 60 == 0) { // Her 60 frame'de bir debug (daha az spam)
            //     std::cout << "🎯 DETECTION DEBUG (Frame " << frame_id << "):" << std::endl;
            //     std::cout << "   Class ID: " << class_id << " (" << (class_id < class_names.size() ? class_names[class_id] : "Unknown") << ")" << std::endl;
            //     std::cout << "   Confidence: " << confidence << std::endl;
            //     std::cout << "   Max Class Conf: " << max_class_confidence << std::endl;
            // }

            int left = static_cast<int>(std::max(0.0f, center_x - width / 2));
            int top = static_cast<int>(std::max(0.0f, center_y - height / 2));
            int right = static_cast<int>(std::min(static_cast<float>(result_frame.cols - 1), center_x + width / 2));
            int bottom = static_cast<int>(std::min(static_cast<float>(result_frame.rows - 1), center_y + height / 2));

            boxes.push_back(torch::tensor({left, top, right, bottom}, torch::kFloat));
            confidences.push_back(confidence);
            class_ids.push_back(class_id);
        }
    }

    std::vector<inference::Detection> detectionResult;
    
    // ✅ Tracker'ı static yaparak state'ini koru, ama gerektiğinde reset et
    static std::unique_ptr<ITracker> myTracker = TrackerFactory::createTracker(TrackerType::Sort);
    static bool trackerNeedsReset = false;
    
    // ✅ Tracker reset kontrolü
    if (trackerNeedsReset) {
        std::cout << "🔄 TRACKER RESET EDİLİYOR - Class karışıklığını önlemek için" << std::endl;
        myTracker.reset(); // Eski tracker'ı yok et
        myTracker = TrackerFactory::createTracker(TrackerType::Sort); // Yeni tracker oluştur
        trackerNeedsReset = false;
        
        // ✅ Debug değişkenlerini de temizle (static değişkenler dışarıda tanımlı)
        // Bu değişkenler tracking loop içinde static olarak tanımlı, orada temizlenecek
        
        std::cout << "✅ Tracker başarıyla reset edildi - Yeni cycle başlıyor" << std::endl;
    }

    // Track box'larını saklamak için vector
    std::vector<std::pair<cv::Rect, std::string>> trackBoxes;

    // 🔄 İnsan tespit kontrolü için flag
    bool humanDetected = false;
    
    if (!boxes.empty()) {
        auto boxes_tensor = torch::stack(boxes);
        auto confidences_tensor = torch::tensor(confidences);
        auto keep_indices = vision::ops::nms(boxes_tensor, confidences_tensor, 0.4);

        float x_scale = static_cast<float>(result_frame.cols) / img_resized.cols;
        float y_scale = static_cast<float>(result_frame.rows) / img_resized.rows;

        for (int i = 0; i < keep_indices.size(0); ++i) {
            int idx = keep_indices[i].item<int>();
            auto box = boxes_tensor[idx].data_ptr<float>();

            int scaled_left = static_cast<int>(box[0] * x_scale);
            int scaled_top = static_cast<int>(box[1] * y_scale);
            int scaled_right = static_cast<int>(box[2] * x_scale);
            int scaled_bottom = static_cast<int>(box[3] * y_scale);

            int class_id = class_ids[idx];
            auto class_color = getClassColor(class_id);

            inference::Detection detection;
            detection.class_id = class_id;
            detection.className = class_names[class_id];
            detection.confidence = confidences[idx];
            detection.box = cv::Rect(scaled_left, scaled_top, scaled_right - scaled_left, scaled_bottom - scaled_top);
            detection.color = class_color;
            detectionResult.push_back(detection);
        }

        std::vector<float> result = myTracker->update(detectionResult, result_frame);

        // Track sonuçlarını işle
        for (size_t i = 0; i < result.size() / 7; i++) {
            float x = result[0 + i * 7];
            float y = result[1 + i * 7];
            float width = result[2 + i * 7];
            float height = result[3 + i * 7];
            float confidence = result[4 + i * 7];
            float classId = result[5 + i * 7];
            float trackId = result[6 + i * 7];

            float x_center = x + (width / 2);
            float y_center = y + (height / 2);

            int track_id_int = static_cast<int>(trackId);
            int class_id_int = static_cast<int>(classId);
            std::string trackIdStr = std::to_string(track_id_int);
            std::string confidenceStr = std::to_string(confidence).substr(0, 5);
            
            cv::Rect detected_roi = cv::Rect(x, y, width, height);
            
            // ✅ Track ID ve Class ID debug çıktısı - GELİŞTİRİLMİŞ
            static std::map<int, std::set<int>> trackIdToClasses;
            static std::map<int, int> trackIdFrameCount; // Track ID'nin kaç frame görüldüğü
            
            trackIdToClasses[track_id_int].insert(class_id_int);
            trackIdFrameCount[track_id_int]++;
            
            // ✅ CLASS KARIŞTIRMA TESPİTİ
            if (trackIdToClasses[track_id_int].size() > 1) {
                std::cout << "🚨 KRİTİK: TRACK ID CLASS KARIŞMASI TESPİT EDİLDİ!" << std::endl;
                std::cout << "   Track ID " << track_id_int << " şu class'larda görüldü: ";
                for (int cls : trackIdToClasses[track_id_int]) {
                    std::string className = (cls < class_names.size()) ? class_names[cls] : "Unknown";
                    std::cout << cls << "(" << className << ") ";
                }
                std::cout << std::endl;
                std::cout << "   Bu track ID " << trackIdFrameCount[track_id_int] << " frame'dir takip ediliyor" << std::endl;
                
                // ✅ Eğer karışıklık 5 frame'den fazla devam ederse tracker'ı reset et
                if (trackIdFrameCount[track_id_int] > 5) {
                    std::cout << "🔄 KARIŞIKLIK ÇÖZÜLMÜYOR - Tracker reset edilecek" << std::endl;
                    trackerNeedsReset = true;
                    trackIdToClasses.clear();
                    trackIdFrameCount.clear();
                }
            }
            
            // Track box'ını kaydet
            trackBoxes.push_back(std::make_pair(detected_roi, trackIdStr));
            
            // cv::circle(result_frame, cv::Point(x_center, y_center), 5, cv::Scalar(0, 0, 255), 5);
            
            // checkROI fonksiyonuna class_id parametresi de eklendi
            // checkROI(result_frame, x_center, y_center, trackIdStr, confidenceStr, detected_roi, class_id_int);

            if (classId == 2) {
                humanDetected = true; // ✅ İnsan tespit edildi!
                
                if (personInFrame.trackerId != trackId) {
                    std::cout << "🆕 YENİ KİŞİ TESPİT EDİLDİ - Track ID: " << trackId << std::endl;
                    personInFrame.frame = result_frame.clone();
                    personInFrame.detected_roi = detected_roi;
                    personInFrame.trackerId = trackId;
                    personInFrame.detectedFrameId = frame_id;
                    personInFrame.gloves = false;
                    personInFrame.helmet = false;
                    personInFrame.mask = false;
                    personInFrame.vest = false;
                    personInFrame.notified = false;
                    personInFrame.cardId = -1;
                    personInFrame.card_status = false;
                }
            } else {
                switch (int(classId)) {
                    case 0: 
                        personInFrame.gloves = true; 
                        break;
                    case 1: 
                        personInFrame.helmet = true; 
                        break;
                    case 3: 
                        personInFrame.mask = true; 
                        break;
                    case 4: // Vest detection
                    {
                        // Global yapılar (dosyanın başında tanımlanmalı)
                        static std::unordered_map<std::string, std::vector<OCRResult>> trackOCRResults;
                        static std::unordered_map<std::string, int> vestFirstDetectedFrame;
                        static std::unordered_map<std::string, cv::Mat> vestCroppedImages;
                        static std::unordered_map<std::string, int> lastCropFrame; // ✅ Track ID -> Son crop yapılan frame
                        
                        std::string trackIdStr = std::to_string(static_cast<int>(trackId));
                        
                        // İlk kez bu vest track ID'si görüldüğünde frame'i kaydet
                        if (vestFirstDetectedFrame.find(trackIdStr) == vestFirstDetectedFrame.end()) {
                            vestFirstDetectedFrame[trackIdStr] = frame_id;
                            lastCropFrame[trackIdStr] = frame_id - 5; // ✅ İlk frame'de crop yapılabilmesi için (5 frame)
                            // std::cout << "🆕 Yeni vest track ID kaydedildi: " << trackIdStr << " (Frame: " << frame_id << ")" << std::endl;
                        }
                        
                        // Bu vest için henüz card ID bulunmamışsa işleme devam et
                        if (personInFrame.cardId == -1) {
                            int firstFrame = vestFirstDetectedFrame[trackIdStr];
                            int currentFrameDiff = frame_id - firstFrame;
                            
                            // 100 frame içindeyse işleme devam et
                            if (currentFrameDiff < 100) {
                                                                        // ✅ CROP frame kontrolü - 5 frame'de bir crop al (daha sık)
                                        int framesSinceLastCrop = frame_id - lastCropFrame[trackIdStr];
                                        bool shouldPerformCrop = framesSinceLastCrop >= 5;
                                
                                // ✅ OCR frame kontrolü - crop alındığı frame'de OCR yap (aynı anda)
                                bool shouldPerformOCR = shouldPerformCrop; // Crop alındığında OCR da yap
                                
                                if (shouldPerformCrop && shouldPerformOCR) { // enable_ocr kontrolünü kaldır
                                                                                            // Console çıktısını azalt - performans için
                                                        // std::cout << "🔄 CROP ve OCR yapılacak (5 frame aralıkla) - Track ID: " << trackIdStr 
                                                        //          << ", Frame: " << frame_id 
                                                        //          << ", Frame Diff: " << currentFrameDiff 
                                                        //          << ", Son crop'tan bu yana: " << framesSinceLastCrop << " frame" << std::endl;
                                    
                                    // ✅ Vest crop al (5 frame'de bir - daha sık)
                                    vest_obj.track_id = trackIdStr;
                                    vest_obj.box = detected_roi;
                                    
                                    std::cout << "🎯 VEST CROP İŞLEMİ (5 frame aralıkla):" << std::endl;
                                    std::cout << "   Mevcut frame boyutu: " << result_frame.cols << "x" << result_frame.rows << std::endl;
                                    std::cout << "   Vest ROI: x=" << detected_roi.x << ", y=" << detected_roi.y 
                                             << ", w=" << detected_roi.width << ", h=" << detected_roi.height << std::endl;
                                    
                                    // ✅ DOĞRU: Mevcut frame'i kullan, eski frame değil!
                                    crop_obj.triggerVestCrop(result_frame, vest_obj.box, vest_obj.track_id, personInFrame, vest_obj);
                                    
                                    // ✅ Son crop frame'ini güncelle (5 frame aralıkla)
                                    lastCropFrame[trackIdStr] = frame_id;
                                    
                                    if (!vest_obj.cropped_vest.empty()) {
                                                                // 📸 Vest crop'unu kaydet (devre dışı - performans için)
                        // std::string vest_crop_filename = "/Users/semihsemerci/tez/ocr_crops/vest_crop_track" + trackIdStr + "_frame" + std::to_string(frame_id) + ".jpg";
                        // cv::imwrite(vest_crop_filename, vest_obj.cropped_vest);
                                        // std::cout << "📸 Vest crop kaydedildi: " << vest_crop_filename << std::endl;
                                        
                                        // Vest görüntüsünü sakla
                                        vestCroppedImages[trackIdStr] = vest_obj.cropped_vest.clone();
                                        
                                        // ID Card detection
                                        // ✅ DOĞRU: Vest crop'unu direkt kullan (kopya alma)
                                        cv::Mat& vest_frame = vest_obj.cropped_vest;  // Referans kullan
                                        cv::Mat img_vest_resized;
                                        
                                        std::cout << "🎯 Vest frame boyutu (ID card detection için): " 
                                                 << vest_frame.cols << "x" << vest_frame.rows << std::endl;
                                        
                                        // ID card modelini yükle (sadece ilk seferde)
                                        static torch::jit::script::Module id_card_model;
                                        static bool model_loaded = false;
                                        if (!model_loaded) {
                                            id_card_model = loadModel("../detection_model/id_card_detection.torchscript");
                                            model_loaded = true;
                                        }
                                        
                                        cv::resize(vest_frame, img_vest_resized, cv::Size(640, 640));
                                        auto img_vest_tensor = matToTensor(img_vest_resized);
                                        auto out_vest_tuple = id_card_model.forward({img_vest_tensor}).toTuple();
                                        auto out_vest_tensor = out_vest_tuple->elements()[0].toTensor();
                                        
                                        torch::Tensor id_card_detections = out_vest_tensor[0];
                                        std::vector<torch::Tensor> vest_boxes;
                                        std::vector<float> vest_confidences;
                                        
                                        // ID Card detection işlemi - Ana detection ile tutarlı hale getirildi
                                        for (int i = 0; i < id_card_detections.size(0); i++) {
                                            float vest_confidence = id_card_detections[i][4].item<float>();
                                            if (vest_confidence > 0.5) { // Threshold artırıldı false positive'leri azaltmak için
                                                // ✅ Ana detection ile aynı format - model çıktısı direkt 640x640 koordinatlarında
                                                float vest_center_x = id_card_detections[i][0].item<float>();
                                                float vest_center_y = id_card_detections[i][1].item<float>();
                                                float vest_width = id_card_detections[i][2].item<float>();
                                                float vest_height = id_card_detections[i][3].item<float>();
                                                
                                                // ✅ Ana detection ile aynı işlem - Center-based koordinatları corner-based koordinatlara dönüştür
                                                int vest_left = static_cast<int>(std::max(0.0f, vest_center_x - vest_width / 2));
                                                int vest_top = static_cast<int>(std::max(0.0f, vest_center_y - vest_height / 2));
                                                int vest_right = static_cast<int>(std::min(640.0f - 1, vest_center_x + vest_width / 2));
                                                int vest_bottom = static_cast<int>(std::min(640.0f - 1, vest_center_y + vest_height / 2));
                                                
                                                // Debug çıktıları kaldırıldı
                                                
                                                vest_boxes.push_back(torch::tensor({vest_left, vest_top, vest_right, vest_bottom}, torch::kFloat));
                                                vest_confidences.push_back(vest_confidence);
                                            }
                                        }
                                        
                                        // ID Card bulunduysa OCR yap
                                        if (!vest_boxes.empty()) {
                                            auto vest_boxes_tensor = torch::stack(vest_boxes);
                                            auto vest_confidences_tensor = torch::tensor(vest_confidences);
                                            auto vest_keep_indices = vision::ops::nms(vest_boxes_tensor, vest_confidences_tensor, 0.4);
                                            
                                            // ✅ DOĞRU: Aynı frame referansını kullan
                                            float vest_x_scale = static_cast<float>(vest_frame.cols) / img_vest_resized.cols;
                                            float vest_y_scale = static_cast<float>(vest_frame.rows) / img_vest_resized.rows;
                                            
                                                                        // std::cout << "🔍 Ölçeklendirme faktörleri:" << std::endl;
                            // std::cout << "   X scale: " << vest_x_scale << " (" << vest_frame.cols << "/640)" << std::endl;
                            // std::cout << "   Y scale: " << vest_y_scale << " (" << vest_frame.rows << "/640)" << std::endl;
                                            
                                            // Debug çıktıları kaldırıldı
                                            
                                            for (int i = 0; i < vest_keep_indices.size(0); ++i) {
                                                int vest_idx = vest_keep_indices[i].item<int>();
                                                auto vest_box = vest_boxes_tensor[vest_idx].data_ptr<float>();
                                                float current_vest_confidence = vest_confidences[vest_idx]; // Confidence değerini al
                                                
                                                // Debug çıktıları kaldırıldı
                                                
                                                int vest_scaled_left = static_cast<int>(vest_box[0] * vest_x_scale);
                                                int vest_scaled_top = static_cast<int>(vest_box[1] * vest_y_scale);
                                                int vest_scaled_right = static_cast<int>(vest_box[2] * vest_x_scale);
                                                int vest_scaled_bottom = static_cast<int>(vest_box[3] * vest_y_scale);
                                
                                                // Koordinatları düzelt
                                                int final_left = std::min(vest_scaled_left, vest_scaled_right);
                                                int final_right = std::max(vest_scaled_left, vest_scaled_right);
                                                int final_top = std::min(vest_scaled_top, vest_scaled_bottom);
                                                int final_bottom = std::max(vest_scaled_top, vest_scaled_bottom);

                                                // Debug çıktıları kaldırıldı

                                                cv::Rect id_card_box = cv::Rect(final_left, final_top, 
                                                                            final_right - final_left, 
                                                                            final_bottom - final_top);
                                                
                                                // Debug: ID card koordinatlarını yazdır (yorum)
                                                // std::cout << "🔍 ID Card koordinatları (vest içindeki):" << std::endl;
                                                // std::cout << "   Vest boyutu: " << vest_obj.cropped_vest.cols << "x" << vest_obj.cropped_vest.rows << std::endl;
                                                // std::cout << "   ID Card Box: x=" << id_card_box.x << ", y=" << id_card_box.y 
                                                //          << ", w=" << id_card_box.width << ", h=" << id_card_box.height << std::endl;
                                                
                                                // ID Card geçerlilik kontrolü
                                                bool is_valid_id_card = validateIDCard(id_card_box, vest_obj.cropped_vest, current_vest_confidence);
                                                
                                                // Sadece geçerli ID card'lar için OCR yap
                                                if (is_valid_id_card) {
                                                    std::cout << "✅ Geçerli ID card bulundu - Crop işlemi başlatılıyor" << std::endl;
                                                    
                                                    // ✅ ID Card crop al - aynı frame referansını kullan
                                                    crop_obj.triggerIdCardCrop(vest_frame, id_card_box, personInFrame, idCard_obj);

                                                    // 📸 ID Card crop'unu kaydet (devre dışı - performans için)
                                                    if (!idCard_obj.cropped_id_card.empty()) {
                                                        // std::string id_card_filename = "/Users/semihsemerci/tez/ocr_crops/id_card_crop_track" + trackIdStr + "_frame" + std::to_string(frame_id) + ".jpg";
                                                        // cv::imwrite(id_card_filename, idCard_obj.cropped_id_card);
                                                        // std::cout << "📸 ID Card crop kaydedildi: " << id_card_filename << std::endl;
                                                    }
                                                    
                                                    if (!idCard_obj.cropped_id_card.empty()) {
                                                        // Geçerli ID card crop'unu kaydet (devre dışı - performans için)
                                                        // std::string valid_id_filename = "/Users/semihsemerci/tez/valid_id_card_track" + trackIdStr + "_frame" + std::to_string(frame_id) + ".jpg";
                                                        // cv::imwrite(valid_id_filename, idCard_obj.cropped_id_card);
                                                        // std::cout << "💾 Geçerli ID card crop kaydedildi: " << valid_id_filename << std::endl;
                                                        
                                                        // std::cout << "✅ ID Card crop başarılı - Boyut: " << idCard_obj.cropped_id_card.cols
                                                        //          << "x" << idCard_obj.cropped_id_card.rows << std::endl;
                                                        
                                                                                                                    // Minimum boyut kontrolü - daha esnek
                                                            if (idCard_obj.cropped_id_card.rows >= 15 && idCard_obj.cropped_id_card.cols >= 30) {
                                                                // ID Card boyutunu kontrol et - küçükse uyarı ver
                                                                
                                                                int cardArea = idCard_obj.cropped_id_card.rows * idCard_obj.cropped_id_card.cols;
                                                                bool isCardTooSmall = (cardArea < 2000); // 2000 piksel altı küçük sayılır
                                                                
                                                                if (isCardTooSmall) {
                                                                    smallCardWarningCount[trackIdStr]++;
                                                                    if (smallCardWarningCount[trackIdStr] >= 3) { // 3 kez küçük tespit edilirse uyarı göster
                                                                        showProximityWarning[trackIdStr] = true;
                                                                        // Console çıktısını azalt - performans için
                                                                        // std::cout << "⚠️ ID Card çok küçük - Kameraya yaklaşın! Boyut: " 
                                                                        //          << idCard_obj.cropped_id_card.cols << "x" << idCard_obj.cropped_id_card.rows 
                                                                        //          << " (Alan: " << cardArea << " piksel)" << std::endl;
                                                                    }
                                                                } else {
                                                                    // Kart yeterince büyükse uyarıyı kapat
                                                                    smallCardWarningCount[trackIdStr] = 0;
                                                                    showProximityWarning[trackIdStr] = false;
                                                                }
                                                            
                                                            // Görüntüyü OCR için uygun boyuta getir - daha büyük boyutlandırma
                                                            cv::Mat resized_id_card;
                                                            int target_height = std::max(48, idCard_obj.cropped_id_card.rows * 3); // 3x büyütme
                                                            int target_width = std::max(150, idCard_obj.cropped_id_card.cols * 3); // 3x büyütme
                                                            
                                                            cv::resize(idCard_obj.cropped_id_card, resized_id_card, 
                                                                      cv::Size(target_width, target_height), 0, 0, cv::INTER_CUBIC);
                                                            
                                                            // 📸 Yeniden boyutlandırılmış OCR görüntüsünü kaydet (devre dışı - performans için)
                                                            // std::string resized_ocr_filename = "/Users/semihsemerci/tez/ocr_crops/resized_for_ocr_track" + trackIdStr + "_frame" + std::to_string(frame_id) + ".jpg";
                                                            // cv::imwrite(resized_ocr_filename, resized_id_card);
                                                            // std::cout << "📸 OCR için boyutlandırılmış görüntü kaydedildi: " << resized_ocr_filename << std::endl;
                                                            
                                                            // std::cout << "🔍 Görüntü OCR için yeniden boyutlandırıldı: " 
                                                            //          << target_width << "x" << target_height << std::endl;
                                                            
                                                            performOCRAndStore(resized_id_card, trackIdStr, frame_id);
                                                            // lastOCRFrame artık gerekli değil, crop ile aynı anda yapılıyor
                                                        
                                                            // Her OCR sonrasında anlık en iyi sonucu kontrol et
                                                            updateBestOCRResult(trackIdStr, personInFrame);
                                                        } else {
                                                            std::cout << "⚠️ ID Card çok küçük OCR için uygun değil - Boyut: " 
                                                                     << idCard_obj.cropped_id_card.cols << "x" << idCard_obj.cropped_id_card.rows 
                                                                     << " (Minimum: 30x15)" << std::endl;
                                                        }
                                                    }
                                                } else {
                                                    // Debug çıktıları ve frame çizimleri kaldırıldı
                                                }
                                            }
                                        } else {
                                            // Debug çıktıları ve frame çizimleri kaldırıldı
                                        }
                                    }
                                } else {
                                    // Crop veya OCR yapılmayacak frame'lerde sadece bilgi ver (isteğe bağlı)
                                    // std::cout << "⏭️ Crop/OCR atlandı - Track ID: " << trackIdStr 
                                    //         << ", Frame: " << frame_id 
                                    //         << ", Son crop'tan bu yana: " << framesSinceLastCrop << " frame"
                                    //         << ", Son OCR'dan bu yana: " << framesSinceLastOCR << " frame" << std::endl;
                                }
                            } else if (currentFrameDiff >= 100) {
                                // 100 frame tamamlandı, en iyi OCR sonucunu seç
                                // std::cout << "⏰ 100 frame tamamlandı - En iyi OCR sonucu seçiliyor..." << std::endl;
                                selectBestOCRResult(trackIdStr, personInFrame);
                            }
                        }
                        
                        personInFrame.vest = true;
                        break;
                    }
                    default: 
                        break; // Diğer sınıflar için işlem yapma
                }
            }
        }

        // Tracking sonuçlarını direkt kullanarak çiz (her obje için unique track ID)
        for (size_t i = 0; i < result.size() / 7; i++) {
            float x = result[0 + i * 7];
            float y = result[1 + i * 7];
            float width = result[2 + i * 7];
            float height = result[3 + i * 7];
            float confidence = result[4 + i * 7];
            float classId = result[5 + i * 7];
            float trackId = result[6 + i * 7];
            
            int class_id_int = static_cast<int>(classId);
            int track_id_int = static_cast<int>(trackId);
            
            // Class name'i al
            std::string className = (class_id_int < class_names.size()) ? class_names[class_id_int] : "Unknown";
            
            // Bounding box çiz
            cv::Rect trackBox(x, y, width, height);
            cv::Scalar color = getClassColor(class_id_int);
            cv::rectangle(result_frame, trackBox, color, 2);
            
            // KKD isimlerini Türkçe yap
            std::string turkishName;
            switch(class_id_int) {
                case 0: turkishName = "Eldiven"; break;
                case 1: turkishName = "Kask"; break;
                case 2: turkishName = "Kisi"; break;
                case 3: turkishName = "Maske"; break;
                case 4: turkishName = "Yelek"; break;
                default: turkishName = className; break;
            }
            
            // Label oluştur (Türkçe isim + confidence)
            std::string label = turkishName + " " + std::to_string(static_cast<int>(confidence * 100)) + "%";
            
            // Renk seç
            cv::Scalar labelColor = color;
            
            cv::putText(result_frame, label, cv::Point(x, y - 5),
                        cv::FONT_HERSHEY_SIMPLEX, 1, labelColor, 2);
        }

        int frame_width = result_frame.cols / 4;
        int frame_height = result_frame.rows;
        // Progress bar kaldırıldı - artık Qt UI'da var
        // Tüm OpenCV panelleri kaldırıldı - artık Qt UI'da var
        
        // Crop trigger'ını koruyalım (algoritma için gerekli)
        if ((frame_id - personInFrame.detectedFrameId) >= 100) { // 100 frame tamamlandığında
            if (!personInFrame.isFullyProtected() && !personInFrame.notified) {
                // ROI'yi frame sınırlarına göre düzelt
                cv::Rect corrected_roi = personInFrame.detected_roi;
                corrected_roi.x = std::max(0, corrected_roi.x);
                corrected_roi.y = std::max(0, corrected_roi.y);
                corrected_roi.width = std::min(corrected_roi.width, result_frame.cols - corrected_roi.x);
                corrected_roi.height = std::min(corrected_roi.height, result_frame.rows - corrected_roi.y);
                
                // Düzeltilmiş ROI ile crop yap
                crop_obj.triggerHumanCrop(personInFrame.frame, corrected_roi, std::to_string(personInFrame.trackerId), personInFrame);
                personInFrame.notified = true;
            }
        }
        
        resultStatus = true;
        
    } else {
        resultStatus = false;
        humanDetected = false;
        // Nesne tespit edilmedi - sadece log
        // std::cout << "❌ Hiç nesne tespit edilmedi." << std::endl;
    }
    
    // 🔄 İNSAN TESPİT KONTROLÜ - Ekranı temizle
    if (!humanDetected && personInFrame.trackerId != -1) {
        std::cout << "❌ EKRANDA İNSAN YOK - Durum sıfırlanıyor..." << std::endl;
        
        // Person objesini sıfırla
        personInFrame.trackerId = -1;
        personInFrame.detectedFrameId = -1;
        personInFrame.gloves = false;
        personInFrame.helmet = false;
        personInFrame.mask = false;
        personInFrame.vest = false;
        personInFrame.notified = false;
        personInFrame.cardId = -1;
        personInFrame.card_status = false;
        personInFrame.detected_roi = cv::Rect(0, 0, 0, 0);
        personInFrame.frame = cv::Mat();
        
        // Global tracking verilerini temizle
        trackTimeMap.clear();
        triggeredMap.clear();
        smallCardWarningCount.clear();
        showProximityWarning.clear();
        trackOCRResults.clear();
        vestFirstDetectedFrame.clear();
        vestCroppedImages.clear();
        
        // ✅ TRACKER RESET - Bu çok önemli!
        trackerNeedsReset = true;
        
        std::cout << "✅ Tüm tracking verileri temizlendi - Tracker de reset edilecek" << std::endl;
    }

    return result_frame; // ✅ Her durumda frame döndürülüyor
}


