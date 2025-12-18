// includes/DashboardPage.h

#ifndef WEB_UI_H
#define WEB_UI_H

#include <Arduino.h>

class DashboardPage
{
public:
    static String getHTML()
    {
        String html = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP Control Panel</title>
    <style>
        :root {
            --primary: #2196F3;
            --bg: #f4f7f6;
            --card-bg: #ffffff;
            --text: #333;
        }
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: var(--bg); color: var(--text); margin: 0; padding: 20px; }
        .container { max-width: 600px; margin: 0 auto; }
        .card { background: var(--card-bg); padding: 20px; border-radius: 15px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); margin-bottom: 20px; }
        h1 { text-align: center; color: var(--primary); }
        h2 { margin-top: 0; font-size: 1.1rem; display: flex; align-items: center; gap: 10px; }
        
        /* Composant: Bouton */ 
        .button { padding: 10px 20px; margin: 5px; border: none; border-radius: 5px; cursor: pointer; background: var(--primary); color: white; text-decoration: none; display: inline-block; }
        .button:hover { background: #45a049; }

        /* Composant: Switch */
        .switch { position: relative; display: inline-block; width: 50px; height: 26px; }
        .switch input { opacity: 0; width: 0; height: 0; }
        .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; transition: .4s; border-radius: 34px; }
        .slider:before { position: absolute; content: ""; height: 18px; width: 18px; left: 4px; bottom: 4px; background-color: white; transition: .4s; border-radius: 50%; }
        input:checked + .slider { background-color: var(--primary); }
        input:checked + .slider:before { transform: translateX(24px); }
        
        /* Composant: TimePicker */
        input[type="time"] { border: 1px solid #ddd; border-radius: 5px; padding: 5px; font-family: inherit; }
        .time-row { display: flex; justify-content: space-between; align-items: center; margin: 10px 0; }

        /* Composant: Grid pour Data */
        .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; }
        .stat { background: #f9f9f9; padding: 15px; border-radius: 10px; text-align: center; }
        .stat-val { font-size: 1.5rem; font-weight: bold; color: var(--primary); }

        /* Composant: Chart pour historique */
        /* Grille et Axes du Chart */
        .chart-container { 
            width: 100%; height: 200px; background: #fff; 
            position: relative; margin-top: 30px; margin-bottom: 20px;
        }
        .chart-line { fill: none; stroke: var(--primary); stroke-width: 3; stroke-linejoin: round; }
        .chart-area { fill: rgba(33, 149, 243, 0.49); }
        .grid-line { stroke: #eee; stroke-width: 1; stroke-dasharray: 4; }
        .axis-text { font-size: 3px; fill: #888; font-weight: bold; }

    </style>
</head>
<body>
    <div class="container">
        <div class="card">
        <h1>Programme Fit'Gazou 🐈</h1>      

        <div class="card">
            <h2>📊 Compteurs</h2>
            <div class="grid">
                <div class="stat"><div>Croquettes</div><div class="stat-val" id="nbCroquettes">--</div></div>
                <div class="stat"><div>Croquinettes</div><div class="stat-val" id="nbCroquinettes">--</div></div>
                <div class="stat"><div>Masse engloutie</div><div class="stat-val"><span id="mass">--</span>g</div></div>
                <div class="stat"><div>Ration cible</div><div class="stat-val"><span id="ration">--</span>g</div></div>
            </div>
        </div>         

        <div class="card">
            <h2>⏰ Temps de passage</h2>
            <div class="grid">
                <div class="stat"><div>Dernière distribution</div><div class="stat-val" id="hCroquettes">--</div></div>
                <div class="stat"><div>Dernière croquinettes</div><div class="stat-val" id="hCroquinettes">--</div></div>
                <div class="stat"><div>Prochaine distribution</div><div class="stat-val" id="hNextCroquettes">--</div></div>
                <div class="stat"><div>Cycle actuel</div><div class="stat-val" id="delay">--</div></div>
            </div>
        </div>

        <div class="card">
            <h2>📈 Évolution de la ration</h2>
            <div class="chart-container" id="chartBox">
                <svg id="feedingChart" viewBox="-10 -5 115 115" preserveAspectRatio="none" style="width:100%; height:100%; overflow: visible;">
                    <g id="chartGrid"></g>
                    <line x1="0" y1="100" x2="100" y2="100" stroke="#ccc" stroke-width="0.5"/>
                    <line x1="0" y1="0" x2="0" y2="100" stroke="#ccc" stroke-width="0.5"/>
                    <path class="chart-area" id="chartArea" d=""></path>
                    <polyline class="chart-line" id="chartLine" points=""></polyline>
                </svg>
            </div>
        </div>

        <div class="card">
            <h2>⚙️ Paramètres & Contrôles</h2>

            <p>
                <a href="#" class="button" onclick="sendCmd('/feedCat', 0)">Croquinette</a>
                <a href="#" class="button" onclick="sendCmd('/feedCat', 1)">Croquette</a>
                <a href="#" class="button" onclick="if(confirm('Reset ?')) sendCmd('/reset', 1)">Reset</a>
            </p>
            
            <hr>
            
            <div class="time-row">
                <span>Distribution automatique</span>
                <label class="switch">
                    <input type="checkbox" id="autoMiam" onchange="sendCmd('/setAutomiam', this.checked ? 1 : 0)">
                    <span class="slider"></span>
                </label>
            </div> 
            <div class="time-row">
                <span>Début de service</span>
                <input type="time" id="timeStart" onchange="sendTime('start', this.value)">
            </div>
            <div class="time-row">
                <span>Fin de service</span>
                <input type="time" id="timeEnd" onchange="sendTime('end', this.value)">
            </div>

        </div>

    </div>

    <script>

        function sendCmd(path, val) { fetch(path + '?v=' + val); }

        function sendTime(type, val) {
            fetch('/setMiamTime?type=' + type + '&val=' + val);
        }

        function updateUI() {
            fetch('/api/data').then(r => r.json()).then(data => {
                for (let key in data) {
                        let el = document.getElementById(key);
                        if (el) {
                            if (el.type === 'checkbox') el.checked = data[key];
                            else if (el.type === 'time') {
                                // On ne met à jour l'input que s'il n'est pas en train d'être modifié
                                if (document.activeElement !== el) el.value = data[key];
                            }
                            else el.innerText = data[key];
                        }
                    }
                // --- LOGIQUE DU GRAPHIQUE ---
                if (data.history && data.timeStart && data.timeEnd) {
                    const rationMax = data.ration + 10 || 100;
                    
                    // Conversion des bornes temporelles en secondes
                    const getSec = (t) => { let s = t.split(':'); return parseInt(s[0])*3600 + parseInt(s[1])*60; };
                    const tStart = getSec(data.timeStart);
                    const tEnd = getSec(data.timeEnd);
                    const tRange = tEnd - tStart;

                    // 1. Dessiner la grille
                    let gridHTML = '';
                    // Abscisses : Chaque heure
                    let hStart = parseInt(data.timeStart.split(':')[0]);
                    let hEnd = parseInt(data.timeEnd.split(':')[0]);
                    for (let h = hStart; h <= hEnd; h++) {
                        let x = ((h * 3600 - tStart) / tRange) * 100;
                        if (x >= 0 && x <= 100) {
                            gridHTML += `<line class="grid-line" x1="${x}" y1="0" x2="${x}" y2="100"></line>`;
                            gridHTML += `<text class="axis-text" x="${x}" y="105" text-anchor="middle">${h}h</text>`;
                        }
                    }
                    // Ordonnées : Graduations tous les 10g
                    for (let g = 0; g <= rationMax; g += 10) {
                        let y = 100 - (g / rationMax * 100);
                        gridHTML += `<line class="grid-line" x1="0" y1="${y}" x2="100" y2="${y}"></line>`;
                        gridHTML += `<text class="axis-text" x="-2" y="${y + 1}" text-anchor="end">${g}g</text>`;
                    }
                    document.getElementById('chartGrid').innerHTML = gridHTML;

                    // 2. Calcul des points
                    const pointsArray = data.history.map(p => {
                        let x = ((p.t - tStart) / tRange) * 100;
                        let y = 100 - (p.m / rationMax * 100);
                        // On bride X entre 0 et 100 pour rester dans la plage de miam
                        return { x: Math.max(0, Math.min(100, x)), y: y, m: p.m, t: p.t };
                    });

                    const pointsStr = pointsArray.map(p => `${p.x},${p.y}`).join(" ");
                    document.getElementById('chartLine').setAttribute("points", pointsStr);
                    
                    if(pointsArray.length > 0) {
                        const areaPath = `M 0,100 L ${pointsStr} L ${pointsArray[pointsArray.length-1].x},100 Z`;
                        document.getElementById('chartArea').setAttribute("d", areaPath);
                    }
                }
                // --- FIN LOGIQUE DU GRAPHIQUE ---
            });
        }
        setInterval(updateUI, 1000);
        updateUI();
    </script>
</body>
</html>
)rawliteral";
        return html;
    }
};

#endif