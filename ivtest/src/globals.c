/*
 * Copyright (c) Tony Bybell 1999-2000.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"
#include "misc.h"

struct globals *make_vcd_class(void)
{
struct globals *g;

g=(struct globals *)calloc_2(1, sizeof(struct globals));

g->sym=NULL;
g->facs=NULL;
g->facs_are_sorted=0;

g->numfacs=0;
g->regions=0;
g->longestname=0;

g->firstnode=NULL;
g->curnode=NULL;

g->hier_delimeter='.';
g->autocoalesce=1;

g->vcd_explicit_zero_subscripts=-1;
g->convert_to_reals=0;
g->atomic_vectors=1;

g->vcd_handle=NULL;
g->vcd_is_compressed=0;

g->vcdbyteno=0;
g->header_over=0;
g->dumping_off=0;
g->start_time=-1;
g->end_time=-1;
g->current_time=-1;
g->time_scale=1;

g->count_glitches=0;
g->num_glitches=0;
g->num_glitch_regions=0;

g->vcd_hier_delimeter[0]=0;
g->vcd_hier_delimeter[1]=0;

g->pv=NULL;
g->rootv=NULL;

g->slistroot=NULL;
g->slistcurr=NULL;
g->slisthier=NULL;
g->slisthier_len=0;

g->T_MAX_STR=1024;
g->yytext=NULL;
g->yylen=0;
g->yylen_cache=0;

g->vcdsymroot=NULL;
g->vcdsymcurr=NULL;
g->sorted=NULL;

g->numsyms=0;

g->he_curr=NULL;
g->he_fini=NULL;

g->vcdbuf=NULL;
g->vst=NULL;
g->vend=NULL;

g->varsplit=NULL;
g->vsplitcurr=NULL;

g->var_prevch=0;

g->currenttime=0;
g->max_time=0;
g->min_time=-1;
g->time_dimension='n';

g->sym=(struct symbol **)calloc_2(SYMPRIME,sizeof(struct symbol *));

return(g);
}
