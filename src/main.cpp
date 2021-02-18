#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <TelnetStream.h>
#include "vector.h"

float pi = 3.14159265359;
int rgbPins[] = {13, 12, 14};
int cycleTime = 2000000.0;

const char * ssid = "TS";
const char * passwd = "_MantatzpdP_";

Vector colorIncVec(64, 255, 128);
Vector color(0, 0, 0);

void wifiSetup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passwd);
  int retryIntervalMs = 500;
  int timeoutCounter = 25 * (1000 / retryIntervalMs);
  while (WiFi.status() != WL_CONNECTED && timeoutCounter > 0)
  {
    delay(retryIntervalMs);
    if (timeoutCounter == (25 * 2 - 3))
    {
      WiFi.reconnect();
      Serial.println("Reconnecting...");
    }
    timeoutCounter--;
  }
}

void otaSetup() {
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void otaTask(void * params) {
  for(;;) {
    ArduinoOTA.handle();
    vTaskDelay(1);
  }
}

void telnetShellTask(void * params) {
  String input;
  for(;;) {
    input.concat(TelnetStream.readString());

    if(!input.isEmpty() && input.endsWith("\n")) {
      // TelnetStream.println("");
      if(input.startsWith("color")) {
        String args = input.substring(input.indexOf(" "));
        if(!args.isEmpty()) {
          String colorArgs[6] = {args, "", "", "", "", ""};
          for(int i = 0; i < 6; i++) {
            colorArgs[i] = colorArgs[i].substring(args.indexOf(" "), args.substring(args.indexOf(" ")).indexOf(" "));
          }
        } else {
          TelnetStream.printf("r: %g g: %g b: %g \n\r", color.x, color.y, color.z);
        }
      } 
      if(input.startsWith("red")) {
        String args = input.substring(input.indexOf(" "));
        if(!args.isEmpty()) {
          int val = args.toInt();
          TelnetStream.printf("Color red is set to %d \n\r", val);
          color.x = (double)(val);
        } else {
          TelnetStream.printf("Value of red is %g \n\r", color.x);
        }
      } 
      if(input.startsWith("exit")) {
        TelnetStream.flush();
        TelnetStream.stop();
      }
      TelnetStream.print("[RGBFADE] uSh -> ");
      input = "";
    }

    vTaskDelay(1);
  }
}

void setup() {
  Serial.begin(9600);
  wifiSetup();
  otaSetup();
  TelnetStream.begin();

  xTaskCreate(otaTask, "OTA Task", 100000, NULL, 1, NULL);
  xTaskCreate(telnetShellTask, "Telnet Shell Task", 50000, NULL, 1, NULL);

  for(int i = 0; i<3; i++) {
    pinMode(rgbPins[i], OUTPUT);
    ledcSetup(i, 5000, 15);
    ledcAttachPin(rgbPins[i], i);
  }
}

void loop() {
  ledcWrite(0, color.x);
  ledcWrite(1, color.y);
  ledcWrite(2, color.z);

  color.add(colorIncVec);
  if(color.mag() > 65535) color.mult(0);

  delay(100);
}