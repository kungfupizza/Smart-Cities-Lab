/*
	Author: Sarthak
*/
#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include <DHT.h>
#include <stdlib.h>
#include <Wire.h>
#include "rgb_lcd.h"
#include "config.h"


rgb_lcd lcd;
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);;
DHT dht(DHTPIN, DHTTYPE);

String valveTopic; 
String displayTopic; 
String intervalTopic;


void setup()
{
  Serial.begin(115200);
  lcd.begin(16, 2);
  lcdSetup();
  WiFi.disconnect();
  delay(1000);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  pinMode(VALVE, OUTPUT);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if(WiFi.status() == WL_CONNECTED)
  {
    colorR = colorG = colorB=0;
    colorG=255;
    lcd.setRGB(colorR, colorG, colorB);
    lcd.setCursor(0,1);
    lcd.print(" ...Connected...");    
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
  IPAddress ip = WiFi.localIP();
  
  mqttClient.setUsernamePassword(username, password);

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(mqttServer);
  mqttClient.connect(mqttServer, mqttPort);
  mqttClient.onMessage(onMqttMessage);

  valveTopic = actuatorTopic + String(DEV_ID) + "/" + "valve";
  displayTopic = actuatorTopic + String(DEV_ID) + "/" + "display";
  intervalTopic = actuatorTopic + String(DEV_ID) + "/" + "interval";
  
  mqttClient.subscribe(valveTopic);
  mqttClient.subscribe(displayTopic);
  mqttClient.subscribe(intervalTopic);
  dht.begin();
  
  delay(1500);
}

void loop()
{
  //Get and send Temp and Humi
  sendDHT();
  
  //Get moisture and send
  sendMoisture();
  
  mqttClient.poll();		//Poll for MQTT msg buffer
  delay(interval);			//Can be updated by User
}

void sendDHT()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  String tempTopic = sensorTopic + String(DEV_ID) + "/" + "temp";
  String humiTopic = sensorTopic + String(DEV_ID) + "/" + "humid";
  
  char buff[6];
  dtostrf(t,0,2,buff);
  mqttClient.beginMessage(tempTopic.c_str());
  mqttClient.print(buff);
  mqttClient.endMessage();
  Serial.println(tempTopic);
  Serial.print("TEMPERATURE ");
  Serial.println(t);
  
  dtostrf(h,0,2,buff);
  mqttClient.beginMessage(humiTopic.c_str());
  mqttClient.print(buff);
  mqttClient.endMessage();
  Serial.println(humiTopic);
  Serial.print("HUMIDITY ");
  Serial.println(h);
}

void sendMoisture()
{
  float moisture = 0;
  moisture = analogRead(MOISTURE_PIN);
  moisture = map(moisture, 300, 600, 0, 1024);
  moisture = (float)(moisture/1024.00) * 100;
  moisture = constrain(moisture, 0, 100);
  
  moisture = abs(100.0 - moisture);
  String moistTopic = sensorTopic + String(DEV_ID) + "/" + "moist";
  char buff[6];
  dtostrf(moisture,0,6,buff);
  mqttClient.beginMessage(moistTopic.c_str());
  mqttClient.print(buff);
  mqttClient.endMessage();
  Serial.println(moistTopic);
  Serial.print("MOISTURE ");
  Serial.println(moisture);

}

void onMqttMessage(int messageSize) {
  // we received a message, print out the topic and contents
  String myMessage = "";
  Serial.print("Received a message with topic '");
  String myTopic = String(mqttClient.messageTopic());
  Serial.println(myTopic);
  for(int i=0; i< messageSize ; i++)
  {
    myMessage += (char)mqttClient.read();
  }
  Serial.print("Message = ");
  Serial.println(myMessage);
  if(myTopic.equals(valveTopic))
  {
    Serial.println("Changing valve value");
    if(myMessage.equals("1"))
    {
      digitalWrite(VALVE, HIGH);
    }
    else if(myMessage.equals("0"))
    {
      digitalWrite(VALVE, LOW);
    }
  }
  else if(myTopic.equals(displayTopic))
  {
    lcd.clear();
    String myCommand = myMessage.substring(0,6);
    long number = strtol( myCommand.c_str(), NULL, 16);

    colorR = (int)(number >> 16);
    colorG = (int)(number >> 8 & 0xFF);
    colorB = (int)(number & 0xFF);
    
    lcd.setRGB(colorR, colorG, colorB);
    String toPrint = myMessage.substring(6);
    if(toPrint.length()>16 || toPrint.length()<32)
    {
      lcd.print(toPrint.substring(0,16));
      lcd.setCursor(0,1);
      lcd.print(toPrint.substring(16));
    }
    else if (toPrint.length() <16)
    {
      lcd.print(toPrint);
    }
    else if(toPrint.length() >32)
    {
      lcd.print(toPrint.substring(0,16));
      lcd.setCursor(0,1);
      lcd.print(toPrint.substring(16,32));      
    }
  }
  else if (myTopic.equals(intervalTopic))
  {
    Serial.println("Changing interval value");
    interval = atoi(myMessage.c_str());
    interval = constrain(interval, 1000, 60000);
  }
  else
  {
    //ignore
  }

}

void lcdSetup()
{
  colorR = colorG = colorB = 0;
  for(int i =0;i<20;i++)
  {
    colorR = random(0,255);
    colorG = random(0,255);
    colorB = random(0,255);
    lcd.setRGB(colorR, colorG, colorB);
    delay(100);
  }
  colorR = colorG = colorB = 0;
  colorB=255;
  lcd.setRGB(colorR, colorG, colorB);
  lcd.print("I am Smart Plant");
  lcd.setCursor(0,1);
  lcd.print(" ...Connecting...");
  
  
}
