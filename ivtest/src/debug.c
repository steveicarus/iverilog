/*
 * Copyright (c) Tony Bybell 1999-2000.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */


/*
 * debug.c 01feb99ajb
 * malloc debugs added on 13jul99ajb
 */
#include "debug.h"

#define DEB_FAIL 333

#undef free_2

#ifdef DEBUG_MALLOC	/* normally this should be undefined..this is *only* for finding stray allocations/frees */
	static struct memchunk *mem=NULL;
	static size_t mem_total=0;
	static int mem_chunks=0;

	static void mem_addnode(void *ptr, size_t size)
	{
	struct memchunk *m;

	m=(struct memchunk *)malloc(sizeof(struct memchunk));
	m->ptr=ptr;
	m->size=size;
	m->next=mem;

	mem=m;
	mem_total+=size;
	mem_chunks++;

	fprintf(stderr,"mem_addnode:  TC:%05d TOT:%010d PNT:%010p LEN:+%d\n",mem_chunks,mem_total,ptr,size);
	}

	static void mem_freenode(void *ptr)
	{
	struct memchunk *m, *mprev=NULL;
	m=mem;

	while(m)
		{
		if(m->ptr==ptr)
			{
			if(mprev)
				{
				mprev->next=m->next;
				}
				else
				{
				mem=m->next;
				}

			mem_total=mem_total-m->size;
			mem_chunks--;
			fprintf(stderr,"mem_freenode: TC:%05d TOT:%010d PNT:%010p LEN:-%d\n",mem_chunks,mem_total,ptr,m->size);
			free(m);
			return;
			}
		mprev=m;
		m=m->next;
		}

	fprintf(stderr,"mem_freenode: PNT:%010p *INVALID*\n",ptr);
	sleep(1);
	}
#endif


/*
 * wrapped malloc family...
 */
void *malloc_2(size_t size)
{
void *ret;
ret=malloc(size);
if(ret)
	{
	DEBUG_M(mem_addnode(ret,size));
	return(ret);
	}
	else
	{
	fprintf(stderr, "FATAL ERROR : Out of memory, sorry.\n");
	exit(DEB_FAIL);
	}
}

void *realloc_2(void *ptr, size_t size)
{
void *ret;
ret=realloc(ptr, size);
if(ret)
	{
	DEBUG_M(mem_freenode(ptr));
	DEBUG_M(mem_addnode(ret,size));
	return(ret);
	}
	else
	{
	fprintf(stderr, "FATAL ERROR : Out of memory, sorry.\n");
	exit(DEB_FAIL);
	}
}

void *calloc_2(size_t nmemb, size_t size)
{
void *ret;
ret=calloc(nmemb, size);
if(ret)
	{
	DEBUG_M(mem_addnode(ret, nmemb*size));
	return(ret);
	}
	else
	{
	fprintf(stderr, "FATAL ERROR: Out of memory, sorry.\n");
	exit(DEB_FAIL);
	}
}


#ifdef DEBUG_MALLOC_LINES
void free_2(void *ptr, char *filename, int lineno)
{
if(ptr)
	{
	DEBUG_M(mem_freenode(ptr));
	free(ptr);
	}
	else
	{
	fprintf(stderr, "WARNING: Attempt to free NULL pointer caught: \"%s\", line %d.\n", filename, lineno);
	}
}
#else
void free_2(void *ptr)
{
if(ptr)
	{
	DEBUG_M(mem_freenode(ptr));
	free(ptr);
	}
	else
	{
	fprintf(stderr, "WARNING: Attempt to free NULL pointer caught.\n");
	}
}
#endif


/*
 * atoi 64-bit version..
 * y/on     default to '1'
 * n/nonnum default to '0'
 */
char *atoi_cont_ptr=NULL;

TimeType atoi_64(char *str)
{
TimeType val=0;
unsigned char ch, nflag=0;

atoi_cont_ptr=NULL;

switch(*str)
	{
	case 'y':
	case 'Y':
		return(LLDescriptor(1));

	case 'o':
	case 'O':
		str++;
		ch=*str;
		if((ch=='n')||(ch=='N'))
			return(LLDescriptor(1));
		else	return(LLDescriptor(0));

	case 'n':
	case 'N':
		return(LLDescriptor(0));
		break;

	default:
		break;
	}

while((ch=*(str++)))
	{
	if((ch>='0')&&(ch<='9'))
		{
		val=(val*10+(ch&15));
		}
	else
	if((ch=='-')&&(val==0)&&(!nflag))
		{
		nflag=1;
		}
	else
	if(val)
		{
		atoi_cont_ptr=str-1;
		break;
		}
	}
return(nflag?(-val):val);
}
