/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);


/* Function processing DMA Rx data. Counts how many capital and small letters are in sentence.
 * Result is supposed to be stored in global variable of type "letter_count_" that is defined in "main.h"
 *
 * @param1 - received sign
 */
void proccesDmaData(uint8_t sign);


/* Space for your global variables. */

	// type your global variables here:

char receivedText[36];
uint8_t textLength = 0;
uint8_t receiving = 0;
volatile uint8_t validTextReady = 0;
letter_count_ letterCount = {0, 0};
volatile uint32_t msTicks = 0;


int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();

  /* Space for your local variables, callback registration ...*/

  	  //type your code here:
  USART2_RegisterCallback(proccesDmaData);
  static char txBuffer[160];
  uint32_t lastReport = 0;

  while (1)
  {
	  /* Periodic transmission of information about DMA Rx buffer state.
	   * Transmission frequency - 0.5Hz.
	   * Message format - "Buffer capacity: %d bytes, occupied memory: %d bytes, load [in %]: %f%"
	   * Example message (what I wish to see in terminal) - Buffer capacity: 1000 bytes, occupied memory: 231 bytes, load [in %]: 23.1%
	   */

	  /* Valid text string information transmission.
	   * Transmission frequency - when new valid string is received.
	   * Message format - "Valid string: %s, lower-case: %d, upper-case: %d"
	   * Example message (what I wish to see in terminal) - Valid string: Platn15uborZnakov, lower-case: 13, upper-case: 2
	   */

  	  	  	  //type your code here:

	  if (!LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_7))
	  {
	      if (validTextReady)
	      {
	          int len = snprintf(txBuffer, sizeof(txBuffer),
	                  "Valid string: %s, lower-case: %d, upper-case: %d\r\n",
	                  receivedText, letterCount.small_letter,
	                  letterCount.capital_letter);

	          validTextReady = 0;
	          USART2_PutBuffer((uint8_t *)txBuffer, len);
	      }
	      else if (msTicks - lastReport >= 10000)
	      {
	          lastReport = msTicks;

	          uint16_t occupied = DMA_USART2_BUFFER_SIZE -
	                  LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_6);

	          uint32_t load10 = occupied * 1000 / DMA_USART2_BUFFER_SIZE;

	          int len = snprintf(txBuffer, sizeof(txBuffer),
	                  "Buffer capacity: %d bytes, occupied memory: %d bytes, load [in %%]: %lu.%lu%%\r\n",
	                  DMA_USART2_BUFFER_SIZE, occupied,
	                  (unsigned long)(load10 / 10),
	                  (unsigned long)(load10 % 10));

	          USART2_PutBuffer((uint8_t *)txBuffer, len);
	      }
	  }

  }
  /* USER CODE END 3 */
}


void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
  {
  Error_Handler();  
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {
    
  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
  
  }
  LL_Init1msTick(8000000);
  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
  LL_SetSystemCoreClock(8000000);
  LL_SYSTICK_EnableIT();
}

/*
 * Implementation of function processing data received via USART.
 */
void proccesDmaData(uint8_t sign)
{
	/* Process received data */

		// type your algorithm here:
	if (validTextReady)
	    return;

	if (sign == '#')
	{
	    receiving = 1;
	    textLength = 0;
	    return;
	}

	if (!receiving)
	    return;

	if (sign == '$')
	{
	    receivedText[textLength] = '\0';

	    letterCount.small_letter = 0;
	    letterCount.capital_letter = 0;

	    for (uint8_t i = 0; i < textLength; i++)
	    {
	        if (receivedText[i] >= 'a' && receivedText[i] <= 'z')
	            letterCount.small_letter++;
	        else if (receivedText[i] >= 'A' && receivedText[i] <= 'Z')
	            letterCount.capital_letter++;
	    }

	    validTextReady = 1;
	    receiving = 0;
	    return;
	}

	if (textLength < 35)
	{
	    receivedText[textLength++] = sign;

	    if (textLength == 35)
	    {
	        receiving = 0;
	        textLength = 0;
	    }
	}

	else
	{
	    receiving = 0;
	    textLength = 0;
	}

}


void Error_Handler(void)
{

}

#ifdef  USE_FULL_ASSERT

void assert_failed(char *file, uint32_t line)
{ 

}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
