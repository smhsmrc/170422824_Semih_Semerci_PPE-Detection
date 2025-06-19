<template>
  <div id="app">
    <!-- Navigation (ana sayfalarda) -->
    <nav v-if="showNavigation" class="main-nav">
      <div class="nav-brand">
        <h2>🦺 PPE Takip Sistemi</h2>
      </div>
      
      <div class="nav-links">
        <router-link to="/main" class="nav-link">
          <i class="icon">🏠</i> Ana Sayfa
        </router-link>
        <router-link to="/violations" class="nav-link">
          <i class="icon">⚠️</i> İhlaller
        </router-link>
        <router-link to="/workers" class="nav-link">
          <i class="icon">👥</i> Çalışanlar
        </router-link>
        <router-link to="/settings" class="nav-link">
          <i class="icon">⚙️</i> Ayarlar
        </router-link>
        <button @click="logout" class="nav-link logout-btn">
          <i class="icon">🚪</i> Çıkış
        </button>
      </div>
    </nav>

    <!-- Ana İçerik -->
    <main :class="{ 'with-nav': showNavigation }">
      <router-view />
    </main>
  </div>
</template>

<script>
import { useAuthStore } from '@/stores/auth.js'
import { authAPI } from '@/api/auth.js'

export default {
  name: 'App',
  setup() {
    const authStore = useAuthStore()
    
    return {
      authStore
    }
  },
  computed: {
    showNavigation() {
      // Login sayfasında navigation gösterme
      return this.$route.path !== '/login'
    }
  },
  methods: {
    // Logout fonksiyonu
    async logout() {
      try {
        // Onay penceresi
        if (!confirm('Çıkış yapmak istediğinizden emin misiniz?')) {
          return
        }
        
        console.log('Logout başlatılıyor...')
        
        // Backend'e logout isteği (opsiyonel)
        try {
          await authAPI.logout()
          console.log('Backend logout başarılı')
        } catch (error) {
          console.warn('Backend logout hatası:', error)
          // Backend hatası olsa bile devam et
        }
        
        // Store'dan logout
        this.authStore.logout()
        console.log('Store logout başarılı')
        
        // Başarı mesajı
        alert('Başarıyla çıkış yapıldı!')
        
        // Login sayfasına yönlendir
        this.$router.push('/login')
        
      } catch (error) {
        console.error('Logout error:', error)
        
        // Hata olsa bile çıkış yap
        this.authStore.logout()
        alert('Çıkış yapılırken bir hata oluştu, yine de çıkış yapılıyor...')
        this.$router.push('/login')
      }
    }
  }
}
</script>

<style>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

html, body {
  width: 100%;
  height: 100%;
  overflow-x: hidden;
}

body {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  background: #f5f5f5;
}

#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  color: #2c3e50;
  width: 100vw;
  min-height: 100vh;
  margin: 0;
  padding: 0;
}

.main-nav {
  background: #2c3e50;
  color: white;
  padding: 0.75rem 1rem;
  display: flex;
  justify-content: space-between;
  align-items: center;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
  width: 100%;
  box-sizing: border-box;
}

.nav-brand h2 {
  color: #ecf0f1;
  font-size: 1.3rem;
  margin: 0;
}

.nav-links {
  display: flex;
  gap: 0.75rem;
  align-items: center;
  flex-wrap: wrap;
}

.nav-link {
  color: #ecf0f1;
  text-decoration: none;
  padding: 0.4rem 0.8rem;
  border-radius: 4px;
  transition: background 0.3s;
  display: flex;
  align-items: center;
  gap: 0.4rem;
  font-size: 0.9rem;
  white-space: nowrap;
}

.nav-link:hover,
.nav-link.router-link-active {
  background: #34495e;
}

.logout-btn {
  background: #e74c3c;
  border: none;
  cursor: pointer;
}

.logout-btn:hover {
  background: #c0392b;
}

main {
  width: 100%;
  min-height: 100vh;
  box-sizing: border-box;
}

main.with-nav {
  min-height: calc(100vh - 60px);
}

.icon {
  font-size: 1em;
}

/* Responsive Design */
@media (max-width: 768px) {
  .main-nav {
    flex-direction: column;
    gap: 0.5rem;
    padding: 0.5rem;
  }
  
  .nav-links {
    justify-content: center;
    gap: 0.5rem;
  }
  
  .nav-link {
    padding: 0.3rem 0.6rem;
    font-size: 0.8rem;
  }
  
  main.with-nav {
    min-height: calc(100vh - 100px);
  }
}

@media (max-width: 480px) {
  .nav-brand h2 {
    font-size: 1.1rem;
  }
  
  .nav-links {
    flex-direction: column;
    width: 100%;
  }
  
  .nav-link {
    justify-content: center;
    width: 100%;
  }
}

header {
  text-align: center;
  margin-bottom: 2rem;
}

header h1 {
  color: #2c3e50;
  font-size: 2rem;
  margin: 0;
}

.error-toast {
  position: fixed;
  bottom: 1rem;
  right: 1rem;
  background: #f44336;
  color: white;
  padding: 1rem;
  border-radius: 4px;
  display: flex;
  align-items: center;
  gap: 1rem;
  box-shadow: 0 2px 4px rgba(0,0,0,0.2);
  animation: slideIn 0.3s ease-out;
}

.error-toast button {
  background: none;
  border: none;
  color: white;
  font-size: 1.5rem;
  cursor: pointer;
  padding: 0;
  line-height: 1;
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
</style>
