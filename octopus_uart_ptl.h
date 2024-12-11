/** ****************************************************************************
 * @copyright Copyright (c) XXX
 * All rights reserved.
 *
 *
 */
#ifndef __OCTOPUS_TASK_MANAGER_COM_UART_PTL_H__
#define __OCTOPUS_TASK_MANAGER_COM_UART_PTL_H__

/*******************************************************************************
 * INCLUDES
 */
 
#include "octopus_platform.h"

#ifdef __cplusplus
extern "C"{
#endif

/*******************************************************************************
 * DEBUG SWITCH MACROS
 */

/*******************************************************************************
 * MACROS
 */
#define PTL_UART_CHANNEL            		1
#define PTL_FRAME_HEADER_SIZE           5
#define PTL_FRAME_MIN_SIZE              7
#define PTL_FRAME_MAX_SIZE              255
#define PTL_FRAME_DATA_START          	5
#define PTL_HEADER             				  0xFA

/*******************************************************************************
 * TYPEDEFS
 */
typedef enum UartChannel
{
  UartChannel_Uart0 = 0,
  UartChannel_Uart1 = 1,
	UartChannel_Uart2 = 2,
} UartChannel;

typedef enum{
    /*MCU -> APP*/
    M2A_PTL_HEADER    = 0x55,        //(MCU -> APP)FRAME HEADER
    /*APP -> MCU*/
    A2M_PTL_HEADER    = 0xAA,        //(APP -> MCU)FRAME HEADER
}ptl_frame_header_t;

typedef enum{ //module id
    /*MCU -> APP*/
    M2A_MOD_SYSTEM    = 0x00,        //(MCU -> APP)ϵͳ��ʼ��
    M2A_MOD_UPDATE    = 0x01,        //(MCU -> APP)ϵͳ����
    M2A_MOD_TRANSFER  = 0x02,        //(MCU -> APP)����͸��
    M2A_MOD_METER     = 0x03,        //(MCU -> APP)�Ǳ�
    M2A_MOD_INDICATOR = 0x04,        //(MCU -> APP)ָʾ��
    M2A_MOD_DRIV_INFO = 0x05,        //(MCU -> APP)�г���Ϣ
    M2A_MOD_SETUP     = 0x06,        //(MCU -> APP)����
    /*APP -> MCU*/
    A2M_MOD_SYSTEM    = 0x80,        //(APP -> MCU)ϵͳ��ʼ��
    A2M_MOD_UPDATE    = 0x81,        //(APP -> MCU)ϵͳ����
    A2M_MOD_TRANSFER  = 0x82,        //(APP -> MCU)����͸��
    A2M_MOD_METER     = 0x83,        //(APP -> MCU)�Ǳ�
    A2M_MOD_INDICATOR = 0x84,        //(APP -> MCU)ָʾ��
    A2M_MOD_DRIV_INFO = 0x85,        //(APP -> MCU)�г���Ϣ
    A2M_MOD_SETUP     = 0x86,        //(APP -> MCU)����

    /*�����ж�MODULE����Ч��*/
    /*MCU -> APP*/
    M2A_MOD_START   = M2A_MOD_SYSTEM,
    M2A_MOD_END     = M2A_MOD_SETUP,
    /*APP -> MCU*/
    A2M_MOD_START   = A2M_MOD_SYSTEM,
    A2M_MOD_END     = A2M_MOD_SETUP,
}ptl_frame_type_t;

typedef enum{
    /*MOD_SYSTEM*/
    CMD_MODSYSTEM_HANDSHAKE         = 0x00,      //ϵͳ����
    CMD_MODSYSTEM_ACC_STATE         = 0x01,      //ACC״̬
    CMD_MODSYSTEM_APP_STATE         = 0x02,      //Ӧ�û���״̬
    CMD_MODSYSTEM_POWER_ON          = 0x03,       //��Դ
	CMD_MODSYSTEM_POWER_OFF         = 0x04,      //�رյ�Դ

    /*MOD_UPDATE*/
    CMD_MODUPDATE_CHECK_FW_STATE    = 0x06,      //��ѯ�̼�״̬
    CMD_MODUPDATE_UPDATE_FW_STATE   = 0x07,      //�̼�״̬����
    CMD_MODUPDATE_ENTER_FW_UPDATE   = 0x08,      //����̼�����
    CMD_MODUPDATE_EXIT_FW_UPDATE    = 0x09,      //��ɹ̼�����
    CMD_MODUPDATE_SEND_FW_DATA      = 0x0A,      //���͹̼�����
    CMD_MODUPDATE_REBOOT            = 0x0B,      //ϵͳ��λ
    /*MOD_TRANSFER*/
    CMD_MODTRANSFER_A2M             = 0x0C,      //A2M����͸��
    CMD_MODTRANSFER_M2A             = 0x0D,      //M2A����͸��
    /*MOD_METER*/
    CMD_MODMETER_RPM_SPEED          = 0x0E,      //ת�ٱ�/���ٱ�
    CMD_MODMETER_FUEL_TEMPTER       = 0x0F,      //ȼ�ͱ�/ˮ�±�
    CMD_MODMETER_SOC                = 0x10,      //SOC��
    /*MOD_INDICATOR*/
    CMD_MODINDICATOR_INDICATOR      = 0x11,      //ָʾ��
    CMD_MODINDICATOR_ERROR_INFO     = 0x12,      //������Ϣ
    /*MOD_DRIV_INFO*/
    CMD_MODDRIVINFO_ODO             = 0x14,      //��ʻ���
    CMD_MODDRIVINFO_DRIV_DATA       = 0x15,      //��ʻ����
    CMD_MODDRIVINFO_GEAR            = 0x16,      //��λ��Ϣ
    CMD_MODDRIVINFO_NAVI            = 0x17,      //���׵���
    CMD_MODDRIVINFO_DRIV_DATA_CLEAR = 0x18,      //��ʻ���ݸ�λ
    /*MOD_SETUP*/
    CMD_MODSETUP_UPDATE_TIME        = 0x19,      //ʱ�����
    CMD_MODSETUP_SET_TIME           = 0x1A,      //����ʱ��
    CMD_MODSETUP_KEY                = 0x1B,      //����
	
}ptl_frame_cmd_t;

typedef struct
{
    uint16_t 						size;
    uint8_t 						buff[PTL_FRAME_MAX_SIZE];
}ptl_proc_buff_t;

typedef struct{
    ptl_frame_type_t 		frame_type;
    ptl_frame_cmd_t 		cmd;
    uint8_t 						data_len;
    uint8_t 					 *data;
}ptl_frame_payload_t;

/**
 * module handler for receive and send
 */
typedef bool (*module_send_handler_t)			(ptl_frame_type_t frame_type, uint16_t param1, uint16_t param2, ptl_proc_buff_t *buff);
typedef bool (*module_receive_handler_t)	(ptl_frame_payload_t *payload, ptl_proc_buff_t *ackbuff);

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * GLOBAL VARIABLES DECLEAR
 */

/*******************************************************************************
 * GLOBAL FUNCTIONS DECLEAR
 */

void com_uart_init_running(void);
void com_uart_start_running(void);
void com_uart_assert_running(void);
void com_uart_running(void);
void com_uart_post_running(void);
void com_uart_stop_running(void);

bool com_uart_reqest_running(uint8_t source);
bool com_uart_release_running(uint8_t source);

void com_uart_set_opposite_running(bool running);
bool com_uart_is_com_error(void);

void ptl_com_uart_register_module(ptl_frame_type_t frame_type, module_send_handler_t send_handler, module_receive_handler_t receive_handler);
void ptl_com_uart_build_frame(ptl_frame_type_t frame_type, ptl_frame_cmd_t cmd, uint8_t *data, uint8_t datelen, ptl_proc_buff_t *framebuff);
void ptl_com_uart_build_frame_header(ptl_frame_type_t frame_type, ptl_frame_cmd_t cmd, uint8_t datalen, ptl_proc_buff_t *buff);
void ptl_com_uart_receive_handler(uint8_t data);
void ptl_frame_analysis_handler(void);
uint8_t ptl_com_uart_get_checksum(uint8_t *data, uint8_t len);


#ifdef __cplusplus
}
#endif


#endif
