/* (c) 2013 Etienne Bagnoud <etienne.bagnoud@irovision.ch>
 *
 * This file is part of XComp project under MIT Licence.
 * See LICENCE file.
 */
#ifndef SYS_H__
#define SYS_H__

#include <pthread.h>

#define THREAD_ID   pthread_t
#define MUTEX       pthread_mutex_t

int sys_check_file(const char * path);
int sys_get_processor(void);
int sys_mutex_init(MUTEX ** mtx);
int sys_mutex_lock(MUTEX * mtx);
int sys_mutex_unlock(MUTEX * mtx);
void sys_mutex_destroy(MUTEX * mtx);
int sys_create_thread(THREAD_ID * id, void *(*func)(void *), void * args);
int sys_wait_thread(THREAD_ID * id);
#endif /* SYS_H__ */
