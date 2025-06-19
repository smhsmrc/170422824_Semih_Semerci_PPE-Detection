#ifndef DB_H
#define DB_H

#include <opencv2/opencv.hpp>
#include <sqlite3.h>
#include <vector>
#include <string>
#include <variant>

class DB {
public:
    struct Tensor {
        std::string trackId;
        std::vector<uchar> imageData;
        std::string timestamp;
        bool helmet;
        bool vest;
        bool gloves;
        bool mask;
        bool cart_status; 
        int card_id;
        std::string firstName;
        std::string lastName;
        std::string department;
    };

    struct Worker {
        std::string firstName;
        std::string lastName;
        int workerNumber;
        std::string department;
    };

    DB();

    // ✅ DÜZELTME: cart_status parametresi eklendi
    void saveCroppedImage(cv::Mat cropped_image, const std::string &trackIdStr, 
                         bool helmet, bool vest, bool gloves, bool mask, 
                         int card_id = -1, bool cart_status = false);
    
    Tensor getLastViolationFromDB();
    std::vector<Tensor> getAllViolationsFromDB();

    bool deleteViolationByTimestamp(const std::string& timestamp);
    bool deleteAllViolations();
    bool deleteViolationById(int id);
    bool deleteViolationsByDateRange(const std::string& startDate, const std::string& endDate);
    bool deleteViolationsByCardId(int cardId);

    bool initializeDatabase(sqlite3 **db);
    bool createTableIfNotExists(sqlite3 *db);
    
    // ✅ DÜZELTME: Parametre sırası düzeltildi
    bool insertImageToDB(sqlite3 *db, const std::string &trackIdStr, const std::vector<uchar> &imageData, 
                        bool helmet, bool vest, bool gloves, bool mask, bool cart_status, int card_id = -1);

    // WorkerList DB fonksiyonları
    bool insertWorker(const std::string& firstName, const std::string& lastName, int workerNumber, const std::string& department);
    std::vector<Worker> getAllWorkersFromDB();
    bool removeWorkerByNumber(int workerNumber);

private:
    bool openDatabase(sqlite3 **db);
};

#endif // DB_H
