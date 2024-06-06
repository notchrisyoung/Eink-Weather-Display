#ifndef BatteryVoltage_h
#define BatteryVoltage_h

#include "Arduino.h"
#include "esp_adc_cal.h"

#define DEF_PIN 36
#define DIV_FACTOR 2
#define VOLTAGE_REF
#define DEF_CONV_FACTOR 1.591
#define DEF_READS 20

class BatteryVoltage {
    public:
		BatteryVoltage();
	    BatteryVoltage(int addressPin);
	    BatteryVoltage(int addressPin, double convFactor);
	    BatteryVoltage(int addressPin, double convFactor, int reads);
	    int getBatteryChargeLevel();
	    double getBatteryVolts();
	    int getAnalogPin();
	    int pinRead();
	    double getConvFactor();
  	private:
	    int    _addressPin;               //!< ADC pin used, default is GPIO34 - ADC1_6
	    int    _reads;                    //Number of reads of ADC pin to calculate an average value
	    double _convFactor;               //!< Convertion factor to translate analog units to volts
	    double _vs[101];                 //Array with voltage - charge definitions
		int _vref;
	    void   _initVoltsArray();
        void   _adc_init();
	    int    _getChargeLevel(double volts);
	    int    _analogRead(int pinNumber);
	    double _analogReadToVolts(int readValue);
};
#endif
