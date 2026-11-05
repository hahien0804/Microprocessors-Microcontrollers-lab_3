/* TIM2 samples inputs; elapsed milliseconds define debounce and hold time. */
#include "input_reading.h"
#define DEBOUNCE_MS 20U
static GPIO_TypeDef* const ports[3] = {BUTTON_1_GPIO_Port, BUTTON_2_GPIO_Port, BUTTON_3_GPIO_Port};
static const uint16_t pins[3] = {BUTTON_1_Pin, BUTTON_2_Pin, BUTTON_3_Pin};
static GPIO_PinState candidate[3] = {GPIO_PIN_SET,GPIO_PIN_SET,GPIO_PIN_SET};
static volatile GPIO_PinState stable[3] = {GPIO_PIN_SET,GPIO_PIN_SET,GPIO_PIN_SET};
static uint32_t changed[3];
static volatile uint32_t pressed[3];
static volatile uint8_t event[3];
void button_reading(void) {
  uint32_t now = HAL_GetTick();
  for (unsigned i=0; i<3; ++i) {
    GPIO_PinState raw = HAL_GPIO_ReadPin(ports[i],pins[i]);
    if (raw != candidate[i]) { candidate[i]=raw; changed[i]=now; }
    if (raw != stable[i] && (uint32_t)(now-changed[i]) >= DEBOUNCE_MS) {
      stable[i]=raw;
      if (raw==GPIO_PIN_RESET) { event[i]=1; pressed[i]=now; }
    }
  }
}
unsigned char is_button_pressed(unsigned char i) {
  if (i>=3) return 0;
  uint32_t mask=__get_PRIMASK(); __disable_irq();
  uint8_t result=event[i]; event[i]=0;
  __set_PRIMASK(mask); return result;
}
unsigned char is_button_held(unsigned char i) {
  return i<3 && stable[i]==GPIO_PIN_RESET;
}
unsigned char is_button_pressed_1s(unsigned char i) {
  return i<3 && stable[i]==GPIO_PIN_RESET &&
         (uint32_t)(HAL_GetTick()-pressed[i])>=1000U;
}
