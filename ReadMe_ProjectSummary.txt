================================================================================
             ESP32 CLINICAL QUEUE MANAGEMENT SYSTEM (QMS)
                    DOCUMENTATION & PROJECT SUMMARY
================================================================================

PROJECT CONTEXT & PROBLEM STATEMENT
-----------------------------------
* Author: Jolly-Webster Mabhena.
* Core Challenge: In resource-constrained clinics, traditional First-Come, First-Served queue management leads to long wait times and dangerous delays for patients needing urgent medical attention.
* Proposed Solution: An edge-computed triage workstation that categorizes incoming patients into CRITICAL, URGENT, and ROUTINE tiers based on objective vital metrics.

DEVELOPMENT METHODOLOGY
-----------------------
* Model: Hardware-Software Co-Design Agile Methodology.
* Simulation Phase: The circuit and logic were initially designed and validated inside the Wokwi embedded online simulator to test state machines and scoring algorithms.
* Physical Prototyping: Built on a solderless breadboard using an ESP32 30-pin DevKit board, basic sensor modules, and passive pushbuttons.
* Design Strategy: Offline edge-computing architecture ensuring zero reliance on internet connectivity, external databases, or third-party cloud infrastructure.

TECHNICAL HARDWARE SPECIFICATIONS
----------------------------------
* Microcontroller: ESP32 DevKit V1 executing C++ firmware written in Arduino IDE.
* Presence Sensor: HC-SR04 Ultrasonic Sonar Module for proximity detection within 30cm.
* Temperature Input Proxy: 10k Linear Potentiometer connected to ESP32 ADC simulating non-contact thermal data.
* Display: 1602 LCD with I2C for 2-wire serial communications.
* Web Interface: Built-in SoftAP Wi-Fi serving an HTTP dashboard with persistent CSS/JS themes.
* Storage: NVS Flash persistence via `Preferences.h` for cumulative patient counters.

FUTURE IMPROVEMENTS & ROADMAP
-----------------------------
* Physical Hardware Sensors: Swap out the potentiometer thermal proxy with a physical MLX90614 I2C contactless infrared sensor once hardware becomes available.
* Diagnostic Expansion: Incorporate physical pulse-oximetry (SpO2) and heart-rate sensors for broader diagnostic data collection.
* Multi-Room Networking: Expand Wi-Fi functionality to connect multiple ESP32 nodes across separate clinic consultation rooms synced to a centralized dashboard.
* Reliability Upgrades: Integrate a hardware Watchdog Timer (WDT) in firmware to automatically recover from system hangs or unexpected fault states.