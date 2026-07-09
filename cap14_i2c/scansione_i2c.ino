/*
  cap14 — Scansione del bus I2C
  ==============================
  Prova sistematicamente tutti gli indirizzi I2C possibili (da 0x01 a 0x7F)
  e stampa sulla seriale quali dispositivi rispondono. Utile per verificare
  i dispositivi collegati prima di usarli in uno sketch più complesso.

  Collegamento:
    SDA → D2 (GPIO 4)
    SCL → D1 (GPIO 5)
    + resistenze di pull-up 4,7 kΩ verso 3V3 su entrambe le linee
      (spesso già presenti sul modulo del dispositivo)

  Indirizzi tipici in laboratorio:
    0x3C — display OLED SSD1306
    0x76 — sensore BME280 (o 0x77 con pin SDO a HIGH)

  Librerie richieste:
    - Wire (inclusa nel pacchetto esp8266 by ESP8266 Community)

  Riferimento: capitolo 14, par. 14.2 e 14.3
*/

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Scansione bus I2C...");
  Serial.println("SDA = D2 (GPIO 4)  |  SCL = D1 (GPIO 5)");
  Serial.println();

  Wire.begin();  // inizializza I2C sui pin predefiniti D2/D1

  int dispositiviTrovati = 0;

  // Scansione di tutti gli indirizzi validi (0x01–0x7F)
  for (byte indirizzo = 1; indirizzo < 127; indirizzo++) {
    Wire.beginTransmission(indirizzo);
    // endTransmission() restituisce:
    //   0 = successo (dispositivo risponde)
    //   2 = NACK all'indirizzo (nessun dispositivo a quell'indirizzo)
    //   altri = errore di comunicazione
    byte errore = Wire.endTransmission();

    if (errore == 0) {
      Serial.print("Dispositivo trovato all'indirizzo 0x");
      if (indirizzo < 16) Serial.print("0");
      Serial.println(indirizzo, HEX);
      dispositiviTrovati++;
    }
  }

  Serial.println();
  if (dispositiviTrovati == 0) {
    Serial.println("Nessun dispositivo I2C trovato.");
    Serial.println("Verificare i collegamenti e le resistenze di pull-up.");
  } else {
    Serial.print(dispositiviTrovati);
    Serial.println(" dispositivo/i trovato/i.");
  }
}

void loop() {
  // La scansione viene eseguita una sola volta in setup().
  // Ricaricare lo sketch per ripetere la scansione.
}
