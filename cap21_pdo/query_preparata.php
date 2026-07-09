<?php
/*
  cap21 — Inserimento sicuro con PDO e prepared statement
  =========================================================
  Versione sicura dello script di inserimento dati:
    - usa PDO (PHP Data Objects) per la connessione al database
    - usa prepared statement con named placeholder (:nome) per prevenire
      la SQL injection in modo strutturale (non per "attenzione")
    - usa try/catch per una gestione degli errori chiara e separata
    - valida i parametri sia per presenza sia per plausibilità

  URL di esempio (dalla NodeMCU):
    http://192.168.1.50/query_preparata.php?temp=23.5&umid=60&rssi=-61

  SICUREZZA: il file iotdata.db NON deve essere nella document root
  di Apache. Spostarlo in /var/data/ o altra directory fuori da /var/www/html/.

  Riferimento: capitolo 21, par. 21.2 e 21.4
*/

// In produzione: usare percorso assoluto fuori dalla document root
$db_path = '/var/data/iotdata.db';

try {
    // Connessione al database
    $db = new PDO("sqlite:" . $db_path);

    // ERRMODE_EXCEPTION: qualunque errore SQL genera una PDOException
    // intercettata dal catch, invece di fallire silenziosamente
    $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    // ── Lettura e validazione dei parametri ────────────────────────────────
    // isset() verifica la presenza; throw lancia un'eccezione gestita dal catch
    $timestamp = time();

    $temperatura = isset($_GET['temperatura']) ? $_GET['temperatura'] : null;
    $umidita     = isset($_GET['umidita'])     ? $_GET['umidita']     : null;
    $rssi        = isset($_GET['rssi'])        ? $_GET['rssi']        : null;

    if ($temperatura === null || $umidita === null || $rssi === null) {
        throw new Exception("Dati mancanti. Usare: ?temperatura=x&umidita=y&rssi=z");
    }

    // Validazione applicativa: i valori devono essere plausibili (par. 21.6)
    if (!is_numeric($temperatura) || $temperatura < -50 || $temperatura > 60) {
        throw new Exception("Temperatura non plausibile: " . $temperatura);
    }
    if (!is_numeric($umidita) || $umidita < 0 || $umidita > 100) {
        throw new Exception("Umidità non plausibile: " . $umidita);
    }

    // ── Query preparata con named placeholder ──────────────────────────────
    // prepare() compila la struttura della query senza ancora i valori.
    // I :placeholder sono punti di inserimento sicuri, mai interpretati come SQL.
    $stmt = $db->prepare(
        "INSERT INTO data (timeStamp, Temp, Humidity, Rssi, TIMEt)
         VALUES (:timestamp, :temp, :humidity, :rssi, :timet)"
    );

    // execute() fornisce i valori come array associativo.
    // PDO li tratta sempre come dati letterali, mai come codice SQL.
    $stmt->execute([
        ':timestamp' => $timestamp,
        ':temp'      => (float)$temperatura,
        ':humidity'  => (float)$umidita,
        ':rssi'      => (float)$rssi,
        ':timet'     => date("Y-m-d H:i:s"),
    ]);

    $db = null;  // chiude la connessione e rilascia il lock SQLite

    http_response_code(200);
    echo "Dati inseriti correttamente.";

} catch (PDOException $e) {
    // Errore del database (file non trovato, tabella inesistente, ecc.)
    http_response_code(500);
    echo "Errore PDO: " . $e->getMessage();

} catch (Exception $e) {
    // Errore di validazione (parametri mancanti o valori fuori range)
    http_response_code(400);
    echo "Errore: " . $e->getMessage();
}
?>
