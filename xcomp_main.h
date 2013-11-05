#ifndef XCOMP_MAIN_H__ 
#define XCOMP_MAIN_H__

#include "xcomp.h"
#include "sys.h"

const char * ERROR_STR[] = {
    "No error",
    "Wrong input, rejected",
    "No access to file, rejected",
    "Allocation failed, sad",
    "Line parsing failed",
    "Append to group failed",
    "File open failed",
    "Function parameters are wrong or NULL",
    "Mutex init failed",
    "Processing could not be initialized",
    "Processing interrupted, migth be not enough memory",
    "Removing data from stack failed",
    "Adding data to stack failed"
};

#define ERROR(x)      fprintf(stderr, "[%3d:%s:%d:%s] %s\n", (x), __FILE__, \
        __LINE__, __FUNCTION__, ERROR_STR[(x)])
#define WRONG_INPUT         1
#define WRONG_FILE          2
#define ALLOCATION_FAILED   3
#define TOKEN_PARSING       4
#define GROUP_APPEND        5
#define FILE_OPEN           6
#define WRONG_PARAMS        7
#define MUTEX_INIT          8
#define PROCESSING_ERROR    9
#define PROCESSING_INT      10
#define STACK_POP           11
#define STACK_PUSH          12

typedef struct {
    Tokk _forfree;
    char * path;
    char * _column;
    int column;
    Group data;
} XFile;

typedef struct {
    XFile * f1;
    XFile * f2;
} Op;

#define RESULT_MATCH        0
#define RESULT_TOO_SHORT    1
#define RESULT_EMPTY        2

typedef struct {
    int type;
    char * value;
    XFile * f1;
    XFile * f2;
    int pos1;
    int pos2;
} Result;

typedef struct {
    struct SStack * ops;
    struct SStack * result_set;
    MUTEX * mtx;
    MUTEX * mtx_results;
} Params;

#define new_xfile(x, tokk, col) do { \
    (x) = malloc(sizeof(XFile)); \
    if((x) != NULL) { \
        (x)->_forfree = (tokk);  \
        (x)->path = (tokk)[0]; \
        (x)->_column = (tokk)[1]; \
        (x)->column = (col); \
        (x)->data = NULL; \
    } } while(0)

#define del_xfile(x) do { \
    if((x) != NULL) { \
        bytetok_free(((XFile *)(x))->_forfree); \
        free((x)); \
    } } while(0)

#endif /* XCOMP_MAIN_H__ */
