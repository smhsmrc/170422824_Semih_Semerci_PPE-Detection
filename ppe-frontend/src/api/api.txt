const API_BASE_URL = 'http://localhost:5001/api';

console.log('🔍 API_BASE_URL:', API_BASE_URL);

// Health check
export async function checkHealth() {
  try {
    const response = await fetch(`${API_BASE_URL}/health`);
    return await response.json();
  } catch (error) {
    console.error('Health check failed:', error);
    throw error;
  }
}

// Detection status  
export async function fetchDetection() {
  try {
    const response = await fetch(`${API_BASE_URL}/ppe/detection`);
    return await response.json();
  } catch (error) {
    console.error('Detection fetch failed:', error);
    throw error;
  }
}

// Latest detections
export async function fetchLatestDetections() {
  try {
    const response = await fetch(`${API_BASE_URL}/ppe/detections`);
    return await response.json();
  } catch (error) {
    console.error('Latest detections failed:', error);
    throw error;
  }
}

// Stats - Backend'deki route'a uygun
export async function fetchStats() {
  try {
    const response = await fetch(`${API_BASE_URL}/ppe/stats`);  // stats/daily → ppe/stats
    return await response.json();
  } catch (error) {
    console.error('Stats fetch failed:', error);
    throw error;
  }
}

// PPE Detection
export async function detectPPE(imageData) {
  try {
    const response = await fetch(`${API_BASE_URL}/ppe/detect`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ image: imageData })
    });
    return await response.json();
  } catch (error) {
    console.error('PPE detection failed:', error);
    throw error;
  }
}

// Image upload
export async function uploadImageForDetection(formData) {
  try {
    const response = await fetch(`${API_BASE_URL}/upload`, {
      method: 'POST',
      body: formData
    });
    return await response.json();
  } catch (error) {
    console.error('Image upload failed:', error);
    throw error;
  }
}
