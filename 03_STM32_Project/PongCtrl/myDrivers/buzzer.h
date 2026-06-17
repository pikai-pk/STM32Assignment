#ifndef __BUZZER_H
#define __BUZZER_H

#include "main.h"

/* 蜂鸣器事件类型，用于 FreeRTOS 队列传递胜负结果。 */
typedef enum
{
    BUZZER_EVENT_NONE = 0,
    BUZZER_EVENT_WIN,
    BUZZER_EVENT_LOSE,
    BUZZER_EVENT_RESET_SCORE
} BuzzerEvent_t;

/* 初始化蜂鸣器 PWM，默认关闭输出。 */
void Buzzer_Init(void);

/* 设置蜂鸣器频率，单位 Hz。freq 为 0 时关闭蜂鸣器。 */
void Buzzer_SetFrequency(uint16_t freq);

/* 开启蜂鸣器 PWM 输出。 */
void Buzzer_Start(void);

/* 关闭蜂鸣器 PWM 输出。 */
void Buzzer_Stop(void);

/* 播放胜利提示音。 */
void Buzzer_PlayWinMusic(void);

/* 播放失败提示音。 */
void Buzzer_PlayLoseMusic(void);

#endif
