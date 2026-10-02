#ifndef APP_LOGIC_H
#define APP_LOGIC_H
/* PA1 = 0: кнопка нажата. 20 одинаковых отсчётов по 1 мс подавляют дребезг. */
typedef struct {
    unsigned char stable_pressed;
    unsigned char different_samples;
} ButtonFilter;

static unsigned char button_update(ButtonFilter *s, unsigned char pressed)
{
    if (pressed == s->stable_pressed) {
        s->different_samples = 0;
    } else if (++s->different_samples >= 20) {
        s->stable_pressed = pressed;
        s->different_samples = 0;
    }
    return s->stable_pressed;
}

/* ADC 0..4095 -> CCR1 0..1000. ARR=999: 0 = выключено, 1000 = 100%. */
static unsigned short led_duty(unsigned short adc, unsigned char pressed)
{
    if (!pressed) return 0;
    if (adc > 4095U) adc = 4095U;
    return (unsigned short)(((unsigned long)adc * 1000UL) / 4095UL);
}
#endif
