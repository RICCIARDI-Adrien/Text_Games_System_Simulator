/** @file Register_File.h
 * PIC16F876 register file and data RAM.
 * @author Adrien RICCIARDI
 */
#ifndef H_REGISTER_FILE_H
#define H_REGISTER_FILE_H

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** How many memory banks in the register file. */
#define REGISTER_FILE_BANKS_COUNT 4
/** How many registers in a bank. */
#define REGISTER_FILE_REGISTERS_IN_BANK_COUNT 128

// All register addresses (addresses are relative to the beginning of the register bank)
// TODO define missing ones when needed
#define REGISTER_FILE_REGISTER_ADDRESS_INDF 0x00 // Replicated in all other banks, special behavior
#define REGISTER_FILE_REGISTER_ADDRESS_PCL 0x02 // Replicated in all other banks
#define REGISTER_FILE_REGISTER_ADDRESS_STATUS 0x03 // Replicated in all other banks
#define REGISTER_FILE_REGISTER_ADDRESS_FSR 0x04 // Replicated in all other banks
#define REGISTER_FILE_REGISTER_ADDRESS_PCLATH 0x0A // Replicated in all other banks
#define REGISTER_FILE_REGISTER_ADDRESS_INTCON 0x0B // Replicated in all other banks
#define REGISTER_FILE_REGISTER_ADDRESS_PIR1 0x0C
#define REGISTER_FILE_REGISTER_ADDRESS_PIE1 0x0C
#define REGISTER_FILE_REGISTER_ADDRESS_TXSTA 0x18
#define REGISTER_FILE_REGISTER_ADDRESS_TXREG 0x19
#define REGISTER_FILE_REGISTER_ADDRESS_RCREG 0x1A

// All register banks
// TODO define missing ones when needed
#define REGISTER_FILE_REGISTER_BANK_STATUS 0 // The real data byte is stored in bank 0, the replicated registers all point to this bank
#define REGISTER_FILE_REGISTER_BANK_FSR 0 // The real data byte is stored in bank 0, the replicated registers all point to this bank
#define REGISTER_FILE_REGISTER_BANK_INTCON 0 // The real data byte is stored in bank 0, the replicated registers all point to this bank
#define REGISTER_FILE_REGISTER_BANK_PIR1 0
#define REGISTER_FILE_REGISTER_BANK_PIE1 1
#define REGISTER_FILE_REGISTER_BANK_TXSTA 1
#define REGISTER_FILE_REGISTER_BANK_TXREG 0
#define REGISTER_FILE_REGISTER_BANK_RCREG 0

/** STATUS register Register Bank Select bit. */
#define REGISTER_FILE_REGISTER_BIT_STATUS_IRP (1 << 7)
/** STATUS register Zero bit. */
#define REGISTER_FILE_REGISTER_BIT_STATUS_Z (1 << 2)
/** STATUS register Digit carry/borrow bit. */
#define REGISTER_FILE_REGISTER_BIT_STATUS_DC (1 << 1)
/** STATUS register Carry/borrow bit. */
#define REGISTER_FILE_REGISTER_BIT_STATUS_C (1 << 0)

/** INTCON register Global Interrupt Enable bit. */
#define REGISTER_FILE_REGISTER_BIT_INTCON_GIE (1 << 7)
/** INTCON register Peripheral Interrupt Enable bit. */
#define REGISTER_FILE_REGISTER_BIT_INTCON_PEIE (1 << 6)
/** INTCON register TMR0 Overflow Interrupt Enable bit. */
#define REGISTER_FILE_REGISTER_BIT_INTCON_T0IE (1 << 5)
/** INTCON register RB0/INT External Interrupt Enable bit. */
#define REGISTER_FILE_REGISTER_BIT_INTCON_INTE (1 << 4)
/** INTCON register RB Port Change Interrupt Enable bit. */
#define REGISTER_FILE_REGISTER_BIT_INTCON_RBIE (1 << 3)
/** INTCON register TMR0 Overflow Interrupt Flag bit. */
#define REGISTER_FILE_REGISTER_BIT_INTCON_T0IF (1 << 2)
/** INTCON register RB0/INT External Interrupt Flag bit. */
#define REGISTER_FILE_REGISTER_BIT_INTCON_INTF (1 << 1)
/** INTCON register RB Port Change Interrupt Flag bit. */
#define REGISTER_FILE_REGISTER_BIT_INTCON_RBIF (1 << 0)

/** PIR1 register USART Receive Interrupt Enable bit. */
#define REGISTER_FILE_REGISTER_BIT_PIR1_RCIF (1 << 5)
/** PIR1 register USART Transmit Interrupt Enable bit. */
#define REGISTER_FILE_REGISTER_BIT_PIR1_TXIF (1 << 4)

/** PIE1 register USART Receive Interrupt Enable bit. */
#define REGISTER_FILE_REGISTER_BIT_PIE1_RCIE (1 << 5)
/** PIE1 register USART Transmit Interrupt Enable bit. */
#define REGISTER_FILE_REGISTER_BIT_PIE1_TXIE (1 << 4)

/** TXSTA register Transmit Enable bit. */
#define REGISTER_FILE_REGISTER_BIT_TXSTA_TXEN (1 << 5)

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
typedef union
{
	unsigned char Data; //! Store a byte of data.
	unsigned char *Pointer_Data; //! Locate another data from another register.
} TRegisterFileRegisterContent;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure the register file and all Special Function Registers. */
void RegisterFileInitialize(void);

/** Read a byte from the specified address in the current bank.
 * @param Address The address bits 6..0, bits 8..7 are located in the STATUS register.
 * @return The read data.
 * @note This function is protected against concurrent access and can be used everywhere but in register callback functions.
 */
unsigned char RegisterFileBankedRead(unsigned int Address);

/** Write a byte of data to the specified address in the current bank.
 * @param Address The address bits 6..0, bits 8..7 are located in the STATUS register.
 * @param Data The data to write.
 * @note This function is protected against concurrent access and can be used everywhere but in register callback functions.
 */
void RegisterFileBankedWrite(unsigned int Address, unsigned char Data);

/** Read a byte from the specified address in the specified bank.
 * @param Bank The bank number.
 * @param Address The address to read from.
 * @return The read data.
 * @note This function is protected against concurrent access and can be used everywhere but in register callback functions.
 */
unsigned char RegisterFileDirectRead(unsigned int Bank, unsigned int Address);

/** Write a byte of data to the specified address in the specified bank.
 * @param Bank The bank number.
 * @param Address The address to write to.
 * @param Data The data to write.
 * @note This function is protected against concurrent access and can be used everywhere but in register callback functions.
 */
void RegisterFileDirectWrite(unsigned int Bank, unsigned int Address, unsigned char Data);

/** Read a byte from the specified address in the specified bank.
 * @param Bank The bank number.
 * @param Address The address to read from.
 * @return The read data.
 * @note This function is not protected against concurrent access and should be used only in register callback functions.
 */
unsigned char RegisterFileDirectReadFromCallback(unsigned int Bank, unsigned int Address);

/** Write a byte of data to the specified address in the specified bank.
 * @param Bank The bank number.
 * @param Address The address to write to.
 * @param Data The data to write.
 * @note This function is not protected against concurrent access and should be used only in register callback functions.
 */
void RegisterFileDirectWriteFromCallback(unsigned int Bank, unsigned int Address, unsigned char Data);

/** Dump the whole register file content. */
void RegisterFileDump(void);

/** Tell if an interrupt must be serviced or not.
 * @return 0 if no interrupt has fired,
 * @return 1 if the core must branch to the interrupt handler.
 */
int RegisterFileHasInterruptFired(void);

#endif
