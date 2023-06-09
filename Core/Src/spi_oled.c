#include "main.h"
#include "oled.h"

uint8_t oledDMABuf[OLED_WIDTH * OLED_HEIGHT / 8];
volatile uint8_t barData[OLED_WIDTH];
volatile uint8_t barSegment[OLED_WIDTH];
volatile uint8_t disData[OLED_WIDTH * OLED_HEIGHT / 8];

extern uint16_t triggerFreqIdx;
extern uint16_t triggerLinePos;
extern float32_t fftOutput[];
extern SPI_HandleTypeDef hspi1;

static void SPI_OLED_WR_Byte(uint8_t data, uint8_t cmd)
{

    if (cmd)
    {
        OLED_DC_Set();
    }
    else
    {
        OLED_DC_Clr();
    }

    oledDMABuf[0] = data;

    HAL_SPI_Transmit_DMA(&hspi1, oledDMABuf, 1);
    HAL_Delay(20);

    OLED_DC_Set();
}

void SPI_OLED_Init(void)
{
    OLED_RST_Set();
    HAL_Delay(100);
    OLED_RST_Clr();
    HAL_Delay(100);
    OLED_RST_Set();

    SPI_OLED_WR_Byte(0xAE, OLED_CMD); // --turn off oled panel
    SPI_OLED_WR_Byte(0x00, OLED_CMD); // ---set low column address
    SPI_OLED_WR_Byte(0x10, OLED_CMD); // ---set high column address
    SPI_OLED_WR_Byte(0x40, OLED_CMD); // --set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    SPI_OLED_WR_Byte(0x81, OLED_CMD); // --set contrast control register
    SPI_OLED_WR_Byte(0xCF, OLED_CMD); // Set SEG Output Current Brightness
    SPI_OLED_WR_Byte(0xA1, OLED_CMD); // --Set SEG/Column Mapping     0xa0锟斤拷锟揭凤拷锟斤拷 0xa1锟斤拷锟斤拷
    SPI_OLED_WR_Byte(0xC8, OLED_CMD); // Set COM/Row Scan Direction   0xc0锟斤拷锟铰凤拷锟斤拷 0xc8锟斤拷锟斤拷
    SPI_OLED_WR_Byte(0xA6, OLED_CMD); // --set normal display
    SPI_OLED_WR_Byte(0xA8, OLED_CMD); // --set multiplex ratio(1 to 64)
    SPI_OLED_WR_Byte(0x3f, OLED_CMD); // --1/64 duty
    SPI_OLED_WR_Byte(0xD3, OLED_CMD); // -set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    SPI_OLED_WR_Byte(0x00, OLED_CMD); // -not offset
    SPI_OLED_WR_Byte(0xd5, OLED_CMD); // --set display clock divide ratio/oscillator frequency
    SPI_OLED_WR_Byte(0x80, OLED_CMD); // --set divide ratio, Set Clock as 100 Frames/Sec
    SPI_OLED_WR_Byte(0xD9, OLED_CMD); // --set pre-charge period
    SPI_OLED_WR_Byte(0xF1, OLED_CMD); // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    SPI_OLED_WR_Byte(0xDA, OLED_CMD); // --set com pins hardware configuration
    SPI_OLED_WR_Byte(0x12, OLED_CMD);
    SPI_OLED_WR_Byte(0xDB, OLED_CMD); // --set vcomh
    SPI_OLED_WR_Byte(0x40, OLED_CMD); // Set VCOM Deselect Level
    SPI_OLED_WR_Byte(0x20, OLED_CMD); // -Set Page Addressing Mode (0x00/0x01/0x02)
    SPI_OLED_WR_Byte(0x00, OLED_CMD);
    SPI_OLED_WR_Byte(0x8D, OLED_CMD); // --set Charge Pump enable/disable
    SPI_OLED_WR_Byte(0x14, OLED_CMD); // --set(0x10) disable
    SPI_OLED_WR_Byte(0xA4, OLED_CMD); // Disable Entire Display On (0xa4/0xa5)
    SPI_OLED_WR_Byte(0xA6, OLED_CMD); // Disable Inverse Display On (0xa6/a7)
    SPI_OLED_WR_Byte(0xAF, OLED_CMD); // --turn on oled panel
    SPI_OLED_WR_Byte(0xAF, OLED_CMD); /*display ON*/
    SPI_OLED_WR_Byte(0xb0, OLED_CMD); // 设置页地址（0~7）
    SPI_OLED_WR_Byte(0x00, OLED_CMD); // 设置显示位置—列低地址为0
    SPI_OLED_WR_Byte(0x10, OLED_CMD); // 设置显示位置—列高地址为0
}

void white_screen(void)
{
    uint16_t i;

    for (i = 0; i < ALL_SCREE_DATA_LEN; i++)
    {
        oledDMABuf[i] = 0xff;
    }

    HAL_SPI_Transmit_DMA(&hspi1, oledDMABuf, ALL_SCREE_DATA_LEN);
}

void full_screen_oled_test(void)
{
    uint16_t i;

    for (i = 0; i < 1024; i++)
    {
        oledDMABuf[i] = 0xff;
    }

    HAL_SPI_Transmit_DMA(&hspi1, oledDMABuf, 1024);
}

void pick_data_low_frequence(float32_t *sourceData, uint8_t *targetData, uint8_t *targetBarSegment, uint8_t barWidth)
{
    uint16_t sourceIdx = 0;
    uint8_t barIdx = 0;
    uint8_t targetDataIdx = 0;

    uint8_t temp = 0;

    for (sourceIdx = FREQUENCE_SHIFT; sourceIdx < OLED_WIDTH + FREQUENCE_SHIFT; sourceIdx++)
    {
        temp = (uint8_t)(sourceData[sourceIdx] / DATA_WEIGHT);

        for (barIdx = 0; barIdx < barWidth; barIdx++)
        {
            if (temp > OLED_HEIGHT)
            {
                temp = OLED_HEIGHT;
            }
            // if (temp > 8)
            // {
            targetBarSegment[targetDataIdx + barIdx] = (temp - 1) / 8 + 1;
            targetData[targetDataIdx + barIdx] = temp;
            // }
        }

        targetDataIdx = targetDataIdx + barWidth;
    }
}

static void add_target_value_line(void)
{
    uint8_t colIdx = 0;
    uint8_t rowIdx = rowIdx = triggerLinePos / (OLED_HEIGHT / BITS_PER_BYTE);

    uint16_t temp = triggerLinePos % (OLED_HEIGHT / BITS_PER_BYTE);
    uint8_t lineVal = 0x01 << temp;
    for (colIdx = 0; colIdx < OLED_WIDTH; colIdx++)
    {
        disData[colIdx + rowIdx * OLED_WIDTH] = disData[colIdx + rowIdx * OLED_WIDTH] | lineVal;
    }

    return;

#if 0
    // 画一条线
    // 从左到右, 位于第几行
    // uint8_t rowIdx = triggerLinePos / (OLED_HEIGHT / BITS_PER_BYTE);
    uint16_t bytesInSameRow = 0;

    uint8_t *disBytesPtr = &disData[rowIdx * OLED_WIDTH];

    for (colIdx = 0; colIdx < OLED_WIDTH; colIdx++)
    {

        *disBytesPtr = *disBytesPtr | lineVal;
        disBytesPtr = disBytesPtr + colIdx;
    }
#endif
		
}

static void add_target_freq_bar(void)
{
    uint8_t colIdx = triggerFreqIdx;
    uint8_t rowIdx = 0;
    uint8_t temp = 0x0F;
    if (colIdx > TRIGGER_BAR_WIDTH / 2 && colIdx < OLED_WIDTH - TRIGGER_BAR_WIDTH / 2)
    {
        uint16_t startIdx = colIdx + rowIdx * OLED_WIDTH - 1;

        for (uint16_t j = startIdx; j < startIdx + TRIGGER_BAR_WIDTH; j++)
        {
            disData[j] = disData[j] | temp;
        }
        // disData[colIdx + rowIdx * OLED_WIDTH - 1] = disData[colIdx + rowIdx * OLED_WIDTH - 1] | temp;
        // disData[colIdx + rowIdx * OLED_WIDTH] = disData[colIdx + rowIdx * OLED_WIDTH] | temp;
        // disData[colIdx + rowIdx * OLED_WIDTH + 1] = disData[colIdx + rowIdx * OLED_WIDTH + 1] | temp;
    }
}

void SPI_OLED_FULL_DISP(uint8_t ifTrgger)
{

    if (ifTrgger)
    {
        full_screen_oled_test();
        return;
        // 画触发线
        // add_target_value_line();
    }

    memset(barData, 0, OLED_WIDTH);
    memset(barSegment, 0, OLED_WIDTH);

    // 提取数据
    // pick_data_by_step(fftOutput, barData, barSegment, 2, 130);
    pick_data_low_frequence(fftOutput, barData, barSegment, 1);

    // 测试数据
    // init_test_bar_data();

    // 清理数据
    memset(disData, 0, TOTAL_DSIPLAY_DATA_LEN);

    uint8_t rowIdx, colIdx;
    uint16_t counter = 0;
    uint8_t headData = 0;
    uint16_t rowBaseIdx = 0;

    // 将bar数据转成显示数据
    for (rowIdx = 0; rowIdx < 8; rowIdx++)
    {
        rowBaseIdx = rowIdx * OLED_WIDTH;

        for (colIdx = 0; colIdx < OLED_WIDTH; colIdx++)
        {
            // step 1: 获取bar的数据
            volatile uint8_t *barHeight = &barData[colIdx];
            if (*barHeight == 0)
                continue;

            // step 2: 获取bar的段数, 即行数
            volatile uint8_t *barSeg = &barSegment[colIdx];
            counter = colIdx + rowBaseIdx;
            // step3: 比较当前行数, 是否在bar的段数内
            // *barSegment范围1~8,
            // 如果目前rowIdx为0, barSegment为1, 不显示这一行
            // 如果目前rowIdx为2, barSegment为1, 不显示这一行
            // 但是如果rowIdx为7, barSegment为1, 则显示这一行
            // barSegment为1
            // 显示头部
            if (rowIdx == (8 - *barSeg))
            {
                // 显示头部
                uint8_t headLen = *barHeight - (*barSeg - 1) * 8;
                headData = 0;
                if (headLen > 0)
                {
                    for (uint8_t i = 0; i < headLen; i++)
                    {
                        headData |= 0x80 >> i;
                    }
                    disData[counter] = headData;
                }
            }
            else if (rowIdx > (8 - *barSeg))
            {
                // 显示下面的全0xff的部分
                disData[counter] = 0xff;
            }
        }
    }

    // 画线
    add_target_value_line();
    add_target_freq_bar();

    HAL_SPI_Transmit_DMA(&hspi1, disData, TOTAL_DSIPLAY_DATA_LEN);
}
