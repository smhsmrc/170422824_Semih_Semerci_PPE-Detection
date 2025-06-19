<template>
  <div class="workers-container">
    <!-- Sayfa Başlığı -->
    <div class="page-header">
      <div class="header-content">
        <h1>👥 Çalışan Yönetimi</h1>
        <p>Çalışanları kaydedin, düzenleyin ve PPE uyumlarını takip edin</p>
      </div>
      <div class="header-actions">
        <button @click="showAddWorkerModal = true" class="btn btn-success">
          ➕ Yeni Çalışan
        </button>
        <button @click="exportWorkers" class="btn btn-secondary">
          📊 Excel'e Aktar
        </button>
        <button @click="refreshWorkers" class="btn btn-primary" :disabled="isLoading">
          <span v-if="isLoading">🔄 Yenileniyor...</span>
          <span v-else>🔄 Yenile</span>
        </button>
      </div>
    </div>

    <!-- İstatistik Kartları -->
    <div class="stats-cards">
      <div class="stat-card primary">
        <div class="stat-icon">👥</div>
        <div class="stat-content">
          <div class="stat-value">{{ workerStats.total }}</div>
          <div class="stat-label">Toplam Çalışan</div>
        </div>
      </div>
      
      <div class="stat-card success">
        <div class="stat-icon">✅</div>
        <div class="stat-content">
          <div class="stat-value">{{ workerStats.active }}</div>
          <div class="stat-label">Aktif Çalışan</div>
        </div>
      </div>
      
      <div class="stat-card warning">
        <div class="stat-icon">⚠️</div>
        <div class="stat-content">
          <div class="stat-value">{{ workerStats.violations }}</div>
          <div class="stat-label">İhlal Yapan</div>
        </div>
      </div>
      
      <div class="stat-card info">
        <div class="stat-icon">📈</div>
        <div class="stat-content">
          <div class="stat-value">{{ workerStats.complianceRate }}%</div>
          <div class="stat-label">Ortalama Uyum</div>
        </div>
      </div>
    </div>

    <!-- Arama ve Filtreler -->
    <div class="search-filters-section">
      <div class="search-filters-card">
        <div class="search-section">
          <div class="search-box">
            <input 
              v-model="searchQuery" 
              type="text" 
              placeholder="🔍 Çalışan adı, ID veya departman ara..."
              class="search-input"
              @input="applyFilters"
            />
            <button v-if="searchQuery" @click="clearSearch" class="clear-search-btn">
              ✖️
            </button>
          </div>
        </div>
        
        <div class="filters-section">
          <div class="filter-group">
            <label>🏢 Departman:</label>
            <select v-model="filters.department" @change="applyFilters" class="form-control">
              <option value="">Tümü</option>
              <option v-for="dept in departments" :key="dept" :value="dept">
                {{ dept }}
              </option>
            </select>
          </div>
          
          <div class="filter-group">
            <label>📊 Durum:</label>
            <select v-model="filters.status" @change="applyFilters" class="form-control">
              <option value="">Tümü</option>
              <option value="active">Aktif</option>
              <option value="inactive">Pasif</option>
            </select>
          </div>
          
          <div class="filter-group">
            <label>⚠️ İhlal Durumu:</label>
            <select v-model="filters.violationStatus" @change="applyFilters" class="form-control">
              <option value="">Tümü</option>
              <option value="has_violations">İhlali Var</option>
              <option value="no_violations">İhlali Yok</option>
            </select>
          </div>
          
          <div class="filter-actions">
            <button @click="clearFilters" class="btn btn-outline">
              🗑️ Temizle
            </button>
          </div>
        </div>
      </div>
    </div>

    <!-- Çalışan Listesi -->
    <div class="workers-grid-section">
      <div class="grid-header">
        <h3>👥 Çalışan Listesi</h3>
        <div class="view-toggle">
          <button 
            @click="viewMode = 'grid'" 
            :class="['view-btn', { active: viewMode === 'grid' }]"
            title="Kart Görünümü"
          >
            🔲
          </button>
          <button 
            @click="viewMode = 'table'" 
            :class="['view-btn', { active: viewMode === 'table' }]"
            title="Tablo Görünümü"
          >
            📋
          </button>
        </div>
      </div>

      <!-- Kart Görünümü -->
      <div v-if="viewMode === 'grid'" class="workers-grid">
        <div v-if="isLoading" class="loading-state">
          <div class="loading-spinner">🔄 Çalışanlar yükleniyor...</div>
        </div>
        
        <div v-else-if="filteredWorkers.length === 0" class="empty-state">
          <div class="empty-icon">👥</div>
          <h3>Çalışan bulunamadı</h3>
          <p>Arama kriterlerinizi değiştirmeyi deneyin veya yeni çalışan ekleyin</p>
          <button @click="showAddWorkerModal = true" class="btn btn-primary">
            ➕ İlk Çalışanı Ekle
          </button>
        </div>
        
        <div 
          v-else
          v-for="worker in paginatedWorkers" 
          :key="worker.id"
          class="worker-card"
        >
          <div class="worker-avatar">
            <img 
              v-if="worker.photo" 
              :src="`http://localhost:5001/photos/${worker.photo}`" 
              :alt="worker.name"
              class="avatar-img"
            />
            <div v-else class="avatar-placeholder">
              {{ getInitials(worker.name) }}
            </div>
            <div :class="['status-indicator', worker.status]"></div>
          </div>
          
          <div class="worker-info">
            <h4 class="worker-name">{{ worker.name }}</h4>
            <p class="worker-id">ID: {{ worker.worker_id }}</p>
            <p class="worker-department">🏢 {{ worker.department }}</p>
            <p class="worker-position">💼 {{ worker.position }}</p>
          </div>
          
          <div class="worker-stats">
            <div class="stat-item">
              <span class="stat-label">Uyum Oranı:</span>
              <div class="compliance-bar">
                <div 
                  class="compliance-fill" 
                  :style="{ width: worker.compliance_rate + '%' }"
                  :class="getComplianceClass(worker.compliance_rate)"
                ></div>
                <span class="compliance-text">{{ worker.compliance_rate }}%</span>
              </div>
            </div>
            
            <div class="stat-item">
              <span class="stat-label">Toplam İhlal:</span>
              <span :class="['violation-count', { 'has-violations': worker.violation_count > 0 }]">
                {{ worker.violation_count }}
              </span>
            </div>
            
            <div class="stat-item">
              <span class="stat-label">Son Görülme:</span>
              <span class="last-seen">
                {{ worker.last_seen ? formatRelativeTime(worker.last_seen) : 'Hiç' }}
              </span>
            </div>
          </div>
          
          <div class="worker-actions">
            <button @click="viewWorkerDetails(worker)" class="action-btn view-btn" title="Detayları Görüntüle">
              👁️
            </button>
            <button @click="editWorker(worker)" class="action-btn edit-btn" title="Düzenle">
              ✏️
            </button>
            <button @click="viewWorkerViolations(worker)" class="action-btn violations-btn" title="İhlalleri Görüntüle">
              ⚠️
            </button>
            <button @click="deleteWorker(worker)" class="action-btn delete-btn" title="Sil">
              🗑️
            </button>
          </div>
        </div>
      </div>

      <!-- Tablo Görünümü -->
      <div v-else class="workers-table-container">
        <table class="workers-table">
          <thead>
            <tr>
              <th @click="sortBy('name')" class="sortable">
                Ad Soyad
                <span class="sort-icon" v-if="sortField === 'name'">
                  {{ sortDirection === 'asc' ? '↑' : '↓' }}
                </span>
              </th>
              <th @click="sortBy('worker_id')" class="sortable">
                Çalışan ID
                <span class="sort-icon" v-if="sortField === 'worker_id'">
                  {{ sortDirection === 'asc' ? '↑' : '↓' }}
                </span>
              </th>
              <th @click="sortBy('department')" class="sortable">
                Departman
                <span class="sort-icon" v-if="sortField === 'department'">
                  {{ sortDirection === 'asc' ? '↑' : '↓' }}
                </span>
              </th>
              <th>Pozisyon</th>
              <th @click="sortBy('compliance_rate')" class="sortable">
                Uyum Oranı
                <span class="sort-icon" v-if="sortField === 'compliance_rate'">
                  {{ sortDirection === 'asc' ? '↑' : '↓' }}
                </span>
              </th>
              <th @click="sortBy('violation_count')" class="sortable">
                İhlal Sayısı
                <span class="sort-icon" v-if="sortField === 'violation_count'">
                  {{ sortDirection === 'asc' ? '↑' : '↓' }}
                </span>
              </th>
              <th>Durum</th>
              <th>Son Görülme</th>
              <th>İşlemler</th>
            </tr>
          </thead>
          <tbody>
            <tr v-if="isLoading">
              <td colspan="9" class="loading-row">
                <div class="loading-spinner">🔄 Yükleniyor...</div>
              </td>
            </tr>
            <tr v-else-if="filteredWorkers.length === 0">
              <td colspan="9" class="empty-row">
                <div class="empty-state">
                  <div class="empty-icon">👥</div>
                  <p>Çalışan bulunamadı</p>
                </div>
              </td>
            </tr>
            <tr 
              v-else
              v-for="worker in paginatedWorkers" 
              :key="worker.id"
              class="worker-row"
            >
              <td class="name-cell">
                <div class="worker-name-cell">
                  <div class="worker-avatar-small">
                    <img 
                      v-if="worker.photo" 
                      :src="`http://localhost:5001/photos/${worker.photo}`" 
                      :alt="worker.name"
                      class="avatar-img-small"
                    />
                    <div v-else class="avatar-placeholder-small">
                      {{ getInitials(worker.name) }}
                    </div>
                  </div>
                  <div>
                    <div class="worker-name">{{ worker.name }}</div>
                    <div class="worker-email">{{ worker.email }}</div>
                  </div>
                </div>
              </td>
              <td class="id-cell">{{ worker.worker_id }}</td>
              <td class="department-cell">{{ worker.department }}</td>
              <td class="position-cell">{{ worker.position }}</td>
              <td class="compliance-cell">
                <div class="compliance-bar-small">
                  <div 
                    class="compliance-fill" 
                    :style="{ width: worker.compliance_rate + '%' }"
                    :class="getComplianceClass(worker.compliance_rate)"
                  ></div>
                  <span class="compliance-text">{{ worker.compliance_rate }}%</span>
                </div>
              </td>
              <td class="violations-cell">
                <span :class="['violation-badge', { 'has-violations': worker.violation_count > 0 }]">
                  {{ worker.violation_count }}
                </span>
              </td>
              <td class="status-cell">
                <span :class="['status-badge', worker.status]">
                  {{ worker.status === 'active' ? '✅ Aktif' : '❌ Pasif' }}
                </span>
              </td>
              <td class="last-seen-cell">
                {{ worker.last_seen ? formatRelativeTime(worker.last_seen) : 'Hiç' }}
              </td>
              <td class="actions-cell">
                <div class="table-actions">
                  <button @click="viewWorkerDetails(worker)" class="table-action-btn view-btn" title="Detaylar">
                    👁️
                  </button>
                  <button @click="editWorker(worker)" class="table-action-btn edit-btn" title="Düzenle">
                    ✏️
                  </button>
                  <button @click="viewWorkerViolations(worker)" class="table-action-btn violations-btn" title="İhlaller">
                    ⚠️
                  </button>
                  <button @click="deleteWorker(worker)" class="table-action-btn delete-btn" title="Sil">
                    🗑️
                  </button>
                </div>
              </td>
            </tr>
          </tbody>
        </table>
      </div>
    </div>

    <!-- Sayfalama -->
    <div class="pagination-section">
      <div class="pagination-info">
        {{ (currentPage - 1) * itemsPerPage + 1 }} - 
        {{ Math.min(currentPage * itemsPerPage, filteredWorkers.length) }} 
        / {{ filteredWorkers.length }} çalışan
      </div>
      
      <div class="pagination-controls">
        <button 
          @click="currentPage = 1" 
          :disabled="currentPage === 1"
          class="pagination-btn"
        >
          ⏮️
        </button>
        <button 
          @click="currentPage--" 
          :disabled="currentPage === 1"
          class="pagination-btn"
        >
          ◀️
        </button>
        
        <span class="page-numbers">
          <button
            v-for="page in visiblePages"
            :key="page"
            @click="currentPage = page"
            :class="['page-btn', { active: page === currentPage }]"
          >
            {{ page }}
          </button>
        </span>
        
        <button 
          @click="currentPage++" 
          :disabled="currentPage === totalPages"
          class="pagination-btn"
        >
          ▶️
        </button>
        <button 
          @click="currentPage = totalPages" 
          :disabled="currentPage === totalPages"
          class="pagination-btn"
        >
          ⏭️
        </button>
      </div>
      
      <div class="items-per-page">
        <label>Sayfa başına:</label>
        <select v-model="itemsPerPage" @change="currentPage = 1">
          <option value="12">12</option>
          <option value="24">24</option>
          <option value="48">48</option>
          <option value="96">96</option>
        </select>
      </div>
    </div>

    <!-- Çalışan Ekleme/Düzenleme Modal -->
    <div v-if="showAddWorkerModal || showEditWorkerModal" class="modal-overlay" @click="closeWorkerModal">
      <div class="worker-modal" @click.stop>
        <div class="modal-header">
          <h3>{{ showAddWorkerModal ? '➕ Yeni Çalışan Ekle' : '✏️ Çalışan Düzenle' }}</h3>
          <button @click="closeWorkerModal" class="close-btn">✖️</button>
        </div>
        
        <div class="modal-body">
          <form @submit.prevent="saveWorker" class="worker-form">
            <div class="form-grid">
              <div class="form-group">
                <label>👤 Ad Soyad *</label>
                <input 
                  v-model="workerForm.name" 
                  type="text" 
                  required
                  class="form-control"
                  placeholder="Ahmet Yılmaz"
                />
              </div>
              
              <div class="form-group">
                <label>🆔 Çalışan ID *</label>
                <input 
                  v-model="workerForm.worker_id" 
                  type="text" 
                  required
                  class="form-control"
                  placeholder="EMP001"
                />
              </div>
              
              <div class="form-group">
                <label>📧 E-posta</label>
                <input 
                  v-model="workerForm.email" 
                  type="email" 
                  class="form-control"
                  placeholder="ahmet.yilmaz@sirket.com"
                />
              </div>
              
              <div class="form-group">
                <label>📱 Telefon</label>
                <input 
                  v-model="workerForm.phone" 
                  type="tel" 
                  class="form-control"
                  placeholder="+90 555 123 45 67"
                />
              </div>
              
              <div class="form-group">
                <label>🏢 Departman *</label>
                <select v-model="workerForm.department" required class="form-control">
                  <option value="">Departman Seçin</option>
                  <option value="Üretim">Üretim</option>
                  <option value="Kalite Kontrol">Kalite Kontrol</option>
                  <option value="Bakım">Bakım</option>
                  <option value="Depo">Depo</option>
                  <option value="İnsan Kaynakları">İnsan Kaynakları</option>
                  <option value="Muhasebe">Muhasebe</option>
                  <option value="Satış">Satış</option>
                  <option value="Pazarlama">Pazarlama</option>
                </select>
              </div>
              
              <div class="form-group">
                <label>💼 Pozisyon</label>
                <input 
                  v-model="workerForm.position" 
                  type="text" 
                  class="form-control"
                  placeholder="Operatör"
                />
              </div>
              
              <div class="form-group">
                <label>📅 İşe Başlama Tarihi</label>
                <input 
                  v-model="workerForm.hire_date" 
                  type="date" 
                  class="form-control"
                />
              </div>
              
              <div class="form-group">
                <label>📊 Durum</label>
                <select v-model="workerForm.status" class="form-control">
                  <option value="active">✅ Aktif</option>
                  <option value="inactive">❌ Pasif</option>
                </select>
              </div>
            </div>
            
            <div class="form-group">
              <label>📷 Fotoğraf</label>
              <div class="photo-upload">
                <input 
                  ref="photoInput"
                  type="file" 
                  accept="image/*" 
                  @change="handlePhotoUpload"
                  class="photo-input"
                />
                <div class="photo-preview" v-if="workerForm.photoPreview">
                  <img :src="workerForm.photoPreview" alt="Önizleme" class="preview-img" />
                  <button type="button" @click="removePhoto" class="remove-photo-btn">✖️</button>
                </div>
                <div v-else class="photo-placeholder" @click="$refs.photoInput.click()">
                  <div class="placeholder-icon">📷</div>
                  <p>Fotoğraf Yükle</p>
                  <small>JPG, PNG formatında maksimum 5MB</small>
                </div>
              </div>
            </div>
            
            <div class="form-group">
              <label>📝 Notlar</label>
              <textarea 
                v-model="workerForm.notes" 
                class="form-control"
                rows="3"
                placeholder="Çalışan hakkında ek bilgiler..."
              ></textarea>
            </div>
          </form>
        </div>
        
        <div class="modal-footer">
          <button @click="closeWorkerModal" type="button" class="btn btn-secondary">
            ❌ İptal
          </button>
          <button @click="saveWorker" type="button" class="btn btn-primary" :disabled="isSaving">
            <span v-if="isSaving">💾 Kaydediliyor...</span>
            <span v-else>💾 {{ showAddWorkerModal ? 'Ekle' : 'Güncelle' }}</span>
          </button>
        </div>
      </div>
    </div>

    <!-- Çalışan Detay Modal -->
    <div v-if="showDetailModal" class="modal-overlay" @click="closeDetailModal">
      <div class="detail-modal" @click.stop>
        <div class="modal-header">
          <h3>👤 Çalışan Detayları</h3>
          <button @click="closeDetailModal" class="close-btn">✖️</button>
        </div>
        
        <div class="modal-body">
          <div class="worker-detail-content" v-if="selectedWorker">
            <div class="worker-profile">
              <div class="profile-avatar">
                <img 
                  v-if="selectedWorker.photo" 
                  :src="`http://localhost:5001/photos/${selectedWorker.photo}`" 
                  :alt="selectedWorker.name"
                  class="profile-img"
                />
                <div v-else class="profile-placeholder">
                  {{ getInitials(selectedWorker.name) }}
                </div>
              </div>
              
              <div class="profile-info">
                <h2>{{ selectedWorker.name }}</h2>
                <p class="worker-id">ID: {{ selectedWorker.worker_id }}</p>
                <span :class="['status-badge', selectedWorker.status]">
                  {{ selectedWorker.status === 'active' ? '✅ Aktif' : '❌ Pasif' }}
                </span>
              </div>
            </div>
            
            <div class="detail-sections">
              <div class="detail-section">
                <h4>📋 Genel Bilgiler</h4>
                <div class="detail-grid">
                  <div class="detail-item">
                    <strong>🏢 Departman:</strong>
                    <span>{{ selectedWorker.department }}</span>
                  </div>
                  <div class="detail-item">
                    <strong>💼 Pozisyon:</strong>
                    <span>{{ selectedWorker.position || 'Belirtilmemiş' }}</span>
                  </div>
                  <div class="detail-item">
                    <strong>📧 E-posta:</strong>
                    <span>{{ selectedWorker.email || 'Belirtilmemiş' }}</span>
                  </div>
                  <div class="detail-item">
                    <strong>📱 Telefon:</strong>
                    <span>{{ selectedWorker.phone || 'Belirtilmemiş' }}</span>
                  </div>
                  <div class="detail-item">
                    <strong>📅 İşe Başlama:</strong>
                    <span>{{ selectedWorker.hire_date ? formatDate(selectedWorker.hire_date) : 'Belirtilmemiş' }}</span>
                  </div>
                  <div class="detail-item">
                    <strong>👁️ Son Görülme:</strong>
                    <span>{{ selectedWorker.last_seen ? formatDateTime(selectedWorker.last_seen) : 'Hiç' }}</span>
                  </div>
                </div>
              </div>
              
              <div class="detail-section">
                <h4>📊 PPE Uyum İstatistikleri</h4>
                <div class="stats-grid">
                  <div class="stat-box">
                    <div class="stat-value">{{ selectedWorker.compliance_rate }}%</div>
                    <div class="stat-label">Genel Uyum Oranı</div>
                    <div class="compliance-bar">
                      <div 
                        class="compliance-fill" 
                        :style="{ width: selectedWorker.compliance_rate + '%' }"
                        :class="getComplianceClass(selectedWorker.compliance_rate)"
                      ></div>
                    </div>
                  </div>
                  
                  <div class="stat-box">
                    <div class="stat-value">{{ selectedWorker.violation_count }}</div>
                    <div class="stat-label">Toplam İhlal</div>
                  </div>
                  
                  <div class="stat-box">
                    <div class="stat-value">{{ selectedWorker.detection_count || 0 }}</div>
                    <div class="stat-label">Tespit Sayısı</div>
                  </div>
                </div>
              </div>
              
              <div class="detail-section" v-if="selectedWorker.notes">
                <h4>📝 Notlar</h4>
                <p class="notes-text">{{ selectedWorker.notes }}</p>
              </div>
            </div>
          </div>
        </div>
        
        <div class="modal-footer">
          <button @click="editWorker(selectedWorker)" class="btn btn-primary">
            ✏️ Düzenle
          </button>
          <button @click="viewWorkerViolations(selectedWorker)" class="btn btn-warning">
            ⚠️ İhlalleri Görüntüle
          </button>
          <button @click="closeDetailModal" class="btn btn-secondary">
            ❌ Kapat
          </button>
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
export default {
  name: 'WorkerListView',
  data() {
    return {
      workers: [],
      filteredWorkers: [],
      isLoading: false,
      isSaving: false,
      
      // Görünüm modu
      viewMode: 'grid', // 'grid' veya 'table'
      
      // İstatistikler
      workerStats: {
        total: 0,
        active: 0,
        violations: 0,
        complianceRate: 0
      },
      
      // Arama ve filtreler
      searchQuery: '',
      filters: {
        department: '',
        status: '',
        violationStatus: ''
      },
      
      // Sıralama
      sortField: 'name',
      sortDirection: 'asc',
      
      // Sayfalama
      currentPage: 1,
      itemsPerPage: 12,
      
      // Modaller
      showAddWorkerModal: false,
      showEditWorkerModal: false,
      showDetailModal: false,
      selectedWorker: null,
      
      // Form
      workerForm: {
        name: '',
        worker_id: '',
        email: '',
        phone: '',
        department: '',
        position: '',
        hire_date: '',
        status: 'active',
        photo: null,
        photoPreview: null,
        notes: ''
      },
      
      // Mesajlar
      message: '',
      messageType: 'success'
    }
  },
  
  computed: {
    departments() {
      const depts = [...new Set(this.workers.map(w => w.department).filter(Boolean))]
      return depts.sort()
    },
    
    paginatedWorkers() {
      const start = (this.currentPage - 1) * this.itemsPerPage
      const end = start + this.itemsPerPage
      return this.filteredWorkers.slice(start, end)
    },
    
    totalPages() {
      return Math.ceil(this.filteredWorkers.length / this.itemsPerPage)
    },
    
    visiblePages() {
      const pages = []
      const total = this.totalPages
      const current = this.currentPage
      
      let start = Math.max(1, current - 2)
      let end = Math.min(total, current + 2)
      
      for (let i = start; i <= end; i++) {
        pages.push(i)
      }
      
      return pages
    }
  },
  
  mounted() {
    this.loadWorkers()
    this.loadStats()
  },
  
  methods: {
    async loadWorkers() {
      this.isLoading = true
      try {
        const response = await fetch('http://localhost:5001/workers')
        if (response.ok) {
          this.workers = await response.json()
          this.applyFilters()
        }
      } catch (error) {
        console.error('Çalışanlar yüklenemedi:', error)
        this.showMessage('Çalışanlar yüklenirken hata oluştu! ❌', 'error')
      } finally {
        this.isLoading = false
      }
    },
    
    async loadStats() {
      try {
        const response = await fetch('http://localhost:5001/workers/stats')
        if (response.ok) {
          this.workerStats = await response.json()
        }
      } catch (error) {
        console.error('İstatistikler yüklenemedi:', error)
      }
    },
    
    applyFilters() {
      let filtered = [...this.workers]
      
      // Arama filtresi
      if (this.searchQuery) {
        const query = this.searchQuery.toLowerCase()
        filtered = filtered.filter(worker => 
          worker.name.toLowerCase().includes(query) ||
          worker.worker_id.toLowerCase().includes(query) ||
          (worker.department && worker.department.toLowerCase().includes(query)) ||
          (worker.email && worker.email.toLowerCase().includes(query))
        )
      }
      
      // Departman filtresi
      if (this.filters.department) {
        filtered = filtered.filter(worker => worker.department === this.filters.department)
      }
      
      // Durum filtresi
      if (this.filters.status) {
        filtered = filtered.filter(worker => worker.status === this.filters.status)
      }
      
      // İhlal durumu filtresi
      if (this.filters.violationStatus) {
        if (this.filters.violationStatus === 'has_violations') {
          filtered = filtered.filter(worker => worker.violation_count > 0)
        } else if (this.filters.violationStatus === 'no_violations') {
          filtered = filtered.filter(worker => worker.violation_count === 0)
        }
      }
      
      this.filteredWorkers = filtered
      this.sortWorkers()
      this.currentPage = 1
    },
    
    clearSearch() {
      this.searchQuery = ''
      this.applyFilters()
    },
    
    clearFilters() {
      this.filters = {
        department: '',
        status: '',
        violationStatus: ''
      }
      this.searchQuery = ''
      this.applyFilters()
    },
    
    sortBy(field) {
      if (this.sortField === field) {
        this.sortDirection = this.sortDirection === 'asc' ? 'desc' : 'asc'
      } else {
        this.sortField = field
        this.sortDirection = 'asc'
      }
      this.sortWorkers()
    },
    
    sortWorkers() {
      this.filteredWorkers.sort((a, b) => {
        let aVal = a[this.sortField]
        let bVal = b[this.sortField]
        
        if (typeof aVal === 'string') {
          aVal = aVal.toLowerCase()
          bVal = bVal.toLowerCase()
        }
        
        if (this.sortDirection === 'asc') {
          return aVal > bVal ? 1 : -1
        } else {
          return aVal < bVal ? 1 : -1
        }
      })
    },
    
    async refreshWorkers() {
      await this.loadWorkers()
      await this.loadStats()
    },
    
    // Çalışan işlemleri
    viewWorkerDetails(worker) {
      this.selectedWorker = worker
      this.showDetailModal = true
    },
    
    editWorker(worker) {
      this.workerForm = {
        id: worker.id,
        name: worker.name,
        worker_id: worker.worker_id,
        email: worker.email || '',
        phone: worker.phone || '',
        department: worker.department,
        position: worker.position || '',
        hire_date: worker.hire_date || '',
        status: worker.status,
        photo: worker.photo,
        photoPreview: worker.photo ? `http://localhost:5001/photos/${worker.photo}` : null,
        notes: worker.notes || ''
      }
      this.showEditWorkerModal = true
    },
    
    async deleteWorker(worker) {
      if (confirm(`${worker.name} adlı çalışanı silmek istediğinizden emin misiniz?\n\nBu işlem geri alınamaz ve çalışana ait tüm veriler silinecektir.`)) {
        try {
          const response = await fetch(`http://localhost:5001/workers/${worker.id}`, {
            method: 'DELETE'
          })
          
          if (response.ok) {
            await this.refreshWorkers()
            this.showMessage(`${worker.name} başarıyla silindi! ✅`, 'success')
          } else {
            this.showMessage('Çalışan silinirken hata oluştu! ❌', 'error')
          }
        } catch (error) {
          console.error('Silme hatası:', error)
          this.showMessage('Bağlantı hatası! 🔄', 'error')
        }
      }
    },
    
    viewWorkerViolations(worker) {
      // Violations sayfasına yönlendir ve çalışan filtresini uygula
      this.$router.push({
        name: 'violations',
        query: { worker_id: worker.worker_id }
      })
    },
    
    // Modal işlemleri
    closeWorkerModal() {
      this.showAddWorkerModal = false
      this.showEditWorkerModal = false
      this.resetWorkerForm()
    },
    
    closeDetailModal() {
      this.showDetailModal = false
      this.selectedWorker = null
    },
    
    resetWorkerForm() {
      this.workerForm = {
        name: '',
        worker_id: '',
        email: '',
        phone: '',
        department: '',
        position: '',
        hire_date: '',
        status: 'active',
        photo: null,
        photoPreview: null,
        notes: ''
      }
    },
    
    // Form işlemleri
    async saveWorker() {
      this.isSaving = true
      
      try {
        const formData = new FormData()
        
        // Form verilerini ekle
        Object.keys(this.workerForm).forEach(key => {
          if (key !== 'photoPreview' && this.workerForm[key] !== null) {
            formData.append(key, this.workerForm[key])
          }
        })
        
        const url = this.showEditWorkerModal 
          ? `http://localhost:5001/workers/${this.workerForm.id}`
          : 'http://localhost:5001/workers'
        
        const method = this.showEditWorkerModal ? 'PUT' : 'POST'
        
        const response = await fetch(url, {
          method: method,
          body: formData
        })
        
        if (response.ok) {
          await this.refreshWorkers()
          this.closeWorkerModal()
          const message = this.showEditWorkerModal 
            ? 'Çalışan başarıyla güncellendi! ✅'
            : 'Yeni çalışan başarıyla eklendi! ✅'
          this.showMessage(message, 'success')
        } else {
          const errorData = await response.json()
          this.showMessage(errorData.message || 'İşlem başarısız! ❌', 'error')
        }
      } catch (error) {
        console.error('Kaydetme hatası:', error)
        this.showMessage('Bağlantı hatası! 🔄', 'error')
      } finally {
        this.isSaving = false
      }
    },
    
    handlePhotoUpload(event) {
      const file = event.target.files[0]
      if (file) {
        // Dosya boyutu kontrolü (5MB)
        if (file.size > 5 * 1024 * 1024) {
          this.showMessage('Fotoğraf boyutu 5MB\'dan büyük olamaz! ❌', 'error')
          return
        }
        
        // Dosya tipi kontrolü
        if (!file.type.startsWith('image/')) {
          this.showMessage('Sadece resim dosyaları yüklenebilir! ❌', 'error')
          return
        }
        
        this.workerForm.photo = file
        
        // Önizleme oluştur
        const reader = new FileReader()
        reader.onload = (e) => {
          this.workerForm.photoPreview = e.target.result
        }
        reader.readAsDataURL(file)
      }
    },
    
    removePhoto() {
      this.workerForm.photo = null
      this.workerForm.photoPreview = null
      this.$refs.photoInput.value = ''
    },
    
    async exportWorkers() {
      try {
        const response = await fetch('http://localhost:5001/workers/export', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({
            filters: this.filters,
            searchQuery: this.searchQuery
          })
        })
        
        if (response.ok) {
          const blob = await response.blob()
          const url = window.URL.createObjectURL(blob)
          const link = document.createElement('a')
          link.href = url
          link.download = `calisanlar_${new Date().toISOString().split('T')[0]}.xlsx`
          link.click()
          window.URL.revokeObjectURL(url)
          this.showMessage('Excel dosyası başarıyla indirildi! 📊', 'success')
        }
      } catch (error) {
        console.error('Dışa aktarma hatası:', error)
        this.showMessage('Dışa aktarma sırasında hata oluştu! ❌', 'error')
      }
    },
    
    // Yardımcı fonksiyonlar
    getInitials(name) {
      return name.split(' ').map(n => n[0]).join('').toUpperCase()
    },
    
    getComplianceClass(rate) {
      if (rate >= 90) return 'excellent'
      if (rate >= 75) return 'good'
      if (rate >= 50) return 'average'
      return 'poor'
    },
    
    formatDate(dateString) {
      return new Date(dateString).toLocaleDateString('tr-TR')
    },
    
    formatDateTime(timestamp) {
      return new Date(timestamp * 1000).toLocaleString('tr-TR')
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
.workers-container {
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
  gap: 1rem;
}

.stats-cards {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 1rem;
  margin-bottom: 2rem;
}

.stat-card {
  background: white;
  padding: 1.5rem;
  border-radius: 12px;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
  display: flex;
  align-items: center;
  gap: 1rem;
}

.stat-card.primary { border-left: 4px solid #007bff; }
.stat-card.success { border-left: 4px solid #28a745; }
.stat-card.warning { border-left: 4px solid #ffc107; }
.stat-card.info { border-left: 4px solid #17a2b8; }

.stat-icon {
  font-size: 2rem;
}

.stat-value {
  font-size: 1.5rem;
  font-weight: bold;
  color: #2c3e50;
}

.stat-label {
  color: #6c757d;
  font-size: 0.9rem;
}

.search-filters-section {
  margin-bottom: 2rem;
}

.search-filters-card {
  background: white;
  padding: 1.5rem;
  border-radius: 12px;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
}

.search-section {
  margin-bottom: 1rem;
}

.search-box {
  position: relative;
  max-width: 500px;
}

.search-input {
  width: 100%;
  padding: 0.75rem 1rem;
  padding-right: 3rem;
  border: 2px solid #e9ecef;
  border-radius: 25px;
  font-size: 1rem;
  transition: border-color 0.3s;
}

.search-input:focus {
  outline: none;
  border-color: #007bff;
}

.clear-search-btn {
  position: absolute;
  right: 10px;
  top: 50%;
  transform: translateY(-50%);
  background: none;
  border: none;
  cursor: pointer;
  padding: 0.25rem;
  border-radius: 50%;
  transition: background 0.3s;
}

.clear-search-btn:hover {
  background: #f8f9fa;
}

.filters-section {
  display: flex;
  flex-wrap: wrap;
  gap: 1rem;
  align-items: end;
}

.filter-group {
  min-width: 150px;
}

.filter-group label {
  display: block;
  margin-bottom: 0.5rem;
  font-weight: 600;
  color: #495057;
}

.form-control {
  width: 100%;
  padding: 0.5rem;
  border: 2px solid #e9ecef;
  border-radius: 6px;
  font-size: 0.9rem;
}

.form-control:focus {
  outline: none;
  border-color: #007bff;
}

.workers-grid-section {
  background: white;
  border-radius: 12px;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
  overflow: hidden;
}

.grid-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 1.5rem;
  border-bottom: 1px solid #e9ecef;
}

.grid-header h3 {
  margin: 0;
  color: #2c3e50;
}

.view-toggle {
  display: flex;
  gap: 0.5rem;
}

.view-btn {
  background: #f8f9fa;
  border: 1px solid #dee2e6;
  padding: 0.5rem 0.75rem;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.3s;
}

.view-btn.active,
.view-btn:hover {
  background: #007bff;
  color: white;
  border-color: #007bff;
}

/* Kart Görünümü */
.workers-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
  gap: 1.5rem;
  padding: 1.5rem;
}

.worker-card {
  background: white;
  border: 1px solid #e9ecef;
  border-radius: 12px;
  padding: 1.5rem;
  transition: all 0.3s;
  position: relative;
}

.worker-card:hover {
  box-shadow: 0 4px 12px rgba(0,0,0,0.15);
  transform: translateY(-2px);
}

.worker-avatar {
  position: relative;
  width: 80px;
  height: 80px;
  margin: 0 auto 1rem;
}

.avatar-img {
  width: 100%;
  height: 100%;
  border-radius: 50%;
  object-fit: cover;
  border: 3px solid #e9ecef;
}

.avatar-placeholder {
  width: 100%;
  height: 100%;
  border-radius: 50%;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  display: flex;
  align-items: center;
  justify-content: center;
  color: white;
  font-weight: bold;
  font-size: 1.5rem;
}

.status-indicator {
  position: absolute;
  bottom: 5px;
  right: 5px;
  width: 20px;
  height: 20px;
  border-radius: 50%;
  border: 3px solid white;
}

.status-indicator.active {
  background: #28a745;
}

.status-indicator.inactive {
  background: #dc3545;
}

.worker-info {
  text-align: center;
  margin-bottom: 1rem;
}

.worker-name {
  font-size: 1.1rem;
  font-weight: bold;
  color: #2c3e50;
  margin-bottom: 0.25rem;
}

.worker-id {
  color: #6c757d;
  font-size: 0.9rem;
  margin-bottom: 0.5rem;
}

.worker-department,
.worker-position {
  color: #495057;
  font-size: 0.85rem;
  margin-bottom: 0.25rem;
}

.worker-stats {
  margin-bottom: 1rem;
}

.stat-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 0.5rem;
}

.stat-label {
  font-size: 0.8rem;
  color: #6c757d;
}

.compliance-bar {
  position: relative;
  background: #e9ecef;
  border-radius: 10px;
  height: 16px;
  overflow: hidden;
  flex: 1;
  margin-left: 0.5rem;
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

.violation-count {
  font-weight: bold;
  color: #28a745;
}

.violation-count.has-violations {
  color: #dc3545;
}

.last-seen {
  font-size: 0.8rem;
  color: #6c757d;
}

.worker-actions {
  display: flex;
  justify-content: center;
  gap: 0.5rem;
}

.action-btn {
  background: white;
  border: 1px solid #dee2e6;
  padding: 0.5rem;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.3s;
  font-size: 1rem;
}

.action-btn:hover {
  background: #f8f9fa;
}

.view-btn:hover { border-color: #007bff; }
.edit-btn:hover { border-color: #28a745; }
.violations-btn:hover { border-color: #ffc107; }
.delete-btn:hover { border-color: #dc3545; }

/* Tablo Görünümü */
.workers-table-container {
  overflow-x: auto;
}

.workers-table {
  width: 100%;
  border-collapse: collapse;
}

.workers-table th,
.workers-table td {
  padding: 1rem;
  text-align: left;
  border-bottom: 1px solid #e9ecef;
}

.workers-table th {
  background: #f8f9fa;
  font-weight: 600;
  color: #495057;
  position: sticky;
  top: 0;
  z-index: 10;
}

.workers-table th.sortable {
  cursor: pointer;
  user-select: none;
  transition: background 0.3s;
}

.workers-table th.sortable:hover {
  background: #e9ecef;
}

.sort-icon {
  margin-left: 0.5rem;
  color: #007bff;
}

.worker-row:hover {
  background: #f8f9fa;
}

.worker-name-cell {
  display: flex;
  align-items: center;
  gap: 1rem;
}

.worker-avatar-small {
  width: 40px;
  height: 40px;
}

.avatar-img-small {
  width: 100%;
  height: 100%;
  border-radius: 50%;
  object-fit: cover;
}

.avatar-placeholder-small {
  width: 100%;
  height: 100%;
  border-radius: 50%;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  display: flex;
  align-items: center;
  justify-content: center;
  color: white;
  font-weight: bold;
  font-size: 0.9rem;
}

.worker-name {
  font-weight: 600;
  color: #2c3e50;
}

.worker-email {
  font-size: 0.8rem;
  color: #6c757d;
}

.compliance-bar-small {
  position: relative;
  background: #e9ecef;
  border-radius: 8px;
  height: 12px;
  width: 80px;
  overflow: hidden;
}

.violation-badge {
  background: #28a745;
  color: white;
  padding: 0.25rem 0.5rem;
  border-radius: 12px;
  font-size: 0.75rem;
  font-weight: bold;
}

.violation-badge.has-violations {
  background: #dc3545;
}

.status-badge {
  padding: 0.25rem 0.5rem;
  border-radius: 12px;
  font-size: 0.75rem;
  font-weight: bold;
}

.status-badge.active {
  background: #d4edda;
  color: #155724;
}

.status-badge.inactive {
  background: #f8d7da;
  color: #721c24;
}

.table-actions {
  display: flex;
  gap: 0.25rem;
}

.table-action-btn {
  background: none;
  border: 1px solid #dee2e6;
  padding: 0.25rem 0.5rem;
  border-radius: 4px;
  cursor: pointer;
  transition: all 0.3s;
  font-size: 0.8rem;
}

.table-action-btn:hover {
  background: #f8f9fa;
}

/* Loading ve Empty States */
.loading-state,
.empty-state {
  text-align: center;
  padding: 3rem;
  grid-column: 1 / -1;
}

.loading-spinner {
  font-size: 1.2rem;
  color: #6c757d;
}

.empty-state {
  color: #6c757d;
}

.empty-icon {
  font-size: 4rem;
  margin-bottom: 1rem;
}

.empty-state h3 {
  color: #495057;
  margin-bottom: 0.5rem;
}

.loading-row,
.empty-row {
  text-align: center;
  padding: 3rem;
}

/* Sayfalama */
.pagination-section {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 1.5rem;
  border-top: 1px solid #e9ecef;
  background: #f8f9fa;
}

.pagination-info {
  color: #6c757d;
  font-size: 0.9rem;
}

.pagination-controls {
  display: flex;
  align-items: center;
  gap: 0.5rem;
}

.pagination-btn,
.page-btn {
  background: white;
  border: 1px solid #dee2e6;
  padding: 0.5rem 0.75rem;
  border-radius: 4px;
  cursor: pointer;
  transition: all 0.3s;
}

.pagination-btn:hover:not(:disabled),
.page-btn:hover {
  background: #e9ecef;
}

.pagination-btn:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.page-btn.active {
  background: #007bff;
  color: white;
  border-color: #007bff;
}

.page-numbers {
  display: flex;
  gap: 0.25rem;
}

.items-per-page {
  display: flex;
  align-items: center;
  gap: 0.5rem;
}

.items-per-page label {
  color: #6c757d;
  font-size: 0.9rem;
}

.items-per-page select {
  padding: 0.25rem 0.5rem;
  border: 1px solid #dee2e6;
  border-radius: 4px;
}

/* Modal Stilleri */
.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0,0,0,0.8);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
  animation: fadeIn 0.3s ease-out;
}

.worker-modal,
.detail-modal {
  background: white;
  border-radius: 12px;
  max-width: 90vw;
  max-height: 90vh;
  overflow: hidden;
  animation: slideIn 0.3s ease-out;
}

.worker-modal {
  width: 800px;
}

.detail-modal {
  width: 700px;
}

<style scoped>
/* ... önceki stiller ... */

.modal-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 1.5rem;
  border-bottom: 1px solid #e9ecef;
  background: #f8f9fa;
}

.modal-header h3 {
  margin: 0;
  color: #2c3e50;
}

.close-btn {
  background: none;
  border: none;
  font-size: 1.2rem;
  cursor: pointer;
  padding: 0.25rem;
  border-radius: 4px;
  transition: background 0.3s;
}

.close-btn:hover {
  background: #e9ecef;
}

.modal-body {
  padding: 1.5rem;
  max-height: 60vh;
  overflow-y: auto;
}

.modal-footer {
  display: flex;
  justify-content: flex-end;
  gap: 1rem;
  padding: 1.5rem;
  border-top: 1px solid #e9ecef;
  background: #f8f9fa;
}

/* Form Stilleri */
.worker-form {
  width: 100%;
}

.form-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 1rem;
  margin-bottom: 1rem;
}

.form-group {
  display: flex;
  flex-direction: column;
}

.form-group label {
  margin-bottom: 0.5rem;
  font-weight: 600;
  color: #495057;
}

.form-control {
  padding: 0.75rem;
  border: 2px solid #e9ecef;
  border-radius: 6px;
  font-size: 1rem;
  transition: border-color 0.3s;
}

.form-control:focus {
  outline: none;
  border-color: #007bff;
}

.form-control:required {
  border-left: 4px solid #007bff;
}

textarea.form-control {
  resize: vertical;
  min-height: 80px;
}

/* Fotoğraf Upload */
.photo-upload {
  margin-top: 0.5rem;
}

.photo-input {
  display: none;
}

.photo-preview {
  position: relative;
  display: inline-block;
}

.preview-img {
  width: 120px;
  height: 120px;
  border-radius: 50%;
  object-fit: cover;
  border: 3px solid #e9ecef;
}

.remove-photo-btn {
  position: absolute;
  top: -5px;
  right: -5px;
  background: #dc3545;
  color: white;
  border: none;
  border-radius: 50%;
  width: 24px;
  height: 24px;
  cursor: pointer;
  font-size: 0.8rem;
  display: flex;
  align-items: center;
  justify-content: center;
}

.photo-placeholder {
  width: 120px;
  height: 120px;
  border: 2px dashed #dee2e6;
  border-radius: 12px;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  cursor: pointer;
  transition: all 0.3s;
  text-align: center;
  padding: 1rem;
}

.photo-placeholder:hover {
  border-color: #007bff;
  background: #f8f9fa;
}

.placeholder-icon {
  font-size: 2rem;
  margin-bottom: 0.5rem;
  color: #6c757d;
}

.photo-placeholder p {
  margin: 0 0 0.25rem 0;
  font-weight: 600;
  color: #495057;
}

.photo-placeholder small {
  color: #6c757d;
  font-size: 0.75rem;
}

/* Detay Modal */
.worker-detail-content {
  width: 100%;
}

.worker-profile {
  display: flex;
  align-items: center;
  gap: 1.5rem;
  margin-bottom: 2rem;
  padding-bottom: 1.5rem;
  border-bottom: 1px solid #e9ecef;
}

.profile-avatar {
  width: 100px;
  height: 100px;
}

.profile-img {
  width: 100%;
  height: 100%;
  border-radius: 50%;
  object-fit: cover;
  border: 4px solid #e9ecef;
}

.profile-placeholder {
  width: 100%;
  height: 100%;
  border-radius: 50%;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  display: flex;
  align-items: center;
  justify-content: center;
  color: white;
  font-weight: bold;
  font-size: 2rem;
}

.profile-info h2 {
  margin: 0 0 0.5rem 0;
  color: #2c3e50;
}

.profile-info .worker-id {
  color: #6c757d;
  margin-bottom: 0.5rem;
}

.detail-sections {
  display: flex;
  flex-direction: column;
  gap: 1.5rem;
}

.detail-section h4 {
  color: #495057;
  margin-bottom: 1rem;
  padding-bottom: 0.5rem;
  border-bottom: 2px solid #e9ecef;
}

.detail-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 1rem;
}

.detail-item {
  display: flex;
  flex-direction: column;
  gap: 0.25rem;
}

.detail-item strong {
  color: #495057;
  font-size: 0.9rem;
}

.detail-item span {
  color: #6c757d;
}

.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
  gap: 1rem;
}

.stat-box {
  background: #f8f9fa;
  padding: 1rem;
  border-radius: 8px;
  text-align: center;
}

.stat-box .stat-value {
  font-size: 1.5rem;
  font-weight: bold;
  color: #2c3e50;
  margin-bottom: 0.25rem;
}

.stat-box .stat-label {
  color: #6c757d;
  font-size: 0.8rem;
  margin-bottom: 0.5rem;
}

.stat-box .compliance-bar {
  height: 8px;
  margin-top: 0.5rem;
}

.notes-text {
  background: #f8f9fa;
  padding: 1rem;
  border-radius: 8px;
  color: #495057;
  line-height: 1.5;
  margin: 0;
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
  text-decoration: none;
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

.btn-secondary:hover {
  background: #545b62;
}

.btn-success {
  background: #28a745;
  color: white;
}

.btn-success:hover {
  background: #1e7e34;
}

.btn-warning {
  background: #ffc107;
  color: #212529;
}

.btn-warning:hover {
  background: #e0a800;
}

.btn-outline {
  background: white;
  color: #6c757d;
  border: 1px solid #dee2e6;
}

.btn-outline:hover {
  background: #f8f9fa;
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
@keyframes fadeIn {
  from { opacity: 0; }
  to { opacity: 1; }
}

@keyframes slideIn {
  from {
    opacity: 0;
    transform: translateY(-20px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

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
@media (max-width: 1024px) {
  .workers-grid {
    grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
    gap: 1rem;
  }
  
  .form-grid {
    grid-template-columns: 1fr;
  }
  
  .detail-grid {
    grid-template-columns: 1fr;
  }
  
  .stats-grid {
    grid-template-columns: 1fr;
  }
}

@media (max-width: 768px) {
  .workers-container {
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
  
  .stats-cards {
    grid-template-columns: 1fr;
  }
  
  .search-filters-card {
    padding: 1rem;
  }
  
  .filters-section {
    flex-direction: column;
    align-items: stretch;
  }
  
  .filter-group {
    min-width: auto;
  }
  
  .workers-grid {
    grid-template-columns: 1fr;
    padding: 1rem;
  }
  
  .workers-table-container {
    font-size: 0.8rem;
  }
  
  .workers-table th,
  .workers-table td {
    padding: 0.5rem;
  }
  
  .worker-name-cell {
    flex-direction: column;
    gap: 0.5rem;
    text-align: center;
  }
  
  .table-actions {
    flex-direction: column;
    gap: 0.25rem;
  }
  
  .pagination-section {
    flex-direction: column;
    gap: 1rem;
  }
  
  .pagination-controls {
    justify-content: center;
  }
  
  .worker-modal,
  .detail-modal {
    width: 95vw;
    margin: 1rem;
  }
  
  .worker-profile {
    flex-direction: column;
    text-align: center;
  }
  
  .modal-footer {
    flex-direction: column;
  }
}

@media (max-width: 480px) {
  .search-input {
    font-size: 16px; /* iOS zoom önleme */
  }
  
  .worker-card {
    padding: 1rem;
  }
  
  .worker-actions {
    flex-wrap: wrap;
  }
  
  .action-btn {
    flex: 1;
    min-width: 40px;
  }
  
  .compliance-bar {
    min-width: 60px;
  }
  
  .photo-placeholder {
    width: 100px;
    height: 100px;
  }
  
  .preview-img {
    width: 100px;
    height: 100px;
  }
}

/* Print Stilleri */
@media print {
  .workers-container {
    background: white;
  }
  
  .header-actions,
  .search-filters-section,
  .pagination-section,
  .worker-actions,
  .table-actions {
    display: none;
  }
  
  .workers-grid {
    grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
    gap: 1rem;
  }
  
  .worker-card {
    break-inside: avoid;
    box-shadow: none;
    border: 1px solid #ccc;
  }
}

/* Dark Mode Desteği */
@media (prefers-color-scheme: dark) {
  .workers-container {
    background: #1a1a1a;
    color: #e9ecef;
  }
  
  .page-header,
  .search-filters-card,
  .workers-grid-section,
  .worker-card {
    background: #2d2d2d;
    border-color: #404040;
  }
  
  .form-control {
    background: #404040;
    border-color: #555;
    color: #e9ecef;
  }
  
  .btn-secondary {
    background: #404040;
  }
  
  .modal-overlay {
    background: rgba(0,0,0,0.9);
  }
  
  .worker-modal,
  .detail-modal {
    background: #2d2d2d;
    color: #e9ecef;
  }
}

/* Accessibility */
@media (prefers-reduced-motion: reduce) {
  * {
    animation-duration: 0.01ms !important;
    animation-iteration-count: 1 !important;
    transition-duration: 0.01ms !important;
  }
}

/* Focus Stilleri */
.btn:focus,
.form-control:focus,
.pagination-btn:focus,
.page-btn:focus {
  outline: 2px solid #007bff;
  outline-offset: 2px;
}

/* Yüksek Kontrast Desteği */
@media (prefers-contrast: high) {
  .worker-card {
    border: 2px solid #000;
  }
  
  .btn {
    border: 2px solid currentColor;
  }
  
  .form-control {
    border: 2px solid #000;
  }
}
</style>

