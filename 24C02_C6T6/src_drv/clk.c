#include "clk.h"

void clk_setupHSE(){
	
	// Turn Clock Security System on
	RCC->CR |= RCC_CR_CSSON;
	
	// Turn HSE on : 
	// Won't work IDK why, 
	// TODO - Check Xtal and load Caps 
	RCC->CR |= RCC_CR_HSEON;
	
	__NOP();
	__NOP();
	
	// Wait until HSE is ready
	while(!(RCC->CR & (1 << 17)));
	
	// Switch to HSE
	RCC->CFGR |= 1 << RCC_CFGR_SW_Pos;
	
	// Wait until clock is switched to HSE
	while(!(RCC->CCIPR & RCC_CFGR_SWS_HSE));
	
}

void clk_enableGPIOClk(){
	RCC->IOPENR = 0x0000002F;
	
}

void clk_enableI2CClk(){
	RCC->APBENR1 |= (1 << 21);
	//RCC->CCIPR |= 1 << RCC_CCIPR_I2C1SEL_Pos;
}

void clk_initBSP(){
	//clk_setupHSE();
	clk_enableGPIOClk();
	clk_enableI2CClk();

}