<template>
  <div class="dashboard-layout">
    <Sidebar :logo="brandLogo" />
    
    <main class="main-content">
      
      <Topbar title="Hydroponic Data Logs" />

      <section class="filter-section">
        <div class="filter-group">
          <label>Start Date</label>
          <div class="input-with-icon">
            <input type="date" v-model="filters.startDate" class="white-date-input" />
          </div>
        </div>

        <div class="filter-group">
          <label>End Date</label>
          <div class="input-with-icon">
            <input type="date" v-model="filters.endDate" class="white-date-input" />
          </div>
        </div>

        <div class="filter-group">
          <label>Category</label>
          <select v-model="filters.categoryType" class="category-select">
            <option value="all">All Data</option>
            <option value="sensor">Sensor</option>
            <option value="actuator">Actuator</option>
            <option value="environment">Environment</option>
          </select>
        </div>

        <div class="action-buttons">
          <button 
            class="btn-reset" 
            @click="resetFilters"
            :disabled="!hasActiveFilters">
            <i class="fas fa-undo"></i> Reset Filter   
          </button>

          <button 
            class="btn-export" 
            @click="exportData"
            :disabled="filteredData.length === 0">
            <i class="fas fa-download"></i> Export Data
          </button>
        </div>
      </section>

      <section class="table-card">
        <div class="table-container">
          <table class="data-table">
            <thead>
              <tr>
                <th>No</th>
                <th>Id</th>
                <th>Timestamp</th>
                <template v-if="showCol('sensor')">
                  <th v-for="n in 6" :key="'m'+n">Moisture {{ n }} (%)</th>
                  <th>Flow Rate (L/m)</th>
                  <th>Total Liters (L)</th>
                  <th>Distance (m)</th>
                </template>

                <template v-if="showCol('environment')">
                  <th>pH</th>
                  <th>TDS (ppm)</th>
                  <th>Temp Atas (°C)</th>
                  <th>Temp Bawah (°C)</th>
                  <th>Hum Atas (%)</th>
                  <th>Hum Bawah (%)</th>
                </template>
              
                <template v-if="showCol('actuator')">
                  <th>Pump Status</th>
                  <th>Light Status</th>
                  <th>Automation Status</th>
                </template>
              </tr>
            </thead>
            <tbody>
              <tr v-for="(item, index) in paginatedData" :key="index">
                <td>{{ index + 1 }}</td>
                <td>{{ item.id }}</td>
                <td>{{ item.timestamp }}</td>
                
                <template v-if="showCol('sensor')">
                  <td>{{ item.moisture1 }}</td>
                  <td>{{ item.moisture2 }}</td>
                  <td>{{ item.moisture3 }}</td>
                  <td>{{ item.moisture4 }}</td>
                  <td>{{ item.moisture5 }}</td>
                  <td>{{ item.moisture6 }}</td>
                  <td>{{ item.flowrate }}</td>
                  <td>{{ item.total_liters }}</td>
                  <td>{{ item.distance }}</td>
                </template>

                <template v-if="showCol('environment')">
                  <td>{{ item.ph !== null ? item.ph.toFixed(2) : '-' }}</td>
                  <td>{{ item.tds }}</td>
                  <td>{{ item.temperature_atas }}</td>
                  <td>{{ item.temperature_bawah }}</td>
                  <td>{{ item.humidity_atas }}</td>
                  <td>{{ item.humidity_bawah }}</td>
                </template>

                <template v-if="showCol('actuator')">
                  <td>
                    <span :class="['badge', item.pump_status ? 'on' : 'off']">
                      {{ item.pump_status ? 'ON' : 'OFF' }}
                    </span>
                  </td>
                  <td>
                    <span :class="['badge', item.light_status ? 'on' : 'off']">
                      {{ item.light_status ? 'ON' : 'OFF' }}
                    </span>
                  </td>
                  <td>
                    <span :class="['badge', item.automation_status ? 'on' : 'off']">
                      {{ item.automation_status ? 'ON' : 'OFF' }}
                    </span>
                  </td>
                </template>
              </tr>
              <tr v-if="isLoading">
                <td colspan="25" style="text-align: center; padding: 40px;">Memuat data...</td>
              </tr>
              <tr v-else-if="fetchError">
                <td colspan="25" style="text-align: center; padding: 40px;">{{ fetchError }}</td>
              </tr>
              <tr v-else-if="filteredData.length === 0">
                <td colspan="25" style="text-align: center; padding: 40px;">Data tidak ditemukan</td>
              </tr>
            </tbody>
          </table>
        </div>

        <div class="pagination-outer-container">
          <div class="pagination-settings">
            <label>Rows per page:</label>
            <select v-model="itemsPerPage" @change="currentPage = 1" class="rows-select">
              <option :value="10">10</option>
              <option :value="25">25</option>
              <option :value="50">50</option>
              <option :value="100">100</option>
            </select>
          </div>

          <div class="pagination-container">
            <button class="pagination-button" :disabled="currentPage === 1" @click="currentPage--">← Prev</button>

            <div class="pagination-info">
              Halaman <strong>{{ currentPage }}</strong> dari <strong>{{ totalPages || 1 }}</strong>
            </div>

            <button class="pagination-button" :disabled="currentPage === totalPages || totalPages === 0" @click="currentPage++">Next →</button>
          </div>

          <div class="pagination-status">
            Showing <strong>{{ dataRange.start }}-{{ dataRange.end }}</strong> of <strong>{{ filteredData.length }}</strong>
          </div>
        </div>
      </section>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, reactive, computed, watch } from 'vue';
import { authState } from "../auth";
import Sidebar from '@/components/Sidebar.vue';
import Topbar from '@/components/Topbar.vue';
import brandLogo from '@/assets/images/logo-hydroponic.png';
import { HydroponicsService, type HydroponicOut } from '../api';
import { getApiErrorMessage } from '../utils/apiError';

// --- State ---
const filters = reactive({
  startDate: '',
  endDate: '',
  categoryType: 'all'
});

const currentPage = ref(1);
const itemsPerPage = ref(10);
const sensorLogs = ref<HydroponicTableItem[]>([]);
const isLoading = ref(false);
const fetchError = ref('');

type HydroponicTableItem = {
  id: string;
  timestamp: string;
  moisture1: number | null;
  moisture2: number | null;
  moisture3: number | null;
  moisture4: number | null;
  moisture5: number | null;
  moisture6: number | null;
  flowrate: number | null;
  total_liters: number | null;
  distance: number | null;
  ph: number | null;
  tds: number | null;
  temperature_atas: number | null;
  temperature_bawah: number | null;
  humidity_atas: number | null;
  humidity_bawah: number | null;
  pump_status: boolean;
  light_status: boolean;
  automation_status: boolean;
};

const mapHydroponicDataToTableItem = (item: HydroponicOut): HydroponicTableItem => {
  return {
    id: item.dataid ?? '-',
    timestamp: item.timestamp,
    moisture1: item.moisture1 ?? null,
    moisture2: item.moisture2 ?? null,
    moisture3: item.moisture3 ?? null,
    moisture4: item.moisture4 ?? null,
    moisture5: item.moisture5 ?? null,
    moisture6: item.moisture6 ?? null,
    flowrate: item.flowrate ?? null,
    total_liters: item.total_litres ?? null,
    distance: item.distance_cm ?? null,
    ph: item.ph ?? null,
    tds: item.tds ?? null,
    temperature_atas: item.temperature_atas ?? null,
    temperature_bawah: item.temperature_bawah ?? null,
    humidity_atas: item.humidity_atas ?? null,
    humidity_bawah: item.humidity_bawah ?? null,
    pump_status: item.pump_status ?? false,
    light_status: item.light_status ?? false,
    automation_status: item.automation_status ?? false
  };
};

const escapeCsvField = (value: unknown): string => {
  if (value === null || value === undefined) return '';

  const strValue = String(value);
  if (/[",\n]/.test(strValue)) {
    return `"${strValue.replace(/"/g, '""')}"`;
  }

  return strValue;
};

const buildCsvFromRows = (rows: Array<Record<string, unknown>>): string => {
  if (!rows.length) return '';

  const headers = Object.keys(rows[0]);
  const headerLine = headers.map(escapeCsvField).join(',');
  const bodyLines = rows.map((row) => headers.map((header) => escapeCsvField(row[header])).join(','));

  return [headerLine, ...bodyLines].join('\n');
};

// --- Fungsi Fetch Data dari API ---
const fetchDataFromAPI = async () => {
  isLoading.value = true;
  fetchError.value = '';

  try {
    const limit = 100;
    let currentApiPage = 1;
    let totalPages = 1;
    const allData: HydroponicOut[] = [];

    do {
      const response = await HydroponicsService.getHydroponicData(currentApiPage, limit);
      allData.push(...response.data);
      totalPages = response.meta.total_pages || 1;
      currentApiPage += 1;
    } while (currentApiPage <= totalPages);

    sensorLogs.value = allData
      .filter((item) => item.timestamp)
      .map(mapHydroponicDataToTableItem);
  } catch (error) {
    const message = getApiErrorMessage(error, 'Gagal mengambil data dari server.');
    console.error('Gagal mengambil data hidroponik:', message);
    fetchError.value = message;
    sensorLogs.value = [];
  } finally {
    isLoading.value = false;
  }
};

onMounted(() => {
  fetchDataFromAPI();
});

// --- Logic Filter & Menampilkan Kolom ---
const showCol = (cat: string) => {
  if (filters.categoryType === 'all') return true;
  return filters.categoryType === cat;
};

const filteredData = computed(() => {
  if (!sensorLogs.value.length) return [];
  
  return sensorLogs.value.filter(item => {
    if (!item.timestamp) return false;

    const itemDate = new Date(item.timestamp).getTime();
    const start = filters.startDate ? new Date(filters.startDate).getTime() : -Infinity;
    const end = filters.endDate ? new Date(filters.endDate).getTime() : Infinity;
    
    return itemDate >= start && itemDate <= (end + 86400000); // + 1 hari
  });
});

// --- FUNGSI EXPORT CSV TERBARU ---
const exportData = () => {
  if (filteredData.value.length === 0) return;
  
  // Memformat data agar sesuai dengan kolom yang SEDANG TAMPIL di tabel
  const dataToExport = filteredData.value.map((item): Record<string, unknown> => {
    // Data dasar yang selalu tampil
    const exportItem: Record<string, unknown> = {
      'Id': item.id,
      'Timestamp': item.timestamp,
    };

    // Tambahkan data Sensor jika kategori 'All' atau 'Sensor'
    if (showCol('sensor')) {
      exportItem['Moisture 1 (%)'] = item.moisture1;
      exportItem['Moisture 2 (%)'] = item.moisture2;
      exportItem['Moisture 3 (%)'] = item.moisture3;
      exportItem['Moisture 4 (%)'] = item.moisture4;
      exportItem['Moisture 5 (%)'] = item.moisture5;
      exportItem['Moisture 6 (%)'] = item.moisture6;
      exportItem['Flow Rate (L/m)'] = item.flowrate;
      exportItem['Total Liters (L)'] = item.total_liters;
      exportItem['Distance (m)'] = item.distance;
    }

    // Tambahkan data Environment jika kategori 'All' atau 'Environment'
    if (showCol('environment')) {
      exportItem['pH'] = item.ph !== null ? Number(item.ph.toFixed(2)) : null;
      exportItem['TDS (ppm)'] = item.tds !== null ? Number(item.tds.toFixed(2)) : null;
      exportItem['Temp Atas (°C)'] = item.temperature_atas;
      exportItem['Temp Bawah (°C)'] = item.temperature_bawah;
      exportItem['Hum Atas (%)'] = item.humidity_atas;
      exportItem['Hum Bawah (%)'] = item.humidity_bawah;
    }

    // Tambahkan data Actuator jika kategori 'All' atau 'Actuator'
    if (showCol('actuator')) {
      exportItem['Pump Status'] = item.pump_status ? 'ON' : 'OFF';
      exportItem['Light Status'] = item.light_status ? 'ON' : 'OFF';
      exportItem['Automation Status'] = item.automation_status ? 'ON' : 'OFF';
    }

    return exportItem;
  });
  
  // Mengubah data JSON yang sudah difilter menjadi string CSV
  const csv = buildCsvFromRows(dataToExport);
  
  // Membuat Blob dan URL untuk trigger download
  const blob = new Blob([csv], { type: 'text/csv;charset=utf-8;' });
  const url = URL.createObjectURL(blob);
  
  // Membuat nama file secara dinamis berdasarkan filter yang sedang aktif
  let fileName = 'hydroponic_data';
  if (filters.categoryType !== 'all') fileName += `_${filters.categoryType}`;
  if (filters.startDate) fileName += `_from_${filters.startDate}`;
  if (filters.endDate) fileName += `_to_${filters.endDate}`;
  fileName += '.csv';

  // Membuat elemen anchor sementara untuk men-download
  const link = document.createElement('a');
  link.setAttribute('href', url);
  link.setAttribute('download', fileName);
  link.style.visibility = 'hidden';
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
};

// --- Pagination Logic ---
const totalPages = computed(() => {
  const count = Math.ceil(filteredData.value.length / itemsPerPage.value);
  return count > 0 ? count : 1;
});

const dataRange = computed(() => {
  if (filteredData.value.length === 0) return { start: 0, end: 0 };
  
  const start = (currentPage.value - 1) * itemsPerPage.value + 1;
  const end = Math.min(currentPage.value * itemsPerPage.value, filteredData.value.length);
  
  return { start, end };
});

const paginatedData = computed(() => {
  const start = (currentPage.value - 1) * itemsPerPage.value;
  return filteredData.value.slice(start, start + itemsPerPage.value);
});

watch([() => filters.categoryType, () => itemsPerPage.value, () => filters.startDate, () => filters.endDate], () => {
  currentPage.value = 1;
});

const hasActiveFilters = computed(() => {
  return filters.startDate !== '' || filters.endDate !== '' || filters.categoryType !== 'all';
});

const resetFilters = () => {
  filters.startDate = '';
  filters.endDate = '';
  filters.categoryType = 'all';
  currentPage.value = 1;
};
</script>

<style scoped>
* {
  box-sizing: border-box;
}

/* Layout dasar */
.dashboard-layout { 
  display: flex; 
  width: 100%;
  background-color: #f8fafc; 
  min-height: 100vh; 
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
}

.main-content { 
  flex: 1; 
  min-width: 0; 
  padding: 24px 32px;
}

.input-with-icon {
  position: relative;
  display: flex;
  align-items: center;
}

/* Filter Section */
.filter-section {
  display: flex; 
  gap: 20px; 
  align-items: flex-end;
  background: white; 
  padding: 20px; 
  border-radius: 12px;
  border: 1px solid #e2e8f0;
  flex-wrap: wrap; 
}

.filter-group { 
  display: flex; 
  flex-direction: column; 
  gap: 8px; 
  flex: 1;
  min-width: 150px;
  max-width: 250px;
}

.filter-group label { 
  font-size: 13px; 
  font-weight: 600; 
  color: #475568; 
}

.white-date-input {
  padding: 8px 12px;
  border: 1px solid #e2e8f0;
  border-radius: 8px;
  background-color: #ffffff;
  color: #1e293b;
  font-family: inherit;
  width: 100%;
  outline: none;
  transition: border-color 0.2s;
  appearance: none;
  height: 42px;
}

.white-date-input:focus { 
  border-color: #3b82f6; 
}

.white-date-input::-webkit-calendar-picker-indicator {
    background-image: url('data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" width="16" height="15" viewBox="0 0 24 24"><path fill="%2364748b" d="M20 3h-1V1h-2v2H7V1H5v2H4c-1.1 0-2 .9-2 2v16c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2zm0 18H4V8h16v13z"/></svg>');
}

.category-select {
  height: 42px;
  width: 100%;
  padding: 8px 12px;
  border: 1px solid #e2e8f0;
  border-radius: 8px;
  background: white;
  min-width: 150px;
  color: #1e293b;
  cursor: pointer;
  box-sizing: border-box;
}

/* Action Buttons Wrapper */
.action-buttons {
  display: flex;
  gap: 12px;
  margin-left: auto;
}

.btn-export {
  display: flex;
  align-items: center;
  gap: 8px;
  background-color: #3b82f6;
  color: white;
  border: none;
  padding: 0 16px;
  border-radius: 8px;
  cursor: pointer;
  height: 42px;
  font-weight: 500;
  white-space: nowrap;
  transition: all 0.2s;
}

.btn-export:disabled {
  background-color: #94a3b8;
  cursor: not-allowed;
}

.btn-export:hover:not(:disabled) {
  background-color: #2563eb;
  transform: translateY(-1px);
}

.btn-reset {
  display: flex;
  align-items: center;
  gap: 8px;
  background-color: #16a34a; 
  color: white; 
  border: none; 
  padding: 0 16px; 
  border-radius: 8px; 
  cursor: pointer; 
  height: 42px; 
  font-weight: 500;
  white-space: nowrap;
  transition: all 0.2s;
}

.btn-reset:disabled { 
  background-color: #86efac; 
  cursor: not-allowed;
}

.btn-reset:hover:not(:disabled) {
  background-color: #dc2626; 
  transform: translateY(-1px);
}

/* Table */
.table-card { background: white; margin-top: 30px; border-radius: 12px; border: 1px solid #e2e8f0; overflow: hidden; }
.table-container { width: 100%; overflow-x: auto; }
.data-table { width: 100%; border-collapse: collapse; min-width: 1000px; }
.data-table th { background: #f8fafc; padding: 16px; text-align: left; font-size: 13px; color: #64748b; border-bottom: 2px solid #edf2f7; }
.data-table td { padding: 16px; font-size: 14px; border-bottom: 1px solid #f1f5f9; color: #1e293b; }

/* Badge for Status */
.badge { padding: 6px 12px; border-radius: 6px; font-size: 12px; font-weight: 700; display: inline-block; text-align: center; min-width: 45px; }
.badge.on { background-color: #dcfce7; color: #166534; border: 1px solid #bbf7d0;}
.badge.off { background-color: #fee2e2; color: #991b1b; border: 1px solid #fecaca;}

/* Pagination */
.pagination-outer-container { display: flex; justify-content: space-between; align-items: center; padding: 20px; background: white; border-top: 1px solid #f1f5f9; flex-wrap: wrap; gap: 20px; }
.pagination-settings { display: flex; align-items: center; gap: 10px; font-size: 14px; color: #64748b; }
.rows-select { padding: 5px 10px; border: 1px solid #e2e8f0; border-radius: 6px; background-color: #ffffff; color: #1e293b; cursor: pointer; outline: none; font-family: inherit; font-size: 14px; min-width: 70px; }
.rows-select:hover { border-color: #cbd5e1; }
.rows-select:focus { border-color: #3b82f6; box-shadow: 0 0 0 2px rgba(59, 130, 246, 0.1); }
.pagination-container { display: flex; align-items: center; gap: 20px; }
.pagination-info { font-size: 14px; color: #1e293b; min-width: 120px; text-align: center; }
.pagination-status { font-size: 14px; color: #64748b; }
.pagination-status strong { color: #1e293b; }
.pagination-button { padding: 8px 16px; border-radius: 8px; border: none; background-color: #3b82f6; color: white; font-weight: 600; cursor: pointer; transition: opacity 0.2s; }
.pagination-button:disabled { background-color: #cbd5e1; cursor: not-allowed; }

@media (max-width: 768px) {
  .main-content { margin-left: 0; max-width: 100%; padding: 16px; }
  .filter-section { flex-direction: column; align-items: stretch; }
  .filter-group { max-width: 100%; }
  .action-buttons { width: 100%; flex-direction: column; }
  .btn-export, .btn-reset { width: 100%; justify-content: center; }
}
</style>