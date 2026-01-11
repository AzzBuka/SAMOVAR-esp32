#include "web_portal.h"
#include "config.h"
#include "wifi_manager.h"

extern WebServer server;

// =====================================================
// HTML: СТРАНИЦА КОНФИГУРАЦИИ
// =====================================================
const char* htmlConfig = R"rawliteral(
<!DOCTYPE html>
<html lang='ru'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>SAMOVAR Configuration</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            padding: 40px;
            max-width: 550px;
            width: 100%;
        }
        h1 {
            color: #333;
            margin-bottom: 10px;
            font-size: 28px;
            text-align: center;
        }
        .subtitle {
            color: #666;
            text-align: center;
            margin-bottom: 30px;
            font-size: 14px;
        }
        .section-title {
            color: #667eea;
            font-size: 18px;
            font-weight: 600;
            margin: 25px 0 15px 0;
            padding-bottom: 8px;
            border-bottom: 2px solid #f0f0f0;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            color: #333;
            font-weight: 600;
            margin-bottom: 8px;
            font-size: 14px;
        }
        label .required {
            color: #f5576c;
            margin-left: 3px;
        }
        input, select {
            width: 100%;
            padding: 12px 15px;
            border: 2px solid #e0e0e0;
            border-radius: 10px;
            font-size: 16px;
            transition: all 0.3s;
        }
        input:focus, select:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        input::placeholder {
            color: #aaa;
        }
        button {
            width: 100%;
            padding: 14px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 10px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: transform 0.2s, box-shadow 0.2s;
            margin-top: 10px;
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 20px rgba(102, 126, 234, 0.4);
        }
        button:active {
            transform: translateY(0);
        }
        .info {
            background: #f0f4ff;
            border-left: 4px solid #667eea;
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 25px;
            font-size: 14px;
            color: #555;
        }
        .scan-btn {
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
            margin-bottom: 15px;
        }
        .loading {
            display: none;
            text-align: center;
            margin-top: 15px;
            color: #667eea;
        }
        .networks {
            max-height: 200px;
            overflow-y: auto;
            border: 2px solid #e0e0e0;
            border-radius: 10px;
            margin-top: 10px;
            display: none;
        }
        .network-item {
            padding: 12px 15px;
            border-bottom: 1px solid #f0f0f0;
            cursor: pointer;
            transition: background 0.2s;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .network-item:hover {
            background: #f8f9ff;
        }
        .network-item:last-child {
            border-bottom: none;
        }
        .signal {
            font-size: 12px;
            color: #999;
        }
        .help-text {
            font-size: 12px;
            color: #999;
            margin-top: 5px;
        }
        .error {
            color: #f5576c;
            font-size: 13px;
            margin-top: 5px;
            display: none;
        }
    </style>
</head>
<body>
    <div class='container'>
        <h1>🔥 SAMOVAR</h1>
        <p class='subtitle'>Первоначальная настройка системы</p>
        
        <div class='info'>
            ℹ️ Заполните все поля. После сохранения устройство подключится к WiFi и Telegram боту.
        </div>
        
        <form id='configForm' action='/save' method='POST' onsubmit='return validateForm()'>
            
            <div class='section-title'>📡 WiFi настройки</div>
            
            <button type='button' class='scan-btn' onclick='scanNetworks()'>📡 Сканировать сети</button>
            <div class='loading' id='loading'>Поиск сетей...</div>
            <div class='networks' id='networks'></div>
            
            <div class='form-group'>
                <label for='ssid'>Название сети (SSID) <span class='required'>*</span></label>
                <input type='text' id='ssid' name='ssid' required placeholder='Введите SSID'>
                <div class='error' id='ssidError'>Поле обязательно для заполнения</div>
            </div>
            
            <div class='form-group'>
                <label for='password'>Пароль WiFi <span class='required'>*</span></label>
                <input type='password' id='password' name='password' required placeholder='Введите пароль'>
                <div class='error' id='passwordError'>Поле обязательно для заполнения</div>
            </div>
            
            <div class='section-title'>💬 Telegram Bot настройки</div>
            
            <div class='form-group'>
                <label for='botToken'>Bot Token <span class='required'>*</span></label>
                <input type='text' id='botToken' name='botToken' required placeholder='123456789:ABCdefGHIjklMNOpqrsTUVwxyz'>
                <div class='help-text'>Получите у @BotFather в Telegram</div>
                <div class='error' id='botTokenError'>Поле обязательно для заполнения</div>
            </div>
            
            <div class='form-group'>
                <label for='chatID'>Chat ID <span class='required'>*</span></label>
                <input type='text' id='chatID' name='chatID' required placeholder='123456789'>
                <div class='help-text'>Узнайте у @userinfobot в Telegram</div>
                <div class='error' id='chatIDError'>Поле обязательно для заполнения</div>
            </div>
            
            <button type='submit'>💾 Сохранить и запустить</button>
        </form>
    </div>
    
    <script>
        function validateForm() {
            let isValid = true;
            const ssid = document.getElementById('ssid').value.trim();
            const password = document.getElementById('password').value.trim();
            const botToken = document.getElementById('botToken').value.trim();
            const chatID = document.getElementById('chatID').value.trim();
            
            document.querySelectorAll('.error').forEach(el => el.style.display = 'none');
            
            if(!ssid) { document.getElementById('ssidError').style.display = 'block'; isValid = false; }
            if(!password) { document.getElementById('passwordError').style.display = 'block'; isValid = false; }
            if(!botToken) { document.getElementById('botTokenError').style.display = 'block'; isValid = false; }
            if(!chatID) { document.getElementById('chatIDError').style.display = 'block'; isValid = false; }
            
            if(!isValid) alert('❌ Пожалуйста, заполните все обязательные поля!');
            return isValid;
        }
        
        function scanNetworks() {
            document.getElementById('loading').style.display = 'block';
            document.getElementById('networks').style.display = 'none';
            
            fetch('/scan')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('loading').style.display = 'none';
                    const networksDiv = document.getElementById('networks');
                    networksDiv.innerHTML = '';
                    
                    if(data.networks && data.networks.length > 0) {
                        data.networks.forEach(net => {
                            const item = document.createElement('div');
                            item.className = 'network-item';
                            item.innerHTML = `<span>${net.ssid} ${net.secured ? '🔒' : '🔓'}</span><span class='signal'>${net.rssi} dBm</span>`;
                            item.onclick = () => {
                                document.getElementById('ssid').value = net.ssid;
                                document.getElementById('password').focus();
                            };
                            networksDiv.appendChild(item);
                        });
                        networksDiv.style.display = 'block';
                    } else {
                        networksDiv.innerHTML = '<div class="network-item">Сети не найдены</div>';
                        networksDiv.style.display = 'block';
                    }
                })
                .catch(err => {
                    document.getElementById('loading').style.display = 'none';
                    alert('Ошибка сканирования: ' + err);
                });
        }
    </script>
</body>
</html>
)rawliteral";

// =====================================================
// HTML: СТРАНИЦА УСПЕШНОГО СОХРАНЕНИЯ
// =====================================================
const char* htmlSuccess = R"rawliteral(
<!DOCTYPE html>
<html lang='ru'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Настройка завершена</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Arial, sans-serif;
            background: linear-gradient(135deg, #11998e 0%, #38ef7d 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            padding: 40px;
            max-width: 500px;
            width: 100%;
            text-align: center;
        }
        .success-icon {
            font-size: 80px;
            margin-bottom: 20px;
            animation: bounce 1s ease infinite;
        }
        @keyframes bounce {
            0%, 100% { transform: translateY(0); }
            50% { transform: translateY(-10px); }
        }
        h1 { color: #333; margin-bottom: 15px; font-size: 28px; }
        p { color: #666; margin-bottom: 10px; line-height: 1.6; }
        .countdown { font-size: 48px; font-weight: bold; color: #11998e; margin: 30px 0; }
        .info-box {
            background: #f0fff4;
            border: 2px solid #38ef7d;
            border-radius: 10px;
            padding: 15px;
            margin-top: 20px;
            text-align: left;
        }
        .info-box strong { color: #11998e; }
    </style>
    <script>
        let count = 5;
        setInterval(() => {
            count--;
            document.getElementById('countdown').textContent = count;
            if(count <= 0) {
                document.body.innerHTML = '<div class="container"><h1>✅ Устройство перезагружается...</h1><p>Точка доступа будет отключена.</p><p>Проверьте Telegram бот!</p></div>';
            }
        }, 1000);
    </script>
</head>
<body>
    <div class='container'>
        <div class='success-icon'>✅</div>
        <h1>Настройки сохранены!</h1>
        <p>Устройство подключится к WiFi и Telegram боту.</p>
        <div class='countdown' id='countdown'>5</div>
        <p style='font-size: 14px; color: #999;'>Перезагрузка через <span id='countdown'>5</span> секунд...</p>
        <div class='info-box'>
            <p><strong>📱 Что делать дальше:</strong></p>
            <p>1. Переподключитесь к вашей WiFi сети</p>
            <p>2. Откройте Telegram бота</p>
            <p>3. Отправьте команду /help</p>
        </div>
    </div>
</body>
</html>
)rawliteral";

// =====================================================
// ЗАПУСК РЕЖИМА AP
// =====================================================
void startAPMode() {
  Serial.println("\n=== STARTING AP MODE ===");
  apMode = true;
  
  WiFi.mode(WIFI_AP);
  String apName = "SAMOVAR_" + String((uint32_t)ESP.getEfuseMac(), HEX);
  WiFi.softAP(apName.c_str(), AP_PASSWORD);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.println("AP Name: " + apName);
  Serial.println("AP Password: " + String(AP_PASSWORD));
  Serial.println("AP IP: " + IP.toString());
  Serial.println("Open browser: http://" + IP.toString());
  
  // Главная страница
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlConfig);
  });
  
  // Сканирование сетей
  server.on("/scan", HTTP_GET, []() {
    Serial.println("Scanning networks...");
    int n = WiFi.scanNetworks();
    String json = "{\"networks\":[";
    
    for(int i = 0; i < n; i++) {
      if(i > 0) json += ",";
      json += "{";
      json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
      json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
      json += "\"secured\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false");
      json += "}";
    }
    json += "]}";
    
    server.send(200, "application/json", json);
    WiFi.scanDelete();
  });
  
  // Сохранение настроек
  server.on("/save", HTTP_POST, []() {
    if(server.hasArg("ssid") && server.hasArg("password") && 
       server.hasArg("botToken") && server.hasArg("chatID")) {
      
      String ssid = server.arg("ssid");
      String password = server.arg("password");
      String token = server.arg("botToken");
      String chat = server.arg("chatID");
      
      if(ssid.length() == 0 || password.length() == 0 || 
         token.length() == 0 || chat.length() == 0) {
        server.send(400, "text/plain", "All fields are required!");
        return;
      }
      
      Serial.println("Received configuration:");
      Serial.println("  SSID: " + ssid);
      Serial.println("  Bot Token: ***");
      Serial.println("  Chat ID: " + chat);
      
      saveSystemConfig(ssid, password, token, chat);
      
      server.send(200, "text/html", htmlSuccess);
      
      delay(5000);
      ESP.restart();
    } else {
      server.send(400, "text/plain", "Missing required fields!");
    }
  });
  
  server.begin();
  Serial.println("Web server started");
}
