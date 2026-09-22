//menu.h
#include <htc.h>
#include "lcd.h"
#include "kyb.h"

#define MENU_IDLE 0
#define MENU_MANUAL 1
#define MENU_OPTIONS 2
#define MENU_SET_TIME 3
#define MENU_SET_A1 4
#define MENU_SET_A2 5
#define MENU_SET_A3 6

unsigned char MENU_MODE, key;
char mkey;

void menu_init() {
  MENU_MODE = MENU_SET_TIME;
  mkey = 1;
}

void menu() {
  switch (MENU_MODE) {
	  case MENU_IDLE: {
		switch (key) {
			case KEY_MENU:
			  MENU_MODE = MENU_OPTIONS;
			  break;
			case KEY_MSTART:
			  MENU_MODE = MENU_MANUAL;
			  break;
			case KEY_MSTOP:
			  MENU_MODE = MENU_IDLE;
			  break;
			}
		break;
	  }
	  case MENU_MANUAL: {
		switch (key) {
			case KEY_MSTOP:
			  MENU_MODE = MENU_IDLE;
			  break;
		}
		break;
	  }
	  case MENU_OPTIONS: {
		__delay_ms(60);
		switch (key) {
			case KEY_LEFT:
			  mkey--;
			  if (mkey < 1) mkey = 1;
			  break;
			case KEY_RIGHT:
			  mkey++;
			  if (mkey > 4) mkey = 4;
			  break;
			case KEY_SET: {
			  switch (mkey) {
				  case 1:
					MENU_MODE = MENU_SET_TIME;
					break;
				  case 2:
					MENU_MODE = MENU_SET_A1;
					break;
				  case 3:
					MENU_MODE = MENU_SET_A2;
					break;
				  case 4:
					MENU_MODE = MENU_SET_A3;
					break;
				}
			}
			break;
			case KEY_MSTOP:
			  MENU_MODE = MENU_IDLE;
			  break;
		}
		break;
	  }
	  case MENU_SET_TIME: {
		__delay_ms(60);
		if (key == KEY_MSTOP) {
		  MENU_MODE = MENU_OPTIONS;
		  mkey = 1;
		}
		break;
	  }
	  case MENU_SET_A1: {
		__delay_ms(60);
		if (key == KEY_MSTOP) {
		  MENU_MODE = MENU_OPTIONS;
		  mkey = 2;
		}
		break;
	  }
	  case MENU_SET_A2: {
		__delay_ms(60);
		if (key == KEY_MSTOP) {
		  MENU_MODE = MENU_OPTIONS;
		  mkey = 3;
		}
		break;
	  }
	  case MENU_SET_A3: {
		__delay_ms(60);
		if (key == KEY_MSTOP) {
		  MENU_MODE = MENU_OPTIONS;
		  mkey = 4;
		}
		break;
	  }
  }
  //end of menu select
  //begin lcd render
  switch (MENU_MODE) {
	  case MENU_IDLE: {
		lcd_clear();
		lcd_goto(0);
		lcd_puts("    SECUTRIX    ");
		lcd_goto(0x40);
		lcd_puts("  CURRENT TIME  ");
		break;
	  }
	  case MENU_MANUAL: {
		lcd_clear();
		lcd_goto(0);
		lcd_puts("  MANUAL MODE!  ");
		break;
	  }
	  case MENU_OPTIONS: {
		lcd_clear();
		lcd_goto(0);
		switch (mkey) {
			case 1:
			  lcd_puts("   SET TIME?   >");
			  break;
			case 2:
			  lcd_puts("<  SET ALARM1? >");
			  break;
			case 3:
			  lcd_puts("<  SET ALARM2? >");
			  break;
			case 4:
			  lcd_puts("<  SET ALARM3?  ");
			  break;
		}
		break;
	  }
	  case MENU_SET_TIME: {
		lcd_clear();
		lcd_goto(0);
		lcd_puts("SET TIME MENU");
		break;
	  }
	  case MENU_SET_A1: {
		lcd_clear();
		lcd_goto(0);
		lcd_puts("SET ALARM1 MENU");
		break;
	  }
	  case MENU_SET_A2: {
		lcd_clear();
		lcd_goto(0);
		lcd_puts("SET ALARM2 MENU");
		break;
	  }
	  case MENU_SET_A3: {
		lcd_clear();
		lcd_goto(0);
		lcd_puts("SET ALARM3 MENU");
		break;
	  }
	}
}
