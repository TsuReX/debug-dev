/**
  ******************************************************************************
  * @file    CRC/CRC_Bytes_Stream_7bit_CRC/Src/main.c
  * @author  MCD Application Team
  * @brief   This sample code shows how to use the STM32F3xx CRC HAL API
  *          to compute 7-bit CRC codes from buffers of data bytes (8-bit data),
  *          based on a user-defined generating polynomial.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32F3xx_HAL_Examples
  * @{
  */

/** @addtogroup CRC_Bytes_Stream_7bit_CRC
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BUFFER_SIZE_5  5  /* CRC7_DATA8_TEST5[] is 5-byte long   */ 
#define BUFFER_SIZE_17 17 /* CRC7_DATA8_TEST17[] is 17-byte long */ 
#define BUFFER_SIZE_1  1  /* CRC7_DATA8_TEST1[] is 1-byte long   */ 
#define BUFFER_SIZE_2  2  /* CRC7_DATA8_TEST2[] is 2-byte long   */ 

/* User-defined polynomial */
#define CRC_POLYNOMIAL_7B  0x65  /* X^7 + X^6 + X^5 + X^2 + 1,
                                   used in Train Communication Network, IEC 60870-5[17] */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* CRC handler declaration */
CRC_HandleTypeDef   CrcHandle;

/* Used for storing CRC Value */
__IO uint32_t uwCRCValue = 0;

/* Bytes buffers that will consecutively yield CRCs */
static const uint8_t CRC7_DATA8_TEST5[5]   = {0x12,0x34,0xBA,0x71,0xAD};
static const uint8_t CRC7_DATA8_TEST17[17] = {0x12,0x34,0xBA,0x71,0xAD,
                                              0x11,0x56,0xDC,0x88,0x1B,
                                              0xEE,0x4D,0x82, 0x93,0xA6,
                                              0x7F,0xC3};
static const uint8_t CRC7_DATA8_TEST1[1]   = {0x19};                                                
static const uint8_t CRC7_DATA8_TEST2[2]   = {0xAB,0xCD};

       

/* Expected CRC Values */
/* The 7 LSB bits are the 7-bit long CRC */
uint32_t uwExpectedCRCValue_1 = 0x00000057;    /* First byte stream CRC  */
uint32_t uwExpectedCRCValue_2 = 0x0000006E;    /* Second byte stream CRC */
uint32_t uwExpectedCRCValue_3 = 0x0000004B;    /* Third byte stream CRC  */
uint32_t uwExpectedCRCValue_4 = 0x00000026;    /* Fourth byte stream CRC */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{

  /* STM32F3xx HAL library initialization:
       - Configure the Flash prefetch
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  HAL_Init();
  
  /* Configure the system clock to 64 MHz */
  SystemClock_Config();

  /* Configure LED3 */
  BSP_LED_Init(LED3);


  /****************************************************************************/
  /*                                                                          */
  /*                     CRC peripheral initialization                        */
  /*                                                                          */    
  /****************************************************************************/
    
  CrcHandle.Instance = CRC;

  /* The default polynomial is not used. The one to be used must be defined 
     in CrcHandle.Init.GeneratingPolynomial */  
  CrcHandle.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_DISABLE;
  
  /* Set the value of the generating polynomial. 
    The one used in that example is the 7-bit long CRC generating
    polynomial X^7 + X^6 + X^5 + X^2 + 1 */
  CrcHandle.Init.GeneratingPolynomial    = CRC_POLYNOMIAL_7B;
  
  /* The user-defined generating polynomial yields a 7-bit long CRC */
  CrcHandle.Init.CRCLength               = CRC_POLYLENGTH_7B;

  /* The default init value is used */
  CrcHandle.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_ENABLE;

  /* The input data are not inverted */
  CrcHandle.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_NONE;

  /* The output data are not inverted */
  CrcHandle.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;

  /* The input data are bytes (8-bit long data) */
  CrcHandle.InputDataFormat              = CRC_INPUTDATA_FORMAT_BYTES;

  /* De-initialize the CRC peripheral */
  if (HAL_CRC_DeInit(&CrcHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }  

  /* Then, initialize the CRC handle */
  if (HAL_CRC_Init(&CrcHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }


  /****************************************************************************/
  /*                                                                          */
  /*         CRC computation of a first bytes stream                          */
  /*                                                                          */    
  /****************************************************************************/

  /* The 7-bit long CRC of a 5-byte buffer is computed. After IP initialization, 
     the CRC calculator is initialized with the default value that is 0x7F for 
     a 7-bit CRC.
    
    The computed CRC is stored in uint32_t uwCRCValue. The 7-bit long CRC is made of
    uwCRCValue 7 LSB bits. */

  uwCRCValue = HAL_CRC_Accumulate(&CrcHandle, (uint32_t *)&CRC7_DATA8_TEST5, BUFFER_SIZE_5);

  /* Compare the CRC value to the expected one */
  if (uwCRCValue != uwExpectedCRCValue_1)
  {
    /* Wrong CRC value: enter Error_Handler */
    Error_Handler();
  }

  
  /****************************************************************************/
  /*                                                                          */
  /*         CRC computation of a second bytes stream                         */
  /*                                                                          */    
  /****************************************************************************/

  /* The 7-bit long CRC of a 17-byte buffer is computed. The CRC calculator
    is not re-initialized, instead the previously computed CRC is used
    as initial value. */

  uwCRCValue = HAL_CRC_Accumulate(&CrcHandle, (uint32_t *)&CRC7_DATA8_TEST17, BUFFER_SIZE_17);

  /* Compare the CRC value to the expected one */
  if (uwCRCValue != uwExpectedCRCValue_2)
  {
    /* Wrong CRC value: enter Error_Handler */
    Error_Handler();
  }


  /****************************************************************************/
  /*                                                                          */
  /*         CRC computation of a single byte                                 */
  /*                                                                          */    
  /****************************************************************************/

  /* The 7-bit long CRC of a 1-byte buffer is computed. The CRC calculator
    is not re-initialized, instead the previously computed CRC is used
    as initial value. */

  uwCRCValue = HAL_CRC_Accumulate(&CrcHandle, (uint32_t *)&CRC7_DATA8_TEST1, BUFFER_SIZE_1);

  /* Compare the CRC value to the expected one */
  if (uwCRCValue != uwExpectedCRCValue_3)
  {
    /* Wrong CRC value: enter Error_Handler */
    Error_Handler();
  }


  /****************************************************************************/
  /*                                                                          */
  /*         CRC computation of the last bytes stream                         */
  /*                                                                          */    
  /****************************************************************************/

  /* The 7-bit long CRC of a 2-byte buffer is computed. The CRC calculator
    is re-initialized with the default value that is 0x7F for a 7-bit CRC.
    This is done with a call to HAL_CRC_Calculate() instead of 
    HAL_CRC_Accumulate(). */

  uwCRCValue = HAL_CRC_Calculate(&CrcHandle, (uint32_t *)&CRC7_DATA8_TEST2, BUFFER_SIZE_2);

  /* Compare the CRC value to the expected one */
  if (uwCRCValue != uwExpectedCRCValue_4)
  {
    /* Wrong CRC value: enter Error_Handler */
    Error_Handler();
  }
  else
  {
    /* Right CRC value: Turn LED3 on */
    BSP_LED_On(LED3);
  }  


  /* Infinite loop */
  while (1)
  {
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 64000000
  *            HCLK(Hz)                       = 64000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            PLLMUL                         = RCC_PLL_MUL16 (16)
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* HSI Oscillator already ON after system reset, activate PLL with HSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  while (1)
  {
    /* Error if LED3 is slowly blinking (1 sec. period) */
    BSP_LED_Toggle(LED3); 
    HAL_Delay(1000);   
  }

}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
