/** @file Main.c
 * Simulate the Text Games System board. See http://adrien-ricciardi.pagesperso-orange.fr/Projects/Text_Games_System/Text_Games_System.html for more details.
 * @author Adrien RICCIARDI
 */
#include <Core.h>
#include <errno.h>
#include <Log.h>
#include <Peripheral_ADC.h>
#include <Peripheral_I2C_EEPROM.h>
#include <Peripheral_Timer.h>
#include <Peripheral_UART.h>
#include <Program_Memory.h>
#include <pthread.h>
#include <Register_File.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Convert a Ctrl+key combination to the corresponding value returned by getchar().
 * @param Key The key to associate to Ctrl.
 * @return The corresponding getchar() code.
 */
#define MAIN_CONTROL_KEY_COMBINATION(Key) (Key & 0x1F)

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
	if (system("stty raw -echo") != 0) printf("WARNING : tty initialization failed.\n");
}

/** Restore the console default behavior. */
static inline void MainUninitializeConsole(void)
{
	if (system("stty cooked echo") != 0) printf("WARNING : tty uninitialization failed.\n");
	
	// Show cursor
	printf("\x1B[?25h");
}

/** Execute the PIC program.
 * @return always 0.
 */
static void *MainThreadExecuteProgram(void __attribute__((unused)) *Pointer_Parameters)
{
	LOG(LOG_LEVEL_DEBUG, "Thread started.\n");

	while (!Main_Is_Simulator_Exiting)
	{
		CoreExecuteNextInstruction();
		
		// Clock the timers
		PeripheralTimerIncrement();
	}

	LOG(LOG_LEVEL_DEBUG, "Thread exited.\n");
	pthread_exit(0);
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	char *String_Log_File, *String_Program_Hex_File, *String_EEPROM_File;
	TLogLevel Log_Level;
	pthread_t Thread_ID;
	int Character_Code;
	
	// Check parameters
	if (argc != 5)
	{
		printf("Usage : %s Log_File Log_Level Program_Hex_File EEPROM_File\n"
			"  Log_File : the file that will contain all logs.\n"
			"  Log_Level : how much log to write to the log file (error = 0, warning = 1, debug = 2).\n"
			"  Program_Hex_File : an Intel Hex file containing the program code.\n"
			"  EEPROM_File : a 4096-byte file containing the EEPROM data.\n"
			"Use Ctrl+C to exit program.\n"
			"Use Ctrl+D to write a dump of the register file to the log file.\n", argv[0]);
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
	String_EEPROM_File = argv[4];
	
	// Initialize subsystems
	LogInitialize(String_Log_File, Log_Level);
	RegisterFileInitialize();
	PeripheralADCInitialize();
	
	// Load the program to execute
	if (ProgramMemoryLoadHexFile(String_Program_Hex_File) != 0)
	{
		printf("Error : failed to load the hex file. See logs for more information.\n");
		return EXIT_FAILURE;
	}
	
	// Load the EEPROM content
	if (PeripheralI2CEEPROMInitialize(String_EEPROM_File) != 0)
	{
		printf("Error : failed to load the EEPROM file. See logs for more information.\n");
		return EXIT_FAILURE;
	}

	// Create a thread that will execute the PIC program
	if (pthread_create(&Thread_ID, NULL, MainThreadExecuteProgram, NULL) != 0)
	{
		printf("Error : failed to create the CPU thread (%s).\n", strerror(errno));
		return EXIT_FAILURE;
	}

	// Handle the "user interface"
	MainInitializeConsole();
	while (1)
	{
		Character_Code = getchar();
		if (Character_Code == MAIN_CONTROL_KEY_COMBINATION('c')) break; // Ctrl+c
		else if (Character_Code == MAIN_CONTROL_KEY_COMBINATION('d')) RegisterFileDump(); // Ctrl+d, stands for "dump"

		// Send the character to the UART
		PeripheralUARTReceiveByte((unsigned char) Character_Code);
	}
	MainUninitializeConsole(); // TODO this function and below code are not reached when the simulator must do an emergency exit
	
	// Wait for the thread to terminate
	Main_Is_Simulator_Exiting = 1;
	if (pthread_join(Thread_ID, NULL) != 0)
	{
		printf("Error : failed to join the CPU thread (%s).\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	// Store the EEPROM content to the EEPROM file
	if (PeripheralI2CEEPROMStoreMemoryToFile(String_EEPROM_File))
	{
		printf("Error : failed to save the EEPROM memory content to the EEPROM file. See logs for more information.\n");
		return EXIT_FAILURE;
	}

	LOG(LOG_LEVEL_ERROR, "Program successfully exited.\n");
	return EXIT_SUCCESS;
}
