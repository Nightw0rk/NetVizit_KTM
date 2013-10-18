#include <stm32f10x.h>
#include "OneWireLib.h"
#include "RS485.h"
#include "delay.h"

#define ADD_KEY 2
#define PROCESS	4
#define SEND_STAT		6
#define SEND_ANSWER	7

#define LED_ON()		GPIO_SetBits(GPIOC,GPIO_Pin_8)
#define LED_OFF()		GPIO_ResetBits(GPIOC,GPIO_Pin_8)

extern volatile uint8_t ClientTxBuf[];
extern volatile uint8_t ClientRxBuf[];
volatile uint8_t report[ClientTxBufSize];

volatile uint8_t dev_address=0x00;
volatile uint8_t buf_index=0, flags=0x00;
volatile uint8_t key[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

void NewCommandHandler(void);
void start_process(void);
void stop_process(void);

int main(void)
{
	uint8_t i;
	GPIO_InitTypeDef GPIO_InitStructure; //______for debug_________
	
	/*Enable DMA*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	RS485_Init();
	NVIC_init();			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	OW_init();
	__enable_irq();

	/*PC8 and PC9 - LEDs*/
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC,GPIO_Pin_8 | GPIO_Pin_9);
	/*PA0 - USER button*/
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	LED_ON();
	us_delay(1);	//to init
	ms_delay(1);	//timers (i dont know why timers dont work properly at first calling)
	//LED_OFF();

	while(1)
	{		
		//****************************************//
		//main part of this cycle									//
		//****************************************//
		OWReadKey(key);
		report[0]=dev_address;
		report[1]=0x22;
		for	(i=2;i<10;i++)
				report[i]=key[i-2];
		report[10]=0x13;
		SendMsgToClient(report,11);
		//****************************************//
		//end of main part of this cycle					//
		//****************************************//
		
		if( flags & (1<<SEND_ANSWER) )
		{
			SendMsgToClient(ClientRxBuf,11);
			flags&=~(1<<SEND_ANSWER);
		}
		
		if ( (flags & (1<<ADD_KEY)) && (!(flags & (1<<SEND_ANSWER))) )
		{
			uint8_t j;
			start_process();	
			
			OWReadKey(key);
			report[0]=dev_address;
			report[1]=0x22;
			for	(j=2;j<10;j++)
					report[j]=key[j-2];
			report[10]=0x13;
			SendMsgToClient(report,11);
			
			flags&=~(1<<ADD_KEY);
			stop_process();
		}	
		
		if ( (flags & (1<<SEND_STAT)) && (!(flags & (1<<SEND_ANSWER))) )
		{
			start_process();
			
			/**add code here**/
			
			flags&=~(1<<SEND_STAT);
			stop_process();
		}
		
		if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)==Bit_SET)
		{
			/*button handler*/
			ms_delay(1);
			ms_delay(1000);
			LED_OFF();
	
			/**add code here**/
			
			LED_ON();
		}
		
	}
}
	
void TIM7_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM7,TIM_IT_Update)==SET)
	{
		buf_index=0;
		TIM_ClearITPendingBit(TIM7,TIM_IT_Update);
	}
}
void USART3_IRQHandler(void)
{
	if(USART_GetITStatus(USART3,USART_IT_RXNE)==SET)
	{
		RS485_TimerStart();
		ClientRxBuf[buf_index]=(uint8_t)USART_ReceiveData(USART3);
		buf_index++;
		if(buf_index==ClientRxBufSize)
		{
			RS485_TimerStop();
			buf_index=0;
			/*new command from client has been received*/	
			NewCommandHandler();
		}
	}
}
void DMA1_Channel2_IRQHandler(void)
{
	/*result of operation has been transmitted to client*/
	/*wait for transmitting of the last byte*/
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);
	/*disable DMA channel 2*/
	DMA_Cmd(DMA1_Channel2,DISABLE);
  /* Clear DMA1 Channel2 Half Transfer, Transfer Complete and Global interrupt pending bits */
  DMA_ClearITPendingBit(DMA1_IT_TC2);
}
void NewCommandHandler(void)
{
	uint8_t j;
	
	/*	ClienRxBuf[0]:	*/
	/*	|X|X|X|X|ADDR3|ADDR2|ADDR1|ADDR0|		*/
	
	/*if address is the same, it's our message and the last byte must be equal 0x13*/
	if(  (ClientRxBuf[0]==dev_address) && (ClientRxBuf[10]==0x13) )
	{
		flags|=1<<SEND_ANSWER;
		switch (ClientRxBuf[1])
		{
			case 0x22:
			/*команда: записать ключ*/
				for	(j=2;j<10;j++)
					key[j-2]=ClientRxBuf[j];
				flags|=1<<ADD_KEY;	//флаг добавления ключа
				break;
			case 0x24:
			/*send status*/
				flags|=1<<SEND_STAT;
				break;
			default:
				/*обнулить все*/
				flags=0;
				break;
		}
	}
}
void start_process(void)
{
	flags|=1<<PROCESS;
	LED_OFF();
	__disable_irq();
}
void stop_process(void)
{
	flags&=~(1<<PROCESS);
	__enable_irq();
	LED_ON();
}
