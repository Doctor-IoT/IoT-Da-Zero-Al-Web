<?php
/*
  cap11 — Visualizzazione di dati ricevuti via GET
  ==================================================
  Pagina PHP che riceve parametri via GET e li mostra
  in una pagina HTML formattata nel browser.

  URL di esempio:
    http://192.168.1.50/mostra_dati.php?temp=23.5&umid=60&rssi=-61

  Riferimento: capitolo 11, par. 11.5 e 11.9
*/

// Lettura e validazione dei parametri
$temp = isset($_GET['temp']) ? (float)$_GET['temp'] : null;
$umid = isset($_GET['umid']) ? (float)$_GET['umid'] : null;
$rssi = isset($_GET['rssi']) ? (int)$_GET['rssi']   : null;
$ora  = date("H:i:s");
?>
<!DOCTYPE html>
<html lang="it">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Dati ricevuti — IoT Lab</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 600px; margin: 40px auto; padding: 0 20px; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ccc; padding: 8px 12px; text-align: left; }
        th { background-color: #f0f0f0; }
        .ora { color: #666; font-size: 0.9em; margin-top: 10px; }
        .errore { color: red; }
    </style>
</head>
<body>
<h1>Dati ricevuti dalla NodeMCU</h1>

<?php if ($temp !== null || $umid !== null || $rssi !== null): ?>
<table>
    <tr><th>Grandezza</th><th>Valore</th></tr>
    <?php if ($temp !== null): ?>
    <tr><td>Temperatura</td><td><?php echo number_format($temp, 1); ?> °C</td></tr>
    <?php endif; ?>
    <?php if ($umid !== null): ?>
    <tr><td>Umidità relativa</td><td><?php echo number_format($umid, 1); ?> %</td></tr>
    <?php endif; ?>
    <?php if ($rssi !== null): ?>
    <tr><td>Wi-Fi RSSI</td><td><?php echo $rssi; ?> dBm</td></tr>
    <?php endif; ?>
</table>
<p class="ora">Ricevuto alle <?php echo $ora; ?></p>

<?php else: ?>
<p class="errore">Nessun dato ricevuto.<br>
Usare: <code>?temp=23.5&amp;umid=60&amp;rssi=-61</code></p>
<?php endif; ?>

</body>
</html>
