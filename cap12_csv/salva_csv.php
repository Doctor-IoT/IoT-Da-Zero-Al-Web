<?php
/*
  cap12 — Salvataggio misure su file CSV
  ========================================
  Script PHP che riceve temperatura e umidità via GET e le aggiunge,
  insieme al timestamp, in un file CSV sul server (modalità append).

  URL di esempio:
    http://192.168.1.50/salva_csv.php?temp=23.5&umid=60

  Il file misure.csv viene creato nella stessa cartella dello script
  se non esiste ancora; ogni nuova richiesta aggiunge una riga in fondo.

  Nota sui permessi: la cartella deve essere scrivibile dal processo
  del server web (es. www-data su Linux):
    chown www-data:www-data /var/www/html/

  Riferimento: capitolo 12, par. 12.4
*/

$fileCSV = "misure.csv";

// Verifica presenza parametri
if (!isset($_GET['temp']) || !isset($_GET['umid'])) {
    http_response_code(400);
    echo "ERRORE - Parametri mancanti. Usare: ?temp=valore&umid=valore";
    exit;
}

$temp = (float)$_GET['temp'];
$umid = (float)$_GET['umid'];

// Apertura in modalità append (a):
//   - se il file non esiste, lo crea;
//   - se esiste, si posiziona in fondo senza cancellare nulla.
// Usare "w" per errore cancellerebbe tutti i dati accumulati!
$file = fopen($fileCSV, "a");
if ($file === false) {
    http_response_code(500);
    echo "ERRORE - Impossibile aprire il file CSV. Verificare i permessi.";
    exit;
}

// fputcsv() scrive l'array come riga CSV corretta,
// gestendo automaticamente virgolette e separatori.
$riga = array(
    date("Y-m-d H:i:s"),  // timestamp ISO 8601, generato lato server
    $temp,
    $umid
);
fputcsv($file, $riga);
fclose($file);

// Risposta di conferma — letta dalla NodeMCU con http.getString()
echo "OK - Misura salvata: temp=" . $temp . ", umid=" . $umid;
echo " alle " . date("H:i:s");
?>
