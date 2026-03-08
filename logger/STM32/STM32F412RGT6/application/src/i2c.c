#include "i2c.h"
#include "dma.h"

// PB6: I2C1_SCL, PB7: I2C1_SDA
#define I2C1_SCL_PIN 6U
#define I2C1_SDA_PIN 7U

#define PERIPH_CLK_HZ 16U
#define I2C_100KHZ    80U
#define SD_MODE_MAX_RISE_TIME 17U
#define I2C_TIMEOUT   1000U

extern volatile uint8_t i2c1_dma_tx_done;
extern volatile uint8_t i2c1_dma_rx_done;
extern volatile uint8_t i2c1_dma_err;

#define I2C_FAIL() do { I2C1->CR1 |= I2C_CR1_STOP; i2c1_recover(); return -1; } while(0)
#define I2C_DMA_TIMEOUT  2000U

static int i2c1_check_error_and_clear(void)
{
    uint32_t sr1 = I2C1->SR1;

    if (sr1 & (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR | I2C_SR1_TIMEOUT)) {
        I2C1->SR1 &= ~(I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR | I2C_SR1_TIMEOUT);
        return -1;
    }
    return 0;
}

static void i2c1_recover(void)
{
    I2C1->CR1 |= I2C_CR1_STOP;
    I2C1->CR2 &= ~I2C_CR2_DMAEN;

    dma_i2c1_abort();

    I2C1->CR1 |= I2C_CR1_SWRST;
    for (volatile int i = 0; i < 1000; i++) { __NOP(); }
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    i2c1_init();
}

static int wait_flag_set(volatile uint32_t *reg, uint32_t mask)
{
    uint32_t t = I2C_TIMEOUT;
    while (((*reg) & mask) == 0U) {
        if (i2c1_check_error_and_clear() < 0) return -1;
        if (--t == 0U) return -1;
    }
    return 1;
}

static int wait_flag_clr(volatile uint32_t *reg, uint32_t mask)
{
    uint32_t t = I2C_TIMEOUT;
    while (((*reg) & mask) != 0U) {
        if (i2c1_check_error_and_clear() < 0) return -1;
        if (--t == 0U) return -1;
    }
    return 1;
}

void i2c1_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    (void)RCC->APB1ENR;

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    (void)RCC->AHB1ENR;

    GPIOB->MODER &= ~((3U << (I2C1_SCL_PIN * 2U)) | (3U << (I2C1_SDA_PIN * 2U)));
    GPIOB->MODER |=  ((2U << (I2C1_SCL_PIN * 2U)) | (2U << (I2C1_SDA_PIN * 2U)));

    GPIOB->OTYPER |= (1U << I2C1_SCL_PIN) | (1U << I2C1_SDA_PIN);
    GPIOB->OSPEEDR |= ((3U << (I2C1_SCL_PIN * 2U)) | (3U << (I2C1_SDA_PIN * 2U)));

    GPIOB->PUPDR &= ~((3U << (I2C1_SCL_PIN * 2U)) | (3U << (I2C1_SDA_PIN * 2U)));
    GPIOB->PUPDR |=  ((1U << (I2C1_SCL_PIN * 2U)) | (1U << (I2C1_SDA_PIN * 2U)));

    GPIOB->AFR[0] &= ~((0xFU << (I2C1_SCL_PIN * 4U)) | (0xFU << (I2C1_SDA_PIN * 4U)));
    GPIOB->AFR[0] |=  ((4U << (I2C1_SCL_PIN * 4U)) | (4U << (I2C1_SDA_PIN * 4U)));

    I2C1->CR1 = I2C_CR1_SWRST;
    I2C1->CR1 = 0;

    I2C1->CR2 = 0;
    I2C1->CR2 |= PERIPH_CLK_HZ;

    I2C1->CCR = I2C_100KHZ;
    I2C1->TRISE = SD_MODE_MAX_RISE_TIME;

    I2C1->CR1 |= I2C_CR1_PE;
}

int i2c1_write_raw(uint8_t dev_addr, const uint8_t *data, uint8_t len)
{
    if (!data || len == 0) return -1;

    if (wait_flag_clr(&I2C1->SR2, I2C_SR2_BUSY) < 0) I2C_FAIL();

    I2C1->CR1 |= I2C_CR1_START;
    if (wait_flag_set(&I2C1->SR1, I2C_SR1_SB) < 0) I2C_FAIL();

    (void)I2C1->SR1;
    I2C1->DR = (dev_addr << 1) | 0;
    if (wait_flag_set(&I2C1->SR1, I2C_SR1_ADDR) < 0) I2C_FAIL();

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    for (uint8_t i = 0; i < len; i++) {
        if (wait_flag_set(&I2C1->SR1, I2C_SR1_TXE) < 0) I2C_FAIL();
        I2C1->DR = data[i];
    }

    if (wait_flag_set(&I2C1->SR1, I2C_SR1_BTF) < 0) I2C_FAIL();
    I2C1->CR1 |= I2C_CR1_STOP;
    return 1;
}

int i2c1_read_raw(uint8_t dev_addr, uint8_t *data, uint8_t len)
{
    if (!data || len == 0) return -1;

    if (wait_flag_clr(&I2C1->SR2, I2C_SR2_BUSY) < 0) I2C_FAIL();

    I2C1->CR1 |= I2C_CR1_START;
    if (wait_flag_set(&I2C1->SR1, I2C_SR1_SB) < 0) I2C_FAIL();

    (void)I2C1->SR1;
    I2C1->DR = (dev_addr << 1) | 1;
    if (wait_flag_set(&I2C1->SR1, I2C_SR1_ADDR) < 0) I2C_FAIL();

    if (len == 1) {
        I2C1->CR1 &= ~I2C_CR1_ACK;
        __disable_irq();
        (void)I2C1->SR1;
        (void)I2C1->SR2;
        I2C1->CR1 |= I2C_CR1_STOP;
        __enable_irq();

        if (wait_flag_set(&I2C1->SR1, I2C_SR1_RXNE) < 0) I2C_FAIL();
        data[0] = (uint8_t)I2C1->DR;
        return 1;
    }

    I2C1->CR1 |= I2C_CR1_ACK;
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    for (uint8_t i = 0; i < len; i++) {
        if (i == (len - 2)) {
            I2C1->CR1 &= ~I2C_CR1_ACK;
        }
        if (wait_flag_set(&I2C1->SR1, I2C_SR1_RXNE) < 0) I2C_FAIL();
        data[i] = (uint8_t)I2C1->DR;
    }

    I2C1->CR1 |= I2C_CR1_STOP;
    return 1;
}

static int wait_dma_done(volatile uint8_t *done_flag)
{
    uint32_t t = I2C_DMA_TIMEOUT;
    while (!(*done_flag)) {
        if (--t == 0U) return -1;
    }
    return 1;
}

int i2c1_write_raw_dma(uint8_t dev_addr, const uint8_t *data, uint16_t len)
{
    if (!data || len == 0) return -1;

    if (wait_flag_clr(&I2C1->SR2, I2C_SR2_BUSY) < 0) I2C_FAIL();

    i2c1_dma_tx_done = 0;
    i2c1_dma_err = 0;

    I2C1->CR1 |= I2C_CR1_START;
    if (wait_flag_set(&I2C1->SR1, I2C_SR1_SB) < 0) I2C_FAIL();

    (void)I2C1->SR1;
    I2C1->DR = (dev_addr << 1) | 0;
    if (wait_flag_set(&I2C1->SR1, I2C_SR1_ADDR) < 0) I2C_FAIL();

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->CR2 |= I2C_CR2_DMAEN;

    dma_i2c1_tx_start((uint32_t)data, len);

    if (wait_dma_done(&i2c1_dma_tx_done) < 0) {
        I2C1->CR2 &= ~I2C_CR2_DMAEN;
        I2C1->CR1 |= I2C_CR1_STOP;
        i2c1_recover();
        return -1;
    }

    if (i2c1_dma_err) {
        I2C1->CR2 &= ~I2C_CR2_DMAEN;
        I2C1->CR1 |= I2C_CR1_STOP;
        i2c1_recover();
        return -1;
    }

    if (wait_flag_set(&I2C1->SR1, I2C_SR1_BTF) < 0) {
        I2C1->CR2 &= ~I2C_CR2_DMAEN;
        I2C1->CR1 |= I2C_CR1_STOP;
        return -1;
    }

    I2C1->CR2 &= ~I2C_CR2_DMAEN;
    I2C1->CR1 |= I2C_CR1_STOP;
    return 1;
}

int i2c1_read_raw_dma(uint8_t dev_addr, uint8_t *data, uint16_t len)
{
    if (!data || len == 0) return -1;

    if (wait_flag_clr(&I2C1->SR2, I2C_SR2_BUSY) < 0) I2C_FAIL();

    i2c1_dma_rx_done = 0;
    i2c1_dma_err = 0;

    if (len == 1) {
        return i2c1_read_raw(dev_addr, data, 1);
    }

    I2C1->CR1 |= I2C_CR1_ACK;
    I2C1->CR2 |= I2C_CR2_LAST;

    I2C1->CR1 |= I2C_CR1_START;
    if (wait_flag_set(&I2C1->SR1, I2C_SR1_SB) < 0) I2C_FAIL();

    (void)I2C1->SR1;
    I2C1->DR = (dev_addr << 1) | 1;
    if (wait_flag_set(&I2C1->SR1, I2C_SR1_ADDR) < 0) I2C_FAIL();

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->CR2 |= I2C_CR2_DMAEN;

    dma_i2c1_rx_start((uint32_t)data, len);

    if (wait_dma_done(&i2c1_dma_rx_done) < 0) {
        I2C1->CR2 &= ~I2C_CR2_DMAEN;
        I2C1->CR1 |= I2C_CR1_STOP;
        i2c1_recover();
        return -1;
    }

    if (i2c1_dma_err) {
        I2C1->CR2 &= ~I2C_CR2_DMAEN;
        I2C1->CR1 |= I2C_CR1_STOP;
        i2c1_recover();
        return -1;
    }

    I2C1->CR2 &= ~I2C_CR2_DMAEN;
    I2C1->CR1 |= I2C_CR1_STOP;

    I2C1->CR2 &= ~I2C_CR2_LAST;
    I2C1->CR1 |= I2C_CR1_ACK;

    return 1;
}

int i2c1_write_u8_u16_dma(uint8_t addr7, uint8_t reg, uint16_t value){
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = (uint8_t)((value >> 8) & 0xFF);
    buf[2] = (uint8_t)( value        & 0xFF);
    return i2c1_write_raw_dma(addr7, buf, 3);
}

int i2c1_read_u8_u16_dma (uint8_t addr7, uint8_t reg, uint16_t *value)
{
    if (i2c1_write_raw_dma(addr7, &reg, 1) < 0) return -1;
    uint8_t buf[2];
    if (i2c1_read_raw_dma(addr7, buf, 2) < 0) return -1;
    *value = ((uint16_t)buf[0] << 8) | buf[1];
    return 1;
}
