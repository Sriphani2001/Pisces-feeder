\#include <SPI.h>\\
\#include <WiFiNINA.h>\\
\#include <ArduinoMqttClient.h>\\
\#include <ArduinoJson.h>\\
\#include <Arduino_LSM6DS3.h>\\
\#include <OneWire.h>\\
\#include <DallasTemperature.h>\\

\#include<Servo.h>\\
Servo Myservo;\\
int pos;\\

// Data wire is conntec to the Arduino digital pin 4\\
#define ONE_WIRE_BUS 4\\

// Setup a oneWire instance to communicate with any OneWire devices\\
OneWire oneWire(ONE_WIRE_BUS);\\

// Pass our oneWire reference to Dallas Temperature sensor \\
DallasTemperature sensors(&oneWire);\\
///////please enter your SSID and Password\\
char ssid[] = "realme3";        // your network SSID (name)\\
char pass[] = "0123456789";    // your network password (use for WPA, or use as key for WEP)\\
int status = WL_IDLE_STATUS;     // the WiFi radio's status\\

WiFiClient wifiClient;\\
MqttClient mqttClient(wifiClient);\\

// MQTT server setup\\
const char broker[] = "155.4.118.139"; \\
int        port     = 1883;\\
const char topic[]  = "P11";\\

//Test varaibles\\

String PoParam="";\\
String DID;\\
float Xaxis, Yaxis, Zaxis;\\
const long interval = 1000;\\
unsigned long Millis = 0;\\
String subMessage = "";\\
boolean stat=0;\\
String subString ="Led is OFF";\\

float x, y, z, temp;\\

void setup() {\\
  //Initialize serial and wait for port to open:\\
  Serial.begin(9600);\\
  while (!Serial) {\\
    ; // wait for serial port to connect. Needed for native USB port only\\
  }\\

  // check for the WiFi module:\\
  if (WiFi.status() == WL_NO_MODULE) {\\
    Serial.println("Communication with WiFi module failed!");\\
    // don't continue\\
    while (true);\\
  }\\
Myservo.attach(5);\\
  // attempt to connect to WiFi network:\\
  while (status != WL_CONNECTED) {\\
    Serial.print("Attempting to connect to WPA SSID: ");\\
    Serial.println(ssid);\\
    // Connect to WPA/WPA2 network:\\
    status = WiFi.begin(ssid, pass);\\
    // wait 10 seconds for connection:\\
    delay(100);\\
    
  }\\

    while (!mqttClient.connect(broker, port)) {\\
    Serial.print("MQTT connection failed! Error code = ");\\
    Serial.println(mqttClient.connectError());\\
  }\\

  // subscribe to a topic\\
  mqttClient.subscribe(topic);\\

  // you're connected now, so print out the data:\\
  Serial.println("You're connected to the network");\\
  Serial.print("SSID: ");\\
  Serial.println(WiFi.SSID());\\

   if (!IMU.begin()) {\\
    Serial.println("Failed to initialize IMU!");\\

    while (1);\\
  }\\

  Serial.print(IMU.accelerationSampleRate());\\

}\\

void loop() {\\

  
  int sensorValue = analogRead(A0);\\
  float voltage = sensorValue * (5.0 / 1024.0);\\
 
  Serial.println ("turbiduty (in V):");\\
  Serial.println (voltage);\\
  Serial.println();\\
  delay(1000);\\

  
  // Call sensors.requestTemperatures() to issue a global temperature and Requests\\ to all devices on the bus\\
  sensors.requestTemperatures(); \\
  
  Serial.print("Temperature(C): ");\\
  
  // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the\\ first IC on the wire\\
  Serial.print(sensors.getTempCByIndex(0)); \\
  Serial.print("  Temperature(F): ");\\
  Serial.println(sensors.getTempFByIndex(0));\\
  delay(1000);\\
  StaticJsonDocument<200> OutMes;\\
  StaticJsonDocument<200> inMes;\\

    IMU.readAcceleration(x, y, z);\\
    
  
//Data Serialization\\
    OutMes[String("ID")]="Device 1";\\
    OutMes["X"]=(x);\\
    OutMes["Y"]=(y);\\
    OutMes["Z"]=(z);\\
    OutMes["feedstatus"]=(subString);\\
    OutMes["temperature"]=(temp);\\
   serializeJson(OutMes, PoParam);\\
   //Serial.println(PoParam);\\

   //Read incoming message\\
  int messageSize = mqttClient.parseMessage();\\
  if (messageSize) {\\
    subMessage = "";\\
   
    // use the Stream interface to print the contents\\
    while (mqttClient.available()) {\\
      subMessage = subMessage + (char)mqttClient.read();\\
    }\\
   
    //Parse Messages from Sender\\
    deserializeJson(inMes, subMessage);\\
    DID=inMes["ID"].as<String>(); \\
    Xaxis=inMes["X"]; \\
    Yaxis=inMes["Y"];\\
    Zaxis=inMes["Z"];\\
    if(DID=="Device 2"){\\
       Serial.println(subMessage);\\
      }\\
    
    //Taking Action acording to subscribed message\\
    if(subMessage == "ONOFF") {\\
      if(stat==1){\\
        digitalWrite(LED_BUILTIN, LOW);\\
        stat=0;\\
        subString = "feed is loaded";\\
        
            for(pos=0;pos<=180;pos++){\\
            Myservo.write(pos);\\
            delay(1);\\
            }\\
            delay(1000);\\
            
        }\\
        else if(stat==0){\\
           digitalWrite(LED_BUILTIN, HIGH);\\
           stat=1;\\
           subString = "feed is given";\\
           
           for(pos=180;pos>=0;pos--){\\
           Myservo.write(pos);\\
           delay(5);\\
           }\\
           delay(1000);\\
          }\\
     
    } \\
  }\\

  //Publishing measured data through MQTT\\
   if(millis()-Millis>interval){\\
       mqttClient.beginMessage(topic);\\
        mqttClient.print(PoParam);\\
        mqttClient.endMessage();\\
        Millis=millis();\\
  }\\

  delay(10);\\
  PoParam="";\\
 
}\\