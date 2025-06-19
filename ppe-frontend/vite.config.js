import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import { fileURLToPath, URL } from 'node:url'

export default defineConfig({
  plugins: [vue()],
  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./src', import.meta.url))
    }
  },
  define: {
    // Process environment variables için
    'process.env': process.env,
    global: 'globalThis',
  },
  server: {
    port: 5173,
    proxy: {
      '/api': 'http://localhost:5001'
    }
  }
})
