#include "HCSR04.h"

#if DEVICE_HCSR04

#include <stdint.h>

/*
 * s_time：回波脉冲宽度计数器，单位 10 µs
 * HCSR04_Read 在回波上升沿清零，下降沿读取；
 * HAL_TIM_PeriodElapsedCallback 每 10 µs 将其加 1。
 * volatile 防止编译器将中断与主循环的跨上下文访问优化掉。
 */
static volatile uint32_t s_time = 0;

/**
  * @brief  基于 DWT 周期计数器的微秒级忙等延时
  * @param  us 延时时间（微秒）
  * @retval 无
  */
static void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    /* us 换算为 CPU 周期数后忙等，精度优于 HAL_Delay（最小 1 ms） */
    uint32_t ticks = us * (SystemCoreClock / 1000000U);
    while ((DWT->CYCCNT - start) < ticks);
}

/**
  * @brief  初始化 HC-SR04 模块（使能 DWT 并启动定时器中断）
  * @note   须在 HAL_Init() 和 MX_TIMx_Init() 之后调用
  * @retval 无
  */
void HCSR04_Init(void)
{
    /* 使能 DWT 周期计数器，供 delay_us 做微秒级忙等延时 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT       = 0U;
    DWT->CTRL        |= DWT_CTRL_CYCCNTENA_Msk;

    /* 启动定时器更新中断，溢出周期 10 µs（PSC/ARR 在 CubeMX 中配置） */
    HAL_TIM_Base_Start_IT(&HCSR04_TIM);
}

/**
  * @brief  阻塞式超声波测距
  * @note   建议调用周期 ≥ 60 ms，避免余波干扰
  * @retval 距离（mm），超出量程或传感器无响应时返回 0
  */
int16_t HCSR04_Read(void)
{
    uint32_t time_end;
    uint32_t tick;

    /* 发送触发脉冲：Trig 拉高 15 µs 后拉低，启动一次测距 */
    HCSR04_TRIG(GPIO_PIN_SET);
    delay_us(15);
    HCSR04_TRIG(GPIO_PIN_RESET);

    /* 等待 Echo 上升沿，超时 50 ms 视为传感器未响应 */
    tick = HAL_GetTick();
    while (HCSR04_ECHO == GPIO_PIN_RESET)
    {
        if (HAL_GetTick() - tick > 50U)
        {
            return 0;
        }
    }

    /* 回波上升沿：清零计时器，开始累计脉冲宽度 */
    s_time = 0;

    /* 等待 Echo 下降沿，超时 40 ms 视为超出量程 */
    tick = HAL_GetTick();
    while (HCSR04_ECHO == GPIO_PIN_SET)
    {
        if (HAL_GetTick() - tick > 40U)
        {
            return 0;
        }
    }

    /* 读取脉冲宽度；≥ 38 ms（3800 个 10 µs）为超出量程，返回 0 */
    time_end = s_time;
    if (time_end >= 3800U)
    {
        return 0;
    }

    /* 距离(mm) = time_end × 10µs × 346m/s / 2 = time_end × 346 / 200 */
    return (int16_t)((uint32_t)(time_end * 346U) / 200U);
}

/**
  * @brief  定时器溢出中断回调（覆盖 HAL 弱定义）
  * @note   每当 HCSR04_TIM 溢出（10 µs）时将 s_time 加 1，
  *         通过 htim->Instance 比较忽略其他定时器。
  *         若项目其他地方也定义了此回调，请删除本函数并在
  *         自定义回调中手动调用本库的逻辑。
  * @param  htim 触发中断的定时器句柄指针
  * @retval 无
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == HCSR04_TIM.Instance)
    {
        s_time++;
    }
}

#endif /* DEVICE_HCSR04 */
