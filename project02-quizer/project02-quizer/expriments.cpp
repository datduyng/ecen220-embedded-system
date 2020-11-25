
#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include "avr/interrupt.h"
#include "expriments.h"

#define FOSC 16000000 // Clock speed
#define BAUDRATE 9600
#define BAUD2UBRR(baud) FOSC/16/baud-1

volatile uint8_t *pUBRR0L,
                 *pUBRR0H,
                 *pUCSRnA,
                 *pUCSR0B, // USART Control and Status Register 0 B
                 *pUCSR0C,   // USART Control and Status Register 0 C
                 *pUDRn,
                 *ioDDRB,
                 *ioPORTB, // config button
                 *ioPINB  // config button
;



// Global variable
#define UART_BUFFER_SIZE 10

uint8_t rxUartBuffer[UART_BUFFER_SIZE+1];
uint16_t rxUartIndex = 0;

char msg[100];

uart_state uartState = READING;
quiz_state quizState = INTRO;

#define BUTTON_PRESSED     0
#define BUTTON_NOT_PRESSED 1
uint8_t pinPB0State = 0;

volatile unsigned long startQuestionTimer = 0;

void mainProgram() {
  mySerialBegin(BAUDRATE);
  configurePB0ButtonAndPB1();
  uint16_t numQuestions = 0;
  uint16_t questionIndex = 0;
  uint16_t correctAnswers = 0;
  MathQuestion question;
  while(1){
    /* Intializes random number generator */
    srand(millis()); // seed rand() else rand() would just return repeated sequence upon each reboot
    if (debouncePB0()) {
      if (pinPB0State == BUTTON_NOT_PRESSED) {
      } else {
        quizState = RESTART_QUIZ;
      }
    }
    
    if(serialAvailable() && (uartState != OVERFLOW || uartState != FILLED) ) {
      if (rxUartIndex >= UART_BUFFER_SIZE) {
        mySerialWrite("\nERROR: overflow, truncated\n");
        uartState = OVERFLOW;
        flushRxUartBuffer();
      } else {
        uint8_t ascii_char = serialRead();
        rxUartBuffer[rxUartIndex++] = ascii_char;
        if (ascii_char == '\n') {
          rxUartBuffer[rxUartIndex] = '\0'; // null terminator
          uartState = FILLED;
          mySerialWrite(">>> ");
          mySerialWrite((char *)rxUartBuffer);
        }
      }
    }
    if (quizState == INTRO) {
      mySerialWrite("\n===============================\n");
      mySerialWrite("Welcome to quizer version 1.0.0\n");
      quizState = ASK_NUM_QUESTION;
    } else if (quizState == ASK_NUM_QUESTION) {
      mySerialWrite("How many number of questions would you like to quiz on? (5 <= numQuestions <= 25)\n");
      quizState = WAIT_NUM_QUESTION;
    } else if (quizState == WAIT_NUM_QUESTION) {
      if (uartState == FILLED) {
        numQuestions = atoi((char*)rxUartBuffer);
        sprintf(msg, "This many question: %d\n", numQuestions);
        mySerialWrite((char *) msg);
        flushRxUartBuffer();
        if (numQuestions >= 5 && numQuestions <= 25) {
          quizState = ASK_QUESTION;
        } else {
          mySerialWrite("ERROR: Make sure number of questions is (5 <= numQuestions <= 25)\n");
          quizState = ASK_NUM_QUESTION;
        }
      }
    } else if (quizState == ASK_QUESTION) {
      question = generateRandomMathQuestion();
      sprintf(msg, "What is %d %c %d? (%d out of %d questions)\n", question.num1, question.mathOperator, question.num2, questionIndex+1, numQuestions);
      mySerialWrite((char *) msg);
      startQuestionTimer = millis();
      quizState = WAIT_FOR_ANSWER;
    } else if (quizState == WAIT_FOR_ANSWER) {
      if (uartState == FILLED) {
        unsigned long totalTime = (millis() - startQuestionTimer);
        uint16_t answer = atoi((char*)rxUartBuffer);
        flushRxUartBuffer();
        
        if (answer == question.expectedResult) { // correct
          sprintf(msg, "CORRECT: %d - time: %ldms\n", answer, totalTime);
          correctAnswers++;
        } else {
          for (int i=0; i<3; i++) {
            turnOnPB1LED();
            myHardDelay(100);
            turnOffPB1LED();
            myHardDelay(100);
          }
          sprintf(msg, "WRONG: %d - time: %ldms\n", answer, totalTime);
        }
        mySerialWrite((char *) msg);
        sprintf(msg, "Current score: %d/%d\n", correctAnswers, numQuestions);
        mySerialWrite(msg);
        questionIndex++;
        if (questionIndex == numQuestions) {
          quizState = FINISH_QUIZ;
        } else {
          quizState = ASK_QUESTION;
        }
      }
    } else if (quizState == FINISH_QUIZ) {
      sprintf(msg, "Congrats!! Your score: %d/%d\n", correctAnswers, numQuestions);
      mySerialWrite(msg);
      quizState = RESTART_QUIZ;
    } else if (quizState == RESTART_QUIZ) {
      mySerialWrite("Restarting...\n\n");
      numQuestions = 0;
      questionIndex = 0;
      correctAnswers = 0;
      quizState = INTRO;
    }
  }
}
/*************************** Configure PB0 Button to restart quiz and PB1 as output to turn on LED *************/
void configurePB0ButtonAndPB1() {
  ioPINB  = (uint8_t *) 0x23;
  ioDDRB = (uint8_t *) 0x24;
  ioPORTB = (uint8_t *) 0x25;

  pinPB0State = BUTTON_NOT_PRESSED;

    //make PB1 as output
  *ioDDRB = 0x02; // DDRB[7:0] 0000 0010
  
  //Enable internal pull up for PB0
  *ioPORTB = 0x01; 


}

void turnOnPB1LED() {
  *ioPORTB = (*ioPORTB) | 0x02; // 0000 0010
}

void turnOffPB1LED() {
  *ioPORTB = (*ioPORTB) & 0b11111101; // 0000 0000
}

uint8_t readInputPB0() {
  return ((*ioPINB) & 0x01) >> 0;
}

int debouncePB0() {
  uint8_t currentPB0Val = readInputPB0();

  if (currentPB0Val != pinPB0State) {
    // have a potential pin change!!
    myHardDelay(50); // wait for bounce to end
    currentPB0Val = readInputPB0();
    if (currentPB0Val != pinPB0State) { // if still diff, then it was a transtient and was an actual pin changed
      pinPB0State = currentPB0Val;
      return 1; // pin changed
    }
  }
  return 0; // pin didn't changed
}

/*****************Button***********************/

void flushRxUartBuffer() {
  memset(rxUartBuffer, 0, sizeof(rxUartBuffer));
  uartState = READING;
  rxUartIndex = 0;
}

void mySerialBegin(uint32_t baudrate) {
  pUCSRnA = (uint8_t *) 0xC0;
  pUCSR0B = (uint8_t *) 0xC1;  // USART Control and Status Register 0 B
  pUCSR0C = (uint8_t *) 0xC2;  // USART Control and Status Register 0 C
  pUBRR0L = (uint8_t *) 0xC4;
  pUBRR0H = (uint8_t *) 0xC5;
  pUDRn   = (uint8_t *) 0xC6;
  
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



boolean serialAvailable() {
  #define RXC 7 // USART Receive complete, 
  return ((*pUCSRnA) & (1 << RXC)) == (1 << RXC);
}


uint8_t serialRead() {
  return *pUDRn;
}

void mySerialWrite(uint8_t * msg) {
  while ((*msg) != 0) {
    mySerialWriteOne(*msg);
    msg++;
  }
}


MathQuestion generateRandomMathQuestion() {
  MathQuestion question;
  question.mathOperator = (rand() % 2) == 0 ? '*' : '/';
  int num1 = (rand() % 10) + 1, num2 = (rand() % 10) + 2;
  if (question.mathOperator == '*') {
    question.num1 = num1;
    question.num2 = num2;
    question.expectedResult = num1 * num2;
  } else {
    question.num1 = num1 * num2;
    question.num2 = num1;
    question.expectedResult = num2;
  }
  return question;
}


void myHardDelay(uint32_t ms) {
  volatile int16_t count;

  while (ms) {
    for (count = 0; count < 835; count++);
    ms -= 1;
  }
}
