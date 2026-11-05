/*
 * led_display.c
 */
#include "led_display.h"
#include "timer.h"

/* LEDs: anode -> +3.3V, cathode -> MCU pin  =>  LED ON when the pin is LOW */
#define LED_ON   GPIO_PIN_RESET
#define LED_OFF  GPIO_PIN_SET

/* 7-seg: common anode through a PNP transistor
 *   EN low  -> PNP conducts -> digit enabled
 *   SEG low -> segment on                                                  */
#define DIGIT_ON   GPIO_PIN_RESET
#define DIGIT_OFF  GPIO_PIN_SET

#define NUM_DIGITS 4

/* [road][colour - 1] */
static GPIO_TypeDef* const LIGHT_PORT[2][3] = {
  {LED_RED_1_GPIO_Port, LED_AMBER_1_GPIO_Port, LED_GREEN_1_GPIO_Port},
  {LED_RED_2_GPIO_Port, LED_AMBER_2_GPIO_Port, LED_GREEN_2_GPIO_Port}};
static const uint16_t LIGHT_PIN[2][3] = {
  {LED_RED_1_Pin, LED_AMBER_1_Pin, LED_GREEN_1_Pin},
  {LED_RED_2_Pin, LED_AMBER_2_Pin, LED_GREEN_2_Pin}};

static GPIO_TypeDef* const SEG_PORT[7] = {SEG0_GPIO_Port, SEG1_GPIO_Port, SEG2_GPIO_Port,
                                          SEG3_GPIO_Port, SEG4_GPIO_Port, SEG5_GPIO_Port, SEG6_GPIO_Port};
static const uint16_t SEG_PIN[7] = {SEG0_Pin, SEG1_Pin, SEG2_Pin, SEG3_Pin, SEG4_Pin, SEG5_Pin, SEG6_Pin};
static GPIO_TypeDef* const EN_PORT[NUM_DIGITS] = {EN0_GPIO_Port, EN1_GPIO_Port, EN2_GPIO_Port, EN3_GPIO_Port};
static const uint16_t EN_PIN[NUM_DIGITS] = {EN0_Pin, EN1_Pin, EN2_Pin, EN3_Pin};

/* bit0 = a ... bit6 = g, 1 = segment on */
static const uint8_t SEG_CODE[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

static int led_buffer[NUM_DIGITS] = {0, 0, 0, 0};
static int index_led = 0;

/* ---------------- traffic LEDs ---------------- */
void setRoadLight(int road, int light) {
  for (int c = 0; c < 3; c++) {
    HAL_GPIO_WritePin(LIGHT_PORT[road][c], LIGHT_PIN[road][c],
                      (c == light - 1) ? LED_ON : LED_OFF);
  }
}

void setColorBothRoads(int light, uint8_t on) {
  for (int road = 0; road < 2; road++) {
    for (int c = 0; c < 3; c++) {
      GPIO_PinState s = (c == light - 1 && on) ? LED_ON : LED_OFF;
      HAL_GPIO_WritePin(LIGHT_PORT[road][c], LIGHT_PIN[road][c], s);
    }
  }
}

/* ---------------- 7-segment ---------------- */
static void display7SEG(int num) {
  if (num < 0 || num > 9) num = 0;
  uint8_t code = SEG_CODE[num];
  for (int i = 0; i < 7; i++) {
    HAL_GPIO_WritePin(SEG_PORT[i], SEG_PIN[i], ((code >> i) & 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
  }
}

static void disableAll7SEG(void) {
  for (int i = 0; i < NUM_DIGITS; i++) {
    HAL_GPIO_WritePin(EN_PORT[i], EN_PIN[i], DIGIT_OFF);
  }
}

static void update7SEG(int index) {
  disableAll7SEG();                 /* 1. turn every digit off (no ghosting) */
  display7SEG(led_buffer[index]);   /* 2. put the segments of this digit     */
  HAL_GPIO_WritePin(EN_PORT[index], EN_PIN[index], DIGIT_ON);  /* 3. enable  */
}

void setDisplayValues(int left, int right) {
  if (left < 0)   left = 0;
  if (left > 99)  left = 99;
  if (right < 0)  right = 0;
  if (right > 99) right = 99;
  led_buffer[0] = left / 10;
  led_buffer[1] = left % 10;
  led_buffer[2] = right / 10;
  led_buffer[3] = right % 10;
}

void led_display_init(void) {
  setColorBothRoads(LIGHT_OFF, 0);
  disableAll7SEG();
  setTimer(T_SCAN, SCAN_7SEG_MS);
}

void led_display_run(void) {
  if (isTimerExpired(T_SCAN)) {
    setTimer(T_SCAN, SCAN_7SEG_MS);
    update7SEG(index_led);
    index_led = (index_led + 1) % NUM_DIGITS;
  }
}
