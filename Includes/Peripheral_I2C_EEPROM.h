/** @file Peripheral_I2C_EEPROM.h
 * Emulate the external 24LC32 EEPROM connected to the I2C module.
 * @author Adrien RICCIARDI
 */
#ifndef H_PERIPHERAL_I2C_EEPROM_H
#define H_PERIPHERAL_I2C_EEPROM_H

#include <Register_File.h>

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Load a file content into the EEPROM memory.
 * @param String_EEPROM_File The file containing the EEPROM memory.
 * @return 0 if the file content was successfully loaded,
 * @return 1 if an error occurred.
 */
int PeripheralI2CEEPROMInitialize(char *String_EEPROM_File);

/** Store the EEPROM memory content to the specified EEPROM file.
 * @param String_EEPROM_File The destination file.
 * @return 0 if the memory content was successfully written to the file,
 * @return 1 if an error occurred.
 */
int PeripheralI2CEEPROMStoreMemoryToFile(char *String_EEPROM_File);

/** The callback that must be called when the SSPCON2 register is written.
 * @param Pointer_Content The register content.
 * @param Data The new register value.
 */
void PeripheralI2CEEPROMWriteSSPCON2(TRegisterFileRegisterContent *Pointer_Content, unsigned char Data);

/** The callback that must be called when the SSPBUF register is written.
 * @param Pointer_Content The register content.
 * @param Data The I2C byte to send.
 */
void PeripheralI2CEEPROMWriteSSPBUF(TRegisterFileRegisterContent *Pointer_Content, unsigned char Data);

#endif