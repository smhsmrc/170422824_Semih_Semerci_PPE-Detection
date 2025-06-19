#include "../include/settings.h"
#include "../../db/include/db.h"  
#include "../../model/include/mail.h"  
#include <QDialog>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox> 
#include <QSettings>  // QSettings için header eklendi
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

Settings::Settings(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 10, 20, 10); // Alt boşluğu da azalt
    mainLayout->setSpacing(20); // Widget'lar arası boşluğu azalt

    // Mail Bildirim Ayarları Grubu
    QGroupBox *mailGroup = new QGroupBox("Mail Bildirim Ayarları");
    mailGroup->setMaximumWidth(800); // Maksimum genişlik sınırı
    mailGroup->setStyleSheet(R"(
        QGroupBox {
            font-size: 18px;
            font-weight: bold;
            border: 2px solid #bdc3c7;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
    )");
    
    QVBoxLayout *mailLayout = new QVBoxLayout(mailGroup);

    QPushButton *toggleBtn = new QPushButton("Otomatik Mail: Açık");
    toggleBtn->setCheckable(true);
    toggleBtn->setChecked(true);
    toggleBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #e74c3c;
            color: white;
            padding: 5px 10px;
            border-radius: 12px;
            font-size: 12px;
        }
        QPushButton:checked {
            background-color: #1abc9c;
        }
    )");

    QSettings appSettings("SecurityApp", "MailSettings");
    autoMailEnabled = appSettings.value("email/autoMailEnabled", true).toBool();

    // Toggle butonunun başlangıç durumunu ayarla
    toggleBtn->setChecked(autoMailEnabled);
    toggleBtn->setText(autoMailEnabled ? "Otomatik Mail: Açık" : "Otomatik Mail: Kapalı");

    connect(toggleBtn, &QPushButton::toggled, this, [=](bool checked) {
        autoMailEnabled = checked;  // Flag'i güncelle
        toggleBtn->setText(checked ? "Otomatik Mail: Açık" : "Otomatik Mail: Kapalı");
        
        // Flag'i QSettings'e kaydet
        QSettings toggleSettings("SecurityApp", "MailSettings");
        toggleSettings.setValue("email/autoMailEnabled", autoMailEnabled);
        
        // MailSender sınıfına da bu durumu bildir
        MailSender::getInstance().setAutoMailEnabled(autoMailEnabled);
        
        // Debug için konsola yazdır
        std::cout << "Otomatik mail gönderme: " << (autoMailEnabled ? "Açık" : "Kapalı") << std::endl;
    });

    QHBoxLayout *toggleLayout = new QHBoxLayout();
    toggleLayout->addWidget(toggleBtn);
    toggleLayout->addStretch();
    mailLayout->addLayout(toggleLayout);

    QHBoxLayout *mailRow = new QHBoxLayout();
    QLabel *mailLabel = new QLabel("Alıcı Mail Adresi:");
    mailLabel->setStyleSheet("font-size: 14px;");

    this->recipientMail = new QLineEdit();
    recipientMail->setPlaceholderText("ornek@firma.com");
    recipientMail->setFixedWidth(300);
    recipientMail->setStyleSheet("padding: 6px; border-radius: 4px; font-size: 13px;");

    mailRow->addWidget(mailLabel);
    mailRow->addWidget(recipientMail);
    mailRow->addStretch();
    mailLayout->addLayout(mailRow);
    
    // Mail grubunu ana layout'a ekle
    mainLayout->addWidget(mailGroup);

    // Kaydedilmiş e-posta adresini yükle
    QString savedEmail = appSettings.value("email/recipient", "").toString();
    recipientMail->setText(savedEmail);
    
    // Singleton üzerinden MailSender'a erişim
    MailSender::getInstance().recipientEmail = savedEmail.toStdString();
    
    // Kaydedilmiş e-posta adresini konsola yaz (debug için)
    std::cout << "Yüklenen e-posta adresi: [" << MailSender::getInstance().recipientEmail << "]" << std::endl;

    // KKD Ayarları Bölümü
    QGroupBox *kKDGroup = new QGroupBox("KKD Ayarları");
    kKDGroup->setMaximumWidth(800); // Maksimum genişlik sınırı
    kKDGroup->setStyleSheet(R"(
        QGroupBox {
            font-size: 18px;
            font-weight: bold;
            border: 2px solid #bdc3c7;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
    )");

    QVBoxLayout *kKDLayout = new QVBoxLayout(kKDGroup);

    QLabel *kKDDescription = new QLabel("Aşağıdaki KKD'ler için ihlal kontrolü yapılacaktır:");
    kKDDescription->setStyleSheet("font-size: 14px; color: #7f8c8d; margin-bottom: 10px;");
    kKDLayout->addWidget(kKDDescription);

    // KKD Checkbox'ları
    glovesCheckBox = new QCheckBox("🧤 Eldiven (Gloves)");
    helmetCheckBox = new QCheckBox("⛑️ Kask (Helmet)");
    maskCheckBox = new QCheckBox("😷 Maske (Mask)");
    vestCheckBox = new QCheckBox("🦺 Yelek (Vest)");
    idCardCheckBox = new QCheckBox("🆔 ID Kart (ID Card)");

    // Checkbox stilini ayarla - daha kompakt
    QString checkboxStyle = R"(
        QCheckBox {
            font-size: 14px;
            padding: 6px;
            spacing: 8px;
            min-width: 160px;
            max-width: 200px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
        }
        QCheckBox::indicator:unchecked {
            border: 2px solid #bdc3c7;
            border-radius: 4px;
            background-color: white;
        }
        QCheckBox::indicator:checked {
            border: 2px solid #27ae60;
            border-radius: 4px;
            background-color: #27ae60;
            image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTYiIGhlaWdodD0iMTYiIHZpZXdCb3g9IjAgMCAxNiAxNiIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTEzLjUgNEw2IDExLjVMMi41IDgiIHN0cm9rZT0id2hpdGUiIHN0cm9rZS13aWR0aD0iMiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIi8+Cjwvc3ZnPgo=);
        }
        QCheckBox:hover {
            background-color: #ecf0f1;
            border-radius: 4px;
        }
    )";

    glovesCheckBox->setStyleSheet(checkboxStyle);
    helmetCheckBox->setStyleSheet(checkboxStyle);
    maskCheckBox->setStyleSheet(checkboxStyle);
    vestCheckBox->setStyleSheet(checkboxStyle);
    idCardCheckBox->setStyleSheet(checkboxStyle);

    // KKD checkbox'larını grid layout'ta düzenle (2x3 grid)
    QGridLayout *kKDGridLayout = new QGridLayout();
    kKDGridLayout->setSpacing(15);
    kKDGridLayout->setContentsMargins(10, 10, 10, 10);
    
    // İlk satır
    kKDGridLayout->addWidget(glovesCheckBox, 0, 0);
    kKDGridLayout->addWidget(helmetCheckBox, 0, 1);
    kKDGridLayout->addWidget(maskCheckBox, 0, 2);
    
    // İkinci satır
    kKDGridLayout->addWidget(vestCheckBox, 1, 0);
    kKDGridLayout->addWidget(idCardCheckBox, 1, 1);
    
    // Grid'i ana layout'a ekle
    kKDLayout->addLayout(kKDGridLayout);

    // KKD ayarlarını yükle
    loadKKDSettings();

    // KKD checkbox'larının değişimini dinle
    connect(glovesCheckBox, &QCheckBox::toggled, this, &Settings::saveKKDSettings);
    connect(helmetCheckBox, &QCheckBox::toggled, this, &Settings::saveKKDSettings);
    connect(maskCheckBox, &QCheckBox::toggled, this, &Settings::saveKKDSettings);
    connect(vestCheckBox, &QCheckBox::toggled, this, &Settings::saveKKDSettings);
    connect(idCardCheckBox, &QCheckBox::toggled, this, &Settings::saveKKDSettings);

    mainLayout->addWidget(kKDGroup);
    
    // Kaydet butonu - KKD grubundan sonra
    QPushButton *saveBtn = new QPushButton("Ayarları Kaydet");
    saveBtn->setStyleSheet("padding: 12px 30px; background-color: #1abc9c; color: white; border-radius: 6px; font-size: 14px; font-weight: bold; margin-top: 20px;");
    saveBtn->setMaximumWidth(200);
    mainLayout->addWidget(saveBtn, 0, Qt::AlignCenter);

    // Kaydet butonuna basıldığında mail adresini al ve kaydet
    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        QString mailAddress = recipientMail->text();
        
        if (mailAddress.isEmpty()) {
            QMessageBox::warning(this, "Uyarı", "Lütfen geçerli bir e-posta adresi girin.");
            return;
        }
        
        // Tırnak işaretlerini kaldır (eğer varsa)
        mailAddress = mailAddress.remove('"');
        
        // Singleton üzerinden MailSender'a erişim
        MailSender::getInstance().recipientEmail = mailAddress.toStdString();
        
        // E-posta adresini kalıcı olarak kaydet
        QSettings saveSettings("SecurityApp", "MailSettings");
        saveSettings.setValue("email/recipient", mailAddress);
        saveSettings.sync(); // Ayarların hemen kaydedilmesini sağla
        
        // KKD ayarlarını da kaydet
        saveKKDSettings();
        
        // Kontrol için konsola yazdır
        std::cout << "Kaydedilen e-posta: [" << MailSender::getInstance().recipientEmail << "]" << std::endl;
        
        // Başarılı mesajı göster
        QMessageBox::information(this, "Başarılı", "Ayarlar başarıyla kaydedildi.");
    });
    
    // Uygulama başlatıldığında MailSender'a başlangıç durumunu bildir
    MailSender::getInstance().setAutoMailEnabled(autoMailEnabled);
}

// KKD ayarlarını kaydetme fonksiyonu
void Settings::saveKKDSettings() {
    QSettings settings("SecurityApp", "KKDSettings");
    settings.setValue("kkd/gloves", glovesCheckBox->isChecked());
    settings.setValue("kkd/helmet", helmetCheckBox->isChecked());
    settings.setValue("kkd/mask", maskCheckBox->isChecked());
    settings.setValue("kkd/vest", vestCheckBox->isChecked());
    settings.setValue("kkd/id_card", idCardCheckBox->isChecked());
    settings.sync();
    
    // Debug için konsola yazdır
    std::cout << "KKD Ayarları kaydedildi:" << std::endl;
    std::cout << "  - Eldiven: " << (glovesCheckBox->isChecked() ? "Açık" : "Kapalı") << std::endl;
    std::cout << "  - Kask: " << (helmetCheckBox->isChecked() ? "Açık" : "Kapalı") << std::endl;
    std::cout << "  - Maske: " << (maskCheckBox->isChecked() ? "Açık" : "Kapalı") << std::endl;
    std::cout << "  - Yelek: " << (vestCheckBox->isChecked() ? "Açık" : "Kapalı") << std::endl;
    std::cout << "  - ID Kart: " << (idCardCheckBox->isChecked() ? "Açık" : "Kapalı") << std::endl;
}

// KKD ayarlarını yükleme fonksiyonu
void Settings::loadKKDSettings() {
    QSettings settings("SecurityApp", "KKDSettings");
    
    // Varsayılan olarak tüm KKD'ler açık
    glovesCheckBox->setChecked(settings.value("kkd/gloves", true).toBool());
    helmetCheckBox->setChecked(settings.value("kkd/helmet", true).toBool());
    maskCheckBox->setChecked(settings.value("kkd/mask", true).toBool());
    vestCheckBox->setChecked(settings.value("kkd/vest", true).toBool());
    idCardCheckBox->setChecked(settings.value("kkd/id_card", true).toBool());
}

// Static fonksiyon - KKD ayarlarını almak için
std::map<std::string, bool> Settings::getKKDSettings() {
    QSettings settings("SecurityApp", "KKDSettings");
    std::map<std::string, bool> kKDSettings;
    
    kKDSettings["gloves"] = settings.value("kkd/gloves", true).toBool();
    kKDSettings["helmet"] = settings.value("kkd/helmet", true).toBool();
    kKDSettings["mask"] = settings.value("kkd/mask", true).toBool();
    kKDSettings["vest"] = settings.value("kkd/vest", true).toBool();
    kKDSettings["id_card"] = settings.value("kkd/id_card", true).toBool();
    
    return kKDSettings;
}
