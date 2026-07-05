#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// Configurações da tela OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Configurações do DHT11
#define DHTPIN 26
DHT dht(DHTPIN, DHT11);

// Configurações WiFi
const char *ssid = "AP1";
const char *password = "10303401";

WebServer server(80);

// Protótipos das funções (declarar antes de usar)
float readDHTTemperature();
float readDHTHumidity();

void handleRoot() {
  char msg[1500];
  float temp = readDHTTemperature();
  float hum = readDHTHumidity();

  snprintf(msg, 1500,
           "<html>\
  <head>\
    <meta http-equiv='refresh' content='4'/>\
    <meta name='viewport' content='width=device-width, initial-scale=1'>\
    <link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.7.2/css/all.css' integrity='sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr' crossorigin='anonymous'>\
    <title>PAISAGISMO FATEC SP</title>\
    <style>\
    html { font-family: Arial; display: inline-block; margin: 0px auto; text-align: center;}\
    h2 { font-size: 2.0rem; }\
    p { font-size: 2.5rem; }\
    .units { font-size: 1.2rem; }\
    .dht-labels{ font-size: 1.5rem; vertical-align:middle; padding-bottom: 15px;}\
    </style>\
  </head>\
  <body>\
      <h2>PAISAGISMO FATEC SP</h2>\
      <p>\
        <i class='fas fa-thermometer-half' style='color:#ca3517;'></i>\
        <span class='dht-labels'>Temperatura</span>\
        <span>%.2f</span>\
        <sup class='units'>&deg;C</sup>\
      </p>\
      <p>\
        <i class='fas fa-tint' style='color:#00add6;'></i>\
        <span class='dht-labels'>Umidade</span>\
        <span>%.2f</span>\
        <sup class='units'>&percnt;</sup>\
      </p>\
  </body>\
</html>",
           temp, hum);
  server.send(200, "text/html", msg);
}

void updateOLED(float temperature, float humidity) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("PAISAGISMO FATEC SP");
  display.display();
  delay(100);
  
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print("Temp: ");
  display.print(temperature, 1);
  
  
  display.setCursor(0, 45);
  display.print("Umi:  ");
  display.print(humidity, 1);
  
  display.display();
}

void setup(void) {
  Serial.begin(115200);
  dht.begin();
  
  // Inicializa a tela OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Iniciando...");
  display.display();
  
  // Conecta ao WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Conectando ao WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Conectado a ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Mostra IP na OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Conectado!");
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
  delay(2000);

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  
  // Atualiza a tela OLED a cada 5 segundos
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 5000) {
    float temp = readDHTTemperature();
    float hum = readDHTHumidity();
    if (temp != -1 && hum != -1) {
      updateOLED(temp, hum);
    }
    lastDisplayUpdate = millis();
  }
  
  delay(2); // Permite que a CPU execute outras tarefas
}

float readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {    
    Serial.println("Falha ao ler o sensor DHT!");
    return -1;
  }
  else {
    Serial.print("Temperatura: ");
    Serial.println(t);
    return t;
  }
}

float readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Falha ao ler o sensor DHT!");
    return -1;
  }
  else {
    Serial.print("Umidade: ");
    Serial.println(h);
    return h;
  }
}