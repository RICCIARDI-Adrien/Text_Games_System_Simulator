/** @file Program_Memory.c
 * @see Program_Memory.h for description.
 * @author Adrien RICCIARDI
 */
#include <Log.h>
#include <Program_Memory.h>

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** The program memory. */
static unsigned short Program_Memory[PROGRAM_MEMORY_SIZE]
// TEST
= { 0x2037 };
//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
unsigned short ProgramMemoryRead(unsigned short Address)
{
	if (Address >= PROGRAM_MEMORY_SIZE)
	{
		LOG(LOG_LEVEL_WARNING, "WARNING : the requested address (0x%04X) is out of program memory bounds.\n", Address);
		return 0x3FFF; // Empty flash location
	}
	
	return Program_Memory[Address];
}