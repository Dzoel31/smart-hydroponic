<template>
  <div class="dashboard-layout">
    <Sidebar :logo="brandLogo" />
    <main class="main-content">
      
      <Topbar :title="`Hello, ${displayRole}!`" />

      <div class="dashboard-grid">
        <div class="metrics-row">
          <div class="metric-card" v-for="(metric, index) in metrics" :key="index">
            <div class="metric-header">
              <span>{{ metric.title }}</span>
              <span class="metric-icon">{{ metric.icon }}</span>
            </div>
            <div class="metric-value">{{ metric.value }} <span class="unit">{{ metric.unit }}</span></div>
          </div>
        </div>

        <div class="bento-layout">
          
          <div class="bento-card chart-card ph-chart">
            <div class="card-header">
              <h3>pH</h3>
              <span class="subtitle">Last 7 days</span>
            </div>
            <div class="chart-container">
              <Line :data="phChartData" :options="chartOptions" />
            </div>
          </div>

          <div class="bento-card chart-card tds-chart">
            <div class="card-header">
              <h3>TDS</h3>
              <span class="subtitle">Last 7 days</span>
            </div>
            <div class="chart-container">
              <Line :data="tdsChartData" :options="chartOptions" />
            </div>
          </div>

          <div class="bento-card control-panel">
            <h3>Kontrol Panel</h3>
            <div class="control-content">
              <div class="control-item">
                <div class="control-text">
                  <h4>Automatisasi</h4>
                  <p>Kontrol Automatisasi pompa dan lampu</p>
                </div>
                <label class="toggle-switch">
                  <input type="checkbox" v-model="controls.automation" @change="toggleControl('automation')">
                  <span class="slider"></span>
                </label>
              </div>

              <div class="control-item" :class="{ 'disabled': controls.automation }">
                <div class="control-text">
                  <h4>Pompa</h4>
                  <p>Kontrol Pompa</p>
                </div>
                <label class="toggle-switch">
                  <input type="checkbox" v-model="controls.pump" :disabled="controls.automation" @change="toggleControl('pump')">
                  <span class="slider"></span>
                </label>
              </div>

              <div class="control-item" :class="{ 'disabled': controls.automation }">
                <div class="control-text">
                  <h4>Lampu</h4>
                  <p>Kontrol Lampu</p>
                </div>
                <label class="toggle-switch">
                  <input type="checkbox" v-model="controls.light" :disabled="controls.automation" @change="toggleControl('light')">
                  <span class="slider"></span>
                </label>
              </div>
            </div>
          </div>
        </div>
      </div>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, computed, onMounted, onUnmounted } from "vue";
import { authState } from "../auth";
import Sidebar from '@/components/Sidebar.vue';
import Topbar from '@/components/Topbar.vue';
import brandLogo from '@/assets/images/logo-hydroponic.png';
import { HydroponicsService } from "../api";

import { Line } from 'vue-chartjs';
import {
  Chart as ChartJS,
  Title,
  Tooltip,
  Legend,
  LineElement,
  CategoryScale,
  LinearScale,
  PointElement,
  type ChartOptions
} from 'chart.js';

ChartJS.register(Title, Tooltip, Legend, LineElement, CategoryScale, LinearScale, PointElement);

const displayRole = computed(() => {
  if (!authState.isLoggedIn) return 'Guest';

  const role = authState.user?.role || 'User';

  return role.charAt(0).toUpperCase() + role.slice(1);
});

const chartOptions: ChartOptions<'line'>= {
  responsive: true,
  maintainAspectRatio: false,
  plugins: {
    legend: { display: false },
    tooltip: { enabled: true }
  },
  scales: {
    x: { display: true },
    y: { display: true }
  },
  animation: {
    duration: 800
  }
};

const phChartData = ref({
  labels: [] as string[],
  datasets: [{
    label: 'pH Level',
    data: [] as number[],
    borderColor: '#4caf50',
    backgroundColor: 'rgba(76, 175, 80, 0.2)',
    fill: true,
    tension: 0.4,
    pointRadius: 3,
  }]
});

const tdsChartData = ref({
  labels: [] as string[],
  datasets: [{
    label: 'TDS (ppm)',
    data: [] as number[],
    borderColor: '#3b82f6',
    backgroundColor: 'rgba(59, 130, 246, 0.2)',
    fill: true,
    tension: 0.4,
    pointRadius: 3
  }]
});

const loadAllHydroponicRows = async () => {
  const rows: Array<{ timestamp: string; ph?: number | null; tds?: number | null }> = [];
  const limit = 100;
  let currentPage = 1;
  let totalPages = 1;

  do {
    const response = await HydroponicsService.getHydroponicData(currentPage, limit);
    rows.push(...response.data);
    totalPages = response.meta.total_pages || 1;
    currentPage += 1;
  } while (currentPage <= totalPages);

  return rows;
};

const loadWeeklyChartData = async () => {
  try {
    const rawData = await loadAllHydroponicRows();
    if (!rawData || rawData.length === 0) {
      console.warn("Data dari API kosong");
      return;
    }

    // 1. Kelompokkan Data per Hari TANPA memfilter tanggal dulu
    const dailyData = rawData.reduce((acc: any, item: any) => {
      if (!item.timestamp) return acc;
      
      const dateObj = new Date(item.timestamp);
      if (isNaN(dateObj.getTime())) return acc; // Abaikan jika format tanggal rusak

      // Buat kunci unik berdasarkan tanggal (misal: "1 Jan 2026")
      const dateKey = dateObj.toDateString(); 

      if (!acc[dateKey]) {
        acc[dateKey] = { dateObj, ph: [], tds: [] };
      }
      if (typeof item.ph === 'number') acc[dateKey].ph.push(item.ph);
      if (typeof item.tds === 'number') acc[dateKey].tds.push(item.tds);
      
      return acc;
    }, {});

    // 2. Ubah objek ke array, urutkan dari terlama ke terbaru, lalu ambil 7 hari terakhir
    const sortedDays = Object.values(dailyData)
      .sort((a: any, b: any) => a.dateObj.getTime() - b.dateObj.getTime())
      .slice(-7); // Ambil 7 data terujung

    const labels: string[] = [];
    const phValues: number[] = [];
    const tdsValues: number[] = [];

    // 3. Hitung rata-rata dan masukkan ke array untuk Chart.js
    sortedDays.forEach((dayData: any) => {
      // Format label untuk sumbu X (misal: "1 Jan")
      labels.push(dayData.dateObj.toLocaleDateString('id-ID', { day: 'numeric', month: 'short' }));
      
      const avgPh = dayData.ph.reduce((a: number, b: number) => a + b, 0) / (dayData.ph.length || 1);
      const avgTds = dayData.tds.reduce((a: number, b: number) => a + b, 0) / (dayData.tds.length || 1);
      
      phValues.push(avgPh);
      tdsValues.push(avgTds);
    });

    // 4. Update state agar vue-chartjs merender ulang
    phChartData.value.labels = labels;
    phChartData.value.datasets[0].data = phValues;
    phChartData.value = { ...phChartData.value }; 

    tdsChartData.value.labels = labels;
    tdsChartData.value.datasets[0].data = tdsValues;
    tdsChartData.value = { ...tdsChartData.value }; 

  } catch (error) {
    console.error("Gagal mengambil riwayat grafik dari endpoint /hydroponics/data:", error);
  }
};

const controls = reactive({
  automation: true,
  pump: true,
  light: true
});

const toggleControl = (type: string) => {
  console.log(`Toggling ${type} to: ${controls[type as keyof typeof controls]}`);
  
  if (type === 'automation' && controls.automation) {
    console.log("Automatisasi Aktif. Mengambil alih kontrol...");
  }
};

// Data Mock untuk Metric Cards
const metrics = ref([
  { title: 'Water Flow', value: '0', unit: 'l/m', icon: '💦' },
  { title: 'Water Level', value: '10.87', unit: 'cm', icon: '🎚️' },
  { title: 'Total Liters', value: '0', unit: 'l', icon: '🪣' },
  { title: 'Avg Moisture', value: '54.60', unit: '%', icon: '🍄‍🟫'},
  { title: 'Avg Temperature', value: '25.9', unit: '°C', icon: '🌡️' },
  { title: 'Avg Humidity', value: '87.9', unit: '%', icon: '☁️' },
  { title: 'TDS', value: '1837.993', unit: 'ppm', icon: '🔋'},
  { title: 'pH', value: '6.54', unit: '', icon: '🧪' }
]);

const formatMetric = (value: number | null | undefined, digits: number): string => {
  return typeof value === 'number' ? value.toFixed(digits) : '0';
};

let metricsInterval: ReturnType<typeof setInterval> | null = null;

const refreshLatestMetrics = async () => {
  try {
    const data = await HydroponicsService.getLatestHydroponicData();
    metrics.value[0].value = formatMetric(data?.flowrate, 2);
    metrics.value[1].value = formatMetric(data?.distance_cm, 2);
    metrics.value[2].value = formatMetric(data?.total_litres, 2);
    metrics.value[3].value = formatMetric(data?.moisture_avg, 2);
    metrics.value[4].value = formatMetric(data?.temperature_avg, 1);
    metrics.value[5].value = formatMetric(data?.humidity_avg, 1);
    metrics.value[6].value = formatMetric(data?.tds, 3);
    metrics.value[7].value = formatMetric(data?.ph, 2);
  } catch (error) {
    console.error("Error fetching latest data:", error);
  }
};

onMounted(() => {
  loadWeeklyChartData();
  refreshLatestMetrics();
  metricsInterval = setInterval(refreshLatestMetrics, 5000);
});

onUnmounted(() => {
  if (metricsInterval) {
    clearInterval(metricsInterval);
  }
});
</script>

<style scoped>
* { box-sizing: border-box; }

/* KUNCI: Flexbox Layouting */
.dashboard-layout {
  display: flex;
  width: 100%;
  min-height: 100vh;
  background-color: #f8fafc;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  color: #1e293b;
}

.main-content {
  flex: 1; /* Otomatis mengisi seluruh ruang sisa di kanan Sidebar */
  min-width: 0; /* Mencegah overflow horizontal */
  padding: 24px 32px;
  transition: padding 0.3s ease;
}

/* ... (Kode .metrics-row, .bento-layout, .status-panel, dll TETAP SAMA) ... */
/* METRICS ROW */
.metrics-row { display: grid; grid-template-columns: repeat(4, 1fr); gap: 20px; margin-bottom: 24px; }
.metric-card { background-color: #ffffff; padding: 20px; border-radius: 12px; border: 1px solid #e2e8f0; box-shadow: 0 1px 3px rgba(0,0,0,0.02); }
.metric-header { display: flex; justify-content: space-between; align-items: center; color: #64748b; font-size: 14px; font-weight: 600; margin-bottom: 12px; }
.metric-value { font-size: 28px; font-weight: 700; color: #0f172a; margin-bottom: 8px; }
.metric-value .unit { font-size: 16px; color: #64748b; }
.text-green { color: #16a34a; }

/* BENTO GRID LAYOUT */
.bento-layout { display: grid; grid-template-columns: 1fr 1fr; grid-auto-rows: minmax(150px, auto); gap: 20px; }
.bento-card { background-color: #ffffff; padding: 24px; border-radius: 12px; border: 1px solid #e2e8f0; box-shadow: 0 1px 3px rgba(0,0,0,0.02); }
.card-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }
.card-header h3 { margin: 0; font-size: 16px; }
.subtitle { font-size: 13px; color: #64748b; }

/* CHARTS */
.chart-card { display: flex; flex-direction: column; }
.ph-chart { grid-column: 1 / 2; }
.tds-chart { grid-column: 2 / 3; }
.chart-container { flex: 1; width: 100%; display: flex; align-items: flex-end; }
.mock-line-chart { width: 100%; height: 80px; overflow: visible; }
.chart-labels { display: flex; justify-content: space-between; margin-top: 10px; font-size: 11px; color: #94a3b8; }

/* CONTROL PANEL */
.control-panel {
  grid-column: 1 / 3; /* Lebar penuh */
}
.control-panel h3 {
  margin: 0 0 20px 0;
  font-size: 18px;
  color: #1e293b;
}

.control-content {
  display: flex;
  flex-direction: column;
}

.control-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px 0;
  border-bottom: 1px solid #e2e8f0;
}

.control-item:last-child {
  border-bottom: none;
  padding-bottom: 0;
}

.control-text h4 {
  margin: 0 0 4px 0;
  font-size: 15px;
  color: #0f172a;
}

.control-text p {
  margin: 0;
  font-size: 13px;
  color: #64748b;
}

.control-item.disabled {
  opacity: 0.5;
  pointer-events: none;
}

/* TOGGLE SWITCH STYLE */
.toggle-switch {
  position: relative;
  display: inline-block;
  width: 50px;
  height: 26px;
}

.toggle-switch input { 
  opacity: 0;
  width: 0;
  height: 0;
}

.slider {
  position: absolute;
  cursor: pointer;
  top: 0; left: 0; right: 0; bottom: 0;
  background-color: #cbd5e1;
  transition: 0.3s ease-in-out;
  border-radius: 26px;
}

.slider:before {
  position: absolute;
  content: "";
  height: 20px;
  width: 20px;
  left: 3px;
  bottom: 3px;
  background-color: white;
  border-radius: 50%;
  box-shadow: 0 2px 5px rgba(0,0,0,0.3);
  transition: 0.3s ease-in-out;
}

input:checked + .slider {
  background-color: #22c55e;
}

input:checked + .slider:before {
  transform: translateX(24px);
}

input:focus + .slider {
  box-shadow: 0 0 0 2px rgba(34, 197, 94, 0.3);
}

/* RESPONSIVE (Sangat Bersih) */
@media (max-width: 1024px) {
  .metrics-row { grid-template-columns: repeat(2, 1fr); }
  .bento-layout { grid-template-columns: 1fr 1fr; }
}

@media (max-width: 768px) {
  .main-content { padding: 16px; }
  .bento-layout { grid-template-columns: 1fr; }
  .status-content { flex-direction: column; align-items: flex-start; gap: 20px; }
  .ph-chart, .tds-chart, .status-panel { grid-column: 1 / -1; }
}

@media (max-width: 480px) {
  .metrics-row { grid-template-columns: 1fr; }
}
</style>