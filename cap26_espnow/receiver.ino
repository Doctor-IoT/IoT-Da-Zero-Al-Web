/*
  cap26 — ESP-NOW: nodo ricevente (receiver)
  ===========================================
  Riceve pacchetti di dati via ESP-NOW dalla scheda trasmittente e li stampa
  sul Monitor seriale. Non richiede nessun punto di accesso Wi-Fi.

  Come usare:
    1. Carica questo sketch sulla scheda RICEVENTE.
    2. Apri il Monitor seriale (115200 baud) e annota l'indirizzo MAC stampato
       nella sezione "Indirizzo MAC di questa scheda".
    3. Inserisci quell'indirizzo nell'array indirizzoRicevente[] in sender.ino.
    4. Carica sender.ino sulla scheda TRASMITTENTE.

  La struttura messaggio_dati deve essere IDENTICA — campo per campo e tipo per tipo —
  sia in questo sketch che in sender.ino. ESP-NOW trasmette byte grezzi: memcpy()
  li copia nella nostra struttura assumendo che il layout di memoria sia lo stesso
  su entrambe le schede (cfr. par. 26.6).

  Librerie richieste (già incluse nel pacchetto esp8266 by ESP8266 Community):
    - ESP8266WiFi.h
    - espnow.h

  Riferimento: capitolo 26, par. 26.5 e 26.6
*/

#include <ESP8266WiFi.h>
#include <espnow.h>

// Stessa identica struttura usata dal trasmittente: stessa sequenza di campi,
// stesso tipo, stesso ordine. Se uno di questi tre elementi differisse,
// i dati verrebbero decodificati in modo errato senza alcun messaggio di errore.
typedef struct messaggio_dati {
  float temperatura;
  float umidita;
  int contatore;
} messaggio_dati;

messaggio_dati datiRicevuti;

// ── Callback di ricezione ──────────────────────────────────────────────────
// Questa funzione NON va chiamata direttamente: viene registrata presso la libreria
// ESP-NOW e invocata automaticamente ogni volta che arriva un messaggio.
// Il loop() può rimanere completamente vuoto: la ricezione è event-driven.
void onDatiRicevuti(uint8_t *mac_addr, uint8_t *dati, uint8_t lunghezza) {
  // memcpy() copia i byte grezzi ricevuti nella struttura locale.
  // Questo è il momento in cui i "byte anonimi" diventano campi con nome e tipo.
  memcpy(&datiRicevuti, dati, sizeof(datiRicevuti));

  // Stampa il MAC del mittente e i valori decodificati
  Serial.print("Da: ");
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2],
           mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);

  Serial.print(" | Temperatura: ");
  Serial.print(datiRicevuti.temperatura, 1);
  Serial.print(" °C");

  Serial.print(" | Umidità: ");
  Serial.print(datiRicevuti.umidita, 1);
  Serial.print(" %");

  Serial.print(" | Pacchetto #");
  Serial.println(datiRicevuti.contatore);
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  // ESP-NOW richiede la modalità stazione (WIFI_STA) anche senza connettersi
  // a nessuna rete Wi-Fi.
  WiFi.mode(WIFI_STA);

  // Stampa l'indirizzo MAC di questa scheda: il trasmittente ne ha bisogno
  // per configurare il peer (cfr. sender.ino, array indirizzoRicevente[])
  Serial.print("Indirizzo MAC di questa scheda (receiver): ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != 0) {
    Serial.println("Errore nell'inizializzazione di ESP-NOW");
    return;
  }

  // Ruolo SLAVE: questa scheda riceve dati dai peer (schede controller)
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);

  // Registriamo la callback invocata a ogni messaggio ricevuto
  esp_now_register_recv_cb(onDatiRicevuti);

  Serial.println("ESP-NOW inizializzato. In ascolto di pacchetti...");
}

void loop() {
  // Il loop() è intenzionalmente vuoto: tutta la logica di ricezione
  // è delegata alla callback onDatiRicevuti(), invocata dalla libreria ESP-NOW
  // in modo analogo a un interrupt (cfr. cap. 15) — senza polling attivo.
}
