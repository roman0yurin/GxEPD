// class GxGDEM0154E97LT : Display class for GDEM0154E97LT e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com
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

char vcom=0x52;	//-1.55V

static const uint8_t EPD_GATE_VOLTAGE    = 0x17;	//VGH=20V
static const uint8_t EPD_SRC_VOLTAGE_A   = 0x41;	//VSH1=15V
static const uint8_t EPD_SRC_VOLTAGE_B   = 0x0 ;        //VSH2=0V
static const uint8_t EPD_SRC_VOLTAGE_C   = 0x32;        //VSL=-15V
static const uint8_t EPD_DUMMY_PERIOD    = 0x15;
static const uint8_t EPD_GATE_PERIOD     = 0x0B;

//default update waveform LUT for GDEM0154E97LT
IMPL_USED const unsigned char HEAVY_FULL_UPDATE_LUT[]=
        {
                0x80,	0xA5,	0x10,	0x0,	0x0,	0x0,	0x0,
                0x10,	0xA5,	0x80,	0x0,	0x0,	0x0,	0x0,
                0x80,	0xA5,	0x10,	0x0,	0x0,	0x0,	0x0,
                0x10,	0xA5,	0x80,	0x0,	0x0,	0x0,	0x0,
                0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                0x6,	0x8,	0x0,	0x0,	0x2,
                0xC,	0x0,	0xC,	0x0,	0x5,
                0x8,	0x6,	0x0,	0x0,	0x2,
                0x0,	0x0,	0x0,	0x0,	0x0,
                0x0,	0x0,	0x0,	0x0,	0x0,
                0x0,	0x0,	0x0,	0x0,	0x0,
                0x0,	0x0,	0x0,	0x0,	0x0
        };


const unsigned char FULL_UPDATE_LUT[]=
        {
                0x80,	0xA5,	0x10,	0x0,	0x0,	0x0,	0x0,
                0x10,	0xA5,	0x80,	0x0,	0x0,	0x0,	0x0,
                0x80,	0xA5,	0x10,	0x0,	0x0,	0x0,	0x0,
                0x10,	0xA5,	0x80,	0x0,	0x0,	0x0,	0x0,
                0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                0x6,	0x8,	0x0,	0x0,	0x0,
                0xC,	0x0,	0xC,	0x0,	0x1,
                0x8,	0x6,	0x0,	0x0,	0x3,
                0x0,	0x0,	0x0,	0x0,	0x0,
                0x0,	0x0,	0x0,	0x0,	0x0,
                0x0,	0x0,	0x0,	0x0,	0x0,
                0x0,	0x0,	0x0,	0x0,	0x0
        };

IMPL_USED const unsigned char PARTIAL_LUT[]=
        {
                0x80,	0xA5,	0x10,	0x0,	0x0,	0x0,	0x0,
                0x10,	0xA5,	0x80,	0x0,	0x0,	0x0,	0x0,
                0x80,	0xA5,	0x10,	0x0,	0x0,	0x0,	0x0,
                0x10,	0xA5,	0x80,	0x0,	0x0,	0x0,	0x0,
                0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                0x0,	0x0,	0x0,	0x0,	0x0,
                0x0,	0x0,	0x0,	0x0,	0x0,
                0xA,	0x6,	0x0,	0x0,	0x5,
                0x0,	0x0,	0x0,	0x0,	0x0,
                0x0,	0x0,	0x0,	0x0,	0x0,
                0x0,	0x0,	0x0,	0x0,	0x0,
                0x0,	0x0,	0x0,	0x0,	0x0
        };


GxGDEM0154E97LT::GxGDEM0154E97LT(GxIO& io, core::pin::GpioOutputRef rst, core::pin::GpioRef busy)
  : GxEPD(GxGDEM0154E97LT_WIDTH, GxGDEM0154E97LT_HEIGHT), IO(io),
    _current_page(-1),
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

void GxGDEM0154E97LT::Set_Counter(uint16_t x, uint16_t y){
    //x is addressed in 19 bytes, so just use the closest byte
    //maybe do something later to skip the extra bits instead
    if(x>151){
        x=151;
    }
    if(y>151){
        y=151;
    }
    x=x/8;

    uint8_t y1=y&0xFF;
    uint8_t y2=(y>>8)&0xFF;

    IO.writeCommandTransaction(CMD_X_CNT);
    IO.writeDataTransaction(x);
    IO.writeCommandTransaction(CMD_Y_CNT);
    IO.writeDataTransaction(y1);
    IO.writeDataTransaction(y2);
}


void GxGDEM0154E97LT::setWriteWindow(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2){
    //x is addressed in 19 bytes, so round down to the closest byte
    //maybe do something later to mask the extra bits as well
    uint8_t x_strt=x1/8;
    uint8_t x_end=x2/8;

    uint8_t y_strt1=y1&0xFF;
    uint8_t y_strt2=(y1>>8)&0xFF;
    uint8_t y_end1=y2&0xFF;
    uint8_t y_end2=(y2>>8)&0xFF;

    IO.writeCommandTransaction(CMD_X_POS);
    IO.writeDataTransaction(x_strt);
    IO.writeDataTransaction(x_end);

    IO.writeCommandTransaction(CMD_Y_POS);
    IO.writeDataTransaction(y_strt1);
    IO.writeDataTransaction(y_strt2);
    IO.writeDataTransaction(y_end1);
    IO.writeDataTransaction(y_end2);
    Set_Counter(x1,y1);

}




void GxGDEM0154E97LT::init()
{
    IO.reset();
   _sleep(); //до вызова wakeup можно больше не тратить энергию
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
	if (_current_page != -1)
	    return;
	_wakeUp();
	IO.writeCommandTransaction(CMD_WRITE_RAM);
	IO.startTransaction(false);
	for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE; i++){
		IO.writeData((i < sizeof(_black_buffer)) ? ~_black_buffer[i] : 0xFF);
	}
    //enable display update bypass option
	IO.endTransaction();
	IO.writeCommandTransaction(CMD_UPDATE_CTRL_1);
	IO.writeDataTransaction(0x40);
    //enable clock signal, analog, LUT, pattern display, disable CP, OSC
    IO.writeCommandTransaction(CMD_UPDATE_CTRL_2);
    IO.writeDataTransaction(0xC7);

    IO.writeCommandTransaction(CMD_UPDATE); //display refresh
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
  _wakeUp();
  IO.writeCommandTransaction(CMD_WRITE_RAM);
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
  IO.writeCommandTransaction(CMD_UPDATE); //display refresh
  _waitWhileBusy("draw");
  _sleep();
}

void GxGDEM0154E97LT::drawBitmap(const uint8_t* bitmap, uint32_t size, int16_t mode)
{
  if (_current_page != -1) return;
  // example bitmaps are normal on b/w, but inverted on red
  if (mode & bm_default) mode |= bm_normal;
  if (mode & bm_partial_update)
  {
    UNSUPPORTED;
  }
  else
  {
    _wakeUp();
	IO.writeCommandTransaction(CMD_WRITE_RAM);
	IO.startTransaction(false);
    for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if (size == 0 || i < size)
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
    IO.writeCommandTransaction(CMD_UPDATE);
    _waitWhileBusy("draw");
    _sleep();
  }
}

void GxGDEM0154E97LT::eraseDisplay(bool using_partial_update)
{
  if (_current_page != -1) return;
  if (using_partial_update)
  {
    UNSUPPORTED;
  }
  else
  {
    _wakeUp();
    IO.writeCommandTransaction(CMD_WRITE_RAM);
    IO.startTransaction(false);
    for (uint32_t i = 0; i < GxGDEM0154E97LT_BUFFER_SIZE * 2; i++)
    {
      IO.writeData(0xFF); // white is 0xFF on device
    }
  	IO.endTransaction();
	IO.writeCommandTransaction(CMD_UPDATE); //display refresh
    _waitWhileBusy("eraseDisplay");
    _sleep();
  }
}

void GxGDEM0154E97LT::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
    UNSUPPORTED;//нет partialUpdate
}

void GxGDEM0154E97LT::updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation)
{
   UNSUPPORTED;//нет partialUpdate
}

void GxGDEM0154E97LT::_writeToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation){
  UNSUPPORTED;//нет partialUpdate
}

void GxGDEM0154E97LT::powerDown()
{
  _sleep();
}

void GxGDEM0154E97LT::_waitWhileBusy(const char* comment)
{
  unsigned long start = HAL_GetTick();
  while (true)
  {
    if (!_busy.read())
    	break;

    delay(1);
    if (HAL_GetTick() - start > 10000) // >14.9s !
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

boolean inSleep = false;

void GxGDEM0154E97LT::_wakeUp()
{
    _waitWhileBusy("init");
    IO.writeCommandTransaction(0x1);
    IO.writeCommandTransaction(CMD_RESET);
    _waitWhileBusy("init");
    //undocumented commands from the STM32 driver, just using the recommended settings
    IO.writeCommandTransaction(CMD_ANALOG_CTRL); //set analog block control
    IO.writeDataTransaction(0x54);
    IO.writeCommandTransaction(CMD_DIGITAL_CTRL); //set digital block control
    IO.writeDataTransaction(0x3B);

    IO.writeCommandTransaction(CMD_DRIVER_OUTPUT);
    IO.writeDataTransaction(0x97); //152 MUX gate lines
    IO.writeDataTransaction(0x00); //152 MUX gate lines
    IO.writeDataTransaction(0x00); //gate direction=0, scan mode=0, scan direction=0

    IO.writeCommandTransaction(CMD_TMP_CTRL);
    IO.writeDataTransaction(0x80); //use internal temperature sensor

    IO.writeCommandTransaction(CMD_DATA_ENTRY_MODE);
    IO.writeDataTransaction(0x03);   //address counter auto-increments from top left to bottom right

    //////////////////////////////////////////
    //todo:make initial window based on device size
    //////////////////////////////////////////
    setWriteWindow(0, 151, 0, 151);

    IO.writeCommandTransaction(CMD_BORDER_CTRL);
    IO.writeDataTransaction(0x01);	//use GS transition LH for VBD

    LUT_Written_by_MCU();
}

void GxGDEM0154E97LT::_sleep(void)
{
    IO.writeCommandTransaction(CMD_SLEEP);
    IO.writeDataTransaction(0x00);
    delay(2);
    inSleep = true;
}

void GxGDEM0154E97LT::drawPaged(void (*drawCallback)(void))
{
  if (_current_page != -1) return;
  _wakeUp();
	IO.writeCommandTransaction(CMD_WRITE_RAM);
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
	IO.writeCommandTransaction(CMD_UPDATE); //display refresh
  _waitWhileBusy("draw");
  _sleep();
}

void GxGDEM0154E97LT::drawPaged(void (*drawCallback)(uint32_t), uint32_t p)
{
  if (_current_page != -1) return;
  _wakeUp();
  IO.writeCommandTransaction(CMD_WRITE_RAM);
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
	IO.writeCommandTransaction(CMD_UPDATE); //display refresh
  _waitWhileBusy("draw");
  _sleep();
}

void GxGDEM0154E97LT::drawPaged(void (*drawCallback)(const void*), const void* p)
{
  if (_current_page != -1) return;
  _wakeUp();
	IO.writeCommandTransaction(CMD_WRITE_RAM);
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
	IO.writeCommandTransaction(CMD_UPDATE); //display refresh
  _waitWhileBusy("draw");
  _sleep();
}

void GxGDEM0154E97LT::drawPaged(void (*drawCallback)(const void*, const void*), const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _wakeUp();
  IO.writeCommandTransaction(CMD_WRITE_RAM);
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
  IO.writeCommandTransaction(CMD_UPDATE); //display refresh
  _waitWhileBusy("draw");
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

void GxGDEM0154E97LT::drawCornerTest(uint8_t em)
{
  if (_current_page != -1) return;
  _wakeUp();
  IO.writeCommandTransaction(CMD_WRITE_RAM);
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
  update();
  _sleep();
}


/*
* Function name: Epaper_LUT
 * Description : Loading waveforms for SSD1675
 * Enter :wave_data - device-specific driving waveform
 * Output : None
 */
void GxGDEM0154E97LT::Epaper_LUT(uint8_t *wave_data)
{
    uint8_t count;
    int LUTSize=sizeof(HEAVY_FULL_UPDATE_LUT)/sizeof(HEAVY_FULL_UPDATE_LUT[0]);

    IO.writeCommandTransaction(CMD_WRITE_LUT);
    IO.startTransaction(false);
    for(count=0;count<LUTSize;count++)
        IO.writeData(*wave_data++);
    IO.endTransaction();
    _waitWhileBusy();

}

void GxGDEM0154E97LT::LUT_Written_by_MCU(void)
{

    IO.writeCommandTransaction(CMD_WRITE_VCOM);
    IO.writeDataTransaction( vcom); //manually set VCOM to -1.55V, should change this to use VCOM sense

    IO.writeCommandTransaction(CMD_GATE_V);
    IO.writeDataTransaction(EPD_GATE_VOLTAGE);

    IO.writeCommandTransaction(CMD_SRC_V);
    IO.writeDataTransaction(EPD_SRC_VOLTAGE_A);
    IO.writeDataTransaction(EPD_SRC_VOLTAGE_B);
    IO.writeDataTransaction(EPD_SRC_VOLTAGE_C);

    IO.writeCommandTransaction(CMD_DMY_LINE_T);
    IO.writeDataTransaction(EPD_DUMMY_PERIOD);
    IO.writeCommandTransaction(CMD_GATE_LINE_T);
    IO.writeDataTransaction(EPD_GATE_PERIOD);

    if(partialUpdate)
        Epaper_LUT((uint8_t *)PARTIAL_LUT);
    else
        Epaper_LUT((uint8_t *)HEAVY_FULL_UPDATE_LUT);

}

void GxGDEM0154E97LT::updateFromRAM() {
    _wakeUp();
    IO.writeCommandTransaction(CMD_UPDATE); //display refresh
    _waitWhileBusy("draw");
    _sleep();
}


