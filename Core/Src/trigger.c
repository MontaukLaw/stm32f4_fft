#include "trigger.h"

// 代表在圈圈中的状态
#define WHEEL_STATE_R_TO_RG 1
#define WHEEL_STATE_RG_TO_G 2
#define WHEEL_STATE_G_TO_GB 3
#define WHEEL_STATE_GB_TO_B 4
#define WHEEL_STATE_B_TO_BR 5
#define WHEEL_STATE_BR_TO_R 6
#define TRIGGER_DELAY 5

static uint8_t u2Buffer[10];

static uint8_t r = 255;
static uint8_t g = 0;
static uint8_t b = 0;

extern UART_HandleTypeDef huart2;
extern float32_t fftOutput[];
// 乘以12就是实际频率
uint16_t triggerFreqIdx = 0;
uint16_t triggerLinePos = 1;

// 每次运行都会更换一个颜色
void rainbow_color_change(void)
{
    static uint8_t wheelColorState = WHEEL_STATE_R_TO_RG;

    switch (wheelColorState)
    {
    case WHEEL_STATE_R_TO_RG:
        g++;
        if (g == 255)
        {
            wheelColorState = WHEEL_STATE_RG_TO_G;
        }
        break;
    case WHEEL_STATE_RG_TO_G:
        r--;
        if (r == 0)
        {
            wheelColorState = WHEEL_STATE_G_TO_GB;
        }

        break;
    case WHEEL_STATE_G_TO_GB:
        b++;
        if (b == 255)
        {
            wheelColorState = WHEEL_STATE_GB_TO_B;
        }
        break;

    case WHEEL_STATE_GB_TO_B:
        g--;
        if (g == 0)
        {
            wheelColorState = WHEEL_STATE_B_TO_BR;
        }
        break;
    case WHEEL_STATE_B_TO_BR:
        r++;
        if (r == 255)
        {
            wheelColorState = WHEEL_STATE_BR_TO_R;
        }

        break;
    case WHEEL_STATE_BR_TO_R:
        b--;
        if (b == 0)
        {
            wheelColorState = WHEEL_STATE_R_TO_RG;
        }

        break;
    }
}

uint8_t trigger_control(uint16_t triggerLinePos)
{
    float32_t triggerVF32 = (float32_t)(DATA_WEIGHT * (MAX_ROW - triggerLinePos));
    uint8_t ifTrigger = 0;
    static uint8_t delayTrigger = 0;

    // 避免重复触发, 因为10ms判断一次，所以需要延迟一段时间
    if (delayTrigger > 0)
    {
        delayTrigger--;
    }

    if (delayTrigger)
    {
        ifTrigger = 1;
        return ifTrigger;
    }
    
    uint8_t i = 0;

    // 保证频率idx不会小于1
    if (triggerFreqIdx > TRIGGER_BAR_WIDTH / 2)
    {
        for (i = 0; i < TRIGGER_BAR_WIDTH; i++)
        {
            float32_t triggerValue = fftOutput[triggerFreqIdx - 1 + i];

            // 一旦触发，就会发送数据
            if (triggerVF32 < triggerValue && delayTrigger == 0)
            {
                u2Buffer[0] = r + g + b;
                u2Buffer[1] = r;
                u2Buffer[2] = g;
                u2Buffer[3] = b;
                u2Buffer[4] = 0x0d;
                u2Buffer[5] = 0x0a;

                HAL_UART_Transmit(&huart2, u2Buffer, 6, 100);

                ifTrigger = 1;
                delayTrigger = TRIGGER_DELAY;
            }
        }
    }

    return ifTrigger;
}

void change_trigger_value(uint8_t inscreaseOrDecrease)
{
    if (inscreaseOrDecrease)
    {
        if (triggerLinePos < MAX_ROW)
        {
            triggerLinePos++;
        }
    }
    else
    {
        if (triggerLinePos)
        {
            triggerLinePos--;
        }
    }
}

void change_trigger_frequecy(uint8_t inscreaseOrDecrease)
{
    if (inscreaseOrDecrease)
    {
        if (triggerFreqIdx < MAX_FREQ_IDX)
        {
            triggerFreqIdx++;
        }
    }
    else
    {
        if (triggerFreqIdx > 0)
        {
            triggerFreqIdx--;
        }
    }
}

