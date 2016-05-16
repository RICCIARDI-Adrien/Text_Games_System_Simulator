/** @file Log.h
 * Write logging informations to a file.
 * @author Adrien RICCIARDI
 */
#ifndef H_LOG_H
#define H_LOG_H

//-------------------------------------------------------------------------------------------------
// Constants and macros
//-------------------------------------------------------------------------------------------------
/** Write a log message to the log file.
 * @param String_Format A printf-like format string.
 */
#define LOG(Log_Level, String_Format, ...) LogWrite(Log_Level, "[%s:%d] " String_Format, __FUNCTION__, __LINE__, ##__VA_ARGS__)

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
/** All available log levels. */
typedef enum
{
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_DEBUG
} TLogLevel;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Initialize the logging system.
 * @param String_Log_File The file that will contain the log messages.
 * @param Log_Level The application log level.
 * @note There is no need to close the logging system, it will be automatically closed on exit.
 */
void LogInitialize(char *String_Log_File, TLogLevel Log_Level);

/** Write a string to the logging system.
 * @param Log_Level The string log level. The string may not be written if the program log level is set too high.
 * @param String_Format A printf-like string.
 */
void LogWrite(TLogLevel Log_Level, char *String_Format, ...);

#endif