#include <stdint.h>

//global
volatile uint8_t *ioPORTB;
volatile uint8_t *ioDDRB;

int main() 
{
  ioPORTB = (uint8_t *)0x25;
  ioDDRB = (uint8_t *) 0x24;

  //make PB1 as output
  *ioDDRB = 0x02; // DDRB[7:0] 0000 0010
  
  while (1) {
    // set PB1 high
    *ioPORTB = 0x02; // 0000 0010
    // set PB1 low
    *ioPORTB = 0x00; // 0000 0000
  }
}
