#include <stm32f10x.h>

//buffer size for information transmitted to client
#define ClientTxBufSize 11
//buffer size for information recieved from client
#define ClientRxBufSize	11

void RS485_Init(void);
void RS485_TimerStart(void);
void RS485_TimerStop(void);
void SendMsgToClient(uint8_t *msg, uint8_t count);
void NVIC_init(void);
