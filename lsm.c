/*
 * lsm.c
 *
 *  Created on: Feb 11, 2024
 *      Author: satwikagarwal
 */

#include "lsm.h"



uint8_t LSM6DS_Init(lsm_ctx_t *dev, SPI_HandleTypeDef *spiHandle, GPIO_TypeDef *csPinBank, uint16_t csPin){
	dev->spiHandle = spiHandle;
	dev->csPinBank = csPinBank;
	dev->csPin = csPin;
//	HAL_GPIO_WritePin(dev->csPinBank, dev->csPin, GPIO_PIN_SET);
	HAL_Delay(100);

	uint8_t ret;
	//read failed
	if(readRegister(dev, WHO_AM_I , &ret, 1)){
		sprintf(dev->errorMsg, "Read Failed");
		return 1;
	};
	//WHO_AM_I is not equal to 108
	if(ret != DEVICE_ID){
		sprintf(dev->errorMsg, "Device Id does not match");
		return 1;
	}

////	doing a software reset using CTRL3_C setting SW_RESET bit = 1 and  IF_INC bit 1
//	uint8_t data = 0b00000101;
//	if(writeRegister(dev, CTRL3_C, &data, 1 ));

	//disable i3c interface recommended by datasheet CTRL9_XL reg
	uint8_t data ;
	if(readRegister(dev, CTRL9_XL, &data, 1)){
		sprintf(dev->errorMsg, "Error reading CTRL9_XL data %u",data);
		return 1;
	}

	data = data | (0x01 << 1);
	if(writeRegister(dev, CTRL9_XL, &data, 1)){
		sprintf(dev->errorMsg, "Error writing CTRL9_XL data");
		return 1;
	}

	//accel setup CTRL1_XL 12hz, +-32g, lpf2 filtering
	data = 0b00010110;
	if(writeRegister(dev, CTRL1_XL, &data, 1)){
		sprintf(dev->errorMsg, "Error writing CTRL1_XL data");
		return 1;
	}
	HAL_Delay(200);
	if(readRegister(dev, CTRL1_XL, &data, 1)){
		sprintf(dev->errorMsg, "Error reading CTRL1_XL data %u",data);
		return 1;
	}

	//gyro setup CTRL2_G 12hz, +-500dps,
	data = 0b00010100;
	if(writeRegister(dev, CTRL2_G, &data, 1)){
		sprintf(dev->errorMsg, "Error writing CTRL1_XL data");
		return 1;
	}
	HAL_Delay(200);
	if(readRegister(dev, CTRL2_G, &data, 1)){
		sprintf(dev->errorMsg, "Error reading CTRL1_XL data %u",data);
		return 1;
	}

	return 0;

}

uint8_t getTemperatureReadings(lsm_ctx_t *dev){

	uint8_t isDataReady;
	if(readRegister(dev, STATUS_REG, &isDataReady, 1)){
		sprintf(dev->errorMsg, "Error in reading ");
		return 1;
	};
	//check if TDA bit is 1 for	Temperature data is ready
	if(isDataReady & 0x04){
		int16_t val;
		if(readRegister(dev, OUT_TEMP_L, dev->raw_temp_data, 2)){
			sprintf(dev->errorMsg, "Error in reading ");
			return 1;
		}
		val = (int16_t)dev->raw_temp_data[1];
		val = (val * 256) + (int16_t)dev->raw_temp_data[0];
		dev->temp = (((float) val / 256.0f) + 25.0f);
	}
	else{
		sprintf(dev->errorMsg, " Temp Data not Ready: %u ",isDataReady);
		return 1;
	}
	return 0;

}


uint8_t getGyroReadings(lsm_ctx_t *dev){

	uint8_t isDataReady;
	if(readRegister(dev, STATUS_REG, &isDataReady, 1)){
		sprintf(dev->errorMsg, "Error in reading ");
		return 1;
	};
	//check if 	GDA bit is 1 for accel data ready
	if(isDataReady & 0x02){
		int16_t val;
		if(readRegister(dev, OUTX_L_G, dev->raw_gyro_data, 6)){
			sprintf(dev->errorMsg, "Error in reading");
			return 1;
		}

		//for gyro_X
		val = (int16_t)dev->raw_accel_data[1];
		val = (val*256) + (int16_t)dev->raw_gyro_data[0];
		//converting to mg (miligee) using conversion factor as given in datasheet under angular rate sensitivity for 500dps
		dev->gyro_x= ((float)val) * 17.50f ;

		//doing same for gyro_y
		val = (int16_t)dev->raw_accel_data[3];
		val = (val*256) + (int16_t)dev->raw_gyro_data[2];
		//converting to mg (miligee) using conversion factor as given in datasheet under angular rate sensitivity for 500dps
		dev->gyro_y = ((float)val) * 17.50f ;

		//doing same for gyro_z
		val = (int16_t)dev->raw_accel_data[5];
		val = (val*256) + (int16_t)dev->raw_gyro_data[4];
		//converting to mg (miligee) using conversion factor as given in datasheet under angular rate sensitivity for 500dps
		dev->gyro_z = ((float)val) * 17.50f ;

	}
	else{
		sprintf(dev->errorMsg, " Temp Data not Ready: %u ",isDataReady);
		return 1;
	}
	return 0;

}


uint8_t getAccelReadings(lsm_ctx_t *dev){

	uint8_t isDataReady;
	if(readRegister(dev, STATUS_REG, &isDataReady, 1)){
		sprintf(dev->errorMsg, "Error in reading ");
		return 1;
	};
	//check if 	XLDA bit is 1 for accel data ready
	if(isDataReady & 0x01){
		int16_t val;
		if(readRegister(dev, OUTX_L_A, dev->raw_accel_data, 6)){
			sprintf(dev->errorMsg, "Error in reading");
			return 1;
		}

		//for accel_X
		val = (int16_t)dev->raw_accel_data[1];
		val = (val*256) + (int16_t)dev->raw_accel_data[0];
		//converting to mg (miligee) using conversion factor as given in datasheet under lineat acceleration sensitivity for +-32g
		//1 milligee (mg) is equivalent to 0.00981 m/s².
		dev->accel_x = ((float)val) * 0.976f ;

		//doing same for accal_y
		val = (int16_t)dev->raw_accel_data[3];
		val = (val*256) + (int16_t)dev->raw_accel_data[2];
		//converting to mg (miligee) using conversion factor as given in datasheet under lineat acceleration sensitivity for +-32g
		//1 milligee (mg) is equivalent to 0.00981 m/s².
		dev->accel_y = ((float)val) * 0.976f ;

		//doing same for accal_z
		val = (int16_t)dev->raw_accel_data[5];
		val = (val*256) + (int16_t)dev->raw_accel_data[4];
		//converting to mg (miligee) using conversion factor as given in datasheet under lineat acceleration sensitivity for +-32g
		//1 milligee (mg) is equivalent to 0.00981 m/s².
		dev->accel_z = ((float)val) * 0.976f ;


	}
	else{
		sprintf(dev->errorMsg, " Temp Data not Ready: %u ",isDataReady);
		return 1;
	}
	return 0;

}


uint8_t readRegister(lsm_ctx_t *dev, uint8_t reg, uint8_t *data , uint16_t len){
	uint8_t add = 0x80 | reg ;
	uint8_t status;
	HAL_GPIO_WritePin(dev->csPinBank, dev->csPin, GPIO_PIN_RESET);
	status = HAL_SPI_Transmit(dev->spiHandle, &add, 1, HAL_MAX_DELAY) != HAL_OK;
	status = HAL_SPI_Receive(dev->spiHandle, data, len , HAL_MAX_DELAY) != HAL_OK;
	HAL_GPIO_WritePin(dev->csPinBank, dev->csPin, GPIO_PIN_SET);
	return status;
}
uint8_t writeRegister(lsm_ctx_t *dev, uint8_t reg, uint8_t *data,uint16_t len ){
	uint8_t add = reg;
	uint8_t status;
	HAL_GPIO_WritePin(dev->csPinBank, dev->csPin, GPIO_PIN_RESET);
	status = HAL_SPI_Transmit(dev->spiHandle, &add, 1, HAL_MAX_DELAY) != HAL_OK;
	status = HAL_SPI_Transmit(dev->spiHandle, data, len , HAL_MAX_DELAY) != HAL_OK;
	HAL_GPIO_WritePin(dev->csPinBank, dev->csPin, GPIO_PIN_SET);
	return status;
}
