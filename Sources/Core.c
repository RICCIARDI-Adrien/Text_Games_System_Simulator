/** @file Core.c
 * @see Core.h for description.
 * @author Adrien RICCIARDI
 */
#include <Core.h>
#include <Log.h>
#include <Program_Memory.h>
#include <Register_File.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** The Carry flag was affected by the last operation. */
#define CORE_AFFECTED_FLAG_CARRY (1 << 0)
/** The Digit Carry flag was affected by the last operation. */
#define CORE_AFFECTED_FLAG_DIGIT_CARRY (1 << 1)
/** The Zero flag was affected by the last operation. */
#define CORE_AFFECTED_FLAG_ZERO (1 << 2)

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** The recursive internal stack. */
static unsigned short Core_Stack[CORE_STACK_SIZE];
/** The stack pointer. */
static int Core_Stack_Pointer = 0;

/** The working register. */
static unsigned char Core_Register_W;

/** The program counter. */
// TODO update program counter if PCL register is explicitly written
static unsigned short Core_Program_Counter = 0;

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

/** Update the STATUS register flags according to the result of an operation.
 * @param Operation_Result The operation result, stored on more bits than the real operands size to allow carry report detection.
 * @param Affected_Flags_Bitmask Tell which flags should be affected by the operation (use the constants from the CORE_AFFECTED_FLAG_* pool).
 */
static void CoreUpdateStatusRegister(unsigned short Operation_Result, int Affected_Flags_Bitmask)
{
	unsigned char Status_Register;
	
	// Get the current STATUS register value
	Status_Register = RegisterFileBankedRead(REGISTER_FILE_REGISTER_ADDRESS_STATUS);
	LOG(LOG_LEVEL_DEBUG, "Current STATUS value : 0x%02X.\n", Status_Register);
	
	// Check for carry report
	if (Affected_Flags_Bitmask & CORE_AFFECTED_FLAG_CARRY)
	{
		if (Operation_Result & 0x0100) Status_Register |= REGISTER_FILE_REGISTER_BIT_STATUS_C;
		else Status_Register &= ~REGISTER_FILE_REGISTER_BIT_STATUS_C;
	}
	
	// Check for digit carry report
	if (Affected_Flags_Bitmask & CORE_AFFECTED_FLAG_DIGIT_CARRY)
	{
		if (Operation_Result & 0x0010) Status_Register |= REGISTER_FILE_REGISTER_BIT_STATUS_DC;
		else Status_Register &= ~REGISTER_FILE_REGISTER_BIT_STATUS_DC;
	}
	
	// Check for the Zero flag
	if (Affected_Flags_Bitmask & CORE_AFFECTED_FLAG_ZERO)
	{
		if ((unsigned char) Operation_Result == 0) Status_Register |= REGISTER_FILE_REGISTER_BIT_STATUS_Z;
		else Status_Register &= ~REGISTER_FILE_REGISTER_BIT_STATUS_Z;
	}
	
	// Update the STATUS register value
	RegisterFileBankedWrite(REGISTER_FILE_REGISTER_ADDRESS_STATUS, Status_Register);
	LOG(LOG_LEVEL_DEBUG, "New STATUS value : 0x%02X.\n", Status_Register);
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void CoreExecuteNextInstruction(void)
{
	unsigned char Byte_Operand_1, Byte_Operand_2, Temp_Byte, Current_Carry_Value, New_Carry_Value;
	unsigned short Instruction, Temp_Word, Word_Operand;
	
	// Fetch the next instruction
	Instruction = ProgramMemoryRead(Core_Program_Counter);
	LOG(LOG_LEVEL_DEBUG, "Fetched next instruction at PC = 0x%04X, instruction : 0x%04X.\n", Core_Program_Counter, Instruction);
	
	// Decode and execute the instruction
	
	// No operand instruction format (must be executed before MOVWF as this instruction also starts by 0)
	switch (Instruction & 0x3FFF)
	{
		// NOP
		case 0x0000:
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : NOP.\n");
			goto Exit;
			
		// RETURN
		case 0x0008:
			// Pop the return address
			Core_Program_Counter = CoreStackPop();
			LOG(LOG_LEVEL_DEBUG, "Found instruction : RETURN.\n");
			goto Exit;
			
		// RETFIE
		case 0x0009:
			// Set the INTCON Global Interrupt Enable flag
			Temp_Byte = RegisterFileBankedRead(REGISTER_FILE_REGISTER_ADDRESS_INTCON); // Get the current INTCON value
			Temp_Byte |= REGISTER_FILE_REGISTER_BIT_INTCON_GIE; // Set GIE bit
			RegisterFileBankedWrite(REGISTER_FILE_REGISTER_ADDRESS_INTCON, Temp_Byte); // Set the new INTCON value
			// Pop the return address
			Core_Program_Counter = CoreStackPop();
			LOG(LOG_LEVEL_DEBUG, "Found instruction : RETFIE.\n");
			goto Exit;
			
		// SLEEP
		case 0x0063:
			// TODO if useful
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : SLEEP (NOT IMPLEMENTED).\n");
			goto Exit;
		
		// CLRWDT
		case 0x0064:
			// TODO if useful
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : CLRWDT (NOT IMPLEMENTED).\n");
			goto Exit;
			
		// CLRW
		case 0x0100:
			// Do the operation
			Core_Register_W = 0;
			// Handle STATUS flags
			CoreUpdateStatusRegister(0, CORE_AFFECTED_FLAG_ZERO);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : CLRW.\n");
			goto Exit;
	}
	
	// One 3-bit operand followed by one 7-bit operand instruction format
	Byte_Operand_1 = (unsigned char) ((Instruction >> 7) & 0x0007); // Get the first operand
	Byte_Operand_2 = (unsigned char) (Instruction & 0x007F); // Get the second operand
	switch ((Instruction >> 10) & 0x000F)
	{
		// BCF
		case 0x04:
			// Get the current register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Clear the bit
			Temp_Byte &= ~(1 << Byte_Operand_1);
			// Set the new register value
			RegisterFileBankedWrite(Byte_Operand_2, Temp_Byte);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : BCF 0x%02X, %d.\n", Byte_Operand_2, Byte_Operand_1);
			goto Exit;
			
		// BSF
		case 0x05:
			// Get the current register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Set the bit
			Temp_Byte |= 1 << Byte_Operand_1;
			// Set the new register value
			RegisterFileBankedWrite(Byte_Operand_2, Temp_Byte);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : BSF 0x%02X, %d.\n", Byte_Operand_2, Byte_Operand_1);
			goto Exit;
			
		// BTFSC
		case 0x06:
			// Get the register to test value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Skip next instruction if the requested bit is clear
			if (!(Temp_Byte & (1 << Byte_Operand_1))) Core_Program_Counter += 2;
			else Core_Program_Counter++; // Point on next instruction
			LOG(LOG_LEVEL_DEBUG, "Found instruction : BTFSC 0x%02X, %d.\n", Byte_Operand_2, Byte_Operand_1);
			goto Exit;
			
		// BTFSS
		case 0x07:
			// Get the register to test value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Skip next instruction if the requested bit is set
			if (Temp_Byte & (1 << Byte_Operand_1)) Core_Program_Counter += 2;
			else Core_Program_Counter++; // Point on next instruction
			LOG(LOG_LEVEL_DEBUG, "Found instruction : BTFSS 0x%02X, %d.\n", Byte_Operand_2, Byte_Operand_1);
			goto Exit;
	}
	
	// One 1-bit operand followed by one 7-bit operand instruction format
	Byte_Operand_1 = (unsigned char) ((Instruction >> 7) & 0x0001); // Get the first operand
	Byte_Operand_2 = (unsigned char) (Instruction & 0x007F); // Get the second operand
	switch ((Instruction >> 8) & 0x003F)
	{
		// MOVWF
		case 0x00:
			// Do the operation
			RegisterFileBankedWrite(Byte_Operand_2, Core_Register_W);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : MOVWF 0x%02X.\n", Byte_Operand_2);
			goto Exit;
			
		// CLRF
		case 0x01:
			// Do the operation
			RegisterFileBankedWrite(Byte_Operand_2, 0);
			// Handle STATUS flags
			CoreUpdateStatusRegister(0, CORE_AFFECTED_FLAG_ZERO);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : CLRF 0x%02X.\n", Byte_Operand_2);
			goto Exit;
			
		// SUBWF
		case 0x02:
			// Get the operand register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Do the operation
			Temp_Word = Temp_Byte - Core_Register_W;
			// Handle STATUS flags
			CoreUpdateStatusRegister(Temp_Word, CORE_AFFECTED_FLAG_CARRY | CORE_AFFECTED_FLAG_DIGIT_CARRY | CORE_AFFECTED_FLAG_ZERO);
			// Update the right destination register
			if (Byte_Operand_1 == 0) Core_Register_W = (unsigned char) Temp_Word;
			else RegisterFileBankedWrite(Byte_Operand_2, (unsigned char) Temp_Word);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : SUBWF 0x%02X, %c.\n", Byte_Operand_2, Byte_Operand_1 == 0 ? 'W' : 'F');
			goto Exit;
			
		// DECF
		case 0x03:
			// Get the operand register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Do the operation
			Temp_Byte--;
			// Handle STATUS flags
			CoreUpdateStatusRegister(Temp_Byte, CORE_AFFECTED_FLAG_ZERO);
			// Update the right destination register
			if (Byte_Operand_1 == 0) Core_Register_W = Temp_Byte;
			else RegisterFileBankedWrite(Byte_Operand_2, Temp_Byte);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : DECF 0x%02X, %c.\n", Byte_Operand_2, Byte_Operand_1 == 0 ? 'W' : 'F');
			goto Exit;
			
		// IORWF
		case 0x04:
			// Get the operand register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Do the operation
			Temp_Byte |= Core_Register_W;
			// Handle STATUS flags
			CoreUpdateStatusRegister(Temp_Byte, CORE_AFFECTED_FLAG_ZERO);
			// Update the right destination register
			if (Byte_Operand_1 == 0) Core_Register_W = Temp_Byte;
			else RegisterFileBankedWrite(Byte_Operand_2, Temp_Byte);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : IORWF 0x%02X, %c.\n", Byte_Operand_2, Byte_Operand_1 == 0 ? 'W' : 'F');
			goto Exit;
			
		// ANDWF
		case 0x05:
			// Get the operand register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Do the operation
			Temp_Byte &= Core_Register_W;
			// Handle STATUS flags
			CoreUpdateStatusRegister(Temp_Byte, CORE_AFFECTED_FLAG_ZERO);
			// Update the right destination register
			if (Byte_Operand_1 == 0) Core_Register_W = Temp_Byte;
			else RegisterFileBankedWrite(Byte_Operand_2, Temp_Byte);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : ANDWF 0x%02X, %c.\n", Byte_Operand_2, Byte_Operand_1 == 0 ? 'W' : 'F');
			goto Exit;
			
		// XORWF
		case 0x06:
			// Get the operand register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Do the operation
			Temp_Byte ^= Core_Register_W;
			// Handle STATUS flags
			CoreUpdateStatusRegister(Temp_Byte, CORE_AFFECTED_FLAG_ZERO);
			// Update the right destination register
			if (Byte_Operand_1 == 0) Core_Register_W = Temp_Byte;
			else RegisterFileBankedWrite(Byte_Operand_2, Temp_Byte);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : XORWF 0x%02X, %c.\n", Byte_Operand_2, Byte_Operand_1 == 0 ? 'W' : 'F');
			goto Exit;
			
		// ADDWF
		case 0x07:
			// Get the operand register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Do the operation
			Temp_Word = Temp_Byte + Core_Register_W;
			// Handle STATUS flags
			CoreUpdateStatusRegister(Temp_Word, CORE_AFFECTED_FLAG_CARRY | CORE_AFFECTED_FLAG_DIGIT_CARRY | CORE_AFFECTED_FLAG_ZERO);
			// Update the right destination register
			if (Byte_Operand_1 == 0) Core_Register_W = (unsigned char) Temp_Word;
			else RegisterFileBankedWrite(Byte_Operand_2, (unsigned char) Temp_Word);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : ADDWF 0x%02X, %c.\n", Byte_Operand_2, Byte_Operand_1 == 0 ? 'W' : 'F');
			goto Exit;
			
		// MOVF
		case 0x08:
			// Get the operand register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Handle STATUS flags
			CoreUpdateStatusRegister(Temp_Byte, CORE_AFFECTED_FLAG_ZERO);
			// Update the right destination register
			if (Byte_Operand_1 == 0) Core_Register_W = Temp_Byte;
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : MOVF 0x%02X, %c.\n", Byte_Operand_2, Byte_Operand_1 == 0 ? 'W' : 'F');
			goto Exit;
			
		// COMF
		case 0x09:
			// Get the operand register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Do the operation
			Temp_Byte = ~Temp_Byte;
			// Handle STATUS flags
			CoreUpdateStatusRegister(Temp_Byte, CORE_AFFECTED_FLAG_ZERO);
			// Update the right destination register
			if (Byte_Operand_1 == 0) Core_Register_W = Temp_Byte;
			else RegisterFileBankedWrite(Byte_Operand_2, Temp_Byte);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : COMF 0x%02X, %c.\n", Byte_Operand_2, Byte_Operand_1 == 0 ? 'W' : 'F');
			goto Exit;
			
		// INCF
		case 0x0A:
			// Get the operand register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Do the operation
			Temp_Byte++;
			// Handle STATUS flags
			CoreUpdateStatusRegister(Temp_Byte, CORE_AFFECTED_FLAG_ZERO);
			// Update the right destination register
			if (Byte_Operand_1 == 0) Core_Register_W = Temp_Byte;
			else RegisterFileBankedWrite(Byte_Operand_2, Temp_Byte);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : INCF 0x%02X, %c.\n", Byte_Operand_2, Byte_Operand_1 == 0 ? 'W' : 'F');
			goto Exit;
			
		// DECFSZ
		case 0x0B:
			// Get the operand register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Do the operation
			Temp_Byte--;
			// Update the right destination register
			if (Byte_Operand_1 == 0) Core_Register_W = Temp_Byte;
			else RegisterFileBankedWrite(Byte_Operand_2, Temp_Byte);
			// Skip next instruction if the result is zero
			if (Temp_Byte == 0) Core_Program_Counter += 2;
			else Core_Program_Counter++; // Point on next instruction
			LOG(LOG_LEVEL_DEBUG, "Found instruction : DECFSZ 0x%02X, %c.\n", Byte_Operand_2, Byte_Operand_1 == 0 ? 'W' : 'F');
			goto Exit;
			
		// RRF
		case 0x0C:
			// Get the operand register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Keep operand least significant bit
			New_Carry_Value = Temp_Byte & 1;
			// Do the operation
			Temp_Byte >>= 1;
			// Put the current carry bit value at the operand most significant bit position
			Current_Carry_Value = RegisterFileBankedRead(REGISTER_FILE_REGISTER_ADDRESS_STATUS) & REGISTER_FILE_REGISTER_BIT_STATUS_C;
			if (Current_Carry_Value != 0) Temp_Byte |= 0x80;
			// Update STATUS carry flag using a little hack
			if (New_Carry_Value != 0) Temp_Word = 0x0100;
			else Temp_Word = 0;
			CoreUpdateStatusRegister(Temp_Word, CORE_AFFECTED_FLAG_CARRY);
			// Update the right destination register
			if (Byte_Operand_1 == 0) Core_Register_W = Temp_Byte;
			else RegisterFileBankedWrite(Byte_Operand_2, Temp_Byte);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : RRF 0x%02X, %c.\n", Byte_Operand_2, Byte_Operand_1 == 0 ? 'W' : 'F');
			goto Exit;
			
		// RLF
		case 0x0D:
			// Get the operand register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Keep operand most significant bit
			New_Carry_Value = Temp_Byte & 0x80;
			// Do the operation
			Temp_Byte <<= 1;
			// Put the current carry bit value at the operand least significant bit position
			Current_Carry_Value = RegisterFileBankedRead(REGISTER_FILE_REGISTER_ADDRESS_STATUS) & REGISTER_FILE_REGISTER_BIT_STATUS_C; 
			if (Current_Carry_Value != 0) Temp_Byte |= 0x01;
			// Update STATUS carry flag using a little hack
			if (New_Carry_Value != 0) Temp_Word = 0x0100;
			else Temp_Word = 0;
			CoreUpdateStatusRegister(Temp_Word, CORE_AFFECTED_FLAG_CARRY);
			// Update the right destination register
			if (Byte_Operand_1 == 0) Core_Register_W = Temp_Byte;
			else RegisterFileBankedWrite(Byte_Operand_2, Temp_Byte);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : RLF 0x%02X, %c.\n", Byte_Operand_2, Byte_Operand_1 == 0 ? 'W' : 'F');
			goto Exit;
			
		// SWAPF
		case 0x0E:
			// Get the operand register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Do the operation
			New_Carry_Value = Temp_Byte & 0x0F; // Recycle New_Carry_Value variable to store the low operand nibble
			Temp_Byte = ((New_Carry_Value << 4) & 0xF0) | ((Temp_Byte >> 4) & 0x0F);
			// Update the right destination register
			if (Byte_Operand_1 == 0) Core_Register_W = Temp_Byte;
			else RegisterFileBankedWrite(Byte_Operand_2, Temp_Byte);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : SWAPF 0x%02X, %c.\n", Byte_Operand_2, Byte_Operand_1 == 0 ? 'W' : 'F');
			goto Exit;
			
		// INCFSZ
		case 0x0F:
			// Get the operand register value
			Temp_Byte = RegisterFileBankedRead(Byte_Operand_2);
			// Do the operation
			Temp_Byte++;
			// Update the right destination register
			if (Byte_Operand_1 == 0) Core_Register_W = Temp_Byte;
			else RegisterFileBankedWrite(Byte_Operand_2, Temp_Byte);
			// Skip next instruction if the result is zero
			if (Temp_Byte == 0) Core_Program_Counter += 2;
			else Core_Program_Counter++; // Point on next instruction
			LOG(LOG_LEVEL_DEBUG, "Found instruction : INCFSZ 0x%02X, %c.\n", Byte_Operand_2, Byte_Operand_1 == 0 ? 'W' : 'F');
			goto Exit;
	}
	
	// One 11-bit operand instruction format
	Word_Operand = Instruction & 0x07FF; // Get the operand
	Temp_Byte = RegisterFileBankedRead(REGISTER_FILE_REGISTER_ADDRESS_PCLATH) & 0x18; // Get upper 2 bits
	switch ((Instruction >> 11) & 0x0007)
	{
		// CALL
		case 0x04:
			// Push the Program Counter return value (PC + 1)
			CoreStackPush(Core_Program_Counter + 1);
			// Do the operation
			Core_Program_Counter = (Temp_Byte << 8) | Word_Operand;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : CALL 0x%04X.\n", Word_Operand);
			goto Exit;
		
		// GOTO
		case 0x05:
			// Do the operation
			Core_Program_Counter = (Temp_Byte << 8) | Word_Operand;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : GOTO 0x%04X.\n", Word_Operand);
			goto Exit;
	}
	
	// One 8-bit operand instruction format
	Byte_Operand_1 = (unsigned char) Instruction; // Get the operand
	switch ((Instruction >> 8) & 0x007F)
	{
		// MOVLW
		case 0x30:
			// Do the operation
			Core_Register_W = Byte_Operand_1;
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : MOVLW 0x%02X.\n", Byte_Operand_1);
			goto Exit;
			
		// RETLW
		case 0x34:
			// Put the return value in W
			Core_Register_W = Byte_Operand_1;
			// Pop the return address
			Core_Program_Counter = CoreStackPop();
			LOG(LOG_LEVEL_DEBUG, "Found instruction : RETLW 0x%02X.\n", Byte_Operand_1);
			goto Exit;
			
		// IORLW
		case 0x38:
			// Do the operation
			Core_Register_W |= Byte_Operand_1;
			// Handle STATUS flags
			CoreUpdateStatusRegister(Core_Register_W, CORE_AFFECTED_FLAG_ZERO);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : IORLW 0x%02X.\n", Byte_Operand_1);
			goto Exit;
			
		// ANDLW
		case 0x39:
			// Do the operation
			Core_Register_W &= Byte_Operand_1;
			// Handle STATUS flags
			CoreUpdateStatusRegister(Core_Register_W, CORE_AFFECTED_FLAG_ZERO);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : ANDLW 0x%02X.\n", Byte_Operand_1);
			goto Exit;
			
		// XORLW
		case 0x3A:
			// Do the operation
			Core_Register_W ^= Byte_Operand_1;
			// Handle STATUS flags
			CoreUpdateStatusRegister(Core_Register_W, CORE_AFFECTED_FLAG_ZERO);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : XORLW 0x%02X.\n", Byte_Operand_1);
			goto Exit;
			
		// SUBLW
		case 0x3C:
			// Compute the subtraction on a greater number of bits to find out a borrow report
			Temp_Word = Core_Register_W - Byte_Operand_1;
			Core_Register_W = (unsigned char) Temp_Word;
			// Handle STATUS flags
			CoreUpdateStatusRegister(Temp_Word, CORE_AFFECTED_FLAG_CARRY | CORE_AFFECTED_FLAG_DIGIT_CARRY | CORE_AFFECTED_FLAG_ZERO);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : SUBLW 0x%02X.\n", Byte_Operand_1);
			goto Exit;
			
		// ADDLW
		case 0x3E:
			// Compute the addition on a greater number of bits to find out a carry report
			Temp_Word = Core_Register_W + Byte_Operand_1;
			Core_Register_W = (unsigned char) Temp_Word;
			// Handle STATUS flags
			CoreUpdateStatusRegister(Temp_Word, CORE_AFFECTED_FLAG_CARRY | CORE_AFFECTED_FLAG_DIGIT_CARRY | CORE_AFFECTED_FLAG_ZERO);
			// Point on next instruction
			Core_Program_Counter++;
			LOG(LOG_LEVEL_DEBUG, "Found instruction : ADDLW 0x%02X.\n", Byte_Operand_1);
			goto Exit;
	}
	
	// Unknown instructions are executed as NOP
	// Point on next instruction
	Core_Program_Counter++;
	LOG(LOG_LEVEL_WARNING, "WARNING : unknown instruction found, executing as NOP.\n");
	
Exit:
	// Check for interrupt
	if (RegisterFileHasInterruptFired())
	{
		// Disable the interrupts to avoid looping to the interrupt handler at each instruction
		Temp_Byte = RegisterFileBankedRead(REGISTER_FILE_REGISTER_ADDRESS_INTCON);
		Temp_Byte &= ~REGISTER_FILE_REGISTER_BIT_INTCON_GIE;
		RegisterFileBankedWrite(REGISTER_FILE_REGISTER_ADDRESS_INTCON, Temp_Byte);
		
		// Push the Program Counter return value (PC + 1)
		CoreStackPush(Core_Program_Counter + 1);
		// Branch to the interrupt handler entry point
		Core_Program_Counter = 0x0004;
		LOG(LOG_LEVEL_DEBUG, "Interrupt fired. Branching to interrupt handler entry point.\n");
	}

	// Save the new Program Counter value to PCL
	RegisterFileBankedWrite(REGISTER_FILE_REGISTER_ADDRESS_PCL, (unsigned char) Core_Program_Counter);
	LOG(LOG_LEVEL_DEBUG, "Finished instruction execution, new Program Counter value is : 0x%04X.\n", Core_Program_Counter);
}
