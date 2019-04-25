#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "display.h"

uint8_t _width = ILI9341_TFTWIDTH;
uint8_t _height = ILI9341_TFTHEIGHT;


void drawPixel(int16_t x, int16_t y, uint16_t color) {

  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

  setAddrWindow(x, y, x + 1, y + 1);
  CD_DATA;
  write8special(color >> 8);
  write8special(color);
  CS_IDLE;

}

void Display_drawPixel(int16_t x, int16_t y, uint16_t color) {
	drawPixel( x, y, color);
}


void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
                                        uint16_t y1) {
  CS_ACTIVE;
  CD_COMMAND;
  write8special(ILI9341_CASET); // Column addr set
  CD_DATA;
  write8special(x0 >> 8);
  write8special(x0 & 0xFF);     // XSTART
  write8special(x1 >> 8);
  write8special(x1 & 0xFF);     // XEND

  CD_COMMAND;

  write8special(ILI9341_PASET); // Row addr set
  CD_DATA;
  write8special(y0 >> 8);
  write8special(y0);     // YSTART
  write8special(y1 >> 8);
  write8special(y1);     // YEND
  CD_COMMAND;
  write8special(ILI9341_RAMWR); // write to RAM

}

void Display_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
        uint16_t y1) {
	setAddrWindow( x0,  y0,  x1, y1);
}


void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                                   uint16_t color) {

  // rudimentary clipping (drawChar w/big text requires this)
  if ((x >= _width) || (y >= _height || h < 1 || w < 1)) {
	  return;
  }
  if ((x + w - 1) >= _width)  w = _width  - x;
  if ((y + h - 1) >= _height) h = _height - y;
  if (w == 1 && h == 1) {
    drawPixel(x, y, color);
    return;
  }
}

void Display_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	fillRect(x, y, w, h, color);
}


void fillScreen(uint16_t color) {
	fillRect(0, 0,  _width, _height, color);
}

void Display_fillScreen(uint16_t color) {
	fillScreen(color);
}
