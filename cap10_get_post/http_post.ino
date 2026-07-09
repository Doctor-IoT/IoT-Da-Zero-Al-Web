/*
  cap10 — Invio dati via HTTP POST dalla NodeMCU
  ================================================
  La scheda invia una misura al server tramite una richiesta POST:
  i dati sono nel corpo della richiesta, non nell'URL.

  Il metodo POST è preferibile al GET per l'invio di misure perché:
    - i dati non compaiono nell'URL (né nei log del server né nella cache)
    - non esiste limite pratico alla quantità di dati trasmissibili
    - è semanticamente corretto: si "crea" una nuova riga di dati sul server

  Librerie richieste:
    - ESP8266WiFi       (inclusa nel pacchetto esp8266 by ESP8266 Community)
    - ESP8266HTTPClient (inclusa nel pacchetto esp8266 by ESP8266 Community)

  Riferimento: capitolo 10, par. 10.4
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// ── Credenziali di rete ────────────────────────────────────────────────────
const char* ssid     = "NomeDellaRete";      // sostituire con il proprio SSID
const char* password = "PasswordDellaRete";  // sostituire con la propria password

// ── Indirizzo del server PHP ───────────────────────────────────────────────
const char* serverURL = "http://192.168.1.50/sendata.php";

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

// ── Funzione di invio POST ────────────────────────────────────────────────
void inviaDatiPOST(float temperatura, float umidita, long rssi) {
  WiFiClient client;
  HTTPClient http;

  http.begin(client, serverURL);

  // Il Content-Type dichiara il formato del corpo della richiesta.
  // application/x-www-form-urlencoded è lo stesso formato usato da GET
  // (chiave=valore&chiave=valore), ma nel corpo invece che nell'URL.
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Costruzione del corpo della richiesta POST
  String corpo = "temp=" + String(temperatura, 1);
  corpo += "&umid=" + String(umidita, 1);
  corpo += "&rssi=" + String((int)rssi);

  Serial.print("POST a " + String(serverURL) + " — corpo: ");
  Serial.println(corpo);

  int codiceRisposta = http.POST(corpo);

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

    float temperatura = 23.5;   // sostituire con lettura reale del sensore
    float umidita = 60.0;       // sostituire con lettura reale del sensore
    long rssi = WiFi.RSSI();

    inviaDatiPOST(temperatura, umidita, rssi);
  }
}
