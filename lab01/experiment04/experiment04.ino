#include <stdint.h>

//global
volatile uint8_t *ioPORTB, *ioDDRB, *ioPINB;

void myHardDelay(uint32_t ms) {
  volatile int16_t count;

  for (;ms > 0; ms--) {
    for (count = 0; count < 835; count++);
  }
}
int main() 
{
  init(); // init arduino libraries 
  Serial.begin(9600);
  
  ioPORTB = (uint8_t *)0x25;
  ioDDRB = (uint8_t *) 0x24;
  ioPINB = (uint8_t *) 0x23;

  *ioDDRB = 0x00; // // make port B become input including DDRB[0]
  *ioPORTB = 0x01; // Use input pull up on PB0
 
  while (1) {
    uint8_t reading = (*ioPINB & 0x01);

    if (reading == 0) {
      Serial.write("Pressed\n");
      if ((*ioPINB & 0x01) == 1) {
        Serial.write("Released while pressing");
      }
    } else {
      Serial.write("Released\n");
      if ((*ioPINB & 0x01) == 0) {
        Serial.write("Pressed while releasing");
      }
    }
    myHardDelay(40);
  }
}
