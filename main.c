/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher
 *----------------------------------------------------------------------------*/

#include "sam.h"                        /* Device header */

#define LED0_MASK PORT_PA07             /* LED0 on SAML10-XPRO: PA07 */

static volatile uint32_t msTicks;       /* counts 1ms timeTicks */

extern uint32_t SystemCoreClock;
extern void     SystemCoreClockUpdate(void);

void SysTick_Handler(void) {
  msTicks++;                        /* increment counter necessary in Delay() */
}

void Delay (uint32_t dlyTicks) {
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
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

int main (void) {

  SystemCoreClockUpdate();
  SysTick_Config(SystemCoreClock / 1000);         /* Setup SysTick for 1 msec */

  LED_Init();                             
  
  while(1) {
    LED0_On();
    Delay(500U);
    LED0_Off();
    Delay(500U);
  }
}
