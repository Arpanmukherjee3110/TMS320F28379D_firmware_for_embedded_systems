# 🔧 TI F28379D Embedded Firmware – Internship Projects

This repository contains a collection of **embedded firmware projects** developed using the **Texas Instruments TMS320F28379D** microcontroller during my internship. The focus is on **real-time control**, **power electronics**, **ADC-based feedback**, **PWM signal generation**, and **serial communication** for modern energy systems.

---

## 📌 Projects Overview

### ⚡ 1. Closed-Loop Buck Converter Control
> Dual-loop voltage and current regulation with high-speed SPWM

- SPWM from `ePWM1A`
- Outer loop: Voltage PI controller
- Inner loop: Current PI controller
- ADC feedback with voltage divider (gain = 100)
- ISR-based duty cycle update

---

### 🔄 2. H-Bridge Inverter (SPWM Control)
> Sinusoidal PWM for inverter operation

- ePWM with complementary outputs
- Dead-band insertion for safe switching
- Open-loop and closed-loop configurations

---

### 🌀 3. Dual-Loop SPWM Inverter
> Precision inverter with voltage-current nested control

- Outer PI loop for voltage tracking
- Inner PI loop for current shaping
- Real-time sine table based SPWM modulation

---

### ⚡ 4. Power Factor Correction (PFC) Boost Converter
> High-performance front-end converter with improved power factor

- ADC-based current shaping
- PWM control with dynamic duty adjustment
- Suitable for smart grid and renewable energy interfaces

---

### 🔗 5. UART Serial Communication
> Send and receive float data over SCIA

- UART initialization and ISR-based handling
- Real-time variable telemetry
- Float-to-byte conversion for monitoring/control

---

### 📊 6. ADC Sampling & Feedback
> Multi-channel analog signal processing

- Reads voltage/current via ADC channels
- Gain scaling using resistive dividers
- Pre-conditioning for controller input

---

## 🛠 Development Environment

- **MCU:** TI LaunchPad F28379D (TMS320F28379D)
- **IDE:** Code Composer Studio (CCS) v12.4.0
- **SDK:** C2000Ware 5.00.00.00
- **Language:** Embedded C (bare-metal)
- **Target Applications:** DC-DC converters, Inverters, Smart energy systems

---


---

## 🚀 Learning Outcomes

- Mastery of ePWM, ADC, and UART modules
- Real-time control with ISR-based regulation
- Design and tuning of PI controllers
- Embedded software design for power systems
- Strong foundation for AI-assisted control

---

## 💡 Future Plans

- 📶 Add CAN/I2C communication
- 🧠 Integrate AI/ML for predictive control
- 📊 Log data to SD card for offline analysis
- 🌐 Web interface using UART-to-RPi bridge

---

## 👨‍💻 Author

**Arpan Mukherjee**  
*B.Tech | Embedded Systems & Power Electronics*  
Intern @ [Your Organization Name]  
[LinkedIn](https://www.linkedin.com/in/arpan-mukherjee-2aa657254/) • [Email](mailto:arpanmukherjee3110@gmail.com)

---




