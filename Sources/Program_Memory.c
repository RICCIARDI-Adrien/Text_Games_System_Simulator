/** @file Program_Memory.c
 * @see Program_Memory.h for description.
 * @author Adrien RICCIARDI
 */
#include <Hex_Parser.h>
#include <Log.h>
#include <Program_Memory.h>
#include <stdio.h>

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** The program memory. */
static unsigned short Program_Memory[PROGRAM_MEMORY_SIZE];

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

int ProgramMemoryLoadHexFile(char *String_Hex_File)
{
	FILE *Pointer_File;
	char String_Line[256];
	int Instructions_Count, i, Return_Value = 1;
	THexParserInstruction Instructions[HEX_PARSER_MAXIMUM_INSTRUCTIONS_PER_LINE];

	// Try to open the file
	Pointer_File = fopen(String_Hex_File, "r");
	if (Pointer_File == NULL)
	{
		LOG(LOG_LEVEL_ERROR, "ERROR : failed to open the file '%s'.\n", String_Hex_File);
		goto Exit;
	}
	LOG(LOG_LEVEL_DEBUG, "Loading '%s' hex file content...\n", String_Hex_File);

	// Read the file data and convert it to binary instructions
	while (1)
	{
		// Get the next hex record
		if (fgets(String_Line, sizeof(String_Line), Pointer_File) == NULL)
		{
			LOG(LOG_LEVEL_ERROR, "ERROR : reached the hex file end without finding an end-of-file record.\n");
			goto Exit;
		}
		LOG(LOG_LEVEL_DEBUG, "Read hex record : %s", String_Line); // No need to append a newline character because the read line already has it

		// Convert it to binary
		Instructions_Count = HexParserDecodeLine(String_Line, Instructions);
		LOG(LOG_LEVEL_DEBUG, "Found %d instructions in record.\n", Instructions_Count);

		// Process all instructions
		for (i = 0; i < Instructions_Count; i++)
		{
			// Is the end of the file reached ?
			if (Instructions[i].Is_End_Of_File)
			{
				LOG(LOG_LEVEL_DEBUG, "Hex file successfully loaded.\n");
				Return_Value = 0;
				goto Exit;
			}

			// Is the instruction valid ?
			if (!Instructions[i].Is_Instruction_Valid) continue;
			
			// Discard the configuration word
			if (Instructions[i].Address == 0x2007) continue;
			
			// Does the instruction address fit in the memory ?
			if (Instructions[i].Address >= PROGRAM_MEMORY_SIZE)
			{
				LOG(LOG_LEVEL_ERROR, "ERROR : the instruction address (0x%04X) is crossing the program memory bounds.\n", Instructions[i].Address);
				goto Exit;
			}

			// Store the instruction in the memory
			Program_Memory[Instructions[i].Address] = Instructions[i].Code;
		}
	}

Exit:
	if (Pointer_File != NULL) fclose(Pointer_File);
	return Return_Value;
}
