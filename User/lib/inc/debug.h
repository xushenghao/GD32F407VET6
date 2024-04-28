/***
 * @Author:
 * @Date: 2023-04-04 08:13:11
 * @LastEditors: xxx
 * @LastEditTime: 2023-04-04 13:21:46
 * @Description:
 * @email:
 * @Copyright (c) 2023 by xxx, All Rights Reserved.
 */
#ifndef __DEBUG_H
#define __DEBUG_H
#include "lib.h"

/*形参*/
#define _DBG_LINE_ , uint16_t line
/*实参*/
#define __DBG_LINE , __LINE__

extern BOOL DBG_ASSERT(uint8_t cond _DBG_LINE_);

#endif //__DEBUG_H
