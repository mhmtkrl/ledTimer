#include "stm32f4xx.h"                  						// Device header
#include "softTimer.h"

#define PushingTime 5000
#define	UserButton	(GPIOA->IDR & 0x01)							//User Button -> PA0

void delayMs(uint32_t delay);
void SystemFullSpeed(void);

uint16_t hard_counter = 0, soft_counter = 0;
uint8_t timerNo = 0;

uint8_t oneSec = 0, twoSec = 0, twosecled = 0;
uint8_t buttonCounter = 0, prevButtonCounter = 0;

uint8_t shortModeCounter = 0;

uint32_t clock = 0;

uint8_t here = 0;

typedef enum{
	modeShort,
	modeLong,
	modeSpec
}modes;

uint8_t ledMode = 0, prevLedMode = 0;

void TIM7_IRQHandler() {														//Timer7 ISR function
	if(TIM7->SR) {
		//1sec interrupt software flag
		soft_counter++;
		if(soft_counter >= 1000/2) {
			oneSec = 1;
		}
		if(soft_counter >= 2000/2) {
			oneSec = 0;
			twosecled = 1;
		}
		if(soft_counter >= 3000/2) {
			oneSec = 1;
			twosecled = 1;
		}
		if(soft_counter >= 4000/2) {
			oneSec = 0;
			twosecled = 0;
			soft_counter = 0;
		}
		//////////////////////////////
		if(twoSec != oneSec) shortModeCounter++;
		if(ledMode == modeShort && shortModeCounter > 16) shortModeCounter = 0;
		if(ledMode == modeLong && shortModeCounter > 26) shortModeCounter = 0;
		if(ledMode == modeSpec && shortModeCounter > 26) shortModeCounter = 0;
		
		//Decide ledMode long or short period
		if(prevButtonCounter && hard_counter >= PushingTime) {
			ledMode = modeLong;
		//	shortModeCounter = 0;
		}
		if(buttonCounter && hard_counter < PushingTime && ledMode == modeLong) {
			ledMode = modeSpec;
		//	shortModeCounter = 0;
		}
		if(prevButtonCounter && hard_counter < PushingTime && ledMode == modeSpec) {
			ledMode = modeShort;
			shortModeCounter = 0;
		}
		
		//From short mode to long mode -> long mode led sequence
		if(prevLedMode == modeShort && ledMode == modeLong) {
			shortModeCounter = 0;
		}
		
		///////////////////////////
		//Measure pushing time
		if(buttonCounter) {
			hard_counter++;
			if(hard_counter >= PushingTime) {
				hard_counter = PushingTime;
				GPIOD->ODR |= (1ul << 12);
			}
		}
		else {
			hard_counter = 0;
			GPIOD->ODR &= ~(1ul << 12);
		}
		/////////////////////////////
//		//During long mode
//		if(ledMode == modeLong || ledMode == modeSpec) {
//			GPIOD->ODR &= ~(1ul << 13);
//			if(oneSec) {
//				GPIOD->ODR |= 1ul << 15;
//			}
//			else {
//				GPIOD->ODR &= ~(1ul << 15);
//			}
//		}
//		////////////////////////////
		//During long mode 1sec
		if(((ledMode == modeLong) || ledMode == modeSpec) && shortModeCounter <= 8) {
			here = 1;
			GPIOD->ODR &= ~(1ul << 13);
			if(oneSec) {
				GPIOD->ODR |= 1ul << 15;
			}
			else {
				GPIOD->ODR &= ~(1ul << 15);
			}
		}
		/////////////////////////////
		//During long mode 2sec
		if(((ledMode == modeLong) || ledMode == modeSpec) && (shortModeCounter > 8)) {
			here = 0;
			GPIOD->ODR &= ~(1ul << 13);
			if(twosecled) {
				GPIOD->ODR |= 1ul << 15;
			}
			else {
				GPIOD->ODR &= ~(1ul << 15);
			}
		}
		/////////////////////////////
		//During short mode 1sec
		if(ledMode == modeShort && shortModeCounter <= 4) {
			
			GPIOD->ODR &= ~(1ul << 15);
			
			if(oneSec) {
				GPIOD->ODR |= 1ul << 13;
			}
			else {
				GPIOD->ODR &= ~(1ul << 13);
			}
		}
		/////////////////////////////
		//During short mode 2sec
		if(ledMode == modeShort && shortModeCounter > 4 && shortModeCounter <= 13) {
			
			GPIOD->ODR &= ~(1ul << 15);
			
			if(twosecled) {
				GPIOD->ODR |= 1ul << 13;
			}
			else {
				GPIOD->ODR &= ~(1ul << 13);
			}
		}
		/////////////////////////////
		twoSec = oneSec;
		prevLedMode = ledMode;
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
	
	while(1) {
		//Detect user button
		if(UserButton) {
			delayMs(16800000);
			//Rising Edge
			GPIOD->ODR |= 1ul << 14;
			buttonCounter = 1;
			prevButtonCounter = 0;
			while(UserButton);
			//Falling Edege
			buttonCounter = 0;
			prevButtonCounter = 1;
			GPIOD->ODR &= ~(1ul << 14);
		}
		
	}
}
