/*
 *	Real Time Clock Initialization
 */

#include <htc.h>

#define AM 0
#define PM 1

// Timer Configuration...LSB 0000
near char A1_ON, A2_ON, A3_ON;

// Set start time here
#define START_H 12
#define START_M 0
#define START_S 0
#define START_AP AM
#define ALARM_PIN RC2

// Sanity check on initial start time
#if START_H > 12 || START_H < 1
#error Invalid hour specification (START_H exceeds range = 1-12)
#endif
#if START_M > 59 || START_M < 0
#error Invalid minute specification (START_M exceeds range = 0-59)
#endif
#if START_S > 59 || START_S < 0
#error Invalid second specification (START_S exceeds range = 0-59)
#endif

volatile near unsigned char tickCounter;
volatile near unsigned char newSecond;

near char hours;
near char minutes;
near char seconds;
near bit ampm;

near char a1_hours=0;
near char a1_minutes=0;
near bit a1_ampm=AM;
near int a1_duration;

near char a2_hours=0;
near char a2_minutes=0;
near bit a2_ampm=AM;
near int a2_duration;

near char a3_hours=0;
near char a3_minutes=0;
near bit a3_ampm=AM;
near int a3_duration;
