<template>
  <div class="ppe-detection">
    <div class="controls">
      <button @click="toggleStream" :class="{ active: isStreaming }">
        {{ isStreaming ? 'Durdur' : 'Başlat' }}
      </button>
      <div class="status" :class="{ healthy: isHealthy }">
        {{ healthStatus }}
      </div>
    </div>

    <div class="video-container">
      <!-- Preload edilmiş image'lar -->
      <img 
        v-for="(frame, index) in frameBuffer" 
        :key="`preload-${index}`"
        :src="`data:image/jpeg;base64,${frame}`" 
        class="preload-image"
        style="display: none;"
        @load="onPreloadComplete"
      />
      
      <!-- Ana görüntü -->
      <img 
        v-if="currentFrame" 
        :src="`data:image/jpeg;base64,${currentFrame}`" 
        alt="PPE Detection Feed"
        class="video-feed"
        loading="eager"
        decoding="async"
        @load="onFrameLoaded"
        @error="onFrameError"
      />
      <div v-else class="no-video">
        <p>Kamera görüntüsü bekleniyor...</p>
      </div>
      
      <!-- FPS Göstergesi -->
      <div v-if="isStreaming" class="fps-indicator">
        FPS: {{ currentFPS }}
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'PPEDetection',
  data() {
    return {
      isStreaming: false,
      currentFrame: null,
      nextFrame: null,
      isHealthy: false,
      healthStatus: 'Bağlantı kontrol ediliyor...',
      frameInterval: null,
      healthInterval: null,
      frameRequestInProgress: false,
      consecutiveErrors: 0,
      maxConsecutiveErrors: 5,
      currentFPS: 0,
      frameCount: 0,
      fpsStartTime: 0,
      lastFrameTime: 0,
      frameBuffer: [],
      maxBufferSize: 3,
      lastSuccessfulFrame: null,
      animationFrameId: null,
      lastFrameRequest: 0,
      frameRequestDelay: 100
    }
  },
  mounted() {
    this.checkHealth();
    this.healthInterval = setInterval(this.checkHealth, 5000);
  },
  beforeUnmount() {
    if (this.animationFrameId) {
      cancelAnimationFrame(this.animationFrameId);
    }
    if (this.healthInterval) {
      clearInterval(this.healthInterval);
    }
  },
  methods: {
    async toggleStream() {
      try {
        if (this.isStreaming) {
          await this.$http.post('/api/ppe/stream/stop');
          this.isStreaming = false;
          if (this.animationFrameId) {
            cancelAnimationFrame(this.animationFrameId);
            this.animationFrameId = null;
          }
          this.frameRequestInProgress = false;
          this.consecutiveErrors = 0;
          this.resetFPS();
          this.resetFrameBuffer();
        } else {
          await this.$http.post('/api/ppe/stream/start');
          this.isStreaming = true;
          this.consecutiveErrors = 0;
          this.resetFPS();
          this.resetFrameBuffer();
          this.startFrameUpdates();
        }
      } catch (error) {
        console.error('Stream toggle error:', error);
      }
    },

    startFrameUpdates() {
      const updateFrame = async (timestamp) => {
        if (!this.isStreaming) return;
        
        // Frame rate kontrolü
        if (timestamp - this.lastFrameRequest < this.frameRequestDelay) {
          this.animationFrameId = requestAnimationFrame(updateFrame);
          return;
        }
        
        this.lastFrameRequest = timestamp;
        
        // Eğer bir frame isteği devam ediyorsa, bu frame'i atla
        if (this.frameRequestInProgress) {
          this.animationFrameId = requestAnimationFrame(updateFrame);
          return;
        }
        
        this.frameRequestInProgress = true;
        
        try {
          const response = await this.$http.get('/api/ppe/frame');
          const newFrame = response.data;
          
          // Frame validation - boş veya çok küçük frame'leri reddet
          if (newFrame && newFrame.length > 1000) {
            // Frame buffer'a ekle
            this.frameBuffer.push(newFrame);
            if (this.frameBuffer.length > this.maxBufferSize) {
              this.frameBuffer.shift(); // En eski frame'i çıkar
            }
            
            // Smooth transition için nextFrame'e ata
            this.nextFrame = newFrame;
            
            // Eğer currentFrame yoksa hemen ata, yoksa smooth transition yap
            if (!this.currentFrame) {
              this.currentFrame = newFrame;
            } else {
              // requestAnimationFrame ile smooth transition
              requestAnimationFrame(() => {
                this.currentFrame = this.nextFrame;
              });
            }
            
            this.lastSuccessfulFrame = newFrame;
            this.consecutiveErrors = 0; // Başarılı istek, hata sayacını sıfırla
          } else {
            // Geçersiz frame, son başarılı frame'i kullan
            if (this.lastSuccessfulFrame && this.consecutiveErrors < 2) {
              this.currentFrame = this.lastSuccessfulFrame;
            }
            this.consecutiveErrors++;
          }
        } catch (error) {
          this.consecutiveErrors++;
          console.error('Frame update error:', error);
          
          // Hata durumunda buffer'dan frame kullan
          if (this.frameBuffer.length > 0 && this.consecutiveErrors < 3) {
            this.currentFrame = this.frameBuffer[this.frameBuffer.length - 1];
          }
          
          // Çok fazla ardışık hata varsa stream'i durdur
          if (this.consecutiveErrors >= this.maxConsecutiveErrors) {
            console.warn('Çok fazla ardışık hata, stream durduruluyor...');
            this.isStreaming = false;
            return;
          }
        } finally {
          this.frameRequestInProgress = false;
        }
        
        // Bir sonraki frame için schedule et
        if (this.isStreaming) {
          this.animationFrameId = requestAnimationFrame(updateFrame);
        }
      };
      
      // İlk frame'i başlat
      this.animationFrameId = requestAnimationFrame(updateFrame);
    },

    async checkHealth() {
      try {
        const response = await this.$http.get('/api/ppe/health');
        this.isHealthy = response.status === 200;
        this.healthStatus = this.isHealthy ? 'Bağlı' : 'Bağlantı Hatası';
      } catch (error) {
        this.isHealthy = false;
        this.healthStatus = 'Bağlantı Hatası';
      }
    },

    onFrameLoaded() {
      // FPS hesaplama
      const now = performance.now();
      if (this.fpsStartTime === 0) {
        this.fpsStartTime = now;
        this.frameCount = 1;
      } else {
        this.frameCount++;
        const elapsed = (now - this.fpsStartTime) / 1000;
        if (elapsed >= 1) {
          this.currentFPS = Math.round(this.frameCount / elapsed);
          this.fpsStartTime = now;
          this.frameCount = 0;
        }
      }
      this.lastFrameTime = now;
    },

    onFrameError(error) {
      console.error('Frame loading error:', error);
    },

    resetFPS() {
      this.currentFPS = 0;
      this.frameCount = 0;
      this.fpsStartTime = 0;
      this.lastFrameTime = 0;
    },

    resetFrameBuffer() {
      this.frameBuffer = [];
      this.nextFrame = null;
      this.lastSuccessfulFrame = null;
      this.consecutiveErrors = 0;
    },

    onPreloadComplete() {
      // Preload işlemi tamamlandığında yapılacak işlemler
    }
  }
}
</script>

<style scoped>
.ppe-detection {
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
  width: 100%;
  height: 100%;
  min-height: 0;
}

.controls {
  display: flex;
  gap: 1rem;
  align-items: center;
  justify-content: center;
  padding: 0.5rem 0;
  flex-shrink: 0;
}

button {
  padding: 0.5rem 1rem;
  border: none;
  border-radius: 4px;
  background: #4CAF50;
  color: white;
  cursor: pointer;
  transition: background 0.3s;
  font-weight: bold;
  font-size: 0.9rem;
}

button:hover {
  background: #45a049;
}

button.active {
  background: #f44336;
}

button.active:hover {
  background: #da190b;
}

.status {
  padding: 0.5rem 1rem;
  border-radius: 4px;
  background: #f44336;
  color: white;
  font-weight: bold;
  font-size: 0.8rem;
}

.status.healthy {
  background: #4CAF50;
}

.video-container {
  flex: 1;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #000;
  border-radius: 8px;
  overflow: hidden;
  min-height: 0;
  position: relative;
  /* Anti-flickering optimizasyonları */
  will-change: contents;
  transform: translateZ(0); /* Hardware acceleration */
  backface-visibility: hidden;
}

.video-feed {
  width: 100%;
  height: 100%;
  object-fit: cover;
  display: block;
  transition: opacity 0.05s ease-out; /* Daha hızlı transition */
  image-rendering: -webkit-optimize-contrast; /* Daha iyi rendering */
  image-rendering: crisp-edges;
  /* Anti-flickering */
  will-change: transform, opacity;
  transform: translateZ(0); /* Hardware acceleration */
  backface-visibility: hidden;
  -webkit-transform: translateZ(0);
  -webkit-backface-visibility: hidden;
  /* Smooth rendering */
  image-rendering: optimizeSpeed;
  image-rendering: -moz-crisp-edges;
  image-rendering: -webkit-optimize-contrast;
  image-rendering: optimize-contrast;
  -ms-interpolation-mode: nearest-neighbor;
}

.video-feed.loading {
  opacity: 0.9;
}

.preload-image {
  position: absolute;
  pointer-events: none;
  z-index: -1;
}

.no-video {
  color: white;
  text-align: center;
  padding: 2rem;
}

.no-video p {
  margin: 0;
  font-size: 1.1rem;
  opacity: 0.8;
}

.fps-indicator {
  position: absolute;
  top: 10px;
  right: 10px;
  background: rgba(0, 0, 0, 0.7);
  color: white;
  padding: 0.25rem 0.5rem;
  border-radius: 4px;
  font-size: 0.8rem;
  font-weight: bold;
  font-family: monospace;
}

/* Responsive tasarım */
@media (max-width: 768px) {
  .controls {
    flex-direction: column;
    gap: 0.5rem;
  }
  
  .video-container {
    min-height: 300px;
  }
  
  .fps-indicator {
    top: 5px;
    right: 5px;
    font-size: 0.7rem;
  }
}
</style> 