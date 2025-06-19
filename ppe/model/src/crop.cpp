#include "../include/detection.h"
#include "../include/crop.h"
#include "../../db/include/db.h"

// Qt'den sonra torch include'larını ekleyin ve makro çakışmasını önleyin
#ifdef slots
#undef slots
#endif

#include "../include/detection.h"  // Sadece cpp dosyasında include edin
#include <torch/script.h>

// Qt için tekrar tanımlayın
#ifndef QT_NO_KEYWORDS
#define slots Q_SLOTS
#endif

// Constructor - DetectionModule pointer'ını initialize edin
Crop::Crop(QObject *parent) : QObject(parent) {
    detectionModule = new DetectionModule();
    // std::cout << "Crop nesnesi oluşturuldu" << std::endl;
}

Crop::~Crop() {
    delete detectionModule;
    // std::cout << "Crop nesnesi yok edildi" << std::endl;
}

// Violation crop slot - mevcut fonksiyon
void Crop::cropSlot(const cv::Mat &frame, cv::Rect detected_roi, std::string trackIdStr, Person &personInFrame) {
    try {
        // ROI bounds kontrolü
        if (detected_roi.x < 0 || detected_roi.y < 0 || 
            detected_roi.x + detected_roi.width > frame.cols || 
            detected_roi.y + detected_roi.height > frame.rows ||
            detected_roi.width <= 0 || detected_roi.height <= 0) {
            std::cerr << "❌ Geçersiz ROI - Violation crop atlandı (Track ID: " << trackIdStr << ")" << std::endl;
            return;
        }
        
        cv::Mat frame_copy = frame.clone(); // Frame'i kopyalayın çünkü const
        cv::rectangle(frame_copy, detected_roi, cv::Scalar(0, 0, 255), 10);
        cv::Mat cropped_image = frame_copy(detected_roi);

        DB db;
        db.saveCroppedImage(cropped_image, trackIdStr, 
                            personInFrame.helmet, 
                            personInFrame.vest, 
                            personInFrame.gloves, 
                            personInFrame.mask);
        
        std::cout << "✅ Violation crop kaydedildi (Track ID: " << trackIdStr << ")" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Crop slot hatası: " << e.what() << std::endl;
    }
}

// Vest crop slot - id_card_detection çağırır
void Crop::vestCropSlot(const cv::Mat &frame, cv::Rect detected_roi, std::string trackIdStr, Person &personInFrame,  DetectionModule::Vest &vest) {

    std::cout << "🧥 VEST CROP SLOT TETİKLENDİ (10 frame aralıkla - Track ID: " << trackIdStr << ")" << std::endl;
    std::cout << "   Gelen frame boyutu: " << frame.cols << "x" << frame.rows << std::endl;
    std::cout << "   Vest ROI: x=" << detected_roi.x << ", y=" << detected_roi.y 
              << ", w=" << detected_roi.width << ", h=" << detected_roi.height << std::endl;
    try {
        // ROI bounds kontrolü
        if (detected_roi.x < 0 || detected_roi.y < 0 || 
            detected_roi.x + detected_roi.width > frame.cols || 
            detected_roi.y + detected_roi.height > frame.rows ||
            detected_roi.width <= 0 || detected_roi.height <= 0) {
            std::cerr << "❌ Geçersiz ROI - Vest crop atlandı (Track ID: " << trackIdStr << ")" << std::endl;
            return;
        }
        
        cv::Mat frame_copy = frame.clone();
        cv::rectangle(frame_copy, detected_roi, cv::Scalar(255, 0, 0), 10);  // Mavi çerçeve
        cv::Mat cropped_image = frame_copy(detected_roi);

        std::cout << "✅ VEST CROP BAŞARILI!" << std::endl;
        std::cout << "   Crop boyutu: " << cropped_image.cols << "x" << cropped_image.rows << std::endl;

        vest.cropped_vest = cropped_image;

        
    } catch (const std::exception& e) {
        std::cerr << "❌ Vest crop slot hatası: " << e.what() << std::endl;
    }
}

// Human crop slot - DB'ye human crop kaydet
void Crop::humanCropSlot(const cv::Mat &frame, cv::Rect detected_roi, std::string trackIdStr, Person &personInFrame) {
    try {
        // ROI bounds kontrolü - detaylı debug
        if (detected_roi.x < 0 || detected_roi.y < 0 || 
            detected_roi.x + detected_roi.width > frame.cols || 
            detected_roi.y + detected_roi.height > frame.rows ||
            detected_roi.width <= 0 || detected_roi.height <= 0) {
            std::cerr << "❌ Geçersiz ROI - Human crop atlandı (Track ID: " << trackIdStr << ")" << std::endl;
            std::cerr << "   Frame boyutu: " << frame.cols << "x" << frame.rows << std::endl;
            std::cerr << "   ROI: x=" << detected_roi.x << ", y=" << detected_roi.y 
                     << ", w=" << detected_roi.width << ", h=" << detected_roi.height << std::endl;
            std::cerr << "   ROI sınırları: x+w=" << (detected_roi.x + detected_roi.width) 
                     << ", y+h=" << (detected_roi.y + detected_roi.height) << std::endl;
            return;
        }
        
        cv::Mat frame_copy = frame.clone();
        cv::rectangle(frame_copy, detected_roi, cv::Scalar(255, 0, 0), 10);  // Mavi çerçeve
        cv::Mat cropped_image = frame_copy(detected_roi);


        
        // DB'ye human crop kaydet
        DB db;
        db.saveCroppedImage(cropped_image, trackIdStr, 
                            personInFrame.helmet, 
                            personInFrame.vest, 
                            personInFrame.gloves, 
                            personInFrame.mask,
                            personInFrame.cardId,
                            personInFrame.card_status
                        
                        ); // cardId ekleniyor
        
        std::cout << "✅ Human crop kaydedildi (Track ID: " << trackIdStr << ")" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Human crop slot hatası: " << e.what() << std::endl;
    }
}

void Crop::idCardCropSlot(const cv::Mat &frame, cv::Rect detected_roi, Person &personInFrame,  DetectionModule::IdCard &idCard_obj) {
    try {
        std::cout << "🎯 ID Card crop slot tetiklendi" << std::endl;
        std::cout << "   Frame boyutu: " << frame.cols << "x" << frame.rows << std::endl;
        std::cout << "   ROI: x=" << detected_roi.x << ", y=" << detected_roi.y 
                  << ", w=" << detected_roi.width << ", h=" << detected_roi.height << std::endl;
        
        // ✅ ROI bounds kontrolü - detaylı debug
        bool roi_valid = (detected_roi.x >= 0 && detected_roi.y >= 0 && 
                         detected_roi.x + detected_roi.width <= frame.cols && 
                         detected_roi.y + detected_roi.height <= frame.rows &&
                         detected_roi.width > 0 && detected_roi.height > 0);
        
        if (!roi_valid) {
            std::cerr << "❌ GEÇERSİZ ROI - ID Card crop atlandı!" << std::endl;
            std::cerr << "   Frame boyutu: " << frame.cols << "x" << frame.rows << std::endl;
            std::cerr << "   ROI koordinatları: x=" << detected_roi.x << ", y=" << detected_roi.y 
                     << ", w=" << detected_roi.width << ", h=" << detected_roi.height << std::endl;
            std::cerr << "   ROI sınırları: x+w=" << (detected_roi.x + detected_roi.width) 
                     << ", y+h=" << (detected_roi.y + detected_roi.height) << std::endl;
            
            // Hangi kontrol başarısız oldu?
            if (detected_roi.x < 0) std::cerr << "   ❌ x < 0" << std::endl;
            if (detected_roi.y < 0) std::cerr << "   ❌ y < 0" << std::endl;
            if (detected_roi.x + detected_roi.width > frame.cols) std::cerr << "   ❌ x+w > frame.cols" << std::endl;
            if (detected_roi.y + detected_roi.height > frame.rows) std::cerr << "   ❌ y+h > frame.rows" << std::endl;
            if (detected_roi.width <= 0) std::cerr << "   ❌ width <= 0" << std::endl;
            if (detected_roi.height <= 0) std::cerr << "   ❌ height <= 0" << std::endl;
            
            return;
        }
        
        cv::Mat frame_copy = frame.clone();
        
        // ✅ Güvenli crop işlemi
        try {
            // ROI'yi frame sınırlarına clamp et (güvenlik için)
            cv::Rect safe_roi = detected_roi;
            safe_roi.x = std::max(0, safe_roi.x);
            safe_roi.y = std::max(0, safe_roi.y);
            safe_roi.width = std::min(safe_roi.width, frame.cols - safe_roi.x);
            safe_roi.height = std::min(safe_roi.height, frame.rows - safe_roi.y);
            
            if (safe_roi != detected_roi) {
                std::cout << "⚠️ ROI düzeltildi:" << std::endl;
                std::cout << "   Orijinal: x=" << detected_roi.x << ", y=" << detected_roi.y 
                         << ", w=" << detected_roi.width << ", h=" << detected_roi.height << std::endl;
                std::cout << "   Düzeltilmiş: x=" << safe_roi.x << ", y=" << safe_roi.y 
                         << ", w=" << safe_roi.width << ", h=" << safe_roi.height << std::endl;
            }
            
            cv::rectangle(frame_copy, safe_roi, cv::Scalar(0, 255, 255), 10);  // Sarı çerçeve
            cv::Mat cropped_image = frame_copy(safe_roi);

            idCard_obj.cropped_id_card = cropped_image; // ID Card crop'unu sakla
            
            std::cout << "✅ ID Card crop başarılı!" << std::endl;
            std::cout << "   Crop boyutu: " << cropped_image.cols << "x" << cropped_image.rows << std::endl;
            
        } catch (const cv::Exception& crop_error) {
            std::cerr << "❌ Crop işlemi OpenCV hatası: " << crop_error.what() << std::endl;
            throw;
        }
        
    } catch (const cv::Exception& e) {
        std::cerr << "❌ ID Card crop OpenCV hatası: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ ID Card crop genel hatası: " << e.what() << std::endl;
    }
}


// Signal tetikleyici fonksiyonlar
void Crop::triggerCrop(const cv::Mat &frame, cv::Rect detected_roi, std::string trackIdStr, Person &personInFrame) {
    emit cropSignal(frame, detected_roi, trackIdStr, personInFrame);
}

void Crop::triggerVestCrop(const cv::Mat &frame, cv::Rect detected_roi, std::string trackIdStr, Person &personInFrame, DetectionModule::Vest &vest) {
    emit vestCropSignal(frame, detected_roi, trackIdStr, personInFrame, vest);
}

void Crop::triggerHumanCrop(const cv::Mat &frame, cv::Rect detected_roi, std::string trackIdStr, Person &personInFrame) {
    emit humanCropSignal(frame, detected_roi, trackIdStr, personInFrame);
}

void Crop::triggerIdCardCrop(const cv::Mat &frame, cv::Rect detected_roi, Person &personInFrame, DetectionModule::IdCard &idCard_obj) {
    emit idCardCropSignal(frame, detected_roi, personInFrame, idCard_obj);
}
