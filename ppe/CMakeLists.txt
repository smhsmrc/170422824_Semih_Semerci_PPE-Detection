
cmake_minimum_required(VERSION 3.10)
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

if (NOT DEFINED NCNN_DIR)
    message("NCNN_DIR is not defined falling back to: ~/InstallationFiles/ncnn/build/install/lib/cmake/ncnn/")
    set(NCNN_DIR ~/InstallationFiles/ncnn/build/install/lib/cmake/ncnn/)
endif()

project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS -pthread)
add_compile_options(-pthread)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(OpenCV_DIR /Users/semihsemerci/detect/opencv/build) 
set(CMAKE_PREFIX_PATH "/opt/homebrew/Cellar/qt/6.7.2;/Users/semihsemerci/detect/libtorch;/Users/semihsemerci/Desktop/trackersCpp-master/ncnn/build/install")

# ===========================================
# PaddleOCR Entegrasyonu - KAYNAK DOSYALARI İLE
# ===========================================
option(WITH_PADDLE_OCR "Enable PaddleOCR support" ON)

if(WITH_PADDLE_OCR)
    message(STATUS "🚀 PaddleOCR entegrasyonu aktif...")
    
    # PaddleOCR dizin ayarları
    set(PADDLE_OCR_DIR ${CMAKE_SOURCE_DIR}/cpp_infer)
    set(PADDLE_LIB ${PADDLE_OCR_DIR}/paddle_inference_install_dir/paddle)

    # Ana kütüphane kontrolü
    if(EXISTS "${PADDLE_LIB}/lib/libpaddle_inference.dylib")
        message(STATUS "✅ PaddleOCR ana kütüphanesi bulundu")
        
        # Include dizinleri
        include_directories("${PADDLE_LIB}/include")
        include_directories("${PADDLE_OCR_DIR}")
        include_directories("${PADDLE_OCR_DIR}/include")
        include_directories("${PADDLE_OCR_DIR}/src")
        
        # AutoLog include dizinini ekleyin
        include_directories("${PADDLE_OCR_DIR}/build/third-party/extern_autolog-src")
        
        include_directories("${CMAKE_SOURCE_DIR}/ocr/include")
        
        # PaddleOCR kaynak dosyalarını bulun - main.cpp'yi hariç tutun
        file(GLOB_RECURSE PADDLE_OCR_SOURCES 
            "${PADDLE_OCR_DIR}/src/*.cpp"
            "${PADDLE_OCR_DIR}/src/*.cc"
        )
        
        # main.cpp dosyasını listeden çıkarın (çakışma olmasın)
        list(FILTER PADDLE_OCR_SOURCES EXCLUDE REGEX ".*main\\.cpp$")
        
        # Kaynak dosyalarını listeleyin
        message(STATUS "📄 PaddleOCR kaynak dosyaları:")
        foreach(src_file ${PADDLE_OCR_SOURCES})
            message(STATUS "   - ${src_file}")
        endforeach()
        
        # Ana PaddleOCR kütüphanesi
        set(PADDLE_OCR_LIBS "${PADDLE_LIB}/lib/libpaddle_inference.dylib")
        
        # Homebrew kütüphaneleri
        find_library(GLOG_LIB glog PATHS /opt/homebrew/lib NO_DEFAULT_PATH)
        find_library(GFLAGS_LIB gflags PATHS /opt/homebrew/lib NO_DEFAULT_PATH)
        find_library(PROTOBUF_LIB protobuf PATHS /opt/homebrew/lib NO_DEFAULT_PATH)
        
        if(GLOG_LIB)
            list(APPEND PADDLE_OCR_LIBS ${GLOG_LIB})
            message(STATUS "📦 glog eklendi: ${GLOG_LIB}")
        endif()
        
        if(GFLAGS_LIB)
            list(APPEND PADDLE_OCR_LIBS ${GFLAGS_LIB})
            message(STATUS "📦 gflags eklendi: ${GFLAGS_LIB}")
        endif()
        
        if(PROTOBUF_LIB)
            list(APPEND PADDLE_OCR_LIBS ${PROTOBUF_LIB})
            message(STATUS "📦 protobuf eklendi: ${PROTOBUF_LIB}")
        endif()

        # Sistem kütüphaneleri
        find_library(PNG_LIBRARY png)
        if(PNG_LIBRARY)
            list(APPEND PADDLE_OCR_LIBS ${PNG_LIBRARY})
            message(STATUS "📸 PNG eklendi: ${PNG_LIBRARY}")
        endif()

        # PaddleOCR tanımlaması
        add_definitions(-DWITH_PADDLE_OCR)
        
        message(STATUS "✅ PaddleOCR support enabled")
        message(STATUS "📚 Kütüphaneler: ${PADDLE_OCR_LIBS}")
        message(STATUS "🔍 AutoLog include: ${PADDLE_OCR_DIR}/build/third-party/extern_autolog-src")
    else()
        message(WARNING "❌ PaddleOCR ana kütüphanesi bulunamadı: ${PADDLE_LIB}/lib/libpaddle_inference.dylib")
        set(WITH_PADDLE_OCR OFF)
        set(PADDLE_OCR_LIBS "")
        set(PADDLE_OCR_SOURCES "")
    endif()
else()
    message(STATUS "⚠️ PaddleOCR support disabled")
    set(PADDLE_OCR_LIBS "")
    set(PADDLE_OCR_SOURCES "")
endif()


find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(X11 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia MultimediaWidgets Widgets)

# ✨ Qt MOC otomatik ayarı
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
set(BOOST_ROOT /usr/local/include)
find_package (Boost REQUIRED COMPONENTS system filesystem log thread)
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIR})
endif ()

# NCNN
message("(trackersCpp) NCNN_DIR: ${NCNN_DIR}")
set(ncnn_DIR ${NCNN_DIR})

find_package(ncnn REQUIRED)
if(NOT TARGET ncnn)
    message(WARNING "ncnn NOT FOUND! CHECK trackersCpp/CMakeLists.txt")
else()
    message("ncnn FOUND")
endif()

# Vulkan
set(Vulkan_INCLUDE_DIR "/opt/homebrew/include")
set(Vulkan_LIBRARY "/opt/homebrew/lib/libvulkan.dylib")
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIR})

# Torch ve TorchVision'ın yollarını ayarla
set(TorchVision_DIR "/Users/semihsemerci/tez/torchvision/build/install/share/cmake/TorchVision")

# LibTorch'u dahil et
find_package(Torch REQUIRED)

# libcurl'u dahil et
find_package(CURL REQUIRED)

# TorchVision'ı dahil et
find_package(TorchVision REQUIRED)

# Include ve linkleme yollarını belirt
include_directories("/usr/local/include/torchvisio/ops")

# yaml-cpp'yi bul ve ekle
find_package(yaml-cpp REQUIRED)

# ✨ MOC artık otomatik - qt6_wrap_cpp çağrılarını kaldırıyoruz

add_executable(main 
    main.cpp 
    ui/pages/src/mainPage.cpp
    ui/pages/src/loginPage.cpp
    ui/pages/src/violation.cpp
    ui/pages/src/settings.cpp
    model/src/detection.cpp
    model/src/progressBar.cpp
    model/src/person.cpp
    model/src/mail.cpp
    ui/pages/src/workerList.cpp
    db/src/db.cpp
    model/src/crop.cpp
    # ✨ Header dosyaları - MOC için gerekli
    ui/pages/include/mainPage.h
    ui/pages/include/loginPage.h
    ui/pages/include/violation.h
    ui/pages/include/workerList.h
    ui/pages/include/settings.h
    model/include/crop.h
    db/include/db.h
    # PaddleOCR kaynak dosyalarını ekleyin
    ${PADDLE_OCR_SOURCES}
    ${SRC_FILES}
)

# EIGEN
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

include_directories(
    ncnn
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/BYTETracker)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/Centroid)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/OCSort)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/Sort)

file(GLOB_RECURSE BYTETracker ${CMAKE_CURRENT_SOURCE_DIR}/src/BYTETracker/*.cpp)
file(GLOB_RECURSE Centroid ${CMAKE_CURRENT_SOURCE_DIR}/src/Centroid/*.cpp)
file(GLOB_RECURSE OCSort ${CMAKE_CURRENT_SOURCE_DIR}/src/OCSort/*.cpp)
file(GLOB_RECURSE Sort ${CMAKE_CURRENT_SOURCE_DIR}/src/Sort/*.cpp)
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Qt6 kütüphanesini ve Vulkan'ı linkle
target_link_libraries(main 
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets
    ${Vulkan_LIBRARY}
    "${TORCH_LIBRARIES}" 
    "/usr/local/lib/libtorchvision.dylib" 
    ${OpenCV_LIBS} 
    yaml-cpp::yaml-cpp
    ncnn
    sqlite3
    CURL::libcurl
    ${X11_LIBRARIES}    
    ${CMAKE_THREAD_LIBS_INIT}
    trackersCpp
    # PaddleOCR kütüphaneleri
    ${PADDLE_OCR_LIBS}
)

add_library(trackersCpp
            ${BYTETracker}
            ${Centroid}
            ${OCSort}
            ${Sort}
)  

target_include_directories(trackersCpp PUBLIC
    ${OpenCV_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}
    ${GSTREAMER_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/BYTETracker
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Centroid
    ${CMAKE_CURRENT_SOURCE_DIR}/include/OCSort
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Sort
    ${EIGEN3_INCLUDE_DIR}
)
