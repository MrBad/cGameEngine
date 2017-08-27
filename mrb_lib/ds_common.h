#ifndef _DS_COMMON_H
#define _DS_COMMON_H

typedef int  (*cmp_func_t)(void *data1, void *data2);
typedef void (*del_func_t)(void *data);
typedef int  (*visit_func_t) (void *data, void *ctx);

#endif
