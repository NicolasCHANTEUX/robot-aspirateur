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

// L'INTERFACE WEB (HTML + JavaScript embarqué)
// C'est ce que votre téléphone affichera !
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Robot Aspirateur - Carte Live</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; background-color: #222; color: white; margin: 0; padding: 20px;}
    canvas { background-color: #eee; border-radius: 10px; box-shadow: 0px 0px 15px rgba(0,0,0,0.5); width: 100%; max-width: 500px;}
    h2 { margin-bottom: 5px; }
  </style>
</head>
<body>
  <h2>Cartographie Live</h2>
  <p>Statut Wi-Fi: Connecté | WebSocket: <span id="ws_status" style="color:red">Déconnecté</span></p>
  
  <canvas id="mapCanvas" width="500" height="500"></canvas>

  <script>
    var canvas = document.getElementById("mapCanvas");
    var ctx = canvas.getContext("2d");
    
    // Connexion au WebSocket de l'ESP32
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket = new WebSocket(gateway);
    
    websocket.onopen = function(event) {
      document.getElementById("ws_status").innerText = "Connecté";
      document.getElementById("ws_status").style.color = "lime";
    };
    
    websocket.onmessage = function(event) {
      var data = JSON.parse(event.data);
      
      // 1. On dessine la zone nettoyée (chemin du robot) en blanc
      ctx.fillStyle = "rgba(255, 255, 255, 0.5)";
      ctx.fillRect(data.rx - 5, data.ry - 5, 10, 10);
      
      // 2. Si un obstacle est détecté (< 40cm), on le dessine en noir
      if (data.obs_dist > 0 && data.obs_dist < 40) {
        // Calcul trigonométrique côté client pour placer l'obstacle
        let ox = data.rx + data.obs_dist * Math.cos(data.ra);
        let oy = data.ry + data.obs_dist * Math.sin(data.ra);
        ctx.fillStyle = "black";
        ctx.fillRect(ox - 2.5, oy - 2.5, 5, 5);
      }

      // 3. On dessine le robot par-dessus tout le reste en rouge
      // (Optionnel: Effacer l'ancien robot nécessite une logique de rafraîchissement plus complexe, 
      // ici il laissera une "traînée" visuelle pour voir son parcours complet !)
      ctx.fillStyle = "red";
      ctx.beginPath();
      ctx.arc(data.rx, data.ry, 4, 0, 2 * Math.PI);
      ctx.fill();
    };
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
