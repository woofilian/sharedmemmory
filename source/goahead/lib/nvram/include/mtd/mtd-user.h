/*
 * $Id: mtd-user.h,v 1.1 2010/08/16 06:37:14 mars_ch Exp $
 *
 * MTD ABI header for use by user space only.
 */

#ifndef __MTD_USER_H__
#define __MTD_USER_H__

#include <stdint.h>

/* This file is blessed for inclusion by userspace */
#include <mtd/mtd-abi.h>

typedef struct mtd_info_user mtd_info_t;
typedef struct erase_info_user erase_info_t;
typedef struct region_info_user region_info_t;

#endif /* __MTD_USER_H__ */
