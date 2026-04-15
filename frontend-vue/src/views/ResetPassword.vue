<template>
    <div class="forgot-container" :style="{ '--bg-image': `url(${bgImg})` }">
        <div class="forgot-card">
            <div class="logo-header">
                <img :src="logoUPN" alt="Logo UPNVJ" class="logo" />
                <img :src="logoHydroponic" alt="Logo Smart Hydroponic" class="logo" />
            </div>

            <h2>Forgot Password?</h2>
            <p class="subtitle">Enter your registered email address and we'll send you a link to reset your password.</p>
            
            <form @submit.prevent="handleReset">
                <div class="input-group">
                    <span class="icon">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                            <path d="M4 4h16v16H4z"></path>
                            <path d="M22 6l-10 7L2 6"></path>
                        </svg>
                    </span>
                    <input 
                        type="email"
                        v-model="email"
                        placeholder="Email Address"
                        required
                        :disabled="isLoading"
                    />
                </div>

                <button type="submit" class="reset-btn" :disabled="isLoading">
                    {{ isLoading ? 'SENDING...' : 'SEND RESET LINK' }}
                </button>

                <div class="back-link-wrapper">
                    <router-link to="/login" class="back-link">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="back-icon">
                            <line x1="19" y1="12" x2="5" y2="12"></line>
                            <polyline points="12 19 5 12 12 5"></polyline>
                        </svg>
                        Back to Login
                    </router-link>
                </div>
            </form>
        </div>

        <p class="copyright">
            © 2026 - Tim Riset Internet of Things UPNVJ
        </p>

        <div v-if="isSuccess" class="popup-overlay">
            <div class="popup-content">
                <div class="success-icon">
                    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                        <path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"></path>
                        <polyline points="22 4 12 14.01 9 11.01"></polyline>
                    </svg>
                </div>
                <h3>Check Your Email!</h3>
                <p>We have sent a password reset link to <strong>{{ email }}</strong>.</p>
            </div>
        </div>

    </div>
</template>

<script setup lang="ts">
import { ref } from "vue";
import { useRouter } from "vue-router";

// Pastikan path gambar ini sesuai dengan struktur folder Anda
import bgImg from "@/assets/images/bg2.jpeg"; // Pakai gambar yang sama dengan login
import logoUPN from "@/assets/images/logo-upn.png";
import logoHydroponic from "@/assets/images/logo-hydroponic.png";

const email = ref<string>("");
const isLoading = ref<boolean>(false);
const isSuccess = ref<boolean>(false);

const router = useRouter();

const handleReset = (): void => {
    isLoading.value = true;

    // Trik Simulasi Pengiriman Email (Loading 1 detik)
    setTimeout(() => {
        isLoading.value = false;
        isSuccess.value = true;

        // Tahan pop-up sukses selama 3 detik, lalu kembalikan ke halaman login
        setTimeout(() => {
            isSuccess.value = false;
            router.push("/login");
        }, 3000);
        
    }, 1000);
};
</script>

<style scoped>
/* BACKGROUND CONTAINER */
.forgot-container {
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
    position: relative;
}

.forgot-container::before {
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

.forgot-container::after {
    content:"";
    position:absolute;
    inset:0;
    background:rgba(0,0,0,0.25);
    z-index:-1;
}

/* CENTERED GLASS CARD */
.forgot-card {
    background: rgba(255, 255, 255, 0.85);
    backdrop-filter: blur(8px);
    width: 100%;
    max-width: 420px;
    padding: 40px;
    margin-top: 20px;
    border-radius: 12px;
    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.2);
    box-sizing: border-box;
    text-align: center;
    z-index: 1;
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
    line-height: 1.5;
}

/* INPUT FIELDS */
.input-group {
    position: relative;
    margin-bottom: 20px;
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

.input-group input:disabled {
    background-color: #f3f4f6;
    color: #9ca3af;
    cursor: not-allowed;
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
}

.icon svg {
    width: 18px;
    height: 18px;
}

/* BUTTON */
.reset-btn {
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
    margin-bottom: 20px;
}

.reset-btn:hover:not(:disabled) {
    background: #388e3c;
}

.reset-btn:disabled {
    background: #a5d6a7;
    cursor: not-allowed;
}

/* BACK TO LOGIN LINK */
.back-link-wrapper {
    margin-top: 10px;
}

.back-link {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    color: #64748b;
    text-decoration: none;
    font-size: 14px;
    font-weight: 500;
    transition: color 0.2s;
}

.back-link:hover {
    color: #388e3c;
}

.back-icon {
    width: 16px;
    height: 16px;
}

/* COPYRIGHT */
.copyright {
    margin-top: 25px;
    font-size: 12px;
    color: #ffffff;
    text-shadow: 1px 1px 3px rgba(0, 0, 0, 0.8);
    z-index: 1;
}

/* POP-UP STYLES (Sama dengan RegistPage) */
.popup-overlay {
    position: fixed;
    top: 0;
    left: 0;
    width: 100vw;
    height: 100vh;
    background: rgba(0, 0, 0, 0.6);
    backdrop-filter: blur(4px);
    display: flex;
    align-items: center;
    justify-content: center;
    z-index: 9999;
}

.popup-content {
    background: #ffffff;
    padding: 30px 40px;
    border-radius: 16px;
    text-align: center;
    box-shadow: 0 10px 25px rgba(0, 0, 0, 0.2);
    animation: popIn 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275) forwards;
    max-width: 350px;
}

.success-icon {
    width: 60px;
    height: 60px;
    background-color: #dcfce7;
    color: #16a34a;
    border-radius: 50%;
    display: flex;
    align-items: center;
    justify-content: center;
    margin: 0 auto 16px;
}

.success-icon svg {
    width: 32px;
    height: 32px;
}

.popup-content h3 {
    margin: 0 0 8px 0;
    color: #1e293b;
    font-size: 20px;
}

.popup-content p {
    margin: 0;
    color: #64748b;
    font-size: 14px;
    line-height: 1.5;
}

@keyframes popIn {
    0% { transform: scale(0.8); opacity: 0; }
    100% { transform: scale(1); opacity: 1; }
}

/* RESPONSIVE */
@media (max-width: 480px) {
    .forgot-card {
        width: 90%;
        padding: 30px 20px;
    }
}
</style>