<?php
/*
  cap20 — Creazione del database SQLite
  =======================================
  Script da eseguire una sola volta: crea il file iotdata.db e
  la tabella data con le colonne usate nel laboratorio IoT.

  Eseguire da riga di comando:
    php crea_db.php

  oppure, se il server ha la directory /var/data/:
    php /var/www/html/crea_db.php

  SICUREZZA: il file .db NON deve essere dentro la document root
  di Apache (/var/www/html/), altrimenti chiunque potrebbe scaricarlo.
  Usare una directory fuori dalla document root, per esempio /var/data/.

  Schema reale del laboratorio IoT dell'IIS Cassata Gattapone (par. 20.6):
    timeStamp : Unix timestamp (secondi da 1/1/1970)
    Temp      : temperatura in °C (REAL)
    Humidity  : umidità relativa % (REAL)
    Pressure  : pressione in Pa (REAL) — dividere per 100 per ottenere hPa
    Rssi      : RSSI Wi-Fi in dBm (REAL)
    TIMEt     : data/ora in formato leggibile (TEXT, ridondante ma comodo)

  Riferimento: capitolo 20, par. 20.5 e 20.6
*/

// Il file .db viene creato in questa stessa directory se non esiste.
// In produzione, spostarlo fuori dalla document root del web server.
$db_path = 'iotdata.db';

try {
    // new PDO() crea il file SQLite se non esiste ancora
    $db = new PDO("sqlite:" . $db_path);
    $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    // Versione semplificata della tabella di laboratorio
    // IF NOT EXISTS: eseguire lo script più volte non causa errori
    $db->exec("
        CREATE TABLE IF NOT EXISTS data (
            id        INTEGER PRIMARY KEY AUTOINCREMENT,
            timeStamp INTEGER NOT NULL,
            Temp      REAL,
            Humidity  REAL,
            Pressure  REAL,
            Rssi      REAL,
            TIMEt     TEXT
        )
    ");

    $db = null;

    echo "Database creato correttamente: " . realpath($db_path) . PHP_EOL;
    echo "Tabella 'data' creata (o già esistente)." . PHP_EOL;

} catch (PDOException $e) {
    echo "Errore PDO: " . $e->getMessage() . PHP_EOL;
    exit(1);
}
?>
