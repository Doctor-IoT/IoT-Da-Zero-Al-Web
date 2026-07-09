<?php
/*
  cap23 — Endpoint JSON per CanvasJS
  ====================================
  Interroga il database SQLite e restituisce le ultime 24 ore di misure
  in formato JSON puro, pronto per essere consumato da grafico.html.

  URL di esempio:
    http://192.168.1.50/dati.php

  Risposta: array JSON di oggetti {x: timestamp_ms, y: temperatura}
    [{"x":1718700000000,"y":23.5}, {"x":1718703600000,"y":23.8}, ...]

  Separare il "fornitore di dati" (questo file) dalla "pagina di visualizzazione"
  (grafico.html) è buona pratica: permette di cambiare la libreria grafica
  o di aggiungere altre pagine senza toccare la logica di accesso al database.

  Riferimento: capitolo 23, par. 23.2 e 23.4
*/

// Content-Type JSON: il browser (e il codice JavaScript) sa che riceverà JSON
header('Content-Type: application/json');

$db_path = 'iotdata.db';
$dataPoints = array();

try {
    $db = new PDO("sqlite:" . $db_path);
    $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    // Filtriamo solo le ultime 24 ore
    $timestamp_24h_fa = time() - (24 * 60 * 60);

    // Query preparata: stessa tecnica di cap21 per evitare SQL injection
    $stmt = $db->prepare(
        "SELECT timeStamp, Temp FROM data WHERE timeStamp >= :inizio ORDER BY timeStamp ASC"
    );
    $stmt->bindParam(':inizio', $timestamp_24h_fa, PDO::PARAM_INT);
    $stmt->execute();

    $righe = $stmt->fetchAll(PDO::FETCH_ASSOC);

    foreach ($righe as $riga) {
        // x: timestamp in millisecondi (JS usa millisecondi, Unix usa secondi)
        // y: temperatura come numero float, non come stringa
        array_push($dataPoints, array(
            "x" => $riga['timeStamp'] * 1000,
            "y" => (float)$riga['Temp']
        ));
    }

    $db = null;

} catch (PDOException $e) {
    // In caso di errore, restituiamo un oggetto JSON con il messaggio di errore
    http_response_code(500);
    echo json_encode(array("error" => $e->getMessage()));
    exit;
}

// JSON_NUMERIC_CHECK garantisce che i valori numerici siano serializzati
// come numeri JSON autentici (non come stringhe tra virgolette)
echo json_encode($dataPoints, JSON_NUMERIC_CHECK);
?>
