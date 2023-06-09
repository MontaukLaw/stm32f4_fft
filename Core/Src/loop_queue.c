#include "arm_math.h"
#include "loop_queue.h"

// 512个点+64个点的fft计算时间
float32_t adcData[WRITE_IDX_SIZE];
volatile float32_t pickDataBuf[FFT_INPUT_DATA_LEN];

uint16_t writeIdx = 0;
uint16_t readIdx;

void write_idx_plus(void)
{
    writeIdx++;
    if (writeIdx >= WRITE_IDX_SIZE)
    {
        writeIdx = 0;
    }
}

uint16_t save_data(float32_t adcValue)
{
    adcData[writeIdx] = adcValue;
    write_idx_plus();
    adcData[writeIdx] = 0;
    write_idx_plus();
    return writeIdx;
}

void pick_data(void)
{

    memset(pickDataBuf, 0, FFT_INPUT_DATA_LEN * sizeof(float32_t));
    // 0 1 2 3 4 5 6 7 8 9 10
    // 3种情况
    // 1. writtingIdx = 0, 直接用从1开始到10的数据
    // 2. writtingIdx = 10, 直接用从0开始到9的数据

    if (writeIdx == 0)
    {
			  float32_t * adcDataPtr = &adcData[EACH_10_MS_LEN]; 
        memcpy(pickDataBuf, adcDataPtr, FFT_INPUT_DATA_LEN * sizeof(float32_t));
    }
    else if (writeIdx == FFT_INPUT_DATA_LEN)
    {
        memcpy(pickDataBuf, adcData, FFT_INPUT_DATA_LEN * sizeof(float32_t));
    }
    else
    {
        // if idx = 2; 即从3到10, 再从0到1
        // 0 1 2 3 4 5 6 7 8 9 10
        // |_|_|_|_|_|_|_|_|_|_|_|
        float32_t *oldertDataPtr = &adcData[writeIdx + EACH_10_MS_LEN];
        uint16_t part1DataLen = (WRITE_IDX_SIZE - writeIdx) * sizeof(float32_t);

        // 这一部分复制到data中
        memcpy(pickDataBuf, oldertDataPtr, part1DataLen);

        // 获取更新的数据长度 从0到64
        uint16_t updateDataLen = writeIdx * sizeof(float32_t);

        memcpy(&pickDataBuf[FFT_INPUT_DATA_LEN - writeIdx], adcData, updateDataLen);
    }
}

void circle_queue_test(void)
{
    uint16_t i;
    for (i = 0; i < 2000; i++)
    {
        uint16_t wIdx = save_data((float32_t)i);
        if (wIdx % EACH_10_MS_LEN == 0)
        {
            pick_data();
        }
    }
}
