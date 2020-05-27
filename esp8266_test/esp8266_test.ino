#include <SoftwareSerial.h>

#define RX 10
#define TX 11
String AP = "monty";       // CHANGE ME
String PASS = "monty123"; // CHANGE ME
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
  
void setup() {
  
  // Set LED pin for output.
  pinMode(sharpLEDPin, OUTPUT);
  Serial.begin(9600);
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=3",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
  
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
