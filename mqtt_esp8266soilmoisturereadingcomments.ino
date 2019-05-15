
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
int sensor = A0; //define the input variable for the soil moisture sensor. The soil moisture sensor is connected to the pin A0 of the Nodemcu.

//Setting up the WiFi Access Point

#define WLAN_SSID       "   " //your wifi username
#define WLAN_PASS       "   " //your wifi password

//Setting up the Adafruit.io Setup

#define AIO_SERVER      "io.adafruit.com" //defining the server to connect
#define AIO_SERVERPORT  1883                   //port used by adafruit
#define AIO_USERNAME    "    " //user name in the account
#define AIO_KEY         "    " //AIO key under the user name in adafruit(predefined by adafruit)

//************ Global State******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

//Defining the relevant feeds

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Soil_moisture_sensor_data"); //feedname for soil moisture

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/On and Off the Water Pump"); //feed name for the water pump

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
//always make sure we're connected to the MQTT server, adafruit have a helper program called MQTT_connect()
void MQTT_connect();

void setup() {
  Serial.begin(115200); //Sets the data rate in bits per second (baud) for serial data transmission. Nodemcu can operate under 9600 and 115200
  delay(10); //delay 10 mili seconds

  pinMode(sensor,INPUT); //pin sensor(A0) operate as input
  pinMode(2,OUTPUT); //pin 2 or D4 of Nodemcu operate as output

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  //prints '.' every half second until the Nodemcu gets connected to the wifi router
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

//prints the wifi connected status and the IP address of the Nodemcu
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&onoffbutton);
}

uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  
 //Data is subscribed or input to the adafruit server every 5 seconds
 //if the subscription is for onoffbutton it activates the last read value

 
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
      uint16_t num = atoi((char *)onoffbutton.lastread); //The data also shows up as a string, so we use atoi to convert it from ascii to integer. Then we can save it to sliderval and use that to analog/PWM write to our PWM pin
      if (num == 1){
      digitalWrite(2,HIGH); //motor on
      }
      else{
        digitalWrite(2,LOW); //motor off
      }
      }
    }
  

  // Now we can publish stuff!
  Serial.print(F("\nSending photocell val "));
  Serial.print(x);
  Serial.print("...");
  int sensor_value = analogRead(sensor); //get the data from the soil moisture sensor(analog values)
  if (! photocell.publish(sensor_value)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  delay(5000);


}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
