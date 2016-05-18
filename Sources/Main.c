/** @file Main.c
 * Simulate the Text Games System board. See http://adrien-ricciardi.pagesperso-orange.fr/Projects/Text_Games_System/Text_Games_System.html for more details.
 * @author Adrien RICCIARDI
 */
#include <Core.h>
#include <Log.h>
#include <Program_Memory.h>
#include <Register_File.h>
#include <stdio.h>
#include <stdlib.h>

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	char *String_Log_File, *String_Program_Hex_File;
	TLogLevel Log_Level;
	
	// Check parameters
	if (argc != 4)
	{
		printf("Usage : %s Log_File Log_Level Program_Hex_File\n"
			"  Log_File : the file that will contain all logs.\n"
			"  Log_Level : how much log to write to the log file (error = 0, warning = 1, debug = 2).\n"
			"  Program_Hex_File : an Intel Hex file containing the program code.\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	// Retrieve parameters
	String_Log_File = argv[1];
	// Retrieve log level
	if (sscanf(argv[2], "%d", (int *) &Log_Level) != 1)
	{
		printf("Error : the log level must be an integer value between 0 and 2.\n");
		return EXIT_FAILURE;
	}
	String_Program_Hex_File = argv[3];
	
	// Initialize subsystems
	LogInitialize(String_Log_File, Log_Level);
	RegisterFileInitialize();
	
	// Load the program to execute
	ProgramMemoryLoadHexFile(String_Program_Hex_File);
	
	// TEST
	CoreExecuteNextInstruction();
	
	return EXIT_SUCCESS;
}