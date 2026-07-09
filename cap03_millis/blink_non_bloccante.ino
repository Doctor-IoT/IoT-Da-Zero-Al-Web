/*
  cap03 — Pattern millis(): blink non bloccante
  =============================================
  Due LED lampeggianti con periodi diversi, senza mai usare delay().
  Il loop() rimane libero di rispondere ad altri eventi in qualsiasi momento.

  Librerie richieste: nessuna (core ESP8266)

  Collegamento:
    LED_A → D5 (GPIO 14) tramite resistenza da 220 Ω verso GND
    LED_B → D6 (GPIO 12) tramite resistenza da 220 Ω verso GND

  Riferimento: capitolo 3, par. 3.5
*/

const int PIN_LED_A = 14;   // D5 sulla NodeMCU
const int PIN_LED_B = 12;   // D6 sulla NodeMCU

const unsigned long PERIODO_A = 500;    // ms: LED_A lampeggia ogni 500 ms
const unsigned long PERIODO_B = 1300;   // ms: LED_B lampeggia ogni 1300 ms

bool statoA = LOW;
bool statoB = LOW;

unsigned long ultimoCambioA = 0;
unsigned long ultimoCambioB = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_A, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  digitalWrite(PIN_LED_A, LOW);
  digitalWrite(PIN_LED_B, LOW);
}

void loop() {
  unsigned long adesso = millis();

  // LED A: cambia stato ogni PERIODO_A millisecondi
  if (adesso - ultimoCambioA >= PERIODO_A) {
    ultimoCambioA = adesso;
    statoA = !statoA;
    digitalWrite(PIN_LED_A, statoA);
  }

  // LED B: cambia stato ogni PERIODO_B millisecondi (indipendente da LED A)
  if (adesso - ultimoCambioB >= PERIODO_B) {
    ultimoCambioB = adesso;
    statoB = !statoB;
    digitalWrite(PIN_LED_B, statoB);
  }

  // Qui potrebbero coesistere altre operazioni (lettura sensori,
  // gestione rete, risposta a comandi seriali...) senza nessun blocco.
}
