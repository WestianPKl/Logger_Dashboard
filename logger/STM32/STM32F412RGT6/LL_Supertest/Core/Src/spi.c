#include "spi.h"

#define SPI1_PORT GPIOA
#define SPI1_CS1_PORT GPIOA
#define SPI1_CS2_PORT GPIOC

#define SPI1_CS1_PIN LL_GPIO_PIN_4
#define SPI1_CS2_PIN LL_GPIO_PIN_12

#define SPI1_SCK_PIN LL_GPIO_PIN_5
#define SPI1_MISO_PIN LL_GPIO_PIN_6
#define SPI1_MOSI_PIN LL_GPIO_PIN_7

void spi1_cs1_high(void)
{
    LL_GPIO_SetOutputPin(SPI1_CS1_PORT, SPI1_CS1_PIN);
}

void spi1_cs1_low(void)
{
    LL_GPIO_ResetOutputPin(SPI1_CS1_PORT, SPI1_CS1_PIN);
}

void spi1_cs2_high(void)
{
    LL_GPIO_SetOutputPin(SPI1_CS2_PORT, SPI1_CS2_PIN);
}

void spi1_cs2_low(void)
{
    LL_GPIO_ResetOutputPin(SPI1_CS2_PORT, SPI1_CS2_PIN);
}

void dma_spi1_rx(uint32_t dst, uint32_t len){
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);
    while (LL_DMA_IsEnabledStream(DMA2, LL_DMA_STREAM_0)) {}

    LL_DMA_ClearFlag_TC0(DMA2);
    LL_DMA_ClearFlag_TE0(DMA2);
    LL_DMA_ClearFlag_DME0(DMA2);
    LL_DMA_ClearFlag_FE0(DMA2);

    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_0, (uint32_t)&SPI1->DR);
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_0, (uint32_t)dst);
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, len);

    LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_0);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);
}

void dma_spi1_tx(uint32_t src, uint32_t len){
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_3);
    while (LL_DMA_IsEnabledStream(DMA2, LL_DMA_STREAM_3)) {}

    LL_DMA_ClearFlag_TC3(DMA2);
    LL_DMA_ClearFlag_TE3(DMA2);
    LL_DMA_ClearFlag_DME3(DMA2);
    LL_DMA_ClearFlag_FE3(DMA2);

    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_3, (uint32_t)&SPI1->DR);
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_3, (uint32_t)src);
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_3, len);

    LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_3);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_3);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_3);
}