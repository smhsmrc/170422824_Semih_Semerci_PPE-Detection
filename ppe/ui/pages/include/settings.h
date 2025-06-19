#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <map>
#include <string>

class Settings : public QWidget {

public:
    explicit Settings(QWidget *parent = nullptr);
    
    // KKD ayarlarını almak için static fonksiyon
    static std::map<std::string, bool> getKKDSettings();

private:
    QLineEdit *recipientMail;
    bool autoMailEnabled; // Otomatik mail gönderme flag'i
    
    // KKD checkbox'ları
    QCheckBox *glovesCheckBox;
    QCheckBox *helmetCheckBox;
    QCheckBox *maskCheckBox;
    QCheckBox *vestCheckBox;
    QCheckBox *idCardCheckBox;
    
    // KKD ayarlarını kaydetme fonksiyonu
    void saveKKDSettings();
    void loadKKDSettings();
};

#endif // SETTINGS_H
