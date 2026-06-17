#include "disp_seg.h"

static const uint8_t SEG_TAB[10] = {
    0x3FU, 0x06U, 0x5BU, 0x4FU, 0x66U,
    0x6DU, 0x7DU, 0x07U, 0x7FU, 0x6FU
};

static uint8_t g_scan_bit = 0U;

static void SEG_OutputDisable(void)
{
#ifdef DISEN_Pin
    HAL_GPIO_WritePin(DISEN_GPIO_Port, DISEN_Pin, GPIO_PIN_SET);
#endif
    HAL_GPIO_WritePin(A3_GPIO_Port, A3_Pin, GPIO_PIN_RESET);
}

static void SEG_OutputEnable(void)
{
    HAL_GPIO_WritePin(A3_GPIO_Port, A3_Pin, GPIO_PIN_SET);
#ifdef DISEN_Pin
    HAL_GPIO_WritePin(DISEN_GPIO_Port, DISEN_Pin, GPIO_PIN_RESET);
#endif
}

void SEG_Init(void)
{
    g_scan_bit = 0U;
    SEG_OutputDisable();
    SEG_SendByte(0x00U);
}

void SEG_SendByte(uint8_t byte)
{
    uint8_t i;

    for (i = 0U; i < 8U; i++)
    {
        HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(SER_GPIO_Port, SER_Pin, (byte & 0x80U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        byte <<= 1U;
        HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_SET);
    }

    HAL_GPIO_WritePin(DISLK_GPIO_Port, DISLK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DISLK_GPIO_Port, DISLK_Pin, GPIO_PIN_SET);
}

void SEG_SelectBit(uint8_t bit)
{
    HAL_GPIO_WritePin(A0_GPIO_Port, A0_Pin, (bit & 0x01U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(A1_GPIO_Port, A1_Pin, (bit & 0x02U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(A2_GPIO_Port, A2_Pin, (bit & 0x04U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void SEG_DisplayOne(uint8_t bit, uint8_t num)
{
    if (bit > 3U)
    {
        return;
    }

    if (num > 9U)
    {
        num = 0U;
    }

    SEG_OutputDisable();
    SEG_SendByte(SEG_TAB[num]);
    SEG_SelectBit(bit);
    SEG_OutputEnable();
}

void SEG_DisplayScan(uint16_t num)
{
    uint8_t digits[4];

    if (num > 9999U)
    {
        num = 9999U;
    }

    digits[0] = (uint8_t)(num / 1000U);
    digits[1] = (uint8_t)((num % 1000U) / 100U);
    digits[2] = (uint8_t)((num % 100U) / 10U);
    digits[3] = (uint8_t)(num % 10U);

    SEG_DisplayOne(g_scan_bit, digits[g_scan_bit]);

    g_scan_bit++;
    if (g_scan_bit >= 4U)
    {
        g_scan_bit = 0U;
    }
}
