<template>
  <div class="layout-wrapper">
    <Sidebar :logo="brandLogo" />

    <main class="main-content">
      <Topbar title="Hydroponic Analytics" />

      <section class="toolbar-card">
        <div class="toolbar-meta">
          <h2>Hydroponic Trend Analytics</h2>
          <p>
            Last update:
            <span class="last-updated">{{ lastUpdatedLabel }}</span>
          </p>
        </div>

        <div class="toolbar-controls">
          <label class="period-label" for="period-select">Range</label>
          <select id="period-select" v-model="selectedPeriod" class="period-select" :disabled="isLoading">
            <option v-for="option in periodOptions" :key="option.value" :value="option.value">
              {{ option.label }}
            </option>
          </select>
        </div>

        <button class="refresh-btn" :disabled="isLoading" @click="refreshData">
          {{ isLoading ? 'Refreshing...' : 'Refresh' }}
        </button>

        <button class="reset-btn" @click="resetRange" :disabled="isLoading">
          {{ isLoading ? 'Resetting...' : 'Reset Range' }}
        </button>
      </section>

      <div v-if="errorMessage" class="error-banner">
        {{ errorMessage }}
      </div>

      <section class="main-chart-card">
        <div class="card-header">
          <h3>Moisture Trends</h3>
          <span class="badge-text">%</span>
        </div>
        <div class="chart-container chart-main">
          <Line v-if="hasTimelineData" :data="moistureChartData" :options="moistureOptions" />
          <p v-else class="empty-state">No hydroponic data in the selected range.</p>
        </div>
      </section>

      <section class="sub-charts-grid">
        <article class="sub-chart-card">
          <div class="card-header">
            <h3>Temperature Trends</h3>
            <span class="badge-text">°C</span>
          </div>
          <div class="chart-container">
            <Line v-if="hasTimelineData" :data="temperatureChartData" :options="timelineOptions" />
            <p v-else class="empty-state">No data</p>
          </div>
        </article>

        <article class="sub-chart-card">
          <div class="card-header">
            <h3>Humidity Trends</h3>
            <span class="badge-text">%</span>
          </div>
          <div class="chart-container">
            <Line v-if="hasTimelineData" :data="humidityChartData" :options="timelineOptions" />
            <p v-else class="empty-state">No data</p>
          </div>
        </article>

        <article class="sub-chart-card">
          <div class="card-header">
            <h3>pH Trends</h3>
          </div>
          <div class="chart-container">
            <Line v-if="hasTimelineData" :data="phChartData" :options="timelineOptions" />
            <p v-else class="empty-state">No data</p>
          </div>
        </article>

        <article class="sub-chart-card">
          <div class="card-header">
            <h3>TDS Trends</h3>
          </div>
          <div class="chart-container">
            <Line v-if="hasTimelineData" :data="tdsChartData" :options="timelineOptions" />
            <p v-else class="empty-state">No data</p>
          </div>
        </article>
      </section>
    </main>
  </div>
</template>

<script setup lang="ts">
import { computed, onMounted, onUnmounted, ref, watch } from 'vue';
import { Line } from 'vue-chartjs';
import {
  CategoryScale,
  Chart as ChartJS,
  Legend,
  LineElement,
  LinearScale,
  PointElement,
  Tooltip,
  type ChartData,
  type ChartOptions,
} from 'chart.js';
import Sidebar from '@/components/Sidebar.vue';
import Topbar from '@/components/Topbar.vue';
import brandLogo from '@/assets/images/logo-hydroponic.png';
import { HydroponicsService, type HydroponicOut, type ResponseList_HydroponicOut_ } from '../api';
import { getApiErrorMessage } from '../utils/apiError';

ChartJS.register(CategoryScale, LinearScale, PointElement, LineElement, Tooltip, Legend);

type PeriodKey = '24h' | '3d' | '7d' | '1m' | '3m' | '6m';

type TimelinePoint = {
  timestamp: string;
  moisture1: number | null;
  moisture2: number | null;
  moisture3: number | null;
  moisture4: number | null;
  moisture5: number | null;
  moisture6: number | null;
  moistureAvg: number | null;
  temperatureTop: number | null;
  temperatureBottom: number | null;
  temperatureAvg: number | null;
  humidityTop: number | null;
  humidityBottom: number | null;
  humidityAvg: number | null;
  ph: number | null;
  tds: number | null;
};

const periodOptions: Array<{ value: PeriodKey; label: string }> = [
  { value: '24h', label: 'Last 24 Hours' },
  { value: '3d', label: 'Last 3 Days' },
  { value: '7d', label: 'Last 7 Days' },
  { value: '1m', label: 'Last 1 Month' },
  { value: '3m', label: 'Last 3 Months' },
  { value: '6m', label: 'Last 6 Months' },
];

const selectedPeriod = ref<PeriodKey>('24h');
const timelineSeries = ref<Array<HydroponicOut>>([]);
const isLoading = ref(false);
const errorMessage = ref('');
const lastUpdatedAt = ref<Date | null>(null);
let refreshTimerId: number | null = null;

const toSeriesNumber = (value: number | null | undefined): number | null => {
  return typeof value === 'number' ? Number(value.toFixed(2)) : null;
};

const periodConfig = (period: PeriodKey): { days: number; limit: number } => {
  switch (period) {
    case '24h':
      return { days: 1, limit: 300 };
    case '3d':
      return { days: 3, limit: 500 };
    case '7d':
      return { days: 7, limit: 700 };
    case '1m':
      return { days: 30, limit: 1000 };
    case '3m':
      return { days: 90, limit: 1500 };
    case '6m':
      return { days: 180, limit: 2000 };
    default:
      return { days: 1, limit: 300 };
  }
};

const getPeriodRange = (period: PeriodKey): { startIso: string; endIso: string } => {
  const now = new Date();
  const { days } = periodConfig(period);
  const start = new Date(now);
  start.setDate(start.getDate() - days);

  return {
    startIso: start.toISOString(),
    endIso: now.toISOString(),
  };
};

const downsampleSeries = (rows: Array<HydroponicOut>, maxPoints = 180): Array<HydroponicOut> => {
  if (rows.length <= maxPoints) {
    return rows;
  }

  const step = Math.ceil(rows.length / maxPoints);
  const sampled = rows.filter((_, index) => index % step === 0);
  const last = rows[rows.length - 1];
  if (last && sampled[sampled.length - 1] !== last) {
    sampled.push(last);
  }
  return sampled;
};

const parseAndSortRows = (rows: Array<HydroponicOut>): Array<HydroponicOut> => {
  return rows
    .filter((row) => Boolean(row.timestamp))
    .sort((a, b) => new Date(a.timestamp).getTime() - new Date(b.timestamp).getTime());
};

const hasTimelineData = computed(() => timelineSeries.value.length > 0);

const lastUpdatedLabel = computed(() => {
  if (!lastUpdatedAt.value) {
    return 'Never';
  }

  return lastUpdatedAt.value.toLocaleString();
});

const timelinePoints = computed<Array<TimelinePoint>>(() => {
  return timelineSeries.value.map((row) => {
    const moistureValues = [
      toSeriesNumber(row.moisture1),
      toSeriesNumber(row.moisture2),
      toSeriesNumber(row.moisture3),
      toSeriesNumber(row.moisture4),
      toSeriesNumber(row.moisture5),
      toSeriesNumber(row.moisture6),
    ].filter((value): value is number => value !== null);

    const tempTop = toSeriesNumber(row.temperature_atas);
    const tempBottom = toSeriesNumber(row.temperature_bawah);
    const humTop = toSeriesNumber(row.humidity_atas);
    const humBottom = toSeriesNumber(row.humidity_bawah);
    const phValue = toSeriesNumber(row.ph);
    const tdsValue = toSeriesNumber(row.tds);

    return {
      timestamp: row.timestamp,
      moisture1: toSeriesNumber(row.moisture1),
      moisture2: toSeriesNumber(row.moisture2),
      moisture3: toSeriesNumber(row.moisture3),
      moisture4: toSeriesNumber(row.moisture4),
      moisture5: toSeriesNumber(row.moisture5),
      moisture6: toSeriesNumber(row.moisture6),
      moistureAvg:
        toSeriesNumber(row.moisture_avg) ??
        (moistureValues.length ? Number((moistureValues.reduce((sum, value) => sum + value, 0) / moistureValues.length).toFixed(2)) : null),
      temperatureTop: tempTop,
      temperatureBottom: tempBottom,
      temperatureAvg:
        toSeriesNumber(row.temperature_avg) ??
        (tempTop !== null && tempBottom !== null ? Number(((tempTop + tempBottom) / 2).toFixed(2)) : null),
      humidityTop: humTop,
      humidityBottom: humBottom,
      humidityAvg:
        toSeriesNumber(row.humidity_avg) ??
        (humTop !== null && humBottom !== null ? Number(((humTop + humBottom) / 2).toFixed(2)) : null),
      ph: phValue,
      tds: tdsValue,
    };
  });
});

const labels = computed<Array<string>>(() => {
  return timelinePoints.value.map((point) => {
    const date = new Date(point.timestamp);
    if (selectedPeriod.value === '24h' || selectedPeriod.value === '3d') {
      return date.toLocaleString([], { month: 'short', day: 'numeric', hour: '2-digit', minute: '2-digit' });
    }
    return date.toLocaleDateString([], { month: 'short', day: 'numeric' });
  });
});

const buildLineDataset = (
  label: string,
  data: Array<number | null>,
  color: string,
  dash: Array<number> = []
) => ({
  label,
  data,
  borderColor: color,
  backgroundColor: `${color}20`,
  borderWidth: 2,
  pointRadius: 0,
  pointHoverRadius: 4,
  tension: 0.25,
  spanGaps: true,
  borderDash: dash,
});

const baseTimelineOptions: ChartOptions<'line'> = {
  responsive: true,
  maintainAspectRatio: false,
  plugins: {
    legend: {
      display: true,
      position: 'top',
      labels: {
        boxWidth: 12,
      },
    },
  },
  scales: {
    x: {
      ticks: {
        maxTicksLimit: 8,
        autoSkip: true,
      },
      grid: {
        display: false,
      },
    },
    y: {
      beginAtZero: true,
    },
  },
};

const moistureOptions = computed<ChartOptions<'line'>>(() => ({
  ...baseTimelineOptions,
  scales: {
    ...baseTimelineOptions.scales,
    y: {
      beginAtZero: true,
      suggestedMax: 100,
      ticks: {
        callback: (value) => `${value}%`,
      },
    },
  },
}));

const timelineOptions = computed<ChartOptions<'line'>>(() => ({
  ...baseTimelineOptions,
}));

const moistureChartData = computed<ChartData<'line'>>(() => ({
  labels: labels.value,
  datasets: [
    buildLineDataset('Moisture 1', timelinePoints.value.map((point) => point.moisture1), '#2563eb'),
    buildLineDataset('Moisture 2', timelinePoints.value.map((point) => point.moisture2), '#3b82f6'),
    buildLineDataset('Moisture 3', timelinePoints.value.map((point) => point.moisture3), '#60a5fa'),
    buildLineDataset('Moisture 4', timelinePoints.value.map((point) => point.moisture4), '#0284c7'),
    buildLineDataset('Moisture 5', timelinePoints.value.map((point) => point.moisture5), '#0ea5e9'),
    buildLineDataset('Moisture 6', timelinePoints.value.map((point) => point.moisture6), '#38bdf8'),
    buildLineDataset('Moisture Avg', timelinePoints.value.map((point) => point.moistureAvg), '#1e3a8a', [6, 4]),
  ],
}));

const temperatureChartData = computed<ChartData<'line'>>(() => ({
  labels: labels.value,
  datasets: [
    buildLineDataset('Temperature Top', timelinePoints.value.map((point) => point.temperatureTop), '#f59e0b'),
    buildLineDataset('Temperature Bottom', timelinePoints.value.map((point) => point.temperatureBottom), '#f97316'),
    buildLineDataset('Temperature Avg', timelinePoints.value.map((point) => point.temperatureAvg), '#c2410c', [6, 4]),
  ],
}));

const humidityChartData = computed<ChartData<'line'>>(() => ({
  labels: labels.value,
  datasets: [
    buildLineDataset('Humidity Top', timelinePoints.value.map((point) => point.humidityTop), '#0ea5e9'),
    buildLineDataset('Humidity Bottom', timelinePoints.value.map((point) => point.humidityBottom), '#0284c7'),
    buildLineDataset('Humidity Avg', timelinePoints.value.map((point) => point.humidityAvg), '#155e75', [6, 4]),
  ],
}));

const phChartData = computed<ChartData<'line'>>(() => ({
  labels: labels.value,
  datasets: [
    buildLineDataset('pH', timelinePoints.value.map((point) => point.ph), '#16a34a'),
  ],
}));

const tdsChartData = computed<ChartData<'line'>>(() => ({
  labels: labels.value,
  datasets: [
    buildLineDataset('TDS', timelinePoints.value.map((point) => point.tds), '#059669'),
  ],
}));

const fetchHydroponicRows = async (period: PeriodKey): Promise<Array<HydroponicOut>> => {
  const { startIso, endIso } = getPeriodRange(period);
  const { limit } = periodConfig(period);
  const rows: Array<HydroponicOut> = [];

  let page = 1;
  let totalPages = 1;

  while (page <= totalPages) {
    const response: ResponseList_HydroponicOut_ = await HydroponicsService.getHydroponicData(
      page,
      limit,
      startIso,
      endIso,
    );

    rows.push(...response.data);
    totalPages = Math.max(response.meta.total_pages ?? 1, 1);
    page += 1;

    if (page > 15) {
      break;
    }
  }

  return rows;
};

const resetRange = (): void => {
  selectedPeriod.value = '24h';
  void refreshData();
};

const refreshData = async (): Promise<void> => {
  isLoading.value = true;
  errorMessage.value = '';

  try {
    const rows = await fetchHydroponicRows(selectedPeriod.value);
    const sortedRows = parseAndSortRows(rows);
    timelineSeries.value = downsampleSeries(sortedRows);
    lastUpdatedAt.value = new Date();
  } catch (error) {
    const message = getApiErrorMessage(error, 'Failed to fetch timeline data. Please try again.');
    console.error('Failed to refresh hydroponic analytics:', message);
    errorMessage.value = message;
    timelineSeries.value = [];
  } finally {
    isLoading.value = false;
  }
};

watch(selectedPeriod, () => {
  void refreshData();
});

onMounted(async () => {
  await refreshData();
  refreshTimerId = window.setInterval(refreshData, 5 * 60 * 1000);
});

onUnmounted(() => {
  if (refreshTimerId !== null) {
    window.clearInterval(refreshTimerId);
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

.toolbar-card,
.main-chart-card,
.sub-chart-card {
  background: #fff;
  border: 1px solid #e2e8f0;
  border-radius: 12px;
  padding: 20px 24px;
  box-shadow: 0 1px 3px rgba(0, 0, 0, 0.02);
}

.toolbar-card {
  display: flex;
  justify-content: flex-start;
  align-items: center;
  gap: 16px;
  flex-wrap: wrap;
}

.toolbar-meta h2 {
  margin: 0 0 8px;
  font-size: 18px;
  color: #0f172a;
}

.toolbar-meta p {
  margin: 0;
  color: #64748b;
  font-size: 14px;
}

.last-updated {
  font-weight: 600;
}

.toolbar-controls {
  margin-left: auto;
  display: flex;
  align-items: center;
  gap: 8px;
}

.period-label {
  color: #64748b;
  font-size: 13px;
  font-weight: 600;
}

.period-select {
  border: 1px solid #cbd5e1;
  border-radius: 10px;
  padding: 9px 12px;
  background: #fff;
  color: #0f172a;
  font-weight: 600;
}

.refresh-btn {
  border: none;
  border-radius: 10px;
  padding: 10px 16px;
  background: #2563eb;
  color: #fff;
  font-weight: 600;
  cursor: pointer;
}

.refresh-btn:disabled {
  opacity: 0.7;
  cursor: wait;
}

.reset-btn {
  border: none;
  border-radius: 10px;
  padding: 10px 16px;
  background: #dc2626;
  color: #fff;
  font-weight: 600;
  cursor: pointer;
}

.reset-btn:disabled {
  opacity: 0.7;
  cursor: wait;
}

.error-banner {
  background: #fef2f2;
  border: 1px solid #fecaca;
  color: #b91c1c;
  border-radius: 12px;
  padding: 12px 14px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 16px;
}

.card-header h3 {
  margin: 0;
  font-size: 16px;
  color: #0f172a;
}

.badge-text {
  font-size: 13px;
  color: #64748b;
  font-weight: 600;
}

.chart-container {
  position: relative;
  width: 100%;
  height: 180px;
}

.chart-main {
  height: 360px;
}

.sub-charts-grid {
  display: grid;
  grid-template-columns: 1fr;
  gap: 24px;
}

.empty-state {
  margin: 0;
  color: #64748b;
  font-size: 14px;
  height: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
}

@media (max-width: 920px) {
  .main-content {
    padding: 16px;
  }

  .toolbar-card {
    align-items: stretch;
  }

  .toolbar-controls {
    margin-left: 0;
  }

  .sub-charts-grid {
    grid-template-columns: 1fr;
  }
}
</style>