/* Практика 1. Демидов Иван Вадимович, ЭФБО-08-24.
 * STM32F103C8Tx: PA0 = D1 через R1 220 Ом; PA1 = SW1 (подтяжка R2);
 * PA2 = движок RV1 10 кОм. Нажатие включает LED, RV1 задаёт яркость.
 * CMSIS Device STM32F1, define STM32F103xB. HAL не требуется.
 */
#include "stm32f1xx.h"
#include "app_logic.h"

volatile unsigned short adc_value;
volatile unsigned short pwm_value;
volatile unsigned char button_pressed;

static void clock_init(void)
{
    /* Явно выбираем внутренний HSI 8 МГц, внешний кварц не нужен. */
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0) {}
    RCC->CFGR &= ~RCC_CFGR_SW;
    while ((RCC->CFGR & RCC_CFGR_SWS) != 0) {}
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 |
                   RCC_CFGR_PPRE2 | RCC_CFGR_ADCPRE);
    /* AHB=APB1=APB2=8 МГц; ADC=APB2/2=4 МГц. */
    SystemCoreClock = 8000000UL;
    SysTick->LOAD = 7999UL;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    /* TICKINT=0: прерывания не используются. */
}

static void wait_1ms(void)
{
    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0) {}
}

static void gpio_pwm_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;
    AFIO->MAPR &= ~AFIO_MAPR_TIM2_REMAP; /* TIM2_CH1 на PA0. */
    /* PA0: AF push-pull, 2 МГц (0xA).
     * PA1: вход floating (0x4), поскольку R2 уже подтягивает его к +3,3 В.
     * PA2: аналоговый вход (0x0). Остальные выводы не изменяются. */
    GPIOA->CRL = (GPIOA->CRL & ~0xFFFUL) | 0x04AUL;
    TIM2->PSC = 7;   /* 8 МГц / 8 = 1 МГц. */
    TIM2->ARR = 999; /* 1 МГц / 1000 = 1 кГц PWM. */
    TIM2->CCR1 = 0;
    TIM2->CCMR1 = (6UL << 4) | TIM_CCMR1_OC1PE; /* PWM mode 1, preload. */
    TIM2->CCER = TIM_CCER_CC1E; /* Активный высокий уровень. */
    TIM2->CR1 = TIM_CR1_ARPE;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;
}

static void adc_init(void)
{
    volatile unsigned long i;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->APB2RSTR |= RCC_APB2RSTR_ADC1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_ADC1RST;
    ADC1->CR1 = 0;
    ADC1->SMPR2 = 7UL << 6; /* Канал 2: выборка 239,5 такта. */
    ADC1->SQR1 = 0;         /* Одно преобразование. */
    ADC1->SQR2 = 0;
    ADC1->SQR3 = 2;         /* ADC1_IN2 = PA2. */
    ADC1->CR2 = ADC_CR2_ADON;
    for (i = 0; i < 1000UL; ++i) {} /* Стабилизация ADC после включения. */
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while (ADC1->CR2 & ADC_CR2_RSTCAL) {}
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL) {}
    /* EXTSEL=111: программный запуск SWSTART. */
    ADC1->CR2 |= (7UL << 17) | ADC_CR2_EXTTRIG;
}

static unsigned short adc_read(void)
{
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while ((ADC1->SR & ADC_SR_EOC) == 0) {}
    return (unsigned short)ADC1->DR;
}

int main(void)
{
    ButtonFilter filter = {0, 0};
    clock_init();
    gpio_pwm_init();
    adc_init();
    for (;;) {
        wait_1ms();
        button_pressed = button_update(&filter,
            (unsigned char)((GPIOA->IDR & (1UL << 1)) == 0));
        adc_value = adc_read();
        pwm_value = led_duty(adc_value, button_pressed);
        TIM2->CCR1 = pwm_value;
    }
}
