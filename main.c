#include "stm32f4xx.h"                  						// Device header
#include "softTimer.h"

#define	UserButton	(GPIOA->IDR & 0x01)							//User Button -> PA0

void delayMs(uint32_t delay);

uint8_t hard_counter = 0, soft_counter = 0;
uint8_t timerNo = 0;

void TIM7_IRQHandler() {														//Timer7 ISR function
	if(TIM7->SR) {
		SoftTimer_ISR();
		hard_counter++;
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
	
	//Timer Manual control
	TIM7->CR1 &= ~(1ul << 0);
	
	SoftTimer_ResetTimer(TIMER_A);
	SoftTimer_Init();
	SoftTimer_SetTimer(TIMER_A, 0);
	
	while(1) {
		//User button sequence
		if(UserButton) {
			delayMs(10000);
			//Timer Enable
			TIM7->CR1 |= 1ul << 0;
		}
		else {
			//Timer Disable
			TIM7->CR1 &= ~(1ul << 0);
		}
		
		//Hardware timer led blink
		if(hard_counter > 10) {
			GPIOD->ODR |= 1ul << 12;
			GPIOD->ODR &= ~(1ul << 13);
		}
	 if(hard_counter > 20){
		 hard_counter = 0;
			GPIOD->ODR |= 1ul << 13;
		 GPIOD->ODR &= ~(1ul << 12);
		}
		
		//Software Timer Sequence
		timerNo = SoftTimer_GetTimerStatus(TIMER_A);
		if(timerNo) {
			SoftTimer_ResetTimer(TIMER_A);
			soft_counter++;
			SoftTimer_SetTimer(TIMER_A, 0);
		}
		
		//Software timer led blink
		if(soft_counter > 10) {
			GPIOD->ODR |= 1ul << 14;
			GPIOD->ODR &= ~(1ul << 15);
		}
	 if(soft_counter > 20){
		 soft_counter = 0;
		 GPIOD->ODR |= 1ul << 15;
		 GPIOD->ODR &= ~(1ul << 14);
		}

	}
}
