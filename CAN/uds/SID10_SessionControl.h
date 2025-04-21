/******************************************************************************
* 文件名称: SID10_SessionControl.c
* 内容摘要: 诊断会话控制头文件
* 创建者の: 孔佳伟
* 个人主页: https://gitee.com/thin-wind/jump
* 修改记录: 
******************************************************************************/

#ifndef _SID2E_SESSION_CONTROL_H_
#define _SID2E_SESSION_CONTROL_H_

#include <stdint.h>
#include "uds_type.h"
#include "uds_service.h"

/******************************************************************************
* 函数名称: void service_10_SessionControl(const uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 10 服务 - 诊断会话控制
* 输入参数: uint8_t*    msg_buf         --数据首地址
    　　　　uint8_t     msg_dlc         --数据长度
* 输出参数: 无
* 函数返回: 无
* 其它说明: 无
******************************************************************************/
void service_10_SessionControl(const uint8_t* msg_buf, uint16_t msg_dlc);


/******************************************************************************
* 函数名称: bool_t service_10_check_len(const uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 检查 10 服务数据长度是否合法
* 输入参数: uint16_t msg_dlc         --数据长度
* 输出参数: 无
* 函数返回: TRUE: 合法; FALSE: 非法
* 其它说明: 无
******************************************************************************/
bool_t service_10_check_len(const uint8_t* msg_buf, uint16_t msg_dlc);


/******************************************************************************
* 函数名称: void set_current_session(uds_session_t session)
* 功能说明: 设置当前诊断会话状态
* 输入参数: uds_session_t session       --会话状态
* 输出参数: 无
* 函数返回: 无
* 其它说明: 无
******************************************************************************/
void set_current_session(uds_session_t session);


/******************************************************************************
* 函数名称: uds_session_t get_current_session(void)
* 功能说明: 读取当前诊断会话状态
* 输入参数: 无
* 输出参数: 无
* 函数返回: 当前会话状态
* 其它说明: 无
******************************************************************************/
uds_session_t get_current_session(void);


#endif
/****************EOF****************/
