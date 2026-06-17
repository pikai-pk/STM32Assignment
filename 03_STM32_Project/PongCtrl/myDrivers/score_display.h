#ifndef __SCORE_DISPLAY_H
#define __SCORE_DISPLAY_H

#include "main.h"

/* 初始化分数显示，默认显示 0000。 */
void ScoreDisplay_Init(void);

/* 分数清零，游戏退出、网络断开或重新连接时调用。 */
void ScoreDisplay_Reset(void);

/* 高频刷新四位数码管，建议 1~2ms 调用一次。 */
void ScoreDisplay_Tick(void);

/* 胜利一次分数加 1，并立即刷新显示。 */
void ScoreDisplay_AddWin(void);

/* 获取当前分数，方便串口调试打印。 */
uint16_t ScoreDisplay_GetScore(void);

#endif
