#include "uart.h"

void dma1_uart2_rx_config(uint8_t *dst, uint16_t len)
{
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_5);
    while (LL_DMA_IsEnabledStream(DMA1, LL_DMA_STREAM_5)) {}

    LL_DMA_ClearFlag_TC5(DMA1);
    LL_DMA_ClearFlag_TE5(DMA1);
    LL_DMA_ClearFlag_DME5(DMA1);
    LL_DMA_ClearFlag_FE5(DMA1);

    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_5, (uint32_t)&USART2->DR);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_5, (uint32_t)dst);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, len);

    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);
}

void dma1_uart2_tx_start(uint8_t *src, uint16_t len)
{   
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_6);
    while (LL_DMA_IsEnabledStream(DMA1, LL_DMA_STREAM_6)) {}

    LL_DMA_ClearFlag_TC6(DMA1);
    LL_DMA_ClearFlag_TE6(DMA1);
    LL_DMA_ClearFlag_DME6(DMA1);
    LL_DMA_ClearFlag_FE6(DMA1);

    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)&USART2->DR);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)src);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, len);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_6);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_6);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);
}

void dma2_uart1_rx_config(uint8_t *dst, uint16_t len)
{
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_2);
    while (LL_DMA_IsEnabledStream(DMA2, LL_DMA_STREAM_2)) {}

    LL_DMA_ClearFlag_TC2(DMA2);
    LL_DMA_ClearFlag_TE2(DMA2);
    LL_DMA_ClearFlag_DME2(DMA2);
    LL_DMA_ClearFlag_FE2(DMA2);

    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_2, (uint32_t)&USART1->DR);
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_2, (uint32_t)dst);
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_2, len);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_2);
}

void dma2_uart1_tx_start(uint8_t *src, uint16_t len)
{
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_7);
    while (LL_DMA_IsEnabledStream(DMA2, LL_DMA_STREAM_7)) {}

    LL_DMA_ClearFlag_TC7(DMA2);
    LL_DMA_ClearFlag_TE7(DMA2);
    LL_DMA_ClearFlag_DME7(DMA2);
    LL_DMA_ClearFlag_FE7(DMA2);
    
    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_7, (uint32_t)&USART1->DR);
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_7, (uint32_t)src);
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_7, len);
    LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_7);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_7);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_7);
}