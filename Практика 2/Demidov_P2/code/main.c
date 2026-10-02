/* Практика 2, вариант 11. Демидов И.В., ЭФБО-08-24.
 * AT89S51 + 27C128 16 Кбайт. Проверяется 350 байт 1000h..115Dh.
 * D1 включается при P1.0=0. OE ПЗУ подключён к PSEN, поэтому CODE/MOVC.
 * НЕ использовать xdata/MOVX: эта схема подключена к памяти программ.
 */
#if defined(__SDCC_mcs51)
# include <8051.h>
# define CODE __code
__sfr __at (0x8E) AUXR;
__sbit __at (0x90) ERROR_LED;
#elif defined(__C51__)
# include <REG51.H>
# define CODE code
sfr AUXR = 0x8E;
sbit ERROR_LED = P1^0;
#else
# error Use SDCC mcs51 or Keil C51 for AT89S51
#endif
#include "checksum.h"
#include "rom_reference.h"

/* Отдельная константа хранится в резидентной Flash вместе с программой.
 * volatile запрещает заменить все обращения к ней готовым числом в коде.
 */
static volatile const unsigned short CODE expected_sum = ROM_EXPECTED_SUM;
volatile unsigned short observed_sum;
volatile unsigned char test_finished;
volatile unsigned char test_passed;

void main(void)
{
    unsigned short i;
    unsigned short sum;
    volatile const unsigned char CODE *rom;

    ERROR_LED = 1; /* До завершения проверки светодиод выключен. */
    test_finished = 0;
    test_passed = 0;
    P0 = 0xFF;    /* Разрешить альтернативные функции шин адреса/данных. */
    P2 = 0xFF;
    AUXR &= 0xFE; /* DISALE=0: разрешить обычный вывод ALE. */

    rom = (volatile const unsigned char CODE *)ROM_START;
    sum = 0;
    for (i = 0; i < ROM_LENGTH; ++i) {
        /* Чтение из CODE вызывает MOVC и сигнал PSEN внешнего ПЗУ. */
        sum = checksum_add(sum, rom[i]);
    }
    observed_sum = sum;
    test_passed = (sum == expected_sum);
    ERROR_LED = test_passed ? 1 : 0;
    test_finished = 1;
    for (;;) {} /* Сохранить результат; SW1 запускает проверку заново. */
}
