/*
 * PIN Mapping
 *
 * ----------------------
 * LCD    | Arduino
 * ----------------------
 * D7     |   PD2
 * D6     |   PD3
 * D5     |   PD4
 * D4     |   PD5
 * E      |   PB3
 * RS     |   PB4
 * VSS    |   Ground
 * R/W    |   Ground
 * VDD    |   5V
 */

#include <avr/io.h>
#include <util/delay.h>

#define LCD_DISPLAY_OFF                  0x8
#define LCD_DISPLAY_ON                   0xC
#define LCD_CLEAR_DISPLAY                0x1
#define LCD_ENTRY_MODE_SET               0x6
#define LCD_FUNCTION_SET_4_BITS          0x2
#define LCD_FUNCTION_SET_8_BITS          0x3
#define LCD_FUNCTION_SET_4_BITS_2_LINES  0x28

#define EXTRACT_HIGH_ORDER_BITS(value) (value >> 4)
#define EXTRACT_LOW_ORDER_BITS(value) (value & 0xF)

void write_niddle(uint8_t value)
{
    // Write data
    PORTD &= ~_BV(PD2) & ~_BV(PD3) & ~_BV(PD4) & ~_BV(PD5);

    if (value & 0x8) PORTD |= _BV(PD2);
    if (value & 0x4) PORTD |= _BV(PD3);
    if (value & 0x2) PORTD |= _BV(PD4);
    if (value & 0x1) PORTD |= _BV(PD5);
   
    // Pulse E
    PORTB |= _BV(PB3);
    _delay_ms(1);
    PORTB &= ~_BV(PB3);
    _delay_ms(1);
}

void send_command(uint8_t command)
{
    PORTB &= ~_BV(PB4);
    write_niddle(EXTRACT_HIGH_ORDER_BITS(command));
    write_niddle(EXTRACT_LOW_ORDER_BITS(command));
    _delay_ms(2);
}


void init_lcd()
{
    _delay_ms(40);   

    PORTB &= ~_BV(PB4);
    
    write_niddle(LCD_FUNCTION_SET_8_BITS);
    _delay_ms(4.1);
    
    write_niddle(LCD_FUNCTION_SET_8_BITS);
    _delay_us(100);
    
    write_niddle(LCD_FUNCTION_SET_8_BITS);
    write_niddle(LCD_FUNCTION_SET_4_BITS);

    send_command(LCD_FUNCTION_SET_4_BITS_2_LINES);
    send_command(LCD_DISPLAY_OFF);
    send_command(LCD_CLEAR_DISPLAY);
    send_command(LCD_ENTRY_MODE_SET); 
    send_command(LCD_DISPLAY_ON);
}

void setup()
{
    DDRD |= _BV(DDD2) | _BV(DDD3) | _BV(DDD4) | _BV(DDD5);
    DDRB |= _BV(DDB3) | _BV(DDB4);
    
    init_lcd();
}

int main(void)
{
    setup();

    for(;;)
    {
        // Nothing to do
    }

    return 0;
}
