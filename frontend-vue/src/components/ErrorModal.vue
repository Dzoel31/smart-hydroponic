<template>
  <Transition name="error-modal-fade">
    <div
      v-if="modelValue"
      class="error-modal-overlay"
      role="presentation"
      @click.self="close"
    >
      <section
        class="error-modal"
        role="dialog"
        aria-modal="true"
        :aria-labelledby="titleId"
        :aria-describedby="messageId"
      >
        <div class="error-modal__icon" aria-hidden="true">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M12 9v4"></path>
            <path d="M12 17h.01"></path>
            <path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.72 3h16.92a2 2 0 0 0 1.72-3L13.71 3.86a2 2 0 0 0-3.42 0z"></path>
          </svg>
        </div>

        <h3 :id="titleId" class="error-modal__title">{{ title }}</h3>
        <p :id="messageId" class="error-modal__message">{{ message }}</p>

        <div class="error-modal__actions">
          <button type="button" class="error-modal__button" @click="close">
            {{ confirmText }}
          </button>
        </div>
      </section>
    </div>
  </Transition>
</template>

<script setup lang="ts">
import { computed } from 'vue';

withDefaults(
  defineProps<{
    modelValue: boolean;
    title?: string;
    message?: string;
    confirmText?: string;
  }>(),
  {
    title: 'Terjadi Kesalahan',
    message: 'Sistem tidak dapat memproses permintaan saat ini.',
    confirmText: 'Tutup',
  },
);

const emit = defineEmits<{
  (event: 'update:modelValue', value: boolean): void;
  (event: 'close'): void;
}>();

const titleId = computed(() => 'error-modal-title');
const messageId = computed(() => 'error-modal-message');

const close = (): void => {
  emit('update:modelValue', false);
  emit('close');
};
</script>

<style scoped>
.error-modal-overlay {
  position: fixed;
  inset: 0;
  z-index: 2000;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 24px;
  background: rgba(15, 23, 42, 0.62);
  backdrop-filter: blur(6px);
}

.error-modal {
  width: min(100%, 420px);
  border-radius: 20px;
  background: #ffffff;
  padding: 28px;
  box-shadow: 0 24px 60px rgba(15, 23, 42, 0.24);
  text-align: center;
}

.error-modal__icon {
  width: 56px;
  height: 56px;
  margin: 0 auto 16px;
  border-radius: 999px;
  display: grid;
  place-items: center;
  color: #b91c1c;
  background: #fee2e2;
}

.error-modal__icon svg {
  width: 28px;
  height: 28px;
}

.error-modal__title {
  margin: 0;
  color: #0f172a;
  font-size: 1.25rem;
  font-weight: 700;
}

.error-modal__message {
  margin: 12px 0 0;
  color: #475569;
  font-size: 0.95rem;
  line-height: 1.6;
}

.error-modal__actions {
  margin-top: 24px;
  display: flex;
  justify-content: center;
}

.error-modal__button {
  min-width: 120px;
  border: none;
  border-radius: 12px;
  padding: 12px 18px;
  background: linear-gradient(135deg, #ef4444, #dc2626);
  color: #ffffff;
  font-weight: 700;
  cursor: pointer;
  transition: transform 0.15s ease, box-shadow 0.15s ease;
}

.error-modal__button:hover {
  transform: translateY(-1px);
  box-shadow: 0 12px 24px rgba(220, 38, 38, 0.24);
}

.error-modal-fade-enter-active,
.error-modal-fade-leave-active {
  transition: opacity 0.18s ease;
}

.error-modal-fade-enter-from,
.error-modal-fade-leave-to {
  opacity: 0;
}
</style>