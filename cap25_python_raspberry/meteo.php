<?php
/**
 * meteo.php — Dashboard stazione meteo con storico 24 ore e grafico.
 * ====================================================================
 * Requisiti:
 *   - Apache 2 + PHP 8 con estensione sqlite3
 *   - Database /var/local/meteo.db (creato e popolato da dht11_logger.py)
 *   - File canvasjs.min.js nella stessa directory /var/www/html/
 *     (scaricabile da canvasjs.com/download — versione gratuita per uso non commerciale)
 *
 * Installazione:
 *   sudo apt install apache2 php libapache2-mod-php -y
 *   sudo cp meteo.php /var/www/html/
 *   sudo cp canvasjs.min.js /var/www/html/
 *
 * URL di accesso (da qualsiasi dispositivo nella rete locale):
 *   http://<indirizzo_IP_raspberry>/meteo.php
 *   # L'indirizzo IP si ottiene con: hostname -I
 *
 * Riferimento: capitolo 25, par. 25.7 e 25.9
 */

define('DB_PATH',     '/var/local/meteo.db');
define('ORE_STORIA',  24);   // finestra temporale visualizzata

// ── Connessione al database ───────────────────────────────────────────────────
$db = new SQLite3(DB_PATH, SQLITE3_OPEN_READONLY);

// ── Statistiche aggregate (ultimi ORE_STORIA ore) ────────────────────────────
$da = time() - ORE_STORIA * 3600;

$agg = $db->querySingle("
    SELECT
        ROUND(AVG(temperatura), 1) AS t_media,
        ROUND(MIN(temperatura), 1) AS t_min,
        ROUND(MAX(temperatura), 1) AS t_max,
        ROUND(AVG(umidita),     1) AS u_media,
        ROUND(MIN(umidita),     1) AS u_min,
        ROUND(MAX(umidita),     1) AS u_max,
        COUNT(*)                   AS n_misurazioni
    FROM misurazioni
    WHERE ts >= $da
", true);

// ── Dati per il grafico (campionati ogni 30 minuti) ───────────────────────────
// Raggruppiamo le misurazioni in blocchi da 1800 secondi con ts / 1800.
// SQLite tronca all'intero inferiore, quindi tutte le righe con lo stesso
// quoziente appartengono alla stessa finestra di 30 minuti.
$res = $db->query("
    SELECT
        (ts / 1800) * 1800          AS ts_slot,
        ROUND(AVG(temperatura), 1)  AS temp,
        ROUND(AVG(umidita),     1)  AS umid
    FROM misurazioni
    WHERE ts >= $da
    GROUP BY ts / 1800
    ORDER BY ts_slot
");

// CanvasJS usa oggetti {x: timestamp_ms, y: valore} — non semplici array di valori.
// Il timestamp va moltiplicato per 1000: JavaScript (e CanvasJS) lavora in millisecondi.
$temp_points = $umid_points = [];
while ($row = $res->fetchArray(SQLITE3_ASSOC)) {
    $ms = $row['ts_slot'] * 1000;              // Unix seconds → JavaScript ms
    $temp_points[] = ['x' => $ms, 'y' => (float)$row['temp']];
    $umid_points[] = ['x' => $ms, 'y' => (float)$row['umid']];
}
$db->close();

// Serializzazione in JSON per CanvasJS (JSON_NUMERIC_CHECK evita che i float
// vengano serializzati come stringhe)
$j_temp_points = json_encode($temp_points, JSON_NUMERIC_CHECK);
$j_umid_points = json_encode($umid_points, JSON_NUMERIC_CHECK);

// ── Ultime N misurazioni per la tabella ──────────────────────────────────────
$db2 = new SQLite3(DB_PATH, SQLITE3_OPEN_READONLY);
$ultimi = $db2->query("
    SELECT ts, temperatura, umidita
    FROM misurazioni
    ORDER BY ts DESC
    LIMIT 10
");
$db2->close();
?>
<!DOCTYPE html>
<html lang="it">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Stazione Meteo — Raspberry Pi</title>
  <style>
    body { font-family: Arial, sans-serif; background: #f0f4f8; color: #2d3748; margin: 0; }
    header { background: #1a365d; color: white; padding: 16px 24px;
             display: flex; justify-content: space-between; align-items: center; }
    header h1 { font-size: 20px; margin: 0; }
    header .ts { font-size: 12px; opacity: 0.8; }
    .main  { padding: 18px 24px; max-width: 960px; margin: 0 auto; }
    .cards { display: flex; gap: 14px; margin-bottom: 18px; }
    .card  { flex: 1; background: white; border-radius: 8px; padding: 16px;
             box-shadow: 0 1px 4px rgba(0,0,0,0.1); }
    .card .lbl { font-size: 11px; text-transform: uppercase; color: #718096; }
    .card .val { font-size: 30px; font-weight: 700; margin: 6px 0 3px; color: #e53e3e; }
    .card.umid .val { color: #2b6cb0; }
    .card.cnt  .val { color: #38a169; font-size: 26px; }
    .card .rng { font-size: 11px; color: #718096; }
    .box   { background: white; border-radius: 8px; padding: 18px 22px;
             box-shadow: 0 1px 4px rgba(0,0,0,0.1); margin-bottom: 16px; }
    .box h2 { font-size: 14px; color: #4a5568; margin-bottom: 14px; }
    table  { width: 100%; border-collapse: collapse; font-size: 13px; }
    th     { background: #edf2f7; padding: 8px 12px; text-align: left; font-size: 12px; }
    td     { padding: 7px 12px; border-bottom: 1px solid #f0f4f8; }
    .tc { color: #e53e3e; font-weight: 600; }
    .uc { color: #2b6cb0; font-weight: 600; }
  </style>
</head>
<body>

<header>
  <h1>🌡 Stazione Meteo &mdash; Raspberry Pi</h1>
  <div class="ts">Aggiornato: <?= date('d/m/Y H:i:s') ?> &bull; <?= ORE_STORIA ?>h di storico</div>
</header>

<div class="main">

  <!-- Schede KPI -->
  <div class="cards">
    <div class="card">
      <div class="lbl">Temperatura media (<?= ORE_STORIA ?>h)</div>
      <div class="val"><?= $agg['t_media'] ?>°C</div>
      <div class="rng">Min <?= $agg['t_min'] ?>°C &nbsp;·&nbsp; Max <?= $agg['t_max'] ?>°C</div>
    </div>
    <div class="card umid">
      <div class="lbl">Umidità relativa media (<?= ORE_STORIA ?>h)</div>
      <div class="val"><?= $agg['u_media'] ?>%</div>
      <div class="rng">Min <?= $agg['u_min'] ?>% &nbsp;·&nbsp; Max <?= $agg['u_max'] ?>% RH</div>
    </div>
    <div class="card cnt">
      <div class="lbl">Misurazioni registrate</div>
      <div class="val"><?= number_format($agg['n_misurazioni'], 0, ',', ' ') ?></div>
      <div class="rng">nelle ultime <?= ORE_STORIA ?> ore (1 al minuto)</div>
    </div>
  </div>

  <!-- Grafico CanvasJS con doppio asse Y (temperatura e umidità) -->
  <div class="box">
    <h2>Andamento temperatura e umidità &mdash; ultime <?= ORE_STORIA ?> ore</h2>
    <!-- CanvasJS disegnerà il grafico in questo div; l'altezza va specificata -->
    <div id="chartContainer" style="height: 380px; width: 100%;"></div>
  </div>

  <!-- Tabella ultime misurazioni -->
  <div class="box">
    <h2>Ultime 10 misurazioni</h2>
    <table>
      <thead><tr><th>Orario</th><th>Temperatura</th><th>Umidità RH</th></tr></thead>
      <tbody>
        <?php while ($r = $ultimi->fetchArray(SQLITE3_ASSOC)): ?>
        <tr>
          <td><?= date('H:i', $r['ts']) ?></td>
          <td class="tc"><?= $r['temperatura'] ?>°C</td>
          <td class="uc"><?= $r['umidita'] ?>%</td>
        </tr>
        <?php endwhile; ?>
      </tbody>
    </table>
  </div>

</div>

<script>
  window.onload = function () {
    var chart = new CanvasJS.Chart("chartContainer", {
      theme: "light1",
      animationEnabled: true,
      zoomEnabled: true,         // zoom con selezione mouse
      title: { text: "Andamento ultime <?= ORE_STORIA ?> ore" },
      axisX: {
        title: "Orario",
        valueFormatString: "HH:mm"   // mostra i timestamp come "09:30"
      },
      axisY: {                        // asse Y sinistro — temperatura
        title: "Temperatura (°C)",
        suffix: "°C"
      },
      axisY2: {                       // asse Y destro — umidità
        title: "Umidità (% RH)",
        suffix: "%"
      },
      legend: { cursor: "pointer" },
      data: [
        {
          type: "spline",
          name: "Temperatura",
          showInLegend: true,
          xValueType: "dateTime",     // i valori x sono timestamp JavaScript (ms)
          color: "#e53e3e",
          dataPoints: <?= $j_temp_points ?>
        },
        {
          type: "spline",
          name: "Umidità",
          axisYType: "secondary",     // collega al secondo asse Y (axisY2)
          showInLegend: true,
          xValueType: "dateTime",
          color: "#2b6cb0",
          dataPoints: <?= $j_umid_points ?>
        }
      ]
    });
    chart.render();
  }
</script>

<!-- Libreria CanvasJS: file locale nella stessa cartella della pagina PHP.
     Scaricabile gratuitamente (uso non commerciale) da: canvasjs.com/download  -->
<script src="canvasjs.min.js"></script>
</body>
</html>
