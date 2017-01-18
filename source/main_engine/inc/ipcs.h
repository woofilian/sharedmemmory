/*
 * =====================================================================================
 *
 *       Filename:  ipcs.h
 *
 *    Description:  ipcs
 *
 *        Version:  1.0
 *        Created:  2011年05月23日 01时52分14秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  maohw (), maohongwei@gmail.com
 *        Company:  jxj
 *
 * =====================================================================================
 */
#ifndef __ipcs_h__
#define __ipcs_h__
#include "main.h"

int apcs_init(unsigned short port, void *p);
int apcs_reg_msg_dispatch(sdk_msg_dispatch_cb fun);
int apcs_reg_msg_dispatch_ptz(sdk_msg_dispatch_cb fun);
int apcs_deinit(void);

int apcs_msg_send(int module_id, sdk_msg_t *pmsg);

#endif //__ipcs_h__
