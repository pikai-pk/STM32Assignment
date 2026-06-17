#include "buzzer.h"
#include "tim.h"
#include "cmsis_os2.h"

typedef struct
{
    uint16_t freq;
    uint16_t duration_ms;
} Note_t;

static const Note_t WinMusic[] = {
    {523, 150},
    {659, 150},
    {784, 150},
    {1046, 300},
    {0, 100},
    {1046, 300}
};

static const Note_t LoseMusic[] = {
    {392, 250},
    {370, 250},
    {349, 400},
    {330, 600}
};

/* 初始化蜂鸣器 PWM，系统启动后先保持静音。 */
void Buzzer_Init(void)
{
    Buzzer_Stop();
}

/* 设置蜂鸣器 PWM 频率，使用 TIM3 的 1MHz 计数频率计算 ARR/CCR。 */
void Buzzer_SetFrequency(uint16_t freq)
{
    uint32_t period;

    if (freq == 0U)
    {
        Buzzer_Stop();
        return;
    }

    period = 1000000UL / freq;
    if (period < 2UL)
    {
        period = 2UL;
    }

    __HAL_TIM_SET_AUTORELOAD(&htim3, period - 1UL);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, period / 2UL);
}

/* 开启蜂鸣器 PWM 输出。 */
void Buzzer_Start(void)
{
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

/* 关闭蜂鸣器 PWM 输出，并把占空比清零。 */
void Buzzer_Stop(void)
{
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0U);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}

/* 按音符表播放一段提示音，freq 为 0 表示休止。 */
static void Buzzer_PlayNotes(const Note_t *notes, uint16_t count)
{
    uint16_t i;

    for (i = 0U; i < count; ++i)
    {
        if (notes[i].freq == 0U)
        {
            Buzzer_Stop();
        }
        else
        {
            Buzzer_SetFrequency(notes[i].freq);
            Buzzer_Start();
        }

        osDelay(notes[i].duration_ms);
        Buzzer_Stop();
        osDelay(30U);
    }
}

/* 播放胜利提示音，使用上升音调表示获胜。 */
void Buzzer_PlayWinMusic(void)
{
    Buzzer_PlayNotes(WinMusic, (uint16_t)(sizeof(WinMusic) / sizeof(WinMusic[0])));
}

/* 播放失败提示音，使用下降音调表示失败。 */
void Buzzer_PlayLoseMusic(void)
{
    Buzzer_PlayNotes(LoseMusic, (uint16_t)(sizeof(LoseMusic) / sizeof(LoseMusic[0])));
}
