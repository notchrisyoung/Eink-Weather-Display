#include "Arduino.h"
#include "BatteryVoltage.h"

BatteryVoltage::BatteryVoltage(){
    _reads = DEF_READS;
    _convFactor = DEF_CONV_FACTOR;
    _addressPin = DEF_PIN;
	_vref = 1100;
    _initVoltsArray();
}
BatteryVoltage::BatteryVoltage(int addressPin){
    _reads = DEF_READS;
    _convFactor = DEF_CONV_FACTOR;
    _addressPin = addressPin;
	_vref = 1100;
    _initVoltsArray();
}
BatteryVoltage::BatteryVoltage(int addressPin, double convFactor){
    _reads = DEF_READS;
    _convFactor = convFactor;
    _addressPin = addressPin;
	_vref = 1100;
    _initVoltsArray();
}
BatteryVoltage::BatteryVoltage(int addressPin, double convFactor, int reads){
    _reads = reads;
    _convFactor = convFactor;
    _addressPin = addressPin;
	_vref = 1100;
    _initVoltsArray();
}

int BatteryVoltage::getAnalogPin(){
    return _addressPin;
}
double BatteryVoltage::getConvFactor(){
    return _convFactor;
}
double BatteryVoltage::getBatteryVolts(){
    int readValue = analogRead(_addressPin);
    return _analogReadToVolts(readValue);
}
int BatteryVoltage::getBatteryChargeLevel(){
    int readValue = _analogRead(_addressPin);
    double volts = _analogReadToVolts(readValue);
    return _getChargeLevel(volts);
}
int BatteryVoltage::pinRead(){
    return _analogRead(_addressPin);
}
void BatteryVoltage::_adc_init() {
	esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
        _vref = adc_chars.vref;
    }
}
void BatteryVoltage::_initVoltsArray(){
    _vs[0] = 3.200;
    _vs[1] = 3.250; _vs[2] = 3.300; _vs[3] = 3.350; _vs[4] = 3.400; _vs[5] = 3.450;
    _vs[6] = 3.500; _vs[7] = 3.550; _vs[8] = 3.600; _vs[9] = 3.650; _vs[10] = 3.700;
    _vs[11] = 3.703; _vs[12] = 3.706; _vs[13] = 3.710; _vs[14] = 3.713; _vs[15] = 3.716;
    _vs[16] = 3.719; _vs[17] = 3.723; _vs[18] = 3.726; _vs[19] = 3.729; _vs[20] = 3.732;
    _vs[21] = 3.735; _vs[22] = 3.739; _vs[23] = 3.742; _vs[24] = 3.745; _vs[25] = 3.748;
    _vs[26] = 3.752; _vs[27] = 3.755; _vs[28] = 3.758; _vs[29] = 3.761; _vs[30] = 3.765;
    _vs[31] = 3.768; _vs[32] = 3.771; _vs[33] = 3.774; _vs[34] = 3.777; _vs[35] = 3.781;
    _vs[36] = 3.784; _vs[37] = 3.787; _vs[38] = 3.790; _vs[39] = 3.794; _vs[40] = 3.797;
    _vs[41] = 3.800; _vs[42] = 3.805; _vs[43] = 3.811; _vs[44] = 3.816; _vs[45] = 3.821;
    _vs[46] = 3.826; _vs[47] = 3.832; _vs[48] = 3.837; _vs[49] = 3.842; _vs[50] = 3.847;
    _vs[51] = 3.853; _vs[52] = 3.858; _vs[53] = 3.863; _vs[54] = 3.868; _vs[55] = 3.874;
    _vs[56] = 3.879; _vs[57] = 3.884; _vs[58] = 3.889; _vs[59] = 3.895; _vs[60] = 3.900;
    _vs[61] = 3.906; _vs[62] = 3.911; _vs[63] = 3.917; _vs[64] = 3.922; _vs[65] = 3.928;
    _vs[66] = 3.933; _vs[67] = 3.939; _vs[68] = 3.944; _vs[69] = 3.950; _vs[70] = 3.956;
    _vs[71] = 3.961; _vs[72] = 3.967; _vs[73] = 3.972; _vs[74] = 3.978; _vs[75] = 3.983;
    _vs[76] = 3.989; _vs[77] = 3.994; _vs[78] = 4.000; _vs[79] = 4.008; _vs[80] = 4.015;
    _vs[81] = 4.023; _vs[82] = 4.031; _vs[83] = 4.038; _vs[84] = 4.046; _vs[85] = 4.054;
    _vs[86] = 4.062; _vs[87] = 4.069; _vs[88] = 4.077; _vs[89] = 4.085; _vs[90] = 4.092;
    _vs[91] = 4.100; _vs[92] = 4.111; _vs[93] = 4.122; _vs[94] = 4.133; _vs[95] = 4.144;
    _vs[96] = 4.156; _vs[97] = 4.167; _vs[98] = 4.178; _vs[99] = 4.189; _vs[100] = 4.200;
}
double BatteryVoltage::_analogReadToVolts(int readValue){
    float battery_voltage = ((float)readValue / 4095.0) * 2.0 * 3.3 * (_vref / 1000.0);
	Serial.println(String(battery_voltage));
	return battery_voltage;
}
int BatteryVoltage::_analogRead(int pinNumber){
	int totalValue = 0;
	int averageValue = 0;
	for(int i = 0; i < _reads; i++){
		totalValue += analogRead(pinNumber);
	}
	averageValue = totalValue / _reads;
	return averageValue;
}
int BatteryVoltage::_getChargeLevel(double volts){
  int idx = 50;
  int prev = 0;
  int half = 0;
  if (volts >= 4.19){
    return 100;
  }
  if (volts <= 3.0){
    return 0;
  }
  while(true){
    half = abs(idx - prev) / 2;
    prev = idx;
    if(volts >= _vs[idx]){
      idx = idx + half;
    }else{
      idx = idx - half;
    }
    if (prev == idx){
      break;
    }
  }
  return idx;
}
