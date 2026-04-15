import { createRouter, createWebHistory } from 'vue-router';
import { authState } from '@/auth';

const routes = [
  { path: '/', redirect: '/dashboard' },
  {
    path: '/login',
    name: 'Login',
    component: () => import('@/views/LoginPage.vue')
  },
  {
    path: '/reset-password',
    name: 'ForgotPassword',
    component: () => import('@/views/ResetPassword.vue')
  },
  {
    path: '/dashboard',
    name: 'Dashboard',
    component: () => import('@/views/Dashboard.vue'),
  },
  {
    path: '/data-logs',
    name: 'SensorData',
    component: () => import('@/views/DataLogs.vue'),
    meta: { requiresAuth: true, allowedRoles: ['admin', 'superadmin'] }
  },
  {
    path: '/analytics',
    name: 'Analytics',
    component: () => import('@/views/Analytics.vue'),
  },
  {
    path: '/manage-admin',
    name: 'ManageAdmin',
    component: () => import('@/views/ManajemenAkses.vue'),
    meta: { requiresAuth: true, allowedRoles: ['superadmin'] }
  },
  {
    path: '/profile',
    name: 'Profile',
    component: () => import('@/views/Profile.vue'),
    meta: { requiresAuth: true, allowedRoles: ['admin', 'superadmin'] }
  }
];

const router = createRouter({
  history: createWebHistory(),
  routes
});

router.beforeEach((to, from, next) => {
  const requiresAuth = to.matched.some(record => record.meta.requiresAuth);
  const allowedRoles = to.meta.allowedRoles as string[];
  const currentRole = authState.user?.role || null;

  if (requiresAuth) {
    if (!authState.isLoggedIn) {
      next('/login');
    } else if (allowedRoles && (!currentRole || !allowedRoles.includes(currentRole))) {
      next('/dashboard');
    } else {
      next();
    }
  } else {
    next();
  }
});

export default router;