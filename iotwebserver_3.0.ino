#include<DHT.h>
#include<ESP8266WiFi.h>
#include<ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include<FS.h>
#include<LittleFS.h>
//#include<WiFiManager.h>
#include<ArduinoJson.h>
#include <WebSocketsServer.h>
#define FS_MAX_OPEN_FILES 10


//=======================VARIABBLES==================================//
int numOfUsers = 0;
bool loggedin = false;
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
String jsonString;
int count = 3;
bool ap = false;
bool sentry_mode = false;
const int reconInterval = 30000;
const int updateTime = 1500;
unsigned long previousTime = 0;
unsigned long previousUpdateTime = 0;
unsigned long previousNotifTime = 0;
const char* www_username = "admin";
const char* www_password = "esp8266";
const char* www_realm = "Custom Auth Realm";
String authFailResponse = "Authentication Failed";
const char* apSSID = "IOTSetup";
const char* apPassword = "ap_$y$admin-/-/-/";
const char* ssid;
const char* password;
const char* userid;
const char* userpassword;
#define buzzer D8
#define door1 D5
#define moisture A0
#define DHTTYPE DHT11
#define sensorpin D6
#define motion D7
DHT dht(sensorpin, DHTTYPE);
int waitTime = 15;
int notification_timeout = 0;
const char* host = "maker.ifttt.com";
const int httpsPort = 82;


//==================VOID SETUP============================//
void setup() {
  pinMode (door1, INPUT_PULLUP);
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.begin(9600);
  pinMode(motion, INPUT);
  pinMode(sensorpin, INPUT);
  dht.begin();
  bool filemount = LittleFS.begin();
  if (!filemount) {
    Serial.println("Failed to mount littlefs");
    return;
  } else {
    Serial.println("File system mounted successfully");
    attemptConnection();
  }

}

//======================VOID LOOP==================//

void loop()
{
  server.handleClient();
  webSocket.loop();
  unsigned long currentTime = millis();
  if ((WiFi.status() != WL_CONNECTED) && (currentTime - previousTime >= reconInterval) && !ap && count > 0) {
    tone(buzzer, 2000, 500);
    delay(100);
    tone(buzzer, 2000, 500);
    delay(100);
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    previousTime = currentTime;
    count--;
  }
  else if ((WiFi.status() != WL_CONNECTED) && count == 0) {
    ESP.restart();
  }
  if (currentTime - previousUpdateTime >= updateTime) {
    update_webpage();
    previousUpdateTime = currentTime;

  }
  if (sentry_mode) {
    if (digitalRead(door1) == HIGH||digitalRead(motion)==HIGH) {
      Serial.println("Door open");
      //      Serial.println(digitalRead(door1));
      buzzer_alert();
      
      if (currentTime - previousNotifTime >= notification_timeout) {
        notify();
        previousNotifTime = currentTime;

      }

    }
  }

}


//=================SERVING MAIN/LOGIN PAGE===============//

void serveloginpage() {
  File file = LittleFS.open("/main/login.html", "r");
  if (!file) {
    Serial.println("Failed to open login_html for reading");
    return;
  }
  Serial.println("opened login_html file for streaming");
  server.streamFile(file, "text/HTML");
  file.close();
}
void serveloginscript() {
  File file = LittleFS.open("/main/loginscript.js", "r");
  if (!file) {
    Serial.println("Failed to open login_script for reading");
    return;
  }
  Serial.println("opened login_script for streaming");
  server.streamFile(file, "text/javascript");
  file.close();
}
void serveloginstyle() {
  File file = LittleFS.open("/main/loginstyle.css", "r");
  if (!file) {
    Serial.println("Failed to open login_style for reading");
    return;
  }
  Serial.println("opened login_style for streaming");
  server.streamFile(file, "text/css");
  file.close();
}

void servepage() {
  if ((loggedin != 1) || (numOfUsers > 1)) {
    Serial.println("error");
    server.send(404);
    return;
  }
  numOfUsers += 1;
  File file = LittleFS.open("/main/index.html", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("opened file for streaming");
  server.streamFile(file, "text/HTML");
  file.close();
}

void servemainscript() {
  File file = LittleFS.open("/main/script.js", "r");
  server.streamFile(file, "text/javascript");
  file.close();
}

void servemaincss() {
  File file = LittleFS.open("/main/style.css", "r");
  server.streamFile(file, "text/css");
  file.close();
}


//======================SERVING SETUP PAGE====================//
void serveSetupPage() {
  if (!server.authenticate(www_username, www_password))
  {
    return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
  }
  File file = LittleFS.open("/setup/setup.html", "r");
  if (!file) {
    serveSetupPage();
  }
  else {
    server.streamFile(file, "text/html");
    file.close();
  }
}
void serveSetupscript() {
  File file = LittleFS.open("/setup/setupscript.js", "r");
  if (!file) {
    serveSetupscript();
  }
  else {
    server.streamFile(file, "text/javascript");
    file.close();
  }
}
void serveSetupstyle() {
  File file = LittleFS.open("/setup/setupstyle.css", "r");
  if (!file) {
    serveSetupstyle();
  }
  else {
    server.streamFile(file, "text/css");
    file.close();
  }
}



//======================ACCEPTING AND STORING WIFI CREDENTIALS====================//
void storeCredentials() {
  String cred = "SSID is:" + server.arg("id") + "\nPassword is: " + server.arg("pass");
  const char* id = (server.arg("id")).c_str();
  const char* pass = (server.arg("pass")).c_str();
  Serial.print(cred);

  if (saveConfig(id, pass)) {
    server.send(200, "text/plane", "Credentials saved. You may close this tab");
    Serial.println("Restarting system.Please Standby.");
    delay(5000);
    server.stop();
    WiFi.softAPdisconnect (true);
    delay(5000);
    Serial.println("bye");
    ESP.restart();
  }
  if (!saveConfig(id, pass)) {
    server.send(200, "text/plane", "Failed to save credentials. Please try again");
    Serial.println("failed to save config");
  }


}


//======Chekcing for WiFi Credentials and Starting as Access point on failing.================//

void beginAccessPoint() {
  tone(buzzer, 2000, 50);
  delay(100);
  tone(buzzer, 2000, 50);
  delay(100);
  Serial.println();
  Serial.println("Cannot find network !! Starting AP");
  WiFi.softAP(apSSID, apPassword);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("Aaccess point started at : ");
  ap = true;
  Serial.println(myIP);
  server.on("/", serveSetupPage);
  server.on("/setupscript.js", serveSetupscript);
  server.on("/setupstyle.css", serveSetupstyle);
  server.on("/authdata", storeCredentials);
  server.on("/restart", Restart);
  server.begin();
  waitTime = 15;
}


//==============AUTOCONNECTING TO PRELOADED CREDENTIALS==============//
void autoConnct() {
  ap = false;
  server.close();
  server.stop();
  WiFi.softAPdisconnect (true);
  Serial.println();
  Serial.println("Connected!");
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());
  server.on("/", serveloginpage);
  server.on("/loginscript.js", serveloginscript);
  server.on("/loginstyle.css", serveloginstyle);
  server.on("/index", servepage);
  server.on("/style.css", servemaincss);
  server.on("/script.js", servemainscript);
  server.on("/toggledevice", toggleDevice);
  server.on("/login", authenticate_user);
  server.on("/create", create);
  server.on("/sentry", handleSentryMode);
  server.begin(80);
  tone(buzzer, 1000, 100);
  delay(200);
  tone(buzzer, 2000, 50);
  delay(100);
  tone(buzzer, 1000, 100);
  delay(1000);
}


//=============================Starting connection=================//
void attemptConnection() {
  if (!loadConfig()) {
    Serial.println("Could not load credentials!");
  }
  Serial.println(ssid);
  Serial.println(password);
  WiFi.begin(String(ssid).c_str(), String(password).c_str());
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
    if (waitTime == 0 && WiFi.status() != WL_CONNECTED) {
      beginAccessPoint();
      break;
    }
    waitTime--;

  }
  if (WiFi.status() == WL_CONNECTED) {
    autoConnct();
  }
}


//====================Saving info to config file===================//

bool saveConfig(const char* id, const char* pass) {
  StaticJsonDocument<200> doc;
  doc["ssid"] = id;
  doc["password"] = pass;

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, configFile);
  return true;
}

//================Get info from config file===================//

bool loadConfig() {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }
  if (size == 0) {
    Serial.println("Config file is empty");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<200> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
    return false;
  }

  ssid = doc["ssid"];
  password = doc["password"];


  Serial.println("Loaded ssid");
  Serial.print(ssid);
  Serial.println("\nLoaded password: ");
  Serial.print(password);
  Serial.println();
  return true;
}

void Restart() {
  delay(3000);
  server.send(200, "text/plain", "System Restarting in 5 Seconds ...");
  delay(2500);
  WiFi.softAPdisconnect (true);
  server.stop();
  delay(2500);
  ESP.restart();
}

//====================UPDATE SENSOR VALUE ON WEBPAGE===================//
void update_webpage()
{
  float t = dht.readTemperature()  ;
  float h = dht.readHumidity() ;
  int door = digitalRead(door1);
  int moist_val = analogRead(moisture); 
  moist_val = constrain(moist_val,400,1023);  
  moist_val = map(moist_val,400,1023,100,0);
  //  Serial.println(t);
  //  Serial.println(h);
  StaticJsonDocument<120> doc;
  // create an object
  JsonObject object = doc.to<JsonObject>();
  //  object["PIN_Status"] = pin_status ;
  object["Temp"] = round(t) ;
  object["Hum"] = round(h) ;
  object["moist"]=moist_val;
  object["door"] = door;
  serializeJson(doc, jsonString); // serialize the object and save teh result to teh string variable.
  //Serial.println(jsonString); // print the string for debugging.
  webSocket.broadcastTXT(jsonString); // send the JSON object through the websocket
  jsonString = ""; // clear the String.
}

//=====================websocket event======================================//

void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED: // enum that read status this is used for debugging.
      Serial.print("WS Type ");
      Serial.print(type);
      Serial.println(": DISCONNECTED");
      loggedin = false;
      numOfUsers = 0;
      break;
    case WStype_CONNECTED:  // Check if a WebSocket client is connected or not
      Serial.print("WS Type ");
      Serial.print(type);
      Serial.println(": CONNECTED");
      break;
    case WStype_TEXT: // check responce from client
      Serial.println(); // the payload variable stores teh status internally
      Serial.println(payload[0]);
      //      toggleDevice(payload[0]);
      break;
  }
}

//===========================TOGGLE DEVICE=================//

void toggleDevice() {
  String Pinnum = (server.arg("devid"));
  int pinnum = Pinnum.toInt();
  switch (pinnum) {
    case 1:
      pinMode(16, OUTPUT);
      if (digitalRead(16) == LOW) {
        digitalWrite(16, HIGH);
      }
      else {
        digitalWrite(16, LOW);
      }
      server.send(200, "ok");
      break;
    case 2:
      pinMode(5, OUTPUT);
      if (digitalRead(5) == LOW) {
        digitalWrite(5, HIGH);
      }
      else {
        digitalWrite(5, LOW);
      }
      server.send(200, "ok");
      break;

    case 3:
      pinMode(4, OUTPUT);
      if (digitalRead(4) == LOW) {
        digitalWrite(4, HIGH);
      }
      else {
        digitalWrite(4, LOW);
      }
      server.send(200, "ok");
      break;
    case 4:
      pinMode(0, OUTPUT);
      if (digitalRead(0) == LOW) {
        digitalWrite(0, HIGH);
      }
      else {
        digitalWrite(0, LOW);
      }
      server.send(200, "ok");
      break;
    case 5:
      pinMode(2, OUTPUT);
      if (digitalRead(2) == LOW) {
        digitalWrite(2, HIGH);
      }
      else {
        digitalWrite(2, LOW);
      }
      server.send(200, "ok");
      break;
    case 6:
      pinMode(14, OUTPUT);
      if (digitalRead(14) == LOW) {
        digitalWrite(14, HIGH);
      }
      else {
        digitalWrite(14, LOW);
      }
      server.send(200, "ok");
      break;
    case 7:
      pinMode(12, OUTPUT);
      if (digitalRead(12) == LOW) {
        digitalWrite(12, HIGH);
      }
      else {
        digitalWrite(12, LOW);
      }
      server.send(200, "ok");
      break;
  }
}

//==================================CREATE USER=====================//
void create() {
  const char* uid = server.arg("id").c_str();
  const char* upass = server.arg("pass").c_str();
  StaticJsonDocument<200> doc;
  doc["usrid"] = uid;
  doc["usrpass"] = upass;
  File configFile = LittleFS.open("/userinfo.json", "w");
  if (!configFile) {
    Serial.println("Failed to open userinfo file for writing");
  }
  serializeJson(doc, configFile);
  Serial.println("User created");
  Serial.println("User ID:");
  Serial.print(uid);
  Serial.println("\nUser password:");
  Serial.println(upass);

}

//==================AUTHENTICATE USER================//

bool authenticate_user() {
  const char* uid = server.arg("id").c_str();
  const char* usrpass = server.arg("pass").c_str();
  Serial.println("Web Arguments");
  Serial.println(uid);
  Serial.println(usrpass);

  File userInfoFile = LittleFS.open("/userinfo.json", "r");
  if (!userInfoFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = userInfoFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }
  if (size == 0) {
    Serial.println("Config file is empty");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);

  userInfoFile.readBytes(buf.get(), size);

  StaticJsonDocument<200> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
    return false;
  }
  userid = doc["usrid"];
  userpassword = doc["usrpass"];
  Serial.println("Loaded credentials");
  Serial.println(userid);
  Serial.println(userpassword);

  if ((strcmp(userid, uid) != 0) || (strcmp(userpassword, usrpass) != 0)) {
    Serial.println("No such account found");
    server.send(200, "text/plane", "No such account exists");
    return false;
  }
  loggedin = true;
  Serial.println("User found");
  server.send(200);
  //  servepage;
  Serial.println("User:");
  Serial.print(userid);
  Serial.println("\nPassword: ");
  Serial.print(userpassword);
  Serial.println();
  Serial.println(loggedin);
  Serial.println(numOfUsers);
  return true;
}
void handleSentryMode() {
  const char* todo = server.arg("do").c_str();
  const char* condition = "on";
  Serial.println(server.arg("do"));
  if (strcmp(todo, "on") == 0) {
    sentry_mode = true;
    Serial.println("sentry engaged");
    server.send(200);
    return;
  }
  sentry_mode = false;
  Serial.println("Sentry dis-engaged");
  return;
}

void notify() {
  notification_timeout = 60000;
  WiFiClient client;
  HTTPClient http;  //Declare an object of class HTTPClient

  http.begin(client, "http://maker.ifttt.com/trigger/alert_intrusion/json/with/key/du78iGCQQtpqh5bvCNnDJN"); //Specify request destination
  int httpCode = http.GET(); //Send the request
  Serial.println(httpCode);

  if (httpCode > 0) { //Check the returning code

    String payload = http.getString();   //Get the request response payload
    Serial.println(1);                     //Print the response payload

  }

  http.end();

}
void buzzer_alert(){
  digitalWrite(D8,HIGH);
  delay(500);
  digitalWrite(D8,LOW);
  
}
