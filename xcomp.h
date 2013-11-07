/* (c) 2013 Etienne Bagnoud <etienne.bagnoud@irovision.ch>
 *
 * This file is part of XComp project under MIT Licence.
 * See LICENCE file.
 */
#ifndef xcomp_h__
#define xcomp_h__

#include <stdio.h>

struct SStack {
    int size;
    int current;
    char ** data;
};


/* Simple stack */
int sstack_init(struct SStack ** s, int size);
int sstack_push(struct SStack * s, void * p);
int sstack_pop(struct SStack * s, void ** p);
void sstack_free(struct SStack * s);

/* ** '[\r\n]'-free readline */
char * f_readln(FILE * fp);

/* ** Simple one-char tokenizer
 * string is modified and must not be freed
 */
typedef char ** Tokk; /* opaque structure ^^ */
Tokk bytetok(char * string, char tok, int * count);
int bytetok_size(Tokk b);
#define bytetok_free(x) do { if((x)!=NULL) { free(((Tokk)(x))[0]); \
    free((x)); }} while(0);

/* ** Group
 * Group is an array with no hole. NULL is a hole.
 * It can't store NULL. Don't change values manually to NULL, use group_delete.
 */
typedef struct s_Group {
    int size;
    char ** data;
} sGroup;
typedef sGroup * Group;

#define group_free(g)   do { if((g)->data != NULL) free((g)->data); \
   if((g) != NULL) free((g)); } while(0) 
#define group_size(g)   (g)->size
#define G(g)    (g)->data


Group group_init(void);
int group_append(Group g, void * p);
int group_prepend(Group g, void * p);
int group_delete(Group g, int index);
int group_add(Group g, void * p, int index);

#endif /* xcomp_h__ */
