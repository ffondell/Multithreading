#include <stdlib.h>
#include <stdio.h>
#include "strbuf.h"

#ifndef DEBUG
#define DEBUG 0
#endif

int sb_init(strbuf_t *L, size_t length)
{
    L->data = malloc(sizeof(char) * length);
    if (!L->data) return 1;

    L->length = length;
    L->used   = 0;

    return 0;
}

void sb_destroy(strbuf_t *L)
{
    free(L->data);
}


int sb_append(strbuf_t *L, char item)
{
    if (L->used == L->length) {
	size_t size = L->length * 2;
	char *p = realloc(L->data, sizeof(char) * size);
	if (!p) return 1;

	L->data = p;
	L->length = size;

	if (DEBUG) printf("Increased size to %lu\n", size);
    }

    L->data[L->used] = item;
    ++L->used;

    return 0;
}


int sb_remove(strbuf_t *L, int *item)
{
    if (L->used == 0) return 1;

    --L->used;

    if (item) *item = L->data[L->used];

    return 1;
}

int sb_insert(strbuf_t *list, int index, char item){
	if(list->length==0){
		char *created=malloc(sizeof(char));
		if(!created){
			return 1;
		}
		list->data=created;
		list->length=1;
	}
	else if(list->length==list->used){
		list->length=list->length*2;
		size_t s=list->length;
		char *created=realloc(list->data,sizeof(char)*s);
		if (!created){
			return 1;
		}
		list->data=created;
	}
	if((index>list->length-1)||(index>list->used)){
		sb_append(list,item);
	}
	else{
		list->used++;
		for(int i = 0;i<list->used-index;i++){
			list->data[list->used-i]=list->data[list->used-i-1];
		}
		list->data[index]=item;
		
	}
	return 0;
}

int sb_concat(strbuf_t *sb, char *str){
	int i=0;
	while(str[i]!='\0'){
		sb_append(sb,str[i]);
		i=i+1;
	}
	return 0;
}

int main(int argc, char *argv[]){


	return 0;
}

