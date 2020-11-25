
#include <Arduino.h>

/************************************************/
/************EXPERIMENT 01***********************/
/************************************************/


volatile uint8_t *ioDDRB,
                 *ioPORTB, // used in experiment 02 only to config button
                 *ioPINB,  // used in experiment 02 only to config button
                 *pTCCR1A, // timer counter control register A 
                 *pTCCR1B, // timer counter control register B . A and B help group information and not nessary similar to I/O A and B.
                 *pOCR1AH, // output control register A high
                 *pOCR1AL, // output control register A low 
                 *pICR1H,  // input capture register high
                 *pICR1L;  // input capture register low


void experiment01() {
  ioDDRB  = (uint8_t *) 0x24;
  
  pTCCR1A = (uint8_t *) 0x80;
  pTCCR1B = (uint8_t *) 0x81;
  pOCR1AH = (uint8_t *) 0x89;
  pOCR1AL = (uint8_t *) 0x88;
  pICR1H = (uint8_t *) 0x87;
  pICR1L = (uint8_t *) 0x86;
  
  /**
   * Write a program to generate a PWM signal with a period of 20 msec on OC1A pin(PB1 - pin 9) using the Timer/Counter 1 peripheral
   * Select Fast PWM mode with TOP value controlled via the ICR1 register
   * Try to create PWM signle with period 20ms on OC1A
   * using clock @ 2Mhz = 16Mhz/8
   * 
   * IRC1/2Mhz = 20ms 
   * IRC1 = 20ms * 2Mhz = 40,000 or 001110001000000b
   */
   //make PB1 an output
   *ioDDRB = 0x02; // DDRB[7:0] = 0000 0010
   uint16_t IRC1_val = 40000;
   uint16_t OCR1A_val = 30000; // value range [0, IRC1_val-1]
  *pTCCR1A = 0b10000010;  // CÓM1A = 10b clear ÓC1A on compare match and set @ bottm
                          // WGM[1:0] = 10b (mode 14 - Fast PWM with ICR1 as TOP value)
  *pTCCR1B = 0b00011010;  // WGM[4:3] = 11b (mode 14 - Fast PWM with ICR1 as TOP value)
                          // CSI[2:0] = 010b (prescale as 8 e.g CLK/8 = 2Mghz)
  *pICR1H  = (IRC1_val >> 8);
  *pICR1L  = IRC1_val & 0xFF;

  *pOCR1AH = (OCR1A_val >> 8);
  *pOCR1AL = OCR1A_val & 0xFF;

  while(1);
}



/************************************************/
/************EXPERIMENT 02***********************/
/************************************************/
// Experiment 02: Drive Servo with different state configure through button


#define STOP_VALUE  3000
#define CW_VALUE    2800
#define CCW_VALUE   3200
#define PRESSED     0
#define NOT_PRESSED 1

#define STATE_SERVO_STOP 0
#define STATE_SERVO_CW   1
#define STATE_SERVO_CCW  2

uint8_t state = 0;
uint8_t pinPB0State = 0;
               
void changeState() {
  /*
   * State 0: stop
   * State 1: rotate servo CW
   * State 2: rotate servo CCW
   */
  state = (state + 1) % 3;
}


void changeTopValue(uint16_t IRC1_val) {
  *pICR1H  = (IRC1_val >> 8);
  *pICR1L  = IRC1_val & 0xFF;
}

void changeDutyCycle(uint16_t OCR1A_val) { // value range [0, IRC1_val-1]
  *pOCR1AH = (OCR1A_val >> 8);
  *pOCR1AL = OCR1A_val & 0xFF;
}

uint8_t readInputPB0() {
  return ((*ioPINB) & 0x01) >> 0;
}

void myHardDelay(uint32_t ms) {
  volatile int16_t count;

  while (ms) {
    for (count = 0; count < 835; count++);
    ms -= 1;
  }
}

int debouncePB0(void) {
  uint8_t currentPB0Val = readInputPB0();

  if (currentPB0Val != pinPB0State) {
    // have a potential pin change!!
    myHardDelay(20); // wait for bounce to end
    currentPB0Val = readInputPB0();
    if (currentPB0Val != pinPB0State) { // if still diff, then it was a transtient and was an actual pin changed
      pinPB0State = currentPB0Val;
      return 1; // pin changed
    }
  }
  return 0; // pin didn't changed
}

void experiment02() { // measure ADC0 reading 
  ioPINB  = (uint8_t *) 0x23;
  ioDDRB  = (uint8_t *) 0x24;
  ioPORTB = (uint8_t *) 0x25;

  pTCCR1A = (uint8_t *) 0x80;
  pTCCR1B = (uint8_t *) 0x81;
  pOCR1AH = (uint8_t *) 0x89;
  pOCR1AL = (uint8_t *) 0x88;
  pICR1H  = (uint8_t *) 0x87;
  pICR1L  = (uint8_t *) 0x86;
  
  /**
   * Write a program to generate a PWM signal with a period of 20 msec on OC1A pin(PB1 - pin 9) using the Timer/Counter 1 peripheral
   * Select Fast PWM mode with TOP value controlled via the ICR1 register
   * Try to create PWM signle with period 20ms on OC1A
   * using clock @ 2Mhz = 16Mhz/8
   * 
   * IRC1/2Mhz = 20ms 
   * IRC1 = 20ms * 2Mhz = 40,000 or 001110001000000b
   */
   //make PB1 an output and PB0 an input
   *ioDDRB = 0x02; // DDRB[7:0] = 0000 0010

   //Enable internal pull up for PB0
   *ioPORTB = 0x01; 
   
   uint16_t IRC1_val = 40000;

  *pTCCR1A = 0b10000010; // CÓM1A = 10b clear ÓC1A on compare match and set @ bottm
                          // WGM[1:0] = 10b (mode 14 - Fast PWM with ICR1 as TOP value)
  *pTCCR1B = 0b00011010;  // WGM[4:3] = 11b (mode 14 - Fast PWM with ICR1 as TOP value)
                          // CSI[2:0] = 010b (prescale as 8 e.g CLK/8 = 2Mghz)

  changeTopValue(IRC1_val);
  changeDutyCycle(STOP_VALUE);
  
  pinPB0State = NOT_PRESSED;

  while(1) {
    if (debouncePB0()) {
      if (pinPB0State == NOT_PRESSED) {
        Serial.println("PB0 not pressed");
      } else {
        Serial.println("PB0 pressed");
        changeState();
      }
    }
    Serial.print("Current state ");
    if (state == STATE_SERVO_STOP) {
      Serial.println("(0:STOP)");
      changeDutyCycle(STOP_VALUE);
    } else if (state == STATE_SERVO_CW) {
      Serial.println("(1:CW)");
      changeDutyCycle(CW_VALUE);
    } else if (state == STATE_SERVO_CCW) {
      Serial.println("(2:CCW)");
      changeDutyCycle(CCW_VALUE);
    } else {
      // why are you here?
    }

  }
}
