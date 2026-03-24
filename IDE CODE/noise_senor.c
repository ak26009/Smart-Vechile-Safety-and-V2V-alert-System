#include <stdint.h>

/* ================= REGISTERS ================= */
#define RCC_AHB1ENR   (*(volatile uint32_t*)0x40023830)
#define RCC_APB2ENR   (*(volatile uint32_t*)0x40023844)

#define GPIOA_MODER   (*(volatile uint32_t*)0x40020000)

#define ADC1_SR       (*(volatile uint32_t*)0x40012000)
#define ADC1_CR2      (*(volatile uint32_t*)0x40012008)
#define ADC1_SQR3     (*(volatile uint32_t*)0x40012034)
#define ADC1_DR       (*(volatile uint32_t*)0x4001204C)

/* ================= GLOBAL ================= */
volatile uint16_t noise_adc = 0;

/* ================= DELAY ================= */
void delay_ms(uint32_t ms)
{
    while(ms--)
        for(volatile uint32_t i=0;i<16000;i++);
}

/* ================= ADC INIT ================= */
void adc_init(void)
{
    RCC_AHB1ENR |= (1 << 0);   // GPIOA clock
    RCC_APB2ENR |= (1 << 8);   // ADC1 clock

    GPIOA_MODER |= (3 << (0 * 2)); // PA0 analog mode

    ADC1_SQR3 = 0;  // Channel 0
    ADC1_CR2 |= (1 << 0); // ADON
}

/* ================= ADC READ ================= */
uint16_t adc_read(void)
{
    ADC1_CR2 |= (1 << 30); // SWSTART
    while(!(ADC1_SR & (1 << 1))); // EOC
    return ADC1_DR;
}

/* ================= MAIN ================= */
int main(void)
{
    adc_init();

    while(1)
    {
        noise_adc = adc_read(); // 👈 WATCH THIS VARIABLE
        delay_ms(100);
    }
}

