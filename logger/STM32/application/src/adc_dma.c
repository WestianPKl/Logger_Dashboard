#include "adc_dma.h"
#include "dma.h"

#define PC0 0U
#define PC1 1U

#define ADC_CH1 1U
#define ADC_CH2 2U

void adc_init(uint8_t continuous_mode)
{
    (void)continuous_mode;

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN | RCC_AHB2ENR_ADCEN;
    (void)RCC->AHB2ENR;

    RCC->CCIPR &= ~RCC_CCIPR_ADCSEL_Msk;
    RCC->CCIPR |=  (1U << RCC_CCIPR_ADCSEL_Pos);

    GPIOC->MODER &= ~((3U << (PC0 * 2U)) | (3U << (PC1 * 2U)));
    GPIOC->MODER |=  ((3U << (PC0 * 2U)) | (3U << (PC1 * 2U)));
    GPIOC->PUPDR &= ~((3U << (PC0 * 2U)) | (3U << (PC1 * 2U)));

    GPIOC->ASCR |= (1U << PC0) | (1U << PC1);

    if (ADC1->CR & ADC_CR_ADEN) {
        ADC1->CR |= ADC_CR_ADDIS;
        while (ADC1->CR & ADC_CR_ADEN) {}
    }

    ADC1->CR &= ~ADC_CR_DEEPPWD;
    ADC1->CR |= ADC_CR_ADVREGEN;
    for (volatile int i = 0; i < 10000; i++) {}

    ADC123_COMMON->CCR &= ~ADC_CCR_CKMODE_Msk;
    ADC123_COMMON->CCR |=  (1U << ADC_CCR_CKMODE_Pos);

    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL) {}

    ADC1->SMPR1 &= ~(ADC_SMPR1_SMP1_Msk | ADC_SMPR1_SMP2_Msk);
    ADC1->SMPR1 |=  (7U << ADC_SMPR1_SMP1_Pos) | (7U << ADC_SMPR1_SMP2_Pos);

    ADC1->SQR1 &= ~(ADC_SQR1_L_Msk | ADC_SQR1_SQ1_Msk | ADC_SQR1_SQ2_Msk);
    ADC1->SQR1 |=  (1U << ADC_SQR1_L_Pos);
    ADC1->SQR1 |=  (ADC_CH1 << ADC_SQR1_SQ1_Pos);
    ADC1->SQR1 |=  (ADC_CH2 << ADC_SQR1_SQ2_Pos);
}

void adc_dma_init(uint8_t continuous_mode, uint16_t *dst, uint16_t len)
{
    adc_init(continuous_mode);
    dma2_adc_config(continuous_mode, dst, len);

    ADC1->CFGR &= ~(ADC_CFGR_CONT | ADC_CFGR_DMACFG | ADC_CFGR_DMAEN |
                    ADC_CFGR_EXTSEL_Msk | ADC_CFGR_EXTEN_Msk);

    if (continuous_mode == 1U) {
        ADC1->CFGR |= ADC_CFGR_DMACFG | ADC_CFGR_DMAEN | ADC_CFGR_CONT;
    } else if (continuous_mode == 2U) {
        ADC1->CFGR |= ADC_CFGR_DMACFG | ADC_CFGR_DMAEN;
        ADC1->CFGR |= (9U << ADC_CFGR_EXTSEL_Pos);
        ADC1->CFGR |= (1U << ADC_CFGR_EXTEN_Pos);
    } else {
        ADC1->CFGR |= ADC_CFGR_DMAEN;
    }

    ADC1->ISR |= ADC_ISR_ADRDY;
    ADC1->CR  |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) {}

    ADC1->CR |= ADC_CR_ADSTART;
}