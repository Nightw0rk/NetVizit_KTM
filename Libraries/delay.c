#include "delay.h"

/*delay for milliseconds*/
void ms_delay(uint16_t time)
{
	RCC->APB1ENR|=RCC_APB1ENR_TIM6EN;
	TIM6->CR1|=TIM_CR1_OPM;//OPM=1  //one-pulse mode
	TIM6->CNT=0x0000;
	TIM6->PSC=0x5DBF; //(24000-1) for 24MHz
	TIM6->ARR=time;

	TIM6->CR1|=TIM_CR1_CEN;  //counter enable
	while(!(TIM6->SR & TIM_SR_UIF))
		;
	TIM6->SR&=~TIM_SR_UIF;
} 
/*delay for microseconds*/
void us_delay(uint16_t time)
{
	RCC->APB1ENR|=RCC_APB1ENR_TIM6EN;
	TIM6->CR1|=TIM_CR1_OPM;//OPM=1  //one-pulse mode
	TIM6->CNT=0x0000;
	TIM6->PSC=0x17; //(24-1) for 24MHz
	TIM6->ARR=time;

	TIM6->CR1|=TIM_CR1_CEN;  //counter enable
	while(!(TIM6->SR & TIM_SR_UIF))
		;
	TIM6->SR&=~TIM_SR_UIF;
} 
