
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WifiManager.h>

#include <Stepper.h>

#define LED_A 0
#define LED_R 4
#define LED_G 2
#define LED_B 15
#define MOTOR_A 16
#define MOTOR_B 17
#define LEDC_CHANNEL_0 0
#define LEDC_CHANNEL_1 1
#define LEDC_CHANNEL_2 2
#define LEDC_TIMER_8_BIT 8
#define LEDC_TIMER_13_BIT 13
#define LEDC_BASE_FREQ 5000
size_t j = 0;
/* #define CADVDD 27
#define CADGND 32
#define CADIN 25 */

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

//-------------------------------------- main --------------------------------------
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
        break;
    case WS_EVT_ERROR:
        break;
    }
}

void setup()
{
    Serial.begin(115200);
    ///   Serial.onReceive(Receiver::gotUARTdata);
    // LEDs
    pinMode(LED_A, OUTPUT);
    pinMode(LED_B, OUTPUT);
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT);
    ledcAttachPin(LED_R, LEDC_CHANNEL_0);
    ledcSetup(LEDC_CHANNEL_1, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT);
    ledcAttachPin(LED_G, LEDC_CHANNEL_1);
    ledcSetup(LEDC_CHANNEL_2, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT);
    ledcAttachPin(LED_B, LEDC_CHANNEL_2);
    resetLeds();

    // MOTOR
    pinMode(MOTOR_B, OUTPUT);
    pinMode(MOTOR_A, OUTPUT);
    digitalWrite(MOTOR_A, LOW);
    digitalWrite(MOTOR_B, LOW);

    //--------------------------- socket -------------------------

    initWebSocket(onEvent);
}

void loop()
{
    // Serial.println("loop");
    /* for (size_t i = 0; i < 256; i++)
    {
        delay(1);
        ledAnalogWrite(j, i);
    }

    ledAnalogWrite(j, 0); */
    j = getLed();
    if (j == 0)
    {
        digitalWrite(MOTOR_A, LOW);
        digitalWrite(MOTOR_B, LOW);
    }
    else if (j == 1)
    {
        digitalWrite(MOTOR_A, HIGH);
        digitalWrite(MOTOR_B, LOW);
    }
    else if (j == 2)
    {
        digitalWrite(MOTOR_A, HIGH);
        digitalWrite(MOTOR_B, LOW);
    }
    // j++;
    /*     if (j > 2)
        {
            j = 0;
        } */
}
