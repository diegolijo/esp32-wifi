//
// A simple server implementation with regex routes:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//
// Add buildflag ASYNCWEBSERVER_REGEX to enable the regex support
// For platformio: platformio.ini:
//  build_flags =
//      -DASYNCWEBSERVER_REGEX
// For arduino IDE: create/update platform.local.txt
// Windows: C:\Users\(username)\AppData\Local\Arduino15\packages\espxxxx\hardware\espxxxx\{version}\platform.local.txt
// Linux: ~/.arduino15/packages/espxxxx/hardware/espxxxx/{version}/platform.local.txt
//
// compiler.cpp.extra_flags=-DASYNCWEBSERVER_REGEX=1

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
/* #ifdef ESP32 */
#include <WiFi.h>
#include <AsyncTCP.h>
/* #elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif */
#include <ESPAsyncWebServer.h>

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

int encoder0Pos = 0;

DHT_Unified dht(DHTPIN, DHTTYPE);

AsyncWebServer server(8015);

const char *ssid = "WIFI_JIT";
const char *password = "1234zxcv";

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
    /*     pinMode(CADVDD, OUTPUT);
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
    // wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.printf("WiFi Failed!\n");
        return;
    }
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
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
}

void loop()
{
    read_switch();
    int value = read_encoder();
    /*     int value = analogRead(CADIN); */
    ledAnalogWrite(2, value);
}
