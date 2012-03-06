#ifndef _RTX_H
#define _RTX_H

#define GROUP "G021"

typedef unsigned int U32;

#define __SVC_0  __svc_indirect(0)

extern int k_release_processor(void);
#define release_processor() _release_processor((U32)k_release_processor)
//extern int __SVC_0 _release_processor(U32 p_func);
int __SVC_0 _release_processor(U32 p_func);

extern int k_set_process_priority(int, int);
#define set_process_priority(pid, plevel) k_set_process_priority(pid, plevel)

extern int k_get_process_priority(int);
#define get_process_priority(pid) k_get_process_priority(pid)

extern void* k_request_memory_block(void);
#define request_memory_block() _request_memory_block((U32)k_request_memory_block)
extern void* _request_memory_block(U32 p_func) __SVC_0;

extern int k_release_memory_block(void *);
#define release_memory_block(p_mem_blk) _release_memory_block((U32)k_release_memory_block, p_mem_blk)
extern int _release_memory_block(U32 p_func, void * p_mem_blk) __SVC_0;

extern int k_send_message (int, void *);
#define send_message(pid, message) k_send_message(pid, message)

extern void* k_receive_message (int *);
#define receive_message(pid) k_receive_message(pid)

#endif // !_RTX_H_
