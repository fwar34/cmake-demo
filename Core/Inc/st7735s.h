/*
 * st7735s.h 0.96寸lcd屏幕
 *
 *  Created on: Mar 12, 2025
 *      Author: fwar3
 */

#ifndef INC_ST7735S_H_
#define INC_ST7735S_H_

#include <inttypes.h>
#include <stdbool.h>

#define LCD_WIDTH 160
#define LCD_HEIGHT 80

typedef enum {
	LINE_DIR_HORIZONTAL, LINE_DIR_VERTICAL, LINE_DIR_OBLIQUE,
} LineDir;

typedef struct {
	uint16_t x;
	uint16_t y;
} Point;

typedef struct {
	Point leftUp;
//	Point rightDown;
	uint16_t width;
	uint16_t height;
} Rect;

#define BLACK_RGB565 0x0000
#define WHITE_RGB565 0xFFFF
//LCD colors used as inversion of RGB565
#define WHITE     ~0xFFFF
#define BLACK     ~0x0000
#define RED       ~0xF800
#define GREEN     ~0x07E0
#define BLUE      ~0x001F
#define YELLOW    (uint16_t)~0xFFE0
#define PURPLE    (uint16_t)~0xF81F
#define MAGENTA   ~0xF81F
#define NAVY      ~0x000F
#define DGREEN    ~0x03E0
#define DCYAN     ~0x03EF
#define MAROON    ~0x7800
#define OLIVE     ~0x7BE0
#define LGRAY     ~0xC618
#define DGRAY     ~0x7BEF
#define CYAN      ~0x07FF

void LcdInit();
void LcdReset();
void LcdClear(uint16_t color);
void LcdDrawBlock(uint8_t xStart, uint8_t yStart, uint8_t width, uint8_t height,
		uint16_t color);
void LcdDrawPoint(uint16_t x, uint16_t y, uint16_t color);
void LcdDrawLine(uint16_t startX, uint16_t startY, uint16_t len, uint16_t color,
		uint8_t direction);
void LcdDrawRect(const Rect *rect, uint16_t color);
void LcdDrawCircle(const Point *center, uint16_t radius, uint16_t color);
//void LcdShowChar(uint16_t x, uint16_t y, char c, uint16_t frontColor, uint16_t backColor, uint8_t fontSize);
void TFT_ShowChar(uint16_t x, uint16_t y, char ch, uint16_t back_color,
		uint16_t font_color, uint8_t font_size, bool setLcdCS);
void LCD_ShowCharStr(uint16_t x, uint16_t y, uint32_t max_width, char *str,
		uint16_t back_color, uint16_t font_color, uint8_t font_size);
void LCD_ShowCharNumber(uint16_t x, uint16_t y, uint32_t max_width,
		uint8_t number, uint16_t back_color, uint16_t font_color,
		uint8_t font_size);
void LcdShowBmp(const uint8_t *pbmp, uint8_t x0, uint8_t y0, uint8_t x_Len,
		uint8_t y_Len);
void LcdDrawRgb565(const uint16_t *rgb565, uint16_t xStart, uint16_t yStart, uint16_t width, uint16_t height);
void LcdDrawData(const uint8_t *rgb565, uint16_t xStart, uint16_t yStart, uint16_t width, uint16_t height);

#endif /* INC_ST7735S_H_ */
