/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "loop_queue.h"
#include "trigger.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t adcBuffer[AVG_NUMBER];
char uartBuffer[100];
uint16_t avgNoise = 0;
uint16_t fftStartIdx = 0;
float32_t fftOutput[FFT_OUTPUT_DATA_SIZE];
float32_t triggerValue = 0;
float32_t hanningWindow[1];

static uint8_t keyState = 0;
static float32_t adcAvgValue = 0;

extern const arm_cfft_instance_f32 arm_cfft_sR_f32_len512;
extern float32_t pickDataBuf[];
extern uint16_t triggerFreqIdx;
extern uint16_t triggerLinePos;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void generateHanningWindow(float32_t *window, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        window[i] = 0.5f * (1.0f - arm_cos_f32(2.0f * PI * i / (length - 1)));
    }
}

void window_filter(void)
{
    uint16_t i = 0;
    for (i = 0; i < FFT_INPUT_DATA_LEN; i++)
    {
        pickDataBuf[i] = pickDataBuf[i] * hanningWindow[i];
    }
}

// fft
void do_fft(void)
{
    // float32_t maxValue = 0;
    // uint32_t testIndex = 0;
    // static uint32_t fftSize = 512;
    // 加窗
    // window_filter();

    memset(fftOutput, 0, FFT_OUTPUT_DATA_SIZE * sizeof(float32_t));

    arm_cfft_f32(&arm_cfft_sR_f32_len512, pickDataBuf, 0, 1);
    arm_cmplx_mag_f32(pickDataBuf, fftOutput, 512);

    fftOutput[0] = 0;
    fftOutput[1] = 0;

    // float32_t selectedValue = fftOutput[triggerFreqIdx];
    uint8_t ifTriggered = trigger_control(triggerLinePos);

    // OLED_FULL_DISP();
    SPI_OLED_FULL_DISP(ifTriggered);
}

// check key press status
void key_check(void)
{
    static uint16_t keyCounter = 0;

    // 调节tim3占空�????
    // TIM3_PWM_SetDutyCycle();

    if (keyCounter)
    {
        keyCounter--;
    }

    if (keyCounter)
    {
        return;
    }

    if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
    {
        keyState = KEY_UP;
        keyCounter = KEY_DELAY;
    }
    else if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET)
    {
        keyState = KEY_DOWN;
        keyCounter = KEY_DELAY;
    }
    else if (HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin) == GPIO_PIN_RESET)
    {
        keyState = KEY_LEFT;
        keyCounter = KEY_DELAY;
    }
    else if (HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin) == GPIO_PIN_RESET)
    {
        keyState = KEY_RIGHT;
        keyCounter = KEY_DELAY;
    }

    if (keyState)
    {
        sprintf(uartBuffer, "key:%d\r\n", keyState);
        HAL_UART_Transmit(&huart1, (uint8_t *)uartBuffer, strlen(uartBuffer), 0xFFFF);
        switch (keyState)
        {
        case KEY_UP:
            change_trigger_value(1);
            break;
        case KEY_DOWN:
            change_trigger_value(0);
            break;
        case KEY_LEFT:
            change_trigger_frequecy(1);
            break;
        case KEY_RIGHT:
            change_trigger_frequecy(0);
            break;
        }
        keyState = 0;
    }
}

static uint16_t get_avg_noise(uint16_t counterNumber)
{
    uint16_t i = 0;
    uint32_t sum = 0;
    for (i = 0; i < counterNumber; i++)
    {
        sum += (uint16_t)adcBuffer[i];
    }

    return (uint16_t)(sum / counterNumber);
}

static float32_t filter_white_noise(uint16_t *adcValue)
{

    int16_t temp = ((int16_t)get_avg_noise(ADC_BUFFER_SIZE) - (int16_t)avgNoise);
    // return temp;

    if (temp > WHITE_NOISE_THRESHOLD)
    {
        return (float32_t)(temp - WHITE_NOISE_THRESHOLD);
    }
    else if (temp < -WHITE_NOISE_THRESHOLD)
    {
        return (float32_t)(temp + WHITE_NOISE_THRESHOLD);
    }

    return 0;
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */
    generateHanningWindow(hanningWindow, FFT_INPUT_DATA_LEN);
    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();
    MX_ADC1_Init();
    MX_TIM5_Init();
    MX_SPI1_Init();
    MX_USART2_UART_Init();
    /* USER CODE BEGIN 2 */

    // collect avg noise
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcBuffer, AVG_NUMBER);
    HAL_Delay(1000);
    avgNoise = get_avg_noise(AVG_NUMBER);
    sprintf(uartBuffer, "avg noise:%d\r\n", avgNoise);
    HAL_UART_Transmit(&huart1, (uint8_t *)uartBuffer, strlen(uartBuffer), 100);

    HAL_ADC_Stop_DMA(&hadc1);

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcBuffer, ADC_BUFFER_SIZE);

    // 打开tim5
    HAL_TIM_Base_Start_IT(&htim5);

    SPI_OLED_Init();
    // full_screen_oled_test();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
        // full_screen_oled_test();
        if (fftStartIdx)
        {
            // full_screen_oled_test();
            // HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
            // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
            do_fft();
            rainbow_color_change();
            // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
            fftStartIdx = 0;
        }
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 84;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

    static uint8_t fillFlag = 0;
    uint16_t writeIdx = 0;
    if (htim->Instance == TIM5)
    {

        writeIdx = save_data(adcAvgValue);

        if (writeIdx % EACH_10_MS_LEN == 0)
        {
            pick_data();
            key_check();
            fftStartIdx = 1;
        }
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{

    adcAvgValue = filter_white_noise(adcBuffer);
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
