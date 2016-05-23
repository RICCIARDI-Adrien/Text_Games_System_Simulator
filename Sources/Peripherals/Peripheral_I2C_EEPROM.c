/** @file Peripheral_I2C_EEPROM.c
 * @see Peripheral_I2C_EEPROM.h for description.
 * @author Adrien RICCIARDI
 */
#include <errno.h>
#include <Log.h>
#include <Peripheral_I2C_EEPROM.h>
#include <Register_File.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** The EEPROM bus address for a write operation. */
#define PERIPHERAL_I2C_EEPROM_WRITE_ADDRESS 0xA0
/** The EEPROM bus address for a read operation. */
#define PERIPHERAL_I2C_EEPROM_READ_ADDRESS 0xA1

/** The EEPROM memory size in bytes. */
#define PERIPHERAL_I2C_EEPROM_MEMORY_SIZE 4096
/** The EEPROM internal address register used bits. */
#define PERIPHERAL_I2C_EEPROM_ADDRESS_REGISTER_MASK 0x0FFF

//-------------------------------------------------------------------------------------------------
// Private types
//-------------------------------------------------------------------------------------------------
/** All EEPROM protocol states. */
typedef enum
{
	PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DEVICE_ADDRESS,
	PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DATA_HIGH_BYTE_ADDRESS,
	PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DATA_LOW_BYTE_ADDRESS,
	PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DATA_BYTE
} TPeripheralI2CEEPROMState;

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** The EEPROM content. */
static unsigned char Peripheral_I2C_EEPROM_Memory[PERIPHERAL_I2C_EEPROM_MEMORY_SIZE];
/** The EEPROM address register. */
static unsigned short Peripheral_I2C_EEPROM_Address_Register = 0;

/** The EEPROM internal state machine current state. */
static TPeripheralI2CEEPROMState Peripheral_I2C_EEPROM_State = PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DEVICE_ADDRESS;

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
int PeripheralI2CEEPROMInitialize(char *String_EEPROM_File)
{
	FILE *Pointer_File;
	int Return_Value = 1, Result;
	
	// Try to open the file
	Pointer_File = fopen(String_EEPROM_File, "rb"); // Must specify 'binary' mode on Windows
	if (Pointer_File == NULL)
	{
		LOG(LOG_LEVEL_ERROR, "Error : could not open the EEPROM file '%s' (%s).\n", String_EEPROM_File, strerror(errno));
		goto Exit;
	}
	
	// Read the file content
	Result = fread(Peripheral_I2C_EEPROM_Memory, 1, PERIPHERAL_I2C_EEPROM_MEMORY_SIZE, Pointer_File);
	if (ferror(Pointer_File))
	{
		LOG(LOG_LEVEL_ERROR, "Error : failed to read the EEPROM file (%s).\n", strerror(errno));
		goto Exit;
	}
	
	LOG(LOG_LEVEL_DEBUG, "EEPROM file successfully read (%d bytes).\n", Result);
	Return_Value = 0;
	
Exit:
	if (Pointer_File != NULL) fclose(Pointer_File); // Close the file if needed
	return Return_Value;
}

void PeripheralI2CEEPROMWriteSSPCON2(TRegisterFileRegisterContent *Pointer_Content, unsigned char Data)
{
	unsigned char PIR1_Register;
	
	// Start, Repeated Start and Stop conditions must set the I2C interrupt flag and be cleared by hardware
	if ((Data & REGISTER_FILE_REGISTER_BIT_SSPCON2_ACKEN) || (Data & REGISTER_FILE_REGISTER_BIT_SSPCON2_RCEN) || (Data & REGISTER_FILE_REGISTER_BIT_SSPCON2_PEN) || (Data & REGISTER_FILE_REGISTER_BIT_SSPCON2_RSEN) || (Data & REGISTER_FILE_REGISTER_BIT_SSPCON2_SEN))
	{
		if (Data & REGISTER_FILE_REGISTER_BIT_SSPCON2_ACKEN) LOG(LOG_LEVEL_DEBUG, "EEPROM sent (N)ACK.\n");
		if (Data & REGISTER_FILE_REGISTER_BIT_SSPCON2_RCEN) LOG(LOG_LEVEL_DEBUG, "EEPROM is in reception mode.\n");
		if (Data & REGISTER_FILE_REGISTER_BIT_SSPCON2_PEN)
		{
			Peripheral_I2C_EEPROM_State = PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DEVICE_ADDRESS;
			LOG(LOG_LEVEL_DEBUG, "EEPROM sent I2C Stop.\n");
		}
		if (Data & REGISTER_FILE_REGISTER_BIT_SSPCON2_RSEN)
		{
			Peripheral_I2C_EEPROM_State = PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DEVICE_ADDRESS;
			LOG(LOG_LEVEL_DEBUG, "EEPROM sent I2C Repeated Start.\n");
		}
		if (Data & REGISTER_FILE_REGISTER_BIT_SSPCON2_SEN) LOG(LOG_LEVEL_DEBUG, "EEPROM sent I2C Start.\n");
		
		// Set SSPIF flag to tell that the condition has been transmitted to the bus
		PIR1_Register = RegisterFileDirectReadFromCallback(REGISTER_FILE_REGISTER_BANK_PIR1, REGISTER_FILE_REGISTER_ADDRESS_PIR1);
		PIR1_Register |= REGISTER_FILE_REGISTER_BIT_PIR1_SSPIF;
		RegisterFileDirectWriteFromCallback(REGISTER_FILE_REGISTER_BANK_PIR1, REGISTER_FILE_REGISTER_ADDRESS_PIR1, PIR1_Register);
		
		// Clear the conditions (but RCEN that is handled only by the user firmware)
		Data &= ~(REGISTER_FILE_REGISTER_BIT_SSPCON2_ACKEN | REGISTER_FILE_REGISTER_BIT_SSPCON2_PEN | REGISTER_FILE_REGISTER_BIT_SSPCON2_RSEN | REGISTER_FILE_REGISTER_BIT_SSPCON2_SEN);
	}
	
	// Store the register value
	Pointer_Content->Data = Data;
}

void PeripheralI2CEEPROMWriteSSPBUF(TRegisterFileRegisterContent *Pointer_Content, unsigned char Data)
{
	unsigned char PIR1_Register;
	
	switch (Peripheral_I2C_EEPROM_State)
	{
		// The master sent the EEPROM device address and the operation type
		case PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DEVICE_ADDRESS:
			if (Data == PERIPHERAL_I2C_EEPROM_READ_ADDRESS)
			{
				// Write the corresponding memory cell value to SSPBUF so it will return this value when read
				Pointer_Content->Data = Peripheral_I2C_EEPROM_Memory[Peripheral_I2C_EEPROM_Address_Register];
				LOG(LOG_LEVEL_DEBUG, "EEPROM read value 0x%02X at current address 0x%04X.\n", Pointer_Content->Data, Peripheral_I2C_EEPROM_Address_Register);

				// EEPROM address register is auto-incrementing
				Peripheral_I2C_EEPROM_Address_Register++;
				if (Peripheral_I2C_EEPROM_Address_Register >= PERIPHERAL_I2C_EEPROM_MEMORY_SIZE) Peripheral_I2C_EEPROM_Address_Register = 0; // Handle wrap-around
			}
			else if (Data == PERIPHERAL_I2C_EEPROM_WRITE_ADDRESS)
			{
				Peripheral_I2C_EEPROM_State = PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DATA_HIGH_BYTE_ADDRESS;
				LOG(LOG_LEVEL_DEBUG, "EEPROM write operation.\n");
			}
			else LOG(LOG_LEVEL_WARNING, "Received and discarded a bad I2C address (0x%02X).\n", Data);
			break;
			
		// The master sent the high byte of the address to write to
		case PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DATA_HIGH_BYTE_ADDRESS:
			Peripheral_I2C_EEPROM_Address_Register = (Data << 8) & PERIPHERAL_I2C_EEPROM_ADDRESS_REGISTER_MASK;
			// Wait for the low byte
			Peripheral_I2C_EEPROM_State = PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DATA_LOW_BYTE_ADDRESS;
			LOG(LOG_LEVEL_DEBUG, "EEPROM received address high byte (0x%02X).\n", Data);
			break;
			
		// The master sent the address to write to low byte
		case PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DATA_LOW_BYTE_ADDRESS:
			Peripheral_I2C_EEPROM_Address_Register |= Data;
			// Wait for the data to write
			Peripheral_I2C_EEPROM_State = PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DATA_BYTE;
			LOG(LOG_LEVEL_DEBUG, "EEPROM received address low byte (0x%02X), EEPROM address register : 0x%04X.\n", Data, Peripheral_I2C_EEPROM_Address_Register);
			break;
			
		// The master sent the payload byte to write
		case PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DATA_BYTE:
			Peripheral_I2C_EEPROM_Memory[Peripheral_I2C_EEPROM_Address_Register] = Data;
			// The operation is finished
			Peripheral_I2C_EEPROM_State = PERIPHERAL_I2C_EEPROM_STATE_RECEIVE_DEVICE_ADDRESS;
			LOG(LOG_LEVEL_DEBUG, "EEPROM received data to write : 0x%02X.\n", Data);
			break;
			
		// Should never be reached
		default:
			LOG(LOG_LEVEL_ERROR, "ERROR : Inconsistent EEPROM state machine state (%d).\n", Peripheral_I2C_EEPROM_State);
			exit(EXIT_FAILURE);
	}
	
	// Set SSPIF flag to tell that the EEPROM access is terminated
	PIR1_Register = RegisterFileDirectReadFromCallback(REGISTER_FILE_REGISTER_BANK_PIR1, REGISTER_FILE_REGISTER_ADDRESS_PIR1);
	PIR1_Register |= REGISTER_FILE_REGISTER_BIT_PIR1_SSPIF;
	RegisterFileDirectWriteFromCallback(REGISTER_FILE_REGISTER_BANK_PIR1, REGISTER_FILE_REGISTER_ADDRESS_PIR1, PIR1_Register);
}
