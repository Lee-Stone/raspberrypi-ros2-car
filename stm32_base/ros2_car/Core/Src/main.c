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
#include "OLED.h"
#include "MPU6050.h"
#include "Encoder.h"
#include "TB6612.h"
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
#include <stdio.h>

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&huart2 , (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

float cmd_vel(char *str, char *data)
{
	char *loc,*token;
	float a;

	loc = strstr(data, str);
	token = strtok(loc, str);
	a = atof(token);

	return a;
}

float p_calc(float kp, float target, float current)
{
	float err = target - current;

	return kp * err;
}

float pi_calc(float kp, float ki, float *err_sum, float ki_max, float target, float current)
{
	float err = target - current;

	*err_sum += err;

	if (*err_sum > ki_max) *err_sum = ki_max;
	if (*err_sum < -ki_max) *err_sum = -ki_max;

	return kp * err + ki * *err_sum;
}

float gyroz = 0.0;
float left_speed = 0.0, right_speed = 0.0;
float target_speed = 0.0, target_angle = 0.0;
float left_speed_out = 0.0, right_speed_out = 0.0, angle_out = 0.0;
float left_speed_err_sum = 0.0, right_speed_err_sum = 0.0;

uint8_t tick_motor = 0, tick_encoder = 0, tick_gyroz = 0;
uint8_t rx_flag = 0, rx_byte = 0, rx_data[64];
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
  MX_I2C2_Init();
  MX_TIM3_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  OLED_ShowString(1, 1, "Init...");

  while(MPU6050_DMP_Init());
  Encoder_Init();
  TB6612_Init();

  OLED_Clear();
  OLED_ShowString(1, 1, "Gyroz:");
  OLED_ShowString(2, 1, "Lspd:");
  OLED_ShowString(3, 1, "Rspd:");

  HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
  HAL_TIM_Base_Start_IT(&htim4);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	gyroz = MPU6050_DMP_GetROSGyroz();
	Encoder_Read(&left_speed, &right_speed);

	OLED_ShowSignedFloat(1, 7, gyroz, 4);
	OLED_ShowSignedFloat(2, 7, left_speed, 4);
	OLED_ShowSignedFloat(3, 7, -right_speed, 4);

	printf("(%.2f,%.2f,%.2f)\r\n", gyroz, left_speed, -right_speed);

	if(rx_flag == 1)
	{
		// [修复]：使用临时缓冲区，防止解析过程中rx_data被中断修改
		char temp_rx_data[64];

		// 1. 暂时关闭串口接收中断（或只要保证拷贝过程原子性）
		// 为了简单安全，我们直接拷贝。因为中断是单字节接收，拷贝速度远快于波特率，通常也是安全的。
		// 但最严谨的做法是先拷贝再清除标志位
		strcpy(temp_rx_data, (char*)rx_data);

		// 2. 清除标志位，允许下一帧数据开始准备
		rx_flag = 0;

		// 3. 解析拷贝出来的数据
		float temp_speed = cmd_vel("x=", temp_rx_data);
		float temp_angle = cmd_vel("z=", temp_rx_data);

		// 4. 简单的滤波或限幅，防止解析出非法值（可选但推荐）
		if(temp_speed > -10.0 && temp_speed < 10.0) target_speed = temp_speed;
		if(temp_angle > -2 && temp_angle < 2) target_angle = temp_angle;
	}

	HAL_Delay(10);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart == &huart2)
  {
	// (x=1.24,z=2.54)
	// 接收并解析格式为 "(data)" 的数据
	static uint8_t rx_state = 0;
	static uint8_t rx_cnt = 0;

	if(rx_state == 0)
	{
	  if(rx_byte == '(' && rx_flag == 0)
	  {
		rx_state = 1;
		rx_cnt = 0;
		memset(rx_data, 0, sizeof(rx_data));
	  }
	}
	else if(rx_state == 1)
	{
	  if(rx_byte == ')')
	  {
		rx_state = 0;
		rx_data[rx_cnt] = '\0';
		rx_flag = 1;
	  }
	  else
	  {
		rx_data[rx_cnt] = rx_byte;
		rx_cnt++;
	  }
	}

	HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim4)
    {
        if(tick_motor++ >= 2)
        {
            TB6612_Motor(left_speed_out + angle_out, right_speed_out - angle_out);
            tick_motor = 0;
        }
        if(tick_gyroz++ >= 2)
        {
            angle_out = p_calc(-1500.0, target_angle, gyroz);
            tick_gyroz = 0;
        }
        if(tick_encoder++ >= 10)
        {
            left_speed_out = pi_calc(1000.0, 200.0, &left_speed_err_sum, 10000.0, target_speed, (left_speed - right_speed) / 2);
            right_speed_out = pi_calc(1000.0, 200.0, &right_speed_err_sum, 10000.0, target_speed, (left_speed - right_speed) / 2);
            tick_encoder = 0;
        }
    }
}
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
