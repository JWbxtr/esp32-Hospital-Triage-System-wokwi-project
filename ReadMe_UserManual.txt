================================================================================
             ESP32 CLINICAL QUEUE MANAGEMENT SYSTEM (QMS)
                      CLINICAL USER MANUAL & GUIDE
================================================================================

QUICK START & SYSTEM BOOT
-------------------------
1. Plug power into the ESP32 board via USB/5V supply.
2. Watch the local 16x2 LCD screen:
   - First line will show "BOOTING OS...".
   - Second line will show "WIFI READY" once the local Access Point is running.
3. The screen will automatically open to the "MAIN MENU".

BUTTON CONTROLS AT A GLANCE
---------------------------
- CYCLE BUTTON : Press to move through menu items or change option selections.
- OK BUTTON : Press to confirm your selection and proceed to the next step.

STEP-BY-STEP TRIAGE PROCESS
---------------------------
Step 1: Start Triage Session
  - From the MAIN MENU, select "1. RUN TRIAGE" using the OK button.

Step 2: Patient Presence Detection
  - The screen displays "SCANNING RADAR".
  - Have the patient step within 30cm of the ultrasonic sensor.
  - Once detected, the LCD flashes "PATIENT DETECTED" and advances automatically.

Step 3: Thermal Reading (DIAG 1: THERMAL)
  - Adjust the potentiometer knob to match/simulate the patient's body temperature.
  - The display updates temperature in real-time (e.g., "TMP:37.5C [OK]").
  - Press the OK button to record the temperature reading.

Step 4: Respiratory Effort (DIAG 2: BREATH)
  - Press CYCLE to flip through options: > NORMAL, > LABOURED, or > CRITICAL.
  - Press OK when the correct condition is selected.

Step 5: Hydration Status (DIAG 3: STATUS)
  - Press CYCLE to flip through options: > NORMAL, > MILD/WEAK, or > SEVERE.
  - Press OK to confirm.

Step 6: Vulnerability Group (DIAG 4: VULN)
  - Press CYCLE to select between: > NONE, > PREGNANT, or > PEDIATRIC.
  - Press OK to compute the triage outcome.

Step 7: View Triage Result
  - The LCD shows the priority tier (e.g., "PRIO:1 CRITICAL" or "PRIO:3 ROUTINE").

  - Press the OK button to clear the result screen, update the patient counter, and return to the main menu.

CONNECTING TO THE LOCAL WEB DASHBOARD
-------------------------------------
Staff can monitor patient queue activity remotely using any phone or laptop:
1. Open Wi-Fi settings on your phone/PC.
2. Connect to the network: QMS-Clinic-Queue
3. Enter password: Password123
4. Open your web browser and go to: http://192.168.4.1
5. You can view the live status of active diagnoses, total patients triaged, and toggle light/dark display modes.

SYSTEM CONFIGURATION & INFO
---------------------------
- Reset Patient Counter: Go to "2. SYS CONFIG" from the Main Menu. Toggle to ">YES" using CYCLE, then press OK to clear the stored triage count from Flash memory.
- System Credits: Select "3. SYS INFO" from the Main Menu to cycle through the names of the engineering team members.