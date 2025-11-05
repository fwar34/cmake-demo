#include "timer3.h"
#include "main.h"

static TIM_HandleTypeDef htim3;

// 定时器初始化函数
void TIM3_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    // 使能定时器时钟
    __HAL_RCC_TIM3_CLK_ENABLE();

    // 定时器基本配置
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = HAL_RCC_GetHCLKFreq() / 1000000 - 1;  // 使定时器计数周期为1微秒
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 0xFFFF;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    {
        Error_Handler();
    }

    // 配置定时器时钟源
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }

    // 配置定时器主模式
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

// 微秒延迟函数
void delay_us(uint32_t us)
{
    __disable_irq();  // 禁用中断
    __HAL_TIM_SET_COUNTER(&htim3, 0);  // 计数器清零
    HAL_TIM_Base_Start(&htim3);        // 启动定时器

    while (__HAL_TIM_GET_COUNTER(&htim3) < us);  // 等待计数器达到指定值

    HAL_TIM_Base_Stop(&htim3);         // 停止定时器
    __enable_irq();  // 恢复中断
}

void StartTimer3()
{
	__HAL_TIM_SET_COUNTER(&htim3, 0);  // 计数器清零
	HAL_TIM_Base_Start(&htim3);        // 启动定时器
}

void StopTimer3()
{
	HAL_TIM_Base_Stop(&htim3);         // 停止定时器
}

uint32_t Timer3Count()
{
	return __HAL_TIM_GET_COUNTER(&htim3);
}
