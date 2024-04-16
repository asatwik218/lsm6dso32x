/*
 * lsm.h
 *
 *  Created on: Feb 11, 2024
 *      Author: satwikagarwal
 */

#ifndef INC_LSM_H_
#define INC_LSM_H_

#include "stm32f4xx_hal.h"
#include <usbd_cdc_if.h>

#define DEVICE_ID     0x6C
#define INT1_CTRL	  0x0D
#define INT2_CTRL	  0x0E
#define WHO_AM_I      0x0F
#define CTRL1_XL	  0x10
#define CTRL2_G		  0x11
#define CTRL3_C		  0x12
#define CTRL4_C		  0x13
#define CTRL5_C		  0x14
#define CTRL6_C		  0x15
#define CTRL7_G		  0x16
#define CTRL8_XL	  0x17
#define CTRL9_XL	  0x18
#define CTRL10_C	  0x19
#define STATUS_REG	  0x1E
#define OUT_TEMP_L 	  0x20
#define OUT_TEMP_H    0x21
#define OUTX_L_G	  0x22
#define OUTX_H_G	  0x23
#define OUTY_L_G	  0x24
#define OUTY_H_G	  0x25
#define OUTZ_L_G	  0x26
#define OUTZ_H_G	  0x27
#define OUTX_L_A	  0x28
#define OUTX_H_A	  0x29
#define OUTY_L_A	  0x2A
#define OUTY_H_A	  0x2B
#define OUTZ_L_A	  0x2C
#define OUTZ_H_A	  0x2D






typedef struct{
	SPI_HandleTypeDef *spiHandle;
	GPIO_TypeDef *csPinBank;
	uint16_t csPin;

	uint8_t raw_temp_data[2];
	uint8_t raw_accel_data[6];
	uint8_t raw_gyro_data[6];

	float temp;
	float accel_x;
	float accel_y;
	float accel_z;
	float gyro_x;
	float gyro_y;
	float gyro_z;

	char errorMsg[30];

} lsm_ctx_t;

uint8_t LSM6DS_Init(lsm_ctx_t *dev, SPI_HandleTypeDef *spiHandle, GPIO_TypeDef *csPinBank, uint16_t csPin);
uint8_t getTemperatureReadings(lsm_ctx_t *dev);
uint8_t getGyroReadings(lsm_ctx_t *dev);
uint8_t getAccelReadings(lsm_ctx_t *dev);
uint8_t readRegister(lsm_ctx_t *dev, uint8_t reg, uint8_t *data , uint16_t len);
uint8_t writeRegister(lsm_ctx_t *dev, uint8_t reg, uint8_t *data, uint16_t len );


#endif /* INC_LSM_H_ */
