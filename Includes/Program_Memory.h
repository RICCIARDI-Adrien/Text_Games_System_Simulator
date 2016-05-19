/** @file Program_Memory.h
 * PIC16F876 Flash program memory.
 * @author Adrien RICCIARDI
 */
#ifndef H_PROGRAM_MEMORY_H
#define H_PROGRAM_MEMORY_H

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** How many instructions can hold the program memory. */
#define PROGRAM_MEMORY_SIZE 8192

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Read a program memory location.
 * @return The 14-bit data.
 */
unsigned short ProgramMemoryRead(unsigned short Address);

/** Load an Intel Hex file content to the program memory.
 * @param String_Hex_File The file to load.
 * @return 0 if the file was successfully loaded,
 * @return 1 if an error occurred. See logs for more information.
 */
int ProgramMemoryLoadHexFile(char *String_Hex_File);

// TODO for flash access
// programread
// programwrite

#endif
