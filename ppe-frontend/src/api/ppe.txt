import axios from 'axios';

const CPP_SERVER_URL = 'http://localhost:8080/api/ppe';

export const ppeAPI = {
    // PPE monitoring başlat
    async startMonitoring() {
        try {
            const response = await axios.post(`${CPP_SERVER_URL}/start`);
            return response.data;
        } catch (error) {
            throw new Error(error.response?.data?.message || 'Monitoring başlatılamadı');
        }
    },
    
    // PPE monitoring durdur
    async stopMonitoring() {
        try {
            const response = await axios.post(`${CPP_SERVER_URL}/stop`);
            return response.data;
        } catch (error) {
            throw new Error(error.response?.data?.message || 'Monitoring durdurulamadı');
        }
    },
    
    // Son tespitleri ve kamera görüntüsünü al
    async getDetections() {
        try {
            const response = await axios.get(`${CPP_SERVER_URL}/camera/stream`);
            return {
                success: response.data.success,
                data: {
                    frame: response.data.data.frame,
                    detections: response.data.data.detections,
                    stats: response.data.data.stats
                }
            };
        } catch (error) {
            throw new Error(error.response?.data?.message || 'Tespit verileri alınamadı');
        }
    },
    
    // İstatistikleri al
    async getStats() {
        try {
            const response = await axios.get(`${CPP_SERVER_URL}/stats`);
            return response.data;
        } catch (error) {
            throw new Error(error.response?.data?.message || 'İstatistik verileri alınamadı');
        }
    },
    
    // Health check
    async checkHealth() {
        try {
            const response = await axios.get('http://localhost:8080/health');
            return response.data;
        } catch (error) {
            throw new Error('C++ server bağlantısı kurulamadı');
        }
    }
};

// Yeni: C++ server entegrasyonu için yardımcı fonksiyonlar
export const ppeService = {
    isConnected: false,
    
    async initialize() {
        try {
            const health = await ppeAPI.checkHealth();
            this.isConnected = health.status === 'ok';
            return this.isConnected;
        } catch (error) {
            this.isConnected = false;
            throw error;
        }
    },
    
    async startDetection() {
        if (!this.isConnected) {
            await this.initialize();
        }
        return ppeAPI.startMonitoring();
    },
    
    async stopDetection() {
        if (!this.isConnected) return;
        return ppeAPI.stopMonitoring();
    },
    
    async getFrame() {
        if (!this.isConnected) return null;
        const response = await ppeAPI.getDetections();
        return response.data;
    }
};