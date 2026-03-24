#include <stdint.h>

/* ================= REGISTERS ================= */
#define RCC_AHB1ENR   (*(volatile uint32_t*)0x40023830)
#define RCC_APB1ENR   (*(volatile uint32_t*)0x40023840)
#define RCC_APB2ENR   (*(volatile uint32_t*)0x40023844)

#define GPIOA_MODER   (*(volatile uint32_t*)0x40020000)
#define GPIOA_IDR     (*(volatile uint32_t*)0x40020010)
#define GPIOA_AFRL    (*(volatile uint32_t*)0x40020020)

#define ADC1_SR       (*(volatile uint32_t*)0x40012000)
#define ADC1_CR2      (*(volatile uint32_t*)0x40012008)
#define ADC1_SMPR2    (*(volatile uint32_t*)0x40012010)
#define ADC1_SQR3     (*(volatile uint32_t*)0x40012034)
#define ADC1_DR       (*(volatile uint32_t*)0x4001204C)

#define USART2_SR     (*(volatile uint32_t*)0x40004400)
#define USART2_DR     (*(volatile uint32_t*)0x40004404)
#define USART2_BRR    (*(volatile uint32_t*)0x40004408)
#define USART2_CR1    (*(volatile uint32_t*)0x4000440C)

/* ================= DELAY ================= */
void delay_ms(uint32_t d)
{
    for(uint32_t i=0;i<d*16000;i++);
}

/* ================= UART ================= */
void uart_init(void)
{
    RCC_AHB1ENR |= (1<<0);     // GPIOA
    RCC_APB1ENR |= (1<<17);    // USART2

    // PA2 → USART2_TX (AF7)
    GPIOA_MODER &= ~(3<<(2*2));
    GPIOA_MODER |=  (2<<(2*2));
    GPIOA_AFRL  |=  (7<<(4*2));

    USART2_BRR = 0x683;       // 9600 baud @16MHz
    USART2_CR1 = (1<<13) | (1<<3);
}

void uart_char(char c)
{
    while(!(USART2_SR & (1<<7)));
    USART2_DR = c;
}

void uart_string(char *s)
{
    while(*s) uart_char(*s++);
}

/* ================= ADC (Noise Sensor) ================= */
void adc_init(void)
{
    RCC_AHB1ENR |= (1<<0);    // GPIOA
    RCC_APB2ENR |= (1<<8);    // ADC1

    // PA0 analog mode
    GPIOA_MODER |= (3<<(0*2));

    ADC1_SMPR2 |= (7<<0);     // Sampling time
    ADC1_SQR3 = 0;            // Channel 0
    ADC1_CR2 |= (1<<0);       // ADC ON
}

uint16_t adc_read(void)
{
    ADC1_CR2 |= (1<<30);      // Start conversion
    while(!(ADC1_SR & (1<<1)));
    return ADC1_DR;
}

/* ================= IR SENSOR ================= */
void ir_init(void)
{
    RCC_AHB1ENR |= (1<<0);    // GPIOA
    GPIOA_MODER &= ~(3<<(1*2));  // PA1 input
}

uint8_t ir_read(void)
{
    return (GPIOA_IDR & (1<<1)) ? 1 : 0;
}

/* ================= MAIN ================= */
int main(void)
{
    uart_init();
    adc_init();
    ir_init();

    char buf[50];

    while(1)
    {
        uint16_t noise = adc_read();
        uint8_t ir = ir_read();

        int i = 0;

        // Noise value
        buf[i++]='N'; buf[i++]='o'; buf[i++]='i'; buf[i++]='s'; buf[i++]='e';
        buf[i++]=':'; buf[i++]=' ';
        buf[i++] = (noise/1000)%10 + '0';
        buf[i++] = (noise/100)%10  + '0';
        buf[i++] = (noise/10)%10   + '0';
        buf[i++] = (noise%10)      + '0';

        buf[i++]=' '; buf[i++]='|'; buf[i++]=' ';

        // IR status
        buf[i++]='I'; buf[i++]='R'; buf[i++]=':'; buf[i++]=' ';
        if(ir == 0)
        {
            buf[i++]='O'; buf[i++]='B'; buf[i++]='S'; buf[i++]='T';
        }
        else
        {
            buf[i++]='C'; buf[i++]='L'; buf[i++]='E'; buf[i++]='A'; buf[i++]='R';
        }

        buf[i++]='\r'; buf[i++]='\n';
        buf[i]=0;

        uart_string(buf);
        delay_ms(300);
    }
}

