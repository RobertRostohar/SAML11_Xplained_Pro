/**
 * Copyright (c) 2019 Microchip Technology Inc.
 *
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "RTE_Components.h"
#include CMSIS_device_header

#include "secure.h"    /* Header file with secure interface API */

#define LED0_MASK PORT_PA07             /* LED0 on SAML10-XPRO: PA07 */

void LED_Init(void) {
  PORT_SEC_REGS->GROUP[0].PORT_DIRSET = LED0_MASK; 
}

void LED0_On (void) {
  PORT_SEC_REGS->GROUP[0].PORT_OUTCLR = LED0_MASK; 
}

void LED0_Off (void) {
  PORT_SEC_REGS->GROUP[0].PORT_OUTSET = LED0_MASK; 
}
