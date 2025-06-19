<template>
  <div class="dashboard-container">
    <!-- Sayfa Başlığı -->
    <div class="page-header">
      <div class="header-content">
        <h1>📊 Dashboard</h1>
        <p>PPE Güvenlik Sistemi - Genel Durum ve İstatistikler</p>
      </div>
      <div class="header-actions">
        <div class="date-range-selector">
          <label>📅 Tarih Aralığı:</label>
          <select v-model="selectedPeriod" @change="loadDashboardData" class="period-select">
            <option value="today">Bugün</option>
            <option value="week">Bu Hafta</option>
            <option value="month">Bu Ay</option>
            <option value="quarter">Bu Çeyrek</option>
            <option value="year">Bu Yıl</option>
            <option value="custom">Özel Aralık</option>
          </select>
        </div>
        <button @click="refreshDashboard" class="btn btn-primary" :disabled="isLoading">
          <span v-if="isLoading">🔄 Yenileniyor...</span>
          <span v-else>🔄 Yenile</span>
        </button>
      </div>
    </div>

    <!-- Özel Tarih Aralığı -->
    <div v-if="selectedPeriod === 'custom'" class="custom-date-range">
      <div class="date-inputs">
        <div class="date-group">
          <label>Başlangıç:</label>
          <input v-model="customDateRange.start" type="date" class="form-control">
        </div>
        <div class="date-group">
          <label>Bitiş:</label>
          <input v-model="customDateRange.end" type="date" class="form-control">
        </div>
        <button @click="loadDashboardData" class="btn btn-primary">Uygula</button>
      </div>
    </div>

    <!-- Ana Metrikler -->
    <div class="main-metrics">
      <div class="metric-card primary">
        <div class="metric-icon">👥</div>
        <div class="metric-content">
          <div class="metric-value">{{ dashboardData.totalWorkers || 0 }}</div>
          <div class="metric-label">Toplam Çalışan</div>
          <div class="metric-change" :class="getChangeClass(dashboardData.workersChange)">
            {{ formatChange(dashboardData.workersChange) }}
          </div>
        </div>
      </div>

      <div class="metric-card success">
        <div class="metric-icon">✅</div>
        <div class="metric-content">
          <div class="metric-value">{{ dashboardData.activeWorkers || 0 }}</div>
          <div class="metric-label">Aktif Çalışan</div>
          <div class="metric-change" :class="getChangeClass(dashboardData.activeWorkersChange)">
            {{ formatChange(dashboardData.activeWorkersChange) }}
          </div>
        </div>
      </div>

      <div class="metric-card warning">
        <div class="metric-icon">⚠️</div>
        <div class="metric-content">
          <div class="metric-value">{{ dashboardData.totalViolations || 0 }}</div>
          <div class="metric-label">Toplam İhlal</div>
          <div class="metric-change" :class="getChangeClass(dashboardData.violationsChange, true)">
            {{ formatChange(dashboardData.violationsChange) }}
          </div>
        </div>
      </div>

      <div class="metric-card info">
        <div class="metric-icon">📈</div>
        <div class="metric-content">
          <div class="metric-value">{{ dashboardData.complianceRate || 0 }}%</div>
          <div class="metric-label">Genel Uyum Oranı</div>
          <div class="metric-change" :class="getChangeClass(dashboardData.complianceChange)">
            {{ formatChange(dashboardData.complianceChange) }}%
          </div>
        </div>
      </div>
    </div>

    <!-- Grafik ve İstatistik Bölümü -->
    <div class="charts-section">
      <!-- Sol Kolon -->
      <div class="charts-left">
        <!-- İhlal Trend Grafiği -->
        <div class="chart-card">
          <div class="chart-header">
            <h3>📈 İhlal Trend Analizi</h3>
            <div class="chart-controls">
              <select v-model="violationChartType" @change="updateViolationChart" class="chart-select">
                <option value="daily">Günlük</option>
                <option value="weekly">Haftalık</option>
                <option value="monthly">Aylık</option>
              </select>
            </div>
          </div>
          <div class="chart-container">
            <canvas ref="violationTrendChart" class="chart-canvas"></canvas>
          </div>
        </div>

        <!-- Departman İstatistikleri -->
        <div class="chart-card">
          <div class="chart-header">
            <h3>🏢 Departman Bazlı İstatistikler</h3>
          </div>
          <div class="department-stats">
            <div 
              v-for="dept in dashboardData.departmentStats" 
              :key="dept.name"
              class="department-item"
            >
              <div class="dept-info">
                <div class="dept-name">{{ dept.name }}</div>
                <div class="dept-workers">{{ dept.workerCount }} çalışan</div>
              </div>
              <div class="dept-metrics">
                <div class="dept-compliance">
                  <div class="compliance-bar">
                    <div 
                      class="compliance-fill" 
                      :style="{ width: dept.complianceRate + '%' }"
                      :class="getComplianceClass(dept.complianceRate)"
                    ></div>
                    <span class="compliance-text">{{ dept.complianceRate }}%</span>
                  </div>
                </div>
                <div class="dept-violations">
                  <span class="violation-count" :class="{ 'has-violations': dept.violations > 0 }">
                    {{ dept.violations }} ihlal
                  </span>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- Sağ Kolon -->
      <div class="charts-right">
        <!-- İhlal Türleri Dağılımı -->
        <div class="chart-card">
          <div class="chart-header">
            <h3>🎯 İhlal Türleri Dağılımı</h3>
          </div>
          <div class="chart-container">
            <canvas ref="violationTypesChart" class="chart-canvas"></canvas>
          </div>
          <div class="violation-types-legend">
            <div 
              v-for="(type, index) in dashboardData.violationTypes" 
              :key="type.name"
              class="legend-item"
            >
              <div class="legend-color" :style="{ backgroundColor: getViolationTypeColor(index) }"></div>
              <div class="legend-info">
                <div class="legend-name">{{ type.name }}</div>
                <div class="legend-count">{{ type.count }} ({{ type.percentage }}%)</div>
              </div>
            </div>
          </div>
        </div>

        <!-- Günlük Özet -->
        <div class="chart-card">
          <div class="chart-header">
            <h3>📋 Günlük Özet</h3>
            <div class="summary-date">{{ formatDate(new Date()) }}</div>
          </div>
          <div class="daily-summary">
            <div class="summary-item">
              <div class="summary-icon">🔍</div>
              <div class="summary-content">
                <div class="summary-value">{{ dashboardData.todayDetections || 0 }}</div>
                <div class="summary-label">Bugünkü Tespit</div>
              </div>
            </div>
            
            <div class="summary-item">
              <div class="summary-icon">⚠️</div>
              <div class="summary-content">
                <div class="summary-value">{{ dashboardData.todayViolations || 0 }}</div>
                <div class="summary-label">Bugünkü İhlal</div>
              </div>
            </div>
            
            <div class="summary-item">
              <div class="summary-icon">👤</div>
              <div class="summary-content">
                <div class="summary-value">{{ dashboardData.activeWorkersToday || 0 }}</div>
                <div class="summary-label">Bugün Aktif</div>
              </div>
            </div>
            
            <div class="summary-item">
              <div class="summary-icon">📊</div>
              <div class="summary-content">
                <div class="summary-value">{{ dashboardData.todayComplianceRate || 0 }}%</div>
                <div class="summary-label">Bugünkü Uyum</div>
              </div>
            </div>
          </div>
        </div>

        <!-- Hızlı Eylemler -->
        <div class="chart-card">
          <div class="chart-header">
            <h3>⚡ Hızlı Eylemler</h3>
          </div>
          <div class="quick-actions">
            <button @click="$router.push('/workers')" class="action-card">
              <div class="action-icon">👥</div>
              <div class="action-content">
                <div class="action-title">Çalışan Yönetimi</div>
                <div class="action-desc">Çalışanları görüntüle ve yönet</div>
              </div>
            </button>
            
            <button @click="$router.push('/violations')" class="action-card">
              <div class="action-icon">⚠️</div>
              <div class="action-content">
                <div class="action-title">İhlal Raporları</div>
                <div class="action-desc">Tüm ihlalleri incele</div>
              </div>
            </button>
            
            <button @click="$router.push('/monitoring')" class="action-card">
              <div class="action-icon">📹</div>
              <div class="action-content">
                <div class="action-title">Canlı İzleme</div>
                <div class="action-desc">Anlık kamera görüntüleri</div>
              </div>
            </button>
            
            <button @click="exportReport" class="action-card">
              <div class="action-icon">📊</div>
              <div class="action-content">
                <div class="action-title">Rapor Oluştur</div>
                <div class="action-desc">Detaylı analiz raporu</div>
              </div>
            </button>
          </div>
        </div>
      </div>
    </div>

    <!-- Son İhlaller ve Aktiviteler -->
    <div class="bottom-section">
      <!-- Son İhlaller -->
      <div class="recent-violations">
        <div class="section-header">
          <h3>⚠️ Son İhlaller</h3>
          <button @click="$router.push('/violations')" class="view-all-btn">
            Tümünü Görüntüle →
          </button>
        </div>
        
        <div class="violations-list">
          <div v-if="isLoading" class="loading-state">
            <div class="loading-spinner">🔄 Yükleniyor...</div>
          </div>
          
          <div v-else-if="dashboardData.recentViolations?.length === 0" class="empty-state">
            <div class="empty-icon">✅</div>
            <p>Harika! Son zamanlarda ihlal tespit edilmedi.</p>
          </div>
          
          <div 
            v-else
            v-for="violation in dashboardData.recentViolations" 
            :key="violation.id"
            class="violation-item"
            @click="viewViolationDetails(violation)"
          >
            <div class="violation-time">
              {{ formatRelativeTime(violation.timestamp) }}
            </div>
            <div class="violation-content">
              <div class="violation-worker">
                <strong>{{ violation.worker_name }}</strong>
                <span class="worker-id">({{ violation.worker_id }})</span>
              </div>
              <div class="violation-details">
                <div class="violation-types">
                  <span 
                    v-for="type in violation.violation_types" 
                    :key="type"
                    class="violation-badge"
                    :class="getViolationBadgeClass(type)"
                  >
                    {{ type }}
                  </span>
                </div>
                <div class="violation-confidence">
                  Güven: {{ violation.confidence }}%
                </div>
              </div>
            </div>
            <div class="violation-actions">
              <button class="view-btn" title="Detayları Görüntüle">👁️</button>
            </div>
          </div>
        </div>
      </div>

      <!-- Sistem Durumu -->
      <div class="system-status">
        <div class="section-header">
          <h3>🖥️ Sistem Durumu</h3>
        </div>
        
        <div class="status-grid">
          <div class="status-item">
            <div class="status-indicator" :class="dashboardData.systemStatus?.camera || 'offline'"></div>
            <div class="status-content">
              <div class="status-title">Kamera Sistemi</div>
              <div class="status-desc">
                {{ dashboardData.systemStatus?.cameraCount || 0 }} kamera aktif
              </div>
            </div>
          </div>
          
          <div class="status-item">
            <div class="status-indicator" :class="dashboardData.systemStatus?.ai || 'offline'"></div>
            <div class="status-content">
              <div class="status-title">AI Analiz Motoru</div>
              <div class="status-desc">
                {{ dashboardData.systemStatus?.aiProcessingTime || 0 }}ms ortalama
              </div>
            </div>
          </div>
          
          <div class="status-item">
            <div class="status-indicator" :class="dashboardData.systemStatus?.database || 'offline'"></div>
            <div class="status-content">
              <div class="status-title">Veritabanı</div>
              <div class="status-desc">
                {{ dashboardData.systemStatus?.dbSize || 0 }}MB kullanım
              </div>
            </div>
          </div>
          
          <div class="status-item">
            <div class="status-indicator" :class="dashboardData.systemStatus?.storage || 'offline'"></div>
            <div class="status-content">
              <div class="status-title">Depolama</div>
              <div class="status-desc">
                {{ dashboardData.systemStatus?.storageUsed || 0 }}% dolu
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- Başarı/Hata Mesajları -->
    <div v-if="message" :class="['message', messageType]">
      {{ message }}
    </div>
  </div>
</template>

<script>
import Chart from 'chart.js/auto'

export default {
  name: 'DashboardView',
  data() {
    return {
      isLoading: false,
      selectedPeriod: 'week',
      customDateRange: {
        start: '',
        end: ''
      },
      violationChartType: 'daily',
      
      dashboardData: {
        totalWorkers: 0,
        activeWorkers: 0,
        totalViolations: 0,
        complianceRate: 0,
        workersChange: 0,
        activeWorkersChange: 0,
        violationsChange: 0,
        complianceChange: 0,
        todayDetections: 0,
        todayViolations: 0,
        activeWorkersToday: 0,
        todayComplianceRate: 0,
        departmentStats: [],
        violationTypes: [],
        recentViolations: [],
        violationTrend: [],
        systemStatus: {
          camera: 'online',
          ai: 'online',
          database: 'online',
          storage: 'online',
          cameraCount: 0,
          aiProcessingTime: 0,
          dbSize: 0,
          storageUsed: 0
        }
      },
      
      charts: {
        violationTrend: null,
        violationTypes: null
      },
      
      message: '',
      messageType: 'success'
    }
  },
  
  mounted() {
    this.loadDashboardData()
    this.setupAutoRefresh()
  },
  
  beforeUnmount() {
    // Chart.js nesnelerini temizle
    Object.values(this.charts).forEach(chart => {
      if (chart) chart.destroy()
    })
    
    // Auto refresh'i temizle
    if (this.refreshInterval) {
      clearInterval(this.refreshInterval)
    }
  },
  
  methods: {
    async loadDashboardData() {
      this.isLoading = true
      
      try {
        const params = new URLSearchParams()
        params.append('period', this.selectedPeriod)
        
        if (this.selectedPeriod === 'custom') {
          params.append('start_date', this.customDateRange.start)
          params.append('end_date', this.customDateRange.end)
        }
        
        const response = await fetch(`http://localhost:5001/dashboard?${params}`)
        
        if (response.ok) {
          this.dashboardData = await response.json()
          this.$nextTick(() => {
            this.initializeCharts()
          })
        } else {
          this.showMessage('Dashboard verileri yüklenemedi! ❌', 'error')
        }
      } catch (error) {
        console.error('Dashboard yükleme hatası:', error)
        this.showMessage('Bağlantı hatası! 🔄', 'error')
      } finally {
        this.isLoading = false
      }
    },
    
    async refreshDashboard() {
      await this.loadDashboardData()
      this.showMessage('Dashboard güncellendi! ✅', 'success')
    },
    
    setupAutoRefresh() {
      // Her 5 dakikada bir otomatik yenile
      this.refreshInterval = setInterval(() => {
        this.loadDashboardData()
      }, 5 * 60 * 1000)
    },
    
    initializeCharts() {
      this.createViolationTrendChart()
      this.createViolationTypesChart()
    },
    
    createViolationTrendChart() {
      const ctx = this.$refs.violationTrendChart
      if (!ctx) return
      
      // Önceki chart'ı temizle
      if (this.charts.violationTrend) {
        this.charts.violationTrend.destroy()
      }
      
      const data = this.dashboardData.violationTrend || []
      
      this.charts.violationTrend = new Chart(ctx, {
        type: 'line',
        data: {
          labels: data.map(item => this.formatChartDate(item.date)),
          datasets: [{
            label: 'İhlal Sayısı',
            data: data.map(item => item.violations),
            borderColor: '#dc3545',
            backgroundColor: 'rgba(220, 53, 69, 0.1)',
            tension: 0.4,
            fill: true
          }, {
            label: 'Tespit Sayısı',
            data: data.map(item => item.detections),
            borderColor: '#007bff',
            backgroundColor: 'rgba(0, 123, 255, 0.1)',
            tension: 0.4,
            fill: false
          }]
        },
        options: {
          responsive: true,
          maintainAspectRatio: false,
          plugins: {
            legend: {
              position: 'top'
            }
          },
          scales: {
            y: {
              beginAtZero: true,
              ticks: {
                precision: 0
              }
            }
          }
        }
      })
    },
    
    createViolationTypesChart() {
      const ctx = this.$refs.violationTypesChart
      if (!ctx) return
      
      // Önceki chart'ı temizle
      if (this.charts.violationTypes) {
        this.charts.violationTypes.destroy()
      }
      
      const data = this.dashboardData.violationTypes || []
      
      if (data.length === 0) return
      
      this.charts.violationTypes = new Chart(ctx, {
        type: 'doughnut',
        data: {
          labels: data.map(item => item.name),
          datasets: [{
            data: data.map(item => item.count),
            backgroundColor: data.map((_, index) => this.getViolationTypeColor(index)),
            borderWidth: 2,
            borderColor: '#fff'
          }]
        },
        options: {
          responsive: true,
          maintainAspectRatio: false,
          plugins: {
            legend: {
              display: false
            }
          }
        }
      })
    },
    
    updateViolationChart() {
      this.loadDashboardData()
    },
    
    async exportReport() {
      try {
        const params = new URLSearchParams()
        params.append('period', this.selectedPeriod)
        
        if (this.selectedPeriod === 'custom') {
          params.append('start_date', this.customDateRange.start)
          params.append('end_date', this.customDateRange.end)
        }
        
        const response = await fetch(`http://localhost:5001/reports/dashboard?${params}`, {
          method: 'POST'
        })
        
        if (response.ok) {
          const blob = await response.blob()
          const url = window.URL.createObjectURL(blob)
          const link = document.createElement('a')
          link.href = url
          link.download = `dashboard_raporu_${new Date().toISOString().split('T')[0]}.pdf`
          link.click()
          window.URL.revokeObjectURL(url)
          this.showMessage('Rapor başarıyla oluşturuldu! 📊', 'success')
        }
      } catch (error) {
        console.error('Rapor oluşturma hatası:', error)
        this.showMessage('Rapor oluşturulamadı! ❌', 'error')
      }
    },
    
    viewViolationDetails(violation) {
      this.$router.push(`/violations/${violation.id}`)
    },
    
    // Yardımcı fonksiyonlar
    getChangeClass(change, reverse = false) {
      if (change === 0) return 'neutral'
      if (reverse) {
        return change > 0 ? 'negative' : 'positive'
      }
      return change > 0 ? 'positive' : 'negative'
    },
    
    formatChange(change) {
      if (change === 0) return '0'
      const sign = change > 0 ? '+' : ''
      return `${sign}${change}`
    },
    
    getComplianceClass(rate) {
      if (rate >= 90) return 'excellent'
      if (rate >= 75) return 'good'
      if (rate >= 50) return 'average'
      return 'poor'
    },
    
    getViolationTypeColor(index) {
      const colors = [
        '#dc3545', '#ffc107', '#fd7e14', '#20c997',
        '#6f42c1', '#e83e8c', '#17a2b8', '#6c757d'
      ]
      return colors[index % colors.length]
    },
    
    getViolationBadgeClass(type) {
      const typeMap = {
        'Baret': 'danger',
        'Maske': 'warning',
        'Eldiven': 'info',
        'Yelek': 'danger'
      }
      return typeMap[type] || 'secondary'
    },
    
    formatDate(date) {
      return new Date(date).toLocaleDateString('tr-TR')
    },
    
    formatChartDate(dateString) {
      const date = new Date(dateString)
      if (this.violationChartType === 'daily') {
        return date.toLocaleDateString('tr-TR', { day: '2-digit', month: '2-digit' })
      } else if (this.violationChartType === 'weekly') {
        return `${date.getDate()}/${date.getMonth() + 1}`
      } else {
        return date.toLocaleDateString('tr-TR', { month: 'short', year: '2-digit' })
      }
    },
    
    formatRelativeTime(timestamp) {
      const now = Date.now()
      const time = timestamp * 1000
      const diff = now - time
      
      const minutes = Math.floor(diff / 60000)
      const hours = Math.floor(diff / 3600000)
      const days = Math.floor(diff / 86400000)
      
      if (minutes < 1) return 'Az önce'
      if (minutes < 60) return `${minutes} dakika önce`
      if (hours < 24) return `${hours} saat önce`
      if (days < 30) return `${days} gün önce`
      return this.formatDate(new Date(time))
    },
    
    showMessage(text, type) {
      this.message = text
      this.messageType = type
      setTimeout(() => {
        this.message = ''
      }, 3000)
    }
  }
}
</script>

<style scoped>
.dashboard-container {
  padding: 2rem;
  background: #f8f9fa;
  min-height: 100vh;
}

.page-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 2rem;
  background: white;
  padding: 1.5rem;
  border-radius: 12px;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
}

.header-content h1 {
  color: #2c3e50;
  margin-bottom: 0.5rem;
}

.header-content p {
  color: #6c757d;
  margin: 0;
}

.header-actions {
  display: flex;
  align-items: center;
  gap: 1rem;
}

.date-range-selector {
  display: flex;
  align-items: center;
  gap: 0.5rem;
}

.date-range-selector label {
  color: #495057;
  font-weight: 600;
}

.period-select {
  padding: 0.5rem;
  border: 2px solid #e9ecef;
  border-radius: 6px;
  font-size: 0.9rem;
}

.custom-date-range {
  background: white;
  padding: 1rem 1.5rem;
  border-radius: 12px;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
  margin-bottom: 2rem;
}

.date-inputs {
  display: flex;
  align-items: end;
  gap: 1rem;
}

.date-group {
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
}

.date-group label {
  font-weight: 600;
  color: #495057;
}

.form-control {
  padding: 0.5rem;
  border: 2px solid #e9ecef;
  border-radius: 6px;
  font-size: 0.9rem;
}

.main-metrics {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
  gap: 1.5rem;
  margin-bottom: 2rem;
}

.metric-card {
  background: white;
  padding: 1.5rem;
  border-radius: 12px;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
  display: flex;
  align-items: center;
  gap: 1rem;
  position: relative;
  overflow: hidden;
}

.metric-card::before {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  width: 4px;
  height: 100%;
}

.metric-card.primary::before { background: #007bff; }
.metric-card.success::before { background: #28a745; }
.metric-card.warning::before { background: #ffc107; }
.metric-card.info::before { background: #17a2b8; }

.metric-icon {
  font-size: 2.5rem;
  opacity: 0.8;
}

.metric-content {
  flex: 1;
}

.metric-value {
  font-size: 2rem;
  font-weight: bold;
  color: #2c3e50;
  margin-bottom: 0.25rem;
}

.metric-label {
  color: #6c757d;
  font-size: 0.9rem;
  margin-bottom: 0.5rem;
}

.metric-change {
  font-size: 0.8rem;
  font-weight: 600;
  padding: 0.25rem 0.5rem;
  border-radius: 12px;
  display: inline-block;
}

.metric-change.positive {
  background: #d4edda;
  color: #155724;
}

.metric-change.negative {
  background: #f8d7da;
  color: #721c24;
}

.metric-change.neutral {
  background: #e2e3e5;
  color: #383d41;
}

.charts-section {
  display: grid;
  grid-template-columns: 2fr 1fr;
  gap: 2rem;
  margin-bottom: 2rem;
}

.charts-left,
.charts-right {
  display: flex;
  flex-direction: column;
  gap: 1.5rem;
}

.chart-card {
  background: white;
  border-radius: 12px;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
  overflow: hidden;
}

.chart-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 1.5rem;
  border-bottom: 1px solid #e9ecef;
  background: #f8f9fa;
}

.chart-header h3 {
  margin: 0;
  color: #2c3e50;
}

.chart-controls {
  display: flex;
  gap: 0.5rem;
}

.chart-select {
  padding: 0.25rem 0.5rem;
  border: 1px solid #dee2e6;
  border-radius: 4px;
  font-size: 0.8rem;
}

.summary-date {
  color: #6c757d;
  font-size: 0.9rem;
}

.chart-container {
  padding: 1.5rem;
  height: 300px;
  position: relative;
}

.chart-canvas {
  max-height: 100%;
}

/* Departman İstatistikleri */
.department-stats {
  padding: 1.5rem;
}

.department-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 1rem;
  border: 1px solid #e9ecef;
  border-radius: 8px;
  margin-bottom: 1rem;
  transition: all 0.3s;
}

.department-item:hover {
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
  transform: translateY(-1px);
}

.dept-info {
  flex: 1;
}

.dept-name {
  font-weight: 600;
  color: #2c3e50;
  margin-bottom: 0.25rem;
}

.dept-workers {
  color: #6c757d;
  font-size: 0.8rem;
}

.dept-metrics {
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
  min-width: 120px;
}

.dept-compliance {
  display: flex;
  align-items: center;
}

.compliance-bar {
  position: relative;
  background: #e9ecef;
  border-radius: 10px;
  height: 16px;
  overflow: hidden;
  flex: 1;
}

.compliance-fill {
  height: 100%;
  transition: width 0.3s;
  border-radius: 10px;
}

.compliance-fill.excellent {
  background: linear-gradient(90deg, #28a745, #20c997);
}

.compliance-fill.good {
  background: linear-gradient(90deg, #17a2b8, #6f42c1);
}

.compliance-fill.average {
  background: linear-gradient(90deg, #ffc107, #fd7e14);
}

.compliance-fill.poor {
  background: linear-gradient(90deg, #dc3545, #e83e8c);
}

.compliance-text {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  font-size: 0.7rem;
  font-weight: bold;
  color: white;
  text-shadow: 1px 1px 2px rgba(0,0,0,0.5);
}

.dept-violations {
  text-align: right;
}

.violation-count {
  font-size: 0.8rem;
  font-weight: 600;
  color: #28a745;
}

.violation-count.has-violations {
  color: #dc3545;
}

/* İhlal Türleri Legend */
.violation-types-legend {
  padding: 1rem 1.5rem;
  border-top: 1px solid #e9ecef;
}

.legend-item {
  display: flex;
  align-items: center;
  gap: 0.75rem;
  margin-bottom: 0.5rem;
}

.legend-color {
  width: 16px;
  height: 16px;
  border-radius: 4px;
}

.legend-info {
  flex: 1;
}

.legend-name {
  font-weight: 600;
  color: #2c3e50;
  font-size: 0.9rem;
}

.legend-count {
  color: #6c757d;
  font-size: 0.8rem;
}

/* Günlük Özet */
.daily-summary {
  padding: 1.5rem;
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 1rem;
}

.summary-item {
  display: flex;
  align-items: center;
  gap: 1rem;
  padding: 1rem;
  background: #f8f9fa;
  border-radius: 8px;
}

.summary-icon {
  font-size: 1.5rem;
}

.summary-content {
  flex: 1;
}

.summary-value {
  font-size: 1.2rem;
  font-weight: bold;
  color: #2c3e50;
  margin-bottom: 0.25rem;
}

.summary-label {
  color: #6c757d;
  font-size: 0.8rem;
}

/* Hızlı Eylemler */
.quick-actions {
  padding: 1.5rem;
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 1rem;
}

.action-card {
  display: flex;
  align-items: center;
  gap: 1rem;
  padding: 1rem;
  background: #f8f9fa;
  border: 1px solid #e9ecef;
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.3s;
  text-align: left;
}

.action-card:hover {
  background: #e9ecef;
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(0,0,0,0.15);
}

.action-icon {
  font-size: 1.5rem;
}

.action-content {
  flex: 1;
}

.action-title {
  font-weight: 600;
  color: #2c3e50;
  margin-bottom: 0.25rem;
}

.action-desc {
  color: #6c757d;
  font-size: 0.8rem;
}

/* Alt Bölüm */
.bottom-section {
  display: grid;
  grid-template-columns: 2fr 1fr;
  gap: 2rem;
}

.recent-violations,
.system-status {
  background: white;
  border-radius: 12px;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
  overflow: hidden;
}

.section-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 1.5rem;
  border-bottom: 1px solid #e9ecef;
  background: #f8f9fa;
}

.section-header h3 {
  margin: 0;
  color: #2c3e50;
}

.view-all-btn {
  background: none;
  border: none;
  color: #007bff;
  cursor: pointer;
  font-weight: 600;
  transition: color 0.3s;
}

.view-all-btn:hover {
  color: #0056b3;
}

/* Son İhlaller */
.violations-list {
  max-height: 400px;
  overflow-y: auto;
}

.violation-item {
  display: flex;
  align-items: center;
  gap: 1rem;
  padding: 1rem 1.5rem;
  border-bottom: 1px solid #e9ecef;
  cursor: pointer;
  transition: background 0.3s;
}

.violation-item:hover {
  background: #f8f9fa;
}

.violation-time {
  color: #6c757d;
  font-size: 0.8rem;
  min-width: 80px;
}

.violation-content {
  flex: 1;
}

.violation-worker {
  margin-bottom: 0.5rem;
}

.violation-worker strong {
  color: #2c3e50;
}

.worker-id {
  color: #6c757d;
  font-size: 0.9rem;
  margin-left: 0.5rem;
}

.violation-details {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.violation-types {
  display: flex;
  gap: 0.25rem;
  flex-wrap: wrap;
}

.violation-badge {
  padding: 0.125rem 0.5rem;
  border-radius: 12px;
  font-size: 0.7rem;
  font-weight: bold;
}

.violation-badge.danger {
  background: #f8d7da;
  color: #721c24;
}

.violation-badge.warning {
  background: #fff3cd;
  color: #856404;
}

.violation-badge.info {
  background: #d1ecf1;
  color: #0c5460;
}

.violation-badge.secondary {
  background: #e2e3e5;
  color: #383d41;
}

.violation-confidence {
  color: #6c757d;
  font-size: 0.8rem;
}

.violation-actions {
  display: flex;
  gap: 0.5rem;
}

.view-btn {
  background: none;
  border: 1px solid #dee2e6;
  padding: 0.25rem 0.5rem;
  border-radius: 4px;
  cursor: pointer;
  transition: all 0.3s;
}

.view-btn:hover {
  background: #f8f9fa;
  border-color: #007bff;
}

/* Sistem Durumu */
.status-grid {
  padding: 1.5rem;
  display: grid;
  gap: 1rem;
}

.status-item {
  display: flex;
  align-items: center;
  gap: 1rem;
  padding: 1rem;
  background: #f8f9fa;
  border-radius: 8px;
}

.status-indicator {
  width: 12px;
  height: 12px;
  border-radius: 50%;
  position: relative;
}

.status-indicator.online {
  background: #28a745;
  box-shadow: 0 0 0 3px rgba(40, 167, 69, 0.3);
}

.status-indicator.warning {
  background: #ffc107;
  box-shadow: 0 0 0 3px rgba(255, 193, 7, 0.3);
}

.status-indicator.offline {
  background: #dc3545;
  box-shadow: 0 0 0 3px rgba(220, 53, 69, 0.3);
}

.status-indicator.online::after {
  content: '';
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  width: 6px;
  height: 6px;
  background: white;
  border-radius: 50%;
}

.status-content {
  flex: 1;
}

.status-title {
  font-weight: 600;
  color: #2c3e50;
  margin-bottom: 0.25rem;
}

.status-desc {
  color: #6c757d;
  font-size: 0.8rem;
}

/* Loading ve Empty States */
.loading-state,
.empty-state {
  text-align: center;
  padding: 3rem;
  color: #6c757d;
}

.loading-spinner {
  font-size: 1.2rem;
}

.empty-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 1rem;
}

.empty-icon {
  font-size: 3rem;
}

/* Butonlar */
.btn {
  padding: 0.5rem 1rem;
  border: none;
  border-radius: 6px;
  cursor: pointer;
  font-weight: 600;
  transition: all 0.3s;
  display: inline-flex;
  align-items: center;
  gap: 0.5rem;
}

.btn-primary {
  background: #007bff;
  color: white;
}

.btn-primary:hover:not(:disabled) {
  background: #0056b3;
}

.btn:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}

/* Mesajlar */
.message {
  position: fixed;
  top: 20px;
  right: 20px;
  padding: 1rem 1.5rem;
  border-radius: 8px;
  font-weight: 600;
  z-index: 1100;
  animation: slideInRight 0.3s ease-out;
  max-width: 400px;
}

.message.success {
  background: #d4edda;
  color: #155724;
  border: 1px solid #c3e6cb;
}

.message.error {
  background: #f8d7da;
  color: #721c24;
  border: 1px solid #f5c6cb;
}

/* Animasyonlar */
@keyframes slideInRight {
  from {
    opacity: 0;
    transform: translateX(20px);
  }
  to {
    opacity: 1;
    transform: translateX(0);
  }
}

/* Responsive Tasarım */
@media (max-width: 1200px) {
  .charts-section {
    grid-template-columns: 1fr;
  }
  
  .bottom-section {
    grid-template-columns: 1fr;
  }
}

@media (max-width: 768px) {
  .dashboard-container {
    padding: 1rem;
  }
  
  .page-header {
    flex-direction: column;
    gap: 1rem;
    text-align: center;
  }
  
  .header-actions {
    width: 100%;
    justify-content: center;
    flex-wrap: wrap;
  }
  
  .date-range-selector {
    flex-direction: column;
    text-align: center;
  }
  
  .main-metrics {
    grid-template-columns: 1fr;
  }
  
  .metric-card {
    flex-direction: column;
    text-align: center;
  }
  
  .metric-icon {
    font-size: 2rem;
  }
  
  .chart-container {
    height: 250px;
  }
  
  .daily-summary {
    grid-template-columns: 1fr;
  }
  
  .quick-actions {
    grid-template-columns: 1fr;
  }
  
  .department-item {
    flex-direction: column;
    gap: 1rem;
    text-align: center;
  }
  
  .dept-metrics {
    width: 100%;
  }
  
  .violation-item {
    flex-direction: column;
    align-items: flex-start;
    gap: 0.5rem;
  }
  
  .violation-details {
    width: 100%;
    flex-direction: column;
    gap: 0.5rem;
  }
  
  .status-grid {
    grid-template-columns: 1fr;
  }
}

@media (max-width: 480px) {
  .custom-date-range .date-inputs {
    flex-direction: column;
  }
  
  .chart-header {
    flex-direction: column;
    gap: 1rem;
  }
  
  .violation-types {
    flex-direction: column;
    gap: 0.25rem;
  }
  
  .action-card {
    flex-direction: column;
    text-align: center;
  }
}

/* Print Stilleri */
@media print {
  .dashboard-container {
    background: white;
  }
  
  .header-actions,
  .chart-controls,
  .quick-actions,
  .violation-actions {
    display: none;
  }
  
  .charts-section,
  .bottom-section {
    grid-template-columns: 1fr;
  }
  
  .chart-container {
    height: 200px;
  }
}

/* Dark Mode Desteği */
@media (prefers-color-scheme: dark) {
  .dashboard-container {
    background: #1a1a1a;
    color: #e9ecef;
  }
  
  .page-header,
  .custom-date-range,
  .metric-card,
  .chart-card,
  .recent-violations,
  .system-status {
    background: #2d2d2d;
    border-color: #404040;
  }
  
  .chart-header,
  .section-header {
    background: #404040;
  }
  
  .form-control,
  .period-select,
  .chart-select {
    background: #404040;
    border-color: #555;
    color: #e9ecef;
  }
  
  .summary-item,
  .action-card,
  .department-item,
  .status-item {
    background: #404040;
    border-color: #555;
  }
}
</style>
