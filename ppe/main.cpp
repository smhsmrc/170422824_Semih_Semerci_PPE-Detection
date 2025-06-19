#include <QApplication>
#include <QLoggingCategory>
#include <QDebug>
#include "./ui/pages/include/loginPage.h"

// Özel mesaj filtresi - CSS uyarılarını gizler
void messageFilter(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    // CSS "Unknown property" uyarılarını filtrele
    if (msg.contains("Unknown property")) {
        return; // Bu mesajı gösterme
    }
    
    // Diğer mesajları normal şekilde göster
    QString formatted = QString("[%1] %2").arg(
        type == QtDebugMsg ? "DEBUG" :
        type == QtInfoMsg ? "INFO" :
        type == QtWarningMsg ? "WARNING" :
        type == QtCriticalMsg ? "CRITICAL" :
        type == QtFatalMsg ? "FATAL" : "UNKNOWN"
    ).arg(msg);
    
    // Sadece önemli mesajları terminale yazdır
    if (type != QtDebugMsg) {
        fprintf(stderr, "%s\n", formatted.toLocal8Bit().constData());
    }
}


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Özel mesaj filtresini etkinleştir (CSS uyarılarını gizler)
    qInstallMessageHandler(messageFilter);
    
    // CSS uyarılarını gizle (Qt CSS desteklenmeyen özellikler için)
    QLoggingCategory::setFilterRules("qt.qpa.stylesheet.debug=false");
    QLoggingCategory::setFilterRules("*.debug=false");
    
    // CSS uyarılarını tamamen gizlemek için:
    qputenv("QT_LOGGING_RULES", "qt.qpa.stylesheet.debug=false");
    LoginPage window;
    window.setWindowTitle("Kişisel Koruyucu Donanım Tespiti");
    window.showFullScreen();
    return app.exec();
}


