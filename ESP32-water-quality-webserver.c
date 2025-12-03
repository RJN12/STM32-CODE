/*
 * ESP32 Water Quality Monitor Web Server
 * 
 * Receives data from STM32 via UART and hosts a web page
 * Displays real-time TDS, pH, and Turbidity readings
 * 
 * Hardware Connections:
 * STM32 TX (USART3) → ESP32 RX (GPIO16/RX2)
 * STM32 RX (USART3) → ESP32 TX (GPIO17/TX2)
 * STM32 GND → ESP32 GND
 * 
 * Author: test
 * Date: 2025
 */

#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>

// ========================================
// WiFi Configuration
// ========================================
const char* ssid = "xxxxxx";           // Change this to your WiFi name
const char* password = "xxxxxx";   // Change this to your WiFi password

// ========================================
// UART Configuration
// ========================================
HardwareSerial STM32Serial(2);  // Use UART2 on ESP32
#define RXD2 16  // GPIO16 (RX) - Connect to STM32 TX
#define TXD2 17  // GPIO17 (TX) - Connect to STM32 RX

// ========================================
// Web Server
// ========================================
WebServer server(80);

// ========================================
// Data Storage
// ========================================
struct WaterQualityData {
    int tds;
    float ph;
    int turbidity;
    String quality;
    String lastUpdate;
    bool dataValid;
} currentData;

// ========================================
// HTML Web Page
// ========================================
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Water Quality Monitor</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        
        .header {
            background: white;
            padding: 30px;
            border-radius: 15px;
            text-align: center;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            margin-bottom: 30px;
        }
        
        .header h1 {
            color: #2d3748;
            font-size: 36px;
            margin-bottom: 10px;
        }
        
        .header p {
            color: #718096;
            font-size: 18px;
        }
        
        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-left: 10px;
            animation: pulse 2s infinite;
        }
        
        .status-online { background: #48bb78; }
        .status-offline { background: #f56565; }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        
        .cards-container {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
            gap: 25px;
            margin-bottom: 30px;
        }
        
        .card {
            background: white;
            padding: 30px;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            text-align: center;
            transition: transform 0.3s;
        }
        
        .card:hover {
            transform: translateY(-5px);
        }
        
        .card-icon {
            font-size: 48px;
            margin-bottom: 15px;
        }
        
        .card-label {
            color: #718096;
            font-size: 16px;
            font-weight: 600;
            margin-bottom: 10px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .card-value {
            color: #2d3748;
            font-size: 48px;
            font-weight: 700;
            margin-bottom: 5px;
        }
        
        .card-unit {
            color: #a0aec0;
            font-size: 18px;
        }
        
        .card-tds { border-top: 5px solid #3b82f6; }
        .card-ph { border-top: 5px solid #ec4899; }
        .card-turbidity { border-top: 5px solid #f59e0b; }
        
        .quality-card {
            background: white;
            padding: 40px;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            text-align: center;
            margin-bottom: 30px;
        }
        
        .quality-status {
            font-size: 64px;
            font-weight: 900;
            margin: 20px 0;
            text-transform: uppercase;
        }
        
        .quality-excellent { color: #22c55e; }
        .quality-good { color: #3b82f6; }
        .quality-fair { color: #f59e0b; }
        .quality-bad { color: #ef4444; }
        
        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 15px;
            margin-top: 20px;
        }
        
        .info-item {
            background: #f7fafc;
            padding: 15px;
            border-radius: 10px;
        }
        
        .info-label {
            color: #718096;
            font-size: 14px;
            margin-bottom: 5px;
        }
        
        .info-value {
            color: #2d3748;
            font-size: 18px;
            font-weight: 600;
        }
        
        .footer {
            text-align: center;
            color: white;
            margin-top: 30px;
            padding: 20px;
        }
        
        .refresh-btn {
            background: #3b82f6;
            color: white;
            border: none;
            padding: 15px 40px;
            border-radius: 10px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            margin: 20px 0;
            transition: all 0.3s;
        }
        
        .refresh-btn:hover {
            background: #2563eb;
            transform: scale(1.05);
        }
        
        @media (max-width: 768px) {
            .header h1 { font-size: 28px; }
            .card-value { font-size: 36px; }
            .quality-status { font-size: 48px; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🌊 Water Quality Monitor</h1>
            <p>Real-time monitoring from STM32F767ZI
                <span class="status-indicator status-online" id="statusDot"></span>
            </p>
        </div>
        
        <div class="cards-container">
            <div class="card card-tds">
                <div class="card-icon">💧</div>
                <div class="card-label">TDS</div>
                <div class="card-value" id="tdsValue">--</div>
                <div class="card-unit">ppm</div>
            </div>
            
            <div class="card card-ph">
                <div class="card-icon">⚗️</div>
                <div class="card-label">pH Level</div>
                <div class="card-value" id="phValue">--</div>
                <div class="card-unit">pH</div>
            </div>
            
            <div class="card card-turbidity">
                <div class="card-icon">🌊</div>
                <div class="card-label">Turbidity</div>
                <div class="card-value" id="turbidityValue">--</div>
                <div class="card-unit">NTU</div>
            </div>
        </div>
        
        <div class="quality-card">
            <h2 style="color: #718096; margin-bottom: 10px;">Water Quality Status</h2>
            <div class="quality-status" id="qualityStatus">LOADING...</div>
            
            <div class="info-grid">
                <div class="info-item">
                    <div class="info-label">Last Updated</div>
                    <div class="info-value" id="lastUpdate">--</div>
                </div>
                <div class="info-item">
                    <div class="info-label">Device</div>
                    <div class="info-value">STM32F767ZI</div>
                </div>
                <div class="info-item">
                    <div class="info-label">Connection</div>
                    <div class="info-value">UART 115200</div>
                </div>
            </div>
            
            <button class="refresh-btn" onclick="fetchData()">🔄 Refresh Data</button>
        </div>
        
        <div class="footer">
            <p>Powered by ESP32 | Data refresh every 2 seconds</p>
            <p style="margin-top: 10px; font-size: 14px; opacity: 0.8;">
                © 2025 Water Quality Monitoring System
            </p>
        </div>
    </div>
    
    <script>
        function fetchData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    // Update values
                    document.getElementById('tdsValue').textContent = data.tds;
                    document.getElementById('phValue').textContent = data.ph;
                    document.getElementById('turbidityValue').textContent = data.turbidity;
                    document.getElementById('qualityStatus').textContent = data.quality;
                    document.getElementById('lastUpdate').textContent = data.lastUpdate;
                    
                    // Update quality status color
                    const statusElement = document.getElementById('qualityStatus');
                    statusElement.className = 'quality-status';
                    
                    if (data.quality === 'EXCELLENT') {
                        statusElement.classList.add('quality-excellent');
                    } else if (data.quality === 'GOOD') {
                        statusElement.classList.add('quality-good');
                    } else if (data.quality === 'FAIR') {
                        statusElement.classList.add('quality-fair');
                    } else if (data.quality === 'BAD') {
                        statusElement.classList.add('quality-bad');
                    }
                    
                    // Update status indicator
                    const statusDot = document.getElementById('statusDot');
                    if (data.dataValid) {
                        statusDot.className = 'status-indicator status-online';
                    } else {
                        statusDot.className = 'status-indicator status-offline';
                    }
                })
                .catch(error => {
                    console.error('Error fetching data:', error);
                    document.getElementById('statusDot').className = 'status-indicator status-offline';
                });
        }
        
        // Auto-refresh every 2 seconds
        setInterval(fetchData, 2000);
        
        // Initial load
        fetchData();
    </script>
</body>
</html>
)rawliteral";

// ========================================
// Function Prototypes
// ========================================
void handleRoot();
void handleData();
void parseSTM32Data(String data);
void connectToWiFi();

// ========================================
// Setup Function
// ========================================
void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=================================");
    Serial.println("ESP32 Water Quality Web Server");
    Serial.println("=================================\n");
    
    // Initialize UART for STM32 communication
    STM32Serial.begin(115200, SERIAL_8N1, RXD2, TXD2);
    Serial.println("✓ UART initialized (115200 baud)");
    Serial.printf("  RX Pin: GPIO%d\n", RXD2);
    Serial.printf("  TX Pin: GPIO%d\n\n", TXD2);
    
    // Initialize data
    currentData.tds = 0;
    currentData.ph = 0.0;
    currentData.turbidity = 0;
    currentData.quality = "WAITING";
    currentData.lastUpdate = "Never";
    currentData.dataValid = false;
    
    // Connect to WiFi
    connectToWiFi();
    
    // Setup web server routes
    server.on("/", handleRoot);
    server.on("/data", handleData);
    
    // Start server
    server.begin();
    Serial.println("\n✓ Web server started!");
    Serial.println("\n=================================");
    Serial.printf("Access the web interface at:\n");
    Serial.printf("http://%s\n", WiFi.localIP().toString().c_str());
    Serial.println("=================================\n");
}

// ========================================
// Main Loop
// ========================================
void loop() {
    // Handle web server requests
    server.handleClient();
    
    // Check for data from STM32
    if (STM32Serial.available()) {
        String receivedData = STM32Serial.readStringUntil('\n');
        receivedData.trim();
        
        if (receivedData.length() > 0) {
            Serial.println("Received from STM32: " + receivedData);
            parseSTM32Data(receivedData);
        }
    }
}

// ========================================
// Connect to WiFi
// ========================================
void connectToWiFi() {
    Serial.print("Connecting to WiFi: ");
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✓ WiFi connected successfully!");
        Serial.print("  IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("  Signal Strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
    } else {
        Serial.println("✗ WiFi connection failed!");
        Serial.println("  Please check SSID and password");
    }
}

// ========================================
// Handle Root Page Request
// ========================================
void handleRoot() {
    server.send(200, "text/html", htmlPage);
    Serial.println("Web page requested by client");
}

// ========================================
// Handle Data API Request
// ========================================
void handleData() {
    // Create JSON response
    String json = "{";
    json += "\"tds\":" + String(currentData.tds) + ",";
    json += "\"ph\":" + String(currentData.ph, 2) + ",";
    json += "\"turbidity\":" + String(currentData.turbidity) + ",";
    json += "\"quality\":\"" + currentData.quality + "\",";
    json += "\"lastUpdate\":\"" + currentData.lastUpdate + "\",";
    json += "\"dataValid\":" + String(currentData.dataValid ? "true" : "false");
    json += "}";
    
    server.send(200, "application/json", json);
}

// ========================================
// Parse Data from STM32
// ========================================
void parseSTM32Data(String data) {
    // Expected format: "TDS:  19 ppm | pH:10.97 | Turb:  80 NTU | GOOD"
    
    int tdsIndex = data.indexOf("TDS:");
    int phIndex = data.indexOf("pH:");
    int turbIndex = data.indexOf("Turb:");
    int qualityIndex = data.lastIndexOf("|");
    
    if (tdsIndex != -1 && phIndex != -1 && turbIndex != -1 && qualityIndex != -1) {
        // Extract TDS value
        String tdsStr = data.substring(tdsIndex + 4, data.indexOf("ppm"));
        tdsStr.trim();
        currentData.tds = tdsStr.toInt();
        
        // Extract pH value
        String phStr = data.substring(phIndex + 3, data.indexOf("|", phIndex));
        phStr.trim();
        currentData.ph = phStr.toFloat();
        
        // Extract Turbidity value
        String turbStr = data.substring(turbIndex + 5, data.indexOf("NTU"));
        turbStr.trim();
        currentData.turbidity = turbStr.toInt();
        
        // Extract Quality status
        String qualityStr = data.substring(qualityIndex + 1);
        qualityStr.trim();
        currentData.quality = qualityStr;
        
        // Update timestamp
        currentData.lastUpdate = String(millis() / 1000) + "s ago";
        currentData.dataValid = true;
        
        // Debug output
        Serial.println("\n--- Parsed Data ---");
        Serial.printf("TDS: %d ppm\n", currentData.tds);
        Serial.printf("pH: %.2f\n", currentData.ph);
        Serial.printf("Turbidity: %d NTU\n", currentData.turbidity);
        Serial.printf("Quality: %s\n", currentData.quality.c_str());
        Serial.println("-------------------\n");
    } else {
        Serial.println("⚠ Failed to parse data - invalid format");
    }
}
