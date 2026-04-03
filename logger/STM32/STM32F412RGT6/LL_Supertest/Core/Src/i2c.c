#include "i2c.h"
#include "stm32f4xx_hal.h"

void dma_i2c1_rx(uint32_t dst, uint16_t len)
{
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_0);
    while (LL_DMA_IsEnabledStream(DMA1, LL_DMA_STREAM_0)) {}

    LL_DMA_ClearFlag_TC0(DMA1);
    LL_DMA_ClearFlag_TE0(DMA1);
    LL_DMA_ClearFlag_DME0(DMA1);
    LL_DMA_ClearFlag_FE0(DMA1);

    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_0, (uint32_t)&I2C1->DR);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_0, (uint32_t)dst);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_0, len);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_0);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_0);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_0);
}

void dma_i2c1_tx(uint32_t src, uint16_t len){
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_1);
    while (LL_DMA_IsEnabledStream(DMA1, LL_DMA_STREAM_1)) {}

    LL_DMA_ClearFlag_TC1(DMA1);
    LL_DMA_ClearFlag_TE1(DMA1);
    LL_DMA_ClearFlag_DME1(DMA1);
    LL_DMA_ClearFlag_FE1(DMA1);

    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_1, (uint32_t)&I2C1->DR);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_1, (uint32_t)src);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_1, len);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_1);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_1);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_1);
}

void dma_i2c1_abort(void)
{
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_0);
    LL_DMA_ClearFlag_TC0(DMA1);
    LL_DMA_ClearFlag_TE0(DMA1);
    LL_DMA_ClearFlag_DME0(DMA1);
    LL_DMA_ClearFlag_FE0(DMA1);
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_1);
    LL_DMA_ClearFlag_TC1(DMA1);
    LL_DMA_ClearFlag_TE1(DMA1);
    LL_DMA_ClearFlag_DME1(DMA1);
    LL_DMA_ClearFlag_FE1(DMA1);
}

#define I2C1_SCL_PIN LL_GPIO_PIN_6
#define I2C1_SDA_PIN LL_GPIO_PIN_7

#define PERIPH_CLK_MHZ         42U
#define I2C_100KHZ_CCR         210U
#define SD_MODE_MAX_RISE_TIME  43U

#define I2C_TIMEOUT      100000U
#define I2C_DMA_TIMEOUT  200000U

extern volatile uint8_t i2c1_dma_tx_done;
extern volatile uint8_t i2c1_dma_rx_done;
extern volatile uint8_t i2c1_dma_err;

#define I2C_FAIL() do { \
    LL_I2C_GenerateStopCondition(I2C1); \
    i2c1_recover(); \
    return -1; \
} while(0)

static int i2c1_check_error_and_clear(void)
{
    if (LL_I2C_IsActiveFlag_BERR(I2C1)) {
        LL_I2C_ClearFlag_BERR(I2C1);
        return -1;
    }
    if (LL_I2C_IsActiveFlag_ARLO(I2C1)) {
        LL_I2C_ClearFlag_ARLO(I2C1);
        return -1;
    }
    if (LL_I2C_IsActiveFlag_AF(I2C1)) {
        LL_I2C_ClearFlag_AF(I2C1);
        return -1;
    }
    if (LL_I2C_IsActiveFlag_OVR(I2C1)) {
        LL_I2C_ClearFlag_OVR(I2C1);
        return -1;
    }
    if (LL_I2C_IsActiveSMBusFlag_TIMEOUT(I2C1)) {
        LL_I2C_ClearSMBusFlag_TIMEOUT(I2C1);
        return -1;
    }
    return 0;
}

static void i2c1_recover(void)
{
    LL_I2C_GenerateStopCondition(I2C1);
    LL_I2C_DisableDMAReq_TX(I2C1);
    LL_I2C_DisableDMAReq_RX(I2C1);

    dma_i2c1_abort();

    LL_I2C_EnableReset(I2C1);
    for (volatile uint32_t i = 0; i < 1000U; i++) { __NOP(); }
    LL_I2C_DisableReset(I2C1);

    LL_I2C_InitTypeDef I2C_InitStruct = {0};

    LL_I2C_DisableOwnAddress2(I2C1);
    LL_I2C_DisableGeneralCall(I2C1);
    LL_I2C_EnableClockStretching(I2C1);
    I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
    I2C_InitStruct.ClockSpeed = 100000;
    I2C_InitStruct.DutyCycle = LL_I2C_DUTYCYCLE_2;
    I2C_InitStruct.OwnAddress1 = 0;
    I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
    I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
    LL_I2C_Init(I2C1, &I2C_InitStruct);
    LL_I2C_SetOwnAddress2(I2C1, 0);
    LL_I2C_EnableDMAReq_RX(I2C1);
    LL_I2C_EnableDMAReq_TX(I2C1);
    LL_I2C_Enable(I2C1);

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

static int i2c1_start_addr(uint8_t addr7, uint8_t read)
{
    LL_I2C_GenerateStartCondition(I2C1);
    if (wait_flag_set(&I2C1->SR1, I2C_SR1_SB) < 0) return -1;

    (void)I2C1->SR1;
    LL_I2C_TransmitData8(I2C1, (uint8_t)((addr7 << 1) | (read ? 1U : 0U)));

    if (wait_flag_set(&I2C1->SR1, I2C_SR1_ADDR) < 0) return -1;
    return 1;
}

static int i2c1_write_bytes(const uint8_t *data, uint16_t len)
{
    if (!data || !len) return -1;

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    for (uint16_t i = 0; i < len; i++) {
        if (wait_flag_set(&I2C1->SR1, I2C_SR1_TXE) < 0) return -1;
        LL_I2C_TransmitData8(I2C1, data[i]);
    }

    if (wait_flag_set(&I2C1->SR1, I2C_SR1_BTF) < 0) return -1;
    return 1;
}

static int i2c1_read_bytes(uint8_t *data, uint16_t len)
{
    if (!data || !len) return -1;

    if (len == 1U) {
        LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);

        __disable_irq();
        (void)I2C1->SR1;
        (void)I2C1->SR2;
        LL_I2C_GenerateStopCondition(I2C1);
        __enable_irq();

        if (wait_flag_set(&I2C1->SR1, I2C_SR1_RXNE) < 0) return -1;
        data[0] = LL_I2C_ReceiveData8(I2C1);
        LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
        return 1;
    }

    if (len == 2U) {
        SET_BIT(I2C1->CR1, I2C_CR1_POS);
        LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);

        __disable_irq();
        (void)I2C1->SR1;
        (void)I2C1->SR2;
        __enable_irq();

        if (wait_flag_set(&I2C1->SR1, I2C_SR1_BTF) < 0) return -1;

        __disable_irq();
        LL_I2C_GenerateStopCondition(I2C1);
        data[0] = LL_I2C_ReceiveData8(I2C1);
        __enable_irq();

        data[1] = LL_I2C_ReceiveData8(I2C1);

        CLEAR_BIT(I2C1->CR1, I2C_CR1_POS);
        LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
        return 1;
    }

    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    for (uint16_t i = 0; i < (len - 3U); i++) {
        if (wait_flag_set(&I2C1->SR1, I2C_SR1_RXNE) < 0) return -1;
        data[i] = LL_I2C_ReceiveData8(I2C1);
    }

    if (wait_flag_set(&I2C1->SR1, I2C_SR1_BTF) < 0) return -1;

    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);

    __disable_irq();
    data[len - 3U] = LL_I2C_ReceiveData8(I2C1);
    LL_I2C_GenerateStopCondition(I2C1);
    data[len - 2U] = LL_I2C_ReceiveData8(I2C1);
    __enable_irq();

    if (wait_flag_set(&I2C1->SR1, I2C_SR1_RXNE) < 0) return -1;
    data[len - 1U] = LL_I2C_ReceiveData8(I2C1);

    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
    return 1;
}

int i2c1_write_raw(uint8_t dev_addr, const uint8_t *data, uint8_t len)
{
    if (!data || len == 0U) return -1;
    if (wait_flag_clr(&I2C1->SR2, I2C_SR2_BUSY) < 0) I2C_FAIL();
    if (i2c1_start_addr(dev_addr, 0) < 0) I2C_FAIL();
    if (i2c1_write_bytes(data, len) < 0) I2C_FAIL();
    LL_I2C_GenerateStopCondition(I2C1);
    return 1;
}

int i2c1_read_raw(uint8_t dev_addr, uint8_t *data, uint8_t len)
{
    if (!data || len == 0U) return -1;
    if (wait_flag_clr(&I2C1->SR2, I2C_SR2_BUSY) < 0) I2C_FAIL();
    if (i2c1_start_addr(dev_addr, 1) < 0) I2C_FAIL();
    if (i2c1_read_bytes(data, len) < 0) I2C_FAIL();
    return 1;
}

int i2c1_reg_write(uint8_t addr7, uint8_t reg, const uint8_t *data, uint16_t len)
{
    if (wait_flag_clr(&I2C1->SR2, I2C_SR2_BUSY) < 0) I2C_FAIL();
    if (i2c1_start_addr(addr7, 0) < 0) I2C_FAIL();

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    if (wait_flag_set(&I2C1->SR1, I2C_SR1_TXE) < 0) I2C_FAIL();
    LL_I2C_TransmitData8(I2C1, reg);

    if (len && data) {
        for (uint16_t i = 0; i < len; i++) {
            if (wait_flag_set(&I2C1->SR1, I2C_SR1_TXE) < 0) I2C_FAIL();
            LL_I2C_TransmitData8(I2C1, data[i]);
        }
    }

    if (wait_flag_set(&I2C1->SR1, I2C_SR1_BTF) < 0) I2C_FAIL();

    LL_I2C_GenerateStopCondition(I2C1);
    return 1;
}

int i2c1_reg_read(uint8_t addr7, uint8_t reg, uint8_t *data, uint16_t len)
{
    if (!data || len == 0U) return -1;

    if (wait_flag_clr(&I2C1->SR2, I2C_SR2_BUSY) < 0) I2C_FAIL();
    if (i2c1_start_addr(addr7, 0) < 0) I2C_FAIL();

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    if (wait_flag_set(&I2C1->SR1, I2C_SR1_TXE) < 0) I2C_FAIL();
    LL_I2C_TransmitData8(I2C1, reg);

    if (wait_flag_set(&I2C1->SR1, I2C_SR1_BTF) < 0) I2C_FAIL();

    if (i2c1_start_addr(addr7, 1) < 0) I2C_FAIL();
    if (i2c1_read_bytes(data, len) < 0) I2C_FAIL();

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
    if (!data || len == 0U) return -1;

    if (wait_flag_clr(&I2C1->SR2, I2C_SR2_BUSY) < 0) I2C_FAIL();

    i2c1_dma_tx_done = 0;
    i2c1_dma_err = 0;

    if (i2c1_start_addr(dev_addr, 0) < 0) I2C_FAIL();

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    LL_I2C_EnableDMAReq_TX(I2C1);
    dma_i2c1_tx((uint32_t)data, len);

    if (wait_dma_done(&i2c1_dma_tx_done) < 0) {
        LL_I2C_DisableDMAReq_TX(I2C1);
        LL_I2C_GenerateStopCondition(I2C1);
        i2c1_recover();
        return -1;
    }

    if (i2c1_dma_err) {
        LL_I2C_DisableDMAReq_TX(I2C1);
        LL_I2C_GenerateStopCondition(I2C1);
        i2c1_recover();
        return -1;
    }

    if (wait_flag_set(&I2C1->SR1, I2C_SR1_BTF) < 0) {
        LL_I2C_DisableDMAReq_TX(I2C1);
        LL_I2C_GenerateStopCondition(I2C1);
        return -1;
    }

    LL_I2C_DisableDMAReq_TX(I2C1);
    LL_I2C_GenerateStopCondition(I2C1);
    return 1;
}

int i2c1_read_raw_dma(uint8_t dev_addr, uint8_t *data, uint16_t len)
{
    if (!data || len == 0U) return -1;

    if (wait_flag_clr(&I2C1->SR2, I2C_SR2_BUSY) < 0) I2C_FAIL();

    i2c1_dma_rx_done = 0;
    i2c1_dma_err = 0;

    if (len == 1U) {
        return i2c1_read_raw(dev_addr, data, 1U);
    }

    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
    SET_BIT(I2C1->CR2, I2C_CR2_LAST);

    if (i2c1_start_addr(dev_addr, 1) < 0) I2C_FAIL();

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    LL_I2C_EnableDMAReq_RX(I2C1);
    dma_i2c1_rx((uint32_t)data, len);

    if (wait_dma_done(&i2c1_dma_rx_done) < 0) {
        LL_I2C_DisableDMAReq_RX(I2C1);
        LL_I2C_GenerateStopCondition(I2C1);
        i2c1_recover();
        return -1;
    }

    if (i2c1_dma_err) {
        LL_I2C_DisableDMAReq_RX(I2C1);
        LL_I2C_GenerateStopCondition(I2C1);
        i2c1_recover();
        return -1;
    }

    LL_I2C_DisableDMAReq_RX(I2C1);
    LL_I2C_GenerateStopCondition(I2C1);

    CLEAR_BIT(I2C1->CR2, I2C_CR2_LAST);
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);

    return 1;
}

int i2c1_write_u8_u16_dma(uint8_t addr7, uint8_t reg, uint16_t value)
{
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = (uint8_t)((value >> 8) & 0xFF);
    buf[2] = (uint8_t)(value & 0xFF);
    return i2c1_write_raw_dma(addr7, buf, 3);
}

int i2c1_read_u8_u16_dma(uint8_t addr7, uint8_t reg, uint16_t *value)
{
    if (!value) return -1;

    if (i2c1_reg_read(addr7, reg, (uint8_t *)value, 2) < 0) return -1;

    uint8_t *p = (uint8_t *)value;
    *value = ((uint16_t)p[0] << 8) | p[1];
    return 1;
}