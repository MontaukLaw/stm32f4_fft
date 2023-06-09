#ifndef __OLED_H_
#define __OLED_H_

#include "main.h"

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

#define OLED_MODE 0
#define SIZE 16
#define XLevelL 0x00
#define XLevelH 0x10
#define MAX_COL 128

#define Brightness 0xFF
#define X_WIDTH 128
#define Y_WIDTH 64
#define BITS_PER_BYTE 8
#define TRIGGER_BAR_WIDTH 3
#define ALL_SCREE_DATA_LEN (X_WIDTH * Y_WIDTH / BITS_PER_BYTE)  

// 因为最终想显示的条尽量多, 从1024个数据中, 找到
// #define FFT_RESULT_NMB 256
// #define FFT_RESULT_NMB 512
// #define FFT_RESULT_NMB 1024
// oled屏幕col数量为128
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
// 每8个原始数据形成一个bar
// #define DATA_STEP (FFT_RESULT_NMB / OLED_WIDTH)
#define TOTAL_DSIPLAY_DATA_LEN (OLED_WIDTH * OLED_HEIGHT / 8)

// RES
#define OLED_RST_Clr() HAL_GPIO_WritePin(RES_GPIO_Port, RES_Pin, GPIO_PIN_RESET);
#define OLED_RST_Set() HAL_GPIO_WritePin(RES_GPIO_Port, RES_Pin, GPIO_PIN_SET);

// DC
#define OLED_DC_Clr() HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_RESET)
#define OLED_DC_Set() HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET)

#if 0
// CS
#define OLED_CS_Clr() HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
#define OLED_CS_Set() HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

// CLK D0
#define OLED_SCLK_Clr() HAL_GPIO_WritePin(D0_GPIO_Port, D0_Pin, GPIO_PIN_RESET);
#define OLED_SCLK_Set() HAL_GPIO_WritePin(D0_GPIO_Port, D0_Pin, GPIO_PIN_SET);
// DIN D1
#define OLED_SDIN_Clr() HAL_GPIO_WritePin(D1_GPIO_Port, D1_Pin, GPIO_PIN_RESET);
#define OLED_SDIN_Set() HAL_GPIO_WritePin(D1_GPIO_Port, D1_Pin, GPIO_PIN_SET)
#endif

#define OLED_CMD 0
#define OLED_DATA 1

// void OLED_WR_Byte(u8 dat, u8 cmd);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x, u8 y, u8 t);
void OLED_Fill(u8 x1, u8 y1, u8 x2, u8 y2, u8 dot);
void OLED_ShowChar(u8 x, u8 y, u8 chr);
void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size);
void OLED_ShowString(u8 x, u8 y, u8 *p);
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_ShowCHinese(u8 x, u8 y, u8 no);
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[]);

void OLED_DRAW_LINE_HOR(uint8_t rowIdx);
void OLED_BAR(uint8_t colIdx, uint8_t barHeight, uint8_t barWidth);
void OLED_FULL_DISP(void);
void init_test_data(void);

void SPI_OLED_Init(void);

void full_screen_oled_test(void);

// void SPI_OLED_FULL_DISP(void);

void white_screen(void);

void SPI_OLED_FULL_DISP(uint8_t ifTrgger);

#endif
