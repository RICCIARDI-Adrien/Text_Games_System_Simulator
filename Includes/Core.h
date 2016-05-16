/** @file Core.h
 * The PIC16F876 ALU, prefetch and decoding units, and stack.
 * @author Adrien RICCIARDI
 */
#ifndef H_CORE_H
#define H_CORE_H

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** How many levels the recursive internal stack has. */
#define CORE_STACK_SIZE 8

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Decode and execute the next instruction. All needed register file registers will be accordingly modified. */
void CoreExecuteNextInstruction(void);

#endif