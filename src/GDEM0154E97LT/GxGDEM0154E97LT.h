// class GxGDEW0154Z17 : Display class for GDEW0154Z17 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com
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

#ifndef _GxGDEW0154Z17_H_
#define _GxGDEW0154Z17_H_

#include "../GxEPD.h"

#define GxGDEM0154E97LT_WIDTH 152
#define GxGDEM0154E97LT_HEIGHT 152

#define GxGDEM0154E97LT_BUFFER_SIZE (uint32_t(GxGDEM0154E97LT_WIDTH) * uint32_t(GxGDEM0154E97LT_HEIGHT) / 8)

// divisor for AVR, should be factor of GxGDEW0154Z17_HEIGHT
#define GxGDEM0154E97LT_PAGES 8

#define GxGDEM0154E97LT_PAGE_HEIGHT (GxGDEM0154E97LT_HEIGHT / GxGDEM0154E97LT_PAGES)
#define GxGDEM0154E97LT_PAGE_SIZE (GxGDEM0154E97LT_BUFFER_SIZE / GxGDEM0154E97LT_PAGES)
#define GxGDEM0154E97LT_PU_DELAY 500

class GxGDEM0154E97LT : public GxEPD
{
  public:
    static const uint8_t CMD_DRIVER_OUTPUT = 0x01;          //gate setting
    static const uint8_t CMD_GATE_V = 0x03;                 //set gate related driving voltage
    static const uint8_t CMD_SRC_V = 0x04;                  //set source output voltage magnitude
    static const uint8_t CMD_GATE_SCAN_POS = 0x0F;          //set the scanning start position of the gate driver
    static const uint8_t CMD_SLEEP = 0x10;                  //deep sleep mode control
    static const uint8_t CMD_DATA_ENTRY_MODE = 0x11;        //define data entry sequence
    static const uint8_t CMD_RESET = 0x12;                  //soft reset, sets commands and parameters to defaults
    static const uint8_t CMD_TMP_CTRL = 0x1A;               //write to temperature register
    static const uint8_t CMD_UPDATE = 0x20;                 //activate display update sequence
    static const uint8_t CMD_UPDATE_CTRL_1 = 0x21;          //set update bypass options, used to display ram content
    static const uint8_t CMD_UPDATE_CTRL_2 = 0x22;          //set update sequence options
    static const uint8_t CMD_WRITE_RAM = 0x24;              //write proceeding data to RAM
    static const uint8_t CMD_VCOM_SENSE = 0x28;             //read VCOM value
    static const uint8_t CMD_VCOM_SENSE_DRTN = 0x29;        //set duration for VCOM sensing
    static const uint8_t CMD_VCOM_OTP = 0x2A;               //program VCOM register into OTP (non-volatile memory)
    static const uint8_t CMD_WRITE_VCOM = 0x2C;             //write VCOM register from MCU interface
    static const uint8_t CMD_READ_STTS = 0x2F;              //read status bit
    static const uint8_t CMD_WS_OTP = 0x30;                 //program waveform setting into OTP
    static const uint8_t CMD_WRITE_LUT = 0x32;              //write LUT register
    static const uint8_t CMD_SET_OTP = 0x36;                //program OTP selection
    static const uint8_t CMD_OTP_CTRL = 0x37;               //parameters for OTP selection
    static const uint8_t CMD_DMY_LINE_T = 0x3A;             //set dummy line period in terms of TGate
    static const uint8_t CMD_GATE_LINE_T = 0x3B;            //set gate line period TGate in us
    static const uint8_t CMD_BORDER_CTRL = 0x3C;            //select border waveform for VBD
    static const uint8_t CMD_X_POS = 0x44;                  //specify the x start/end positions of the window address
    static const uint8_t CMD_Y_POS = 0x45;                  //specify the y start/end positions of the window address
    static const uint8_t CMD_X_CNT = 0x4E;                  //set the RAM x counter value
    static const uint8_t CMD_Y_CNT = 0x4F;                  //set the RAM y counter value
    static const uint8_t CMD_ANALOG_CTRL = 0x74;            //set analog block control
    static const uint8_t CMD_DIGITAL_CTRL = 0x7E;           //set digital block control


#if defined(ESP8266)
    //GxGDEW0154Z17(GxIO& io, int8_t rst = D4, int8_t busy = D2);
    // use pin numbers, other ESP8266 than Wemos may not use Dx names
    GxGDEW0154Z17(GxIO& io, int8_t rst = 2, int8_t busy = 4);
#else
    GxGDEM0154E97LT(GxIO& io, core::pin::GpioOutputRef rst, core::pin::GpioRef busy);
#endif

	~GxGDEM0154E97LT() override = default;
	void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void init() override ; // = 0 : disabled

    /*
    * Function name: setWriteWindow
     * Description : Define a window for writing display data. Coordinates start at zero.
     * Enter : x1: starting x coordinate
     *          x2: ending x coordinate
     *          y1: starting y coordinate
     *          y2: ending y coordinate
     * Output : None
     */
    void setWriteWindow(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2);


    /*
    * Function name: Set_Counter
     * Description : Set display RAM counter. Coordinates start at zero.
     * Enter : x: x coordinate
     *          y: y coordinate
     * Output : None
     */
    void Set_Counter(uint16_t x, uint16_t y);


    void fillScreen(uint16_t color) override; // to buffer
    void update(void) override;
    // to buffer, may be cropped, drawPixel() used, update needed
    void  drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode = bm_normal) override;
    // to full screen, filled with white if size is less, no update needed, black  /white / red, for example bitmaps
    void drawExamplePicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size) override;
    // to full screen, filled with white if size is less, no update needed, black  /white / red, general version
    void drawPicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size, int16_t mode = bm_normal) override;
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size = 0, int16_t mode = bm_normal) override; // only bm_normal, bm_invert, bm_partial_update modes implemented
    void eraseDisplay(bool using_partial_update = false) override;
    // partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true) override;
    // partial update of rectangle at (xs,ys) from buffer to screen at (xd,yd), does not power off
    void updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation = true);
    // terminate cleanly updateWindow or updateToWindow before removing power or long delays
    void powerDown();
    // paged drawing, for limited RAM, drawCallback() is called GxGDEW0154Z17_PAGES times
    // each call of drawCallback() should draw the same
    void drawPaged(void (*drawCallback)(void));
    void drawPaged(void (*drawCallback)(uint32_t), uint32_t);
    void drawPaged(void (*drawCallback)(const void*), const void*);
    void drawPaged(void (*drawCallback)(const void*, const void*), const void*, const void*);
    void drawCornerTest(uint8_t em = 0x01);
    void LUT_Written_by_MCU(void);

    void updateFromRAM() override;

    /*
    * Function name: Epaper_LUT
     * Description : Loading waveforms for SSD1675
     * Enter :wave_data - device-specific driving waveform
     * Output : None
     */
    void Epaper_LUT(uint8_t * wave_data);

  private:
    template <typename T> static inline void
    swap(T& a, T& b)
    {
      T t = a;
      a = b;
      b = t;
    }
    void _writeToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation = true);
    void _wakeUp();
    void _sleep();
    void _waitWhileBusy(const char* comment = 0);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
  private:
#if defined(__AVR)
    uint8_t _black_buffer[GxGDEW0154Z17_PAGE_SIZE];
    uint8_t _red_buffer[GxGDEW0154Z17_PAGE_SIZE];
#else
    uint8_t _black_buffer[GxGDEM0154E97LT_BUFFER_SIZE];
#endif
    GxIO& IO;
    int16_t _current_page;
    core::pin::GpioOutputRef _rst;
		core::pin::GpioRef _busy;
#if defined(ESP8266) || defined(ESP32)
  public:
    // the compiler of these packages has a problem with signature matching to base classes
    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
    {
      Adafruit_GFX::drawBitmap(x, y, bitmap, w, h, color);
    };
#endif
};

#ifndef GxEPD_Class
#define GxEPD_Class GxGDEW0154Z17
#define GxEPD_WIDTH GxGDEW0154Z17_WIDTH
#define GxEPD_HEIGHT GxGDEW0154Z17_HEIGHT
#define GxEPD_BitmapExamples <GxGDEW0154Z17/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEW0154Z17/BitmapExamples.h"
#endif

#endif

