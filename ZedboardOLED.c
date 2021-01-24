/* 	ZedboardOLED.c
 * 	###############################################################################
 *
 *	Simple driver for the 128x32 OLED on the Zedboard
 *
 *
 *	The presence of the ZedboardOLED_v1_0 IP code in the PL-side of the Zynq chip
 *	is required to make this driver work.
 *
 *
 *	Texas A&M Qatar invests efforts and resources providing this open source code,
 *	All text above, and the splash screen below must be included in any redistribution
 *	this driver is subjected to BSD license, check license.txt for more information
 *	Written by Ali Aljaani/ Texas A&M Qatar University.
 *
 * 	###############################################################################
 * 	v1.0 -- 08/15/2014
 *  Eng. Ali Aljaani
 *  Company: TAMUQ University
 * 	###############################################################################
 *
 * 	Edited by Karl Janson in 04/19/2017:
 * 	1. Added definition of write_reg so that the driver can be used from Linux
 *  2. Code cleanup
 *
 * 	*/

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include "ZedboardOLED.h"

extern char displayBuffer[16];

/**
 * Writes a value to a register at baseAddress + offset 
 * @param baseAddress   Base address of the device to access
 * @param offset        Offset of the register to be accessed
 * @param value         Value to be written to the register
 */
void write_reg(void *baseAddress, unsigned short offset,  uint32_t value)
{
    *((uint32_t *)(baseAddress + offset)) = value;
}

/**
 * Reads a value from a register at baseAddress + offset 
 * @param baseAddress   Base address of the device to access
 * @param offset        Offset of the register to be accessed
 * 
 * @return              Value stored in the register
 */
uint32_t read_reg(void *baseAddress, unsigned short offset, uint32_t value)
{
    return *((uint32_t *)(baseAddress + offset));
}

/*****************************************************************************/
/**
*
* clears the screen, example: clean();
*
*
* @param	void *oledBaseAddr: base address for the OLED IP
*
* @return	none.
******************************************************************************/
void oledClear(void *oledBaseAddr)
{

	// Clear the buffer
	memset(displayBuffer, 0, sizeof(displayBuffer));

	oledUpdate(oledBaseAddr);
}

/*****************************************************************************/
/**
*
* Updates the OLED display
*
* @param	void *oledBaseAddr: base address for the OLED IP
*
******************************************************************************/

void oledUpdate(void *oledBaseAddr)
{
	// Send the updated databuffer to the display
	memcpy(oledBaseAddr, displayBuffer, sizeof(displayBuffer));

	// Update the display
	write_reg(oledBaseAddr, OLED_UPDATE_BIT, 1);
	usleep(SLEEP_TIME_US);

	write_reg(oledBaseAddr, OLED_UPDATE_BIT, 0);
	usleep(SLEEP_TIME_US);
}

/*****************************************************************************/
/**
*
* prints a character on the OLED at the page and the position specified by the second
* and third arguments,example print_char('A',0,0);
*
* @param	char character:  the character to be printed.
*
* @param	int row(0-3): the OLED is divided into 4 pages numbers, 0 is the upper
*			3 is the lower.
* @param	int position(0-15): each row can hold 16 characters
* 			0 is the leftmost: 15 is the rightmost
* @param	void *oledBaseAddr: base address for the OLED IP
*
* @return	int , 1 on success , 0 on failure.

******************************************************************************/
int oledPrintChar(char character, int row, int pos, void *oledBaseAddr)
{

	if (pos > 15)
	{
		printf("Position %d - Invalid position, position should be between (0-15).\n", pos);
		return 1;
	}

	if (row > 3)
	{
		printf("Row %d - Invalid row, row should be between (0-3).\n", row);
		return 1;
	}

	int offset = row * OLED_LINE_LENGTH;
	int regPos = (pos % REG_LENGTH) * CHAR_LENGTH_BITS;

	// Write the character into the buffer. Dispaly is accessed using 16 32-bit registers,
	// each register contains 4 8-bit characters
	displayBuffer[((pos - (pos % REG_LENGTH)) + offset) / REG_LENGTH] |= (character << regPos);

	return 0;
}

/*****************************************************************************/
/**
*
* prints a string of characters on the OLED at the page specified by the second
* argument, maximum string per page =16,example: print_char("Texas A&M Qatar,0);
*
* @param	char *string , the string message to be printed , maximum 16 letters.
*
* @param	int row(0-3) , the OLED is divided into 4 pages numbers, 0 is the upper
*			3 is the lower.
* @param	void *oledBaseAddr: base address for the OLED IP
*
* @return	int , 1 on success , 0 on failure.
*
******************************************************************************/
int oledPrintString(char *string, int row, void *oledBaseAddr)
{
	char *charPointer;
	charPointer = string;
	int ln = strlen(string);

	for (int i = 0; i < ln; ++i)
	{
		oledPrintChar(*charPointer, row, i, oledBaseAddr);
		charPointer++;
	}

	oledUpdate(oledBaseAddr);

	return 0;
}