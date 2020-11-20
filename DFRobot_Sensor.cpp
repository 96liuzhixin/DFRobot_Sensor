/*!
 * @file DFRobot_Sensor.cpp
 * @brief 定义DFRobot_Sensor 类的基础结构，基础方法的实现
 *
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @author [Fary](fary_young@dfrobot.com)
 * @version  V1.0
 * @date  2020-11-20
 * @https://github.com/fary99/DFRobot_Sensor
 */
#include <DFRobot_Sensor.h>

DFRobot_Sensor::DFRobot_Sensor(uint8_t mode)
:_mode(*(uint8_t*)&mode){
  
}

int DFRobot_Sensor::begin(void){
  uint8_t id=0xDF;
  uint8_t buf;
  if(readReg(SENSOR_ADDR_ID,&buf,1) == 0){
    DBG("bus data access error");
    return ERR_DATA_BUS;
  }
  DBG("real sensor id=");
  DBG(id);
  if(id != DFRobot_Sensor_ID){
    return ERR_IC_VERSION;
  }
  writeReg(SENSOR_ADDR_CONFIG,&_mode,1); //向配置寄存器的地址写入，初始化传感器
  DBG("begin ok!");
  return ERR_OK;
}

uint16_t DFRobot_Sensor::soundStrengthDB(void){
  sCombineData_t data;
  readReg(SENSOR_ADDR_DATA, &data, sizeof(data));
  DBG("sound reg raw data is");
  DBG(data.sound);
  return data.sound <<3;
}

uint32_t DFRobot_Sensor::lightStrengthLux(void){
  sCombineData_t data;
  readReg(SENSOR_ADDR_DATA, &data, sizeof(data));
  DBG("light reg raw data is");
  DBG(data.sound);
  return data.light * 10000;
}

void DFRobot_Sensor::setLED(uint8_t r,uint8_t g, uint8_t b){
  sColor_t data = {.b = b>>3, .g = g>>2, .r = r>>3};
  writeReg(SENSOR_ADDR_LED, &data, sizeof(data));
}

void DFRobot_Sensor::setLED(uint16_t color){
  writeReg(SENSOR_ADDR_LED, &color, 1);
}

uint8_t DFRobot_Sensor::switchMode(uint8_t mode){
  uint8_t tmp;
  #ifdef ENABLE_DBG
    readReg(SENSOR_ADDR_CONFIG, &tmp; sizeof(tmp));
    DBG("before switch Mod, ModeReg = ");
    DBG(tmp);
  #endif
  writeReg(SENSOR_ADDR_CONFIG, &mode, sizeof(mode));
  #ifdef ENABLE_DBG
    readReg(SENSOR_ADDR_CONFIG, &tmp; sizeof(tmp));
    DBG("after switch Mod, ModeReg = ");
    DBG(tmp);
  #endif
  return 0;
}

DFRobot_Sensor_IIC::DFRobot_Sensor_IIC(TwoWire *pWire, uint8_t mode):DFRobot_Sensor(mode){
  _deviceAddr = DFRobot_Sensor_IIC_ADDR;
  _pWire = pWire;
}

int DFRobot_Sensor_IIC::begin(void){
  Wire.begin();
  return DFRobot_Sensor::begin();
}

void DFRobot_Sensor_IIC::wirteReg(uint8_t reg, void* pBuf, size_t size){
  if(pBuf == NULL){
    DBG("pBuf ERROR!! : null pointer");
  }
  
  uint8_t * _PBuf = (uint8_t*)pBuf;
  _pWire->beginTransmission(_deviceAddr);
  for(uint16_t i=0; i<size; i++){
    _pWire->write(_pBuf[i]);
  }
  _pWire->endTransmission();
}

uint8_t DFRobot_Sensor_IIC::readReg(uint8_t reg, void* pBuf, size_t size){
  if(pBuf == NULL){
    DBG("pBuf ERROR!! : null pointer");
  }
  
  uint8_t* _pBuf = (uint8_t*)pBuf;
  _pWire->beginTransmission(_deviceAddr);
  if(_pWire->endTransmission() != 0){
    return 0;
  }
  
  _pWire->requestFrom(_deviceAddr, (uint8_t) size);
  for(uint16_t i=0 ; i<size; i++){
    _pBuf[i] = _pWire->read();
  }
  _pWire->endTransmission();
  
  return size;
}

DFRobot_Sensor_SPI::DFRobot_Sensor_SPI(SPIClass *pSpi, uint8_t csPin, uint8_t mode):DFRobot_Sensor(mode){
  _pSpi = pSpi;
  _csPin = csPin;
}

int DFRobot_Sensor_SPI::begin(void){
  pinMode(csPin, OUTPUT);
  _pSpi->begin();
  return DFRobot_Sensor::begin();
}

void DFRobot_Sensor_SPI::wirteReg(uint8_t reg, void* pBuf, size_t size){
  if(pBuf == NULL){
    DBG("pBuf ERROR!! : null pointer");
  }
  
  uint8_t * _PBuf = (uint8_t*)pBuf;
  _pSpi->beginTransaction(SPISetting(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_csPin, LOW);
  _pSpi->transfer(reg);
  while(size--){
    _pSpi->transfer(*_pBuf);
    _pBuf++;
  }
  digitalWrite(_csPin, HIGH);
  _pSpi->endTransaction();
}

uint8_t DFRobot_Sensor_SPI::readReg(uint8_t reg, void* pBuf, size_t size){
  if(pBuf == NULL){
    DBG("pBuf ERROR!! : null pointer");
  }
  
  uint8_t * _PBuf = (uint8_t*)pBuf;
  size_t count = 0;
  _pSpi->beginTransaction(SPISetting(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_csPin,LOW);
  _pSpi->transfer(reg);
  while(size--){
    *_PBuf = _pSpi->transfer(0x00);
    _pBuf++;
    count++;
  }
  digitalWrite(_csPin, HIGH);
  _pSpi->endTransaction();
  return count;
}


