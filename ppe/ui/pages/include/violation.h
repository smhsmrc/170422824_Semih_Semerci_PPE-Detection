#ifndef VIOLATION_H
#define VIOLATION_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QMouseEvent>
#include <QPixmap>
#include "../../db/include/db.h"

// ✅ Forward declaration
class ImageButton;

class ViolationPage : public QWidget {
    Q_OBJECT  // ✅ Bu satır eksikti!

public:
    explicit ViolationPage(QWidget *parent = nullptr);
    void loadViolations();

// protected:
//     void mousePressEvent(QMouseEvent *event) override;

private slots:  // ✅ Bu kısım da eklenebilir (opsiyonel)
    void deleteSelectedViolation();
    void deleteAllViolations();

private:
    QTableWidget *table;
    QPushButton *deleteSelectedBtn;
    QPushButton *deleteAllBtn;
    QPushButton *refreshBtn;
};

// ✅ ImageButton sınıfını header'da tanımla
class ImageButton : public QPushButton {
    Q_OBJECT  // ✅ Bu da eklenebilir (opsiyonel, ama önerilir)

public:
    ImageButton(const QPixmap &pixmap, const DB::Tensor &violation, QWidget *parent);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    void showImagePopup();
    QPixmap m_pixmap;
    DB::Tensor m_violation;
};

#endif
