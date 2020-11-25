#include <stdint.h>;
#include "expriments.h";

int main(void) {
  init();
  Serial.begin(57600);
  experiment01(); 
  return 0;
}
