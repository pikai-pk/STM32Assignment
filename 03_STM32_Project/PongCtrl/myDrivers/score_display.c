#include "score_display.h"
#include "disp_seg.h"

static volatile uint16_t g_score = 0U;
static volatile uint16_t g_self_test_ticks = 0U;

void ScoreDisplay_Init(void)
{
    SEG_Init();
    g_score = 0U;
    g_self_test_ticks = 250U;
}

void ScoreDisplay_Reset(void)
{
    g_score = 0U;
}

void ScoreDisplay_Tick(void)
{
    if (g_self_test_ticks > 0U)
    {
        g_self_test_ticks--;
        SEG_DisplayScan(8888U);
        return;
    }

    SEG_DisplayScan(g_score);
}

void ScoreDisplay_AddWin(void)
{
    if (g_score < 9999U)
    {
        g_score++;
    }
}

uint16_t ScoreDisplay_GetScore(void)
{
    return g_score;
}
