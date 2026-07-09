# IoT — Da Zero al Web: sketch di riferimento

Questo repository raccoglie gli sketch e gli script di riferimento del manuale
**"IoT — Da Zero al Web"** di Francesco Orfei, pubblicato su Amazon KDP.

> 📘 **Il libro è disponibile su Amazon** → *(inserire link)*

---

## Come usare questo repository

Il codice è organizzato per capitolo. Ogni cartella contiene lo sketch o lo
script relativo al capitolo corrispondente nel libro, con il nome del file che
richiama l'argomento trattato.

**Questo non è un repository da copiare e incollare.**
È uno strumento di confronto: scrivi il tuo codice a mano seguendo il libro,
poi verifica qui se hai ottenuto lo stesso risultato. Troverai anche commenti
aggiuntivi che non sempre compaiono nel testo stampato.

---

## Struttura

```
cap03_millis/
    blink_non_bloccante.ino      — pattern millis() vs delay()

cap06_wifi/
    connessione_wifi.ino         — connessione a rete Wi-Fi

cap07_ntp/
    orologio_ntp.ino             — sincronizzazione orario via NTP

cap08_webserver/
    webserver_base.ino           — web server integrato sulla NodeMCU

cap09_luxmetro/
    luxmetro_iot.ino             — luxmetro IoT completo (Parte I)

cap10_get_post/
    http_get.ino                 — richiesta HTTP GET dalla NodeMCU
    http_post.ino                — invio dati via HTTP POST

cap11_php/
    ricevi_dati.php              — script PHP di ricezione
    mostra_dati.php              — pagina di visualizzazione

cap12_csv/
    salva_csv.php                — archiviazione dati su file CSV

cap13_autenticazione/
    autenticazione_http.ino      — protezione con HTTP Basic Auth (NodeMCU)

cap14_i2c/
    scansione_i2c.ino            — scansione del bus I²C
    bmp280_lettura.ino           — lettura sensore BMP280

cap20_sqlite/
    crea_db.php                  — creazione database SQLite
    inserisci.php                — inserimento misure

cap21_pdo/
    query_preparata.php          — query PDO con prepared statement

cap23_grafici/
    dati.php                     — endpoint JSON per CanvasJS
    grafico.html                 — pagina con grafico interattivo

cap25_python_raspberry/
    dht11_logger.py              — registrazione DHT11 su SQLite (eseguito da crontab)
    meteo.php                    — dashboard meteo con grafico e storico 24 ore

cap26_espnow/
    sender.ino                   — nodo mittente ESP-NOW
    receiver.ino                 — nodo ricevitore / gateway ESP-NOW
```

*(La struttura si aggiornerà man mano che nuovi capitoli vengono aggiunti.)*

---

## Requisiti

- **Hardware:** scheda NodeMCU (ESP8266), opzionalmente Raspberry Pi
- **IDE:** Arduino IDE 1.8+ oppure Arduino IDE 2.x
  - Scheda: `esp8266` by ESP8266 Community (Board Manager)
  - Librerie: elencate nei commenti di ogni sketch
- **Server:** PHP 7.4+ con estensione PDO e PDO_SQLite abilitata
- **Python:** Python 3.x (per gli script Raspberry Pi)

---

## Licenza

Il codice in questo repository è rilasciato sotto **licenza MIT**.
Sei libero di usarlo, modificarlo e distribuirlo, anche in progetti personali
o commerciali, a condizione di mantenere il riferimento al copyright originale.

Vedi il file [`LICENSE`](LICENSE) per il testo completo.

---

## Autore

**Francesco Orfei** — ingegnere elettronico, dottore di ricerca in fisica e
tecnologie fisiche, insegna Elettronica e IoT all'IIS Cassata Gattapone di
Gubbio. Si occupa di Internet of Things dal 2005.
