/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

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
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  Send a single byte via USART1 (blocking, polling TXE flag)
  * @param  data: byte to send
  * @retval None
  */
static void USART1_SendByte(uint8_t data)
{
  while (!LL_USART_IsActiveFlag_TXE(USART1));
  LL_USART_TransmitData8(USART1, data);
}

/**
  * @brief  Send a data packet via USART1
  * @param  pData: pointer to data buffer
  * @param  len: number of bytes to send
  * @retval None
  */
static void USART1_SendPacket(uint8_t *pData, uint16_t len)
{
  for (uint16_t i = 0; i < len; i++)
  {
    USART1_SendByte(pData[i]);
  }
  /* Wait for last byte transmission to complete */
  while (!LL_USART_IsActiveFlag_TC(USART1));
}

/* ---- USART2 发送函数 ---- */

/**
  * @brief  Send a single byte via USART2 (blocking, polling TXE flag)
  * @param  data: byte to send
  * @retval None
  */
static void USART2_SendByte(uint8_t data)
{
  while (!LL_USART_IsActiveFlag_TXE(USART2));
  LL_USART_TransmitData8(USART2, data);
}

/**
  * @brief  Send a data packet via USART2
  * @param  pData: pointer to data buffer
  * @param  len: number of bytes to send
  * @retval None
  */
static void USART2_SendPacket(uint8_t *pData, uint16_t len)
{
  for (uint16_t i = 0; i < len; i++)
  {
    USART2_SendByte(pData[i]);
  }
  /* Wait for last byte transmission to complete */
  while (!LL_USART_IsActiveFlag_TC(USART2));
}

/**
  * @brief  判断是否为闰年
  * @param  year: 年份 (如 2026)
  * @retval 1=闰年, 0=平年
  */
static uint8_t IsLeapYear(uint16_t year)
{
  return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

/**
  * @brief  获取某月天数
  * @param  year:  年份
  * @param  month: 月份 1-12
  * @retval 该月天数
  */
static uint8_t DaysInMonth(uint16_t year, uint8_t month)
{
  static const uint8_t days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  if (month == 2 && IsLeapYear(year)) return 29;
  return days[month - 1];
}

/**
  * @brief  解析 $GNZDA 帧, 提取 UTC 时间并换算北京时间 (UTC+8)
  * @param  frame: 接收缓冲区 (以 '\0' 结尾 或 以 '\r' 结尾的原始 NMEA 帧)
  * @param  len:   帧长度
  * @param  out:   输出解析后的时间结构体
  * @retval 0=成功, -1=格式错误
  *
  * NMEA ZDA 格式: $GNZDA,hhmmss.ss,dd,mm,yyyy,xx,xx*CS
  *   示例: $GNZDA,071614.000,11,06,2026,00,00*4D
  */
int Parse_ZDA(const uint8_t *frame, uint16_t len, ZDA_Time_t *out)
{
  uint16_t pos = 0;
  char field[16];       /* 单个字段暂存 */
  uint8_t fi = 0;       /* 字段索引 */
  uint8_t fp = 0;       /* 字段内位置 */

  /* 跳过帧头 $GNZDA,  (7个字符) */
  if (len < 7 || frame[0] != '$')
    return -1;
  pos = 7;  /* 指向 hhmmss.ss 的第一个字符 */

  /* 逐字符解析逗号分隔的字段 */
  while (pos < len && fi < 6)
  {
    char c = (char)frame[pos++];

    if (c == ',' || c == '*' || c == '\r' || c == '\n')
    {
      field[fp] = '\0';                 /* 字段结束 */

      switch (fi)
      {
        case 0: /* UTC时间 hhmmss.ss — 取前6位整数 */
          if (fp >= 6)
          {
            out->hour   = (uint8_t)((field[0]-'0')*10 + (field[1]-'0'));
            out->minute = (uint8_t)((field[2]-'0')*10 + (field[3]-'0'));
            out->second = (uint8_t)((field[4]-'0')*10 + (field[5]-'0'));
          }
          else return -1;
          break;

        case 1: /* 日 dd */
          out->day = (uint8_t)((fp >= 2) ? ((field[0]-'0')*10 + (field[1]-'0')) : (field[0]-'0'));
          break;

        case 2: /* 月 mm */
          out->month = (uint8_t)((fp >= 2) ? ((field[0]-'0')*10 + (field[1]-'0')) : (field[0]-'0'));
          break;

        case 3: /* 年 yyyy */
          if (fp >= 4)
            out->year = (uint16_t)((field[0]-'0')*1000 + (field[1]-'0')*100
                                 + (field[2]-'0')*10   + (field[3]-'0'));
          else return -1;
          break;

        case 4: /* 本地区域小时 (未使用) */
        case 5: /* 本地区域分钟 (未使用) */
          break;
      }

      fi++;
      fp = 0;

      if (c == '*') break;   /* 遇到校验和, 字段结束 */
    }
    else
    {
      if (fp < sizeof(field) - 1)
        field[fp++] = c;
    }
  }

  /* 校验基本范围 */
  if (out->month < 1 || out->month > 12)  return -1;
  if (out->day   < 1 || out->day   > 31)  return -1;
  if (out->hour  > 23 || out->minute > 59 || out->second > 59) return -1;

  /* ---- UTC → 北京时间 (UTC+8) ---- */
  {
    uint8_t  bj_h = out->hour + 8;
    uint8_t  bj_d = out->day;
    uint8_t  bj_m = out->month;
    uint16_t bj_y = out->year;
    uint8_t  carry_day = (bj_h >= 24) ? 1 : 0;

    bj_h %= 24;

    if (carry_day)
    {
      bj_d++;
      if (bj_d > DaysInMonth(bj_y, bj_m))
      {
        bj_d = 1;
        bj_m++;
        if (bj_m > 12)
        {
          bj_m = 1;
          bj_y++;
        }
      }
    }

    out->bj_hour  = bj_h;
    out->bj_day   = bj_d;
    out->bj_month = bj_m;
    out->bj_year  = bj_y;
  }

  return 0;
}

/* USER CODE END 0 */

uint32_t timestamp = 0;  /* 北京时间压缩后的时间戳 (大端序) */
uint32_t alarm    = 0;   /* 报警状态: 0=正常, 1=报警 */
uint32_t distance = 0;   /* 漏水距离, 单位cm */

/* USART2 发送缓冲区 (USART3 接收解析后 → USART2 发出) */
uint8_t  usart2_tx_buf[256];
uint16_t usart2_tx_len;

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* ---- USART3 → USART2: 收一次, 解析一次, 发一次 ---- */
    if (usart3_rx_done)
    {
      /* 解析 16 字节二进制协议:
         Byte 0-3:  帧头 0x55 0xAA 0x55 0xAA (ISR 中已验证)
         Byte 4-7:  时间戳 (uint32_t, 大端序)
         Byte 8-11: 报警信息 (uint32_t, 大端序)
         Byte 12-15: 漏水距离 (uint32_t, 大端序, 单位cm)
      */
      uint32_t rx_timestamp = ((uint32_t)usart3_rx_buf[4]  << 24)
                            | ((uint32_t)usart3_rx_buf[5]  << 16)
                            | ((uint32_t)usart3_rx_buf[6]  << 8)
                            | ((uint32_t)usart3_rx_buf[7]);
      uint32_t rx_alarm     = ((uint32_t)usart3_rx_buf[8]  << 24)
                            | ((uint32_t)usart3_rx_buf[9]  << 16)
                            | ((uint32_t)usart3_rx_buf[10] << 8)
                            | ((uint32_t)usart3_rx_buf[11]);
      uint32_t rx_distance  = ((uint32_t)usart3_rx_buf[12] << 24)
                            | ((uint32_t)usart3_rx_buf[13] << 16)
                            | ((uint32_t)usart3_rx_buf[14] << 8)
                            | ((uint32_t)usart3_rx_buf[15]);

      if(rx_alarm == 0)
      {
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_6|LL_GPIO_PIN_7);
        LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_4);
      }
      else
      {
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_6|LL_GPIO_PIN_7);
        LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_4);
      }

      /* 将收到的 16 字节数据复制到发送缓冲区 */
      memcpy(usart2_tx_buf, usart3_rx_buf, 16);

      /* 通过 USART2 发出 */
      USART2_SendPacket(usart2_tx_buf, 16);

      /* 清除标志, 准备接收下一帧 */
      usart3_rx_done = 0;
      usart3_rx_len  = 0;
    }
  }
  /* USER CODE END WHILE */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_0)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {

  }
  LL_Init1msTick(16000000);
  LL_SetSystemCoreClock(16000000);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
