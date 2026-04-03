#include "app_flags.h"
#include "main.h"
#include "stm32f412rx.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_hal.h"
#include "support.h"

extern CAN_HandleTypeDef hcan1;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}


void DMA1_Stream0_IRQHandler(void)
{
  if (LL_DMA_IsActiveFlag_TC0(DMA1)) {
    LL_DMA_ClearFlag_TC0(DMA1);
    i2c1_dma_err = 0;
    i2c1_dma_rx_done = 1;
  }

  if (LL_DMA_IsActiveFlag_TE0(DMA1)) {
    LL_DMA_ClearFlag_TE0(DMA1);
    i2c1_dma_err = 1;
    i2c1_dma_rx_done = 1;
  }
}

void DMA1_Stream1_IRQHandler(void)
{
  if (LL_DMA_IsActiveFlag_TC1(DMA1)) {
    LL_DMA_ClearFlag_TC1(DMA1);
    i2c1_dma_err = 0;
    i2c1_dma_tx_done = 1;
  }

  if (LL_DMA_IsActiveFlag_TE1(DMA1)) {
    LL_DMA_ClearFlag_TE1(DMA1);
    i2c1_dma_err = 1;
    i2c1_dma_tx_done = 1;
  }
}


void DMA1_Stream5_IRQHandler(void)
{
  if (LL_DMA_IsActiveFlag_TC5(DMA1)) {
    LL_DMA_ClearFlag_TC5(DMA1);
  }
  if (LL_DMA_IsActiveFlag_TE5(DMA1)) {
    LL_DMA_ClearFlag_TE5(DMA1);
  }
}


void DMA1_Stream6_IRQHandler(void)
{
  if (LL_DMA_IsActiveFlag_TE6(DMA1)) {
    LL_DMA_ClearFlag_TE6(DMA1);
    uart2_tx_busy = 0;
  }

  if (LL_DMA_IsActiveFlag_TC6(DMA1)) {
    LL_DMA_ClearFlag_TC6(DMA1);
    uart2_tx_busy = 0;
  }
}

void CAN1_TX_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&hcan1);
}

void CAN1_RX0_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&hcan1);
}

void CAN1_RX1_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&hcan1);
}

void CAN1_SCE_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&hcan1);
}

void I2C1_EV_IRQHandler(void)
{

}


void I2C1_ER_IRQHandler(void)
{

}

void SPI1_IRQHandler(void)
{

}


void TIM8_UP_TIM13_IRQHandler(void)
{
  if (LL_TIM_IsActiveFlag_UPDATE(TIM13) == 1)
  {
    LL_TIM_ClearFlag_UPDATE(TIM13);

    tick_1ms++;

    if ((tick_1ms % 100U) == 0U) {
      flag_100ms = 1U;
    }

    if (tick_1ms >= 1000U) {
      flag_1s = 1U;
      tick_1ms = 0U;
    }
  }
}

void DMA2_Stream0_IRQHandler(void)
{
  if (LL_DMA_IsActiveFlag_TC0(DMA2)) {
    LL_DMA_ClearFlag_TC0(DMA2);
    spi1_dma_rx_done = 1;
  }
  if (LL_DMA_IsActiveFlag_TE0(DMA2)) {
    LL_DMA_ClearFlag_TE0(DMA2);
    spi1_dma_rx_done = 1;
  }
}

void DMA2_Stream2_IRQHandler(void)
{
  if (LL_DMA_IsActiveFlag_TC2(DMA2)) {
    LL_DMA_ClearFlag_TC2(DMA2);
  }
  if (LL_DMA_IsActiveFlag_TE2(DMA2)) {
    LL_DMA_ClearFlag_TE2(DMA2);
  }
}


void DMA2_Stream3_IRQHandler(void)
{
  if (LL_DMA_IsActiveFlag_TC3(DMA2)) {
    LL_DMA_ClearFlag_TC3(DMA2);
    spi1_dma_tx_done = 1;
  }
  if (LL_DMA_IsActiveFlag_TE3(DMA2)) {
    LL_DMA_ClearFlag_TE3(DMA2);
    spi1_dma_tx_done = 1;
  }
}

void DMA2_Stream4_IRQHandler(void)
{
  if (LL_DMA_IsActiveFlag_TC4(DMA2)) {
    LL_DMA_ClearFlag_TC4(DMA2);
  }
  if (LL_DMA_IsActiveFlag_TE4(DMA2)) {
    LL_DMA_ClearFlag_TE4(DMA2);
  }
}

void DMA2_Stream7_IRQHandler(void)
{
  if (LL_DMA_IsActiveFlag_TE7(DMA2)) {
    LL_DMA_ClearFlag_TE7(DMA2);
    uart1_tx_busy = 0;
  }

  if (LL_DMA_IsActiveFlag_TC7(DMA2)) {
    LL_DMA_ClearFlag_TC7(DMA2);
    uart1_tx_busy = 0;
  }
}

void EXTI0_IRQHandler(void)
{
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) != RESET)
  {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);

    uint32_t now = HAL_GetTick();
    if ((now - btn1_last_tick) >= 30)
    {
      btn1_last_tick = now;
      btn1_flag = 1;
    }
  }
}

void EXTI1_IRQHandler(void)
{
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_1) != RESET)
  {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_1);

    uint32_t now = HAL_GetTick();
    if ((now - btn2_last_tick) >= 30)
    {
      btn2_last_tick = now;
      btn2_flag = 1;
    }
  }
}

void EXTI9_5_IRQHandler(void)
{
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_5) != RESET)
  {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_5);
    //MFP flag
  }
}