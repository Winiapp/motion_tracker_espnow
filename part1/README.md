# ⚡ ESP-NOW Motion Tracker Project

This project demonstrates a **motion tracking system** built on the **ESP-NOW protocol** — a lightweight, low-power wireless communication method developed by Espressif.

---

## 📡 Project Overview

- The system consists of **three sender nodes** and **one receiver (gateway)**.
- Each sender transmits data to the receiver using ESP-NOW for **low-latency, peer-to-peer communication**.
- The receiver aggregates data from all senders and can forward it via **Bluetooth Serial** for monitoring or logging.

---

## 🧪 Initial Testing Phase

During the initial tests:
- Each sender counted numbers from **1 to 10000** at **50 ms intervals**.
- The transmitted data was successfully received, logged, and verified by the receiver.
- Experimental results were collected and will be published later as detailed **documentation** and **performance analysis**.

---

## 🧭 System Notes

- The **receiver** acts as a central gateway, collecting and forwarding messages from all nodes.
- The **senders** can be extended to include real sensor data (e.g., motion or acceleration).
- This setup forms the foundation for future **distributed motion tracking** and **sensor data aggregation** applications using ESP-NOW.

---

## 🛠️ Hardware & Software

- **Microcontrollers:** ESP32 boards (sender ×3, receiver ×1)
- **Protocol:** ESP-NOW (Wi-Fi STA mode)
- **IDE:** Arduino IDE or ESP-IDF v5.5+
- **Optional:** Bluetooth Serial for wireless data forwarding




