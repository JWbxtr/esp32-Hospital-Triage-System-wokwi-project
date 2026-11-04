#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Preferences preferences;
WebServer server(80);

// Pin Config
const int trigPin = 5;
const int echoPin = 18;
const int potPin = 34; 
const int btnCycle = 4; // Cycles_options
const int btnOK = 15;   // Confirms_my_selection
const int ledRed = 23;  // Priority_LED
const int ledGreen = 19;// Routine_LED

// Core State Machines
enum State { MENU, RUN_TRIAGE, SYS_CONFIG, SYS_INFO };
State currentState = MENU;

// Updated 4-Step Clinical Triage Flow
enum TriageState { WAITING, GET_TEMP, GET_RESP, GET_HYDRATION, GET_VULN, SHOW_RESULT };
TriageState triageState = WAITING;


float currentTemp = 0.0;
int currentResp = 0;       // 0: Normal, 1: Laboured, 2: Critical
int currentHydration = 0;  // 0: Normal, 1: Mild/Weak, 2: Severe Dehydration
int currentVuln = 0;       // 0: None, 1: Pregnant, 2: Pediatric
int totalTriages = 0;
int prioLevel = 3;
String prioLabel = "PENDING";

// mapping for my Web Dashboard & LCD
const char* respStr[] = {"Normal", "Laboured", "Critical"};
const char* hydStr[] = {"Normal", "Mild/Weak", "Severe Dehydration"};
const char* vulnStr[] = {"None", "Pregnant", "Pediatric"};

// Menu Selection Variables
int menuIndex = 0;
const int totalMenuItems = 3; 
const char* menuItems[] = {
  "1. RUN TRIAGE",
  "2. SYS CONFIG",
  "3. SYS INFO"
};

// Info Credits 
int infoIndex = 0;
const int totalCredits = 4;
const char* credits[] = {
  "MABHENA. JW"
};


unsigned long lastBtnPress = 0;

bool isBtnPressed(int pin) {
  if (digitalRead(pin) == LOW && (millis() - lastBtnPress > 300)) {
    lastBtnPress = millis();
    return true;
  }
  return false;
}

// ====== WEB DASHBOAR==========
void handleRoot() {
  String html = "<!DOCTYPE html><html lang='en'><head>";
  html += "<meta charset='UTF-8'>"; 
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<title>Group 8 Triage Hub</title>";
  
  // JavaScript handling my Light/Dark mode.
  html += "<script>";
  html += "let theme = localStorage.getItem('theme') || 'dark';";
  html += "document.documentElement.setAttribute('data-theme', theme);";
  html += "function toggleMode() {";
  html += "  theme = theme === 'dark' ? 'light' : 'dark';";
  html += "  localStorage.setItem('theme', theme);";
  html += "  document.documentElement.setAttribute('data-theme', theme);";
  html += "}";
  html += "</script>";
  
  //  theme
  html += "<style>";
  html += ":root[data-theme='dark'] { --bg: #131314; --box-bg: #1e1f20; --text: #e3e3e3; --border: #444746; --btn-bg: #004a77; --btn-text: #c2e7ff; --norm: #4dff4d; --hl: #ffea00; }";
  html += ":root[data-theme='light'] { --bg: #f0f4f9; --box-bg: #ffffff; --text: #1f1f1f; --border: #c4c7c5; --btn-bg: #c2e7ff; --btn-text: #001d35; --norm: #1f883d; --hl: #d97706; }";
  
  html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: var(--bg); color: var(--text); text-align: center; padding: 20px; margin: 0; transition: background 0.3s, color 0.3s; }";
  html += ".box { background: var(--box-bg); border: 1px solid var(--border); border-radius: 12px; padding: 20px; margin: 15px auto; max-width: 450px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }";
  html += ".crit { color: #ff4d4d; font-weight: bold; }";
  html += ".norm { color: var(--norm); font-weight: bold; }";
  html += ".highlight { color: var(--hl); font-weight: bold; }";
  html += ".theme-btn { background: var(--btn-bg); color: var(--btn-text); border: none; padding: 10px 20px; border-radius: 20px; cursor: pointer; font-weight: 600; font-size: 14px; margin-bottom: 20px; transition: 0.2s; }";
  html += ".theme-btn:hover { opacity: 0.8; }";
  html += "hr { border: 0; border-top: 1px solid var(--border); margin: 15px 0; }";
  html += "h2 { margin-top: 0; letter-spacing: 1px; }";
  html += "p { margin: 8px 0; }";
  html += "</style></head><body>";
  
  html += "<button class='theme-btn' onclick='toggleMode()'>Toggle Light / Dark Mode</button>";
  html += "<h2>GROUP 8 TRIAGE HUB</h2>";
  html += "<div class='box'><h3>Total Patients Triaged: <span class='highlight'>" + String(totalTriages) + "</span></h3></div>";
  
  html += "<div class='box'><h3>Live Terminal Status</h3>";
  
  if (currentState == MENU) {
    html += "<p>Mode: <b>Main Menu</b> (System Idle)</p>";
  } else if (currentState == SYS_INFO) {
    html += "<p>Mode: <b>System Info</b></p>";
  } else if (currentState == SYS_CONFIG) {
    html += "<p>Mode: <b class='highlight'>System Config</b></p><hr>";
    html += "<p>Status: Awaiting memory reset confirmation...</p>";
  } else if (currentState == RUN_TRIAGE) {
    html += "<p>Mode: <b class='highlight'>Triage Active</b></p><hr>";
    
    if (triageState == WAITING) html += "<p>Status: Waiting for Patient (Radar Scanning)</p>";
    else {
      html += "<p>Status: Diagnosing Patient...</p>";
      html += "<p><b>Temp:</b> " + String(currentTemp, 1) + " &deg;C</p>";
      if (triageState > GET_RESP || triageState == SHOW_RESULT) html += "<p><b>Breathing:</b> " + String(respStr[currentResp]) + "</p>";
      if (triageState > GET_HYDRATION || triageState == SHOW_RESULT) html += "<p><b>Hydration/Status:</b> " + String(hydStr[currentHydration]) + "</p>";
      if (triageState > GET_VULN || triageState == SHOW_RESULT) html += "<p><b>Vulnerability:</b> " + String(vulnStr[currentVuln]) + "</p>";
    }
    
    if (triageState == SHOW_RESULT) {
      html += "<hr><h3>FINAL PRIORITY: ";
      if (prioLevel == 1) html += "<span class='crit'>" + prioLabel + "</span></h3>";
      else if (prioLevel == 2) html += "<span class='highlight'>" + prioLabel + "</span></h3>";
      else html += "<span class='norm'>" + prioLabel + "</span></h3>";
    }
  }
  
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

// ======================== SETUP ========================
void setup() {
  lcd.init();
  lcd.backlight();
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(btnCycle, INPUT_PULLUP);
  pinMode(btnOK, INPUT_PULLUP);
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  
  preferences.begin("triage-sys", false);
  totalTriages = preferences.getInt("count", 0);

  lcd.setCursor(0, 0);
  lcd.print("BOOTING OS...");

  WiFi.softAP("QMS-Clinic-Queue", "Password123");
  server.on("/", handleRoot);
  server.begin();

  lcd.setCursor(0, 1);
  lcd.print("WIFI READY");
  delay(1500);
  lcd.clear();
}

// ========= LOOP ========================
void loop() {
  server.handleClient(); 

  switch (currentState) {
    case MENU: drawMenuScreen(); break;
    case RUN_TRIAGE: runTriageEngine(); break;
    case SYS_CONFIG: runConfigEngine(); break;
    case SYS_INFO: runInfoEngine(); break;
  }
}

// ======================== SUB-SYSTE====================

void drawMenuScreen() {
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, LOW); 
  
  lcd.setCursor(0, 0);
  lcd.print("   MAIN MENU    ");
  lcd.setCursor(0, 1);
  lcd.print(">"); 
  lcd.print(menuItems[menuIndex]);
  
  if (isBtnPressed(btnCycle)) {
    menuIndex = (menuIndex + 1) % totalMenuItems;
    lcd.clear();
  }
  
  if (isBtnPressed(btnOK)) {
    lcd.clear();
    if (menuIndex == 0) {
      currentState = RUN_TRIAGE;
      triageState = WAITING;
    } else if (menuIndex == 1) {
      currentState = SYS_CONFIG;
    } else if (menuIndex == 2) {
      currentState = SYS_INFO;
      infoIndex = 0;
    }
  }
}

void runConfigEngine() {
  static int resetChoice = 0; 

  lcd.setCursor(0, 0);
  lcd.print("RST PATIENT CNT?");
  lcd.setCursor(0, 1);
  
  if (resetChoice == 0) {
    lcd.print(">NO       YES   ");
  } else {
    lcd.print(" NO      >YES   ");
  }

  if (isBtnPressed(btnCycle)) {
    resetChoice = (resetChoice + 1) % 2;
  }

  if (isBtnPressed(btnOK)) {
    if (resetChoice == 1) {
      totalTriages = 0;
      preferences.putInt("count", totalTriages);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MEMORY CLEARED!");
      delay(1500); 
    }
    
    resetChoice = 0; 
    lcd.clear();
    currentState = MENU;
  }
}

void runTriageEngine() {
  switch (triageState) {
    case WAITING: {
      lcd.setCursor(0, 0);
      lcd.print("SCANNING RADAR  ");
      long dist = getDistance();
      lcd.setCursor(0, 1);
      if (dist == -1) lcd.print("SENSOR READY    ");
      else { lcd.print("DIST: "); lcd.print(dist); lcd.print("cm      "); }

      if ((dist > 0 && dist < 30)) {
        lcd.clear();
        lcd.print("PATIENT DETECTED");
        delay(1500);
        triageState = GET_TEMP;
      }
      break;
    }

    case GET_TEMP: {
      int raw = analogRead(potPin);
      currentTemp = 35.0 + (raw / 4095.0) * 6.0;

      lcd.setCursor(0, 0);
      lcd.print("DIAG 1: THERMAL ");
      lcd.setCursor(0, 1);
      lcd.print("TMP:"); lcd.print(currentTemp, 1); lcd.print("C [OK]");

      if (isBtnPressed(btnOK)) {
        lcd.clear();
        triageState = GET_RESP;
      }
      break;
    }

    case GET_RESP: {
      lcd.setCursor(0, 0);
      lcd.print("DIAG 2: BREATH  ");
      lcd.setCursor(0, 1);
      if (currentResp == 0) lcd.print("> NORMAL   [CHG]");
      else if (currentResp == 1) lcd.print("> LABOURED [CHG]");
      else lcd.print("> CRITICAL [CHG]");

      if (isBtnPressed(btnCycle)) {
        currentResp = (currentResp + 1) % 3;
      }
      if (isBtnPressed(btnOK)) {
        lcd.clear();
        triageState = GET_HYDRATION;
      }
      break;
    }

    case GET_HYDRATION: {
      lcd.setCursor(0, 0);
      lcd.print("DIAG 3: STATUS  ");
      lcd.setCursor(0, 1);
      if (currentHydration == 0) lcd.print("> NORMAL   [CHG]");
      else if (currentHydration == 1) lcd.print("> MILD/WEAK[CHG]");
      else lcd.print("> SEVERE   [CHG]");

      if (isBtnPressed(btnCycle)) {
        currentHydration = (currentHydration + 1) % 3;
      }
      if (isBtnPressed(btnOK)) {
        lcd.clear();
        triageState = GET_VULN;
      }
      break;
    }

    case GET_VULN: {
      lcd.setCursor(0, 0);
      lcd.print("DIAG 4: VULN    ");
      lcd.setCursor(0, 1);
      if (currentVuln == 0) lcd.print("> NONE     [CHG]");
      else if (currentVuln == 1) lcd.print("> PREGNANT [CHG]");
      else lcd.print("> PEDIATRIC[CHG]");

      if (isBtnPressed(btnCycle)) {
        currentVuln = (currentVuln + 1) % 3;
      }
      if (isBtnPressed(btnOK)) {
        lcd.clear();
        triageState = SHOW_RESULT;
      }
      break;
    }

    case SHOW_RESULT: {
      // Balanced Clinical Scoring Formula
      float tScore = (currentTemp - 35.0) / 6.0;
      if (tScore > 1.0) tScore = 1.0;
      
      float rScore = (currentResp == 0) ? 0.0 : (currentResp == 1 ? 0.5 : 1.0);
      float hScore = (currentHydration == 0) ? 0.0 : (currentHydration == 1 ? 0.5 : 1.0);
      float vScore = (currentVuln == 0) ? 0.0 : 0.4;
      
      float finalScore = (0.3 * tScore) + (0.3 * rScore) + (0.3 * hScore) + (0.1 * vScore);
      
      // Clinical Safety Tripwires / Overrides
      if (currentTemp >= 39.5 && finalScore < 0.65) {
        finalScore = 0.65; // Forces URGENT for high malaria fevers
      }
      if (currentResp == 2 || currentHydration == 2 || currentTemp >= 40.5) {
        finalScore = 0.95; // Immediate CRITICAL override for severe shock, critical breathing, or extreme fever
      }

      // Priority Mapping
      if (finalScore >= 0.75) { 
        prioLabel = "CRITICAL"; 
        prioLevel = 1; 
        digitalWrite(ledRed, HIGH); 
        digitalWrite(ledGreen, LOW);
      } else if (finalScore >= 0.40) { 
        prioLabel = "URGENT"; 
        prioLevel = 2; 
        digitalWrite(ledRed, HIGH); 
        digitalWrite(ledGreen, LOW);
      } else { 
        prioLabel = "ROUTINE"; 
        prioLevel = 3; 
        digitalWrite(ledRed, LOW); 
        digitalWrite(ledGreen, HIGH);
      }

      lcd.setCursor(0, 0);
      lcd.print("PRIO:"); lcd.print(prioLevel); lcd.print(" "); lcd.print(prioLabel);
      lcd.setCursor(0, 1);
      lcd.print("PRESS OK TO RST ");
      
      static bool processed = false;
      if (!processed) {
        totalTriages++;
        preferences.putInt("count", totalTriages);
        processed = true;
      }

      if (isBtnPressed(btnOK)) {
        currentResp = 0;
        currentHydration = 0;
        currentVuln = 0;
        processed = false;
        lcd.clear();
        currentState = MENU; 
      }
      break;
    }
  }
}

void runInfoEngine() {
  lcd.setCursor(0, 0);
  lcd.print("SYSTEM ENGINEERS");
  lcd.setCursor(0, 1);
  lcd.print(credits[infoIndex]);
  
  if (isBtnPressed(btnCycle)) {
    infoIndex = (infoIndex + 1) % totalCredits;
    lcd.clear();
  }
  if (isBtnPressed(btnOK)) {
    currentState = MENU;
    lcd.clear();
  }
}

// Ultrasonic Sensor Reader
long getDistance() {
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(5);         
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(15);        
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000); 
  if (duration == 0) return -1; 
  return duration * 0.034 / 2;
}