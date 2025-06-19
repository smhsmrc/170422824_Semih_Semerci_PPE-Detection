import { createRouter, createWebHistory } from 'vue-router'
import { useAuthStore } from '@/stores/auth'
import LoginView from '@/views/LoginView.vue'
import MainView from '@/views/MainView.vue'
import DashboardView from '@/views/DashboardView.vue'
import MonitoringView from '@/views/MonitoringView.vue'
import WorkersView from '@/views/WorkersView.vue'
import ViolationView from '@/views/ViolationView.vue'
import SettingsView from '@/views/SettingsView.vue'

const routes = [
  {
    path: '/',
    redirect: '/login'
  },
  {
    path: '/login',
    name: 'Login',
    component: LoginView
  },
  // ✅ BASIT ROUTE YAPISI
  {
    path: '/main',
    name: 'Main',
    component: MainView,
    meta: { requiresAuth: true }
  },
  {
    path: '/dashboard',
    name: 'Dashboard',
    component: DashboardView,
    meta: { requiresAuth: true }
  },
  {
    path: '/monitoring',
    name: 'Monitoring',
    component: MonitoringView,
    meta: { requiresAuth: true }
  },
  {
    path: '/workers',
    name: 'Workers',
    component: WorkersView,
    meta: { requiresAuth: true }
  },
  {
    path: '/violations',
    name: 'Violations',
    component: ViolationView,
    meta: { requiresAuth: true }
  },
  {
    path: '/settings',
    name: 'Settings',
    component: SettingsView,
    meta: { requiresAuth: true }
  }
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

// Navigation Guard
router.beforeEach((to, from, next) => {
  const authStore = useAuthStore()
  
  if (to.matched.some(record => record.meta.requiresAuth)) {
    if (!authStore.isLoggedIn) {
      next('/login')
    } else {
      next()
    }
  } else if (to.path === '/login' && authStore.isLoggedIn) {
    // Eğer zaten giriş yapmışsa main'e yönlendir
    next('/main')
  } else {
    next()
  }
})

export default router
