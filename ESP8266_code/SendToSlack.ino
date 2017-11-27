#include <ESP8266WiFi.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include "WiFiManager.h"  

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

//#include "images.h"

#define ssid "iotmaker_nho"
#define password "@iotmaker.vn"
//#define ssid "OISP_PUBLIC"
//#define password "bachkhoaquocte2017"
#define mqtt_server "iot.eclipse.org"
#define mqtt_topic_pub_Register "/Server/Register"
#define mqtt_topic_pub_toSlack "/Server/toSlack"
#define mqtt_topic_sub_Register "/ESP8266/Register"
//#define mqtt_user "okzxtdhr"
//#define mqtt_pwd "vFlJrrfn3lf0"
#define led 16
const uint16_t mqtt_port = 1883;

char msg[50];
int flag = 0;
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
int value = 0;

int getFingerprintIDez();
SoftwareSerial mySerial(15, 13);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

SSD1306  display(0x3c, 4, 5);

void setup() {
  // Set up OLED
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  //drawFontFaceDemo();
  
  delay(3000);
  
  Serial.begin(115200);
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    display.clear();
    display.drawString(2,10, "Found fingerprint\nsensor!");
    display.display();
    delay(3000);
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    display.clear();
    display.drawString(2,10, "Did not find\nfingerprint sensor");
    display.display();
    while (1);
  }
  Serial.println("Waiting for valid finger...");
  setup_wifi();
  display.clear();
  display.drawString(9,10, "Connected Wifi");
  display.display();
  delay(3000);
  Serial.println("Connected......");
  pinMode(led,OUTPUT);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void configModeCallback (WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup_wifi() {
  /*delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  */
  
  WiFiManager wifiManager;
  //wifiManager.resetSettings();
  display.clear();
  display.drawString(16,10, "Wifi Manager");
  display.drawString(6,26, "IP: 192.168.4.1");
  display.display();
  delay(3000);
  wifiManager.setAPCallback(configModeCallback);
  if (!wifiManager.autoConnect())
  {
    Serial.println("failed to connect and hit timeout");
    ESP.reset();
    delay(1000);
  } 
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String s = "";
  for (int i = 6; i < length; i++) {
    Serial.print((char)payload[i]);
    /*if ((char)payload[i]=='#')
      break;*/
    s+=(char)payload[i];
  }
  display.clear();
  display.drawString(10,10, "Register ID");
  display.display();
    while (!getFingerprintEnroll(s.toInt()));
  if (flag == 1){
    display.clear();
    display.drawString(16,10, "Complete");
    display.display();
    client.publish(mqtt_topic_pub_Register, "ok");
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("/Server")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("/MQTTLen", "ESP_reconnected");
      // ... and resubscribe
      client.subscribe(mqtt_topic_sub_Register);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
} 


uint8_t getFingerprintEnroll(int id) {
  flag = 0;
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  display.clear();
  display.drawString(8,10, "Remove finger");
  display.display();
  delay(1000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  display.clear();
  display.drawString(0,10, "    Place same\n   finger again");
  display.display();
  delay(2000);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    flag = 1;
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}

void drawFontFaceDemo() {
    // Font Demo1
    // create more fonts at http://oleddisplay.squix.ch/
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Hello world");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 10, "Hello world");
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 26, "Hello world");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  display.clear();
  display.drawString(2,10, "      Scan your\n     Fingerprint");
  display.display();
  int newID = getFingerprintIDez();
  if (newID != -1){
    //snprintf (msg, 75, (String)newID); 
    String S = (String)newID;
    for (int i = 0;i < S.length();i++)
      msg[i] = S[i];
    client.publish(mqtt_topic_pub_toSlack, msg);
    display.clear();
    display.drawString(20,0, "Accept!!!");
    display.display();
    delay(5000);
  }
}
