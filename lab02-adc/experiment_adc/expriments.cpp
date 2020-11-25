
#include <Arduino.h>

volatile uint8_t *ioPORTB, 
                 *ioDDRB;

volatile uint8_t *pADMUX, // configure voltage ref and pin
                 *pADCSRA, // Control status register A
                 *pADCL,
                 *pADCH;

uint16_t readADC0() {
  uint16_t result;
  *pADCSRA = (*pADCSRA) | 0x40; // start conversion
  while (((*pADCSRA) & 0x40) == 0x40) { // while still conversion
    // do nothing. waiting ADSC bit to be cleared by hardware
  }
  
  result = *pADCL; // lower 8bits of ADC conversion (0x00??)
  result = result | (((uint16_t)*pADCH) << 8);
  result = result & 0x3FFF; // clear bits 15-10 (ADC res is only 10 bits)
  return result;
}

void experiment01() {
  pADMUX = (uint8_t *)0x7C;
  pADCSRA = (uint8_t *)0x7A;
  pADCL = (uint8_t *)0x78;
  pADCH = (uint8_t *)0x79;

  uint16_t result;
  
  *pADMUX = 0x40; //0100 0000. 01: using Vcc. 0000: ADC0
  *pADCSRA =  0x87; // 1000 0111. 1. ADC enable. 111: prescaler = clock/128

  while (1) {
    Serial.print("result = ");
    Serial.println(readADC0());
  }
}


void myHardDelay(uint32_t ms) {
  volatile int16_t count;

  while (ms) {
    for (count = 0; count < 835; count++);
    ms -= 1;
  }
}

void setIOPORTB(boolean high) {
  if (high) {
    *ioPORTB = 0x01;// 0000 0001 . // set PB0 high
  } else {
    // set PB1 low
    *ioPORTB = 0x00; // 0000 0000  
  }
}

void experiment02() { // measure ADC0 reading 
  pADMUX = (uint8_t *)0x7C;
  pADCSRA = (uint8_t *)0x7A;
  pADCL = (uint8_t *)0x78;
  pADCH = (uint8_t *)0x79;

  *pADMUX = 0x40; //0100 0000. 01: using Vcc. 0000: ADC0
  *pADCSRA =  0x87; // 1000 0111. 1. ADC enable. 111: prescaler = clock/128
  
  ioPORTB = (uint8_t *)0x25;
  ioDDRB = (uint8_t *) 0x24;
  
  *ioDDRB = 0x01; // DDRB[7:0] 0000 0001 // set PB0 as output , PB0 - port 8 on arduino

  while (1) {
    setIOPORTB(true);
    readADC0();
    setIOPORTB(false);
    myHardDelay(1);
  }
}

void experiment03() { //measure ADC0 reading then convert it to volt 
  pADMUX = (uint8_t *)0x7C;
  pADCSRA = (uint8_t *)0x7A;
  pADCL = (uint8_t *)0x78;
  pADCH = (uint8_t *)0x79;

  *pADMUX = 0x40; //0100 0000. 01: using Vcc. 0000: ADC0
  *pADCSRA =  0x87; // 1000 0111. 1. ADC enable. 111: prescaler = clock/128

  float adc_reading;
  while (1) {
    adc_reading = (float) (readADC0() + readADC0()) / 2. + 1.;
    float vin = adc_reading * 5. / 1024.;// vin = (ADC+1) * Vref / 1024
    Serial.print("Voltage reading = ");
    Serial.println(vin);
    myHardDelay(100);
  }
}
