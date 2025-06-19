#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <torch/script.h> 
#include "../include/mainPage.h"
#include "../../../model/include/detection.h"
#include "../../../model/include/person.h"
#include <QWidget>
#include <QPushButton>
#include <QList>
#include <QVBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QTimer>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QProgressBar>
#include <QFrame>


class MainPage : public QWidget {
    Q_OBJECT

public:
    explicit MainPage(QWidget *parent = nullptr);
    ~MainPage();

signals:
    void logoutRequested(); // ✨ Logout sinyali

private slots:
    void updateFrame();
    void updateKKDStatus(const Person& person, int frameId);
    void resetKKDStatus(); // ✅ Yeni fonksiyon: İnsan tespit edilmediğinde UI'ı sıfırla

private:
    void createKKDStatusPanel();
    QLabel* createKKDStatusItem(const QString& icon, const QString& name, bool detected);
    void updateKKDItemStyle(QLabel* label, bool detected);
    void updateKKDItemStyleWithSettings(QLabel* label, bool detected, bool isEnabled);
    // UI Elements
    QStackedWidget *stackedWidget;
    QList<QPushButton*> menuButtons;
    QLabel *camLabel;

    QPushButton *startCameraBtn;
    QPushButton *stopCameraBtn;
    QLabel *cameraStatusLabel;

    // 🛡️ KKD Durum Paneli Widget'ları
    QGroupBox *kKDStatusGroup;
    QLabel *helmetStatusLabel;
    QLabel *vestStatusLabel; 
    QLabel *glovesStatusLabel;
    QLabel *maskStatusLabel;
    QLabel *idCardStatusLabel;  // ID Card için KKD status
    QLabel *cardIdLabel;        // Card ID numarasını göster
    QLabel *overallStatusLabel;
    QProgressBar *detectionProgressBar;
    QLabel *trackIdLabel;

    // Kamera ve Detection
    cv::VideoCapture cap;
    QTimer timer;
    DetectionModule detector;
    torch::jit::script::Module model;
    std::vector<std::string> class_names;
};

#endif // MAINPAGE_H