#include "main.h"
#include "cmsis_os.h"
#include "task.h"
#include "i2c.h"
#include "lcd.h"
#include "uart.h"
#include "spi.h"
#include "rtc_locale.h"
#include "fm24cl16b.h"
#include "mcp7940n.h"
#include "uart_protocol.h"
#include "flash_log.h"
#include "mx25l25673gm2i.h"
#include "support.h"
#include "event_groups.h"
#include "app_flags.h"
#include "timers.h"
#include "can.h"
#include "can_protocol.h"
#include "adc.h"
#include "timer.h"
#include "gpio.h"

sht40_data_t sht40_data = {0};
bme280_data_t bme280_data = {0};
rtc_date_time_t rtc_date_time = {0};



void SystemClock_Config(void);
static void MX_DMA_Init(void);
static void esp32_timer_callback(TimerHandle_t xTimer);
static void measure_timer_callback(TimerHandle_t xTimer);

TimerHandle_t esp32TimerHandle;
TimerHandle_t measureTimerHandle;

SemaphoreHandle_t sensorDataMutex;
SemaphoreHandle_t flashLogMutex;
QueueHandle_t framCmdQueue;

TaskHandle_t FRAMTaskHandle, SensorTaskHandle, MeasureTaskHandle;

void FRAMTask(void *argument);
void SensorTask(void *argument);
void MeasureTask(void *argument);

int main(void)
{
  __disable_irq();
  SCB->VTOR = 0x08008000U;
  __DSB();
  __ISB();

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_CAN1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM8_Init();
  MX_TIM13_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_RTC_Init();

  __enable_irq();

  osKernelInitialize();

  appEvents = xEventGroupCreate();

  i2cMutex = xSemaphoreCreateMutex();
  spiMutex = xSemaphoreCreateMutex();
  rtcDataMutex = xSemaphoreCreateMutex();
  sensorDataMutex = xSemaphoreCreateMutex();
  flashLogMutex = xSemaphoreCreateMutex();
  canMutex = xSemaphoreCreateMutex();

  if ((i2cMutex == NULL) || (spiMutex == NULL) || (rtcDataMutex == NULL) ||
    (sensorDataMutex == NULL) || (flashLogMutex == NULL) || (appEvents == NULL) || (canMutex == NULL))
  {
      Error_Handler();
  }

  uartRxQueue = xQueueCreate(10, sizeof(uart_rx_msg_t));
  framCmdQueue = xQueueCreate(10, sizeof(fram_msg_t));
  lcdCmdQueue = xQueueCreate(10, sizeof(lcd_msg_t));
  rtcCmdQueue = xQueueCreate(10, sizeof(rtc_msg_t));
  canQueue = xQueueCreate(10, sizeof(can_rx_msg_t));


  if ((uartRxQueue == NULL) || (framCmdQueue == NULL)  || (lcdCmdQueue == NULL) || (rtcCmdQueue == NULL)  || (canQueue == NULL)) {
    Error_Handler();
  }

  backlightTimerHandle = xTimerCreate("BacklightTimer", pdMS_TO_TICKS(20000), pdFALSE, NULL, lcd_backlight_timer_callback);
  esp32TimerHandle = xTimerCreate("ESP32Timer", pdMS_TO_TICKS(1000), pdFALSE, NULL, esp32_timer_callback);
  measureTimerHandle = xTimerCreate("MeasureTimer", pdMS_TO_TICKS(600000), pdTRUE, NULL, measure_timer_callback);

  if (backlightTimerHandle == NULL || esp32TimerHandle == NULL || measureTimerHandle == NULL)
  {
      Error_Handler();
  }

  if( xTaskCreate(ReceiverTask, "Receiver", 512, NULL, 3, &ReceiverTaskHandle) != pdPASS )
  {
      Error_Handler();
  }
  if( xTaskCreate(FRAMTask, "FRAM", 384, NULL, 2, &FRAMTaskHandle) != pdPASS )
  {
      Error_Handler();
  }

  if( xTaskCreate(Button1Task, "Button1", 128, NULL, 1, &Button1TaskHandle) != pdPASS )
  {
      Error_Handler();
  }
  if( xTaskCreate(Button2Task, "Button2", 128, NULL, 1, &Button2TaskHandle) != pdPASS )
  {
      Error_Handler();
  }

  MX_GPIO_EXTI_Enable();

  osKernelStart();

  while (1){}

}

void FRAMTask(void *argument)
{
    uint8_t cfg_flags = 0;
    fram_msg_t cmd;

    {
        if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
        {
            if (fm24cl16b_read_byte(FRAM_ADDR_FLAGS, &cfg_flags) == 1)
            {
                app_flags_apply(cfg_flags);
            }
            xSemaphoreGive(i2cMutex);
        }
    }
    if (xTaskCreate(LCDTask, "LCD", 384, NULL, 1, &LCDTaskHandle) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(RTCTask, "RTC", 384, NULL, 2, &RTCTaskHandle) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(SensorTask, "Sensor", 256, NULL, 2, &SensorTaskHandle) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(MeasureTask, "Measure", 384, NULL, 2, &MeasureTaskHandle) != pdPASS) {
        Error_Handler();
    }
    if (xTaskCreate(CANTask, "CAN", 256, NULL, 2, &CANTaskHandle) != pdPASS) {
        Error_Handler();
    }

    measure_trigger_update();

    if ((xEventGroupGetBits(appEvents) & EVT_EXT_RTC_PRESENT) == 0U)
    {
        xTaskNotifyGive(MeasureTaskHandle);
    }

    if (xEventGroupGetBits(appEvents) & EVT_FLASH_PRESENT)
    {
        if (flash_log_init() != 1)
        {
            xEventGroupClearBits(appEvents, EVT_FLASH_PRESENT);
        }
    }

    while (1)
    {
        uint8_t need_flash_init = 0U;

        if (xQueueReceive(framCmdQueue, &cmd, portMAX_DELAY) == pdTRUE)
        {
            cmd.status = -1;

            if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
            {
                switch (cmd.cmd)
                {
                    case FRAM_CMD_WRITE:
                    {
                        if ((cmd.len > 0U) &&
                            (cmd.len <= FRAM_MAX_DATA_LEN) &&
                            (((uint32_t)cmd.addr + cmd.len) <= FM24CL16B_SIZE_BYTES))
                        {
                            cmd.status = fm24cl16b_write(cmd.addr, cmd.data, cmd.len);
                        }
                        break;
                    }

                    case FRAM_CMD_READ:
                    {
                        if ((cmd.len > 0U) &&
                            (cmd.len <= FRAM_MAX_DATA_LEN) &&
                            (((uint32_t)cmd.addr + cmd.len) <= FM24CL16B_SIZE_BYTES))
                        {
                            cmd.status = fm24cl16b_read(cmd.addr, cmd.data, cmd.len);
                        }
                        break;
                    }

                    case FRAM_CMD_WRITE_FLAGS:
                    {
                        if (fm24cl16b_write_byte(FRAM_ADDR_FLAGS, cmd.data[0]) == 1)
                        {
                            cfg_flags = cmd.data[0];
                            app_flags_apply(cfg_flags);
                            xEventGroupSetBits(appEvents,
                                               EVT_LCD_REINIT |
                                               EVT_RTC_REINIT |
                                               EVT_BME280_REINIT);

                            if (LCDTaskHandle != NULL) {
                                xTaskNotify(LCDTaskHandle, LCD_NOTIFY_REINIT, eSetBits);
                            }

                            measure_trigger_update();

                            cmd.data[0] = cfg_flags;
                            cmd.len = 1U;
                            cmd.status = 1;

                            if (cfg_flags & FRAM_FLAG_FLASH_PRESENT)
                            {
                                need_flash_init = 1U;
                            }
                        }
                        break;
                    }

                    case FRAM_CMD_READ_FLAGS:
                    {
                        if (fm24cl16b_read_byte(FRAM_ADDR_FLAGS, &cfg_flags) == 1)
                        {
                            cmd.data[0] = cfg_flags;
                            cmd.len = 1U;
                            app_flags_apply(cfg_flags);
                            cmd.status = 1;
                        }
                        break;
                    }

                    default:
                    {
                        cmd.status = -1;
                        break;
                    }
                }

                xSemaphoreGive(i2cMutex);
            }

            if (need_flash_init)
            {
                if (flash_log_init() != 1)
                {
                    xEventGroupClearBits(appEvents, EVT_FLASH_PRESENT);
                }
            }

            if (cmd.replyTask != NULL)
            {
                xTaskNotify(cmd.replyTask, (uint32_t)cmd.status, eSetValueWithOverwrite);
            }
        }
    }
}

void SensorTask(void *argument)
{
    sht40_data_t sht40_data_local = {.temperature = 0, .humidity = 0};
    bme280_data_t bme280_data_local = {.temperature = 0, .humidity = 0, .pressure = 0};
    EventBits_t bits = xEventGroupGetBits(appEvents);

    if (bits & EVT_SHT40_PRESENT && xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
    {
        if (sht40_read_serial_number() == (uint32_t)-1)
        {
            xEventGroupClearBits(appEvents, EVT_SHT40_PRESENT);
        }
        xSemaphoreGive(i2cMutex);
    }

    if (bits & EVT_BME280_PRESENT)
    {
      if(xSemaphoreTake(spiMutex, portMAX_DELAY) == pdTRUE)
      {
        if (bme280_init() != 1)
        {
            xEventGroupClearBits(appEvents, EVT_BME280_PRESENT);
        }
        xSemaphoreGive(spiMutex);
      }
    }

    while (1)
    {
        sht40_data_local.temperature = 0;
        sht40_data_local.humidity = 0;

        bits = xEventGroupGetBits(appEvents);
        if (bits & EVT_SHT40_PRESENT)
        {
            if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
            {
                if (sht40_data_read_int(&sht40_data_local.temperature, &sht40_data_local.humidity) != 1)
                {
                    sht40_data_local.temperature = 0;
                    sht40_data_local.humidity = 0;
                    xEventGroupSetBits(appEvents, EVT_SHT40_ERROR);
                }
                else
                {
                    xEventGroupClearBits(appEvents, EVT_SHT40_ERROR);
                }

                xSemaphoreGive(i2cMutex);
            }
        }

        bits = xEventGroupGetBits(appEvents);
        if (bits & EVT_BME280_REINIT)
        {
            if (bits & EVT_BME280_PRESENT)
            {
                if (xSemaphoreTake(spiMutex, portMAX_DELAY) == pdTRUE)
                {
                    if (bme280_init() != 1)
                    {
                        xEventGroupClearBits(appEvents, EVT_BME280_PRESENT);
                    }
                    xSemaphoreGive(spiMutex);
                }
            }

            xEventGroupClearBits(appEvents, EVT_BME280_REINIT);
        }

        bme280_data_local.temperature = 0;
        bme280_data_local.humidity = 0;
        bme280_data_local.pressure = 0;

        bits = xEventGroupGetBits(appEvents);
        if (bits & EVT_BME280_PRESENT)
        {
            if (xSemaphoreTake(spiMutex, portMAX_DELAY) == pdTRUE)
            {
                if (bme280_trigger_forced() != 1)
                {
                    xEventGroupSetBits(appEvents, EVT_BME280_ERROR);
                }
                xSemaphoreGive(spiMutex);
            }

            vTaskDelay(pdMS_TO_TICKS(20));
            
            if (xSemaphoreTake(spiMutex, portMAX_DELAY) == pdTRUE)
            {
                if (bme280_read_data(&bme280_data_local.temperature,
                                     &bme280_data_local.humidity,
                                     &bme280_data_local.pressure) == 1)
                {
                    xEventGroupClearBits(appEvents, EVT_BME280_ERROR);
                }
                else
                {
                    bme280_data_local.temperature = 0;
                    bme280_data_local.humidity = 0;
                    bme280_data_local.pressure = 0;
                    xEventGroupSetBits(appEvents, EVT_BME280_ERROR);
                }

                xSemaphoreGive(spiMutex);
            }
        }

        if (xSemaphoreTake(sensorDataMutex, portMAX_DELAY) == pdTRUE)
        {
            sht40_data = sht40_data_local;
            bme280_data = bme280_data_local;
            xSemaphoreGive(sensorDataMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void MeasureTask(void *argument)
{
    uint32_t notify_count;
    uint32_t mfp_seconds = 0U;
    uint8_t last_ext_rtc_mode = 0U;

    while (1)
    {
        notify_count = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        EventBits_t bits = xEventGroupGetBits(appEvents);
        uint8_t ext_rtc_mode = ((bits & EVT_EXT_RTC_PRESENT) != 0U) ? 1U : 0U;

        if (ext_rtc_mode != last_ext_rtc_mode)
        {
            mfp_seconds = 0U;
            last_ext_rtc_mode = ext_rtc_mode;
        }

        if (ext_rtc_mode)
        {
            mfp_seconds += notify_count;

            if (mfp_seconds < 600U) {
                continue;
            }

            do {
                mfp_seconds -= 600U;
            } while (mfp_seconds >= 600U);
        }

        bits = xEventGroupGetBits(appEvents);
        if ((bits & EVT_FLASH_PRESENT) == 0U) {
            continue;
        }

        uint32_t timestamp = 0U;
        bme280_data_t bme280_data_local = {0};
        sht40_data_t sht40_data_local = {0};
        rtc_date_time_t rtc_date_time_local = {0};
        uint8_t append = 0U;

        if (xSemaphoreTake(rtcDataMutex, pdMS_TO_TICKS(50)) != pdTRUE) {
            continue;
        }

        rtc_date_time_local = rtc_date_time;
        xSemaphoreGive(rtcDataMutex);

        timestamp =
            ((uint32_t)rtc_date_time_local.year    << 26) |
            ((uint32_t)rtc_date_time_local.month   << 22) |
            ((uint32_t)rtc_date_time_local.day     << 17) |
            ((uint32_t)rtc_date_time_local.hours   << 12) |
            ((uint32_t)rtc_date_time_local.minutes << 6)  |
            ((uint32_t)rtc_date_time_local.seconds);

        if (xSemaphoreTake(sensorDataMutex, pdMS_TO_TICKS(50)) != pdTRUE) {
            continue;
        }

        bme280_data_local = bme280_data;
        sht40_data_local = sht40_data;
        xSemaphoreGive(sensorDataMutex);

        bits = xEventGroupGetBits(appEvents);

        if ((bits & EVT_BME280_PRESENT) && ((bits & EVT_BME280_ERROR) == 0U))
        {
            flash_log_append(timestamp, &bme280_data_local, NULL);
            append = 1U;
        }
        else if ((bits & EVT_SHT40_PRESENT) &&
                 ((bits & EVT_SHT40_ERROR) == 0U) &&
                 ((bits & EVT_BME280_PRESENT) == 0U))
        {

            flash_log_append(timestamp, NULL, &sht40_data_local);
            append = 1U;
        }

        if (append == 1U)
        {
            HAL_GPIO_WritePin(ESP32_Write_GPIO_Port, ESP32_Write_Pin, GPIO_PIN_SET);
            app_io_state.esp32_write_state = 1U;
            (void)xTimerReset(esp32TimerHandle, 0);
        }
    }   
}

void measure_trigger_update(void)
{
    EventBits_t bits = xEventGroupGetBits(appEvents);
    BaseType_t active;

    if (measureTimerHandle == NULL) {
        return;
    }

    active = xTimerIsTimerActive(measureTimerHandle);

    if (bits & EVT_EXT_RTC_PRESENT)
    {
        if (active != pdFALSE) {
            (void)xTimerStop(measureTimerHandle, 0);
        }
    }
    else
    {
        if (active == pdFALSE) {
            (void)xTimerStart(measureTimerHandle, 0);
        }
    }
}

static void measure_timer_callback(TimerHandle_t xTimer)
{
    if (MeasureTaskHandle != NULL)
    {
        xTaskNotifyGive(MeasureTaskHandle);
    }
}

static void esp32_timer_callback(TimerHandle_t xTimer)
{
    HAL_GPIO_WritePin(ESP32_Write_GPIO_Port, ESP32_Write_Pin, GPIO_PIN_RESET);
    app_io_state.esp32_write_state = 0U;
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    HAL_PWR_EnableBkUpAccess();

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI |
                                       RCC_OSCILLATORTYPE_LSI |
                                       RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 84;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 |
                                  RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_DMA_Init(void)
{

  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

  HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);

  HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
