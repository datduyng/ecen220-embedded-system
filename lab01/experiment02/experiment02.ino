#include <stdint.h>

//global
volatile uint8_t *ioPORTB;
volatile uint8_t *ioDDRB;

const int msgLen = 1;
uint8_t msg[msgLen+1];

int main() 
{
  init(); // init arduino libraries 
  Serial.begin(9600);
  
  ioPORTB = (uint8_t *)0x25;
  ioDDRB = (uint8_t *) 0x24;

  //make PB1 as output
  *ioDDRB = 0x02; // DDRB[7:0] 0000 0010

  for (int i=0; i<msgLen; i++) {
    msg[i] = 'r';
  }
  msg[msgLen] = '\0';
  
  uint32_t count = 0;
  while (1) {
    // set PB1 high
    *ioPORTB = 0x02; // 0000 0010

    Serial.write((char *) msg);
    
    // set PB1 low
    *ioPORTB = 0x00; // 0000 0000
    Serial.write((char *) msg);
  }
}
