#define DOMOFON_1		2
#define DOMOFON_2		1
#define KTM					0

/**********DOMOFON_2**********/
//control pins for switching destination of transmitting data
#define	SWPORT_2 	GPIOB
#define S0_2 			GPIO_Pin_4
#define S1_2 			GPIO_Pin_5
//control pins for switching source of receiving data
#define RXPORT_2 	GPIOB
#define A_2 			GPIO_Pin_6
#define B_2 			GPIO_Pin_7
#define C_2 			GPIO_Pin_8

/**********DOMOFON_1**********/
//control pins for switching destination of transmitting data
#define	SWPORT_1 	GPIOC
#define S0_1 			GPIO_Pin_2
#define S1_1 			GPIO_Pin_3
//control pins for switching source of receiving data
#define RXPORT_1 	GPIOC
#define A_1 			GPIO_Pin_10
#define B_1 			GPIO_Pin_11
#define C_1		 		GPIO_Pin_12

//buffer size for information transmitted from device
#define VizitTxBufSize 20
//buffer size for information recieved by device
#define VizitRxBufSize 20
//buffer size for information transmitted to client
#define ClientTxBufSize 11
//buffer size for information recieved from client
#define ClientRxBufSize	11

void SetCurrentDomofon(uint8_t domofon);
/*switch RX line to 5V*/
void RxTo5V(uint8_t domofon);
/*switch RX line to not connected line*/
void RxToNC(uint8_t domofon);
/*switch RX line to BUD*/
void RxToBUD(uint8_t domofon);
/*switch RX line to BVD*/
void RxToBVD(uint8_t domofon);
/*Add new record at the table of flats*/
void AddFlat(uint8_t *block_num,uint8_t *door);
/*Delete record from the table of flats*/
void DelFlat(uint8_t *door);
/*Add new key*/
void AddKey(uint8_t *key);
/*Deleting key*/
void DelKey(uint8_t *key);
/*Call to flat for check*/
uint8_t CheckFLat(uint8_t *door);
/*waiting for domofon is free*/
void waitForFree(void);
/*send new message to display on BVD*/
void SendMsgToDisp(const uint8_t *msg);
/*receive from BUD 20 bytes for BVD and send 0x1BB(ACK)*/
int ReceiveMsgFromBUD(void);
void ReceiveMsgFromBUD_circ(void);
/*send codes of buttons to BUD(emulation of pressing buttons)*/
int SendNumberToBUD(uint8_t *num,uint8_t button_count);
/*send code of key to BUD(emulation of using key)*/
void SendKeyToBUD(uint8_t *num,uint8_t button_count);
/*send #999 and password with and to display sevice message*/
void EnterToProp(void);
/*send "*" for exiting and output to display standart message*/
void ExitFromProp(void);
/*send message to client*/
void SendMsgToClient(uint8_t *msg, uint8_t count);
/*initialization of DMA and NVIC for recieving information from client*/
void ClientRecInit(void);
