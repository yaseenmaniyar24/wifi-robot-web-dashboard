/*
  Wi-Fi Robot with Web Dashboard, Telemetry and Safe-Stop
  Board: ESP32 Dev Module
  Driver: TB6612FNG
  Drive: 4 DC motors, paired left and right (only if driver current limits allow)

  IMPORTANT:
  - Do NOT power 3-6V yellow TT motors directly from a rectangular 9V PP3 battery.
  - Laptop USB powers ESP32; separate regulated motor supply powers TB6612FNG VM.
  - ESP32 GND, driver GND and motor-supply negative must be common.
  - Verify combined stall current of each pair of motors before connecting two motors
    to one TB6612FNG channel. Use a second driver / higher-current driver if necessary.
  - The software E-stop is not a substitute for a physical motor-power disconnect.
*/

#include <WiFi.h>
#include <WebServer.h>

const char* AP_SSID = "YASEEN_ROBOT";
const char* AP_PASSWORD = "robot123"; // Change before public/demo use.

WebServer server(80);

// TB6612FNG -> ESP32 GPIO mapping
constexpr uint8_t PWMA = 25;
constexpr uint8_t AIN1 = 27;
constexpr uint8_t AIN2 = 14;
constexpr uint8_t PWMB = 26;
constexpr uint8_t BIN1 = 32;
constexpr uint8_t BIN2 = 33;
constexpr uint8_t STBY = 23;

// HC-SR04 -> ESP32 (ECHO must pass through a voltage divider/level shifter)
constexpr uint8_t TRIG_PIN = 5;
constexpr uint8_t ECHO_PIN = 18;

constexpr uint8_t PWM_FREQ = 1000;
constexpr uint8_t PWM_BITS = 8;
constexpr uint32_t COMMAND_TIMEOUT_MS = 700;
constexpr uint32_t SENSOR_INTERVAL_MS = 100;
constexpr int SAFE_DISTANCE_CM = 20;

int motorSpeed = 150;
int distanceCm = -1;
char directionNow = 'S';
bool emergencyLatched = false;
bool obstacleBlocked = false;
uint32_t lastValidMotionCommand = 0;
uint32_t lastSensorRead = 0;

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html><html lang="en"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Yaseen Robot Control</title>
<style>
:root{color-scheme:dark}*{box-sizing:border-box}
body{margin:0;padding:18px;background:#0d1728;color:#edf4ff;font:16px Arial,sans-serif}
main{max-width:760px;margin:auto}h1{margin:8px 0 4px}p{color:#aebdd2}
.card{background:#17263c;border:1px solid #2a405e;border-radius:16px;padding:18px;margin:14px 0}
.pad{display:grid;grid-template-columns:repeat(3, minmax(70px,1fr));gap:10px;max-width:360px;margin:12px auto}
button{border:0;border-radius:12px;padding:17px 10px;font-size:17px;font-weight:bold;color:white;background:#2563eb;touch-action:none}
.stop{background:#b91c1c}.reset{background:#087f5b}.wide{width:100%;margin:6px 0}
input{width:100%}.stats{display:grid;grid-template-columns:repeat(2,1fr);gap:10px}
.stat{background:#0e1b2d;padding:14px;border-radius:12px}.value{font-size:24px;font-weight:bold}
.small{font-size:13px}.status{font-weight:bold}
</style></head><body><main>
<h1>🤖 Yaseen Wi-Fi Robot</h1><p>4WD control · telemetry · safe-stop</p>
<section class="card"><h2>Movement</h2><p>Hold a direction to move. Release to stop.</p>
<div class="pad">
<div></div><button data-dir="F">▲ Forward</button><div></div>
<button data-dir="L">◀ Left</button><button class="stop" id="stop">STOP</button><button data-dir="R">Right ▶</button>
<div></div><button data-dir="B">▼ Reverse</button><div></div>
</div>
<label for="speed">Speed: <b id="speedVal">150</b>/255</label>
<input id="speed" type="range" min="0" max="255" value="150">
</section>
<section class="card"><h2>Live telemetry</h2><div class="stats">
<div class="stat">Distance<div class="value" id="distance">-- cm</div></div>
<div class="stat">Direction<div class="value" id="direction">STOP</div></div>
<div class="stat">Obstacle<div class="value" id="obstacle">--</div></div>
<div class="stat">Emergency stop<div class="value" id="emergency">--</div></div>
</div><p id="connection" class="status">Connecting…</p></section>
<section class="card"><h2>Safety</h2>
<button class="stop wide" id="estop">EMERGENCY STOP</button>
<button class="reset wide" id="reset">Reset E-stop</button>
<p class="small">Reset only clears the software latch. Check that the robot is clear before moving.</p>
</section></main>
<script>
let held=false, activeDir='S', repeatTimer=null;
const $=id=>document.getElementById(id);
async function command(d){
  const s=$('speed').value;
  try{const r=await fetch(`/cmd?d=${d}&s=${s}`,{cache:'no-store'}); if(!r.ok) throw Error(await r.text());}
  catch(e){$('connection').textContent='Command/network error — robot should timeout-stop'; held=false; clearInterval(repeatTimer);}
}
function begin(d){held=true;activeDir=d;command(d);clearInterval(repeatTimer);repeatTimer=setInterval(()=>{if(held)command(activeDir)},200);}
function release(){if(!held)return;held=false;clearInterval(repeatTimer);command('S');}
document.querySelectorAll('[data-dir]').forEach(b=>{
  b.addEventListener('pointerdown',e=>{e.preventDefault();begin(b.dataset.dir)});
  ['pointerup','pointerleave','pointercancel','lostpointercapture'].forEach(ev=>b.addEventListener(ev,release));
});
$('stop').addEventListener('click',()=>{held=false;clearInterval(repeatTimer);command('S')});
$('speed').addEventListener('input',()=>{$('speedVal').textContent=$('speed').value});
$('estop').addEventListener('click',async()=>{held=false;clearInterval(repeatTimer);try{await fetch('/estop')}catch(e){}});
$('reset').addEventListener('click',async()=>{try{await fetch('/reset')}catch(e){}});
async function poll(){try{
 const r=await fetch('/data',{cache:'no-store'});if(!r.ok)throw Error();const d=await r.json();
 $('distance').textContent=(d.distance<0?'No echo':d.distance+' cm');
 $('direction').textContent=d.direction;$('obstacle').textContent=d.obstacle?'BLOCKED':'Clear';
 $('emergency').textContent=d.emergency?'ACTIVE':'OFF';$('connection').textContent='Connected to robot AP';
}catch(e){$('connection').textContent='Disconnected';held=false;clearInterval(repeatTimer);}}
setInterval(poll,300);poll();
</script></body></html>
)HTML";

void stopMotors() {
  ledcWrite(PWMA, 0);
  ledcWrite(PWMB, 0);
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
  digitalWrite(STBY, LOW); // standby when stopped
  directionNow = 'S';
}

void setDirection(char d) {
  if (emergencyLatched || obstacleBlocked) { stopMotors(); return; }
  digitalWrite(STBY, HIGH);

  switch (d) {
    case 'F':
      digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); break;
    case 'B':
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
      digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); break;
    case 'L':
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); break;
    case 'R':
      digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); break;
    default: stopMotors(); return;
  }
  ledcWrite(PWMA, motorSpeed);
  ledcWrite(PWMB, motorSpeed);
  directionNow = d;
}

int readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000UL);
  if (duration == 0) return -1;
  return static_cast<int>(duration * 0.0343 / 2.0);
}

void handleCommand() {
  if (!server.hasArg("d")) { server.send(400, "text/plain", "Missing direction"); return; }
  char d = server.arg("d")[0];
  if (d!='F' && d!='B' && d!='L' && d!='R' && d!='S') {
    server.send(400, "text/plain", "Invalid direction"); return;
  }
  if (server.hasArg("s")) motorSpeed = constrain(server.arg("s").toInt(), 0, 255);

  if (d == 'S') {
    stopMotors();
    lastValidMotionCommand = millis();
    server.send(200, "text/plain", "Stopped"); return;
  }
  if (emergencyLatched) { stopMotors(); server.send(423, "text/plain", "E-stop latched"); return; }
  if (d == 'F' && (distanceCm < 0 || distanceCm < SAFE_DISTANCE_CM)) {
    obstacleBlocked = true; stopMotors();
    server.send(423, "text/plain", "Obstacle or invalid distance reading"); return;
  }
  obstacleBlocked = false;
  lastValidMotionCommand = millis();
  setDirection(d);
  server.send(200, "text/plain", "OK");
}

void handleEmergency() {
  emergencyLatched = true;
  stopMotors();
  server.send(200, "text/plain", "Emergency stop latched");
}
void handleReset() {
  stopMotors();
  emergencyLatched = false;
  obstacleBlocked = false;
  lastValidMotionCommand = 0;
  server.send(200, "text/plain", "Reset; robot remains stopped");
}
void handleTelemetry() {
  String json = "{";
  json += "\"distance\":" + String(distanceCm) + ",";
  json += "\"speed\":" + String(motorSpeed) + ",";
  json += "\"direction\":\"" + String(directionNow) + "\",";
  json += "\"emergency\":" + String(emergencyLatched ? "true" : "false") + ",";
  json += "\"obstacle\":" + String(obstacleBlocked ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT); pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
  ledcAttach(PWMA, PWM_FREQ, PWM_BITS);
  ledcAttach(PWMB, PWM_FREQ, PWM_BITS);
  stopMotors(); // default safe state

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print("Robot dashboard: http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](){ server.send_P(200, "text/html", INDEX_HTML); });
  server.on("/cmd", HTTP_GET, handleCommand);
  server.on("/data", HTTP_GET, handleTelemetry);
  server.on("/estop", HTTP_GET, handleEmergency);
  server.on("/reset", HTTP_GET, handleReset);
  server.begin();
}

void loop() {
  server.handleClient();

  if (millis() - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = millis();
    distanceCm = readDistanceCm();
    if (directionNow == 'F' &&
        (distanceCm < 0 || distanceCm < SAFE_DISTANCE_CM)) {
      obstacleBlocked = true;
      stopMotors();
    }
  }

  if (directionNow != 'S' &&
      millis() - lastValidMotionCommand > COMMAND_TIMEOUT_MS) {
    stopMotors();
  }
}
