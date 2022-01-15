/*
 * Copyright (c) Tony Bybell 1999-2000
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */


/*
 * vcdiff.c  12apr00 ajb
 */
#include <stdio.h>
#include "misc.h"
#include "globals.h"
#include "vcd.h"

int compare_symbols(TimeType max0, TimeType max1, struct symbol *s0, struct symbol *s1)
{
hptr h0, h1;
TimeType t0, t1;
int rc=0;

h0=&(s0->n->head);
h1=&(s1->n->head);

if(!s0->n->ext)
	{
	/* bit case */
	while((h0)&&(h1))
		{
		t0=h0->time;
		t1=h1->time;
		if((t0>max0)||(t1>max0)||(t0>max1)||(t1>max1)) break;

		if (h0->v.val!=h1->v.val)
			{
			fprintf(stdout, "*** '%s' value mismatch: "TTFormat"='%c' vs "TTFormat"='%c'\n", s0->name, h0->time, "0xz1"[h0->v.val], h1->time, "0xz1"[h1->v.val]);
			rc+=1;
			}

		if((h0->next)&&(h1->next))
			{
			if(t0==t1)
				{
				h0=h0->next;
				h1=h1->next;
				}
			else
			if(t0<t1)
				{
				h0=h0->next;
				}
			else
				{
				h1=h1->next;
				}
			continue;
			}
			else
			{
			return(rc);
			}
		}
	}
	else
	{
	/* vec case */
	while((h0)&&(h1))
		{
		t0=h0->time;
		t1=h1->time;
		if((t0>max0)||(t1>max0)||(t0>max1)||(t1>max1)) break;

		if ((h0->time>=0)&&(h1->time>=0))
		if ((h0->flags&(HIST_REAL|HIST_STRING))==(h1->flags&(HIST_REAL|HIST_STRING)))
			{
			if((h0->flags&HIST_REAL)&&(!(h0->flags&HIST_STRING)))
				{
				if(*((double *)h0->v.vector)!=*((double *)h1->v.vector))
					{
					fprintf(stdout, "*** '%s' value mismatch: "TTFormat"='%f' vs "TTFormat"='%f'\n", s0->name, h0->time, *((double *)h0->v.vector), h1->time, *((double *)h1->v.vector));
					rc+=1;
					}
				}
				else
				{
				if((h0->v.vector)&&(h1->v.vector))
					{
					if(strcmp(h0->v.vector, h1->v.vector))
						{
						fprintf(stdout, "*** '%s' value mismatch: "TTFormat"='%s' vs "TTFormat"='%s'\n", s0->name, h0->time, h0->v.vector, h1->time, h1->v.vector);
						rc+=1;
						}
					}
				}
			}

		if((h0->next)&&(h1->next))
			{
			if(t0==t1)
				{
				h0=h0->next;
				h1=h1->next;
				}
			else
			if(t0<t1)
				{
				h0=h0->next;
				}
			else
				{
				h1=h1->next;
				}
			continue;
			}
			else
			{
			return(rc);
			}
		}
	}

return(rc);
}


/*
 * the meat and potatoes...
 */
int main(int argc, char **argv)
{
int i, j;
struct globals *v[2];
int warnings=0;

if(argc<3)
	{
	fprintf(stderr, "Usage\n-----\n");
	fprintf(stderr, "%s file1 file2\n\n",argv[0]);
	fprintf(stderr,"Using -vcd as a filename accepts input from stdin.\n");
	exit(VCD_FAIL);
	}


if((!strcmp("-vcd",argv[1]))&&(!strcmp("-vcd",argv[2])))
	{
	fprintf(stderr, "Can only accept stdin input for one file, exiting\n");
	exit(VCD_FAIL);
	}

for(i=0;i<2;i++)
	{
	v[i]=make_vcd_class();
	vcd_main(v[i],argv[i+1]);
	fprintf(stdout,"\n");
	}

if((v[0]->numfacs)!=(v[1]->numfacs))
	{
	fprintf(stdout, "*** Number of symbols differ: %d vs %d.\n\n",v[0]->numfacs, v[1]->numfacs);
	warnings++;
	}

for(i=0;i<2;i++)
	{
	for(j=0;j<v[i]->numfacs;j++)
		{
		struct symbol *as;

		as=symfind(v[1-i],v[i]->facs[j]->name);
		if(!as)
			{
			fprintf(stdout, "*** '%s' not found in '%s'\n",v[i]->facs[j]->name, argv[2-i]);
			warnings++;
			}
			else
			{
			struct ExtNode *en0, *en1;
			en0=v[i]->facs[j]->n->ext;
			en1=as->n->ext;

			if( ((!en0)&&(!en1)) || ( ((en0)&&(en1)) && (en0->msi==en1->msi) && (en0->lsi==en1->lsi) ) )
				{
				v[i]->facs[j]->altsym=as;
				}
				else
				{
				fprintf(stdout, "*** '%s' size/direction mismatch\n", v[i]->facs[j]->name);
				warnings++;
				}
			}
		}
	}


for(j=0;j<v[0]->numfacs;j++)
	{
	struct symbol *s, *as;

	s=v[0]->facs[j];
	as=s->altsym;
	if(as)
		{
		warnings+=compare_symbols(v[0]->max_time, v[1]->max_time, s, as);
		}
	}

printf("\nEncountered %d warnings, exiting.\n",warnings);
exit(warnings?VCD_FAIL:0);
}
