#include "../../proj/tl_common.h"
#include "../../proj_lib/blt_ll/blt_ll.h"
#include "ui.h"


#define FW_VERSION_ID1 0x01
#define FW_VERSION_ID2 0x10

#define FW_Soft_VERSION_ID1 00
#define FW_Soft_VERSION_ID2 01

typedef struct
{
  /** Minimum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  u16 intervalMin;
  /** Maximum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  u16 intervalMax;
  /** Number of LL latency connection events (0x0000 - 0x03e8) */
  u16 latency;
  /** Connection Timeout (0x000A - 0x0C80 * 10 ms) */
  u16 timeout;
} gap_periConnectParams_t;

const u16 clientCharacterCfgUUID = GATT_UUID_CLIENT_CHAR_CFG;

//const u16 extReportRefUUID = GATT_UUID_EXT_REPORT_REF;

const u16 reportRefUUID = GATT_UUID_REPORT_REF;

//const u16 characterPresentFormatUUID = GATT_UUID_CHAR_PRESENT_FORMAT;

const u16 my_primaryServiceUUID = GATT_UUID_PRIMARY_SERVICE;

const u16 my_characterUUID = GATT_UUID_CHARACTER;

const u16 my_devServiceUUID = SERVICE_UUID_DEVICE_INFORMATION;

const u16 my_PnPUUID = CHARACTERISTIC_UUID_PNP_ID;

const u16 my_devNameUUID = GATT_UUID_DEVICE_NAME;

const u16 my_serviceChangeUUID = GATT_UUID_SERVICE_CHANGE;

const u16 my_appearanceUIID = 0x2a01;
const u16 my_periConnParamUUID = 0x2a04;


const u16 my_gattServiceUUID = SERVICE_UUID_GENERIC_ATTRIBUTE; //0x1801

extern u8 tbl_adv[];



const u8 PROP_READ = CHAR_PROP_READ;

const u8 PROP_WRITE = CHAR_PROP_WRITE;

const u8 PROP_INDICATE = CHAR_PROP_INDICATE;

const u8 PROP_WRITE_NORSP = CHAR_PROP_WRITE_WITHOUT_RSP;

const u8 PROP_READ_NOTIFY = CHAR_PROP_READ | CHAR_PROP_NOTIFY;

const u8 PROP_READ_WRITE_NORSP = CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP;

const u8 PROP_READ_WRITE_WRITENORSP = CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP;

const u8 PROP_READ_WRITE = CHAR_PROP_READ|CHAR_PROP_WRITE;

const u8 PROP_READ_WRITE_NORSP_NOTIFY = CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP|CHAR_PROP_NOTIFY;

const u8    my_PnPtrs [] = {0x02, 0x12, 0x34, FW_Soft_VERSION_ID1, FW_Soft_VERSION_ID2, FW_VERSION_ID2, FW_VERSION_ID1};

//const u8    my_PnPtrs [] = {0x02, 0x5e, 0x04, 0x8e, 0x02, FW_VERSION_ID2, FW_VERSION_ID1};

u16 serviceChangeVal[4] = {0};
static u8 serviceChangeCCC[2]={0,0};

const u16 my_gapServiceUUID = SERVICE_UUID_GENERIC_ACCESS;
const u16 my_appearance = GAP_APPEARE_ROLE;//global
const gap_periConnectParams_t my_periConnParameters = {30, 60, 4, 1000};

extern u8 os_check;
u8* att_get_reportMap(){
	return 0;
}
int att_get_reportMapSize(){
	return 0;
	}

const u8 ota_service_uuid[16] = {0x11,0x19,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00};
const u8 ota_write_char_uuid[16] ={0x12,0x2B,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00} ;

u8 ota_data[20];

////////////////////////Private Service//////////////////////////////////
static const u16 FFE0_UUID = 0xffe0;
static const u16 FFE1_charUUID = 0xffe1;
static const u8  FFE1_prop = CHAR_PROP_READ | CHAR_PROP_NOTIFY;
u8 FFE1_value[1] = {0x00};

//////////////////////// TX Power Service//////////////////////////////////
static const u16 TxPower_serviceUUID  = SERVICE_UUID_TX_POWER;
static const u8 TxPower_prop =CHAR_PROP_READ;
u8 Txpower_value = 7;    //TX_POWER_MAX;

//////////////////////// linkLoss Service  //////////////////////////////////
static const u16 linkLoss_serviceUUID  = SERVICE_UUID_LINK_LOSS;
static const u8 linkLoss_prop = CHAR_PROP_READ | CHAR_PROP_WRITE;//CHAR_PROP_WRITE | CHAR_PROP_NOTIFY;
u8 linkLoss_value = 60;
u8 linkLoss_valueInCCC[2];

//////////////////////// Immediate Alert Service//////////////////////////////////
static const u16 immediateAlert_serviceUUID  = SERVICE_UUID_IMMEDIATE_ALERT;
static const u8 immediateAlertLevel_prop = CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP;//CHAR_PROP_WRITE | CHAR_PROP_NOTIFY;
u8 immediateAlertLevel_value = 0;
u8 immediateAlertLevel_valueInCCC[2];

const u16 my_batServiceUUID       			= SERVICE_UUID_BATTERY;
const u16 my_batCharUUID       				= CHARACTERISTIC_UUID_BATTERY_LEVEL;
u8 my_batVal 						= {100};
u8  generalValInCCC[2];

const u16 my_2a06_UIID = 0x2a06 ;
const u16 my_2a07_UUID = 0x2a07 ;

const attribute_t my_Attributes[] = 
{
//DFSADF
	{12,0,0,0,0}, //

/**************************************************************************
	1						Battery (Services)
***************************************************************************/
	{4,2,2,	(u8*)(&my_primaryServiceUUID),     (u8*)(&my_batServiceUUID)},
	{0,2,1,	(u8*)(&my_characterUUID),          (u8*)(&PROP_READ_NOTIFY)},	  //prop
	{0,2,1,	(u8*)(&my_batCharUUID),            (u8*)(&my_batVal)}, //value
	{0,2,2,	(u8*)(&clientCharacterCfgUUID),    (u8*)(generalValInCCC)},  //value

/**************************************************************************
	5						Immediate Alert  (Services)
***************************************************************************/
	{3,2,2,(u8*)(&my_primaryServiceUUID),	(u8*)(&immediateAlert_serviceUUID)},
	{0,2,1,(u8*)(&my_characterUUID), 		(u8*)(&immediateAlertLevel_prop)},
	{0,2,1,(u8*)(&my_2a06_UIID),	        (u8*)(&immediateAlertLevel_value)},
/**************************************************************************
	8						Private  (Services)
***************************************************************************/
	{4,2,2,(u8*)(&my_primaryServiceUUID),	(u8*)(&FFE0_UUID)},
	{0,2,1,(u8*)(&my_characterUUID), 		(u8*)(&FFE1_prop)},
	{0,2,1,(u8*)(&FFE1_charUUID),	        (u8*)(&FFE1_value)},
	{0,2,sizeof(generalValInCCC),(u8*)(&clientCharacterCfgUUID),    (u8*)(generalValInCCC)},  //value

};

void    shutter_att_init ()
{
    /*blt_set_att_table ((u8 *)my_Attributes);*/
    extern attribute_t* gAttributes;
    gAttributes = (attribute_t *)my_Attributes;

//	blt_smp_set_pairing_type(1,0);
//  blt_smp_func_init ();
}
