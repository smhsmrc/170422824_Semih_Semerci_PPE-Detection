// src/services/auth.js
const API_BASE_URL = process.env.VUE_APP_API_URL || 'http://localhost:5001/api';

export const authAPI = {
  // Login
  async login(credentials) {
    const response = await fetch(`${API_BASE_URL}/auth/login`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(credentials)
    });
    return await response.json();
  },

  // Register
  async register(userData) {
    const response = await fetch(`${API_BASE_URL}/auth/register`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(userData)
    });
    return await response.json();
  },

  // Logout
  async logout() {
    const token = localStorage.getItem('token');
    const response = await fetch(`${API_BASE_URL}/auth/logout`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${token}`
      }
    });
    return await response.json();
  },

  // Get Profile
  async getProfile() {
    const token = localStorage.getItem('token');
    const response = await fetch(`${API_BASE_URL}/auth/profile`, {
      method: 'GET',
      headers: {
        'Authorization': `Bearer ${token}`
      }
    });
    return await response.json();
  }
};
