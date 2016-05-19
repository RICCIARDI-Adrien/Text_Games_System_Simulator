/** @file Register_File.c
 * @see Register_File.h for description.
 * @author Adrien RICCIARDI
 */
#include <Log.h>
#include <Register_File.h>

//-------------------------------------------------------------------------------------------------
// Private types
//-------------------------------------------------------------------------------------------------
typedef union
{
	unsigned char Data; //! Store a byte of data.
	unsigned char *Pointer_Data; //! Locate another data from another register.
} TRegisterFileRegisterContent;

/** The callback called when a register is read.
 * @param Pointer_Content The register content according to its type.
 * @return The read value.
 */
typedef unsigned char (*TRegisterFileRegisterReadCallback)(TRegisterFileRegisterContent *Pointer_Content);

/** The callback called when a register is written.
 * @param Pointer_Content The register content according to its type.
 * @param Data The data to write.
 */
typedef void (*TRegisterFileRegisterWriteCallback)(TRegisterFileRegisterContent *Pointer_Content, unsigned char Data);

/** A register file entry. */
typedef struct
{
	TRegisterFileRegisterReadCallback ReadCallback; //! What to do when the register is read.
	TRegisterFileRegisterWriteCallback WriteCallback; //! What to do when the register is written.
	TRegisterFileRegisterContent Content; //! What the register may content.
} TRegisterFileRegister;

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** The register file split in banks. */
static TRegisterFileRegister Register_File[REGISTER_FILE_BANKS_COUNT][REGISTER_FILE_REGISTERS_IN_BANK_COUNT];

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Get the data stored at this register used as RAM storage.
 * @param Pointer_Content The register content.
 * @return The read data.
 */
static unsigned char RegisterFileNormalRAMRead(TRegisterFileRegisterContent *Pointer_Content)
{
	return Pointer_Content->Data;
}

/** Write a data to this register used as RAM storage.
 * @param Pointer_Content The register content.
 * @param Data The data to write.
 */
static void RegisterFileNormalRAMWrite(TRegisterFileRegisterContent *Pointer_Content, unsigned char Data)
{
	Pointer_Content->Data = Data;
}

/** Read another register data.
 * @param Pointer_Content The physical register to read data from.
 * @return The target register data.
 */
static unsigned char RegisterFileRemappedRAMRead(TRegisterFileRegisterContent *Pointer_Content)
{
	return *(Pointer_Content->Pointer_Data);
}

/** Write data to another register.
 * @param Pointer_Content The physical register to write data to.
 * @param Data The data to write.
 */
static void RegisterFileRemappedRAMWrite(TRegisterFileRegisterContent *Pointer_Content, unsigned char Data)
{
	*(Pointer_Content->Pointer_Data) = Data;
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void RegisterFileInitialize(void)
{
	int Bank, Address;
	
	// Start by considering all registers as simple and independent RAM locations
	for (Bank = 0; Bank < REGISTER_FILE_BANKS_COUNT; Bank++)
	{
		for (Address = 0; Address < REGISTER_FILE_REGISTERS_IN_BANK_COUNT; Address++)
		{
			Register_File[Bank][Address].ReadCallback = RegisterFileNormalRAMRead;
			Register_File[Bank][Address].WriteCallback = RegisterFileNormalRAMWrite;
			Register_File[Bank][Address].Content.Data = 0;
		}
	}
	
	//===============================================
	// Configure core registers
	//===============================================
	// Remap PCL register data from bank 1 to 3 to bank 0
	for (Bank = 1; Bank < REGISTER_FILE_BANKS_COUNT; Bank++)
	{
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_PCL].Content.Pointer_Data = &Register_File[0][REGISTER_FILE_REGISTER_ADDRESS_PCL].Content.Data;
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_PCL].ReadCallback = RegisterFileRemappedRAMRead;
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_PCL].WriteCallback = RegisterFileRemappedRAMWrite;
	}
	
	// Set STATUS initial value
	Register_File[0][REGISTER_FILE_REGISTER_ADDRESS_STATUS].Content.Data = 0x18;
	// Remap STATUS register data from bank 1 to 3 to bank 0
	for (Bank = 1; Bank < REGISTER_FILE_BANKS_COUNT; Bank++)
	{
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_STATUS].Content.Pointer_Data = &Register_File[0][REGISTER_FILE_REGISTER_ADDRESS_STATUS].Content.Data;
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_STATUS].ReadCallback = RegisterFileRemappedRAMRead;
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_STATUS].WriteCallback = RegisterFileRemappedRAMWrite;
	}
	
	// Remap FSR register data from bank 1 to 3 to bank 0
	for (Bank = 1; Bank < REGISTER_FILE_BANKS_COUNT; Bank++)
	{
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_FSR].Content.Pointer_Data = &Register_File[0][REGISTER_FILE_REGISTER_ADDRESS_FSR].Content.Data;
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_FSR].ReadCallback = RegisterFileRemappedRAMRead;
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_FSR].WriteCallback = RegisterFileRemappedRAMWrite;
	}
	
	// Remap PCLATH register data from bank 1 to 3 to bank 0
	for (Bank = 1; Bank < REGISTER_FILE_BANKS_COUNT; Bank++)
	{
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_PCLATH].Content.Pointer_Data = &Register_File[0][REGISTER_FILE_REGISTER_ADDRESS_PCLATH].Content.Data;
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_PCLATH].ReadCallback = RegisterFileRemappedRAMRead;
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_PCLATH].WriteCallback = RegisterFileRemappedRAMWrite;
	}
	
	// Remap INTCON register data from bank 1 to 3 to bank 0
	for (Bank = 1; Bank < REGISTER_FILE_BANKS_COUNT; Bank++)
	{
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_INTCON].Content.Pointer_Data = &Register_File[0][REGISTER_FILE_REGISTER_ADDRESS_INTCON].Content.Data;
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_INTCON].ReadCallback = RegisterFileRemappedRAMRead;
		Register_File[Bank][REGISTER_FILE_REGISTER_ADDRESS_INTCON].WriteCallback = RegisterFileRemappedRAMWrite;
	}
	
	//===============================================
	// Configure remapped data access located at banks end
	//===============================================
	for (Address = 0x70; Address < 0x80; Address++)
	{
		// Remap register data from bank 1 to 3 to bank 0
		for (Bank = 1; Bank < REGISTER_FILE_BANKS_COUNT; Bank++)
		{
			Register_File[Bank][Address].Content.Pointer_Data = &Register_File[0][Address].Content.Data;
			Register_File[Bank][Address].ReadCallback = RegisterFileRemappedRAMRead;
			Register_File[Bank][Address].WriteCallback = RegisterFileRemappedRAMWrite;
		}
	}
	
	// TODO fill needed peripheral special registers
}

unsigned char RegisterFileRead(unsigned char Address)
{
	int Current_Bank;
	TRegisterFileRegister *Pointer_Register;
	
	// Get the current bank number from STATUS register
	Current_Bank = ((Register_File[0][REGISTER_FILE_REGISTER_ADDRESS_STATUS].Content.Data) >> 5) & 0x03;
	
	// Get the required register
	Pointer_Register = &Register_File[Current_Bank][Address];
		
	return Pointer_Register->ReadCallback(&Pointer_Register->Content);
}

void RegisterFileWrite(unsigned char Address, unsigned char Data)
{
	int Current_Bank;
	TRegisterFileRegister *Pointer_Register;
	
	// Get the current bank number from STATUS register
	Current_Bank = ((Register_File[0][REGISTER_FILE_REGISTER_ADDRESS_STATUS].Content.Data) >> 5) & 0x03;
	
	// Get the required register
	Pointer_Register = &Register_File[Current_Bank][Address];
	
	Pointer_Register->WriteCallback(&Pointer_Register->Content, Data);
}

void RegisterFileDump(void)
{
	int Address;
	
	LOG(LOG_LEVEL_DEBUG, "Address | Bank 0 | Bank 1 | Bank 2 | Bank 3\n");
	LOG(LOG_LEVEL_DEBUG, "--------+--------+--------+--------+--------\n");
	for (Address = 0; Address < REGISTER_FILE_REGISTERS_IN_BANK_COUNT; Address++) LOG(LOG_LEVEL_DEBUG, "0x%02X    |  0x%02X  |  0x%02X  |  0x%02X  |  0x%02X\n", Address, Register_File[0][Address].ReadCallback(&Register_File[0][Address].Content), Register_File[1][Address].ReadCallback(&Register_File[1][Address].Content), Register_File[2][Address].ReadCallback(&Register_File[2][Address].Content), Register_File[3][Address].ReadCallback(&Register_File[3][Address].Content));
}

int RegisterFileHasInterruptFired(void)
{
        unsigned char INTCON_Register, PIE1_Register, PIR1_Register;

        // Check flags not depending from PIE bit
        INTCON_Register = Register_File[REGISTER_FILE_REGISTER_BANK_INTCON][REGISTER_FILE_REGISTER_ADDRESS_INTCON].Content.Data;
        if (!(INTCON_Register & REGISTER_FILE_REGISTER_BIT_INTCON_GIE)) return 0; // Interrupts are disabled
        // T0I
        if (INTCON_Register & (REGISTER_FILE_REGISTER_BIT_INTCON_T0IE | REGISTER_FILE_REGISTER_BIT_INTCON_T0IF)) return 1;
        // INT
        if (INTCON_Register & (REGISTER_FILE_REGISTER_BIT_INTCON_INTE | REGISTER_FILE_REGISTER_BIT_INTCON_INTF)) return 1;
        // RBI
        if (INTCON_Register & (REGISTER_FILE_REGISTER_BIT_INTCON_RBIE | REGISTER_FILE_REGISTER_BIT_INTCON_RBIF)) return 1;

        // Check peripheral flags
        if (!(INTCON_Register & REGISTER_FILE_REGISTER_BIT_INTCON_PEIE)) return 0; // Peripheral interrupts are disabled
        PIE1_Register = Register_File[REGISTER_FILE_REGISTER_BANK_PIE1][REGISTER_FILE_REGISTER_ADDRESS_PIE1].Content.Data;
	PIR1_Register = Register_File[REGISTER_FILE_REGISTER_BANK_PIR1][REGISTER_FILE_REGISTER_ADDRESS_PIR1].Content.Data;
	// RCI
	if ((PIE1_Register & REGISTER_FILE_REGISTER_BIT_PIE1_RCIE) && (PIR1_Register & REGISTER_FILE_REGISTER_BIT_PIR1_RCIF)) return 1;
	// TXI
	if ((PIE1_Register & REGISTER_FILE_REGISTER_BIT_PIE1_TXIE) && (PIR1_Register & REGISTER_FILE_REGISTER_BIT_PIR1_TXIF)) return 1;

        // TODO implement other needed peripherals

	return 0;
}

