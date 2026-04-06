<template>
  <aside class="sidebar" :class="{ 'is-collapsed': isCollapsed }">
    
    <div class="brand" @click="toggleSidebar">
      <span class="brand-logo">
        <img :src="logo" alt="Logo Smart Hydroponic" class="logo-img" />
      </span>
      <h2>Smart Hydroponic</h2>
    </div>

    <nav class="nav-menu">
      <router-link to="/dashboard" class="nav-item" active-class="active">
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
          <rect x="3" y="3" width="7" height="7"></rect>
          <rect x="14" y="3" width="7" height="7"></rect>
          <rect x="14" y="14" width="7" height="7"></rect>
          <rect x="3" y="14" width="7" height="7"></rect>
        </svg>
        <span class="nav-text">Dashboard</span>
      </router-link>

      <template v-if="userRole === 'admin' || userRole === 'superadmin'">
        <router-link to="/sensor-data" class="nav-item" active-class="active">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <ellipse cx="12" cy="5" rx="9" ry="3"></ellipse>
            <path d="M21 12c0 1.66-4 3-9 3s-9-1.34-9-3"></path>
            <path d="M3 5v14c0 1.66 4 3 9 3s9-1.34 9-3V5"></path>
          </svg>
          <span class="nav-text">Hydroponic Data</span>
        </router-link>

        <router-link to="/analytics" class="nav-item" active-class="active">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <polyline points="22 12 18 12 15 21 9 3 6 12 2 12"></polyline>
          </svg>
          <span class="nav-text">Analytics</span>
        </router-link>
      </template>
      
      <template v-if="userRole === 'superadmin'">
        <router-link to="/manage-admin" class="nav-item" active-class="active">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <circle cx="9" cy="7" r="4"></circle>
            <path d="M17 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2"></path>
            <circle cx="17" cy="11" r="3"></circle>
            <path d="M21 21v-1a3 3 0 0 0-3-3"></path>
          </svg>
          <span class="nav-text">Manajemen Admin</span>
        </router-link>
      </template>

      <template v-if="userRole === 'admin' || userRole === 'superadmin'">
        <router-link to="/profile" class="nav-item" active-class="active">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"></path>
            <circle cx="12" cy="7" r="4"></circle>
          </svg>
          <span class="nav-text">Profile</span>
        </router-link>
      </template>
    </nav>
  </aside>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from 'vue';
import { authState } from '@/auth';

defineProps({
  logo: String
});

const userRole = computed(() => {
  if (authState.isLoggedIn && authState.user) {
    return authState.user.role;
  }
  return 'guest';
});

const isCollapsed = ref(false);

const toggleSidebar = () => {
  isCollapsed.value = !isCollapsed.value;
};

// Deteksi ukuran layar otomatis saat browser di-resize
const checkScreenSize = () => {
  if (window.innerWidth <= 1024) {
    isCollapsed.value = true;
  } else {
    isCollapsed.value = false;
  }
};

onMounted(() => {
  checkScreenSize();
  window.addEventListener('resize', checkScreenSize);
});

onUnmounted(() => {
  window.removeEventListener('resize', checkScreenSize);
});
</script>

<style scoped>
* {
  box-sizing: border-box;
  margin: 0;
  padding: 0;
}

/* SIDEBAR CONTAINER */
.sidebar {
  width: 260px;
  height: 100vh;
  background-color: #ffffff;
  border-right: 1px solid #e2e8f0;
  padding: 24px 16px;
  display: flex;
  flex-direction: column;
  box-sizing: border-box;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  position: sticky;
  top: 0;
  flex-shrink: 0;
  z-index: 100;
  transition: width 0.3s ease, padding 0.3s ease; /* Efek mulus saat buka-tutup */
  overflow-y: auto;
  overflow-x: hidden;
}

/* Sembunyikan scrollbar agar rapi */
.sidebar::-webkit-scrollbar { display: none; }

/* BRANDING */
.brand {
  display: flex;
  align-items: center;
  padding: 7px 20px 35px 20px; 
  gap: 12px;
  cursor: pointer;
  transition: padding 0.3s ease;
}

.brand:hover { background-color: #f8fafc; }

.brand h2 {
  font-size: 18px;
  font-weight: 700;
  color: #4caf50;
  line-height: 1.2;
  white-space: normal;
  transition: opacity 0.2s ease, visibility 0.2s ease;
}

.logo-img {
  width: 60px;
  height: 60px;
  object-fit: contain;
  transition: all 0.3s ease;
}

/* NAVIGATION MENU */
.nav-menu {
  display: flex;
  flex-direction: column;
  gap: 8px;
  padding: 0 16px 16px 16px;
  flex: 1;
}

.nav-text {
  white-space: nowrap;
}

/* MENU ITEM */
.nav-item {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 12px 16px;
  text-decoration: none;
  color: #64748b;
  border-radius: 8px;
  font-weight: 500;
  font-size: 15px;
  transition: all 0.2s ease-in-out;
  overflow: hidden;
}

.nav-item svg {
  width: 20px;
  height: 20px;
  flex-shrink: 0;
}

.nav-item:hover { background-color: #f8fafc; color: #0f172a; }
.nav-item.active { background-color: #e8f5e9; color: #2e7d32; font-weight: 600; }
.spacer { flex-grow: 1; }

.sidebar.is-collapsed {
  width: 88px;
  padding: 24px 12px;
  align-items: center;
}

.sidebar.is-collapsed .brand h2, 
.sidebar.is-collapsed .nav-text {
  display: none;
}

.sidebar.is-collapsed .brand {
  justify-content: center;
  padding: 8px 0;
  margin-bottom: 24px;
  width: 100%;
}

.sidebar.is-collapsed .logo-img {
  width: 40px;
  height: 40px;
  margin: 0 auto;
}

.sidebar.is-collapsed .nav-item {
  justify-content: center;
  padding: 14px 0;
  width: 100%;
}

.sidebar.is-collapsed .nav-item svg {
  margin: 0;
}
</style>