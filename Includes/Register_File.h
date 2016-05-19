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

// All registers (addresses are relative to the beginning of the register bank)
// TODO define missing ones when needed
#define REGISTER_FILE_REGISTER_ADDRESS_PCL 0x02 // Replicated in all other banks
#define REGISTER_FILE_REGISTER_ADDRESS_STATUS 0x03 // Replicated in all other banks
#define REGISTER_FILE_REGISTER_ADDRESS_FSR 0x04 // Replicated in all other banks
#define REGISTER_FILE_REGISTER_ADDRESS_PCLATH 0x0A // Replicated in all other banks
#define REGISTER_FILE_REGISTER_ADDRESS_INTCON 0x0B // Replicated in all other banks

/** STATUS register Carry flag. */
#define REGISTER_FILE_REGISTER_BIT_STATUS_CARRY (1 << 0)
/** STATUS register Digit Carry flag. */
#define REGISTER_FILE_REGISTER_BIT_STATUS_DIGIT_CARRY (1 << 1)
/** STATUS register Zero flag. */
#define REGISTER_FILE_REGISTER_BIT_STATUS_ZERO (1 << 2)

/** INTCON register Global Interrupt Enable bit. */
#define REGISTER_FILE_REGISTER_BIT_INTCON_GLOBAL_INTERRUPT_ENABLE (1 << 7)

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure the register file and all Special Function Registers. */
void RegisterFileInitialize(void);

/** Read a byte from the specified address.
 * @param Address The address bits 7..0, bits 9..8 are located in the STATUS register.
 * @return The read data.
 */
unsigned char RegisterFileRead(unsigned char Address);

/** Write a byte of data to the specified address.
 * @param Address The address bits 7..0, bits 9..8 are located in the STATUS register.
 * @param Data The data to write.
 */
void RegisterFileWrite(unsigned char Address, unsigned char Data);

/** Dump the whole register file content. */
void RegisterFileDump(void);

// TODO read & write PC ?

#endif