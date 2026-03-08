#include "spi_dma.h"
#include "stm32l4xx.h"

#define SPI1_CS_PORT   GPIOA
#define SPI1_CS_PIN    15U

void spi1_cs_high(void)
{
    SPI1_CS_PORT->BSRR = (1U << SPI1_CS_PIN);
}

void spi1_cs_low(void)
{
    SPI1_CS_PORT->BSRR = (1U << (SPI1_CS_PIN + 16U));
}

void spi1_init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;
    (void)RCC->AHB2ENR;

    GPIOA->MODER &= ~(3U << (SPI1_CS_PIN * 2U));
    GPIOA->MODER |=  (1U << (SPI1_CS_PIN * 2U));
    GPIOA->OTYPER &= ~(1U << SPI1_CS_PIN);
    GPIOA->OSPEEDR |= (3U << (SPI1_CS_PIN * 2U));
    GPIOA->PUPDR &= ~(3U << (SPI1_CS_PIN * 2U));
    spi1_cs_high();

    GPIOB->MODER &= ~((3U << (3U * 2U)) | (3U << (4U * 2U)) | (3U << (5U * 2U)));
    GPIOB->MODER |=  ((2U << (3U * 2U)) | (2U << (4U * 2U)) | (2U << (5U * 2U)));

    GPIOB->OTYPER &= ~((1U << 3U) | (1U << 4U) | (1U << 5U));
    GPIOB->OSPEEDR |= ((3U << (3U * 2U)) | (3U << (4U * 2U)) | (3U << (5U * 2U)));
    GPIOB->PUPDR &= ~((3U << (3U * 2U)) | (3U << (4U * 2U)) | (3U << (5U * 2U)));

    GPIOB->AFR[0] &= ~((0xFU << (3U * 4U)) | (0xFU << (4U * 4U)) | (0xFU << (5U * 4U)));
    GPIOB->AFR[0] |=  ((5U   << (3U * 4U)) | (5U   << (4U * 4U)) | (5U   << (5U * 4U)));

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    (void)RCC->APB2ENR;

    SPI1->CR1 = 0;
    SPI1->CR2 = 0;

    SPI1->CR2 |= (7U << SPI_CR2_DS_Pos);
    SPI1->CR2 |= SPI_CR2_FRXTH;

    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR1 |= (3U << SPI_CR1_BR_Pos);
    SPI1->CR1 |= SPI_CR1_CPOL | SPI_CR1_CPHA;
    SPI1->CR1 &= ~SPI_CR1_LSBFIRST;
    SPI1->CR1 &= ~SPI_CR1_RXONLY;

    SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;

    SPI1->CR2 |= SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN;

    SPI1->CR1 |= SPI_CR1_SPE;
}