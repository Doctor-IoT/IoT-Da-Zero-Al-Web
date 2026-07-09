<?php
/*
  cap11 — Ricezione di dati via GET
  ===================================
  Script PHP che riceve parametri da una richiesta HTTP GET (inviata
  dalla NodeMCU oppure digitando direttamente l'URL nel browser) e
  restituisce una stringa di conferma con i valori ricevuti.

  URL di esempio:
    http://192.168.1.50/ricevi_dati.php?temp=23.5&umid=60

  Riferimento: capitolo 11, par. 11.5 e 11.9
*/

// isset() verifica che il parametro sia presente nella richiesta
// prima di tentare di leggerlo — evita avvisi PHP per parametri mancanti.
if (isset($_GET['temp']) && isset($_GET['umid'])) {

    // I valori arrivano sempre come stringhe; (float) li converte in numeri.
    $temp = (float)$_GET['temp'];
    $umid = (float)$_GET['umid'];

    // Verifica di plausibilità (opzionale ma consigliata)
    if ($temp < -40 || $temp > 85) {
        http_response_code(400);
        echo "ERRORE - Temperatura fuori range: " . $temp;
        exit;
    }
    if ($umid < 0 || $umid > 100) {
        http_response_code(400);
        echo "ERRORE - Umidità fuori range: " . $umid;
        exit;
    }

    // Risposta di conferma — questa stringa è il corpo HTTP che
    // la NodeMCU legge con http.getString().
    echo "OK - Ricevuto: temperatura=" . $temp . ", umidità=" . $umid;
    echo " alle " . date("H:i:s");

} else {
    // Parametri mancanti: risposta con codice 400 Bad Request
    http_response_code(400);
    echo "ERRORE - Parametri mancanti. Usare: ?temp=valore&umid=valore";
}
?>
