#include "RTE_Components.h"
#include CMSIS_device_header

#include "secure.h"                     /* Header file with secure interface API */

#define TZ_START_NS 0x00002000          /* Start address of the non-secure application */

typedef void (*funcptr_void) (void) __attribute__((cmse_nonsecure_call));

void Delay(volatile uint32_t count) {
  while (count--);
}

int main() {
  funcptr_void NonSecure_ResetHandler;

  Delay(200000);

  LED_Init();

  LED0_On();
  Delay(100000);
  LED0_Off();

  __TZ_set_MSP_NS(*((uint32_t *)(TZ_START_NS)));

  NonSecure_ResetHandler = (funcptr_void)(*((uint32_t *)((TZ_START_NS) + 4U)));

  NonSecure_ResetHandler();

  while (1) {
    __NOP();
  }
}
