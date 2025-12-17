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
        h2 { margin-top: 0; font-size: 1.1rem; display: flex; align-items: center; gap: 10px; }
        
        /* Composant: Bouton */ 
        .button { padding: 10px 20px; margin: 5px; border: none; border-radius: 5px; cursor: pointer; background: #4CAF50; color: white; text-decoration: none; display: inline-block; }
        .button:hover { background: #45a049; }

        /* Composant: Switch */
        .switch { position: relative; display: inline-block; width: 50px; height: 26px; }
        .switch input { opacity: 0; width: 0; height: 0; }
        .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; transition: .4s; border-radius: 34px; }
        .slider:before { position: absolute; content: ""; height: 18px; width: 18px; left: 4px; bottom: 4px; background-color: white; transition: .4s; border-radius: 50%; }
        input:checked + .slider { background-color: var(--primary); }
        input:checked + .slider:before { transform: translateX(24px); }

        /* Composant: Grid pour Sensors */
        .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; }
        .stat { background: #f9f9f9; padding: 15px; border-radius: 10px; text-align: center; }
        .stat-val { font-size: 1.5rem; font-weight: bold; color: var(--primary); }

    </style>
</head>
<body>
    <div class="container">
        <div class="card">
            <h2>⚙️ Nourrir le chat</h2>

            <div style="display:flex; justify-content: space-between; align-items:center;">
                <span>Interrupteur</span>
                <label class="switch">
                    <input type="checkbox" id="sysCheck" onchange="sendCmd('/setSys', this.checked ? 1 : 0)">
                    <span class="slider"></span>
                </label>
            </div> 

            <p>
                <a href="#" class="button" onclick="sendCmd('/feedCat', 0)">Croquinette</a>
                <a href="#" class="button" onclick="sendCmd('/feedCat', 1)">Croquette</a>
                <a href="#" class="button" onclick="sendCmd('/reset', 1)">Reset</a>
            </p>


        <div class="card">
            <h2>📊 Compteurs</h2>
            <div class="grid">
                <div class="stat">
                    <div>Croquettes</div>
                    <div class="stat-val"><span id="nbCroquettes">--</span></div>
                </div>
                <div class="stat">
                    <div>Croquinettes</div>
                    <div class="stat-val"><span id="nbCroquinettes">--</span></div>
                </div>
                <div class="stat">
                    <div>Dernières Croquettes</div>
                    <div class="stat-val"><span id="hCroquettes">--</span>min</div>
                </div>
                <div class="stat">
                    <div>Dernières Croquinettes</div>
                    <div class="stat-val"><span id="hCroquinettes">--</span>min</div>
                </div>
                <div class="stat">
                    <div>Prochaines Croquettes</div>
                    <div class="stat-val"><span id="hNextCroquettes">--</span>min</div>
                </div>
                <div class="stat">
                    <div>Délais de distribution</div>
                    <div class="stat-val"><span id="delay">--</span>min</div>
                </div>
                <div class="stat">
                    <div>Masse engloutie</div>
                    <div class="stat-val"><span id="mass">--</span>g</div>
                </div>
                <div class="stat">
                    <div>Ration idéale</div>
                    <div class="stat-val"><span id="ration">--</span>g</div>
                </div>
            </div>
        </div>
    </div>

    <script>
        function sendCmd(path, val) { fetch(path + '?v=' + val); }

        function updateUI() {
            fetch('/api/data').then(r => r.json()).then(data => {
                document.getElementById('nbCroquettes').innerText = data.nbCroquettes;
                document.getElementById('nbCroquinettes').innerText = data.nbCroquinettes;
                document.getElementById('hCroquettes').innerText = data.hCroquettes;
                document.getElementById('hCroquinettes').innerText = data.hCroquinettes;
                document.getElementById('hNextCroquettes').innerText = data.hNextCroquettes;
                document.getElementById('delay').innerText = data.delay;
                document.getElementById('mass').innerText = data.mass;
                document.getElementById('ration').innerText = data.ration;
                document.getElementById('sysCheck').checked = data.sys; 
            });
        }
        setInterval(updateUI, 2000);
        updateUI();
    </script>
</body>
</html>
)rawliteral";
        return html;
    }
};

#endif