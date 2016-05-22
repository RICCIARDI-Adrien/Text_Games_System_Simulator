/** @file Peripheral_UART.h
 * Emulate the 19200 bit/s UART by reading/writing to the system console.
 * @author Adrien RICCIARDI
 */
#ifndef H_PERIPHERAL_UART_H
#define H_PERIPHERAL_UART_H

#include <Register_File.h>

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** The callback that must be called when the RCREG register is read.
 * @param Pointer_Content The register content.
 * @return The last UART received byte.
 */
unsigned char PeripheralUARTReadRCREG(TRegisterFileRegisterContent *Pointer_Content);

/** The callback that must be called when the TXREG register is written.
 * @param Pointer_Content The register content.
 * @param Data The data byte to send over the UART.
 */
void PeripheralUARTWriteTXREG(TRegisterFileRegisterContent *Pointer_Content, unsigned char Data);

/** The callback that must be called when the TXSTA register is written.
 * @param Pointer_Content The register content.
 * @param Data The TXSTA value.
 */
void PeripheralUARTWriteTXSTA(TRegisterFileRegisterContent *Pointer_Content, unsigned char Data);

/** Send a byte to the PIC UART.
 * @param Data The received data.
 */
void PeripheralUARTReceiveByte(unsigned char Data);

#endif
