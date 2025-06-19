<template>
  <div class="camera-stream-container">
    <!-- Kamera Görüntüsü -->
    <div class="camera-display">
      <video 
        ref="videoElement"
        :class="{ 'hidden': !isStreaming }"
        autoplay 
        muted 
        playsinline
        @loadedmetadata="onVideoLoaded"
        @error="onVideoError"
      ></video>
      
      <canvas 
        ref="canvasElement"
        :class="{ 'hidden': !isStreaming }"
        class="detection-canvas"
        @click="onCanvasClick"
      ></canvas>
      
      <!-- Bekleme Durumu -->
      <div v-if="!isStreaming" class="camera-placeholder">
        <div class="camera-icon">
          <i class="fas fa-camera"></i>
        </div>
        <h3>{{ cameraStatus }}</h3>
        <p>{{ statusMessage }}</p>
      </div>

      <!-- Tespit Overlay -->
      <div v-if="isStreaming && currentDetections.length > 0" class="detection-overlay">
        <div class="detection-count">
          {{ currentDetections.length }} kişi tespit edildi
        </div>
        <div v-if="violationCount > 0" class="violation-alert">
          ⚠️ {{ violationCount }} ihlal tespit edildi!
        </div>
      </div>
    </div>

    <!-- Kontrol Butonları -->
    <div class="camera-controls">
      <button 
        @click="toggleCamera" 
        :class="['btn', isStreaming ? 'btn-danger' : 'btn-success']"
        :disabled="isLoading"
      >
        <i :class="isStreaming ? 'fas fa-stop' : 'fas fa-play'"></i>
        {{ isStreaming ? 'Durdur' : 'Başlat' }}
      </button>
      
      <button 
        @click="toggleDetection" 
        :class="['btn', 'btn-warning']"
        :disabled="!isStreaming"
      >
        <i :class="isDetecting ? 'fas fa-eye-slash' : 'fas fa-eye'"></i>
        {{ isDetecting ? 'Tespiti Durdur' : 'Tespit Başlat' }}
      </button>

      <button 
        @click="takeSnapshot" 
        :class="['btn', 'btn-info']"
        :disabled="!isStreaming"
      >
        <i class="fas fa-camera"></i>
        Fotoğraf Çek
      </button>

      <button 
        @click="toggleRecording" 
        :class="['btn', isRecording ? 'btn-danger' : 'btn-primary']"
        :disabled="!isStreaming"
      >
        <i :class="isRecording ? 'fas fa-stop-circle' : 'fas fa-record-vinyl'"></i>
        {{ isRecording ? 'Kaydı Durdur' : 'Kayıt Başlat' }}
      </button>
    </div>

    <!-- Tespit Bilgileri -->
    <div v-if="isDetecting" class="detection-info">
      <div class="detection-stats">
        <div class="stat-item">
          <span class="stat-label">FPS:</span>
          <span class="stat-value">{{ currentFPS }}</span>
        </div>
        <div class="stat-item">
          <span class="stat-label">Çözünürlük:</span>
          <span class="stat-value">{{ resolution }}</span>
        </div>
        <div class="stat-item">
          <span class="stat-label">Son Tespit:</span>
          <span class="stat-value">{{ lastDetectionTime }}</span>
        </div>
        <div class="stat-item">
          <span class="stat-label">Toplam Tespit:</span>
          <span class="stat-value">{{ totalDetections }}</span>
        </div>
      </div>
    </div>

    <!-- Ayarlar -->
    <div class="camera-settings">
      <div class="settings-row">
        <label>🎯 Tespit Hassasiyeti:</label>
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
      
      <div class="settings-row">
        <label>🔊 Sesli Uyarı:</label>
        <label class="switch">
          <input v-model="audioAlerts" @change="updateSettings" type="checkbox">
          <span class="slider"></span>
        </label>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'CameraStream',
  props: {
    cameraId: {
      type: String,
      default: 'default'
    },
    streamUrl: {
      type: String,
      default: ''
    },
    autoStart: {
      type: Boolean,
      default: false
    }
  },
  
  data() {
    return {
      isStreaming: false,
      isDetecting: false,
      isRecording: false,
      isLoading: false,
      cameraStatus: 'Kamera Bekleniyor',
      statusMessage: 'Tespit başlatmak için "Başlat" butonuna tıklayın',
      currentFPS: 0,
      resolution: '0x0',
      lastDetectionTime: 'Henüz tespit yapılmadı',
      totalDetections: 0,
      
      // Tespit verileri
      currentDetections: [],
      violationCount: 0,
      
      // Ayarlar
      detectionSensitivity: 0.7,
      audioAlerts: true,
      
      // Kamera ve Canvas referansları
      stream: null,
      detectionInterval: null,
      fpsInterval: null,
      frameCount: 0,
      
      // Simülasyon verileri
      simulatedWorkers: [
        { id: 'W001', name: 'Ahmet Yılmaz' },
        { id: 'W002', name: 'Mehmet Kaya' },
        { id: 'W003', name: 'Fatma Demir' },
        { id: 'W004', name: 'Ali Çelik' },
        { id: 'W005', name: 'Ayşe Şahin' }
      ],
      
      equipmentTypes: ['Baret', 'Maske', 'Eldiven', 'Yelek']
    }
  },
  
  mounted() {
    if (this.autoStart) {
      this.startCamera()
    }
  },
  
  beforeUnmount() {
    this.stopCamera()
  },
  
  methods: {
    // Backend bağlantısını kontrol et
    async checkBackendConnection() {
      try {
        const response = await fetch('http://localhost:8080/health');
        const data = await response.json();
        
        if (data.status === 'healthy') {
          console.log('✅ C++ Backend bağlantısı başarılı');
          this.cameraStatus = 'Backend Bağlandı';
          this.statusMessage = 'C++ detection servisi aktif';
        }
      } catch (error) {
        console.error('❌ C++ Backend bağlantı hatası:', error);
        this.cameraStatus = 'Backend Hatası';
        this.statusMessage = 'C++ detection servisi bulunamadı (Port: 8080)';
      }
    },

    async startCamera() {
      this.isLoading = true;
      this.cameraStatus = 'Kamera Başlatılıyor...';
      
      try {
        // C++ backend'den stream al
        const streamResponse = await fetch('http://localhost:8080/camera/status');
        const streamData = await streamResponse.json();
        
        if (streamData.connected) {
          // C++ backend kamerasını kullan
          this.$refs.videoElement.src = 'http://localhost:8080/api/ppe/camera/stream';
          this.onVideoLoaded();
        } else {
          // Fallback: Browser kamerasını kullan
          this.stream = await navigator.mediaDevices.getUserMedia({
            video: {
              width: { ideal: 1280 },
              height: { ideal: 720 },
              facingMode: 'environment'
            },
            audio: false
          });
          this.$refs.videoElement.srcObject = this.stream;
        }
        
      } catch (error) {
        console.error('Kamera başlatma hatası:', error);
        this.isLoading = false;
        this.cameraStatus = 'Kamera Hatası';
        this.statusMessage = 'Kamera erişim izni gerekli veya backend bulunamadı';
        this.$emit('camera-error', error);
      }
    },

    async toggleDetection() {
      if (this.isDetecting) {
        // Detection'ı durdur
        await fetch('http://localhost:8080/ppe/stop', { method: 'POST' });
        this.stopDetection();
      } else {
        // Detection'ı başlat
        await fetch('http://localhost:8080/ppe/start', { method: 'POST' });
        this.startDetection();
      }
    },

    async processFrame() {
        if (!this.isDetecting || !this.isStreaming) return;

        try {
            // Canvas'a video frame'ini çiz
            const canvas = this.$refs.canvasElement;
            const video = this.$refs.videoElement;
            const ctx = canvas.getContext('2d');
            
            canvas.width = video.videoWidth;
            canvas.height = video.videoHeight;
            ctx.drawImage(video, 0, 0);

            // Canvas'ı blob'a çevir
            canvas.toBlob(async (blob) => {
                if (!blob) return;

                const formData = new FormData();
                formData.append('image', blob, 'frame.jpg');

                try {
                    const response = await fetch('http://localhost:3001/api/detect-ppe', {
                        method: 'POST',
                        body: formData
                    });

                    if (response.ok) {
                        const result = await response.json();
                        this.handleDetectionResult(result);
                    }
                } catch (error) {
                    console.error('Detection error:', error);
                }
            }, 'image/jpeg', 0.8);

        } catch (error) {
            console.error('Frame processing error:', error);
        }
    },

    handleDetectionResult(result) {
        if (result.success && result.detections) {
            this.currentDetections = result.detections;
            this.drawDetections(result.detections);
            
            // İhlal kontrolü
            const violations = result.detections.filter(d => 
                !d.helmet || !d.vest || !d.gloves
            );
            
            if (violations.length > 0) {
                this.violationCount = violations.length;
                this.triggerAlert(violations);
            }
        }
    },

    drawDetections(detections) {
        const canvas = this.$refs.canvasElement;
        const ctx = canvas.getContext('2d');
        
        // Önceki çizimleri temizle
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        
        detections.forEach(detection => {
            // Bounding box çiz
            ctx.strokeStyle = detection.helmet && detection.vest ? '#00ff00' : '#ff0000';
            ctx.lineWidth = 3;
            ctx.strokeRect(detection.x, detection.y, detection.width, detection.height);
            
            // Label çiz
            ctx.fillStyle = detection.helmet && detection.vest ? '#00ff00' : '#ff0000';
            ctx.font = '16px Arial';
            ctx.fillText(
                `Person ${detection.confidence.toFixed(2)}`, 
                detection.x, 
                detection.y - 10
            );
            
            // PPE durumu
            const ppeStatus = [];
                    if (!detection.helmet) ppeStatus.push('Baret');
        if (!detection.mask) ppeStatus.push('Maske');
        if (!detection.gloves) ppeStatus.push('Eldiven');
        if (!detection.vest) ppeStatus.push('Yelek');
            
            if (ppeStatus.length > 0) {
                ctx.fillText(
                    `Eksik: ${ppeStatus.join(', ')}`, 
                    detection.x, 
                    detection.y + detection.height + 20
                );
            }
        });
    },

    // Detection loop'u başlat
    startDetectionLoop() {
        if (this.detectionInterval) {
            clearInterval(this.detectionInterval);
        }
        
        this.detectionInterval = setInterval(() => {
            this.processFrame();
        }, 1000); // 1 saniyede bir
    }
        

    startDetection() {
      this.isDetecting = true;
      
      // C++ backend'den detection sonuçlarını al
      this.detectionInterval = setInterval(async () => {
        try {
          const response = await fetch('http://localhost:8080/ppe/detections');
          const data = await response.json();
          
          if (data.status === 'success') {
            this.currentDetections = data.detections || [];
            this.totalDetections = data.total_detections || 0;
            this.lastDetectionTime = data.last_detection_time || 'Henüz tespit yapılmadı';
            
            // Violation sayısını hesapla
            this.violationCount = this.currentDetections.filter(d => 
              d.type.includes('missing') || d.confidence < 0.5
            ).length;
            
            // Canvas'a detection box'ları çiz
            this.drawDetections();
          }
        } catch (error) {
          console.error('Detection fetch error:', error);
        }
      }, 100); // 10 FPS
    },

    drawDetections() {
      const canvas = this.$refs.canvasElement;
      const video = this.$refs.videoElement;
      
      if (!canvas || !video) return;
      
      const ctx = canvas.getContext('2d');
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      
      // Detection box'larını çiz
      this.currentDetections.forEach(detection => {
        const [x, y, w, h] = detection.bbox;
        
        // Box rengi (ihlal varsa kırmızı, yoksa yeşil)
        const color = detection.type.includes('missing') ? '#ff0000' : '#00ff00';
        
        ctx.strokeStyle = color;
        ctx.lineWidth = 2;
        ctx.strokeRect(x, y, w, h);
        
        // Label
        ctx.fillStyle = color;
        ctx.font = '14px Arial';
        const label = `${detection.type} (${Math.round(detection.confidence * 100)}%)`;
        ctx.fillText(label, x, y - 5);
      });
    },

    onVideoLoaded() {
      this.isStreaming = true
      this.isLoading = false
      this.cameraStatus = 'Kamera Aktif'
      this.statusMessage = 'Tespit için "Tespit Başlat" butonuna tıklayın'
      
      // Canvas boyutlarını ayarla
      this.setupCanvas()
      
      // FPS sayacını başlat
      this.startFPSCounter()
      
      // Çözünürlük bilgisini güncelle
      const video = this.$refs.videoElement
      this.resolution = `${video.videoWidth}x${video.videoHeight}`
      
      // Parent'a bildir
      this.$emit('camera-loaded', {
        resolution: this.resolution,
        cameraId: this.cameraId
      })
      
      console.log('Kamera başarıyla başlatıldı')
    },

    onVideoError(error) {
      console.error('Video hatası:', error)
      this.isLoading = false
      this.cameraStatus = 'Bağlantı Hatası'
      this.statusMessage = 'Video akışı başlatılamadı'
      this.$emit('camera-error', error)
    },

    stopCamera() {
      if (this.stream) {
        this.stream.getTracks().forEach(track => track.stop())
        this.stream = null
      }
      
      // Tüm işlemleri durdur
      this.stopDetection()
      this.stopRecording()
      
      if (this.fpsInterval) {
        clearInterval(this.fpsInterval)
        this.fpsInterval = null
      }
      
      this.isStreaming = false
      this.cameraStatus = 'Kamera Durduruldu'
      this.statusMessage = 'Yeniden başlatmak için "Başlat" butonuna tıklayın'
      this.currentFPS = 0
      this.currentDetections = []
      this.violationCount = 0
      
      this.$emit('camera-stopped')
      console.log('Kamera durduruldu')
    },

    toggleDetection() {
      if (this.isDetecting) {
        this.stopDetection()
      } else {
        this.startDetection()
      }
    },

    startDetection() {
      if (!this.isStreaming) return
      
      this.isDetecting = true
      this.totalDetections = 0
      console.log('PPE tespiti başlatıldı')
      
      // Her 500ms'de bir frame yakala ve analiz et
      this.detectionInterval = setInterval(() => {
        this.captureAndAnalyze()
      }, 500)
      
      this.$emit('detection-started')
    },

    stopDetection() {
      if (this.detectionInterval) {
        clearInterval(this.detectionInterval)
        this.detectionInterval = null
      }
      
      this.isDetecting = false
      this.currentDetections = []
      this.violationCount = 0
      
      // Canvas'ı temizle
      const canvas = this.$refs.canvasElement
      if (canvas) {
        const ctx = canvas.getContext('2d')
        ctx.clearRect(0, 0, canvas.width, canvas.height)
      }
      
      this.$emit('detection-stopped')
      console.log('PPE tespiti durduruldu')
    },

    setupCanvas() {
      const video = this.$refs.videoElement
      const canvas = this.$refs.canvasElement
      
      if (video && canvas) {
        canvas.width = video.videoWidth
        canvas.height = video.videoHeight
        canvas.style.width = video.offsetWidth + 'px'
        canvas.style.height = video.offsetHeight + 'px'
      }
    },

    captureAndAnalyze() {
      const video = this.$refs.videoElement
      const canvas = this.$refs.canvasElement
      const ctx = canvas.getContext('2d')
      
      // Video frame'ini canvas'a çiz (gizli olarak)
      const hiddenCanvas = document.createElement('canvas')
      hiddenCanvas.width = video.videoWidth
      hiddenCanvas.height = video.videoHeight
      const hiddenCtx = hiddenCanvas.getContext('2d')
      hiddenCtx.drawImage(video, 0, 0, hiddenCanvas.width, hiddenCanvas.height)
      
      // Frame'i base64 olarak al
      const imageData = hiddenCanvas.toDataURL('image/jpeg', 0.8)
      
      // Tespit simülasyonu yap
      this.simulateDetection(imageData, ctx)
      
      this.frameCount++
    },

    simulateDetection(imageData, ctx) {
      // Canvas'ı temizle
      ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height)
      
      // Rastgele tespit simülasyonu
      const detectionCount = Math.floor(Math.random() * 4) + 1 // 1-4 kişi
      const newDetections = []
      let violations = 0
      
      for (let i = 0; i < detectionCount; i++) {
        const worker = this.simulatedWorkers[Math.floor(Math.random() * this.simulatedWorkers.length)]
        
        // Rastgele pozisyon
        const x = Math.random() * (ctx.canvas.width - 200)
        const y = Math.random() * (ctx.canvas.height - 300)
        const width = 150 + Math.random() * 100
        const height = 200 + Math.random() * 150
        
        // Ekipman durumları
        const equipment = this.equipmentTypes.map(type => {
          const isDetected = Math.random() > (1 - this.detectionSensitivity)
          return {
            type,
            status: isDetected ? 'detected' : 'missing',
            confidence: Math.floor(Math.random() * 30) + 70
          }
        })
        
        const hasViolation = equipment.some(eq => eq.status === 'missing')
        if (hasViolation) violations++
        
        const detection = {
          id: `det_${Date.now()}_${i}`,
          workerId: worker.id,
          workerName: worker.name,
          bbox: { x, y, width, height },
          equipment,
          isViolation: hasViolation,
          confidence: Math.floor(Math.random() * 20) + 80,
          timestamp: Date.now()
        }
        
        newDetections.push(detection)
        
        // Tespit kutusunu çiz
        this.drawDetectionBox(ctx, detection)
      }
      
      // Durumu güncelle
      this.currentDetections = newDetections
      this.violationCount = violations
      this.totalDetections++
      this.lastDetectionTime = new Date().toLocaleTimeString('tr-TR')
      
      // Parent'a bildir
      if (newDetections.length > 0) {
        this.$emit('detection', {
          timestamp: Date.now(),
          detections: newDetections,
          totalDetections: this.totalDetections,
          violations: violations,
          image: imageData,
          cameraId: this.cameraId
        })
        
        // Ses uyarısı
        if (violations > 0 && this.audioAlerts) {
          this.playAlertSound()
        }
      }
    },

    drawDetectionBox(ctx, detection) {
      const { bbox, isViolation, workerName, confidence } = detection
      
      // Kutu rengi
      ctx.strokeStyle = isViolation ? '#dc3545' : '#28a745'
      ctx.lineWidth = 3
      ctx.fillStyle = isViolation ? 'rgba(220, 53, 69, 0.2)' : 'rgba(40, 167, 69, 0.2)'
      
      // Tespit kutusunu çiz
      ctx.fillRect(bbox.x, bbox.y, bbox.width, bbox.height)
      ctx.strokeRect(bbox.x, bbox.y, bbox.width, bbox.height)
      
      // Etiket arka planı
      const labelHeight = 30
      ctx.fillStyle = isViolation ? '#dc3545' : '#28a745'
      ctx.fillRect(bbox.x, bbox.y - labelHeight, bbox.width, labelHeight)
      
      // Etiket metni
      ctx.fillStyle = 'white'
      ctx.font = 'bold 12px Arial'
      ctx.textAlign = 'left'
      const label = `${workerName} (${confidence}%)`
      ctx.fillText(label, bbox.x + 5, bbox.y - 10)
      
      // İhlal varsa uyarı ikonu
      if (isViolation) {
        ctx.font = '16px Arial'
        ctx.fillText('⚠️', bbox.x + bbox.width - 25, bbox.y - 8)
      }
      
      // Eksik ekipmanları göster
      if (isViolation) {
        const missingEquipment = detection.equipment
          .filter(eq => eq.status === 'missing')
          .map(eq => eq.type)
          .join(', ')
        
        ctx.font = '10px Arial'
        ctx.fillStyle = '#dc3545'
        ctx.fillText(`Eksik: ${missingEquipment}`, bbox.x + 5, bbox.y + bbox.height + 15)
      }
    },

    onCanvasClick(event) {
      if (!this.currentDetections.length) return
      
      const rect = event.target.getBoundingClientRect()
      const x = event.clientX - rect.left
      const y = event.clientY - rect.top
      
      // Tıklanan tespit varsa detayları göster
      const clickedDetection = this.currentDetections.find(detection => {
        const bbox = detection.bbox
        return x >= bbox.x && x <= bbox.x + bbox.width &&
               y >= bbox.y && y <= bbox.y + bbox.height
      })
      
      if (clickedDetection) {
        this.$emit('detection-clicked', clickedDetection)
      }
    },

    async takeSnapshot() {
      if (!this.isStreaming) return
      
      try {
        const video = this.$refs.videoElement
        const canvas = document.createElement('canvas')
        canvas.width = video.videoWidth
        canvas.height = video.videoHeight
        
        const ctx = canvas.getContext('2d')
        ctx.drawImage(video, 0, 0, canvas.width, canvas.height)
        
        // Tespit kutularını da ekle
        if (this.currentDetections.length > 0) {
          this.currentDetections.forEach(detection => {
            this.drawDetectionBox(ctx, detection)
          })
        }
        
        // İndir
        const link = document.createElement('a')
        link.download = `snapshot_${this.cameraId}_${new Date().toISOString()}.jpg`
        link.href = canvas.toDataURL('image/jpeg', 0.9)
        link.click()
        
        this.$emit('snapshot-taken', {
          cameraId: this.cameraId,
          timestamp: Date.now(),
          detections: this.currentDetections
        })
        
        console.log('Ekran görüntüsü alındı')
      } catch (error) {
        console.error('Ekran görüntüsü alınamadı:', error)
      }
    },

    toggleRecording() {
      if (this.isRecording) {
        this.stopRecording()
      } else {
        this.startRecording()
      }
    },

    startRecording() {
      this.isRecording = true
      this.$emit('recording-started', { cameraId: this.cameraId })
      console.log('Kayıt başlatıldı')
    },

    stopRecording() {
      this.isRecording = false
      this.$emit('recording-stopped', { cameraId: this.cameraId })
      console.log('Kayıt durduruldu')
    },

    updateSettings() {
      this.$emit('settings-changed', {
        cameraId: this.cameraId,
        detectionSensitivity: this.detectionSensitivity,
        audioAlerts: this.audioAlerts
      })
    },

    playAlertSound() {
      // Basit ses bildirimi
      if ('speechSynthesis' in window && this.audioAlerts) {
        const utterance = new SpeechSynthesisUtterance('İhlal tespit edildi')
        utterance.volume = 0.1
        utterance.rate = 1.5
        speechSynthesis.speak(utterance)
      }
    },

    startFPSCounter() {
      this.frameCount = 0
      this.fpsInterval = setInterval(() => {
        this.currentFPS = this.frameCount
        this.frameCount = 0
        this.$emit('fps-update', this.currentFPS)
      }, 1000)
    }
  }
}
</script>

<style scoped>
.camera-stream-container {
  background: white;
  border-radius: 8px;
  overflow: hidden;
}

.camera-display {
  position: relative;
  width: 100%;
  background: #000;
  min-height: 300px;
}

video, canvas {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

canvas.detection-canvas {
  position: absolute;
  top: 0;
  left: 0;
  pointer-events: auto;
  cursor: crosshair;
}

.hidden {
  display: none;
}

.camera-placeholder {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  color: white;
  background: #2c3e50;
}

.camera-icon {
  font-size: 4rem;
  margin-bottom: 20px;
  opacity: 0.5;
}

.detection-overlay {
  position: absolute;
  top: 10px;
  left: 10px;
  right: 10px;
  z-index: 10;
}

.detection-count {
  background: rgba(0, 123, 255, 0.9);
  color: white;
  padding: 0.5rem 1rem;
  border-radius: 20px;
  font-weight: 600;
  margin-bottom: 0.5rem;
  display: inline-block;
}

.violation-alert {
  background: rgba(220, 53, 69, 0.9);
  color: white;
  padding: 0.5rem 1rem;
  border-radius: 20px;
  font-weight: 600;
  animation: alertPulse 2s infinite;
  display: inline-block;
}

@keyframes alertPulse {
  0%, 100% { opacity: 0.9; }
  50% { opacity: 1; }
}

.camera-controls {
  display: flex;
  gap: 10px;
  padding: 15px;
  background: #f8f9fa;
  border-top: 1px solid #e9ecef;
  justify-content: center;
  flex-wrap: wrap;
}

.btn {
  padding: 8px 16px;
  border: none;
  border-radius: 6px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.3s ease;
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 0.9rem;
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

.btn-warning {
  background: #ffc107;
  color: #212529;
}

.btn-warning:hover:not(:disabled) {
  background: #e0a800;
}

.btn-info {
  background: #17a2b8;
  color: white;
}

.btn-info:hover:not(:disabled) {
  background: #138496;
}

.btn-primary {
  background: #007bff;
  color: white;
}

.btn-primary:hover:not(:disabled) {
  background: #0056b3;
}

.detection-info {
  padding: 15px;
  background: #f8f9fa;
  border-top: 1px solid #e9ecef;
}

.detection-stats {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
  gap: 15px;
}

.stat-item {
  display: flex;
  flex-direction: column;
  gap: 5px;
  text-align: center;
}

.stat-label {
  font-size: 0.8rem;
  color: #666;
  font-weight: 500;
}

.stat-value {
  font-size: 1rem;
  font-weight: 600;
  color: #333;
}

.camera-settings {
  padding: 15px;
  background: #f8f9fa;
  border-top: 1px solid #e9ecef;
}

.settings-row {
  display: flex;
  align-items: center;
  gap: 15px;
  margin-bottom: 10px;
}

.settings-row label {
  font-weight: 600;
  color: #495057;
  min-width: 120px;
}

.sensitivity-slider {
  flex: 1;
  margin: 0 10px;
}

.sensitivity-value {
  font-weight: bold;
  color: #007bff;
  min-width: 40px;
}

/* Switch Komponenti */
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
  background-color: #007bff;
}

input:checked + .slider:before {
  transform: translateX(26px);
}

/* Responsive */
@media (max-width: 768px) {
  .camera-controls {
    flex-direction: column;
    align-items: center;
  }
  
  .btn {
    width: 100%;
    justify-content: center;
    max-width: 200px;
  }
  
  .detection-stats {
    grid-template-columns: 1fr 1fr;
  }
  
  .settings-row {
    flex-direction: column;
    align-items: flex-start;
    gap: 8px;
  }
  
  .settings-row label {
    min-width: auto;
  }
}
</style>
