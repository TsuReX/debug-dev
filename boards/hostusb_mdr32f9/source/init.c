/*
 * @file	src/config.c
 *
 * @brief	Определения функций инициализации SoC и периферии.
 *
 * @author	Vasily Yurchenko <vasily.v.yurchenko@yandex.ru>
 */

#include "init.h"
#include "systimer.h"
#include "drivers.h"
#include "config.h"

/**
 * @brief	Настраивает внутреннюю флеш память для корректного взаимодействия с ядром,
 * 			работающем на частоте 48 МГц.
 */
static void flash_init(void)
{
	/* Настройка времени задержки доступа к флешке.
	 * Это необходимо для корректной работы флешки при различных частотах HCLK.
	 */
	EEPROM_SetLatency(2);
}

/**
 * @brief TODO
 */
static void sysclk_init_64mhz()
{
#if defined(EXT_CRYSTAL_8MHZ_CPU_64MHZ)

	RST_CLK_HSEconfig(RST_CLK_HSE_ON);

	while (RST_CLK_HSEstatus() != SUCCESS);

	/* Good HSE clock */
	/* Select HSE clock as CPU_PLL input clock source */

	/* Set PLL multiplier to 8 */
	RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul8);

	/* Enable CPU_PLL */
	RST_CLK_CPU_PLLcmd(ENABLE);

	while (RST_CLK_CPU_PLLstatus() != SUCCESS);

	/* Good CPU PLL */
	/* Set CPU_C3_prescaler to 1 */
	RST_CLK_CPUclkPrescaler(RST_CLK_CPUclkDIV1);

	/* Set CPU_C2_SEL to CPU_PLL output instead of CPU_C1 clock */
	RST_CLK_CPU_PLLuse(ENABLE);

	/* Select CPU_C3 clock on the CPU clock MUX */
	RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3);

#elif defined(INT_CRYSTAL_8MHZ_CPU_64MHZ)

	/* Select HSI/2 clock as CPU_PLL input clock source */
	RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSIdiv1, RST_CLK_CPU_PLLmul8);

	/* Enable CPU_PLL */
	RST_CLK_CPU_PLLcmd(ENABLE);

	while (RST_CLK_CPU_PLLstatus() != SUCCESS);

	/* Set CPU_C3_prescaler to 1 */
	RST_CLK_CPUclkPrescaler(RST_CLK_CPUclkDIV1);

	/* Switch CPU_C2_SEL to CPU_C1 clock instead of CPU_PLL output */
	RST_CLK_CPU_PLLuse(ENABLE);

	/* Select CPU_C3 clock on the CPU clock MUX */
	RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3);

#else

#error "Crystal type must be defined"

#endif

}

/**
 * @brief	Производит настройку источников тактирования:
 * 				SYSCLK, AHBCLK, APB1CLK, APB2CLK, RTCCLK, MCO;
 *
 *			The system Clock is configured as follow :
 *				System Clock source		= PLL (HSI)
 *				SYSCLK (MHz)			= 64
 *				AHBCLK/HCLK (MHz)		= 64
 *				APB1CLK	(MHz)			= 32
 *				APB2CLK	(MHz)			= 64
 *				ADCCLK (MHz)			= 8
 *				RTCCLK (KHz)			= 40
 */
static void rcc_init(void)
{
	sysclk_init_64mhz();
}
#if 0
/**
 * @brief	Наcтраивает системный таймер ядра (SysTick)
 *
 * @param	hclk_freq	частота шины HCLK в герцах
 */
static void systick_init(uint32_t hclk_freq)
{
	/* Производится настройка системного таймера ядра для определения интервала времени равного 100 микросекундам.  */

	SysTick->LOAD  = (uint32_t)((hclk_freq / 10000) - 1UL);  /* set reload register */
	SysTick->VAL   = 0UL;                                       /* Load the SysTick Counter Value */
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
				   SysTick_CTRL_ENABLE_Msk;                   /* Enable the Systick Timer */
}
#endif
/**
 * @brief	Настройка устройств платформы(платы)
 */
void board_init(void)
{
	PORT_InitTypeDef port_descriptor;
	PORT_StructInit(&port_descriptor);

	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTA, ENABLE);
	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTB, ENABLE);
//	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTC, ENABLE);
	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTD, ENABLE);
	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTE, ENABLE);
//	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTF, ENABLE);

	port_descriptor.PORT_OE    = PORT_OE_OUT;
	port_descriptor.PORT_FUNC  = PORT_FUNC_PORT;
	port_descriptor.PORT_MODE  = PORT_MODE_DIGITAL;
	port_descriptor.PORT_SPEED = PORT_SPEED_SLOW;

	/* PA7 PS_ON */
	port_descriptor.PORT_Pin   = (PORT_Pin_7);
	PORT_Init(MDR_PORTA, &port_descriptor);
	PORT_SetBits(MDR_PORTA, PORT_Pin_7);

	/* PD7 DEBUG_LED */
	PORT_Init(MDR_PORTD, &port_descriptor);

	/* PE0 KM.KB_CS */
	port_descriptor.PORT_Pin   = (PORT_Pin_0);
	PORT_Init(MDR_PORTE, &port_descriptor);
	PORT_SetBits(MDR_PORTE, PORT_Pin_0);

	/* PB8 KM.MS_CS */
	port_descriptor.PORT_Pin   = (PORT_Pin_8);
	PORT_Init(MDR_PORTB, &port_descriptor);
	PORT_SetBits(MDR_PORTB, PORT_Pin_8);

#if 0
	/* PB6 DEBUG */
	port_descriptor.PORT_Pin   = (PORT_Pin_6);
	PORT_Init(MDR_PORTB, &port_descriptor);
	PORT_SetBits(MDR_PORTB, PORT_Pin_6);
#endif

#if 0
	/* PD0 DEBUG */
	port_descriptor.PORT_Pin   = (PORT_Pin_0);
	PORT_Init(MDR_PORTD, &port_descriptor);
	PORT_SetBits(MDR_PORTD, PORT_Pin_0);
#endif

#if 0
	/* PF0,1,3 DEBUG */
	port_descriptor.PORT_Pin   = (PORT_Pin_0 | PORT_Pin_1 | PORT_Pin_3);
	PORT_Init(MDR_PORTF, &port_descriptor);
#endif

#if 0
	/* PC0,1 DEBUG */
	port_descriptor.PORT_Pin   = (PORT_Pin_0 | PORT_Pin_1);
	PORT_Init(MDR_PORTC, &port_descriptor);
#endif

}

/**
 * @brief	Настройка внутренних подсистем системы на кристалле.
 */
void soc_init(void)
{
	/* Настройка внутренней флеш памяти. */
	flash_init();
	/* Настройка подсистемы тактирования. */
	rcc_init();
	/* Настраивает системный таймер ядра. */
	systimer_init(HCLK_FREQ, 10);
}

