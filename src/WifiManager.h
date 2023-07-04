// WifiManager.h
#ifndef MY_WIFI_HELPER_H
#define MY_WIFI_HELPER_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

void notFound(AsyncWebServerRequest *request);

void initWebSocket(AwsEventHandler handler);

void notifyClients(int value);

int getLed();

#endif // MY_WIFI_HELPER_H
