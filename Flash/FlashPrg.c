/**************************************************************************//**
 * @file     FlashPrg.c
 * @brief    Flash Programming Functions adapted for New Device Flash
 * @version  V1.0.0
 * @date     10. January 2018
 ******************************************************************************/
/*
 * Copyright (c) 2010-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "RTE_Components.h"
#include CMSIS_device_header

#include "FlashOS.h"            // FlashOS Structures

/* 
   Mandatory Flash Programming Functions (Called by FlashOS):
                int Init        (unsigned long adr,   // Initialize Flash
                                 unsigned long clk,
                                 unsigned long fnc);
                int UnInit      (unsigned long fnc);  // De-initialize Flash
                int EraseSector (unsigned long adr);  // Erase Sector Function
                int ProgramPage (unsigned long adr,   // Program Page Function
                                 unsigned long sz,
                                 unsigned char *buf);

   Optional  Flash Programming Functions (Called by FlashOS):
                int BlankCheck  (unsigned long adr,   // Blank Check
                                 unsigned long sz,
                                 unsigned char pat);
                int EraseChip   (void);               // Erase complete Device
      unsigned long Verify      (unsigned long adr,   // Verify Function
                                 unsigned long sz,
                                 unsigned char *buf);

       - BlanckCheck  is necessary if Flash space is not mapped into CPU memory space
       - Verify       is necessary if Flash space is not mapped into CPU memory space
       - if EraseChip is not provided than EraseSector for all sectors is called
*/


static void PrepareFlashOperation (void) {
  nvmctrl_registers_t *nvmctrl;
  pac_registers_t *pac;
  uint8_t dal;

  // Select NVMCTRL and PAC Register Base on Debug Access Level
  dal = (DSU_EXT_REGS->DSU_STATUSB & DSU_STATUSB_DAL_Msk) >> DSU_STATUSB_DAL_Pos;
  if (dal == DSU_STATUSB_DAL_FULL_DEBUG) {
    nvmctrl = NVMCTRL_SEC_REGS;
    pac = PAC_SEC_REGS;
  } else {
    nvmctrl = NVMCTRL_REGS;
    pac = PAC_REGS;
  }

  // PAC: Disable Write Protection for NVMCTRL
  pac->PAC_WRCTRL = PAC_WRCTRL_PERID(34) |
                    PAC_WRCTRL_KEY(PAC_WRCTRL_KEY_CLEAR_Val);

  // NVMCTRL: Set Read Wait States and Disable Cache
  nvmctrl->NVMCTRL_CTRLB = NVMCTRL_CTRLB_RWS(15) |
                           NVMCTRL_CTRLB_CACHEDIS_Msk;

  // NVMCTRL: Set Manual Write
  nvmctrl->NVMCTRL_CTRLC = NVMCTRL_CTRLC_MANW_Msk;
}


static void ClearFlashPageBuffer (void) {
  nvmctrl_registers_t *nvmctrl;
  uint8_t dal;

  // Select NVMCTRL Register Base on Debug Access Level
  dal = (DSU_EXT_REGS->DSU_STATUSB & DSU_STATUSB_DAL_Msk) >> DSU_STATUSB_DAL_Pos;
  if (dal == DSU_STATUSB_DAL_FULL_DEBUG) {
    nvmctrl = NVMCTRL_SEC_REGS;
  } else {
    nvmctrl = NVMCTRL_REGS;
  }

  // Only clear page buffer if necessary. PBC commands can otherwise fail.
  if ((nvmctrl->NVMCTRL_STATUS & NVMCTRL_STATUS_LOAD_Msk) == 0U) {
    return;
  }

  // NVMCTRL: Clear all errors and DONE flag
  nvmctrl->NVMCTRL_INTFLAG = NVMCTRL_INTFLAG_DONE_Msk  |
                             NVMCTRL_INTFLAG_PROGE_Msk |
                             NVMCTRL_INTFLAG_LOCKE_Msk |
                             NVMCTRL_INTFLAG_NVME_Msk  |
                             NVMCTRL_INTFLAG_KEYE_Msk;

  // NVMCTRL: Clear Page Buffer
  nvmctrl->NVMCTRL_CTRLA = NVMCTRL_CTRLA_CMD(NVMCTRL_CTRLA_CMD_PBC_Val) |
                           NVMCTRL_CTRLA_CMDEX(NVMCTRL_CTRLA_CMDEX_KEY_Val);

  // NVMCTRL: Wait for LOAD and READY flag
  while ((nvmctrl->NVMCTRL_STATUS & (NVMCTRL_STATUS_LOAD_Msk  |
                                     NVMCTRL_STATUS_READY_Msk)) != NVMCTRL_STATUS_READY_Msk);
}


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

  switch (fnc) {
    case 1: // Erase
      PrepareFlashOperation();
      break;
    case 2: // Program
      PrepareFlashOperation();
      ClearFlashPageBuffer();
      break;
    case 3: // Verify
      break;
    default:
      return (1);               // Unsupported Function Code
  }

  return (0);                   // Finished without Errors
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {
  return (0);                   // Finished without Errors
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

// int EraseChip (void) {
//   return (0);                   // Finished without Errors
// }


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {
  nvmctrl_registers_t *nvmctrl;
  uint8_t dal;

  // Select NVMCTRL Register Base on Debug Access Level
  dal = (DSU_EXT_REGS->DSU_STATUSB & DSU_STATUSB_DAL_Msk) >> DSU_STATUSB_DAL_Pos;
  if (dal == DSU_STATUSB_DAL_FULL_DEBUG) {
    nvmctrl = NVMCTRL_SEC_REGS;
  } else {
    nvmctrl = NVMCTRL_REGS;
  }

  // NVMCTRL: Clear all errors and DONE flag
  nvmctrl->NVMCTRL_INTFLAG = NVMCTRL_INTFLAG_DONE_Msk  |
                             NVMCTRL_INTFLAG_PROGE_Msk |
                             NVMCTRL_INTFLAG_LOCKE_Msk |
                             NVMCTRL_INTFLAG_NVME_Msk  |
                             NVMCTRL_INTFLAG_KEYE_Msk;

  if (dal < DSU_STATUSB_DAL_FULL_DEBUG) {
    // Indirect addressing
    ClearFlashPageBuffer();
    *((unsigned long *)(adr)) = 0xFFFFFFFFU;
  } else {
    // Direct addressing
    nvmctrl->NVMCTRL_ADDR = adr;
  }

  // NVMCTRL: Erase Row Command
  nvmctrl->NVMCTRL_CTRLA = NVMCTRL_CTRLA_CMD(NVMCTRL_CTRLA_CMD_ER_Val) |
                           NVMCTRL_CTRLA_CMDEX(NVMCTRL_CTRLA_CMDEX_KEY_Val);

  // NVMCTRL: Wait for READY flag
  while ((nvmctrl->NVMCTRL_STATUS & NVMCTRL_STATUS_READY_Msk) == 0U);

  // NVMCTRL: Check for Errors and DONE flag
  if ((nvmctrl->NVMCTRL_INTFLAG & (NVMCTRL_INTFLAG_DONE_Msk  | 
                                   NVMCTRL_INTFLAG_PROGE_Msk |
                                   NVMCTRL_INTFLAG_LOCKE_Msk |
                                   NVMCTRL_INTFLAG_NVME_Msk  |
                                   NVMCTRL_INTFLAG_KEYE_Msk)) != NVMCTRL_INTFLAG_DONE_Msk) {
    return (1);                 // Failed to Erase Row
  }

  return (0);                   // Finished without Errors
}


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
  nvmctrl_registers_t *nvmctrl;
  uint8_t dal;

  // Select NVMCTRL Register Base on Debug Access Level
  dal = (DSU_EXT_REGS->DSU_STATUSB & DSU_STATUSB_DAL_Msk) >> DSU_STATUSB_DAL_Pos;
  if (dal == DSU_STATUSB_DAL_FULL_DEBUG) {
    nvmctrl = NVMCTRL_SEC_REGS;
  } else {
    nvmctrl = NVMCTRL_REGS;
  }

  sz = (sz + 3U) & ~3U;         // Align size to 4 bytes

  // Write Page Data
  while (sz) {
    *((unsigned long *)(adr)) = *((unsigned long *)buf);
    adr += 4U;
    buf += 4U;
    sz  -= 4U;
  }

  // NVMCTRL: Clear all errors and DONE flag
  nvmctrl->NVMCTRL_INTFLAG = NVMCTRL_INTFLAG_DONE_Msk  |
                             NVMCTRL_INTFLAG_PROGE_Msk |
                             NVMCTRL_INTFLAG_LOCKE_Msk |
                             NVMCTRL_INTFLAG_NVME_Msk  |
                             NVMCTRL_INTFLAG_KEYE_Msk;

  // NVMCTRL: Write Page Command
  nvmctrl->NVMCTRL_CTRLA = NVMCTRL_CTRLA_CMD(NVMCTRL_CTRLA_CMD_WP_Val) |
                           NVMCTRL_CTRLA_CMDEX(NVMCTRL_CTRLA_CMDEX_KEY_Val);

  // NVMCTRL: Wait for READY flag
  while ((nvmctrl->NVMCTRL_STATUS & NVMCTRL_STATUS_READY_Msk) == 0U);

  // NVMCTRL: Check for Errors and DONE flag
  if ((nvmctrl->NVMCTRL_INTFLAG & (NVMCTRL_INTFLAG_DONE_Msk  | 
                                   NVMCTRL_INTFLAG_PROGE_Msk |
                                   NVMCTRL_INTFLAG_LOCKE_Msk |
                                   NVMCTRL_INTFLAG_NVME_Msk  |
                                   NVMCTRL_INTFLAG_KEYE_Msk)) != NVMCTRL_INTFLAG_DONE_Msk) {
    return (1);                 // Failed to Program Page
  }

  return (0);                   // Finished without Errors
}
