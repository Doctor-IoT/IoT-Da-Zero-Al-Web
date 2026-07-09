/*
  cap14 — Lettura sensore BME280 e display OLED SSD1306
  =======================================================
  Legge temperatura, umidità e pressione dal sensore BME280 via I2C
  e mostra i valori sia sul Monitor seriale che sul display OLED SSD1306.
  La lettura avviene ogni 2 secondi usando il pattern millis() (cap. 3).

  Collegamento (entrambi i dispositivi sullo stesso bus I2C):
    SDA → D2 (GPIO 4)
    SCL → D1 (GPIO 5)
    Indirizzi: BME280 = 0x76 | SSD1306 = 0x3C

  Librerie richieste (installabili tramite Gestione Librerie Arduino IDE):
    - Wire             (inclusa nel pacchetto esp8266)
    - Adafruit GFX Library    (cerca "Adafruit GFX" di Adafruit)
    - Adafruit SSD1306        (cerca "Adafruit SSD1306" di Adafruit)
    - Adafruit BME280 Library (cerca "Adafruit BME280" di Adafruit)

  Riferimento: capitolo 14, par. 14.4, 14.5 e 14.6
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BME280.h>

// ── Display OLED 128×64 pixel ──────────────────────────────────────────────
// Terzo parametro: oggetto Wire (bus I2C); quarto: -1 = nessun pin RESET
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ── Sensore BME280 ─────────────────────────────────────────────────────────
Adafruit_BME280 bme;

// ── Timing non bloccante ───────────────────────────────────────────────────
unsigned long ultimaLettura = 0;
const unsigned long INTERVALLO = 2000;  // ms

void setup() {
  Serial.begin(115200);
  Serial.println();

  Wire.begin();  // inizializza I2C su D2/D1

  // ── Inizializzazione display ───────────────────────────────────────────
  // SSD1306_SWITCHCAPVCC: il display genera internamente la tensione del pannello
  // 0x3C: indirizzo I2C del display (talvolta 0x3D)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("ERRORE: display SSD1306 non trovato all'indirizzo 0x3C!");
    while (true);  // si blocca: senza display, non ha senso proseguire
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Avvio...");
  display.display();  // IMPORTANTE: senza questa chiamata nulla appare sul pannello

  // ── Inizializzazione sensore BME280 ───────────────────────────────────
  // 0x76: indirizzo I2C più comune per il BME280 (0x77 se pin SDO collegato a VCC)
  if (!bme.begin(0x76)) {
    Serial.println("ERRORE: sensore BME280 non trovato all'indirizzo 0x76!");
    // Prova anche 0x77
    if (!bme.begin(0x77)) {
      Serial.println("ERRORE: sensore BME280 non trovato nemmeno all'indirizzo 0x77!");
      while (true);
    }
    Serial.println("Sensore BME280 trovato all'indirizzo 0x77.");
  } else {
    Serial.println("Sensore BME280 trovato all'indirizzo 0x76.");
  }

  Serial.println("Setup completato.");
}

void loop() {
  if (millis() - ultimaLettura >= INTERVALLO) {
    ultimaLettura = millis();

    // ── Lettura del sensore ──────────────────────────────────────────────
    // Il BME280 restituisce già valori calibrati (nessuna formula di conversione)
    float t = bme.readTemperature();           // °C
    float h = bme.readHumidity();              // % umidità relativa
    float p = bme.readPressure() / 100.0F;    // Pa → hPa (millibar)

    // ── Stampa sul Monitor seriale ───────────────────────────────────────
    Serial.print("Temperatura: ");
    Serial.print(t, 1);
    Serial.println(" °C");

    Serial.print("Umidità: ");
    Serial.print(h, 1);
    Serial.println(" %");

    Serial.print("Pressione: ");
    Serial.print(p, 1);
    Serial.println(" hPa");

    Serial.println("---");

    // ── Aggiornamento display OLED ───────────────────────────────────────
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);

    display.print("Temp: ");
    display.print(t, 1);
    display.println(" C");

    display.print("Umid: ");
    display.print(h, 1);
    display.println(" %");

    display.print("Pres: ");
    display.print(p, 1);
    display.println(" hPa");

    // display.display() trasferisce il frame buffer al pannello fisico.
    // Va chiamato una volta sola dopo aver composto l'intera schermata.
    display.display();
  }
}
