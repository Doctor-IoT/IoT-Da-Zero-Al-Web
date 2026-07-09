/*
  cap08 — Web server di base con ESP8266WebServer
  ================================================
  La NodeMCU diventa un server HTTP: risponde alla root "/" con una
  pagina HTML che mostra un pulsante per accendere e spegnere il LED
  di bordo. Include anche un esempio di gestione della pagina 404.

  Par. 8.2 mostra il parsing "a basso livello" con WiFiServer/WiFiClient;
  par. 8.4 introduce ESP8266WebServer, che astrae tutto il parsing HTTP.
  Questo sketch usa la libreria ad alto livello (par. 8.4).

  Librerie richieste:
    - ESP8266WiFi      (inclusa nel pacchetto esp8266 by ESP8266 Community)
    - ESP8266WebServer (inclusa nel pacchetto esp8266 by ESP8266 Community)

  Riferimento: capitolo 8, par. 8.4 e 8.5
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ── Credenziali di rete ────────────────────────────────────────────────────
const char* ssid     = "NomeDellaRete";      // sostituire con il proprio SSID
const char* password = "PasswordDellaRete";  // sostituire con la propria password

const int LED_BORDO = 2;   // LED integrato sulla NodeMCU (logica invertita: LOW = acceso)
bool ledAcceso = false;

ESP8266WebServer server(80);   // server in ascolto sulla porta 80 (HTTP standard)

// ── Generazione della pagina HTML ─────────────────────────────────────────
String paginaHTML() {
  String stato = ledAcceso ? "ACCESO" : "SPENTO";
  String linkAzione = ledAcceso ? "/ledoff" : "/ledon";
  String testoBottone = ledAcceso ? "Spegni LED" : "Accendi LED";

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset=\"utf-8\">";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<title>NodeMCU Web Server</title>";
  html += "<style>body{font-family:Arial,sans-serif;text-align:center;margin-top:40px;}";
  html += "button{padding:10px 20px;font-size:1em;cursor:pointer;}</style>";
  html += "</head><body>";
  html += "<h1>Ciao dal NodeMCU!</h1>";
  html += "<p>LED di bordo: <strong>" + stato + "</strong></p>";
  html += "<a href=\"" + linkAzione + "\"><button>" + testoBottone + "</button></a>";
  html += "<hr><p>IP: " + WiFi.localIP().toString() + "</p>";
  html += "</body></html>";
  return html;
}

// ── Route handlers ─────────────────────────────────────────────────────────
void handleRoot() {
  server.send(200, "text/html", paginaHTML());
}

void handleLedOn() {
  ledAcceso = true;
  digitalWrite(LED_BORDO, LOW);   // logica invertita: LOW = acceso
  server.send(200, "text/html", paginaHTML());
}

void handleLedOff() {
  ledAcceso = false;
  digitalWrite(LED_BORDO, HIGH);  // logica invertita: HIGH = spento
  server.send(200, "text/html", paginaHTML());
}

void handleNotFound() {
  server.send(404, "text/plain", "Pagina non trovata");
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  pinMode(LED_BORDO, OUTPUT);
  digitalWrite(LED_BORDO, HIGH);  // LED spento all'avvio

  Serial.println("Connessione Wi-Fi in corso...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connesso!");
  Serial.print("Indirizzo IP: ");
  Serial.println(WiFi.localIP());

  // Registra le route: associa ogni URL a una funzione handler
  server.on("/",       handleRoot);
  server.on("/ledon",  handleLedOn);
  server.on("/ledoff", handleLedOff);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Server HTTP avviato. Aprire il browser all'indirizzo sopra.");
}

void loop() {
  // Verifica se sono arrivate nuove richieste HTTP e le instrada
  // verso il handler corretto — nessun blocking, loop() rimane libero.
  server.handleClient();
}
