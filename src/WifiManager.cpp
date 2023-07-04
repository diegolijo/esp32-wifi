/* #include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h> */
#include "WifiManager.h"
// wifi
IPAddress local_IP(172, 18, 2, 10); // Set your Static IP address
IPAddress gateway(172, 18, 2, 1);   // Set your Gateway IP address
IPAddress subnet(255, 255, 255, 0);
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

/* void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
} */
int ledValue = 1; // Valor del LED (ejemplo: 1)

//------------------------------------ socket --------------------------------------

void initWebSocket(AwsEventHandler handler)
{

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

    //---------------------------socket---------------------------
    ws.onEvent(handler);
    server.addHandler(&ws);

    //---------------------------wifi http---------------------------
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                String result; //TODO 
                AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", result); 
              	response->addHeader("Access-Control-Allow-Origin", "*");
                request->send(response); });

    // Send a GET request to http://172.18.2.10:8015/motor/0
    server.on("^\\/motor\\/([0-9]+)$", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String sensorNumber = request->pathArg(0);
        Serial.println("param. " + sensorNumber);
      
            if (sensorNumber.equals("0"))
            {
                if(ledValue>=2){
                  ledValue = 0  ;
                }else{
                   ledValue++;
                }          
            }
            else if (sensorNumber.equals("1"))
            { 
  
                if(ledValue==0){
                  ledValue = 2;
                }else{
                   ledValue--;
                }
            }
            else
            {
                // Código para casos no esperados
                // ...
            }
            
            // respuesta
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "{\"result\": \"OK\"}");
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response); });

    // Send a GET request to http://172.18.2.10:8015/sensor/0/action/2
    server.on("^\\/sensor\\/([0-9]+)\\/action\\/([a-zA-Z0-9]+)$", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String sensorNumber = request->pathArg(0);
        String action = request->pathArg(1);
        request->send(200, "text/plain", "Hello, sensor: " + sensorNumber + ", with action: " + action); });

    /* TODO server.onNotFound([this](AsyncWebServerRequest *request) {
      notFound(request);
    } );*/

    server.begin();
}

void notifyClients(int value)
{
    ws.textAll("{\"encoder\":" + String(value) + "}");
}

int getLed()
{

    return ledValue;
}