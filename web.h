#include <WiFiClient.h>
#include <ESP32WebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>
  #include "RingB.h"
  
extern ESP32WebServer server;
extern void web_setup();
extern void web_log(char *s);
extern void web_log(char s);                                            
extern  RingB ron;
extern bool custer;
//extern AsyncWebServerRequest *srequest;

