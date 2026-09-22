#include <htc.h>
#include "lcd.h"
#include "rtc.h"

__CONFIG(PROTECT & XT & WDTDIS & PWRTDIS & BORDIS & LVPDIS /*& DUNPROT*/ );

#ifndef _XTAL_FREQ
// Unless specified elsewhere, 4MHz system frequency is assumed
#define _XTAL_FREQ 4000000
#endif

//kyb defs
#define TONE1 500
#define TONE_PIN RC0

//character defs
#define LEFTARW 0x00
#define RIGHTARW 0x01
#define UPARW 0x02
#define DOWNARW 0x03
#define BELL 0x04
#define BATT 0x05

#define KEY_NOKEY 0x0
#define KEY_MENU 0x01
#define KEY_UP 0x02
#define KEY_LEFT 0x03
#define KEY_SET 0x04
#define KEY_RIGHT 0x05
#define KEY_MSTART 0x06
#define KEY_DOWN 0x07
#define KEY_MSTOP 0x08

#define MENU_MASK 0x01
#define UP_MASK 0x02
#define LEFT_MASK 0x04
#define SET_MASK 0x08
#define RIGHT_MASK 0x10
#define MSTR_MASK 0x20
#define DOWN_MASK 0x40
#define MSTP_MASK 0x80

//menu defs
#define MENU_IDLE 0x0
#define MENU_MANUAL 0x01
#define MENU_OPTIONS 0x02
#define MENU_SET_TIME 0x03
#define MENU_SET_A1 0x04
#define MENU_SET_A2 0x05
#define MENU_SET_A3 0x06

#define MOTOR_OFF 0
#define MOTOR_ON 1
#define motor_stat RC2

unsigned near char MENU_MODE, key, mkey, cursorpos, duration, mincntr, alflag;
near bit lcd_change, buz;

//buzzer function
void buzz(void) {
    unsigned char tone = TONE1;
    while (tone--) {
        __delay_us(50); // tone generation
        TONE_PIN ^= 1; // generate buzz
    }
}

//draws time supplied
void showtime(char ahours, char aminutes, char aseconds, char aampm, char set) {
    lcd_goto(0x40);
    lcd_puts("  ");
    if (set) {
        // Print hours
        if (ahours / 10)
            lcd_putch('1');
        else
            lcd_putch('0');
        lcd_putch((ahours % 10) + '0');
        lcd_putch(':');
        // print minutes
        lcd_putch((aminutes / 10) + '0');
        lcd_putch((aminutes % 10) + '0');
        // print seconds
        lcd_putch(':');
        lcd_putch((aseconds / 10) + '0');
        lcd_putch((aseconds % 10) + '0');
        lcd_putch(' ');
        if (aampm)
            lcd_putch('P');
        else
            lcd_putch('A');
        lcd_putch('M');
    } else {
        lcd_puts("--:--:-- --");
    }
    lcd_puts("    ");
}

unsigned char mmkey, mcurpos, tset;

void drawamenu(char stat) {
    switch (mmkey) {
    case 1: {
        lcd_puts("   ON ALARM");
        switch (stat) {
        case 1:
            lcd_puts("1?   ");
            lcd_goto(0x47);
            if (A1_ON) lcd_puts("YES");
            else lcd_puts("NO");
            break;
        case 2:
            lcd_puts("2?   ");
            lcd_goto(0x47);
            if (A2_ON) lcd_puts("YES");
            else lcd_puts("NO");
            break;
        case 3:
            lcd_puts("3?   ");
            lcd_goto(0x47);
            if (A3_ON) lcd_puts("YES");
            else lcd_puts("NO");
            break;
        }
        lcd_goto(0x4F);
        lcd_putch(DOWNARW);
        lcd_goto(0x47);
        break;
    }
    case 2: {
        lcd_puts("   SET ALARM:");
        lcd_goto(0x0f);
        lcd_putch(UPARW);
        switch (stat) {
        case 1:
            showtime(a1_hours, a1_minutes, 0, a1_ampm, A1_ON);
            break;
        case 2:
            showtime(a2_hours, a2_minutes, 0, a2_ampm, A2_ON);
            break;
        case 3:
            showtime(a3_hours, a3_minutes, 0, a3_ampm, A3_ON);
            break;
        }
        lcd_goto(0x4F);
        lcd_putch(DOWNARW);
        switch (mcurpos) {
        case 0:
            lcd_goto(0x50);
            break;
        case 1:
            lcd_goto(0x43);
            break;
        case 2:
            lcd_goto(0x46);
            break;
        case 3:
            lcd_goto(0x4C);
            break;
        }
        break;
    }
    case 3: {
        lcd_puts(" SET DURATION:");
        lcd_goto(0x0f);
        lcd_putch(UPARW);
        lcd_goto(0x44);
        switch (stat) {
        case 1:
            lcd_putch((a1_duration / 100) + '0');
            if (a1_duration >= 100) lcd_putch(((a1_duration - 100) / 10) + '0');
            else lcd_putch((a1_duration / 10) + '0');
            lcd_putch((a1_duration % 10) + '0');
            break;
        case 2:
            lcd_putch((a2_duration / 100) + '0');
            if (a2_duration >= 100) lcd_putch(((a2_duration - 100) / 10) + '0');
            else lcd_putch((a2_duration / 10) + '0');
            lcd_putch((a2_duration % 10) + '0');
            break;
        case 3:
            lcd_putch((a3_duration / 100) + '0');
            if (a3_duration >= 100) lcd_putch(((a3_duration - 100) / 10) + '0');
            else lcd_putch((a3_duration / 10) + '0');
            lcd_putch((a3_duration % 10) + '0');
            break;
        }
        lcd_puts(" Mins");
        if (tset == 2) lcd_goto(0x44);
        else lcd_goto(0x50);
        break;
    }
    }
}

void alarmmenu(char stat, char akey) {
    char a_hr, a_min, a_ampm, a_dur, a_on;
    switch (stat) {
    case 1: {
        a_hr = a1_hours;
        a_min = a1_minutes;
        a_on = A1_ON;
        a_ampm = a1_ampm;
        a_dur = a1_duration;
    }
    break;
    case 2: {
        a_hr = a2_hours;
        a_min = a2_minutes;
        a_on = A2_ON;
        a_ampm = a2_ampm;
        a_dur = a2_duration;
    }
    break;
    case 3: {
        a_hr = a3_hours;
        a_min = a3_minutes;
        a_on = A3_ON;
        a_ampm = a3_ampm;
        a_dur = a3_duration;
    }
    break;
    }
    switch (akey) {
    case KEY_UP: {
        switch (tset) {
        case 0:
            mmkey--;
            if (mmkey < 1) mmkey = 1;
            lcd_change = 1;
            buz = 1;
            break;
        case 1:
            switch (mcurpos) {
            case 1:
                a_hr++;
                if (a_hr > 12) a_hr = 1;
                lcd_change = 1;
                buz = 1;
                break;
            case 2:
                a_min++;
                if (a_min > 59) a_min = 0;
                lcd_change = 1;
                buz = 1;
                break;
            case 3:
                a_ampm = 1;
                lcd_change = 1;
                buz = 1;
                break;
            }
            break;
        case 2:
            a_dur++;
            if (a_dur > 180) a_dur = 0;
            lcd_change = 1;
            buz = 1;
            break;
        }
        break;
    }
    case KEY_DOWN: {
        switch (tset) {
        case 0:
            mmkey++;
            if (mmkey > 3) mmkey = 3;
            lcd_change = 1;
            buz = 1;
            break;
        case 1:
            switch (mcurpos) {
            case 1:
                a_hr--;
                if (a_hr < 1) a_hr = 12;
                lcd_change = 1;
                buz = 1;
                break;
            case 2:
                a_min--;
                if (a_min <= 0) a_min = 59;
                lcd_change = 1;
                buz = 1;
                break;
            case 3:
                a_ampm = 0;
                lcd_change = 1;
                buz = 1;
                break;
            }
            break;
        case 2:
            a_dur--;
            if (a_dur <= 0) a_dur = 180;
            lcd_change = 1;
            buz = 1;
            break;
        }
        break;
    }
    case KEY_LEFT:
        if (tset == 1) {
            mcurpos--;
            if (mcurpos < 1) mcurpos = 1;
            lcd_change = 1;
            buz = 1;
        }
        break;
    case KEY_RIGHT:
        if (tset == 1) {
            mcurpos++;
            if (mcurpos > 3) mcurpos = 3;
            lcd_change = 1;
            buz = 1;
        }
        break;
    case KEY_MSTOP:
        if (tset) tset = 0;
        else {
            MENU_MODE = MENU_OPTIONS;
            mkey = stat + 1;
            lcd_change = 1;
            buz = 1;
        }
        break;
    case KEY_SET: {
        switch (mmkey) {
        case 1:
            tset = 0;
            if (a_on) a_on = 0;
            else a_on = 1;
            lcd_change = 1;
            buz = 1;
            break;
        case 2:
            tset = 1;
            mcurpos = 1;
            lcd_change = 1;
            buz = 1;
            break;
        case 3:
            tset = 2;
            lcd_change = 1;
            buz = 1;
            break;
        }
    }
    }
    switch (stat) {
    case 1: {
        a1_hours = a_hr;
        a1_minutes = a_min;
        A1_ON = a_on;
        a1_ampm = a_ampm;
        a1_duration = a_dur;
    }
    break;
    case 2: {
        a2_hours = a_hr;
        a2_minutes = a_min;
        A2_ON = a_on;
        a2_ampm = a_ampm;
        a2_duration = a_dur;
    }
    break;
    case 3: {
        a3_hours = a_hr;
        a3_minutes = a_min;
        A3_ON = a_on;
        a3_ampm = a_ampm;
        a3_duration = a_dur;
    }
    break;
    }
}

//interrupt function
void interrupt isr(void) {
    //Timer 2 interrupt fn
    if ((TMR2IE) && (TMR2IF)) {
        //interrupt period is 40 mSec, 25 interrupts = 1 Sec
        if (++tickCounter == 25) {
            tickCounter = 0;
            newSecond++; // Notify a second has accumulated
        }
        TMR2IF = 0; // clear event flag
    }
}

void main(void) {
    //gen init
    MENU_MODE = MENU_SET_TIME;
    mkey = 0x01;
    key = KEY_NOKEY;
    cursorpos = 0x01;
    lcd_change = 1;
    buz = 1;
    mmkey = 1;
    tset = 0;
    motor_stat = MOTOR_OFF;
    duration = 0;
    mincntr = 0;
    alflag = 0;

    //kyb init starts
    TRISD = 0xff;
    PSPMODE = 0;

    //buzzer init starts
    TRISC = 0;
    PORTC = 0;

    //lcd init begins
    lcd_init();

    //insert new characters into LCD
    static unsigned char newchars[] = {
        0b10000010,
        0b10000110,
        0b10001110,
        0b10011110,
        0b10001110,
        0b10000110,
        0b10000010,
        0b10000000,
        0b10001000,
        0b10001100,
        0b10001110,
        0b10001111,
        0b10001110,
        0b10001100,
        0b10001000,
        0b10000000,
        0b10000000,
        0b10000100,
        0b10000100,
        0b10001110,
        0b10001110,
        0b10011111,
        0b10011111,
        0b10000000,
        0b10000000,
        0b10011111,
        0b10011111,
        0b10001110,
        0b10001110,
        0b10000100,
        0b10000100,
        0b10000000,
        0b10000000,
        0b10000100,
        0b10001110,
        0b10001110,
        0b10001110,
        0b10011111,
        0b10000100,
        0b10000000,
        0b10001110,
        0b10011111,
        0b10010111,
        0b10010111,
        0b10010111,
        0b10010111,
        0b10010111,
        0b10011111
    };
    LCD_RS = 0;
    LCD_RW = 0;
    lcd_write(0x40);
    lcd_puts(newchars);
    LCD_RS = 0;
    lcd_write(0x80);
    //end of insertion 

    //RTC init begins
    PIE1 = 0b00000010;
    INTCON = 0b01000000;
    T2CON = 0b01001110;
    PR2 = 0b11111001;
    hours = START_H;
    minutes = START_M;
    seconds = START_S;
    ampm = START_AP;
    A1_ON = 0;
    A2_ON = 0;
    A3_ON = 0;
    ALARM_PIN = 0;
    //RTC init ends

    GIE = 0;

    while (1) {
        //get keys
        key = PORTD;
        switch (key) {
        case 0x00:
            key = KEY_NOKEY;
            break;
        case MENU_MASK:
            key = KEY_MENU;
            break;
        case UP_MASK:
            key = KEY_UP;
            break;
        case LEFT_MASK:
            key = KEY_LEFT;
            break;
        case SET_MASK:
            key = KEY_SET;
            break;
        case RIGHT_MASK:
            key = KEY_RIGHT;
            break;
        case MSTR_MASK:
            key = KEY_MSTART;
            break;
        case DOWN_MASK:
            key = KEY_DOWN;
            break;
        case MSTP_MASK:
            key = KEY_MSTOP;
            break;
        }

        //update time if fresh sec
        if (newSecond) {
            newSecond--;
            if (++seconds > 59) {
                seconds = 0;
                if (A1_ON)
                    if (a1_hours == hours && a1_minutes == (minutes + 1) && a1_ampm == ampm) {
                        mincntr = 0;
                        duration = a1_duration;
                        motor_stat = MOTOR_ON;
                        alflag = 1;
                    }
                if (A2_ON)
                    if (a2_hours == hours && a2_minutes == (minutes + 1) && a2_ampm == ampm) {
                        mincntr = 0;
                        duration = a2_duration;
                        motor_stat = MOTOR_ON;
                        alflag = 2;
                    }
                if (A3_ON)
                    if (a3_hours == hours && a3_minutes == (minutes + 1) && a3_ampm == ampm) {
                        mincntr = 0;
                        duration = a3_duration;
                        motor_stat = MOTOR_ON;
                        alflag = 3;
                    }
                if (alflag) ++mincntr;
                if (++minutes > 59) {
                    minutes = 0;
                    hours++;
                    if (hours == 12)
                        ampm ^= 1;
                    if (hours > 12)
                        hours = 1;
                }
            }
            if (MENU_MODE == MENU_IDLE || MENU_MODE == MENU_OPTIONS) lcd_change = 1;
        }
        //set the mode
        __delay_ms(60);
        if (key) {
            switch (MENU_MODE) {
            case MENU_IDLE: {
                switch (key) {
                case KEY_MENU:
                    MENU_MODE = MENU_OPTIONS;
                    mkey = 1;
                    lcd_change = 1;
                    buz = 1;
                    break;
                case KEY_MSTART:
                    MENU_MODE = MENU_MANUAL;
                    motor_stat = MOTOR_ON;
                    lcd_change = 1;
                    buz = 1;
                    break;
                case KEY_MSTOP:
                    MENU_MODE = MENU_IDLE;
                    motor_stat = MOTOR_OFF;
                    alflag = 0;
                    lcd_change = 1;
                    buz = 1;
                    break;
                }
                break;
            }
            case MENU_MANUAL: {
                if (key == KEY_MSTOP) {
                    MENU_MODE = MENU_IDLE;
                    motor_stat = MOTOR_OFF;
                    alflag = 0;
                    lcd_change = 1;
                    buz = 1;
                }
                break;
            }
            case MENU_OPTIONS: {
                __delay_ms(60);
                switch (key) {
                case KEY_LEFT:
                    mkey--;
                    if (mkey < 1) mkey = 1;
                    lcd_change = 1;
                    buz = 1;
                    break;
                case KEY_RIGHT:
                    mkey++;
                    if (mkey > 4) mkey = 4;
                    lcd_change = 1;
                    buz = 1;
                    break;
                case KEY_SET: {
                    switch (mkey) {
                    case 1:
                        MENU_MODE = MENU_SET_TIME;
                        GIE = 0;
                        cursorpos = 1;
                        lcd_change = 1;
                        mkey = 1;
                        buz = 1;
                        break;
                    case 2:
                        MENU_MODE = MENU_SET_A1;
                        mmkey = 1;
                        mcurpos = 0;
                        tset = 0;
                        lcd_change = 1;
                        buz = 1;
                        break;
                    case 3:
                        MENU_MODE = MENU_SET_A2;
                        mmkey = 1;
                        mcurpos = 0;
                        tset = 0;
                        lcd_change = 1;
                        buz = 1;
                        break;
                    case 4:
                        MENU_MODE = MENU_SET_A3;
                        mmkey = 1;
                        mcurpos = 0;
                        tset = 0;
                        lcd_change = 1;
                        buz = 1;
                        break;
                    }
                }
                break;
                case KEY_MSTOP:
                    MENU_MODE = MENU_IDLE;
                    lcd_change = 1;
                    buz = 1;
                    break;
                }
                break;
            }
            case MENU_SET_TIME: {
                __delay_ms(60);
                switch (key) {
                case KEY_LEFT:
                    cursorpos--;
                    if (cursorpos < 1) cursorpos = 1;
                    lcd_change = 1;
                    buz = 1;
                    break;
                case KEY_RIGHT:
                    cursorpos++;
                    if (cursorpos > 4) cursorpos = 4;
                    lcd_change = 1;
                    buz = 1;
                    break;
                case KEY_UP: {
                    switch (cursorpos) {
                    case 0x01:
                        hours++;
                        if (hours > 12) hours = 1;
                        break;
                    case 0x02:
                        minutes++;
                        if (minutes > 59) minutes = 0;
                        break;
                    case 0x03:
                        seconds++;
                        if (seconds > 59) seconds = 0;
                        break;
                    case 0x04:
                        ampm = 1;
                        break;
                    }
                    lcd_change = 1;
                    buz = 1;
                    break;
                }
                case KEY_DOWN: {
                    switch (cursorpos) {
                    case 0x01:
                        hours--;
                        if (hours < 1) hours = 12;
                        break;
                    case 0x02:
                        minutes--;
                        if (minutes <= 0) minutes = 59;
                        break;
                    case 0x03:
                        seconds--;
                        if (seconds <= 0) seconds = 59;
                        break;
                    case 0x04:
                        ampm = 0;
                        break;
                    }
                    lcd_change = 1;
                    buz = 1;
                    break;
                }
                case KEY_MSTOP:
                    MENU_MODE = MENU_OPTIONS;
                    mkey = 1;
                    lcd_change = 1;
                    GIE = 1;
                    buz = 1;
                    break;
                }
                break;
            }
            case MENU_SET_A1: {
                __delay_ms(60);
                alarmmenu(1, key);
                break;
            }
            case MENU_SET_A2: {
                __delay_ms(60);
                alarmmenu(2, key);
                break;
            }
            case MENU_SET_A3: {
                __delay_ms(60);
                alarmmenu(3, key);
                break;
            }
            }
        }
        //switch off da moto
        if (alflag)
            if (duration < mincntr) {
                motor_stat = MOTOR_OFF;
                alflag = 0;
            }

        //make a buzz
        if (buz) {
            buzz();
            buz = 0;
        }

        //end of menu select
        //begin lcd render
        if (lcd_change) {
            lcd_clear();
            lcd_goto(0x0);
            switch (MENU_MODE) {
            case MENU_IDLE: {
                lcd_puts("    SECUTRIX  ");
                switch (alflag) {
                case 0:
                    lcd_puts("  ");
                    break;
                case 1:
                    lcd_putch(BELL);
                    lcd_putch('1');
                    break;
                case 2:
                    lcd_putch(BELL);
                    lcd_putch('2');
                    break;
                case 3:
                    lcd_putch(BELL);
                    lcd_putch('3');
                    break;
                }
                showtime(hours, minutes, seconds, ampm, 1);
                break;
            }
            case MENU_MANUAL: {
                lcd_puts("    SECUTRIX    ");
                lcd_goto(0x40);
                lcd_puts("  MANUAL MODE!  ");
                break;
            }
            case MENU_OPTIONS: {
                switch (mkey) {
                case 0x01: {
                    lcd_puts("   SET TIME?   ");
                    lcd_putch(RIGHTARW);
                    if (lcd_change) {
                        showtime(hours, minutes, seconds, ampm, 1);
                    }
                    break;
                }
                case 0x02: {
                    lcd_putch(LEFTARW);
                    lcd_puts("  SET ALARM1? ");
                    lcd_putch(RIGHTARW);
                    showtime(a1_hours, a1_minutes, 0, a1_ampm, A1_ON);
                    break;
                }
                case 0x03: {
                    lcd_putch(LEFTARW);
                    lcd_puts("  SET ALARM2? ");
                    lcd_putch(RIGHTARW);
                    showtime(a2_hours, a2_minutes, 0, a2_ampm, A2_ON);
                    break;
                }
                case 0x04: {
                    lcd_putch(LEFTARW);
                    lcd_puts("  SET ALARM3?  ");
                    showtime(a3_hours, a3_minutes, 0, a3_ampm, A3_ON);
                    break;
                }
                }
            }
            break;
            case MENU_SET_TIME: {
                lcd_puts("   SET TIME:   ");
                showtime(hours, minutes, seconds, ampm, 1);
                switch (cursorpos) {
                case 1:
                    lcd_goto(0x43);
                    break;
                case 2:
                    lcd_goto(0x46);
                    break;
                case 3:
                    lcd_goto(0x49);
                    break;
                case 4:
                    lcd_goto(0x4C);
                    break;
                }
                break;
            }
            case MENU_SET_A1: {
                drawamenu(1);
                break;
            }
            case MENU_SET_A2: {
                drawamenu(2);
                break;
            }
            case MENU_SET_A3: {
                drawamenu(3);
                break;
            }
            }
            lcd_change = 0;
        }
    }
}#include <htc.h>

#include "lcd.h"

#include "rtc.h"

__CONFIG(PROTECT & XT & WDTDIS & PWRTDIS & BORDIS & LVPDIS /*& DUNPROT*/ );

#ifndef _XTAL_FREQ
// Unless specified elsewhere, 4MHz system frequency is assumed
#define _XTAL_FREQ 4000000
#endif

//kyb defs
#define TONE1 500
#define TONE_PIN RC0

//character defs
#define LEFTARW 0x00
#define RIGHTARW 0x01
#define UPARW 0x02
#define DOWNARW 0x03
#define BELL 0x04
#define BATT 0x05

#define KEY_NOKEY 0x0
#define KEY_MENU 0x01
#define KEY_UP 0x02
#define KEY_LEFT 0x03
#define KEY_SET 0x04
#define KEY_RIGHT 0x05
#define KEY_MSTART 0x06
#define KEY_DOWN 0x07
#define KEY_MSTOP 0x08

#define MENU_MASK 0x01
#define UP_MASK 0x02
#define LEFT_MASK 0x04
#define SET_MASK 0x08
#define RIGHT_MASK 0x10
#define MSTR_MASK 0x20
#define DOWN_MASK 0x40
#define MSTP_MASK 0x80

//menu defs
#define MENU_IDLE 0x0
#define MENU_MANUAL 0x01
#define MENU_OPTIONS 0x02
#define MENU_SET_TIME 0x03
#define MENU_SET_A1 0x04
#define MENU_SET_A2 0x05
#define MENU_SET_A3 0x06

#define MOTOR_OFF 0
#define MOTOR_ON 1
#define motor_stat RC2

unsigned near char MENU_MODE, key, mkey, cursorpos, duration, mincntr, alflag;
near bit lcd_change, buz;

//buzzer function
void buzz(void) {
    unsigned char tone = TONE1;
    while (tone--) {
        __delay_us(50); // tone generation
        TONE_PIN ^= 1; // generate buzz
    }
}

//draws time supplied
void showtime(char ahours, char aminutes, char aseconds, char aampm, char set) {
    lcd_goto(0x40);
    lcd_puts("  ");
    if (set) {
        // Print hours
        if (ahours / 10)
            lcd_putch('1');
        else
            lcd_putch('0');
        lcd_putch((ahours % 10) + '0');
        lcd_putch(':');
        // print minutes
        lcd_putch((aminutes / 10) + '0');
        lcd_putch((aminutes % 10) + '0');
        // print seconds
        lcd_putch(':');
        lcd_putch((aseconds / 10) + '0');
        lcd_putch((aseconds % 10) + '0');
        lcd_putch(' ');
        if (aampm)
            lcd_putch('P');
        else
            lcd_putch('A');
        lcd_putch('M');
    } else {
        lcd_puts("--:--:-- --");
    }
    lcd_puts("    ");
}

unsigned char mmkey, mcurpos, tset;

void drawamenu(char stat) {
    switch (mmkey) {
    case 1: {
        lcd_puts("   ON ALARM");
        switch (stat) {
        case 1:
            lcd_puts("1?   ");
            lcd_goto(0x47);
            if (A1_ON) lcd_puts("YES");
            else lcd_puts("NO");
            break;
        case 2:
            lcd_puts("2?   ");
            lcd_goto(0x47);
            if (A2_ON) lcd_puts("YES");
            else lcd_puts("NO");
            break;
        case 3:
            lcd_puts("3?   ");
            lcd_goto(0x47);
            if (A3_ON) lcd_puts("YES");
            else lcd_puts("NO");
            break;
        }
        lcd_goto(0x4F);
        lcd_putch(DOWNARW);
        lcd_goto(0x47);
        break;
    }
    case 2: {
        lcd_puts("   SET ALARM:");
        lcd_goto(0x0f);
        lcd_putch(UPARW);
        switch (stat) {
        case 1:
            showtime(a1_hours, a1_minutes, 0, a1_ampm, A1_ON);
            break;
        case 2:
            showtime(a2_hours, a2_minutes, 0, a2_ampm, A2_ON);
            break;
        case 3:
            showtime(a3_hours, a3_minutes, 0, a3_ampm, A3_ON);
            break;
        }
        lcd_goto(0x4F);
        lcd_putch(DOWNARW);
        switch (mcurpos) {
        case 0:
            lcd_goto(0x50);
            break;
        case 1:
            lcd_goto(0x43);
            break;
        case 2:
            lcd_goto(0x46);
            break;
        case 3:
            lcd_goto(0x4C);
            break;
        }
        break;
    }
    case 3: {
        lcd_puts(" SET DURATION:");
        lcd_goto(0x0f);
        lcd_putch(UPARW);
        lcd_goto(0x44);
        switch (stat) {
        case 1:
            lcd_putch((a1_duration / 100) + '0');
            if (a1_duration >= 100) lcd_putch(((a1_duration - 100) / 10) + '0');
            else lcd_putch((a1_duration / 10) + '0');
            lcd_putch((a1_duration % 10) + '0');
            break;
        case 2:
            lcd_putch((a2_duration / 100) + '0');
            if (a2_duration >= 100) lcd_putch(((a2_duration - 100) / 10) + '0');
            else lcd_putch((a2_duration / 10) + '0');
            lcd_putch((a2_duration % 10) + '0');
            break;
        case 3:
            lcd_putch((a3_duration / 100) + '0');
            if (a3_duration >= 100) lcd_putch(((a3_duration - 100) / 10) + '0');
            else lcd_putch((a3_duration / 10) + '0');
            lcd_putch((a3_duration % 10) + '0');
            break;
        }
        lcd_puts(" Mins");
        if (tset == 2) lcd_goto(0x44);
        else lcd_goto(0x50);
        break;
    }
    }
}

void alarmmenu(char stat, char akey) {
    char a_hr, a_min, a_ampm, a_dur, a_on;
    switch (stat) {
    case 1: {
        a_hr = a1_hours;
        a_min = a1_minutes;
        a_on = A1_ON;
        a_ampm = a1_ampm;
        a_dur = a1_duration;
    }
    break;
    case 2: {
        a_hr = a2_hours;
        a_min = a2_minutes;
        a_on = A2_ON;
        a_ampm = a2_ampm;
        a_dur = a2_duration;
    }
    break;
    case 3: {
        a_hr = a3_hours;
        a_min = a3_minutes;
        a_on = A3_ON;
        a_ampm = a3_ampm;
        a_dur = a3_duration;
    }
    break;
    }
    switch (akey) {
    case KEY_UP: {
        switch (tset) {
        case 0:
            mmkey--;
            if (mmkey < 1) mmkey = 1;
            lcd_change = 1;
            buz = 1;
            break;
        case 1:
            switch (mcurpos) {
            case 1:
                a_hr++;
                if (a_hr > 12) a_hr = 1;
                lcd_change = 1;
                buz = 1;
                break;
            case 2:
                a_min++;
                if (a_min > 59) a_min = 0;
                lcd_change = 1;
                buz = 1;
                break;
            case 3:
                a_ampm = 1;
                lcd_change = 1;
                buz = 1;
                break;
            }
            break;
        case 2:
            a_dur++;
            if (a_dur > 180) a_dur = 0;
            lcd_change = 1;
            buz = 1;
            break;
        }
        break;
    }
    case KEY_DOWN: {
        switch (tset) {
        case 0:
            mmkey++;
            if (mmkey > 3) mmkey = 3;
            lcd_change = 1;
            buz = 1;
            break;
        case 1:
            switch (mcurpos) {
            case 1:
                a_hr--;
                if (a_hr < 1) a_hr = 12;
                lcd_change = 1;
                buz = 1;
                break;
            case 2:
                a_min--;
                if (a_min <= 0) a_min = 59;
                lcd_change = 1;
                buz = 1;
                break;
            case 3:
                a_ampm = 0;
                lcd_change = 1;
                buz = 1;
                break;
            }
            break;
        case 2:
            a_dur--;
            if (a_dur <= 0) a_dur = 180;
            lcd_change = 1;
            buz = 1;
            break;
        }
        break;
    }
    case KEY_LEFT:
        if (tset == 1) {
            mcurpos--;
            if (mcurpos < 1) mcurpos = 1;
            lcd_change = 1;
            buz = 1;
        }
        break;
    case KEY_RIGHT:
        if (tset == 1) {
            mcurpos++;
            if (mcurpos > 3) mcurpos = 3;
            lcd_change = 1;
            buz = 1;
        }
        break;
    case KEY_MSTOP:
        if (tset) tset = 0;
        else {
            MENU_MODE = MENU_OPTIONS;
            mkey = stat + 1;
            lcd_change = 1;
            buz = 1;
        }
        break;
    case KEY_SET: {
        switch (mmkey) {
        case 1:
            tset = 0;
            if (a_on) a_on = 0;
            else a_on = 1;
            lcd_change = 1;
            buz = 1;
            break;
        case 2:
            tset = 1;
            mcurpos = 1;
            lcd_change = 1;
            buz = 1;
            break;
        case 3:
            tset = 2;
            lcd_change = 1;
            buz = 1;
            break;
        }
    }
    }
    switch (stat) {
    case 1: {
        a1_hours = a_hr;
        a1_minutes = a_min;
        A1_ON = a_on;
        a1_ampm = a_ampm;
        a1_duration = a_dur;
    }
    break;
    case 2: {
        a2_hours = a_hr;
        a2_minutes = a_min;
        A2_ON = a_on;
        a2_ampm = a_ampm;
        a2_duration = a_dur;
    }
    break;
    case 3: {
        a3_hours = a_hr;
        a3_minutes = a_min;
        A3_ON = a_on;
        a3_ampm = a_ampm;
        a3_duration = a_dur;
    }
    break;
    }
}

//interrupt function
void interrupt isr(void) {
    //Timer 2 interrupt fn
    if ((TMR2IE) && (TMR2IF)) {
        //interrupt period is 40 mSec, 25 interrupts = 1 Sec
        if (++tickCounter == 25) {
            tickCounter = 0;
            newSecond++; // Notify a second has accumulated
        }
        TMR2IF = 0; // clear event flag
    }
}

void main(void) {
    //gen init
    MENU_MODE = MENU_SET_TIME;
    mkey = 0x01;
    key = KEY_NOKEY;
    cursorpos = 0x01;
    lcd_change = 1;
    buz = 1;
    mmkey = 1;
    tset = 0;
    motor_stat = MOTOR_OFF;
    duration = 0;
    mincntr = 0;
    alflag = 0;

    //kyb init starts
    TRISD = 0xff;
    PSPMODE = 0;

    //buzzer init starts
    TRISC = 0;
    PORTC = 0;

    //lcd init begins
    lcd_init();

    //insert new characters into LCD
    static unsigned char newchars[] = {
        0b10000010,
        0b10000110,
        0b10001110,
        0b10011110,
        0b10001110,
        0b10000110,
        0b10000010,
        0b10000000,
        0b10001000,
        0b10001100,
        0b10001110,
        0b10001111,
        0b10001110,
        0b10001100,
        0b10001000,
        0b10000000,
        0b10000000,
        0b10000100,
        0b10000100,
        0b10001110,
        0b10001110,
        0b10011111,
        0b10011111,
        0b10000000,
        0b10000000,
        0b10011111,
        0b10011111,
        0b10001110,
        0b10001110,
        0b10000100,
        0b10000100,
        0b10000000,
        0b10000000,
        0b10000100,
        0b10001110,
        0b10001110,
        0b10001110,
        0b10011111,
        0b10000100,
        0b10000000,
        0b10001110,
        0b10011111,
        0b10010111,
        0b10010111,
        0b10010111,
        0b10010111,
        0b10010111,
        0b10011111
    };
    LCD_RS = 0;
    LCD_RW = 0;
    lcd_write(0x40);
    lcd_puts(newchars);
    LCD_RS = 0;
    lcd_write(0x80);
    //end of insertion 

    //RTC init begins
    PIE1 = 0b00000010;
    INTCON = 0b01000000;
    T2CON = 0b01001110;
    PR2 = 0b11111001;
    hours = START_H;
    minutes = START_M;
    seconds = START_S;
    ampm = START_AP;
    A1_ON = 0;
    A2_ON = 0;
    A3_ON = 0;
    ALARM_PIN = 0;
    //RTC init ends

    GIE = 0;

    while (1) {
        //get keys
        key = PORTD;
        switch (key) {
        case 0x00:
            key = KEY_NOKEY;
            break;
        case MENU_MASK:
            key = KEY_MENU;
            break;
        case UP_MASK:
            key = KEY_UP;
            break;
        case LEFT_MASK:
            key = KEY_LEFT;
            break;
        case SET_MASK:
            key = KEY_SET;
            break;
        case RIGHT_MASK:
            key = KEY_RIGHT;
            break;
        case MSTR_MASK:
            key = KEY_MSTART;
            break;
        case DOWN_MASK:
            key = KEY_DOWN;
            break;
        case MSTP_MASK:
            key = KEY_MSTOP;
            break;
        }

        //update time if fresh sec
        if (newSecond) {
            newSecond--;
            if (++seconds > 59) {
                seconds = 0;
                if (A1_ON)
                    if (a1_hours == hours && a1_minutes == (minutes + 1) && a1_ampm == ampm) {
                        mincntr = 0;
                        duration = a1_duration;
                        motor_stat = MOTOR_ON;
                        alflag = 1;
                    }
                if (A2_ON)
                    if (a2_hours == hours && a2_minutes == (minutes + 1) && a2_ampm == ampm) {
                        mincntr = 0;
                        duration = a2_duration;
                        motor_stat = MOTOR_ON;
                        alflag = 2;
                    }
                if (A3_ON)
                    if (a3_hours == hours && a3_minutes == (minutes + 1) && a3_ampm == ampm) {
                        mincntr = 0;
                        duration = a3_duration;
                        motor_stat = MOTOR_ON;
                        alflag = 3;
                    }
                if (alflag) ++mincntr;
                if (++minutes > 59) {
                    minutes = 0;
                    hours++;
                    if (hours == 12)
                        ampm ^= 1;
                    if (hours > 12)
                        hours = 1;
                }
            }
            if (MENU_MODE == MENU_IDLE || MENU_MODE == MENU_OPTIONS) lcd_change = 1;
        }
        //set the mode
        __delay_ms(60);
        if (key) {
            switch (MENU_MODE) {
            case MENU_IDLE: {
                switch (key) {
                case KEY_MENU:
                    MENU_MODE = MENU_OPTIONS;
                    mkey = 1;
                    lcd_change = 1;
                    buz = 1;
                    break;
                case KEY_MSTART:
                    MENU_MODE = MENU_MANUAL;
                    motor_stat = MOTOR_ON;
                    lcd_change = 1;
                    buz = 1;
                    break;
                case KEY_MSTOP:
                    MENU_MODE = MENU_IDLE;
                    motor_stat = MOTOR_OFF;
                    alflag = 0;
                    lcd_change = 1;
                    buz = 1;
                    break;
                }
                break;
            }
            case MENU_MANUAL: {
                if (key == KEY_MSTOP) {
                    MENU_MODE = MENU_IDLE;
                    motor_stat = MOTOR_OFF;
                    alflag = 0;
                    lcd_change = 1;
                    buz = 1;
                }
                break;
            }
            case MENU_OPTIONS: {
                __delay_ms(60);
                switch (key) {
                case KEY_LEFT:
                    mkey--;
                    if (mkey < 1) mkey = 1;
                    lcd_change = 1;
                    buz = 1;
                    break;
                case KEY_RIGHT:
                    mkey++;
                    if (mkey > 4) mkey = 4;
                    lcd_change = 1;
                    buz = 1;
                    break;
                case KEY_SET: {
                    switch (mkey) {
                    case 1:
                        MENU_MODE = MENU_SET_TIME;
                        GIE = 0;
                        cursorpos = 1;
                        lcd_change = 1;
                        mkey = 1;
                        buz = 1;
                        break;
                    case 2:
                        MENU_MODE = MENU_SET_A1;
                        mmkey = 1;
                        mcurpos = 0;
                        tset = 0;
                        lcd_change = 1;
                        buz = 1;
                        break;
                    case 3:
                        MENU_MODE = MENU_SET_A2;
                        mmkey = 1;
                        mcurpos = 0;
                        tset = 0;
                        lcd_change = 1;
                        buz = 1;
                        break;
                    case 4:
                        MENU_MODE = MENU_SET_A3;
                        mmkey = 1;
                        mcurpos = 0;
                        tset = 0;
                        lcd_change = 1;
                        buz = 1;
                        break;
                    }
                }
                break;
                case KEY_MSTOP:
                    MENU_MODE = MENU_IDLE;
                    lcd_change = 1;
                    buz = 1;
                    break;
                }
                break;
            }
            case MENU_SET_TIME: {
                __delay_ms(60);
                switch (key) {
                case KEY_LEFT:
                    cursorpos--;
                    if (cursorpos < 1) cursorpos = 1;
                    lcd_change = 1;
                    buz = 1;
                    break;
                case KEY_RIGHT:
                    cursorpos++;
                    if (cursorpos > 4) cursorpos = 4;
                    lcd_change = 1;
                    buz = 1;
                    break;
                case KEY_UP: {
                    switch (cursorpos) {
                    case 0x01:
                        hours++;
                        if (hours > 12) hours = 1;
                        break;
                    case 0x02:
                        minutes++;
                        if (minutes > 59) minutes = 0;
                        break;
                    case 0x03:
                        seconds++;
                        if (seconds > 59) seconds = 0;
                        break;
                    case 0x04:
                        ampm = 1;
                        break;
                    }
                    lcd_change = 1;
                    buz = 1;
                    break;
                }
                case KEY_DOWN: {
                    switch (cursorpos) {
                    case 0x01:
                        hours--;
                        if (hours < 1) hours = 12;
                        break;
                    case 0x02:
                        minutes--;
                        if (minutes <= 0) minutes = 59;
                        break;
                    case 0x03:
                        seconds--;
                        if (seconds <= 0) seconds = 59;
                        break;
                    case 0x04:
                        ampm = 0;
                        break;
                    }
                    lcd_change = 1;
                    buz = 1;
                    break;
                }
                case KEY_MSTOP:
                    MENU_MODE = MENU_OPTIONS;
                    mkey = 1;
                    lcd_change = 1;
                    GIE = 1;
                    buz = 1;
                    break;
                }
                break;
            }
            case MENU_SET_A1: {
                __delay_ms(60);
                alarmmenu(1, key);
                break;
            }
            case MENU_SET_A2: {
                __delay_ms(60);
                alarmmenu(2, key);
                break;
            }
            case MENU_SET_A3: {
                __delay_ms(60);
                alarmmenu(3, key);
                break;
            }
            }
        }
        //switch off da moto
        if (alflag)
            if (duration < mincntr) {
                motor_stat = MOTOR_OFF;
                alflag = 0;
            }

        //make a buzz
        if (buz) {
            buzz();
            buz = 0;
        }

        //end of menu select
        //begin lcd render
        if (lcd_change) {
            lcd_clear();
            lcd_goto(0x0);
            switch (MENU_MODE) {
            case MENU_IDLE: {
                lcd_puts("    SECUTRIX  ");
                switch (alflag) {
                case 0:
                    lcd_puts("  ");
                    break;
                case 1:
                    lcd_putch(BELL);
                    lcd_putch('1');
                    break;
                case 2:
                    lcd_putch(BELL);
                    lcd_putch('2');
                    break;
                case 3:
                    lcd_putch(BELL);
                    lcd_putch('3');
                    break;
                }
                showtime(hours, minutes, seconds, ampm, 1);
                break;
            }
            case MENU_MANUAL: {
                lcd_puts("    SECUTRIX    ");
                lcd_goto(0x40);
                lcd_puts("  MANUAL MODE!  ");
                break;
            }
            case MENU_OPTIONS: {
                switch (mkey) {
                case 0x01: {
                    lcd_puts("   SET TIME?   ");
                    lcd_putch(RIGHTARW);
                    if (lcd_change) {
                        showtime(hours, minutes, seconds, ampm, 1);
                    }
                    break;
                }
                case 0x02: {
                    lcd_putch(LEFTARW);
                    lcd_puts("  SET ALARM1? ");
                    lcd_putch(RIGHTARW);
                    showtime(a1_hours, a1_minutes, 0, a1_ampm, A1_ON);
                    break;
                }
                case 0x03: {
                    lcd_putch(LEFTARW);
                    lcd_puts("  SET ALARM2? ");
                    lcd_putch(RIGHTARW);
                    showtime(a2_hours, a2_minutes, 0, a2_ampm, A2_ON);
                    break;
                }
                case 0x04: {
                    lcd_putch(LEFTARW);
                    lcd_puts("  SET ALARM3?  ");
                    showtime(a3_hours, a3_minutes, 0, a3_ampm, A3_ON);
                    break;
                }
                }
            }
            break;
            case MENU_SET_TIME: {
                lcd_puts("   SET TIME:   ");
                showtime(hours, minutes, seconds, ampm, 1);
                switch (cursorpos) {
                case 1:
                    lcd_goto(0x43);
                    break;
                case 2:
                    lcd_goto(0x46);
                    break;
                case 3:
                    lcd_goto(0x49);
                    break;
                case 4:
                    lcd_goto(0x4C);
                    break;
                }
                break;
            }
            case MENU_SET_A1: {
                drawamenu(1);
                break;
            }
            case MENU_SET_A2: {
                drawamenu(2);
                break;
            }
            case MENU_SET_A3: {
                drawamenu(3);
                break;
            }
            }
            lcd_change = 0;
        }
    }
}
