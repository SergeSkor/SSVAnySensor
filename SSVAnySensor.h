/*
SSVAnySensor.h
Author Serge Skorodinsky 
*/

#ifndef _SSVANYSENSOR_H
#define _SSVANYSENSOR_H

// the standard deviation increases the lib (<100 bytes)
// it can be in/excluded by un/commenting next line
#define ANYSENSOR_USE_STDEV


class SSVAnySensor; //declared ahead
typedef bool  (*CB_AnySensorReportFunction) (SSVAnySensor &self);  //passing Self by reference. Also possible to pass as pointer. Which way is better?
typedef float (*CB_AnySensorMeasFunction) (SSVAnySensor &self);  //passing Self by reference. Also possible to pass as pointer. Which way is better?

enum TAutoMeasTrigger {AMT_ByInterval, AMT_None};
enum TAutoReportTrigger {ART_ByInterval, ART_ByCount, ART_None /*, ART_ByIntervalAndCount, ART_ByIntervalOrCount*/}; 

// the definition of the SSVAnySensor class.
class SSVAnySensor
{
  public:
    SSVAnySensor(word SensorID, CB_AnySensorMeasFunction DoMeasurementFunc, CB_AnySensorReportFunction DoReportFunc);  //default constructor
    ~SSVAnySensor(); //destructor.
    void process(); //run in the loop()
    word getSensorID();
    //sensor Data
  void  DataClear();
  void  DataPush(float);
  long  DataCount();
  float DataSum();
  float DataAverage();
  float DataMinimum();
  float DataMaximum();
  float DataLast(); 
  #ifdef ANYSENSOR_USE_STDEV
  float DataPopStDev();      // population stdev
  float DataUnbiasedStDev();
  #endif
  
  TAutoMeasTrigger getAutoMeasTrigger();
  void setAutoMeasTrigger(TAutoMeasTrigger value);
  
  TAutoReportTrigger getAutoReportTrigger();
  void setAutoReportTrigger(TAutoReportTrigger value);

  unsigned long getMeasurementInterval();
  void setMeasurementInterval(unsigned long value);

  unsigned long getReportInterval();
  void setReportInterval(unsigned long value);

  unsigned long getReportCount();
  void setReportCount(unsigned long value);

  const char* getSensorName();
  void setSensorName(const char* value);

  const char* getSensorUnits();
  void setSensorUnits(const char* value);

  const char* getSensorParamName();
  void setSensorParamName(const char* value);
  
  const char* getSensorDesc();
  void setSensorDesc(const char* value);

  void setSensorParamName_And_Units(const char* aParamName, const char* aUnits);
  void setSensorName_And_Desc(const char* aName, const char* aDesc);
  
//fReportCount

  private:
    word fSensorID;
    //measurement
    unsigned long fMeasurementInterval;
    unsigned long fLastMeasurementTS;
    CB_AnySensorMeasFunction OnMeasurementFunc = NULL;
    //reporting
    unsigned long fReportInterval;
    unsigned long fLastReportTS;
    unsigned long fReportCount;
    
    CB_AnySensorReportFunction OnReportFunc = NULL;
    //stat data
    unsigned long  _cnt;
    float _sum;
    float _min;
    float _max;
    float _last; 
    #ifdef ANYSENSOR_USE_STDEV
    float _store;           // store to minimise computation
    float _ssqdif;        // sum of squares difference
    #endif

    TAutoMeasTrigger fAutoMeasTrigger;
    TAutoReportTrigger fAutoReportTrigger;  

    const char* fSensorName;
    const char* fSensorUnits;
    const char* fSensorParamName;
    const char* fSensorDesc;
};

//default constructor
SSVAnySensor::SSVAnySensor(word SensorID, CB_AnySensorMeasFunction DoMeasurementFunc, CB_AnySensorReportFunction DoReportFunc)
{
  unsigned long ts=millis();
  fSensorID = SensorID;
  fMeasurementInterval = 1000;
  fLastMeasurementTS=ts;
  OnMeasurementFunc = DoMeasurementFunc;

  fReportInterval = 5000;
  fLastReportTS=ts;
  OnReportFunc = DoReportFunc;
  fReportCount = 0;

  DataClear();
  fAutoMeasTrigger = AMT_ByInterval;    //{AMT_ByInterval, AMT_None};
  fAutoReportTrigger = ART_ByInterval;  // {ART_ByInterval, ART_ByCount, ART_None 

  fSensorName = "";
  fSensorUnits = "";
  fSensorParamName = "";
  fSensorDesc = "";
}

// destructor.
SSVAnySensor::~SSVAnySensor()
{
 // free(Stat); 
}

void SSVAnySensor::process() //run in the loop()
{
unsigned long NowIs = millis();

//check if Measurement needed
if (OnMeasurementFunc != NULL) 
  {
  if ((fAutoMeasTrigger==AMT_ByInterval) && (NowIs - fLastMeasurementTS >= fMeasurementInterval))
    {
    float v=OnMeasurementFunc(*this); //run Measurement callback 
    if (isnan(v)) 
      { //measurement fails
      }
      else 
      { //measurement success
      DataPush(v); //skip if nan (measurement fails)
      fLastMeasurementTS = NowIs;
      }
    }
  }

//check if Reporting needed
if (OnReportFunc != NULL) 
  {
  bool cond1 = ( (fAutoReportTrigger==ART_ByInterval) && (NowIs - fLastReportTS >= fReportInterval) );
  bool cond2 = ( (fAutoReportTrigger==ART_ByCount) && (_cnt >= fReportCount) );
  if (cond1 || cond2)
    {
    bool res=OnReportFunc(*this); //run Measurement callback
    if (res) 
      { //report success
      DataClear(); //clear stat
      fLastReportTS = NowIs;
      }
      else
        {  //report fails
        }
    }
  }
  
}

word SSVAnySensor::getSensorID()
{
return fSensorID;
}

// resets all counters
void SSVAnySensor::DataClear()
{
  _cnt = 0; 
  _sum = 0.0;
  _min = 0.0;
  _max = 0.0;
  _last = 0.0; //added by S.S.
  #ifdef ANYSENSOR_USE_STDEV
  _ssqdif = 0.0;  // not _ssq but sum of square differences
                  // which is SUM(from i = 1 to N) of 
                        // (f(i)-_ave_N)**2
  #endif
}

// adds a new value to the data-set
void SSVAnySensor::DataPush(float f)
{
_last = f; //added by S.S.
  if (_cnt < 1)
  {
    _min = f;
    _max = f;
  } else {
    if (f < _min) _min = f;
    if (f > _max) _max = f;           
        } // end of if (_cnt == 0) else
        _sum += f;
  _cnt++;
#ifdef ANYSENSOR_USE_STDEV 
        if (_cnt >1) {
           _store = (_sum / _cnt - f);
           _ssqdif = _ssqdif + _cnt * _store * _store / (_cnt-1);
        } // end if > 1

#endif
}

// returns the number of values added
long SSVAnySensor::DataCount()
{
  return _cnt;
}

// returns the sum of the data-set (0 if no values added)
float SSVAnySensor::DataSum()
{
  return _sum;
}

// returns the average of the data-set added sofar
float SSVAnySensor::DataAverage()
{
  if (_cnt < 1) return NAN; // original code returned 0
  return _sum / _cnt;
}

// returns the sum of the data-set (0 if no values added)
float SSVAnySensor::DataMinimum()
{
  return _min;
}

// returns the sum of the data-set (0 if no values added)
float SSVAnySensor::DataMaximum()
{
  return _max;
}

// returns the last value, added by S.S
float SSVAnySensor::DataLast()
{
  return _last;
}

// Population standard deviation = s = sqrt [ S ( Xi - µ )2 / N ]
// http://www.suite101.com/content/how-is-standard-deviation-used-a99084
#ifdef ANYSENSOR_USE_STDEV  
float SSVAnySensor::DataPopStDev()
{
  if (_cnt < 1) return NAN; // otherwise DIV0 error
  return sqrt( _ssqdif / _cnt);
}

float SSVAnySensor::DataUnbiasedStDev()
{
  if (_cnt < 2) return NAN; // otherwise DIV0 error
  return sqrt( _ssqdif / (_cnt - 1));
}
#endif


TAutoMeasTrigger SSVAnySensor::getAutoMeasTrigger()
{
  return fAutoMeasTrigger;
}

void SSVAnySensor::setAutoMeasTrigger(TAutoMeasTrigger value)
{
  fAutoMeasTrigger = value;
}

TAutoReportTrigger SSVAnySensor::getAutoReportTrigger()
{
  return fAutoReportTrigger;
}

void SSVAnySensor::setAutoReportTrigger(TAutoReportTrigger value)
{
  fAutoReportTrigger = value;
}


unsigned long SSVAnySensor::getMeasurementInterval()
{
  return fMeasurementInterval;
}

void SSVAnySensor::setMeasurementInterval(unsigned long value)
{
  fMeasurementInterval = value;
}

unsigned long SSVAnySensor::getReportInterval()
{
  return fReportInterval;
}

void SSVAnySensor::setReportInterval(unsigned long value)
{
  fReportInterval = value;
}

unsigned long SSVAnySensor::getReportCount()
{
  return fReportCount;
}

void SSVAnySensor::setReportCount(unsigned long value)
{
  fReportCount = value;
}

const char* SSVAnySensor::getSensorName()
{
  return fSensorName;
}

void SSVAnySensor::setSensorName(const char* value)
{
  fSensorName = value;
}

const char* SSVAnySensor::getSensorUnits()
{
  return fSensorUnits;
}

void SSVAnySensor::setSensorUnits(const char* value)
{
  fSensorUnits = value;
}

const char* SSVAnySensor::getSensorParamName()
{
  return fSensorParamName;
}

void SSVAnySensor::setSensorParamName(const char* value)
{
  fSensorParamName = value;
}

const char* SSVAnySensor::getSensorDesc()
{
  return fSensorDesc;
}

void SSVAnySensor::setSensorDesc(const char* value)
{
  fSensorDesc = value;
}

void SSVAnySensor::setSensorParamName_And_Units(const char* aParamName, const char* aUnits)
{
  fSensorParamName = aParamName;
  fSensorUnits = aUnits;
}

void SSVAnySensor::setSensorName_And_Desc(const char* aName, const char* aDesc)
{
  fSensorName = aName;
  fSensorDesc = aDesc;
}


#endif // _SSVANYSENSOR_H
