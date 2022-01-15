/*
 * Copyright (c) Tony Bybell 1999-2000.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "analyzer.h"
#include "misc.h"
#include "bsearch.h"

static TimeType max_compare_time;
static hptr max_compare_pos;
static hptr *max_compare_index;

static int compar_histent(const void *s1, const void *s2)
{
TimeType key, obj, delta;
hptr cpos;
int rv;

key=*((TimeType *)s1);
obj=(cpos=(*((hptr *)s2)))->time;

if((obj<=key)&&(obj>max_compare_time))
	{
	max_compare_time=obj;
	max_compare_pos=cpos;
	max_compare_index=(hptr *)s2;
	}

delta=key-obj;
if(delta<0) rv=-1;
else if(delta>0) rv=1;
else rv=0;

return(rv);
}

hptr bsearch_node(nptr n, TimeType key)
{
max_compare_time=-2; max_compare_pos=NULL; max_compare_index=NULL;

bsearch(&key, n->harray, n->numhist, sizeof(hptr), compar_histent);
if((!max_compare_pos)||(max_compare_time<0))
	{
	max_compare_pos=n->harray[1]; /* aix bsearch fix */
	max_compare_index=&(n->harray[1]);
	}

return(max_compare_pos);
}

/*****************************************************************************************/

static int compar_facs(const void *key, const void *v2)
{
struct symbol *s2;
int rc;

s2=*((struct symbol **)v2);
rc=sigcmp((char *)key,s2->name);
return(rc);
}

struct symbol *bsearch_facs(struct globals *obj, char *ascii)
{
struct symbol **rc;

if ((!ascii)||(!strlen(ascii))) return(NULL);
rc=(struct symbol **)bsearch(ascii, obj->facs, obj->numfacs, sizeof(struct symbol *), compar_facs);
if(rc) return(*rc); else return(NULL);
}
