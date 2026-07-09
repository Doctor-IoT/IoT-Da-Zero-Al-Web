/*
  cap26 — ESP-NOW: nodo trasmittente (sender)
  ============================================
  Invia una struttura di dati (temperatura, umidità, contatore) via ESP-NOW
  alla scheda ricevente, senza bisogno di un punto di accesso Wi-Fi.

  Prima di caricare questo sketch:
    1. Carica receiver.ino sulla scheda RICEVENTE e apri il Monitor seriale.
    2. Annota l'indirizzo MAC stampato dalla scheda ricevente (es. 48:3F:DA:6D:FE:71).
    3. Modifica l'array indirizzoRicevente[] qui sotto con quell'indirizzo.

  La struttura messaggio_dati deve essere IDENTICA — campo per campo e tipo per tipo —
  sia in questo sketch che in receiver.ino. ESP-NOW invia byte grezzi senza
  informazioni di formato: la corrispondenza tra le due strutture è responsabilità
  del programmatore (cfr. par. 26.6).

  Librerie richieste (già incluse nel pacchetto esp8266 by ESP8266 Community):
    - ESP8266WiFi.h
    - espnow.h

  Riferimento: capitolo 26, par. 26.3 e 26.4
*/

#include <ESP8266WiFi.h>
#include <espnow.h>

// Indirizzo MAC della scheda ricevente (da sostituire con quello reale,
// letto in precedenza con WiFi.macAddress() sulla scheda destinataria)
uint8_t indirizzoRicevente[] = {0x48, 0x3F, 0xDA, 0x6D, 0xFE, 0x71};

// La struttura dei dati da inviare: deve essere IDENTICA, byte per byte,
// sia sulla scheda trasmittente che su quella ricevente
typedef struct messaggio_dati {
  float temperatura;
  float umidita;
  int contatore;
} messaggio_dati;

messaggio_dati datiDaInviare;
unsigned long ultimoInvio = 0;
const unsigned long INTERVALLO = 2000;

// ── Callback di invio ──────────────────────────────────────────────────────
// Questa funzione NON va chiamata direttamente: viene registrata presso la libreria
// ESP-NOW e invocata automaticamente al termine di ogni trasmissione.
// Parametri: mac_addr = indirizzo MAC del destinatario; stato = 0 se successo.
void onDatiInviati(uint8_t *mac_addr, uint8_t stato) {
  Serial.print("Stato dell'ultimo invio: ");
  Serial.println(stato == 0 ? "successo" : "fallito");
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  // ESP-NOW richiede la modalità stazione (WIFI_STA) anche senza connettersi
  // a nessuna rete Wi-Fi: serve per inizializzare correttamente il chip radio.
  WiFi.mode(WIFI_STA);

  Serial.print("Indirizzo MAC di questa scheda (sender): ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != 0) {
    Serial.println("Errore nell'inizializzazione di ESP-NOW");
    return;
  }

  // Ruolo CONTROLLER: questa scheda invia dati ai peer registrati
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Registriamo la scheda ricevente come "peer": da questo momento la libreria
  // conosce il suo indirizzo MAC e può recapitarle i messaggi.
  // Parametri: MAC, ruolo remoto, canale Wi-Fi (1), chiave di cifratura (NULL = nessuna), lunghezza chiave
  esp_now_add_peer(indirizzoRicevente, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  // Registriamo la callback che verrà invocata al termine di ogni trasmissione
  esp_now_register_send_cb(onDatiInviati);

  Serial.println("ESP-NOW inizializzato. Avvio invio ogni 2 secondi...");
  datiDaInviare.contatore = 0;
}

void loop() {
  if (millis() - ultimoInvio >= INTERVALLO) {
    ultimoInvio = millis();

    // Aggiorniamo i campi della struttura con valori di prova.
    // In un progetto reale, qui si leggerebbero i sensori (es. BME280 del cap. 14).
    datiDaInviare.temperatura = 23.5 + (millis() % 100) / 100.0;
    datiDaInviare.umidita = 55.0;
    datiDaInviare.contatore++;

    // esp_now_send() è ASINCRONO: ritorna subito, la trasmissione avviene
    // in background, e onDatiInviati() viene chiamata quando è completata.
    esp_now_send(indirizzoRicevente, (uint8_t *) &datiDaInviare, sizeof(datiDaInviare));

    Serial.print("Pacchetto #");
    Serial.print(datiDaInviare.contatore);
    Serial.print(" inviato — temp: ");
    Serial.print(datiDaInviare.temperatura, 1);
    Serial.print(" °C, umid: ");
    Serial.print(datiDaInviare.umidita, 1);
    Serial.println(" %");
  }
}
