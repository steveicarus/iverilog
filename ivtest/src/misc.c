/*
 * Copyright (c) Tony Bybell 1999-2000.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "misc.h"
#include "bsearch.h"


/*
 * Generic hash function for symbol names...
 */
int hash(char *s)
{
char *p;
unsigned int h=0, g;
for(p=s;*p;p++)
        {
        h=(h<<4)+(*p);
        if((g=h&0xf0000000))
                {
                h=h^(g>>24);
                h=h^g;
                }
        }
return(h%SYMPRIME);
}


/*
 * add symbol to table.  no duplicate checking
 * is necessary as aet's are "correct."
 */
struct symbol *symadd(struct globals *obj, char *name, int hv)
{
struct symbol *s;

s=(struct symbol *)calloc_2(1,sizeof(struct symbol));
strcpy(s->name=(char *)malloc_2(strlen(name)+1),name);
s->next=obj->sym[hv];
obj->sym[hv]=s;
return(s);
}


/*
 * find a slot already in the table...
 */
struct symbol *symfind(struct globals *obj, char *s)
{
int hv;
struct symbol *temp;

if(!obj->facs_are_sorted)
	{
	hv=hash(s);
	if(!(temp=obj->sym[hv])) return(NULL); /* no hash entry, add here wanted to add */

	while(temp)
	        {
	        if(!strcmp(temp->name,s))
	                {
	                return(temp); /* in table already */
	                }
	        if(!temp->next) break;
	        temp=temp->next;
	        }

	return(NULL); /* not found, add here if you want to add*/
	}
	else	/* no sense hashing if the facs table is built */
	{
	DEBUG(printf("BSEARCH: %s\n",s));
	return(bsearch_facs(obj, s));
	}
}


/*
 * compares two facilities a la strcmp but preserves
 * numbers for comparisons
 *
 * there are two flavors..the slow and accurate to any
 * arbitrary number of digits version (first) and the
 * fast one good to 2**31-1.  we default to the faster
 * version since there's probably no real need to
 * process ints larger than two billion anyway...
 */

#ifdef WAVE_USE_SIGCMP_INFINITE_PRECISION
int sigcmp(char *s1, char *s2)
{
char *n1, *n2;
unsigned char c1, c2;
int len1, len2;

for(;;)
	{
	c1=(unsigned char)*s1;
	c2=(unsigned char)*s2;

	if((c1==0)&&(c2==0)) return(0);
	if((c1>='0')&&(c1<='9')&&(c2>='0')&&(c2<='9'))
		{
		n1=s1; n2=s2;
		len1=len2=0;

		do	{
			len1++;
			c1=(unsigned char)*(n1++);
			} while((c1>='0')&&(c1<='9'));
		if(!c1) n1--;

		do	{
			len2++;
			c2=(unsigned char)*(n2++);
			} while((c2>='0')&&(c2<='9'));
		if(!c2) n2--;

		do	{
			if(len1==len2)
				{
				c1=(unsigned char)*(s1++);
				len1--;
				c2=(unsigned char)*(s2++);
				len2--;
				}
			else
			if(len1<len2)
				{
				c1='0';
				c2=(unsigned char)*(s2++);
				len2--;
				}
			else
				{
				c1=(unsigned char)*(s1++);
				len1--;
				c2='0';
				}

			if(c1!=c2) return((int)c1-(int)c2);
			} while(len1);

		s1=n1; s2=n2;
		continue;
		}
		else
		{
		if(c1!=c2) return((int)c1-(int)c2);
		}

	s1++; s2++;
	}
}
#else
int sigcmp(char *s1, char *s2)
{
unsigned char c1, c2;
int u1, u2;

for(;;)
	{
	c1=(unsigned char)*(s1++);
	c2=(unsigned char)*(s2++);

	if((!c1)&&(!c2)) return(0);
	if((c1<='9')&&(c2<='9')&&(c2>='0')&&(c1>='0'))
		{
		u1=(int)(c1&15);
		u2=(int)(c2&15);

		while(((c2=(unsigned char)*s2)>='0')&&(c2<='9'))
			{
			u2*=10;
			u2+=(unsigned int)(c2&15);
			s2++;
			}

		while(((c2=(unsigned char)*s1)>='0')&&(c2<='9'))
			{
			u1*=10;
			u1+=(unsigned int)(c2&15);
			s1++;
			}

		if(u1==u2) continue;
			else return((int)u1-(int)u2);
		}
		else
		{
		if(c1!=c2) return((int)c1-(int)c2);
		}
	}
}
#endif


/*
 * Quicksort algorithm from p154 of
 * "Introduction to Algorithms" by Cormen, Leiserson, and Rivest.
 * 05-jul-97bsi
 */

int partition(struct symbol **a, int p, int r)
{
struct symbol *x, *t;
int i,j;

x=a[p];
i=p-1;
j=r+1;

while(1)
	{
	do
		{
		j--;
		} while(sigcmp(a[j]->name,x->name)>0);

	do	{
		i++;
		} while(sigcmp(a[i]->name,x->name)<0);

	if(i<j)
		{
		t=a[i];
		a[i]=a[j];
		a[j]=t;
		}
		else
		{
		return(j);
		}
	}
}

void quicksort(struct symbol **a, int p, int r)
{
int q;

if(p<r)
	{
	q=partition(a,p,r);
	quicksort(a,p,q);
	quicksort(a,q+1,r);
	}
}
