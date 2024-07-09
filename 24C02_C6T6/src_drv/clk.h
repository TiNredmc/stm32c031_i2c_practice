#ifndef CLK_H
#define CLK_H

#include "stm32c031xx.h"

void clk_setupHSE();
void clk_enableGPIOClk();
void clk_enableI2CClk();
void clk_initBSP();

#endif