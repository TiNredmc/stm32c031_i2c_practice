#include <stdint.h>

#include "stm32c031xx.h"
#include "clk.h"
#include "systick.h"
#include "i2c.h"

float degC_temp;

void GPIO_init(){

	// I2C GPIO PB8 (SCL) and PB9 (SDA)
	GPIOB->MODER &= 
	~(
		(3 << 8*2) |
		(3 << 9*2)
		);
	GPIOB->MODER |= 
		(2 << 8*2)	|
		(2 << 9*2);	

	GPIOB->OTYPER |= 
		(1 << 8) |
		(1 << 9);
	
	GPIOB->OSPEEDR |= 
		(6 << 0)	|
		(6 << 4);
	
	GPIOB->AFR[1] |= 
		(6 << 0)	|
		(6 << 4);
	
}

uint32_t i2c_millis = 0;
uint32_t blink_millis = 0;

int main(){
	clk_initBSP();
	systick_init(16000000, 1000);
	GPIO_init();
	i2c_initMaster(0x00, I2C_100K);
	delay_ms(100);
	
	while(1){
		
		if((millis() - i2c_millis) > 100){
			i2c_millis = millis();
			degC_temp = i2c_lm75GetTemp();
		}

		
	}
}