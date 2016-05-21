/** @file Peripheral_UART.c
 * @see Peripheral_UART.h for description.
 * @author Adrien RICCIARDI
 */
#include <Log.h>
#include <Peripheral_UART.h>
#include <Register_File.h>
#include <stdio.h>

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
unsigned char PeripheralUARTReadRCREG(TRegisterFileRegisterContent *Pointer_Content)
{
	unsigned char PIR1_Register;
	
	// Clear interrupt flag
	PIR1_Register = RegisterFileDirectReadFromCallback(REGISTER_FILE_REGISTER_BANK_PIR1, REGISTER_FILE_REGISTER_ADDRESS_PIR1);
	PIR1_Register &= ~REGISTER_FILE_REGISTER_BIT_PIR1_RCIF;
	RegisterFileDirectWriteFromCallback(REGISTER_FILE_REGISTER_BANK_PIR1, REGISTER_FILE_REGISTER_ADDRESS_PIR1, PIR1_Register);
	
	return Pointer_Content->Data;
}

void PeripheralUARTWriteTXREG(TRegisterFileRegisterContent __attribute__((unused)) *Pointer_Content, unsigned char Data)
{
	// Display the transmitted data (no need to clear and set the TXIF flag because the write operation will block the instruction execution flow, just let TXIF set)
	putchar(Data);
	fflush(stdout);
}

void PeripheralUARTWriteTXSTA(TRegisterFileRegisterContent *Pointer_Content, unsigned char Data)
{
	unsigned char PIR1_Register;
	
	// Set TXIF flag if TXEN is set as specified in the datasheet
	if (Data & REGISTER_FILE_REGISTER_BIT_TXSTA_TXEN)
	{
		PIR1_Register = RegisterFileDirectReadFromCallback(REGISTER_FILE_REGISTER_BANK_PIR1, REGISTER_FILE_REGISTER_ADDRESS_PIR1);
		PIR1_Register |= REGISTER_FILE_REGISTER_BIT_PIR1_TXIF;
		RegisterFileDirectWriteFromCallback(REGISTER_FILE_REGISTER_BANK_PIR1, REGISTER_FILE_REGISTER_ADDRESS_PIR1, PIR1_Register);
	}
	
	Pointer_Content->Data = Data;
}

void PeripheralUARTReceiveByte(unsigned char Data)
{
	unsigned char PIR1_Register;
	
	LOG(LOG_LEVEL_DEBUG, "Received byte '0x%02X' from UART.\n", Data);
	
	// Fill RCREG register
	RegisterFileDirectWrite(REGISTER_FILE_REGISTER_BANK_RCREG, REGISTER_FILE_REGISTER_ADDRESS_RCREG, Data);
	
	// Set RCIF flag
	// TODO atomic access for the PIR register during read-modify-write operation
	PIR1_Register = RegisterFileDirectRead(REGISTER_FILE_REGISTER_BANK_PIR1, REGISTER_FILE_REGISTER_ADDRESS_PIR1);
	PIR1_Register |= REGISTER_FILE_REGISTER_BIT_PIR1_RCIF;
	RegisterFileDirectWrite(REGISTER_FILE_REGISTER_BANK_PIR1, REGISTER_FILE_REGISTER_ADDRESS_PIR1, PIR1_Register);
}
