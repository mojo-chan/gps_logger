/*
 * eeprom.c
 * Project:	Micro Phocus Comms Device
 * Author:	Paul Qureshi
 * Created: 26/06/2012 13:25:00
 *
 * EEPROM driver based on Atmel app note code (eeprom_driver.c)
 */

#include <avr/io.h>
#include <string.h>

#include "eeprom.h"

/**************************************************************************************************
** Wait for NVM access to finish.
*/
inline void EEP_WaitForNVM(void)
{
	while ((NVM.STATUS & NVM_NVMBUSY_bm) == NVM_NVMBUSY_bm)
		;
}

/**************************************************************************************************
** Load data into the EEPROM page buffer. Max size is EEPROM_PAGE_SIZE (32 bytes).
**
** N.B. EEPROM memory mapping must be disabled for this function to work.
*/
void EEP_LoadPageBuffer(const uint8_t *data, uint8_t size)
{
	if (size > EEPROM_PAGE_SIZE)
		size = EEPROM_PAGE_SIZE;

	EEP_WaitForNVM();
	NVM.CMD = NVM_CMD_LOAD_EEPROM_BUFFER_gc;

	// Set address to zero, as only the lower bits matters. ADDR0 is maintained inside the loop below.
	NVM.ADDR1 = 0x00;
	NVM.ADDR2 = 0x00;

	// Load multiple bytes into page buffer
	for (uint8_t i = 0; i < size; ++i) {
		NVM.ADDR0 = i;
		NVM.DATA0 = *data++;
	}
}

/**************************************************************************************************
** Write EEPROM page buffer to EEPROM memory. EEPROM will be erased before writing. Only page
** buffer locations that have been written will be saved, others will remain untouched.
**
** page_addr should be between 0 and EEPROM_SIZE/EEPROM_PAGE_SIZE
**
** N.B. EEPROM memory mapping must be disabled for this function to work.
*/
void EEP_AtomicWritePage(uint8_t page_addr)
{
	EEP_WaitForNVM();

	// Calculate page address
	uint16_t address = (uint16_t)(page_addr*EEPROM_PAGE_SIZE);

	// Set address
	NVM.ADDR0 = address & 0xFF;
	NVM.ADDR1 = (address >> 8) & 0x1F;
	NVM.ADDR2 = 0x00;

	// Issue EEPROM Atomic Write (Erase&Write) command
	NVM.CMD = NVM_CMD_ERASE_WRITE_EEPROM_PAGE_gc;
	NVM_EXEC();
	
	EEP_WaitForNVM();
}

/**************************************************************************************************
** Erase the entire EEPROM.
**
** N.B. EEPROM memory mapping must be disabled for this function to work.
*/
void EEP_EraseAll(void)
{
	uint8_t blank[32] = {	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF	};

	EEP_LoadPageBuffer(blank, 32);		// this marks every byte in the page as needing an update, otherwise the EraseAll command doesn't work
	EEP_WaitForNVM();

	// Issue EEPROM Erase All command
	NVM.CMD = NVM_CMD_ERASE_EEPROM_gc;
	NVM_EXEC();
}

