#include <WiFi.h>
#include <MQUnifiedsensor.h>
#include <ThingsBoard.h>

const char* ssid = "CVG _2.4";           
const char* password = "800203572CVG";

#define TOKEN               "WsaX121LxjkHFpGwgWyT"
#define THINGSBOARD_SERVER  "thingsboard.cloud"

// Initialize ThingsBoard client
WiFiClient espClient;
// Initialize ThingsBoard instance
ThingsBoard tb(espClient);
// the Wifi radio's status
int status = WL_IDLE_STATUS;

//Definitions
#define placa "ESP-32"
#define Voltage_Resolution 3.3
#define pin 34 //Analog input 0 of your arduino
#define type "MQ-135" //MQ135
#define ADC_Bit_Resolution 12 // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  

double          CO2          =   (0);

MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

void setup()
{
  Serial.begin(9600);
  if(Serial) Serial.println("Serial is open");

  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  //Set math model to calculate the PPM concentration and the value of constants
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
 
  MQ135.init(); 
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
  MQ135.serialDebug(false);
}

void loop()
{
  if (!tb.connected()) {
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }

  MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
 
  MQ135.setA(110.47); MQ135.setB(-2.862);
  CO2 = MQ135.readSensor();

  MQ135.setA(605.18); MQ135.setB(-3.937);
  float CO = MQ135.readSensor();

  MQ135.setA(77.255); MQ135.setB(-3.18);
  float Alcohol = MQ135.readSensor();

  MQ135.setA(44.947); MQ135.setB(-3.445);
  float Toluene = MQ135.readSensor();

  MQ135.setA(102.2 ); MQ135.setB(-2.473);
  float NH4 = MQ135.readSensor();

  MQ135.setA(34.668); MQ135.setB(-3.369);
  float Acetone = MQ135.readSensor();
  
  Serial.print("CO2: ");
  Serial.println(CO2);
  Serial.println("Sending data...");

  tb.sendTelemetryFloat("CO2", CO2);  
  
  tb.sendTelemetryFloat("CO", CO);  
  
  tb.sendTelemetryFloat("Alcohol", Alcohol);  
  
  tb.sendTelemetryFloat("Toluene", Toluene);  
  
  tb.sendTelemetryFloat("NH4", NH4);  
  
  tb.sendTelemetryFloat("Acetone", Acetone);  

  tb.loop();
  
  delay(1000); //Sampling frequency
}
