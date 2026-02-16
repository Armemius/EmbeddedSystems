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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "kb.h"
#include "pca9538.h"
#include "oled.h"
#include "music.h"
#include <stdio.h>
#include <inttypes.h>
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
char *i64toa(int64_t v, char *buf)
{
    char *p = buf;
    uint64_t x;

    if (v < 0) {
        *p++ = '-';
        x = -v;
    } else {
        x = v;
    }

    char *start = p;
    do {
        *p++ = (x % 10) + '0';
        x /= 10;
    } while (x);

    *p = '\0';

    for (char *l = start, *r = p - 1; l < r; l++, r--) {
        char t = *l; *l = *r; *r = t;
    }

    return buf;
}

/* USER CODE END 0 */

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
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  	init_music();
    oled_Init();

    oled_Fill(1);
    oled_SetCursor(10, 5);
    oled_WriteString("Calculator", Font_7x10, 0);
    oled_SetCursor(10, 25);
    oled_WriteString("SDK-1.1", Font_7x10, 0);
    oled_SetCursor(10, 45);
	oled_WriteString("By Armemius", Font_7x10, 0);
   	oled_UpdateScreen();
   	play_amogus();
   	oled_Fill(0);
   	oled_UpdateScreen();

    kb_init();
    uint8_t state = 0;
    uint8_t opcode = 1;
    char buf_a[20];
    char buf_b[20];
    char buf_c[32];
    char opcodes[4] = {'+', '-', '*', '/'};
    uint8_t a_size = 0;
    uint8_t b_size = 0;
    int32_t a = 0;
    int32_t b = 0;
    int32_t c = 0;
    char old = 0, ch = 0;
//  	play_amogus();
//  	handlers[0] = show_code;
    oled_SetCursor(5, 5);
	oled_Fill(0);
	oled_WriteChar('0', Font_7x10, 1);
	oled_UpdateScreen();
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	old = ch;
	ch = kb_get_symb();
	if (ch == old || ch == 0) {
		continue;
	}

	play_note(800);
	HAL_Delay(150);

	if (state == 1 && ch == '*') {
		opcode = (opcode + 1) % 4;
	}
	if (state == 1 && '0' <= ch && ch <= '9') {
		state = 2;
	}
	if (state == 0 && a_size < 8) {
		if ('0' <= ch && ch <= '9') {
			uint32_t a_old = a;
			a = a * 10 + ch - '0';
			if (a != a_old) {
				a_size++;
			}
			snprintf(buf_a, sizeof(buf_a), "%ld", a);
		}
	}
	if (state == 0 && ch == '*') {
		state = 1;
	}
	if (state == 3 && ch == '#') {
		state = 0;
		a = 0;
		b = 0;
		c = 0;
		a_size = 0;
		b_size = 0;
		snprintf(buf_a, sizeof(buf_b), "%ld", 0);
		snprintf(buf_b, sizeof(buf_b), "%ld", 0);
		snprintf(buf_c, sizeof(buf_c), "%ld", 0);
	}
	if (state == 2 && ch == '#') {
		state = 3;
	}

	if (state == 2 && b_size < 8) {
		if ('0' <= ch && ch <= '9') {
			uint32_t b_old = b;
			b = b * 10 + ch - '0';
			if (b != b_old) {
				b_size++;
			}
			snprintf(buf_b, sizeof(buf_b), "%ld", b);
		}
	}
	oled_SetCursor(5, 5);
	oled_Fill(0);
	oled_WriteString(buf_a, Font_7x10, 1);

	if (state > 0) {
		oled_WriteChar(opcodes[opcode], Font_7x10, 1);
	}

	if (state > 1) {
		oled_SetCursor(5, 15);
		oled_WriteString(buf_b, Font_7x10, 1);
	}
	if (state == 3) {
		int64_t c;
		if (b == 0 && opcode == 3) {
			snprintf(buf_c, sizeof(buf_c), "error");
		} else {
			switch(opcode) {
			case 0:
				c = (uint64_t)a + (uint64_t)b;
				break;
			case 1:
				c = (uint64_t)a - (uint64_t)b;
				break;
			case 2:
				c = (uint64_t)a * (uint64_t)b;
				break;
			case 3:
				c = (uint64_t)a / (uint64_t)b;
				break;
			}
			i64toa(c, buf_c);
		}
	}
	if (state > 2) {
		oled_WriteChar('=', Font_7x10, 1);
		oled_SetCursor(5, 25);
		oled_WriteString(buf_c, Font_7x10, 1);
	}

	oled_UpdateScreen();
	stop_note();
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
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
