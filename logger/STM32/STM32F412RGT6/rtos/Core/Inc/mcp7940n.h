#ifndef MCP7940N_H
#define MCP7940N_H

#include <stdint.h>
#include "main.h"
#include "rtc.h"

#define MCP7940N_ADDR              0x6F

#define MCP7940N_REG_RTCSEC        0x00
#define MCP7940N_REG_RTCMIN        0x01
#define MCP7940N_REG_RTCHOUR       0x02
#define MCP7940N_REG_RTCWKDAY      0x03
#define MCP7940N_REG_RTCDATE       0x04
#define MCP7940N_REG_RTCMTH        0x05
#define MCP7940N_REG_RTCYEAR       0x06
#define MCP7940N_REG_CONTROL       0x07
#define MCP7940N_REG_OSCTRIM       0x08

#define MCP7940N_REG_ALM0SEC       0x0A
#define MCP7940N_REG_ALM1SEC       0x11

#define MCP7940N_REG_PWRDNMIN      0x18
#define MCP7940N_REG_SRAM_START    0x20
#define MCP7940N_REG_SRAM_END      0x5F
#define MCP7940N_SRAM_SIZE         64U

#define MCP7940N_BIT_ST            0x80
#define MCP7940N_BIT_VBATEN        0x08
#define MCP7940N_BIT_PWRFAIL       0x10
#define MCP7940N_BIT_OSCRUN        0x20

#define MCP7940N_BIT_OUT       0x80
#define MCP7940N_BIT_SQWEN     0x40
#define MCP7940N_BIT_ALM1EN    0x20
#define MCP7940N_BIT_ALM0EN    0x10
#define MCP7940N_BIT_EXTOSC    0x08
#define MCP7940N_BIT_CRSTRIM   0x04
#define MCP7940N_BIT_SQWFS1    0x02
#define MCP7940N_BIT_SQWFS0    0x01

/*
    * @brief  Structure holding a date/time snapshot for the MCP7940N external RTC.
*/
typedef struct {
    uint8_t sec;    // 0..59
    uint8_t min;    // 0..59
    uint8_t hour;   // 0..23
    uint8_t wday;   // 1..7
    uint8_t mday;   // 1..31
    uint8_t month;  // 1..12
    uint8_t year;   // 0..99
} mcp7940n_datetime_t;

/*
    * @brief  RTC command types for the RTC task command queue.
*/
typedef enum {
    RTC_CMD_SET_DATETIME = 0,
    RTC_CMD_GET_DATETIME
} rtc_cmd_t;

/*
    * @brief  RTC message structure for the RTC task command queue.
*/
typedef struct {
    rtc_cmd_t cmd;
    mcp7940n_datetime_t ext_datetime;
    rtc_date_time_t datetime;
    uint8_t flag;
} rtc_msg_t;


/*
    * @brief  Initialize the MCP7940N: enable the oscillator and optionally enable battery backup.
    * @param  enable_battery_backup: Non-zero to enable VBATEN, 0 to disable.
    * @retval 1 on success, -1 on failure.
*/
int mcp7940n_init(uint8_t enable_battery_backup);

/*
    * @brief  Read the current date and time from the MCP7940N registers.
    * @param  dt: Pointer to the structure where the date/time will be stored.
    * @retval 1 on success, -1 on failure.
*/
int mcp7940n_get_datetime(mcp7940n_datetime_t *dt);

/*
    * @brief  Set the date and time on the MCP7940N, validating ranges before writing.
    * @param  dt: Pointer to the structure containing the date/time to set.
    * @retval 1 on success, -1 on failure.
*/
int mcp7940n_set_datetime(const mcp7940n_datetime_t *dt);

/*
    * @brief  Read a single register from the MCP7940N via interrupt-driven I2C.
    * @param  reg: Register address.
    * @param  value: Pointer where the read byte will be stored.
    * @retval 1 on success, -1 on failure.
*/
int mcp7940n_read_reg(uint8_t reg, uint8_t *value);

/*
    * @brief  Write a single register on the MCP7940N via interrupt-driven I2C.
    * @param  reg: Register address.
    * @param  value: Byte value to write.
    * @retval 1 on success, -1 on failure.
*/
int mcp7940n_write_reg(uint8_t reg, uint8_t value);

/*
    * @brief  Read multiple consecutive registers from the MCP7940N.
    * @param  reg: Starting register address.
    * @param  data: Pointer to the destination buffer.
    * @param  len: Number of bytes to read.
    * @retval 1 on success, -1 on failure.
*/
int mcp7940n_read_regs(uint8_t reg, uint8_t *data, uint16_t len);

/*
    * @brief  Write multiple consecutive registers on the MCP7940N.
    * @param  reg: Starting register address.
    * @param  data: Pointer to the source data.
    * @param  len: Number of bytes to write.
    * @retval 1 on success, -1 on failure.
*/
int mcp7940n_write_regs(uint8_t reg, const uint8_t *data, uint16_t len);

/*
    * @brief  Read data from the MCP7940N 64-byte battery-backed SRAM.
    * @param  offset: Offset within the SRAM area (0..63).
    * @param  data: Pointer to the destination buffer.
    * @param  len: Number of bytes to read.
    * @retval 1 on success, -1 on failure.
*/
int mcp7940n_read_sram(uint8_t offset, uint8_t *data, uint8_t len);

/*
    * @brief  Write data to the MCP7940N 64-byte battery-backed SRAM.
    * @param  offset: Offset within the SRAM area (0..63).
    * @param  data: Pointer to the source data.
    * @param  len: Number of bytes to write.
    * @retval 1 on success, -1 on failure.
*/
int mcp7940n_write_sram(uint8_t offset, const uint8_t *data, uint8_t len);

/*
    * @brief  Enable the MFP output driven by Alarm 0, disabling SQW and Alarm 1.
    * @retval 1 on success, -1 on failure.
*/
int mcp7940n_mfp_alarm0_enable(void);

/*
    * @brief  Configure the MFP output as a 1 Hz square wave, disabling alarms.
    * @retval 1 on success, -1 on failure.
*/
int mcp7940n_mfp_sqw_1hz(void);

#endif // MCP7940N_H