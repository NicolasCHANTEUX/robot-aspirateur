#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "communication.h"
#include "config.h"
#include "debug.h"

// Création du serveur sur le port 80 et du WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// L'INTERFACE WEB (HTML + JavaScript embarqué) - Version Premium
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Robot Aspirateur Pro</title>
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #0f0c29, #302b63, #24243e);
      color: white; overflow: hidden; height: 100vh;
    }
    
    #mapCanvas {
      position: absolute; top: 0; left: 0; width: 100%; height: 100%;
      background-color: #1a1a1a; cursor: grab;
    }
    
    /* Tableau de bord en verre dépoli (Glassmorphism) */
    .dashboard {
      position: absolute; bottom: 20px; left: 50%; transform: translateX(-50%);
      width: 90%; max-width: 400px; padding: 20px;
      background: rgba(255, 255, 255, 0.1);
      backdrop-filter: blur(10px); -webkit-backdrop-filter: blur(10px);
      border-radius: 20px; border: 1px solid rgba(255, 255, 255, 0.2);
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5);
      display: flex; flex-direction: column; gap: 15px;
    }
    
    .status { font-size: 14px; opacity: 0.9; }
    .ws-badge { 
      display: inline-block; padding: 4px 10px; border-radius: 12px;
      font-size: 11px; font-weight: bold;
      background: rgba(255, 0, 0, 0.3); color: #ff4444;
    }
    .ws-badge.connected { background: rgba(0, 255, 0, 0.3); color: #00ff88; }
    
    /* Barre de batterie */
    .battery-bar {
      width: 100%; height: 30px; background: rgba(0, 0, 0, 0.3);
      border-radius: 10px; position: relative; overflow: hidden;
    }
    .battery-fill {
      height: 100%; background: linear-gradient(90deg, #00f2fe, #4facfe);
      width: 75%; transition: width 0.5s, background 0.5s;
      box-shadow: 0 0 15px rgba(0, 242, 254, 0.5);
    }
    .battery-fill.low { background: linear-gradient(90deg, #ff6b6b, #ee5a6f); }
    
    /* Boutons modernes */
    .btn-group { display: flex; gap: 10px; }
    .btn {
      flex: 1; border: none; border-radius: 10px; padding: 15px;
      font-size: 16px; font-weight: bold; cursor: pointer;
      transition: transform 0.2s, box-shadow 0.2s;
    }
    .btn:active { transform: scale(0.95); }
    .btn-start {
      background: linear-gradient(135deg, #00f2fe, #4facfe);
      color: #000; box-shadow: 0 4px 15px rgba(0, 242, 254, 0.4);
    }
    .btn-stop {
      background: linear-gradient(135deg, #ff6b6b, #ee5a6f);
      color: white; box-shadow: 0 4px 15px rgba(255, 107, 107, 0.4);
    }
    
    /* Info du haut */
    .top-info {
      position: absolute; top: 20px; left: 50%; transform: translateX(-50%);
      padding: 10px 20px; background: rgba(0, 0, 0, 0.5);
      backdrop-filter: blur(10px); border-radius: 15px;
      font-size: 18px; font-weight: bold; letter-spacing: 1px;
    }
  </style>
</head>
<body>
  <div class="top-info">🤖 ROBOT ASPIRATEUR</div>
  <canvas id="mapCanvas"></canvas>
  
  <div class="dashboard">
    <div class="status">
      WebSocket: <span class="ws-badge" id="ws_status">Déconnecté</span>
    </div>
    
    <div>
      <div style="font-size: 12px; opacity: 0.7; margin-bottom: 5px;">Batterie</div>
      <div class="battery-bar">
        <div class="battery-fill" id="battery"></div>
      </div>
    </div>
    
    <div class="btn-group">
      <button class="btn btn-start" onclick="sendCommand('start')">▶ Démarrer</button>
      <button class="btn btn-stop" onclick="sendCommand('stop')">⏹ Arrêt</button>
    </div>
  </div>

  <script>
    const canvas = document.getElementById("mapCanvas");
    const ctx = canvas.getContext("2d");
    
    // Adapter le canvas à la taille de l'écran
    function resizeCanvas() {
      canvas.width = window.innerWidth;
      canvas.height = window.innerHeight;
    }
    resizeCanvas();
    window.addEventListener('resize', resizeCanvas);
    
    // Variables de zoom/pan
    let offsetX = canvas.width / 2;
    let offsetY = canvas.height / 2;
    let scale = 1.5;
    let isDragging = false;
    let lastX, lastY;
    
    // Données du robot
    let robotData = { rx: 250, ry: 250, ra: 0, obs_dist: 100 };
    let path = []; // Historique du chemin
    let obstacles = []; // Liste des obstacles détectés
    
    // WebSocket
    const gateway = `ws://${window.location.hostname}/ws`;
    const websocket = new WebSocket(gateway);
    
    websocket.onopen = function() {
      document.getElementById("ws_status").innerText = "Connecté";
      document.getElementById("ws_status").classList.add("connected");
    };
    
    websocket.onmessage = function(event) {
      const data = JSON.parse(event.data);
      robotData = data;
      path.push({ x: data.rx, y: data.ry });
      if (path.length > 500) path.shift();
      
      if (data.obs_dist > 0 && data.obs_dist < 40) {
        let ox = data.rx + data.obs_dist * Math.cos(data.ra);
        let oy = data.ry + data.obs_dist * Math.sin(data.ra);
        obstacles.push({ x: ox, y: oy });
        if (obstacles.length > 300) obstacles.shift();
      }
    };
    
    function sendCommand(cmd) {
      websocket.send(JSON.stringify({ command: cmd }));
    }
    
    // Gestion souris (PC)
    canvas.addEventListener('mousedown', (e) => {
      isDragging = true; lastX = e.clientX; lastY = e.clientY;
      canvas.style.cursor = 'grabbing';
    });
    canvas.addEventListener('mousemove', (e) => {
      if (isDragging) {
        offsetX += e.clientX - lastX;
        offsetY += e.clientY - lastY;
        lastX = e.clientX; lastY = e.clientY;
      }
    });
    canvas.addEventListener('mouseup', () => {
      isDragging = false; canvas.style.cursor = 'grab';
    });
    canvas.addEventListener('wheel', (e) => {
      e.preventDefault();
      scale *= e.deltaY < 0 ? 1.1 : 0.9;
      scale = Math.max(0.5, Math.min(scale, 5));
    });
    
    // Gestion tactile (mobile)
    let touchDist = 0;
    canvas.addEventListener('touchstart', (e) => {
      if (e.touches.length === 1) {
        lastX = e.touches[0].clientX; lastY = e.touches[0].clientY;
        isDragging = true;
      } else if (e.touches.length === 2) {
        isDragging = false;
        const dx = e.touches[0].clientX - e.touches[1].clientX;
        const dy = e.touches[0].clientY - e.touches[1].clientY;
        touchDist = Math.sqrt(dx * dx + dy * dy);
      }
    });
    canvas.addEventListener('touchmove', (e) => {
      e.preventDefault();
      if (e.touches.length === 1 && isDragging) {
        offsetX += e.touches[0].clientX - lastX;
        offsetY += e.touches[0].clientY - lastY;
        lastX = e.touches[0].clientX; lastY = e.touches[0].clientY;
      } else if (e.touches.length === 2) {
        const dx = e.touches[0].clientX - e.touches[1].clientX;
        const dy = e.touches[0].clientY - e.touches[1].clientY;
        const newDist = Math.sqrt(dx * dx + dy * dy);
        scale *= newDist / touchDist;
        scale = Math.max(0.5, Math.min(scale, 5));
        touchDist = newDist;
      }
    });
    canvas.addEventListener('touchend', () => { isDragging = false; });
    
    // Fonction de dessin
    function draw() {
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      ctx.save();
      ctx.translate(offsetX, offsetY);
      ctx.scale(scale, scale);
      
      // Grille de fond
      ctx.strokeStyle = "rgba(100, 100, 100, 0.2)";
      ctx.lineWidth = 1 / scale;
      for (let i = -500; i < 500; i += 50) {
        ctx.beginPath(); ctx.moveTo(i, -500); ctx.lineTo(i, 500); ctx.stroke();
        ctx.beginPath(); ctx.moveTo(-500, i); ctx.lineTo(500, i); ctx.stroke();
      }
      
      // Chemin nettoyé (traînée cyan)
      ctx.strokeStyle = "rgba(0, 242, 254, 0.3)";
      ctx.lineWidth = 8 / scale;
      ctx.beginPath();
      for (let i = 0; i < path.length; i++) {
        if (i === 0) ctx.moveTo(path[i].x, path[i].y);
        else ctx.lineTo(path[i].x, path[i].y);
      }
      ctx.stroke();
      
      // Obstacles
      ctx.fillStyle = "rgba(255, 255, 255, 0.8)";
      obstacles.forEach(obs => {
        ctx.fillRect(obs.x - 3, obs.y - 3, 6, 6);
      });
      
      // Robot avec halo lumineux
      ctx.shadowBlur = 20;
      ctx.shadowColor = "#00f2fe";
      ctx.fillStyle = "#00f2fe";
      ctx.beginPath();
      ctx.arc(robotData.rx, robotData.ry, 8, 0, Math.PI * 2);
      ctx.fill();
      
      // Cône de vision (direction)
      ctx.shadowBlur = 0;
      ctx.beginPath();
      ctx.moveTo(robotData.rx, robotData.ry);
      ctx.lineTo(
        robotData.rx + Math.cos(robotData.ra - 0.5) * 25,
        robotData.ry + Math.sin(robotData.ra - 0.5) * 25
      );
      ctx.lineTo(
        robotData.rx + Math.cos(robotData.ra + 0.5) * 25,
        robotData.ry + Math.sin(robotData.ra + 0.5) * 25
      );
      ctx.fillStyle = "rgba(0, 242, 254, 0.5)";
      ctx.fill();
      
      ctx.restore();
      requestAnimationFrame(draw);
    }
    draw();
  </script>
</body>
</html>
)rawliteral";

void communicationInit() {
  debugLog("[WIFI] Connexion à " + String(WIFI_SSID));
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Attente de la connexion
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  debugLog("\n[WIFI] Connecté ! Adresse IP : " + WiFi.localIP().toString());

  // Quand on tape l'adresse IP, envoyer la page HTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Démarrer le WebSocket et le Serveur
  server.addHandler(&ws);
  server.begin();
  debugLog("[WEBSOCKET] Serveur démarré");
}

// Fonction appelée par tacheCartographie pour diffuser les données
void communicationEnvoyerMiseAJour(PositionRobot pos, float distanceObstacle) {
  // S'il y a au moins un téléphone connecté à la page
  if (ws.count() > 0) {
    // On prépare un message JSON formaté
    StaticJsonDocument<128> doc;
    doc["rx"] = pos.x;        // Robot X
    doc["ry"] = pos.y;        // Robot Y
    doc["ra"] = pos.angle;    // Robot Angle (Radians)
    doc["obs_dist"] = distanceObstacle; // Distance ultrasons
    
    String payload;
    serializeJson(doc, payload);
    
    // On l'envoie à tous les clients connectés !
    ws.textAll(payload);
  }
}

// Fonction pour nettoyer les clients déconnectés (économise la RAM)
void communicationCleanupClients() {
  ws.cleanupClients();
}
