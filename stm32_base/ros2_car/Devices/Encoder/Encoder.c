#include "Encoder.h"

#if DEVICE_ENCODER

/**
    * @brief  编码器初始化
    * @param  无
    * @retval 无
    */
void Encoder_Init(void)
{
    HAL_TIM_Encoder_Start(&Encoder_Left, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&Encoder_Right, TIM_CHANNEL_ALL);
}

/**
    * @brief  读取编码器速度
    * @param  left  左轮速度指针
    * @param  right 右轮速度指针
    * @retval 无
    */
void Encoder_Read(float *left, float *right)
{
    int16_t count_left;
    int16_t count_right;
    float distance_per_pulse_mm;
    uint32_t now_tick;
    float dt;
    static uint32_t last_tick = 0;

    if ((left == NULL) || (right == NULL)) {
        return;
    }

    now_tick = HAL_GetTick();

    count_left = (int16_t)__HAL_TIM_GET_COUNTER(&Encoder_Left);
    count_right = (int16_t)__HAL_TIM_GET_COUNTER(&Encoder_Right);

    __HAL_TIM_SET_COUNTER(&Encoder_Left, 0);
    __HAL_TIM_SET_COUNTER(&Encoder_Right, 0);

    if (last_tick == 0U) {
        last_tick = now_tick;
        return;
    }

    dt = (now_tick - last_tick) / 1000.0f;
    if (dt <= 0.0f) {
        return;
    }

    last_tick = now_tick;

    distance_per_pulse_mm = (ENCODER_WHEEL_DIAMETER * ENCODER_PI) / ENCODER_PULSE_PER_REV;

    *left = (count_left * distance_per_pulse_mm) / dt;
    *right = (count_right * distance_per_pulse_mm) / dt;
}

#endif
