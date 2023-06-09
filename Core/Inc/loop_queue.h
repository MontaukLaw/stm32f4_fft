#ifndef _LOOP_QUEUE_H_
#define _LOOP_QUEUE_H_

// 每个80ms内的fft次数
#define ADC_TIMES_PER_FFT 8

// 0   1   2   3   4   5   6   7   8   9
// |___|___|___|___|___|___|___|___|___|
// #define WRITE_IDX_SIZE (ADC_TIMES_PER_FFT + 1)

// 基调是fft的大小为4096，输出数据是4096/2
#define FFT_SIZE 1024
// #define FFT_SIZE 1024
// 输入数据是FFT_SIZE*2
#define FFT_INPUT_DATA_SIZE (FFT_SIZE * 2)
// ADC采样数据应该是
#define ADC_DATA_BUFFER_SIZE_FOR_ONE_FFT (FFT_INPUT_DATA_SIZE / 2)

// 先简单乘以2
#define ADC_DATA_BUFFER_SIZE_FOR_ARRAY (ADC_DATA_BUFFER_SIZE_FOR_ONE_FFT * 2)

// FFT的输入是1024
#define FFT_INPUT_DATA_LEN FFT_SIZE

// 128其实是1024/8
#define EACH_10_MS_LEN 128

#define FFT_OUTPUT_DATA_SIZE (FFT_SIZE / 2)

// 每个80ms内数据保存的最大次数
#define WRITE_IDX_SIZE (FFT_INPUT_DATA_LEN + EACH_10_MS_LEN)

#include "stm32f4xx_hal.h"
#include "main.h"

uint16_t save_data(float32_t adcValue);

void circle_queue_test(void);

void pick_data(void);

#endif

