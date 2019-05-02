// created by Jean-Marc Zingg to be the GxIO io base class for the GxTFT library
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE

#ifndef _GxIO_H_
#define _GxIO_H_

#include <stdint.h>

class GxIO
{
  public:
    virtual void reset() = 0;
    virtual void init() = 0;
//    virtual uint8_t transferTransaction(uint8_t d);
//    virtual uint16_t transfer16Transaction(uint16_t d);
//    virtual uint8_t readDataTransaction() = 0;
//    virtual uint16_t readData16Transaction() = 0;
//    virtual uint8_t readData() = 0;
//    virtual uint16_t readData16() = 0;
//    virtual uint32_t readRawData32(uint8_t part) = 0; // debug purpose
    virtual void writeCommandTransaction(uint8_t c) = 0;
    virtual void writeDataTransaction(uint8_t d) = 0;
    virtual void writeData16Transaction(uint16_t d, uint32_t num = 1) = 0;
    virtual void writeCommand(uint8_t c) = 0;
    virtual void writeData(uint8_t d) = 0;
    virtual void writeData(uint8_t* d, uint32_t num) = 0;
    virtual void writeData16(uint16_t d, uint32_t num = 1) = 0;
    virtual void writeAddrMSBfirst(uint16_t d) = 0;
//    virtual void startTransaction() = 0;
//    virtual void endTransaction() = 0;
//    virtual void selectRegister(bool rs_low) = 0;
    virtual void setBackLight(bool lit) = 0;
};

#endif

