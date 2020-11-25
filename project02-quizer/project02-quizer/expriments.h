
#ifndef EXPERIMENTS_H
#define EXPERIMENTS_H
#include <stdio.h>

typedef enum {
  OVERFLOW = 0,
  FILLED,
  READING
} uart_state;


typedef enum {
  INTRO = 0,
  ASK_NUM_QUESTION,
  WAIT_NUM_QUESTION,
  ASK_QUESTION,
  WAIT_FOR_ANSWER,
  FINISH_QUIZ,
  RESTART_QUIZ
} quiz_state;


typedef struct{
  char mathOperator; // * or /
  int num1;
  int num2;
  int expectedResult;
} MathQuestion;

void mySerialBegin(uint32_t baudrate);
uint8_t serialRead();
boolean serialAvailable();
void mySerialWriteOne(uint8_t data);
void mySerialWrite(uint8_t * msg);
void flushRxUartBuffer();

void myHardDelay(uint32_t ms);

uint8_t readInputPB0();
void configurePB0ButtonAndPB1();
int debouncePB0();
void turnOnPB1LED();
void turnOffPB1LED();

MathQuestion generateRandomMathQuestion();

void mainProgram();

#endif
