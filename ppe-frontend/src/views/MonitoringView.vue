<template>
  <div class="monitoring-container">
    <!-- Sayfa Başlığı -->
    <div class="page-header">
      <div class="header-content">
        <h1>📹 Canlı İzleme</h1>
        <p>Gerçek zamanlı kamera görüntüleri ve PPE tespit sistemi</p>
      </div>
      <div class="header-actions">
        <div class="monitoring-status" :class="monitoringStatus">
          <div class="status-indicator"></div>
          <span>{{ getStatusText() }}</span>
        </div>
        <button 
          @click="toggleMonitoring" 
          :class="['btn', monitoringActive ? 'btn-danger' : 'btn-success']"
          :disabled="isLoading"
        >
          <span v-if="monitoringActive">⏹️ Durdur</span>
          <span v-else>▶️ Başlat</span>
        </button>
        <button @click="refreshCameras" class="btn btn-primary" :disabled="isLoading">
          🔄 Yenile
        </button>
      </div>
    </div>

    <!-- Kontrol Paneli -->
    <div class="control-panel">
      <div class="panel-section">
        <h3>🎛️ Kontrol Paneli</h3>
        <div class="controls-grid">
          <div class="control-group">
            <label>📹 Görünüm Modu:</label>
            <select v-model="viewMode" @change="updateLayout" class="form-control">
              <option value="grid">Izgara Görünümü</option>
              <option value="focus">Odaklanmış Görünüm</option>
              <option value="list">Liste Görünümü</option>
            </select>
          </div>
          
          <div class="control-group">
            <label>🔍 Tespit Hassasiyeti:</label>
            <input 
              v-model="detectionSensitivity" 
              @change="updateSettings"
              type="range" 
              min="0.3" 
              max="0.9" 
              step="0.1" 
              class="sensitivity-slider"
            >
            <span class="sensitivity-value">{{ Math.round(detectionSensitivity * 100) }}%</span>
          </div>
          
          <div class="control-group">
            <label>⚠️ Uyarı Eşiği:</label>
            <input 
              v-model="alertThreshold" 
              @change="updateSettings"
              type="range" 
              min="1" 
              max="10" 
              step="1" 
              class="threshold-slider"
            >
            <span class="threshold-value">{{ alertThreshold }} ihlal</span>
          </div>
          
          <div class="control-group">
            <label>🔊 Sesli Uyarı:</label>
            <label class="switch">
              <input v-model="audioAlerts" @change="updateSettings" type="checkbox">
              <span class="slider"></span>
            </label>
          </div>
        </div>
      </div>

      <div class="panel-section">
        <h3>📊 Anlık İstatistikler</h3>
        <div class="stats-grid">
          <div class="stat-item">
            <div class="stat-icon">📹</div>
            <div class="stat-content">
              <div class="stat-value">{{ cameras.filter(c => c.status === 'active').length }}</div>
              <div class="stat-label">Aktif Kamera</div>
            </div>
          </div>
          
          <div class="stat-item">
            <div class="stat-icon">👥</div>
            <div class="stat-content">
              <div class="stat-value">{{ liveStats.currentDetections || 0 }}</div>
              <div class="stat-label">Şu An Tespit</div>
            </div>
          </div>
          
          <div class="stat-item">
            <div class="stat-icon">⚠️</div>
            <div class="stat-content">
              <div class="stat-value">{{ liveStats.todayViolations || 0 }}</div>
              <div class="stat-label">Bugünkü İhlal</div>
            </div>
          </div>
          
          <div class="stat-item">
            <div class="stat-icon">📈</div>
            <div class="stat-content">
              <div class="stat-value">{{ liveStats.complianceRate || 0 }}%</div>
              <div class="stat-label">Anlık Uyum</div>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- Kamera Görünümleri -->
    <div class="cameras-section">
      <div class="section-header">
        <h3>📹 Kamera Görünümleri</h3>
        <div class="camera-filters">
          <select v-model="selectedLocation" @change="filterCameras" class="form-control">
            <option value="">Tüm Lokasyonlar</option>
            <option v-for="location in locations" :key="location" :value="location">
              {{ location }}
            </option>
          </select>
          <select v-model="cameraStatus" @change="filterCameras" class="form-control">
            <option value="">Tüm Durumlar</option>
            <option value="active">Aktif</option>
            <option value="inactive">Pasif</option>
            <option value="error">Hata</option>
          </select>
        </div>
      </div>

      <!-- Izgara Görünümü -->
      <div v-if="viewMode === 'grid'" class="cameras-grid" :class="getGridClass()">
        <div 
          v-for="camera in filteredCameras" 
          :key="camera.id"
          class="camera-card"
          :class="{ 'has-violations': camera.currentViolations > 0 }"
        >
          <div class="camera-header">
            <div class="camera-info">
              <h4>{{ camera.name }}</h4>
              <span class="camera-location">📍 {{ camera.location }}</span>
            </div>
            <div class="camera-status" :class="camera.status">
              <div class="status-dot"></div>
              <span>{{ getStatusText(camera.status) }}</span>
            </div>
          </div>
          
          <!-- Kamera Stream Alanı -->
          <div class="camera-stream">
            <PPEDetection />
          </div>
          
          <div class="camera-footer">
            <div class="camera-metrics">
              <div class="metric">
                <span class="metric-label">Tespit:</span>
                <span class="metric-value">{{ camera.currentDetections || 0 }}</span>
              </div>
              <div class="metric">
                <span class="metric-label">İhlal:</span>
                <span class="metric-value violation-count" :class="{ 'has-violations': camera.currentViolations > 0 }">
                  {{ camera.currentViolations || 0 }}
                </span>
              </div>
              <div class="metric">
                <span class="metric-label">FPS:</span>
                <span class="metric-value">{{ camera.fps || 0 }}</span>
              </div>
            </div>
            
            <div class="camera-actions">
              <button @click="focusCamera(camera)" class="action-btn" title="Odakla">👁️</button>
              <button @click="takeSnapshot(camera)" class="action-btn" title="Ekran Görüntüsü">📸</button>
              <button @click="showCameraSettings(camera)" class="action-btn" title="Ayarlar">⚙️</button>
            </div>
          </div>
        </div>
      </div>

      <!-- Odaklanmış Görünüm -->
      <div v-if="viewMode === 'focus' && focusedCamera" class="focused-view">
        <div class="focused-camera">
          <div class="focused-header">
            <div class="camera-info">
              <h2>{{ focusedCamera.name }}</h2>
              <p>📍 {{ focusedCamera.location }}</p>
            </div>
            <div class="focused-controls">
              <button @click="viewMode = 'grid'" class="btn btn-secondary">🔙 Geri</button>
              <button @click="toggleFullscreen" class="btn btn-primary">🔍 Tam Ekran</button>
            </div>
          </div>
          <div class="focused-stream">
            <PPEDetection />
          </div>
        </div>
      </div>

      <!-- Liste Görünümü -->
      <div v-if="viewMode === 'list'" class="list-view">
        <div class="list-header">
          <h3>📋 Kamera Listesi</h3>
        </div>
        <div class="cameras-list">
          <div 
            v-for="camera in filteredCameras" 
            :key="camera.id"
            class="camera-list-item"
            :class="{ 'has-violations': camera.currentViolations > 0 }"
          >
            <div class="list-item-info">
              <h4>{{ camera.name }}</h4>
              <p>📍 {{ camera.location }}</p>
              <div class="list-item-status" :class="camera.status">
                <div class="status-dot"></div>
                <span>{{ getStatusText(camera.status) }}</span>
              </div>
            </div>
            <div class="list-item-metrics">
              <div class="metric">
                <span class="metric-label">Tespit:</span>
                <span class="metric-value">{{ camera.currentDetections || 0 }}</span>
              </div>
              <div class="metric">
                <span class="metric-label">İhlal:</span>
                <span class="metric-value violation-count" :class="{ 'has-violations': camera.currentViolations > 0 }">
                  {{ camera.currentViolations || 0 }}
                </span>
              </div>
            </div>
            <div class="list-item-actions">
              <button @click="focusCamera(camera)" class="action-btn">👁️ Görüntüle</button>
              <button @click="showCameraSettings(camera)" class="action-btn">⚙️ Ayarlar</button>
            </div>
            <PPEDetection />
          </div>
        </div>
      </div>
    </div>

    <!-- Son Tespitler Paneli -->
    <div class="recent-detections">
      <div class="section-header">
        <h3>🔍 Son Tespitler</h3>
        <button @click="clearDetections" class="btn btn-secondary">🗑️ Temizle</button>
      </div>
      <div v-if="recentDetections.length > 0" class="detections-timeline">
        <div 
          v-for="detection in recentDetections.slice(0, 10)" 
          :key="detection.id"
          class="timeline-item"
          :class="{ 'has-violation': detection.violations.length > 0 }"
        >
          <div class="timeline-time">{{ formatTime(detection.timestamp) }}</div>
          <div class="timeline-content">
            <div class="detection-info">
              <span class="camera-name">📹 {{ getCameraName(detection.camera_id) }}</span>
              <span class="person-id">👤 {{ detection.person_id }}</span>
            </div>
            <div class="equipment-summary">
              <span :class="['equipment-status', detection.equipment.helmet ? 'ok' : 'violation']">
                🪖 {{ detection.equipment.helmet ? '✓' : '✗' }}
              </span>
              <span :class="['equipment-status', detection.equipment.vest ? 'ok' : 'violation']">
                🦺 {{ detection.equipment.vest ? '✓' : '✗' }}
              </span>
              <span :class="['equipment-status', detection.equipment.gloves ? 'ok' : 'violation']">
                🧤 {{ detection.equipment.gloves ? '✓' : '✗' }}
              </span>
            </div>
            <div v-if="detection.violations.length > 0" class="violations-list">
              <span class="violation-tag" v-for="violation in detection.violations" :key="violation">
                ⚠️ {{ violation }}
              </span>
            </div>
          </div>
        </div>
      </div>
      <p v-else class="no-detections">Henüz tespit kaydı yok</p>
    </div>

    <!-- Ayarlar Modal'ı -->
    <div v-if="showSettingsModal" class="modal-overlay" @click="closeSettingsModal">
      <div class="modal-content" @click.stop>
        <div class="modal-header">
          <h3>⚙️ {{ selectedCamera?.name }} Ayarları</h3>
          <button @click="closeSettingsModal" class="close-btn">✕</button>
        </div>
        <div class="modal-body">
          <div class="setting-group">
            <label>Tespit Hassasiyeti:</label>
            <input 
              v-model="cameraSettings.sensitivity" 
              type="range" 
              min="0.3" 
              max="0.9" 
              step="0.1"
            >
            <span>{{ Math.round(cameraSettings.sensitivity * 100) }}%</span>
          </div>
          <div class="setting-group">
            <label>Çözünürlük:</label>
            <select v-model="cameraSettings.resolution">
              <option value="720p">720p</option>
              <option value="1080p">1080p</option>
              <option value="4K">4K</option>
            </select>
          </div>
          <div class="setting-group">
            <label>FPS:</label>
            <select v-model="cameraSettings.fps">
              <option value="15">15 FPS</option>
              <option value="30">30 FPS</option>
              <option value="60">60 FPS</option>
            </select>
          </div>
        </div>
        <div class="modal-footer">
          <button @click="saveSettings" class="btn btn-primary">💾 Kaydet</button>
          <button @click="closeSettingsModal" class="btn btn-secondary">❌ İptal</button>
        </div>
      </div>
    </div>

    <!-- Bildirim Alanı -->
    <div v-if="notifications.length > 0" class="notifications">
      <div 
        v-for="notification in notifications" 
        :key="notification.id"
        class="notification"
        :class="notification.type"
      >
        <div class="notification-content">
          <span class="notification-icon">{{ getNotificationIcon(notification.type) }}</span>
          <span class="notification-message">{{ notification.message }}</span>
        </div>
        <button @click="dismissNotification(notification.id)" class="notification-close">✕</button>
      </div>
    </div>
  </div>
</template>

<script>
import PPEDetection from '../components/PPEDetection.vue';
import { ppeAPI } from '@/api/ppe.js';

export default {
  name: 'MonitoringView',
  components: { PPEDetection },
  data() {
    return {
      // Monitoring durumu
      monitoringActive: false,
      isLoading: false,
      
      // Görünüm ayarları
      viewMode: 'grid',
      detectionSensitivity: 0.7,
      alertThreshold: 3,
      audioAlerts: true,
      
      // Kamera verileri
      cameras: [
        {
          id: 'cam_001',
          name: 'Ana Giriş Kamerası',
          location: 'Ana Giriş',
          status: 'active',
          streamUrl: '/api/ppe/camera/stream?id=cam_001',
          currentDetections: 0,
          currentViolations: 0,
          fps: 30,
          detections: []
        },
        {
          id: 'cam_002',
          name: 'Üretim Alanı Kamerası',
          location: 'Üretim Sahası',
          status: 'active',
          streamUrl: '/api/ppe/camera/stream?id=cam_002',
          currentDetections: 0,
          currentViolations: 0,
          fps: 30,
          detections: []
        },
        {
          id: 'cam_003',
          name: 'Depo Kamerası',
          location: 'Depo',
          status: 'inactive',
          streamUrl: '/api/ppe/camera/stream?id=cam_003',
          currentDetections: 0,
          currentViolations: 0,
          fps: 0,
          detections: []
        }
      ],
      
      // Filtreleme
      selectedLocation: '',
      cameraStatus: '',
      filteredCameras: [],
      
      // Odaklanmış görünüm
      focusedCamera: null,
      
      // İstatistikler
      liveStats: {
        currentDetections: 0,
        todayViolations: 0,
        complianceRate: 0
      },
      
      // Son tespitler
      recentDetections: [],
      
      // Ayarlar modal'ı
      showSettingsModal: false,
      selectedCamera: null,
      cameraSettings: {
        sensitivity: 0.7,
        resolution: '1080p',
        fps: 30
      },
      
      // Bildirimler
      notifications: [],
      
      // Update interval'ları
      statsUpdateInterval: null,
      detectionsUpdateInterval: null,
      
      // Ses dosyaları
      alertSound: null
    }
  },
  
  computed: {
    monitoringStatus() {
      return this.monitoringActive ? 'active' : 'inactive';
    },
    
    locations() {
      return [...new Set(this.cameras.map(camera => camera.location))];
    }
  },
  
  async mounted() {
    await this.initializeMonitoring();
    this.filteredCameras = [...this.cameras];
    this.loadAudioFiles();
  },
  
  beforeUnmount() {
    this.stopUpdates();
  },
  
  methods: {
    // Monitoring başlatma/durdurma
    async toggleMonitoring() {
      this.isLoading = true;
      try {
        if (this.monitoringActive) {
          await ppeAPI.stopMonitoring();
          this.stopUpdates();
          this.showNotification('Monitoring durduruldu ⏹️', 'info');
        } else {
          await ppeAPI.startMonitoring();
          this.startUpdates();
          this.showNotification('Monitoring başlatıldı ▶️', 'success');
        }
        this.monitoringActive = !this.monitoringActive;
      } catch (error) {
        this.showNotification(`Hata: ${error.message} ❌`, 'error');
      } finally {
        this.isLoading = false;
      }
    },
    
    // İlk kurulum
    async initializeMonitoring() {
      try {
        await this.loadStats();
        this.showNotification('Monitoring sistemi hazır 🚀', 'success');
      } catch (error) {
        this.showNotification('Sistem başlatma hatası ⚠️', 'error');
      }
    },
    
    // Güncellemeleri başlat
    startUpdates() {
      this.statsUpdateInterval = setInterval(() => {
        this.loadStats();
      }, 3000);
      
      this.detectionsUpdateInterval = setInterval(() => {
        this.loadDetections();
      }, 1000);
    },
    
    // Güncellemeleri durdur
    stopUpdates() {
      if (this.statsUpdateInterval) {
        clearInterval(this.statsUpdateInterval);
        this.statsUpdateInterval = null;
      }
      
      if (this.detectionsUpdateInterval) {
        clearInterval(this.detectionsUpdateInterval);
        this.detectionsUpdateInterval = null;
      }
    },
    
    // İstatistikleri yükle
    async loadStats() {
      try {
        const response = await ppeAPI.getStats();
        this.liveStats = response.data || {};
        
        // Kamera verilerini güncelle
        this.cameras.forEach(camera => {
          camera.currentDetections = Math.floor(Math.random() * 5);
          camera.currentViolations = Math.floor(Math.random() * 3);
          camera.fps = camera.status === 'active' ? 30 : 0;
        });
      } catch (error) {
        console.error('İstatistik yükleme hatası:', error);
      }
    },
    
    // Tespitleri yükle
    async loadDetections() {
      try {
        const response = await ppeAPI.getDetections();
        const detections = response.data.detections || [];
        
        // Yeni tespitleri işle
        detections.forEach(detection => {
          this.processNewDetection(detection);
        });
        
        this.recentDetections = detections.slice(0, 20);
      } catch (error) {
        console.error('Tespit yükleme hatası:', error);
      }
    },
    
    // Yeni tespit işleme
    processNewDetection(detection) {
      // Kameraya tespit ekle
      const camera = this.cameras.find(c => c.id === detection.camera_id);
      if (camera) {
        if (!camera.detections) camera.detections = [];
        camera.detections.unshift(detection);
        camera.detections = camera.detections.slice(0, 5); // Son 5 tespit
        
        // İhlal varsa uyarı
        if (detection.violations && detection.violations.length > 0) {
          this.handleViolationAlert(detection);
        }
      }
    },
    
    // İhlal uyarısı
    handleViolationAlert(detection) {
      const cameraName = this.getCameraName(detection.camera_id);
      const message = `${cameraName} - PPE İhlali tespit edildi!`;
      
      this.showNotification(message, 'warning');
      
      if (this.audioAlerts && this.alertSound) {
        this.alertSound.play().catch(e => console.log('Ses çalınamadı:', e));
      }
    },
    
    // Kamera stream URL'i
    getCameraStreamUrl(cameraId) {
      return `${import.meta.env.VITE_API_BASE_URL}/ppe/camera/stream?id=${cameraId}&t=${Date.now()}`;
    },
    
    // Durum metni
    getStatusText(status = null) {
      const currentStatus = status || this.monitoringStatus;
      const statusTexts = {
        active: 'Aktif',
        inactive: 'Pasif',
        error: 'Hata'
      };
      return statusTexts[currentStatus] || 'Bilinmiyor';
    },
    
    // Grid sınıfı
    getGridClass() {
      const cameraCount = this.filteredCameras.length;
      if (cameraCount <= 1) return 'grid-1';
      if (cameraCount <= 4) return 'grid-2x2';
      if (cameraCount <= 6) return 'grid-3x2';
      return 'grid-3x3';
    },
    
    // Kamera filtreleme
    filterCameras() {
      this.filteredCameras = this.cameras.filter(camera => {
        const locationMatch = !this.selectedLocation || camera.location === this.selectedLocation;
        const statusMatch = !this.cameraStatus || camera.status === this.cameraStatus;
        return locationMatch && statusMatch;
      });
    },
    
    // Kameraları yenile
    async refreshCameras() {
      this.isLoading = true;
      try {
        await this.loadStats();
        await this.loadDetections();
        this.showNotification('Kameralar yenilendi 🔄', 'success');
      } catch (error) {
        this.showNotification('Yenileme hatası ⚠️', 'error');
      } finally {
        this.isLoading = false;
      }
    },
    
    // Kameraya odaklan
    focusCamera(camera) {
      this.focusedCamera = camera;
      this.viewMode = 'focus';
    },
    
    // Ekran görüntüsü al
    async takeSnapshot(camera) {
      try {
        // Burada snapshot API'si çağrılacak
        this.showNotification(`${camera.name} ekran görüntüsü alındı 📸`, 'success');
      } catch (error) {
        this.showNotification('Ekran görüntüsü alınamadı ❌', 'error');
      }
    },
    
    // Kamera ayarlarını göster
    showCameraSettings(camera) {
      this.selectedCamera = camera;
      this.cameraSettings = {
        sensitivity: this.detectionSensitivity,
        resolution: '1080p',
        fps: 30
      };
      this.showSettingsModal = true;
    },
    
    // Ayarları kaydet
    async saveSettings() {
      try {
        // Burada ayarlar API'sine gönderilecek
        this.showNotification('Ayarlar kaydedildi 💾', 'success');
        this.closeSettingsModal();
      } catch (error) {
        this.showNotification('Ayarlar kaydedilemedi ❌', 'error');
      }
    },
    
    // Ayarlar modal'ını kapat
    closeSettingsModal() {
      this.showSettingsModal = false;
      this.selectedCamera = null;
    },
    
    // Tam ekran toggle
    toggleFullscreen() {
      const element = document.querySelector('.focused-stream');
      if (!document.fullscreenElement) {
        element.requestFullscreen().catch(err => {
          console.error('Tam ekran hatası:', err);
        });
      } else {
        document.exitFullscreen();
      }
    },
    
    // Layout güncelle
    updateLayout() {
      // Görünüm modu değiştiğinde gerekli işlemler
      if (this.viewMode === 'grid') {
        this.focusedCamera = null;
      }
    },
    
    // Ayarları güncelle
    updateSettings() {
      // Ayarlar değiştiğinde API'ye gönder
      console.log('Ayarlar güncellendi:', {
        sensitivity: this.detectionSensitivity,
        alertThreshold: this.alertThreshold,
        audioAlerts: this.audioAlerts
      });
    },
    
    // Tespitleri temizle
    clearDetections() {
      this.recentDetections = [];
      this.cameras.forEach(camera => {
        camera.detections = [];
        camera.currentDetections = 0;
        camera.currentViolations = 0;
      });
      this.showNotification('Tespit geçmişi temizlendi 🗑️', 'info');
    },
    
    // Zaman formatla
    formatTime(timestamp) {
      if (!timestamp) return '';
      const date = new Date(timestamp * 1000);
      return date.toLocaleTimeString('tr-TR');
    },
    
    // Kamera adını al
    getCameraName(cameraId) {
      const camera = this.cameras.find(c => c.id === cameraId);
      return camera ? camera.name : 'Bilinmeyen Kamera';
    },
    
    // Bildirim göster
    showNotification(message, type = 'info') {
      const notification = {
        id: Date.now(),
        message,
        type,
        timestamp: Date.now()
      };
      
      this.notifications.unshift(notification);
      
      // 5 saniye sonra otomatik kaldır
      setTimeout(() => {
        this.dismissNotification(notification.id);
      }, 5000);
    },
    
    // Bildirimi kapat
    dismissNotification(id) {
      this.notifications = this.notifications.filter(n => n.id !== id);
    },
    
    // Bildirim ikonu
    getNotificationIcon(type) {
      const icons = {
        success: '✅',
        error: '❌',
        warning: '⚠️',
        info: 'ℹ️'
      };
      return icons[type] || 'ℹ️';
    },
    
    // Ses dosyalarını yükle
    loadAudioFiles() {
      try {
        this.alertSound = new Audio('/sounds/alert.mp3');
        this.alertSound.volume = 0.5;
      } catch (error) {
        console.log('Ses dosyası yüklenemedi:', error);
      }
    }
  }
}
</script>

<style scoped>
/* Ana Container */
.monitoring-container {
  padding: 20px;
  background: #f8f9fa;
  min-height: 100vh;
}

/* Sayfa Başlığı */
.page-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  background: white;
  padding: 20px;
  border-radius: 12px;
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
  margin-bottom: 20px;
}

.header-content h1 {
  margin: 0;
  color: #2c3e50;
  font-size: 28px;
}

.header-content p {
  margin: 5px 0 0 0;
  color: #7f8c8d;
  font-size: 14px;
}

.header-actions {
  display: flex;
  align-items: center;
  gap: 15px;
}

.monitoring-status {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px 16px;
  border-radius: 20px;
  font-size: 14px;
  font-weight: 500;
}

.monitoring-status.active {
  background: #d4edda;
  color: #155724;
}

.monitoring-status.inactive {
  background: #f8d7da;
  color: #721c24;
}

.status-indicator {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  animation: pulse 2s infinite;
}

.monitoring-status.active .status-indicator {
  background: #28a745;
}

.monitoring-status.inactive .status-indicator {
  background: #dc3545;
}

@keyframes pulse {
  0% { opacity: 1; }
  50% { opacity: 0.5; }
  100% { opacity: 1; }
}

/* Butonlar */
.btn {
  padding: 10px 20px;
  border: none;
  border-radius: 6px;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.3s;
  display: flex;
  align-items: center;
  gap: 8px;
}

.btn:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}

.btn-success {
  background: #28a745;
  color: white;
}

.btn-success:hover:not(:disabled) {
  background: #218838;
}

.btn-danger {
  background: #dc3545;
  color: white;
}

.btn-danger:hover:not(:disabled) {
  background: #c82333;
}

.btn-primary {
  background: #007bff;
  color: white;
}

.btn-primary:hover:not(:disabled) {
  background: #0056b3;
}

.btn-secondary {
  background: #6c757d;
  color: white;
}

.btn-secondary:hover:not(:disabled) {
  background: #545b62;
}

/* Kontrol Paneli */
.control-panel {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 20px;
  margin-bottom: 20px;
}

.panel-section {
  background: white;
  padding: 20px;
  border-radius: 12px;
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
}

.panel-section h3 {
  margin: 0 0 20px 0;
  color: #2c3e50;
  font-size: 18px;
}

.controls-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 20px;
}

.control-group {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.control-group label {
  font-weight: 500;
  color: #495057;
  font-size: 14px;
}

.form-control {
  padding: 8px 12px;
  border: 1px solid #ced4da;
  border-radius: 4px;
  font-size: 14px;
}

.sensitivity-slider, .threshold-slider {
  width: 100%;
}

.sensitivity-value, .threshold-value {
  font-size: 12px;
  color: #6c757d;
  text-align: center;
}

/* Switch */
.switch {
  position: relative;
  display: inline-block;
  width: 50px;
  height: 24px;
}

.switch input {
  opacity: 0;
  width: 0;
  height: 0;
}

.slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #ccc;
  transition: .4s;
  border-radius: 24px;
}

.slider:before {
  position: absolute;
  content: "";
  height: 18px;
  width: 18px;
  left: 3px;
  bottom: 3px;
  background-color: white;
  transition: .4s;
  border-radius: 50%;
}

input:checked + .slider {
  background-color: #28a745;
}

input:checked + .slider:before {
  transform: translateX(26px);
}

/* İstatistikler */
.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
  gap: 15px;
}

.stat-item {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 15px;
  background: #f8f9fa;
  border-radius: 8px;
  border-left: 4px solid #007bff;
}

.stat-icon {
  font-size: 24px;
}

.stat-content {
  flex: 1;
}

.stat-value {
  font-size: 24px;
  font-weight: bold;
  color: #2c3e50;
  line-height: 1;
}

.stat-label {
  font-size: 12px;
  color: #6c757d;
  margin-top: 4px;
}

/* Kamera Bölümü */
.cameras-section {
  background: white;
  border-radius: 12px;
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
  overflow: hidden;
  margin-bottom: 20px;
}

.section-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20px;
  border-bottom: 1px solid #e9ecef;
}

.section-header h3 {
  margin: 0;
  color: #2c3e50;
}

.camera-filters {
  display: flex;
  gap: 10px;
}

.camera-filters .form-control {
  min-width: 150px;
}

/* Kamera Grid */
.cameras-grid {
  padding: 20px;
  display: grid;
  gap: 20px;
}

.grid-1 { grid-template-columns: 1fr; }
.grid-2x2 { grid-template-columns: repeat(2, 1fr); }
.grid-3x2 { grid-template-columns: repeat(3, 1fr); }
.grid-3x3 { grid-template-columns: repeat(3, 1fr); }

.camera-card {
  background: white;
  border: 2px solid #e9ecef;
  border-radius: 12px;
  overflow: hidden;
  transition: all 0.3s;
}

.camera-card:hover {
  box-shadow: 0 4px 12px rgba(0,0,0,0.15);
}

.camera-card.has-violations {
  border-color: #dc3545;
  animation: violationPulse 2s infinite;
}

@keyframes violationPulse {
  0%, 100% { border-color: #dc3545; }
  50% { border-color: #ff6b7a; }
}

.camera-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 15px;
  background: #f8f9fa;
  border-bottom: 1px solid #e9ecef;
}

.camera-info h4 {
  margin: 0;
  color: #2c3e50;
  font-size: 16px;
}

.camera-location {
  font-size: 12px;
  color: #6c757d;
}

.camera-status {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 12px;
  font-weight: 500;
}

.camera-status.active {
  color: #28a745;
}

.camera-status.inactive {
  color: #6c757d;
}

.camera-status.error {
  color: #dc3545;
}

.status-dot {
  width: 6px;
  height: 6px;
  border-radius: 50%;
  background: currentColor;
}

/* Kamera Stream */
.camera-stream {
  position: relative;
  aspect-ratio: 16/9;
  background: #000;
  overflow: hidden;
}

.stream-image {
  width: 100%;
  height: 100%;
  object-fit: cover;
  display: block;
}

.stream-placeholder {
  display: flex;
  align-items: center;
  justify-content: center;
  height: 100%;
  color: #6c757d;
}

.placeholder-content {
  text-align: center;
}

.placeholder-icon {
  font-size: 48px;
  margin-bottom: 10px;
  opacity: 0.5;
}

/* Tespit Overlay */
.detection-overlay {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  pointer-events: none;
}

.detection-box {
  position: absolute;
  border: 2px solid #28a745;
  background: rgba(40, 167, 69, 0.1);
  pointer-events: all;
  cursor: pointer;
  transition: all 0.3s;
}

.detection-box:hover {
  background: rgba(40, 167, 69, 0.2);
}

.detection-label {
  position: absolute;
  top: -25px;
  left: 0;
  background: rgba(0, 0, 0, 0.8);
  color: white;
  padding: 2px 8px;
  font-size: 11px;
  border-radius: 4px;
  white-space: nowrap;
}

.equipment-indicators {
  position: absolute;
  bottom: -20px;
  left: 0;
  display: flex;
  gap: 4px;
}

.equipment-icon {
  font-size: 12px;
}

.equipment-icon.ok {
  filter: brightness(1.2);
}

.equipment-icon.missing {
  filter: grayscale(1) brightness(0.7);
}

/* Kamera Footer */
.camera-footer {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 15px;
  background: #f8f9fa;
  border-top: 1px solid #e9ecef;
}

.camera-metrics {
  display: flex;
  gap: 15px;
}

.metric {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 2px;
}

.metric-label {
  font-size: 11px;
  color: #6c757d;
}

.metric-value {
  font-size: 14px;
  font-weight: bold;
  color: #2c3e50;
}

.violation-count.has-violations {
  color: #dc3545;
}

.camera-actions {
  display: flex;
  gap: 8px;
}

.action-btn {
  padding: 6px 10px;
  border: none;
  background: #e9ecef;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
  transition: all 0.3s;
}

.action-btn:hover {
  background: #dee2e6;
}

/* Odaklanmış Görünüm */
.focused-view {
  padding: 20px;
}

.focused-camera {
  display: grid;
  grid-template-columns: 1fr 300px;
  gap: 20px;
  height: calc(100vh - 200px);
}

.focused-header {
  grid-column: 1 / -1;
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}

.focused-header h2 {
  margin: 0;
  color: #2c3e50;
}

.focused-controls {
  display: flex;
  gap: 10px;
}

.focused-stream {
  background: #000;
  border-radius: 8px;
  overflow: hidden;
  position: relative;
}

.focused-stream-image {
  width: 100%;
  height: 100%;
  object-fit: contain;
}

.focused-placeholder {
  display: flex;
  align-items: center;
  justify-content: center;
  height: 100%;
  color: #6c757d;
}

.focused-sidebar {
  background: #f8f9fa;
  border-radius: 8px;
  padding: 20px;
  overflow-y: auto;
}

/* Tespit Detayları */
.detection-details h4 {
  margin: 0 0 15px 0;
  color: #2c3e50;
}

.detections-list {
  display: flex;
  flex-direction: column;
  gap: 15px;
}

.detection-item {
  background: white;
  padding: 15px;
  border-radius: 8px;
  border: 1px solid #e9ecef;
}

.detection-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 10px;
}

.person-id {
  font-weight: 500;
  color: #2c3e50;
}

.detection-time {
  font-size: 12px;
  color: #6c757d;
}

.equipment-status {
  display: flex;
  flex-direction: column;
  gap: 8px;
  margin-bottom: 10px;
}

.equipment-item {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 14px;
}

.equipment-item.violation {
  color: #dc3545;
}

.detection-confidence {
  font-size: 12px;
  color: #6c757d;
  text-align: right;
}

/* Liste Görünümü */
.list-view {
  padding: 20px;
}

.cameras-list {
  display: flex;
  flex-direction: column;
  gap: 15px;
}

.camera-list-item {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 20px;
  background: white;
  border: 1px solid #e9ecef;
  border-radius: 8px;
  transition: all 0.3s;
}

.camera-list-item:hover {
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
}

.camera-list-item.has-violations {
  border-left: 4px solid #dc3545;
}

.list-item-info h4 {
  margin: 0 0 5px 0;
  color: #2c3e50;
}

.list-item-info p {
  margin: 0 0 10px 0;
  color: #6c757d;
  font-size: 14px;
}

.list-item-status {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 12px;
}

.list-item-metrics {
  display: flex;
  gap: 20px;
}

.list-item-actions {
  display: flex;
  gap: 10px;
}

/* Son Tespitler */
.recent-detections {
  background: white;
  border-radius: 12px;
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
  margin-bottom: 20px;
}

.detections-timeline {
  padding: 20px;
  max-height: 400px;
  overflow-y: auto;
}

.timeline-item {
  display: flex;
  gap: 15px;
  padding: 15px 0;
  border-bottom: 1px solid #e9ecef;
}

.timeline-item:last-child {
  border-bottom: none;
}

.timeline-item.has-violation {
  background: rgba(220, 53, 69, 0.05);
  margin: 0 -20px;
  padding: 15px 20px;
  border-radius: 4px;
}

.timeline-time {
  font-size: 12px;
  color: #6c757d;
  min-width: 60px;
}

.timeline-content {
  flex: 1;
}

.detection-info {
  display: flex;
  gap: 15px;
  margin-bottom: 8px;
}

.camera-name, .person-id {
  font-size: 14px;
  font-weight: 500;
}

.equipment-summary {
  display: flex;
  gap: 10px;
  margin-bottom: 8px;
}

.equipment-status.ok {
  color: #28a745;
}

.equipment-status.violation {
  color: #dc3545;
}

.violations-list {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
}

.violation-tag {
  background: #f8d7da;
  color: #721c24;
  padding: 2px 8px;
  border-radius: 12px;
  font-size: 12px;
}

.no-detections {
  text-align: center;
  color: #6c757d;
  padding: 40px;
  font-style: italic;
}

/* Modal */
.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.5);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
}

.modal-content {
  background: white;
  border-radius: 12px;
  box-shadow: 0 4px 20px rgba(0,0,0,0.15);
  max-width: 500px;
  width: 90%;
  max-height: 80vh;
  overflow-y: auto;
}

.modal-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20px;
  border-bottom: 1px solid #e9ecef;
}

.modal-header h3 {
  margin: 0;
  color: #2c3e50;
}

.close-btn {
  background: none;
  border: none;
  font-size: 20px;
  cursor: pointer;
  color: #6c757d;
  padding: 0;
  width: 30px;
  height: 30px;
  display: flex;
  align-items: center;
  justify-content: center;
}

.close-btn:hover {
  color: #dc3545;
}

.modal-body {
  padding: 20px;
}

.setting-group {
  margin-bottom: 20px;
}

.setting-group label {
  display: block;
  margin-bottom: 8px;
  font-weight: 500;
  color: #495057;
}

.modal-footer {
  display: flex;
  justify-content: flex-end;
  gap: 10px;
  padding: 20px;
  border-top: 1px solid #e9ecef;
}

/* Bildirimler */
.notifications {
  position: fixed;
  top: 20px;
  right: 20px;
  z-index: 1100;
  display: flex;
  flex-direction: column;
  gap: 10px;
  max-width: 400px;
}

.notification {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 15px;
  border-radius: 8px;
  box-shadow: 0 4px 12px rgba(0,0,0,0.15);
  animation: slideIn 0.3s ease-out;
}

.notification.success {
  background: #d4edda;
  color: #155724;
  border-left: 4px solid #28a745;
}

.notification.error {
  background: #f8d7da;
  color: #721c24;
  border-left: 4px solid #dc3545;
}

.notification.warning {
  background: #fff3cd;
  color: #856404;
  border-left: 4px solid #ffc107;
}

.notification.info {
  background: #d1ecf1;
  color: #0c5460;
  border-left: 4px solid #17a2b8;
}

.notification-content {
  display: flex;
  align-items: center;
  gap: 10px;
  flex: 1;
}

.notification-close {
  background: none;
  border: none;
  font-size: 16px;
  cursor: pointer;
  color: currentColor;
  opacity: 0.7;
  padding: 0;
  margin-left: 10px;
}

.notification-close:hover {
  opacity: 1;
}

@keyframes slideIn {
  from {
    transform: translateX(100%);
    opacity: 0;
  }
  to {
    transform: translateX(0);
    opacity: 1;
  }
}

/* Responsive */
@media (max-width: 1200px) {
  .control-panel {
    grid-template-columns: 1fr;
  }
  
  .focused-camera {
    grid-template-columns: 1fr;
    grid-template-rows: auto 1fr auto;
  }
  
  .focused-sidebar {
    grid-row: 3;
    max-height: 300px;
  }
}

@media (max-width: 768px) {
  .monitoring-container {
    padding: 10px;
  }
  
  .page-header {
    flex-direction: column;
    gap: 15px;
    text-align: center;
  }
  
  .header-actions {
    flex-wrap: wrap;
    justify-content: center;
  }
  
  .controls-grid {
    grid-template-columns: 1fr;
  }
  
  .stats-grid {
    grid-template-columns: repeat(2, 1fr);
  }
  
  .cameras-grid {
    grid-template-columns: 1fr !important;
  }
  
  .section-header {
    flex-direction: column;
    gap: 15px;
  }
  
  .camera-filters {
    width: 100%;
  }
  
  .camera-filters .form-control {
    min-width: auto;
    flex: 1;
  }
  
  .notifications {
    left: 10px;
    right: 10px;
    max-width: none;
  }
}

@media (max-width: 480px) {
  .stats-grid {
    grid-template-columns: 1fr;
  }
  
  .camera-metrics {
    flex-direction: column;
    gap: 8px;
  }
  
  .metric {
    flex-direction: row;
    justify-content: space-between;
  }
  
  .camera-actions {
    flex-direction: column;
  }
}
</style>

