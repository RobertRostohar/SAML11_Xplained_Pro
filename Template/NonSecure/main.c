#include "RTE_Components.h"
#include CMSIS_device_header

#include "..\Secure\veneer.h"

void Delay(volatile uint32_t count) {
  while (count--);
}

int main() {

  Delay(500000);

  ansc_LED_Init();                             
  
  while(1) {
    ansc_LED0_On();
    Delay(200000);
    ansc_LED0_Off();
    Delay(200000);
  }
}
