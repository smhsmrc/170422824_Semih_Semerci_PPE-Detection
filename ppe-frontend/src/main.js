import { createApp } from 'vue'
import { createPinia } from 'pinia'
import axios from 'axios'
import App from './App.vue'
import router from './router'
import { useAuthStore } from './stores/auth'

// Axios konfigürasyonu
axios.defaults.baseURL = 'http://localhost:5001'
axios.defaults.timeout = 10000

const app = createApp(App)
const pinia = createPinia()

// Axios'u global olarak ekle
app.config.globalProperties.$http = axios

app.use(pinia)
app.use(router)

// Auth store'u başlat
const authStore = useAuthStore()
authStore.initAuth()

app.mount('#app')
