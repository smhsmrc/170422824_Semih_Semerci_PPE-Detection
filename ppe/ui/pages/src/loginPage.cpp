#include <torch/script.h> 
#include "../include/loginPage.h"
#include "../include/mainPage.h"
#include "../../../model/include/detection.h" 
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QWidget>
#include <QList>
#include <QString>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QGraphicsBlurEffect>

LoginPage::LoginPage(QWidget *parent) : QMainWindow(parent) {
    // ✨ Ana StackedWidget - Tek pencere çözümü
    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    // ✨ Login Sayfası
    QWidget *loginWidget = new QWidget();
    QVBoxLayout *loginLayout = new QVBoxLayout(loginWidget);
    loginLayout->setContentsMargins(0, 0, 0, 0);

    // 🖼️ Ana Fotoğraf Arka Plan + Overlay
    QWidget *backgroundWidget = new QWidget();
    backgroundWidget->setStyleSheet(R"(
        QWidget {
            background-image: url(/Users/semihsemerci/Desktop/tez/ui/images/main_page_background.jpeg);
            background-repeat: no-repeat;
            background-position: center;
            background-size: cover;
        }
    )");

    // 🌈 Gradient Overlay - Ana fotoğrafın üzerine
    QWidget *overlayWidget = new QWidget();
    overlayWidget->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(25, 32, 72, 0.7),
                stop:0.3 rgba(42, 82, 152, 0.6),
                stop:0.6 rgba(74, 144, 226, 0.5),
                stop:1 rgba(116, 185, 255, 0.4));
        }
    )");

    // 🎨 Glassmorphism Ana Container - Daha geniş
    QWidget *mainContainer = new QWidget();
    mainContainer->setStyleSheet(R"(
        QWidget {
            background: rgba(255, 255, 255, 0.15);
            border: 2px solid rgba(255, 255, 255, 0.2);
            border-radius: 30px;
            backdrop-filter: blur(20px);
        }
    )");
    mainContainer->setFixedSize(580, 480);

    // 🌟 İkinci katman gölge efekti
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(40);
    shadowEffect->setColor(QColor(0, 0, 0, 120));
    shadowEffect->setOffset(0, 20);
    mainContainer->setGraphicsEffect(shadowEffect);

    QVBoxLayout *containerLayout = new QVBoxLayout(mainContainer);
    containerLayout->setContentsMargins(60, 50, 60, 50);
    containerLayout->setSpacing(22);

    // 🛡️ Logo Container - Daha büyük ve etkili
    QWidget *logoContainer = new QWidget();
    logoContainer->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(52, 152, 219, 0.2),
                stop:1 rgba(155, 89, 182, 0.2));
            border-radius: 45px;
            border: 1px solid rgba(255, 255, 255, 0.3);
        }
    )");
    logoContainer->setFixedSize(90, 90);

    QVBoxLayout *logoLayout = new QVBoxLayout(logoContainer);
    logoLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *iconLabel = new QLabel();
    iconLabel->setText("🛡️");
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet(R"(
        QLabel {
            font-size: 52px;
            background: transparent;
            color: white;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.3);
        }
    )");
    logoLayout->addWidget(iconLabel);

    // 🎯 Ana Başlık - Daha küçük ve sığacak şekilde
    QLabel *titleLabel = new QLabel("KİŞİSEL KORUYUCU\nDONANIM TESPİTİ");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 20px;
            font-weight: bold;
            color: white;
            line-height: 1.2;
            letter-spacing: 1px;
            text-shadow: 2px 2px 8px rgba(0, 0, 0, 0.5);
            margin: 10px 0px;
        }
    )");

    // 📱 Alt başlık - Daha kısa ve sığacak şekilde
    QLabel *subtitleLabel = new QLabel("Güvenlik ve İş Sağlığı");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 13px;
            color: rgba(255, 255, 255, 0.9);
            font-weight: 300;
            letter-spacing: 1px;
            margin-bottom: 15px;
        }
    )");

    // 🛡️ KKD Emojileri - Kask, Yelek, Maske, Eldiven
    QWidget *featuresContainer = new QWidget();
    QHBoxLayout *featuresLayout = new QHBoxLayout(featuresContainer);
    featuresLayout->setContentsMargins(0, 5, 0, 5);
    
    QStringList features = {"⛑️", "🦺", "😷", "🧤"}; // Kask, Yelek, Maske, Eldiven
    for (const QString &feature : features) {
        QLabel *featureIcon = new QLabel(feature);
        featureIcon->setAlignment(Qt::AlignCenter);
        featureIcon->setStyleSheet(R"(
            QLabel {
                font-size: 22px;
                background: rgba(255, 255, 255, 0.15);
                border-radius: 18px;
                padding: 6px;
                margin: 0px 3px;
                border: 1px solid rgba(255, 255, 255, 0.2);
            }
        )");
        featuresLayout->addWidget(featureIcon);
    }

    // 🚀 Premium Giriş Butonu - Geniş layout için
    startButton = new QPushButton("🚀 SİSTEME GİRİŞ");
    startButton->setFixedSize(320, 60);
    startButton->setCursor(Qt::PointingHandCursor);
    startButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(52, 152, 219, 0.9),
                stop:0.5 rgba(155, 89, 182, 0.9),
                stop:1 rgba(52, 152, 219, 0.9));
            color: white;
            border: 2px solid rgba(255, 255, 255, 0.3);
            border-radius: 28px;
            font-size: 16px;
            font-weight: bold;
            letter-spacing: 1px;
            text-shadow: 1px 1px 3px rgba(0, 0, 0, 0.3);
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(52, 152, 219, 1),
                stop:0.5 rgba(155, 89, 182, 1),
                stop:1 rgba(52, 152, 219, 1));
            border: 2px solid rgba(255, 255, 255, 0.5);
            transform: translateY(-3px);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(41, 128, 185, 1),
                stop:0.5 rgba(142, 68, 173, 1),
                stop:1 rgba(41, 128, 185, 1));
            transform: translateY(1px);
        }
    )");

    // 🚪 Çıkış Butonu
    QPushButton *exitButton = new QPushButton("🚪 ÇIKIŞ");
    exitButton->setFixedSize(320, 50);
    exitButton->setCursor(Qt::PointingHandCursor);
    exitButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(231, 76, 60, 0.8),
                stop:0.5 rgba(192, 57, 43, 0.8),
                stop:1 rgba(231, 76, 60, 0.8));
            color: white;
            border: 2px solid rgba(255, 255, 255, 0.3);
            border-radius: 22px;
            font-size: 14px;
            font-weight: bold;
            letter-spacing: 1px;
            text-shadow: 1px 1px 3px rgba(0, 0, 0, 0.3);
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(231, 76, 60, 1),
                stop:0.5 rgba(192, 57, 43, 1),
                stop:1 rgba(231, 76, 60, 1));
            border: 2px solid rgba(255, 255, 255, 0.5);
            transform: translateY(-2px);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(192, 57, 43, 1),
                stop:0.5 rgba(155, 39, 176, 1),
                stop:1 rgba(192, 57, 43, 1));
            transform: translateY(1px);
        }
    )");

    // 💫 Animasyonlu Alt Çizgi
    QWidget *bottomLine = new QWidget();
    bottomLine->setFixedHeight(3);
    bottomLine->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(52, 152, 219, 0.7),
                stop:0.5 rgba(155, 89, 182, 0.7),
                stop:1 rgba(52, 152, 219, 0.7));
            border-radius: 1px;
        }
    )");

    // 🎨 Layout'a elementleri ekle - Optimize spacing
    containerLayout->addWidget(logoContainer, 0, Qt::AlignCenter);
    containerLayout->addSpacing(3);
    containerLayout->addWidget(titleLabel);
    containerLayout->addWidget(subtitleLabel);
    containerLayout->addWidget(featuresContainer);
    containerLayout->addSpacing(12);
    containerLayout->addWidget(startButton, 0, Qt::AlignCenter);
    containerLayout->addSpacing(6);
    containerLayout->addWidget(exitButton, 0, Qt::AlignCenter);
    containerLayout->addSpacing(8);
    containerLayout->addWidget(bottomLine);
    containerLayout->addStretch();

    // 📍 Sağ alt köşe yerleşimi - DÜZELTME
    QVBoxLayout *overlayLayout = new QVBoxLayout(overlayWidget);
    overlayLayout->setContentsMargins(0, 0, 50, 50); // Sağ ve alt margin
    overlayLayout->addStretch(); // Üstte boşluk
    
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addStretch(); // Solda boşluk
    hLayout->addWidget(mainContainer);
    
    overlayLayout->addLayout(hLayout);

    // Background widget'a overlay ekle
    QVBoxLayout *bgLayout = new QVBoxLayout(backgroundWidget);
    bgLayout->setContentsMargins(0, 0, 0, 0);
    bgLayout->addWidget(overlayWidget);

    loginLayout->addWidget(backgroundWidget);

    // ✨ Login sayfasını stack'e ekle
    stack->addWidget(loginWidget); // Index 0

    // ✨ MainPage'i burada oluştur ve stack'e ekle
    MainPage *mainPage = new MainPage(this);
    stack->addWidget(mainPage); // Index 1

    // ✨ Başlangıçta login sayfasını göster
    stack->setCurrentIndex(0);

    // 🖥️ Tam ekran olarak aç
    this->showFullScreen();
    QApplication::processEvents();

    // 🎬 Etkileyici giriş animasyonu
    QPropertyAnimation *slideInAnimation = new QPropertyAnimation(mainContainer, "geometry");
    slideInAnimation->setDuration(1200);
    slideInAnimation->setEasingCurve(QEasingCurve::OutElastic);
    
    // Hedef konum (sağ alt) - Geniş container için
    int targetX = this->width() - 580 - 50;
    int targetY = this->height() - 480 - 50;
    QRect endGeometry(targetX, targetY, 580, 480);
    
    // Başlangıç konumu (sağ dışı)
    QRect startGeometry(this->width() + 100, targetY, 580, 480);
    
    mainContainer->setGeometry(startGeometry);
    slideInAnimation->setStartValue(startGeometry);
    slideInAnimation->setEndValue(endGeometry);
    slideInAnimation->start();

    // ✨ Navigasyon - Sisteme Giriş
    QObject::connect(startButton, &QPushButton::clicked, [this, mainContainer]() {
        // 🎬 Zoom-out çıkış animasyonu
        QPropertyAnimation *zoomOutAnimation = new QPropertyAnimation(mainContainer, "geometry");
        zoomOutAnimation->setDuration(600);
        zoomOutAnimation->setEasingCurve(QEasingCurve::InBack);
        
        QRect currentGeometry = mainContainer->geometry();
        QRect exitGeometry(currentGeometry.center().x() - 50, currentGeometry.center().y() - 50, 100, 100);
        
        zoomOutAnimation->setStartValue(currentGeometry);
        zoomOutAnimation->setEndValue(exitGeometry);
        
        QObject::connect(zoomOutAnimation, &QPropertyAnimation::finished, [this]() {
            stack->setCurrentIndex(1);
        });
        
        zoomOutAnimation->start();
    });

    // 🚪 Çıkış Butonu - Uygulamayı Sonlandır
    QObject::connect(exitButton, &QPushButton::clicked, [this, mainContainer]() {
        // 🎬 Fade-out çıkış animasyonu
        QPropertyAnimation *fadeOutAnimation = new QPropertyAnimation(mainContainer, "geometry");
        fadeOutAnimation->setDuration(500);
        fadeOutAnimation->setEasingCurve(QEasingCurve::InQuart);
        
        QRect currentGeometry = mainContainer->geometry();
        QRect exitGeometry(currentGeometry.x(), this->height() + 100, currentGeometry.width(), currentGeometry.height());
        
        fadeOutAnimation->setStartValue(currentGeometry);
        fadeOutAnimation->setEndValue(exitGeometry);
        
        QObject::connect(fadeOutAnimation, &QPropertyAnimation::finished, [this]() {
            QApplication::quit(); // Uygulamayı sonlandır
        });
        
        fadeOutAnimation->start();
    });

    // ✨ MainPage'den çıkış yapıldığında login'e dön
    QObject::connect(mainPage, &MainPage::logoutRequested, [this, mainContainer]() {
        stack->setCurrentIndex(0);
        
        // Geri gelirken animasyon - Geniş container için
        int targetX = this->width() - 580 - 50;
        int targetY = this->height() - 480 - 50;
        QRect targetGeometry(targetX, targetY, 580, 480);
        QRect startGeometry(this->width() + 100, targetY, 580, 480);
        
        mainContainer->setGeometry(startGeometry);
        
        QPropertyAnimation *returnAnimation = new QPropertyAnimation(mainContainer, "geometry");
        returnAnimation->setDuration(800);
        returnAnimation->setEasingCurve(QEasingCurve::OutCubic);
        returnAnimation->setStartValue(startGeometry);
        returnAnimation->setEndValue(targetGeometry);
        returnAnimation->start();
    });
}

LoginPage::~LoginPage() {
    // Temizleme işlemleri
}
