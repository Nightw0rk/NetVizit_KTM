#include <stm32f10x.h>

#define	OWTXPORT	 	GPIOD
#define OWTXRCC 		RCC_APB2Periph_GPIOD
#define OWTX	 			GPIO_Pin_2

#define	OWRXPORT	 	GPIOA
#define OWRXRCC 		RCC_APB2Periph_GPIOA
#define OWRX				GPIO_Pin_11

//control pins for switching RD and BEEP
#define	OWSWPORT 	GPIOC
#define OWSWRCC 	RCC_APB2Periph_GPIOC
#define OWS0 			GPIO_Pin_0
#define OWS1 			GPIO_Pin_1

//control pin for BEEP control
#define	BEEPPORT 	GPIOB
#define BEEPRCC 	RCC_APB2Periph_GPIOB
#define BEEP 			GPIO_Pin_13


/*pin initialization*/
void OW_init(void);
/*ask control block to read key(one pulse 10us widt)*/
void OWOnLine(void);
/*read one byte from control block*/
uint8_t OWReadByte(void);
/*read key from panel*/
int OWReadKey(uint8_t *key);
/*control block wants to read key from panel, but we say that key is absent*/
int OWReadBlank(uint8_t *key);
/*wait for free KTM*/
void waitFreeKTM(void);
