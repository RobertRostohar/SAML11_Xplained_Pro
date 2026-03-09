#include "RTE_Components.h"
#include CMSIS_device_header

#define TZ_START_AS 0x00001000          /* Start address of the secure application */

#define LED0_MASK PORT_PA07             /* LED0 on SAML10-XPRO: PA07 */

typedef void (*funcptr_void) (void);

void Delay(volatile uint32_t count) {
  while (count--);
}

void LED_Init(void) {
  PORT_SEC_REGS->GROUP[0].PORT_DIRSET = LED0_MASK; 
}

void LED0_On (void) {
  PORT_SEC_REGS->GROUP[0].PORT_OUTCLR = LED0_MASK; 
}

void LED0_Off (void) {
  PORT_SEC_REGS->GROUP[0].PORT_OUTSET = LED0_MASK; 
}

int main() {
  funcptr_void Secure_Application_ResetHandler;

  LED_Init();

  LED0_On();
  Delay(10000);
  LED0_Off();

  Secure_Application_ResetHandler = (funcptr_void)(*((uint32_t *)((TZ_START_AS) + 4U)));

  Secure_Application_ResetHandler();

  while (1) {
    __NOP();
  }
}
