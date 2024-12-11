/** ****************************************************************************
 * @copyright Copyright (c) XXX
 * All rights reserved.
 *
 *
 */
#ifndef __OCTOPUS_TASK_MANAGER_SYSTEM_H__
#define __OCTOPUS_TASK_MANAGER_SYSTEM_H__

/*******************************************************************************
 * INCLUDES
 */
 
#include "octopus_platform.h"

#ifdef __cplusplus
extern "C"{
#endif
/*******************************************************************************
 * MACROS
 */
#define OTMS_VERSION "V1.0.0"
#define OTMS_RELEASE_DATA_TIME  __DATE__ " " __TIME__

#define APP_VER_STR "V1.0.0"
#define HW_VER_STR  "V1.0.0"
#define PRJ_VER_STR "KD070E01"
#define VER_STR "MCU:"APP_VER_STR"HW:"HW_VER_STR"PRJ:"PRJ_VER_STR"\n"

#define TASK_MANAGER_STATE_MACHINE_MCU 0 // main control
//#define TASK_MANAGER_STATE_MACHINE_SOC 1   //

#define SYSTEM_MPU_STATE_INIT           (0x00)  //APP��ʼ��
#define SYSTEM_MPU_STATE_ENTER_PLAY     (0x01)  //APP��ʼ���Ž�������
#define SYSTEM_MPU_STATE_ENTER_FINISH   (0x02)  //APP���������������
#define SYSTEM_MPU_STATE_COMPLETED      (0x03)  //APP��ȫ�������
#define SYSTEM_MPU_STATE_LEAVE_PLAY     (0x04)  //APP��ʼ�����˳�����
#define SYSTEM_MPU_STATE_LEAVE_FINISH   (0x05)  //APP�˳������������

////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************
 * TYPEDEFS
 */
typedef enum {
    MCU_UPDATE_ST_INIT          = (0x00),  //������ʼ��
    MCU_UPDATE_ST_CHECK_FILE    = (0x01),  //��������ļ�
    MCU_UPDATE_ST_WAIT_CONFIRM  = (0x02),  //�ȴ�ȷ������
    MCU_UPDATE_ST_START         = (0x03),  //��ʼ����
    MCU_UPDATE_ST_WAIT_BOOT     = (0x04),  //�ȴ�MCU����boot���
    MCU_UPDATE_ST_TRANSFER      = (0x05),  //��������
    MCU_UPDATE_ST_COMPLETED     = (0x06),  //�������
    MCU_UPDATE_ST_FAILED        = (0x07),  //����ʧ��
}mcu_update_state_t;

typedef enum MB_STATE{
    MB_ST_INIT = 0,
    MB_ST_LOWPOWER,
    MB_ST_BOOTING,
    MB_ST_STANDBY,
    MB_ST_ON,
    MB_ST_PARTIAL,
    MB_ST_SHUTDOWN,
    MB_ST_OFF
}mb_state_t;

/*******************************************************************************
 * GLOBAL FUNCTIONS DECLEAR
 */

void app_system_init_running(void);
void app_system_start_running(void);
void app_system_assert_running(void);
void app_system_running(void);
void app_system_post_running(void);
void app_system_stop_running(void);

void system_set_mpu_status(uint8_t status);
bool system_get_power_off_req(void);
uint8_t system_get_mpu_status(void);


void system_handshake_with_app(void);
void system_handshake_with_mcu(void);
mb_state_t system_get_mb_state(void);



#ifdef __cplusplus
}
#endif


#endif
