import { defineStore } from 'pinia'
import axios from 'axios'
import { io } from 'socket.io-client'

export const useMonitoringStore = defineStore('monitoring', {
  state: () => ({
    cameras: [],
    activeCamera: null,
    detections: [],
    violations: [],
    statistics: {
      totalWorkers: 0,
      activeWorkers: 0,
      violations: 0,
      complianceRate: 0
    },
    socket: null,
    isConnected: false
  }),

  actions: {
    async fetchCameras() {
      try {
        const response = await axios.get('/api/cameras')
        this.cameras = response.data
      } catch (error) {
        console.error('Kameralar yüklenemedi:', error)
      }
    },

    async fetchDetections() {
      try {
        const response = await axios.get('/api/detections/latest')
        this.detections = response.data
      } catch (error) {
        console.error('Tespitler yüklenemedi:', error)
      }
    },

    async fetchStatistics() {
      try {
        const response = await axios.get('/api/statistics')
        this.statistics = response.data
      } catch (error) {
        console.error('İstatistikler yüklenemedi:', error)
      }
    },

    connectSocket() {
      this.socket = io(process.env.VUE_APP_SOCKET_URL || 'http://localhost:3000')
      
      this.socket.on('connect', () => {
        this.isConnected = true
        console.log('Socket bağlantısı kuruldu')
      })

      this.socket.on('disconnect', () => {
        this.isConnected = false
        console.log('Socket bağlantısı kesildi')
      })

      this.socket.on('detection', (data) => {
        this.detections.unshift(data)
        if (this.detections.length > 100) {
          this.detections = this.detections.slice(0, 100)
        }
      })

      this.socket.on('violation', (data) => {
        this.violations.unshift(data)
      })

      this.socket.on('statistics', (data) => {
        this.statistics = data
      })
    },

    disconnectSocket() {
      if (this.socket) {
        this.socket.disconnect()
        this.socket = null
        this.isConnected = false
      }
    }
  }
})
