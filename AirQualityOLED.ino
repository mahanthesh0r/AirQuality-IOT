
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>             // Arduino SPI library#include <Adafruit_GFX.h>    // Core graphics library
#include <SoftwareSerial.h>


//ESP8266
#define RX 5
#define TX 6
String AP = "Ramachandra";       // CHANGE ME
String PASS = "7892611653"; // CHANGE ME
String API = "D6F5HJGBM5BG3A6S";   // CHANGE ME
String HOST = "184.106.153.149";
String PORT = "80";
String field = "field1";
String field2 = "field2";
String field3 = "field3";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
float valSensor = 0.0f;
float valSensor2 = 0.0f;
float valSensor3 = 0.0f;
SoftwareSerial esp8266(RX,TX);


//Dust Sensor program options
const int sharpLEDPin = 7;   // Arduino digital pin 7 connect to sensor LED.
const int sharpVoPin = A5;   // Arduino analog pin 5 connect to sensor Vo.

// For averaging last N raw voltage readings.
#ifdef USE_AVG
#define N 100
static unsigned long VoRawTotal = 0;
static int VoRawCount = 0;
#endif // USE_AVG

// Set the typical output voltage in Volts when there is zero dust. 
static float Voc = 0.6;

// Use the typical sensitivity in units of V per 100ug/m3.
const float K = 0.5;

 /////////////////////////////////////////////////////////////////////////////


// ST7789 TFT module connections
#define TFT_CS    10  // define chip select pin
#define TFT_DC     9  // define data/command pin
#define TFT_RST    8  // define reset pin, or set to -1 and connect to Arduino RESET pin

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


void setup(void) {
 
  
  pinMode(sharpLEDPin, OUTPUT);
  Serial.begin(9600);
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=3",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
  // if the display has CS pin try with SPI_MODE0
  tft.init(240, 240, SPI_MODE3);    // Init ST7789 display 240x240 pixel
 
  // if the screen is flipped, remove this command
  tft.setRotation(2);
 
  Serial.println(F("Initialized"));
 
  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;
 
  Serial.println(time, DEC);
  delay(500);
 

//  // tft print function!
 tftSplashScreen();
  delay(4000);
 
 
}
 
void loop() {  
 valSensor = getSensorData();
 valSensor2 = getSensorData2();
 valSensor3 = getSensorData3();
 String getData = "GET /update?api_key="+ API +"&"+ field +"="+String(valSensor)+"&"+ field2 +"="+String(valSensor2)+"&"+ field3 +"="+String(valSensor3);
 sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
 esp8266.println(getData);delay(1500);countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");
 tftPrintData(valSensor, valSensor2, valSensor3);
}

//MQ135 Sensor data
float getSensorData(){
  return analogRead(A0); // Replace with 
}

//MQ7 sensor data
float getSensorData2() {
  return analogRead(A1);
}

//Dust Sensor data
float getSensorData3(){
   // Turn on the dust sensor LED by setting digital pin LOW.
  digitalWrite(sharpLEDPin, LOW);

  // Wait 0.28ms before taking a reading of the output voltage as per spec.
  delayMicroseconds(280);

  // Record the output voltage. This operation takes around 100 microseconds.
  int VoRaw = analogRead(sharpVoPin);
  
  // Turn the dust sensor LED off by setting digital pin HIGH.
  digitalWrite(sharpLEDPin, HIGH);

  // Wait for remainder of the 10ms cycle = 10000 - 280 - 100 microseconds.
  delayMicroseconds(9620);

  // Use averaging if needed.
  float Vo = VoRaw;
  #ifdef USE_AVG
  VoRawTotal += VoRaw;
  VoRawCount++;
  if ( VoRawCount >= N ) {
    Vo = 1.0 * VoRawTotal / N;
    VoRawCount = 0;
    VoRawTotal = 0;
  } else {
    return;
  }
  #endif // USE_AVG

  // Compute the output voltage in Volts.
  Vo = Vo / 1024.0 * 5.0;

  // Convert to Dust Density in units of ug/m3.
  float dV = Vo - Voc;
  if ( dV < 0 ) {
    dV = 0;
    Voc = Vo;
  }
  float dustDensity = dV / K * 100.0;
  return dustDensity;
  
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }
 
 
void tftSplashScreen() {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(28, 100);
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(3.5);
  tft.println("Air Quality");
}

void tftPrintData(float data, float data1, float data2) {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(30, 40);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.println("MQ135: ");
  tft.setCursor(130, 40);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.print(data);
  
  tft.setCursor(30, 85);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.println("MQ7: ");
  tft.setCursor(130, 85);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.print(data1);

  tft.setCursor(30, 125);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.println("Dust: ");
  tft.setCursor(130, 125);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.print(data2);

  tft.setCursor(20, 170);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("AIR QUALITY INDEX");
  tft.setCursor(90, 180);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(4);
  //tft.print("4.9");

}
