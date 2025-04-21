/******************************************************************************
* 文件名称: SID22_ReadDataByIdentifier.h
* 内容摘要: 根据标识符读取数据头文件
* 创建者の: 孔佳伟
* 个人主页: https://gitee.com/thin-wind/jump
* 修改记录: 
******************************************************************************/


#ifndef _SID22_READ_DATA_BY_IDENTIFIER_H_
#define _SID22_READ_DATA_BY_IDENTIFIER_H_


#include <stdint.h>
#include "uds_type.h"


/******************************************************************************
* 函数名称: void service_22_ReadDataByIdentifier(const uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 22 服务 - 根据标识符读取数据
* 输入参数: uint8_t*    msg_buf         --数据首地址
    　　　　uint8_t     msg_dlc         --数据长度
* 输出参数: 无
* 函数返回: 无
* 其它说明: 无
******************************************************************************/
void service_22_ReadDataByIdentifier(const uint8_t* msg_buf, uint16_t msg_dlc);

/******************************************************************************
* 函数名称: bool_t service_22_check_len(const uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 检查 22 服务数据长度是否合法
* 输入参数: uint16_t msg_dlc         --数据长度
* 输出参数: 无
* 函数返回: TRUE: 合法; FALSE: 非法
* 其它说明: 无
******************************************************************************/
bool_t service_22_check_len(const uint8_t* msg_buf, uint16_t msg_dlc);


#endif

/****************EOF****************/
