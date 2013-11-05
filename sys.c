#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sys.h"
#include "xcomp.h"

int sys_check_file(const char * path)
{
    int ret = 0;
    struct stat buffer;

    if(stat(path, &buffer) == 0) {
        if(S_ISREG(buffer.st_mode)) {
            ret = 1;
        }
    }

    return ret;
}

int sys_get_processor(void)
{
    FILE * fp = NULL;
    int processors = 0;
    char * l = NULL;
    Tokk p = NULL;
    char * val = NULL;
    int count = 0;
    int i = 0;

    fp = fopen("/proc/cpuinfo", "r");
    if(fp != NULL) {
        while((l = f_readln(fp)) != NULL) {
            p = bytetok(l, ':', &count);
            if(p != NULL) {
                /* could have been better ^^ */
                if(count >= 2) {
                    val = p[0];
                    for(i = 0; val[i] != '\0'; i++);
                    i--;
                    for(; val[i] == ' '; i--);
                    for(; val[i] == '\t'; i--);
                    i++;
                    val[i]='\0';
                    if(strcmp("cpu cores", val) == 0) {
                        for(val = p[1]; *val == ' '; val++);
                        processors += atoi(val);
                    }
                }
                bytetok_free(p);
            } else {
                free(l);
            }
        
        }
        fclose(fp);
    }

    /* let one core free */
    return processors == 0 ? 1 : processors - 1;
}

int sys_mutex_init(MUTEX ** mtx)
{
    int ret = 0;
    if(mtx != NULL) {
        *mtx = malloc(sizeof(**mtx));
        if(*mtx != NULL) {
            pthread_mutex_init(*mtx, NULL);
            ret = 1;
        }
    }

    return ret;
}

int sys_mutex_lock(MUTEX * mtx) 
{
    return pthread_mutex_lock(mtx);
}

int sys_mutex_unlock(MUTEX * mtx)
{
    return pthread_mutex_unlock(mtx);
}

void sys_mutex_destroy(MUTEX * mtx)
{
    if(mtx != NULL) {
        pthread_mutex_destroy(mtx);
        free(mtx);
    }
}

int sys_create_thread(THREAD_ID * id, void *(*func)(void *), void * args)
{
    return pthread_create(id, NULL, func, args);
}

int sys_wait_thread(THREAD_ID * id)
{
    return  pthread_join(*id, NULL);
}

