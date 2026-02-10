#include "NetworkManager.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// HTML Content (Updated Vitals Card)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head><title>K9 Monitor</title><meta name="viewport" content="width=device-width, initial-scale=1">
<style>body{font-family:Arial;text-align:center;background:#222;color:#fff}.card{background:#333;padding:20px;margin:10px auto;max-width:400px;border-radius:10px}
.value{font-size:2em;font-weight:bold}.status-alert{color:#f44336;animation:blink 1s infinite}@keyframes blink{50%{opacity:0}}
.btn{background:#00bcd4;color:white;padding:15px;border:none;border-radius:5px;width:100%}</style></head>
<body><h1>K9 Seizure Monitor</h1>
<div class="card">STATUS<div id="status" class="value">INIT...</div></div>
<div class="card">G-FORCE<div id="gforce" class="value">0.0</div></div>
<div class="card">VITALS<div id="vitals">-- BPM | -- % | -- C</div></div>
<div class="card"><button id="pauseBtn" class="btn" onclick="toggle()">PAUSE</button></div>
<script>
var ws = new WebSocket(`ws://${location.hostname}/ws`);
ws.onmessage = (e) => {
  var d = JSON.parse(e.data);
  document.getElementById('gforce').innerText = d.g.toFixed(2);
  
  // Format: BPM | SpO2 | Skin Temp
  var bpm = d.vHR ? d.hr : "--";
  var spo = d.vS ? d.sp : "--";
  var temp = d.ht > 0 ? d.ht.toFixed(1) + "C" : "--";
  document.getElementById('vitals').innerText = `${bpm} BPM | ${spo}% | ${temp}`;
  
  var s = document.getElementById('status'); var b = document.getElementById('pauseBtn');
  if(d.p){ s.innerText="PAUSED"; s.style.color="orange"; b.innerText="RESUME"; }
  else if(d.sz){ s.innerText="SEIZURE!"; s.className="value status-alert"; }
  else { s.innerText="MONITORING"; s.className="value"; s.style.color="#4CAF50"; b.innerText="PAUSE"; }
};
function toggle(){ fetch('/api/control?action=toggle', {method:'POST'}); }
</script></body></html>
)rawliteral";

SystemState* _globalStateRef = nullptr;

void initNetwork() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    Serial.print("Connecting WiFi");
    int retry = 0;
    while(WiFi.status() != WL_CONNECTED && retry < 15) { delay(500); Serial.print("."); retry++; }
    
    if(WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n>> Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\n>> WiFi Failed. Starting AP...");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_AP);
        WiFi.softAP(AP_SSID, NULL, 1);
        Serial.printf(">> AP Started. IP: %s\n", WiFi.softAPIP().toString().c_str());
    }

    server.addHandler(&ws);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){ req->send(200, "text/html", index_html); });
    
    server.on("/api/control", HTTP_POST, [](AsyncWebServerRequest *req){
        if(req->hasParam("action") && _globalStateRef) {
            String a = req->getParam("action")->value();
            if(a == "toggle") _globalStateRef->isPaused = !_globalStateRef->isPaused;
        }
        req->send(200, "text/plain", "OK");
    });
    
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    server.begin();
}

void updateWebClients(SystemState &state) {
    _globalStateRef = &state; 
    ws.cleanupClients();
    
    // Broadcast JSON including 'ht' (Health Temp)
    char buf[256];
    snprintf(buf, sizeof(buf), 
        "{\"p\":%d,\"sz\":%d,\"g\":%.2f,\"hr\":%ld,\"sp\":%ld,\"ht\":%.2f,\"vHR\":%d,\"vS\":%d}",
        state.isPaused, state.seizureDetected, state.gForce,
        (long)state.heartRate, (long)state.spo2, state.healthTemp,
        (int)state.validHR, (int)state.validSPO2
    );
    ws.textAll(buf);
}