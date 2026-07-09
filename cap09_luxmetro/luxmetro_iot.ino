/*
  cap09 — LuxMeter IoT — NodeMCU 1.0
  ====================================
  Progetto di sintesi della Parte I: misura la luminosità ambientale
  tramite un fotoresistore LDR su A0 e mostra il valore — insieme all'ora
  NTP, all'RSSI Wi-Fi e all'uptime — su una pagina web con canvas HTML5.

  Collegamento:
    LDR + R fissa (1 kΩ) in partitore tra 3V3 e GND → pin A0

  Formula di calibrazione (par. 9.5–9.6):
    Lux = 39,9 * e^(1,73 * V) - 39,9
    dove V = ADC_val * 3,3 / 1023
    → semplificato in: Lux = 39,9 * EULERO^(ADC_val * LUXconst) - 39,9

  Comandi seriali (par. 9.7):
    a  Attiva/disattiva modalità ADMIN (stampa il lux a ogni lettura)
    b  Lampeggio LED
    l  Alterna stato LED
    m  Stampa uptime, RSSI e Lux
    r  Sequenza di lampeggio con pausa estesa
    s  Stato LED
    t  Tempo di esecuzione e ora NTP

  Librerie richieste:
    - ESP8266WiFi      (inclusa nel pacchetto esp8266 by ESP8266 Community)
    - ESP8266WebServer (inclusa nel pacchetto esp8266 by ESP8266 Community)
    - NTPClient        (da installare: cerca "NTPClient" di Fabrice Weinberg)
    - WiFiUdp          (inclusa nel pacchetto esp8266 by ESP8266 Community)

  Riferimento: capitolo 9, par. 9.7
*/

#include <EEPROM.h>
#include <ESP.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiMulti.h>
#include <NTPClient.h>
#include <time.h>
#include <WiFiClientSecure.h>

#define firmwareRev  "WebPage V.1.0"
#define LED          2
#define ONESEC       1000
#define TBLINK       100
#define ALTBLINK     300
#define EULERO       2.71828183f
#define LUXconst     0.00558065f   // = 1,73 * 3,3 / 1023
                                   // Lux = 39,9 * e^(1,73 * ADCval * 3,3 / 1023)

// ── Credenziali di rete ────────────────────────────────────────────────────
const char* ssid     = "NomeDellaRete";      // sostituire con il proprio SSID
const char* password = "PasswordDellaRete";  // sostituire con la propria password

ESP8266WebServer server(80);

bool ADMIN = 0;
double LUXval;
char incomingByte;
unsigned long myTime;
int unused = 0;
long rssi;
long utcOffsetInSeconds = 3600;       // UTC+1 (ora italiana)
long ntpUpdateInterval  = 3600000;    // Aggiornamento NTP ogni ora (ms)
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday",
                               "Wednesday", "Thursday", "Friday", "Saturday" };
unsigned long uptime;
unsigned char uptimeDays;
unsigned char uptimeHours;
unsigned char uptimeMinutes;
unsigned char uptimeSeconds;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "it.pool.ntp.org", utcOffsetInSeconds, ntpUpdateInterval);

void ADCread(void);
void LEDstatus(void);
void ProcessSerialData(void);
void handle_OnConnect(void);
void handle_NotFound(void);

void handle_OnConnect() { SendHTMLchunked(0); }
void handle_NotFound()  { SendHTMLchunked(0); }

void setup() {
  Serial.begin(115200);
  Serial.println("");
  pinMode(LED, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  timeClient.begin();
  myTime = millis();
}

void loop() {
  timeClient.update();
  server.handleClient();
  if ((millis() - myTime) >= ONESEC) {
    myTime += ONESEC;
    ADCread();
    rssi = WiFi.RSSI();
  }
  while (Serial.available()) {
    ProcessSerialData();
  }
}

void SendHTMLchunked(uint8_t textNumber) {
  String ptr = "";
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(F("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=0.5\"><title>Test Page</title>"));
  server.sendContent(F("<style>form {margin: 0 auto; width: 600px; padding: 1em; border: 1px solid #CCC; border-radius: 1em;}"));
  server.sendContent(F("form div + div {margin-top: 1em;}"));
  server.sendContent(F("label {display: inline-block; width: 140px; text-align: right; padding-right:5px;}"));
  server.sendContent(F(".wide {display: inline-block; width: 220px; text-align: right; padding-right:5px;}"));
  server.sendContent(F("input, textarea {font: 1em sans-serif; width: 240px; box-sizing: border-box; border: 1px solid #999;}"));
  server.sendContent(F("input, canvas {vertical-align: bottom;}"));
  server.sendContent(F("input:focus, textarea:focus {border-color: #000;}"));
  server.sendContent(F(".small {width: 60px; type=\"number\"; maxlength=\"3\"; min=\"0\"; max=\"255\";}"));
  server.sendContent(F(".setp {width: 100px; type=\"number\"; maxlength=\"3\"; min=\"-40\"; max=\"85\";}"));
  server.sendContent(F("textarea {vertical-align: top; height: 5em;}"));
  server.sendContent(F("button {margin: 1em;}.button {background-color: #f9d20b"));
  server.sendContent(F("; color:white;text-align: center;border-radius: .5em;}</style></head><body bgcolor=\"#f9d20b"));
  server.sendContent(F("\"><center><div class=\"container text-center\" style=\"background-color:white; width: 660px; border-radius: 10px; border-color: rgb(14, 99, 174); border-width: medium; border-style: solid;\">"));
  switch (textNumber) {
    case 0:
      {
        unsigned int barlen;
        server.sendContent(F("<h1>Sensors @ Iot Lab</h1><form action=\"/\" method=\"post\"><div>Local Time: "));
        server.sendContent(timeClient.getFormattedTime() + " " + String(daysOfTheWeek[timeClient.getDay()]));
        // Canvas RSSI: gradiente rosso (segnale debole) → verde (segnale forte)
        server.sendContent(F("</div><div><label for=\"rssi\">WiFi RSSI dBm</label><canvas id=\"rssi\" width=\"240\" height=\"20\" style=\"border:1px solid #999;\">HTML5 canvas not supported!</canvas>"));
        server.sendContent(F("<script>var c=document.getElementById(\"rssi\"); var ctx=c.getContext(\"2d\");"));
        server.sendContent(F("var grd=ctx.createLinearGradient(0,0,180,0); grd.addColorStop(0,\"red\"); grd.addColorStop(1,\"lime\");"));
        server.sendContent(F("ctx.font=\"1em sans-serif\"; ctx.textAlign=\"right\"; ctx.fillText(\""));
        server.sendContent(String((int)rssi));
        server.sendContent(F("\",230,16); ctx.fillStyle=grd; ctx.fillRect(0,0,"));
        barlen = (int)(((rssi + 40) * 3) + 180);
        if (barlen > 180)
          barlen = 180;
        server.sendContent(String(barlen));  // -100 dBm a -40 dBm, 3 px per dBm
        server.sendContent(F(",20);</script></div>"));
        // Canvas Lux: gradiente nero (buio) → acqua (piena luce)
        server.sendContent(F("<div><label for=\"lux\">Light Intensity Lux</label><canvas id=\"lux\" width=\"240\" height=\"20\" style=\"border:1px solid #999;\">HTML5 canvas not supported!</canvas>"));
        server.sendContent(F("<script>var c=document.getElementById(\"lux\"); var ctx=c.getContext(\"2d\");"));
        server.sendContent(F("var grd=ctx.createLinearGradient(0,0,180,0); grd.addColorStop(0,\"black\"); grd.addColorStop(1,\"aqua\");"));
        server.sendContent(F("ctx.font=\"1em sans-serif\"; ctx.textAlign=\"right\"; ctx.fillText(\""));
        server.sendContent(String((int)LUXval));
        server.sendContent(F("\",230,16); ctx.fillStyle=grd; ctx.fillRect(0,0,"));
        barlen = (int)(LUXval / 40);    // 0 a 7200 Lux, 1 px ogni 40 Lux
        if (barlen > 180)
          barlen = 180;
        server.sendContent(String(barlen));
        server.sendContent(F(",20);</script></div>"));
        // Uptime
        server.sendContent(F("<div><label for=\"uptime\">Uptime dd:hh:mm:ss </label><input type=\"text\" name=\"uptime\" readonly value=\""));
        uptime = millis() / 1000;
        uptimeDays    = uptime / 86400;
        uptimeHours   = (uptime % 86400) / 3600;
        uptimeMinutes = (uptime % 3600) / 60;
        uptimeSeconds = uptime % 60;
        ptr = String(uptimeDays) + ":" + String(uptimeHours) + ":" + String(uptimeMinutes) + ":" + String(uptimeSeconds) + "\n";
        server.sendContent(ptr);
        server.sendContent(F("\"></div><div><button class=\"button\" type=\"submit\">REFRESH</button></div></form>"));
      }
      break;
  }
  server.sendContent(F("<br><p>"));
  server.sendContent(firmwareRev);
  server.sendContent(F("</p><p>Made by IoT Lab @ IIS Cassata Gattapone, Gubbio, Italy<br><a href=https://doctoriot.wordpress.com target=\"_new\">Doctor IoT</a></p></div></center></body></html>"));
  server.sendContent(F(""));  // chunk vuoto: segnala al browser la fine del trasferimento
  server.client().stop();
}

void ProcessSerialData(void) {
  incomingByte = Serial.read();
  int i;
  switch (incomingByte) {
    case 'a':         // Attiva/disattiva la modalità ADMIN
      ADMIN ^= 1;
      if (ADMIN)
        Serial.println("ADMIN mode!");
      else
        Serial.println("USER mode!");
      break;
    case 'b':         // Lampeggio del LED integrato
      i = 0;
      if (digitalRead(LED)) {
        while (i <= 5) {
          digitalWrite(LED, !digitalRead(LED));
          delay(TBLINK);
          i++;
        }
      } else {
        while (i <= 6) {
          digitalWrite(LED, !digitalRead(LED));
          delay(TBLINK);
          i++;
        }
      }
      break;
    case 'l':         // Alterna lo stato del LED
      digitalWrite(LED, !digitalRead(LED));
      Serial.print("LED status: ");
      LEDstatus();
      break;
    case 'm':         // Stampa le misure correnti
      uptime = millis() / 1000;
      uptimeDays    = uptime / 86400;
      uptimeHours   = (uptime % 86400) / 3600;
      uptimeMinutes = (uptime % 3600) / 60;
      uptimeSeconds = uptime % 60;
      Serial.println("dd:hh:mm:ss = " + String(uptimeDays) + ":" + String(uptimeHours) + ":" + String(uptimeMinutes) + ":" + String(uptimeSeconds));
      Serial.println("WiFi RSSI = " + String((int)rssi));
      Serial.println("LUX = " + String(LUXval));
      break;
    case 'r':         // Sequenza di lampeggio con pausa estesa
      i = 0;
      while (i <= 5) {
        digitalWrite(LED, !digitalRead(LED));
        if (i == 2)
          delay(ALTBLINK);
        else
          delay(TBLINK);
        i++;
      }
      break;
    case 's':         // Stato del LED
      LEDstatus();
      break;
    case 't':         // Tempo di esecuzione e ora NTP
      Serial.print("Running time: ");
      Serial.print(millis() / ONESEC);
      Serial.println("s");
      Serial.println(timeClient.getFormattedTime());
      break;
    default:
      Serial.println("Cmd not recognized!");
      break;
  }
}

void ADCread(void) {
  LUXval = 39.9f * pow(EULERO, ((float)analogRead(A0) * 0.00558065f)) - 39.9f;
  if (ADMIN == 1) {
    Serial.println(LUXval);
  }
}

void LEDstatus(void) {
  if (digitalRead(LED)) {
    Serial.println("OFF");
  } else {
    Serial.println("ON");   // logica invertita: LOW = acceso
  }
}
