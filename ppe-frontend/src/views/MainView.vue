<template>
  <div class="main-container">
    <!-- Durum Çubuğu -->
    <div class="status-bar">
      <div class="status-item">
        <span class="status-label">🔗 Bağlantı:</span>
        <span :class="['status-value', connectionStatus]">
          {{ connectionStatusText }}
        </span>
      </div>
      <div class="status-item">
        <span class="status-label">👥 Aktif Çalışan:</span>
        <span class="status-value">{{ activeWorkerCount }}</span>
      </div>
      <div class="status-item">
        <span class="status-label">⚠️ Bugünkü İhlaller:</span>
        <span class="status-value">{{ todayViolations }}</span>
      </div>
    </div>

    <!-- Ana İçerik -->
    <div class="content-grid">
      <!-- Sol Panel - Kamera -->
      <div class="camera-panel">
        <div class="panel-header">
          <h3>📹 Kamera Görüntüsü</h3>
          </div>
        <PPEDetection @error="handleError" />
        </div>

      <!-- Sağ Panel - Bilgiler -->
      <div class="info-panel">
        <!-- Son Tespitler -->
        <div class="info-card">
          <h4>🎯 Son Tespitler</h4>
          <div class="detection-list">
            <div
              v-for="detection in recentDetections.slice(0, 5)"
              :key="detection.id"
              class="detection-item"
            >
              <div class="detection-time">
                {{ formatTime(detection.timestamp) }}
              </div>
              <div class="detection-details">
                <strong>ID:</strong>
                {{ detection.track_id || detection.worker_id }}<br />
                <span
                  :class="[
                    'ppe-status',
                    detection.has_helmet ? 'safe' : 'danger',
                  ]"
                >
                  {{ detection.has_helmet ? "✅ Baret Var" : "❌ Baret Yok" }}
                </span>
              </div>
            </div>
            <div v-if="recentDetections.length === 0" class="no-detections">
              <p>Henüz tespit yapılmadı</p>
            </div>
          </div>
        </div>

        <!-- İstatistikler -->
        <div class="info-card">
          <h4>📊 Günlük İstatistikler</h4>
          <div class="stats-grid">
            <div class="stat-item">
              <div class="stat-value">{{ dailyStats.totalDetections }}</div>
              <div class="stat-label">Toplam Tespit</div>
            </div>
            <div class="stat-item">
              <div class="stat-value safe">{{ dailyStats.safeDetections }}</div>
              <div class="stat-label">Güvenli</div>
            </div>
            <div class="stat-item">
              <div class="stat-value danger">{{ dailyStats.violations }}</div>
              <div class="stat-label">İhlal</div>
            </div>
            <div class="stat-item">
              <div class="stat-value">{{ dailyStats.complianceRate }}%</div>
              <div class="stat-label">Uyum Oranı</div>
            </div>
          </div>
        </div>

        <!-- Hızlı Eylemler -->
        <div class="info-card">
          <h4>⚡ Hızlı Eylemler</h4>
          <div class="action-buttons">
            <button @click="$router.push('/violations')" class="action-btn">
              ⚠️ İhlalleri Görüntüle
            </button>
            <button @click="$router.push('/workers')" class="action-btn">
              👥 Çalışan Listesi
            </button>
            <button @click="$router.push('/settings')" class="action-btn">
              ⚙️ Ayarlar
            </button>
          </div>
        </div>
      </div>
    </div>

    <!-- Hata Toast -->
    <div v-if="error" class="error-toast">
      {{ error }}
      <button @click="error = null">×</button>
    </div>
  </div>
</template>

<script>
import PPEDetection from '../components/PPEDetection.vue';

export default {
  name: "MainView",
  components: { PPEDetection },
  data() {
    return {
      API_BASE_URL: "http://localhost:5001/api",

      // Basit durum yönetimi
      connectionStatus: "connected",
      activeWorkerCount: 0,
      todayViolations: 0,
      recentDetections: [],
      error: null,

      // İstatistikler
      dailyStats: {
        totalDetections: 0,
        safeDetections: 0,
        violations: 0,
        complianceRate: 100,
      },

      // Interval'lar
      pollInterval: null,
    };
  },

  computed: {
    connectionStatusText() {
      switch (this.connectionStatus) {
        case "connected":
          return "Bağlı";
        case "connecting":
          return "Bağlanıyor...";
        case "error":
          return "Bağlantı Hatası";
        case "disconnected":
          return "Bağlantı Kesildi";
        default:
          return "Bilinmiyor";
      }
    },
  },

  mounted() {
    console.log("🚀 MainView yüklendi");
    this.loadDailyStats();
    this.startPolling();
  },

  beforeUnmount() {
    console.log("🛑 MainView kapatılıyor");
    this.stopPolling();
  },

  methods: {
    // Hata yönetimi
    handleError(message) {
      this.error = message;
      setTimeout(() => {
        this.error = null;
      }, 5000);
    },

    // Polling ve stats
    startPolling() {
      console.log("🔄 Polling başlatılıyor...");
      this.pollInterval = setInterval(async () => {
        await this.fetchStats();
      }, 5000);
    },

    stopPolling() {
      if (this.pollInterval) {
        clearInterval(this.pollInterval);
        this.pollInterval = null;
      }
    },

    async fetchStats() {
      try {
        const response = await fetch(`${this.API_BASE_URL}/ppe/stats`);
        const result = await response.json();

        if (result.success && result.data) {
          this.activeWorkerCount = result.data.active_workers || 0;
          this.todayViolations = result.data.today_violations || 0;

          // Günlük istatistikleri güncelle
          if (result.data.daily_stats) {
            this.dailyStats = {
              totalDetections: result.data.daily_stats.total_detections || 0,
              safeDetections: result.data.daily_stats.safe_detections || 0,
              violations: result.data.daily_stats.violations || 0,
              complianceRate: result.data.daily_stats.compliance_rate || 100,
            };
          }

          this.connectionStatus = "connected";
        }
      } catch (error) {
        console.error("❌ Stats fetch hatası:", error);
        this.connectionStatus = "error";
      }
    },

    async loadDailyStats() {
      try {
        const response = await fetch(`${this.API_BASE_URL}/ppe/daily-stats`);
        const result = await response.json();

        if (result.success && result.data) {
          this.dailyStats = result.data;
        }
      } catch (error) {
        console.error("❌ Daily stats yükleme hatası:", error);
      }
    },

    formatTime(timestamp) {
      if (!timestamp) return "--:--:--";
      return new Date(timestamp).toLocaleTimeString("tr-TR");
    },
  },
};
</script>

<style scoped>
/* Ana Container ve Layout */
.main-container {
  padding: 0.5rem;
  background: #f8f9fa;
  min-height: 100vh;
  width: 100vw;
  box-sizing: border-box;
}

.status-bar {
  background: white;
  padding: 0.75rem 1rem;
  border-radius: 8px;
  display: flex;
  gap: 2rem;
  margin-bottom: 0.5rem;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  flex-wrap: wrap;
}

.status-item {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  min-width: 150px;
}

.status-label {
  font-weight: 500;
  color: #6c757d;
  font-size: 0.9rem;
}

.status-value {
  font-weight: bold;
  font-size: 0.9rem;
}

.status-value.connected {
  color: #28a745;
}
.status-value.disconnected {
  color: #ffc107;
}
.status-value.error {
  color: #dc3545;
}

.content-grid {
  display: grid;
  grid-template-columns: 3fr 1fr;
  gap: 0.5rem;
  height: calc(100vh - 120px);
  max-width: 100vw;
}

.camera-panel,
.info-panel {
  background: white;
  border-radius: 8px;
  padding: 0.75rem;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  overflow: hidden;
}

.camera-panel {
  display: flex;
  flex-direction: column;
  min-height: 0;
}

.panel-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 0.5rem;
  padding-bottom: 0.5rem;
  border-bottom: 2px solid #e9ecef;
}

.panel-header h3 {
  margin: 0;
  font-size: 1.1rem;
  color: #495057;
}

/* Info Panel Stilleri */
.info-panel {
  display: flex;
  flex-direction: column;
  gap: 1rem;
  overflow-y: auto;
}

.info-card {
  background: #f8f9fa;
  padding: 1rem;
  border-radius: 8px;
  border: 1px solid #e9ecef;
}

.info-card h4 {
  margin-bottom: 1rem;
  color: #495057;
}

.detection-list {
  max-height: 200px;
  overflow-y: auto;
}

.detection-item {
  display: flex;
  gap: 1rem;
  padding: 0.5rem;
  border-bottom: 1px solid #e9ecef;
}

.detection-item:last-child {
  border-bottom: none;
}

.detection-time {
  font-size: 0.8rem;
  color: #6c757d;
  white-space: nowrap;
}

.no-detections {
  text-align: center;
  color: #6c757d;
  padding: 2rem;
  font-style: italic;
}

.ppe-status.safe {
  color: #28a745;
}
.ppe-status.danger {
  color: #dc3545;
}

.stats-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 1rem;
}

.stat-item {
  text-align: center;
  padding: 1rem;
  background: white;
  border-radius: 8px;
  border: 1px solid #e9ecef;
}

.stat-value {
  font-size: 1.5rem;
  font-weight: bold;
  color: #495057;
}

.stat-value.safe {
  color: #28a745;
}
.stat-value.danger {
  color: #dc3545;
}

.stat-label {
  font-size: 0.8rem;
  color: #6c757d;
  margin-top: 0.25rem;
}

.action-buttons {
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
}

.action-btn {
  padding: 0.75rem;
  background: #007bff;
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  transition: background 0.3s;
}

.action-btn:hover {
  background: #0056b3;
}

/* Hata Toast */
.error-toast {
  position: fixed;
  top: 20px;
  right: 20px;
  background: #dc3545;
  color: white;
  padding: 1rem;
  border-radius: 8px;
  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
  z-index: 1000;
  display: flex;
  align-items: center;
  gap: 1rem;
}

.error-toast button {
  background: none;
  border: none;
  color: white;
  font-size: 1.2rem;
  cursor: pointer;
  padding: 0;
  width: 24px;
  height: 24px;
  display: flex;
  align-items: center;
  justify-content: center;
}

/* Responsive Tasarım */
@media (max-width: 1024px) {
  .content-grid {
    grid-template-columns: 2fr 1fr;
    gap: 0.5rem;
  }
  
  .status-bar {
    gap: 1rem;
  }
  
  .status-item {
    min-width: 120px;
  }
}

@media (max-width: 768px) {
  .main-container {
    padding: 0.25rem;
  }
  
  .content-grid {
    grid-template-columns: 1fr;
    height: auto;
    gap: 0.5rem;
  }

  .camera-panel {
    min-height: 50vh;
  }

  .status-bar {
    flex-direction: column;
    gap: 0.5rem;
    padding: 0.5rem;
  }
  
  .status-item {
    min-width: auto;
    justify-content: center;
  }

  .stats-grid {
    grid-template-columns: 1fr 1fr;
    gap: 0.5rem;
  }
  
  .info-card {
    padding: 0.75rem;
  }
}

@media (max-width: 480px) {
  .main-container {
    padding: 0.125rem;
  }

  .panel-header {
    flex-direction: column;
    gap: 0.5rem;
    align-items: stretch;
  }

  .panel-header h3 {
    font-size: 1rem;
    text-align: center;
  }
  
  .stats-grid {
    grid-template-columns: 1fr;
  }
  
  .status-bar {
    padding: 0.25rem;
  }
  
  .status-label,
  .status-value {
    font-size: 0.8rem;
  }
  
  .camera-panel,
  .info-panel {
    padding: 0.5rem;
  }
}

/* Scroll Bar Stilleri */
.detection-list::-webkit-scrollbar,
.info-panel::-webkit-scrollbar {
  width: 6px;
}

.detection-list::-webkit-scrollbar-track,
.info-panel::-webkit-scrollbar-track {
  background: #f1f1f1;
  border-radius: 3px;
}

.detection-list::-webkit-scrollbar-thumb,
.info-panel::-webkit-scrollbar-thumb {
  background: #c1c1c1;
  border-radius: 3px;
}

.detection-list::-webkit-scrollbar-thumb:hover,
.info-panel::-webkit-scrollbar-thumb:hover {
  background: #a8a8a8;
}
</style>
