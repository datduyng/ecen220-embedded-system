
#include <Arduino.h>
#include "avr/interrupt.h"

#define FOSC 16000000 // Clock speed
#define BAUDRATE 19200
#define BAUD2UBRR(baud) FOSC/16/baud-1

volatile uint8_t *pUBRR0L,
                 *pUBRR0H,
                 *pUCSRnA,
                 *pUCSR0B, // USART Control and Status Register 0 B
                 *pUCSR0C,   // USART Control and Status Register 0 C
                 *pUDRn,
                 *pSREG
;

void myHardDelay(uint32_t ms) {
  volatile int16_t count;

  while (ms) {
    for (count = 0; count < 835; count++);
    ms -= 1;
  }
}

void mySerialBegin(uint32_t baudrate) {
  #define TXEN0 3
  #define RXEN0 4
  #define RXCIE0 7 // RX complete interupt enable bit
  
  #define UCSZ0_01 1
  
  uint32_t ubrr = BAUD2UBRR(baudrate);
  *pUBRR0H = (uint8_t) (ubrr >> 8);
  *pUBRR0L = (uint8_t) ubrr;

  *pUCSRnA = 0x00;
  
  // b[1]Enable receiver and b[0]transmitter
  *pUCSR0B = (1<<RXEN0) | (1<<TXEN0);

  // Set frame format: 8bit data, default 1stop bit
  *pUCSR0C = (3 << UCSZ0_01);
}

void mySerialWriteOne(uint8_t data) {
  #define UDREn 5 // USART Data Register Empty
  
  /* Wait for empty transmit buffer */ 
  while ( !( (*pUCSRnA) & (1<<UDREn)) );
  
  /* Put data into buffer, sends the data */ 
  *pUDRn = data;
}

void mySerialWrite(uint8_t * msg) {
  while ((*msg) != 0) {
    mySerialWriteOne(*msg);
    msg++;
  }
}

void configure_register() {
  pUCSRnA = (uint8_t *) 0xC0;
  pUCSR0B = (uint8_t *) 0xC1;  // USART Control and Status Register 0 B
  pUCSR0C = (uint8_t *) 0xC2;  // USART Control and Status Register 0 C
  pUBRR0L = (uint8_t *) 0xC4;
  pUBRR0H = (uint8_t *) 0xC5;
  pUDRn   = (uint8_t *) 0xC6;
  pSREG = (uint8_t *) 0x5F; // GLOBAL interupt
}

void mySerialBeginWithInterupt(uint32_t baudrate) {
  #define TXEN0 3
  #define RXEN0 4
  #define RXCIE0 7 // RX complete interupt enable bit
  
  #define UCSZ0_01 1
  
  uint32_t ubrr = BAUD2UBRR(baudrate);
  *pUBRR0H = (uint8_t) (ubrr >> 8);
  *pUBRR0L = (uint8_t) ubrr;

  *pSREG |= (0x80); // turn on global interrupt
  
  *pUCSRnA = 0x00;
  
  //b[7]Set RX complete interupt enable, b[1]Enable receiver and b[0]transmitter
  *pUCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1 << RXCIE0);

  // Set frame format: 8bit data, default 1stop bit
  *pUCSR0C = (3 << UCSZ0_01);
}


ISR(USART_RX_vect, ISR_BLOCK) {
  uint8_t rxData = *pUDRn;
  mySerialWriteOne(rxData);
}

void experiment01() {
  configure_register();
  mySerialBeginWithInterupt(BAUDRATE);
  mySerialWrite("Start of program #1 \n");
  while(1);
}

void experiment02() {
  configure_register();
  mySerialBegin(BAUDRATE);
  while(1){
    mySerialWrite("Testing\n");
    myHardDelay(1000);
  }
}
