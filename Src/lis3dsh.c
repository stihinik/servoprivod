#include "lis3dsh.h"
//————————————
extern SPI_HandleTypeDef hspi1;
//————————————
static void Error (void)
{
	LD5_ON;
}
//—————————————
static uint8_t SPIx_WriteRead(uint8_t Byte)
{
  uint8_t receivedbyte = 0;
  if(HAL_SPI_TransmitReceive(&hspi1, (uint8_t*) &Byte, (uint8_t*) &receivedbyte, 1, 0x1000) != HAL_OK)
  {
    Error();
  }
  return receivedbyte;
}
//—————————————
void Accel_IO_Read(uint8_t *pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead)
{
  if(NumByteToRead > 0x01)
  {
    ReadAddr |= (uint8_t)(READWRITE_CMD | MULTIPLEBYTE_CMD);
  }
  else
  {
    ReadAddr |= (uint8_t)READWRITE_CMD;
  }
        CS_ON;
  SPIx_WriteRead(ReadAddr);
  while(NumByteToRead > 0x00)
  {
    /* Send dummy byte (0x00) to generate the SPI clock to ACCELEROMETER (Slave device) */
    *pBuffer = SPIx_WriteRead(DUMMY_BYTE);
    NumByteToRead--;
    pBuffer++;
  }
        CS_OFF;
}
//—————————————
void Accel_IO_Write(uint8_t *pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite)
{
        CS_OFF;
  if(NumByteToWrite > 0x01)
  {
    WriteAddr |= (uint8_t)MULTIPLEBYTE_CMD;
  }
        CS_ON;
        SPIx_WriteRead(WriteAddr);
        while(NumByteToWrite >= 0x01)
  {
    SPIx_WriteRead(*pBuffer);
    NumByteToWrite--;
    pBuffer++;
  }
        CS_OFF;
}
//—————————————
uint8_t Accel_ReadID(void)
{  
  uint8_t ctrl = 0;
  Accel_IO_Read(&ctrl, LIS3DSH_WHO_AM_I_ADDR, 1);
  return ctrl;
}
//—————————————
void Accel_AccFilterConfig(uint8_t FilterStruct)
{
 
}
//—————————————
void AccInit(uint16_t InitStruct)
{
  uint8_t ctrl = 0x00;
  ctrl = (uint8_t) (InitStruct);
        Accel_IO_Write(&ctrl, LIS3DSH_CTRL_REG4_ADDR, 1);
  ctrl = (uint8_t) (InitStruct >> 8);
        Accel_IO_Write(&ctrl, LIS3DSH_CTRL_REG5_ADDR, 1);
}
 
//—————————————
void Accel_GetXYZ(int16_t* pData)
{
  int8_t buffer[6];
  uint8_t crtl, i = 0x00;
  float sensitivity = LIS3DSH_SENSITIVITY_0_06G;
  float valueinfloat = 0; 
	Accel_IO_Read(&crtl, LIS3DSH_CTRL_REG5_ADDR, 1);
  Accel_IO_Read((uint8_t*)&buffer[0], LIS3DSH_OUT_X_L_ADDR, 1);
  Accel_IO_Read((uint8_t*)&buffer[1], LIS3DSH_OUT_X_H_ADDR, 1);
  Accel_IO_Read((uint8_t*)&buffer[2], LIS3DSH_OUT_Y_L_ADDR, 1);
  Accel_IO_Read((uint8_t*)&buffer[3], LIS3DSH_OUT_Y_H_ADDR, 1);
  Accel_IO_Read((uint8_t*)&buffer[4], LIS3DSH_OUT_Z_L_ADDR, 1);
  Accel_IO_Read((uint8_t*)&buffer[5], LIS3DSH_OUT_Z_H_ADDR, 1);
	switch(crtl & LIS3DSH__FULLSCALE_SELECTION)
  {
  case LIS3DSH_FULLSCALE_2:
    sensitivity = LIS3DSH_SENSITIVITY_0_06G;
    break;
  case LIS3DSH_FULLSCALE_4:
    sensitivity = LIS3DSH_SENSITIVITY_0_12G;
    break;
  case LIS3DSH_FULLSCALE_6:
    sensitivity = LIS3DSH_SENSITIVITY_0_18G;
    break;
  case LIS3DSH_FULLSCALE_8:
    sensitivity = LIS3DSH_SENSITIVITY_0_24G;
    break;
  case LIS3DSH_FULLSCALE_16:
    sensitivity = LIS3DSH_SENSITIVITY_0_73G;
    break;
  default:
    break;                
  }
    for(i=0; i<3; i++)
  {
    valueinfloat = ((buffer[2*i+1] << 8) + buffer[2*i]) * sensitivity;
    pData[i] = (int16_t)valueinfloat;
  }
}
//—————————————
void zero (uint16_t buf[15])
{
	uint16_t k=0;
		while (k<15)
		{
			buf[k]=0x00;
			k++;
		}
	k=0;
}
uint16_t Accel_ReadAcc(void)
{
	uint16_t count[15],t,d,m;
	uint16_t i=0x00;
  int16_t buffer[3] = {0};
  int16_t xval, yval , zval = 0x00;
	t=0x00;
	zero(count);
	while(t==0)
	{
		Accel_GetXYZ(buffer);
		xval = buffer[0];
		yval = buffer[1];
		zval = buffer[2];
		d=(yval);
		m=ABS(yval);
		if(d<65000)
		{	
			if ((m<150))
			{
				LD5_ON;
				count[7]++;
			}
			else
			{
				if(ABS(yval)>5000)
				{
						if(zval<300){count[0]++;}
						else if(zval<400){count[1]++;}
						else if(zval<500){count[2]++;}
						else if(zval<600){count[3]++;}
						else if(zval<700){count[4]++;}
						else if(zval<800){count[5]++;}
						else if(zval<900){count[6]++;}
				}
				else
				{
						if(zval<550){count[14]++;}
						else if(zval<650){count[13]++;}
						else if(zval<725){count[12]++;}
						else if(zval<800){count[11]++;}
						else if(zval<875){count[10]++;}
						else if(zval<940){count[9]++;}
						else{count[8]++;}
				 }
		  }
			i=2000;
			if(count[0]>i){t=7;zero(count);}
			if(count[1]>i){t=8;zero(count);}
			if(count[2]>i){t=9;zero(count);}
			if(count[3]>i){t=10;zero(count);}
			if(count[4]>i){t=11;zero(count);}
			if(count[5]>i){t=12;zero(count);}
			if(count[6]>i){t=13;zero(count);}
			if(count[7]>i){t=14;zero(count);}
			if(count[8]>i){t=15;zero(count);}
			if(count[9]>i){t=16;zero(count);}
			if(count[10]>i){t=17;zero(count);}
			if(count[11]>i){t=18;zero(count);}
			if(count[12]>i){t=19;zero(count);}
			if(count[13]>i){t=20;zero(count);}
			if(count[14]>i){t=21;zero(count);}
		}
	}
	
	return t;
}	
//—————————————
void Accel_Ini(void)
{
	uint16_t ctrl = 0x0000;
	HAL_Delay(1000);
	if(Accel_ReadID() == 0x3F) LD4_ON;
	else Error();
	/* Configure MEMS: power mode(ODR) and axes enable */
		ctrl = (uint16_t) (LIS3DSH_DATARATE_100 | LIS3DSH_XYZ_ENABLE);
		/* Configure MEMS: full scale and self test */
		ctrl |= (uint16_t) ((LIS3DSH_SERIALINTERFACE_4WIRE |
														 LIS3DSH_SELFTEST_NORMAL   |
														 LIS3DSH_FULLSCALE_2  |
														 LIS3DSH_FILTER_BW_800) << 8);
    AccInit(ctrl);
    LD6_ON;
}