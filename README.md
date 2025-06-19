# 🛡️ Kişisel Koruyucu Donanım (KKD) Tespit Sistemi

**Modern C++ tabanlı, Qt6 arayüzlü, gerçek zamanlı KKD tespit ve takip sistemi**

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6.7.2-green.svg)](https://www.qt.io/)
[![OpenCV](https://img.shields.io/badge/OpenCV-4.5.3-orange.svg)](https://opencv.org/)
[![PyTorch](https://img.shields.io/badge/PyTorch-1.13.1-red.svg)](https://pytorch.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## 🎓 Akademik Proje

Bu proje, **Marmara Üniversitesi - Teknoloji Fakültesi - Bilgisayar Mühendisliği** bölümü öğrencileri tarafından **Bitirme Projesi** olarak geliştirilmiştir.

### 👥 Geliştirici Ekibi
- **Erol Atik**
  - E-posta: [erolatik@marun.edu.tr]
  - GitHub: [github.com/erollatik]
  - LinkedIn: [linkedin.com/in/erolatik]

- **Semih Semerci**
  - E-posta: [semihsemerci@marun.edu.tr]
  - GitHub: [github.com/smhsmrc]
  - LinkedIn: [linkedin.com/in/semih-semerci]

- **Özge Çetinbaş**
  - E-posta: [kerimecetinbas@marun.edu.tr]
  - GitHub: [github.com/ozgecetinbas]
  - LinkedIn: [linkedin.com/in/ozgecetinbas]

### 🏫 Akademik Danışman
- **Marmara Üniversitesi Teknoloji Fakültesi**
- **Bilgisayar Mühendisliği Bölümü**
- **Prof. Dr. Serhat Özekes** - Proje Danışmanı
  - E-posta: [serhat.ozekes@marmara.edu.tr]
  - Bölüm: Bilgisayar Mühendisliği
  - Fakülte: Teknoloji Fakültesi

### 📅 Proje Dönemi
- **Dönem**: 2025 Bahar Dönemi
- **Proje Türü**: Bitirme Tezi

---

## 📋 İçindekiler

- [🎯 Proje Hakkında](#-proje-hakkında)
- [✨ Özellikler](#-özellikler)
- [🏗️ Mimari](#️-mimari)
- [🚀 Kurulum](#-kurulum)
- [💻 Kullanım](#-kullanım)
- [🔧 Konfigürasyon](#-konfigürasyon)
- [📊 API Dokümantasyonu](#-api-dokümantasyonu)
- [🛠️ Geliştirme](#️-geliştirme)
- [📈 Performans](#-performans)
- [🤝 Katkıda Bulunma](#-katkıda-bulunma)
- [📄 Lisans](#-lisans)

---

## 🎯 Proje Hakkında

Bu proje, iş güvenliği standartlarına uygunluğu sağlamak amacıyla geliştirilmiş, **gerçek zamanlı Kişisel Koruyucu Donanım (KKD) tespit sistemi**dir. Sistem, çalışanların baret, yelek, eldiven ve maske kullanımını otomatik olarak takip eder ve eksiklik durumlarında anında uyarı verir.

### 🎯 Hedefler
- ✅ **Gerçek zamanlı tespit**: 30+ FPS performansla sürekli izleme
- ✅ **Yüksek doğruluk**: %95+ tespit doğruluğu
- ✅ **Çoklu nesne takibi**: SORT algoritması ile kişi takibi
- ✅ **OCR entegrasyonu**: ID kart numarası okuma
- ✅ **Modern arayüz**: Qt6 tabanlı kullanıcı dostu GUI
- ✅ **Veritabanı entegrasyonu**: SQLite ile veri saklama
- ✅ **E-posta bildirimleri**: İhlal durumlarında otomatik uyarı

---

## ✨ Özellikler

### 🔍 Tespit Özellikleri
- **Baret Tespiti**: Güvenlik baretinin varlığını kontrol eder
- **Yelek Tespiti**: Reflektörlü yelek tespiti
- **Eldiven Tespiti**: İş eldiveni kullanımını izler
- **Maske Tespiti**: Koruyucu maske tespiti
- **Kişi Takibi**: SORT algoritması ile çoklu kişi takibi
- **ID Kart OCR**: Yelek üzerindeki ID kart numarasını okur

### 🎮 Kullanıcı Arayüzü
- **Modern Qt6 Arayüzü**: Responsive ve kullanıcı dostu tasarım
- **Canlı Kamera Akışı**: Gerçek zamanlı video görüntüleme
- **KKD Durum Paneli**: Anlık koruyucu donanım durumu
- **İhlal Yönetimi**: Tespit edilen ihlallerin listesi
- **Çalışan Listesi**: Aktif çalışanların takibi
- **Ayarlar Paneli**: Sistem konfigürasyonu

### 🔧 Teknik Özellikler
- **PyTorch Entegrasyonu**: TorchScript modelleri ile hızlı inference
- **PaddleOCR**: Yüksek doğruluklu metin tanıma
- **Çoklu Tracker Desteği**: SORT, Centroid, ByteTracker, OCSort
- **Veritabanı**: SQLite ile veri saklama
- **E-posta Sistemi**: SMTP ile otomatik bildirimler
- **Logging**: Detaylı sistem logları

---

## 🏗️ Mimari

### 📁 Proje Yapısı
```
ppe/
├── 📁 model/                 # Ana tespit modülleri
│   ├── 📁 include/          # Header dosyaları
│   └── 📁 src/              # Kaynak kodlar
├── 📁 ui/                   # Qt6 kullanıcı arayüzü
│   └── 📁 pages/           # Ana sayfalar
├── 📁 cpp_infer/           # PaddleOCR entegrasyonu
├── 📁 ocr_model/           # OCR modelleri
├── 📁 detection_model/     # KKD tespit modelleri
├── 📁 vision/              # TorchVision ops
├── 📁 include/             # Genel header dosyaları
├── 📁 db/                  # Veritabanı modülleri
└── 📄 main.cpp             # Ana uygulama girişi
```

### 🔄 Sistem Akışı
```
📹 Kamera Girişi
    ↓
🔍 YOLO Tespiti (PyTorch)
    ↓
👥 Kişi Takibi (SORT)
    ↓
🛡️ KKD Tespiti
    ↓
📄 ID Kart OCR (PaddleOCR)
    ↓
💾 Veritabanı Kaydı
    ↓
📧 E-posta Bildirimi (İhlal durumunda)
    ↓
🖥️ Qt6 Arayüzü Güncelleme
```

### 🧠 Algoritma Detayları
- **Detection**: YOLOv5 TorchScript modeli
- **Tracking**: SORT (Simple Online and Realtime Tracking)
- **OCR**: PaddleOCR v3 (Chinese + English)
- **Post-processing**: NMS (Non-Maximum Suppression)
- **Data Association**: Hungarian Algorithm

---

## 🚀 Kurulum

### 📋 Sistem Gereksinimleri

#### Minimum Gereksinimler
- **İşletim Sistemi**: macOS 12+, Ubuntu 20.04+, Windows 10+
- **CPU**: Intel i5 / AMD Ryzen 5 (4 çekirdek)
- **RAM**: 8GB
- **GPU**: NVIDIA GTX 1060 (opsiyonel, CUDA desteği)
- **Disk**: 10GB boş alan

#### Önerilen Gereksinimler
- **CPU**: Intel i7 / AMD Ryzen 7 (8 çekirdek)
- **RAM**: 16GB
- **GPU**: NVIDIA RTX 3060+ (CUDA 11.8+)
- **Disk**: SSD 50GB+

### 🔧 Bağımlılıklar

#### macOS (Homebrew)
```bash
# Temel araçlar
brew install cmake git

# Qt6
brew install qt@6

# OpenCV
brew install opencv

# Python (PyTorch için)
brew install python@3.9

# Diğer bağımlılıklar
brew install eigen boost sqlite3 curl
```

#### Ubuntu/Debian
```bash
# Sistem güncellemesi
sudo apt update && sudo apt upgrade -y

# Temel araçlar
sudo apt install -y build-essential cmake git

# Qt6
sudo apt install -y qt6-base-dev qt6-multimedia-dev

# OpenCV
sudo apt install -y libopencv-dev

# Python
sudo apt install -y python3 python3-pip

# Diğer bağımlılıklar
sudo apt install -y libeigen3-dev libboost-all-dev libsqlite3-dev libcurl4-openssl-dev
```

### 📦 PyTorch Kurulumu

#### macOS
```bash
# CPU versiyonu
pip3 install torch torchvision torchaudio

# CUDA versiyonu (NVIDIA GPU varsa)
pip3 install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu118
```

#### Ubuntu
```bash
# CPU versiyonu
pip3 install torch torchvision torchaudio

# CUDA versiyonu
pip3 install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu118
```

### 🏗️ Proje Derleme

```bash
# 1. Projeyi klonla
git clone <repository-url>
cd ppe

# 2. Build dizini oluştur
mkdir build && cd build

# 3. CMake konfigürasyonu
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. Derleme
make -j$(nproc)

# 5. Çalıştırma
./main
```

### 🐳 Docker ile Kurulum (Opsiyonel)

```bash
# Docker image oluştur
docker build -t ppe-detection .

# Container çalıştır
docker run -it --rm \
  --device=/dev/video0:/dev/video0 \
  -p 8080:8080 \
  ppe-detection
```

---

## 💻 Kullanım

### 🎮 Ana Uygulama

```bash
# Uygulamayı başlat
./main

# Tam ekran modunda başlat
./main --fullscreen

# Belirli kamera ile başlat
./main --camera 1

# Model yolu belirt
./main --model /path/to/model.torchscript
```

### 🖥️ Arayüz Kullanımı

#### Ana Sayfa
- **Kamera Başlat/Durdur**: Canlı tespiti kontrol eder
- **KKD Durum Paneli**: Anlık koruyucu donanım durumu
- **Takip ID**: Aktif kişi takip numarası
- **Progress Bar**: Tespit ilerleme durumu

#### İhlaller Sayfası
- **İhlal Listesi**: Tespit edilen tüm ihlaller
- **Filtreleme**: Tarih, kişi, ihlal tipine göre
- **E-posta Gönder**: Seçili ihlalleri mail ile bildir

#### Ayarlar Sayfası
- **KKD Seçimi**: Hangi donanımların kontrol edileceği
- **E-posta Ayarları**: SMTP konfigürasyonu
- **Model Ayarları**: Tespit hassasiyeti
- **Kamera Ayarları**: Çözünürlük, FPS

#### Çalışan Listesi
- **Aktif Çalışanlar**: Şu anda tespit edilen kişiler
- **ID Kart Bilgileri**: OCR ile okunan numaralar
- **Koruyucu Donanım Durumu**: Her çalışanın KKD durumu

### 🔧 Komut Satırı Parametreleri

```bash
./main [OPTIONS]

OPTIONS:
  --help                 Yardım mesajını göster
  --version              Versiyon bilgisini göster
  --camera <index>       Kamera indeksi (varsayılan: 0)
  --model <path>         Model dosya yolu
  --config <path>        Konfigürasyon dosyası
  --fullscreen           Tam ekran modu
  --headless             Arayüz olmadan çalıştır
  --log-level <level>    Log seviyesi (DEBUG, INFO, WARNING, ERROR)
  --output <path>        Çıktı dizini
```

---

## 🔧 Konfigürasyon

### 📄 Konfigürasyon Dosyası

```yaml
# config.yaml
detection:
  model_path: "models/best.torchscript"
  confidence_threshold: 0.5
  nms_threshold: 0.4
  input_size: [640, 640]

tracking:
  algorithm: "SORT"
  max_age: 30
  min_hits: 3
  iou_threshold: 0.3

ocr:
  model_path: "ocr/model/"
  det_db_thresh: 0.3
  det_db_box_thresh: 0.5
  rec_batch_num: 6

camera:
  index: 0
  width: 1920
  height: 1080
  fps: 30

email:
  smtp_server: "smtp.gmail.com"
  smtp_port: 587
  username: "your-email@gmail.com"
  password: "your-app-password"
  recipients: ["admin@company.com"]

database:
  path: "data/ppe.db"
  backup_interval: 24  # saat

logging:
  level: "INFO"
  file: "logs/ppe.log"
  max_size: 100  # MB
```

### 🎛️ Ayarlar Paneli

#### KKD Kontrol Ayarları
- ✅ **Baret Kontrolü**: Güvenlik bareti tespiti
- ✅ **Yelek Kontrolü**: Reflektörlü yelek tespiti
- ✅ **Eldiven Kontrolü**: İş eldiveni tespiti
- ✅ **Maske Kontrolü**: Koruyucu maske tespiti
- ✅ **ID Kart Kontrolü**: OCR ile numara okuma

#### Tespit Hassasiyeti
- **Confidence Threshold**: 0.1 - 1.0 (varsayılan: 0.5)
- **NMS Threshold**: 0.1 - 1.0 (varsayılan: 0.4)
- **Tracking Threshold**: 0.1 - 1.0 (varsayılan: 0.3)

#### E-posta Ayarları
- **SMTP Sunucu**: Gmail, Outlook, vb.
- **Port**: 587 (TLS) veya 465 (SSL)
- **Kimlik Doğrulama**: OAuth2 veya App Password
- **Alıcılar**: Virgülle ayrılmış e-posta listesi


### 📝 Kod Standartları

#### C++ Kod Stili
```cpp
// Dosya adı: PascalCase
// Sınıf adı: PascalCase
// Fonksiyon adı: camelCase
// Değişken adı: snake_case
// Sabit: UPPER_SNAKE_CASE

class DetectionModule {
private:
    cv::Mat current_frame_;
    std::vector<Person> detected_persons_;
    
public:
    bool runDetection(const cv::Mat& frame);
    void updateTracking();
};
```

### 🔍 Debug ve Profiling

#### Debug Modunda Derleme
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

#### Valgrind ile Memory Leak Kontrolü
```bash
valgrind --leak-check=full --show-leak-kinds=all ./main
```

#### Performance Profiling
```bash
# CPU profiling
perf record ./main
perf report

# Memory profiling
valgrind --tool=massif ./main
ms_print massif.out.* > profile.txt
```

## 📄 Lisans

Bu proje **MIT Lisansı** altında lisanslanmıştır. Detaylar için [LICENSE](LICENSE) dosyasına bakın.

### 📚 Kullanılan Kütüphaneler

- **Qt6**: LGPL v3 - [Qt Company](https://www.qt.io/)
- **OpenCV**: Apache 2.0 - [OpenCV](https://opencv.org/)
- **PyTorch**: BSD - [PyTorch](https://pytorch.org/)
- **PaddleOCR**: Apache 2.0 - [PaddlePaddle](https://www.paddlepaddle.org/)
- **Eigen**: MPL2 - [Eigen](http://eigen.tuxfamily.org/)
- **Boost**: Boost Software License - [Boost](https://www.boost.org/)

---

## 📞 İletişim

### 👥 Geliştirici Ekibi
- **Erol Atik**
  - E-posta: [erolatik@marun.edu.tr]
  - GitHub: [github.com/erollatik]
  - LinkedIn: [linkedin.com/in/erolatik]

- **Semih Semerci**
  - E-posta: [semihsemerci@marun.edu.tr]
  - GitHub: [github.com/smhsmrc]
  - LinkedIn: [linkedin.com/in/semih-semerci]

- **Özge Çetinbaş**
  - E-posta: [kerimecetinbas@marun.edu.tr]
  - GitHub: [github.com/ozgecetinbas]
  - LinkedIn: [linkedin.com/in/ozgecetinbas]

### 🏫 Akademik Kurum
- **Marmara Üniversitesi**
- **Teknoloji Fakültesi**
- **Bilgisayar Mühendisliği Bölümü**
- **Adres**: Maltepe/İstanbul
- **Web**: [marmara.edu.tr](https://www.marmara.edu.tr)

### 👨‍🏫 Akademik Danışman
- **Prof. Dr. Serhat Özekes**
  - **Ünvan**: Proje Danışmanı
  - **Bölüm**: Bilgisayar Mühendisliği
  - **Fakülte**: Teknoloji Fakültesi
  - **E-posta**: [serhat.ozekes@marmara.edu.tr]
  - **Marmara Üniversitesi**

## 🙏 Teşekkürler

Bu projede kullanılan açık kaynak kütüphanelerin geliştiricilerine teşekkürler:

- [Ultralytics YOLOv5](https://github.com/ultralytics/yolov5)
- [PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR)
- [SORT](https://github.com/abewley/sort)
- [Qt Company](https://www.qt.io/)
- [OpenCV Team](https://opencv.org/)

### 🎓 Akademik Teşekkürler
- **Marmara Üniversitesi Teknoloji Fakültesi** - Akademik destek için
- **Bilgisayar Mühendisliği Bölümü** - Teknik rehberlik için
- **Prof. Dr. Serhat Özekes** - Proje danışmanlığı ve değerli öneriler için

---

*Bu proje, Marmara Üniversitesi Bilgisayar Mühendisliği Bölümü 2025 Bitirme Projesi olarak geliştirilmiştir.*
