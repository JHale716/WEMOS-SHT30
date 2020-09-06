// This is code for WEMOS D1 with SHT30 port 0x45 using blynk cloud solution for data representation
// I have commented in the code and also setup verbose commentary for the monitor screen for newbie's.
// This is to limit the what's happening anxiety and help you get started with your own projects
// WiFi a a simple setup and just runs. 
// Version 1.0
// by JP Hale jphale716@gmail.com

#include <ESP8266WiFi.h>                                      // load the library for WiFi
#include <Wire.h>                                             // load the library for I2C
#include <BlynkSimpleEsp8266.h>                               // load the library for blynk

#define BLYNK_PRINT Serial                                    // you can comment out, its for terminal window only
#define Addr 0x45                                             // WEMOS SHT30 I2C address is 0x45

char auth[] = "<Blynk Code>";                                 // FILL in your BLYNK code 
char wifissid[] = "<SSID>";                                   // Your WiFi SSID
char wifipswd[] = "<WiFiPASSWORD>";                           // Your Wifi Password
unsigned long timeElapsedLED = 0;                             // Create an Instance for the timer function for the LED
unsigned long timeElapsedBlynk = 10000;                       // Create an Instance for the timer function for the Blynk Keep Alive
unsigned long timeElapsedProg = 300000;                       // Create an Instance for the timer function for the Program
int ledState = LOW;                                           // ledState used to set the LED

const long intervalLED = 5000;                                // interval between blink (milliseconds)
const long intervalProg = 300000;                             // interval at which to check sensor (milliseconds)
const long offsetTemp = 1.75;                                 // temp in deg C that the device creates (quick and dirty)
const long offsetHumid = 6.1147625;                          // Humidity in % that the device reduces (quick and dirty)
const long BlynkPortT = 0;                                    // Blynk Virtual Pin for the Temp data
const long BlynkPortH = 1;                                    // Blynk Virtual Pin for the Humidity data

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);                               // Setup still alive indicator (onboard LED)
  Serial.begin(115200);                                       // Setup serial monitor
  Serial.println();
  Serial.println("Initialisation");
  Serial.println("WEMOS D1 mini with SHT30 Configured to check temperature and humidity");
  Serial.print("Still live LED flash every ");
  Serial.print(intervalLED/1000);
  Serial.println(" seconds");
  Serial.print("Sensor check every ");
  Serial.print(intervalProg/1000/60);
  Serial.println(" minutes");
  Serial.println("> * < Printed for working or alive pulse");
  Serial.println("> ^ < Printed for Blynk keep alive pulse");
  WiFi.begin(wifissid, wifipswd);                             // Setup WiFi
  Serial.print("Connecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED)                       // Wait until connected
  {
    delay(500);                                               // 5 Second delay
    Serial.print("*");                                        // Still alive pulse
  }
  Serial.println();
  Serial.print("Connected, IP address: ");                    // Report connection and IP Address
  Serial.println(WiFi.localIP());  
  Wire.begin();                                               // Initialise I2C communication as MASTER
  Serial.println("Started");                                  // send to serial control messsage
  delay(300);
  Blynk.config(auth);                                         // set blynk authentification string
  Serial.println("Connecting to blynk");                      // send to serial control messsage
  Blynk.connect();                                            // start connection to Blynk server, this command has 30 seconds timeout
  Serial.println("End of setup");                             // send to serial control messsage
}

void sendTemperature()                                        // temperature measuring function
{
  unsigned int data[6];
  Wire.beginTransmission(Addr);                               // Start I2C Transmission
  Wire.write(0x2C);                                           // Send measurement command
  Wire.write(0x06);
  Wire.endTransmission();                                     // Stop I2C transmission
  delay(500);
  Wire.requestFrom(Addr, 6);                                  // Request 6 bytes of data                                                
  if (Wire.available() == 6)                                  // Read 6 bytes of data
  {
    data[0] = Wire.read();                                    // cTemp msb, cTemp lsb, cTemp crc, humidity msb, humidity lsb, humidity crc
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
    data[4] = Wire.read();
    data[5] = Wire.read();
  }
  float cTemp = ((((data[0] * 256.0) + data[1]) * 175) / 65535.0) - 45 - offsetTemp; // Convert the data
  float fTemp = (cTemp * 1.8) + 32;
  float humidity = (((((data[3] * 256.0) + data[4]) * 100) / 65535.0) + offsetHumid);
  Serial.println();
  Serial.println("Raw Data");
  Serial.println("Temperature");
  Serial.print(data[0]);
  Serial.print(" ");
  Serial.println(data[1]);
  Serial.println("Humidity");
  Serial.print(data[3]);
  Serial.print(" ");
  Serial.println(data[4]);
  Serial.println();
  Serial.println("Display Data");
  Serial.print("Relative Humidity : ");                       // Output data to serial monitor
  Serial.print(humidity);
  Serial.println(" %RH");
  Serial.print("Temperature in Celsius : ");
  Serial.print(cTemp);
  Serial.println(" C");
  Serial.print("Temperature in Fahrenheit : ");
  Serial.print(fTemp);
  Serial.println(" F");
  Blynk.virtualWrite(BlynkPortT, cTemp);                      // send to Blynk virtual pin temperature value
  digitalWrite(LED_BUILTIN, HIGH);                            // turn the LED on (HIGH is the voltage level)
  delay(50);                                                  // wait for a 1/2 second
  digitalWrite(LED_BUILTIN, LOW);                             // turn the LED off by making the voltage LOW
  delay(50); 
  Serial.println("Send Blynk Temp");
  Blynk.virtualWrite(BlynkPortH, humidity);                   // send to Blynk virtual pin humidity value
  digitalWrite(LED_BUILTIN, HIGH);                            // turn the LED on (HIGH is the voltage level)
  delay(50);                                                  // wait for a 1/2 second
  digitalWrite(LED_BUILTIN, LOW);                             // turn the LED off by making the voltage LOW
  delay(50); 
  Serial.println("Send Blynk Humidity");
  Serial.println("Update Blynk Success");                     // send to serial control messsage
}

void loop()
{
  unsigned long currentMillis = millis();
  if (currentMillis - timeElapsedBlynk >= 10000) {            // save the last time you Blynk'd
    timeElapsedBlynk = currentMillis;
    Serial.print("^");                                        // Blynk
    Blynk.run();                                              // run blynk
  }
  if (currentMillis - timeElapsedLED >= intervalLED) {        // save the last time you blinked the LED
    timeElapsedLED = currentMillis;
    Serial.print("*");  
    digitalWrite(LED_BUILTIN, LOW);                           // turn the LED on (HIGH is the voltage level)
    delay(100);                                               // wait for a second
    digitalWrite(LED_BUILTIN, HIGH);                          // turn the LED off by making the voltage LOW
    delay(100); 
    }
  if (currentMillis - timeElapsedProg >= intervalProg) {      // save the last time you checked and sent the temperature
    timeElapsedProg = currentMillis;
    sendTemperature();                                         // calls measure temperature function
  }

}
