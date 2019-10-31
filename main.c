#include "stm32f4xx.h"                  						// Device header
#include "softTimer.h"

#define PushingTime 5000
#define	UserButton	(GPIOA->IDR & 0x01)							//User Button -> PA0

void delayMs(uint32_t delay);
void SystemFullSpeed(void);

uint16_t hard_counter = 0, soft_counter = 0;
uint8_t timerNo = 0;

uint8_t oneSec = 0, twoSec = 0;
uint8_t buttonCounter = 0, prevButtonCounter = 0;

uint32_t clock = 0;

typedef enum{
	modeShort,
	modeLong,
	modeSpec
}modes;

uint8_t ledMode = 0;

void TIM7_IRQHandler() {														//Timer7 ISR function
	if(TIM7->SR) {
		//1sec interrupt software flag
		soft_counter++;
		if(soft_counter >= 1000) oneSec = 1;
		if(soft_counter >= 2000) {
			oneSec = 0;
			soft_counter = 0;
		}
		//////////////////////////////
		//Decide ledMode long or short period
		if(prevButtonCounter && hard_counter >= PushingTime) {
			ledMode = modeLong;
		}
		if(buttonCounter && hard_counter < PushingTime && ledMode == modeLong) {
			ledMode = modeSpec;
		}
		if(prevButtonCounter && hard_counter < PushingTime && ledMode == modeSpec) {
			ledMode = modeShort;
		}
		///////////////////////////
		//Measure pushing time
		if(buttonCounter) {
			hard_counter++;
			if(hard_counter >= PushingTime) hard_counter = PushingTime;
		}
		else {
			hard_counter = 0;
		}
		/////////////////////////////

		//During long mode
		if(ledMode == modeLong || ledMode == modeSpec) {
			GPIOD->ODR &= ~(1ul << 13);
			if(oneSec) {
				GPIOD->ODR |= 1ul << 15;
			}
			else {
				GPIOD->ODR &= ~(1ul << 15);
			}
		}
		////////////////////////////
		//During short mode
		if(ledMode == modeShort) {
			GPIOD->ODR &= ~(1ul << 15);
			if(oneSec) {
				GPIOD->ODR |= 1ul << 13;
			}
			else {
				GPIOD->ODR &= ~(1ul << 13);
			}
		}
		/////////////////////////////
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
		//Detect user button
		if(UserButton) {
			delayMs(1680000);
			GPIOD->ODR |= 1ul << 14;
			buttonCounter = 1;
			prevButtonCounter = 0;
			
			while(UserButton) {
				
			}
			buttonCounter = 0;
			prevButtonCounter = 1;
			GPIOD->ODR &= ~(1ul << 14);
		}

			
			
	
	}
}
