<template>
  <div class="layout-wrapper">
    <Sidebar :logo="brandLogo" />
    
    <main class="main-content">
      <Topbar title="Manajemen Admin" />

      <div class="admin-container">
        <div class="metrics-row">
          <div class="metric-card">
            <div class="metric-header">
              <span>Menunggu Persetujuan</span>
              <span class="metric-icon">⏳</span>
            </div>
            <div class="metric-value text-orange">{{ pendingCount }} <span class="unit">Akun</span></div>
          </div>
          <div class="metric-card">
            <div class="metric-header">
              <span>Total Admin Aktif</span>
              <span class="metric-icon">✅</span>
            </div>
            <div class="metric-value text-green">{{ activeCount }} <span class="unit">Akun</span></div>
          </div>
        </div>

        <div class="admin-card">
          <div class="tabs-header">
            <button 
              class="tab-btn" 
              :class="{ active: currentTab === 'pending' }" 
              @click="currentTab = 'pending'"
            >
              Menunggu Persetujuan
              <span class="badge-count" v-if="pendingCount > 0">{{ pendingCount }}</span>
            </button>
            <button 
              class="tab-btn" 
              :class="{ active: currentTab === 'active' }" 
              @click="currentTab = 'active'"
            >
              Admin Aktif & Ditangguhkan
            </button>
          </div>

          <div class="table-container">
            <table class="admin-table">
              <thead>
                <tr>
                  <th>Nama Lengkap</th>
                  <th>Email</th>
                  <th>Tanggal Daftar</th>
                  <th>Status</th>
                  <th>Aksi</th>
                </tr>
              </thead>
              <tbody>
                <tr v-if="filteredAdmins.length === 0">
                  <td colspan="5" class="empty-state">Tidak ada data untuk ditampilkan.</td>
                </tr>
                <tr v-for="admin in filteredAdmins" :key="admin.id">
                  <td class="font-medium">{{ admin.name }}</td>
                  <td class="text-gray">{{ admin.email }}</td>
                  <td class="text-gray">{{ admin.date }}</td>
                  <td>
                    <span class="status-badge" :class="admin.status">
                      {{ formatStatus(admin.status) }}
                    </span>
                  </td>
                  <td class="action-cell">
                    <template v-if="admin.status === 'pending'">
                      <button class="btn btn-approve" @click="handleApprove(admin.id)">Setujui</button>
                      <button class="btn btn-reject" @click="handleReject(admin.id)">Tolak</button>
                    </template>

                    <template v-else-if="admin.status === 'active'">
                      <button class="btn btn-suspend" @click="handleSuspend(admin.id)">Cabut Akses</button>
                    </template>

                    <template v-else-if="admin.status === 'suspended'">
                      <button class="btn btn-approve outline" @click="handleApprove(admin.id)">Pulihkan</button>
                    </template>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>

        </div>
      </div>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue';
import Sidebar from '@/components/Sidebar.vue';
import Topbar from '@/components/Topbar.vue';
import brandLogo from '@/assets/images/logo-hydroponic.png';

// State untuk mengatur tab yang sedang aktif
const currentTab = ref('pending');

// Data Mockup Admin (Nantinya diganti dengan fetch dari API Backend)
const admins = ref([
  { id: 1, name: 'Budi Santoso', email: 'budi@upnvj.ac.id', date: '2026-03-27', status: 'pending' },
  { id: 2, name: 'Rina Wati', email: 'rina@upnvj.ac.id', date: '2026-03-26', status: 'pending' },
  { id: 3, name: 'Dimas Aditya', email: 'dimas@upnvj.ac.id', date: '2026-02-15', status: 'active' },
  { id: 4, name: 'Siti Nurhaliza', email: 'siti@upnvj.ac.id', date: '2026-01-10', status: 'active' },
  { id: 5, name: 'Ahmad Reza', email: 'ahmad@upnvj.ac.id', date: '2025-11-20', status: 'suspended' }
]);

// Computed Properties untuk Metrik & Filter
const pendingCount = computed(() => admins.value.filter(a => a.status === 'pending').length);
const activeCount = computed(() => admins.value.filter(a => a.status === 'active').length);

const filteredAdmins = computed(() => {
  if (currentTab.value === 'pending') {
    return admins.value.filter(a => a.status === 'pending');
  } else {
    // Menampilkan yang active dan suspended
    return admins.value.filter(a => a.status !== 'pending');
  }
});

// Helper untuk format teks status
const formatStatus = (status: string) => {
  const map: Record<string, string> = {
    'pending': 'Menunggu',
    'active': 'Aktif',
    'suspended': 'Ditangguhkan'
  };
  return map[status] || status;
};

// --- FUNGSI AKSI (Simulasi) ---
const handleApprove = (id: number) => {
  if (confirm('Apakah Anda yakin ingin menyetujui akses admin ini?')) {
    const admin = admins.value.find(a => a.id === id);
    if (admin) admin.status = 'active';
    // Nantinya di sini panggil: axios.put(`/api/admins/${id}/approve`)
  }
};

const handleReject = (id: number) => {
  if (confirm('Tolak dan hapus data pendaftar ini?')) {
    admins.value = admins.value.filter(a => a.id !== id);
    // Nantinya di sini panggil: axios.delete(`/api/admins/${id}`)
  }
};

const handleSuspend = (id: number) => {
  if (confirm('Apakah Anda yakin ingin mencabut akses admin ini? Mereka tidak akan bisa login lagi.')) {
    const admin = admins.value.find(a => a.id === id);
    if (admin) admin.status = 'suspended';
    // Nantinya di sini panggil: axios.put(`/api/admins/${id}/suspend`)
  }
};
</script>

<style scoped>
* { box-sizing: border-box; }

.layout-wrapper {
  display: flex;
  width: 100%;
  min-height: 100vh;
  background-color: #f8fafc;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  color: #1e293b;
}

.main-content {
  flex: 1;
  min-width: 0;
  padding: 24px 32px;
}

.admin-container {
  display: flex;
  flex-direction: column;
  gap: 24px;
  margin-top: 20px;
}

/* ... (Kode Metrics, Tabs, Table, Badge TETAP SAMA) ... */
.metrics-row { display: grid; grid-template-columns: repeat(2, minmax(250px, 300px)); gap: 20px; }
.metric-card { background-color: #ffffff; padding: 20px; border-radius: 12px; border: 1px solid #e2e8f0; box-shadow: 0 1px 3px rgba(0,0,0,0.02); }
.metric-header { display: flex; justify-content: space-between; align-items: center; color: #64748b; font-size: 14px; font-weight: 600; margin-bottom: 12px; }
.metric-value { font-size: 28px; font-weight: 700; }
.text-orange { color: #f59e0b; }
.text-green { color: #16a34a; }
.unit { font-size: 14px; color: #94a3b8; font-weight: 500; }

.admin-card { background-color: #ffffff; border-radius: 12px; border: 1px solid #e2e8f0; box-shadow: 0 1px 3px rgba(0,0,0,0.02); overflow: hidden; }
.tabs-header { display: flex; border-bottom: 1px solid #e2e8f0; background-color: #f8fafc; padding: 0 16px; }
.tab-btn { padding: 16px 24px; background: none; border: none; border-bottom: 3px solid transparent; font-size: 15px; font-weight: 600; color: #64748b; cursor: pointer; display: flex; align-items: center; gap: 8px; }
.tab-btn.active { color: #16a34a; border-bottom-color: #16a34a; background-color: #ffffff; }
.badge-count { background-color: #ef4444; color: white; font-size: 12px; padding: 2px 8px; border-radius: 12px; }

.table-container { overflow-x: auto; padding: 16px; }
.admin-table { width: 100%; border-collapse: collapse; text-align: left; }
.admin-table th { padding: 12px 16px; font-size: 13px; font-weight: 600; color: #64748b; border-bottom: 1px solid #e2e8f0; }
.admin-table td { padding: 16px; font-size: 14px; border-bottom: 1px solid #f1f5f9; vertical-align: middle; }
.font-medium { font-weight: 600; color: #0f172a; }
.text-gray { color: #64748b; }
.status-badge { display: inline-block; padding: 4px 12px; border-radius: 20px; font-size: 12px; font-weight: 600; }
.status-badge.pending { background-color: #fef3c7; color: #d97706; }
.status-badge.active { background-color: #dcfce7; color: #15803d; }
.status-badge.suspended { background-color: #fee2e2; color: #b91c1c; }

.action-cell { display: flex; gap: 8px; }
.btn { padding: 6px 12px; border-radius: 6px; font-size: 13px; font-weight: 600; cursor: pointer; border: none; }
.btn-approve { background-color: #22c55e; color: white; }
.btn-reject { background-color: #f1f5f9; color: #ef4444; }
.btn-suspend { background-color: #ef4444; color: white; }
.btn-approve.outline { background-color: transparent; border: 1px solid #22c55e; color: #16a34a; }

/* RESPONSIVE */
@media (max-width: 1024px) {
  .metrics-row { grid-template-columns: 1fr; }
}

@media (max-width: 768px) {
  .main-content { padding: 16px; }
  .tabs-header { flex-direction: column; }
  .tab-btn { width: 100%; justify-content: center; }
  .action-cell { flex-direction: column; }
}
</style>