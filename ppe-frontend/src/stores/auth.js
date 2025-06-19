import { defineStore } from 'pinia'
import { ref } from 'vue'

export const useAuthStore = defineStore('auth', () => {
  const user = ref(null)
  const isLoggedIn = ref(false)

  // Sabit kullanıcı bilgileri
  const validCredentials = {
    username: 'admin',
    password: 'admin'
  }

  const login = async (credentials) => {
    try {
      console.log('Login attempt:', credentials) // Debug için
      
      // Basit doğrulama
      if (credentials.username === validCredentials.username && 
          credentials.password === validCredentials.password) {
        
        user.value = {
          id: 1,
          username: credentials.username,
          name: 'Admin User'
        }
        isLoggedIn.value = true
        
        // LocalStorage'a kaydet
        localStorage.setItem('auth_user', JSON.stringify(user.value))
        localStorage.setItem('auth_token', 'dummy_token_123')
        localStorage.setItem('isAuthenticated', 'true')
        
        console.log('Login successful') // Debug için
        return { success: true }
      } else {
        console.log('Invalid credentials') // Debug için
        return { success: false, message: 'Geçersiz kullanıcı adı veya şifre' }
      }
    } catch (error) {
      console.error('Login error:', error)
      return { success: false, message: 'Giriş sırasında bir hata oluştu' }
    }
  }

  const logout = () => {
    user.value = null
    isLoggedIn.value = false
    localStorage.removeItem('auth_user')
    localStorage.removeItem('auth_token')
    localStorage.removeItem('isAuthenticated')
  }

  const initAuth = () => {
    try {
      const savedUser = localStorage.getItem('auth_user')
      const savedToken = localStorage.getItem('auth_token')
      
      if (savedUser && savedToken) {
        user.value = JSON.parse(savedUser)
        isLoggedIn.value = true
        console.log('Auth initialized from localStorage')
      }
    } catch (error) {
      console.error('Auth init error:', error)
      logout()
    }
  }

  return {
    user,
    isLoggedIn,
    login,
    logout,
    initAuth
  }
})
