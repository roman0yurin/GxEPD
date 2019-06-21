// class GxGDEM0154E97LT : Display class for GDEW0154Z17 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com
//
// based on Demo Example from Good Display, available here: http://www.good-display.com/download_detail/downloadsId=555.html
// Controller: IL0373 : http://www.good-display.com/download_detail/downloadsId=535.html
//
// Author : J-M Zingg
//
// Version : see library.properties
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// Library: https://github.com/ZinggJM/GxEPD

#include "GxGDEM0154E97LT.h"
#include "core/log/logger.h"

//#define DISABLE_DIAGNOSTIC_OUTPUT

GxGDEM0154E97LT::GxGDEM0154E97LT(GxIO& io, core::pin::GpioOutputRef rst, core::pin::GpioRef busy)
  : GxEPD(GxGDEM0154E97LT_WIDTH, GxGDEM0154E97LT_HEIGHT), IO(io),
    _current_page(-1), _using_partial_mode(false),
    _rst(rst), _busy(busy)
{
}

void GxGDEM0154E97LT::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEM0154E97LT_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEM0154E97LT_WIDTH - x - 1;
      y = GxGDEM0154E97LT_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEM0154E97LT_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEM0154E97LT_WIDTH / 8;
  if (_current_page < 1)
  {
    if (i >= sizeof(_black_buffer)) return;
  }
  else
  {
    y -= _current_page * GxGDEM0154E97LT_PAGE_HEIGHT;
    if ((y < 0) || (y >= GxGDEM0154E97LT_PAGE_HEIGHT)) return;
    i = x / 8 + y * GxGDEM0154E97LT_WIDTH / 8;
  }

  _black_buffer[i] = (_black_buffer[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
  if (color == GxEPD_WHITE) return;
  else if (color == GxEPD_BLACK) _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
  else
  {
    if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2)
    {
      _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
    }
  }
}


void GxGDEM0154E97LT::init()
{
  _wakeUp();
  fillScreen(GxEPD_BLACK);
  _current_page = -1;
  _using_partial_mode = false;
}

void GxGDEM0154E97LT::fillScreen(uint16_t color)
{
  uint8_t black = 0x00;
  if (color == GxEPD_WHITE);
  else if (color == GxEPD_BLACK) black = 0xFF;
  else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2) black = 0xFF;
  for (uint16_t x = 0; x < sizeof(_black_buffer); x++)
  {
    _black_buffer[x] = black;
  }
}

void GxGDEM0154E97LT::update(void)
{
	if (_current_page != -1) return;
	_using_partial_mode = false;
	_wakeUp();
	IO.writeCommandTransaction(0x10);
	IO.startTransaction(false);
	for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE; i++){
		IO.writeData((i < sizeof(_black_buffer)) ? ~_black_buffer[i] : 0xFF);
	}
	IO.endTransaction();
	IO.writeCommandTransaction(0x13);
	IO.writeCommandTransaction(0x12); //display refresh
	_waitWhileBusy("update");
	_sleep();
	}

void  GxGDEM0154E97LT::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  if (mode & bm_default) mode |= bm_invert;
  drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxGDEM0154E97LT::drawExamplePicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size)
{
  drawPicture(black_bitmap, red_bitmap, black_size, red_size, bm_invert_red);
}

void GxGDEM0154E97LT::drawPicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size, int16_t mode)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x10);
  IO.startTransaction(false);
  for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE; i++)
  {
    uint8_t data = 0xFF; // white is 0xFF on device
    if (i < black_size)
    {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
      data = pgm_read_byte(&black_bitmap[i]);
#else
      data = black_bitmap[i];
#endif
      if (mode & bm_invert) data = ~data;
    }
    IO.writeData(data);
  }
  IO.endTransaction();
  IO.writeCommandTransaction(0x13);
  IO.startTransaction(false);
  for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE; i++)
  {
    uint8_t data = 0xFF; // white is 0xFF on device
    if (i < red_size)
    {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
      data = pgm_read_byte(&red_bitmap[i]);
#else
      data = red_bitmap[i];
#endif
      if (mode & bm_invert_red) data = ~data;
    }
	IO.writeData(data);
  }
	IO.endTransaction();
	IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("drawPicture");
  _sleep();
}

void GxGDEM0154E97LT::drawBitmap(const uint8_t* bitmap, uint32_t size, int16_t mode)
{
  if (_current_page != -1) return;
  // example bitmaps are normal on b/w, but inverted on red
  if (mode & bm_default) mode |= bm_normal;
  if (mode & bm_partial_update)
  {
    _using_partial_mode = true;
    _wakeUp();
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(0, 0, GxGDEM0154E97LT_WIDTH - 1, GxGDEM0154E97LT_HEIGHT - 1);
	IO.writeCommandTransaction(0x10);
	IO.startTransaction(false);
    for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if (i < size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[i]);
#else
        data = bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      IO.writeData(data);
    }
    IO.endTransaction();
    IO.writeCommandTransaction(0x13);
	IO.startTransaction(false);
    for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE; i++)
    {
    	IO.writeData(0xFF); // white is 0xFF on device
    }
    IO.endTransaction();
	 IO.writeCommandTransaction(0x12); //display refresh
    _waitWhileBusy("drawBitmap");
    IO.writeCommandTransaction(0x92); // partial out
  }
  else
  {
    _using_partial_mode = false;
    _wakeUp();
	IO.writeCommandTransaction(0x10);
	IO.startTransaction(false);
    for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if (i < size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[i]);
#else
        data = bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      IO.writeData(data);
    }
    IO.endTransaction();
	IO.writeCommandTransaction(0x13);
	IO.startTransaction(false);
    for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE; i++)
    {
		IO.writeData(0xFF); // white is 0xFF on device
    }
	IO.endTransaction();
	IO.writeCommandTransaction(0x12); //display refresh
    _waitWhileBusy("drawBitmap");
    _sleep();
  }
}

void GxGDEM0154E97LT::eraseDisplay(bool using_partial_update)
{
  if (_current_page != -1) return;
  if (using_partial_update)
  {
    if (!_using_partial_mode) _wakeUp();
    _using_partial_mode = true; // remember
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(0, 0, GxGDEM0154E97LT_WIDTH - 1, GxGDEM0154E97LT_HEIGHT - 1);
	IO.writeCommandTransaction(0x10);
	IO.startTransaction(false);
    for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE * 2; i++)
    {
      IO.writeData(0xFF); // white is 0xFF on device
    }
    IO.endTransaction();
    IO.writeCommandTransaction(0x13);
    IO.startTransaction(false);
    for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE; i++)
    {
      IO.writeData(0xFF); // white is 0xFF on device
    }
    IO.endTransaction();
	IO.writeCommandTransaction(0x12); //display refresh
    _waitWhileBusy("eraseDisplay");
    IO.writeCommandTransaction(0x92); // partial out
  }
  else
  {
    _using_partial_mode = false; // remember
    _wakeUp();
    IO.writeCommandTransaction(0x10);
    IO.startTransaction(false);
    for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE * 2; i++)
    {
      IO.writeData(0xFF); // white is 0xFF on device
    }
  	IO.endTransaction();
    IO.writeCommandTransaction(0x13);
    IO.startTransaction(false);
    for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE; i++)
    {
      IO.writeData(0xFF); // white is 0xFF on device
    }
    IO.endTransaction();
	IO.writeCommandTransaction(0x12); //display refresh
    _waitWhileBusy("eraseDisplay");
    _sleep();
  }
}

void GxGDEM0154E97LT::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (_current_page != -1) return;
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(x, y);
        swap(w, h);
        x = GxGDEM0154E97LT_WIDTH - x - w - 1;
        break;
      case 2:
        x = GxGDEM0154E97LT_WIDTH - x - w - 1;
        y = GxGDEM0154E97LT_HEIGHT - y - h - 1;
        break;
      case 3:
        swap(x, y);
        swap(w, h);
        y = GxGDEM0154E97LT_HEIGHT - y  - h - 1;
        break;
    }
  }
  if (x >= GxGDEM0154E97LT_WIDTH) return;
  if (y >= GxGDEM0154E97LT_HEIGHT) return;
  // x &= 0xFFF8; // byte boundary, not here, use encompassing rectangle
  uint16_t xe = gx_uint16_min(GxGDEM0154E97LT_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GxGDEM0154E97LT_HEIGHT, y + h) - 1;
  // x &= 0xFFF8; // byte boundary, not needed here
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  IO.writeCommandTransaction(0x91); // partial in
  _setPartialRamArea(x, y, xe, ye);
  IO.writeCommandTransaction(0x10);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
    {
      uint16_t idx = y1 * (GxGDEM0154E97LT_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00; // white is 0x00 in buffer
      IO.writeDataTransaction(~data); // white is 0xFF on device
    }
  }
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("updateWindow");
  IO.writeCommandTransaction(0x92); // partial out
  delay(GxGDEM0154E97LT_PU_DELAY); // don't stress this display
}

void GxGDEM0154E97LT::updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation)
{
  if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  _writeToWindow(xs, ys, xd, yd, w, h, using_rotation);
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("updateToWindow");
  delay(GxGDEM0154E97LT_PU_DELAY); // don't stress this display
}

void GxGDEM0154E97LT::_writeToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        xs = GxGDEM0154E97LT_WIDTH - xs - w - 1;
        xd = GxGDEM0154E97LT_WIDTH - xd - w - 1;
        break;
      case 2:
        xs = GxGDEM0154E97LT_WIDTH - xs - w - 1;
        ys = GxGDEM0154E97LT_HEIGHT - ys - h - 1;
        xd = GxGDEM0154E97LT_WIDTH - xd - w - 1;
        yd = GxGDEM0154E97LT_HEIGHT - yd - h - 1;
        break;
      case 3:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        ys = GxGDEM0154E97LT_HEIGHT - ys  - h - 1;
        yd = GxGDEM0154E97LT_HEIGHT - yd  - h - 1;
        break;
    }
  }
  if (xs >= GxGDEM0154E97LT_WIDTH) return;
  if (ys >= GxGDEM0154E97LT_HEIGHT) return;
  if (xd >= GxGDEM0154E97LT_WIDTH) return;
  if (yd >= GxGDEM0154E97LT_HEIGHT) return;
  // the screen limits are the hard limits
  uint16_t xde = gx_uint16_min(GxGDEM0154E97LT_WIDTH, xd + w) - 1;
  uint16_t yde = gx_uint16_min(GxGDEM0154E97LT_HEIGHT, yd + h) - 1;
  IO.writeCommandTransaction(0x91); // partial in
  // soft limits, must send as many bytes as set by _SetRamArea
  uint16_t yse = ys + yde - yd;
  uint16_t xss_d8 = xs / 8;
  uint16_t xse_d8 = xss_d8 + _setPartialRamArea(xd, yd, xde, yde);
  IO.writeCommandTransaction(0x10);
  for (int16_t y1 = ys; y1 <= yse; y1++)
  {
    for (int16_t x1 = xss_d8; x1 < xse_d8; x1++)
    {
      uint16_t idx = y1 * (GxGDEM0154E97LT_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00; // white is 0x00 in buffer
      IO.writeDataTransaction(~data); // white is 0xFF on device
    }
  }
  IO.writeCommandTransaction(0x92); // partial out
}

void GxGDEM0154E97LT::powerDown()
{
  _using_partial_mode = false; // force _wakeUp()
  _sleep();
}

uint16_t GxGDEM0154E97LT::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
{
  x &= 0xFFF8; // byte boundary
  xe = (xe - 1) | 0x0007; // byte boundary - 1
  IO.writeCommandTransaction(0x90); // partial window
  //IO.writeDataTransaction(x / 256);
  IO.writeDataTransaction(x % 256);
  //IO.writeDataTransaction(xe / 256);
  IO.writeDataTransaction(xe % 256);
  IO.writeDataTransaction(y / 256);
  IO.writeDataTransaction(y % 256);
  IO.writeDataTransaction(ye / 256);
  IO.writeDataTransaction(ye % 256);
  IO.writeDataTransaction(0x01); // don't see any difference
  //IO.writeDataTransaction(0x00); // don't see any difference
  return (7 + xe - x) / 8; // number of bytes to transfer per line
}

void GxGDEM0154E97LT::_waitWhileBusy(const char* comment)
{
  unsigned long start = HAL_GetTick();
  while (true)
  {
    if (_busy.read())
    	break;

    delay(1);
    if (HAL_GetTick() - start > 20000000) // >14.9s !
    {
      if (USE_DEBUG) {
				unsigned long elapsed = HAL_GetTick() - start;
				printfWarning("Epaper busy timeout! Comment: %s, Elapsed: %dms", comment, elapsed);
			}
      break;
    }
  }
  (void) start;
}

void GxGDEM0154E97LT::_wakeUp()
{
	// reset required for wakeup
	IO.reset();

	IO.writeCommandTransaction(0x06); //boost soft start
	IO.writeDataTransaction (0x17); //A
	IO.writeDataTransaction (0x17); //B
	IO.writeDataTransaction (0x17); //C
	IO.writeCommandTransaction(0x04);
	_waitWhileBusy("_wakeUp Power On");
	IO.writeCommandTransaction(0x00); //panel setting
	IO.writeDataTransaction(0x0f);    //LUT from OTP, 160x296
	IO.writeDataTransaction(0x0d);    //VCOM to 0V fast
	IO.writeCommandTransaction(0x61); //resolution setting
	IO.writeDataTransaction (0x98); // HRES=152
	IO.writeDataTransaction (0x00); // VRES_byte1=0
	IO.writeDataTransaction (0x98); // VRES_byte2=152
	IO.writeCommandTransaction(0X50);     //VCOM AND DATA INTERVAL SETTING
	IO.writeDataTransaction(0x77);    //WBmode:VBDF 17|D7 VBDW 97 VBDB 57   WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
	}

void GxGDEM0154E97LT::_sleep(void)
{
	IO.writeCommandTransaction(0x02);      //power off
  _waitWhileBusy("_sleep Power Off");
	IO.writeCommandTransaction(0x07); // deep sleep
	IO.writeDataTransaction (0xa5);
}

void GxGDEM0154E97LT::drawPaged(void (*drawCallback)(void))
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
	IO.writeCommandTransaction(0x10);
  for (_current_page = 0; _current_page < GxGDEM0154E97LT_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    IO.startTransaction(false);
    for (int16_t y1 = 0; y1 < GxGDEM0154E97LT_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEM0154E97LT_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEM0154E97LT_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        IO.writeData(~data); // white is 0xFF on device
      }
    }
    IO.endTransaction();
  }
  _current_page = -1;
	IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEM0154E97LT::drawPaged(void (*drawCallback)(uint32_t), uint32_t p)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x10);
  for (_current_page = 0; _current_page < GxGDEM0154E97LT_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
	IO.startTransaction(false);
    for (int16_t y1 = 0; y1 < GxGDEM0154E97LT_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEM0154E97LT_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEM0154E97LT_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        IO.writeData(~data); // white is 0xFF on device
      }
    }
    IO.endTransaction();
  }
  _current_page = -1;
	IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEM0154E97LT::drawPaged(void (*drawCallback)(const void*), const void* p)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
	IO.writeCommandTransaction(0x10);
  for (_current_page = 0; _current_page < GxGDEM0154E97LT_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
	IO.startTransaction(false);
    for (int16_t y1 = 0; y1 < GxGDEM0154E97LT_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEM0154E97LT_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEM0154E97LT_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        IO.writeData(~data); // white is 0xFF on device
      }
    }
	IO.endTransaction();
  }
  _current_page = -1;
	IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEM0154E97LT::drawPaged(void (*drawCallback)(const void*, const void*), const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x10);
  IO.startTransaction(false);
  for (_current_page = 0; _current_page < GxGDEM0154E97LT_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEM0154E97LT_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEM0154E97LT_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEM0154E97LT_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        IO.writeData(~data); // white is 0xFF on device
      }
    }
  }
  IO.endTransaction();
  _current_page = -1;
  IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEM0154E97LT::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GxGDEM0154E97LT_WIDTH - x - w - 1;
      break;
    case 2:
      x = GxGDEM0154E97LT_WIDTH - x - w - 1;
      y = GxGDEM0154E97LT_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GxGDEM0154E97LT_HEIGHT - y - h - 1;
      break;
  }
}

void GxGDEM0154E97LT::drawPagedToWindow(void (*drawCallback)(void), uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEM0154E97LT_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEM0154E97LT_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEM0154E97LT_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback();
      uint16_t ys = yds % GxGDEM0154E97LT_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds, false);
    }
  }
  _current_page = -1;
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("drawPagedToWindow");
  delay(GxGDEM0154E97LT_PU_DELAY); // don't stress this display
}

void GxGDEM0154E97LT::drawPagedToWindow(void (*drawCallback)(uint32_t), uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEM0154E97LT_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEM0154E97LT_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEM0154E97LT_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p);
      uint16_t ys = yds % GxGDEM0154E97LT_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds, false);
    }
  }
  _current_page = -1;
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("drawPagedToWindow");
  delay(GxGDEM0154E97LT_PU_DELAY); // don't stress this display
}

void GxGDEM0154E97LT::drawPagedToWindow(void (*drawCallback)(const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEM0154E97LT_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEM0154E97LT_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEM0154E97LT_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p);
      uint16_t ys = yds % GxGDEM0154E97LT_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds, false);
    }
  }
  _current_page = -1;
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("drawPagedToWindow");
  delay(GxGDEM0154E97LT_PU_DELAY); // don't stress this display
}

void GxGDEM0154E97LT::drawPagedToWindow(void (*drawCallback)(const void*, const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEM0154E97LT_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEM0154E97LT_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEM0154E97LT_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p1, p2);
      uint16_t ys = yds % GxGDEM0154E97LT_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds, false);
    }
  }
  _current_page = -1;
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("drawPagedToWindow");
  delay(GxGDEM0154E97LT_PU_DELAY); // don't stress this display
}

void GxGDEM0154E97LT::drawCornerTest(uint8_t em)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x10);
  IO.startTransaction(false);
  for (uint32_t y = 0; y < GxGDEM0154E97LT_HEIGHT; y++)
  {
    for (uint32_t x = 0; x < GxGDEM0154E97LT_WIDTH / 8; x++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if ((x < 1) && (y < 8)) data = 0x00;
      if ((x > GxGDEM0154E97LT_WIDTH / 8 - 3) && (y < 16)) data = 0x00;
      if ((x > GxGDEM0154E97LT_WIDTH / 8 - 4) && (y > GxGDEM0154E97LT_HEIGHT - 25)) data = 0x00;
      if ((x < 4) && (y > GxGDEM0154E97LT_HEIGHT - 33)) data = 0x00;
      IO.writeData(data);
    }
  }
  IO.endTransaction();
  IO.writeCommandTransaction(0x13);
	IO.startTransaction(false);
  for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE; i++)
  {
	  IO.writeData(0xFF); // white is 0xFF on device
  }
	IO.endTransaction();
	IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("drawCornerTest");
  _sleep();
}


