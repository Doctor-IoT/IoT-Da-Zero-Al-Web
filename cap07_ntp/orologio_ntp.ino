/*
  cap07 — Orologio di rete con NTP
  =================================
  Si connette al Wi-Fi e sincronizza l'ora con un server NTP.
  Ogni secondo stampa l'ora corrente e il giorno della settimana
  sulla seriale, usando il pattern millis() per evitare delay().

  Librerie richieste:
    - ESP8266WiFi    (inclusa nel pacchetto esp8266 by ESP8266 Community)
    - NTPClient      (da installare: cerca "NTPClient" di Fabrice Weinberg)
    - WiFiUdp        (inclusa nel pacchetto esp8266)

  Riferimento: capitolo 7, par. 7.3 e 7.5
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// ── Credenziali di rete ────────────────────────────────────────────────────
const char* ssid     = "NomeDellaRete";      // sostituire con il proprio SSID
const char* password = "PasswordDellaRete";  // sostituire con la propria password

// ── Client NTP ────────────────────────────────────────────────────────────
WiFiUDP ntpUDP;
// Parametri: oggetto UDP, server NTP, offset UTC in secondi, intervallo aggiornamento in ms
// 3600 = UTC+1 (ora italiana invernale); usare 7200 per l'ora legale estiva.
// 3600000 ms = 1 ora tra un aggiornamento e il successivo.
NTPClient timeClient(ntpUDP, "it.pool.ntp.org", 3600, 3600000);

// ── Nomi dei giorni della settimana ────────────────────────────────────────
// getDay() restituisce 0 (domenica) … 6 (sabato): convenzione POSIX/nordamericana.
char giorniSettimana[7][12] = {
  "Domenica", "Lunedi", "Martedi", "Mercoledi", "Giovedi", "Venerdi", "Sabato"
};

// ── Timing non bloccante ───────────────────────────────────────────────────
unsigned long ultimaStampa = 0;
const unsigned long INTERVALLO_STAMPA = 1000;  // ms

void setup() {
  Serial.begin(115200);
  Serial.println();
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

  timeClient.begin();
  Serial.println("Client NTP avviato.");
}

void loop() {
  // update() controlla se è passato l'intervallo configurato (1 ora)
  // e, solo in quel caso, invia una nuova richiesta al server NTP.
  timeClient.update();

  // Stampa l'ora ogni secondo, senza usare delay()
  unsigned long adesso = millis();
  if (adesso - ultimaStampa >= INTERVALLO_STAMPA) {
    ultimaStampa = adesso;

    Serial.print(timeClient.getFormattedTime());  // es. "14:32:07"
    Serial.print(" - ");
    Serial.println(giorniSettimana[timeClient.getDay()]);

    // Timestamp epoch (secondi da 1° gennaio 1970, 00:00:00 UTC)
    // Serial.println(timeClient.getEpochTime());
  }
}
