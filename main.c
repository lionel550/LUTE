#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRB |= _BV(DDB5);

    for (;;)
    {
        PORTB |= _BV(PORTB5);
        _delay_ms(500);
        PORTB &= ~_BV(PORTB5);
        _delay_ms(500);
    }

    return 0;
}
