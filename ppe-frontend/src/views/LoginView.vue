<template>
  <div class="login-container">
    <div class="login-card">
      <div class="login-header">
        <h1>🦺 PPE Takip Sistemi</h1>
        <p>Sisteme giriş yapın</p>
      </div>
      
      <form @submit.prevent="handleLogin" class="login-form">
        <div class="form-group">
          <label for="username">👤 Kullanıcı Adı:</label>
          <input
            id="username"
            v-model="credentials.username"
            type="text"
            required
            placeholder="Kullanıcı adınızı girin"
          />
        </div>
        
        <div class="form-group">
          <label for="password">🔒 Şifre:</label>
          <input
            id="password"
            v-model="credentials.password"
            type="password"
            required
            placeholder="Şifrenizi girin"
          />
        </div>
        
        <button type="submit" class="login-btn" :disabled="isLoading">
          <span v-if="isLoading">⏳ Giriş yapılıyor...</span>
          <span v-else">🚀 Giriş Yap</span>
        </button>
        
        <div v-if="errorMessage" class="error-message">
          ❌ {{ errorMessage }}
        </div>
      </form>
    </div>
  </div>
</template>

<script>
import { useAuthStore } from '@/stores/auth'

export default {
  name: 'LoginView',
  data() {
    return {
      credentials: {
        username: '',
        password: ''
      },
      isLoading: false,
      errorMessage: ''
    }
  },
  methods: {
    async handleLogin() {
      console.log('Handle login called') // Debug için
      
      if (!this.credentials.username || !this.credentials.password) {
        this.errorMessage = 'Lütfen tüm alanları doldurun'
        return
      }

      this.isLoading = true
      this.errorMessage = ''
      
      try {
        const authStore = useAuthStore()
        
        const result = await authStore.login({
          username: this.credentials.username.trim(),
          password: this.credentials.password.trim()
        })

        console.log('Login result:', result) // Debug için

        if (result.success) {
          console.log('Redirecting to dashboard') // Debug için
          this.$router.push('/main/dashboard')
        } else {
          this.errorMessage = result.message || 'Giriş başarısız'
        }
      } catch (error) {
        console.error('Login error:', error)
        this.errorMessage = 'Giriş sırasında bir hata oluştu'
      } finally {
        this.isLoading = false
      }
    }
  }
}
</script>


<style scoped>
.login-container {
  min-height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
}

.login-card {
  background: white;
  padding: 2rem;
  border-radius: 12px;
  box-shadow: 0 10px 30px rgba(0,0,0,0.2);
  width: 100%;
  max-width: 400px;
}

.login-header {
  text-align: center;
  margin-bottom: 2rem;
}

.login-header h1 {
  color: #2c3e50;
  margin-bottom: 0.5rem;
}

.login-header p {
  color: #7f8c8d;
}

.form-group {
  margin-bottom: 1.5rem;
}

.form-group label {
  display: block;
  margin-bottom: 0.5rem;
  color: #2c3e50;
  font-weight: 500;
}

.form-group input {
  width: 100%;
  padding: 0.75rem;
  border: 2px solid #ecf0f1;
  border-radius: 8px;
  font-size: 1rem;
  transition: border-color 0.3s;
}

.form-group input:focus {
  outline: none;
  border-color: #3498db;
}

.login-btn {
  width: 100%;
  padding: 0.75rem;
  background: #3498db;
  color: white;
  border: none;
  border-radius: 8px;
  font-size: 1rem;
  font-weight: 600;
  cursor: pointer;
  transition: background 0.3s;
}

.login-btn:hover:not(:disabled) {
  background: #2980b9;
}

.login-btn:disabled {
  background: #bdc3c7;
  cursor: not-allowed;
}

.error-message {
  margin-top: 1rem;
  padding: 0.75rem;
  background: #fee;
  color: #e74c3c;
  border-radius: 8px;
  text-align: center;
}
</style>
