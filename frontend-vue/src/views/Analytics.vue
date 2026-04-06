<template>
  <div class="layout-wrapper">
    <Sidebar :logo="brandLogo" />

    <main class="main-content">
      
      <Topbar title="Hydroponic Analytics" />

      <div class="filters-section">
        <div class="filter-group">
          <label>Period</label>
          <div class="custom-select">
            <select v-model="selectedPeriod">
              <option value="3months">Last 24 hours</option>
              <option value="1month">Last 3 Days</option>
              <option value="7days">Last 7 Days</option>
              <option value="1month">Last 1 Month</option>
              <option value="3months">Last 3 Months</option>
              <option value="7days">Last 6 Months</option>
            </select>
          </div>
        </div>
      </div>

      <div v-if="isLoading" class="loading-overlay">
        <p>Processing Data...</p>
      </div>

      <template v-else>
        <div class="main-chart-card">
          <div class="card-header">
            <h3>Moisture Trend</h3>
            <div class="legend">
              <span class="indicator moisture-indicator"></span> Moisture (%)
              <span class="indicator moisture-indicator"></span> Moisture (%)
              <span class="indicator moisture-indicator"></span> Moisture (%)
            </div>
          </div>
        
          <div class="chart-wrapper main-chart-wrapper">
            <svg viewBox="0 0 800 200" class="svg-chart" preserveAspectRatio="none">
              <defs>
                <linearGradient id="moistureGrad" x1="0" y1="0" x2="0" y2="1">
                  <stop offset="0%" stop-color="rgba(59, 130, 246, 0.4)" /> 
                  <stop offset="100%" stop-color="rgba(59, 130, 246, 0)" />
                </linearGradient>
              </defs>

              <line v-for="i in 4" :key="i" x1="0" :y1="i*50" x2="800" :y2="i*50" stroke="#f1f5f9" stroke-width="1" />

              <path :d="getAreaPath('moisture', 800, 200, 100)" fill="url(#moistureGrad)" />
              <path :d="getLinePath('moisture', 800, 200, 100)" fill="none" stroke="#3b82f6" stroke-width="3" />
              
              <circle v-for="(d, i) in chartData" :key="i" 
                :cx="getX(i, chartData.length, 800)" 
                :cy="getY(d.moisture, 100, 200)" 
                r="4" class="point moisture-point" 
              />
            </svg>

            <div class="x-axis">
              <span v-for="d in chartData" :key="d.date">{{ formatDate(d.date) }}</span>
            </div>
          </div>
        </div>

        <div class="sub-charts-grid">
          <div class="sub-chart-card">
            <div class="card-header">
              <h3>Temperature Trend</h3>
              <span class="badge-text">°C</span>
            </div>
            <div class="chart-wrapper sub-chart-wrapper">
              <svg viewBox="0 0 400 100" class="svg-chart" preserveAspectRatio="none">
                <defs>
                  <linearGradient id="tempGrad" x1="0" y1="0" x2="0" y2="1">
                    <stop offset="0%" stop-color="rgba(245, 158, 11, 0.4)" /> 
                    <stop offset="100%" stop-color="rgba(245, 158, 11, 0)" />
                  </linearGradient>
                </defs>
                <path :d="getAreaPath('temp', 400, 100, 50)" fill="url(#tempGrad)" />
                <path :d="getLinePath('temp', 400, 100, 50)" fill="none" stroke="#f59e0b" stroke-width="2.5" />
              </svg>
            </div>
          </div>

          <div class="sub-chart-card">
            <div class="card-header">
              <h3>Humidity Trend</h3>
              <span class="badge-text">%</span>
            </div>
            <div class="chart-wrapper sub-chart-wrapper">
              <svg viewBox="0 0 400 100" class="svg-chart" preserveAspectRatio="none">
                <defs>
                  <linearGradient id="humGrad" x1="0" y1="0" x2="0" y2="1">
                    <stop offset="0%" stop-color="rgba(14, 165, 233, 0.4)" /> 
                    <stop offset="100%" stop-color="rgba(14, 165, 233, 0)" />
                  </linearGradient>
                </defs>
                <path :d="getAreaPath('humidity', 400, 100, 100)" fill="url(#humGrad)" />
                <path :d="getLinePath('humidity', 400, 100, 100)" fill="none" stroke="#0ea5e9" stroke-width="2.5" />
              </svg>
            </div>
          </div>

          <div class="sub-chart-card">
            <div class="card-header">
              <h3>pH</h3>
              <span class="badge-text"></span>
            </div>
            <div class="chart-wrapper sub-chart-wrapper">
              <svg viewBox="0 0 400 100" class="svg-chart" preserveAspectRatio="none">
                <defs>
                  <linearGradient id="pHGrad" x1="0" y1="0" x2="0" y2="1">
                    <stop offset="0%" stop-color="rgba(14, 165, 233, 0.4)" /> 
                    <stop offset="100%" stop-color="rgba(14, 165, 233, 0)" />
                  </linearGradient>
                </defs>
                <path :d="getAreaPath('pH', 400, 100, 100)" fill="url(#pHGrad)" />
                <path :d="getLinePath('pH', 400, 100, 100)" fill="none" stroke="#0ea5e9" stroke-width="2.5" />
              </svg>
            </div>
          </div>

          <div class="sub-chart-card">
            <div class="card-header">
              <h3>TDS</h3>
              <span class="badge-text">ppm</span>
            </div>
            <div class="chart-wrapper sub-chart-wrapper">
              <svg viewBox="0 0 400 100" class="svg-chart" preserveAspectRatio="none">
                <defs>
                  <linearGradient id="tdsGrad" x1="0" y1="0" x2="0" y2="1">
                    <stop offset="0%" stop-color="rgba(14, 165, 233, 0.4)" /> 
                    <stop offset="100%" stop-color="rgba(14, 165, 233, 0)" />
                  </linearGradient>
                </defs>
                <path :d="getAreaPath('tds', 400, 100, 100)" fill="url(#tdsGrad)" />
                <path :d="getLinePath('tds', 400, 100, 100)" fill="none" stroke="#0ea5e9" stroke-width="2.5" />
              </svg>
            </div>
          </div>
        </div>
      </template>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, computed } from 'vue';
import Sidebar from '@/components/Sidebar.vue';
import Topbar from '@/components/Topbar.vue';
import brandLogo from '@/assets/images/logo-hydroponic.png';
import Papa from 'papaparse';

const selectedPeriod = ref<string>('7days');
const rawData = ref<any[]>([]);
const isLoading = ref(true);

// 1. Fetch data dari CSV
const fetchData = () => {
  isLoading.value = true;
  Papa.parse('/sample_hydroponic_data.csv', {
    download: true,
    header: true,
    dynamicTyping: true,
    skipEmptyLines: true,
    complete: (results) => {
      rawData.value = results.data.filter((item: any) => item.timestamp);
      isLoading.value = false;
    },
    error: (err) => {
      console.error("Gagal load CSV:", err);
      isLoading.value = false;
    }
  });
};

onMounted(() => fetchData());

// 2. Agregasi Data Harian Berdasarkan Filter Periode
const chartData = computed(() => {
  if (rawData.value.length === 0) return [];

  const now = new Date();
  let daysToSubtract = 7;
  if (selectedPeriod.value === '1month') daysToSubtract = 30;
  if (selectedPeriod.value === '3months') daysToSubtract = 90;
  
  const limitDate = new Date();
  limitDate.setDate(now.getDate() - daysToSubtract);

  const filtered = rawData.value.filter(item => {
    const d = new Date(item.timestamp);
    return !isNaN(d.getTime()) && d >= limitDate;
  });

  const grouped = filtered.reduce((acc: any, item: any) => {
    const d = new Date(item.timestamp);
    const dateKey = d.toISOString().split('T')[0]; 

    if (!acc[dateKey]) {
      acc[dateKey] = { m: [], t: [], h: [] };
    }
    
    const avgM = ((item.moisture1 || 0) + (item.moisture2 || 0) + (item.moisture3 || 0) + 
                  (item.moisture4 || 0) + (item.moisture5 || 0) + (item.moisture6 || 0)) / 6;
    
    const avgT = ((item.temperature_atas || 0) + (item.temperature_bawah || 0)) / 2;
    const avgH = ((item.humidity_atas || 0) + (item.humidity_bawah || 0)) / 2;

    acc[dateKey].m.push(avgM);
    acc[dateKey].t.push(avgT);
    acc[dateKey].h.push(avgH);
    return acc;
  }, {});

  return Object.keys(grouped).map(date => ({
    date,
    moisture: grouped[date].m.reduce((a:any, b:any) => a+b, 0) / grouped[date].m.length,
    temp: grouped[date].t.reduce((a:any, b:any) => a+b, 0) / grouped[date].t.length,
    humidity: grouped[date].h.reduce((a:any, b:any) => a+b, 0) / grouped[date].h.length,
  })).sort((a, b) => new Date(a.date).getTime() - new Date(b.date).getTime());
});

// 3. Helper Functions untuk Koordinat SVG
const getX = (index: number, total: number, width: number) => {
  if (total <= 1) return 0;
  return (index / (total - 1)) * width;
};

const getY = (value: number, max: number, height: number) => {
  return height - (value / max) * height;
};

const getLinePath = (key: string, width: number, height: number, max: number) => {
  if (chartData.value.length < 2) return "";
  const points = chartData.value.map((d: any, i: number) => {
    return `${getX(i, chartData.value.length, width)},${getY(d[key], max, height)}`;
  });
  return `M${points.join(' L')}`;
};

const getAreaPath = (key: string, width: number, height: number, max: number) => {
  const line = getLinePath(key, width, height, max);
  if (!line) return "";
  return `${line} L${width},${height} L0,${height} Z`;
};

const formatDate = (dateStr: string) => {
  const d = new Date(dateStr);
  return d.toLocaleDateString('en-US', { month: 'short', day: 'numeric' });
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
}

.main-content { 
  flex: 1; 
  min-width: 0;
  padding: 24px 32px; 
  display: flex; 
  flex-direction: column; 
  gap: 24px;
}

/* ... (Kode Filter, Chart, Legend TETAP SAMA) ... */
.filters-section { display: flex; align-items: flex-end; }
.filter-group { display: flex; flex-direction: column; gap: 8px; }
.filter-group label { font-size: 13px; font-weight: 600; color: #64748b; }
.custom-select select { padding: 10px 40px 10px 16px; border: 1px solid #e2e8f0; border-radius: 8px; background-color: #fff; font-size: 14px; color: #0f172a; cursor: pointer; appearance: none; font-weight: 500;}

.main-chart-card, .sub-chart-card { background: #fff; border: 1px solid #e2e8f0; border-radius: 12px; padding: 24px; box-shadow: 0 1px 3px rgba(0,0,0,0.02);}
.card-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 24px; }
.card-header h3 { margin: 0; font-size: 16px; color: #0f172a; }

.chart-wrapper { position: relative; width: 100%; }
.main-chart-wrapper { height: 250px; }
.sub-chart-wrapper { height: 160px; }
.svg-chart { width: 100%; height: 100%; display: block; overflow: visible; }
.x-axis { display: flex; justify-content: space-between; margin-top: 15px; font-size: 11px; color: #94a3b8; }
.sub-charts-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 24px; }

.legend { font-size: 13px; color: #64748b; display: flex; align-items: center; gap: 6px; }
.indicator { width: 12px; height: 12px; border-radius: 2px; }
.moisture-indicator { background-color: #3b82f6; }
.point { fill: #fff; stroke-width: 2px; stroke: #3b82f6; }
.badge-text { font-size: 13px; color: #64748b; font-weight: 500; }
.loading-overlay { padding: 50px; text-align: center; color: #64748b; font-weight: 600; }

/* RESPONSIVE */
@media (max-width: 768px) {
  .main-content { padding: 16px; }
  .sub-charts-grid { grid-template-columns: 1fr; }
}
</style>