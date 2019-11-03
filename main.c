/*
Author 		 : Mehmet KORAL
Date   		 : 30.10.2019
Email  		 : mehmet.koral96@gmail.com
Github 		 : https://github.com/mhmtkrl/ledTimer
MCU		 	   : STM32F407VGt6 with STM32F4 Discovery Board: https://www.st.com/en/evaluation-tools/stm32f4discovery.html
Peripherals: GPIOA -> PA0 is defined for USer button
						 GPIOD -> PD12(Yellow) is on when time is up(3 Sec)
									 -> PD13(Orange) is on for LED sequences
									 -> PD14(Red) is on when user button is pressed
						 TIM7  -> Timer7 was set for generating 1ms ISR
						 
*/

#include "stm32f4xx.h"                  						// Device header
#include "softTimer.h"

#define PushingTime 3000														//Button pressing time in ms
#define	UserButton	(GPIOA->IDR & 0x01)							//User Button -> PA0

void delayMs(uint32_t delay);												//Delay Function
void SystemFullSpeed(void);													//MCU is running on 168MHz with external OSC(8MHz)

uint16_t hard_counter = 0, soft_counter = 0;

uint8_t oneSec = 0, twoSec = 0, twosecled = 0;
uint8_t buttonCounter = 0, prevButtonCounter = 0;

int8_t shortModeCounter = 0;

uint32_t clock = 0;

uint8_t short_one_counter = 0, short_two_counter = 0;
uint8_t long_one_counter = 0, long_two_counter = 0;

//Led Modes
typedef enum{
	modeShort,																				//Mode Short
	modeLong,																					//Mode Long
	modeSpec																					//Mode Special -> from long to short
}modes;	

uint8_t ledMode = 0, prevLedMode = 0;

void TIM7_IRQHandler() {														//Timer7 1ms ISR function
	if(TIM7->SR) {
		//1sec interrupt software flag
		soft_counter++;
		if(soft_counter >= 1000) {
			oneSec = 1;
		}
		if(soft_counter >= 2000) {
			oneSec = 0;
			twosecled = 1;
		}
		if(soft_counter >= 3000) {
			oneSec = 1;
			twosecled = 1;
		}
		if(soft_counter >= 4000) {
			oneSec = 0;
			twosecled = 0;
			soft_counter = 0;
		}
		//////////////////////////////
		if(twoSec != oneSec) shortModeCounter++;				//0.5 sec
		if(ledMode == modeShort && shortModeCounter >= 14) shortModeCounter = 0;
		if(ledMode == modeLong && shortModeCounter >= 26) shortModeCounter = 0;
		if(ledMode == modeSpec && shortModeCounter >= 26) shortModeCounter = 0;
		
		//Decide ledMode ? long or short period
		if(prevButtonCounter && hard_counter >= PushingTime) {
			ledMode = modeLong;
		}
		if(buttonCounter && hard_counter < PushingTime && ledMode == modeLong) {
			ledMode = modeSpec;
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
	
  	////////////////////////////
		//During long mode -> 1sec
		if(((ledMode == modeLong) || ledMode == modeSpec) && shortModeCounter <= 8) {
			GPIOD->ODR &= ~(1ul << 15);
			if(oneSec) {
				GPIOD->ODR |= 1ul << 13;
			}
			else {
				GPIOD->ODR &= ~(1ul << 13);
			}
		}
		/////////////////////////////
		//During long mode -> 2sec
		if(((ledMode == modeLong) || ledMode == modeSpec) && (shortModeCounter > 8) && shortModeCounter < 24) {
			GPIOD->ODR &= ~(1ul << 15);
			if(twosecled) {
				GPIOD->ODR |= 1ul << 13;
			}
			else {
				GPIOD->ODR &= ~(1ul << 13);
			}
		}
		/////////////////////////////
		//During short mode -> 1sec
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
		//During short mode -> 2sec
		if(ledMode == modeShort && shortModeCounter > 4 && shortModeCounter <= 12) {
			
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
	TIM7->PSC = 83 ;
	TIM7->ARR = 999 ;      
  NVIC->ISER[1] = 0x00800000;  											//TIM7 interrupt -> enable
	
	while(1) {
		//Detect user button
		if(UserButton) {
			delayMs(16800000);															//100ms debounce time
			//Rising Edge for User button
			GPIOD->ODR |= 1ul << 14;
			buttonCounter = 1;
			prevButtonCounter = 0;
			while(UserButton);
			//Falling Edege for User button
			buttonCounter = 0;
			prevButtonCounter = 1;
			GPIOD->ODR &= ~(1ul << 14);
		}
		
	}
}
