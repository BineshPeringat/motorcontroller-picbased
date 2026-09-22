//lcd.h
#include	<htc.h>

#ifndef _XTAL_FREQ
// Unless specified elsewhere, 4MHz system frequency is assumed
#define _XTAL_FREQ 4000000
#endif

#define LCD_RS RE2
#define LCD_RW RE1
#define LCD_EN RE0

#define LCD_DATA PORTB

static void LCD_STROBE() {
    LCD_EN = 1;
    __delay_us(500);
    LCD_EN = 0;
}

/* write a byte to the LCD in 4 bit mode */
void lcd_write(unsigned char c) {
    __delay_us(70);
    LCD_DATA = c;
    LCD_STROBE();
}

//	Clear the LCD
void lcd_clear(void) {
    LCD_RS = 0;
    lcd_write(0x1);
    __delay_ms(2);
}

/* write a string of chars to the LCD */
void lcd_puts(const char * s) {
    LCD_RS = 1; // write characters
    while ( * s)
        lcd_write( * s++);
}

/* write one character to the LCD */
void lcd_putch(char c) {
    LCD_RS = 1; // write characters
    lcd_write(c);
}

// Go to the specified position
void lcd_goto(unsigned char pos) {
    LCD_RS = 0;
    lcd_write(0x80 + pos);
}

void lcd_init() {
    char init_value;
    ADCON1 = 0x06;
    CMCON = 0x0D;
    init_value = 0x3;
    TRISE = 0;
    TRISB = 0;
    LCD_RS = 0;
    LCD_EN = 0;
    LCD_RW = 0;
    __delay_ms(30); // wait 15mSec after power applied,
    LCD_DATA = init_value;
    LCD_STROBE();
    __delay_ms(5);
    LCD_STROBE();
    __delay_us(200);
    LCD_STROBE();
    __delay_us(200);
    LCD_DATA = 0x38;
    LCD_STROBE();
    lcd_write(0x38); // set interface length
    lcd_write(0xD); // display on, cursor on, cursor blink
    lcd_clear(); // clear screen
    lcd_write(0x6); // set entry Mode
}
//lcd.h
#include	<htc.h>

#ifndef _XTAL_FREQ
// Unless specified elsewhere, 4MHz system frequency is assumed
#define _XTAL_FREQ 4000000
#endif

#define LCD_RS RE2
#define LCD_RW RE1
#define LCD_EN RE0

#define LCD_DATA PORTB

static void LCD_STROBE() {
    LCD_EN = 1;
    __delay_us(500);
    LCD_EN = 0;
}

/* write a byte to the LCD in 4 bit mode */
void lcd_write(unsigned char c) {
    __delay_us(70);
    LCD_DATA = c;
    LCD_STROBE();
}

//	Clear the LCD
void lcd_clear(void) {
    LCD_RS = 0;
    lcd_write(0x1);
    __delay_ms(2);
}

/* write a string of chars to the LCD */
void lcd_puts(const char * s) {
    LCD_RS = 1; // write characters
    while ( * s)
        lcd_write( * s++);
}

/* write one character to the LCD */
void lcd_putch(char c) {
    LCD_RS = 1; // write characters
    lcd_write(c);
}

// Go to the specified position
void lcd_goto(unsigned char pos) {
    LCD_RS = 0;
    lcd_write(0x80 + pos);
}

void lcd_init() {
    char init_value;
    ADCON1 = 0x06;
    CMCON = 0x0D;
    init_value = 0x3;
    TRISE = 0;
    TRISB = 0;
    LCD_RS = 0;
    LCD_EN = 0;
    LCD_RW = 0;
    __delay_ms(30); // wait 15mSec after power applied,
    LCD_DATA = init_value;
    LCD_STROBE();
    __delay_ms(5);
    LCD_STROBE();
    __delay_us(200);
    LCD_STROBE();
    __delay_us(200);
    LCD_DATA = 0x38;
    LCD_STROBE();
    lcd_write(0x38); // set interface length
    lcd_write(0xD); // display on, cursor on, cursor blink
    lcd_clear(); // clear screen
    lcd_write(0x6); // set entry Mode
}
