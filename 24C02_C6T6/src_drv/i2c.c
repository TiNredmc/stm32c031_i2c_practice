#include "i2c.h"

void I2C1_IRQHandler(){
	uint32_t i2cstatus = I2C1->ISR;
	
	// TODO : You already known.
	switch(i2cstatus){
		case 0:
			
			break;
	
		default:
			break;
	}

	I2C1->ICR |= 0x3F38;// Clear Interrupt flags
	
}

uint8_t i2c_initMaster(uint8_t own_address,
	uint8_t i2c_speed){
	
	// I2C own address must be 0-127
	if(own_address > 127)
		return 1;
	
	I2C1->CR1 = 0;
	
	// Write own address
	I2C1->OAR1 |= 
	(1 << I2C_OAR1_OA1EN_Pos) |
	own_address					;
	
	// Set I2C timing thingy
	switch(i2c_speed){
		case I2C_100K:
			{
				I2C1->TIMINGR = 
				(0x03 << I2C_TIMINGR_PRESC_Pos) |
				(0x04 << I2C_TIMINGR_SCLDEL_Pos) |
				(0x02 << I2C_TIMINGR_SDADEL_Pos) |
				(0x0F << I2C_TIMINGR_SCLH_Pos) |
				(0x13 << I2C_TIMINGR_SCLL_Pos);
			}
			break;
		
		case I2C_400K:
			{
				I2C1->TIMINGR = 
				(0x01 << I2C_TIMINGR_PRESC_Pos) |
				(0x03 << I2C_TIMINGR_SCLDEL_Pos) |
				(0x02 << I2C_TIMINGR_SDADEL_Pos) |
				(0x03 << I2C_TIMINGR_SCLH_Pos) |
				(0x09 << I2C_TIMINGR_SCLL_Pos);
			}
			break;
		
		case I2C_1M:
			{
				I2C1->TIMINGR = 
				(0x00 << I2C_TIMINGR_PRESC_Pos) |
				(0x00 << I2C_TIMINGR_SCLDEL_Pos) |
				(0x00 << I2C_TIMINGR_SDADEL_Pos) |
				(0x02 << I2C_TIMINGR_SCLH_Pos) |
				(0x04 << I2C_TIMINGR_SCLL_Pos);
			}
			break;
	
		default:
			return 1;
	}

	I2C1->CR1 |= (1 << 0);// Enable I2C
	
	return 0;
}
	
void i2c_start(){
	I2C1->CR2 |= (1 << I2C_CR2_START_Pos);
}

void i2c_stop(){
	I2C1->CR2 |= (1 << I2C_CR2_STOP_Pos);
}

void i2c_write(uint8_t i2caddr, uint8_t data){

	I2C1->CR2 |=
		((i2caddr & 0x7F) << 1) 	|
		(1 << I2C_CR2_NBYTES_Pos) |
		(1 << I2C_CR2_AUTOEND_Pos);
	
	I2C1->CR2 |= 	
		(1 << I2C_CR2_START_Pos);
	
	while(!(I2C1->ISR & I2C_ISR_TXIS));// Wait until Slave address is sent
	
	I2C1->TXDR = data;
	
	while(!(I2C1->ISR & 1));
}

// Pretty cool but unused
// kept for display.
typedef struct{
	union {
		uint32_t I2C_CR2;
		
		struct{
			uint32_t reserved1		:5;
			uint32_t pec_byte			:1;
			uint32_t auto_end			:1;
			uint32_t reload				:1;
			uint32_t nbytes				:8;
			uint32_t nack					:1;
			uint32_t stop					:1;
			uint32_t start				:1;
			uint32_t head10r			:1;
			uint32_t add10				:1;
			uint32_t rd_wrn				:1;
			union{
				uint32_t sadd10			:10;
				struct{
					uint32_t padding1	:2;
					uint32_t sadd7		:7;
					uint32_t padding2	:1;
				};
			}sadd;
				
		}i2c_cr2_bit;
	};
	
}i2c_cr2_t;

#define I2C_CR2_T (i2c_cr2_t *)(I2C1_BASE + 0x04)

void i2c_writePtr(
	uint8_t i2caddr, 
	uint8_t *ptr, 
	uint16_t len){
		
	uint8_t writeFSM = 0;
	uint8_t nbyte = 0;
	uint8_t do_once = 0;
	while(1){
		switch(writeFSM){
			case 0:// Initial state and restart state
			{
				if(len > 255)
					nbyte = 0xFF;
				else
					nbyte = len;
				
				I2C1->CR2 =
					((i2caddr & 0x7F) << 1) 	|
					(nbyte << I2C_CR2_NBYTES_Pos) |
					(len > 255 ? 
						(1 << I2C_CR2_RELOAD_Pos) : 
						(1 << I2C_CR2_AUTOEND_Pos)
					);
			
				
				if(do_once == 0){
					do_once = 1;
					// Send start bit
					I2C1->CR2 |= 
						(1 << I2C_CR2_START_Pos);
				}
				
				writeFSM = 1;
			}
			break;
		
			case 1:// Wait until TX empty after slave address write
			{
				if(I2C1->ISR & I2C_ISR_TXIS){
					writeFSM = 2;
				}
			}
			break;
			
			case 2:// Write data to the TX buffer
			{
				I2C1->TXDR = *ptr++;
				len--;
				nbyte--;
				writeFSM = 3;
			}
			break;
			
			case 3:// Check for remaining byte to send
			{
				if((nbyte != 0) && (len != 0)){
					// Continue sending byte when nbyte is not yet zero
					writeFSM = 1;
				}else if ((nbyte == 0) && (len != 0)){
					// nbyte reached zero but len hasn't run out yet
					writeFSM = 0;
				}else{
					// no byte left to send, quiting...
					writeFSM = 255;
				}
				
			}
			break;
			
			case 255:// cleanup and exit
			{
				writeFSM = 0;
				return;
			}	
			break;
			
			default:
				return;
		}
	}	
}

void i2c_readPtr(
	uint8_t i2caddr, 
	uint8_t *ptr, 
	uint16_t len){
		
	uint8_t readFSM = 0;
	uint8_t nbyte = 0;
	uint8_t do_once = 0;
	while(1){
		switch(readFSM){
			case 0:// Initial state and restart state
			{
				if(len > 255)
					nbyte = 0xFF;
				else
					nbyte = len;
				
				I2C1->CR2 =
					((i2caddr & 0x7F) << 1) 	|
					(1 << I2C_CR2_RD_WRN_Pos)	|
					(nbyte << I2C_CR2_NBYTES_Pos) |
					(len > 255 ? 
						(1 << I2C_CR2_RELOAD_Pos) : 
						(1 << I2C_CR2_AUTOEND_Pos)
					);
			
				if(do_once == 0){
					do_once = 1;
					// Send start bit 
					// Also double as re-start after I2C write
					I2C1->CR2 |= 
						(1 << I2C_CR2_START_Pos);
				}
				readFSM = 1;
			}
			break;
		
			case 1:// Wait until TX empty after slave address write
			{
				if(I2C1->ISR & I2C_ISR_RXNE){
					readFSM = 2;
				}
			}
			break;
			
			case 2:// Write data to the TX buffer
			{
				*ptr++ = I2C1->RXDR;
				len--;
				nbyte--;
				readFSM = 3;
			}
			break;
			
			case 3:// Check for remaining byte to send
			{
				if((nbyte != 0) && (len != 0)){
					// Continue reading byte when nbyte is not yet zero
					readFSM = 1;
				}else if ((nbyte == 0) && (len != 0)){
					// nbyte reached zero but len hasn't run out yet
					readFSM = 0;
				}else{
					// no byte left to read, quiting...
					readFSM = 255;
				}
				
			}
			break;
			
			case 255:// cleanup and exit
			{
				readFSM = 0;
				return;
			}	
			break;
			
			default:
				return;
		}
	}
	
}
	
void i2c_24C02Read(
	uint8_t i2caddr,
	uint8_t memaddr,
	uint8_t *ptr, 
	uint8_t len){
	uint32_t temp_stat_reg;
		
		
	// PART 1 - Sending EEPROM memory address for read
		
	I2C1->CR2 =
		((i2caddr & 0x7F) << 1)		|	// 7-bit slave address
		(1 << I2C_CR2_NBYTES_Pos) |	// Number of bytes
		(1 << I2C_CR2_RELOAD_Pos);	// Indicates reload
		
	I2C1->CR2 |= 
		(1 << I2C_CR2_START_Pos);		// Generates start bit
	
	
	// Check until address transmission is done
	while(!(I2C1->ISR & I2C_ISR_TXIS));
	
	// Return if we got nack (No address match)
	if((I2C1->ISR & I2C_ISR_NACKF))
		return;
			
	I2C1->TXDR = memaddr;
	
	while(!(I2C1->ISR & I2C_ISR_TXE));
		
	// PART 2 - read data	
		
	i2c_readPtr(
		i2caddr,
		ptr,
		len
	);
	
}
	
void i2c_24C02Write(
	uint8_t i2caddr,
	uint8_t memaddr,
	uint8_t *ptr, 
	uint8_t len){

	uint8_t writeFSM = 0;
	uint8_t nbyte = 0;
	uint8_t do_once = 0;
		
	len++;	
		
	while(1){
		switch(writeFSM){
			case 0:// Initial state and restart state
			{
				if(len > 255)
					nbyte = 0xFF;
				else
					nbyte = len;
				
				I2C1->CR2 =
					((i2caddr & 0x7F) << 1) 	|
					(nbyte << I2C_CR2_NBYTES_Pos) |
					(len > 255 ? 
						(1 << I2C_CR2_RELOAD_Pos) : 
						(1 << I2C_CR2_AUTOEND_Pos)
					);
			
				if(do_once == 0){
					do_once = 1;
					// Send start bit
					I2C1->CR2 |= 
						(1 << I2C_CR2_START_Pos);
					
					writeFSM = 1;
				}else{
					writeFSM = 4;
				}
			}
			break;
		
			case 1:// Wait until TX empty after slave address write
			{
				if(I2C1->ISR & I2C_ISR_TXIS){
					writeFSM = 2;
				}
			}
			break;
			
			case 2:// Write mem address to the TX buffer
			{
				I2C1->TXDR = memaddr;
				len--;
				nbyte--;
				writeFSM = 3;
			}
			break;
			
			case 3:// Check for remaining byte to send
			{
				if((nbyte != 0) && (len != 0)){
					// Continue sending byte when nbyte is not yet zero
					writeFSM = 4;
				}else if ((nbyte == 0) && (len != 0)){
					// nbyte reached zero but len hasn't run out yet
					writeFSM = 0;
				}else{
					// no byte left to send, quiting...
					writeFSM = 255;
				}
				
			}
			break;
			
			case 4:// Wait until TX empty after slave address write
			{
				if(I2C1->ISR & I2C_ISR_TXIS){
					writeFSM = 5;
				}
			}
			break;
			
			case 5:// Write data to the TX buffer
			{
				I2C1->TXDR = *ptr++;
				len--;
				nbyte--;
				writeFSM = 3;
			}
			break;
			
			case 255:// cleanup and exit
			{
				writeFSM = 0;
				return;
			}	
			break;
			
			default:
				return;
		}
	}	
}