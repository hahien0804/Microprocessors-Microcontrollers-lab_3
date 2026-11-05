/* Main-loop timers use the independent 1 ms HAL SysTick. */
#include "timer.h"
#include "input_reading.h"
static uint32_t started[NUM_TIMERS], interval[NUM_TIMERS];
static uint8_t active[NUM_TIMERS];
void setTimer(int id, int duration_ms) {
  if (id < 0 || id >= NUM_TIMERS) return;
  started[id] = HAL_GetTick();
  interval[id] = duration_ms > 0 ? (uint32_t)duration_ms : 1U;
  active[id] = 1;
}
uint8_t isTimerExpired(int id) {
  if (id < 0 || id >= NUM_TIMERS || !active[id]) return 0;
  if ((uint32_t)(HAL_GetTick() - started[id]) < interval[id]) return 0;
  active[id] = 0;
  return 1;
}
void timer_run(void) { /* Retained for API compatibility; no ISR state. */ }
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM2) button_reading();
}
