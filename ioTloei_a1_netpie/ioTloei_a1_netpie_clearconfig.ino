#include "FS.h"
char* file_config = "/config.json";

void setup() {
  Serial.begin(115200);
  while(!Serial) {}
  SPIFFS.begin();
  File fp = SPIFFS.open(file_config,"w+");
  fp.println("");
  fp.close();
  Serial.println("");
  Serial.println("Clear Config Successfully.");
}

void loop() {
  // put your main code here, to run repeatedly:
}
