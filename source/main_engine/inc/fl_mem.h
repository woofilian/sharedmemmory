/*
 * =====================================================================================
 *
 *       Filename:  fl_mem.h
 *
 *    Description:  fixed length mem pool 
 *
 *        Version:  1.0
 *        Created:  2011年08月29日 10时19分36秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  maohw (), maohongwei@gmail.com
 *        Company:  jxj
 *         Modify:  
 *
 * =====================================================================================
 */
#ifndef __fl_mem_h__
#define __fl_mem_h__

typedef struct fl_mem_pool_s {
	void **ptr;
	int  num;
    int  used;
	int  size;
}fl_mem_pool_t;


typedef int (fl_compare_t)(void *data_orig, void *data_custom);

fl_mem_pool_t *fl_mem_pool_new(int num, int size);
int fl_mem_pool_del(fl_mem_pool_t *pool);
void *fl_mem_alloc(fl_mem_pool_t *pool);
int fl_mem_free(fl_mem_pool_t *pool, void *p);
void *fl_find_custom(fl_mem_pool_t *pool, void *data, fl_compare_t *func);

#endif //__fl_mem_h__
