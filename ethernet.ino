#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

const uint8_t pins[8] = { D0, D1, D2, D3, D4, D5, D6, D7 };

bool isRunning = true;
bool manualMode = false;
bool continuityMode = false; 

int scanSpeed = 500;
int currentStep = 0;

unsigned long lastStepTime = 0; 
unsigned long lastMicros = 0;   

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
ESP8266WebServer server(80);

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <title>TESTER</title>
  <style>
    * {
      box-sizing: border-box;
      -webkit-user-select: none;
      -moz-user-select: none;
      -ms-user-select: none;
      user-select: none;
      -webkit-tap-highlight-color: transparent;
    }

    body { 
      background-color: #121212; 
      color: #e0e0e0; 
      font-family: 'Segoe UI', sans-serif; 
      margin: 0; 
      height: 100vh; 
      display: flex;
      align-items: center; 
      justify-content: center; 
      overflow: hidden; 
    }
    
    .container { 
      width: 320px; 
      background: #1e1e1e; 
      border-radius: 16px; 
      padding: 25px; 
      border: 1px solid #333; 
      display: flex; 
      flex-direction: column; 
      align-items: center;
      box-shadow: 0 10px 40px rgba(0,0,0,0.6);
      position: relative; 
    }

    h2 { color: #03dac6; margin: 0 0 20px 0; font-size: 1.1rem; letter-spacing: 3px; font-weight: 600; cursor: default; }

    .top-controls { width: 100%; z-index: 10; }
    select { 
      width: 100%; padding: 12px; background: #2c2c2c; color: white; 
      border: 1px solid #444; border-radius: 6px; font-size: 16px; outline: none; cursor: pointer;
      appearance: none; text-align: center;
      transition: border-color 0.3s;
    }
    select:focus { border-color: #03dac6; }

    .port-housing {
      background: #000; width: 70px; padding: 12px 6px; 
      border-radius: 6px; border: 2px solid #333; 
      display: flex; flex-direction: column; gap: 4px;
      margin-top: 30px;
      margin-bottom: 30px; 
      box-shadow: inset 0 0 15px #111;
      transition: transform 0.6s cubic-bezier(0.22, 1, 0.36, 1);
      z-index: 5;
    }

    .move-down { transform: translateY(45px); }

    .pin-row { display: flex; width: 100%; }
    .left  { justify-content: flex-start; } 
    .right { justify-content: flex-end;   } 

    .led {
      width: 26px; height: 24px; background: #222; 
      border: 1px solid #333; border-radius: 3px; font-size: 11px; 
      display: flex; align-items: center; justify-content: center; 
      color: #555; font-weight: bold;
    }

    .active .led.odd  { background-color: #00ff00; box-shadow: 0 0 12px #00ff00; color: #000; border-color: #fff; }
    .active .led.even { background-color: #ffcc00; box-shadow: 0 0 12px #ffcc00; color: #000; border-color: #fff; }

    .controls-area { 
      width: 100%; display: flex; justify-content: center; min-height: 150px;
      transition: opacity 0.3s;
    }

    .slider-wrapper { 
      display: flex; 
      flex-direction: column;
      align-items: center; 
      justify-content: center;
      width: 100%;
      gap: 20px;
    }

    input[type=range] {
      -webkit-appearance: none;
      width: 90%; 
      height: 8px; 
      background: #333; 
      border-radius: 5px;
      cursor: pointer; 
      margin: 0; 
      accent-color: #03dac6;
    }

    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      height: 24px;
      width: 24px;
      border-radius: 50%;
      background: #03dac6;
      cursor: pointer;
      margin-top: -8px; 
      box-shadow: 0 0 10px rgba(3, 218, 198, 0.5);
      transition: transform 0.1s;
    }
    
    input[type=range]:active::-webkit-slider-thumb {
      transform: scale(1.2);
    }
    
    input[type=range]::-webkit-slider-runnable-track {
      width: 100%; height: 8px; cursor: pointer; background: #333; border-radius: 5px;
    }

    .speed-info { 
      display: flex; flex-direction: column; align-items: center; 
      color: #888; font-family: monospace; letter-spacing: 1px;
    }
    .speed-val { font-size: 1.4em; color: #fff; font-weight: bold; margin-bottom: 2px;}

    .manual-wrapper { 
      width: 100%; display: none; flex-direction: row; 
      justify-content: space-between; gap: 15px; align-items: center;
    }
    
    .btn { 
      flex: 1; padding: 20px 0; border: none; border-radius: 8px; 
      background: #03dac6; font-weight: bold; font-size: 16px; cursor: pointer; color: #000;
      transition: transform 0.2s ease, opacity 0.2s;
    }
    .btn:active { opacity: 0.8; transform: scale(0.94); }

    .status-msg { 
        display: none; color: #03dac6; font-size: 0.9em; 
        font-weight: bold; position: absolute; bottom: 50px; 
        text-transform: uppercase; letter-spacing: 2px;
        animation: fadeIn 0.5s ease;
    }

    @keyframes fadeIn { from { opacity: 0; transform: translateY(5px); } to { opacity: 1; transform: translateY(0); } }

  </style>
</head>
<body>

<div class="container">
  <h2>RJ45 MASTER</h2>

  <div class="top-controls">
    <select id="modeSel" onchange="setMode()">
      <option value="auto">Auto Scan</option>
      <option value="manual">Manual Step</option>
      <option value="continuity">Continuity Check</option>
      <option value="off">Off</option>
    </select>
  </div>

  <div class="port-housing" id="ledRack">
    <div class="pin-row right" id="r0"><div class="led odd">1</div></div>
    <div class="pin-row left"  id="r1"><div class="led even">2</div></div>
    <div class="pin-row right" id="r2"><div class="led odd">3</div></div>
    <div class="pin-row left"  id="r3"><div class="led even">4</div></div>
    <div class="pin-row right" id="r4"><div class="led odd">5</div></div>
    <div class="pin-row left"  id="r5"><div class="led even">6</div></div>
    <div class="pin-row right" id="r6"><div class="led odd">7</div></div>
    <div class="pin-row left"  id="r7"><div class="led even">8</div></div>
  </div>

  <div class="controls-area" id="ctrlArea">
    
    <div id="auto-ui" class="slider-wrapper">
      <input type="range" min="50" max="2000" step="50" value="500" 
             id="slideSpeed" 
             oninput="uiUpdateLabel(this.value)" 
             onchange="netUpdateSpeed(this.value)">
      <div class="speed-info">
        <span class="speed-val" id="lblSpeed">--</span>
        <span>MS</span>
      </div>
    </div>

    <div id="manual-ui" class="manual-wrapper">
      <button class="btn" onclick="step(-1)">PREV</button>
      <button class="btn" onclick="step(1)">NEXT</button>
    </div>

  </div>
  
  <div id="status-ui" class="status-msg"></div>

</div>

<script>
  let clientStep = 0;
  let clientSpeed = 500;
  let clientMode = "auto";
  let intervalId = null;

  document.body.addEventListener('touchmove', function(e) { 
    if (e.target.type !== 'range') e.preventDefault(); 
  }, { passive: false });

  function render() {
    for(let i=0; i<8; i++) {
      let el = document.getElementById("r" + i);
      
      if (clientMode === "continuity") {
        el.classList.add("active");
      } else if (clientMode !== "off" && i === clientStep) {
        el.classList.add("active");
      } else {
        el.classList.remove("active");
      }
    }
  }

  function startIntervalLoop() {
    if (intervalId) clearInterval(intervalId);
    if (clientMode === "auto") {
      intervalId = setInterval(() => {
        clientStep = (clientStep + 1) % 8;
        render();
      }, clientSpeed);
    }
  }

  function phaseLockSync() {
    const tStart = Date.now();
    fetch('/status')
      .then(r => r.json())
      .then(d => {
        const tEnd = Date.now();
        const latency = (tEnd - tStart) / 2;

        if (document.activeElement.id !== "slideSpeed") {
             clientSpeed = d.speed;
             document.getElementById("slideSpeed").value = clientSpeed;
             document.getElementById("lblSpeed").innerText = clientSpeed;
        }

        clientStep = d.step;
        
        if (d.running) clientMode = "auto";
        else if (d.manual) clientMode = "manual";
        else if (d.continuity) clientMode = "continuity";
        else clientMode = "off";

        document.getElementById("modeSel").value = clientMode;
        updateVisibility();
        render();

        if (clientMode === "auto") {
          const trueElapsed = d.elapsed + latency;
          let timeToNext = clientSpeed - trueElapsed;
          if (timeToNext < 0) timeToNext = 0;
          if (intervalId) clearInterval(intervalId);
          setTimeout(() => {
            clientStep = (clientStep + 1) % 8;
            render();
            startIntervalLoop(); 
          }, timeToNext);
        }
      });
  }

  function uiUpdateLabel(val) { document.getElementById("lblSpeed").innerText = val; }
  function netUpdateSpeed(val) { fetch("/speed?v=" + val).then(() => phaseLockSync()); }
  function setMode() { 
    clientMode = document.getElementById("modeSel").value; 
    updateVisibility(); 
    fetch("/toggle?m=" + clientMode).then(() => phaseLockSync()); 
  }

  function updateVisibility() {
    const s = document.getElementById("auto-ui");
    const m = document.getElementById("manual-ui");
    const st = document.getElementById("status-ui");
    const rack = document.getElementById("ledRack");
    
    s.style.display = "none";
    m.style.display = "none";
    st.style.display = "none";
    
    rack.classList.remove('move-down');

    if (clientMode === "auto") {
      s.style.display = "flex";
    } 
    else if (clientMode === "manual") {
      m.style.display = "flex";
      if (intervalId) clearInterval(intervalId);
    } 
    else if (clientMode === "continuity") {
      st.innerText = "ALL PINS ACTIVE";
      st.style.display = "block";
      rack.classList.add('move-down');
      if (intervalId) clearInterval(intervalId);
    }
    else if (clientMode === "off") {
      st.innerText = "ALL PINS OFF";
      st.style.display = "block";
      rack.classList.add('move-down');
      if (intervalId) clearInterval(intervalId);
    }
    render(); 
  }

  function step(dir) {
    clientStep += dir;
    if (clientStep > 7) clientStep = 0;
    if (clientStep < 0) clientStep = 7;
    render();
    fetch("/step?d=" + dir);
  }

  phaseLockSync(); 
</script>
</body>
</html>
)rawliteral";

void allHiZ() {
  for(int i=0; i<8; i++) pinMode(pins[i], INPUT);
}

void setPins(int s) {
  allHiZ();
  int high = s;
  int low = (s + 1) % 8;
  pinMode(pins[high], OUTPUT); digitalWrite(pins[high], HIGH);
  pinMode(pins[low], OUTPUT); digitalWrite(pins[low], LOW);
}

void handleStatus() {
  unsigned long now = millis();
  unsigned long elapsed = now - lastStepTime; 
  
  String json = "{";
  json += "\"speed\":" + String(scanSpeed) + ",";
  json += "\"step\":" + String(currentStep) + ",";
  json += "\"elapsed\":" + String(elapsed) + ",";
  json += "\"running\":" + String(isRunning ? "true" : "false") + ",";
  json += "\"manual\":" + String(manualMode ? "true" : "false") + ",";
  json += "\"continuity\":" + String(continuityMode ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handleToggle() {
  String m = server.arg("m");
  isRunning = false; manualMode = false; continuityMode = false;

  if (m == "auto") { 
    isRunning = true; lastStepTime = millis(); 
  } else if (m == "manual") { 
    manualMode = true; setPins(currentStep); 
  } else if (m == "continuity") {
    continuityMode = true;
  } else { 
    allHiZ(); 
  }
  server.send(200, "text/plain", "OK");
}

void handleSpeed() {
  if (server.hasArg("v")) {
    scanSpeed = server.arg("v").toInt();
    lastStepTime = millis();
  }
  server.send(200, "text/plain", "OK");
}

void handleStep() {
  if (manualMode && server.hasArg("d")) {
    int d = server.arg("d").toInt();
    if (d > 0) currentStep++; else currentStep--;
    if (currentStep > 7) currentStep = 0;
    if (currentStep < 0) currentStep = 7;
    setPins(currentStep);
  }
  server.send(200, "text/plain", "OK");
}

void handleNotFound() {
  if (server.hostHeader() != "192.168.4.1") {
    server.sendHeader("Location", "http://192.168.4.1", true);
    server.send(302, "text/plain", "");
  } else {
    server.send(404, "text/plain", "Not found"); 
  }
}

void setup() {
  allHiZ();
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255,255,255,0));
  WiFi.softAP("Tester Ethernet", "ethernet");
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", [](){ server.send(200, "text/html", INDEX_HTML); });
  server.on("/status", handleStatus);
  server.on("/toggle", handleToggle);
  server.on("/speed", handleSpeed);
  server.on("/step", handleStep);
  server.onNotFound(handleNotFound);
  server.begin();

  currentStep = 0;
  setPins(currentStep);
  lastStepTime = millis(); 
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (isRunning && !manualMode && !continuityMode) {
    unsigned long now = millis();
    if (now - lastStepTime >= scanSpeed) {
      lastStepTime = now;
      currentStep++;
      if (currentStep > 7) currentStep = 0;
      setPins(currentStep);
    }
  }

  if (continuityMode) {
    unsigned long now = micros();
    if (now - lastMicros >= 400) { 
      lastMicros = now;
      currentStep++;
      if (currentStep > 7) currentStep = 0;
      setPins(currentStep);
    }
  }
}
