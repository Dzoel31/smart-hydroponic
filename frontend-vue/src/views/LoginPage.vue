<template>
    <div class="login-container" :style="{ '--bg-image': `url(${loginImg})` }">
        
        <div class="login-card">
            <div class="logo-header">
                <img :src="logoUPN" alt="Logo UPNVJ" class="logo" />
                <img :src="logoHydroponic" alt="Logo Smart Hydroponic" class="logo" />  
            </div>
                
            <h2>Welcome!</h2>
            <p class="subtitle">Smart Hydroponic Monitoring System FIK UPNVJ</p>
            
            <form @submit.prevent="handleLogin">
                <div class="input-group">
                    <span class="icon">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                            <path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"></path>
                            <circle cx="12" cy="7" r="4"></circle>
                        </svg>
                    </span>
                    <input
                        type="text"                      
                        v-model="username"
                        placeholder="Username" 
                        required
                    />
                </div>

                <div class="input-group">
                    <span class="icon">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                            <rect x="3" y="11" width="18" height="11" rx="2" ry="2"></rect>
                            <path d="M7 11V7a5 5 0 0 1 10 0v4"></path>
                        </svg>
                    </span>
                    <input
                        :type="showPassword ? 'text' : 'password'"
                        v-model="password"
                        placeholder="Password"
                        required
                    />

                    <button
                        type="button"
                        class="btn-toggle-password"
                        @click="showPassword = !showPassword"
                        tabindex="-1"
                        aria-label="Toggle password visibility"
                    >
                        <svg v-if="showPassword" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                            <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path>
                            <circle cx="12" cy="12" r="3"></circle>
                        </svg>
                        <svg v-else viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                            <path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"></path>
                            <line x1="1" y1="1" x2="23" y2="23"></line>
                        </svg>
                    </button>
                </div>

                <div class="forgot-wrapper">
                    <router-link to="/reset-password" class="forgot-link">Forgot Password?</router-link>
                </div>
        
                <button type="submit" class="login-btn">
                    LOG IN
                </button>

                <p class="request-text">
                    Don't have an account?
                    <a href="" class="request-link">Request Access</a>
                </p>
            </form>
        </div>

        <p class="copyright">
            © 2026 - Tim Riset Internet of Things UPNVJ
        </p>
    </div>
</template>

<script setup lang="ts">
import { ref } from "vue";
import { useRouter } from "vue-router";
import { authState } from "../auth";
import { UsersService, ApiError } from "../api";
import { getApiErrorMessage } from "../utils/apiError";

import loginImg from "@/assets/images/bg2.jpeg";
import logoUPN from "@/assets/images/logo-upn.png";
import logoHydroponic from "@/assets/images/logo-hydroponic.png";

const username = ref<string>("");
const password = ref<string>("");
const showPassword = ref<boolean>(false);

const router = useRouter();

const handleLogin = async (): Promise<void> => {
    const inputUsername = username.value.toLowerCase();
    const inputPassword = password.value;
    
    try {
        const token = await UsersService.loginUser({
            username: inputUsername,
            password: inputPassword
        });

        authState.setSession(token.access_token, token.user);
        router.push('/dashboard');

    } catch (error) {
        if (error instanceof ApiError) {
            const message = getApiErrorMessage(error, 'Gagal login.');
            console.error('Error logging in:', message);
        } else {
            console.error('Unexpected error:', error);
        }
    }
};
</script>

<style scoped>
/* BACKGROUND CONTAINER */
.login-container {
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    width: 100vw;
    min-height: 100vh;
    background-size: cover;
    background-position: center;
    background-repeat: no-repeat;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
}

.login-container::before {
    content:"";
    position:absolute;
    inset:0;

    background-image: var(--bg-image);
    background-size: cover;
    background-position: center;

    filter:blur(5px);
    transform:scale(1.0);

    z-index:-2;
}

.login-container::after {
    content:"";
    position:absolute;
    inset:0;
    background:rgba(0,0,0,0.25);
    z-index:-1;
}

/* CENTERED GLASS CARD */
.login-card {
    background: rgba(255, 255, 255, 0.85); /* Warna putih transparan */
    backdrop-filter: blur(8px); /* Efek blur kaca */
    width: 100%;
    max-width: 420px;
    padding: 40px;
    border-radius: 12px;
    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.2);
    box-sizing: border-box;
    text-align: center;
}

/* LOGO HEADER */
.logo-header {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 15px;
    margin-bottom: 20px;
}

.logo {
    height: 40px;
    width: auto;
    object-fit: contain;
}

/* TYPOGRAPHY */
h2 {
    margin: 0 0 8px 0;
    color: #111;
    font-size: 24px;
    font-weight: 700;
}

.subtitle {
    color: #444;
    font-size: 14px;
    margin: 0 0 25px 0;
}

/* INPUT FIELDS */
.input-group {
    position: relative;
    margin-bottom: 16px;
}

.input-group input {
    width: 100%;
    padding: 12px 12px 12px 42px;
    border-radius: 8px;
    border: 1px solid #d1d5db;
    background: #ffffff;
    outline: none;
    color: #333;
    font-size: 14px;
    box-sizing: border-box;
    transition: 0.2s ease-in-out;
}

.input-group input:focus {
    border-color: #419641;
    box-shadow: 0 0 0 2px rgba(65, 150, 65, 0.2);
}

.input-group input::placeholder {
    color: #9ca3af;
}

/* SVG ICONS */
.icon {
    position: absolute;
    left: 14px;
    top: 50%;
    transform: translateY(-50%);
    color: #9ca3af;
    display: flex;
    align-items: center;
    pointer-events: none;
}

.icon svg {
    width: 18px;
    height: 18px;
}

/* TOGGLE PASSWORD BUTTON */
.btn-toggle-password {
    position: absolute;
    right: 14px;
    top: 50%;
    transform: translateY(-50%);
    background: transparent;
    border: none;
    color: #9ca3af;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 0;
    cursor: pointer;
    transition: color 0.2s;
    outline: none;
}

.btn-toggle-password:focus {
    outline: none;
}

.btn-toggle-password:hover {
    color: #419641;
}

.btn-toggle-password svg {
    width: 18px;
    height: 18px;
}

/* FORGOT PASSWORD */
.forgot-wrapper {
    display: flex;
    justify-content: flex-end;
    margin-bottom: 20px;
}

.forgot-link {
    font-size: 13px;
    color: #388e3c;
    text-decoration: none;
    font-weight: 500;
}

.forgot-link:hover {
    text-decoration: underline;
}

/* BUTTON */
.login-btn {
    width: 100%;
    padding: 14px;
    border: none;
    border-radius: 8px;
    background: #4caf50;
    color: white;
    font-weight: 600;
    font-size: 14px;
    letter-spacing: 0.5px;
    cursor: pointer;
    transition: 0.2s;
}

.login-btn:hover {
    background: #388e3c;
}

/* request TEXT */
.request-text {
    margin-top: 20px;
    font-size: 14px;
    color: #444;
}

.request-link {
    color: #388e3c;
    text-decoration: none;
    font-weight: 600;
    margin-left: 4px;
}

.request-link:hover {
    text-decoration: underline;
}

/* COPYRIGHT */
.copyright {
    margin-top: 25px;
    font-size: 12px;
    color: #ffffff;
    text-shadow: 1px 1px 3px rgba(0, 0, 0, 0.8); /* Tambahan shadow agar teks mudah dibaca */
}

/* RESPONSIVE */
@media (max-width: 480px) {
    .login-card {
        width: 90%;
        padding: 30px 20px;
    }
}
</style>