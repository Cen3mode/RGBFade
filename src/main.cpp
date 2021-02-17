#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <TelnetStream.h>

float pi = 3.14159265359;
int rgbPins[] = {13, 12, 14};
int cycleTime = 2000000.0;

const char * ssid = "TS";
const char * passwd = "_MantatzpdP_";

void wifiSetup() {

  pinMode(2, OUTPUT);
  digitalWrite(2, 1);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passwd);
  while(WiFi.waitForConnectResult() != WL_CONNECTED) ESP.restart();
  delay(2000);
  digitalWrite(2, 0);
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

void telnetShell(void * params) {
  char cmd[64];
  for(;;) {
    if(TelnetStream.readBytesUntil('\n', cmd, 64) > 0)
      TelnetStream.println(cmd);
    vTaskDelay(1);
  }
}

class Vector {
  private:

  public:

  double x = 0;
  double y = 0;
  double z = 0;

  Vector(double nx, double ny, double nz) {
    x = nx;
    y = ny;
    z = nz;
  }

  double add(Vector vector) {
    x += vector.x;
    y += vector.y;
    z += vector.z;
  }

  double sub(Vector vector) {
    x -= vector.x;
    y -= vector.y;
    z -= vector.z;
  }

  double mult(double factor) {
    x *= factor;
    y *= factor;
    z *= factor;
  }

  double set(double nx, double ny, double nz) {
    x = nx;
    y = ny;
    z = nz;
  }

  double mag() {
    return sqrt(pow(x, 2)+pow(y, 2)+pow(z, 2));
  }

  double norm() {
    double len = mag();
    x = x/len;
    y = y/len;
    z = z/len;
  }

};

Vector colorIncVec(64, 255, 128);
Vector color(0, 0, 0);

void setup() {
  Serial.begin(9600);
  wifiSetup();
  otaSetup();
  TelnetStream.begin();

  xTaskCreate(otaTask, "OTA_Task", 10000, NULL, 1, NULL);
  xTaskCreate(telnetShell, "Telnet Shell", 10000, NULL, 1, NULL);

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

  //delay(500);
}