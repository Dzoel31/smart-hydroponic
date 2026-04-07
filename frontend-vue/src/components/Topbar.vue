<template>
  <header class="topbar">
    <div class="greeting">
      <h1>{{ title }}</h1>
    </div>

    <div class="user-actions">
      <router-link v-if="!authState.isLoggedIn" to="/login" class="login-action-btn">
        Log in
      </router-link>

      <div v-else class="user-profile-wrapper">
        <div class="user-profile" @click="toggleDropdown">
          <div class="avatar">{{ userInitial }}</div>
          <span>{{ user }}</span>
          <svg :class="{ 'rotate': isDropdownOpen }" viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="2">
            <polyline points="6 9 12 15 18 9"></polyline>
          </svg>
        </div>

        <transition name="fade">
          <div v-if="isDropdownOpen" class="dropdown-menu">
            <button @click="handleLogout" class="dropdown-item">
              <svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                <path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4"></path>
                <polyline points="16 17 21 12 16 7"></polyline>
                <line x1="21" y1="12" x2="9" y2="12"></line>
              </svg>
              Logout
            </button>
          </div>
        </transition>
      </div>
    </div>
  </header>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted, defineProps, computed } from "vue";
import { useRouter } from "vue-router";
import { authState } from "../auth";

defineProps({
  title: {
    type: String,
    required: true,
    default: "Dashboard"
  }
});

const user = computed(() => {
  if (!authState.isLoggedIn) return 'Guest';

  const userName = authState.user?.username || 'Admin';
  
  return userName;
});

const userInitial = computed(() => {
  return user.value.charAt(0).toUpperCase();
});

const router = useRouter();
const isDropdownOpen = ref(false);

const toggleDropdown = () => {
  isDropdownOpen.value = !isDropdownOpen.value;
};

const handleLogout = () => {
  if (authState && typeof authState.logout === 'function') {
      authState.logout();
  }
  isDropdownOpen.value = false;
  router.push('/login');
};

const closeDropdown = (e: MouseEvent) => {
  if (!(e.target as Element).closest('.user-profile-wrapper')) {
    isDropdownOpen.value = false;
  }
};

onMounted(() => window.addEventListener('click', closeDropdown));
onUnmounted(() => window.removeEventListener('click', closeDropdown));
</script>

<style scoped>
.topbar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 32px;
  width: 100%;
}

.greeting h1 {
  font-size: 24px;
  font-weight: 700;
  margin: 0;
  color: #000000;
}

.user-actions {
  display: flex;
  align-items: center;
  gap: 16px;
}

.login-action-btn {
  background-color: #4caf50;
  color: white;
  padding: 8px 16px;
  border-radius: 8px;
  text-decoration: none;
  font-weight: 600;
  font-size: 14px;
  transition: 0.2s;
}

.login-action-btn:hover {
  background-color: #43a047;
}

.user-profile-wrapper {
  position: relative;
}

.user-profile {
  display: flex;
  align-items: center;
  gap: 10px;
  margin-left: 12px;
  cursor: pointer;
  padding: 8px 12px;
  border-radius: 8px;
  transition: background 0.2s;
  color: #1e293b; 
  font-weight: 500;
}

.user-profile:hover {
  background: #f1f5f9;
}

.user-profile svg {
  transition: transform 0.3s ease;
}

.user-profile svg.rotate {
  transform: rotate(180deg);
}

.avatar {
  width: 36px;
  height: 36px;
  background-color: #e2e8f0;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-weight: bold;
  color: #475569;
}

.dropdown-menu {
  position: absolute;
  top: 110%;
  right: 0;
  background: white;
  border: 1px solid #e2e8f0;
  border-radius: 8px;
  box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.1);
  min-width: 140px;
  z-index: 1000;
  padding: 4px;
}

.dropdown-item {
  width: 100%;
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 10px 12px;
  border: none;
  background: none;
  color: #ef4444; 
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  border-radius: 6px;
  text-align: left;
}

.dropdown-item:hover {
  background: #fef2f2;
}

.fade-enter-active, .fade-leave-active {
  transition: opacity 0.2s;
}
.fade-enter-from, .fade-leave-to {
  opacity: 0;
}
</style>