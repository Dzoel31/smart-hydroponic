<template>
  <div class="layout-wrapper">
    <Sidebar :logo="brandLogo" />

    <main class="main-content">
      <div class="profile-page">
        
        <Topbar title="User Profile" />

        <div class="profile-container">
          
          <div class="profile-card">
            <div class="profile-header-bg"></div>
            <div class="avatar-large">{{ avatarInitials }}</div>
            <h2 class="profile-name">{{ userProfile.fullname || userProfile.username || '-' }}</h2>
            <p class="profile-role">{{ displayRole }}</p>
            
            <div class="profile-stats">
              <div class="stat-item">
                <span class="stat-value">Active</span>
                <span class="stat-label">Status</span>
              </div>
              <div class="stat-divider"></div>
              <div class="stat-item">
                <span class="stat-value">{{ joinedDate }}</span>
                <span class="stat-label">Joined</span>
              </div>
            </div>
          </div>

          <div class="details-card">
            <div class="card-header">
              <h3>Personal Information</h3>
              <button 
                v-if="!isEditing" 
                @click="startEdit" 
                class="btn-edit"
              >
                <svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"></path><path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"></path></svg>
                Edit Profile
              </button>
            </div>

            <p v-if="fetchError" class="feedback error">{{ fetchError }}</p>
            <p v-if="saveSuccess" class="feedback success">{{ saveSuccess }}</p>

            <form @submit.prevent="saveProfile" class="profile-form">
              <div class="form-grid">
                
                <div class="form-group">
                  <label>Full Name</label>
                  <input 
                    type="text" 
                    v-model="editForm.fullname" 
                    :disabled="!isEditing"
                    :class="{ 'is-disabled': !isEditing }"
                  />
                </div>

                <div class="form-group">
                  <label>Username</label>
                  <input 
                    type="text" 
                    v-model="editForm.username" 
                    :disabled="!isEditing"
                    :class="{ 'is-disabled': !isEditing }"
                  />
                </div>

                <div class="form-group">
                  <label>Email Address</label>
                  <input 
                    type="email" 
                    v-model="editForm.email"
                    disabled
                    class="is-disabled"
                  />
                </div>

                <div class="form-group">
                  <label>Phone Number</label>
                  <input 
                    type="text" 
                    v-model="editForm.phone_number" 
                    :disabled="!isEditing"
                    :class="{ 'is-disabled': !isEditing }"
                  />
                </div>

                <div class="form-group full-width">
                  <label>Role</label>
                  <input 
                    type="text" 
                    :value="displayRole" 
                    disabled
                    class="is-disabled"
                  />
                </div>

              </div>

              <div class="form-actions" v-if="isEditing">
                <button type="button" @click="cancelEdit" class="btn-cancel" :disabled="isSaving">Cancel</button>
                <button type="submit" class="btn-save" :disabled="isSaving">{{ isSaving ? 'Saving...' : 'Save Changes' }}</button>
              </div>
            </form>

          </div>
        </div>

      </div>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, computed, onMounted } from 'vue';
import { useRouter } from 'vue-router';
import Sidebar from '../components/Sidebar.vue';
import Topbar from '../components/Topbar.vue';
import brandLogo from '../assets/images/logo-hydroponic.png';
import { UsersService, ApiError, type UserOut, type MessageResponse } from '../api';
import { authState } from '../auth';

type ProfileData = {
  userid: string;
  fullname: string;
  username: string;
  email: string;
  phone_number: string;
  role: string;
  created_at: string;
};

const router = useRouter();
const isEditing = ref(false);
const isSaving = ref(false);
const fetchError = ref('');
const saveSuccess = ref('');

const userProfile = reactive<ProfileData>({
  userid: '',
  fullname: '',
  username: '',
  email: '',
  phone_number: '',
  role: '',
  created_at: ''
});

const editForm = reactive<ProfileData>({ ...userProfile });

const displayRole = computed(() => {
  if (!userProfile.role) return '-';
  return userProfile.role.charAt(0).toUpperCase() + userProfile.role.slice(1);
});

const joinedDate = computed(() => {
  if (!userProfile.created_at) return '-';
  const parsed = new Date(userProfile.created_at);
  if (Number.isNaN(parsed.getTime())) return '-';
  return parsed.toLocaleDateString('en-US', { day: '2-digit', month: 'short', year: 'numeric' });
});

const avatarInitials = computed(() => {
  const source = userProfile.fullname || userProfile.username;
  if (!source) return 'U';

  const parts = source.trim().split(/\s+/).filter(Boolean);
  const first = parts[0] || '';
  const second = parts[1] || '';

  if (!first) return 'U';
  if (!second) return first.slice(0, 2).toUpperCase();

  return `${first[0] || ''}${second[0] || ''}`.toUpperCase();
});

const normalizeUser = (user: UserOut): ProfileData => ({
  userid: user.userid,
  fullname: user.fullname || '',
  username: user.username,
  email: user.email || '',
  phone_number: user.phone_number || '',
  role: user.role,
  created_at: user.created_at
});

const syncProfile = (user: UserOut) => {
  const normalized = normalizeUser(user);
  Object.assign(userProfile, normalized);
  Object.assign(editForm, normalized);

  const token = localStorage.getItem('token');
  if (token) {
    authState.setSession(token, user);
  }
};

const loadCurrentUser = async () => {
  fetchError.value = '';

  try {
    const current = await UsersService.getCurrentUser();
    syncProfile(current);
  } catch (error: unknown) {
    if (error instanceof ApiError) {
      if (error.status === 401) {
        authState.logout();
        await router.push('/login');
        return;
      }
      fetchError.value = (error.body as MessageResponse)?.detail || 'Failed to load user profile.';
      return;
    }

    fetchError.value = 'Failed to load user profile.';
  }
};

const startEdit = () => {
  saveSuccess.value = '';
  Object.assign(editForm, userProfile);
  isEditing.value = true;
};

const cancelEdit = () => {
  isEditing.value = false;
  saveSuccess.value = '';
  Object.assign(editForm, userProfile);
};

const saveProfile = async () => {
  if (!userProfile.userid) return;

  isSaving.value = true;
  saveSuccess.value = '';
  fetchError.value = '';

  try {
    const updated = await UsersService.updateUser(userProfile.userid, {
      fullname: editForm.fullname,
      username: editForm.username,
      phone_number: editForm.phone_number
    });

    syncProfile(updated);
    isEditing.value = false;
    saveSuccess.value = 'Profile updated successfully.';
  } catch (error: unknown) {
    if (error instanceof ApiError) {
      fetchError.value = (error.body as MessageResponse)?.detail || 'Failed to update profile.';
      return;
    }

    fetchError.value = 'Failed to update profile.';
  } finally {
    isSaving.value = false;
  }
};

onMounted(async () => {
  if (!authState.isLoggedIn) {
    await router.push('/login');
    return;
  }

  await loadCurrentUser();
});
</script>

<style scoped>
* {
  box-sizing: border-box;
}

.layout-wrapper {
  display: flex;
  width: 100%;
  min-height: 100vh;
  background-color: #f8fafc;
}

.main-content {
  flex: 1;
  min-width: 0;
  padding: 24px 32px;
}

.profile-page {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  color: #1e293b;
  display: flex;
  flex-direction: column;
  width: 100%;
}

.profile-container {
  display: grid;
  grid-template-columns: 320px 1fr;
  gap: 24px;
  align-items: start;
}

.profile-card {
  background-color: #ffffff;
  border: 1px solid #e2e8f0;
  border-radius: 12px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.02);
  overflow: hidden;
  text-align: center;
  padding-bottom: 24px;
}

.profile-header-bg {
  height: 100px;
  background: linear-gradient(135deg, #4caf50 0%, #2e7d32 100%);
}

.avatar-large {
  width: 80px;
  height: 80px;
  background-color: #ffffff;
  border: 4px solid #ffffff;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 28px;
  font-weight: bold;
  color: #4caf50;
  margin: -40px auto 16px;
  box-shadow: 0 4px 6px rgba(0,0,0,0.1);
}

.profile-name {
  font-size: 18px;
  font-weight: 700;
  margin: 0 0 4px 0;
  color: #0f172a;
}

.profile-role {
  font-size: 14px;
  color: #64748b;
  margin: 0 0 24px 0;
}

.profile-stats {
  display: flex;
  justify-content: center;
  align-items: center;
  border-top: 1px solid #e2e8f0;
  padding-top: 20px;
}

.stat-item {
  display: flex;
  flex-direction: column;
  padding: 0 24px;
}

.stat-value {
  font-size: 16px;
  font-weight: 700;
  color: #0f172a;
}

.stat-label {
  font-size: 12px;
  color: #64748b;
  margin-top: 4px;
}

.stat-divider {
  width: 1px;
  height: 30px;
  background-color: #e2e8f0;
}

.details-card {
  background-color: #ffffff;
  border: 1px solid #e2e8f0;
  border-radius: 12px;
  padding: 24px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.02);
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 24px;
  padding-bottom: 16px;
  border-bottom: 1px solid #e2e8f0;
}

.card-header h3 {
  margin: 0;
  font-size: 18px;
  color: #0f172a;
}

.btn-edit {
  display: flex;
  align-items: center;
  gap: 8px;
  background-color: #f1f5f9;
  color: #0f172a;
  border: none;
  padding: 8px 16px;
  border-radius: 6px;
  font-size: 13px;
  font-weight: 600;
  cursor: pointer;
  transition: 0.2s;
}

.btn-edit:hover {
  background-color: #e2e8f0;
}

.profile-form {
  display: flex;
  flex-direction: column;
  gap: 24px;
}

.form-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 20px;
}

.form-group {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.form-group.full-width {
  grid-column: 1 / -1;
}

.form-group label {
  font-size: 13px;
  font-weight: 600;
  color: #475569;
}

.form-group input {
  padding: 10px 12px;
  border: 1px solid #cbd5e1;
  border-radius: 6px;
  font-size: 14px;
  color: #0f172a;
  outline: none;
  transition: all 0.2s;
  background-color: #ffffff;
}

.form-group input:focus {
  border-color: #4caf50;
  box-shadow: 0 0 0 2px rgba(76, 175, 80, 0.2);
}

.form-group input.is-disabled {
  background-color: #f8fafc;
  color: #64748b;
  border-color: #e2e8f0;
  cursor: not-allowed;
}

.form-actions {
  display: flex;
  justify-content: flex-end;
  gap: 12px;
  margin-top: 10px;
}

.btn-cancel {
  background-color: #ffffff;
  color: #475569;
  border: 1px solid #cbd5e1;
  padding: 10px 20px;
  border-radius: 6px;
  font-size: 14px;
  font-weight: 600;
  cursor: pointer;
  transition: 0.2s;
}

.btn-cancel:hover {
  background-color: #f8fafc;
}

.btn-save {
  background-color: #4caf50;
  color: #ffffff;
  border: none;
  padding: 10px 20px;
  border-radius: 6px;
  font-size: 14px;
  font-weight: 600;
  cursor: pointer;
  transition: 0.2s;
}

.btn-save:hover {
  background-color: #43a047;
}

.feedback {
  margin-bottom: 12px;
  font-size: 13px;
  font-weight: 600;
}

.feedback.error {
  color: #b91c1c;
}

.feedback.success {
  color: #15803d;
}

@media (max-width: 1024px) {
  .profile-container {
    grid-template-columns: 1fr;
  }
}

@media (max-width: 768px) {
  .main-content { margin-left: 0; max-width: 100%; padding: 16px; }
  .form-grid { grid-template-columns: 1fr; }
}
</style>