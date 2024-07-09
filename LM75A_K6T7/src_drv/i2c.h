#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdio.h>
#include "stm32c031xx.h"

enum{
	I2C_100K,
	I2C_400K,
	I2C_1M
};

uint8_t i2c_initMaster(uint8_t own_address,
	uint8_t i2c_speed);

void i2c_start();
void i2c_stop();
void i2c_write(uint8_t i2caddr, uint8_t data);
void i2c_writePtr(
	uint8_t i2caddr, 
	uint8_t *ptr, 
	uint16_t len);

void i2c_readPtr(
	uint8_t i2caddr, 
	uint8_t *ptr, 
	uint16_t len);

void i2c_24C02Write(
	uint8_t i2caddr,
	uint8_t memaddr,
	uint8_t *ptr, 
	uint8_t len);

void i2c_24C02Read(
	uint8_t i2caddr,
	uint8_t memaddr,
	uint8_t *ptr, 
	uint8_t len);

float i2c_lm75GetTemp();

#endif