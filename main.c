#include "stm32f4xx.h"                  						// Device header

#define	UserButton	(GPIOA->IDR & 0x01)							//User Button -> PA0

void delayMs(uint32_t delay) {											//Delay function
	while(delay > 0) delay--;
}

int main() {
	RCC->AHB1ENR |= 1ul << 0;													//Clock signal for GPIOA
	RCC->AHB1ENR |= 1ul << 3;													//Clock signal for GPIOD
	
	GPIOD->MODER = 0x55000000;												//PD12 - PD15 -> Output
	
	while(1) {
		if(UserButton) {
			delayMs(10000);
			GPIOD->ODR |= 1ul << 14;
		}
		else {
			GPIOD->ODR &= ~(1ul << 14);
		}
	}
}
