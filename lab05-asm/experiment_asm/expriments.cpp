#include <Arduino.h>

#define DELAY_VALUE 43459
/*
 * the free register are r16 - r31
 */
volatile uint8_t *ioPORTB, *ioDDRB;

void myHardDelayUsec(uint16_t delayInUsec) {
  uint16_t unused;
  asm volatile (
    "loop:            \n\t"
    "sbiw %0, 1       \n\t" // 2 clk from subi. (
    "nop              \n\t" // 1 clk
    "nop              \n\t"
    "nop              \n\t"
    "nop              \n\t"
    "nop              \n\t"
    "nop              \n\t"
    "nop              \n\t"
    "nop              \n\t"
    "nop              \n\t"
    "nop              \n\t"
    "nop              \n\t"
    "nop              \n\t"
    "brne loop        \n\t" // 2 clk. Last loop is 1clk.
    : "=w" (unused)
    : "0" (delayInUsec) // placeholder. at first, gcc will load delayInUsec into 2 register and pass it to %0
  );
}
 

void experiment01() {
  ioPORTB = (uint8_t *)0x25;
  ioDDRB = (uint8_t *) 0x24;
  //make PB1 as output
  *ioDDRB = 0x02; // DDRB[7:0] 0000 0010
  while (1) {
    asm volatile (
      "ldi r16, 0x02 \n\t" // 1 cycle. load r16 with 0x02.
      "sts 0x25, r16 \n\t" // store content in r16 to add 0x25 (PORTB), 2 cycle
    );
    myHardDelayUsec(DELAY_VALUE);
    asm volatile (
      "ldi r17, 0x00 \n\t"
      "sts 0x25, r17 \n\t"
    );   
    myHardDelayUsec(DELAY_VALUE);
  }
}
