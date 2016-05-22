/** @file Peripheral_Timer.c
 * @see Peripheral_Timer.h for description.
 * @author Adrien RICCIARDI
 */
#include <Peripheral_Timer.h>
#include <Register_File.h>

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Increment TMR0 and set the T0IF flag if needed. */
static inline void PeripheralTimer0Increment(void)
{
	unsigned char Timer_Value, INTCON_Register;
	
	Timer_Value = RegisterFileDirectRead(REGISTER_FILE_REGISTER_BANK_TMR0, REGISTER_FILE_REGISTER_ADDRESS_TMR0);
	Timer_Value++;
	RegisterFileDirectWrite(REGISTER_FILE_REGISTER_BANK_TMR0, REGISTER_FILE_REGISTER_ADDRESS_TMR0, Timer_Value);
	
	// Did the timer overflow ?
	if (Timer_Value == 0)
	{
		// Set INTCON.T0IF
		INTCON_Register = RegisterFileDirectRead(REGISTER_FILE_REGISTER_BANK_INTCON, REGISTER_FILE_REGISTER_ADDRESS_INTCON);
		INTCON_Register |= REGISTER_FILE_REGISTER_BIT_INTCON_T0IF;
		RegisterFileDirectWrite(REGISTER_FILE_REGISTER_BANK_INTCON, REGISTER_FILE_REGISTER_ADDRESS_INTCON, INTCON_Register);
	}
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void PeripheralTimerIncrement(void)
{
	static int Timer_0_Prescaler = 0, Prescaler_Value;
	unsigned char Temp_Byte;
	
	// Timer 0 (always enabled, can't be disabled)
	Temp_Byte = RegisterFileDirectRead(REGISTER_FILE_REGISTER_BANK_OPTION_REG, REGISTER_FILE_REGISTER_ADDRESS_OPTION_REG);
	if (Temp_Byte & REGISTER_FILE_REGISTER_BIT_OPTION_REG_PSA) PeripheralTimer0Increment(); // The prescaler is assigned to the watchdog timer, so increment TMR0
	else // The prescaler is assigned to the timer
	{
		Prescaler_Value = 2 << (Temp_Byte & 0x07);
		
		Timer_0_Prescaler++;
		if (Timer_0_Prescaler == Prescaler_Value)
		{
			PeripheralTimer0Increment();
			Timer_0_Prescaler = 0;
		}
	}
		
	// Increment timer 2 only if it is enabled
	Temp_Byte = RegisterFileDirectRead(REGISTER_FILE_REGISTER_BANK_T2CON, REGISTER_FILE_REGISTER_ADDRESS_T2CON);
	if (Temp_Byte & REGISTER_FILE_REGISTER_BIT_T2CON_TMR2ON)
	{
		// TODO handle prescaler
		
		Temp_Byte = RegisterFileDirectRead(REGISTER_FILE_REGISTER_BANK_TMR2, REGISTER_FILE_REGISTER_ADDRESS_TMR2);
		Temp_Byte++;
		RegisterFileDirectWrite(REGISTER_FILE_REGISTER_BANK_TMR2, REGISTER_FILE_REGISTER_ADDRESS_TMR2, Temp_Byte);
		
		// TODO handle postscaler
		
		// TODO handle interrupt flag
	}
}