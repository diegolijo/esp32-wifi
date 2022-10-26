
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Stepper.h>

#define DHTVCC 19
#define DHTGND 5
#define DHTPIN 18
#define DHTTYPE DHT22

#define LED_A 0
#define LED_R 4
#define LED_G 2
#define LED_B 15
#define LEDC_CHANNEL_0 0
#define LEDC_CHANNEL_1 1
#define LEDC_CHANNEL_2 2
#define LEDC_TIMER_8_BIT 8
#define LEDC_TIMER_13_BIT 13
#define LEDC_BASE_FREQ 5000

/* #define CADVDD 27
#define CADGND 32
#define CADIN 25 */

#define SW1 23
#define ENC_A 22
#define ENC_B 21

#define IN1 12
#define IN2 14
#define IN3 27
#define IN4 26

int encoder0Pos = 0;

DHT_Unified dht(DHTPIN, DHTTYPE);

// wifi
IPAddress local_IP(192, 168, 1, 200); // Set your Static IP address
IPAddress gateway(192, 168, 1, 1);    // Set your Gateway IP address
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional

AsyncWebSocket ws("/ws");
// station
AsyncWebServer server(8015);
const char *ssid = "WIFI_JIT";
const char *password = "1234zxcv";
// access point
/* const char *ssid_st = "Arduino";
const char *password_st = "fiestaloca";
WiFiServer station(8015); */

Stepper myStepper(200, IN1, IN2, IN3, IN4);

//------------------------------------ socket --------------------------------------
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        if (strcmp((char *)data, "toggle") == 0)
        {
            // notifyClients(1);
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void initWebSocket()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

void notifyClients(int value)
{
    ws.textAll("{\"encoder\":" + String(value) + "}");
}

//------------------------------------ DHT ---------------------------------------
String getTemHum()
{
    sensors_event_t event;
    float temp, hum;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature))
    {
        Serial.println(F("Error reading temperature!"));
    }
    else
    {
        temp = event.temperature;
    }
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity))
    {
        Serial.println(F("Error reading humidity!"));
    }
    else
    {
        hum = event.relative_humidity;
    }
    Serial.println("humedad: " + String(hum) + "% , temperatura: " + String(temp) + "°C");
    return "{\"humedad\":\"" + String(hum) + "\", \"temperatura\":\"" + String(temp) + "\"}";
}
//------------------------------------ LEDs ---------------------------------------
void ledAnalogWrite(int channel, int value)
{
    int duty = 255 - value;
    ledcWrite(channel, duty);
}

void resetLeds()
{
    digitalWrite(LED_A, HIGH);
    ledAnalogWrite(LEDC_CHANNEL_0, 0);
    ledAnalogWrite(LEDC_CHANNEL_1, 0);
    ledAnalogWrite(LEDC_CHANNEL_2, 0);
}

//------------------------------------ encoder ---------------------------------------
int read_encoder()
{
    static int encoder0PinALast;
    static int n;
    n = digitalRead(ENC_A);
    if ((encoder0PinALast == LOW) && (n == HIGH))
    {
        if (digitalRead(ENC_B) == LOW)
        {
            encoder0Pos -= 1;
        }
        else
        {
            encoder0Pos += 1;
        }
        encoder0Pos = encoder0Pos < 0 ? 0 : (encoder0Pos > 255 ? 255 : encoder0Pos);
        notifyClients(encoder0Pos);
        Serial.println("encoder: " + String(encoder0Pos));
    }
    encoder0PinALast = n;
    return encoder0Pos;
}

void read_switch()
{
    static boolean cheched;
    if (LOW == digitalRead(SW1) && !cheched)
    {
        cheched = true;
        Serial.println("switch!!");
        encoder0Pos = 0;
    }
    if (HIGH == digitalRead(SW1))
    {
        cheched = false;
    }
}
//-------------------------------------- CAD --------------------------------------
void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

//-------------------------------------- main --------------------------------------
void setup()
{
    Serial.begin(115200);
    // LEDs
    pinMode(LED_A, OUTPUT);
    /*     pinMode(LED_B, OUTPUT);
        pinMode(LED_R, OUTPUT);
        pinMode(LED_G, OUTPUT); */
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT);
    ledcAttachPin(LED_R, LEDC_CHANNEL_0);
    ledcSetup(LEDC_CHANNEL_1, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT);
    ledcAttachPin(LED_G, LEDC_CHANNEL_1);
    ledcSetup(LEDC_CHANNEL_2, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT);
    ledcAttachPin(LED_B, LEDC_CHANNEL_2);
    resetLeds();
    // encoder
    pinMode(SW1, INPUT);
    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);
    // CAD
    /*  pinMode(CADVDD, OUTPUT);
        pinMode(CADGND, OUTPUT);
        pinMode(CADIN, INPUT);
        digitalWrite(CADVDD, HIGH);
        digitalWrite(CADGND, LOW); */
    // DHT
    pinMode(DHTGND, OUTPUT);
    pinMode(DHTVCC, OUTPUT);
    digitalWrite(DHTGND, LOW);
    digitalWrite(DHTVCC, HIGH);
    dht.begin();
    sensor_t sensor;
    Serial.println(F("DHTxx Unified Sensor Example"));
    dht.temperature().getSensor(&sensor);
    Serial.println(F("------------------------------------"));
    Serial.println(F("Temperature Sensor"));
    Serial.print(F("Sensor Type: "));
    Serial.println(sensor.name);
    Serial.print(F("Driver Ver:  "));
    Serial.println(sensor.version);
    Serial.print(F("Unique ID:   "));
    Serial.println(sensor.sensor_id);
    Serial.print(F("Max Value:   "));
    Serial.print(sensor.max_value);
    Serial.println(F("°C"));
    Serial.print(F("Min Value:   "));
    Serial.print(sensor.min_value);
    Serial.println(F("°C"));
    Serial.print(F("Resolution:  "));
    Serial.print(sensor.resolution);
    Serial.println(F("°C"));
    Serial.println(F("------------------------------------"));
    dht.humidity().getSensor(&sensor);
    Serial.println(F("Humidity Sensor"));
    Serial.print(F("Sensor Type: "));
    Serial.println(sensor.name);
    Serial.print(F("Driver Ver:  "));
    Serial.println(sensor.version);
    Serial.print(F("Unique ID:   "));
    Serial.println(sensor.sensor_id);
    Serial.print(F("Max Value:   "));
    Serial.print(sensor.max_value);
    Serial.println(F("%"));
    Serial.print(F("Min Value:   "));
    Serial.print(sensor.min_value);
    Serial.println(F("%"));
    Serial.print(F("Resolution:  "));
    Serial.print(sensor.resolution);
    Serial.println(F("%"));
    Serial.println(F("------------------------------------"));
    // wifi station
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    {
        Serial.println("STA Failed to configure");
    }
    // station
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    // access p.
    // WiFi.mode(WIFI_AP);
    // WiFi.softAP(ssid_st, password_st);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.printf("WiFi Failed!\n");
        return;
    }
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    //------------------------------- socket -------------------------
    initWebSocket();
    //---------------------------wifi events---------------------------
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                String result = getTemHum();
                AsyncWebServerResponse *response = request->beginResponse(200, "text/plain",result); 
              	response->addHeader("Access-Control-Allow-Origin", "*");
                request->send(response); });

    // Send a GET request to http://192.168.1.136:8015/sensor/0
    server.on("^\\/sensor\\/([0-9]+)$", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String sensorNumber = request->pathArg(0);
        request->send(200, "text/plain", "Hello, sensor: " + sensorNumber); });

    // Send a GET request to http://192.168.1.136:8015/sensor/0/action/2
    server.on("^\\/sensor\\/([0-9]+)\\/action\\/([a-zA-Z0-9]+)$", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String sensorNumber = request->pathArg(0);
        String action = request->pathArg(1);
        request->send(200, "text/plain", "Hello, sensor: " + sensorNumber + ", with action: " + action); });

    server.onNotFound(notFound);

    server.begin();

    myStepper.setSpeed(300);
}

void loop()
{
        Serial.println("loop");
    read_switch();
    int value = read_encoder();
    /*     int value = analogRead(CADIN); */
    switch (value)
    {
    case 0:
        myStepper.step(-100);
        break;
    case 1:
        myStepper.step(0);
        break;
    case 2:
        myStepper.step(100);
        break;
    default:
        break;
    }
    ledAnalogWrite(2, value);
}
