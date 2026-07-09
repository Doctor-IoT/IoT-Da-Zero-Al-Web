/*
  cap06 — Connessione a rete Wi-Fi
  =================================
  Connette la NodeMCU a una rete Wi-Fi e stampa l'indirizzo IP assegnato
  sulla seriale. Il LED di bordo (GPIO 2, logica invertita) indica lo stato:
    - Lampeggio rapido: connessione in corso
    - Acceso fisso   : connessione riuscita

  Librerie richieste:
    - ESP8266WiFi (inclusa nel pacchetto esp8266 by ESP8266 Community)

  Riferimento: capitolo 6, par. 6.3
*/

#include <ESP8266WiFi.h>

// ── Credenziali di rete ────────────────────────────────────────────────────
const char* ssid     = "NomeDellaRete";      // sostituire con il proprio SSID
const char* password = "PasswordDellaRete";  // sostituire con la propria password

const int LED_BORDO = 2;   // LED integrato sulla NodeMCU (logica invertita: LOW = acceso)

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Avvio connessione Wi-Fi...");

  pinMode(LED_BORDO, OUTPUT);
  digitalWrite(LED_BORDO, HIGH);  // LED spento all'avvio

  WiFi.begin(ssid, password);

  // Attesa connessione: il LED lampeggia ogni 500 ms mentre WiFi.status() != WL_CONNECTED
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BORDO, !digitalRead(LED_BORDO));  // inverte lo stato del LED
  }

  // Connessione riuscita
  digitalWrite(LED_BORDO, LOW);   // LED acceso fisso (logica invertita)
  Serial.println();
  Serial.println("Connesso!");
  Serial.print("Indirizzo IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

void loop() {
  // Niente da fare nel loop per questo sketch di base.
  // Negli sketch successivi (cap07, cap08, cap09) il loop gestirà
  // il client NTP e il web server.
}
