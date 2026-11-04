================================================================================
           ESP32 CLINICAL QUEUE MANAGEMENT SYSTEM (QMS)
                       SYSTEM OPERATION & ARCHITECTURE
================================================================================

1. WHAT THIS SYSTEM DOES
------------------------
This project is an offline, edge-computing triage workstation powered by an ESP32 microcontroller. In primary healthcare clinics, relying strictly on first-come, first-served queues can be dangerous for severe cases. This system solves that by capturing four basic clinical parameters—Temperature, Respiratory Effort, Hydration/Status, and Patient Vulnerability—and running them through an automated triage algorithm to rank patient urgency in real time.

2. HARDWARE ARCHITECTURE & SENSOR SETUP
---------------------------------------
* ESP32 DevKit V1: Serves as the central microcontroller executing all logic directly on-chip without relying on external servers or internet connectivity.
* HC-SR04 Ultrasonic Sensor : Detects when a patient steps up to the triage station within a 30cm zone.
* Thermal Sensing Proxy : Reads an analog voltage mapped to a clinical temperature range of 35.0°C to 41.0°C. This potentiometer serves as a temporary hardware dummy for an MLX90614 infrared sensor due to current component availability.
* Tactile Pushbuttons : Used by medical staff to quickly input qualitative patient observations.
* Local Visual Outputs: A 16x2 I2C LCD display (Address 0x27) shows operational status.

3. HOW THE TRIAGE ALGORITHM WORKS
---------------------------------
When staff completes inputting a patient's details, the firmware calculates an unbias Triage Coefficient (TC) using a weighted scoring formula:

    TC = (0.30 * Stemp) + (0.30 * Sresp) + (0.30 * Shyd) + (0.10 * Svuln)

Sub-score values are mapped from 0.0 (Normal/Low Risk) up to 1.0 (High Risk):
  - Temperature (Stemp): Normalized based on body temp reading.
  - Respiratory (Sresp): Normal = 0.0, Laboured = 0.5, Critical = 1.0.
  - Hydration (Shyd): Normal = 0.0, Mild/Weak = 0.5, Severe = 1.0.
  - Vulnerability (Svuln): None = 0.0, Pregnant = 0.5, Pediatric = 1.0.

Clinical Safety Tripwires (Hard Overrides):
  - Severe Hyperthermia Override: If Temp >= 39.5°C and the total calculated score is less than 0.65, the score is forced to 0.65 to ensure at least an URGENT priority level.
  - Critical Condition Override: If Respiratory is Critical (1.0), Hydration is Severe (1.0), or Temp >= 40.5°C, the score is instantly set to 0.95 to guarantee a CRITICAL classification.

Priority Tier Mapping:
  - CRITICAL (Score >= 0.75): Level 1 Priority. Red LED turns ON, Green LED turns OFF.
  - URGENT (Score 0.40 to 0.74): Level 2 Priority. Red LED turns ON, Green LED turns OFF.
  - ROUTINE (Score < 0.40): Level 3 Priority. Green LED turns ON, Red LED turns OFF.

4. LOCAL DASHBOARD & DATA PERSISTENCE
-------------------------------------
* Wireless Dashboard: The ESP32 spins up its own Wi-Fi Access Point (SSID: QMS-Clinic-Queue) hosting an embedded web server. It serves an HTML/CSS dashboard with live status updates and a dark/light mode toggle to nearby phones or tablets.
* Non-Volatile Memory: Patient triage counts are saved to non-volatile flash storage using the ESP32 Preferences library (`triage-sys` namespace). This ensures total patient records persist even if the board loses power or gets rebooted.