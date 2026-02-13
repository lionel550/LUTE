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
#include <math.h>
#include <string.h>
#include <stddef.h>

#define LCD_DISPLAY_OFF                  0x8
#define LCD_DISPLAY_ON                   0xC
#define LCD_CLEAR_DISPLAY                0x1
#define LCD_ENTRY_MODE_SET               0x6
#define LCD_FUNCTION_SET_4_BITS          0x2
#define LCD_FUNCTION_SET_8_BITS          0x3
#define LCD_FUNCTION_SET_4_BITS_2_LINES  0x28
#define LCD_CURSOR_AT_1ST_LINE           0x80
#define LCD_CURSOR_AT_2ND_LINE           0xC0

#define EXTRACT_HIGH_ORDER_BITS(value) (value >> 4)
#define EXTRACT_LOW_ORDER_BITS(value) (value & 0xF)

#define THERMISTOR_PIN     0
#define PHOTORESISTOR_PIN  1

int VO;
float R1 = 10000;
float LOG_R2, R2;
float C1 = 1.009249522e-03, C2 = 2.378405444e-04, C3 = 2.019202697e-07;

uint16_t analog_read(uint8_t channel);

float read_temperature()
{
    VO = analog_read(THERMISTOR_PIN);
    R2 = R1 * (1023.0 / (float)VO - 1.0);
    LOG_R2 = log(R2);
    float temperature = (1.0 / (C1 + C2 * LOG_R2 + C3 * LOG_R2 * LOG_R2 * LOG_R2));
    return temperature - 273.15;
}

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

void display_text(char *text)
{
    PORTB |= _BV(PB4);
    for (int i = 0; i < strlen(text); i++)
    {
      write_niddle(EXTRACT_HIGH_ORDER_BITS(text[i]));
      write_niddle(EXTRACT_LOW_ORDER_BITS(text[i]));
      _delay_us(50);
    }
}

void send_command(uint8_t command)
{
    PORTB &= ~_BV(PB4);
    write_niddle(EXTRACT_HIGH_ORDER_BITS(command));
    write_niddle(EXTRACT_LOW_ORDER_BITS(command));
    _delay_ms(2);
}

uint16_t analog_read(uint8_t channel)
{
    // Select the channel and set VCC as reference
    ADMUX &= 0x0;
    ADMUX = _BV(REFS0) | channel;
    
    // Start ADC and set prescaler at 128
    ADCSRA |= _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) | _BV(ADSC);

    while (ADCSRA & 0x40) { /* nothing to do */ }
    
    return ADC;
}

// Doesn't handle negative number
void int_to_string(int i, char buffer[], size_t size)
{
    int index = 0;

    if (i == 0) {
        buffer[i] = '0';
        buffer[i + 1] = '\0';
        return;
    }

    int div = 1;
    while (div * 10 < i) {
        div *= 10;
    }

    int rest = i;
    while (rest != 0 && index < size - 1)
    {
        buffer[index] = '0' + (rest / div);
        rest = rest % div;
        div /= 10;
        index++;
    }

    buffer[index] = '\0';
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

void display_temperature()
{
    char buffer_1[5] = {0};
    char buffer_2[5] = {0};

    float temperature = read_temperature();

    int_to_string((int)temperature, buffer_1, sizeof(buffer_1));
    int_to_string((int)((temperature - (int)temperature) * 10), buffer_2, sizeof(buffer_2));
    
    display_text("TEMP: ");
    display_text(buffer_1);
    display_text(".");
    display_text(buffer_2);
    display_text(" C");
}

void display_luminance()
{
    char buffer[5] = {0};

    uint16_t luminance = analog_read(PHOTORESISTOR_PIN);

    int_to_string(luminance, buffer, sizeof(buffer));

    display_text("LIGHT: ");
    display_text(buffer);
    display_text(" LUX");
}

int main(void)
{ 
    setup();
    
    for(;;)
    {
        send_command(LCD_CLEAR_DISPLAY);
        send_command(LCD_CURSOR_AT_1ST_LINE);
        
        display_temperature();
        
        send_command(LCD_CURSOR_AT_2ND_LINE);
        
        display_luminance();

        _delay_ms(2000);
    }

    return 0;
}
