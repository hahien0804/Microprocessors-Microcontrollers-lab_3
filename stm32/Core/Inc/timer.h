/*
 * timer.h
 *
 * Software timers driven by the TIM2 interrupt.
 */
#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "main.h"

/* Application timers use HAL_GetTick (1 ms SysTick), independent of TIM2.
 * Keep the HAL SysTick time base at 1 ms. All API calls are main-loop only. */

/* Software timer IDs */
#define T_SECOND        0   /* 1 s tick of the traffic light countdown     */
#define T_BLINK         1   /* half period of the 2 Hz blinking LEDs       */
#define T_SCAN          2   /* 7-segment multiplexing                      */
#define T_AUTO_INC      3   /* auto increase while BUTTON_2 is held        */
#define NUM_TIMERS      4

void    setTimer(int id, int duration_ms);
uint8_t isTimerExpired(int id);   /* returns 1 once, then clears the flag */
void    timer_run(void);          /* compatibility no-op */

#endif /* INC_TIMER_H_ */
