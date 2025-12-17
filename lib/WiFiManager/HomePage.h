#ifndef HOME_PAGE_H
#define HOME_PAGE_H

#include <Arduino.h>

class HomePage
{
public:
  static String getHTML()
  {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP Controller</title>
  <style>
    body { font-family: Arial; margin: 20px; background: #f5f5f5; }
    .container { max-width: 800px; margin: 0 auto; }
    .card { background: white; padding: 20px; margin: 10px 0; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    h1 { color: #333; }
    .button { padding: 10px 20px; margin: 5px; border: none; border-radius: 5px; cursor: pointer; background: #4CAF50; color: white; text-decoration: none; display: inline-block; }
    .button:hover { background: #45a049; }
  </style>
</head>
<body>
  <div class="container">
    <div class="card">
      <h1>🌐 pSyLab 🌐</h1>
      <p>Contrôlleur ESP</p>
    </div>
    <div class="card">
      <h2>ℹ️ Pages disponibles</h2>
      <p>
        <a href="/status" class="button">Status JSON</a>
        <a href="/info" class="button">Infos WiFi</a>
        <a href="/scan" class="button">Scan Réseaux</a>
        <a href="/restart" class="button">Redémarrer ESP</a>
      </p>
    </div>
  </div>
</body>
</html>
)rawliteral";
    return html;
  }
};

#endif