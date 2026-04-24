#ifndef __CONFIG_H
#define __CONFIG_H

#include "main.h"
#include <stdint.h>

#define DEVICE_OLED     1
#if DEVICE_OLED 
    #include "gpio.h"
    #define OLED_W_SCL(x)		HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, (x))
    #define OLED_W_SDA(x)		HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, (x))
#endif

#define DEVICE_MPU6050  1
#if DEVICE_MPU6050
    #include "i2c.h"
    #define MPU6050_I2C         hi2c2
#endif

#define DEVICE_ENCODER  1
#if DEVICE_ENCODER
    #include "tim.h"
    #define Encoder_Left        htim3
    #define Encoder_Right       htim1
    #define ENCODER_WHEEL_DIAMETER         0.65f
    #define ENCODER_PULSE_PER_REV          1470.0f
    #define ENCODER_PI                     3.1415926f
#endif

#define DEVICE_TB6612 1
#if DEVICE_TB6612
    #include "gpio.h"
    #include "tim.h"
	#define TB6612_PWMA_TIM 			   htim2
	#define TB6612_PWMA_CH  			   TIM_CHANNEL_1
	#define TB6612_PWMB_TIM 			   htim2
	#define TB6612_PWMB_CH  			   TIM_CHANNEL_2
	#define TB6612_DIRA					   -1
	#define TB6612_DIRB					   -1
    #define AIN1(x)		                   HAL_GPIO_WritePin(TB6612_AIN1_GPIO_Port, TB6612_AIN1_Pin, (x))
    #define AIN2(x)		                   HAL_GPIO_WritePin(TB6612_AIN2_GPIO_Port, TB6612_AIN2_Pin, (x))
    #define BIN1(x)		                   HAL_GPIO_WritePin(TB6612_BIN1_GPIO_Port, TB6612_BIN1_Pin, (x))
    #define BIN2(x)		                   HAL_GPIO_WritePin(TB6612_BIN2_GPIO_Port, TB6612_BIN2_Pin, (x))
#endif

#define DEVICE_SERIAL   0
#if DEVICE_SERIAL
    #include "usart.h"
    #define SERIAL_NUMS     3       
#endif

#define DEVICE_HCSR04   0
#if DEVICE_HCSR04
    #include "tim.h"
    #include "gpio.h"
    #define HCSR04_TIM                  htim4
    #define HCSR04_TRIG(x)              HAL_GPIO_WritePin(HCSR04_TRIG_GPIO_Port, HCSR04_TRIG_Pin, (x))
    #define HCSR04_ECHO                 HAL_GPIO_ReadPin(HCSR04_ECHO_GPIO_Port, HCSR04_ECHO_Pin)
#endif

#define DEVICE_TRACKER  0
#if DEVICE_TRACKER
    #include "gpio.h"
    #define L2  HAL_GPIO_ReadPin(TRACKER_L2_GPIO_Port, TRACKER_L2_Pin)
    #define L1  HAL_GPIO_ReadPin(TRACKER_L1_GPIO_Port, TRACKER_L1_Pin)
    #define M   HAL_GPIO_ReadPin(TRACKER_M_GPIO_Port,  TRACKER_M_Pin)
    #define R1  HAL_GPIO_ReadPin(TRACKER_R1_GPIO_Port, TRACKER_R1_Pin)
    #define R2  HAL_GPIO_ReadPin(TRACKER_R2_GPIO_Port, TRACKER_R2_Pin)
#endif

#endif
