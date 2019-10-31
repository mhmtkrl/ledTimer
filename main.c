#include "stm32f4xx.h"                  						// Device header
#include "softTimer.h"

#define	UserButton	(GPIOA->IDR & 0x01)							//User Button -> PA0

void delayMs(uint32_t delay);
void SystemFullSpeed(void);

uint16_t hard_counter = 0, soft_counter = 0;
uint8_t timerNo = 0;

uint8_t oneSec = 0, twoSec = 0;
uint8_t buttonCounter = 0, prevButtonCounter = 0;

typedef enum{
	modeShort,
	modeLong
}modes;

uint8_t ledMode = 0;

void TIM7_IRQHandler() {														//Timer7 ISR function
	if(TIM7->SR) {
		SoftTimer_ISR();
		if((buttonCounter)) {
			hard_counter++;
		}
	}
	TIM7->SR = 0;
}

void delayMs(uint32_t delay) {											//Delay function
	while(delay > 0) delay--;
}

void SystemFullSpeed(void) {
  RCC->CFGR |= 0x00009400;        									//AHB & APB -> max speed
  RCC->CR |= 0x00010000;          									//HSE clock source
  RCC->PLLCFGR = 0x07402A04;      									//PLL -> M=4, N=168, P=2, Q=7 
  RCC->CR |= 0x01000000;          									//PLL start
  FLASH->ACR = 0x00000605;        									//Flash ROM -> 5 wait state 
  RCC->CFGR |= 0x00000002;        									//System clock source -> PLL
}

int main() {
	SystemFullSpeed();
	RCC->AHB1ENR |= 1ul << 0;													//Clock signal for GPIOA
	RCC->AHB1ENR |= 1ul << 3;													//Clock signal for GPIOD
	RCC->APB1ENR = 1ul << 5;													//Clock signal for Timer7
	
	GPIOD->MODER = 0x55000000;												//PD12 - PD15 -> Output
	
	//1ms Timer7 interrupt
	TIM7->CR1 = 0x0085;
	TIM7->DIER = 0x0001;
	TIM7->SR = 0x00;
	TIM7->CNT = 0;
	TIM7->PSC = 69 ;
	TIM7->ARR = 599 ;         
  NVIC->ISER[1] = 0x00800000;  
	
	SoftTimer_ResetTimer(TIMER_A);
	SoftTimer_Init();
	SoftTimer_SetTimer(TIMER_A, 0);
	
	while(1) {
		//User button sequence
		if(UserButton) {
			buttonCounter = 1;
			delayMs(16800000);												//100ms debounce
			

			if(hard_counter >= 3000) {
				ledMode = modeLong;
			}
			else {
				ledMode = modeShort;
			}
		}
		else {
			buttonCounter = 0;
			hard_counter = 0;
		}
		
		//if((buttonCounter != prevButtonCounter) && (ledMode == modeLong)) hard_counter = 0;
		
		//Software Timer Sequence
		timerNo = SoftTimer_GetTimerStatus(TIMER_A);
		if(timerNo) {
			SoftTimer_ResetTimer(TIMER_A);
			soft_counter++;
			SoftTimer_SetTimer(TIMER_A, 0);
		}
		
		if(soft_counter >= 1000) {
			oneSec = 1;
		}
		if(soft_counter >= 2000) {
			twoSec = 1;
			soft_counter = 0;
		}
		//Software timer led blink
		
		if(oneSec) {
			GPIOD->ODR |= 1ul << 12;
		}
		if(twoSec) {
			oneSec = 0;
			GPIOD->ODR &= ~(1ul << 12);
			twoSec = 0;
		}
			
		if(ledMode == modeShort) {
			GPIOD->ODR |= 1ul << 15;
			GPIOD->ODR &= ~(1ul << 14);
		}
		if(ledMode == modeLong) {
			GPIOD->ODR |= 1ul << 14;
			GPIOD->ODR &= ~(1ul << 15);
		}
		prevButtonCounter = buttonCounter;
	}
}
