#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xcomp.h"

/* ** Simple stack */
int sstack_init(struct SStack ** s, int size) 
{
    int ret = 0;

    if(s != NULL) {
        if(size > 0) {
            *s = malloc(sizeof(**s)); 
            if(*s != NULL) {
                (*s)->data = malloc(size * sizeof(*((*s)->data)));
                if((*s)->data == NULL) {
                    free(*s);
                } else {
                    (*s)->size = size;
                    (*s)->current = 0;
                    ret = 1;
                }
            }
        }
    }

    return ret;
}

int sstack_push(struct SStack * s, void * p)
{
    int ret = 0;

    if(s != NULL && p != NULL) {
        if(s->current < s->size) {
            s->data[s->current] = (char *)p;
            s->current++;
            ret = 1;
        }
    }

    return ret;
}

int sstack_pop(struct SStack * s, void ** p)
{
    int ret = 0;

    if(s != NULL && p != NULL) {
        if(s->current > 0) {
            s->current--;
            *p = (void *)s->data[0];
            memmove(&(s->data[0]), &(s->data[1]), (s->size - 1) * sizeof(char *));
            s->data[s->size - 1] = NULL;
            ret = 1;
        }
    }

    return ret;
}

void sstack_free(struct SStack * s) 
{
    if(s != NULL) {
        if(s->data != NULL) free(s->data);
        free(s);
    }
        
}

/* ** '[\r\n]'-free readline */
#define F_READLINE_BUFFER      512 /* Must be > 1 */ 
char * f_readln(FILE * fp)
{
    char buffer[F_READLINE_BUFFER];
    char * line = NULL; char * line_t = NULL;
    char * eol = NULL; char * eol_bis = NULL;
    size_t line_s = 0;
    int stop = 0;

    while(! stop) {
        if(fgets(buffer, F_READLINE_BUFFER, fp) != NULL) {
            eol = strchr(&buffer[0], '\n');
            eol_bis = strchr(&buffer[0], '\r');

            if(eol == NULL && eol_bis != NULL) {
                eol = eol_bis;
            }
            if(eol != NULL && eol_bis != NULL) {
                if(eol > eol_bis) eol = eol_bis;
            }

            if(eol == NULL) {
                line_t = realloc(line, line_s + F_READLINE_BUFFER - 1);
                if(line_t != NULL) {
                    line = line_t;
                    memcpy(line + line_s, buffer, F_READLINE_BUFFER - 1);
                    line_s += F_READLINE_BUFFER - 1;
                } 
            } else {
                line_t = realloc(line, line_s + (eol - &buffer[0]) + 1);
                if(line_t != NULL) {
                    line = line_t;
                    memcpy(line + line_s, buffer, (eol - &buffer[0]));
                    *(line + line_s + (eol - &buffer[0])) = '\0';
                    stop = 1;
                }
            }
        } else {
            if(line != NULL) {
                free(line);
                line = NULL;
            }
            stop = 1;
        }
    }

    return line;
}

/* ** Simple one-char tokenizer */
Tokk bytetok(char * string, char tok, int * count)
{
    char ** result = NULL; char ** result_t = NULL;
    char * current = NULL;
    int size = 1;

    result = malloc(sizeof(char *));
    if(result != NULL) {   
        result[0] = string; 
        while((current = strchr(string, tok)) != NULL) {
            size++;
            result_t = realloc(result, size * sizeof(char *));
            if(result_t != NULL) {
                result = result_t;
                *current = '\0';
                string = current + 1;
                result[size - 1] = string;
            } else {
                free(result);
                result = NULL;
                break;
            }
        }
        if(result != NULL) {
            result_t = realloc(result, (size+1) * sizeof(void *));
            if(result_t != NULL) {
                result = result_t;
                result[size]=NULL;
            } else {
                free(result);
                result = NULL;
            }
        }
    }

    if(count != NULL) {
        if(result != NULL) *count = size;
        else *count = -1;
    }

    return result; 
}

int bytetok_size(Tokk b)
{
    int i = 0;

    for(i = 0; b[i] != NULL; i++);

    return i;
}

/* ** Groups */
Group group_init(void)
{
    Group g = NULL;

    g = malloc(sizeof(*g));
    if (g != NULL) {
        g->size = 0;
        g->data = NULL;
    }

    return g;
}


int _grow_group(Group  g, int size)
{
    int i = 0;
    int ret = 0;
    char ** tmp = NULL;
    
    if(g != NULL && size > 0) {
        tmp = realloc(g->data, sizeof(*(g->data)) * (g->size + size));
        if(tmp != NULL) {
            g->data = tmp;    
            for(i = g->size; i < g->size + size; i++) {
                g->data[i] = NULL;
            }
            g->size += size;
            ret = 1;
        }
    }

    return ret;
}

int _shrink_group(Group g, int size)
{
    int ret = 0;
    char ** tmp = NULL;

    if(g != NULL && size > 0) {
        if(size >= g->size) {
            free(g->data);
            g->data = NULL;
            g->size = 0;
            ret = 1;
        } else {
            tmp = realloc(g->data, sizeof(*(g->data)) * (g->size - size));
            if(tmp != NULL) {
                g->data = tmp;
                g->size -= size;
                ret = 1;
            }
        }
    }

    return ret;
}

int group_append(Group g, void * p)
{
    int ret = 0;

    if(g != NULL && p != NULL) {
        if(g->size <= 0) {
            if( ! _grow_group(g, 1)) {
                goto quick_return; 
            }
        }
    
        if(g->data[g->size - 1] != NULL) {
            if( ! _grow_group(g, 1)) {
                goto quick_return;
            }
        }

        g->data[g->size - 1] = p;
        
        ret = 1;
    }

quick_return:
    return ret;
}

int group_prepend(Group g, void * p)
{
    int ret = 0;

    if(g != NULL && p != NULL) {
        if(g->size <= 0) {
            if( ! _grow_group(g, 1)) {
                goto quick_return;
            }
        }

        if(g->data[g->size -1] != NULL) {
            if( ! _grow_group(g, 1)) {
                goto quick_return;
            }
        }

        if(g->size > 1) {
            memmove(&(g->data[1]), &(g->data[0]), sizeof(*(g->data)) * (g->size -1));
            g->data[0] = p;
        } else {
            g->data[0] = p;
        }
                    
        ret = 1;
    }

quick_return:
    return ret;
}

#define group_move_forward(g, idx, steps)   do { \
    memmove(&((g)->data[(idx) + (steps)]), \
            &((g)->data[(idx)]), \
            ((g)->size - ((idx) + (steps))) * sizeof(*((g)->data))); } while(0)
#define group_move_backward(g, idx, steps)  do { \
    memmove(&((g)->data[(idx)]), \
           &((g)->data[(idx) + (steps)]), \
           ((g)->size - ((idx) + (steps))) * sizeof(*((g)->data))); } while(0)


int group_delete(Group g, int index)
{
    int ret = 0;
    void * bck = NULL;

    if(g != NULL && index < g->size && g->size > 0) {
        bck = g->data[index];
        group_move_backward(g, index, 1);
        g->data[g->size - 1] = NULL;
        if( ! _shrink_group(g, 1)) {
            /* Fail, we move back the data */
            group_move_forward(g, index, 1);
            g->data[index]=bck;
        } else {
            ret = 1;
        }
    }

    return ret;
}

int group_add(Group g, void * p, int index)
{
    int ret = 0;
    if(g != NULL && p != NULL) {
        if(g->size <= 0) {
            ret = group_append(g, p);
        } else if(index >= g->size - 1) {
            ret = group_append(g, p);
        } else if(index <= 0) {
            ret = group_prepend(g, p);
        } else {
            if( _grow_group(g, 1)) {
                group_move_forward(g, index, 1);
                g->data[index] = p;
                ret = 1;
            }
        }
    }

    return ret;
}
