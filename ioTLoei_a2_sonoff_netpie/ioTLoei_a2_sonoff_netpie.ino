/**
 * Firmware Name:             ioTLoei
 * Description:               Config ESP Family to WIFI and/or BROKER
 * Author:                    lamloei
 * Author URI:                http://www.lamloei.com
 * Facebook URI:              https://www.facebook.com/lamloeicom
 * Github URI:                https://github.com/lamloei/iotloei
 * Modified Date:             2016091201
 * Version:                   ALPHA A2
 * Device:                    sonoff & D1
 * Broker:                    netpie.io
 */
#define DEBUG_LOEI true
#define DEVICE_SONOFF true
// #define DEVICE_D1 true

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <MicroGear.h>
#include "FS.h"

/******** PARAMETER ********/
char* file_config   = "/config.json";
char* file_temp     = "/temp.html";
char* wifiapSSID    = "IOTLOEI-";
char* wifiapPSK     = "";
char* t_relay       = "RELAY";
char* t_led         = "LED";
char* t_button      = "BUTTON";
int baudrate        = 74880;
int wifi_delay      = 500;
int wifi_timeout    = 10;
int web_refresh     = 5;
int web_restart     = 60;
int web_timeout_a   = 30000;
int web_timeout_b   = 120000;
int adelay          = 1;
int bdelay          = 300;
int cdelay          = 3000;
const int max_timer = 8;
/******** WEB ROOT ********/
String txt_index_token[] = {
  "CONFIG",
  "GPIO",
  "ADV",
  "SHOW",
  "DELETE",
  "RESET",
  "RESTART",
};
String txt_index_desc[] = {
  "1. FIRST FILL CONFIG FORM",
  "2. AND LET TO PLAY GPIO.",
  "ADVANCED CONFIG",
  "SHOW FILE CONFIG",
  "DELETE FILE CONFIG",
  "HARDWARE RESET",
  "SOFTWARE RESTART",
};
String txt_index_link[] = {
  "/config",
  "/config/gpio",
  "/config/advconfig",
  "/config/show",
  "/config/delete",
  "/config/reset",
  "/config/restart",
};
/******** DEVICE DEFINE ********/
/******** SONOFF ********/
#ifdef DEVICE_SONOFF
  #define RELAY_PIN   12
  #define LED_PIN     13
  #define BUTTON_PIN  0
#endif

/******** D1_MODULE ********/
#ifdef DEVICE_D1
  #define RELAY_PIN   5
  #define LED_PIN     2
  #define BUTTON_PIN  0
#endif

#define BUTTON_OFF  HIGH        // 3
#define BUTTON_ON   LOW         // 4
#define RELAY_OFF   LOW         // 5
#define RELAY_ON    HIGH        // 6
#define LED_OFF     HIGH        // 7
#define LED_ON      LOW         // 8
/******** VAR CONFIG ********/
StaticJsonBuffer<600> jsonBuffer;
char json[600];
char wifiSSID[20];
char wifiPSK[20];
char broker[20];
char netpie_appid[20];
char netpie_appkey[40];
char netpie_appsecret[60];
char netpie_appalias[40];
char name_relay[40];
char name_led[40];
char name_button[40];

/******** VAR CONTROL ********/
int fac               = 0;
int loadcf            = 0;
int state             = 0;
int state_button      = BUTTON_OFF;
char* token_netpie    = "netpie.io";
char* token_yes       = "yes";
char* token_nf_host   = "go.microsoft.com";
char* token_nf_uri    = "/fwlink/";
const byte DNS_PORT   = 53;
const byte DNS_TTL    = 60;
const byte HTTP_PORT  = 80;
/*****************************/
DNSServer dnsServer;
ESP8266WebServer webServer(HTTP_PORT);
IPAddress IP;
WiFiClient client;
MicroGear microgear(client);
/*****************************/
#define cout Serial
#define fs SPIFFS
/*****************************/

/******** API ********/
/*
 * getLogo()                                            -- return logo
 * getHello()                                           -- return hello
 * getInfo()                                            -- return info
 * getPrompt()                                          -- return prompt
 * getTitle()                                           -- return title
 * getVersion()                                         -- return version
 */
String getLogo() {
  return "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAB4AAAA8CAYAAABxVAqfAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsQAAA7EAZUrDhsAAACISURBVFhH7ZdBCoAwDAQb//9njehBc4iHmk0PMyA9FLqQYSu13RkNbPcqh2AZBMsgWIb597oy/7xBzc7jL+K5OJZBsIy24M8eP7s4Az1e13EVjLoM6kSdZOA4ZeaXSY/7HPvsU8fZMyQ6zvZxTJ1krOM4uqgCxzLosYw+xz57TXEDOJZBsIgxDgPmSD20XNoMAAAAAElFTkSuQmCC";
}
String getHello() {
  return "Welcome to ioTLoei. Thank you to choose me.";
}
String getInfo() {
  return "{'name':'iotloei','version':'a2','device':'sonoff','broker':'netpie.io','modified_date':'2016091201'}";
}
String getPrompt() {
  return "ioTLoei:/> ";
}
String getTitle() {
  return "ioTLoei";
}
String getVersion() {
  return "A2 SONOFF NETPIE";
}

/******** LOAD CONFIG ********/
/*
 * loadConfig(bool a=false)                             -- load config to variable
 * 
 */
bool loadConfig(bool a=false) {
  if (a) { loadcf = 0; }
  if (loadcf==0) {
    strcpy(name_relay,t_relay);
    strcpy(name_led,t_led);
    strcpy(name_button,t_button);
    if (!(SPIFFS.exists(file_config))) {
      loadcf=0;
      return false;
    } else {
      File fp = SPIFFS.open(file_config,"r");
      fp.read((uint8_t*)json,fp.size());
      fp.close();
      JsonObject& root = jsonBuffer.parseObject(json);
      if (!root.success()) {
#ifdef DEBUG_LOEI
        cout.print(getPrompt());
        cout.println("Warning: parseObject() failed");
#endif
        loadcf=0;
        return false;
      } else {
        loadcf=1;
        if (root.containsKey("wifiSSID"))           { strcpy(wifiSSID,root["wifiSSID"]); }
        if (root.containsKey("wifiPSK"))            { strcpy(wifiPSK,root["wifiPSK"]); }
        if (root.containsKey("broker"))             { strcpy(broker,root["broker"]); }
        if (root.containsKey("netpie_appid"))       { strcpy(netpie_appid,root["netpie_appid"]); }
        if (root.containsKey("netpie_appkey"))      { strcpy(netpie_appkey,root["netpie_appkey"]); }
        if (root.containsKey("netpie_appsecret"))   { strcpy(netpie_appsecret,root["netpie_appsecret"]); }
        if (root.containsKey("netpie_appalias"))    { strcpy(netpie_appalias,root["netpie_appalias"]); }
        if (root.containsKey("name_relay"))         { strcpy(name_relay,root["name_relay"]); }
        if (root.containsKey("name_led"))           { strcpy(name_led,root["name_led"]); }
        if (root.containsKey("name_button"))        { strcpy(name_button,root["name_button"]); }
      }
    }
  }
  return true;
}

/******** WIFI ********/
/*
 * getWiFiAPName(char* a="", bool b=true)               -- return AP SSID
 * wifiinit()                                           -- INIT
 * wifiap(const char* ssid, const char* psk="")         -- AP MODE
 * wificonnect(const char* ssid, const char* psk="")    -- STA MODE
 */
String getWiFiAPName(char* a="", bool b=true) {
  String c = "";
  if ((strcmp(a,"")==0)) {
    c += String(wifiapSSID);
  } else {
    c += String(a);
  }
  if (b) {
    c += ESP.getChipId();
  }
  return c;
}
void wifiinit() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(adelay);
  WiFi.forceSleepWake();
  delay(adelay);
}
void wifiap(const char* ssid, const char* psk="") {
  wifiinit();
  WiFi.mode(WIFI_AP);
  WiFi.softAP((const char*)ssid, (const char*)psk);
  IP = WiFi.softAPIP();
  dnsServer.setTTL(DNS_TTL);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(DNS_PORT,"*",IP);
#ifdef DEBUG_LOEI
  cout.print(getPrompt());
  cout.print("AP IP = ");
  cout.println((String)IP[0]+"."+(String)IP[1]+"."+(String)IP[2]+"."+(String)IP[3]);
#endif
}
bool wificonnect(const char* ssid, const char* psk="") {
  int i=0;
  wifiinit();
  WiFi.mode(WIFI_STA);
  if (WiFi.begin((const char*)ssid, (const char*)psk)) {
#ifdef DEBUG_LOEI
    cout.print(getPrompt());
    cout.print("WIFI CONNECTTING");
#endif
    while ((WiFi.status() != WL_CONNECTED) && (i < wifi_timeout)) {
      delay(wifi_delay);
#ifdef DEBUG_LOEI
      cout.print(".");
#endif
      i++;
    }
#ifdef DEBUG_LOEI
    cout.println();
#endif
  }
  if (WiFi.status() == WL_CONNECTED) {
    IP = WiFi.localIP();
#ifdef DEBUG_LOEI
    cout.print(getPrompt());
    cout.print("IP = ");
    cout.println((String)IP[0]+"."+(String)IP[1]+"."+(String)IP[2]+"."+(String)IP[3]);
#endif
    return true;
  } else if (WiFi.status() == WL_IDLE_STATUS) {
    return false;
  } else {
    return false;
  }
}

/******** Microgear ********/
/*
 * microgear_sendState(bool a=true)                     -- send message
 * microgear_updateIO()                                 -- update pin
 * microgear_onMsghandler(char *topic, uint8_t* msg, unsigned int msglen)
 *                                                      -- event on message
 * microgear_onConnected(char *attribute, uint8_t* msg, unsigned int msglen)
 *                                                      -- event on connected
 * microgear_setup()                                    -- microgear for setup
 * microgear_loop()                                     -- microgear for loop
 */
void microgear_sendState(bool a=true) {
  String b = "/"+String(netpie_appalias)+"/state";
  char c[60];
  b.toCharArray(c,b.length()+1);
  if (digitalRead(BUTTON_PIN)==BUTTON_OFF)  { microgear.publish(c,"3"); }
  if (digitalRead(BUTTON_PIN)==BUTTON_ON)   { microgear.publish(c,"4"); }
  if (a) {
    if (digitalRead(RELAY_PIN)==RELAY_OFF)  { microgear.publish(c,"5"); }
    if (digitalRead(RELAY_PIN)==RELAY_ON)   { microgear.publish(c,"6"); }
    if (digitalRead(LED_PIN)==LED_OFF)      { microgear.publish(c,"7"); }
    if (digitalRead(LED_PIN)==LED_ON)       { microgear.publish(c,"8"); }
  }
}
void microgear_updateIO() {
#ifdef DEBUG_LOEI
  cout.print(getPrompt());
  cout.print("Incoming message --> ");
  cout.println(String(state));
#endif
  switch (state) {
    case 5: digitalWrite(RELAY_PIN, RELAY_OFF);   break;
    case 6: digitalWrite(RELAY_PIN, RELAY_ON);    break;
    case 7: digitalWrite(LED_PIN, LED_OFF);       break;
    case 8: digitalWrite(LED_PIN, LED_ON);        break;
  }
}
void microgear_onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  String a;
  char m = *(char *)msg;
  if (m=='?') {
    microgear_sendState();
  } else {
    for (int i=0; i<msglen; i++) {
      a += String((char)msg[i]);
    }
    state = a.toInt();
    microgear_updateIO();
  }
}
void microgear_onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
#ifdef DEBUG_LOEI
  cout.print(getPrompt());
  cout.println("Connected to NETPIE...");
#endif
}
void microgear_setup() {
  if ((strcmp(broker,token_netpie)==0) && (strcmp(netpie_appid,"")!=0) && fac == 1) {
#ifdef DEBUG_LOEI
    cout.print(getPrompt());
    cout.println("Broker Connecting...");
#endif
    microgear.on(MESSAGE,microgear_onMsghandler);
    microgear.on(CONNECTED,microgear_onConnected);
    microgear.init(netpie_appkey,netpie_appsecret,netpie_appalias);
    if (client.connect(broker, HTTP_PORT)) {
#ifdef DEBUG_LOEI
      cout.print(getPrompt());
      cout.println("Broker Init..");
#endif
      microgear.connect(netpie_appid);
#ifdef DEBUG_LOEI
      cout.print(getPrompt());
      cout.println("Broker Connected.");
#endif
    }
  }
}
void microgear_loop() {
  if ((strcmp(broker,token_netpie)==0) && (strcmp(netpie_appid,"")!=0) && fac == 1) {
    if (microgear.connected()) {
      microgear.loop();
    }
    else {
      if (client.connect(broker, HTTP_PORT)) {
#ifdef DEBUG_LOEI
        cout.print(getPrompt());
        cout.println("connection lost, reconnect.");
#endif
        microgear.connect(netpie_appid);
#ifdef DEBUG_LOEI
        cout.print(getPrompt());
        cout.println("connection lost, reconnect..");
#endif
      }
    }
  }
}
/******** Handle ********/
/*
 * handleLogo()                                         -- Get HTML Logo
 * handleHeader()                                       -- Get HTML Header
 * handleHeader2()                                      -- Get HTML Header 2
 * handleFooter()                                       -- Get HTML Footer
 * handleRootBody()                                     -- Get HTML Root
 * handleConfigBody()                                   -- GET HTML Config
 * handleConfigBody2()                                  -- GET HTML Config2
 * handleAdvConfigBody()                                -- GET HTML Adv Config
 * handleGPIOBody()                                     -- GET HTML GPIO
 * handleNotFound()                                     -- Get PAGE if Not Page Found
 * handleRoot()                                         -- Get PAGE Root
 * handleConfig()                                       -- Get PAGE Config
 * handleAdvConfig()                                    -- Get PAGE Adv Config
 * handleGPIO()                                         -- Get PAGE GPIO
 * handleSave()                                         -- Get PAGE Save
 * handleShow()                                         -- Get PAGE Show Config
 * handleDelete()                                       -- Get PAGE Delete
 * handleReset()                                        -- Get PAGE Reset
 * handleRestart()                                      -- Get PAGE Restart
 * handler()                                            -- Page Manager
 */
String handleLogo() {
  String a = "http://"+(String)IP[0]+"."+(String)IP[1]+"."+(String)IP[2]+"."+(String)IP[3];
  return "<a href=\""+a+"\"><img width=\"30px\" height=\"60px\" border=\"0\" alt=\"logo\" src=\""+getLogo()+"\" /></a>\n";
}
String handleHeader() {
  return ""
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"  <title>"+getTitle()+"</title>\n"
"  <meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\"/>\n"
"  <meta charset=\"utf-8\"/>\n"
"  <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"/>\n"
"  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/>\n"
"  <link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css\" rel=\"stylesheet\"/>\n"
"  <link href=\"https://gitcdn.github.io/bootstrap-toggle/2.2.2/css/bootstrap-toggle.min.css\" rel=\"stylesheet\"/>\n"
"  <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/2.2.4/jquery.min.js\"></script>\n"
"  <script src=\"http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js\"></script>\n"
"  <script src=\"https://gitcdn.github.io/bootstrap-toggle/2.2.2/js/bootstrap-toggle.min.js\"></script>\n"
"  <script src=\"https://netpie.io/microgear.js\"></script>\n"
"</head>\n"
"<body>\n"
"  <div id=\"container\" class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-4\">\n"
"        <div class=\"form-group col-xs-4 col-sm-4 col-lg-4\">\n"
;}
String handleHeader2() {
  String a = "http://"+(String)IP[0]+"."+(String)IP[1]+"."+(String)IP[2]+"."+(String)IP[3];
  return ""
"        </div>\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          <h2 class=\"text-right\"><a href=\""+a+"\">"+getTitle()+"</a></h2>\n"
"        </div>\n"
"        <div class=\"form-group col-xs-3 col-sm-3 col-lg-3\">\n"
"          "+getVersion()+"<div id=\"data\" class=\"data\" align=\"left\"><font color=\"#DDDDDD\"></font></div>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
;}
String handleFooter() {
  return ""
"  </div>\n"
"</body>\n"
"</html>\n"
;}
String handleRootBody() {
  String a;
  int b = sizeof(txt_index_token)/sizeof(String);
  for (int i=0; i<b; i++) {
    a += "    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n";
    a += "      <div class=\"col-xs-12 col-sm-6 col-lg-4\">\n";
    a += "        <div class=\"form-group col-xs-12 col-sm-3 col-lg-3\">\n";
    a += "          <a href=\""+txt_index_link[i]+"\">"+txt_index_token[i]+"</a>\n";
    a += "        </div>\n";
    a += "        <div class=\"form-group col-xs-12 col-sm-9 col-lg-9\">\n";
    a += "          "+txt_index_desc[i]+"\n";
    a += "        </div>\n";
    a += "      </div>\n";
    a += "    </div>\n";
  }
  return a;
}
String handleConfigBody() {
  return ""
"<form action=\"config/save\" method=\"post\">\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"form-group col-xs-12 col-sm-6 col-lg-4\">\n"
"        <input id=\"wifiSSID\" class=\"form-control\" name=\"wifiSSID\" placeholder=\"WIFI SSID\" value=\""+String(wifiSSID)+"\" type=\"text\"/>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"form-group col-xs-12 col-sm-6 col-lg-4\">\n"
"        <input id=\"wifiPSK\" class=\"form-control\" name=\"wifiPSK\" placeholder=\"WIFI PASSWORD\" value=\""+String(wifiPSK)+"\" type=\"text\"/>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div id=\"netpie\" class=\"collapse in\">\n"
"        <div class=\"form-group col-xs-12 col-sm-6 col-lg-4\">\n"
"          <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"            <a href=\"https://netpie.io/\">netpie.io</a>\n"
"          </div>\n"
"          <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"            <input id=\"netpie_appid\" class=\"form-control\" name=\"netpie_appid\" placeholder=\"APPID\" value=\""+String(netpie_appid)+"\" type=\"text\"/>\n"
"          </div>\n"
"          <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"            <input id=\"netpie_appkey\" class=\"form-control\" name=\"netpie_appkey\" placeholder=\"KEY\" value=\""+String(netpie_appkey)+"\" type=\"text\"/>\n"
"          </div>\n"
"          <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"            <input id=\"netpie_appsecret\" class=\"form-control\" name=\"netpie_appsecret\" placeholder=\"SECRET\" value=\""+String(netpie_appsecret)+"\" type=\"text\"/>\n"
"          </div>\n"
"          <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"            <input id=\"netpie_appalias\" class=\"form-control\" name=\"netpie_appalias\" placeholder=\"ALIAS\" value=\""+String(netpie_appalias)+"\" type=\"text\"/>\n"
"          </div>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
;}
String handleConfigBody2() {
  return ""
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"form-group col-xs-12 col-sm-6 col-lg-4\">\n"
"        <button class=\"btn btn-info form-control\" type=\"submit\">SAVE &amp; RESTART</button>\n"
"      </div>\n"
"    </div>\n"
"  </div>\n"
"<input id=\"broker\" name=\"broker\" value=\"netpie.io\" type=\"hidden\"/>\n"
"<input id=\"formname\" name=\"formname\" value=\"iotloei_config\" type=\"hidden\"/>\n"
"</form>\n"
;}
String handleAdvConfigBody() {
  return handleConfigBody2();
}
String handleGPIOBody() {
  return ""
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-4\">\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          "+String(name_relay)+"\n"
"        </div>\n"
"        <div class=\"form-group col-xs-4 col-sm-4 col-lg-4\">\n"
"          <a onclick=\"name_relay_toggle()\"><input id=\"name_relay\" class=\"name_relay\" data-toggle=\"toggle\" data-onstyle=\"success\" data-offstyle=\"danger\" data-style=\"slow\" type=\"checkbox\"></a>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-4\">\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          "+String(name_led)+"\n"
"        </div>\n"
"        <div class=\"form-group col-xs-4 col-sm-4 col-lg-4\">\n"
"          <a onclick=\"name_led_toggle()\"><input id=\"name_led\" class=\"name_led\" data-toggle=\"toggle\" data-onstyle=\"success\" data-offstyle=\"danger\" data-style=\"slow\" type=\"checkbox\"></a>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-4\">\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          "+String(name_button)+"\n"
"        </div>\n"
"        <div class=\"form-group col-xs-4 col-sm-4 col-lg-4\">\n"
"          <input id=\"name_button\" class=\"name_button\" data-toggle=\"toggle\" data-onstyle=\"success\" data-offstyle=\"danger\" data-style=\"slow\" type=\"checkbox\">\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-4\">\n"
"        <div class=\"form-group col-xs-2 col-xs-offset-3 col-sm-2 col-sm-offset-3 col-lg-2 col-lg-offset-3\">\n"
"          <button class=\"btn btn-warning\" onclick=\"aState()\">?</button>\n"
"        </div>\n"
"        <div class=\"form-group col-xs-3 col-sm-3 col-lg-3\">\n"
"          <button class=\"btn btn-success\" onclick=\"atoggleOn()\">On All</button>\n"
"        </div>\n"
"        <div class=\"form-group col-xs-3 col-sm-3 col-lg-3\">\n"
"          <button class=\"btn btn-danger\" onclick=\"atoggleOff()\">Off All</button>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"<script>\n"
"  const appid = \""+String(netpie_appid)+"\";\n"
"  const appkey = \""+String(netpie_appkey)+"\";\n"
"  const appsecret = \""+String(netpie_appsecret)+"\";\n"
"  const appalias = \""+String(netpie_appalias)+"\"; \n"
"  document.getElementById(\"data\").innerHTML = \"<font color=\\\"#DDDDDD\\\">OFFLINE</font>\";\n"
"  var name_button = 3;\n"
"  var name_relay = 5;\n"
"  var name_led = 7;\n"
"  var microgear = Microgear.create({\n"
"    appid: appid,\n"
"    key: appkey,\n"
"    secret: appsecret,\n"
"    alias : appalias\n"
"  });\n"
"  microgear.connect(appid);\n"
"  microgear.on('message',function(topic,msg) {\n"
"    switch (msg) {\n"
"      case '3': $('.name_button').bootstrapToggle('off'); break;\n"
"      case '4': $('.name_button').bootstrapToggle('on');  break;\n"
"      case '5': $('.name_relay').bootstrapToggle('off'); break;\n"
"      case '6': $('.name_relay').bootstrapToggle('on');  break;\n"
"      case '7': $('.name_led').bootstrapToggle('off'); break;\n"
"      case '8': $('.name_led').bootstrapToggle('on');  break;\n"
"    }\n"
"  });\n"
"  microgear.on('disconnect', function() {\n"
"    document.getElementById(\"data\").innerHTML = \"<font color=\\\"#DDDDDD\\\">OFFLINE</font>\";\n"
"  });\n"
"  microgear.on('connected', function() {\n"
"    microgear.subscribe('/'+appalias+'/state');\n"
"    microgear.chat(appalias,'?');\n"
"    document.getElementById(\"data\").innerHTML = \"<font color=\\\"#00BB00\\\">ONLINE</font>\";\n"
"  });\n"
"  function name_relay_toggle() {\n"
"    if (name_relay == 5)         { name_relay = 6; microgear.chat(appalias,name_relay.toString()); }\n"
"    else if (name_relay == 6)    { name_relay = 5; microgear.chat(appalias,name_relay.toString()); }\n"
"  }\n"
"  function name_led_toggle() {\n"
"    if (name_led == 7)           { name_led = 8; microgear.chat(appalias,name_led.toString()); }\n"
"    else if (name_led == 8)      { name_led = 7; microgear.chat(appalias,name_led.toString()); }\n"
"  }\n"
"  function aState() {\n"
"    microgear.subscribe('/'+appalias+'/state');\n"
"    microgear.chat(appalias,'?');\n"
"  }\n"
"  function atoggleOn() {\n"
"    name_relay =6;\n"
"    name_led =8;\n"
"    $('.name_relay').bootstrapToggle('on');\n"
"    $('.name_led').bootstrapToggle('on');\n"
"    name_relay = 6; microgear.chat(appalias,name_relay.toString());\n"
"    name_led = 8; microgear.chat(appalias,name_led.toString());\n"
"  }\n"
"  function atoggleOff() {\n"
"    name_relay =5;\n"
"    name_led =7;\n"
"    $('.name_relay').bootstrapToggle('off');\n"
"    $('.name_led').bootstrapToggle('off');\n"
"    name_relay = 5; microgear.chat(appalias,name_relay.toString());\n"
"    name_led = 7; microgear.chat(appalias,name_led.toString());\n"
"  }\n"
"</script>\n"
;}
void handleNotFound() {
  String a = "http://"+(String)IP[0]+"."+(String)IP[1]+"."+(String)IP[2]+"."+(String)IP[3];
  String b = "<meta http-equiv=\"refresh\" content=\""+(String)web_refresh+"; url="+a+"/config\"/>\n";
  String c = "Page Not Found. If page not refresh "+(String)web_refresh+" second please <a href=\""+a+"/config\">click here</a>\n";
  String d = "<meta http-equiv=\"refresh\" content=\""+(String)web_refresh+"; url="+a+"\"/>\n";
  String e = "Page Not Found. If page not refresh "+(String)web_refresh+" second please <a href=\""+a+"\">click here</a>\n";
  client.setTimeout(web_timeout_a);
  if ((webServer.hostHeader() == "go.microsoft.com") && (webServer.uri() == "/fwlink/")) {
    webServer.send(200,"text/html",b+handleHeader()+handleLogo()+handleHeader2()+c+handleFooter());
  } else {
    webServer.send(200,"text/html",d+handleHeader()+handleLogo()+handleHeader2()+e+handleFooter());
  }
}
void handleRoot() {
  webServer.send(200,"text/html",handleHeader()+handleLogo()+handleHeader2()+handleRootBody()+handleFooter());
}
void handleConfig() {
  loadConfig();
  webServer.send(200,"text/html",handleHeader()+handleLogo()+handleHeader2()+handleConfigBody()+handleConfigBody2()+handleFooter());
}
void handleAdvConfig() {
  loadConfig();
  client.setTimeout(web_timeout_b);
  webServer.send(200,"text/html",handleHeader()+handleLogo()+handleHeader2()+handleConfigBody()+handleAdvConfigBody()+handleFooter());  
}
void handleGPIO() {
  loadConfig();
  File fp;
  fp = SPIFFS.open(file_temp,"w+");
  fp.println(handleHeader()+handleLogo()+handleHeader2());
  fp.println(handleGPIOBody());
  fp.println(handleFooter());
  fp = SPIFFS.open(file_temp,"r");
  client.setTimeout(web_timeout_b);
  size_t sent = webServer.streamFile(fp, "text/html");
  fp.close();
}
void handleSave() {
  char va[600] = "";
  char vb[60] = "";
  String a = "http://"+(String)IP[0]+"."+(String)IP[1]+"."+(String)IP[2]+"."+(String)IP[3];
  String b;
  String c;
  if (webServer.args()>0) {
    strcat(va,"{\n");
    for (uint8_t i = 0; i < webServer.args(); i++) {
      strcat(va,"\t\"");
      webServer.argName(i).toCharArray(vb,webServer.argName(i).length()+1);
      strcat(va,vb);
      strcat(va,"\":\"");
      webServer.arg(i).toCharArray(vb,webServer.arg(i).length()+1);
      strcat(va,vb);
      strcat(va,"\"");
      if (i==webServer.args()-1) {
        strcat(va,"\n");
      } else {
        strcat(va,",\n");
      }
    }
    strcat(va,"}\n\0");
    File fp = SPIFFS.open(file_config,"w+");
    fp.println(String(va));
    fp.close();
    loadConfig(true);
    if (!wificonnect((const char*)wifiSSID, (const char*)wifiPSK)) {
      wifiap(getWiFiAPName(wifiapSSID, true).c_str(), (const char*)wifiapPSK);
    }
    microgear_setup();
    b = "<meta http-equiv=\"refresh\" content=\""+(String)web_refresh+"; url="+a+"/gpio\"/>\n";
    c = "PAGE SAVED. If page not refresh "+(String)web_refresh+" second please <a href=\""+a+"/gpio\">click here</a>\n";
  } else {
    b = "<meta http-equiv=\"refresh\" content=\""+(String)web_refresh+"; url="+a+"/config\"/>\n";
    c = "<font color=red>Warning: no args to save.</font> If page not refresh "+(String)web_refresh+" second please <a href=\""+a+"/config\">click here</a>\n";
  }
  String d = "";
  d += "    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n";
  d += "      <div class=\"col-xs-12 col-sm-6 col-lg-4\">\n";
  d += "        <div class=\"form-group col-xs-12 col-sm-12 col-lg-12\">\n";
  d += "          <h1><font color=red>PAGE SAVING...</font></h1>";
  d += c;
  d += "        </div>\n";
  d += "      </div>\n";
  d += "    </div>\n";
  d += b;
  webServer.send(200,"text/html",b+handleHeader()+handleLogo()+handleHeader2()+d+handleFooter());
}
void handleShow() {
  if (SPIFFS.exists(file_config)) {
    File fp = SPIFFS.open(file_config, "r");
    size_t sent = webServer.streamFile(fp, "application/json");
    fp.close();
  } else {
    String a = "http://"+(String)IP[0]+"."+(String)IP[1]+"."+(String)IP[2]+"."+(String)IP[3];
    String b = "<meta http-equiv=\"refresh\" content=\""+(String)web_refresh+"; url="+a+"\"/>\n";
    String c = "PAGE CANNOT OPEN FILE CONFIG. If page not refresh "+(String)web_refresh+" second please <a href=\""+a+"\">click here</a>\n";
    String d = "";
    d += "    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n";
    d += "      <div class=\"col-xs-12 col-sm-6 col-lg-4\">\n";
    d += "        <div class=\"form-group col-xs-12 col-sm-12 col-lg-12\">\n";
    d += "          <h1><font color=red>CANNOT OPEN CONFIG</font></h1>";
    d += c;
    d += "        </div>\n";
    d += "      </div>\n";
    d += "    </div>\n";
    d += b;
    webServer.send(200,"text/html",b+handleHeader()+handleLogo()+handleHeader2()+d+handleFooter());
  }
}
void handleDelete() {
  String a = "http://"+(String)IP[0]+"."+(String)IP[1]+"."+(String)IP[2]+"."+(String)IP[3];
  String b = "<meta http-equiv=\"refresh\" content=\""+(String)web_refresh+"; url="+a+"/config\"/>\n";
  String c = "PAGE DELETE. If page not refresh "+(String)web_refresh+" second please <a href=\""+a+"/config\">click here</a>\n";
  String d = "";
  d += "    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n";
  d += "      <div class=\"col-xs-12 col-sm-6 col-lg-4\">\n";
  d += "        <div class=\"form-group col-xs-12 col-sm-12 col-lg-12\">\n";
  if (SPIFFS.exists(file_config)) {
    SPIFFS.remove(file_config);
    d += "          <h1><font color=red>CONFIG REMOVED</font></h1>";
    d += c;
  } else {
    d += "          <h1><font color=red>CONFIG NOT EXISTS</font></h1>";
    d += c;
  }
  d += "        </div>\n";
  d += "      </div>\n";
  d += "    </div>\n";
  d += b;
  webServer.send(200,"text/html",b+handleHeader()+handleLogo()+handleHeader2()+d+handleFooter());
}
void handleReset() {
  String a = "http://"+(String)IP[0]+"."+(String)IP[1]+"."+(String)IP[2]+"."+(String)IP[3];
  String b = "<meta http-equiv=\"refresh\" content=\""+(String)web_restart+"; url="+a+"\"/>\n";
  String c = "PAGE RESET. If page not refresh "+(String)web_restart+" second please <a href=\""+a+"\">click here</a>\n";
  String d = "";
  d += "    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n";
  d += "      <div class=\"col-xs-12 col-sm-6 col-lg-4\">\n";
  d += "        <div class=\"form-group col-xs-12 col-sm-12 col-lg-12\">\n";
  d += "          <h1><font color=red>ESP RESET</font></h1>";
  d += c;
  d += "        </div>\n";
  d += "      </div>\n";
  d += "    </div>\n";
  d += b;
  webServer.send(200,"text/html",b+handleHeader()+handleLogo()+handleHeader2()+d+handleFooter());
#ifdef DEBUG_LOEI
  cout.print(getPrompt());
  cout.println("ESP Reset");
#endif
  delay(bdelay);
  ESP.reset();
  delay(bdelay);
}
void handleRestart() {
  String a = "http://"+(String)IP[0]+"."+(String)IP[1]+"."+(String)IP[2]+"."+(String)IP[3];
  String b = "<meta http-equiv=\"refresh\" content=\""+(String)web_restart+"; url="+a+"\"/>\n";
  String c = "PAGE RESTART. If page not refresh "+(String)web_restart+" second please <a href=\""+a+"\">click here</a>\n";
  String d = "";
  d += "    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n";
  d += "      <div class=\"col-xs-12 col-sm-6 col-lg-4\">\n";
  d += "        <div class=\"form-group col-xs-12 col-sm-12 col-lg-12\">\n";
  d += "          <h1><font color=red>ESP RESTART</font></h1>";
  d += c;
  d += "        </div>\n";
  d += "      </div>\n";
  d += "    </div>\n";
  d += b;
  webServer.send(200,"text/html",b+handleHeader()+handleLogo()+handleHeader2()+d+handleFooter());
#ifdef DEBUG_LOEI
  cout.print(getPrompt());
  cout.println("ESP Restart");
#endif
  delay(bdelay);
  ESP.restart();
  delay(bdelay);
}
void handler() {
  webServer.onNotFound(handleNotFound);
  webServer.on("/",handleRoot);
  webServer.on("/config",handleConfig);
  webServer.on("/config/",handleConfig);
  webServer.on("/config/gpio",handleGPIO);
  webServer.on("/config/advconfig",handleAdvConfig);
  webServer.on("/config/save",handleSave);
  webServer.on("/config/show",handleShow);
  webServer.on("/config/delete",handleDelete);
  webServer.on("/config/reset",handleReset);
  webServer.on("/config/restart",handleRestart);
  webServer.begin();
}

/******** FACTORY ********/
/*
 * factory_reset(int keyb=BUTTON_PIN, int led=LED_PIN)  -- run/ap/clear config
 * 
 */
int factory_reset(int keyb=BUTTON_PIN, int led=LED_PIN) {
  bool a,b,c,d,e,f;
  int delay_factory_begin     = cdelay;
  int delay_factory_ledon     = cdelay;
  int delay_factory_ledoff    = cdelay;
  int delay_factory_blinkon   = bdelay;
  int delay_factory_blinkoff  = bdelay;
  digitalWrite(keyb,HIGH);
  digitalWrite(led,HIGH);
  pinMode(keyb,INPUT);
  pinMode(led,OUTPUT);
  delay(delay_factory_begin);
  a = digitalRead(keyb);
  digitalWrite(led,LOW);
  delay(delay_factory_ledon);
  b = digitalRead(keyb);
  digitalWrite(led,HIGH);
  delay(delay_factory_ledoff);
  c = digitalRead(keyb);
  if (a==1 && b==0 && c==1) {
    digitalWrite(led,LOW);  delay(delay_factory_blinkon); digitalWrite(led,HIGH); delay(delay_factory_blinkoff);
    digitalWrite(led,LOW);  delay(delay_factory_blinkon); digitalWrite(led,HIGH); delay(delay_factory_blinkoff);
    digitalWrite(led,LOW);  delay(delay_factory_blinkon); digitalWrite(led,HIGH); delay(delay_factory_blinkoff);
    d = digitalRead(keyb);
    digitalWrite(led,LOW);
    delay(delay_factory_ledon);
    e = digitalRead(keyb);
    digitalWrite(led,HIGH);
    delay(delay_factory_ledoff);
    f = digitalRead(keyb);
    if (a==1 && b==0 && c==1 && d==1 && e==1 && f==1) {
      return 2;
    } else if (a==1 && b==0 && c==1 && d==0 && e==0 && f==1) {
      return 3;
    }
  } else {
    return 1;
  }
}

/******** LOADER ********/
/*
 * loader()                                             -- Prepare
 * load_ap_without_config()                             -- AP without Config
 * load_ap_with_clear_config()                          -- AP with Clear Config
 * load_runnable()                                      -- Runnable
 */
void loader() {
  digitalWrite(RELAY_PIN, RELAY_OFF);
  digitalWrite(LED_PIN, LED_OFF);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  delay(adelay);
  cout.end();
  delay(adelay);
#ifdef DEBUG_LOEI
  cout.setDebugOutput(true);
#endif
  cout.begin(baudrate);
  fs.begin();
}
void load_ap_without_config(bool a=false) {
  if (a) { loadConfig(true); }
  wifiap(getWiFiAPName(wifiapSSID, true).c_str(), (const char*)wifiapPSK);
}
void load_ap_with_clear_config() {
  if (SPIFFS.exists(file_config)) {
    SPIFFS.remove(file_config);
  }
  load_ap_without_config(false);
}
void load_runnable() {
  if (loadConfig(true)) {
    if (!wificonnect((const char*)wifiSSID, (const char*)wifiPSK)) {
      load_ap_without_config(false);
    }
  } else {
    load_ap_without_config(false);
  }
}

/******** MAIN ********/
/*
 * setup()                                              -- Main Setup
 * loop()                                               -- Main Loop
 */
void button_loop() {
  int a = digitalRead(BUTTON_PIN);
  if (a != state_button) {
    state_button = a;
    microgear_sendState(false);
    Serial.println(state_button);
  }
}
void setup() {
  loader();
  cout.println();
  cout.println(getHello());
  fac = factory_reset(BUTTON_PIN, LED_PIN);

#ifdef DEBUG_LOEI
  cout.print(getPrompt());
  cout.println("Debug Mode = ON");
  cout.print(getPrompt());
  cout.print("Factory Mode = ");
  cout.print(fac);
  cout.print(" -- ");
  switch (fac) {
    case 1: cout.println("STA Mode to Runnable");       break;
    case 2: cout.println("AP Mode without Config");     break;
    case 3: cout.println("AP Mode with Clear Config");  break;
  }
#endif

  switch (fac) {
    case 1: load_runnable();                            break;
    case 2: load_ap_without_config(true);               break;
    case 3: load_ap_with_clear_config();                break;
  }
  handler();
  microgear_setup();

#ifdef DEBUG_LOEI
  cout.print(getPrompt());
  cout.println("Setup OK.");
#endif
}
void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
  microgear_loop();
  button_loop();
}
