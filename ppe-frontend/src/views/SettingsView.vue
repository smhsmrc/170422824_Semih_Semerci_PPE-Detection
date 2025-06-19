<template>
  <div class="settings-container">
    <div class="settings-header">
      <h1>⚙️ Sistem Ayarları</h1>
      <p>Uygulama ayarlarını buradan yapılandırabilirsiniz</p>
    </div>

    <div class="settings-grid">
      <!-- Kamera Ayarları -->
      <div class="settings-card">
        <div class="card-header">
          <h3>📹 Kamera Ayarları</h3>
        </div>
        <div class="card-body">
          <div class="form-group">
            <label>Kamera URL:</label>
            <input 
              v-model="settings.camera.url" 
              type="text" 
              placeholder="rtsp://192.168.1.100:554/stream"
            />
            <div class="url-suggestions">
              <small>💡 Test için önerilen URL'ler:</small>
              <div class="suggestion-buttons">
                <button @click="setTestUrl('webcam')" class="suggestion-btn">
                  📱 Bilgisayar Kamerası
                </button>
                <button @click="setTestUrl('demo')" class="suggestion-btn">
                  🎬 Demo Video
                </button>
                <button @click="setTestUrl('public')" class="suggestion-btn">
                  🌐 Açık Kamera
                </button>
                <button @click="setTestUrl('rtsp')" class="suggestion-btn">
                  📹 RTSP Test
                </button>
              </div>
            </div>
          </div>
          
          <div class="form-group">
            <label>FPS Limiti:</label>
            <input 
              v-model.number="settings.camera.fps" 
              type="number" 
              min="1" 
              max="60"
            />
          </div>
          
          <div class="form-group">
            <label>Çözünürlük:</label>
            <select v-model="settings.camera.resolution">
              <option value="640x480">640x480</option>
              <option value="1280x720">1280x720 (HD)</option>
              <option value="1920x1080">1920x1080 (Full HD)</option>
            </select>
          </div>
          
          <div class="form-group">
            <label>Kamera Durumu:</label>
            <div class="camera-status">
              <span class="status-badge" :class="{ 
                'active': cameraStatus === 'connected',
                'error': cameraStatus === 'error',
                'testing': cameraStatus === 'testing'
              }">
                {{ getCameraStatusText() }}
              </span>
              <button @click="testCameraConnection" class="test-btn" :disabled="!settings.camera.url">
                🔍 Kamera Bağlantısını Test Et
              </button>
              <button @click="startCameraStream" class="stream-btn" :disabled="cameraStatus !== 'connected'">
                📹 Canlı Yayın Başlat
              </button>
            </div>
          </div>
          
          <!-- Kamera Önizleme Alanı -->
          <div v-if="showCameraPreview" class="camera-preview-section">
            <div class="camera-preview-header">
              <h4>📹 Kamera Önizlemesi</h4>
              <button @click="closeCameraPreview" class="close-preview-btn">✕</button>
            </div>
            <div class="camera-preview-container">
              <div v-if="cameraStreamActive" class="camera-stream">
                <video 
                  ref="previewVideo" 
                  autoplay 
                  muted 
                  playsinline
                  class="preview-video"
                  @loadedmetadata="onVideoLoaded"
                  @error="onVideoError"
                ></video>
                <div class="stream-info">
                  <span class="stream-status">🔴 CANLI</span>
                  <span class="stream-url">{{ settings.camera.url }}</span>
                </div>
              </div>
              <div v-else class="camera-loading">
                <div class="loading-spinner"></div>
                <p>Kamera akışı başlatılıyor...</p>
              </div>
            </div>
            <div class="camera-preview-controls">
              <button @click="stopCameraStream" class="stop-stream-btn">
                ⏹️ Akışı Durdur
              </button>
              <button @click="takeSnapshot" class="snapshot-btn">
                📸 Anlık Görüntü Al
              </button>
            </div>
          </div>
        </div>
      </div>

      <!-- E-posta Ayarları -->
      <div class="settings-card">
        <div class="card-header">
          <h3>📧 E-posta Bildirimleri</h3>
        </div>
        <div class="card-body">
          <div class="form-group checkbox-group">
            <label class="checkbox-label">
              <input 
                v-model="settings.email.enabled" 
                type="checkbox"
                @change="updateMailToggle"
              />
              <span class="checkmark"></span>
              E-posta bildirimlerini etkinleştir
            </label>
          </div>
          
          <template v-if="settings.email.enabled">
            <div class="form-group">
              <label>Gönderen E-posta:</label>
              <input 
                v-model="settings.email.senderEmail" 
                type="email" 
                placeholder="sistem@sirket.com"
                readonly
              />
              <small>Sistem tarafından otomatik olarak ayarlanmıştır</small>
            </div>
            
            <div class="form-group">
              <label>Alıcı E-posta Adresi:</label>
              <div class="email-input-group">
                <input 
                  v-model="settings.email.recipientEmail" 
                  type="email" 
                  placeholder="admin@sirket.com"
                  class="email-input"
                  @input="onEmailInputChange"
                />
                <button @click="updateMailRecipient" class="update-btn">
                  Güncelle
                </button>
              </div>
              <small class="input-help">E-posta adresi otomatik olarak kaydedilir</small>
            </div>
            
            <div class="form-group">
              <label>Mail Durumu:</label>
              <div class="status-info">
                <span class="status-badge" :class="{ 'active': settings.email.enabled }">
                  {{ settings.email.enabled ? 'Aktif' : 'Devre Dışı' }}
                </span>
                <span class="recipient-info">
                  Alıcı: {{ settings.email.recipientEmail || 'Belirtilmemiş' }}
                </span>
              </div>
            </div>
            
            <div class="form-group">
              <button @click="sendTestMail" class="test-mail-btn" :disabled="!settings.email.recipientEmail">
                📧 Test Maili Gönder
              </button>
            </div>
          </template>
        </div>
      </div>

      <!-- Genel Ayarlar -->
      <div class="settings-card">
        <div class="card-header">
          <h3>🔧 Genel Ayarlar</h3>
        </div>
        <div class="card-body">
          <div class="form-group">
            <label>Kayıt Saklama Süresi (gün):</label>
            <input 
              v-model.number="settings.general.retentionDays" 
              type="number" 
              min="1" 
              max="365"
            />
          </div>
          
          <div class="form-group">
            <label>Otomatik Yedekleme:</label>
            <select v-model="settings.general.autoBackup">
              <option value="disabled">Devre Dışı</option>
              <option value="daily">Günlük</option>
              <option value="weekly">Haftalık</option>
              <option value="monthly">Aylık</option>
            </select>
          </div>
          
          <div class="form-group checkbox-group">
            <label class="checkbox-label">
              <input 
                v-model="settings.general.enableLogging" 
                type="checkbox"
              />
              <span class="checkmark"></span>
              Detaylı log kaydı
            </label>
          </div>
          
          <div class="form-group checkbox-group">
            <label class="checkbox-label">
              <input 
                v-model="settings.general.enableNotifications" 
                type="checkbox"
              />
              <span class="checkmark"></span>
              Sistem bildirimleri
            </label>
          </div>
        </div>
      </div>
    </div>

    <!-- Kaydet Butonu -->
    <div class="settings-footer">
      <div class="button-group">
        <button @click="resetSettings" class="btn btn-secondary">
          🔄 Varsayılana Sıfırla
        </button>
        <button @click="saveSettings" class="btn btn-primary" :disabled="isSaving">
          <span v-if="isSaving">💾 Kaydediliyor...</span>
          <span v-else>💾 Ayarları Kaydet</span>
        </button>
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
  name: 'SettingsView',
  data() {
    return {
      isSaving: false,
      message: '',
      messageType: 'success',
      emailSaveTimeout: null,
      settings: {
        camera: {
          url: 'rtsp://192.168.1.100:554/stream',
          fps: 30,
          resolution: '1280x720'
        },
        email: {
          enabled: false,
          smtpServer: 'smtp.gmail.com',
          smtpPort: 587,
          senderEmail: 'semihsemerci45@gmail.com',
          senderPassword: '',
          recipientEmail: ''
        },
        general: {
          retentionDays: 30,
          autoBackup: 'weekly',
          enableLogging: true,
          enableNotifications: true
        }
      },
      cameraStatus: 'disconnected',
      cameraPreview: null,
      showCameraPreview: false,
      cameraStreamActive: false,
      previewVideo: null
    }
  },
  mounted() {
    this.loadSettings()
  },
  beforeDestroy() {
    // Timeout'u temizle
    if (this.emailSaveTimeout) {
      clearTimeout(this.emailSaveTimeout)
    }
  },
  methods: {
    async loadSettings() {
      try {
        // Önce localStorage'dan e-posta ayarlarını yükle
        this.loadEmailSettingsFromStorage()
        
        // Load general settings
        const response = await fetch('http://localhost:5001/api/ppe/settings')
        if (response.ok) {
          const data = await response.json()
          this.settings = { ...this.settings, ...data }
        }
        
        // Load mail settings from backend (localStorage'daki değerler varsa onları korur)
        await this.loadMailSettings()
      } catch (error) {
        console.error('Ayarlar yüklenemedi:', error)
      }
    },
    
    loadEmailSettingsFromStorage() {
      try {
        const savedEmailSettings = localStorage.getItem('ppeEmailSettings')
        if (savedEmailSettings) {
          const emailData = JSON.parse(savedEmailSettings)
          this.settings.email = { ...this.settings.email, ...emailData }
          console.log('E-posta ayarları localStorage\'dan yüklendi:', emailData)
        }
      } catch (error) {
        console.error('localStorage\'dan e-posta ayarları yüklenemedi:', error)
      }
    },
    
    saveEmailSettingsToStorage() {
      try {
        const emailSettings = {
          enabled: this.settings.email.enabled,
          recipientEmail: this.settings.email.recipientEmail,
          senderEmail: this.settings.email.senderEmail
        }
        localStorage.setItem('ppeEmailSettings', JSON.stringify(emailSettings))
        console.log('E-posta ayarları localStorage\'a kaydedildi:', emailSettings)
      } catch (error) {
        console.error('E-posta ayarları localStorage\'a kaydedilemedi:', error)
      }
    },

    onEmailInputChange() {
      // E-posta adresi değiştiğinde localStorage'a kaydet (debounce ile)
      clearTimeout(this.emailSaveTimeout)
      this.emailSaveTimeout = setTimeout(() => {
        this.saveEmailSettingsToStorage()
      }, 1000) // 1 saniye bekle
    },
    
    async loadMailSettings() {
      try {
        const response = await fetch('http://localhost:5001/api/ppe/mail/status')
        if (response.ok) {
          const data = await response.json()
          // Sadece localStorage'da değer yoksa backend'den al
          if (!localStorage.getItem('ppeEmailSettings')) {
            this.settings.email.enabled = data.autoMailEnabled || false
            this.settings.email.recipientEmail = data.recipientEmail || ''
          }
        }
      } catch (error) {
        console.error('Mail ayarları yüklenemedi:', error)
      }
    },
    
    async saveSettings() {
      this.isSaving = true
      this.message = ''
      
      try {
        const response = await fetch('http://localhost:5001/api/ppe/settings', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify(this.settings)
        })
        
        if (response.ok) {
          this.showMessage('Ayarlar başarıyla kaydedildi! 🎉', 'success')
        } else {
          this.showMessage('Ayarlar kaydedilirken hata oluştu! ❌', 'error')
        }
      } catch (error) {
        console.error('Ayar kaydetme hatası:', error)
        this.showMessage('Bağlantı hatası! Lütfen tekrar deneyin. 🔄', 'error')
      } finally {
        this.isSaving = false
      }
    },
    
    resetSettings() {
      if (confirm('Tüm ayarları varsayılan değerlere sıfırlamak istediğinizden emin misiniz?')) {
        // localStorage'ı temizle
        try {
          localStorage.removeItem('ppeEmailSettings')
          console.log('E-posta ayarları localStorage\'dan temizlendi')
        } catch (error) {
          console.error('localStorage temizleme hatası:', error)
        }
        
        // Varsayılan ayarları yükle
        this.settings = {
          camera: {
            url: 'rtsp://192.168.1.100:554/stream',
            fps: 30,
            resolution: '1280x720'
          },
          email: {
            enabled: false,
            smtpServer: 'smtp.gmail.com',
            smtpPort: 587,
            senderEmail: 'semihsemerci45@gmail.com',
            senderPassword: '',
            recipientEmail: ''
          },
          general: {
            retentionDays: 30,
            autoBackup: 'weekly',
            enableLogging: true,
            enableNotifications: true
          }
        }
        this.showMessage('Ayarlar varsayılan değerlere sıfırlandı! 🔄', 'success')
      }
    },
    
    showMessage(text, type) {
      this.message = text
      this.messageType = type
      setTimeout(() => {
        this.message = ''
      }, 3000)
    },

    async updateMailToggle() {
      try {
        const response = await fetch('http://localhost:5001/api/ppe/mail/toggle', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({ enabled: this.settings.email.enabled })
        })
        
        if (response.ok) {
          // Başarılı olursa localStorage'a kaydet
          this.saveEmailSettingsToStorage()
          this.showMessage('Mail ayarı güncellendi! ✅', 'success')
        } else {
          this.showMessage('Mail ayarı güncellenemedi! ❌', 'error')
          // Hata durumunda eski duruma geri döndür
          this.settings.email.enabled = !this.settings.email.enabled
        }
      } catch (error) {
        console.error('Mail toggle hatası:', error)
        this.showMessage('Bağlantı hatası! 🔄', 'error')
        this.settings.email.enabled = !this.settings.email.enabled
      }
    },

    async updateMailRecipient() {
      try {
        // E-posta adresi boş kontrolü
        if (!this.settings.email.recipientEmail || this.settings.email.recipientEmail.trim() === '') {
          this.showMessage('Lütfen geçerli bir e-posta adresi girin! ❌', 'error')
          return
        }

        const response = await fetch('http://localhost:5001/api/ppe/mail/set-recipient', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({ email: this.settings.email.recipientEmail })
        })
        
        if (response.ok) {
          // Başarılı olursa localStorage'a kaydet
          this.saveEmailSettingsToStorage()
          this.showMessage('E-posta adresi güncellendi! ✅', 'success')
        } else {
          this.showMessage('E-posta adresi güncellenemedi! ❌', 'error')
        }
      } catch (error) {
        console.error('Mail recipient hatası:', error)
        this.showMessage('Bağlantı hatası! 🔄', 'error')
      }
    },

    async sendTestMail() {
      try {
        this.showMessage('Test maili gönderiliyor... ⏳', 'success')
        
        const response = await fetch('http://localhost:5001/api/ppe/mail/send', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          }
        })
        
        if (response.ok) {
          this.showMessage('Test maili başarıyla gönderildi! 📧✅', 'success')
        } else {
          this.showMessage('Test maili gönderilemedi! ❌', 'error')
        }
      } catch (error) {
        console.error('Test mail hatası:', error)
        this.showMessage('Test maili gönderilirken hata oluştu! 🔄', 'error')
      }
    },

    async testCameraConnection() {
      this.cameraStatus = 'testing'
      try {
        const response = await fetch('http://localhost:5001/api/ppe/camera/test', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({
            url: this.settings.camera.url
          })
        })
        
        if (response.ok) {
          const result = await response.json()
          this.showMessage('Kamera bağlantısı başarılı! ✅', 'success')
          this.cameraStatus = 'connected'
        } else {
          const error = await response.json()
          this.showMessage(`Kamera bağlantısı başarısız: ${error.message} ❌`, 'error')
          this.cameraStatus = 'error'
        }
      } catch (error) {
        console.error('Kamera bağlantısı test hatası:', error)
        this.showMessage('Kamera bağlantısı test edilirken hata oluştu! 🔄', 'error')
        this.cameraStatus = 'error'
      }
    },

    async startCameraStream() {
      try {
        this.showMessage('Kamera akışı başlatılıyor... ⏳', 'success')
        this.showCameraPreview = true
        
        // URL kontrolü
        if (!this.settings.camera.url || this.settings.camera.url.trim() === '') {
          this.showMessage('Lütfen önce kamera URL\'si girin! 📹❌', 'error')
          this.showCameraPreview = false
          return
        }

        console.log('Kamera akışı başlatılıyor, URL:', this.settings.camera.url)
        
        // Backend'e kamera başlatma isteği gönder (opsiyonel)
        try {
          const response = await fetch('http://localhost:5001/api/ppe/camera/start', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/json'
            },
            body: JSON.stringify({
              url: this.settings.camera.url,
              fps: this.settings.camera.fps,
              resolution: this.settings.camera.resolution
            })
          })
          
          if (response.ok) {
            console.log('Backend kamera başlatma başarılı')
          }
        } catch (backendError) {
          console.warn('Backend kamera başlatma hatası (devam ediliyor):', backendError)
        }
        
        // Direkt video akışını başlat
        this.cameraStatus = 'connected'
        this.cameraStreamActive = true
        
        // Kamera akışını video elementine bağla
        await this.connectVideoStream()
        
      } catch (error) {
        console.error('Kamera akışı başlatma hatası:', error)
        this.showMessage('Kamera akışı başlatılırken hata oluştu! 🔄', 'error')
        this.cameraStatus = 'error'
        this.showCameraPreview = false
      }
    },

    stopCameraStream() {
      try {
        // Video elementini al ve durdur
        const video = this.$refs.previewVideo
        if (video) {
          if (video.srcObject) {
            // Webcam stream'i durdur
            const tracks = video.srcObject.getTracks()
            tracks.forEach(track => track.stop())
            video.srcObject = null
          } else {
            // Normal video src'yi temizle
            video.src = ''
          }
          video.pause()
        }
        
        this.cameraStreamActive = false
        this.showCameraPreview = false
        this.cameraStatus = 'disconnected'
        this.showMessage('Kamera akışı durduruldu! ⏹️', 'success')
      } catch (error) {
        console.error('Kamera durdurma hatası:', error)
        this.showMessage('Kamera durdurulamadı! ❌', 'error')
      }
    },

    takeSnapshot() {
      // Implement snapshot taking logic
      this.showMessage('Anlık görüntü alınıyor... ⏳', 'success')
    },

    onVideoLoaded(event) {
      // Implement video loaded metadata logic
      this.showMessage('Kamera önizleme başarılı! 🎥✅', 'success')
    },

    onVideoError(event) {
      this.showMessage('Kamera önizleme hatası! ❌', 'error')
    },

    getCameraStatusText() {
      switch (this.cameraStatus) {
        case 'connected':
          return 'Bağlandı'
        case 'error':
          return 'Bağlantı Hatası'
        case 'testing':
          return 'Test Ediliyor...'
        case 'disconnected':
          return 'Bağlantı Yok'
        default:
          return 'Bilinmeyen Durum'
      }
    },

         setTestUrl(type) {
       const testUrls = {
         webcam: 'webcam', // Özel anahtar kelime
         demo: 'https://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4',
         public: 'https://demo.unified-streaming.com/k8s/features/stable/video/tears-of-steel/tears-of-steel.ism/.m3u8',
         rtsp: 'https://sample-videos.com/zip/10/mp4/SampleVideo_1280x720_1mb.mp4'
       }
       
       this.settings.camera.url = testUrls[type] || this.settings.camera.url
       this.showMessage(`Test URL ayarlandı: ${type.toUpperCase()} 📹`, 'success')
       console.log('Test URL ayarlandı:', this.settings.camera.url)
     },

         closeCameraPreview() {
       // Önce stream'i durdur
       this.stopCameraStream()
       this.showMessage('Kamera önizleme kapatıldı! ✕', 'success')
     },

     async connectVideoStream() {
       try {
         // Video elementini al
         const video = this.$refs.previewVideo
         if (!video) {
           console.error('Video elementi bulunamadı')
           return
         }

         console.log('Video akışı bağlanıyor, URL:', this.settings.camera.url)

         // Farklı URL tiplerini kontrol et
         if (this.settings.camera.url.includes('webcam')) {
           // Bilgisayar kamerası için getUserMedia kullan
           try {
             const stream = await navigator.mediaDevices.getUserMedia({ 
               video: { 
                 width: { ideal: 1280 }, 
                 height: { ideal: 720 } 
               } 
             })
             video.srcObject = stream
             console.log('Webcam stream başarıyla bağlandı')
           } catch (webcamError) {
             console.error('Webcam erişim hatası:', webcamError)
             this.showMessage('Kameraya erişim izni gerekli! 📹❌', 'error')
             return
           }
         } else {
           // Diğer URL'ler için direkt src kullan
           video.src = this.settings.camera.url
           video.crossOrigin = 'anonymous' // CORS sorunları için
           console.log('Video src ayarlandı:', this.settings.camera.url)
         }

         // Video yüklenme olaylarını dinle
         video.onloadedmetadata = () => {
           console.log('Video metadata yüklendi')
           this.showMessage('Video akışı başarıyla bağlandı! 📹', 'success')
         }

         video.onerror = (error) => {
           console.error('Video yükleme hatası:', error)
           this.showMessage('Video yüklenemedi! URL\'yi kontrol edin ❌', 'error')
           this.cameraStreamActive = false
         }

         await video.play()
         console.log('Video oynatma başlatıldı')
       } catch (error) {
         console.error('Video akışı bağlantı hatası:', error)
         this.showMessage('Video akışı bağlanamadı! ❌', 'error')
         this.cameraStreamActive = false
       }
     }
  }
}
</script>

<style scoped>
.settings-container {
  padding: 2rem;
  max-width: 1200px;
  margin: 0 auto;
  background: #f8f9fa;
  min-height: 100vh;
}

.settings-header {
  text-align: center;
  margin-bottom: 2rem;
}

.settings-header h1 {
  color: #2c3e50;
  margin-bottom: 0.5rem;
}

.settings-header p {
  color: #6c757d;
}

.settings-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(400px, 1fr));
  gap: 1.5rem;
  margin-bottom: 2rem;
}

.settings-card {
  background: white;
  border-radius: 12px;
  box-shadow: 0 4px 6px rgba(0,0,0,0.1);
  overflow: hidden;
}

.card-header {
  background: #3498db;
  color: white;
  padding: 1rem;
}

.card-header h3 {
  margin: 0;
  font-size: 1.1rem;
}

.card-body {
  padding: 1.5rem;
}

.form-group {
  margin-bottom: 1.5rem;
}

.form-group label {
  display: block;
  margin-bottom: 0.5rem;
  font-weight: 600;
  color: #2c3e50;
}

.form-group input,
.form-group select,
.form-group textarea {
  width: 100%;
  padding: 0.75rem;
  border: 2px solid #e9ecef;
  border-radius: 8px;
  font-size: 1rem;
  transition: border-color 0.3s;
}

.form-group input:focus,
.form-group select:focus,
.form-group textarea:focus {
  outline: none;
  border-color: #3498db;
}

.form-group small {
  display: block;
  margin-top: 0.25rem;
  color: #6c757d;
  font-size: 0.875rem;
}

.checkbox-group {
  display: flex;
  align-items: center;
}

.checkbox-label {
  display: flex;
  align-items: center;
  cursor: pointer;
  font-weight: 500;
}

.checkbox-label input[type="checkbox"] {
  width: auto;
  margin-right: 0.5rem;
}

.settings-footer {
  text-align: center;
  margin-top: 2rem;
}

.button-group {
  display: flex;
  gap: 1rem;
  justify-content: center;
}

.btn {
  padding: 0.75rem 2rem;
  border: none;
  border-radius: 8px;
  font-size: 1rem;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.3s;
}

.btn-primary {
  background: #28a745;
  color: white;
}

.btn-primary:hover:not(:disabled) {
  background: #218838;
}

.btn-primary:disabled {
  background: #6c757d;
  cursor: not-allowed;
}

.btn-secondary {
  background: #6c757d;
  color: white;
}

.btn-secondary:hover {
  background: #5a6268;
}

.message {
  position: fixed;
  top: 20px;
  right: 20px;
  padding: 1rem 1.5rem;
  border-radius: 8px;
  font-weight: 600;
  z-index: 1000;
  animation: slideIn 0.3s ease-out;
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

@media (max-width: 768px) {
  .settings-grid {
    grid-template-columns: 1fr;
  }
  
  .button-group {
    flex-direction: column;
  }
  
  .settings-container {
    padding: 1rem;
  }
}

.email-input-group {
  display: flex;
  align-items: center;
  gap: 0.5rem;
}

.email-input {
  flex: 1;
  padding: 0.75rem;
  border: 2px solid #e9ecef;
  border-radius: 8px;
  font-size: 1rem;
  transition: border-color 0.3s;
}

.email-input:focus {
  outline: none;
  border-color: #3498db;
}

.update-btn {
  padding: 0.75rem 1.5rem;
  border: none;
  border-radius: 8px;
  background: #28a745;
  color: white;
  cursor: pointer;
  font-weight: 600;
  transition: background 0.3s;
  white-space: nowrap;
}

.update-btn:hover {
  background: #218838;
}

.status-info {
  display: flex;
  align-items: center;
  gap: 1rem;
  padding: 0.75rem;
  background: #f8f9fa;
  border-radius: 8px;
  border: 1px solid #e9ecef;
}

.status-badge {
  padding: 0.25rem 0.75rem;
  border-radius: 20px;
  font-size: 0.875rem;
  font-weight: 600;
  background: #6c757d;
  color: white;
}

.status-badge.active {
  background: #28a745;
}

.recipient-info {
  color: #6c757d;
  font-size: 0.875rem;
}

.test-mail-btn {
  background: #17a2b8;
  color: white;
  border: none;
  padding: 0.75rem 1.5rem;
  border-radius: 8px;
  cursor: pointer;
  font-weight: 600;
  transition: background 0.3s;
}

.test-mail-btn:hover:not(:disabled) {
  background: #138496;
}

.test-mail-btn:disabled {
  background: #6c757d;
  cursor: not-allowed;
}

.camera-status {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  padding: 0.75rem;
  background: #f8f9fa;
  border-radius: 8px;
  border: 1px solid #e9ecef;
}

.test-btn {
  background: #17a2b8;
  color: white;
  border: none;
  padding: 0.75rem 1.5rem;
  border-radius: 8px;
  cursor: pointer;
  font-weight: 600;
  transition: background 0.3s;
}

.test-btn:hover:not(:disabled) {
  background: #138496;
}

.test-btn:disabled {
  background: #6c757d;
  cursor: not-allowed;
}

.stream-btn {
  background: #28a745;
  color: white;
  border: none;
  padding: 0.75rem 1.5rem;
  border-radius: 8px;
  cursor: pointer;
  font-weight: 600;
  transition: background 0.3s;
}

.stream-btn:hover:not(:disabled) {
  background: #218838;
}

.stream-btn:disabled {
  background: #6c757d;
  cursor: not-allowed;
}

.camera-preview-section {
  margin-top: 1.5rem;
  padding: 1.5rem;
  background: white;
  border-radius: 12px;
  box-shadow: 0 4px 6px rgba(0,0,0,0.1);
}

.camera-preview-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 1rem;
}

.camera-preview-header h4 {
  margin: 0;
  font-size: 1.1rem;
  font-weight: 600;
  color: #2c3e50;
}

.close-preview-btn {
  background: #6c757d;
  color: white;
  border: none;
  padding: 0.25rem 0.5rem;
  border-radius: 4px;
  cursor: pointer;
  font-weight: 600;
  transition: background 0.3s;
}

.close-preview-btn:hover {
  background: #5a6268;
}

.camera-preview-container {
  position: relative;
  padding-bottom: 56.25%; /* 16:9 aspect ratio */
}

.camera-stream {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  border-radius: 8px;
}

.preview-video {
  width: 100%;
  height: 100%;
  border-radius: 8px;
  object-fit: cover;
}

.camera-loading {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 1.5rem;
  background: #f8f9fa;
  border-radius: 8px;
  border: 1px solid #e9ecef;
}

.loading-spinner {
  width: 40px;
  height: 40px;
  border: 4px solid #3498db;
  border-radius: 50%;
  border-top-color: transparent;
  animation: spin 1s linear infinite;
  margin-bottom: 1rem;
}

@keyframes spin {
  0% {
    transform: rotate(0deg);
  }
  100% {
    transform: rotate(360deg);
  }
}

.stream-info {
  position: absolute;
  bottom: 0;
  left: 0;
  right: 0;
  padding: 0.5rem;
  background: rgba(0, 0, 0, 0.5);
  border-radius: 0 0 8px 8px;
  color: white;
  font-size: 0.875rem;
}

.camera-preview-controls {
  display: flex;
  justify-content: space-between;
  margin-top: 1rem;
}

.stop-stream-btn {
  background: #dc3545;
  color: white;
  border: none;
  padding: 0.75rem 1.5rem;
  border-radius: 8px;
  cursor: pointer;
  font-weight: 600;
  transition: background 0.3s;
}

.stop-stream-btn:hover:not(:disabled) {
  background: #c82333;
}

.stop-stream-btn:disabled {
  background: #6c757d;
  cursor: not-allowed;
}

.snapshot-btn {
  background: #28a745;
  color: white;
  border: none;
  padding: 0.75rem 1.5rem;
  border-radius: 8px;
  cursor: pointer;
  font-weight: 600;
  transition: background 0.3s;
}

.snapshot-btn:hover:not(:disabled) {
  background: #218838;
}

.snapshot-btn:disabled {
  background: #6c757d;
  cursor: not-allowed;
}

.url-suggestions {
  margin-top: 0.5rem;
  margin-bottom: 0.5rem;
}

.suggestion-buttons {
  display: flex;
  gap: 0.5rem;
}

.suggestion-btn {
  padding: 0.5rem 1rem;
  border: none;
  border-radius: 8px;
  background: #3498db;
  color: white;
  cursor: pointer;
  font-weight: 600;
  transition: background 0.3s;
}

.suggestion-btn:hover {
  background: #2980b9;
}

.input-help {
  display: block;
  margin-top: 0.25rem;
  font-size: 0.875rem;
  color: #6c757d;
  font-style: italic;
}
</style>
