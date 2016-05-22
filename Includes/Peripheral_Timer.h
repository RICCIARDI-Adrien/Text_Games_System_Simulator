/** @file Peripheral_Timer.h
 * Simulate the Timer 0 and Timer 2 modules.
 * @author Adrien RICCIARDI
 */
#ifndef H_PERIPHERAL_TIMER_H
#define H_PERIPHERAL_TIMER_H

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Increment the timer modules according to their internal prescaler/postscaler. */
void PeripheralTimerIncrement(void);

#endif