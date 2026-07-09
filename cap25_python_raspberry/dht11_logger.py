#!/usr/bin/env python3
"""
dht11_logger.py — Legge temperatura e umidità dal DHT11 e le salva in SQLite.
Pensato per essere eseguito una volta al minuto tramite crontab.

Collegamento:
  VCC  → pin 3,3 V o 5 V del Raspberry Pi
  DATA → GPIO 4 (configurabile tramite PIN_DHT11)
  GND  → massa

Installazione dipendenze:
  sudo apt install python3-gpiozero

Creazione directory database:
  sudo mkdir -p /var/local
  sudo chmod 777 /var/local

Pianificazione con crontab (ogni minuto):
  crontab -e
  # aggiungere la riga:
  * * * * *  /usr/bin/python3 /home/pi/dht11_logger.py >> /var/log/dht11.log 2>&1

Verifica dati registrati:
  sqlite3 /var/local/meteo.db "SELECT * FROM misurazioni ORDER BY ts DESC LIMIT 5;"

Riferimento: capitolo 25, par. 25.4 e 25.9
"""
import sqlite3
import time
from gpiozero import OutputDevice, InputDevice

# ── Configurazione ───────────────────────────────────────────────────────────
PIN_DHT11   = 4                      # GPIO fisico a cui è collegato il sensore
DB_PATH     = '/var/local/meteo.db'  # percorso del database SQLite
MAX_RETRIES = 3                      # tentativi massimi in caso di lettura errata


# ── Classe DHT11 ─────────────────────────────────────────────────────────────
class DHT11:
    """Lettura diretta del protocollo DHT11 via GPIO (cfr. par. 25.4)."""
    MAX_DELAY_COUNT   = 100
    BIT_1_DELAY_COUNT = 10
    BITS_LEN          = 40

    def __init__(self, pin):
        self._pin = pin

    def read(self):
        """
        Esegue una singola lettura.
        Restituisce (umidita, temperatura) come float, oppure None se
        il checksum non è valido (dato corrotto o sensore non disponibile).
        """
        bit_count = 0
        delay_count = 0
        bits = ""

        # Impulso di start: porta il pin basso per ≥ 18 ms, poi rilascia
        gpio = OutputDevice(self._pin)
        gpio.off()
        time.sleep(0.02)
        gpio.close()

        # Passa in ricezione e attende la risposta del sensore
        gpio = InputDevice(self._pin, pull_up=False)
        while gpio.value == 1:
            pass   # attesa handshake DHT (80 µs basso + 80 µs alto)

        # Legge i 40 bit: conta le iterazioni durante ogni impulso alto
        while bit_count < self.BITS_LEN:
            while gpio.value == 0:
                pass
            while gpio.value == 1:
                delay_count += 1
                if delay_count > self.MAX_DELAY_COUNT:
                    break
            bits += "1" if delay_count > self.BIT_1_DELAY_COUNT else "0"
            delay_count = 0
            bit_count += 1

        gpio.close()

        # Decodifica i 5 byte
        h_int = int(bits[0:8],  2)
        h_dec = int(bits[8:16], 2)
        t_int = int(bits[16:24], 2)
        t_dec = int(bits[24:32], 2)
        chk   = int(bits[32:40], 2)

        # Verifica checksum: scarta la lettura se corrotto
        if chk != (h_int + h_dec + t_int + t_dec) & 0xFF:
            return None

        return float(f'{h_int}.{h_dec}'), float(f'{t_int}.{t_dec}')


# ── Funzioni di supporto ──────────────────────────────────────────────────────
def init_db(path):
    """Apre (o crea) il database e garantisce l'esistenza della tabella."""
    conn = sqlite3.connect(path)
    conn.execute('''
        CREATE TABLE IF NOT EXISTS misurazioni (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            ts          INTEGER NOT NULL,
            umidita     REAL    NOT NULL,
            temperatura REAL    NOT NULL
        )
    ''')
    conn.commit()
    return conn

def salva_misura(conn, umidita, temperatura):
    """Inserisce una riga nel database con il timestamp corrente."""
    conn.execute(
        'INSERT INTO misurazioni (ts, umidita, temperatura) VALUES (?, ?, ?)',
        (int(time.time()), umidita, temperatura)
    )
    conn.commit()


# ── Programma principale ──────────────────────────────────────────────────────
if __name__ == '__main__':
    sensore = DHT11(PIN_DHT11)
    conn    = init_db(DB_PATH)

    # Tenta la lettura fino a MAX_RETRIES volte (checksum errato = riprova)
    for tentativo in range(1, MAX_RETRIES + 1):
        risultato = sensore.read()
        if risultato is not None:
            umidita, temperatura = risultato
            salva_misura(conn, umidita, temperatura)
            print(f'OK — {temperatura}°C  {umidita}% RH')
            break
        print(f'Tentativo {tentativo}/{MAX_RETRIES}: checksum errato, riprovo…')
        time.sleep(1)
    else:
        print('ERRORE: impossibile leggere il sensore dopo tutti i tentativi.')

    conn.close()
