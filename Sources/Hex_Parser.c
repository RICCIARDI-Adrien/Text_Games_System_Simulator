/** @file Hex_Parser.c
 * @see Hex_Parser.h for description.
 * @author Adrien RICCIARDI
 */
#include <Hex_Parser.h>
#include <stdio.h>
#include <string.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Size of a record. */
#define HEX_PARSER_OFFSET_RECORD_SIZE 0
/** Beginning address of the data contained in the record. */
#define HEX_PARSER_OFFSET_DATA_ADDRESS 1
/** Type of the record. */
#define HEX_PARSER_OFFSET_RECORD_TYPE 3
/** Offset of the beginning of the data into the record. */
#define HEX_PARSER_OFFSET_DATA 4

/** The record holds data. */
#define HEX_PARSER_RECORD_TYPE_DATA 0
/** End of file record. */
#define HEX_PARSER_RECORD_TYPE_END_OF_FILE 1

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Convert an hexadecimal string number into its binary representation.
 * @param High_Nibble The high nibble of the hexadecimal number (corresponding to bits 7 to 4).
 * @param Low_Nibble The low nibble of the hexadecimal number (corresponding to bits 3 to 0).
 * @return The binary value of the number.
 * @warning No checks are done on nibble validity, be careful about parameters value.
 */
static inline unsigned char HexParserConvertHexadecimalToByte(unsigned char High_Nibble, unsigned char Low_Nibble)
{
	// Convert symbol to capitals if needed
	if ((High_Nibble >= 'a') && (High_Nibble <= 'f')) High_Nibble -= 32;
	if ((Low_Nibble >= 'a') && (Low_Nibble <= 'f')) Low_Nibble -= 32;
	
	// High nibble
	if ((High_Nibble >= 'A') && (High_Nibble <= 'F')) High_Nibble -= 55; // A = 10 in decimal
	else High_Nibble -= 48; // Handle digits
	// Low nibble
	if ((Low_Nibble >= 'A') && (Low_Nibble <= 'F')) Low_Nibble -= 55; // A = 10 in decimal
	else Low_Nibble -= 48; // Handle digits
		
	return (unsigned char) (High_Nibble << 4) + Low_Nibble;
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
int HexParserDecodeLine(char *String_Hex_Line, THexParserInstruction Instructions[])
{
	unsigned char Buffer[(HEX_PARSER_MAXIMUM_INSTRUCTIONS_PER_LINE * 2) + 20]; // Should be large enough
	unsigned short Address;
	int i, j = 1, Instructions_Count, Buffer_Size;
	
	// Convert line to binary value
	Buffer_Size = (strlen(String_Hex_Line) - 1) / 2;
	for (i = 0; i < Buffer_Size; i++)
	{
		Buffer[i] = HexParserConvertHexadecimalToByte(String_Hex_Line[j], String_Hex_Line[j + 1]);
		j += 2;
	}

	// Is end of file reached ?
	if (Buffer[HEX_PARSER_OFFSET_RECORD_TYPE] == HEX_PARSER_RECORD_TYPE_END_OF_FILE)
	{
		Instructions[0].Is_Instruction_Valid = 0;
		Instructions[0].Is_End_Of_File = 1;
		return 1;
	}
	
	// Assert this is a data record
	if (Buffer[HEX_PARSER_OFFSET_RECORD_TYPE] != HEX_PARSER_RECORD_TYPE_DATA) return 0;
	
	// Compute instructions count
	Instructions_Count = Buffer[HEX_PARSER_OFFSET_RECORD_SIZE] / 2; // 2 bytes per instruction
	// Find record start address
	Address = ((Buffer[HEX_PARSER_OFFSET_DATA_ADDRESS] << 8) | Buffer[HEX_PARSER_OFFSET_DATA_ADDRESS + 1]) / 2; // The address is in bytes, we need it in words
	
	// Parse data
	for (i = 0; i < Instructions_Count; i++)
	{
		// Set instruction address
		Instructions[i].Address = Address;
		// Next instruction
		Address++;
		
		// Find instruction code
		Instructions[i].Code = (Buffer[HEX_PARSER_OFFSET_DATA + (i * 2) + 1] << 8) | Buffer[HEX_PARSER_OFFSET_DATA + (i * 2)]; // Convert little endian storage in the hex file to big endian
		Instructions[i].Is_Instruction_Valid = 1;
		Instructions[i].Is_End_Of_File = 0;
	}
	return Instructions_Count;
}
