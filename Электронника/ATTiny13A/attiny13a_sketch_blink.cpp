#include <avr/io.h>
#include <util/delay.h>
#define LED_PIN PB0       // Определяем вывод светодиода
#define DELAY_TIME 100    // Задержка в миллисекундах
int main(void) {
    // Устанавливаем вывод LED_PIN в режим выхода
    DDRB |= (1 << LED_PIN);
    while (1) {
        // Включаем светодиод
        PORTB |= (1 << LED_PIN);
        // Задержка
        _delay_ms(DELAY_TIME);
        // Выключаем светодиод
        PORTB &= ~(1 << LED_PIN);
        // Задержка
        _delay_ms(DELAY_TIME);
    }
    return 0;
}