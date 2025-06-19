#include "../include/workerList.h"
#include "../../db/include/db.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>  
#include <QWidget>     
#include <QDialog>
#include <QFormLayout>
#include <QMessageBox>




WorkerListPage::WorkerListPage(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    DB db;
    auto workers = db.getAllWorkersFromDB();

    // Worker'ı yazdırmak için bir fonksiyon
    for (const auto& worker : workers) {
        std::cout << "Ad: " << worker.firstName 
                << ", Soyad: " << worker.lastName 
                << ", Çalışan No: " << worker.workerNumber
                << ", Departman: " << worker.department << std::endl;
    }


    QLabel *title = new QLabel("Çalışan Listesi");
    title->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;");
    mainLayout->addWidget(title);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    addWorkerBtn = new QPushButton("Çalışan Ekle");
    removeWorkerBtn = new QPushButton("Çalışan Sil");

    // Butonların stilini ayarlıyoruz
    QString buttonStyle = R"(
        QPushButton {
            background-color: #2ecc71;
            color: white;
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: bold;
            font-size: 14px;
            border: none;
        }
        QPushButton:hover {
            background-color: #27ae60;
        }
        QPushButton:pressed {
            background-color: #1e8449;
        }
    )";

    addWorkerBtn->setStyleSheet(buttonStyle);
    removeWorkerBtn->setStyleSheet(buttonStyle);

    buttonLayout->addWidget(addWorkerBtn);
    buttonLayout->addWidget(removeWorkerBtn);
    buttonLayout->addStretch();

    QObject::connect(addWorkerBtn, &QPushButton::clicked, this, [this,&db]() {
        QDialog *popup = new QDialog(this,Qt::FramelessWindowHint);
        popup->setWindowTitle("Yeni Çalışan Ekle");
        popup->setFixedSize(400, 300);
        
    
        popup->setStyleSheet(R"(
            QDialog {
                background-color: #1e272e;
                border: 2px solid #1abc9c;
                border-radius: 12px;
            }
            QLabel {
                color: white;
            }
            QLineEdit {
                background-color: #ffffff;
                border: 1px solid #ccc;
                padding: 6px;
                border-radius: 6px;
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
    
        // Form alanları
        // Form alanları
        QLineEdit *firstNameEdit = new QLineEdit(popup);
        firstNameEdit->setStyleSheet("color: black;");

        QLineEdit *lastNameEdit = new QLineEdit(popup);
        lastNameEdit->setStyleSheet("color: black;");

        QLineEdit *workerNoEdit = new QLineEdit(popup);
        workerNoEdit->setValidator(new QIntValidator(1, 999999, popup));
        workerNoEdit->setStyleSheet("color: black;");

        QLineEdit *departmentEdit = new QLineEdit(popup);
        departmentEdit->setStyleSheet("color: black;");

        QFormLayout *formLayout = new QFormLayout();
        formLayout->addRow("Ad:", firstNameEdit);
        formLayout->addRow("Soyad:", lastNameEdit);
        formLayout->addRow("Çalışan No:", workerNoEdit);
        formLayout->addRow("Departman:", departmentEdit);

    
        // Butonlar
        QPushButton *okBtn = new QPushButton("Ekle", popup);
        QPushButton *cancelBtn = new QPushButton("İptal", popup);
    
        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->addStretch();
        btnLayout->addWidget(okBtn);
        btnLayout->addWidget(cancelBtn);
    
        QVBoxLayout *mainLayout = new QVBoxLayout(popup);
        mainLayout->addLayout(formLayout);
        mainLayout->addLayout(btnLayout);
        mainLayout->setContentsMargins(20, 20, 20, 20);
        mainLayout->setSpacing(15);
    
        // Bağlantılar
        QObject::connect(cancelBtn, &QPushButton::clicked, popup, &QDialog::reject);
    
        QObject::connect(okBtn, &QPushButton::clicked, [this, &db, popup, firstNameEdit, lastNameEdit, workerNoEdit, departmentEdit]() {
            // Basit validasyon (örnek)
            if (firstNameEdit->text().isEmpty() || lastNameEdit->text().isEmpty() || workerNoEdit->text().isEmpty() || departmentEdit->text().isEmpty()) {
                // Mesela QMessageBox ile uyarı verebilirsin, buraya ekleyebilirsin
                return;
            }
    
            // Tabloya ekleme
            int newRow = table->rowCount();
            table->insertRow(newRow);
            table->setItem(newRow, 0, new QTableWidgetItem(firstNameEdit->text()));
            table->setItem(newRow, 1, new QTableWidgetItem(lastNameEdit->text()));
            table->setItem(newRow, 2, new QTableWidgetItem(workerNoEdit->text()));
            table->setItem(newRow, 3, new QTableWidgetItem(departmentEdit->text()));
    
            // Veritabanına da ekleyebilirsin, örneğin:
            
            db.insertWorker(
                firstNameEdit->text().toStdString(),
                lastNameEdit->text().toStdString(),
                workerNoEdit->text().toInt(),
                departmentEdit->text().toStdString()
            );

            loadWorkers();
            popup->accept();
        });
    
        popup->exec();
    });


    mainLayout->addLayout(buttonLayout);

    table = new QTableWidget(this);
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels(QStringList() << "Ad" << "Soyad" << "Çalışan No" << "Pozisyon");

    // Tablo başlığı stili
    table->horizontalHeader()->setStyleSheet(R"(
        QHeaderView::section {
            background-color: #2ecc71;
            color: white;
            font-weight: bold;
            border: none;
            padding: 8px;
        }
    )");
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);

    // Tablo genel stili
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
        QTableWidget::item {
            padding: 6px;
            border: none;
        }
        QTableWidget::item:selected {
            background-color: #b9f6ca;
            color: #2c3e50;
        }
    )");

    mainLayout->addWidget(table);

    loadWorkers();

    connect(removeWorkerBtn, &QPushButton::clicked, [this,&db]() {
        auto selectedRows = table->selectionModel()->selectedRows();
        if (!selectedRows.isEmpty()) {
            int row = selectedRows.first().row();
    
            // Onay penceresi aç
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Onay", "Silmek istediğinize emin misiniz?",
                                          QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                // Çalışan numarasını al (3. sütun, indeks 2)
                QTableWidgetItem* item = table->item(row, 2);
                if (item) {
                    int workerNumber = item->text().toInt();
                    qDebug() << "Silinecek çalışan numarası:" << workerNumber;
    
                    // Veritabanından sil
                    if (db.removeWorkerByNumber(workerNumber)) {
                        table->removeRow(row);
                        qDebug() << "Çalışan başarıyla silindi";
                    } else {
                        qDebug() << "Çalışan veritabanından silinemedi";
                        QMessageBox::warning(this, "Hata", "Çalışan veritabanından silinemedi!");
                    }
                }
            } else {
                qDebug() << "Silme işlemi iptal edildi";
            }
        }
    });
}

void WorkerListPage::loadWorkers() {
    table->setRowCount(0);  // Önce tabloyu temizle

    DB db;
    auto workers = db.getAllWorkersFromDB();  // workers'ı burada tanımla

    int row = 0;
    for (const auto &worker : workers) {
        table->insertRow(row);  // Yeni bir satır ekle

        // Her bir hücreye ilgili veriyi ekle
        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(worker.firstName)));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(worker.lastName)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(worker.workerNumber)));
        table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(worker.department)));

        ++row;  // Satırı artır
    }
}
