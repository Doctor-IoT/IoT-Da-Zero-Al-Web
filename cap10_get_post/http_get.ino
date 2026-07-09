/*
  cap10 — Richiesta HTTP GET dalla NodeMCU
  =========================================
  La scheda invia una misura (temperatura simulata e RSSI reale) al server
  tramite una richiesta GET, codificando i parametri nell'URL.

  URL risultante: http://SERVER_IP/sendata.php?temp=23.5&rssi=-61

  Librerie richieste:
    - ESP8266WiFi      (inclusa nel pacchetto esp8266 by ESP8266 Community)
    - ESP8266HTTPClient (inclusa nel pacchetto esp8266 by ESP8266 Community)

  Riferimento: capitolo 10, par. 10.3
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// ── Credenziali di rete ────────────────────────────────────────────────────
const char* ssid     = "NomeDellaRete";      // sostituire con il proprio SSID
const char* password = "PasswordDellaRete";  // sostituire con la propria password

// ── Indirizzo del server PHP ───────────────────────────────────────────────
// Sostituire con l'IP effettivo del server di laboratorio.
const char* serverIP = "192.168.1.50";

// ── Timing ────────────────────────────────────────────────────────────────
const unsigned long INTERVALLO = 10000;  // ms — invio ogni 10 secondi
unsigned long ultimoInvio = 0;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Connessione Wi-Fi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connesso!");
  Serial.print("IP locale: ");
  Serial.println(WiFi.localIP());
}

// ── Funzione di invio GET ─────────────────────────────────────────────────
void inviaDatiGET(float temperatura, long rssi) {
  WiFiClient client;
  HTTPClient http;

  // Costruzione dell'URL con i parametri codificati nella query string
  String url = "http://";
  url += serverIP;
  url += "/sendata.php?temp=" + String(temperatura, 1);
  url += "&rssi=" + String((int)rssi);

  Serial.print("GET: ");
  Serial.println(url);

  http.begin(client, url);
  int codiceRisposta = http.GET();

  // I codici positivi sono codici HTTP (200, 404, 500…)
  // I codici negativi sono errori di connessione di rete.
  if (codiceRisposta > 0) {
    Serial.print("Risposta server (HTTP " + String(codiceRisposta) + "): ");
    Serial.println(http.getString());
  } else {
    Serial.print("Errore di rete: ");
    Serial.println(http.errorToString(codiceRisposta));
  }

  http.end();
}

void loop() {
  if (millis() - ultimoInvio >= INTERVALLO) {
    ultimoInvio = millis();

    // Valori di esempio: temperatura simulata, RSSI reale
    float temperatura = 23.5;
    long rssi = WiFi.RSSI();

    inviaDatiGET(temperatura, rssi);
  }
}
