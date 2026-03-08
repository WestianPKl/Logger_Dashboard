#include "i2c_dma.h"
#include "dma.h"

#define I2C_TIMINGR_100KHZ  0x10909CEC
#define I2C_TIMINGR_400KHZ  0x00702991
#define I2C_TIMEOUT         10000U

static volatile uint8_t i2c1_local_tx_done = 0;
static volatile uint8_t i2c1_local_rx_done = 0;
static volatile uint8_t i2c1_local_err     = 0;

static int wait_flag_set(volatile uint32_t *reg, uint32_t mask)
{
    uint32_t t = I2C_TIMEOUT;
    while (((*reg) & mask) == 0U) {
        if (--t == 0U) return -1;
    }
    return 1;
}

static int wait_flag_clr(volatile uint32_t *reg, uint32_t mask)
{
    uint32_t t = I2C_TIMEOUT;
    while (((*reg) & mask) != 0U) {
        if (--t == 0U) return -1;
    }
    return 1;
}

void i2c1_init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;
    (void)RCC->APB1ENR1;

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    (void)RCC->AHB2ENR;

    GPIOB->MODER &= ~((3U << (6U * 2U)) | (3U << (7U * 2U)));
    GPIOB->MODER |=  ((2U << (6U * 2U)) | (2U << (7U * 2U)));

    GPIOB->OTYPER |= (1U << 6U) | (1U << 7U);
    GPIOB->OSPEEDR |= ((3U << (6U * 2U)) | (3U << (7U * 2U)));

    GPIOB->PUPDR &= ~((3U << (6U * 2U)) | (3U << (7U * 2U)));
    GPIOB->PUPDR |=  ((1U << (6U * 2U)) | (1U << (7U * 2U)));
    GPIOB->AFR[0] &= ~((0xFU << (6U * 4U)) | (0xFU << (7U * 4U)));
    GPIOB->AFR[0] |=  ((4U << (6U * 4U)) | (4U << (7U * 4U)));

    I2C1->CR1 &= ~I2C_CR1_PE;

    I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;

    I2C1->TIMINGR = I2C_TIMINGR_100KHZ;

    I2C1->CR1 |= I2C_CR1_RXDMAEN | I2C_CR1_TXDMAEN;

    I2C1->CR1 |= I2C_CR1_PE;

    dma_i2c1_rx_init();
    dma_i2c1_tx_init();
}

int i2c1_write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint8_t len)
{
    if (!data && len) return -1;

    if (wait_flag_clr(&I2C1->ISR, I2C_ISR_BUSY) < 0) return -1;

    i2c1_local_tx_done = 0;
    i2c1_local_err = 0;

    if (len) {
        dma_i2c1_tx_start((uint32_t)data, len);
    }

    I2C1->CR2 = ((uint32_t)dev_addr << 1)
              | ((uint32_t)(len + 1U) << I2C_CR2_NBYTES_Pos)
              | I2C_CR2_AUTOEND
              | I2C_CR2_START;

    if (wait_flag_set(&I2C1->ISR, I2C_ISR_TXIS) < 0) return -1;
    I2C1->TXDR = reg_addr;

    if (wait_flag_set(&I2C1->ISR, I2C_ISR_STOPF) < 0) return -1;
    I2C1->ICR = I2C_ICR_STOPCF;

    if (len) {
        uint32_t t = I2C_TIMEOUT;
        while (!i2c1_local_tx_done && t--) {}
        if (t == 0U) return -1;
    }

    return 1;
}

int i2c1_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
    if (!data || len == 0) return -1;

    if (wait_flag_clr(&I2C1->ISR, I2C_ISR_BUSY) < 0) return -1;

    i2c1_local_rx_done = 0;
    i2c1_local_err = 0;

    I2C1->CR2 = ((uint32_t)dev_addr << 1)
              | (1U << I2C_CR2_NBYTES_Pos)
              | I2C_CR2_START;

    if (wait_flag_set(&I2C1->ISR, I2C_ISR_TXIS) < 0) return -1;
    I2C1->TXDR = reg_addr;

    if (wait_flag_set(&I2C1->ISR, I2C_ISR_TC) < 0) return -1;

    dma_i2c1_rx_start((uint32_t)data, len);

    I2C1->CR2 = ((uint32_t)dev_addr << 1)
              | ((uint32_t)len << I2C_CR2_NBYTES_Pos)
              | I2C_CR2_RD_WRN
              | I2C_CR2_AUTOEND
              | I2C_CR2_START;

    if (wait_flag_set(&I2C1->ISR, I2C_ISR_STOPF) < 0) return -1;
    I2C1->ICR = I2C_ICR_STOPCF;

    uint32_t t = I2C_TIMEOUT;
    while (!i2c1_local_rx_done && t--) {}
    if (t == 0U) return -1;

    return 1;
}

int i2c1_write_cmd(uint8_t dev_addr, uint8_t cmd)
{
    if (wait_flag_clr(&I2C1->ISR, I2C_ISR_BUSY) < 0) return -1;

    I2C1->CR2 = ((uint32_t)dev_addr << 1)
              | (1U << I2C_CR2_NBYTES_Pos)
              | I2C_CR2_AUTOEND
              | I2C_CR2_START;

    if (wait_flag_set(&I2C1->ISR, I2C_ISR_TXIS) < 0) return -1;
    I2C1->TXDR = cmd;

    if (wait_flag_set(&I2C1->ISR, I2C_ISR_STOPF) < 0) return -1;
    I2C1->ICR = I2C_ICR_STOPCF;

    return 1;
}

int i2c1_write_cmd16(uint8_t dev_addr, uint16_t cmd)
{
    if (wait_flag_clr(&I2C1->ISR, I2C_ISR_BUSY) < 0) return -1;

    I2C1->CR2 = ((uint32_t)dev_addr << 1)
              | (2U << I2C_CR2_NBYTES_Pos)
              | I2C_CR2_AUTOEND
              | I2C_CR2_START;

    if (wait_flag_set(&I2C1->ISR, I2C_ISR_TXIS) < 0) return -1;
    I2C1->TXDR = (uint8_t)((cmd >> 8) & 0xFF);

    if (wait_flag_set(&I2C1->ISR, I2C_ISR_TXIS) < 0) return -1;
    I2C1->TXDR = (uint8_t)(cmd & 0xFF);

    if (wait_flag_set(&I2C1->ISR, I2C_ISR_STOPF) < 0) return -1;
    I2C1->ICR = I2C_ICR_STOPCF;

    return 1;
}

int i2c1_write_raw(uint8_t dev_addr, const uint8_t *data, uint8_t len)
{
    if (!data || len == 0) return -1;
    if (wait_flag_clr(&I2C1->ISR, I2C_ISR_BUSY) < 0) return -1;

    I2C1->CR2 = ((uint32_t)dev_addr << 1)
              | ((uint32_t)len << I2C_CR2_NBYTES_Pos)
              | I2C_CR2_AUTOEND
              | I2C_CR2_START;

    for (uint8_t i = 0; i < len; i++) {
        if (wait_flag_set(&I2C1->ISR, I2C_ISR_TXIS) < 0) return -1;
        I2C1->TXDR = data[i];
    }

    if (wait_flag_set(&I2C1->ISR, I2C_ISR_STOPF) < 0) return -1;
    I2C1->ICR = I2C_ICR_STOPCF;

    return 1;
}

int i2c1_read_raw(uint8_t dev_addr, uint8_t *data, uint8_t len)
{
    if (!data || len == 0) return -1;
    if (wait_flag_clr(&I2C1->ISR, I2C_ISR_BUSY) < 0) return -1;

    I2C1->CR2 = ((uint32_t)dev_addr << 1)
              | ((uint32_t)len << I2C_CR2_NBYTES_Pos)
              | I2C_CR2_RD_WRN
              | I2C_CR2_AUTOEND
              | I2C_CR2_START;

    for (uint8_t i = 0; i < len; i++) {
        if (wait_flag_set(&I2C1->ISR, I2C_ISR_RXNE) < 0) return -1;
        data[i] = (uint8_t)I2C1->RXDR;
    }

    if (wait_flag_set(&I2C1->ISR, I2C_ISR_STOPF) < 0) return -1;
    I2C1->ICR = I2C_ICR_STOPCF;

    return 1;
}