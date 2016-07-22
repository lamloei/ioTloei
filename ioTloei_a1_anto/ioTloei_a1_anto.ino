/**
 * Firmware Name:   ioTloei
 * Description:     Config ESP8266 family to WIFI and/or BROKER
 * Author:          lamloei
 * Author URI:      http://www.lamloei.com
 * Facebook URI:    https://www.facebook.com/lamloeicom
 * Github URI:      https://github.com/lamloei
 * Modified Date:   2016071401
 * Version:         ALPHA A1 ANTO
 */
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <AntoIO.h>
#include "FS.h"

/**** Parameter ****/
char* file_config = "/config.json";
char* wifiapSSID = "ESP_IOTLOEI";
int wifi_delay = 500;
int adelay = 3000;
int baudrate = 115200;
/******************/
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
/******************/
char* token_anto = "anto.io";
/******************/

StaticJsonBuffer<400> jsonBuffer;
char json[400];
char wifiSSID[20];
char wifiPSK[20];
char broker[20];
char anto_user[20];
char anto_key[60];
char anto_thing[40];
int loadcf = 0;

IPAddress IP;
ESP8266WebServer webServer(80);
AntoIO *anto;

bool bIsConnected = false;

int Led0,Led1,Led2,Led3,Led4,Led5,Led6,Led7 = 0;
int value = 0;

/******** API ********/
/*
 * getLogo()          - return logo
 * getTitle()         - return title
 * getVersion()       - return version
 */
String getLogo() {
  return ""
"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADwAAAA8CAYAAAA6/NlyAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsQAAA7EAZUrDhsAAADiSURBVGhD7dbBCgMhDIRh7fu/c6vUw9aDaRsjy+T/oOxhUQidIVufTUnkMZ5pMLA6BlbHwOoYWF26gWv7fXxa7vzSrLVf/zbfe33XWe//Nd9LpNXR4bt0OAqRVkeHT+3DX/bwzrNEWh0dnjMfhQ4fQqSjeGLpQaTHMw06PGc+itXhKERaHR1e8exK66zn7pX5XiKtLl+HW8aXHb52ydPDnWctq7NEWl3/77/OixXLFU+kPYj0eKZBh+fMR7E6HIVIq6PDK55dGbVnLezh8UwjX4dbxs8swJsg0uoYWB0DayvlBduJlDjeEDiuAAAAAElFTkSuQmCC"
;}
String getTitle() {
  return "ioTloei";
}
String getVersion() {
  return "Alpha<br/>A1<wbr/>Anto";
}

/******** LOAD ********/
/*
 * loadConfig()       - load file config into variable
 */
bool loadConfig() {
  if (loadcf==0) {
    if (!(SPIFFS.exists(file_config))) {
      loadcf=0;
      return false;
    }
    File fp = SPIFFS.open(file_config,"r");
    fp.read((uint8_t*)json,fp.size());
    fp.close();
    JsonObject& root = jsonBuffer.parseObject(json);
    if (!root.success()) {
      Serial.println("Warning: parseObject() failed");
      loadcf=0;
      return false;
    } else {
      loadcf=1;
      if (root.containsKey("wifiSSID"))           { strcpy(wifiSSID,root["wifiSSID"]); }
      if (root.containsKey("wifiPSK"))            { strcpy(wifiPSK,root["wifiPSK"]); }
      if (root.containsKey("broker"))             { strcpy(broker,root["broker"]); }
      if (root.containsKey("anto_user"))          { strcpy(anto_user,root["anto_user"]); }
      if (root.containsKey("anto_key"))           { strcpy(anto_key,root["anto_key"]); }
      if (root.containsKey("anto_thing"))         { strcpy(anto_thing,root["anto_thing"]); }
    }
  }
  return true;
}

/******** Handle ********/
/*
 * handleNotFound()     - Show Page if Not Found
 * handleConfig()       - Config Form
 * handleConfigJson()   - Show Config File
 * handleSave()         - Save Args to Config File
 * handleClear()        - Remove Config File
 * handleNew()          - new Config {}
 * handleReset()        - ESP Reset
 * handleRestart()      - ESP Restart
  */
void handleNotFound() {
  webServer.send(200,"text/html","HELLO ioTloei version: "+getVersion());
}
void handleRoot() {
  webServer.send(200,"text/html",""
"<!DOCTYPE html>\n"
"<html><head>\n"
"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n"
"  <title>"+getTitle()+"</title>\n"
"  <meta charset=\"utf-8\">\n"
"  <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n"
"  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
"  <link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css\" rel=\"stylesheet\">\n"
"  <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/2.2.4/jquery.min.js\"></script>\n"
"  <script src=\"http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js\"></script>\n"
"</head>\n"
"<body>\n"
"  <div id=\"container\" class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-3\">\n"
"        <div class=\"form-group col-xs-4 col-sm-4 col-lg-4\">\n"
"<img width=\"60px\" height=\"60px\" alt=\"logo\" src=\""+getLogo()+"\" />\n"
"        </div>\n"
"        <div class=\"form-group col-xs-6 col-sm-6 col-lg-6\">\n"
"          <h2 class=\"text-right\"><a href=\"https://www.facebook.com/lamloeicom\">"+getTitle()+"</a></h2>\n"
"        </div>\n"
"        <div class=\"form-group col-xs-2 col-sm-2 col-lg-2\">\n"
"          "+getVersion()+"\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-3\">\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          <a href=\"config\">Config</a>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-3\">\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          <a href=\"config.json\">Show</a>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-3\">\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          <a href=\"/\">HOME</a>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-3\">\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          <a href=\"clear\">Clear</a>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-3\">\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          <a href=\"new\">New</a>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-3\">\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          <a href=\"reset\">Reset</a>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-3\">\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          <a href=\"restart\">Restart</a>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"  </div>\n"
"</body></html>"
);
}
void handleConfig() {
  loadConfig();
  webServer.send(200,"text/html",""
"<!DOCTYPE html>\n"
"<html><head>\n"
"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n"
"  <title>"+getTitle()+"</title>\n"
"  <meta charset=\"utf-8\">\n"
"  <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n"
"  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
"  <link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css\" rel=\"stylesheet\">\n"
"  <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/2.2.4/jquery.min.js\"></script>\n"
"  <script src=\"http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js\"></script>\n"
"</head>\n"
"<body>\n"
"<form action=\"save\" method=\"post\">\n"
"  <div id=\"container\" class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-3\">\n"
"        <div class=\"form-group col-xs-4 col-sm-4 col-lg-4\">\n"
"<img width=\"60px\" height=\"60px\" alt=\"logo\" src=\""+getLogo()+"\" />\n"
"        </div>\n"
"        <div class=\"form-group col-xs-6 col-sm-6 col-lg-6\">\n"
"          <h2 class=\"text-right\"><a href=\"https://www.facebook.com/lamloeicom\">"+getTitle()+"</a></h2>\n"
"        </div>\n"
"        <div class=\"form-group col-xs-2 col-sm-2 col-lg-2\">\n"
"          "+getVersion()+"\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"form-group col-xs-12 col-sm-6 col-lg-3\">\n"
"        <input id=\"wifiSSID\" class=\"form-control\" name=\"wifiSSID\" placeholder=\"WIFI SSID\" value=\""+String(wifiSSID)+"\" type=\"text\"/>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"form-group col-xs-12 col-sm-6 col-lg-3\">\n"
"        <input id=\"wifiPSK\" class=\"form-control\" name=\"wifiPSK\" placeholder=\"WIFI PASSWORD\" value=\""+String(wifiPSK)+"\" type=\"text\"/>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div id=\"anto\" class=\"collapse in\">\n"
"        <div class=\"form-group col-xs-12 col-sm-6 col-lg-3\">\n"
"          <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"            <a href=\"https://anto.io/\">anto.io</a>\n"
"          </div>\n"
"          <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"            <input id=\"anto_user\" class=\"form-control\" name=\"anto_user\" placeholder=\"USER\" value=\""+String(anto_user)+"\" type=\"text\"/>\n"
"          </div>\n"
"          <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"            <input id=\"anto_key\" class=\"form-control\" name=\"anto_key\" placeholder=\"KEY\" value=\""+String(anto_key)+"\" type=\"text\"/>\n"
"          </div>\n"
"          <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"            <input id=\"anto_thing\" class=\"form-control\" name=\"anto_thing\" placeholder=\"THING\" value=\""+String(anto_thing)+"\" type=\"text\"/>\n"
"          </div>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"form-group col-xs-12 col-sm-6 col-lg-3\">\n"
"        <button class=\"btn btn-info form-control\" type=\"submit\">SAVE &amp; RESTART</button>\n"
"      </div>\n"
"    </div>\n"
"  </div>\n"
"<input id=\"broker\" name=\"broker\" value=\"anto.io\" type=\"hidden\"/>\n"
"<input id=\"formname\" name=\"formname\" value=\"iotloei_config\" type=\"hidden\"/>\n"
"</form>\n"
"</body></html>"
);
}
void handleConfigJson() {
  char cjson[400];
  if (SPIFFS.exists(file_config)) {
    File fp = SPIFFS.open(file_config,"r");
    fp.read((uint8_t*)cjson,fp.size());
    fp.close();
    webServer.send(200,"text/html","<pre>"+String(cjson));
  } else {
    webServer.send(200,"text/plain","Warning: Cannot open Config");
  }
}
void handleSave() {
  char a[400] = "";
  char b[60] = "";
  if (webServer.args()>0) {
    strcat(a,"{\n");
    for (uint8_t i = 0; i < webServer.args(); i++) {
      strcat(a,"\t\"");
      webServer.argName(i).toCharArray(b,webServer.argName(i).length()+1);
      strcat(a,b);
      strcat(a,"\":\"");
      webServer.arg(i).toCharArray(b,webServer.arg(i).length()+1);
      strcat(a,b);
      strcat(a,"\"");
      if (i==webServer.args()-1) {
        strcat(a,"\n");
      } else {
        strcat(a,",\n");
      }
    }
    strcat(a,"}\n\0");
    File fp = SPIFFS.open(file_config,"w+");
    fp.println(String(a));
    fp.close();
    webServer.send(200,"text/html","<pre>"+String(a)+"\n\nto save... and reset<br/>please wait......");
    loadcf=0;
    loadConfig();
    if ((strcmp(broker,token_anto)==0) && (strcmp(anto_user,"")!=0)) {
      Serial.print("Anto library version: ");
      Serial.println(anto->getVersion());
      anto = new AntoIO(anto_user, anto_key, anto_thing);
      // register callback functions
      anto->mqtt.onConnected(anto_connectedCB);
      anto->mqtt.onDisconnected(anto_disconnectedCB);
      anto->mqtt.onData(anto_dataCB);
      anto->mqtt.onPublished(anto_publishedCB);
      // Connect to anto->io MQTT broker
      anto->mqtt.connect();
    }
  } else {
    webServer.send(200,"text/html","Warning: no args to save.");
  }
}
void handleClear() {
  String message;
  if (SPIFFS.exists(file_config)) {
    SPIFFS.remove(file_config);
    message = "<h1><font color=red>CONFIG REMOVED</font></h1>";
  } else {
    message = "<h1><font color=red>CONFIG NOT EXISTS</font></h1>";
  }
  webServer.send(200,"text/html",message);
}
void handleNew() {
  File fp = SPIFFS.open(file_config,"w+");
  fp.println("{}");
  fp.close();
  webServer.send(200,"text/html","New Config {}");
}
void handleReset() {
  webServer.send(200,"text/html","ESP RESET");
  delay(adelay);
  ESP.reset();
  delay(adelay);
}
void handleRestart() {
  webServer.send(200,"text/html","ESP RESTART");
  delay(adelay);
  ESP.restart();
  delay(adelay);
}

/******** Anto ********/
void anto_connectedCB() {
  bIsConnected = true;
  Serial.println("Connected to MQTT Broker");
  anto->sub("LED0");
  anto->sub("LED1");
  anto->sub("LED2");
  anto->sub("LED3");
  anto->sub("LED4");
  anto->sub("LED5");
  anto->sub("LED6");
  anto->sub("LED7");
}
void anto_disconnectedCB() {
  bIsConnected = false;
  Serial.println("Disconnected to MQTT Broker");
}
void anto_dataCB(String& topic, String& msg) {
  uint8_t index = topic.indexOf('/');
  index = topic.indexOf('/', index + 1);
  index = topic.indexOf('/', index + 1);
  topic.remove(0, index + 1);
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(msg);

  if (topic.equals("LED0"))       { value = msg.toInt(); if (value==1) { digitalWrite(D0,LOW); } else { digitalWrite(D0,HIGH); } }
  else if (topic.equals("LED1"))  { value = msg.toInt(); if (value==1) { digitalWrite(D1,LOW); } else { digitalWrite(D1,HIGH); } }
  else if (topic.equals("LED2"))  { value = msg.toInt(); if (value==1) { digitalWrite(D2,LOW); } else { digitalWrite(D2,HIGH); } }
  else if (topic.equals("LED3"))  { value = msg.toInt(); if (value==1) { digitalWrite(D3,LOW); } else { digitalWrite(D3,HIGH); } }
  else if (topic.equals("LED4"))  { value = msg.toInt(); if (value==1) { digitalWrite(D4,LOW); } else { digitalWrite(D4,HIGH); } }
  else if (topic.equals("LED5"))  { value = msg.toInt(); if (value==1) { digitalWrite(D5,LOW); } else { digitalWrite(D5,HIGH); } }
  else if (topic.equals("LED6"))  { value = msg.toInt(); if (value==1) { digitalWrite(D6,LOW); } else { digitalWrite(D6,HIGH); } }
  else if (topic.equals("LED7"))  { value = msg.toInt(); if (value==1) { digitalWrite(D7,LOW); } else { digitalWrite(D7,HIGH); } }
  else if (topic.equals("D0"))    { value = msg.toInt(); if (value==1) { digitalWrite(D0,LOW); } else { digitalWrite(D0,HIGH); } }
  else if (topic.equals("D1"))    { value = msg.toInt(); if (value==1) { digitalWrite(D1,LOW); } else { digitalWrite(D1,HIGH); } }
  else if (topic.equals("D2"))    { value = msg.toInt(); if (value==1) { digitalWrite(D2,LOW); } else { digitalWrite(D2,HIGH); } }
  else if (topic.equals("D3"))    { value = msg.toInt(); if (value==1) { digitalWrite(D3,LOW); } else { digitalWrite(D3,HIGH); } }
  else if (topic.equals("D4"))    { value = msg.toInt(); if (value==1) { digitalWrite(D4,LOW); } else { digitalWrite(D4,HIGH); } }
  else if (topic.equals("D5"))    { value = msg.toInt(); if (value==1) { digitalWrite(D5,LOW); } else { digitalWrite(D5,HIGH); } }
  else if (topic.equals("D6"))    { value = msg.toInt(); if (value==1) { digitalWrite(D6,LOW); } else { digitalWrite(D6,HIGH); } }
  else if (topic.equals("D7"))    { value = msg.toInt(); if (value==1) { digitalWrite(D7,LOW); } else { digitalWrite(D7,HIGH); } }
}
void anto_publishedCB(void) {
  Serial.println("published");
}

/******** Main ********/
void setup() {
  digitalWrite(D0,HIGH);
  digitalWrite(D1,HIGH);
  digitalWrite(D2,HIGH);
  digitalWrite(D3,HIGH);
  digitalWrite(D4,HIGH);
  digitalWrite(D5,HIGH);
  digitalWrite(D6,HIGH);
  digitalWrite(D7,HIGH);
  pinMode(D0,OUTPUT);
  pinMode(D1,OUTPUT);
  pinMode(D2,OUTPUT);
  pinMode(D3,OUTPUT);
  pinMode(D4,OUTPUT);
  pinMode(D5,OUTPUT);
  pinMode(D6,OUTPUT);
  pinMode(D7,OUTPUT);
  Serial.begin(baudrate);
  while(!Serial) {}
  Serial.println("");
  Serial.println(WiFi.SSID());
  SPIFFS.begin();
  loadcf=0;
  loadConfig();

  WiFiManager wifiManager;
  wifiManager.setTimeout(wifi_delay);

  if(!wifiManager.autoConnect((const char*)wifiapSSID)) {
    Serial.println("Failed to connect and hit timeout");
    delay(adelay);
    ESP.reset();
    delay(adelay);
  }
  IP = WiFi.localIP();

  webServer.onNotFound(handleNotFound);
  webServer.on("/",handleRoot);
  webServer.on("/config",handleConfig);
  webServer.on("/config.json",handleConfigJson);
  webServer.on("/save",handleSave);
  webServer.on("/clear",handleClear);
  webServer.on("/new",handleNew);
  webServer.on("/reset",handleReset);
  webServer.on("/restart",handleRestart);
  webServer.begin();

  Serial.println("Broker Connecting...");
  if ((strcmp(broker,token_anto)==0) && (strcmp(anto_user,"")!=0)) {
    Serial.print("Anto library version: ");
    Serial.println(anto->getVersion());
    anto = new AntoIO(anto_user, anto_key, anto_thing);
    // register callback functions
    anto->mqtt.onConnected(anto_connectedCB);
    anto->mqtt.onDisconnected(anto_disconnectedCB);
    anto->mqtt.onData(anto_dataCB);
    anto->mqtt.onPublished(anto_publishedCB);
    // Connect to anto->io MQTT broker
    anto->mqtt.connect();
  }
  Serial.println("setup OK.");
}

void loop() {
  webServer.handleClient();
}
