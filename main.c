#include "stm32f4xx.h"                  						// Device header

#define	UserButton	(GPIOA->IDR & 0x01)							//User Button -> PA0

void delayMs(uint32_t delay);

uint8_t counter = 0;

void TIM7_IRQHandler() {														//Timer7 ISR function
	if(TIM7->SR) {
		counter++;
		if(counter > 1) counter = 0;
	}
	TIM7->SR = 0;
}

void delayMs(uint32_t delay) {											//Delay function
	while(delay > 0) delay--;
}

int main() {
	RCC->AHB1ENR |= 1ul << 0;													//Clock signal for GPIOA
	RCC->AHB1ENR |= 1ul << 3;													//Clock signal for GPIOD
	RCC->APB1ENR = 1ul << 5;													//Clock signal for Timer7
	
	GPIOD->MODER = 0x55000000;												//PD12 - PD15 -> Output
	
	TIM7->CR1 = 0x0085;
	TIM7->DIER = 0x0001;
	TIM7->SR = 0x00;
	TIM7->CNT = 0;
	TIM7->PSC = 29;
	TIM7->ARR = 59999;        
  NVIC->ISER[1] = 0x00800000;  
	
	while(1) {
		//User button sequence
		if(UserButton) {
			delayMs(10000);
			GPIOD->ODR |= 1ul << 14;
		}
		else {
			GPIOD->ODR &= ~(1ul << 14);
		}
		
		//Timer Sequence
		if(counter) {
			GPIOD->ODR |= 1ul << 12;
		}
		else {
			GPIOD->ODR &= ~(1ul << 12);
		}
	}
}
