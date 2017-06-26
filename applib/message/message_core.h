#ifndef _MESSAGE_CORE_H_
#define _MESSAGE_CORE_H_

#define SYN_SIGN		         0x55

#define VEHICLESTATUS            0x30
#define SYSCONTROL_RX            0x20
#define SYSCONTROL_TX            0x21

#define STANDBY                  0x10
#define WAKEUP			         0x11
#define CALIBRATION		         0x80
#define STARTUP                  0x81


#define CAMERA_ERR				 0x01
#define SYSTEM_ERR    			 0x02
#define ACTIVATE_ERR			 0x04
#define ADASINIT_ERR			 0x08




#define RESPONSE_BASIC_LEN         6
#define	RESPONSE_OK                0
#define RESPONSE_FINISHED          0
#define RESPONSE_WAIT              1
#define	RESPONSE_PROCESSING        1
#define RESPONSE_FAILD             2

#define LDWSTATUS	              0x40
#define FCWSTATUS		          0x41
#define LDW_MESSAGESIZE            13
#define FCW_MESSAGESIZE            11

#define VEHICLESTATUS_VALID        4
#define SYSCONTROL_RX_VALID        4

#define STATU_WAKEUP               0x11
#define STATU_STANDBY              0x10

#define SPEED_CONEFFICIENT         0.05625
#define STEERINGANGLE_CONEFFICIENT 0.1
#define LDW_WIDTH_FACTOR           0.01
#define LDW_DIS_FACTOR             0.01
#define FCW_DIS_FACTOR             0.01

#define u8  unsigned char
#define u16 unsigned short
#define u32 unsigned int

typedef struct{
	float speed;
	char  headlightstatus;
	char  ldwenabled;
	char  fcwenabled;
}VEHICLESTATUS_INFO;

typedef struct 
{
	int         Errorcode;
	int 		ldwCred;		//ÖÃÐÅ¶È£º0£º²»¿ÉÓÃ£»1£º×ó£»2£ºÓÒ
	float 		ldwDis;			//Ô½½ç¾àÀë
	float		ldwTtc;		    //Ô½½çÊ±¼ä
	float 		ldwWidth;
	float		ldwCurve;
	float 		ldwType;
}LDW_OUTINFO;

typedef struct 
{
	int  		Errorcode;
	int 		fcwCred;		//ÖÃÐÅ¶È£º0£º²»¿ÉÓÃ£»1£º¿ÉÓÃ
	float 		fcwDis;			//Ç°³µ¾àÀë
	float		fcwTtc;			//Ïà¶ÔÅö×²Ê±¼ä
	float		fcwAttc;		//¾ø¶ÔÅö×²Ê±¼ä
}FCW_OUTINFO;

typedef struct {
	u8 flag;
	u8 Seqnum;
	u8 Commad;
	u8 datalen;
	u8 *data;	
}SYS_CTRLINFO;

typedef struct 
{
	u8 	Seqnum;
	u8  Response;
	u8  datalen;
	u8  *data;
}SYS_RESPONSE;

typedef struct 
{
	LDW_OUTINFO  		ldw_info;
	FCW_OUTINFO	 		fcw_info;
	SYS_CTRLINFO 		ctrl_info;
	VEHICLESTATUS_INFO  vehicle_info;
}WARNNIG_CENTER;

typedef struct {
	u8 *addr;
	int len;
}BUFINFO;

int  messageInit(void);
void messageRelease(void);
void messageCreator(WARNNIG_CENTER center,int m,BUFINFO bufinfo);
void messageSend(char * buf,int len);
void getVehiclestatusInfo(VEHICLESTATUS_INFO *vehicleInfo);
int  issysctrlMsg(void);
int  processcmdMsg(SYS_CTRLINFO *ctrlinfo);

#endif
