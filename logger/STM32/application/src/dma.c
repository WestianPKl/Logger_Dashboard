#include "dma.h"

static inline void dma1_clear_flags_ch1(void){ DMA1->IFCR = DMA_IFCR_CGIF1 | DMA_IFCR_CTCIF1 | DMA_IFCR_CTEIF1; }
static inline void dma1_clear_flags_ch2(void){ DMA1->IFCR = DMA_IFCR_CGIF2 | DMA_IFCR_CTCIF2 | DMA_IFCR_CTEIF2; }
static inline void dma1_clear_flags_ch3(void){ DMA1->IFCR = DMA_IFCR_CGIF3 | DMA_IFCR_CTCIF3 | DMA_IFCR_CTEIF3; }
static inline void dma1_clear_flags_ch4(void){ DMA1->IFCR = DMA_IFCR_CGIF4 | DMA_IFCR_CTCIF4 | DMA_IFCR_CTEIF4; }
static inline void dma1_clear_flags_ch5(void){ DMA1->IFCR = DMA_IFCR_CGIF5 | DMA_IFCR_CTCIF5 | DMA_IFCR_CTEIF5; }
static inline void dma1_clear_flags_ch6(void){ DMA1->IFCR = DMA_IFCR_CGIF6 | DMA_IFCR_CTCIF6 | DMA_IFCR_CTEIF6; }
static inline void dma1_clear_flags_ch7(void){ DMA1->IFCR = DMA_IFCR_CGIF7 | DMA_IFCR_CTCIF7 | DMA_IFCR_CTEIF7; }

static inline void dma2_clear_flags_ch1(void){ DMA2->IFCR = DMA_IFCR_CGIF1 | DMA_IFCR_CTCIF1 | DMA_IFCR_CTEIF1; }
static inline void dma2_clear_flags_ch3(void){ DMA2->IFCR = DMA_IFCR_CGIF3 | DMA_IFCR_CTCIF3 | DMA_IFCR_CTEIF3; }
static inline void dma2_clear_flags_ch6(void){ DMA2->IFCR = DMA_IFCR_CGIF6 | DMA_IFCR_CTCIF6 | DMA_IFCR_CTEIF6; }
static inline void dma2_clear_flags_ch7(void){ DMA2->IFCR = DMA_IFCR_CGIF7 | DMA_IFCR_CTCIF7 | DMA_IFCR_CTEIF7; }

void dma2_adc_config(uint8_t continuous_mode, uint16_t *dst, uint16_t len)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    (void)RCC->AHB1ENR;

    DMA2_Channel3->CCR = 0;
    while (DMA2_Channel3->CCR & DMA_CCR_EN) {}

    DMA2_CSELR->CSELR &= ~DMA_CSELR_C3S_Msk;
    DMA2_CSELR->CSELR |=  (0U << DMA_CSELR_C3S_Pos);

    if (continuous_mode == 1U || continuous_mode == 2U) DMA2_Channel3->CCR |= DMA_CCR_CIRC;
    else DMA2_Channel3->CCR &= ~DMA_CCR_CIRC;

    DMA2_Channel3->CCR &= ~DMA_CCR_MSIZE_Msk;
    DMA2_Channel3->CCR |=  (1U << DMA_CCR_MSIZE_Pos);
    DMA2_Channel3->CCR &= ~DMA_CCR_PSIZE_Msk;
    DMA2_Channel3->CCR |=  (1U << DMA_CCR_PSIZE_Pos);

    DMA2_Channel3->CPAR  = (uint32_t)&ADC1->DR;
    DMA2_Channel3->CMAR  = (uint32_t)dst;
    DMA2_Channel3->CNDTR = (uint16_t)len;

    DMA2_Channel3->CCR |=  DMA_CCR_MINC;
    DMA2_Channel3->CCR &= ~DMA_CCR_PINC;
    DMA2_Channel3->CCR &= ~DMA_CCR_MEM2MEM;
    DMA2_Channel3->CCR |=  DMA_CCR_TEIE | DMA_CCR_TCIE;

    NVIC_EnableIRQ(DMA2_Channel3_IRQn);

    dma2_clear_flags_ch3();
    DMA2_Channel3->CCR |= DMA_CCR_EN;
}


void dma1_uart2_rx_config(uint32_t rx_dst, uint32_t rx_len)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    (void)RCC->AHB1ENR;

    DMA1_Channel6->CCR = 0;
    while (DMA1_Channel6->CCR & DMA_CCR_EN) {}

    dma1_clear_flags_ch6();

    DMA1_CSELR->CSELR &= ~DMA_CSELR_C6S_Msk;
    DMA1_CSELR->CSELR |=  (2U << DMA_CSELR_C6S_Pos);

    DMA1_Channel6->CCR &= ~DMA_CCR_MSIZE_Msk;
    DMA1_Channel6->CCR &= ~DMA_CCR_PSIZE_Msk;

    DMA1_Channel6->CPAR  = (uint32_t)&USART2->RDR;
    DMA1_Channel6->CMAR  = rx_dst;
    DMA1_Channel6->CNDTR = rx_len;

    DMA1_Channel6->CCR &= ~DMA_CCR_DIR;
    DMA1_Channel6->CCR &= ~DMA_CCR_MEM2MEM;
    DMA1_Channel6->CCR |=  DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_TEIE;

    NVIC_EnableIRQ(DMA1_Channel6_IRQn);

    DMA1_Channel6->CCR |= DMA_CCR_EN;
}

void dma1_uart2_tx_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    (void)RCC->AHB1ENR;

    DMA1_Channel7->CCR = 0;
    while (DMA1_Channel7->CCR & DMA_CCR_EN) {}

    dma1_clear_flags_ch7();

    DMA1_CSELR->CSELR &= ~DMA_CSELR_C7S_Msk;
    DMA1_CSELR->CSELR |=  (2U << DMA_CSELR_C7S_Pos);

    DMA1_Channel7->CCR &= ~DMA_CCR_MSIZE_Msk;
    DMA1_Channel7->CCR &= ~DMA_CCR_PSIZE_Msk;

    DMA1_Channel7->CPAR  = (uint32_t)&USART2->TDR;

    DMA1_Channel7->CCR |=  DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TCIE | DMA_CCR_TEIE;
    DMA1_Channel7->CCR &= ~DMA_CCR_MEM2MEM;

    NVIC_EnableIRQ(DMA1_Channel7_IRQn);
}

void dma1_uart2_tx_start(uint32_t tx_src, uint32_t tx_len)
{
    if (tx_len == 0U) return;

    DMA1_Channel7->CCR &= ~DMA_CCR_EN;
    while (DMA1_Channel7->CCR & DMA_CCR_EN) {}

    dma1_clear_flags_ch7();

    DMA1_Channel7->CMAR  = tx_src;
    DMA1_Channel7->CNDTR = (uint16_t)tx_len;

    DMA1_Channel7->CCR |= DMA_CCR_EN;
}

void dma1_mem2mem16_config(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    (void)RCC->AHB1ENR;

    DMA1_Channel1->CCR = 0;
    while (DMA1_Channel1->CCR & DMA_CCR_EN) {}

    DMA1_Channel1->CCR &= ~DMA_CCR_MSIZE_Msk;
    DMA1_Channel1->CCR |=  (1U << DMA_CCR_MSIZE_Pos);
    DMA1_Channel1->CCR &= ~DMA_CCR_PSIZE_Msk;
    DMA1_Channel1->CCR |=  (1U << DMA_CCR_PSIZE_Pos);

    DMA1_Channel1->CCR |= DMA_CCR_MINC | DMA_CCR_PINC;
    DMA1_Channel1->CCR |= DMA_CCR_MEM2MEM;
    DMA1_Channel1->CCR |= DMA_CCR_TEIE | DMA_CCR_TCIE;

    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

void dma1_mem2mem16_start(const uint16_t *src, uint16_t *dst, uint32_t len)
{
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;
    while (DMA1_Channel1->CCR & DMA_CCR_EN) {}

    dma1_clear_flags_ch1();

    DMA1_Channel1->CPAR  = (uint32_t)src;
    DMA1_Channel1->CMAR  = (uint32_t)dst;
    DMA1_Channel1->CNDTR = (uint16_t)len;

    DMA1_Channel1->CCR |= DMA_CCR_EN;
}

void dma2_mem2mem16_config(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    (void)RCC->AHB1ENR;

    DMA2_Channel1->CCR = 0;
    while (DMA2_Channel1->CCR & DMA_CCR_EN) {}

    DMA2_Channel1->CCR &= ~DMA_CCR_MSIZE_Msk;
    DMA2_Channel1->CCR |=  (1U << DMA_CCR_MSIZE_Pos);
    DMA2_Channel1->CCR &= ~DMA_CCR_PSIZE_Msk;
    DMA2_Channel1->CCR |=  (1U << DMA_CCR_PSIZE_Pos);

    DMA2_Channel1->CCR |= DMA_CCR_MINC | DMA_CCR_PINC;
    DMA2_Channel1->CCR |= DMA_CCR_MEM2MEM;
    DMA2_Channel1->CCR |= DMA_CCR_TEIE | DMA_CCR_TCIE;

    NVIC_EnableIRQ(DMA2_Channel1_IRQn);
}

void dma2_mem2mem16_start(const uint16_t *src, uint16_t *dst, uint32_t len)
{
    DMA2_Channel1->CCR &= ~DMA_CCR_EN;
    while (DMA2_Channel1->CCR & DMA_CCR_EN) {}

    dma2_clear_flags_ch1();

    DMA2_Channel1->CPAR  = (uint32_t)src;
    DMA2_Channel1->CMAR  = (uint32_t)dst;
    DMA2_Channel1->CNDTR = (uint16_t)len;

    DMA2_Channel1->CCR |= DMA_CCR_EN;
}

void dma1_spi_rx_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    (void)RCC->AHB1ENR;

    DMA1_Channel2->CCR = 0;
    while (DMA1_Channel2->CCR & DMA_CCR_EN) {}

    DMA1_CSELR->CSELR &= ~DMA_CSELR_C2S_Msk;
    DMA1_CSELR->CSELR |=  (1U << DMA_CSELR_C2S_Pos);

    DMA1_Channel2->CCR &= ~DMA_CCR_MSIZE_Msk;
    DMA1_Channel2->CCR &= ~DMA_CCR_PSIZE_Msk;

    DMA1_Channel2->CCR &= ~DMA_CCR_DIR;
    DMA1_Channel2->CCR &= ~DMA_CCR_MEM2MEM;
    DMA1_Channel2->CCR |=  DMA_CCR_MINC;
    DMA1_Channel2->CCR &= ~DMA_CCR_PINC;
    DMA1_Channel2->CCR &= ~DMA_CCR_CIRC;
    DMA1_Channel2->CCR |=  DMA_CCR_TCIE | DMA_CCR_TEIE;

    NVIC_EnableIRQ(DMA1_Channel2_IRQn);
    dma1_clear_flags_ch2();
}

void dma1_spi_tx_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    (void)RCC->AHB1ENR;

    DMA1_Channel3->CCR = 0;
    while (DMA1_Channel3->CCR & DMA_CCR_EN) {}

    DMA1_CSELR->CSELR &= ~DMA_CSELR_C3S_Msk;
    DMA1_CSELR->CSELR |=  (1U << DMA_CSELR_C3S_Pos);

    DMA1_Channel3->CCR &= ~DMA_CCR_MSIZE_Msk;
    DMA1_Channel3->CCR &= ~DMA_CCR_PSIZE_Msk;

    DMA1_Channel3->CCR |=  DMA_CCR_DIR;
    DMA1_Channel3->CCR &= ~DMA_CCR_MEM2MEM;
    DMA1_Channel3->CCR |=  DMA_CCR_MINC;
    DMA1_Channel3->CCR &= ~DMA_CCR_PINC;
    DMA1_Channel3->CCR &= ~DMA_CCR_CIRC;
    DMA1_Channel3->CCR |=  DMA_CCR_TCIE | DMA_CCR_TEIE;

    NVIC_EnableIRQ(DMA1_Channel3_IRQn);
    dma1_clear_flags_ch3();
}

void dma_spi1_transfer(uint32_t tx_buf, uint32_t tx_len)
{
    if (tx_len == 0U) return;

    DMA1_Channel3->CCR &= ~DMA_CCR_EN;
    while (DMA1_Channel3->CCR & DMA_CCR_EN) {}

    dma1_clear_flags_ch3();

    DMA1_Channel3->CPAR  = (uint32_t)&SPI1->DR;
    DMA1_Channel3->CMAR  = tx_buf;
    DMA1_Channel3->CNDTR = (uint16_t)tx_len;

    DMA1_Channel3->CCR |= DMA_CCR_EN;
}

void dma_spi1_receive(uint32_t rx_buf, uint32_t rx_len)
{
    if (rx_len == 0U) return;

    DMA1_Channel2->CCR &= ~DMA_CCR_EN;
    while (DMA1_Channel2->CCR & DMA_CCR_EN) {}

    dma1_clear_flags_ch2();

    DMA1_Channel2->CPAR  = (uint32_t)&SPI1->DR;
    DMA1_Channel2->CMAR  = rx_buf;
    DMA1_Channel2->CNDTR = (uint16_t)rx_len;

    DMA1_Channel2->CCR |= DMA_CCR_EN;
}

void dma_i2c1_rx_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    (void)RCC->AHB1ENR;

    DMA2_Channel7->CCR = 0;
    while (DMA2_Channel7->CCR & DMA_CCR_EN) {}

    DMA2_CSELR->CSELR &= ~DMA_CSELR_C7S_Msk;
    DMA2_CSELR->CSELR |=  (5U << DMA_CSELR_C7S_Pos);

    DMA2_Channel7->CCR &= ~DMA_CCR_MSIZE_Msk;
    DMA2_Channel7->CCR &= ~DMA_CCR_PSIZE_Msk;

    DMA2_Channel7->CCR &= ~DMA_CCR_DIR;
    DMA2_Channel7->CCR &= ~DMA_CCR_MEM2MEM;
    DMA2_Channel7->CCR |=  DMA_CCR_MINC;
    DMA2_Channel7->CCR &= ~DMA_CCR_PINC;
    DMA2_Channel7->CCR &= ~DMA_CCR_CIRC;

    DMA2_Channel7->CCR |= DMA_CCR_TCIE | DMA_CCR_TEIE;

    NVIC_EnableIRQ(DMA2_Channel7_IRQn);
    dma2_clear_flags_ch7();
}

void dma_i2c1_tx_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    (void)RCC->AHB1ENR;

    DMA2_Channel6->CCR = 0;
    while (DMA2_Channel6->CCR & DMA_CCR_EN) {}

    DMA2_CSELR->CSELR &= ~DMA_CSELR_C6S_Msk;
    DMA2_CSELR->CSELR |=  (5U << DMA_CSELR_C6S_Pos);

    DMA2_Channel6->CCR &= ~DMA_CCR_MSIZE_Msk;
    DMA2_Channel6->CCR &= ~DMA_CCR_PSIZE_Msk;

    DMA2_Channel6->CCR |=  DMA_CCR_DIR;
    DMA2_Channel6->CCR &= ~DMA_CCR_MEM2MEM;
    DMA2_Channel6->CCR |=  DMA_CCR_MINC;
    DMA2_Channel6->CCR &= ~DMA_CCR_PINC;
    DMA2_Channel6->CCR &= ~DMA_CCR_CIRC;

    DMA2_Channel6->CCR |= DMA_CCR_TCIE | DMA_CCR_TEIE;

    NVIC_EnableIRQ(DMA2_Channel6_IRQn);
    dma2_clear_flags_ch6();
}

void dma_i2c1_rx_start(uint32_t dst, uint16_t len)
{
    if (len == 0U) return;

    DMA2_Channel7->CCR &= ~DMA_CCR_EN;
    while (DMA2_Channel7->CCR & DMA_CCR_EN) {}

    dma2_clear_flags_ch7();

    DMA2_Channel7->CPAR  = (uint32_t)&I2C1->RXDR;
    DMA2_Channel7->CMAR  = dst;
    DMA2_Channel7->CNDTR = len;

    DMA2_Channel7->CCR |= DMA_CCR_EN;
}

void dma_i2c1_tx_start(uint32_t src, uint16_t len)
{
    if (len == 0U) return;

    DMA2_Channel6->CCR &= ~DMA_CCR_EN;
    while (DMA2_Channel6->CCR & DMA_CCR_EN) {}

    dma2_clear_flags_ch6();

    DMA2_Channel6->CPAR  = (uint32_t)&I2C1->TXDR;
    DMA2_Channel6->CMAR  = src;
    DMA2_Channel6->CNDTR = len;

    DMA2_Channel6->CCR |= DMA_CCR_EN;
}

void dma_pwm_tim2_ch1_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    (void)RCC->AHB1ENR;

    DMA1_Channel2->CCR = 0;
    while (DMA1_Channel2->CCR & DMA_CCR_EN) {}

    DMA1_CSELR->CSELR &= ~DMA_CSELR_C2S_Msk;
    DMA1_CSELR->CSELR |=  (4U << DMA_CSELR_C2S_Pos);

    DMA1_Channel2->CCR &= ~DMA_CCR_MSIZE_Msk;
    DMA1_Channel2->CCR |=  (1U << DMA_CCR_MSIZE_Pos);
    DMA1_Channel2->CCR &= ~DMA_CCR_PSIZE_Msk;
    DMA1_Channel2->CCR |=  (1U << DMA_CCR_PSIZE_Pos);

    DMA1_Channel2->CCR &= ~DMA_CCR_MEM2MEM;
    DMA1_Channel2->CCR |=  DMA_CCR_MINC;
    DMA1_Channel2->CCR &= ~DMA_CCR_PINC;

    DMA1_Channel2->CCR |=  DMA_CCR_CIRC;
    DMA1_Channel2->CCR |=  DMA_CCR_DIR;

    DMA1_Channel2->CCR |=  DMA_CCR_TEIE | DMA_CCR_TCIE;

    DMA1_Channel2->CPAR = (uint32_t)&TIM2->CCR1;

    NVIC_EnableIRQ(DMA1_Channel2_IRQn);
}

void dma_pwm_tim2_ch1_start(uint32_t src, uint16_t len)
{
    if (len == 0U) return;

    DMA1_Channel2->CCR &= ~DMA_CCR_EN;
    while (DMA1_Channel2->CCR & DMA_CCR_EN) {}

    dma1_clear_flags_ch2();

    DMA1_Channel2->CMAR  = src;
    DMA1_Channel2->CNDTR = len;

    DMA1_Channel2->CCR |= DMA_CCR_EN;
}

void dma1_uart1_rx_config(uint32_t rx_dst, uint32_t rx_len)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    (void)RCC->AHB1ENR;

    DMA1_Channel5->CCR = 0;
    while (DMA1_Channel5->CCR & DMA_CCR_EN) {}

    dma1_clear_flags_ch5();

    DMA1_CSELR->CSELR &= ~DMA_CSELR_C5S_Msk;
    DMA1_CSELR->CSELR |=  (2U << DMA_CSELR_C5S_Pos);

    DMA1_Channel5->CCR &= ~DMA_CCR_MSIZE_Msk;
    DMA1_Channel5->CCR &= ~DMA_CCR_PSIZE_Msk;

    DMA1_Channel5->CPAR  = (uint32_t)&USART1->RDR;
    DMA1_Channel5->CMAR  = rx_dst;
    DMA1_Channel5->CNDTR = rx_len;

    DMA1_Channel5->CCR &= ~DMA_CCR_DIR;
    DMA1_Channel5->CCR &= ~DMA_CCR_MEM2MEM;
    DMA1_Channel5->CCR |=  DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_TEIE;

    NVIC_EnableIRQ(DMA1_Channel5_IRQn);

    DMA1_Channel5->CCR |= DMA_CCR_EN;
}

void dma1_uart1_tx_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    (void)RCC->AHB1ENR;

    DMA1_Channel4->CCR = 0;
    while (DMA1_Channel4->CCR & DMA_CCR_EN) {}

    dma1_clear_flags_ch4();

    DMA1_CSELR->CSELR &= ~DMA_CSELR_C4S_Msk;
    DMA1_CSELR->CSELR |=  (2U << DMA_CSELR_C4S_Pos);

    DMA1_Channel4->CCR &= ~DMA_CCR_MSIZE_Msk;
    DMA1_Channel4->CCR &= ~DMA_CCR_PSIZE_Msk;

    DMA1_Channel4->CPAR  = (uint32_t)&USART1->TDR;

    DMA1_Channel4->CCR |=  DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TCIE | DMA_CCR_TEIE;
    DMA1_Channel4->CCR &= ~DMA_CCR_MEM2MEM;

    NVIC_EnableIRQ(DMA1_Channel4_IRQn);
}

void dma1_uart1_tx_start(uint32_t tx_src, uint32_t tx_len)
{
    if (tx_len == 0U) return;

    DMA1_Channel4->CCR &= ~DMA_CCR_EN;
    while (DMA1_Channel4->CCR & DMA_CCR_EN) {}

    dma1_clear_flags_ch4();

    DMA1_Channel4->CMAR  = tx_src;
    DMA1_Channel4->CNDTR = (uint16_t)tx_len;

    DMA1_Channel4->CCR |= DMA_CCR_EN;
}