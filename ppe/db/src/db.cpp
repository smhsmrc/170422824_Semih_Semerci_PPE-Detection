#include "../include/db.h"
#include "../../model/include/mail.h"
#include <iostream>
#include <sqlite3.h>
#include <chrono>
#include <sstream>
#include <iomanip>

// Constructor implementasyonu
DB::DB() {
    // Boş constructor
}


// ✅ TAMAMLANMIŞ: saveCroppedImage fonksiyonu
void DB::saveCroppedImage(cv::Mat cropped_image, const std::string &trackIdStr, 
                         bool helmet, bool vest, bool gloves, bool mask, int card_id, bool cart_status) {
    try {
        // Görüntüyü JPEG formatında encode et
        std::vector<uchar> buffer;
        if (!cv::imencode(".jpg", cropped_image, buffer)) {
            std::cerr << "❌ Görüntü encode edilemedi!" << std::endl;
            return;
        }

        // Veritabanı bağlantısını başlat
        sqlite3 *db = nullptr;
        if (!initializeDatabase(&db)) {
            std::cerr << "❌ Veritabanı başlatılamadı." << std::endl;
            return;
        }

        // Tablo oluştur (yoksa)
        if (!createTableIfNotExists(db)) {
            std::cerr << "❌ Tablo oluşturulamadı." << std::endl;
            sqlite3_close(db);
            return;
        }

        // Veritabanına kaydet
        if (!insertImageToDB(db, trackIdStr, buffer, helmet, vest, gloves, mask, cart_status, card_id)) {
            std::cerr << "❌ Görüntü veritabanına eklenemedi." << std::endl;
        } else {
            std::cout << "✅ Görüntü başarıyla kaydedildi (Track ID: " << trackIdStr << ", Card ID: " << card_id << ")" << std::endl;
        }
        
        sqlite3_close(db);
        
    } catch (const std::exception& e) {
        std::cerr << "❌ saveCroppedImage hatası: " << e.what() << std::endl;
    }
}

bool DB::initializeDatabase(sqlite3 **db) {
    return openDatabase(db);
}

bool DB::openDatabase(sqlite3 **db) {
    int exitCode = sqlite3_open("violation.db", db);
    if (exitCode) {
        std::cerr << "❌ Veritabanı açılırken hata oluştu: " << sqlite3_errmsg(*db) << std::endl;
        return false;
    }
    return true;
}

// ✅ GÜNCELLENMIŞ: card_id sütunu eklendi
bool DB::createTableIfNotExists(sqlite3 *db) {
    const char *createTableSQL =
        "CREATE TABLE IF NOT EXISTS Images("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "TrackID TEXT NOT NULL, "
        "Image BLOB NOT NULL, "
        "Timestamp TEXT, "
        "Helmet INTEGER NOT NULL DEFAULT 0, "
        "Vest INTEGER NOT NULL DEFAULT 0, "
        "Gloves INTEGER NOT NULL DEFAULT 0, "
        "Mask INTEGER NOT NULL DEFAULT 0, "
        "Cart_Status INTEGER NOT NULL DEFAULT 0, "
        "CardID INTEGER DEFAULT -1);";

    char *errorMessage = nullptr;
    int exitCode = sqlite3_exec(db, createTableSQL, nullptr, 0, &errorMessage);

    if (exitCode != SQLITE_OK) {
        std::cerr << "❌ Tablo oluşturulurken hata oluştu: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        return false;
    }

    return true;
}

// ✅ DÜZELTME: insertImageToDB fonksiyonu
bool DB::insertImageToDB(sqlite3 *db, const std::string &trackIdStr, const std::vector<uchar> &imageData, 
                        bool helmet, bool vest, bool gloves, bool mask, bool cart_status, int card_id) {
    // Timestamp oluştur
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream timestampStream;
    timestampStream << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
    std::string timestampStr = timestampStream.str();

    const char *insertSQL = "INSERT INTO Images (TrackID, Image, Timestamp, Helmet, Vest, Gloves, Mask, Cart_Status, CardID) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    int exitCode = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
    if (exitCode != SQLITE_OK) {
        std::cerr << "❌ SQL sorgusu hazırlanırken hata oluştu: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Parametreleri bağla
    sqlite3_bind_text(stmt, 1, trackIdStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 2, imageData.data(), (int)imageData.size(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, timestampStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, helmet ? 1 : 0);
    sqlite3_bind_int(stmt, 5, vest ? 1 : 0);
    sqlite3_bind_int(stmt, 6, gloves ? 1 : 0);
    sqlite3_bind_int(stmt, 7, mask ? 1 : 0);
    sqlite3_bind_int(stmt, 8, cart_status ? 1 : 0);
    sqlite3_bind_int(stmt, 9, card_id);

    exitCode = sqlite3_step(stmt);
    if (exitCode != SQLITE_DONE) {
        std::cerr << "❌ Veri eklenirken hata oluştu: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    std::cout << "✅ Veri başarıyla eklendi, TrackID: " << trackIdStr << ", CardID: " << card_id << std::endl;
    
    // Mail gönder
    try {
        MailSender& mail = MailSender::getInstance();
        mail.sendEmail();
    } catch (const std::exception& e) {
        std::cerr << "⚠️ Mail gönderilirken hata: " << e.what() << std::endl;
    }

    sqlite3_finalize(stmt);
    return true;
}



DB::Tensor DB::getLastViolationFromDB() {
    sqlite3 *db = nullptr;
    if (!initializeDatabase(&db)) {
        std::cerr << "Veritabanı başlatılamadı." << std::endl;
        return {};
    }

    if (!createTableIfNotExists(db)) {
        std::cerr << "Tablo oluşturulamadı." << std::endl;
        sqlite3_close(db);
        return {};
    }

    // ✅ workerlist.db'yi attach et
    const char *attachSQL = "ATTACH DATABASE 'workerlist.db' AS workerlist;";
    char *errorMessage = nullptr;
    int exitCode = sqlite3_exec(db, attachSQL, nullptr, nullptr, &errorMessage);
    if (exitCode != SQLITE_OK) {
        std::cerr << "workerlist.db attach edilemedi: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        return {};
    }

    // ✅ GÜNCELLENMIŞ: Cart_Status da seçiliyor
    const char *querySQL = R"(
        SELECT 
            i.TrackID, 
            i.Image, 
            i.Timestamp, 
            i.Helmet, 
            i.Vest, 
            i.Gloves, 
            i.Mask, 
            i.Cart_Status,
            i.CardID,
            COALESCE(w.FirstName, 'Bilinmeyen') AS FirstName,
            COALESCE(w.LastName, 'Çalışan') AS LastName,
            COALESCE(w.Department, 'Bilinmeyen Departman') AS Department
        FROM Images i
        LEFT JOIN workerlist.Workers w ON i.CardID = w.CardID
        ORDER BY i.ID DESC
        LIMIT 1;
    )";

    sqlite3_stmt *stmt;
    exitCode = sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr);
    if (exitCode != SQLITE_OK) {
        std::cerr << "SQL sorgusu hazırlanırken hata oluştu: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return {};
    }

    exitCode = sqlite3_step(stmt);
    if (exitCode == SQLITE_ROW) {
        const unsigned char *trackIdText = sqlite3_column_text(stmt, 0);
        std::string trackIdStr = trackIdText ? reinterpret_cast<const char *>(trackIdText) : "";

        const void *blobData = sqlite3_column_blob(stmt, 1);
        int blobSize = sqlite3_column_bytes(stmt, 1);
        std::vector<uchar> imageData;

        if (blobData && blobSize > 0) {
            imageData.assign(
                reinterpret_cast<const uchar *>(blobData),
                reinterpret_cast<const uchar *>(blobData) + blobSize
            );
        }

        std::string timestampStr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        bool helmet = sqlite3_column_int(stmt, 3) != 0;
        bool vest = sqlite3_column_int(stmt, 4) != 0;
        bool gloves = sqlite3_column_int(stmt, 5) != 0;
        bool mask = sqlite3_column_int(stmt, 6) != 0;
        bool cart_status = sqlite3_column_int(stmt, 7) != 0;  // ✅ YENİ ALAN: Cart_Status
        int card_id = sqlite3_column_int(stmt, 8);            // ✅ Index güncellendi

        // ✅ YENİ ALANLAR: İsim, Soyisim, Departman (index'ler güncellendi)
        std::string firstName = "";
        std::string lastName = "";
        std::string department = "";
        
        const char* firstNamePtr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        const char* lastNamePtr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* departmentPtr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        
        if (firstNamePtr) firstName = firstNamePtr;
        if (lastNamePtr) lastName = lastNamePtr;
        if (departmentPtr) department = departmentPtr;

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        // ✅ DÜZELTME: Doğru sırayla return
        return {trackIdStr, imageData, timestampStr, helmet, vest, gloves, mask, cart_status, card_id, firstName, lastName, department};
    }

    std::cerr << "Son satır alınırken hata oluştu veya veri bulunamadı." << std::endl;
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return {};
}


std::vector<DB::Tensor> DB::getAllViolationsFromDB() {
    std::vector<Tensor> violations;
    sqlite3 *db = nullptr;
    if (!initializeDatabase(&db)) {
        std::cerr << "Veritabanı başlatılamadı." << std::endl;
        return violations;
    }

    if (!createTableIfNotExists(db)) {
        std::cerr << "Tablo oluşturulamadı." << std::endl;
        sqlite3_close(db);
        return violations;
    }

    // ✅ workerlist.db'yi attach et
    const char *attachSQL = "ATTACH DATABASE 'workerlist.db' AS workerlist;";
    char *errorMessage = nullptr;
    int exitCode = sqlite3_exec(db, attachSQL, nullptr, nullptr, &errorMessage);
    if (exitCode != SQLITE_OK) {
        std::cerr << "workerlist.db attach edilemedi: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        return violations;
    }

    // ✅ GÜNCELLENMIŞ: Cart_Status da seçiliyor
    const char *querySQL = R"(
        SELECT 
            i.TrackID, 
            i.Image, 
            i.Timestamp, 
            i.Helmet, 
            i.Vest, 
            i.Gloves, 
            i.Mask, 
            i.Cart_Status,
            i.CardID,
            COALESCE(w.FirstName, 'Bilinmeyen') AS FirstName,
            COALESCE(w.LastName, 'Çalışan') AS LastName,
            COALESCE(w.Department, 'Bilinmeyen Departman') AS Department
        FROM Images i
        LEFT JOIN workerlist.Workers w ON i.CardID = w.CardID
        ORDER BY i.ID DESC;
    )";

    sqlite3_stmt *stmt;
    exitCode = sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr);
    if (exitCode != SQLITE_OK) {
        std::cerr << "SQL sorgusu hazırlanırken hata oluştu: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return violations;
    }

    while ((exitCode = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::string trackIdStr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        const void *blobData = sqlite3_column_blob(stmt, 1);
        int blobSize = sqlite3_column_bytes(stmt, 1);
        std::vector<uchar> imageData;
        if (blobData && blobSize > 0) {
            imageData.assign(
                reinterpret_cast<const uchar *>(blobData),
                reinterpret_cast<const uchar *>(blobData) + blobSize
            );
        }
        std::string timestampStr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));

        bool helmet = sqlite3_column_int(stmt, 3) != 0;
        bool vest = sqlite3_column_int(stmt, 4) != 0;
        bool gloves = sqlite3_column_int(stmt, 5) != 0;
        bool mask = sqlite3_column_int(stmt, 6) != 0;
        bool cart_status = sqlite3_column_int(stmt, 7) != 0;  // ✅ YENİ ALAN: Cart_Status
        int card_id = sqlite3_column_int(stmt, 8);            // ✅ Index güncellendi

        // ✅ YENİ ALANLAR: İsim, Soyisim, Departman (index'ler güncellendi)
        std::string firstName = "";
        std::string lastName = "";
        std::string department = "";
        
        const char* firstNamePtr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        const char* lastNamePtr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* departmentPtr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        
        if (firstNamePtr) firstName = firstNamePtr;
        if (lastNamePtr) lastName = lastNamePtr;
        if (departmentPtr) department = departmentPtr;

        // ✅ GÜNCELLENMIŞ: cart_status da push ediliyor
        violations.push_back({trackIdStr, imageData, timestampStr, helmet, vest, gloves, mask, cart_status, card_id, firstName, lastName, department});
    }

    if (exitCode != SQLITE_DONE) {
        std::cerr << "Veri okunurken hata oluştu: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return violations;
}

// WorkerList DB fonksiyonları
bool DB::insertWorker(const std::string& firstName, const std::string& lastName, int workerNumber, const std::string& department) {
    sqlite3 *db;
    int exitCode = sqlite3_open("workerlist.db", &db);
    if (exitCode) {
        std::cerr << "workerlist.db açılamadı: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Tabloyu oluştur (varsa atla)
    const char *createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS Workers(
            ID INTEGER PRIMARY KEY AUTOINCREMENT,
            FirstName TEXT NOT NULL,
            LastName TEXT NOT NULL,
            CardID INTEGER NOT NULL UNIQUE,
            Department TEXT NOT NULL
        );
    )";
    
    char *errorMessage = nullptr;
    exitCode = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errorMessage);
    if (exitCode != SQLITE_OK) {
        std::cerr << "Workers tablosu oluşturulamadı: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        return false;
    }

    // Insert sorgusu
    const char *insertSQL = "INSERT INTO Workers (FirstName, LastName, CardID, Department) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    exitCode = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
    if (exitCode != SQLITE_OK) {
        std::cerr << "SQL sorgusu hazırlanamadı: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, firstName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, lastName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, workerNumber);
    sqlite3_bind_text(stmt, 4, department.c_str(), -1, SQLITE_STATIC);

    exitCode = sqlite3_step(stmt);
    if (exitCode != SQLITE_DONE) {
        // SQLITE_CONSTRAINT hatası UNIQUE kısıtlamasının ihlal edildiği anlamına gelebilir
        if (sqlite3_extended_errcode(db) == SQLITE_CONSTRAINT_UNIQUE) {
            std::cerr << "Bu çalışan numarası zaten kullanılıyor: " << workerNumber << std::endl;
        } else {
            std::cerr << "Veri eklenemedi: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    std::cout << "Çalışan başarıyla eklendi: " << firstName << " " << lastName << " - " << department << std::endl;
    return true;
}

std::vector<DB::Worker> DB::getAllWorkersFromDB() {
    std::vector<Worker> workers;
    sqlite3 *db = nullptr;
    
    // Doğru veritabanını açalım
    int exitCode = sqlite3_open("workerlist.db", &db);
    if (exitCode != SQLITE_OK) {
        std::cerr << "Veritabanı açılamadı: " << sqlite3_errmsg(db) << std::endl;
        return workers;
    }

    const char *querySQL = "SELECT FirstName, LastName, CardID, Department FROM Workers ORDER BY ID DESC;";
    sqlite3_stmt *stmt;
    exitCode = sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr);

    if (exitCode != SQLITE_OK) {
        std::cerr << "SQL sorgusu hazırlanırken hata oluştu: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return workers;
    }

    while ((exitCode = sqlite3_step(stmt)) == SQLITE_ROW) {
        Worker worker;
        worker.firstName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        worker.lastName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        worker.workerNumber = sqlite3_column_int(stmt, 2);
        worker.department = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));

        workers.push_back(worker);
    }

    if (exitCode != SQLITE_DONE) {
        std::cerr << "Veriler alınırken hata oluştu: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return workers;
}

bool DB::removeWorkerByNumber(int workerNumber) {
    sqlite3 *db;
    int exitCode = sqlite3_open("workerlist.db", &db);
    if (exitCode != SQLITE_OK) {
        std::cerr << "workerlist.db açılamadı: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    const char *deleteSQL = "DELETE FROM Workers WHERE CardID = ?;";
    sqlite3_stmt *stmt;

    exitCode = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
    if (exitCode != SQLITE_OK) {
        std::cerr << "SQL sorgusu hazırlanamadı: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(stmt, 1, workerNumber);

    exitCode = sqlite3_step(stmt);
    if (exitCode != SQLITE_DONE) {
        std::cerr << "Çalışan silinemedi: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    // Etkilenen kayıt sayısını kontrol et
    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (changes == 0) {
        std::cout << "Belirtilen çalışan numarasına sahip kayıt bulunamadı: " << workerNumber << std::endl;
        return false;
    }

    std::cout << "Çalışan başarıyla silindi. Çalışan Numarası: " << workerNumber << std::endl;
    return true;
}

bool DB::deleteViolationByTimestamp(const std::string& timestamp) {
    sqlite3 *db = nullptr;
    if (!initializeDatabase(&db)) {
        std::cerr << "Veritabanı başlatılamadı." << std::endl;
        return false;
    }

    const char *deleteSQL = "DELETE FROM Images WHERE Timestamp = ?;";
    sqlite3_stmt *stmt;

    int exitCode = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
    if (exitCode != SQLITE_OK) {
        std::cerr << "SQL sorgusu hazırlanamadı: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, timestamp.c_str(), -1, SQLITE_STATIC);

    exitCode = sqlite3_step(stmt);
    if (exitCode != SQLITE_DONE) {
        std::cerr << "İhlal kaydı silinemedi: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    // Etkilenen kayıt sayısını kontrol et
    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (changes == 0) {
        std::cout << "Belirtilen timestamp'e sahip kayıt bulunamadı: " << timestamp << std::endl;
        return false;
    }

    std::cout << "İhlal kaydı başarıyla silindi. Timestamp: " << timestamp << std::endl;
    return true;
}

bool DB::deleteAllViolations() {
    sqlite3 *db = nullptr;
    if (!initializeDatabase(&db)) {
        std::cerr << "Veritabanı başlatılamadı." << std::endl;
        return false;
    }

    // Önce kaç kayıt var kontrol et
    const char *countSQL = "SELECT COUNT(*) FROM Images;";
    sqlite3_stmt *countStmt;
    int recordCount = 0;

    int exitCode = sqlite3_prepare_v2(db, countSQL, -1, &countStmt, nullptr);
    if (exitCode == SQLITE_OK) {
        if (sqlite3_step(countStmt) == SQLITE_ROW) {
            recordCount = sqlite3_column_int(countStmt, 0);
        }
        sqlite3_finalize(countStmt);
    }

    if (recordCount == 0) {
        std::cout << "Silinecek kayıt bulunmuyor." << std::endl;
        sqlite3_close(db);
        return true; // Boş tablo da başarı sayılabilir
    }

    // Tüm kayıtları sil
    const char *deleteSQL = "DELETE FROM Images;";
    char *errorMessage = nullptr;
    exitCode = sqlite3_exec(db, deleteSQL, nullptr, nullptr, &errorMessage);

    if (exitCode != SQLITE_OK) {
        std::cerr << "Tüm kayıtlar silinemedi: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        return false;
    }

    // Auto-increment sıfırla (isteğe bağlı)
    const char *resetSQL = "DELETE FROM sqlite_sequence WHERE name='Images';";
    sqlite3_exec(db, resetSQL, nullptr, nullptr, nullptr);

    sqlite3_close(db);
    std::cout << recordCount << " adet ihlal kaydı başarıyla silindi." << std::endl;
    return true;
}

bool DB::deleteViolationById(int id) {
    sqlite3 *db = nullptr;
    if (!initializeDatabase(&db)) {
        std::cerr << "Veritabanı başlatılamadı." << std::endl;
        return false;
    }

    const char *deleteSQL = "DELETE FROM Images WHERE ID = ?;";
    sqlite3_stmt *stmt;

    int exitCode = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
    if (exitCode != SQLITE_OK) {
        std::cerr << "SQL sorgusu hazırlanamadı: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    exitCode = sqlite3_step(stmt);
    if (exitCode != SQLITE_DONE) {
        std::cerr << "İhlal kaydı silinemedi: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (changes == 0) {
        std::cout << "Belirtilen ID'ye sahip kayıt bulunamadı: " << id << std::endl;
        return false;
    }

    std::cout << "İhlal kaydı başarıyla silindi. ID: " << id << std::endl;
    return true;
}

bool DB::deleteViolationsByDateRange(const std::string& startDate, const std::string& endDate) {
    sqlite3 *db = nullptr;
    if (!initializeDatabase(&db)) {
        std::cerr << "Veritabanı başlatılamadı." << std::endl;
        return false;
    }

    const char *deleteSQL = "DELETE FROM Images WHERE Timestamp BETWEEN ? AND ?;";
    sqlite3_stmt *stmt;

    int exitCode = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
    if (exitCode != SQLITE_OK) {
        std::cerr << "SQL sorgusu hazırlanamadı: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, startDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, endDate.c_str(), -1, SQLITE_STATIC);

    exitCode = sqlite3_step(stmt);
    if (exitCode != SQLITE_DONE) {
        std::cerr << "Tarih aralığındaki kayıtlar silinemedi: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    std::cout << changes << " adet kayıt silindi (" << startDate << " - " << endDate << " arası)" << std::endl;
    return true;
}

bool DB::deleteViolationsByCardId(int cardId) {
    sqlite3 *db = nullptr;
    if (!initializeDatabase(&db)) {
        std::cerr << "Veritabanı başlatılamadı." << std::endl;
        return false;
    }

    const char *deleteSQL = "DELETE FROM Images WHERE CardID = ?;";
    sqlite3_stmt *stmt;

    int exitCode = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
       if (exitCode != SQLITE_OK) {
        std::cerr << "SQL sorgusu hazırlanamadı: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(stmt, 1, cardId);

    exitCode = sqlite3_step(stmt);
    if (exitCode != SQLITE_DONE) {
        std::cerr << "Kart ID'sine ait kayıtlar silinemedi: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    std::cout << "Kart ID " << cardId << " için " << changes << " adet kayıt silindi." << std::endl;
    return true;
}
