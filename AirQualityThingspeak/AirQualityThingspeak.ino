
#include "ThingSpeak.h"
#include "WiFiEsp.h"
#include "secrets.h"
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789


char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
WiFiEspClient  client;

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(5, 6); // RX, TX
#define ESP_BAUDRATE  19200
#else
#define ESP_BAUDRATE  115200
#endif

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Initialize our values
int valSensor = 0;
int valSensor2 = 0;
int valSensor3 = 0;

//Dust Sensor program options
const int ledPower = 7;   // Arduino digital pin 7 connect to sensor LED.
const int measurePin = A5;   // Arduino analog pin 5 connect to sensor Vo.

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;


 /////////////////////////////////////////////////////////////////////////////

 // ST7789 TFT module connections
#define TFT_CS    10  // define chip select pin
#define TFT_DC     9  // define data/command pin
#define TFT_RST    8  // define reset pin, or set to -1 and connect to Arduino RESET pin

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


void setup() {
  //Initialize serial and wait for port to open
  Serial.begin(115200); 
  // initialize serial for ESP module  
  //setEspBaudRate(ESP_BAUDRATE); 
  Serial1.begin(ESP_BAUDRATE);
  // initialize ESP module
  WiFi.init(&Serial1);
    // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
  Serial.println("found it!");
   
  ThingSpeak.begin(client);  // Initialize ThingSpeak
   // if the display has CS pin try with SPI_MODE0
  tft.init(240, 240, SPI_MODE3);    // Init ST7789 display 240x240 pixel
  // if the screen is flipped, remove this command
  tft.setRotation(2);
  
   uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;
 
  Serial.println(time, DEC);
  delay(500);
 
 // tft print function!
 tftSplashScreen();
  delay(3000);
  
  pinMode(ledPower, OUTPUT);

  
  
  
  
}

void loop() {

  int statusCode = 0;

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

 valSensor = getSensorData();
 valSensor2 = getSensorData2();
 valSensor3 = getSensorData3();
 
  // set the fields with the values
  ThingSpeak.setField(1, valSensor);
  ThingSpeak.setField(2, valSensor2);
  ThingSpeak.setField(3, valSensor3);
  
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
//   if(x == 200){
//    Serial.println("Channel update successful.");
//  }
//  else{
//    Serial.println("Problem updating channel. HTTP error code " + String(x));
//  }
   // Read in field 4 of the public channel recording
  int AQI = ThingSpeak.readFloatField(myChannelNumber, 4);

  // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("AQI : " + String(AQI));
    tftPrintData(valSensor, valSensor2, valSensor3, AQI);

  }
  else{
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }
  delay(15000); // No need to read the temperature too often.  
}

//void setEspBaudRate(unsigned long baudrate){
//  long rates[6] = {115200,74880,57600,38400,19200,9600};
//
//  Serial.print("Setting ESP8266 baudrate to ");
//  Serial.print(baudrate);
//  Serial.println("...");
//
//  for(int i = 0; i < 6; i++){
//    Serial1.begin(rates[i]);
//    delay(100);
//    Serial1.print("AT+UART_DEF=");
//    Serial1.print(baudrate);
//    Serial1.print(",8,1,0,0\r\n");
//    delay(100);  
//  }
//    
//  Serial1.begin(baudrate);
//}

//MQ135 Sensor data
int getSensorData(){
  return analogRead(A0); // Replace with 
}

//MQ7 sensor data
int getSensorData2() {
  return analogRead(A1);
}

//Dust Sensor data
int getSensorData3(){
  digitalWrite(ledPower,LOW); // power on the LED
  delayMicroseconds(samplingTime);

  voMeasured = analogRead(measurePin); // read the dust value

  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH); // turn the LED off
  delayMicroseconds(sleepTime);

  // 0 - 5V mapped to 0 - 1023 integer values
  // recover voltage
  calcVoltage = voMeasured * (5.0 / 1024.0);

  // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
  // Chris Nafis (c) 2012
  dustDensity = 170 * calcVoltage - 0.1;
  return dustDensity;
  
}

void tftSplashScreen() {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(28, 100);
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(3.5);
  tft.println("Air Quality");
}

void tftPrintData(int data, int data1, int data2, int AQI) {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(25, 40);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.println("NOx: ");
  tft.setCursor(130, 40);
  tft.print(data);
  //////////////////
  tft.setCursor(30, 85);
  tft.println("CO: ");
  tft.setCursor(130, 85);
  tft.print(data1);
///////////////////////////
  tft.setCursor(30, 125);  
  tft.println("PM2.5: ");
  tft.setCursor(140, 125);
  tft.print(data2);
/////////////////////////////
  tft.setCursor(20, 160);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("AIR QUALITY INDEX");
  tft.setCursor(90, 190);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(4);
  tft.print(AQI);

}
