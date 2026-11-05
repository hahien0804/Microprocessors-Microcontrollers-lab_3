/*
 * led_display.h
 *
 * Output side: 12 traffic LEDs (6 signals, 2 LEDs each) and
 * 4 multiplexed common-anode 7-segment LEDs.
 */
#ifndef INC_LED_DISPLAY_H_
#define INC_LED_DISPLAY_H_

#include "main.h"

#define ROAD_1          0   /* North - South: LED_xxx_1, 7-seg EN0 EN1 */
#define ROAD_2          1   /* West  - East : LED_xxx_2, 7-seg EN2 EN3 */

#define LIGHT_OFF       0
#define LIGHT_RED       1
#define LIGHT_AMBER     2
#define LIGHT_GREEN     3

#define SCAN_7SEG_MS    2   /* 8 ms frame -> 125 Hz */

void led_display_init(void);
void setRoadLight(int road, int light);          /* only this colour is on      */
void setColorBothRoads(int light, uint8_t on);   /* blinking in modify modes    */
void setDisplayValues(int left, int right);      /* 2 digits + 2 digits (0..99) */
void led_display_run(void);                      /* call in the main loop       */

#endif /* INC_LED_DISPLAY_H_ */
