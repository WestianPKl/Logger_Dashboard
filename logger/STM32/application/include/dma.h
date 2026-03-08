#ifndef DMA_H
#define DMA_H

#include <stdint.h>
#include "stm32l4xx.h"

void dma1_mem2mem16_config(void);
void dma2_mem2mem16_config(void);
void dma1_mem2mem16_start(const uint16_t *src, uint16_t *dst, uint32_t len);
void dma2_mem2mem16_start(const uint16_t *src, uint16_t *dst, uint32_t len);

void dma2_adc_config(uint8_t continuous_mode, uint16_t *dst, uint16_t len);

void dma1_uart2_rx_config(uint32_t rx_dst, uint32_t rx_len);
void dma1_uart2_tx_init(void);
void dma1_uart2_tx_start(uint32_t tx_src, uint32_t tx_len);

void dma1_uart1_rx_config(uint32_t rx_dst, uint32_t rx_len);
void dma1_uart1_tx_init(void);
void dma1_uart1_tx_start(uint32_t tx_src, uint32_t tx_len);

void dma1_spi_rx_init(void);
void dma1_spi_tx_init(void);
void dma_spi1_receive(uint32_t rx_buf, uint32_t rx_len);
void dma_spi1_transfer(uint32_t tx_buf, uint32_t tx_len);

void dma_i2c1_rx_init(void);
void dma_i2c1_tx_init(void);
void dma_i2c1_rx_start(uint32_t dst, uint16_t len);
void dma_i2c1_tx_start(uint32_t src, uint16_t len);

void dma_pwm_tim2_ch1_init(void);
void dma_pwm_tim2_ch1_start(uint32_t src, uint16_t len);

#endif // DMA_H