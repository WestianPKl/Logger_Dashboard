#include "main.h"
#include <stdio.h>
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "stm32f412rx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "i2c.h"
#include "sht40.h"
#include "lcd.h"
#include "uart.h"
#include "bme280.h"
#include "spi.h"
#include "rtc.h"
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

sht40_data_t sht40_data = {0};
bme280_data_t bme280_data = {0};
rtc_date_time_t rtc_date_time = {0};

volatile uint16_t flag_100ms = 0U;

volatile uint16_t adc_data_buffer[ADC_BUFFER_SIZE];

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
CAN_HandleTypeDef hcan1;
I2C_HandleTypeDef hi2c1;
RTC_HandleTypeDef hrtc;
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim8;
TIM_HandleTypeDef htim13;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

void SystemClock_Config(void);
static void MX_GPIO_EXTI_Enable(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_CAN1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM8_Init(void);
static void MX_TIM13_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_RTC_Init(void);
static void lcd_render_screen(void);
static void lcd_backlight_timer_callback(TimerHandle_t xTimer);
static void esp32_timer_callback(TimerHandle_t xTimer);
static void measure_timer_callback(TimerHandle_t xTimer);
static void measure_trigger_update(void);

TimerHandle_t backlightTimerHandle;
TimerHandle_t esp32TimerHandle;
TimerHandle_t measureTimerHandle;
SemaphoreHandle_t i2cMutex;
SemaphoreHandle_t spiMutex;
SemaphoreHandle_t canMutex;
SemaphoreHandle_t rtcDataMutex;
SemaphoreHandle_t sensorDataMutex;
SemaphoreHandle_t flashLogMutex;
QueueHandle_t uartRxQueue;
QueueHandle_t canQueue;
QueueHandle_t framCmdQueue;
QueueHandle_t lcdCmdQueue;
QueueHandle_t rtcCmdQueue;
TaskHandle_t FRAMTaskHandle, RTCTaskHandle, CANTaskHandle, SensorTaskHandle, LCDTaskHandle, ReceiverTaskHandle, Button1TaskHandle, Button2TaskHandle, MeasureTaskHandle;

void FRAMTask(void *argument);
void RTCTask(void *argument);
void CANTask(void *argument);
void SensorTask(void *argument);
void LCDTask(void *argument);
void ReceiverTask(void *argument);
void Button1Task(void *argument);
void Button2Task(void *argument);
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
        EventBits_t bits_to_clear = EVT_EXT_RTC_PRESENT |
                                    EVT_FLASH_PRESENT |
                                    EVT_LCD_PRESENT |
                                    EVT_SHT40_PRESENT |
                                    EVT_BME280_PRESENT |
                                    EVT_ADC_PRESENT |
                                    EVT_CAN_PRESENT;
        EventBits_t bits_to_set = 0;

        if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
        {
            if (fm24cl16b_read_byte(FRAM_ADDR_FLAGS, &cfg_flags) == 1)
            {
                if (cfg_flags & FRAM_FLAG_EXT_RTC_PRESENT) bits_to_set |= EVT_EXT_RTC_PRESENT;
                if (cfg_flags & FRAM_FLAG_FLASH_PRESENT)   bits_to_set |= EVT_FLASH_PRESENT;
                if (cfg_flags & FRAM_FLAG_DISPLAY_PRESENT) bits_to_set |= EVT_LCD_PRESENT;
                if (cfg_flags & FRAM_FLAG_SHT40_PRESENT)   bits_to_set |= EVT_SHT40_PRESENT;
                if (cfg_flags & FRAM_FLAG_BME280_PRESENT)  bits_to_set |= EVT_BME280_PRESENT;
                if (cfg_flags & FRAM_FLAG_ADC_PRESENT)     bits_to_set |= EVT_ADC_PRESENT;
                if (cfg_flags & FRAM_FLAG_CAN_PRESENT)     bits_to_set |= EVT_CAN_PRESENT;
            }
            xSemaphoreGive(i2cMutex);
        }

        xEventGroupClearBits(appEvents, bits_to_clear);
        xEventGroupSetBits(appEvents, bits_to_set);
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
                        EventBits_t bits_to_set = 0;
                        EventBits_t bits_to_clear = EVT_EXT_RTC_PRESENT |
                                                    EVT_FLASH_PRESENT |
                                                    EVT_LCD_PRESENT |
                                                    EVT_SHT40_PRESENT |
                                                    EVT_BME280_PRESENT |
                                                    EVT_ADC_PRESENT |
                                                    EVT_CAN_PRESENT;

                        if (fm24cl16b_write_byte(FRAM_ADDR_FLAGS, cmd.data[0]) == 1)
                        {
                            cfg_flags = cmd.data[0];

                            if (cfg_flags & FRAM_FLAG_EXT_RTC_PRESENT) bits_to_set |= EVT_EXT_RTC_PRESENT;
                            if (cfg_flags & FRAM_FLAG_FLASH_PRESENT)   bits_to_set |= EVT_FLASH_PRESENT;
                            if (cfg_flags & FRAM_FLAG_DISPLAY_PRESENT) bits_to_set |= EVT_LCD_PRESENT;
                            if (cfg_flags & FRAM_FLAG_SHT40_PRESENT)   bits_to_set |= EVT_SHT40_PRESENT;
                            if (cfg_flags & FRAM_FLAG_BME280_PRESENT)  bits_to_set |= EVT_BME280_PRESENT;
                            if (cfg_flags & FRAM_FLAG_ADC_PRESENT)     bits_to_set |= EVT_ADC_PRESENT;
                            if (cfg_flags & FRAM_FLAG_CAN_PRESENT)     bits_to_set |= EVT_CAN_PRESENT;

                            xEventGroupClearBits(appEvents, bits_to_clear);
                            xEventGroupSetBits(appEvents, bits_to_set);
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
                        EventBits_t bits_to_set = 0;
                        EventBits_t bits_to_clear = EVT_EXT_RTC_PRESENT |
                                                    EVT_FLASH_PRESENT |
                                                    EVT_LCD_PRESENT |
                                                    EVT_SHT40_PRESENT |
                                                    EVT_BME280_PRESENT |
                                                    EVT_ADC_PRESENT |
                                                    EVT_CAN_PRESENT;

                        if (fm24cl16b_read_byte(FRAM_ADDR_FLAGS, &cfg_flags) == 1)
                        {
                            cmd.data[0] = cfg_flags;
                            cmd.len = 1U;

                            if (cfg_flags & FRAM_FLAG_EXT_RTC_PRESENT) bits_to_set |= EVT_EXT_RTC_PRESENT;
                            if (cfg_flags & FRAM_FLAG_FLASH_PRESENT)   bits_to_set |= EVT_FLASH_PRESENT;
                            if (cfg_flags & FRAM_FLAG_DISPLAY_PRESENT) bits_to_set |= EVT_LCD_PRESENT;
                            if (cfg_flags & FRAM_FLAG_SHT40_PRESENT)   bits_to_set |= EVT_SHT40_PRESENT;
                            if (cfg_flags & FRAM_FLAG_BME280_PRESENT)  bits_to_set |= EVT_BME280_PRESENT;
                            if (cfg_flags & FRAM_FLAG_ADC_PRESENT)     bits_to_set |= EVT_ADC_PRESENT;
                            if (cfg_flags & FRAM_FLAG_CAN_PRESENT)     bits_to_set |= EVT_CAN_PRESENT;

                            xEventGroupClearBits(appEvents, bits_to_clear);
                            xEventGroupSetBits(appEvents, bits_to_set);

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

void CANTask(void *argument)
{
    if (xEventGroupGetBits(appEvents) & EVT_CAN_PRESENT)
    {
        if (xSemaphoreTake(canMutex, portMAX_DELAY) == pdTRUE)
        {
            if (can_filter_accept_all() != 1)
            {
                xEventGroupClearBits(appEvents, EVT_CAN_PRESENT);
            }
            else if (can_start() != 1)
            {
                xEventGroupClearBits(appEvents, EVT_CAN_PRESENT);
            }

            xSemaphoreGive(canMutex);
        }
        else
        {
            xEventGroupClearBits(appEvents, EVT_CAN_PRESENT);
        }
    }

    while (1)
    {
        if (xEventGroupGetBits(appEvents) & EVT_CAN_PRESENT)
        {
            if (xSemaphoreTake(canMutex, pdMS_TO_TICKS(200)) == pdTRUE)
            {
                send_cyclic_frames();
                xSemaphoreGive(canMutex);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(400));
    }
}

void RTCTask(void *argument)
{
    rtc_date_time_t local_date_time = {0};
    rtc_msg_t msg;
    EventBits_t bits;

    bits = xEventGroupGetBits(appEvents);

    if (bits & EVT_EXT_RTC_PRESENT)
    {
        if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
        {
            int rc1 = mcp7940n_init(0);
            int rc2 = mcp7940n_mfp_sqw_1hz();

            if ((rc1 != 1) || (rc2 != 1)) {
                xEventGroupClearBits(appEvents, EVT_EXT_RTC_PRESENT);
            }
            measure_trigger_update();

            xSemaphoreGive(i2cMutex);
        }
    }

    while (1)
    {
        if (xQueueReceive(rtcCmdQueue, &msg, 0) == pdTRUE)
        {
            if (msg.cmd == RTC_CMD_SET_DATETIME)
            {
                if (xEventGroupGetBits(appEvents) & EVT_EXT_RTC_PRESENT)
                {
                    if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
                    {
                        (void)mcp7940n_set_datetime(&msg.ext_datetime);
                        xSemaphoreGive(i2cMutex);
                    }
                }
                else
                {
                    rtc_set_datetime(msg.datetime.year,
                                     msg.datetime.month,
                                     msg.datetime.day,
                                     msg.datetime.weekday,
                                     msg.datetime.hours,
                                     msg.datetime.minutes,
                                     msg.datetime.seconds);
                }
            }
        }

        bits = xEventGroupGetBits(appEvents);
        if (bits & EVT_RTC_REINIT)
        {
            if (bits & EVT_EXT_RTC_PRESENT)
            {
                if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
                {
                    int rc1 = mcp7940n_init(0);
                    int rc2 = mcp7940n_mfp_sqw_1hz();

                    if ((rc1 != 1) || (rc2 != 1)) {
                        xEventGroupClearBits(appEvents, EVT_EXT_RTC_PRESENT);
                    }
                    measure_trigger_update();

                    xSemaphoreGive(i2cMutex);
                }
            }

            xEventGroupClearBits(appEvents, EVT_RTC_REINIT);
        }

        if (xSemaphoreTake(rtcDataMutex, portMAX_DELAY) == pdTRUE)
        {
            bits = xEventGroupGetBits(appEvents);

            if (bits & EVT_EXT_RTC_PRESENT)
            {
                if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
                {
                    mcp7940n_datetime_t mcp_dt;
                    if (mcp7940n_get_datetime(&mcp_dt) == 1)
                    {
                        local_date_time.year     = mcp_dt.year;
                        local_date_time.month    = mcp_dt.month;
                        local_date_time.day      = mcp_dt.mday;
                        local_date_time.weekday  = mcp_dt.wday;
                        local_date_time.hours    = mcp_dt.hour;
                        local_date_time.minutes  = mcp_dt.min;
                        local_date_time.seconds  = mcp_dt.sec;
                    }
                    xSemaphoreGive(i2cMutex);
                }
            }
            else
            {
                rtc_get_datetime(&local_date_time.year,
                                 &local_date_time.month,
                                 &local_date_time.day,
                                 &local_date_time.weekday,
                                 &local_date_time.hours,
                                 &local_date_time.minutes,
                                 &local_date_time.seconds);
            }

            rtc_date_time = local_date_time;
            xSemaphoreGive(rtcDataMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
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

void LCDTask(void *argument)
{
    uint32_t notifyValue;
    lcd_msg_t msg;
    uint8_t need_render;

    EventBits_t bits = xEventGroupGetBits(appEvents);
    if ((bits & EVT_LCD_PRESENT) && xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
    {
        lcd_init();
        lcd_backlight(0);
        if (!lcd_is_present()) {
            xEventGroupClearBits(appEvents, EVT_LCD_PRESENT);
        }
        xSemaphoreGive(i2cMutex);
    }

    lcd_render_screen();

    while (1)
    {
        need_render = 0U;
        xTaskNotifyWait(0, 0xFFFFFFFFU, &notifyValue, portMAX_DELAY);

        if (notifyValue & LCD_NOTIFY_REINIT)
        {
            EventBits_t bits = xEventGroupGetBits(appEvents);
            if ((bits & EVT_LCD_PRESENT) && xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
            {
                lcd_init();
                lcd_backlight(0);
                if (!lcd_is_present()) {
                    xEventGroupClearBits(appEvents, EVT_LCD_PRESENT);
                }
                xSemaphoreGive(i2cMutex);
            }
            need_render = 1U;
        }

        if (notifyValue & LCD_NOTIFY_COMMAND)
        {
            while (xQueueReceive(lcdCmdQueue, &msg, 0) == pdTRUE)
            {
                if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
                {
                    if (msg.cmd == LCD_BACKLIGHT) {
                        lcd_backlight(msg.flag);
                    } else if (msg.cmd == LCD_CLEAR) {
                        lcd_clear();
                        need_render = 1U;
                    }
                    xSemaphoreGive(i2cMutex);
                }
            }
        }

        if ((notifyValue & LCD_NOTIFY_REFRESH) || need_render)
        {
            lcd_render_screen();
        }
    }
}

static void lcd_render_screen(void)
{
    rtc_date_time_t date_time_local = {0};
    sht40_data_t sht40_data_local = {0};
    bme280_data_t bme280_data_local = {0};
    EventBits_t bits;
    uint8_t second_marker;
    uint16_t current_year;
    uint8_t rtc_not_synced;

    if (xSemaphoreTake(sensorDataMutex, portMAX_DELAY) == pdTRUE)
    {
        sht40_data_local = sht40_data;
        bme280_data_local = bme280_data;
        xSemaphoreGive(sensorDataMutex);
    }

    if (xSemaphoreTake(rtcDataMutex, portMAX_DELAY) == pdTRUE)
    {
        date_time_local = rtc_date_time;
        xSemaphoreGive(rtcDataMutex);
    }

    rtc_utc_to_warsaw(&date_time_local.year, &date_time_local.month, &date_time_local.day,
                      &date_time_local.weekday, &date_time_local.hours,
                      &date_time_local.minutes, &date_time_local.seconds);

    second_marker = (date_time_local.seconds & 1U);
    bits = xEventGroupGetBits(appEvents);

    if ((bits & EVT_LCD_PRESENT) == 0U) {
        return;
    }

    current_year = (uint16_t)(2000U + date_time_local.year);
    rtc_not_synced = (current_year <= 2000U) ? 1U : 0U;

    if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
    {
        lcd_set_cursor(0, 0);

        if (rtc_not_synced)
        {
            lcd_send_string("Initializing... ");
            lcd_set_cursor(0, 1);
            lcd_send_string("                ");
            xSemaphoreGive(i2cMutex);
            return;
        }

        lcd_send_decimal(current_year, 4);
        lcd_send_string("-");
        lcd_send_decimal(date_time_local.month, 2);
        lcd_send_string("-");
        lcd_send_decimal(date_time_local.day, 2);
        lcd_send_string(" ");
        lcd_send_decimal(date_time_local.hours, 2);
        lcd_send_string(second_marker ? ":" : " ");
        lcd_send_decimal(date_time_local.minutes, 2);

        lcd_set_cursor(0, 1);
        lcd_send_string("TH:");
        lcd_send_string(" ");
        if ((bits & EVT_SHT40_PRESENT) && !(bits & EVT_BME280_PRESENT))
        {

            lcd_send_temp_1dp_from_x100(sht40_data_local.temperature);
            lcd_send_string(" ");
            lcd_send_hum_1dp_from_x100(sht40_data_local.humidity);
            lcd_send_string("  ");
        }
        else if (bits & EVT_BME280_PRESENT)
        {
            lcd_send_temp_1dp_from_x100(bme280_data_local.temperature);
            lcd_send_string(" ");
            lcd_send_hum_1dp_from_x100(bme280_data_local.humidity);
            lcd_send_string("  ");
        }
        else
        {
            lcd_send_string("No sensor data   ");
        }

        xSemaphoreGive(i2cMutex);
    }
}

static void lcd_backlight_timer_callback(TimerHandle_t xTimer)
{
    lcd_msg_t msg = { .cmd = LCD_BACKLIGHT, .flag = 0U };

    if (lcdCmdQueue != NULL)
    {
        (void)xQueueSend(lcdCmdQueue, &msg, 0);
    }

    if (LCDTaskHandle != NULL)
    {
        xTaskNotify(LCDTaskHandle, LCD_NOTIFY_COMMAND, eSetBits);
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
        uint8_t append;

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

static void measure_trigger_update(void)
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

void ReceiverTask(void *argument)
{
    uart_rx_msg_t msg;

    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_rx_buf, UART1_RX_BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);

    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart2_rx_buf, UART2_RX_BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);

    while (1)
    {
        if (xQueueReceive(uartRxQueue, &msg, portMAX_DELAY) == pdTRUE)
        {
            uart_feed_bytes(msg.data, msg.len, msg.use_uart1);
        }
    }
}

void Button1Task(void *argument)
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        app_io_state.btn1_pressed = 0U;

        if (HAL_GPIO_ReadPin(GPIOB, BTN1_Pin) == GPIO_PIN_RESET)
        {
            app_io_state.btn1_pressed = 1U;
            HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
            app_io_state.led1_state =
                (HAL_GPIO_ReadPin(LED1_GPIO_Port, LED1_Pin) == GPIO_PIN_SET) ? 1U : 0U;

            vTaskDelay(pdMS_TO_TICKS(10));

            while (HAL_GPIO_ReadPin(GPIOB, BTN1_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            app_io_state.btn1_pressed = 0U;
        }
    }
}

void Button2Task(void *argument)
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        app_io_state.btn2_pressed = 0U;

        if (HAL_GPIO_ReadPin(GPIOB, BTN2_Pin) == GPIO_PIN_RESET)
        {
            app_io_state.btn2_pressed = 1U;

            if (xEventGroupGetBits(appEvents) & EVT_LCD_PRESENT)
            {
                if (lcdCmdQueue != NULL)
                {
                  lcd_msg_t msg = { .cmd = LCD_BACKLIGHT, .flag = 1U };
                  (void)xQueueSend(lcdCmdQueue, &msg, 0);

                  if (LCDTaskHandle != NULL) {
                      xTaskNotify(LCDTaskHandle, LCD_NOTIFY_COMMAND, eSetBits);
                  }
                }
                (void)xTimerReset(backlightTimerHandle, 0);
            }

            vTaskDelay(pdMS_TO_TICKS(10));

            while (HAL_GPIO_ReadPin(GPIOB, BTN2_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            app_io_state.btn2_pressed = 0U;
        }
    }
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

static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }


  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_data_buffer, 2U) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_DMA_DISABLE_IT(&hdma_adc1, DMA_IT_TC);
  __HAL_DMA_DISABLE_IT(&hdma_adc1, DMA_IT_HT);
  __HAL_DMA_DISABLE_IT(&hdma_adc1, DMA_IT_TE);
}

static void MX_CAN1_Init(void)
{
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 6;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_11TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_RTC_Init(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    if (HAL_RTC_Init(&hrtc) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0xA5A5U) {
        sTime.Hours = 0x00;
        sTime.Minutes = 0x00;
        sTime.Seconds = 0x00;
        sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        sTime.StoreOperation = RTC_STOREOPERATION_RESET;

        if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK) {
            Error_Handler();
        }

        sDate.WeekDay = RTC_WEEKDAY_MONDAY;
        sDate.Month   = RTC_MONTH_JANUARY;
        sDate.Date    = 0x01;
        sDate.Year    = 0x00;

        if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK) {
            Error_Handler();
        }

        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0xA5A5U);
    }
}

static void MX_SPI1_Init(void)
{
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM1_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 83;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

static void MX_TIM2_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 83;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
}

static void MX_TIM3_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 83;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

static void MX_TIM4_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 83;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim4);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
}

static void MX_TIM8_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  htim8.Instance = TIM8;
  htim8.Init.Prescaler = 83;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = 999;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim8) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim8);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
}

static void MX_TIM13_Init(void)
{
  htim13.Instance = TIM13;
  htim13.Init.Prescaler = 8399;
  htim13.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim13.Init.Period = 999;
  htim13.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim13.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim13) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim13);
  HAL_TIM_Base_Start_IT(&htim13);
}

static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

  if (huart2.hdmarx != NULL) {
    __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
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

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOC, FLASH_Hold_Pin|CON_1_Pin|CON_2_Pin|CON_3_Pin
                          |CON_4_Pin|ESP32_Write_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB, ESP_GPIO01_Pin|CON_5_Pin|ESP32_Reset_Pin|LED1_Pin
                          |LED2_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(SPI1_CS2_GPIO_Port, SPI1_CS2_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SPI1_CS1_GPIO_Port, SPI1_CS1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC, FLASH_Hold_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = FLASH_Hold_Pin|CON_1_Pin|CON_2_Pin|CON_3_Pin
                          |CON_4_Pin|ESP32_Write_Pin|SPI1_CS2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SPI1_CS1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI1_CS1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = ESP32_Status_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ESP32_Status_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = BTN1_Pin|BTN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = ESP_GPIO01_Pin|CON_5_Pin|ESP32_Reset_Pin|LED1_Pin
                          |LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = MFP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MFP_GPIO_Port, &GPIO_InitStruct);
}

static void MX_GPIO_EXTI_Enable(void)
{
  __HAL_GPIO_EXTI_CLEAR_IT(BTN1_Pin);
  __HAL_GPIO_EXTI_CLEAR_IT(BTN2_Pin);
  __HAL_GPIO_EXTI_CLEAR_IT(MFP_Pin);

  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (GPIO_Pin == BTN1_Pin) {
        if (Button1TaskHandle != NULL) {
            xTaskNotifyFromISR(Button1TaskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
        }
    }
    else if (GPIO_Pin == BTN2_Pin) {
        if (Button2TaskHandle != NULL) {
            xTaskNotifyFromISR(Button2TaskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
        }
    }
    else if (GPIO_Pin == MFP_Pin) {
        if (MeasureTaskHandle != NULL) {
            vTaskNotifyGiveFromISR(MeasureTaskHandle, &xHigherPriorityTaskWoken);
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  } else if (htim->Instance == TIM13)
  {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (LCDTaskHandle != NULL) {
      flag_100ms++;
      if (flag_100ms >= 10U) {
          flag_100ms = 0U;
          xTaskNotifyFromISR(LCDTaskHandle, LCD_NOTIFY_REFRESH, eSetBits, &xHigherPriorityTaskWoken);
      }
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
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
