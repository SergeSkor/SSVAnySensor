//  SSVAnySensor library
//  BME280 sensor (Temp, Humidity, AirPressure over I2C)
//  SSD1306 display

#include "SSVAnySensor.h"

float DoMeasTemp(SSVAnySensor &Self);    //ahead declaration
float DoMeasHumid(SSVAnySensor &Self);    //ahead declaration
float DoMeasAirPressCB(SSVAnySensor &Self);    //ahead declaration
bool DoReportCB(SSVAnySensor &Self);  //ahead declaration
SSVAnySensor MySensorTemp     (1, DoMeasTemp,       DoReportCB);
SSVAnySensor MySensorHumid    (2, DoMeasHumid,      DoReportCB);
SSVAnySensor MySensorAirPress (3, DoMeasAirPressCB, DoReportCB);

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Adafruit_BME280.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET   16 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_BME280 bme; // I2C


// startup point entry (runs once).
void setup () 
{
  Serial.begin(115200);
  Serial.println();

  MySensorTemp.setReportCount(20);
  MySensorTemp.setAutoReportTrigger(ART_ByCount);  // {ART_ByInterval, ART_ByCount, ART_None 

MySensorTemp.setSensorParamName_And_Units    ("Temperature", "*C");
MySensorHumid.setSensorParamName_And_Units   ("Humidity",    "%");
MySensorAirPress.setSensorParamName_And_Units("Pressure",    "mmHg");

MySensorTemp.setSensorName_And_Desc    ("BME280-T", "Desc#1");
MySensorHumid.setSensorName_And_Desc   ("BME280-H", "Desc#2");
MySensorAirPress.setSensorName_And_Desc("BME280-P", "Desc#3");

 //sensor init
 if (!bme.begin(0x76)) { Serial.println(F("Could not find a valid BME280 sensor, check wiring!")); }
  
}

// loop the main sketch.
void loop () 
{
  MySensorTemp.process();
  MySensorHumid.process();
  MySensorAirPress.process();
}

float DoMeasTemp(SSVAnySensor &Self)
{
  float v = bme.readTemperature();
  return v;  //return NAN if measurement fails
}

float DoMeasHumid(SSVAnySensor &Self)
{
  float v = bme.readHumidity();
  return v;  //return NAN if measurement fails
}

float DoMeasAirPressCB(SSVAnySensor &Self)
{
  float v = bme.readPressure()* 0.00750062;
  return v;  //return NAN if measurement fails
}


bool DoReportCB(SSVAnySensor &Self)
{
  Serial.printf("Report: SID: %d,\tName:%s, \tDesc:%s, \tParamName:%s, \tUnits:%s, \tCNT:%ld,\tLast:%.2f,\tAVE:%.2f \r\n", Self.getSensorID(), Self.getSensorName(),   Self.getSensorDesc(), Self.getSensorParamName(), Self.getSensorUnits(), Self.DataCount(), Self.DataLast(), Self.DataAverage() );
  return true;  //return false if reporting fails
}
