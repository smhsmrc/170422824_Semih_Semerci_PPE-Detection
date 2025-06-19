// #include "Detection/detection.h"
#include <torch/script.h> 
#include "../../../model/include/detection.h"
#include "../include/mainPage.h"
#include "../include/violation.h" 
#include "../include/loginPage.h" 
#include "../include/workerList.h" 
#include "../include/settings.h" // yol projenize göre değişebilir
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <QStackedWidget>
#include <QLabel>
#include <QDebug>
#include <iostream>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

// #include "Detection/detection.h"

MainPage::MainPage(QWidget *parent) : QWidget(parent) {
    // Ana widget arka plan rengi
    this->setStyleSheet("QWidget { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1e3c72, stop:1 #2a5298); }");
    
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Model ve detector yükleme
    std::string model_path = "../detection_model/ppe_detection.torchscript";
    model = detector.loadModel(model_path);
    class_names = {"gloves", "helmet", "human", "mask", "vest"};

    // ✨ Geliştirilmiş Menü bölümü
    QWidget *menuWidget = new QWidget();
    menuWidget->setFixedWidth(250);
    menuWidget->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #2c3e50, 
                stop:0.5 #34495e, 
                stop:1 #2c3e50);
            border-right: 3px solid #3498db;
        }
    )");
    
    QVBoxLayout *menuLayout = new QVBoxLayout(menuWidget);
    menuLayout->setContentsMargins(15, 20, 15, 20);
    menuLayout->setSpacing(10);

    // ✨ Logo/Başlık alanı
    QLabel *logoLabel = new QLabel("🛡️ KKD Takip Sistemi");
    logoLabel->setStyleSheet(R"(
        QLabel {
            color: #ecf0f1;
            font-size: 18px;
            font-weight: bold;
            padding: 15px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                stop:0 #3498db, 
                stop:1 #2980b9);
            border-radius: 12px;
            border: 2px solid #5dade2;
        }
    )");
    logoLabel->setAlignment(Qt::AlignCenter);
    menuLayout->addWidget(logoLabel);
    
    menuLayout->addSpacing(20);

    // ✨ Geliştirilmiş buton stili
    QString buttonStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #34495e, 
                stop:0.5 #2c3e50, 
                stop:1 #34495e);
            color: #ecf0f1;
            border: 2px solid #5d6d7e;
            border-radius: 12px;
            padding: 15px 10px;
            margin: 3px 0;
            font-size: 15px;
            font-weight: bold;
            text-align: left;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #3498db, 
                stop:0.5 #2980b9, 
                stop:1 #3498db);
            border: 2px solid #85c1e9;
            /* transform unsupported in Qt */
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #1abc9c, 
                stop:0.5 #16a085, 
                stop:1 #1abc9c);
            border: 2px solid #48c9b0;
        }
    )";

    // Stacked widget oluştur
    stackedWidget = new QStackedWidget();
    stackedWidget->setStyleSheet(R"(
        QStackedWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, 
                stop:0 #f8f9fa, 
                stop:1 #e9ecef);
            border-radius: 15px;
            margin: 10px;
        }
    )");

    // ✨ Ana Sayfa (Kamera ile) - Geliştirilmiş tasarım
    QWidget *page1 = new QWidget();
    page1->setStyleSheet("QWidget { background: transparent; }");
    QVBoxLayout *layout1 = new QVBoxLayout(page1);
    layout1->setContentsMargins(20, 20, 20, 20);
    layout1->setSpacing(15);
    
    // ✨ Geliştirilmiş başlık
    QLabel* label = new QLabel("📹 Canlı Kamera İzleme Sistemi");
    label->setStyleSheet(R"(
        QLabel {
            color: white;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                stop:0 #667eea, 
                stop:1 #764ba2);
            font-size: 22px;
            font-weight: bold;
            padding: 15px 20px;
            border-radius: 12px;
            border: 3px solid #a29bfe;
            /* box-shadow unsupported in Qt */
        }
    )");
    label->setAlignment(Qt::AlignCenter);
    layout1->addWidget(label);

    // ✨ Geliştirilmiş kamera kontrol paneli
    QWidget *controlPanel = new QWidget();
    controlPanel->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                stop:0 #ffffff, 
                stop:1 #f1f2f6);
            border-radius: 15px;
            border: 2px solid #ddd;
        }
    )");
    QHBoxLayout *cameraControlLayout = new QHBoxLayout(controlPanel);
    cameraControlLayout->setContentsMargins(20, 15, 20, 15);
    cameraControlLayout->setSpacing(15);
    
    // ✨ Geliştirilmiş başlat butonu
    startCameraBtn = new QPushButton("▶️  Kamerayı Başlat");
    startCameraBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #00b894, 
                stop:1 #00a085);
            color: white;
            font-weight: bold;
            border: none;
            border-radius: 12px;
            padding: 15px 25px;
            font-size: 16px;
            margin: 5px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #00cec9, 
                stop:1 #00b894);
            /* transform and box-shadow unsupported in Qt */
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #00a085, 
                stop:1 #008f7a);
            /* transform unsupported in Qt */
        }
        QPushButton:disabled {
            background: #bdc3c7;
            color: #7f8c8d;
        }
    )");
    
    // ✨ Geliştirilmiş durdur butonu
    stopCameraBtn = new QPushButton("⏸️  Kamerayı Durdur");
    stopCameraBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #e17055, 
                stop:1 #d63031);
            color: white;
            font-weight: bold;
            border: none;
            border-radius: 12px;
            padding: 15px 25px;
            font-size: 16px;
            margin: 5px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #fd79a8, 
                stop:1 #e84393);
            /* transform and box-shadow unsupported in Qt */
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #d63031, 
                stop:1 #c0392b);
            /* transform unsupported in Qt */
        }
        QPushButton:disabled {
            background: #bdc3c7;
            color: #7f8c8d;
        }
    )");

    // ✨ Geliştirilmiş durum göstergesi
    cameraStatusLabel = new QLabel("🔴 Kamera Durduruldu");
    cameraStatusLabel->setStyleSheet(R"(
        QLabel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                stop:0 #2d3436, 
                stop:1 #636e72);
            color: white;
            font-weight: bold;
            border: 3px solid #e17055;
            border-radius: 12px;
            padding: 12px 20px;
            font-size: 15px;
            min-width: 200px;
        }
    )");
    cameraStatusLabel->setAlignment(Qt::AlignCenter);

    // Lambda fonksiyonlarıyla buton bağlantıları
    connect(startCameraBtn, &QPushButton::clicked, [this]() {
        if (!cap.isOpened()) {
            qDebug() << "Kamera açılamadı!";
            return;
        }
        
        timer.start(25); // FPS optimizasyonu: 35ms -> 25ms (28fps -> 40fps)
        startCameraBtn->setEnabled(false);
        stopCameraBtn->setEnabled(true);
        
        cameraStatusLabel->setText("🟢 Kamera Aktif");
        cameraStatusLabel->setStyleSheet(R"(
            QLabel {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                    stop:0 #00b894, 
                    stop:1 #00cec9);
                color: white;
                font-weight: bold;
                border: 3px solid #55efc4;
                border-radius: 12px;
                padding: 12px 20px;
                font-size: 15px;
                min-width: 200px;
            }
        )");
        
        camLabel->setStyleSheet(R"(
            QLabel {
                border: 4px solid #00cec9;
                border-radius: 15px;
                background: #2d3436;
            }
        )");
    });

    connect(stopCameraBtn, &QPushButton::clicked, [this]() {
        timer.stop();
        startCameraBtn->setEnabled(true);
        stopCameraBtn->setEnabled(false);
        
        cameraStatusLabel->setText("🔴 Kamera Durduruldu");
        cameraStatusLabel->setStyleSheet(R"(
            QLabel {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                    stop:0 #2d3436, 
                    stop:1 #636e72);
                color: white;
                font-weight: bold;
                border: 3px solid #e17055;
                border-radius: 12px;
                padding: 12px 20px;
                font-size: 15px;
                min-width: 200px;
            }
        )");
        
        camLabel->clear();
        camLabel->setText("📷 Kamera Durduruldu\n\n'Kamerayı Başlat' butonuna basın");
        camLabel->setStyleSheet(R"(
            QLabel {
                border: 4px solid #636e72;
                border-radius: 15px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, 
                    stop:0 #2d3436, 
                    stop:1 #636e72);
                color: white;
                font-size: 18px;
                font-weight: bold;
            }
        )");
    });

    cameraControlLayout->addWidget(startCameraBtn);
    cameraControlLayout->addWidget(stopCameraBtn);
    cameraControlLayout->addStretch();
    cameraControlLayout->addWidget(cameraStatusLabel);

    layout1->addWidget(controlPanel);

    // ✨ Kamera ve KKD durum paneli için yatay layout
    QHBoxLayout *cameraAndStatusLayout = new QHBoxLayout();
    cameraAndStatusLayout->setSpacing(10); // Daha az spacing
    cameraAndStatusLayout->setContentsMargins(10, 0, 10, 0); // Kenar boşlukları
    
    // ✨ Geliştirilmiş kamera görüntü alanı - SABİT BOYUT
    camLabel = new QLabel();
    camLabel->setFixedSize(820, 600); // Daha küçük boyut - ekrana sığması için
    camLabel->setStyleSheet(R"(
        QLabel {
            border: 4px solid #636e72;
            border-radius: 15px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, 
                stop:0 #2d3436, 
                stop:1 #636e72);
            color: white;
            font-size: 18px;
            font-weight: bold;
        }
    )");
    camLabel->setAlignment(Qt::AlignCenter);
    camLabel->setText("📷 Kamera Durduruldu\n\n'Kamerayı Başlat' butonuna basın");
    camLabel->setScaledContents(true); // İçeriği otomatik ölçeklendir

    // 🛡️ KKD Durum Paneli Oluştur
    createKKDStatusPanel();

    cameraAndStatusLayout->addWidget(camLabel); // Stretch faktörü kaldırıldı
    cameraAndStatusLayout->addWidget(kKDStatusGroup); // Stretch faktörü kaldırıldı
    cameraAndStatusLayout->setStretchFactor(camLabel, 0); // Kamera sabit boyut
    cameraAndStatusLayout->setStretchFactor(kKDStatusGroup, 0); // KKD paneli sabit boyut

    layout1->addLayout(cameraAndStatusLayout);

    ViolationPage *violationPage = new ViolationPage();
    Settings *settings = new Settings();
    WorkerListPage *workerList = new WorkerListPage();
    
    stackedWidget->addWidget(page1);
    stackedWidget->addWidget(violationPage);
    stackedWidget->addWidget(settings);
    stackedWidget->addWidget(workerList);

    // ✨ Geliştirilmiş menü butonları
    QStringList labels = {"🏠 Ana Sayfa", "⚠️ İhlaller", "⚙️ Ayarlar", "👥 Çalışan Listesi", "🚪 Çıkış"};
    for (int i = 0; i < labels.size(); ++i) {
        QPushButton *btn = new QPushButton(labels[i]);
        btn->setStyleSheet(buttonStyle);
        menuLayout->addWidget(btn);
        menuButtons.append(btn);

        connect(btn, &QPushButton::clicked, this, [this, i, violationPage]() {
            stackedWidget->setCurrentIndex(i);
            
            if (i == 0) {
                // Ana sayfadayken kamera durumunu değiştirme
            } else {
                if (timer.isActive()) {
                    timer.stop();
                    startCameraBtn->setEnabled(true);
                    stopCameraBtn->setEnabled(false);
                    cameraStatusLabel->setText("🔴 Kamera Durduruldu");
                    cameraStatusLabel->setStyleSheet(R"(
                        QLabel {
                            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                                stop:0 #2d3436, 
                                stop:1 #636e72);
                            color: white;
                            font-weight: bold;
                            border: 3px solid #e17055;
                            border-radius: 12px;
                            padding: 12px 20px;
                            font-size: 15px;
                            min-width: 200px;
                        }
                    )");
                }
            }

            if (i == 1) {
                violationPage->loadViolations();
            }
            if(i == 4){
                // ✨ Tek pencere çıkış mantığı
                // Timer'ı durdur
                if (timer.isActive()) {
                    timer.stop();
                }
                
                // Kamerayı kapat
                if (cap.isOpened()) {
                    cap.release();
                }
                
                // Login sayfasına dön (sinyal gönder)
                emit logoutRequested();
            }
        });
    }

    menuLayout->addStretch();

    // Ana layout'u oluştur
    mainLayout->addWidget(menuWidget);
    mainLayout->addWidget(stackedWidget);

    // Kamera başlatma
    cap.open("/Users/semihsemerci/Desktop/sunum_odası/12.mov");
    if (!cap.isOpened()) {
        qDebug() << "Kamera açılamadı!";
        startCameraBtn->setEnabled(false);
        startCameraBtn->setText("❌ Kamera Bulunamadı");
        return;

    
    // ⚡ Performans optimizasyonları
    // Buffer boyutunu minimize et - daha düşük latency
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
    
    // Canlı kamera için resolution ayarları
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap.set(cv::CAP_PROP_FPS, 30);
    
    // Exposure ve diğer ayarlar
    cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25); // Manuel exposure
    cap.set(cv::CAP_PROP_EXPOSURE, 0.5); // Orta seviye exposure

    connect(&timer, &QTimer::timeout, this, &MainPage::updateFrame);
    
    startCameraBtn->setEnabled(true);
    stopCameraBtn->setEnabled(false);
}

// 🛡️ KKD Durum Paneli Oluşturma Fonksiyonu
void MainPage::createKKDStatusPanel() {
    kKDStatusGroup = new QGroupBox("🛡️ KKD Durum İzleme");
    
    // SABİT BOYUT SINIRLAMASI - Ekrana sığması için
    kKDStatusGroup->setFixedWidth(280); // Daha küçük genişlik
    kKDStatusGroup->setMaximumHeight(600); // Kamera ile aynı yükseklik
    
    kKDStatusGroup->setStyleSheet(R"(
        QGroupBox {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #ffffff, 
                stop:1 #f8f9fa);
            border: 3px solid #3498db;
            border-radius: 15px;
            margin-top: 15px;
            padding-top: 10px;
            font-size: 16px;
            font-weight: bold;
            color: #2c3e50;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 5px 15px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                stop:0 #3498db, 
                stop:1 #2980b9);
            color: white;
            border-radius: 8px;
            border: 2px solid #5dade2;
        }
    )");

    QVBoxLayout *statusLayout = new QVBoxLayout(kKDStatusGroup);
    statusLayout->setSpacing(12);
    statusLayout->setContentsMargins(15, 25, 15, 15);

    // ✨ Takip ID ve Progress Bar
    QHBoxLayout *trackLayout = new QHBoxLayout();
    trackIdLabel = new QLabel("👤 Takip ID: --");
    trackIdLabel->setStyleSheet(R"(
        QLabel {
            color: #2c3e50;
            font-size: 14px;
            font-weight: bold;
            padding: 8px 12px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                stop:0 #ecf0f1, 
                stop:1 #bdc3c7);
            border-radius: 8px;
            border: 2px solid #95a5a6;
        }
    )");

    detectionProgressBar = new QProgressBar();
    detectionProgressBar->setRange(0, 100);
    detectionProgressBar->setValue(0);
    detectionProgressBar->setTextVisible(true);
    detectionProgressBar->setFormat("İlerleme: %p%");
    detectionProgressBar->setStyleSheet(R"(
        QProgressBar {
            border: 2px solid #bdc3c7;
            border-radius: 8px;
            text-align: center;
            font-weight: bold;
            background: #ecf0f1;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                stop:0 #3498db, 
                stop:1 #2980b9);
            border-radius: 6px;
        }
    )");

    trackLayout->addWidget(trackIdLabel);
    trackLayout->addWidget(detectionProgressBar);
    statusLayout->addLayout(trackLayout);

    // ✨ Divider
    QFrame *divider1 = new QFrame();
    divider1->setFrameShape(QFrame::HLine);
    divider1->setStyleSheet("QFrame { color: #bdc3c7; background: #bdc3c7; }");
    statusLayout->addWidget(divider1);

    // ✨ KKD Durumları için Grid Layout
    QGridLayout *kKDGrid = new QGridLayout();
    kKDGrid->setSpacing(10);

    // KKD Item'larını oluştur
    helmetStatusLabel = createKKDStatusItem("🪖", "Kask", false);
    vestStatusLabel = createKKDStatusItem("🦺", "Yelek", false);
    glovesStatusLabel = createKKDStatusItem("🧤", "Eldiven", false);
    maskStatusLabel = createKKDStatusItem("😷", "Maske", false);
    idCardStatusLabel = createKKDStatusItem("🆔", "ID Kart", false);

    kKDGrid->addWidget(helmetStatusLabel, 0, 0);
    kKDGrid->addWidget(vestStatusLabel, 0, 1);
    kKDGrid->addWidget(glovesStatusLabel, 1, 0);
    kKDGrid->addWidget(maskStatusLabel, 1, 1);
    kKDGrid->addWidget(idCardStatusLabel, 2, 0);

    statusLayout->addLayout(kKDGrid);

    // ✨ Divider
    QFrame *divider2 = new QFrame();
    divider2->setFrameShape(QFrame::HLine);
    divider2->setStyleSheet("QFrame { color: #bdc3c7; background: #bdc3c7; }");
    statusLayout->addWidget(divider2);

    // ✨ Card ID Gösterimi
    cardIdLabel = new QLabel("🆔 Card ID: --");
    cardIdLabel->setStyleSheet(R"(
        QLabel {
            color: #2c3e50;
            font-size: 14px;
            font-weight: bold;
            padding: 12px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                stop:0 #f39c12, 
                stop:1 #e67e22);
            color: white;
            border-radius: 10px;
            border: 2px solid #f1c40f;
            text-align: center;
        }
    )");
    cardIdLabel->setAlignment(Qt::AlignCenter);
    statusLayout->addWidget(cardIdLabel);

    // ✨ Genel Durum
    overallStatusLabel = new QLabel("🔍 Tespit Bekleniyor...");
    overallStatusLabel->setStyleSheet(R"(
        QLabel {
            color: white;
            font-size: 15px;
            font-weight: bold;
            padding: 15px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                stop:0 #95a5a6, 
                stop:1 #7f8c8d);
            border-radius: 12px;
            border: 3px solid #bdc3c7;
            text-align: center;
        }
    )");
    overallStatusLabel->setAlignment(Qt::AlignCenter);
    statusLayout->addWidget(overallStatusLabel);

    statusLayout->addStretch();
}

// 🛡️ KKD Status Item Oluşturma Helper Fonksiyonu
QLabel* MainPage::createKKDStatusItem(const QString& icon, const QString& name, bool detected) {
    QLabel* label = new QLabel(QString("%1 %2").arg(icon, name));
    updateKKDItemStyle(label, detected);
    return label;
}

// 🛡️ KKD Item Style Güncelleme Fonksiyonu
void MainPage::updateKKDItemStyle(QLabel* label, bool detected) {
    if (detected) {
        label->setStyleSheet(R"(
            QLabel {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                    stop:0 #27ae60, 
                    stop:1 #2ecc71);
                color: white;
                font-weight: bold;
                border: 3px solid #58d68d;
                border-radius: 10px;
                padding: 12px 8px;
                font-size: 13px;
                text-align: center;
            }
        )");
    } else {
        label->setStyleSheet(R"(
            QLabel {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                    stop:0 #e74c3c, 
                    stop:1 #c0392b);
                color: white;
                font-weight: bold;
                border: 3px solid #ec7063;
                border-radius: 10px;
                padding: 12px 8px;
                font-size: 13px;
                text-align: center;
            }
        )");
    }
    label->setAlignment(Qt::AlignCenter);
}

// 🎨 KKD Item Style - Settings ile birlikte (Aktif/Pasif)
void MainPage::updateKKDItemStyleWithSettings(QLabel* label, bool detected, bool isEnabled) {
    if (!isEnabled) {
        // Kontrol edilmiyor - Pasif görünüm (gri ve şeffaf)
        label->setStyleSheet(R"(
            QLabel {
                background: rgba(149, 165, 166, 0.3);
                color: rgba(255, 255, 255, 0.5);
                font-weight: bold;
                border: 2px solid rgba(149, 165, 166, 0.4);
                border-radius: 10px;
                padding: 12px 8px;
                font-size: 13px;
                text-align: center;
            }
        )");
    } else if (detected) {
        // Tespit edildi - Yeşil
        label->setStyleSheet(R"(
            QLabel {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                    stop:0 #27ae60, 
                    stop:1 #2ecc71);
                color: white;
                font-weight: bold;
                border: 3px solid #58d68d;
                border-radius: 10px;
                padding: 12px 8px;
                font-size: 13px;
                text-align: center;
            }
        )");
    } else {
        // Tespit edilmedi - Kırmızı
        label->setStyleSheet(R"(
            QLabel {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                    stop:0 #e74c3c, 
                    stop:1 #c0392b);
                color: white;
                font-weight: bold;
                border: 3px solid #ec7063;
                border-radius: 10px;
                padding: 12px 8px;
                font-size: 13px;
                text-align: center;
            }
        )");
    }
    label->setAlignment(Qt::AlignCenter);
}

// 🛡️ KKD Durum Güncelleme Fonksiyonu
void MainPage::updateKKDStatus(const Person& person, int frameId) {
    // Takip ID güncelle
    trackIdLabel->setText(QString("👤 Takip ID: %1").arg(person.trackerId));
    
    // Progress bar güncelle (100 frame üzerinden)
    int progress = std::min(100, (frameId - person.detectedFrameId) * 100 / 100);
    detectionProgressBar->setValue(progress);
    
    // KKD durumlarını güncelle - Tümü görünür, kontrol edilmeyenler pasif
    std::map<std::string, bool> kKDSettings = Settings::getKKDSettings();
    
    // Her KKD için: aktifse normal güncelle, değilse pasif görünüm
    updateKKDItemStyleWithSettings(helmetStatusLabel, person.helmet, kKDSettings["helmet"]);
    updateKKDItemStyleWithSettings(vestStatusLabel, person.vest, kKDSettings["vest"]);
    updateKKDItemStyleWithSettings(glovesStatusLabel, person.gloves, kKDSettings["gloves"]);
    updateKKDItemStyleWithSettings(maskStatusLabel, person.mask, kKDSettings["mask"]);
    updateKKDItemStyleWithSettings(idCardStatusLabel, person.card_status, kKDSettings["id_card"]);
    
    // Card ID güncelle - Settings'e göre
    if (!kKDSettings["id_card"]) {
        // ID Card kontrolü kapalı - pasif görünüm
        cardIdLabel->setText("🆔 Card ID: Kontrol Edilmiyor");
        cardIdLabel->setStyleSheet(R"(
            QLabel {
                color: rgba(255, 255, 255, 0.5);
                font-size: 14px;
                font-weight: bold;
                padding: 12px;
                background: rgba(149, 165, 166, 0.3);
                border-radius: 10px;
                border: 2px solid rgba(149, 165, 166, 0.4);
                text-align: center;
            }
        )");
    } else if (person.cardId != -1) {
        cardIdLabel->setText(QString("🆔 Card ID: %1").arg(person.cardId));
        cardIdLabel->setStyleSheet(R"(
            QLabel {
                color: white;
                font-size: 14px;
                font-weight: bold;
                padding: 12px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                    stop:0 #27ae60, 
                    stop:1 #2ecc71);
                border-radius: 10px;
                border: 2px solid #58d68d;
                text-align: center;
            }
        )");
    } else {
        cardIdLabel->setText("🆔 Card ID: Tespit Edilemiyor...");
        cardIdLabel->setStyleSheet(R"(
            QLabel {
                color: white;
                font-size: 14px;
                font-weight: bold;
                padding: 12px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                    stop:0 #f39c12, 
                    stop:1 #e67e22);
                border-radius: 10px;
                border: 2px solid #f1c40f;
                text-align: center;
            }
        )");
    }
    
    // Genel durum güncelle
    if (progress >= 100) {
        if (person.isFullyProtected()) {
            overallStatusLabel->setText("✅ TAM KORUMA SAĞLANDI");
            overallStatusLabel->setStyleSheet(R"(
                QLabel {
                    color: white;
                    font-size: 15px;
                    font-weight: bold;
                    padding: 15px;
                    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                        stop:0 #27ae60, 
                        stop:1 #2ecc71);
                    border-radius: 12px;
                    border: 3px solid #58d68d;
                    text-align: center;
                }
            )");
        } else {
            overallStatusLabel->setText("❌ EKSİK KKD TESPİT EDİLDİ");
            overallStatusLabel->setStyleSheet(R"(
                QLabel {
                    color: white;
                    font-size: 15px;
                    font-weight: bold;
                    padding: 15px;
                    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                        stop:0 #e74c3c, 
                        stop:1 #c0392b);
                    border-radius: 12px;
                    border: 3px solid #ec7063;
                    text-align: center;
                }
            )");
        }
    } else {
        overallStatusLabel->setText(QString("🔍 Analiz Devam Ediyor... (%1%)").arg(progress));
        overallStatusLabel->setStyleSheet(R"(
            QLabel {
                color: white;
                font-size: 15px;
                font-weight: bold;
                padding: 15px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                    stop:0 #3498db, 
                    stop:1 #2980b9);
                border-radius: 12px;
                border: 3px solid #5dade2;
                text-align: center;
            }
        )");
    }
}

// 🔄 KKD Durum Sıfırlama Fonksiyonu (İnsan tespit edilmediğinde)
void MainPage::resetKKDStatus() {
    // Takip ID sıfırla
    trackIdLabel->setText("👤 Takip ID: -");
    
    // Progress bar sıfırla
    detectionProgressBar->setValue(0);
    
    // Tüm KKD durumlarını "Bekliyor" durumuna getir
    std::map<std::string, bool> kKDSettings = Settings::getKKDSettings();
    
    updateKKDItemStyleWithSettings(helmetStatusLabel, false, kKDSettings["helmet"]);
    updateKKDItemStyleWithSettings(vestStatusLabel, false, kKDSettings["vest"]);
    updateKKDItemStyleWithSettings(glovesStatusLabel, false, kKDSettings["gloves"]);
    updateKKDItemStyleWithSettings(maskStatusLabel, false, kKDSettings["mask"]);
    updateKKDItemStyleWithSettings(idCardStatusLabel, false, kKDSettings["id_card"]);
    
    // Card ID sıfırla
    if (!kKDSettings["id_card"]) {
        cardIdLabel->setText("🆔 Card ID: Kontrol Edilmiyor");
        cardIdLabel->setStyleSheet(R"(
            QLabel {
                color: rgba(255, 255, 255, 0.5);
                font-size: 14px;
                font-weight: bold;
                padding: 12px;
                background: rgba(149, 165, 166, 0.3);
                border-radius: 10px;
                border: 2px solid rgba(149, 165, 166, 0.4);
                text-align: center;
            }
        )");
    } else {
        cardIdLabel->setText("🆔 Card ID: Bekliyor...");
        cardIdLabel->setStyleSheet(R"(
            QLabel {
                color: rgba(255, 255, 255, 0.7);
                font-size: 14px;
                font-weight: bold;
                padding: 12px;
                background: rgba(149, 165, 166, 0.3);
                border-radius: 10px;
                border: 2px solid rgba(149, 165, 166, 0.4);
                text-align: center;
            }
        )");
    }
    
    // Genel durumu sıfırla
    overallStatusLabel->setText("⏳ İNSAN TESPİTİ BEKLENİYOR");
    overallStatusLabel->setStyleSheet(R"(
        QLabel {
            color: rgba(255, 255, 255, 0.8);
            font-size: 15px;
            font-weight: bold;
            padding: 15px;
            background: rgba(149, 165, 166, 0.4);
            border-radius: 12px;
            border: 3px solid rgba(149, 165, 166, 0.6);
            text-align: center;
        }
    )");
}

MainPage::~MainPage() {
    timer.stop();
    if (cap.isOpened())
        cap.release();
}

void MainPage::updateFrame() {
    static int frame_id = 0;
    static int skip_counter = 0;
    static cv::Mat last_detected_frame; // Son detection sonucunu sakla
    
    cv::Mat frame;
    cap >> frame;

    if (frame.empty()) {
        qDebug() << "Frame boş!";
        return;
    }

    // FPS optimizasyonu: Her 3 frame'den birini işle (daha az CPU kullanımı)
    skip_counter++;
    if (skip_counter % 3 != 0) {
        // Eğer daha önce detection yapılmışsa, son detection sonucunu kullan
        if (!last_detected_frame.empty()) {
            // Son detection frame'ini göster (bounding box'lar korunur) - SABİT BOYUT - HIZLI
            static cv::Mat display_frame; // Static kullanarak heap allocation'ı azalt
            cv::cvtColor(last_detected_frame, display_frame, cv::COLOR_BGR2RGB);
            QImage img(display_frame.data, display_frame.cols, display_frame.rows, display_frame.step, QImage::Format_RGB888);
            QPixmap pixmap = QPixmap::fromImage(img).scaled(820, 600, Qt::KeepAspectRatio, Qt::FastTransformation); // SmoothTransformation -> FastTransformation
            camLabel->setPixmap(pixmap);
        } else {
            // İlk frame'lerde henüz detection yoksa sadece frame'i göster - SABİT BOYUT - HIZLI
            static cv::Mat display_frame2; // Static kullanarak heap allocation'ı azalt
            cv::cvtColor(frame, display_frame2, cv::COLOR_BGR2RGB);
            QImage img(display_frame2.data, display_frame2.cols, display_frame2.rows, display_frame2.step, QImage::Format_RGB888);
            QPixmap pixmap = QPixmap::fromImage(img).scaled(820, 600, Qt::KeepAspectRatio, Qt::FastTransformation); // SmoothTransformation -> FastTransformation
            camLabel->setPixmap(pixmap);
        }
        return;
    }
    
    frame_id++;
    int mod_id = 1;
    
    // OCR artık her 10 frame'de bir vest crop ile birlikte yapılıyor (daha hızlı)
    cv::Mat detected_frame = detector.runDetection(frame, frame_id, model, class_names, true);
    detector.drawROI(detected_frame);

    // 🛡️ KKD durum panelini güncelle - performans için her 10 frame'de bir
    if (frame_id % 10 == 0) { // Her 10 frame'de bir güncelle
        Person currentPerson = DetectionModule::getCurrentPerson();
        if (currentPerson.trackerId != -1) { // ✅ Eğer geçerli person tespit edildiyse
            updateKKDStatus(currentPerson, frame_id);
        } else {
            // ✅ İnsan tespit edilmediğinde UI'ı sıfırla
            resetKKDStatus();
        }
    }

    // Son detection sonucunu sakla
    last_detected_frame = detected_frame.clone();

    // BGR'den RGB'ye çevir - SABİT BOYUT KORUMA - HIZLI
    static cv::Mat display_frame3; // Static kullanarak heap allocation'ı azalt
    cv::cvtColor(detected_frame, display_frame3, cv::COLOR_BGR2RGB);
    QImage img(display_frame3.data, display_frame3.cols, display_frame3.rows, display_frame3.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(img).scaled(820, 600, Qt::KeepAspectRatio, Qt::FastTransformation); // SmoothTransformation -> FastTransformation

    camLabel->setPixmap(pixmap);
    // camLabel->setFixedSize(pixmap.size()); // Bu satır kaldırıldı - sabit boyut korunuyor
}
