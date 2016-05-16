/** @file Core.c
 * @see Core.h for description.
 * @author Adrien RICCIARDI
 */
#include <Core.h>
#include <Log.h>
#include <Program_Memory.h>
#include <Register_File.h>

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** The recursive internal stack. */
static unsigned short Core_Stack[CORE_STACK_SIZE];
/** The stack pointer. */
static int Core_Stack_Pointer = 0;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Add data on the stack top. Recurse on stack overflow.
 * @param Data The data to push.
 */
static void CoreStackPush(unsigned short Data)
{
	if (Core_Stack_Pointer >= CORE_STACK_SIZE)
	{
		LOG(LOG_LEVEL_WARNING, "WARNING : stack overflow detected.\n");
		Core_Stack_Pointer = 0;
	}
	
	Core_Stack[Core_Stack_Pointer] = Data;
	Core_Stack_Pointer++;
}

/** Remove data from the stack top. Recurse on stack underflow.
 * @return The popped data.
 */
static unsigned short CoreStackPop(void)
{
	if (Core_Stack_Pointer <= 0)
	{
		LOG(LOG_LEVEL_WARNING, "WARNING : stack underflow detected.\n");
		Core_Stack_Pointer = CORE_STACK_SIZE - 1;
	}
	
	Core_Stack_Pointer--;
	return Core_Stack[Core_Stack_Pointer];
}

/** Fetch the instruction pointed by the program counter.
 * @param Pointer_Instruction On output, contain the fetched instruction.
 * @param Pointer_Program_Counter On output, contain the Program Counter current value.
 */
static void CoreFetchNextInstruction(unsigned short *Pointer_Instruction, unsigned short *Pointer_Program_Counter)
{
	unsigned short Program_Counter, Instruction;
	
	// Retrieve the program counter value
	Program_Counter = ((RegisterFileRead(REGISTER_FILE_REGISTER_ADDRESS_PCLATH) & 0x1F) << 8) | RegisterFileRead(REGISTER_FILE_REGISTER_ADDRESS_PCL);
	// Retrieve the instruction
	Instruction = ProgramMemoryRead(Program_Counter);
	
	*Pointer_Instruction = Instruction;
	*Pointer_Program_Counter = Program_Counter;
}

/** Write the new Program Counter value.
 * @param Program_Counter The updated Program Counter value to write to the register file.
 */
static void CoreWriteBack(unsigned short Program_Counter)
{
	RegisterFileWrite(REGISTER_FILE_REGISTER_ADDRESS_PCLATH, (Program_Counter >> 8) & 0x1F); // Bits 7..5 must be read as zero
	RegisterFileWrite(REGISTER_FILE_REGISTER_ADDRESS_PCL, (unsigned char) Program_Counter);
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void CoreExecuteNextInstruction(void)
{
	int Opcode;
	unsigned short Instruction, Program_Counter;
	
	// Fetch the next instruction
	CoreFetchNextInstruction(&Instruction, &Program_Counter);
	LOG(LOG_LEVEL_DEBUG, "Fetched next instruction at PC = 0x%04X, instruction : 0x%04X.\n", Program_Counter, Instruction);
	
	// Decode the instruction
	// Is is a CALL or GOTO instruction ?
	switch ((Instruction >> 11) & 0x07)
	{
		// CALL
		case 0x04:
			// Push the Program Counter return value (PC + 1)
			CoreStackPush(Program_Counter + 1);
			// Compute the new Program_Counter value
			Program_Counter &= 0x7FF; // Clear the 11 bits that will be replaced by the instruction content
			Program_Counter |= (Instruction & 0x7FF);
			LOG(LOG_LEVEL_DEBUG, "Found CALL instruction.\n");
			goto Exit;
		
		// GOTO
		case 0x05:
			goto Exit;
	}
	
	// Is it a
	
	Exit:
		// Save the new Program Counter value
		CoreWriteBack(Program_Counter);
		LOG(LOG_LEVEL_DEBUG, "Finished instruction execution, new Program Counter value is : 0x%04X.\n", Program_Counter);
}