#include "../include/mail.h"
#include "../../db/include/db.h"
#include "../../ui/pages/include/settings.h"
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <map>
#include <string>

void loadEnvFile(const std::string& filename = ".env") {
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::string parentPath = "../" + filename;
        file.open(parentPath);
        
        if (!file.is_open()) {
            std::string rootPath = "../../" + filename;
            file.open(rootPath);
        }
    }
    
    if (!file.is_open()) {
        std::cout << "INFO: .env dosyası bulunamadı (denenen yollar: " << filename << ", ../" << filename << ", ../../" << filename << "), sistem environment variable'ları kullanılacak." << std::endl;
        return;
    }
    
    std::cout << "INFO: .env dosyası bulundu ve yükleniyor..." << std::endl;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // KEY=VALUE formatını parse et
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Başta ve sonda boşlukları temizle
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            // Environment variable'ı ayarla (sadece daha önce ayarlanmamışsa)
            if (std::getenv(key.c_str()) == nullptr) {
                #ifdef _WIN32
                    _putenv_s(key.c_str(), value.c_str());
                #else
                    setenv(key.c_str(), value.c_str(), 0); // 0 = overwrite etme
                #endif
                std::cout << "INFO: .env dosyasından yüklendi: " << key << std::endl;
            }
        }
    }
    file.close();
}

void MailSender::setAutoMailEnabled(bool enabled) {
    autoMailEnabled = enabled;
    std::cout << "Otomatik mail gönderme: " << (autoMailEnabled ? "Açık" : "Kapalı") << std::endl;
}

bool MailSender::isAutoMailEnabled() const {
    return autoMailEnabled;
}

void MailSender::sendEmail() {
    // Otomatik mail gönderme kapalıysa, hiçbir şey yapma
    if (!isAutoMailEnabled()) {
        std::cout << "Otomatik mail gönderme kapalı olduğu için mail gönderilmedi." << std::endl;
        return;
    }

    std::cout << "Alıcı e-posta: [" << recipientEmail << "]" << std::endl;
    
    // E-posta adresi kontrolü
    if (recipientEmail.empty()) {
        std::cerr << "Alıcı e-posta adresi boş! E-posta gönderilemedi." << std::endl;
        return;
    }
    
    // .env dosyasını yükle
    loadEnvFile();
    
    // E-posta gönderme ayarları - Environment variables'dan oku
    const char* env_email = std::getenv("SMTP_EMAIL");
    const char* env_password = std::getenv("SMTP_PASSWORD");
    const char* env_server = std::getenv("SMTP_SERVER");
    
    // Environment variable'ları kontrol et
    if (!env_email || !env_password || !env_server) {
        std::cerr << "HATA: SMTP environment variable'ları bulunamadı!" << std::endl;
        std::cerr << "Lütfen .env dosyasını oluşturun veya şu environment variable'ları ayarlayın:" << std::endl;
        std::cerr << "  SMTP_EMAIL=your-email@gmail.com" << std::endl;
        std::cerr << "  SMTP_PASSWORD=your-app-password" << std::endl;
        std::cerr << "  SMTP_SERVER=smtp://smtp.gmail.com:587" << std::endl;
        return;
    }
    
    const std::string gonderici_email = env_email;
    const std::string gonderici_sifre = env_password;
    const std::string smtp_server = env_server;
    std::string alici_email = recipientEmail;
    
    std::cout << "✅ SMTP ayarları başarıyla yüklendi (.env dosyasından)" << std::endl;
    
    // Veritabanından veri al (konu belirlemek için)
    DB db;
    auto tensor = db.getLastViolationFromDB(); 
    
    // Worker durumunu kontrol et
    bool isUnknownWorker = (tensor.firstName == "Bilinmeyen" && tensor.lastName == "Çalışan" && tensor.department == "Bilinmeyen Departman");
    bool hasValidCardId = (tensor.card_id > 0);
    
    // Duruma göre konu belirle
    std::string konu;
    if (isUnknownWorker && hasValidCardId) {
        konu = "🚨 ACİL: Bilinmeyen Çalışan İhlali - Kart ID: " + std::to_string(tensor.card_id);
    } else if (!hasValidCardId) {
        konu = "🚨 KKD İhlali - Kimlik Kartı Okunamadı";
    } else {
        konu = "🚨 KKD İhlal Bildirimi - " + tensor.firstName + " " + tensor.lastName;
    }
   
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL başlatılamadı!" << std::endl;
        return;
    }

    struct curl_slist* recipients = nullptr;
    curl_mime* mime = nullptr;
    curl_mimepart* part = nullptr;

    try {
        // Temel CURL ayarları
        curl_easy_setopt(curl, CURLOPT_URL, smtp_server.c_str());
        curl_easy_setopt(curl, CURLOPT_USERNAME, gonderici_email.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, gonderici_sifre.c_str());
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        // Alıcı ayarları
        recipients = curl_slist_append(nullptr, alici_email.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, gonderici_email.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        // MIME yapısı oluşturma
        mime = curl_mime_init(curl);

        // E-posta başlıkları
        part = curl_mime_addpart(mime);
        std::string headers = konu + "\r\n";
        curl_mime_data(part, headers.c_str(), CURL_ZERO_TERMINATED);
        curl_mime_type(part, "text/html; charset=UTF-8");

        // Resim verisini decode et
        cv::Mat imageData;
        std::string trackId;
        
        trackId = tensor.trackId;
        imageData = cv::imdecode(tensor.imageData, cv::IMREAD_COLOR);
        
        if (trackId.empty()) {
            trackId = "Bilinmeyen";
        }

        // KKD ayarlarını al
        std::map<std::string, bool> kKDSettings = Settings::getKKDSettings();
        
        // İhlal nedenlerini toplama - sadece aktif KKD'ler için
        std::vector<std::string> violationReasons;
        if (kKDSettings["helmet"] && !tensor.helmet) violationReasons.push_back("🪖 Kask takılmamış");
        if (kKDSettings["vest"] && !tensor.vest) violationReasons.push_back("🦺 Yelek giyilmemiş");
        if (kKDSettings["gloves"] && !tensor.gloves) violationReasons.push_back("🧤 Eldiven takılmamış");
        if (kKDSettings["mask"] && !tensor.mask) violationReasons.push_back("😷 Maske takılmamış");

        // İhlal nedenlerini string'e çevirme
        std::string violationList = "";
        for (size_t i = 0; i < violationReasons.size(); ++i) {
            violationList += "    • " + violationReasons[i] + "\n";
        }

        // Worker listesinde olup olmadığını kontrol et
        bool isUnknownWorker = (tensor.firstName == "Bilinmeyen" && tensor.lastName == "Çalışan" && tensor.department == "Bilinmeyen Departman");
        bool hasValidCardId = (tensor.card_id > 0); // Card ID okunabilmiş mi?

        // HTML formatında güzel mesaj oluşturma
        std::ostringstream htmlMessage;
        htmlMessage << "<!DOCTYPE html>\n"
                   << "<html>\n"
                   << "<head>\n"
                   << "    <meta charset='UTF-8'>\n"
                   << "    <style>\n"
                   << "        body { font-family: Arial, sans-serif; line-height: 1.6; color: #333; }\n"
                   << "        .header { background-color: #ff4444; color: white; padding: 20px; text-align: center; border-radius: 8px 8px 0 0; }\n"
                   << "        .content { background-color: #f9f9f9; padding: 20px; border: 1px solid #ddd; }\n"
                   << "        .worker-info { background-color: #e8f4f8; padding: 15px; border-radius: 5px; margin: 10px 0; }\n"
                   << "        .unknown-worker { background-color: #fff3cd; padding: 15px; border-radius: 5px; margin: 10px 0; border-left: 4px solid #ffc107; }\n"
                   << "        .violation-list { background-color: #ffe6e6; padding: 15px; border-radius: 5px; margin: 10px 0; border-left: 4px solid #ff4444; }\n"
                   << "        .footer { background-color: #333; color: white; padding: 10px; text-align: center; border-radius: 0 0 8px 8px; font-size: 12px; }\n"
                   << "        .warning { color: #ff4444; font-weight: bold; }\n"
                   << "        .unknown-warning { color: #856404; font-weight: bold; }\n"
                   << "        .info-row { margin: 5px 0; }\n"
                   << "    </style>\n"
                   << "</head>\n"
                   << "<body>\n"
                   << "    <div class='header'>\n";

        if (isUnknownWorker && hasValidCardId) {
            htmlMessage << "        <h2>🚨 BİLİNMEYEN ÇALIŞAN İHLALİ</h2>\n"
                       << "        <p>Listede Olmayan Çalışan Güvenlik İhlali Yaptı</p>\n";
        } else {
            htmlMessage << "        <h2>🚨 KKD İHLAL BİLDİRİMİ</h2>\n"
                       << "        <p>Güvenlik İhlali Tespit Edildi</p>\n";
        }

        htmlMessage << "    </div>\n"
                   << "    <div class='content'>\n"
                   << "        <h3>İhlal Detayları:</h3>\n";

        if (isUnknownWorker && hasValidCardId) {
            // Bilinmeyen çalışan için özel uyarı
            htmlMessage << "        <div class='unknown-worker'>\n"
                       << "            <h4 class='unknown-warning'>⚠️ DİKKAT: BİLİNMEYEN ÇALIŞAN!</h4>\n"
                       << "            <div class='info-row'><strong>Durum:</strong> Bu kart ID'si çalışan listesinde kayıtlı değil!</div>\n"
                       << "            <div class='info-row'><strong>Kart ID:</strong> " << tensor.card_id << "</div>\n"
                       << "            <div class='info-row'><strong>Tarih/Saat:</strong> " << tensor.timestamp << "</div>\n"
                       << "            <div class='info-row'><strong>Öneri:</strong> Bu çalışanın kimlik bilgilerini kontrol edin ve sisteme kaydedin.</div>\n"
                       << "        </div>\n";
        } else if (!hasValidCardId) {
            // Kart ID okunamayan durum
            htmlMessage << "        <div class='unknown-worker'>\n"
                       << "            <h4 class='unknown-warning'>⚠️ DİKKAT: KART ID OKUNAMADI!</h4>\n"
                       << "            <div class='info-row'><strong>Durum:</strong> Çalışanın kimlik kartı okunamadı</div>\n"
                       << "            <div class='info-row'><strong>Kart ID:</strong> Tespit edilemedi</div>\n"
                       << "            <div class='info-row'><strong>Tarih/Saat:</strong> " << tensor.timestamp << "</div>\n"
                       << "            <div class='info-row'><strong>Öneri:</strong> Çalışanın kimlik kartını kontrol edin.</div>\n"
                       << "        </div>\n";
        } else {
            // Normal çalışan bilgileri
            htmlMessage << "        <div class='worker-info'>\n"
                       << "            <h4>👤 Çalışan Bilgileri:</h4>\n"
                       << "            <div class='info-row'><strong>Ad Soyad:</strong> " << tensor.firstName << " " << tensor.lastName << "</div>\n"
                       << "            <div class='info-row'><strong>Departman:</strong> " << tensor.department << "</div>\n"
                       << "            <div class='info-row'><strong>Kart ID:</strong> " << tensor.card_id << "</div>\n"
                       << "            <div class='info-row'><strong>Tarih/Saat:</strong> " << tensor.timestamp << "</div>\n"
                       << "        </div>\n";
        }
        
        htmlMessage << "        <div class='violation-list'>\n"
                   << "            <h4 class='warning'>⚠️ Tespit Edilen İhlaller:</h4>\n";

        for (const auto& reason : violationReasons) {
            htmlMessage << "            <div class='info-row'>• " << reason << "</div>\n";
        }

        htmlMessage << "        </div>\n";

        // Duruma göre farklı açıklama metinleri
        if (isUnknownWorker && hasValidCardId) {
            htmlMessage << "        <p><strong>Açıklama:</strong> Sisteme kayıtlı olmayan bir çalışan KKD ihlali yapmıştır. "
                       << "Bu durum güvenlik açısından ciddi bir risk oluşturmaktadır.</p>\n"
                       << "        <p class='warning'>ACİL EYLEM GEREKLİ:</p>\n"
                       << "        <ul>\n"
                       << "            <li>Bu çalışanın kimlik bilgilerini derhal kontrol edin</li>\n"
                       << "            <li>Yetkisiz giriş olup olmadığını araştırın</li>\n"
                       << "            <li>Çalışan sisteme kayıtlıysa, bilgilerini güncelleyin</li>\n"
                       << "            <li>Güvenlik protokollerini gözden geçirin</li>\n"
                       << "        </ul>\n";
        } else if (!hasValidCardId) {
            htmlMessage << "        <p><strong>Açıklama:</strong> Kimlik kartı okunamayan bir çalışan KKD ihlali yapmıştır. "
                       << "Bu durum kimlik doğrulama sisteminde sorun olduğunu gösterebilir.</p>\n"
                       << "        <p class='warning'>KONTROL EDİLMESİ GEREKENLER:</p>\n"
                       << "        <ul>\n"
                       << "            <li>Çalışanın kimlik kartının durumunu kontrol edin</li>\n"
                       << "            <li>Kart okuyucu sisteminin çalışıp çalışmadığını kontrol edin</li>\n"
                       << "            <li>Çalışana yeni kart verilmesi gerekip gerekmediğini değerlendirin</li>\n"
                       << "        </ul>\n";
        } else {
            htmlMessage << "        <p><strong>Açıklama:</strong> Yukarıda belirtilen KKD eksiklikleri tespit edilmiştir. "
                       << "Gerekli önlemlerin alınması ve çalışanın uyarılması gerekmektedir.</p>\n"
                       << "        <p class='warning'>Bu durum iş güvenliği açısından risk oluşturmaktadır!</p>\n";
        }

        htmlMessage << "    </div>\n"
                   << "    <div class='footer'>\n"
                   << "        <p>Bu mesaj otomatik olarak KKD İzleme Sistemi tarafından gönderilmiştir.</p>\n"
                   << "        <p>Güvenlik Departmanı - " << tensor.timestamp << "</p>\n"
                   << "    </div>\n"
                   << "</body>\n"
                   << "</html>";

        std::string mesaj_icerik = htmlMessage.str();

        // HTML içeriği
        part = curl_mime_addpart(mime);
        curl_mime_data(part, mesaj_icerik.c_str(), CURL_ZERO_TERMINATED);
        curl_mime_type(part, "text/html; charset=UTF-8");
    
        // Resim ekleme
        if (!imageData.empty()) {
            std::vector<uchar> buffer;
            if (cv::imencode(".jpg", imageData, buffer)) {
                part = curl_mime_addpart(mime);
                curl_mime_data(part, reinterpret_cast<const char*>(buffer.data()), buffer.size());
                curl_mime_type(part, "image/jpeg");
                curl_mime_encoder(part, "base64");
                curl_mime_name(part, "attachment");
                
                // Dosya adını duruma göre oluştur
                std::string filename;
                if (isUnknownWorker && hasValidCardId) {
                    filename = "ihlal_bilinmeyen_calisan_ID" + std::to_string(tensor.card_id) + "_" + 
                              tensor.timestamp.substr(0, 10) + ".jpg";
                } else if (!hasValidCardId) {
                    filename = "ihlal_kimlik_okunamadi_" + tensor.timestamp.substr(0, 10) + ".jpg";
                } else {
                    filename = "ihlal_" + tensor.firstName + "_" + tensor.lastName + "_" + 
                              tensor.timestamp.substr(0, 10) + ".jpg";
                }
                curl_mime_filename(part, filename.c_str());
            }
        }

        // MIME yapısını ayarla
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        // E-postayı gönder
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "E-posta gönderme hatası: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "✅ E-posta başarıyla gönderildi!" << std::endl;
            if (isUnknownWorker && hasValidCardId) {
                std::cout << "📧 Gönderilen: BİLİNMEYEN ÇALIŞAN (Kart ID: " << tensor.card_id 
                          << ") için " << violationReasons.size() << " ihlal bildirimi" << std::endl;
            } else if (!hasValidCardId) {
                std::cout << "📧 Gönderilen: KİMLİK OKUNAMAYAN ÇALIŞAN için " 
                          << violationReasons.size() << " ihlal bildirimi" << std::endl;
            } else {
                std::cout << "📧 Gönderilen: " << tensor.firstName << " " << tensor.lastName 
                          << " için " << violationReasons.size() << " ihlal bildirimi" << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Hata: " << e.what() << std::endl;
    }

    // Temizlik
    if (mime) curl_mime_free(mime);
    if (recipients) curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
}

void MailSender::getMailData() {
    std::cout << "Getting mail data..." << std::endl;
}

size_t MailSender::readCallback(void* ptr, size_t size, size_t nmemb, void* userp) {
    return 0;
}
