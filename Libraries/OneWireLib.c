#include <stm32f10x.h>
#include "OneWireLib.h"
#include "delay.h"

#define LED_ON()		GPIO_SetBits(GPIOC,GPIO_Pin_8)
#define LED_OFF()		GPIO_ResetBits(GPIOC,GPIO_Pin_8)

void OW_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(OWTXRCC | OWRXRCC | OWSWRCC | BEEPRCC | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
	
	/*JTAG disable (PB4 is used in this program and in JTAG)*/
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	
  GPIO_InitStructure.GPIO_Pin =  OWTX;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
 	GPIO_Init(OWTXPORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  OWRX;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
 	GPIO_Init(OWRXPORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  OWS0 | OWS1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
 	GPIO_Init(OWSWPORT, &GPIO_InitStructure);
	/*default parameters (panel->control block)*/
	GPIO_SetBits(OWSWPORT,OWS1);
	GPIO_ResetBits(OWSWPORT,OWS0);
	/*1-wire line in pull-up mode*/
	GPIO_ResetBits(OWTXPORT,OWTX);
	
	GPIO_InitStructure.GPIO_Pin =  BEEP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
 	GPIO_Init(BEEPPORT, &GPIO_InitStructure);
	GPIO_ResetBits(BEEPPORT,BEEP);
}
void OWOnLine(void)
{
	us_delay(1);
	/*say that I am on line*/
	GPIO_SetBits(OWTXPORT,OWTX);
	us_delay(5);//10us
	GPIO_ResetBits(OWTXPORT,OWTX);
}
uint8_t OWReadByte(void)
{
	uint8_t i,rec_byte=0;
	for (i=0;i<8;i++)
	{
		while (GPIO_ReadInputDataBit(OWRXPORT,OWRX)!=Bit_RESET);
		us_delay(10);//15us
		LED_ON();
		if(GPIO_ReadInputDataBit(OWRXPORT,OWRX)==Bit_SET)
			rec_byte|=(1<<i);
		else
			us_delay(45);//50us
		LED_OFF();
	}
	return rec_byte;
}
int OWReadKey(uint8_t *key)
{
	uint8_t i,rec_byte=0x00;
	
	us_delay(1);//init delay function

	/*wait for BU(master) RESET*/
	while (GPIO_ReadInputDataBit(OWRXPORT,OWRX)!=Bit_RESET);
	us_delay(400);
	while (GPIO_ReadInputDataBit(OWRXPORT,OWRX)!=Bit_SET);
	us_delay(40);
	/*wait for slave PRESENCE*/
	while (GPIO_ReadInputDataBit(OWRXPORT,OWRX)!=Bit_RESET);
	us_delay(70);
	while (GPIO_ReadInputDataBit(OWRXPORT,OWRX)!=Bit_SET);
	us_delay(150);//250
	/*read command from master*/
	rec_byte=OWReadByte();
	/*check if command is READ_ROM*/
	if(rec_byte!=0x33)
		return 1;
	/*read key code*/
	for (i=0;i<8;i++)
		key[i]=OWReadByte();
	return 0;
}
int OWReadBlank(uint8_t *key)
{
	uint8_t i,rec_byte=0x00;
	
	us_delay(1);//init delay function

	/*wait for BU(master) RESET*/
	while (GPIO_ReadInputDataBit(OWRXPORT,OWRX)!=Bit_RESET);
	us_delay(400);
	while (GPIO_ReadInputDataBit(OWRXPORT,OWRX)!=Bit_SET);
	us_delay(350);	//??? 
	/*read command from master*/
	rec_byte=OWReadByte();
	/*check if command is READ_ROM*/
	if(rec_byte!=0x33)
		return 1;
	/*read key code*/
	for (i=0;i<8;i++)
		key[i]=OWReadByte();
	return 0;
}
void waitFreeKTM(void)
{
	uint8_t isReadyKTM=0,lineStateLast,lineStateNew;
	
	ms_delay(1);
	//check for KTM is free
	GPIO_SetBits(GPIOC,GPIO_Pin_9);
	/*if symbols are not received for 10ms, DMA will be disabled*/
	TIM6->CNT=0x0000;
	TIM6->PSC=0x5DBF; 	//(24000-1) for 24MHz
	TIM6->ARR=7000;			//5s - usually,500ms - for answer to 0xAA (28160us for 20 symbols)
	TIM6->CR1|=TIM_CR1_CEN;  //counter enable

	lineStateLast=GPIO_ReadInputDataBit(OWRXPORT,OWRX);
	while(isReadyKTM==0)
	{
		lineStateNew=GPIO_ReadInputDataBit(OWRXPORT,OWRX);
		if (lineStateLast!=lineStateNew)
		{
			TIM6->CNT=0x0000;
			lineStateLast=lineStateNew;
		}
		if(TIM6->SR & TIM_SR_UIF)
		{
			TIM6->SR&=~TIM_SR_UIF;
			TIM6->CR1&=~TIM_CR1_CEN;  //counter disable		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			isReadyKTM=1;
		}
	}
	TIM6->CR1&=~TIM_CR1_CEN;  //counter disable
	GPIO_ResetBits(GPIOC,GPIO_Pin_9);
}
