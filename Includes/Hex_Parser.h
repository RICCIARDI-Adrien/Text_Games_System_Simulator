/** @file Hex_Parser.h
 * Simple parser for Intel Hexadecimal file format.
 * @author Adrien RICCIARDI
 */
#ifndef H_HEX_PARSER_H
#define H_HEX_PARSER_H

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** How many instructions can be contained in one hex file line. */
#define HEX_PARSER_MAXIMUM_INSTRUCTIONS_PER_LINE 32

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
/** A decoded hex instruction. */
typedef struct
{
	unsigned short Address; //! Address of the instruction.
	unsigned short Code; //! Instruction code.
	char Is_Instruction_Valid; //! Tell if the instruction is valid (can be sent to the board) or not.
	char Is_End_Of_File; //! Tell if EOF is reached or not.
} THexParserInstruction;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Parse a whole line read from the hex file.
 * @param String_Hex_Line The line from the hex file (like ":A012345678").
 * @param Instructions An array holding all parsed instructions when the function returns (the array must be HEX_PARSER_MAXIMUM_INSTRUCTIONS_PER_LINE wide).
 * @return The instructions count contained in String_Hex_Line.
 */
int HexParserDecodeLine(char *String_Hex_Line, THexParserInstruction Instructions[]);

#endif
