/** @file Peripheral_ADC.h
 * Simulate the PIC16F876 10-bit analog to digital converter.
 * @author Adrien RICCIARDI
 */
#ifndef H_PERIPHERAL_ADC_H
#define H_PERIPHERAL_ADC_H

#include <Register_File.h>

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Initialize the pseudo-random generator that will feed the ADC sampled values. */
void PeripheralADCInitialize(void);

/** The callback that must be called when the ADCON0 register is written.
 * @param Pointer_Content The register content.
 * @param Data The new register value.
 */
void PeripheralADCWriteADCON0(TRegisterFileRegisterContent *Pointer_Content, unsigned char Data);

#endif