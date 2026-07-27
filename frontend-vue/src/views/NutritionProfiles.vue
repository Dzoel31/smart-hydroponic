<template>
  <div class="layout-wrapper">
    <Sidebar :logo="brandLogo" />

    <main class="main-content">
      <Topbar title="Nutrition Profiles" />

      <div v-if="showSuccessModal" class="success-modal-overlay" role="presentation">
        <section class="success-modal" role="dialog" aria-modal="true" aria-labelledby="success-title" aria-describedby="success-message">
          <div class="success-modal__icon" aria-hidden="true">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M20 6 9 17l-5-5"></path>
            </svg>
          </div>
          <h3 id="success-title" class="success-modal__title">Proses Berhasil</h3>
          <p id="success-message" class="success-modal__message">Profil nutrisi berhasil disimpan. Halaman akan dimuat ulang otomatis.</p>
        </section>
      </div>

      <section class="dashboard-card">
        <div class="card-header compact">
          <div>
            <h2>Profil untuk Dashboard</h2>
            <p>Pilih profil nutrisi yang akan dipakai sebagai acuan Analytics dan tampilan dashboard.</p>
          </div>
        </div>

        <div class="dashboard-selector">
          <div class="form-group dashboard-selector__field">
            <label>Profil aktif</label>
            <select v-model="activeProfileId" :disabled="isSavingActiveProfile || profiles.length === 0">
              <option value="">Pilih profil nutrisi</option>
              <option v-for="profile in profiles" :key="profile.nutrition_id" :value="profile.nutrition_id">
                {{ profile.plant_name }}{{ profile.is_active ? ' - aktif' : '' }}
              </option>
            </select>
          </div>

          <button
            type="button"
            class="btn-primary dashboard-selector__button"
            :disabled="!activeProfileId || isSavingActiveProfile || profiles.length === 0"
            @click="applyActiveProfile"
          >
            {{ isSavingActiveProfile ? 'Menyimpan...' : 'Gunakan untuk Dashboard' }}
          </button>
        </div>

        <p v-if="activeProfileLabel" class="dashboard-selector__hint">
          Profil aktif saat ini: <strong>{{ activeProfileLabel }}</strong>
        </p>
        <p v-else class="dashboard-selector__hint muted">
          Belum ada profil aktif. Pilih salah satu profil dan simpan untuk mengisi dashboard.
        </p>
      </section>

      <section class="editor-card">
        <div class="card-header">
          <div>
            <h2>Input Kebutuhan Nutrisi Tanaman</h2>
            <p>Isi profil untuk membantu analisis grafik dan acuan kebutuhan tanaman.</p>
          </div>
          <button
            v-if="editingProfileId"
            type="button"
            class="btn-ghost"
            @click="resetForm"
          >
            Batal Edit
          </button>
        </div>

        <p v-if="message && !showSuccessModal" class="feedback" :class="messageType">{{ message }}</p>

        <form class="nutrition-form" @submit.prevent="submitProfile">
          <div class="form-grid">
            <div class="form-group full-span">
              <label>Jenis Tanaman</label>
              <input v-model.trim="form.plant_name" type="text" placeholder="Contoh: Selada Hijau" required />
            </div>

            <div class="form-group">
              <label>Moisture Minimum (%)</label>
              <input v-model.number="form.moisture_min" type="number" min="0" max="100" step="0.1" required />
            </div>
            <div class="form-group">
              <label>Moisture Maksimum (%)</label>
              <input v-model.number="form.moisture_max" type="number" min="0" max="100" step="0.1" required />
            </div>

            <div class="form-group">
              <label>pH Minimum</label>
              <input v-model.number="form.ph_min" type="number" min="0" max="14" step="0.1" required />
            </div>
            <div class="form-group">
              <label>pH Maksimum</label>
              <input v-model.number="form.ph_max" type="number" min="0" max="14" step="0.1" required />
            </div>

            <div class="form-group">
              <label>TDS Minimum (ppm)</label>
              <input v-model.number="form.tds_min" type="number" min="0" step="1" required />
            </div>
            <div class="form-group">
              <label>TDS Maksimum (ppm)</label>
              <input v-model.number="form.tds_max" type="number" min="0" step="1" required />
            </div>

            <div class="form-group">
              <label>Suhu Minimum (°C)</label>
              <input v-model.number="form.temperature_min" type="number" min="-10" max="60" step="0.1" required />
            </div>
            <div class="form-group">
              <label>Suhu Maksimum (°C)</label>
              <input v-model.number="form.temperature_max" type="number" min="-10" max="60" step="0.1" required />
            </div>

            <div class="form-group">
              <label>Kelembaban Minimum (%)</label>
              <input v-model.number="form.humidity_min" type="number" min="0" max="100" step="0.1" required />
            </div>
            <div class="form-group">
              <label>Kelembaban Maksimum (%)</label>
              <input v-model.number="form.humidity_max" type="number" min="0" max="100" step="0.1" required />
            </div>

            <div class="form-group full-span">
              <label>Catatan</label>
              <textarea v-model.trim="form.notes" rows="4" placeholder="Catatan tambahan kebutuhan tanaman..."></textarea>
            </div>
          </div>

          <div class="form-actions">
            <button type="button" class="btn-secondary" @click="resetForm" :disabled="isSaving">
              Reset
            </button>
            <button type="submit" class="btn-primary" :disabled="isSaving">
              {{ isSaving ? 'Menyimpan...' : editingProfileId ? 'Update Profile' : 'Simpan Profile' }}
            </button>
          </div>
        </form>
      </section>

      <section class="table-card">
        <div class="card-header compact">
          <div>
            <h2>Daftar Profil Nutrisi</h2>
            <p>Gunakan data ini sebagai acuan analisis pada halaman Analytics.</p>
          </div>
        </div>

        <div class="table-container">
          <table class="nutrition-table">
            <thead>
              <tr>
                <th>Jenis Tanaman</th>
                <th>Moisture</th>
                <th>pH</th>
                <th>TDS</th>
                <th>Suhu</th>
                <th>Kelembaban</th>
                <th>Catatan</th>
                <th>Aksi</th>
              </tr>
            </thead>
            <tbody>
              <tr v-if="isLoading">
                <td colspan="8" class="empty-state">Memuat profil nutrisi...</td>
              </tr>
              <tr v-else-if="profiles.length === 0">
                <td colspan="8" class="empty-state">Belum ada profil nutrisi tersimpan.</td>
              </tr>
              <tr v-for="profile in profiles" :key="profile.nutrition_id">
                <td class="font-medium">
                  <span>{{ profile.plant_name }}</span>
                  <span v-if="profile.is_active" class="active-badge">Aktif</span>
                </td>
                <td>{{ formatRange(profile.moisture_min, profile.moisture_max, '%') }}</td>
                <td>{{ formatRange(profile.ph_min, profile.ph_max) }}</td>
                <td>{{ formatRange(profile.tds_min, profile.tds_max, 'ppm') }}</td>
                <td>{{ formatRange(profile.temperature_min, profile.temperature_max, '°C') }}</td>
                <td>{{ formatRange(profile.humidity_min, profile.humidity_max, '%') }}</td>
                <td class="notes-cell">{{ profile.notes || '-' }}</td>
                <td class="action-cell">
                  <button type="button" class="btn-inline" @click="startEdit(profile)">Edit</button>
                  <button type="button" class="btn-inline danger" @click="removeProfile(profile.nutrition_id)">Hapus</button>
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      </section>
    </main>
  </div>
</template>

<script setup lang="ts">
import { onBeforeUnmount, onMounted, reactive, ref } from 'vue';
import { useRouter } from 'vue-router';
import Sidebar from '@/components/Sidebar.vue';
import Topbar from '@/components/Topbar.vue';
import brandLogo from '@/assets/images/logo-hydroponic.png';
import {
  ApiError,
  PlantNutritionProfilesService,
  type PlantNutritionProfileCreate,
  type PlantNutritionProfileOut,
} from '../api';
import { authState } from '../auth';
import { getApiErrorMessage } from '../utils/apiError';

const router = useRouter();
const profiles = ref<PlantNutritionProfileOut[]>([]);
const activeProfileId = ref('');
const activeProfileLabel = ref('');
const isLoading = ref(false);
const isSaving = ref(false);
const isSavingActiveProfile = ref(false);
const message = ref('');
const messageType = ref<'success' | 'error' | ''>('');
const editingProfileId = ref<string | null>(null);
const showSuccessModal = ref(false);
let reloadTimer: number | null = null;

const emptyForm = (): PlantNutritionProfileCreate => ({
  plant_name: '',
  moisture_min: 0,
  moisture_max: 0,
  ph_min: 0,
  ph_max: 0,
  tds_min: 0,
  tds_max: 0,
  temperature_min: 0,
  temperature_max: 0,
  humidity_min: 0,
  humidity_max: 0,
  notes: '',
});

const form = reactive<PlantNutritionProfileCreate>(emptyForm());

const resetForm = () => {
  Object.assign(form, emptyForm());
  editingProfileId.value = null;
};

const normalizeForm = (): PlantNutritionProfileCreate => ({
  plant_name: form.plant_name.trim(),
  moisture_min: Number(form.moisture_min),
  moisture_max: Number(form.moisture_max),
  ph_min: Number(form.ph_min),
  ph_max: Number(form.ph_max),
  tds_min: Number(form.tds_min),
  tds_max: Number(form.tds_max),
  temperature_min: Number(form.temperature_min),
  temperature_max: Number(form.temperature_max),
  humidity_min: Number(form.humidity_min),
  humidity_max: Number(form.humidity_max),
  notes: form.notes?.trim() || null,
});

const formatRange = (min: number, max: number, unit = '') => {
  return `${min.toFixed(1)} - ${max.toFixed(1)}${unit ? ` ${unit}` : ''}`;
};

const syncActiveProfileLabel = () => {
  const activeProfile = profiles.value.find((profile) => profile.nutrition_id === activeProfileId.value);
  activeProfileLabel.value = activeProfile?.plant_name ?? '';
};

const loadProfiles = async () => {
  isLoading.value = true;

  try {
    const [response, activeProfile] = await Promise.all([
      PlantNutritionProfilesService.getNutritionProfiles(1, 100),
      PlantNutritionProfilesService.getActiveNutritionProfile().catch((error) => {
        if (error instanceof ApiError && error.status === 404) {
          return null;
        }
        throw error;
      }),
    ]);

    profiles.value = response.data;
    activeProfileId.value = activeProfile?.nutrition_id || profiles.value.find((profile) => profile.is_active)?.nutrition_id || '';
    syncActiveProfileLabel();
  } catch (error) {
    if (error instanceof ApiError) {
      message.value = getApiErrorMessage(error, 'Gagal mengambil data profil nutrisi.');
      messageType.value = 'error';
      if (error.status === 401) {
        authState.logout();
        await router.push('/login');
      }
      if (error.status === 403) {
        await router.push('/dashboard');
      }
    } else {
      message.value = 'Terjadi gangguan saat mengambil data profil nutrisi.';
      messageType.value = 'error';
    }
  } finally {
    isLoading.value = false;
  }
};

const applyActiveProfile = async () => {
  if (!activeProfileId.value) {
    return;
  }

  isSavingActiveProfile.value = true;
  message.value = '';

  try {
    await PlantNutritionProfilesService.activateNutritionProfile(activeProfileId.value);
    message.value = 'Profil dashboard berhasil diperbarui.';
    messageType.value = 'success';
    await loadProfiles();
  } catch (error) {
    if (error instanceof ApiError) {
      message.value = getApiErrorMessage(error, 'Gagal memperbarui profil dashboard.');
      messageType.value = 'error';
      if (error.status === 401) {
        authState.logout();
        await router.push('/login');
      }
      if (error.status === 403) {
        await router.push('/dashboard');
      }
    } else {
      message.value = 'Terjadi gangguan saat memperbarui profil dashboard.';
      messageType.value = 'error';
    }
  } finally {
    isSavingActiveProfile.value = false;
  }
};

const startEdit = (profile: PlantNutritionProfileOut) => {
  editingProfileId.value = profile.nutrition_id;
  Object.assign(form, {
    plant_name: profile.plant_name,
    moisture_min: profile.moisture_min,
    moisture_max: profile.moisture_max,
    ph_min: profile.ph_min,
    ph_max: profile.ph_max,
    tds_min: profile.tds_min,
    tds_max: profile.tds_max,
    temperature_min: profile.temperature_min,
    temperature_max: profile.temperature_max,
    humidity_min: profile.humidity_min,
    humidity_max: profile.humidity_max,
    notes: profile.notes || '',
  });
  message.value = '';
  messageType.value = '';
};

const submitProfile = async () => {
  isSaving.value = true;
  message.value = '';

  try {
    const payload = normalizeForm();

    if (editingProfileId.value) {
      await PlantNutritionProfilesService.updateNutritionProfile(editingProfileId.value, payload);
    } else {
      await PlantNutritionProfilesService.createNutritionProfile(payload);
    }

    resetForm();
    messageType.value = 'success';
    message.value = 'Profil nutrisi berhasil disimpan.';
    showSuccessModal.value = true;

    if (reloadTimer !== null) {
      window.clearTimeout(reloadTimer);
    }

    reloadTimer = window.setTimeout(async () => {
      showSuccessModal.value = false;
      await loadProfiles();
      window.location.reload();
    }, 1500);
  } catch (error) {
    if (error instanceof ApiError) {
      message.value = getApiErrorMessage(error, 'Gagal menyimpan profil nutrisi.');
      messageType.value = 'error';
      if (error.status === 401) {
        authState.logout();
        await router.push('/login');
      }
      if (error.status === 403) {
        await router.push('/dashboard');
      }
    } else {
      message.value = 'Terjadi gangguan saat menyimpan profil nutrisi.';
      messageType.value = 'error';
    }
  } finally {
    isSaving.value = false;
  }
};

const removeProfile = async (nutritionId: string) => {
  const confirmed = window.confirm('Hapus profil nutrisi ini?');
  if (!confirmed) return;

  try {
    await PlantNutritionProfilesService.deleteNutritionProfile(nutritionId);
    message.value = 'Profil nutrisi berhasil dihapus.';
    messageType.value = 'success';
    if (editingProfileId.value === nutritionId) {
      resetForm();
    }
    await loadProfiles();
  } catch (error) {
    if (error instanceof ApiError) {
      message.value = getApiErrorMessage(error, 'Gagal menghapus profil nutrisi.');
      messageType.value = 'error';
    } else {
      message.value = 'Terjadi gangguan saat menghapus profil nutrisi.';
      messageType.value = 'error';
    }
  }
};

onMounted(async () => {
  await loadProfiles();
});

onBeforeUnmount(() => {
  if (reloadTimer !== null) {
    window.clearTimeout(reloadTimer);
  }
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
  color: #0f172a;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
}

.main-content {
  flex: 1;
  min-width: 0;
  padding: 24px 32px;
}

.editor-card,
.table-card {
  background: #ffffff;
  border: 1px solid #e2e8f0;
  border-radius: 16px;
  box-shadow: 0 1px 3px rgba(15, 23, 42, 0.04);
}

.editor-card,
.table-card {
  padding: 22px;
  margin-bottom: 20px;
}

.dashboard-card {
  background: #ffffff;
  color: #0f172a;
  border: 1px solid #e2e8f0;
  border-radius: 18px;
  padding: 22px;
  margin-bottom: 20px;
  box-shadow: 0 1px 3px rgba(15, 23, 42, 0.04);
}

.dashboard-card .card-header h2,
.dashboard-card .card-header p {
  color: inherit;
}

.dashboard-selector {
  display: flex;
  align-items: end;
  gap: 14px;
}

.dashboard-selector__field {
  flex: 1;
}

.dashboard-selector__field select {
  width: 100%;
  border: 1px solid rgba(148, 163, 184, 0.45);
  border-radius: 12px;
  padding: 12px 14px;
  font: inherit;
  color: #0f172a;
  background: #ffffff;
}

.dashboard-selector__button {
  white-space: nowrap;
}

.dashboard-selector__hint {
  margin: 14px 0 0;
  font-size: 14px;
  line-height: 1.6;
}

.dashboard-selector__hint.muted {
  color: #64748b;
}

.active-badge {
  display: inline-flex;
  align-items: center;
  margin-left: 8px;
  padding: 4px 8px;
  border-radius: 999px;
  background: #dcfce7;
  color: #166534;
  font-size: 11px;
  font-weight: 700;
  vertical-align: middle;
}

.success-modal-overlay {
  position: fixed;
  inset: 0;
  z-index: 2100;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 24px;
  background: rgba(15, 23, 42, 0.55);
  backdrop-filter: blur(6px);
}

.success-modal {
  width: min(100%, 420px);
  border-radius: 20px;
  background: #ffffff;
  padding: 28px;
  text-align: center;
  box-shadow: 0 24px 60px rgba(15, 23, 42, 0.22);
}

.success-modal__icon {
  width: 58px;
  height: 58px;
  margin: 0 auto 16px;
  border-radius: 999px;
  display: grid;
  place-items: center;
  color: #16a34a;
  background: #dcfce7;
}

.success-modal__icon svg {
  width: 30px;
  height: 30px;
}

.success-modal__title {
  margin: 0;
  font-size: 1.3rem;
  color: #0f172a;
}

.success-modal__message {
  margin: 12px 0 0;
  color: #475569;
  line-height: 1.6;
  font-size: 0.95rem;
}

.card-header {
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
  gap: 16px;
  margin-bottom: 18px;
}

.card-header.compact {
  margin-bottom: 12px;
}

.card-header h2 {
  margin: 0 0 6px;
  font-size: 18px;
}

.card-header p {
  margin: 0;
  color: #64748b;
  font-size: 14px;
}

.btn-ghost,
.btn-secondary,
.btn-primary,
.btn-inline {
  border: none;
  border-radius: 10px;
  cursor: pointer;
  transition: transform 0.15s ease, opacity 0.15s ease;
}

.btn-ghost,
.btn-secondary,
.btn-inline {
  background: #e2e8f0;
  color: #0f172a;
}

.btn-primary {
  background: linear-gradient(135deg, #16a34a, #15803d);
  color: #ffffff;
}

.btn-inline.danger {
  background: #fee2e2;
  color: #b91c1c;
}

.btn-ghost:hover,
.btn-secondary:hover,
.btn-primary:hover,
.btn-inline:hover {
  transform: translateY(-1px);
}

.feedback {
  margin: 0 0 16px;
  padding: 12px 14px;
  border-radius: 12px;
  font-size: 14px;
}

.feedback.success {
  background: #dcfce7;
  color: #166534;
}

.feedback.error {
  background: #fee2e2;
  color: #b91c1c;
}

.nutrition-form {
  display: flex;
  flex-direction: column;
  gap: 18px;
}

.form-grid {
  display: grid;
  grid-template-columns: repeat(2, minmax(0, 1fr));
  gap: 16px;
}

.form-group {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.form-group.full-span {
  grid-column: 1 / -1;
}

.form-group label {
  font-size: 13px;
  font-weight: 600;
  color: #334155;
}

.form-group input,
.form-group textarea,
.form-group select {
  width: 100%;
  border: 1px solid #cbd5e1;
  border-radius: 12px;
  padding: 12px 14px;
  font: inherit;
  color: #0f172a;
  background: #ffffff;
}

.form-group textarea {
  resize: vertical;
  min-height: 96px;
}

.form-actions {
  display: flex;
  justify-content: flex-end;
  gap: 12px;
}

.btn-secondary,
.btn-primary {
  padding: 12px 18px;
  font-weight: 700;
}

.table-container {
  overflow-x: auto;
}

.nutrition-table {
  width: 100%;
  border-collapse: collapse;
}

.nutrition-table th,
.nutrition-table td {
  border-bottom: 1px solid #e2e8f0;
  padding: 14px 12px;
  text-align: left;
  vertical-align: top;
}

.nutrition-table th {
  font-size: 13px;
  color: #64748b;
  text-transform: uppercase;
  letter-spacing: 0.04em;
}

.font-medium {
  font-weight: 600;
}

.empty-state {
  text-align: center;
  color: #64748b;
  padding: 32px 12px;
}

.notes-cell {
  max-width: 260px;
  color: #475569;
}

.action-cell {
  display: flex;
  gap: 8px;
  flex-wrap: wrap;
}

.btn-inline {
  padding: 8px 12px;
  font-size: 13px;
}

@media (max-width: 1024px) {
  .form-grid {
    grid-template-columns: 1fr;
  }

  .main-content {
    padding: 20px;
  }
}
</style>
