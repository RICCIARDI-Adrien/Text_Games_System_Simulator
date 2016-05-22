/** @file Peripheral_ADC.c
 * @see Peripheral_ADC.h for description.
 * @author Adrien RICCIARDI.
 */
#include <Log.h>
#include <Peripheral_ADC.h>
#include <Register_File.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void PeripheralADCInitialize(void)
{
	srand(time(NULL));
}

void PeripheralADCWriteADCON0(TRegisterFileRegisterContent *Pointer_Content, unsigned char Data)
{
	int Sampled_Value;
	
	// Start a conversion if ADC module is enabled and if the GO bit is set
	if ((Data & REGISTER_FILE_REGISTER_BIT_ADCON0_ADON) && (Data & REGISTER_FILE_REGISTER_BIT_ADCON0_GO))
	{
		// Sample data
		Sampled_Value = rand() % 1024; // The ADC sample is stored on 10 bits
		LOG(LOG_LEVEL_DEBUG, "ADC sampled value : %d.\n", Sampled_Value & 0x03FF);
			
		// Fill result registers
		// TODO take justification into account if needed
		RegisterFileDirectWriteFromCallback(REGISTER_FILE_REGISTER_BANK_ADRESH, REGISTER_FILE_REGISTER_ADDRESS_ADRESH, (Sampled_Value >> 8) & 0x03);
		RegisterFileDirectWriteFromCallback(REGISTER_FILE_REGISTER_BANK_ADRESL, REGISTER_FILE_REGISTER_ADDRESS_ADRESL, (unsigned char) Sampled_Value);
		
		// Clear GO bit to tell that the conversion is terminated
		Data &= ~REGISTER_FILE_REGISTER_BIT_ADCON0_GO;
	}
	
	Pointer_Content->Data = Data;
}