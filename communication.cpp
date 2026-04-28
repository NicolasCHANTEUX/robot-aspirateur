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

// ======================================================================
// État interne (accès depuis le callback WS et depuis le firmware)
// ======================================================================
namespace {
volatile ModeRobot        modeActuel        = ModeRobot::Veille;
volatile CommandeManuelle commandeManuelle  = CommandeManuelle::Stop;
volatile bool             aspirationManu    = false;

volatile bool flagSauvegarder = false;
char          nomSauvegarde[33] = {0};

volatile bool flagCharger = false;
char          nomChargement[33] = {0};

volatile bool flagListePieces = false;
} // namespace

// ======================================================================
// L'INTERFACE WEB – HTML + JavaScript embarqué (Version 4 Modes)
// ======================================================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Robot Aspirateur Pro</title>
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <style>
    *{margin:0;padding:0;box-sizing:border-box}
    body{font-family:'Segoe UI',Tahoma,Geneva,Verdana,sans-serif;background:linear-gradient(135deg,#0f0c29,#302b63,#24243e);color:white;overflow:hidden;height:100vh}
    #mapCanvas{position:absolute;top:0;left:0;width:100%;height:100%;background-color:#1a1a1a;cursor:grab}
    .dashboard{position:absolute;bottom:20px;left:50%;transform:translateX(-50%);width:90%;max-width:420px;padding:15px;background:rgba(255,255,255,0.1);backdrop-filter:blur(10px);-webkit-backdrop-filter:blur(10px);border-radius:20px;border:1px solid rgba(255,255,255,0.2);box-shadow:0 8px 32px rgba(0,0,0,0.5);display:flex;flex-direction:column;gap:12px}
    .status-row{display:flex;justify-content:space-between;align-items:center;font-size:13px}
    .ws-badge{display:inline-block;padding:3px 10px;border-radius:12px;font-size:11px;font-weight:bold;background:rgba(255,0,0,0.3);color:#ff4444}
    .ws-badge.connected{background:rgba(0,255,0,0.3);color:#00ff88}
    .mode-badge{display:inline-block;padding:3px 10px;border-radius:12px;font-size:11px;font-weight:bold;background:rgba(150,150,150,0.3);color:#ccc}
    .bat-label{font-size:11px;opacity:.7;margin-bottom:4px}
    .battery-bar{width:100%;height:24px;background:rgba(0,0,0,0.3);border-radius:8px;overflow:hidden}
    .battery-fill{height:100%;background:linear-gradient(90deg,#00f2fe,#4facfe);width:75%;transition:width .5s,background .5s;box-shadow:0 0 10px rgba(0,242,254,.4)}
    .battery-fill.low{background:linear-gradient(90deg,#ff6b6b,#ee5a6f)}
    .mode-btns{display:flex;gap:6px}
    .btn-mode{flex:1;border:2px solid rgba(255,255,255,0.2);border-radius:10px;padding:8px 4px;font-size:11px;font-weight:bold;cursor:pointer;background:rgba(255,255,255,0.05);color:#ccc;transition:all .2s}
    .btn-mode:active{transform:scale(.95)}
    .btn-mode.av{background:rgba(150,150,150,.4);border-color:#aaa;color:white}
    .btn-mode.am{background:rgba(0,242,254,.3);border-color:#00f2fe;color:#00f2fe}
    .btn-mode.ad{background:rgba(0,255,100,.3);border-color:#00ff64;color:#00ff64}
    .btn-mode.an{background:rgba(180,0,255,.3);border-color:#b400ff;color:#b400ff}
    .panel{display:none;flex-direction:column;gap:10px}
    .panel.visible{display:flex}
    .dpad{display:grid;grid-template-columns:repeat(3,60px);grid-template-rows:repeat(3,60px);gap:5px;margin:0 auto}
    .dpad-btn{border:none;border-radius:10px;font-size:22px;cursor:pointer;background:rgba(255,255,255,0.15);color:white;transition:background .1s;display:flex;align-items:center;justify-content:center;user-select:none;-webkit-user-select:none}
    .dpad-btn:active,.dpad-btn.pressed{background:rgba(0,242,254,.5)}
    .dpad-empty{background:transparent!important;border:none!important;cursor:default}
    .btn-asp{width:100%;border:none;border-radius:10px;padding:10px;font-size:14px;font-weight:bold;cursor:pointer;background:rgba(255,255,255,.1);color:#ccc;transition:all .2s}
    .btn-asp.active{background:rgba(0,242,254,.3);color:#00f2fe}
    .btn-save{width:100%;border:none;border-radius:10px;padding:12px;font-size:14px;font-weight:bold;cursor:pointer;background:linear-gradient(135deg,#00ff64,#00b34f);color:#000;transition:transform .2s}
    .btn-save:active{transform:scale(.97)}
    .desc-text{font-size:12px;opacity:.8;text-align:center}
    .select-piece{width:100%;padding:10px;border-radius:10px;border:1px solid rgba(255,255,255,.2);background:rgba(255,255,255,.1);color:white;font-size:14px;cursor:pointer}
    .select-piece option{background:#302b63;color:white}
    .btn-nettoyer{width:100%;border:none;border-radius:10px;padding:12px;font-size:14px;font-weight:bold;cursor:pointer;background:linear-gradient(135deg,#b400ff,#7700aa);color:white;transition:transform .2s}
    .btn-nettoyer:active{transform:scale(.97)}
    .modal-overlay{display:none;position:fixed;inset:0;background:rgba(0,0,0,.7);backdrop-filter:blur(5px);z-index:100;align-items:center;justify-content:center}
    .modal-overlay.visible{display:flex}
    .modal-box{background:linear-gradient(135deg,#302b63,#24243e);border:1px solid rgba(255,255,255,.2);border-radius:20px;padding:25px;width:85%;max-width:320px;display:flex;flex-direction:column;gap:15px}
    .modal-box h3{font-size:18px;text-align:center}
    .modal-input{padding:12px;border-radius:10px;border:1px solid rgba(255,255,255,.3);background:rgba(255,255,255,.1);color:white;font-size:16px;width:100%}
    .modal-input::placeholder{color:rgba(255,255,255,.4)}
    .modal-btns{display:flex;gap:10px}
    .btn-mc{flex:1;padding:12px;border:1px solid rgba(255,255,255,.3);border-radius:10px;background:transparent;color:white;cursor:pointer;font-size:14px}
    .btn-mk{flex:1;padding:12px;border:none;border-radius:10px;background:linear-gradient(135deg,#00ff64,#00b34f);color:#000;font-weight:bold;cursor:pointer;font-size:14px}
    .top-info{position:absolute;top:15px;left:50%;transform:translateX(-50%);padding:8px 18px;background:rgba(0,0,0,.5);backdrop-filter:blur(10px);border-radius:15px;font-size:16px;font-weight:bold;letter-spacing:1px;white-space:nowrap}
    .toast{position:fixed;top:70px;left:50%;transform:translateX(-50%);padding:10px 20px;background:rgba(0,255,100,.3);backdrop-filter:blur(10px);border:1px solid #00ff64;border-radius:12px;font-size:13px;opacity:0;transition:opacity .3s;pointer-events:none;z-index:200;white-space:nowrap}
    .toast.show{opacity:1}
  </style>
</head>
<body>
  <div class="top-info">&#x1F916; ROBOT ASPIRATEUR</div>
  <canvas id="mapCanvas"></canvas>
  <div id="toast" class="toast"></div>

  <div class="dashboard">
    <div class="status-row">
      <span>WebSocket: <span class="ws-badge" id="ws_status">D&#233;connect&#233;</span></span>
      <span id="mode-badge" class="mode-badge">&#9208; Veille</span>
    </div>
    <div>
      <div class="bat-label">Batterie</div>
      <div class="battery-bar"><div class="battery-fill" id="battery"></div></div>
    </div>
    <div class="mode-btns">
      <button class="btn-mode av" id="bv" onclick="setMode(0)">&#9208;<br>Veille</button>
      <button class="btn-mode"    id="bm" onclick="setMode(1)">&#x1F579;<br>Manuel</button>
      <button class="btn-mode"    id="bd" onclick="setMode(2)">&#x1F5FA;<br>D&#233;couverte</button>
      <button class="btn-mode"    id="bn" onclick="setMode(3)">&#x1F9F9;<br>Nettoyage</button>
    </div>

    <!-- Panel Manuel -->
    <div class="panel" id="pm">
      <div class="dpad">
        <div class="dpad-btn dpad-empty"></div>
        <button class="dpad-btn" ontouchstart="mv('avancer')" ontouchend="mv('stop')" onmousedown="mv('avancer')" onmouseup="mv('stop')">&#8593;</button>
        <div class="dpad-btn dpad-empty"></div>
        <button class="dpad-btn" ontouchstart="mv('gauche')"  ontouchend="mv('stop')" onmousedown="mv('gauche')"  onmouseup="mv('stop')">&#8592;</button>
        <button class="dpad-btn" ontouchstart="mv('stop')"    onmousedown="mv('stop')">&#9632;</button>
        <button class="dpad-btn" ontouchstart="mv('droite')"  ontouchend="mv('stop')" onmousedown="mv('droite')"  onmouseup="mv('stop')">&#8594;</button>
        <div class="dpad-btn dpad-empty"></div>
        <button class="dpad-btn" ontouchstart="mv('reculer')" ontouchend="mv('stop')" onmousedown="mv('reculer')" onmouseup="mv('stop')">&#8595;</button>
        <div class="dpad-btn dpad-empty"></div>
      </div>
      <button class="btn-asp" id="btn-asp" onclick="toggleAsp()">&#x1F300; Aspiration OFF</button>
    </div>

    <!-- Panel Decouverte -->
    <div class="panel" id="pd">
      <div class="desc-text">Le robot explore la pi&#232;ce. Turbine d&#233;sactiv&#233;e pour maximiser l'autonomie.</div>
      <button class="btn-save" onclick="ouvrirModal()">&#x1F4BE; Sauvegarder la carte</button>
    </div>

    <!-- Panel Nettoyage -->
    <div class="panel" id="pn">
      <select class="select-piece" id="sel-piece">
        <option value="">Choisir une pi&#232;ce...</option>
      </select>
      <button class="btn-nettoyer" onclick="lancerNettoyage()">&#x1F9F9; Lancer le nettoyage</button>
    </div>
  </div>

  <!-- Modal sauvegarde -->
  <div class="modal-overlay" id="modal-save">
    <div class="modal-box">
      <h3>&#x1F4BE; Sauvegarder la carte</h3>
      <input class="modal-input" id="inp-nom" type="text" placeholder="ex: Cuisine, Salon...">
      <div class="modal-btns">
        <button class="btn-mc" onclick="fermerModal()">Annuler</button>
        <button class="btn-mk" onclick="confirmerSauvegarde()">Sauvegarder</button>
      </div>
    </div>
  </div>

  <script>
    const canvas=document.getElementById("mapCanvas"),ctx=canvas.getContext("2d");
    function resizeCanvas(){canvas.width=window.innerWidth;canvas.height=window.innerHeight;}
    resizeCanvas();window.addEventListener('resize',resizeCanvas);

    let oX=canvas.width/2,oY=canvas.height/2,sc=1.5,drag=false,lx,ly;
    let robot={rx:250,ry:250,ra:0,obs_dist:100},path=[],obs=[];
    let aspOn=false,curMode=0;

    const mBadge=["&#9208; Veille","&#x1F579; Manuel","&#x1F5FA; D&#233;couverte","&#x1F9F9; Nettoyage"];
    const mCls  =["av","am","ad","an"];
    const mBtnIds=["bv","bm","bd","bn"];
    const mPanels=[null,"pm","pd","pn"];

    const ws=new WebSocket(`ws://${window.location.hostname}/ws`);
    ws.onopen=function(){
      document.getElementById("ws_status").innerText="Connect&#233;";
      document.getElementById("ws_status").classList.add("connected");
      ws.send(JSON.stringify({command:"list_maps"}));
    };
    ws.onmessage=function(e){
      const d=JSON.parse(e.data);
      if(d.maps!==undefined){
        const sel=document.getElementById("sel-piece"),prev=sel.value;
        sel.innerHTML='<option value="">Choisir une pi&#232;ce...</option>';
        d.maps.forEach(m=>{const o=document.createElement("option");o.value=m;o.textContent=m;sel.appendChild(o);});
        if(prev)sel.value=prev;
        return;
      }
      robot=d;
      path.push({x:d.rx,y:d.ry});if(path.length>500)path.shift();
      if(d.bat!==undefined){const b=document.getElementById("battery");b.style.width=d.bat+"%";d.bat<30?b.classList.add("low"):b.classList.remove("low");}
      if(d.mode!==undefined&&d.mode!==curMode)updateModeUI(d.mode);
      if(d.asp!==undefined){aspOn=d.asp;const btn=document.getElementById("btn-asp");btn.textContent=d.asp?"&#x1F300; Aspiration ON":"&#x1F300; Aspiration OFF";btn.className="btn-asp"+(d.asp?" active":"");}
      if(d.obs_dist>0&&d.obs_dist<40)obs.push({x:d.rx+d.obs_dist*Math.cos(d.ra),y:d.ry+d.obs_dist*Math.sin(d.ra)});
      if(obs.length>300)obs.shift();
    };

    function send(cmd,extra){if(ws.readyState!==1)return;ws.send(JSON.stringify(Object.assign({command:cmd},extra||{})));}
    function setMode(m){send("mode",{value:m});updateModeUI(m);if(m===3)send("list_maps");}
    function updateModeUI(m){
      curMode=m;
      mBtnIds.forEach((id,i)=>{document.getElementById(id).className="btn-mode"+(i===m?" "+mCls[i]:"");});
      const badge=document.getElementById("mode-badge");badge.textContent=mBadge[m];badge.className="mode-badge"+(m>0?" "+mCls[m]:"");
      mPanels.forEach((id,i)=>{if(!id)return;document.getElementById(id).className="panel"+(i===m?" visible":"");});
    }
    function mv(dir){send("move",{direction:dir});}
    function toggleAsp(){aspOn=!aspOn;send("aspiration",{value:aspOn});}
    function ouvrirModal(){document.getElementById("modal-save").classList.add("visible");document.getElementById("inp-nom").focus();}
    function fermerModal(){document.getElementById("modal-save").classList.remove("visible");document.getElementById("inp-nom").value="";}
    function confirmerSauvegarde(){const n=document.getElementById("inp-nom").value.trim();if(!n){document.getElementById("inp-nom").focus();return;}send("save_map",{name:n});fermerModal();toast("&#x2705; Carte \""+n+"\" sauvegard&#233;e !");send("list_maps");}
    function lancerNettoyage(){const s=document.getElementById("sel-piece");if(!s.value){toast("&#x26A0;&#xFE0F; Choisissez une pi&#232;ce !");return;}send("load_map",{name:s.value});toast("&#x1F9F9; Nettoyage de \""+s.value+"\" lanc&#233; !");}
    function toast(msg){const t=document.getElementById("toast");t.innerHTML=msg;t.classList.add("show");setTimeout(()=>t.classList.remove("show"),3000);}

    // Interactions souris
    canvas.addEventListener('mousedown',e=>{drag=true;lx=e.clientX;ly=e.clientY;canvas.style.cursor='grabbing';});
    canvas.addEventListener('mousemove',e=>{if(drag){oX+=e.clientX-lx;oY+=e.clientY-ly;lx=e.clientX;ly=e.clientY;}});
    canvas.addEventListener('mouseup',()=>{drag=false;canvas.style.cursor='grab';});
    canvas.addEventListener('wheel',e=>{e.preventDefault();sc*=e.deltaY<0?1.1:0.9;sc=Math.max(0.5,Math.min(sc,5));});

    // Interactions tactiles
    let td=0;
    canvas.addEventListener('touchstart',e=>{if(e.touches.length===1){lx=e.touches[0].clientX;ly=e.touches[0].clientY;drag=true;}else if(e.touches.length===2){drag=false;const dx=e.touches[0].clientX-e.touches[1].clientX,dy=e.touches[0].clientY-e.touches[1].clientY;td=Math.sqrt(dx*dx+dy*dy);}});
    canvas.addEventListener('touchmove',e=>{e.preventDefault();if(e.touches.length===1&&drag){oX+=e.touches[0].clientX-lx;oY+=e.touches[0].clientY-ly;lx=e.touches[0].clientX;ly=e.touches[0].clientY;}else if(e.touches.length===2){const dx=e.touches[0].clientX-e.touches[1].clientX,dy=e.touches[0].clientY-e.touches[1].clientY,nd=Math.sqrt(dx*dx+dy*dy);sc*=nd/td;sc=Math.max(0.5,Math.min(sc,5));td=nd;}});
    canvas.addEventListener('touchend',()=>{drag=false;});

    // Rendu
    function draw(){
      ctx.clearRect(0,0,canvas.width,canvas.height);
      ctx.save();ctx.translate(oX,oY);ctx.scale(sc,sc);
      // Grille
      ctx.strokeStyle="rgba(100,100,100,0.2)";ctx.lineWidth=1/sc;
      for(let i=-500;i<500;i+=50){ctx.beginPath();ctx.moveTo(i,-500);ctx.lineTo(i,500);ctx.stroke();ctx.beginPath();ctx.moveTo(-500,i);ctx.lineTo(500,i);ctx.stroke();}
      // Chemin
      if(path.length>1){ctx.strokeStyle="rgba(0,242,254,0.3)";ctx.lineWidth=8/sc;ctx.beginPath();path.forEach((p,i)=>i===0?ctx.moveTo(p.x,p.y):ctx.lineTo(p.x,p.y));ctx.stroke();}
      // Obstacles
      ctx.fillStyle="rgba(255,255,255,0.8)";obs.forEach(o=>ctx.fillRect(o.x-3,o.y-3,6,6));
      // Robot
      ctx.shadowBlur=20;ctx.shadowColor="#00f2fe";ctx.fillStyle="#00f2fe";ctx.beginPath();ctx.arc(robot.rx,robot.ry,8,0,Math.PI*2);ctx.fill();
      // Direction
      ctx.shadowBlur=0;ctx.beginPath();ctx.moveTo(robot.rx,robot.ry);ctx.lineTo(robot.rx+Math.cos(robot.ra-.5)*25,robot.ry+Math.sin(robot.ra-.5)*25);ctx.lineTo(robot.rx+Math.cos(robot.ra+.5)*25,robot.ry+Math.sin(robot.ra+.5)*25);ctx.fillStyle="rgba(0,242,254,0.5)";ctx.fill();
      ctx.restore();requestAnimationFrame(draw);
    }
    draw();
  </script>
</body></html>
)rawliteral";

// ======================================================================
// Gestionnaire d'événements WebSocket
// ======================================================================
void communicationInit() {
  debugLog("[WIFI] Connexion a " + String(WIFI_SSID));

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  debugLog("\n[WIFI] Connecte ! Adresse IP : " + WiFi.localIP().toString());

  ws.onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client,
                AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_DATA) {
      AwsFrameInfo* info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len
          && info->opcode == WS_TEXT) {
        String message = String((char*)data, len);

        StaticJsonDocument<128> doc;
        if (deserializeJson(doc, message) || !doc.containsKey("command")) return;

        String cmd = doc["command"].as<String>();

        if (cmd == "mode") {
          uint8_t val = doc["value"] | 0;
          if (val <= 3) modeActuel = static_cast<ModeRobot>(val);
          debugLog("[WEB] Mode -> " + String(val));

        } else if (cmd == "move") {
          String dir = doc["direction"] | "stop";
          if      (dir == "avancer") commandeManuelle = CommandeManuelle::Avancer;
          else if (dir == "reculer") commandeManuelle = CommandeManuelle::Reculer;
          else if (dir == "gauche")  commandeManuelle = CommandeManuelle::TournerGauche;
          else if (dir == "droite")  commandeManuelle = CommandeManuelle::TournerDroite;
          else                       commandeManuelle = CommandeManuelle::Stop;

        } else if (cmd == "aspiration") {
          aspirationManu = doc["value"] | false;

        } else if (cmd == "save_map") {
          String nom = doc["name"] | "";
          nom.trim();
          if (nom.length() > 0 && nom.length() < 32) {
            nom.toCharArray(nomSauvegarde, sizeof(nomSauvegarde));
            flagSauvegarder = true;
            debugLog("[WEB] Sauvegarde demandee : " + nom);
          }

        } else if (cmd == "load_map") {
          String nom = doc["name"] | "";
          nom.trim();
          if (nom.length() > 0 && nom.length() < 32) {
            nom.toCharArray(nomChargement, sizeof(nomChargement));
            flagCharger = true;
            debugLog("[WEB] Chargement demande : " + nom);
          }

        } else if (cmd == "list_maps") {
          flagListePieces = true;
        }
      }
    }
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
  });

  server.addHandler(&ws);
  server.begin();
  debugLog("[WEBSOCKET] Serveur demarre");
}

// ======================================================================
// Envoi de la télémétrie
// ======================================================================
void communicationEnvoyerMiseAJour(PositionRobot pos, float distanceObstacle,
                                   float niveauBatterie, bool aspirationActive) {
  if (ws.count() == 0) return;

  StaticJsonDocument<256> doc;
  doc["rx"]      = pos.x;
  doc["ry"]      = pos.y;
  doc["ra"]      = pos.angle;
  doc["obs_dist"] = distanceObstacle;
  doc["bat"]     = niveauBatterie;
  doc["mode"]    = static_cast<uint8_t>(modeActuel);
  doc["asp"]     = aspirationActive;

  String payload;
  serializeJson(doc, payload);
  ws.textAll(payload);
}

// ======================================================================
// Envoi de la liste des pièces
// ======================================================================
void communicationEnvoyerListePieces(const String& jsonTableau) {
  if (ws.count() == 0) return;
  ws.textAll("{\"maps\":" + jsonTableau + "}");
}

// ======================================================================
// Nettoyage des clients déconnectés
// ======================================================================
void communicationCleanupClients() {
  ws.cleanupClients();
}

// ======================================================================
// Getters
// ======================================================================
ModeRobot communicationGetMode() {
  return modeActuel;
}

CommandeManuelle communicationGetCommandeManuelle() {
  return commandeManuelle;
}

bool communicationGetAspirationManuelle() {
  return aspirationManu;
}

bool communicationSauvegarderDemande(char* nomOut, size_t taille) {
  if (!flagSauvegarder) return false;
  strncpy(nomOut, nomSauvegarde, taille - 1);
  nomOut[taille - 1] = '\0';
  return true;
}

void communicationAcquitterSauvegarde() {
  flagSauvegarder = false;
}

bool communicationChargerDemande(char* nomOut, size_t taille) {
  if (!flagCharger) return false;
  strncpy(nomOut, nomChargement, taille - 1);
  nomOut[taille - 1] = '\0';
  return true;
}

void communicationAcquitterChargement() {
  flagCharger = false;
}

bool communicationListePiecesDemandee() {
  return flagListePieces;
}

void communicationAcquitterListePieces() {
  flagListePieces = false;
}

void communicationSetMode(ModeRobot mode) {
  modeActuel = mode;
}

