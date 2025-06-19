#ifndef MAIL_H
#define MAIL_H

#include <string>
#include <opencv2/core/mat.hpp> // OpenCV için cv::Mat sınıfını içeri aktarmalıyız

class MailSender {
public:
    static MailSender& getInstance() {
        static MailSender instance;
        return instance;
    }
    
    std::string trackId;       // Track ID verisi
    cv::Mat imageData;         // Image verisi (OpenCV Mat)
    std::string recipientEmail;  
    
    // Email gönderme fonksiyonu
    void sendEmail();

    // Mail verilerini alma fonksiyonu
    void getMailData();
    static size_t readCallback(void* ptr, size_t size, size_t nmemb, void* userp); // Burada 'static' olmalı
    
    // Otomatik mail gönderme ayarları için yeni fonksiyonlar
    void setAutoMailEnabled(bool enabled);
    bool isAutoMailEnabled() const;
    
private:
    MailSender() : autoMailEnabled(true) {} // Private constructor, varsayılan olarak açık
    MailSender(const MailSender&) = delete; // Copy constructor'ı engelle
    void operator=(const MailSender&) = delete; // Assignment operatörünü engelle
    
    bool autoMailEnabled; // Otomatik mail gönderme durumu
};

#endif // MAIL_H
