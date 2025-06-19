#include "../include/violation.h"
#include "../../db/include/db.h"  
#include "../include/settings.h"
#include <QDialog>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QMessageBox>
#include <QMouseEvent>

ViolationPage::ViolationPage(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("Tespit Edilen İhlaller");
    title->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;");
    layout->addWidget(title);
    
    // Buton grubu oluştur
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    // Seçili satırı sil butonu
    deleteSelectedBtn = new QPushButton("🗑️ Seçili Kaydı Sil");
    deleteSelectedBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #e74c3c;
            color: white;
            font-weight: bold;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #c0392b;
        }
        QPushButton:pressed {
            background-color: #a93226;
        }
    )");
    
    // Tümünü sil butonu
    deleteAllBtn = new QPushButton("🗑️ Tüm Kayıtları Sil");
    deleteAllBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #8e44ad;
            color: white;
            font-weight: bold;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #7d3c98;
        }
        QPushButton:pressed {
            background-color: #6c3483;
        }
    )");
    
    // Yenile butonu
    refreshBtn = new QPushButton("🔄 Yenile");
    refreshBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #3498db;
            color: white;
            font-weight: bold;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #2980b9;
        }
        QPushButton:pressed {
            background-color: #1f618d;
        }
    )");
    
    // ✅ Signal-Slot bağlantıları
    connect(deleteSelectedBtn, &QPushButton::clicked, this, &ViolationPage::deleteSelectedViolation);
    connect(deleteAllBtn, &QPushButton::clicked, this, &ViolationPage::deleteAllViolations);
    connect(refreshBtn, &QPushButton::clicked, this, &ViolationPage::loadViolations);
    
    buttonLayout->addWidget(deleteSelectedBtn);
    buttonLayout->addWidget(deleteAllBtn);
    buttonLayout->addWidget(refreshBtn);
    buttonLayout->addStretch();
    
    layout->addLayout(buttonLayout);
    
    table = new QTableWidget(this);    
    table->setStyleSheet(R"(
        QTableWidget {
            background-color: #f7f9fb;
            border: 1px solid #dfe6ec;
            font-size: 14px;
            color: #2c3e50;
            gridline-color: #ecf0f1;
            selection-background-color: #d0e6f6;
            selection-color: #2c3e50;
            alternate-background-color: #eef2f7;
        }
    
        QHeaderView::section {
            background-color: #3498db;
            color: white;
            font-weight: bold;
            border: none;
            padding: 8px;
        }
    
        QTableWidget::item {
            padding: 6px;
            border: none;
        }
    
        QTableWidget::item:selected {
            background-color: #aed6f1;
            color: #2c3e50;
        }
    )");
    
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels(QStringList() 
        << "Tarih" 
        << "İsim" 
        << "Soyisim" 
        << "Kart ID"
        << "Departman" 
        << "İhlal Sebebi" 
        << "Resim");

    QHeaderView *header = table->horizontalHeader();
    
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    table->setColumnWidth(0, 140);
    
    header->setSectionResizeMode(1, QHeaderView::Fixed);
    table->setColumnWidth(1, 90);
    
    header->setSectionResizeMode(2, QHeaderView::Fixed);
    table->setColumnWidth(2, 90);
    
    header->setSectionResizeMode(3, QHeaderView::Fixed);
    table->setColumnWidth(3, 80);
    
    header->setSectionResizeMode(6, QHeaderView::Fixed);
    table->setColumnWidth(6, 80);
    
    header->setSectionResizeMode(4, QHeaderView::Stretch);
    header->setSectionResizeMode(5, QHeaderView::Stretch);
    
    table->verticalHeader()->setDefaultSectionSize(72);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);

    layout->addWidget(table);
    
    // İlk yüklemede tabloyu doldur
    loadViolations();
}

void ViolationPage::loadViolations() {
    // Yenileme feedback'i
    refreshBtn->setText("🔄 Yenileniyor...");
    refreshBtn->setEnabled(false);
    
    table->setRowCount(0);

    DB db;
    std::vector<DB::Tensor> violations = db.getAllViolationsFromDB();
    
    int row = 0;
    for (const auto &violation : violations) {
        table->insertRow(row);
        
        // Worker durumunu kontrol et
        bool isUnknownWorker = (violation.firstName == "Bilinmeyen" && violation.lastName == "Çalışan" && violation.department == "Bilinmeyen Departman");
        bool hasValidCardId = (violation.card_id > 0);
    
        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(violation.timestamp)));
        
        // İsim ve soyisim için özel işaretleme
        QTableWidgetItem *firstNameItem = new QTableWidgetItem();
        QTableWidgetItem *lastNameItem = new QTableWidgetItem();
        
        if (isUnknownWorker && hasValidCardId) {
            firstNameItem->setText("⚠️ BİLİNMEYEN");
            lastNameItem->setText("ÇALIŞAN");
            firstNameItem->setBackground(QColor("#fff3cd"));
            lastNameItem->setBackground(QColor("#fff3cd"));
            firstNameItem->setForeground(QColor("#856404"));
            lastNameItem->setForeground(QColor("#856404"));
        } else if (!hasValidCardId) {
            firstNameItem->setText("❓ KİMLİK");
            lastNameItem->setText("OKUNAMADI");
            firstNameItem->setBackground(QColor("#fff3cd"));
            lastNameItem->setBackground(QColor("#fff3cd"));
            firstNameItem->setForeground(QColor("#856404"));
            lastNameItem->setForeground(QColor("#856404"));
        } else {
            firstNameItem->setText(QString::fromStdString(violation.firstName));
            lastNameItem->setText(QString::fromStdString(violation.lastName));
        }
        
        table->setItem(row, 1, firstNameItem);
        table->setItem(row, 2, lastNameItem);
        
        // Kart ID için özel işaretleme
        QTableWidgetItem *cardIdItem = new QTableWidgetItem();
        if (violation.card_id == -1) {
            cardIdItem->setText("❌ Okunamadı");
            cardIdItem->setBackground(QColor("#f8d7da"));
            cardIdItem->setForeground(QColor("#721c24"));
        } else if (isUnknownWorker) {
            cardIdItem->setText("⚠️ " + QString::number(violation.card_id));
            cardIdItem->setBackground(QColor("#fff3cd"));
            cardIdItem->setForeground(QColor("#856404"));
        } else {
            cardIdItem->setText(QString::number(violation.card_id));
        }
        table->setItem(row, 3, cardIdItem);
        
        // Departman için özel işaretleme
        QTableWidgetItem *departmentItem = new QTableWidgetItem();
        if (isUnknownWorker || !hasValidCardId) {
            departmentItem->setText("❓ Bilinmiyor");
            departmentItem->setBackground(QColor("#fff3cd"));
            departmentItem->setForeground(QColor("#856404"));
        } else {
            departmentItem->setText(QString::fromStdString(violation.department));
        }
        table->setItem(row, 4, departmentItem);
        
        // ✅ GÜNCELLENMIŞ: Aktif KKD ayarlarına göre ihlal sebebi oluştur
        // KKD ayarlarını al
        std::map<std::string, bool> kKDSettings = Settings::getKKDSettings();
        
        QStringList violationReasons;
        
        // Sadece aktif KKD kontrollerine göre ihlal sebeplerini ekle
        if (kKDSettings["helmet"] && !violation.helmet) {
            violationReasons << "❌ Kask takılmamış";
        }
        if (kKDSettings["vest"] && !violation.vest) {
            violationReasons << "❌ Yelek giyilmemiş";
        }
        if (kKDSettings["gloves"] && !violation.gloves) {
            violationReasons << "❌ Eldiven takılmamış";
        }
        if (kKDSettings["mask"] && !violation.mask) {
            violationReasons << "❌ Maske takılmamış";
        }
        
        // ID Kart kontrolü - sadece ayarlarda aktifse kontrol et
        if (kKDSettings["id_card"] && !violation.cart_status) {
            violationReasons << "❌ ID Kart tespit edilmemiş";
        }
        
        QString violationDescription;
        QTableWidgetItem *violationItem = new QTableWidgetItem();
        
        if (isUnknownWorker && hasValidCardId) {
            violationDescription = "⚠️ BİLİNMEYEN ÇALIŞAN + " + violationReasons.join(", ");
            violationItem->setBackground(QColor("#fff3cd"));
            violationItem->setForeground(QColor("#856404"));
        } else if (!hasValidCardId) {
            violationDescription = "❓ KİMLİK OKUNAMADI + " + violationReasons.join(", ");
            violationItem->setBackground(QColor("#fff3cd"));
            violationItem->setForeground(QColor("#856404"));
        } else if (violationReasons.isEmpty()) {
            violationDescription = "İhlal tespit edilmedi";
        } else {
            violationDescription = violationReasons.join(", ");
        }
        
        violationItem->setText(violationDescription);
        table->setItem(row, 5, violationItem);

        // Görsel
        cv::Mat image = cv::imdecode(violation.imageData, cv::IMREAD_COLOR);
        if (!image.empty()) {
            QImage qimg(image.data, image.cols, image.rows, image.step, QImage::Format_BGR888);
            QPixmap pixmap = QPixmap::fromImage(qimg);
            QPixmap scaled = pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
            // ✅ Özel resim butonu kullan
            ImageButton *imageButton = new ImageButton(pixmap, violation, this);
            imageButton->setIcon(QIcon(scaled));
            imageButton->setIconSize(QSize(64, 64));
            imageButton->setFixedSize(70, 70);
            imageButton->setStyleSheet("border: none; padding: 2px;");
    
            table->setCellWidget(row, 6, imageButton);
        } else {
            table->setItem(row, 6, new QTableWidgetItem("Görsel yok"));
        }
    
        ++row;
    }
    
    // Yenileme tamamlandı - buton durumunu eski haline getir
    refreshBtn->setText("🔄 Yenile");
    refreshBtn->setEnabled(true);
}

// ✅ GÜNCELLENMIŞ: Gerçek veritabanı silme işlemi
void ViolationPage::deleteSelectedViolation() {
    int currentRow = table->currentRow();
    
    if (currentRow < 0) {
        QMessageBox::warning(this, "Uyarı", "Lütfen silmek istediğiniz kaydı seçin!");
        return;
    }
    
    // Seçili satırdan bilgileri al
    QString timestamp = table->item(currentRow, 0)->text();
    QString workerName = table->item(currentRow, 1)->text() + " " + table->item(currentRow, 2)->text();
    
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Silme Onayı", 
                                  QString("Seçili kaydı silmek istediğinizden emin misiniz?\n\n"
                                         "Çalışan: %1\n"
                                         "Tarih: %2")
                                  .arg(workerName)
                                  .arg(timestamp),
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // ✅ Gerçek veritabanı silme işlemi
        DB db;
        if (db.deleteViolationByTimestamp(timestamp.toStdString())) {
            table->removeRow(currentRow);
            QMessageBox::information(this, "Başarılı", 
                                   QString("Kayıt başarıyla silindi!\n\nÇalışan: %1\nTarih: %2")
                                   .arg(workerName)
                                   .arg(timestamp));
        } else {
            QMessageBox::critical(this, "Hata", 
                                "Kayıt silinirken hata oluştu!\n\n"
                                "Lütfen tekrar deneyin veya sistem yöneticisine başvurun.");
        }
    }
}

// ✅ GÜNCELLENMIŞ: Gerçek veritabanı silme işlemi
void ViolationPage::deleteAllViolations() {
    if (table->rowCount() == 0) {
        QMessageBox::information(this, "Bilgi", "Silinecek kayıt bulunmamaktadır!");
        return;
    }
    
    int recordCount = table->rowCount();
    
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "⚠️ Kritik Silme Onayı", 
                                  QString("Tüm ihlal kayıtlarını silmek istediğinizden emin misiniz?\n\n"
                                         "📊 Silinecek kayıt sayısı: %1 adet\n"
                                         "⚠️ Bu işlem geri alınamaz!\n"
                                         "💾 Yedek almayı unutmayın!")
                                  .arg(recordCount),
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // ✅ İkinci onay - Daha güvenli
        QMessageBox::StandardButton finalReply;
        finalReply = QMessageBox::question(this, "🔴 Son Onay", 
                                          QString("Son kez soruyoruz!\n\n"
                                                 "%1 adet kayıt kalıcı olarak silinecek.\n\n"
                                                 "Devam etmek istediğinizden emin misiniz?")
                                          .arg(recordCount),
                                          QMessageBox::Yes | QMessageBox::Cancel);
        
        if (finalReply == QMessageBox::Yes) {
            // ✅ Gerçek veritabanı silme işlemi
            DB db;
            if (db.deleteAllViolations()) {
                table->setRowCount(0);
                QMessageBox::information(this, "✅ Başarılı", 
                                       QString("Tüm kayıtlar başarıyla silindi!\n\n"
                                              "📊 Silinen kayıt sayısı: %1 adet\n"
                                              "🗃️ Veritabanı temizlendi")
                                       .arg(recordCount));
            } else {
                QMessageBox::critical(this, "❌ Hata", 
                                    "Kayıtlar silinirken hata oluştu!\n\n"
                                    "Olası nedenler:\n"
                                    "• Veritabanı bağlantı hatası\n"
                                    "• Dosya izin problemi\n"
                                    "• Disk alanı yetersizliği\n\n"
                                    "Lütfen sistem yöneticisine başvurun.");
            }
        }
    }
}

// ✅ ImageButton sınıfı implementasyonu
ImageButton::ImageButton(const QPixmap &pixmap, const DB::Tensor &violation, QWidget *parent)
    : QPushButton(parent), m_pixmap(pixmap), m_violation(violation) {
}

void ImageButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        showImagePopup();
    }
    QPushButton::mousePressEvent(event);
}

void ImageButton::showImagePopup() {
    QDialog *popup = new QDialog(this, Qt::FramelessWindowHint);
    popup->setWindowTitle("İhlal Görseli");
    popup->setFixedSize(1000, 900);  // ✅ Daha da büyük popup boyutu

    popup->setStyleSheet(R"(
        QDialog {
            background-color: #1e272e;
            border: 2px solid #1abc9c;
            border-radius: 12px;
        }
        QLabel {
            background-color: #ffffff;
            border: 1px solid #ccc;
            padding: 10px;
            border-radius: 6px;
            color: #2c3e50;
        }
        QPushButton {
            background-color: #1abc9c;
            color: white;
            font-weight: bold;
            border: none;
            border-radius: 6px;
            padding: 8px 12px;
        }
        QPushButton:hover {
            background-color: #16a085;
        }
    )");

    QLabel *workerInfo = new QLabel();
    
    // Worker durumunu kontrol et
    bool isUnknownWorker = (m_violation.firstName == "Bilinmeyen" && m_violation.lastName == "Çalışan" && m_violation.department == "Bilinmeyen Departman");
    bool hasValidCardId = (m_violation.card_id > 0);
    
    QString cardIdText = (m_violation.card_id == -1) ? "Okunamadı" : QString::number(m_violation.card_id);
    
    QString infoText;
    QString workerInfoStyle;
    
    if (isUnknownWorker && hasValidCardId) {
        // Bilinmeyen çalışan durumu
        infoText = QString(
            "<b>⚠️ BİLİNMEYEN ÇALIŞAN İHLALİ</b><br><br>"
            "<span style='color: #e67e22; font-weight: bold;'>DİKKAT: Bu kart ID'si çalışan listesinde kayıtlı değil!</span><br><br>"
            "<b>Tespit Edilen Bilgiler:</b><br>"
            "Kart ID: %1<br>"
            "Tarih: %2<br><br>"
            "<b>Önerilen Eylemler:</b><br>"
            "• Çalışanın kimlik bilgilerini kontrol edin<br>"
            "• Yetkisiz giriş olup olmadığını araştırın<br>"
            "• Sisteme kayıt yapılması gerekip gerekmediğini değerlendirin"
        ).arg(cardIdText)
         .arg(QString::fromStdString(m_violation.timestamp));
        
        workerInfoStyle = "font-size: 14px; margin-bottom: 10px; background-color: #fff3cd; border-left: 4px solid #ffc107; padding: 15px;";
        
    } else if (!hasValidCardId) {
        // Kart ID okunamayan durum
        infoText = QString(
            "<b>⚠️ KİMLİK KARTI OKUNAMADI</b><br><br>"
            "<span style='color: #e67e22; font-weight: bold;'>DİKKAT: Çalışanın kimlik kartı tespit edilemedi!</span><br><br>"
            "<b>Durum:</b><br>"
            "Kart ID: Okunamadı<br>"
            "Tarih: %1<br><br>"
            "<b>Kontrol Edilmesi Gerekenler:</b><br>"
            "• Çalışanın kimlik kartının durumu<br>"
            "• Kart okuyucu sisteminin çalışması<br>"
            "• Yeni kart verilmesi gerekip gerekmediği"
        ).arg(QString::fromStdString(m_violation.timestamp));
        
        workerInfoStyle = "font-size: 14px; margin-bottom: 10px; background-color: #fff3cd; border-left: 4px solid #ffc107; padding: 15px;";
        
    } else {
        // Normal çalışan bilgileri
        infoText = QString(
            "<b>Çalışan Bilgileri:</b><br>"
            "İsim: %1 %2<br>"
            "Departman: %3<br>"
            "Kart ID: %4<br>"
            "Tarih: %5"
        ).arg(QString::fromStdString(m_violation.firstName))
         .arg(QString::fromStdString(m_violation.lastName))
         .arg(QString::fromStdString(m_violation.department))
         .arg(cardIdText)
         .arg(QString::fromStdString(m_violation.timestamp));
        
        workerInfoStyle = "font-size: 14px; margin-bottom: 10px;";
    }
    
    workerInfo->setText(infoText);
    workerInfo->setWordWrap(true);
    workerInfo->setStyleSheet(workerInfoStyle);

    // ✅ YENİ EKLENEN: İhlal detayları
    QLabel *violationDetails = new QLabel();
    
    QString helmetStatus = m_violation.helmet ? "✅ Kask takılı" : "❌ Kask takılmamış";
    QString vestStatus = m_violation.vest ? "✅ Yelek giyili" : "❌ Yelek giyilmemiş";
    QString glovesStatus = m_violation.gloves ? "✅ Eldiven takılı" : "❌ Eldiven takılmamış";
    QString maskStatus = m_violation.mask ? "✅ Maske takılı" : "❌ Maske takılmamış";
    QString cartStatus = m_violation.cart_status ? "✅ ID Kart takılı" : "❌ ID Kart takılmamış";  // ✅ YENİ
    
    QString violationText = QString(
        "<b>Güvenlik Ekipmanları Durumu:</b><br>"
        "%1<br>"
        "%2<br>"
        "%3<br>"
        "%4<br>"
        "%5"  // ✅ YENİ SATIR
    ).arg(helmetStatus)
     .arg(vestStatus)
     .arg(glovesStatus)
     .arg(maskStatus)
     .arg(cartStatus);  // ✅ YENİ PARAMETRE
    
    violationDetails->setText(violationText);
    violationDetails->setWordWrap(true);
    violationDetails->setStyleSheet("font-size: 13px; margin-bottom: 10px;");

    QLabel *imageLabel = new QLabel();
    imageLabel->setPixmap(m_pixmap.scaled(900, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation));  // ✅ Daha da büyük resim boyutu
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("border: 2px solid #1abc9c; border-radius: 8px; background-color: white; padding: 5px;");  // ✅ Resim çerçevesi

    // ✅ Özel kapat butonu
    class CloseButton : public QPushButton {
    public:
        CloseButton(QDialog *dialog) : QPushButton("Kapat"), m_dialog(dialog) {}
    protected:
        void mousePressEvent(QMouseEvent *event) override {
            if (event->button() == Qt::LeftButton) {
                m_dialog->accept();
            }
            QPushButton::mousePressEvent(event);
        }
    private:
        QDialog *m_dialog;
    };
    
    CloseButton *closeBtn = new CloseButton(popup);

    QVBoxLayout *layout = new QVBoxLayout(popup);
    layout->addWidget(workerInfo);
    layout->addWidget(violationDetails);  // ✅ YENİ EKLENEN
    layout->addWidget(imageLabel, 1, Qt::AlignCenter);  // ✅ Stretch factor 1 - resim daha fazla yer kaplasın
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);
    layout->setContentsMargins(15, 15, 15, 15);  // ✅ Daha az margin
    layout->setSpacing(10);  // ✅ Daha az spacing

    popup->exec();
}
