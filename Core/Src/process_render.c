/*
 * process_render.c
 *
 *  Created on: Mar 23, 2025
 *      Author: fwar3
 */
#include "process_render.h"
#include "image.h"
#include "st7735s.h"
#include "stm32f1xx_hal.h"

static uint8_t moveStart = 1;
static uint16_t xStart = 0;
static uint16_t needRefresh = 0;

// uint8_t renderBuffer[2][LCD_WIDTH * LCD_HEIGHT * 2];

void ImageMoveToggle() { moveStart = !moveStart; }

void ImageMoveLeft(uint8_t pixel)
{
    if (xStart == 0) {
        return;
    }

    xStart -= pixel;
    if (xStart > 80) {
        xStart = 0;
    }
    needRefresh = 1;
}

void ImageMoveRight(uint8_t pixel)
{
    if (xStart >= 80) {
        return;
    }

    xStart += pixel;
    if (xStart > 80) {
        xStart = 80;
    }
    needRefresh = 1;
}

void ProcessRender()
{
    if (moveStart != 1 && needRefresh == 0) {
        return;
    }
    static uint32_t lastTick = 0;
    uint32_t currentTick = HAL_GetTick();
    if (currentTick - lastTick < 17) {
        return;
    }
    lastTick = currentTick;

    static uint16_t color = 0x0564;

    //	const Rect rect = {{50, 10}, 30, 40};
    //	LcdDrawRect(&rect, BLACK_RGB565);
    //	LcdDrawBlock(100, 28, 30, 30, 0xF983);
    //	LcdDrawLine(0, 0, 3, WHITE_RGB565, LINE_DIR_VERTICAL);
    //	LcdDrawLine(10, 30, 25, WHITE_RGB565, LINE_DIR_HORIZONTAL);
    //	if (color % 2 == 0) {
    //		color += 33;
    //	} else {
    //		color += 55;
    //	}

    //	const Rect rect2 = {{10, 10}, 20, 20};
    //	LcdDrawRect(&rect2, BLACK_RGB565);

    //	TFT_ShowChar(0, 0, 'A', 0x7564, YELLOW, 16, true);
    //	TFT_ShowChar(0, 16, 'B', BLUE, YELLOW, 16, true);
    //	TFT_ShowChar(0, 32, 'C', BLUE, YELLOW, 16, true);
    //	TFT_ShowChar(0, 48, 'D', BLUE, YELLOW, 16, true);
    //	TFT_ShowChar(0, 64, 'E', BLUE, YELLOW, 16, true);
    //	TFT_ShowChar(0, 80, 'F', BLUE, YELLOW, 16, true);
    //	TFT_ShowChar(0, 96, 'G', BLUE, YELLOW, 16, true);
    //	TFT_ShowChar(0, 112, 'H', BLUE, YELLOW, 16, true);
    //	TFT_ShowChar(0, 128, 'I', BLUE, YELLOW, 16, true);
    //	TFT_ShowChar(0, 144, 'J', BLUE, YELLOW, 16, true);
    //
    LCD_ShowCharStr(40, 32, 80, "A", YELLOW, BLACK, 16);
    //	LCD_ShowCharNumber(48,64,128,20,YELLOW,BLACK,12);
    //	LCD_ShowCharNumber(60,64,128,22,YELLOW,BLACK,12);
    //
    //	TFT_ShowChar(64, 84, 'Y', BLUE, OLIVE, 32, true);
    //
    // 图像rgb565数组使用utils目录下"image2rgb565scale.py 图片文件 宽
    // 高"命令生成， 可以不指定宽或者高，脚本自动按照图片原始的宽高比计算新的值

    if (xStart > 0 && xStart <= 80) {
        LcdDrawBlock(0, 0, xStart, LCD_HEIGHT, color);
    }

    LcdDrawData(IMG_DATA, xStart, 0, IMG_WIDTH, IMG_HEIGHT);
    //	LcdDrawData((uint8_t*)IMG_DATA, xStart, 0, IMG_WIDTH, IMG_HEIGHT);

    if (xStart < 80) {
        LcdDrawBlock(xStart + IMG_WIDTH, 0, LCD_WIDTH - xStart - IMG_WIDTH,
                     LCD_HEIGHT, color);
    }

    if (needRefresh != 1) {
        xStart++;
        xStart %= 81;
    }
    needRefresh = 0;
    //	color += 100;

    //	LcdDrawRgb565(IMG_DATA, 60, 0, IMG_WIDTH, IMG_HEIGHT);
}
