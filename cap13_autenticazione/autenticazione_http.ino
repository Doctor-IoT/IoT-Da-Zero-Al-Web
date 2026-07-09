/*
  cap13 — Autenticazione HTTP Basic sul web server NodeMCU
  ==========================================================
  Il web server distingue una pagina pubblica (root "/") e una pagina
  protetta ("/pannello") accessibile solo dopo aver inserito le credenziali
  corrette. Il meccanismo usato è l'HTTP Basic Authentication, gestito
  interamente dalla libreria ESP8266WebServer senza bisogno di form HTML.

  Flusso di autenticazione (par. 13.2):
    1. Il browser richiede "/pannello"
    2. La scheda risponde con 401 + header WWW-Authenticate
    3. Il browser mostra il pop-up di login (generato dal browser, non da noi)
    4. Il browser reinvia la richiesta con le credenziali in Base64
    5. La scheda verifica con server.authenticate() e risponde con la pagina

  Limiti di sicurezza (par. 13.5):
    - Le credenziali non sono cifrate (solo codificate in Base64)
    - Per protezione reale servirebbero HTTPS e TLS
    - Adatto solo a reti locali di laboratorio

  Librerie richieste:
    - ESP8266WiFi      (inclusa nel pacchetto esp8266 by ESP8266 Community)
    - ESP8266WebServer (inclusa nel pacchetto esp8266 by ESP8266 Community)

  Riferimento: capitolo 13, par. 13.3 e 13.4
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ── Credenziali di rete ────────────────────────────────────────────────────
const char* ssid     = "NomeDellaRete";      // sostituire con il proprio SSID
const char* password = "PasswordDellaRete";  // sostituire con la propria password

// ── Credenziali per HTTP Basic Auth (diverse dalla password Wi-Fi!) ────────
const char* auth_username = "admin";
const char* auth_password = "password123";

ESP8266WebServer server(80);

// ── Pagina HTML protetta ───────────────────────────────────────────────────
String paginaProtetta() {
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta charset='utf-8'><title>Pannello di controllo</title>";
  s += "</head><body>";
  s += "<h1>Pannello di controllo</h1>";
  s += "<p>Autenticazione riuscita!</p>";
  s += "<p>IP scheda: " + WiFi.localIP().toString() + "</p>";
  s += "<p>RSSI: " + String(WiFi.RSSI()) + " dBm</p>";
  s += "<hr><a href='/'>Torna alla pagina pubblica</a>";
  s += "</body></html>";
  return s;
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connesso! IP: ");
  Serial.println(WiFi.localIP());

  // ── Pagina pubblica: nessuna autenticazione ────────────────────────────
  server.on("/", []() {
    server.send(200, "text/html",
      "<!DOCTYPE html><html><body>"
      "<h1>Benvenuto</h1>"
      "<p>Vai al <a href='/pannello'>pannello di controllo</a> (richiede autenticazione).</p>"
      "</body></html>");
  });

  // ── Pagina protetta: richiede autenticazione ────────────────────────────
  // server.on() con terzo parametro: funzione lambda (C++ moderno)
  server.on("/pannello", HTTP_GET, []() {
    // authenticate() verifica le credenziali della richiesta corrente.
    // Se non presenti o errate, requestAuthentication() invia il 401.
    if (!server.authenticate(auth_username, auth_password)) {
      return server.requestAuthentication();
    }
    // Solo se autenticato, si arriva qui
    server.send(200, "text/html", paginaProtetta());
  });

  server.begin();
  Serial.println("Server HTTP avviato.");
  Serial.println("  /         — pagina pubblica");
  Serial.println("  /pannello — pagina protetta (admin / password123)");
}

void loop() {
  server.handleClient();
}
