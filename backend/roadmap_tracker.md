# Roadmap Task Tracker: Smart Hydroponic

Dokumen ini adalah ringkasan dari *milestone* pada *roadmap* utama untuk melacak penyelesaian tugas (task) harian/mingguan. Tabel ini telah dikelompokkan berdasarkan area kerja: **IoT, Backend, Frontend**, dan **Umum**. Ubah `[ ]` menjadi `[x]` saat tugas telah selesai diimplementasikan.

## Ringkasan Tugas & Timeline
- **Umum**: 2 tugas (Sepanjang proyek)
- **IoT**: 5 tugas (Agustus 2026 - Januari 2027)
- **Backend**: 14 tugas (Agustus 2026 - Januari 2027)
- **Frontend**: 7 tugas (Agustus 2026 - Januari 2027)
- **Total**: 28 tugas

Timeline yang ditentukan sebagai target penyelesaian pengembangan. Saling koordinasi dan diskusi.

| Kategori | Fase / Jadwal | Tugas (Task) | Status |
| :--- | :--- | :--- | :---: |
| **Umum** | Persiapan (Sepanjang Proyek) | *Transfer Knowledge* arsitektur sistem dari pengurus lama. | `[ ]` |
| **Umum** | Persiapan (Sepanjang Proyek) | Memahami cara membaca dokumentasi dan *codebase*. | `[ ]` |
| **IoT** | Persiapan (Sepanjang Proyek) | Pengajuan komponen dan alat baru. | `[ ]` |
| **IoT** | Fase 1 & 2 (Agt-Sep 2026) | Merapikan perkabelan (*cable management*) dan dikelompokkan menjadi modul (aman air & estetis). | `[ ]` |
| **IoT** | Fase 1 & 2 (Agt-Sep 2026) | Perbaikan dan kalibrasi sensor (pH, TDS). | `[ ]` |
| **IoT** | Fase 4 (Nov-Des 2026) | Memastikan *hardware* terus mencatat & mengirim data sensor tanpa putus. | `[ ]` |
| **IoT** | Fase 4 (Nov-Des 2026) | Pengurus melakukan pendataan label manual (berat basah, tinggi, dll) untuk *dataset*. | `[ ]` |
| **Backend** | Fase 1 & 2 (Agt-Sep 2026) | Memperbaiki sistem *monitoring* agar stabil tanpa *lag* atau kehilangan data. | `[ ]` |
| **Backend** | Fase 1 & 2 (Agt-Sep 2026) | Menjalankan sistem otomatisasi nutrisi tingkat dasar. | `[ ]` |
| **Backend** | Fase 1 & 2 (Agt-Sep 2026) | Fitur log untuk merekam kondisi yang terjadi pada sistem hidroponik. | `[ ]` |
| **Backend** | Fase 1 & 2 (Agt-Sep 2026) | Fitur notifikasi sistem & penyediaan data *real-time* untuk UI notifikasi web. | `[ ]` |
| **Backend** | Fase 1 & 2 (Agt-Sep 2026) | Menyesuaikan *database* agar menyimpan historis per jenis tanaman (menyediakan data untuk kolom tabel Frontend). | `[ ]` |
| **Backend** | Fase 1 & 2 (Agt-Sep 2026) | Menyediakan API/skema *database* untuk kebutuhan nutrisi berdasarkan umur/fase pertumbuhan tanaman. | `[ ]` |
| **Backend** | Fase 1 & 2 (Agt-Sep 2026) | Membuat *endpoint* ringkasan metrik dari Analytics untuk dikonsumsi halaman Dashboard utama. | `[ ]` |
| **Backend** | Fase 1 & 2 (Agt-Sep 2026) | Integrasi MCP (Model Context Protocol) untuk sistem hidroponik + Chatbot. | `[ ]` |
| **Backend** | Fase 3 (Okt 2026) | Mengimplementasikan logika *Fuzzy* untuk otomatisasi nutrisi dari banyak sensor. | `[ ]` |
| **Backend** | Fase 3 (Okt 2026) | **Fase Stabilisasi:** Sistem dibiarkan berjalan dengan logika *Fuzzy* untuk diamati. | `[ ]` |
| **Backend** | Fase 4 (Nov-Des 2026) | Membiarkan sistem hidroponik berjalan secara stabil hingga 1-2 siklus panen. | `[ ]` |
| **Backend** | Fase 5 (Jan 2027) | Mengekstrak dan membersihkan *dataset* hasil dari pengumpulan di Fase 4. | `[ ]` |
| **Backend** | Fase 5 (Jan 2027) | Melakukan *training* model Machine Learning (regresi/klasifikasi). | `[ ]` |
| **Backend** | Fase 5 (Jan 2027) | Mengintegrasikan model AI terlatih (*trained model*) ke dalam *backend*. | `[ ]` |
| **Frontend** | Fase 1 & 2 (Agt-Sep 2026) | Tampilan menu untuk menambahkan profil nutrisi masing-masing tanaman. | `[ ]` |
| **Frontend** | Fase 1 & 2 (Agt-Sep 2026) | Mengubah alur *dashboard*: *Memilih jenis tanaman yang dipantau ➔ Tampilan Utama ➔ Menu*. | `[ ]` |
| **Frontend** | Fase 1 & 2 (Agt-Sep 2026) | Deskripsi *chart* pada web menggunakan analisis deskriptif dari LLM (AI Generatif). | `[ ]` |
| **Frontend** | Fase 1 & 2 (Agt-Sep 2026) | Menampilkan kebutuhan nutrisi spesifik berdasarkan umur/fase pertumbuhan tanaman. | `[ ]` |
| **Frontend** | Fase 1 & 2 (Agt-Sep 2026) | UI notifikasi *real-time* di *website* (misal: alert pompa mati/air habis). | `[ ]` |
| **Frontend** | Fase 1 & 2 (Agt-Sep 2026) | Menambahkan kolom jenis tanaman dan format *timestamp* pada tabel riwayat data. | `[ ]` |
| **Frontend** | Fase 1 & 2 (Agt-Sep 2026) | Menampilkan ringkasan metrik dari halaman Analytics langsung ke tampilan Dashboard utama. | `[ ]` |

Jangan ragu untuk bertanya dan improvisasi. Sangat boleh untuk menambahkan ide atau fungsional ke dalam Sistem Hidroponik.