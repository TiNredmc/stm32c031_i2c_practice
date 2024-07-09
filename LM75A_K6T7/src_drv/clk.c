#include "clk.h"

void clk_setupHSE(){
	
	// Turn HSE on
	RCC->CR |= RCC_CR_HSEON;
	
	// Wait until HSE is ready
	while(!(RCC->CR & RCC_CR_HSERDY));
	
	// Switch to HSE
	RCC->CFGR |= 1 << 0;
	
	// Wait until clock is switched to HSE
	while(!(RCC->CFGR & RCC_CFGR_SWS_HSE));
	
}

void clk_enableGPIOClk(){
	RCC->IOPENR |= 
		(1 << RCC_IOPENR_GPIOAEN_Pos)	|
		(1 << RCC_IOPENR_GPIOBEN_Pos)	|
		(1 << RCC_IOPENR_GPIOCEN_Pos)	|
		(1 << RCC_IOPENR_GPIODEN_Pos)	|
		(1 << RCC_IOPENR_GPIOFEN_Pos);
	
}

void clk_enableI2CClk(){
	RCC->APBENR1 |= (1 << RCC_APBENR1_I2C1EN_Pos);
	RCC->CCIPR |= 1 << RCC_CCIPR_I2C1SEL_Pos;
}

void clk_initBSP(){
	clk_setupHSE();
	clk_enableGPIOClk();
	clk_enableI2CClk();

}