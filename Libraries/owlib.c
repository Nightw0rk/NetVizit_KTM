#include <stm32f10x.h>
#include "OneWireLib.h"
// #define	OWTXPORT	 	GPIOD
// #define OWTXRCC 		RCC_APB2Periph_GPIOD
// #define OWTX	 			GPIO_Pin_2

// #define	OWRXPORT	 	GPIOA
// #define OWRXRCC 		RCC_APB2Periph_GPIOA
// #define OWRX				GPIO_Pin_11

#define PORT  GPIOС   //указать порт, к которому подключены датчики
#define TIMER TIM3    //задаем таймер, используемый для формирования задержек
 
//*********************************************************************************************
//function  импульс сброса                                                                   //
//argument  маска порта                                                                      //
//return    0 - устройство обнаружен, 1 - не обнаружено, 2 - к.з. на линии                   //
//*********************************************************************************************
uint8_t ds_reset_pulse(void)
{
	uint16_t result;   

	//if((OWRXPORT->IDR & OWRX)==0)  return 2;         //проверить линию на отсутствие замыкания
	TIMER->CNT=0;
	while(TIMER->CNT<140){}; 												//ждем 140мкс до ресета
	OWTXPORT->ODR |=  OWTX;                          //потянуть шину к земле
	TIMER->CNT=0;
	while(TIMER->CNT<480){};                        //ждать 480 микросекунд
	OWTXPORT->ODR &= ~OWTX;                          //отпустить шину
	while(TIMER->CNT<550){};                        //ждать 70 микросекунд
	result     =  OWRXPORT->IDR & OWRX;              //прочитать шину 
	while(TIMER->CNT<960){};                        //дождаться окончания инициализации
	if(result) return 1;                            //датчик не обнаружен
	return 0;                                       //датчик обнаружен      
}
void bad_fuction(void)
{
	//very bad fuction
}
/*
//*********************************************************************************************
//function  передача бита                                                                    //
//argument  значение передаваемого бита,маска порта                                          //
//return    none                                                                             //
//*********************************************************************************************
void ds_write_bit(uint8_t bit,uint16_t PinMask)
{
   TIMER->CNT=0;
   PORT->ODR &= ~PinMask;                          //потянуть шину к земле
   while(TIMER->CNT<2){};                          //ждать 1 микросекунду
   if(bit) PORT->ODR |=  PinMask;                  //если передаем 1, то отпускаем шину
   while(TIMER->CNT<60){};                         //задержка 60 микросекунд 
   PORT->ODR |=  PinMask;                          //отпускаем шину 
}

//*********************************************************************************************
//function  чтение бита                                                                      //
//argument  маска порта                                                                      //
//return    прочитанный бит                                                                  //
//*********************************************************************************************
uint16_t ds_read_bit(uint16_t PinMask)
{
   uint16_t result;
 
   TIMER->CNT=0;
   PORT->ODR &= ~PinMask;                          //потянуть шину к земле
   while(TIMER->CNT<2){};
   PORT->ODR |=  PinMask;                          //отпускаем шину  
   while(TIMER->CNT<15){};                         //задержка 15 микросекунд
   result     =  PORT->IDR & PinMask;              //прочитать шину
   while(TIMER->CNT<60){};                         //оставшееся время 
   return result;                                  //возвратить результат
}

//*********************************************************************************************
//function  запись байта                                                                     //
//argument  передаваемый байт,маска порта                                                    //
//return    none                                                                             //
//*********************************************************************************************
void ds_write_byte(uint8_t byte, uint16_t PinMask)
{
   uint8_t i;
   for(i=0;i<8;i++) ds_write_bit(byte&(1<<i), PinMask);
}
//*********************************************************************************************
//function  чтение байта                                                                     //
//argument  маска порта                                                                      //
//return    прочитанный байт                                                                 //
//*********************************************************************************************
uint8_t ds_read_byte(uint16_t PinMask)
{
   uint8_t i,result = 0;
   for(i=0;i<8;i++) 
   if(ds_read_bit(PinMask)) result |= 1<<i; 
   return result;
}
*/
void ow_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;   //подать тактирование на TIM3                           /
	TIM3->PSC     = 24-1;                  //настроить делитель для формирования микросекунд
	TIM3->ARR     = 1000; 
	TIM3->CR1     = TIM_CR1_CEN;
	
	//UC->RD-3, BEEP is connected to BEEP
	GPIO_ResetBits(OWSWPORT,OWS1);
	GPIO_SetBits(OWSWPORT,OWS0);
}
