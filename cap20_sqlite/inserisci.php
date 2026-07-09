<?php
/*
  cap20 — Inserimento misure nel database SQLite (versione base)
  ===============================================================
  Riceve temperatura, umidità e RSSI via GET dalla NodeMCU
  e li inserisce nella tabella data del database iotdata.db.

  URL di esempio (dalla NodeMCU o dal browser per test):
    http://192.168.1.50/inserisci.php?temp=23.5&umid=60&rssi=-61

  NOTA: questa versione usa query costruita con concatenazione di stringhe,
  per illustrare la forma più semplice. Il Capitolo 21 mostra perché questo
  sia insicuro e come usare invece i prepared statement con PDO.

  Riferimento: capitolo 20 (per il concetto); vedere cap21/query_preparata.php
  per la versione sicura con PDO e prepared statement.
*/

$db_path = 'iotdata.db';

// Verifica presenza dei parametri
if (!isset($_GET['temp']) || !isset($_GET['umid']) || !isset($_GET['rssi'])) {
    http_response_code(400);
    echo "ERRORE - Parametri mancanti. Usare: ?temp=valore&umid=valore&rssi=valore";
    exit;
}

$timestamp = time();
$temp = (float)$_GET['temp'];
$umid = (float)$_GET['umid'];
$rssi = (float)$_GET['rssi'];
$timeTxt = date("Y-m-d H:i:s");

try {
    $db = new PDO("sqlite:" . $db_path);
    $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    // ATTENZIONE: questo stile di costruzione della query è vulnerabile
    // alla SQL injection. Usare i prepared statement del cap21 per codice sicuro.
    $db->exec(
        "INSERT INTO data (timeStamp, Temp, Humidity, Rssi, TIMEt) VALUES ("
        . $timestamp . ", " . $temp . ", " . $umid . ", " . $rssi
        . ", '" . $timeTxt . "')"
    );

    $db = null;

    echo "OK - Dati inseriti: temp=" . $temp . " umid=" . $umid
       . " rssi=" . $rssi . " alle " . $timeTxt;

} catch (PDOException $e) {
    http_response_code(500);
    echo "Errore PDO: " . $e->getMessage();
}
?>
