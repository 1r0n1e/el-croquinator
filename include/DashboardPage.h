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

        /* Composant: Grid pour Sensors */
        .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; }
        .stat { background: #f9f9f9; padding: 15px; border-radius: 10px; text-align: center; }
        .stat-val { font-size: 1.5rem; font-weight: bold; color: var(--primary); }

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