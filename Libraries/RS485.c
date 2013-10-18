#include <stm32f10x.h>
#include "RS485.h"

volatile uint8_t ClientTxBuf[ClientTxBufSize];
volatile uint8_t ClientRxBuf[ClientRxBufSize];

void RS485_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	/*PB10 - TX OF USART3*/
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*PB11 - RX OF USART3*/
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/*USART3 INIT*/
	USART_InitStructure.USART_BaudRate=9600;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;
	USART_InitStructure.USART_StopBits=USART_StopBits_1;
	USART_InitStructure.USART_Parity=USART_Parity_No;
	USART_InitStructure.USART_Mode=USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_Init(USART3, &USART_InitStructure);
	
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
	/*Enable USART3*/
	USART_Cmd(USART3, ENABLE);
	/*Enable DMA*/
	USART_DMACmd(USART3, USART_DMAReq_Tx /*| USART_DMAReq_Rx*/, ENABLE);
	
	/* Enable USART IRQ */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable timer IRQ */
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
void RS485_TimerStart(void)
{
	RCC->APB1ENR|=RCC_APB1ENR_TIM7EN;
	TIM7->CR1|=TIM_CR1_OPM;//OPM=1  //one-pulse mode
	TIM7->CNT=0x0000;
	TIM7->PSC=0x17; //(24-1) for 24MHz
	TIM7->ARR=1;

	TIM7->CR1|=TIM_CR1_CEN;  //counter enable
	while(!(TIM7->SR & TIM_SR_UIF))
		;
	TIM7->SR&=~TIM_SR_UIF;
	
	/*Timer initialization for RS485 receive protection*/
	//RCC->APB1ENR|=RCC_APB1ENR_TIM7EN;
	TIM7->CR1|=TIM_CR1_OPM;			//OPM=1  (one-pulse mode)
	TIM7->DIER=TIM_DIER_UIE;		//update interrupt enable
	TIM7->CNT=0x0000;
	TIM7->PSC=0x17;			//(24-1) for 24MHz
	TIM7->ARR=5000;			//5ms
	
	TIM7->SR=0;//&=~TIM_SR_UIF;
	TIM7->CR1|=TIM_CR1_CEN;  //counter enable
}
void RS485_TimerStop(void)
{
	TIM7->CR1&=~TIM_CR1_CEN;  //counter disable
	TIM7->DIER&=~TIM_DIER_UIE;
}
/*send message to client*/
void SendMsgToClient(uint8_t *msg, uint8_t count)
{
	DMA_InitTypeDef  DMA_InitStructure;
	uint8_t i;
	
	/*reset buffer*/
	for(i=0;i<ClientTxBufSize;i++)
		ClientTxBuf[i]=0x00;
	/*fill the buffer with bytes to send*/
	for(i=0;i<count;i++)
		ClientTxBuf[i]=msg[i];
	
	/* DMA1 channel2 configuration */
	DMA_DeInit(DMA1_Channel2);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART3->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ClientTxBuf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = ClientTxBufSize;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);
	/* Enable DMA1 Channel2 Transfer Complete interrupt */
	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);		
	
	USART_ClearFlag(USART3,USART_FLAG_TC);
	DMA_Cmd(DMA1_Channel2,ENABLE);
	/*DMA will be disabled in interrupt*/
}
/*initialization of NVIC for other channels
	USART3_TX-CH2: RS485 transmit to client(master)
	USART3_RX-CH3: RS485 receive from client(master)
*/
void NVIC_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable DMA1 channel2 IRQ Channel */
	/*is enabled in ClientRecInit()*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	/* Enable DMA1 channel2 IRQ Channel */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
