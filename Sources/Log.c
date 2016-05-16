/** @file Log.c
 * @see Log.h for description.
 * @author Adrien RICCIARDI
 */
#include <Log.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** The log file. */
static FILE *Pointer_Log_File;

/** The accepted log level. */
static TLogLevel Log_Maximum_Level;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Automatically close the log file on program exit. */
static void LogExitCloseFile(void)
{
	fclose(Pointer_Log_File);
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void LogInitialize(char *String_Log_File, TLogLevel Log_Level)
{
	// Try to open the file
	Pointer_Log_File = fopen(String_Log_File, "w");
	if (Pointer_Log_File == NULL)
	{
		printf("Error : failed to open the log file '%s'\n", String_Log_File);
		exit(EXIT_FAILURE);
	}
	
	// Automatically close the log file on program exit
	atexit(LogExitCloseFile);
	
	Log_Maximum_Level = Log_Level;
	
	LOG(LOG_LEVEL_ERROR, "Set log level to %d.\n", Log_Maximum_Level);
}

void LogWrite(TLogLevel Log_Level, char *String_Format, ...)
{
	va_list Arguments_List;
	
	if (Log_Level <= Log_Maximum_Level)
	{
		va_start(Arguments_List, String_Format);
		vfprintf(Pointer_Log_File, String_Format, Arguments_List);
		va_end(Arguments_List);
	}
}