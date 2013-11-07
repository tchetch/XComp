/* (c) 2013 Etienne Bagnoud <etienne.bagnoud@irovision.ch>
 *
 * This file is part of XComp project under MIT Licence.
 * See LICENCE file.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xcomp_main.h"

extern const char * ERROR_STR[];

unsigned int howmany(unsigned int input) 
{
    input--;
    return input < 1 ? 0 : input + howmany(input);
}

int getparams(FILE * fp, Group g)
{
    int ret = 0;
    char * l = NULL;
    int c = 0;
    int col = 0;
    Tokk p = NULL;
    XFile * newfile = NULL;

    while((l = f_readln(fp)) != NULL) {
        p = bytetok(l, ':', &c);
        if(p == NULL || c != 2) {
            if(p != NULL) {
                bytetok_free(p);
            } else {
                free(l);
            }
            ERROR(WRONG_INPUT);
        } else {
            if(!sys_check_file(p[0])) {
                bytetok_free(p);
                ERROR(WRONG_FILE);
            } else {
                col = atoi(p[1]);
                if(col < 0) {
                    bytetok_free(p);
                    ERROR(WRONG_INPUT);
                } else {
                    new_xfile(newfile, p, col);
                    if(newfile != NULL) {
                        if(group_append(g, newfile)) {
                            ret++;
                        } else {
                            free(newfile);
                            bytetok_free(p);
                            ERROR(ALLOCATION_FAILED);
                        }
                    } else {
                        ERROR(ALLOCATION_FAILED);
                        bytetok_free(p);
                    }
                }
            }
        }
    }

    return ret;
}


void discard_duplicate(Group fileset)
{
    int duplicate = 0;
    int i = 0;
    int j = 0;
    
    do {
        duplicate = 0;
        for(i = 0; i < group_size(fileset); i++) {
            for(j = i+1; j < group_size(fileset); j++) {
                if(strcmp(((XFile *)G(fileset)[i])->path,
                            ((XFile *)G(fileset)[j])->path) == 0)
                {
                    if(((XFile *)G(fileset)[i])->column ==
                        ((XFile *)G(fileset)[j])->column) {
                        printf("* Remove duplicate %s:%d\n", 
                                ((XFile *)G(fileset)[i])->path,
                                ((XFile *)G(fileset)[j])->column);
                        del_xfile((XFile *)G(fileset)[j]);
                        group_delete(fileset, j);
                        duplicate = 1;
                    }
                }
            }
        }
    } while(duplicate);
}

void unique_file(Group fileset, Group filetoload)
{
    int i = 0;
    int j = 0;
    int forget = 0;

    for(i = 0; i < group_size(fileset); i++) {
        for(j = 0; j < group_size(filetoload); j++) {
            if(strcmp(((XFile *)G(fileset)[i])->path, 
                        ((XFile *)G(filetoload)[j])->path) == 0) {
                forget = 1;
            }
        }
        if(! forget) {
            group_append(filetoload, G(fileset)[i]);
        }
        forget = 0;
    }
}

void copy_fileset(Group fileset, Group fileset_copy)
{
    int i = 0;

    for(i = 0; i < group_size(fileset); i++) {
        group_append(fileset_copy, G(fileset)[i]);
    }
}

#define TOKEN_SEP   '\t'

int load_files(Group filetoload)
{
    FILE * fp = NULL;
    char * line = NULL;
    Tokk c = NULL;
    int ret = 1;
    int i = 0;

    for(i = 0; i < group_size(filetoload); i++) {
        fp = fopen(((XFile *)G(filetoload)[i])->path, "r");
        if(fp != NULL) {
            ((XFile *)G(filetoload)[i])->data = group_init();
            while((line = f_readln(fp)) != NULL) {
                c = bytetok(line, TOKEN_SEP, NULL);
                if(c != NULL) {
                    group_append(((XFile *)G(filetoload)[i])->data, c);
                }
            }

            fclose(fp);
        }
    }

    return ret;
}

/* Use only into process_xfiles */
#define make_result(r, _type, _pos1, _pos2)    do { \
    (r) = malloc(sizeof(*(r))); \
    if((r) == NULL) goto early_stop; \
    (r)->type = (_type); \
    (r)->f1 = f1; (r)->f2 = f2; \
    (r)->pos1 = (_pos1); (r)->pos2 = (_pos2); \
    (r)->value = t1[f1->column]; \
    if( ! group_append(results, (r))) { free((r)); goto early_stop; } \
}while(0)

Group process_xfiles(XFile * f1, XFile * f2, int * early_stop)
{
    int i = 0;
    int j = 0;
    Result * r = NULL;
    Group results = NULL;
    Group d1 = NULL;
    Group d2 = NULL;
    Tokk t1 = NULL;
    Tokk t2 = NULL;

    d1 = f1->data;
    d2 = f2->data;

    if(d1 == NULL) {
        return NULL;
    }
    if(d2 == NULL) {
        return NULL;
    }

    if(early_stop == NULL) {
        return NULL;
    }
    *early_stop = 0;

    results = group_init();
    if(results == NULL) {
        return NULL;
    }

    for(i = 0; i < group_size(d1); i++) {
        t1 = (Tokk)(G(d1)[i]);
        if(! (f1->column < bytetok_size(t1))) {
            make_result(r, RESULT_TOO_SHORT, i, -1);
        } else {
            if(t1[f1->column][0] == '\0') {
                make_result(r, RESULT_EMPTY, i, -1);
                continue;
            }
            for(j = 0; j < group_size(d2); j++) {
                t2 = (Tokk)(G(d2)[j]);
                if( ! (f2->column < bytetok_size(t2))) {
                    make_result(r, RESULT_TOO_SHORT, i, j);
                } else {
                    if(t2[f2->column][0] == '\0') {
                        make_result(r, RESULT_EMPTY, i, j);
                        continue;
                    } 
                    
                    if(strcmp(t1[f1->column], t2[f2->column]) == 0) {
                        make_result(r, RESULT_MATCH, i, j);
                    }
                }
            }
        }
    }

    return results;

early_stop:
    *early_stop = 1;
    return results;
}

static void * thread_function(void * p)
{
    Params * parameters = (Params *)p;
    Group results = NULL;
    Op * operation = NULL;
    int early_stop = 0;
    int stack_ret = 0;
    int run = 1;
    
    if(parameters == NULL) {
        ERROR(WRONG_PARAMS);
    } else {
        while(run) {
            sys_mutex_lock(parameters->mtx);
            stack_ret = sstack_pop(parameters->ops, (void **)&operation);
            sys_mutex_unlock(parameters->mtx);

            if(!stack_ret) {
                run = 0;
            } else {
                results = process_xfiles(operation->f1, operation->f2,
                        &early_stop);
                if(results == NULL) {
                    ERROR(PROCESSING_ERROR); 
                } else {
                    if(early_stop) {
                        ERROR(PROCESSING_INT);
                    }
                    sys_mutex_lock(parameters->mtx_results);
                    if( ! sstack_push(parameters->result_set, results)) {
                        ERROR(STACK_PUSH);
                    }
                    sys_mutex_unlock(parameters->mtx_results);
                }
                free(operation);
            }
        }
    }

    return NULL;
}

int main(int argc, char ** argv)
{
    Group fileset = NULL;
    Group fileset_copy = NULL;
    Group filetoload = NULL;
    Group threads = NULL;
    Group results = NULL;
    void * _results = NULL;
    Result * one_result = NULL;
    THREAD_ID * one_thread = NULL;
    Op * operation = NULL;
    XFile * any_file = NULL;
    Params * t_params = NULL;
    struct SStack * operations = NULL;
    struct SStack * result_set = NULL;
    int i = 0; int j = 0;
    int processors = 0;

    if((fileset = group_init()) == NULL) {
        ERROR(ALLOCATION_FAILED);
        exit(-1);
    }
    
    if((filetoload = group_init()) == NULL) {
        group_free(fileset);
        ERROR(ALLOCATION_FAILED);
        exit(-1);
    }

    if((fileset_copy = group_init()) == NULL) {
        group_free(fileset);
        group_free(filetoload);
        ERROR(ALLOCATION_FAILED);
        exit(-1);
    }

    t_params = malloc(sizeof(*t_params));
    if(t_params == NULL) {
        group_free(fileset);
        group_free(filetoload);
        group_free(fileset_copy);
        ERROR(ALLOCATION_FAILED);
        exit(-1);
    } else {
        if( ! sys_mutex_init(&(t_params->mtx))) {
            free(t_params);
            group_free(fileset);
            group_free(filetoload);
            group_free(fileset_copy);
            ERROR(MUTEX_INIT);
            exit(-1);
        }
        if( ! sys_mutex_init(&(t_params->mtx_results))) {
            free(t_params);
            group_free(fileset);
            group_free(filetoload);
            group_free(fileset_copy);
            ERROR(MUTEX_INIT);
            exit(-1);
        }
    }
    if(getparams(stdin, fileset)) {
        /* Discard search in same file, same column */
        discard_duplicate(fileset);
        
        /* Create the list of file to load */
        unique_file(fileset, filetoload);
    
        /* Keep a fileset copy ready */
        copy_fileset(fileset, fileset_copy);

        printf("# File to compare : %d\n"
                "# File to load : %d\n"
                "# Operation needed :  %d\n", 
                group_size(fileset), 
                group_size(filetoload),
                howmany(group_size(fileset)));
       
        /* Load files and set file pointer for fileset 
         * Don't delete filetoload we use it to free file data
         */ 
        load_files(filetoload);
        for(i = 0; i < group_size(fileset); i++) {
            if(((XFile *)G(fileset)[i])->data == NULL) {

                for(j = 0; j < group_size(filetoload); j++) {
                
                    if(strcmp(
                                ((XFile *)G(fileset)[i])->path,
                                ((XFile *)G(filetoload)[j])->path) == 0) {
                        ((XFile *)G(fileset)[i])->data = 
                            ((XFile *)G(filetoload)[j])->data;
                    
                    }
                }
            }
        }

        if(! sstack_init(&operations, howmany(group_size(fileset))) || 
              ! sstack_init(&result_set, howmany(group_size(fileset)))  ) {
            sstack_free(operations);
            sstack_free(result_set);
            ERROR(ALLOCATION_FAILED);
        } else {

            /* Create the set of comparaison */
            while(group_size(fileset) > 1) {
                for(i = 1; i < group_size(fileset); i++) {
                    operation = malloc(sizeof(*operation)); /* this will be 
                                                               freed in 
                                                               thread_function
                                                             */
                    if(operation == NULL) {
                        ERROR(ALLOCATION_FAILED);
                    } else {
                        operation->f1 = (XFile *)G(fileset)[0];
                        operation->f2 = (XFile *)G(fileset)[i];
                        
                        if(!sstack_push(operations, operation)) {
                            free(operation);
                            ERROR(ALLOCATION_FAILED);
                        } else {
                        }
                    }
                }

                group_delete(fileset, 0);
            }
            group_free(fileset);

            /* THREADS PART */
            threads = group_init();
            processors = sys_get_processor();
            
            t_params->ops = operations;
            t_params->result_set = result_set;

            if(processors > group_size(operations)) {
                processors = group_size(operations);
            }

            printf("# Running %d threads\n", processors);

            for(i = 0; i < processors; i++) {
                one_thread = calloc(1, sizeof(*one_thread));
                if(one_thread != NULL) {
                    if(sys_create_thread(one_thread, thread_function, 
                            (void *)t_params) == 0) {
                        group_append(threads, one_thread);
                    }
                }
            }

            for(i = 0; i < group_size(threads); i++) {
                sys_wait_thread((THREAD_ID *)G(threads)[i]);
                free(G(threads)[i]);
            }
            group_free(threads);

            sys_mutex_destroy(t_params->mtx);
            sys_mutex_destroy(t_params->mtx_results);
            free(t_params);

            while(sstack_pop(operations, (void **)&operation)) {
                free(operation); 
            }
            sstack_free(operations);

            int resm = 0;
            int resf = 0;

            printf("Valeur,Fichier 1, Colone 1, Ligne 1, Fichier 2,"
                   "Colone 2, Ligne 2\n");
            while(sstack_pop(result_set, &_results)) {
                results = (Group)_results;
                for(i = 0; i < group_size(results); i++) {
                    if(((Result *)G(results)[i])->type == RESULT_MATCH) {
                        one_result = (Result *)G(results)[i];
                        printf("\"%s\",\"%s\",%d,%d,\"%s\",%d,%d\n",
                                one_result->value,
                                one_result->f1->path,
                                one_result->f1->column,
                                one_result->pos1,
                                one_result->f2->path,
                                one_result->f2->column,
                                one_result->pos2);
                        resm++;
                    } else {
                        resf++;
                    }
                    free(G(results)[i]); 
                }
                group_free(results);
            }
            sstack_free(result_set);

            printf("# Match found : %d\n", resm);
            printf("# Error : %d\n", resf);

            /* Results would point to nowhere after this point. */
            for(i = 0; i < group_size(filetoload); i++) {
                any_file = (XFile *)G(filetoload)[i];
                for(j = 0; j < group_size(any_file->data); j++) {
                    bytetok_free((Tokk)G(any_file->data)[j]);
                }
                group_free(any_file->data);
            }
            group_free(filetoload);

            for(i = 0; i < group_size(fileset_copy); i++) {
                del_xfile((XFile *)G(fileset_copy)[i]);
            }
            group_free(fileset_copy);
        }
    }
    
    return 0;
}
