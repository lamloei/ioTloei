/**
 * Firmware Name:   ioTloei
 * Description:     Config ESP8266 family to WIFI and/or BROKER
 * Author:          lamloei
 * Author URI:      http://www.lamloei.com
 * Facebook URI:    https://www.facebook.com/lamloeicom
 * Github URI:      https://github.com/lamloei
 * Modified Date:   2016071401
 * Version:         ALPHA A1
 */
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <MicroGear.h>
#include "FS.h"

/**** Parameter ****/
char* file_config = "/config.json";
char* wifiapSSID = "ESP_IOTLOEI";
int wifi_delay = 500;
int adelay = 3000;
int baudrate = 115200;
/******************/
#define D4 2
#define D5 14
#define D6 12
#define D7 13
/******************/
char* token_netpie = "netpie.io";
/******************/

StaticJsonBuffer<400> jsonBuffer;
char json[400];
char wifiSSID[20];
char wifiPSK[20];
char broker[20];
char netpie_appid[20];
char netpie_appkey[40];
char netpie_appsecret[60];
char netpie_appalias[20];
int loadcf = 0;
int state = 0;

IPAddress IP;
ESP8266WebServer webServer(80);
WiFiClient client;
MicroGear microgear(client);

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
  return "Alpha<br/>A1<wbr/>Netpie";
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
      if (root.containsKey("netpie_appid"))       { strcpy(netpie_appid,root["netpie_appid"]); }
      if (root.containsKey("netpie_appkey"))      { strcpy(netpie_appkey,root["netpie_appkey"]); }
      if (root.containsKey("netpie_appsecret"))   { strcpy(netpie_appsecret,root["netpie_appsecret"]); }
      if (root.containsKey("netpie_appalias"))    { strcpy(netpie_appalias,root["netpie_appalias"]); }
    }
  }
  return true;
}

/******** Handle ********/
/*
 * handleNotFound()     - Show Page if Not Found
 * handleConfig()       - Config Form
 * handleGPIO()         - Test GPIO
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
"          <a href=\"gpio\">GPIO</a>\n"
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
"      <div id=\"netpie\" class=\"collapse in\">\n"
"        <div class=\"form-group col-xs-12 col-sm-6 col-lg-3\">\n"
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
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"form-group col-xs-12 col-sm-6 col-lg-3\">\n"
"        <button class=\"btn btn-info form-control\" type=\"submit\">SAVE &amp; RESTART</button>\n"
"      </div>\n"
"    </div>\n"
"  </div>\n"
"<input id=\"broker\" name=\"broker\" value=\"netpie.io\" type=\"hidden\"/>\n"
"<input id=\"formname\" name=\"formname\" value=\"iotloei_config\" type=\"hidden\"/>\n"
"</form>\n"
"</body></html>"
);
}
void handleGPIO() {
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
"  <link href=\"https://gitcdn.github.io/bootstrap-toggle/2.2.2/css/bootstrap-toggle.min.css\" rel=\"stylesheet\">\n"
"  <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/2.2.4/jquery.min.js\"></script>\n"
"  <script src=\"http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js\"></script>\n"
"  <script src=\"https://gitcdn.github.io/bootstrap-toggle/2.2.2/js/bootstrap-toggle.min.js\"></script>\n"
"  <script src=\"https://netpie.io/microgear.js\"></script>\n"
"<script>\n"
"/**** parameter ****/\n"
"  const appid = \""+String(netpie_appid)+"\";\n"
"  const appkey = \""+String(netpie_appkey)+"\";\n"
"  const appsecret = \""+String(netpie_appsecret)+"\";\n"
"  const appalias = \""+String(netpie_appalias)+"\"; \n"
"/********/  \n"
"</script>\n"
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
"          "+getVersion()+"<div id=\"data\" class=\"data\" align=\"right\"><font color=\"#DDDDDD\">OFFLINE</font></div>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-3\">\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          D4\n"
"        </div>\n"
"        <div class=\"form-group col-xs-4 col-sm-4 col-lg-4\">\n"
"          <a onclick=\"D4_toggle()\"><input id=\"D4b\" class=\"D4b\" data-toggle=\"toggle\" data-onstyle=\"success\" data-offstyle=\"danger\" data-style=\"slow\" type=\"checkbox\"></a>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-3\">\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          D5\n"
"        </div>\n"
"        <div class=\"form-group col-xs-4 col-sm-4 col-lg-4\">\n"
"          <a onclick=\"D5_toggle()\"><input id=\"D5b\" class=\"D5b\" data-toggle=\"toggle\" data-onstyle=\"success\" data-offstyle=\"danger\" data-style=\"slow\" type=\"checkbox\"></a>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-3\">\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          D6\n"
"        </div>\n"
"        <div class=\"form-group col-xs-4 col-sm-4 col-lg-4\">\n"
"          <a onclick=\"D6_toggle()\"><input id=\"D6b\" class=\"D6b\" data-toggle=\"toggle\" data-onstyle=\"success\" data-offstyle=\"danger\" data-style=\"slow\" type=\"checkbox\"></a>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-3\">\n"
"        <div class=\"form-group col-xs-5 col-sm-5 col-lg-5\">\n"
"          D7\n"
"        </div>\n"
"        <div class=\"form-group col-xs-4 col-sm-4 col-lg-4\">\n"
"          <a onclick=\"D7_toggle()\"><input id=\"D7b\" class=\"D7b\" data-toggle=\"toggle\" data-onstyle=\"success\" data-offstyle=\"danger\" data-style=\"slow\" type=\"checkbox\"></a>\n"
"        </div>\n"
"      </div>\n"
"    </div>\n"
"    <div class=\"col-xs-12 col-sm-12 col-lg-12\">\n"
"      <div class=\"col-xs-12 col-sm-6 col-lg-3\">\n"
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
"  </div>\n"
"<script>\n"
"  var D4s = 1;\n"
"  var D5s = 3;\n"
"  var D6s = 5;\n"
"  var D7s = 7;\n"
"  var microgear = Microgear.create({\n"
"    appid: appid,\n"
"    key: appkey,\n"
"    secret: appsecret,\n"
"    alias : appalias\n"
"  });\n"
"  microgear.on('message',function(topic,msg) {\n"
"    switch (msg) {\n"
"      case '1': $('.D4b').bootstrapToggle('off'); break;\n"
"      case '2': $('.D4b').bootstrapToggle('on');  break;\n"
"      case '3': $('.D5b').bootstrapToggle('off'); break;\n"
"      case '4': $('.D5b').bootstrapToggle('on');  break;\n"
"      case '5': $('.D6b').bootstrapToggle('off'); break;\n"
"      case '6': $('.D6b').bootstrapToggle('on');  break;\n"
"      case '7': $('.D7b').bootstrapToggle('off'); break;\n"
"      case '8': $('.D7b').bootstrapToggle('on');  break;\n"
"    }\n"
"  });\n"
"  microgear.on('connected', function() {\n"
"    microgear.subscribe('/'+appalias+'/state');\n"
"    microgear.chat(appalias,'?');\n"
"    document.getElementById(\"data\").innerHTML = \"<font color=\\\"#00BB00\\\">ONLINE</font>\";\n"
"  });\n"
"  microgear.connect(appid);\n"
"  function D4_toggle() {\n"
"    if (D4s == 1)       { D4s = 2; microgear.chat(appalias,D4s.toString()); }\n"
"    else if (D4s == 2)  { D4s = 1; microgear.chat(appalias,D4s.toString()); }\n"
"  }\n"
"  function D5_toggle() {\n"
"    if (D5s == 3)       { D5s = 4; microgear.chat(appalias,D5s.toString()); }\n"
"    else if (D5s == 4)  { D5s = 3; microgear.chat(appalias,D5s.toString()); }\n"
"  }\n"
"  function D6_toggle() {\n"
"    if (D6s == 5)       { D6s = 6; microgear.chat(appalias,D6s.toString()); }\n"
"    else if (D6s == 6)  { D6s = 5; microgear.chat(appalias,D6s.toString()); }\n"
"  }\n"
"  function D7_toggle() {\n"
"    if (D7s == 7)       { D7s = 8; microgear.chat(appalias,D7s.toString()); }\n"
"    else if (D7s == 8)  { D7s = 7; microgear.chat(appalias,D7s.toString()); }\n"
"  }\n"
"  function aState() {\n"
"    microgear.subscribe('/'+appalias+'/state');\n"
"    microgear.chat(appalias,'?');\n"
"  }\n"
"  function atoggleOn() {\n"
"    D4s =2;\n"
"    D5s =4;\n"
"    D6s =6;\n"
"    D7s =8;\n"
"    $('.D4b').bootstrapToggle('on');\n"
"    $('.D5b').bootstrapToggle('on');\n"
"    $('.D6b').bootstrapToggle('on');\n"
"    $('.D7b').bootstrapToggle('on');\n"
"    D4s = 2; microgear.chat(appalias,D4s.toString());\n"
"    D5s = 4; microgear.chat(appalias,D5s.toString());\n"
"    D6s = 6; microgear.chat(appalias,D6s.toString());\n"
"    D7s = 8; microgear.chat(appalias,D7s.toString());\n"
"  }\n"
"  function atoggleOff() {\n"
"    D4s =1;\n"
"    D5s =3;\n"
"    D6s =5;\n"
"    D7s =7;\n"
"    $('.D4b').bootstrapToggle('off');\n"
"    $('.D5b').bootstrapToggle('off');\n"
"    $('.D6b').bootstrapToggle('off');\n"
"    $('.D7b').bootstrapToggle('off');\n"
"    D4s = 1; microgear.chat(appalias,D4s.toString());\n"
"    D5s = 3; microgear.chat(appalias,D5s.toString());\n"
"    D6s = 5; microgear.chat(appalias,D6s.toString());\n"
"    D7s = 7; microgear.chat(appalias,D7s.toString());\n"
"  }\n"
"</script>\n"
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
    if ((strcmp(broker,token_netpie)==0) && (strcmp(netpie_appid,"")!=0)) {
      if (microgear.connected()) {
        //microgear.disconnect();
      }
      microgear.on(MESSAGE,microgear_onMsghandler);
      microgear.on(CONNECTED,microgear_onConnected);
      microgear.init(netpie_appkey,netpie_appsecret,netpie_appalias);
      microgear.connect(netpie_appid);
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

/******** Microgear ********/
void microgear_sendState() {
  String a = "/"+String(netpie_appalias)+"/state";
  char b[40];
  a.toCharArray(b,a.length()+1);
  if (digitalRead(D4)==HIGH)  { microgear.publish(b,"1"); }
  if (digitalRead(D4)==LOW)   { microgear.publish(b,"2"); }
  if (digitalRead(D5)==HIGH)  { microgear.publish(b,"3"); }
  if (digitalRead(D5)==LOW)   { microgear.publish(b,"4"); }
  if (digitalRead(D6)==HIGH)  { microgear.publish(b,"5"); }
  if (digitalRead(D6)==LOW)   { microgear.publish(b,"6"); }
  if (digitalRead(D7)==HIGH)  { microgear.publish(b,"7"); }
  if (digitalRead(D7)==LOW)   { microgear.publish(b,"8"); }
}
void microgear_updateIO() {
  Serial.print("Incoming message --> ");
  Serial.println(String(state));
  if (state==1) { digitalWrite(D4,HIGH); }
  if (state==2) { digitalWrite(D4,LOW); }
  if (state==3) { digitalWrite(D5,HIGH); }
  if (state==4) { digitalWrite(D5,LOW); }
  if (state==5) { digitalWrite(D6,HIGH); }
  if (state==6) { digitalWrite(D6,LOW); }
  if (state==7) { digitalWrite(D7,HIGH); }
  if (state==8) { digitalWrite(D7,LOW); }
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
  Serial.println("Connected to NETPIE...");
}

/******** Main ********/
void setup() {
  digitalWrite(D4,HIGH);
  digitalWrite(D5,HIGH);
  digitalWrite(D6,HIGH);
  digitalWrite(D7,HIGH);
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
  webServer.on("/gpio",handleGPIO);
  webServer.on("/config.json",handleConfigJson);
  webServer.on("/save",handleSave);
  webServer.on("/clear",handleClear);
  webServer.on("/new",handleNew);
  webServer.on("/reset",handleReset);
  webServer.on("/restart",handleRestart);
  webServer.begin();

  Serial.println("Broker Connecting...");
  if ((strcmp(broker,token_netpie)==0) && (strcmp(netpie_appid,"")!=0)) {
    microgear.on(MESSAGE,microgear_onMsghandler);
    microgear.on(CONNECTED,microgear_onConnected);
    microgear.init(netpie_appkey,netpie_appsecret,netpie_appalias);
    microgear.connect(netpie_appid);
  }
  Serial.println("setup OK.");
}

void loop() {
  webServer.handleClient();
  if ((strcmp(broker,token_netpie)==0) && (strcmp(netpie_appid,"")!=0)) {
    if (microgear.connected()) {
      microgear.loop();
    }
    else {
      Serial.println("connection lost, reconnect...");
      microgear.connect(netpie_appid);
    }
  }
}
