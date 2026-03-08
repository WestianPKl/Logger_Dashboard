#include "rtc.h"

#define RTC_WPR_KEY1 0xCAU
#define RTC_WPR_KEY2 0x53U

#define RTC_PREDIV_A 127U
#define RTC_PREDIV_S 255U

void rtc_write_protect_disable(void)
{
    RTC->WPR = RTC_WPR_KEY1;
    RTC->WPR = RTC_WPR_KEY2;
}

void rtc_write_protect_enable(void)
{
    RTC->WPR = 0xFFU;
}

static void rtc_enter_init_mode(void)
{
    RTC->ISR |= RTC_ISR_INIT;
    while ((RTC->ISR & RTC_ISR_INITF) == 0U) {}
}

static void rtc_exit_init_mode(void)
{
    RTC->ISR &= ~RTC_ISR_INIT;
}

static void rtc_wait_for_synchro(void)
{
    RTC->ISR &= ~RTC_ISR_RSF;
    while ((RTC->ISR & RTC_ISR_RSF) == 0U) {}
}

static void rtc_exti_enable_rising(uint32_t line)
{
    EXTI->PR1   = (1UL << line);
    EXTI->IMR1  |= (1UL << line);
    EXTI->RTSR1 |= (1UL << line);
    EXTI->FTSR1 &= ~(1UL << line);
}

void rtc_exti_clear(uint32_t line)
{
    EXTI->PR1 = (1UL << line);
}

static uint8_t to_bcd(uint8_t v)
{
    return (uint8_t)(((v / 10U) << 4) | (v % 10U));
}

static uint8_t from_bcd(uint8_t b)
{
    return (uint8_t)(((b >> 4) * 10U) + (b & 0x0FU));
}

static uint8_t rtc_is_configured(void)
{
    return (RCC->BDCR & RCC_BDCR_RTCEN) != 0U;
}

void rtc_init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    (void)RCC->APB1ENR1;

    PWR->CR1 |= PWR_CR1_DBP;
    while ((PWR->CR1 & PWR_CR1_DBP) == 0U) {}


    if (!rtc_is_configured())
    {
        RCC->BDCR |= RCC_BDCR_LSEON;
        while ((RCC->BDCR & RCC_BDCR_LSERDY) == 0U) {}

        RCC->BDCR &= ~RCC_BDCR_RTCSEL_Msk;
        RCC->BDCR |= (1U << RCC_BDCR_RTCSEL_Pos);

        RCC->BDCR |= RCC_BDCR_RTCEN;

        rtc_write_protect_disable();
        rtc_enter_init_mode();

        RTC->CR = 0;

        RTC->PRER = ((RTC_PREDIV_A & 0x7FU) << RTC_PRER_PREDIV_A_Pos) |
                    ((RTC_PREDIV_S & 0x7FFFU) << RTC_PRER_PREDIV_S_Pos);

        rtc_exit_init_mode();
        rtc_write_protect_enable();
    }

    rtc_write_protect_disable();
    rtc_wait_for_synchro();
    rtc_write_protect_enable();
}

void rtc_write_time(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    if (hours > 23U) hours = 23U;
    if (minutes > 59U) minutes = 59U;
    if (seconds > 59U) seconds = 59U;

    rtc_write_protect_disable();
    rtc_enter_init_mode();

    uint32_t tr =
        ((uint32_t)to_bcd(hours)   << RTC_TR_HU_Pos) |
        ((uint32_t)to_bcd(minutes) << RTC_TR_MNU_Pos) |
        ((uint32_t)to_bcd(seconds) << RTC_TR_SU_Pos);

    RTC->TR = tr;

    rtc_exit_init_mode();
    rtc_write_protect_enable();
}

void rtc_write_date(uint8_t year, uint8_t month, uint8_t date, uint8_t weekday)
{
    if (year > 99U) year = (uint8_t)(year % 100U);
    if (month < 1U) month = 1U; else if (month > 12U) month = 12U;
    if (date  < 1U) date  = 1U; else if (date  > 31U) date  = 31U;
    if (weekday < 1U) weekday = 1U; else if (weekday > 7U) weekday = 7U;

    rtc_write_protect_disable();
    rtc_enter_init_mode();

    uint32_t dr =
        ((uint32_t)to_bcd(year)  << RTC_DR_YU_Pos) |
        ((uint32_t)to_bcd(month) << RTC_DR_MU_Pos) |
        ((uint32_t)to_bcd(date)  << RTC_DR_DU_Pos) |
        ((uint32_t)weekday       << RTC_DR_WDU_Pos);

    RTC->DR = dr;

    rtc_exit_init_mode();
    rtc_write_protect_enable();
}

void rtc_read_time(uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
    if (!hours || !minutes || !seconds) return;

    uint32_t tr1, tr2;
    do {
        tr1 = RTC->TR;
        tr2 = RTC->TR;
    } while (tr1 != tr2);

    uint8_t hb = (uint8_t)((tr1 & (RTC_TR_HT_Msk | RTC_TR_HU_Msk)) >> RTC_TR_HU_Pos);
    uint8_t mb = (uint8_t)((tr1 & (RTC_TR_MNT_Msk | RTC_TR_MNU_Msk)) >> RTC_TR_MNU_Pos);
    uint8_t sb = (uint8_t)((tr1 & (RTC_TR_ST_Msk | RTC_TR_SU_Msk)) >> RTC_TR_SU_Pos);

    *hours   = from_bcd(hb);
    *minutes = from_bcd(mb);
    *seconds = from_bcd(sb);
}

void rtc_read_date(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *weekday)
{
    if (!year || !month || !date || !weekday) return;

    uint32_t dr1, dr2;
    do {
        dr1 = RTC->DR;
        dr2 = RTC->DR;
    } while (dr1 != dr2);

    uint8_t yb = (uint8_t)((dr1 & (RTC_DR_YT_Msk | RTC_DR_YU_Msk)) >> RTC_DR_YU_Pos);
    uint8_t mb = (uint8_t)((dr1 & (RTC_DR_MT_Msk | RTC_DR_MU_Msk)) >> RTC_DR_MU_Pos);
    uint8_t db = (uint8_t)((dr1 & (RTC_DR_DT_Msk | RTC_DR_DU_Msk)) >> RTC_DR_DU_Pos);

    *year    = from_bcd(yb);
    *month   = from_bcd(mb);
    *date    = from_bcd(db);
    *weekday = (uint8_t)((dr1 & RTC_DR_WDU_Msk) >> RTC_DR_WDU_Pos);
}

void rtc_alarmA_disable(void)
{
    rtc_write_protect_disable();

    RTC->CR &= ~RTC_CR_ALRAE;
    while ((RTC->ISR & RTC_ISR_ALRAWF) == 0U) {}
    RTC->CR &= ~RTC_CR_ALRAIE;
    RTC->ISR &= ~RTC_ISR_ALRAF;

    RTC->ISR &= ~RTC_ISR_ALRAF;

    rtc_write_protect_enable();

    rtc_exti_clear(18U);
}

int rtc_alarmA_set_hms(uint8_t h, uint8_t m, uint8_t s, uint8_t daily)
{
    if (h > 23U || m > 59U || s > 59U) return -1;

    rtc_write_protect_disable();

    RTC->CR &= ~RTC_CR_ALRAE;
    while ((RTC->ISR & RTC_ISR_ALRAWF) == 0U) {}

    uint32_t alrmar = 0;
    alrmar |= ((uint32_t)to_bcd(s) << RTC_ALRMAR_SU_Pos);
    alrmar |= ((uint32_t)to_bcd(m) << RTC_ALRMAR_MNU_Pos);
    alrmar |= ((uint32_t)to_bcd(h) << RTC_ALRMAR_HU_Pos);

    if (daily) {
        alrmar |= RTC_ALRMAR_MSK4;
    } else {
        uint8_t yy, mo, dd, wd;
        rtc_read_date(&yy, &mo, &dd, &wd);

        uint8_t b = to_bcd(dd);
        alrmar |= ((uint32_t)(b & 0x0FU) << RTC_ALRMAR_DU_Pos);
        alrmar |= ((uint32_t)((b >> 4) & 0x03U) << RTC_ALRMAR_DT_Pos);
        alrmar &= ~RTC_ALRMAR_WDSEL;
    }

    RTC->ALRMAR = alrmar;

    RTC->ISR &= ~RTC_ISR_ALRAF;
    RTC->CR  |= RTC_CR_ALRAIE;
    RTC->CR  |= RTC_CR_ALRAE;

    rtc_write_protect_enable();

    rtc_exti_enable_rising(18U);
    NVIC_EnableIRQ(RTC_Alarm_IRQn);
    return 0;
}

int rtc_alarmA_set_day_hms(uint8_t day, uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    if (day < 1U || day > 31U) return -1;
    if (hours > 23U || minutes > 59U || seconds > 59U) return -1;

    rtc_write_protect_disable();

    RTC->CR &= ~RTC_CR_ALRAE;
    while ((RTC->ISR & RTC_ISR_ALRAWF) == 0U) {}

    uint32_t alrmar = 0;
    alrmar |= ((uint32_t)to_bcd(seconds) << RTC_ALRMAR_SU_Pos);
    alrmar |= ((uint32_t)to_bcd(minutes) << RTC_ALRMAR_MNU_Pos);
    alrmar |= ((uint32_t)to_bcd(hours)   << RTC_ALRMAR_HU_Pos);

    uint8_t db = to_bcd(day);
    alrmar |= ((uint32_t)(db & 0x0FU) << RTC_ALRMAR_DU_Pos);
    alrmar |= ((uint32_t)((db >> 4) & 0x03U) << RTC_ALRMAR_DT_Pos);

    alrmar &= ~RTC_ALRMAR_WDSEL;
    alrmar &= ~RTC_ALRMAR_MSK4;

    RTC->ALRMAR = alrmar;

    RTC->ISR &= ~RTC_ISR_ALRAF;
    RTC->CR  |= RTC_CR_ALRAIE;
    RTC->CR  |= RTC_CR_ALRAE;

    rtc_write_protect_enable();

    rtc_exti_enable_rising(18U);
    NVIC_EnableIRQ(RTC_Alarm_IRQn);
    return 0;
}

void rtc_timestamp_enable_rising(void)
{
    rtc_write_protect_disable();

    RTC->CR &= ~RTC_CR_TSEDGE;
    RTC->ISR &= ~(RTC_ISR_TSF | RTC_ISR_TSOVF | RTC_ISR_ITSF);
    RTC->CR |= RTC_CR_TSIE;
    RTC->CR |= RTC_CR_TSE;

    rtc_write_protect_enable();

    rtc_exti_clear(19U);
    rtc_exti_enable_rising(19U);
    NVIC_EnableIRQ(TAMP_STAMP_IRQn);
}

int rtc_timestamp_read(uint8_t *mo, uint8_t *dd, uint8_t *wd,
                       uint8_t *hh, uint8_t *min, uint8_t *ss)
{
    if (!mo || !dd || !wd || !hh || !min || !ss) return -1;
    if ((RTC->ISR & RTC_ISR_TSF) == 0U) return -2;

    uint32_t tstr = RTC->TSTR;
    uint32_t tsdr = RTC->TSDR;

    uint8_t hb = (uint8_t)((tstr & (RTC_TSTR_HT_Msk | RTC_TSTR_HU_Msk)) >> RTC_TSTR_HU_Pos);
    uint8_t mb = (uint8_t)((tstr & (RTC_TSTR_MNT_Msk | RTC_TSTR_MNU_Msk)) >> RTC_TSTR_MNU_Pos);
    uint8_t sb = (uint8_t)((tstr & (RTC_TSTR_ST_Msk | RTC_TSTR_SU_Msk)) >> RTC_TSTR_SU_Pos);

    *hh  = from_bcd(hb);
    *min = from_bcd(mb);
    *ss  = from_bcd(sb);

    uint8_t mob = (uint8_t)((tsdr & (RTC_TSDR_MT_Msk | RTC_TSDR_MU_Msk)) >> RTC_TSDR_MU_Pos);
    uint8_t db  = (uint8_t)((tsdr & (RTC_TSDR_DT_Msk | RTC_TSDR_DU_Msk)) >> RTC_TSDR_DU_Pos);

    *mo = from_bcd(mob);
    *dd = from_bcd(db);
    *wd = (uint8_t)((tsdr & RTC_TSDR_WDU_Msk) >> RTC_TSDR_WDU_Pos);

    rtc_write_protect_disable();
    RTC->ISR &= ~(RTC_ISR_TSF | RTC_ISR_TSOVF | RTC_ISR_ITSF);
    rtc_write_protect_enable();

    return 0;
}

void rtc_tamper1_enable(void)
{
    rtc_write_protect_disable();

    RTC->ISR &= ~RTC_ISR_TAMP1F;

    RTC->TAMPCR |= RTC_TAMPCR_TAMPIE;
    RTC->TAMPCR |= RTC_TAMPCR_TAMP1E;
    RTC->TAMPCR |= RTC_TAMPCR_TAMP1IE;
    RTC->TAMPCR |= RTC_TAMPCR_TAMP1NOERASE;

    rtc_write_protect_enable();

    rtc_exti_clear(19U);
    rtc_exti_enable_rising(19U);
    NVIC_EnableIRQ(TAMP_STAMP_IRQn);
}

uint8_t rtc_tamper1_get_and_clear(void)
{
    uint8_t f = (RTC->ISR & RTC_ISR_TAMP1F) ? 1U : 0U;
    if (f) {
        rtc_write_protect_disable();
        RTC->ISR &= ~RTC_ISR_TAMP1F;
        rtc_write_protect_enable();
    }
    return f;
}

void rtc_wakeup_disable(void)
{
    rtc_write_protect_disable();

    RTC->CR &= ~RTC_CR_WUTE;
    while ((RTC->ISR & RTC_ISR_WUTWF) == 0U) {}

    RTC->CR &= ~RTC_CR_WUTIE;
    RTC->ISR &= ~RTC_ISR_WUTF;

    rtc_write_protect_enable();
}

int rtc_wakeup_start_seconds(uint16_t seconds)
{
    if (seconds == 0U) return -1;

    rtc_write_protect_disable();

    RTC->CR &= ~RTC_CR_WUTE;
    while ((RTC->ISR & RTC_ISR_WUTWF) == 0U) {}

    RTC->CR &= ~RTC_CR_WUCKSEL_Msk;
    RTC->CR |=  (4U << RTC_CR_WUCKSEL_Pos);

    RTC->WUTR = (uint32_t)(seconds - 1U);

    RTC->ISR &= ~RTC_ISR_WUTF;
    RTC->CR  |= RTC_CR_WUTIE;
    RTC->CR  |= RTC_CR_WUTE;

    rtc_write_protect_enable();

    rtc_exti_enable_rising(20U);
    NVIC_EnableIRQ(RTC_WKUP_IRQn);
    return 0;
}