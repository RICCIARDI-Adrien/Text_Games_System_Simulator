/** @file Main.c
 * Simulate the Text Games System board. See http://adrien-ricciardi.pagesperso-orange.fr/Projects/Text_Games_System/Text_Games_System.html for more details.
 * @author Adrien RICCIARDI
 */
#include <Core.h>
#include <errno.h>
#include <Log.h>
#include <Program_Memory.h>
#include <pthread.h>
#include <Register_File.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** Tell whether the simulator is quitting or not. */
static volatile int Main_Is_Simulator_Exiting = 0;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Do not display typed in text and disable all default console features. */
static inline void MainInitializeConsole(void)
{
        system("stty raw -echo");
}

/** Restore the console default behavior. */
static inline void MainUninitializeConsole(void)
{
	system("stty cooked echo");
}

/** Execute the PIC program.
 * @return always 0.
 */
static void *MainThreadExecuteProgram(void __attribute__((unused)) *Pointer_Parameters)
{
	LOG(LOG_LEVEL_DEBUG, "Thread started.\n");

	while (!Main_Is_Simulator_Exiting) CoreExecuteNextInstruction();

	LOG(LOG_LEVEL_DEBUG, "Thread exited.\n");
	pthread_exit(0);
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	char *String_Log_File, *String_Program_Hex_File;
	TLogLevel Log_Level;
	pthread_t Thread_ID;
	int Character_Code;
	
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
	if (ProgramMemoryLoadHexFile(String_Program_Hex_File) != 0)
	{
		printf("Error : failed to load the hex file. See logs for more information.\n");
		return EXIT_FAILURE;
	}

	// Create a thread that will execute the PIC program
	if (pthread_create(&Thread_ID, NULL, MainThreadExecuteProgram, NULL) != 0)
	{
		printf("Error : failed to create a thread (%s).\n", strerror(errno));
		return EXIT_FAILURE;
	}

	// Handle the "user interface"
	MainInitializeConsole();
	while (1)
	{
		Character_Code = getchar();
		if (Character_Code == 3) break; // Ctrl+C
	}
	MainUninitializeConsole();

	// Wait for the thread to terminate
	Main_Is_Simulator_Exiting = 1;
	if (pthread_join(Thread_ID, NULL) != 0)
	{
		printf("Error : failed to join the thread (%s).\n", strerror(errno));
		return EXIT_FAILURE;
	}

	LOG(LOG_LEVEL_ERROR, "Program successfully exited.\n");
	return EXIT_SUCCESS;
}
