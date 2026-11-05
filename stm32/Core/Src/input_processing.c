/* Road 1 uses the independently selected R/Y/G durations.
 * Road 2 is red during road 1 green/amber. During road 1 red,
 * road 2 uses green for max(R-Y,0), then amber for min(R,Y).
 * This coordination policy supports all 1..99 values without conflicting
 * greens or blocking the required 1 -> 2 -> 3 -> 4 -> 1 mode cycle.
 * The handout does not specify the side-road timing policy.
 */
#include "input_processing.h"
#include "input_reading.h"
#include "led_display.h"
#include "timer.h"

#define BLINK_HALF_PERIOD_MS  250   /* toggle every 250 ms -> 2 Hz          */
#define AUTO_INC_MS           500   /* +1 every 500 ms while BUTTON_2 held  */
#define MIN_DURATION          1
#define MAX_DURATION          99

enum Mode  { MODE_NORMAL = 1, MODE_RED = 2, MODE_AMBER = 3, MODE_GREEN = 4 };
enum Phase { PH_GREEN1_RED2, PH_AMBER1_RED2, PH_RED1_GREEN2, PH_RED1_AMBER2 };

/* Saved durations; a temporarily inconsistent set is allowed while editing. */
static int durationRed   = 5;
static int durationAmber = 2;
static int durationGreen = 3;

static enum Mode  mode  = MODE_NORMAL;
static enum Phase phase = PH_GREEN1_RED2;
static int     phaseRemain = 0;   /* seconds left in the current phase */
static int     tempValue   = 0;   /* value being modified (modes 2-4)  */
static uint8_t blinkOn     = 0;
static uint8_t autoIncRunning = 0;

/* ======================= TRAFFIC FSM ======================= */
static void traffic_output(void) {
  switch (phase) {
    case PH_GREEN1_RED2: setRoadLight(ROAD_1, LIGHT_GREEN); setRoadLight(ROAD_2, LIGHT_RED);   break;
    case PH_AMBER1_RED2: setRoadLight(ROAD_1, LIGHT_AMBER); setRoadLight(ROAD_2, LIGHT_RED);   break;
    case PH_RED1_GREEN2: setRoadLight(ROAD_1, LIGHT_RED);   setRoadLight(ROAD_2, LIGHT_GREEN); break;
    case PH_RED1_AMBER2: setRoadLight(ROAD_1, LIGHT_RED);   setRoadLight(ROAD_2, LIGHT_AMBER); break;
  }
}

/* time left of the colour currently shown on each road */
static void traffic_display(void) {
  int t1 = 0, t2 = 0;
  switch (phase) {
    case PH_GREEN1_RED2: t1 = phaseRemain;                 t2 = phaseRemain + durationAmber; break;
    case PH_AMBER1_RED2: t1 = phaseRemain;                 t2 = phaseRemain;                 break;
    case PH_RED1_GREEN2: t1 = phaseRemain + durationAmber; t2 = phaseRemain;                 break;
    case PH_RED1_AMBER2: t1 = phaseRemain;                 t2 = phaseRemain;                 break;
  }
  setDisplayValues(t1, t2);
}

static void traffic_enterPhase(enum Phase p) {
  phase = p;
  switch (p) {
    case PH_GREEN1_RED2: phaseRemain = durationGreen; break;
    case PH_AMBER1_RED2: phaseRemain = durationAmber; break;
    case PH_RED1_GREEN2: phaseRemain = durationRed - durationAmber; break;
    default: phaseRemain = durationRed < durationAmber ? durationRed : durationAmber; break;
  }
  traffic_output();
  traffic_display();
}

static void traffic_start(void) {
  mode = MODE_NORMAL;
  traffic_enterPhase(PH_GREEN1_RED2);
  setTimer(T_SECOND, 1000);
}

static void traffic_tick(void) {          /* called every second */
  phaseRemain--;
  if (phaseRemain <= 0) {
    enum Phase next = (enum Phase)((phase + 1) % 4);
    if (next == PH_RED1_GREEN2 && durationRed <= durationAmber)
      next = PH_RED1_AMBER2; /* no room for a side-road green interval */
    traffic_enterPhase(next);
  } else {
    traffic_display();
  }
}

/* ======================= MODIFY MODES ======================= */
static int colorOfMode(enum Mode m) {
  switch (m) {
    case MODE_RED:   return LIGHT_RED;
    case MODE_AMBER: return LIGHT_AMBER;
    case MODE_GREEN: return LIGHT_GREEN;
    default:         return LIGHT_OFF;
  }
}

static int durationOfMode(enum Mode m) {
  switch (m) {
    case MODE_RED:   return durationRed;
    case MODE_AMBER: return durationAmber;
    case MODE_GREEN: return durationGreen;
    default:         return 0;
  }
}

/* Save exactly the selected main-road duration, without changing other colors. */
static int saveDuration(enum Mode m, int v) {
  if (v < MIN_DURATION || v > MAX_DURATION) return durationOfMode(m);
  switch (m) {
    case MODE_RED: durationRed = v; break;
    case MODE_AMBER: durationAmber = v; break;
    case MODE_GREEN: durationGreen = v; break;
    default: return 0;
  }
  return v;
}

static void modify_display(void) {
  setDisplayValues(tempValue, (int)mode);    /* value | mode number */
}

static void modify_enter(enum Mode m) {
  mode = m;
  tempValue = durationOfMode(m);
  blinkOn = 1;
  setColorBothRoads(colorOfMode(m), blinkOn);
  setTimer(T_BLINK, BLINK_HALF_PERIOD_MS);
  autoIncRunning = 0;
  modify_display();
}

static void increaseValue(void) {
  tempValue = (tempValue >= MAX_DURATION) ? MIN_DURATION : tempValue + 1;
  modify_display();
}

/* ======================= MAIN FSM ======================= */
void fsm_init(void) {
  traffic_start();
}

void fsm_for_input_processing(void) {
  /* read every event once per loop so no old press is left behind */
  uint8_t pressMode = is_button_pressed(BUTTON_MODE);
  uint8_t pressInc  = is_button_pressed(BUTTON_INC);
  uint8_t pressSet  = is_button_pressed(BUTTON_SET);

  switch (mode) {
    case MODE_NORMAL:
      if (isTimerExpired(T_SECOND)) {
        setTimer(T_SECOND, 1000);
        traffic_tick();
      }
      if (pressMode) modify_enter(MODE_RED);
      break;

    case MODE_RED:
    case MODE_AMBER:
    case MODE_GREEN:
      /* 2 Hz blinking of the selected colour */
      if (isTimerExpired(T_BLINK)) {
        setTimer(T_BLINK, BLINK_HALF_PERIOD_MS);
        blinkOn = !blinkOn;
        setColorBothRoads(colorOfMode(mode), blinkOn);
      }

      /* BUTTON_2: +1 per press, auto repeat after holding 1 s */
      if (pressInc) increaseValue();
      if (is_button_pressed_1s(BUTTON_INC)) {
        if (!autoIncRunning) {
          autoIncRunning = 1;
          setTimer(T_AUTO_INC, AUTO_INC_MS);
        } else if (isTimerExpired(T_AUTO_INC)) {
          setTimer(T_AUTO_INC, AUTO_INC_MS);
          increaseValue();
        }
      } else {
        autoIncRunning = 0;
      }

      /* BUTTON_3: save */
      if (pressSet) {
        tempValue = saveDuration(mode, tempValue);
        modify_display();
      }

      /* BUTTON_1: next mode (an unsaved value is discarded) */
      if (pressMode) {
        if (mode == MODE_RED)        modify_enter(MODE_AMBER);
        else if (mode == MODE_AMBER) modify_enter(MODE_GREEN);
        else traffic_start(); /* required mode 4 -> mode 1 transition */
      }
      break;
  }
}
