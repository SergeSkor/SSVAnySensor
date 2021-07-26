#include "SSVAnySensor.h"

float DoMeasCB(SSVAnySensor &Self);    //measure callback function ahead declaration
bool DoReportCB(SSVAnySensor &Self);  //report callback function ahead declaration
SSVAnySensor MySensor (99, DoMeasCB, DoReportCB);  //SensorID, measureCB, ReportCB

void setup () 
{
  Serial.begin(115200);
  Serial.println();

  MySensor.setReportCount(20);  //report every XX valid measurements, if AutoReportTrigger == ART_ByCount
  MySensor.setReportInterval(10000);  //report every XX mS, if AutoReportTrigger == ART_ByInterval
  
  MySensor.setMeasurementInterval(2000); //perform measurement ever XX mS, if AutoMeasTrigger == AMT_ByInterval
  
  MySensor.setAutoReportTrigger(ART_ByInterval);  // Possible values: ART_ByInterval, ART_ByCount, ART_None 
  MySensor.setAutoMeasTrigger(AMT_ByInterval);   // Possible values: AMT_ByInterval, AMT_None 
}

void loop () 
{
  MySensor.process();
}

float DoMeasCB(SSVAnySensor &Self)
{
  float v;
  if (random(100) > 80) v=NAN; else v=random(100);
  
  Serial.printf("DoMeasurement Function, ts: %ld, SID: %d, Value: %.2f \r\n", millis(), Self.getSensorID(), v );
  Serial.printf("CNT: %ld, Last: %.2f, AVE: %.2f \r\n", Self.DataCount(), Self.DataLast(), Self.DataAverage()  );  
  return v;  //return NAN if measurement fails
}

bool DoReportCB(SSVAnySensor &Self)
{
  Serial.printf("    DoReport Function, ts: %ld, SID: %d\r\n", millis(), Self.getSensorID() );
  Serial.printf("    CNT: %ld, Last: %.2f, AVE: %.2f \r\n", Self.DataCount(), Self.DataLast(), Self.DataAverage()  );
  return true;  //return false if reporting fails
}
