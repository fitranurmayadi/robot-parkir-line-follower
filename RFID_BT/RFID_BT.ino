#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h> // WIFI library

#include <HardwareSerial.h>
#include <PubSubClient.h>   //include mqtt pub sub
#include <WiFiClient.h>     // include mqtt
#include <ArduinoJson.h>    //parse doc to JSON
#include <MFRC522.h>        // include RFID
#include <HardwareSerial.h> //Include BT
// include I2C library
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
String messageToScroll = "SELAMAT DATANG DI PB PARKIR SILAHKAN TEMPELKAN RFID ANDA";

//untuk bluetooth
boolean DEBUG_IS = true;
String data_bt = " ";
String slot;
String bayar;
String tunggak;
String keluar = "GB";

//inisiasi object untuk menyimpan mqtt topic
char message_buff[100];
char bt_address[100];

//set username and password mqtt broker
const char *mqttServer = "----------";
const int mqttPort = 1883;
const char *mqttUser = "---------";
const char *mqttPassword = "--------";

// set sssid and pass for WIFI
const char *ssid = "----------";
const char *pass = "----------";

// Configurasi pin
#define RST_PIN 15
#define SS_PIN 5

//inisiasi task
TaskHandle_t Handle_ViewLCD;
TaskHandle_t Handle_RFID;

// Create MFRC522 instance
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

//inisiasi client
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


//declare scrolltext
void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i = 0; i < 20; i++) {
    message = " " + message;
  }
  message = message + " ";
  for (int pos = 0; pos < message.length(); pos++)  {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + 20));
    delay(delayTime);
  }
}

void send_to_bt(String command, int time_delay, boolean DEBUG) {
  data_bt = "";
  Serial.println(command);
  Serial2.println(command);
  Serial2.flush();
  delay(time_delay);
  while (Serial2.available() <= 0);
  char a;
  while (a != '\r')
  {
    if (Serial2.available())
    {
      char c = Serial2.read();
      a = c;
      data_bt += c;
    }
  }
  if (DEBUG)
    Serial.println(data_bt);
}


void setup_bt(String bt_address)
{
  //setup bluetooth
  String pair = "at+pair=" + String(bt_address) + ",10";
  String link = "at+link=" + String(bt_address);
  send_to_bt("at+init", 1000, DEBUG_IS);
  send_to_bt("at+disc", 1000, DEBUG_IS);
  send_to_bt(pair, 1000, DEBUG_IS);
  send_to_bt(link, 3000, DEBUG_IS);
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client", mqttUser, mqttPassword))
    {
      client.subscribe("esp32/slot");
      client.subscribe("esp32/bayar");
      client.subscribe("esp32/tunggak");
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void DisplayLCD(void *param) {
  Serial.print("Task LCD running on core ");
  Serial.println(xPortGetCoreID());
  while (1) {
    scrollText(0, messageToScroll, 250, 20);
  }
}

void RFID_Task(void *param) {
  Serial.print("Task RFID running on core ");
  Serial.println(xPortGetCoreID());
  while (1) {
    // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
    for (byte i = 0; i < 6; i++)
      key.keyByte[i] = 0xFF;
    byte block;
    byte len;

    if (!mfrc522.PICC_IsNewCardPresent())
    {
      return;
    }
    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial())
    {
      return;
    }
    // looping topic mqtt
    if (!client.connected())
    {
      reconnect();
    }

    long now = millis();
    if (now - lastMsg > 5000)
    {
      lastMsg = now;

      //Show UID on serial monitor
      Serial.print("UID tag :");
      String content = "";
      for (byte i = 0; i < mfrc522.uid.size; i++)
      {
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      Serial.println(content);
      Serial.println();
      content.toCharArray(message_buff, content.length() + 1);

      // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
      byte buffer1[18];
      block = 4;
      len = 18;
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK)
      {
        Serial.print(F("Authentication failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
      }
      status = mfrc522.MIFARE_Read(block, buffer1, &len);
      if (status != MFRC522::STATUS_OK)
      {
        Serial.print(F("Reading failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
      }
      String value = "";
      for (uint8_t i = 0; i < 16; i++)
      {
        value += (char)buffer1[i];
      }
      value.trim();
      Serial.print(value);
      value.toCharArray(bt_address, value.length() + 1);
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();

      //jika ada rfid terbaca, kirim datanya ke server mqtt
      // fungsi publish data dalam bentuk doc
      StaticJsonDocument<200> doc;
      doc["uid"] = message_buff;
      doc["bt_add"] = bt_address;
      char JSONmessageBuffer[100];
      serializeJson(doc, JSONmessageBuffer);
      client.publish("esp32/uid", JSONmessageBuffer);
      delay(1000);

      if (bt_address != 0)
      {
        if (slot != 0)
        {
          setup_bt(bt_address);
          send_to_bt("A1", 1000, DEBUG_IS);
          send_to_bt("at+disc", 1000, DEBUG_IS);
        }
        else
        {
          setup_bt(bt_address);
          send_to_bt(keluar, 1000, DEBUG_IS);
          send_to_bt("at+disc", 1000, DEBUG_IS);
        }
        delay(1000);
      }
      else
      {
        Serial.print("Kosong");
      }

    }
    client.loop();

  }
}

void callback(char *topic, byte *message, unsigned int length) {
  if (topic == "esp32/slot")  {
    String messageTemp;
    for (int i = 0; i < length; i++)    {
      messageTemp += (char)message[i];
    }
    slot = messageTemp;
  }
  else if (topic == "esp32/bayar")  {
    String messageTemp;
    for (int i = 0; i < length; i++)    {
      messageTemp += (char)message[i];
    }
    bayar = messageTemp;
  }
  else if (topic == "esp32/tunggak")  {
    String messageTemp;
    for (int i = 0; i < length; i++)
    {
      messageTemp += (char)message[i];
    }
    tunggak = messageTemp;
  }

}

void setup()
{
  Serial.begin(9600);    // Initiate a serial communication
  Serial1.begin(115200); // Initiate a serial communication
  Serial2.begin(38400);  //Baudrateb module bluetooth
  SPI.begin();           // Initiate  SPI bus
  //  Serial.println(F("Read personal data on a MIFARE PICC:")); //shows in serial that it is ready to read
  // setup task
  xTaskCreate(DisplayLCD, "DisplayLCD", 5000, NULL, 0, &Handle_ViewLCD);
  xTaskCreate(RFID_Task, "RFID", 5000, NULL, 0, &Handle_RFID);

  delay(500);
  //setup lcd
  lcd.init();
  lcd.backlight();
  // Start connecting to wifi network and wait for connection to complete
  WiFi.begin(ssid, pass);
  Serial.print("Menghubungkan  ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print('.');
  }

  Serial.println('\n');
  Serial.println("Koneksi Terhubung");
  Serial.println(ssid);
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  reconnect();

  mfrc522.PCD_Init(); // Initiate MFRC522
  Serial.println(" \n Tempelkan RFID Anda....");
  Serial.println();
}

void loop()
{
}
