#include "TB6612.h"

#if DEVICE_TB6612

/**
  * @brief  TB6612初始化
  * @param  无
  * @retval 无
  */
void TB6612_Init(void)
{
	HAL_TIM_PWM_Start(&TB6612_PWMA_TIM, TB6612_PWMA_CH);
	HAL_TIM_PWM_Start(&TB6612_PWMB_TIM, TB6612_PWMB_CH);
}

/**
  * @brief  电机控制
  * @param  motora 左电机PWM值（带符号）
  * @param  motorb 右电机PWM值（带符号）
  * @retval 无
  */
void TB6612_Motor(int16_t motora, int16_t motorb)
{
    int16_t max_pwma = (int16_t)__HAL_TIM_GET_AUTORELOAD(&TB6612_PWMA_TIM);
    int16_t max_pwmb = (int16_t)__HAL_TIM_GET_AUTORELOAD(&TB6612_PWMB_TIM);

    motora = motora * TB6612_DIRA;
    motorb = motorb * TB6612_DIRB;

    if (motora > max_pwma) motora = max_pwma;
    if (motora < -max_pwma) motora = -max_pwma;
    if (motorb > max_pwmb) motorb = max_pwmb;
    if (motorb < -max_pwmb) motorb = -max_pwmb;

    if(motora < 0)
    {
        AIN1(0);
        AIN2(1);
        motora = -motora;
    }
    else
    {
        AIN1(1);
        AIN2(0);
    }
    __HAL_TIM_SET_COMPARE(&TB6612_PWMA_TIM, TB6612_PWMA_CH, motora);

    if(motorb < 0)
    {
        BIN1(0);
        BIN2(1);
        motorb = -motorb;
    }
    else
    {
        BIN1(1);
        BIN2(0);
    }
    __HAL_TIM_SET_COMPARE(&TB6612_PWMB_TIM, TB6612_PWMB_CH, motorb);
}

#endif
