/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

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

  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* RXNE: 接收数据寄存器非空 */
  if (LL_USART_IsActiveFlag_RXNE(USART1))
  {
    /* 缓冲区溢出保护 */
    if (usart1_rx_len < sizeof(usart1_rx_buf))
    {
      usart1_rx_buf[usart1_rx_len++] = LL_USART_ReceiveData8(USART1);
    }
    else
    {
      /* 溢出: 读取DR避免RXNE持续触发, 丢弃旧数据从头接收 */
      (void)LL_USART_ReceiveData8(USART1);
      usart1_rx_len = 0;
    }
  }

  /* IDLE: 总线空闲 — 一帧接收完成 */
  if (LL_USART_IsActiveFlag_IDLE(USART1))
  {
    LL_USART_ClearFlag_IDLE(USART1);   /* 读SR→DR 清除IDLE标志 */

    /* 只有实际收到数据时才通知主循环 (过滤上电时的虚假IDLE) */
    if (usart1_rx_len > 0)
    {
      usart1_rx_done = 1;
    }
  }

  /* ORE: 溢出错误 — 必须清除, 否则后续数据无法接收 */
  if (LL_USART_IsActiveFlag_ORE(USART1))
  {
    LL_USART_ClearFlag_ORE(USART1);
  }

  /* USER CODE END USART1_IRQn 0 */
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */

  /* RXNE: 接收数据寄存器非空 */
  if (LL_USART_IsActiveFlag_RXNE(USART3))
  {
    uint8_t byte = LL_USART_ReceiveData8(USART3);

    /* 缓冲区溢出保护 */
    if (usart3_rx_len < sizeof(usart3_rx_buf))
    {
      usart3_rx_buf[usart3_rx_len++] = byte;
    }
    else
    {
      /* 溢出: 丢弃旧数据, 从头接收 */
      usart3_rx_len = 0;
    }
  }

  /* IDLE: 总线空闲 — 一帧接收完成 */
  if (LL_USART_IsActiveFlag_IDLE(USART3))
  {
    LL_USART_ClearFlag_IDLE(USART3);   /* 读SR→DR 清除IDLE标志 */

    /* 检查 16 字节二进制帧: 帧头 0x55 0xAA 0x55 0xAA */
    if (usart3_rx_len >= 16
        && usart3_rx_buf[0] == 0x55
        && usart3_rx_buf[1] == 0xAA
        && usart3_rx_buf[2] == 0x55
        && usart3_rx_buf[3] == 0xAA)
    {
      usart3_rx_done = 1;   /* 有效帧: 通知主循环处理 */
    }
    else
    {
      usart3_rx_len = 0;    /* 帧头不匹配或长度不足: 丢弃整帧 */
    }
  }

  /* ORE: 溢出错误 — 必须清除, 否则后续数据无法接收 */
  if (LL_USART_IsActiveFlag_ORE(USART3))
  {
    LL_USART_ClearFlag_ORE(USART3);
  }

  /* USER CODE END USART3_IRQn 0 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
