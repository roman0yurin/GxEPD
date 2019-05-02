// created by Jean-Marc Zingg to be the GxIO io base class for the GxTFT library
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE

#include "GxIO.h"

//uint8_t GxIO::transferTransaction(uint8_t d)
//{
//  writeDataTransaction(d);
//  return readDataTransaction();
//}
//
//uint16_t GxIO::transfer16Transaction(uint16_t d)
//{
//  writeData16Transaction(d);
//  return readData16Transaction();
//}

uint8_t GxIO::readDataTransaction()
{
	UNSUPPORTED
  return 0;
};

uint16_t GxIO::readData16Transaction()
{
	UNSUPPORTED
  return 0;
};

uint8_t GxIO::readData()
{
	UNSUPPORTED
  return 0;
};

uint16_t GxIO::readData16()
{
	UNSUPPORTED
  return 0;
};

uint32_t GxIO::readRawData32(uint8_t part)
{
	UNSUPPORTED
  return 0;
};



