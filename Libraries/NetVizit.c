#include <stm32f10x.h>
#include "NetVizit.h"
#include "delay.h"
#include "periph_init.h"
#include "RS485.h"

#define LOGON

#define LED_ON()		GPIO_SetBits(GPIOC,GPIO_Pin_8)
#define LED_OFF()		GPIO_ResetBits(GPIOC,GPIO_Pin_8)

#define EQUAL			1
#define NOT_EQUAL	0

volatile uint16_t VizitTxBuf[VizitTxBufSize];
volatile uint16_t VizitRxBuf[VizitRxBufSize]; 
volatile uint8_t ClientTxBuf[ClientTxBufSize];
volatile uint8_t ClientRxBuf[ClientRxBufSize];

const uint8_t frame_work[]={0x8C,0x48,0x41,0xA0,0x45,0x50,0xA5,0x54,0x45,0x20,0x48,0x4F,
	0x4D,0x45,0x50,0x3A,0x5F,0x01,0x01,0x30};   //НАБЕРИТЕ НОМЕР:_
const uint8_t frame_wait_def[]={0x8C,0xA3,0xE0,0xA5,0x54,0x45,0x20,0x4F,0x54,0x42,0x45,0x54,
	0x41,0x20,0x20,0x20,0x20,0x04,0x01,0x25};	//ЖДИТЕ ОТВЕТА
const uint8_t frame_wait[]={0x8C,0xA8,0x4F,0xE0,0x4F,0xA3,0xE0,0xA5,0x54,0x45,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x01,0x01,0xC9}; //ПОДОЖДИТЕ
uint8_t password[]={0x03,0x04,0x02,0x01};	//3421
uint8_t ent_to_prop[]={0x0C,0x09,0x09,0x09};	//#999

volatile uint8_t current_domofon=1;
volatile GPIO_TypeDef *port;
volatile uint16_t select0, select1;
/*delay for receiving message from BUD*/
volatile uint16_t receive_delay=5000;

void SetCurrentDomofon(uint8_t domofon)
{
	current_domofon=domofon;
	if(domofon==DOMOFON_1)
	{
		port=SWPORT_1;
		select0=S0_1;
		select1=S1_1;
	}
	if(domofon==DOMOFON_2)
	{
		port=SWPORT_2;
		select0=S0_2;
		select1=S1_2;
	}
}
/*switch RX line of USART2 to 5V*/
void RxTo5V(uint8_t domofon)
{
// 	if(domofon==DOMOFON_2)
// 	{	
		GPIO_ResetBits(RXPORT_2,A_2);
		GPIO_SetBits(RXPORT_2,B_2);
		GPIO_SetBits(RXPORT_2,C_2);
// 	}
// 	if(domofon==DOMOFON_1)
// 	{	
		GPIO_ResetBits(RXPORT_1,A_1);
		GPIO_SetBits(RXPORT_1,B_1);
		GPIO_SetBits(RXPORT_1,C_1);
// 	}
}
/*switch RX line of USART2 to not connected line*/
void RxToNC(uint8_t domofon)
{
	if(domofon==DOMOFON_2)
	{	
		GPIO_SetBits(RXPORT_2,A_2);
		GPIO_SetBits(RXPORT_2,B_2);
		GPIO_SetBits(RXPORT_2,C_2);
	}
	if(domofon==DOMOFON_1)
	{	
		GPIO_SetBits(RXPORT_1,A_1);
		GPIO_SetBits(RXPORT_1,B_1);
		GPIO_SetBits(RXPORT_1,C_1);
	}
}
/*switch RX line of USART2 to BUD*/
void RxToBUD(uint8_t domofon)
{
	if(domofon==DOMOFON_2)
	{	
		GPIO_SetBits(RXPORT_2,A_2);
		GPIO_ResetBits(RXPORT_2,B_2);
		GPIO_SetBits(RXPORT_2,C_2);
		RxToNC(DOMOFON_1);
	}
	if(domofon==DOMOFON_1)
	{	
		GPIO_SetBits(RXPORT_1,A_1);
		GPIO_ResetBits(RXPORT_1,B_1);
		GPIO_SetBits(RXPORT_1,C_1);
		RxToNC(DOMOFON_2);
	}
}
/*switch RX line of USART2 to BVD*/
void RxToBVD(uint8_t domofon)
{
	if(domofon==DOMOFON_2)
	{	
		GPIO_SetBits(RXPORT_2,A_2);
		GPIO_SetBits(RXPORT_2,B_2);
		GPIO_ResetBits(RXPORT_2,C_2);
		RxToNC(DOMOFON_1);
	}
	if(domofon==DOMOFON_1)
	{	
		GPIO_SetBits(RXPORT_1,A_1);
		GPIO_SetBits(RXPORT_1,B_1);
		GPIO_ResetBits(RXPORT_1,C_1);
		RxToNC(DOMOFON_2);
	}
}
/*Add new record at the table of flats*/
void AddFlat(uint8_t *block_num,uint8_t *door)
{
/*
	uint8_t n;
	EnterToProp();
	//send 8(adding)
	n=0x08;
	SendNumberToBUD(&n,1);
	SendNumberToBUD(block_num,4);	
	ReceiveMsgFromBUD();
	SendNumberToBUD(door,4);
	ReceiveMsgFromBUD();
	ExitFromProp(); 
*/  
	uint8_t n;
	uint8_t passwrd[3]={0x01,0x01,0x01};
	EnterToProp();
	//send 2(switch on)
	n=0x02;
	SendNumberToBUD(&n,1);
	SendNumberToBUD(door,4);	
	ReceiveMsgFromBUD();					//INDIVIDUALNIY KOD
	SendNumberToBUD(passwrd,3);		
	ReceiveMsgFromBUD();					//GOTOVO
	ReceiveMsgFromBUD();					//NOMER KVARTIRY
	ExitFromProp();   
}
/*Delete record from the table of flats*/
void DelFlat(uint8_t *door)
{
/*
	uint8_t n;
	EnterToProp();
	//send 9(deleting)
	n=0x09;
	SendNumberToBUD(&n,1);
	SendNumberToBUD(door,4);
	ReceiveMsgFromBUD();	//GOTOVO 				(TAKOGO NOMERA NET)
	ReceiveMsgFromBUD();	//STIRANIE N 		(NABERITE NOMER:_)
	ExitFromProp(); 
*/  
	uint8_t n;
	EnterToProp();
	//send 0(switch off)
	n=0x0A;
	SendNumberToBUD(&n,1);
	SendNumberToBUD(door,4);
	ReceiveMsgFromBUD();	//GOTOVO 
	ReceiveMsgFromBUD();	//N KVARTIRY
	ExitFromProp();  	
}
/*Add new key*/
void AddKey(uint8_t *key)
{
	uint8_t n;
	EnterToProp();
	//send 3(writing)
	n=0x03;
	SendNumberToBUD(&n,1);
	SendKeyToBUD(key+1,3);
	ReceiveMsgFromBUD();	//GOTOVO		(ZAPISAN RANEE)
	ReceiveMsgFromBUD();	//PRILOJI KLUCH
	ExitFromProp();   
}
/*Deleting key*/
void DelKey(uint8_t *key)
{
	uint8_t n;
	EnterToProp();
	//send 4(deleting)
	n=0x04;
	SendNumberToBUD(&n,1);
	SendKeyToBUD(key+1,3);
	ReceiveMsgFromBUD();	//STERTO		(TAKOGO KLUCHA NET)
	ReceiveMsgFromBUD();	//PRILOJI KLUCH
	ExitFromProp();   		
}
/*Call to flat for check*/
uint8_t CheckFLat(uint8_t *door)
{
	uint8_t i;
	uint8_t state=EQUAL;
	
	waitForFree();
	us_delay(1);
	/*domofon is free, let's start programming*/
	/*reconnect to BVD*/
	GPIO_ResetBits(port,select0);
	GPIO_ResetBits(port,select1);
	/*USART2_RX switch to BVD*/
	//RxToBVD();
	//ms_delay(50);
	/* send to display message:"__PODOJDITE___"  */
	SendMsgToDisp(frame_wait);
	//Delay(50);
	/*reconnect to BUD*/
	GPIO_SetBits(port,select0);
	GPIO_ResetBits(port,select1);
	/*USART2_RX switch to BUD*/
	//RxToBUD();
	/*send #999*/
	SendNumberToBUD(door,4);
	/*wait for message to display from BUD*/
	ReceiveMsgFromBUD();
	for(i=1;i<20;i++)
		if(VizitRxBuf[i]!=frame_wait_def[i])
		{
			state=NOT_EQUAL;
			break;
		}
	ExitFromProp();  
	return state; //equal=1;not_equal=0
}
/*waiting for domofon is free*/
void waitForFree(void)
{
	uint8_t i,query=0xAA,n=0;
	uint8_t state=EQUAL;
	
	/*chek for domofon is free*/
	ms_delay(1);
	GPIO_SetBits(GPIOC,GPIO_Pin_9);
	do
	{

		if(state==NOT_EQUAL)
		{
			ms_delay(1);
			/*connect BUD with BVD*/
			GPIO_SetBits(port,select0);
			GPIO_SetBits(port,select1);
			LED_ON();
			ms_delay(5000);
			LED_OFF();
		}
		state=EQUAL;
		/*USART2_RX switch to BUD*/
		//RxToBUD();
		/*reconnect to BUD*/
		GPIO_SetBits(port,select0);
		GPIO_ResetBits(port,select1);
		/*send 0xAA*/
		receive_delay=500;
		if(SendNumberToBUD(&query,1)!=0)
			state=NOT_EQUAL;
		receive_delay=5000;
		n++;
		for(i=1;i<20;i++)
			if(VizitRxBuf[i]!=frame_work[i])
			{
				state=NOT_EQUAL;
				break;
			}
	}
	while(state!=EQUAL);
	GPIO_ResetBits(GPIOC,GPIO_Pin_9);
}
/*send message to domofon display*/
void SendMsgToDisp(const uint8_t *msg)
{
	uint8_t i;
	uint16_t ch;
	DMA_InitTypeDef  DMA_InitStructure;
	do
	{	
		/*copy data for send to buffer*/
		for(i=0;i<VizitTxBufSize;i++)
			VizitTxBuf[i]=(i==0)?0x018C : msg[i];
		/* DMA1 channel7 configuration */
		//DMA_DeInit(DMA1_Channel7);
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)VizitTxBuf;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		DMA_InitStructure.DMA_BufferSize = VizitTxBufSize;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
		DMA_Init(DMA1_Channel7, &DMA_InitStructure);
		
		USART_ClearFlag(USART2,USART_FLAG_TC);
		DMA_Cmd(DMA1_Channel7,ENABLE);
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);
		DMA_Cmd(DMA1_Channel7,DISABLE);	
		/*clear REC buffer*/
		if(USART_GetFlagStatus(USART2,USART_FLAG_RXNE)==SET)
			ch=USART_ReceiveData(USART2);
		RxToBVD(current_domofon);
		/*wait for 0x01BB*/
		while(USART_GetFlagStatus(USART2,USART_FLAG_RXNE)!=SET);
		ch=USART_ReceiveData(USART2);
		RxTo5V(current_domofon);
  } while(ch!=0x01BB);
	
}
/*receive from BUD 20 bytes for BVD and send 0x1BB(ACK)*/
int ReceiveMsgFromBUD(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
//	uint16_t ch=0x00;

	/* DMA1 channel6 configuration */
	//DMA_DeInit(DMA1_Channel6);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)VizitRxBuf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = VizitRxBufSize;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);
	
	/*clear USART RX buffer*/
	if(USART_GetFlagStatus(USART2,USART_FLAG_RXNE)==SET)
		USART_ReceiveData(USART2);
	DMA_ClearFlag(DMA1_FLAG_TC6);
	RxToBUD(current_domofon);
	/*enable DMA*/
	DMA_Cmd(DMA1_Channel6,ENABLE);
	ms_delay(1);
	/*if symbols are not received for 10ms, DMA will be disabled*/
	TIM6->CNT=0x0000;
	TIM6->PSC=0x5DBF; 	//(24-1) for 24MHz
	TIM6->ARR=receive_delay;	//5s - usually,500ms - for answer to 0xAA (28160us for 20 symbols)
	TIM6->CR1|=TIM_CR1_CEN;  //counter enable
	/*wait for receiving all bytes*/
	while(DMA_GetFlagStatus(DMA1_FLAG_TC6)!=SET)
	{
		if(TIM6->SR & TIM_SR_UIF)
		{
			TIM6->SR&=~TIM_SR_UIF;
			TIM6->CR1&=~TIM_CR1_CEN;  //counter disable		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			DMA_Cmd(DMA1_Channel6,DISABLE);								//!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			RxTo5V(current_domofon);
			return 1;
		}
	}
	TIM6->CR1&=~TIM_CR1_CEN;  //counter disable
	DMA_Cmd(DMA1_Channel6,DISABLE);
	RxTo5V(current_domofon);
	/*send 0x01BB after receiving a frame*/
	USART_ClearFlag(USART2,USART_FLAG_TC);
 	USART_SendData(USART2,(0x01BB));
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);
	return 0;

// #ifdef LOGON
// 	SendMsgToClient(VizitRxBuf,20);
// #endif
}
/*not used*/
void ReceiveMsgFromBUD_circ(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	/* DMA1 channel6 configuration */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)VizitRxBuf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = VizitRxBufSize;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);
	/* Enable DMA1 Channel6 Transfer Complete interrupt */
	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
	
	/* Enable DMA1 channel6 IRQ Channel */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	if(USART_GetFlagStatus(USART2,USART_FLAG_RXNE)==SET)
		USART_ReceiveData(USART2);
	DMA_ClearFlag(DMA1_FLAG_TC6);
	DMA_Cmd(DMA1_Channel6,ENABLE);
}
/*send codes of buttons to BUD(emulation of pressing buttons)*/
int SendNumberToBUD(uint8_t *num,uint8_t button_count)
{
	uint8_t i;
	for(i=0;i<button_count;i++)
	{
		ms_delay(1);
		ms_delay(30);
		/*send msg[i], receive msg to display and send 0x1BB*/
		USART_ClearFlag(USART2,USART_FLAG_TC);
		USART_SendData(USART2,(0x0100+num[i]));
		if(ReceiveMsgFromBUD()!=0)
			return 1;
	}
	return 0;
}
/*send code of key to BUD(emulation of using key)*/
void SendKeyToBUD(uint8_t *num,uint8_t button_count)
{
	uint8_t i;
	for(i=0;i<button_count;i++)
	{
		/*send msg[i]*/
		USART_ClearFlag(USART2,USART_FLAG_TC);
		USART_SendData(USART2,num[i]);
		us_delay(6000);
	}
}
/*send #999 and password and output to display sevice message*/
void EnterToProp(void)
{
	waitForFree();
	us_delay(1);
	/*domofon is free, let's start programming*/
	/*reconnect to BVD*/
	GPIO_ResetBits(port,select0);
	GPIO_ResetBits(port,select1);
	/*USART2_RX switch to BVD*/
	//RxToBVD();
	//ms_delay(50);
	/* send to display message:"__PODOJDITE___"  */
	SendMsgToDisp(frame_wait);
	//Delay(50);
	/*reconnect to BUD*/
	GPIO_SetBits(port,select0);
	GPIO_ResetBits(port,select1);
	/*USART2_RX switch to BUD*/
	//RxToBUD();
	/*send #999*/
	SendNumberToBUD(ent_to_prop,4);
	/*wait for new message to display from BUD*/
	ReceiveMsgFromBUD();
	/*send password*/
	SendNumberToBUD(password,4);
	ReceiveMsgFromBUD();
}
/*send "*" for exiting and output to display standart message*/
void ExitFromProp(void)
{
	uint8_t n;
	/*   send "*"    */
	n=0x0B;
	SendNumberToBUD(&n,1);
	/*reconnect to BVD*/
	GPIO_ResetBits(port,select0);		//S0-L; S1-L
	GPIO_ResetBits(port,select1);
	/*USART2_RX switch to BVD*/
	//RxToBVD();
	/* send to dispaly message:"NABERITE NOMER:_" */
	SendMsgToDisp(frame_work);
	/*USART2_RX switch to +5V*/
	//RxTo5V();
	/*connect BUD with BVD*/
  GPIO_SetBits(port,select0);
	GPIO_SetBits(port,select1);
	GPIO_ResetBits(GPIOC,GPIO_Pin_8 | GPIO_Pin_9);
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
	
	RS485_TrOn();
	
	USART_ClearFlag(USART3,USART_FLAG_TC);
	DMA_Cmd(DMA1_Channel2,ENABLE);
	/*DMA will be disabled in interrupt*/
}
/*initialization of DMA and NVIC for recieving information from client (or server)*/
void ClientRecInit(void)
{
// 	DMA_InitTypeDef  DMA_InitStructure;
// 	NVIC_InitTypeDef NVIC_InitStructure;
// 	/* DMA1 channel3 configuration */
// 	DMA_DeInit(DMA1_Channel3);
// 	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART3->DR);
// 	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ClientRxBuf;
// 	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
// 	DMA_InitStructure.DMA_BufferSize = ClientRxBufSize;
// 	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
// 	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
// 	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
// 	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
// 	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
// 	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
// 	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
// 	DMA_Init(DMA1_Channel3, &DMA_InitStructure);
// 	/* Enable DMA1 Channel3 Transfer Complete interrupt */
// 	DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);
// 	
// 	/* Enable DMA1 channel3 IRQ Channel */
// 	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
// 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
// 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
// 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
// 	NVIC_Init(&NVIC_InitStructure);

// 	if(USART_GetFlagStatus(USART3,USART_FLAG_RXNE)==SET)
// 		USART_ReceiveData(USART3);
// 	DMA_ClearFlag(DMA1_FLAG_TC3);
// 	DMA_Cmd(DMA1_Channel3,ENABLE);
}
