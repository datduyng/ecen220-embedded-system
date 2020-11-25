#include <stdint.h>

//global
volatile uint8_t *ioPORTB;
volatile uint8_t *ioDDRB;


void myHardDelay(uint32_t ms) {
  volatile int16_t count;

  while (ms) {
    for (count = 0; count < 835; count++);
    ms -= 1;
  }
}

int main() 
{
  init(); // init arduino libraries 
  Serial.begin(9600);
  
  ioPORTB = (uint8_t *)0x25;
  ioDDRB = (uint8_t *) 0x24;

  //make PB1 as output
  *ioDDRB = 0x02; // DDRB[7:0] 0000 0010


  while (1) {
    // set PB1 high
    *ioPORTB = 0x02; // 0000 0010

    myHardDelay(50);
    
    // set PB1 low
    *ioPORTB = 0x00; // 0000 0000
    myHardDelay(50);
  }
}
