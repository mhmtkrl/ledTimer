#include "stm32f4xx.h"                  						// Device header

void delayMs(uint32_t delay) {											//Delay function
	while(delay > 0) delay--;
}

int main() {
	RCC->AHB1ENR |= 1ul << 3;													//Clock signal for GPIOD
	
	GPIOD->MODER = 0x55000000;												//PD12 - PD15 -> Output
	
	while(1) {
		GPIOD->ODR |= 1ul << 12;
		delayMs(1000000);
		GPIOD->ODR &= ~(1ul << 12);
		delayMs(1000000);
	}
}
