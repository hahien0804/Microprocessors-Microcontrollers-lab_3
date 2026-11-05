/*
 * input_reading.h
 *
 * Reading + debouncing the 3 buttons (active low, external 10k pull-up).
 */
#ifndef INC_INPUT_READING_H_
#define INC_INPUT_READING_H_

#include "main.h"

#define NO_OF_BUTTONS   3
#define BUTTON_MODE     0   /* BUTTON_1 - PB12: select mode             */
#define BUTTON_INC      1   /* BUTTON_2 - PB13: increase the value      */
#define BUTTON_SET      2   /* BUTTON_3 - PB14: set (save) the value    */

void          button_reading(void);                  /* called by TIM2 interrupt */
unsigned char is_button_pressed(unsigned char index);    /* one event per press */
unsigned char is_button_held(unsigned char index);       /* debounced level     */
unsigned char is_button_pressed_1s(unsigned char index); /* held for >= 1 s     */

#endif /* INC_INPUT_READING_H_ */
