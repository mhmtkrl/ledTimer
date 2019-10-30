#ifndef SOFT_TIMER_H
#define SOFT_TIMER_H

#include <stdint.h>
#include <stdbool.h> 

#define TIMER_COUNT			2		// istenilen soft counter sayisi
//
#define TIMER_A				0		// örnek timerlar
#define TIMER_B				1

void SoftTimer_Init(void);
void SoftTimer_ISR(void); 
void SoftTimer_SetTimer(uint8_t TimerNo, uint32_t SetTimeMs);
bool SoftTimer_GetTimerStatus(uint8_t TimerNo);
void SoftTimer_ResetTimer(uint8_t TimerNo);

#endif
