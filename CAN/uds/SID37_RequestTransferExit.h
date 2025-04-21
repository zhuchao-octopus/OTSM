/******************************************************************************
* 文件名称: SID37_RequestTransferExit.h
* 内容摘要: 请求退出传输头文件
* 创建者の: 孔佳伟
* 个人主页: https://gitee.com/thin-wind/jump
* 修改记录: 
******************************************************************************/


#ifndef _SID37_REQUEST_TRANSFER_EXIT_H_
#define _SID37_REQUEST_TRANSFER_EXIT_H_

#include <stdint.h>
#include "uds_type.h"


/******************************************************************************
* 函数名称: bool_t service_37_check_len(const uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 检查 37 服务数据长度是否合法
* 输入参数: uint16_t msg_dlc         --数据长度
* 输出参数: 无
* 函数返回: TRUE: 合法; FALSE: 非法
* 其它说明: 无
******************************************************************************/
bool_t service_37_check_len(const uint8_t* msg_buf, uint16_t msg_dlc);


/******************************************************************************
* 函数名称: void service_37_RequestTransferExit(const uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 37 服务 - 请求退出传输
* 输入参数: uint8_t*    msg_buf         --数据首地址
    　　　　uint8_t     msg_dlc         --数据长度
* 输出参数: 无
* 函数返回: 无
* 其它说明: 无
******************************************************************************/
void service_37_RequestTransferExit(const uint8_t* msg_buf, uint16_t msg_dlc);


#endif

/****************EOF****************/
