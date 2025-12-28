#include "st7735s.h"
#include "font.h"
#include "spi.h"
#include "st7735s_config.h"
#include "usart.h"
#include <stdio.h>
#include <stdlib.h>
#include <stm32f1xx_hal.h>
#include <string.h>

#define SPI_DMA_BUFFER_SIZE 1024

#define LCD_BEGIN()                                                            \
    HAL_GPIO_WritePin(spi_cs_GPIO_Port, spi_cs_Pin, GPIO_PIN_RESET)
#define LCD_END() HAL_GPIO_WritePin(spi_cs_GPIO_Port, spi_cs_Pin, GPIO_PIN_SET)

#define LCD_CMD()                                                              \
    HAL_GPIO_WritePin(spi_dc_GPIO_Port, spi_dc_Pin, GPIO_PIN_RESET)
#define LCD_DATA() HAL_GPIO_WritePin(spi_dc_GPIO_Port, spi_dc_Pin, GPIO_PIN_SET)

#define CLR_LCD_RES                                                            \
    HAL_GPIO_WritePin(spi_res_GPIO_Port, spi_res_Pin, GPIO_PIN_RESET)
#define SET_LCD_RES                                                            \
    HAL_GPIO_WritePin(spi_res_GPIO_Port, spi_res_Pin, GPIO_PIN_SET)

#define SPI_LOCK()                                                             \
    while (spiTxComplete == 0)                                                 \
        ;                                                                      \
    spiTxComplete = 0;
#define SPI_UNLOCK() spiTxComplete = 1;

static uint8_t spiTxComplete = 1;
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi2) {
        LCD_END();
        spiTxComplete = 1;
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi2) {
        LCD_END();
        spiTxComplete = 1;
        UartPrintf("spi2 dma error");
    }
}

// 写命令 dcx = 0
void LcdWriteCmd(uint8_t data)
{
    //	SPI_LOCK();
    //	LCD_BEGIN();
    LCD_CMD();

    HAL_SPI_Transmit(&hspi2, &data, sizeof(uint8_t), HAL_MAX_DELAY);

    //	LCD_END();
    //	SPI_UNLOCK();
}

// 写数据 dcx = 1
void LcdWriteData(uint8_t data)
{
    //	SPI_LOCK();
    //	LCD_BEGIN();
    LCD_DATA();

    //	HAL_SPI_Transmit_DMA(&hspi2, &data, 1);
    HAL_SPI_Transmit(&hspi2, &data, sizeof(uint8_t), HAL_MAX_DELAY);

    //	LCD_END();
    //	SPI_UNLOCK();
}

void LcdWriteDataSPI(uint8_t *data, uint16_t dataSize)
{
    //	SPI_LOCK();
    //	LCD_BEGIN();
    LCD_DATA();

    //	HAL_SPI_Transmit_DMA(&hspi2, &data, 1);
    HAL_SPI_Transmit(&hspi2, data, dataSize, HAL_MAX_DELAY);

    //	LCD_END();
    //	SPI_UNLOCK();
}

void LcdWriteDataU16(uint16_t data)
{
    //	SPI_LOCK();
    //	LcdWriteData(data >> 8);
    //	LcdWriteData(data & 0xFF);
    uint8_t tmp[2];
    tmp[0] = data >> 8;
    tmp[1] = data & 0xFF;
    //	LCD_BEGIN();
    LCD_DATA();
    //	HAL_SPI_Transmit_DMA(&hspi2, tmp, 2);
    HAL_SPI_Transmit(&hspi2, tmp, sizeof(tmp), HAL_MAX_DELAY);
    //	LCD_END();
    //	SPI_UNLOCK();
}

void LcdWriteDataArray(const uint8_t *array, uint16_t arrayLen)
{
    //	SPI_LOCK();
    //	LCD_BEGIN();
    LCD_DATA();
    //	if (HAL_SPI_Transmit_DMA(&hspi2, array, arrayLen) != HAL_OK) {
    //		static char *message = "LcdWriteDataArray failed!";
    //		send_data_safely(message, strlen(message));
    //	}

    HAL_SPI_Transmit(&hspi2, array, arrayLen, HAL_MAX_DELAY);
    //	LCD_END();
}

// res低电平>5us,这里使用1ms，最长复位时间120ms
void LcdReset()
{
    SET_LCD_RES;
    CLR_LCD_RES;
    HAL_Delay(100);
    SET_LCD_RES;
    HAL_Delay(120);
}

void LcdSetParam()
{
    LCD_BEGIN();
    LcdWriteCmd(0x11); // 退出睡眠
    HAL_Delay(120);

    //	LcdWriteCmd(0x21);
    LcdWriteCmd(0x21);

    LcdWriteCmd(0xB1);
    LcdWriteData(0x05);
    LcdWriteData(0x3A);
    LcdWriteData(0x3A);

    LcdWriteCmd(0xB2);
    LcdWriteData(0x05);
    LcdWriteData(0x3A);
    LcdWriteData(0x3A);

    LcdWriteCmd(0xB3);
    LcdWriteData(0x05);
    LcdWriteData(0x3A);
    LcdWriteData(0x3A);
    LcdWriteData(0x05);
    LcdWriteData(0x3A);
    LcdWriteData(0x3A);

    LcdWriteCmd(0xB4);
    LcdWriteData(0x03);

    LcdWriteCmd(0xC0);
    LcdWriteData(0x62);
    LcdWriteData(0x02);
    LcdWriteData(0x04);

    LcdWriteCmd(0xC1);
    LcdWriteData(0xC0);

    LcdWriteCmd(0xC2);
    LcdWriteData(0x0D);
    LcdWriteData(0x00);

    LcdWriteCmd(0xC3);
    LcdWriteData(0x8D);
    LcdWriteData(0x6A);

    LcdWriteCmd(0xC4);
    LcdWriteData(0x8D);
    LcdWriteData(0xEE);

    LcdWriteCmd(0xC5); /*VCOM*/
    LcdWriteData(0x0E);

    LcdWriteCmd(0xE0);
    LcdWriteData(0x10);
    LcdWriteData(0x0E);
    LcdWriteData(0x02);
    LcdWriteData(0x03);
    LcdWriteData(0x0E);
    LcdWriteData(0x07);
    LcdWriteData(0x02);
    LcdWriteData(0x07);
    LcdWriteData(0x0A);
    LcdWriteData(0x12);
    LcdWriteData(0x27);
    LcdWriteData(0x37);
    LcdWriteData(0x00);
    LcdWriteData(0x0D);
    LcdWriteData(0x0E);
    LcdWriteData(0x10);

    LcdWriteCmd(0xE1);
    LcdWriteData(0x10);
    LcdWriteData(0x0E);
    LcdWriteData(0x03);
    LcdWriteData(0x03);
    LcdWriteData(0x0F);
    LcdWriteData(0x06);
    LcdWriteData(0x02);
    LcdWriteData(0x08);
    LcdWriteData(0x0A);
    LcdWriteData(0x13);
    LcdWriteData(0x26);
    LcdWriteData(0x36);
    LcdWriteData(0x00);
    LcdWriteData(0x0D);
    LcdWriteData(0x0E);
    LcdWriteData(0x10);

    LcdWriteCmd(0x3A);
    LcdWriteData(0x05);

    LcdWriteCmd(0x36);
    //	LcdWriteData(0xA8); //
    //	LcdWriteData(0xC8); //
    LcdWriteData(0x60);
    //	LcdWriteData(0xF0);

    LcdWriteCmd(0x29);
    LCD_END();
}

void LcdInit()
{
    LcdReset();
    LcdSetParam();
}

// 设置显示区域
void Lcd_SetRegion(uint16_t xStart, uint16_t yStart, uint16_t xEnd,
                   uint16_t yEnd)
{
    // 列地址设置
    LcdWriteCmd(0x2A);
    //	LcdWriteData(xStart >> 8);
    //	LcdWriteData(xStart + 1);
    //	LcdWriteData(xEnd >> 8);
    //	LcdWriteData(xEnd + 1);
    uint8_t xRegion[4] = {xStart >> 8, xStart + 1, xEnd >> 8, xEnd + 1};
    LcdWriteDataArray(xRegion, 4);

    // 行地址设置
    LcdWriteCmd(0x2B);
    //	LcdWriteData(yStart >> 8);
    //	LcdWriteData(yStart + 0x1A);
    //	LcdWriteData(yEnd >> 8);
    //	LcdWriteData(yEnd + 0x1A);
    uint8_t yRegion[4] = {yStart >> 8, yStart + 0x1A, yEnd >> 8, yEnd + 0x1A};
    LcdWriteDataArray(yRegion, 4);

    LcdWriteCmd(0x2C); // 写入显存
}

/**
 * startX: 矩形左上角起点X坐标
 * startY: 矩形左上角起点Y坐标
 * width: 矩形宽度
 * height: 矩形高度
 */
void LcdDrawBlock(uint8_t xStart, uint8_t yStart, uint8_t width, uint8_t height,
                  uint16_t color)
{
    uint8_t xEnd = xStart + width - 1;
    uint8_t yEnd = yStart + height - 1;

    uint16_t pxColor[width * height];
    for (uint16_t i = 0; i < width * height; ++i) {
        pxColor[i] = color;
    }

    LCD_BEGIN();
    Lcd_SetRegion(xStart, yStart, xEnd, yEnd);
    LcdWriteDataSPI((uint8_t *)pxColor, width * height * 2);
    LCD_END();
}

void LcdClear(uint16_t color)
{
    uint16_t data[LCD_WIDTH] = {0};
    for (int i = 0; i < LCD_WIDTH; ++i) {
        data[i] = color;
    }
    LCD_BEGIN();
    for (int i = 0; i < LCD_HEIGHT; i++) {
        LcdWriteDataSPI((uint8_t *)data, LCD_WIDTH * 2);
    }
    LCD_END();
}

void LcdDrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    Lcd_SetRegion(x, y, x, y);
    LcdWriteDataU16(color);
}

/**
 * startX: 线条起点X坐标
 * endX: 线条起点Y坐标
 * len: 线条像素长度
 */
void LcdDrawLine(uint16_t startX, uint16_t startY, uint16_t length,
                 uint16_t color, uint8_t direction)
{
    if (direction == LINE_DIR_HORIZONTAL) {
        LcdDrawBlock(startX, startY, length, 1, color);
    } else if (direction == LINE_DIR_VERTICAL) {
        LcdDrawBlock(startX, startY, 1, length, color);
    }
}

void LcdDrawRect(const Rect *rect, uint16_t color)
{
    LCD_BEGIN();
    LcdDrawLine(rect->leftUp.x, rect->leftUp.y, rect->width, color,
                LINE_DIR_HORIZONTAL);
    LcdDrawLine(rect->leftUp.x, rect->leftUp.y + rect->height - 1, rect->width,
                color, LINE_DIR_HORIZONTAL);

    LcdDrawLine(rect->leftUp.x, rect->leftUp.y, rect->height, color,
                LINE_DIR_VERTICAL);
    LcdDrawLine(rect->leftUp.x + rect->width - 1, rect->leftUp.y, rect->height,
                color, LINE_DIR_VERTICAL);
    LCD_END();
}

void LcdDrawCircle(const Point *center, uint16_t radius, uint16_t color)
{
    UNUSED(center);
    UNUSED(radius);
    UNUSED(color);
}

// Display ch (char type) in (x, y) with back_color, front_color and front_size.
// font.h is essential.
void TFT_ShowChar(uint16_t x, uint16_t y, char ch, uint16_t back_color,
                  uint16_t font_color, uint8_t font_size, bool setLcdCS)
{
    int i = 0, j = 0;
    unsigned char temp = 0;
    uint8_t size = 0;
    //	uint8_t t = 0;
    // uint8_t buffer[512] = {0};
    // uint8_t *tmp = buffer;
    uint16_t buffer[512] = {0};
    uint16_t *tmp = buffer;

    if ((x > (LCD_WIDTH - font_size / 2)) || (y > (LCD_HEIGHT - font_size)))
        return;

    if (setLcdCS) {
        LCD_BEGIN();
    }
    Lcd_SetRegion(x, y, x + font_size / 2 - 1, y + font_size - 1);

    int16_t index = ch - ' ';

    if ((font_size == 16) || (font_size == 32)) {
        size = (font_size / 8 + ((font_size % 8) ? 1 : 0)) * (font_size / 2);

        for (i = 0; i < size; i++) {
            if (font_size == 16) {
                temp = asc2_1608[index][i];
            } else if (font_size == 32) {
                //				temp = asc2_3216[index][i];
            } else {
                if (setLcdCS) {
                    LCD_END();
                }
                return;
            }

            for (j = 0; j < 8; j++) {
                if (temp & 0x80) {
                    LcdWriteDataU16(font_color);
                    // tmp[0] = font_color << 8;
                    // tmp[1] = font_color & 0xFF;
                    *tmp = font_color;
                } else {
                    LcdWriteDataU16(back_color);
                    // tmp[0] = back_color << 8;
                    // tmp[1] = back_color & 0xFF;
                    *tmp = back_color;
                }
                temp <<= 1;
                // tmp += 2;
                tmp++;
            }
        }
    } else if (font_size == 12) {
        size = (font_size / 8 + ((font_size % 8) ? 1 : 0)) * (font_size / 2);

        for (i = 0; i < size; i++) {
            temp = asc2_1206[index][i];

            for (j = 0; j < 6; j++) {
                if (temp & 0x80) {
                    LcdWriteDataU16(font_color);
                } else {
                    LcdWriteDataU16(back_color);
                }

                temp <<= 1;
            }
        }
    } else if (font_size == 24) {
        //		size = (font_size * 16) / 8;
        //
        //		for (i = 0; i < size; i++) {
        //			temp = asc2_2412[index][i];
        //			if (i % 2 == 0)
        //				t = 8;
        //			else
        //				t = 4;
        //			for (j = 0; j < t; j++) {
        //				if (temp & 0x80) {
        //					LcdWriteDataU16(font_color);
        //				} else {
        //					LcdWriteDataU16(back_color);
        //				}
        //
        //				temp <<= 1;
        //			}
        //		}
    } else {
        if (setLcdCS) {
            LCD_END();
        }
        return;
    }
    if (setLcdCS) {
        LCD_END();
    }
}

// Display str (string type) in (x, y) with back_color, front_color and
// front_size under max_width. font.h is essential.
void LCD_ShowCharStr(uint16_t x, uint16_t y, uint32_t max_width, char *str,
                     uint16_t back_color, uint16_t font_color,
                     uint8_t font_size)
{

    max_width += x;

    LCD_BEGIN();
    while ((*str <= '~') && (*str >= ' ')) {
        if (x >= max_width) {
            break;
        }

        TFT_ShowChar(x, y, *str, back_color, font_color, font_size, false);
        x += font_size / 2;
        str++;
    }
    LCD_END();
}

// Display number (byte type) in (x, y) with back_color, front_color and
// front_size under max_width. font.h is essential.
void LCD_ShowCharNumber(uint16_t x, uint16_t y, uint32_t max_width,
                        uint8_t number, uint16_t back_color,
                        uint16_t font_color, uint8_t font_size)
{
    char number_ascii[10];

    sprintf(number_ascii, "%d", number);

    LCD_ShowCharStr(x, y, max_width, number_ascii, back_color, font_color,
                    font_size);
}

/*************************************************
函数名：f_Lcd_ShowBmp_ColorValue
功能：指定位置显示图片
入口参数：
    pbmp 	: 图片数组指针(像素点字节颜色值，高字节在前)
    x0	 	: 图片显示起点x坐标
    y0	 	: 图片显示起点y坐标
    x_Len	: 图片宽度
    y_Len	: 图片高度
返回值：无
*************************************************/
void LcdShowBmp(const uint8_t *pbmp, uint8_t x0, uint8_t y0, uint8_t x_Len,
                uint8_t y_Len)
{
    uint8_t x1 = x0 + x_Len - 1;
    uint8_t y1 = y0 + y_Len - 1;
    uint16_t xy = (x_Len * y_Len) << 1;
    uint16_t i = 0;

    LCD_BEGIN();
    Lcd_SetRegion(x0, y0, x1, y1);
    LcdWriteCmd(0x2C);

    for (i = 0; (i << 1) < xy; i++)
        LcdWriteDataU16((pbmp[i << 1] << 8) + pbmp[(i << 1) + 1]);
    LCD_END();
}

void LcdDrawRgb565(const uint16_t *rgb565, uint16_t xStart, uint16_t yStart,
                   uint16_t width, uint16_t height)
{
    uint16_t xEnd = xStart + width - 1;
    uint16_t yEnd = yStart + height - 1;
    LCD_BEGIN();
    Lcd_SetRegion(xStart, yStart, xEnd, yEnd);
    for (uint16_t i = 0; i < width * height; i++) {
        LcdWriteDataU16(rgb565[i]);
    }
    LCD_END();
}

void LcdDrawData(const uint8_t *rgb565, uint16_t xStart, uint16_t yStart,
                 uint16_t width, uint16_t height)
{
    uint16_t xEnd = xStart + width - 1;
    uint16_t yEnd = yStart + height - 1;
    LCD_BEGIN();
    Lcd_SetRegion(xStart, yStart, xEnd, yEnd);

    LcdWriteDataArray(rgb565, width * height * 2);
    LCD_END();
}
