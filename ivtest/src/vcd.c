/*
 * Copyright (c) Tony Bybell 1999-2000.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */


/*
 * vcd.c			23jan99ajb
 * evcd parts			29jun99ajb
 * profiler optimizations	15jul99ajb
 * more profiler optimizations	25jan00ajb
 * finsim parameter fix		26jan00ajb
 * vector rechaining code	03apr00ajb
 * multiple var section code	06apr00ajb
 * stripped from gtkwave	09apr00ajb
 */
#include "vcd.h"

#undef VCD_BSEARCH_IS_PERFECT		/* bsearch is imperfect under linux, but OK under AIX */

/******************************************************************/

static void add_histent(struct globals *obj, TimeType time, struct Node *n, char ch, int regadd, char *vector);
static void add_tail_histents(struct globals *obj);
static void vcd_build_symbols(struct globals *obj);
static void vcd_cleanup(struct globals *obj);
static void evcd_strcpy(char *dst, char *src);

/******************************************************************/

enum Tokens   { T_VAR, T_END, T_SCOPE, T_UPSCOPE,
		T_COMMENT, T_DATE, T_DUMPALL, T_DUMPOFF, T_DUMPON,
		T_DUMPVARS, T_ENDDEFINITIONS,
		T_DUMPPORTS, T_DUMPPORTSOFF, T_DUMPPORTSON, T_DUMPPORTSALL,
		T_TIMESCALE, T_VERSION, T_VCDCLOSE,
		T_EOF, T_STRING, T_UNKNOWN_KEY };

char *tokens[]={ "var", "end", "scope", "upscope",
		 "comment", "date", "dumpall", "dumpoff", "dumpon",
		 "dumpvars", "enddefinitions",
		 "dumpports", "dumpportsoff", "dumpportson", "dumpportsall",
		 "timescale", "version", "vcdclose",
		 "", "", "" };

#define NUM_TOKENS 18

#define T_GET(x) tok=get_token(x);if((tok==T_END)||(tok==T_EOF))break;

/******************************************************************/

enum VarTypes { V_EVENT, V_PARAMETER,
		V_INTEGER, V_REAL, V_REG, V_SUPPLY0,
		V_SUPPLY1, V_TIME, V_TRI, V_TRIAND, V_TRIOR,
		V_TRIREG, V_TRI0, V_TRI1, V_WAND, V_WIRE, V_WOR, V_PORT,
		V_END, V_LB, V_COLON, V_RB, V_STRING };

char *vartypes[]={ "event", "parameter",
		"integer", "real", "reg", "supply0",
		"supply1", "time", "tri", "triand", "trior",
		"trireg", "tri0", "tri1", "wand", "wire", "wor", "port",
		"$end", "", "", "", ""};

#define NUM_VTOKENS 19

/******************************************************************/

/*
 * histent structs are NEVER freed so this is OK..
 */
#define VCD_HISTENT_GRANULARITY 100

static struct HistEnt *histent_calloc(struct globals *obj)
{
if(obj->he_curr==obj->he_fini)
	{
	obj->he_curr=(struct HistEnt *)calloc_2(VCD_HISTENT_GRANULARITY, sizeof(struct HistEnt));
	obj->he_fini=obj->he_curr+VCD_HISTENT_GRANULARITY;
	}

return(obj->he_curr++);
}

/******************************************************************/

static struct queuedevent *queuedevents=NULL;

/******************************************************************/

/*
 * bsearch compare
 */
static int vcdsymbsearchcompare(const void *s1, const void *s2)
{
char *v1;
struct vcdsymbol *v2;

v1=(char *)s1;
v2=*((struct vcdsymbol **)s2);

return(strcmp(v1, v2->id));
}


/*
 * actual bsearch
 */
static struct vcdsymbol *bsearch_vcd(struct globals *obj, char *key)
{
struct vcdsymbol **v;
struct vcdsymbol *t;

v=(struct vcdsymbol **)bsearch(key, obj->sorted, obj->numsyms,
	sizeof(struct vcdsymbol *), vcdsymbsearchcompare);

if(v)
	{
	#ifndef VCD_BSEARCH_IS_PERFECT
		for(;;)
			{
			t=*v;

			if((v==obj->sorted)||(strcmp((*(--v))->id, key)))
				{
				return(t);
				}
			}
	#else
		return(*v);
	#endif
	}
	else
	{
	return(NULL);
	}
}


/*
 * sort on vcdsymbol pointers
 */
static int vcdsymcompare(const void *s1, const void *s2)
{
struct vcdsymbol *v1, *v2;

v1=*((struct vcdsymbol **)s1);
v2=*((struct vcdsymbol **)s2);

return(strcmp(v1->id, v2->id));
}


/*
 * create sorted (by id) table
 */
static void create_sorted_table(struct globals *obj)
{
struct vcdsymbol *v;
struct vcdsymbol **pnt;

if(obj->sorted)
	{
	free_2(obj->sorted);	/* this means we saw a 2nd enddefinition chunk! */
	}

if(obj->numsyms)
	{
	pnt=obj->sorted=(struct vcdsymbol **)calloc_2(obj->numsyms, sizeof(struct vcdsymbol *));
	v=obj->vcdsymroot;
	while(v)
		{
		*(pnt++)=v;
		v=v->next;
		}

	qsort(obj->sorted, obj->numsyms, sizeof(struct vcdsymbol *), vcdsymcompare);
	}
}

/******************************************************************/

/*
 * single char get inlined/optimized
 */
static void getch_alloc(struct globals *obj)
{
obj->vend=obj->vst=obj->vcdbuf=(char *)calloc_2(1,VCD_BSIZ);
}

static void getch_free(struct globals *obj)
{
free_2(obj->vcdbuf);
obj->vcdbuf=obj->vst=obj->vend=NULL;
}


static int getch_fetch(struct globals *obj)
{
size_t rd;

if(feof(obj->vcd_handle)||errno) return(-1);

obj->vcdbyteno+=(obj->vend-obj->vcdbuf);
rd=fread(obj->vcdbuf, sizeof(char), VCD_BSIZ, obj->vcd_handle);
obj->vend=(obj->vst=obj->vcdbuf)+rd;

if(!rd) return(-1);

return((int)(*(obj->vst++)));
}

#define getch(x) ((x->vst!=x->vend)?((int)(*(x->vst++))):(getch_fetch(x)))


static int getch_patched(struct globals *obj)
{
char ch;

ch=*obj->vsplitcurr;
if(!ch)
	{
	return(-1);
	}
	else
	{
	obj->vsplitcurr++;
	return((int)ch);
	}
}

/*
 * simple tokenizer
 */
static int get_token(struct globals *obj)
{
int ch;
int i, len=0;
int is_string=0;

for(;;)
	{
	ch=getch(obj);
	if(ch<0) return(T_EOF);
	if(ch<=' ') continue;	/* val<=' ' is a quick whitespace check      */
	break;			/* (take advantage of fact that vcd is text) */
	}
if(ch=='$')
	{
	obj->yytext[len++]=ch;
	for(;;)
		{
		ch=getch(obj);
		if(ch<0) return(T_EOF);
		if(ch<=' ') continue;
		break;
		}
	}
	else
	{
	is_string=1;
	}

for(obj->yytext[len++]=ch;;obj->yytext[len++]=ch)
	{
	if(len==obj->T_MAX_STR)
		{
		obj->yytext=(char *)realloc_2(obj->yytext, (obj->T_MAX_STR=obj->T_MAX_STR*2)+1);
		}
	ch=getch(obj);
	if(ch<=' ') break;
	}
obj->yytext[len]=0;	/* terminator */

if(is_string)
	{
	obj->yylen=len;
	return(T_STRING);
	}

for(i=0;i<NUM_TOKENS;i++)
	{
	if(!strcmp(obj->yytext+1,tokens[i]))
		{
		return(i);
		}
	}

return(T_UNKNOWN_KEY);
}


static int get_vartoken_patched(struct globals *obj)
{
int ch;
int i, len=0;

if(!obj->var_prevch)
	{
	for(;;)
		{
		ch=getch_patched(obj);
		if(ch<0) { free_2(obj->varsplit); obj->varsplit=NULL; return(V_END); }
		if((ch==' ')||(ch=='\t')||(ch=='\n')||(ch=='\r')) continue;
		break;
		}
	}
	else
	{
	ch=obj->var_prevch;
	obj->var_prevch=0;
	}

if(ch=='[') return(V_LB);
if(ch==':') return(V_COLON);
if(ch==']') return(V_RB);

for(obj->yytext[len++]=ch;;obj->yytext[len++]=ch)
	{
	if(len==obj->T_MAX_STR)
		{
		obj->yytext=(char *)realloc_2(obj->yytext, (obj->T_MAX_STR=obj->T_MAX_STR*2)+1);
		}
	ch=getch_patched(obj);
	if(ch<0) break;
	if((ch==':')||(ch==']'))
		{
		obj->var_prevch=ch;
		break;
		}
	}
obj->yytext[len]=0;	/* terminator */

for(i=0;i<NUM_VTOKENS;i++)
	{
	if(!strcmp(obj->yytext,vartypes[i]))
		{
		if(ch<0) { free_2(obj->varsplit); obj->varsplit=NULL; }
		return(i);
		}
	}

obj->yylen=len;
if(ch<0) { free_2(obj->varsplit); obj->varsplit=NULL; }
return(V_STRING);
}

static int get_vartoken(struct globals *obj)
{
int ch;
int i, len=0;

if(obj->varsplit)
	{
	int rc=get_vartoken_patched(obj);
	if(rc!=V_END) return(rc);
	obj->var_prevch=0;
	}

if(!obj->var_prevch)
	{
	for(;;)
		{
		ch=getch(obj);
		if(ch<0) return(V_END);
		if((ch==' ')||(ch=='\t')||(ch=='\n')||(ch=='\r')) continue;
		break;
		}
	}
	else
	{
	ch=obj->var_prevch;
	obj->var_prevch=0;
	}

if(ch=='[') return(V_LB);
if(ch==':') return(V_COLON);
if(ch==']') return(V_RB);

for(obj->yytext[len++]=ch;;obj->yytext[len++]=ch)
	{
	if(len==obj->T_MAX_STR)
		{
		obj->yytext=(char *)realloc_2(obj->yytext, (obj->T_MAX_STR=obj->T_MAX_STR*2)+1);
		}
	ch=getch(obj);
	if((ch==' ')||(ch=='\t')||(ch=='\n')||(ch=='\r')||(ch<0)) break;
	if((ch=='[')&&(obj->yytext[0]!='\\'))
		{
		obj->varsplit=obj->yytext+len;		/* keep looping so we get the *last* one */
		}
	else
	if(((ch==':')||(ch==']'))&&(!obj->varsplit)&&(obj->yytext[0]!='\\'))
		{
		obj->var_prevch=ch;
		break;
		}
	}
obj->yytext[len]=0;	/* absolute terminator */
if((obj->varsplit)&&(obj->yytext[len-1]==']'))
	{
	char *vst;
	vst=malloc_2(strlen(obj->varsplit)+1);
	strcpy(vst, obj->varsplit);

	*obj->varsplit=0x00;		/* zero out var name at the left bracket */
	len=obj->varsplit-obj->yytext;

	obj->varsplit=obj->vsplitcurr=vst;
	obj->var_prevch=0;
	}
	else
	{
	obj->varsplit=NULL;
	}

for(i=0;i<NUM_VTOKENS;i++)
	{
	if(!strcmp(obj->yytext,vartypes[i]))
		{
		return(i);
		}
	}

obj->yylen=len;
return(V_STRING);
}

static int get_strtoken(struct globals *obj)
{
int ch;
int len=0;

if(!obj->var_prevch)
      {
      for(;;)
              {
              ch=getch(obj);
              if(ch<0) return(V_END);
              if((ch==' ')||(ch=='\t')||(ch=='\n')||(ch=='\r')) continue;
              break;
              }
      }
      else
      {
      ch=obj->var_prevch;
      obj->var_prevch=0;
      }

for(obj->yytext[len++]=ch;;obj->yytext[len++]=ch)
      {
	if(len==obj->T_MAX_STR)
		{
		obj->yytext=(char *)realloc_2(obj->yytext, (obj->T_MAX_STR=obj->T_MAX_STR*2)+1);
		}
      ch=getch(obj);
      if((ch==' ')||(ch=='\t')||(ch=='\n')||(ch=='\r')||(ch<0)) break;
      }
obj->yytext[len]=0;        /* terminator */

obj->yylen=len;
return(V_STRING);
}

static void sync_end(struct globals *obj, char *hdr)
{
int tok;

if(hdr) DEBUG(fprintf(stderr,"%s",hdr));
for(;;)
	{
	tok=get_token(obj);
	if((tok==T_END)||(tok==T_EOF)) break;
	if(hdr)DEBUG(fprintf(stderr," %s",yytext));
	}
if(hdr) DEBUG(fprintf(stderr,"\n"));
}

static char *build_slisthier(struct globals *obj)
{
struct slist *s;
int len=0;

if(!obj->slistroot)
	{
	if(obj->slisthier)
		{
		free_2(obj->slisthier);
		}

	obj->slisthier_len=0;
	obj->slisthier=(char *)malloc_2(1);
	*obj->slisthier=0;
	return(obj->slisthier);
	}

s=obj->slistroot; len=0;
while(s)
	{
	len+=s->len+(s->next?1:0);
	s=s->next;
	}

obj->slisthier=(char *)malloc_2((obj->slisthier_len=len)+1);
s=obj->slistroot; len=0;
while(s)
	{
	strcpy(obj->slisthier+len,s->str);
	len+=s->len;
	if(s->next)
		{
		strcpy(obj->slisthier+len,obj->vcd_hier_delimeter);
		len++;
		}
	s=s->next;
	}
return(obj->slisthier);
}


void append_vcd_slisthier(struct globals *obj, char *str)
{
struct slist *s;
s=(struct slist *)calloc_2(1,sizeof(struct slist));
s->len=strlen(str);
s->str=(char *)malloc_2(s->len+1);
strcpy(s->str,str);

if(obj->slistcurr)
	{
	obj->slistcurr->next=s;
	obj->slistcurr=s;
	}
	else
	{
	obj->slistcurr=obj->slistroot=s;
	}

build_slisthier(obj);
DEBUG(fprintf(stderr, "SCOPE: %s\n",obj->slisthier));
}


static void parse_valuechange(struct globals *obj)
{
struct vcdsymbol *v;
char *vector;
int vlen;

switch(obj->yytext[0])
	{
	case '0':
	case '1':
	case 'x':
	case 'X':
	case 'z':
	case 'Z':
		if(obj->yylen>1)
			{
			v=bsearch_vcd(obj, obj->yytext+1);
			if(!v)
				{
				fprintf(stderr,"Near byte %d, Unknown VCD identifier: '%s'\n",obj->vcdbyteno+(obj->vst-obj->vcdbuf),obj->yytext+1);
				}
				else
				{
				if(v->vartype!=V_EVENT)
					{
					v->value[0]=obj->yytext[0];
					DEBUG(fprintf(stderr,"%s = '%c'\n",v->name,v->value[0]));
					add_histent(obj, obj->current_time,v->narray[0],v->value[0],1, NULL);
					}
					else
					{
					v->value[0]=(obj->dumping_off)?'x':'1'; /* only '1' is relevant */
					if(obj->current_time!=(v->ev->last_event_time+1))
						{
						/* dump degating event */
						DEBUG(fprintf(stderr,"#"TTFormat" %s = '%c' (event)\n",v->ev->last_event_time+1,v->name,'0'));
						add_histent(obj, v->ev->last_event_time+1,v->narray[0],'0',1, NULL);
						}
					DEBUG(fprintf(stderr,"%s = '%c' (event)\n",v->name,v->value[0]));
					add_histent(obj, obj->current_time,v->narray[0],v->value[0],1, NULL);
					v->ev->last_event_time=obj->current_time;
					}
				}
			}
			else
			{
			fprintf(stderr,"Near byte %d, Malformed VCD identifier\n", obj->vcdbyteno+(obj->vst-obj->vcdbuf));
			}
		break;

	case 'b':
	case 'B':
		{
		/* extract binary number then.. */
		vector=malloc_2(obj->yylen_cache=obj->yylen);
		strcpy(vector,obj->yytext+1);
		vlen=obj->yylen-1;

		get_strtoken(obj);
		v=bsearch_vcd(obj, obj->yytext);
		if(!v)
			{
			fprintf(stderr,"Near byte %d, Unknown identifier: '%s'\n",obj->vcdbyteno+(obj->vst-obj->vcdbuf), obj->yytext);
			free_2(vector);
			}
			else
			{
			if((obj->convert_to_reals)&&(v->vartype==V_REAL))
				{
				double *d;
				char *pnt;
				char ch;
				TimeType k=0;

				pnt=vector;
				while((ch=*(pnt++))) { k=(k<<1)|((ch=='1')?1:0); }
				free_2(vector);

				d=malloc_2(sizeof(double));
				*d=(double)k;

				if(!v)
					{
					fprintf(stderr,"Near byte %d, Unknown identifier: '%s'\n",obj->vcdbyteno+(obj->vst-obj->vcdbuf), obj->yytext);
					free_2(d);
					}
					else
					{
					add_histent(obj, obj->current_time, v->narray[0],'g',1,(char *)d);
					}
				break;
				}

			if(vlen<v->size)	/* fill in left part */
				{
				char extend;
				int i, fill;

				extend=(vector[0]=='1')?'0':vector[0];

				fill=v->size-vlen;
				for(i=0;i<fill;i++)
					{
					v->value[i]=extend;
					}
				strcpy(v->value+fill,vector);
				}
			else if(vlen==v->size)	/* straight copy */
				{
				strcpy(v->value,vector);
				}
			else			/* too big, so copy only right half */
				{
				int skip;

				skip=vlen-v->size;
				strcpy(v->value,vector+skip);
				}
			DEBUG(fprintf(stderr,"%s = '%s'\n",v->name, v->value));

			if((v->size==1)||(!obj->atomic_vectors))
				{
				int i;
				for(i=0;i<v->size;i++)
					{
					add_histent(obj, obj->current_time, v->narray[i],v->value[i],1, NULL);
					}
				free_2(vector);
				}
				else
				{
				if(obj->yylen_cache!=(v->size+1))
					{
					free_2(vector);
					vector=malloc_2(v->size+1);
					}
				strcpy(vector,v->value);
				add_histent(obj, obj->current_time, v->narray[0],0,1,vector);
				}

			}
		break;
		}

	case 'p':
		/* extract port dump value.. */
		vector=malloc_2(obj->yylen_cache=obj->yylen);
		strcpy(vector,obj->yytext+1);
		vlen=obj->yylen-1;

		get_strtoken(obj);	/* throw away 0_strength_component */
		get_strtoken(obj); /* throw away 0_strength_component */
		get_strtoken(obj); /* this is the id                  */
		v=bsearch_vcd(obj, obj->yytext);
		if(!v)
			{
			fprintf(stderr,"Near byte %d, Unknown identifier: '%s'\n",obj->vcdbyteno+(obj->vst-obj->vcdbuf), obj->yytext);
			free_2(vector);
			}
			else
			{
			if((obj->convert_to_reals)&&(v->vartype==V_REAL))	/* should never happen, but just in case.. */
				{
				double *d;
				char *pnt;
				char ch;
				TimeType k=0;

				pnt=vector;
				while((ch=*(pnt++))) { k=(k<<1)|((ch=='1')?1:0); }
				free_2(vector);

				d=malloc_2(sizeof(double));
				*d=(double)k;

				if(!v)
					{
					fprintf(stderr,"Near byte %d, Unknown identifier: '%s'\n",obj->vcdbyteno+(obj->vst-obj->vcdbuf), obj->yytext);
					free_2(d);
					}
					else
					{
					add_histent(obj, obj->current_time, v->narray[0],'g',1,(char *)d);
					}
				break;
				}

			if(vlen<v->size)	/* fill in left part */
				{
				char extend;
				int i, fill;

				extend='0';

				fill=v->size-vlen;
				for(i=0;i<fill;i++)
					{
					v->value[i]=extend;
					}
				evcd_strcpy(v->value+fill,vector);
				}
			else if(vlen==v->size)	/* straight copy */
				{
				evcd_strcpy(v->value,vector);
				}
			else			/* too big, so copy only right half */
				{
				int skip;

				skip=vlen-v->size;
				evcd_strcpy(v->value,vector+skip);
				}
			DEBUG(fprintf(stderr,"%s = '%s'\n",v->name, v->value));

			if((v->size==1)||(!obj->atomic_vectors))
				{
				int i;
				for(i=0;i<v->size;i++)
					{
					add_histent(obj, obj->current_time, v->narray[i],v->value[i],1, NULL);
					}
				free_2(vector);
				}
				else
				{
				if(obj->yylen_cache<v->size)
					{
					free_2(vector);
					vector=malloc_2(v->size+1);
					}
				strcpy(vector,v->value);
				add_histent(obj, obj->current_time, v->narray[0],0,1,vector);
				}
			}
		break;


	case 'r':
	case 'R':
		{
		double *d;

		d=malloc_2(sizeof(double));
		sscanf(obj->yytext+1,"%lg",d);

		get_strtoken(obj);
		v=bsearch_vcd(obj, obj->yytext);
		if(!v)
			{
			fprintf(stderr,"Near byte %d, Unknown identifier: '%s'\n",obj->vcdbyteno+(obj->vst-obj->vcdbuf), obj->yytext);
			free_2(d);
			}
			else
			{
			add_histent(obj, obj->current_time, v->narray[0],'g',1,(char *)d);
			}

		break;
		}

#ifndef STRICT_VCD_ONLY
	case 's':
	case 'S':
		{
		char *d;

		d=(char *)malloc_2(obj->yylen);
		strcpy(d, obj->yytext+1);

		get_strtoken(obj);
		v=bsearch_vcd(obj, obj->yytext);
		if(!v)
			{
			fprintf(stderr,"Near byte %d, Unknown identifier: '%s'\n",obj->vcdbyteno+(obj->vst-obj->vcdbuf), obj->yytext);
			free_2(d);
			}
			else
			{
			add_histent(obj, obj->current_time, v->narray[0],'s',1,(char *)d);
			}

		break;
		}
#endif
	}

}


static void evcd_strcpy(char *dst, char *src)
{
static char *evcd="DUNZduLHXTlh01?FAaBbCcf";
static char  *vcd="01xz0101xz0101xzxxxxxxx";

char ch;
int i;

while((ch=*src))
	{
	for(i=0;i<23;i++)
		{
		if(evcd[i]==ch)
			{
			*dst=vcd[i];
			break;
			}
		}
	if(i==23) *dst='x';

	src++;
	dst++;
	}

*dst=0;	/* null terminate destination */
}


static void vcd_parse(struct globals *obj)
{
int tok;

for(;;)
	{
	switch(tok=get_token(obj))
		{
		case T_COMMENT:
			sync_end(obj, "COMMENT:");
			break;
		case T_DATE:
			sync_end(obj, "DATE:");
			break;
		case T_VERSION:
			sync_end(obj, "VERSION:");
			break;
		case T_TIMESCALE:
			{
			int vtok;
			int i;
			char prefix=' ';

			vtok=get_token(obj);
			if((vtok==T_END)||(vtok==T_EOF)) break;
			obj->time_scale=atoi_64(obj->yytext);
			if(!obj->time_scale) obj->time_scale=1;
			for(i=0;i<obj->yylen;i++)
				{
				if((obj->yytext[i]<'0')||(obj->yytext[i]>'9'))
					{
					prefix=obj->yytext[i];
					break;
					}
				}
			if(prefix==' ')
				{
				vtok=get_token(obj);
				if((vtok==T_END)||(vtok==T_EOF)) break;
				prefix=obj->yytext[0];
				}
			switch(prefix)
				{
				case ' ':
				case 'm':
				case 'u':
				case 'n':
				case 'p':
				case 'f':
					obj->time_dimension=prefix;
					break;
				case 's':
					obj->time_dimension=' ';
					break;
				default:	/* unknown */
					obj->time_dimension='n';
					break;
				}

			DEBUG(fprintf(stderr,"TIMESCALE: "TTFormat" %cs\n",obj->time_scale, obj->time_dimension));
			sync_end(obj, NULL);
			}
			break;
		case T_SCOPE:
			T_GET(obj);
			T_GET(obj);
			if(tok==T_STRING)
				{
				struct slist *s;
				s=(struct slist *)calloc_2(1,sizeof(struct slist));
				s->len=obj->yylen;
				s->str=(char *)malloc_2(obj->yylen+1);
				strcpy(s->str,obj->yytext);

				if(obj->slistcurr)
					{
					obj->slistcurr->next=s;
					obj->slistcurr=s;
					}
					else
					{
					obj->slistcurr=obj->slistroot=s;
					}

				build_slisthier(obj);
				DEBUG(fprintf(stderr, "SCOPE: %s\n",obj->slisthier));
				}
			sync_end(obj, NULL);
			break;
		case T_UPSCOPE:
			if(obj->slistroot)
				{
				struct slist *s;

				s=obj->slistroot;
				if(!s->next)
					{
					free_2(s->str);
					free_2(s);
					obj->slistroot=obj->slistcurr=NULL;
					}
				else
				for(;;)
					{
					if(!s->next->next)
						{
						free_2(s->next->str);
						free_2(s->next);
						s->next=NULL;
						obj->slistcurr=s;
						break;
						}
					s=s->next;
					}
				build_slisthier(obj);
				DEBUG(fprintf(stderr, "SCOPE: %s\n",obj->slisthier));
				}
			sync_end(obj, NULL);
			break;
		case T_VAR:
			if((obj->header_over)&&(0))
			{
			fprintf(stderr,"$VAR encountered after $ENDDEFINITIONS near byte %d.  VCD is malformed, exiting.\n",obj->vcdbyteno+(obj->vst-obj->vcdbuf));
			exit(VCD_FAIL);
			}
			else
			{
			int vtok;
			struct vcdsymbol *v=NULL;

			obj->var_prevch=0;
			obj->varsplit=NULL;
			vtok=get_vartoken(obj);
			if(vtok>V_PORT) goto bail;

			v=(struct vcdsymbol *)calloc_2(1,sizeof(struct vcdsymbol));
			v->vartype=vtok;
			v->msi=v->lsi=obj->vcd_explicit_zero_subscripts; /* indicate [un]subscripted status */

			if(vtok==V_PORT)
				{
				vtok=get_vartoken(obj);
				if(vtok==V_STRING)
					{
					v->size=atoi_64(obj->yytext);
					if(!v->size) v->size=1;
					}
					else
					if(vtok==V_LB)
					{
					vtok=get_vartoken(obj);
					if(vtok==V_END) goto err;
					if(vtok!=V_STRING) goto err;
					v->msi=atoi_64(obj->yytext);
					vtok=get_vartoken(obj);
					if(vtok==V_RB)
						{
						v->lsi=v->msi;
						v->size=1;
						}
						else
						{
						if(vtok!=V_COLON) goto err;
						vtok=get_vartoken(obj);
						if(vtok!=V_STRING) goto err;
						v->lsi=atoi_64(obj->yytext);
						vtok=get_vartoken(obj);
						if(vtok!=V_RB) goto err;

						if(v->msi>v->lsi)
							{
							v->size=v->msi-v->lsi+1;
							}
							else
							{
							v->size=v->lsi-v->msi+1;
							}
						}
					}
					else goto err;

				vtok=get_strtoken(obj);
				if(vtok==V_END) goto err;
				v->id=(char *)malloc_2(obj->yylen+1);
				strcpy(v->id, obj->yytext);

				vtok=get_vartoken(obj);
				if(vtok!=V_STRING) goto err;
				if(obj->slisthier_len)
					{
					v->name=(char *)malloc_2(obj->slisthier_len+1+obj->yylen+1);
					strcpy(v->name,obj->slisthier);
					strcpy(v->name+obj->slisthier_len,obj->vcd_hier_delimeter);
					strcpy(v->name+obj->slisthier_len+1,obj->yytext);
					}
					else
					{
					v->name=(char *)malloc_2(obj->yylen+1);
					strcpy(v->name,obj->yytext);
					}

                                if(obj->pv)
                                        {
                                        if(!strcmp(obj->pv->name,v->name))
                                                {
                                                obj->pv->chain=v;
                                                v->root=obj->rootv;
                                                if(obj->pv==obj->rootv) obj->pv->root=obj->rootv;
                                                }
                                                else
                                                {
                                                obj->rootv=v;
                                                }
                                        }
					else
					{
					obj->rootv=v;
					}
                                obj->pv=v;
				}
				else	/* regular vcd var, not an evcd port var */
				{
				vtok=get_vartoken(obj);
				if(vtok==V_END) goto err;
				v->size=atoi_64(obj->yytext);
				if(!v->size) v->size=1;
				vtok=get_strtoken(obj);
				if(vtok==V_END) goto err;
				v->id=(char *)malloc_2(obj->yylen+1);
				strcpy(v->id, obj->yytext);

				vtok=get_vartoken(obj);
				if(vtok!=V_STRING) goto err;
				if(obj->slisthier_len)
					{
					v->name=(char *)malloc_2(obj->slisthier_len+1+obj->yylen+1);
					strcpy(v->name,obj->slisthier);
					strcpy(v->name+obj->slisthier_len,obj->vcd_hier_delimeter);
					strcpy(v->name+obj->slisthier_len+1,obj->yytext);
					}
					else
					{
					v->name=(char *)malloc_2(obj->yylen+1);
					strcpy(v->name,obj->yytext);
					}

                                if(obj->pv)
                                        {
                                        if(!strcmp(obj->pv->name,v->name))
                                                {
                                                obj->pv->chain=v;
                                                v->root=obj->rootv;
                                                if(obj->pv==obj->rootv) obj->pv->root=obj->rootv;
                                                }
                                                else
                                                {
                                                obj->rootv=v;
                                                }
                                        }
					else
					{
					obj->rootv=v;
					}
                                obj->pv=v;

				vtok=get_vartoken(obj);
				if(vtok==V_END) goto dumpv;
				if(vtok!=V_LB) goto err;
				vtok=get_vartoken(obj);
				if(vtok!=V_STRING) goto err;
				v->msi=atoi_64(obj->yytext);
				vtok=get_vartoken(obj);
				if(vtok==V_RB)
					{
					v->lsi=v->msi;
					goto dumpv;
					}
				if(vtok!=V_COLON) goto err;
				vtok=get_vartoken(obj);
				if(vtok!=V_STRING) goto err;
				v->lsi=atoi_64(obj->yytext);
				vtok=get_vartoken(obj);
				if(vtok!=V_RB) goto err;
				}

			dumpv:
			if((v->vartype==V_REAL)||((obj->convert_to_reals)&&((v->vartype==V_INTEGER)||(v->vartype==V_PARAMETER))))
				{
				v->vartype=V_REAL;
				v->size=1;		/* override any data we parsed in */
				v->msi=v->lsi=0;
				}
			else
			if((v->size>1)&&(v->msi<=0)&&(v->lsi<=0))
				{
				if(v->vartype==V_EVENT)
					{
					v->size=1;
					}
					else
					{
					/* any criteria for the direction here? */
					v->msi=v->size-1;
					v->lsi=0;
					}
				}
			else
			if((v->msi>v->lsi)&&((v->msi-v->lsi+1)!=v->size))
				{
				if((v->vartype!=V_EVENT)&&(v->vartype!=V_PARAMETER)) goto err;
				v->size=v->msi-v->lsi+1;
				}
			else
			if((v->lsi>=v->msi)&&((v->lsi-v->msi+1)!=v->size))
				{
				if((v->vartype!=V_EVENT)&&(v->vartype!=V_PARAMETER)) goto err;
				v->size=v->msi-v->lsi+1;
				}

			/* initial conditions */
			v->value=(char *)malloc_2(v->size+1);
			v->value[v->size]=0;
			v->narray=(struct Node **)calloc_2(v->size,sizeof(struct Node *));
				{
				int i;
				if(obj->atomic_vectors)
					{
					for(i=0;i<v->size;i++)
						{
						v->value[i]='x';
						}
					v->narray[0]=(struct Node *)calloc_2(1,sizeof(struct Node));
					v->narray[0]->head.time=-1;
					v->narray[0]->head.v.val=1;
					}
					else
					{
					for(i=0;i<v->size;i++)
						{
						v->value[i]='x';

						v->narray[i]=(struct Node *)calloc_2(1,sizeof(struct Node));
						v->narray[i]->head.time=-1;
						v->narray[i]->head.v.val=1;
						}
					}
				}

			if(v->vartype==V_EVENT)
				{
				struct queuedevent *q;
				v->ev=q=(struct queuedevent *)calloc_2(1,sizeof(struct queuedevent));
				q->sym=v;
				q->last_event_time=-1;
				q->next=queuedevents;
				queuedevents=q;
				}

			if(!obj->vcdsymroot)
				{
				obj->vcdsymroot=obj->vcdsymcurr=v;
				}
				else
				{
				obj->vcdsymcurr->next=v;
				obj->vcdsymcurr=v;
				}
			obj->numsyms++;

			DEBUG(fprintf(stderr,"VAR %s %d %s %s[%d:%d]\n",
				vartypes[v->vartype], v->size, v->id, v->name,
					v->msi, v->lsi));
			goto bail;
			err:
			if(v)
				{
				if(v->name) free_2(v->name);
				if(v->id) free_2(v->id);
				if(v->value) free_2(v->value);
				free_2(v);
				}

			bail:
			if(vtok!=V_END) sync_end(obj, NULL);
			break;
			}
		case T_ENDDEFINITIONS:
			obj->header_over=1;	/* do symbol table management here */
			create_sorted_table(obj);
			if(!obj->sorted)
				{
				fprintf(stderr, "No symbols in VCD file..nothing to do!\n");
				exit(VCD_FAIL);
				}
			break;
		case T_STRING:
			if(obj->header_over)
				{
				/* catchall for events when header over */
				if(obj->yytext[0]=='#')
					{
					TimeType time;
					time=atoi_64(obj->yytext+1);

					if(obj->start_time<0)
						{
						obj->start_time=time;
						}

					obj->current_time=time;
					if(obj->end_time<time) obj->end_time=time;	/* in case of malformed vcd files */
					DEBUG(fprintf(stderr,"#"TTFormat"\n",time));
					}
					else
					{
					parse_valuechange(obj);
					}
				}
			break;
		case T_DUMPALL:	/* dump commands modify vals anyway so */
		case T_DUMPPORTSALL:
			break;	/* just loop through..                 */
		case T_DUMPOFF:
		case T_DUMPPORTSOFF:
			obj->dumping_off=1;
			break;
		case T_DUMPON:
		case T_DUMPPORTSON:
			obj->dumping_off=0;
			break;
		case T_DUMPVARS:
		case T_DUMPPORTS:
			break;
		case T_VCDCLOSE:
			break;	/* next token will be '#' time related followed by $end */
		case T_END:	/* either closure for dump commands or */
			break;	/* it's spurious                       */
		case T_UNKNOWN_KEY:
			sync_end(obj, NULL);	/* skip over unknown keywords */
			break;
		case T_EOF:
			return;
		default:
			DEBUG(fprintf(stderr,"UNKNOWN TOKEN\n"));
		}
	}
}


/*******************************************************************************/

void add_histent(struct globals *obj, TimeType time, struct Node *n, char ch, int regadd, char *vector)
{
struct HistEnt *he;
char heval;

if(!vector)
{
if(!n->curr)
	{
	he=histent_calloc(obj);
        he->time=-1;
        he->v.val=1;

	n->curr=he;
	n->head.next=he;

	add_histent(obj, time,n,ch,regadd, vector);
	}
	else
	{
	if(regadd) { time*=(obj->time_scale); }

        if(ch=='0') heval=0; else
        if(ch=='1') heval=3; else
        if((ch=='x')||(ch=='X')) heval=1; else
        heval=2;

	if((n->curr->v.val!=heval)||(time==obj->start_time)) /* same region == go skip */
		{
		if((n->curr->time==time)&&(obj->count_glitches))
			{
			DEBUG(printf("Warning: Glitch at time ["TTFormat"] Signal [%p], Value [%c->%c].\n",
				time, n, "0XZ1"[n->curr->v.val], ch));
			n->curr->v.val=heval;		/* we have a glitch! */

			obj->num_glitches++;
			if(!(n->curr->flags&HIST_GLITCH))
				{
				n->curr->flags|=HIST_GLITCH;	/* set the glitch flag */
				obj->num_glitch_regions++;
				}
			}
			else
			{
			he=histent_calloc(obj);
			he->time=time;
			he->v.val=heval;

			n->curr->next=he;
			n->curr=he;
			obj->regions+=regadd;
			}
                }
       }
}
else
{
switch(ch)
	{
	case 's': /* string */
	{
	if(!n->curr)
		{
		he=histent_calloc(obj);
		he->flags=(HIST_STRING|HIST_REAL);
	        he->time=-1;
	        he->v.vector=NULL;

		n->curr=he;
		n->head.next=he;

		add_histent(obj, time,n,ch,regadd, vector);
		}
		else
		{
		if(regadd) { time*=(obj->time_scale); }

			if((n->curr->time==time)&&(obj->count_glitches))
				{
				DEBUG(printf("Warning: String Glitch at time ["TTFormat"] Signal [%p].\n",
					time, n));
				if(n->curr->v.vector) free_2(n->curr->v.vector);
				n->curr->v.vector=vector;		/* we have a glitch! */

				obj->num_glitches++;
				if(!(n->curr->flags&HIST_GLITCH))
					{
					n->curr->flags|=HIST_GLITCH;	/* set the glitch flag */
					obj->num_glitch_regions++;
					}
				}
				else
				{
				he=histent_calloc(obj);
				he->flags=(HIST_STRING|HIST_REAL);
				he->time=time;
				he->v.vector=vector;

				n->curr->next=he;
				n->curr=he;
				obj->regions+=regadd;
				}
	       }
	break;
	}
	case 'g': /* real number */
	{
	if(!n->curr)
		{
		he=histent_calloc(obj);
		he->flags=HIST_REAL;
	        he->time=-1;
	        he->v.vector=NULL;

		n->curr=he;
		n->head.next=he;

		add_histent(obj, time,n,ch,regadd, vector);
		}
		else
		{
		if(regadd) { time*=(obj->time_scale); }

		if(
		  (n->curr->v.vector&&vector&&(*(double *)n->curr->v.vector!=*(double *)vector))
			||(time==obj->start_time)
			||(!n->curr->v.vector)
			) /* same region == go skip */
			{
			if((n->curr->time==time)&&(obj->count_glitches))
				{
				DEBUG(printf("Warning: Real number Glitch at time ["TTFormat"] Signal [%p].\n",
					time, n));
				if(n->curr->v.vector) free_2(n->curr->v.vector);
				n->curr->v.vector=vector;		/* we have a glitch! */

				obj->num_glitches++;
				if(!(n->curr->flags&HIST_GLITCH))
					{
					n->curr->flags|=HIST_GLITCH;	/* set the glitch flag */
					obj->num_glitch_regions++;
					}
				}
				else
				{
				he=histent_calloc(obj);
				he->flags=HIST_REAL;
				he->time=time;
				he->v.vector=vector;

				n->curr->next=he;
				n->curr=he;
				obj->regions+=regadd;
				}
	                }
			else
			{
			free_2(vector);
			}
	       }
	break;
	}
	default:
	{
	if(!n->curr)
		{
		he=histent_calloc(obj);
	        he->time=-1;
	        he->v.vector=NULL;

		n->curr=he;
		n->head.next=he;

		add_histent(obj, time,n,ch,regadd, vector);
		}
		else
		{
		if(regadd) { time*=(obj->time_scale); }

		if(
		  (n->curr->v.vector&&vector&&(strcmp(n->curr->v.vector,vector)))
			||(time==obj->start_time)
			||(!n->curr->v.vector)
			) /* same region == go skip */
			{
			if((n->curr->time==time)&&(obj->count_glitches))
				{
				DEBUG(printf("Warning: Glitch at time ["TTFormat"] Signal [%p], Value [%c->%c].\n",
					time, n, "0XZ1"[n->curr->v.val], ch));
				if(n->curr->v.vector) free_2(n->curr->v.vector);
				n->curr->v.vector=vector;		/* we have a glitch! */

				obj->num_glitches++;
				if(!(n->curr->flags&HIST_GLITCH))
					{
					n->curr->flags|=HIST_GLITCH;	/* set the glitch flag */
					obj->num_glitch_regions++;
					}
				}
				else
				{
				he=histent_calloc(obj);
				he->time=time;
				he->v.vector=vector;

				n->curr->next=he;
				n->curr=he;
				obj->regions+=regadd;
				}
	                }
			else
			{
			free_2(vector);
			}
	       }
	break;
	}
	}
}

}


static void add_tail_histents(struct globals *obj)
{
int i,j;

/* dump out any pending events 1st */
struct queuedevent *q;
q=queuedevents;
while(q)
	{
	struct vcdsymbol *v;

	v=q->sym;
	if(obj->current_time!=(v->ev->last_event_time+1))
		{
		/* dump degating event */
		DEBUG(fprintf(stderr,"#"TTFormat" %s = '%c' (event)\n",v->ev->last_event_time+1,v->name,'0'));
		add_histent(obj, v->ev->last_event_time+1,v->narray[0],'0',1, NULL);
		}
	q=q->next;
	}

/* then do 'x' trailers */

for(i=0;i<obj->numsyms;i++)
	{
	struct vcdsymbol *v;
	v=obj->sorted[i];
	if(v->vartype==V_REAL)
		{
		double *d;

		d=malloc_2(sizeof(double));
		*d=1.0;
		add_histent(obj, MAX_HISTENT_TIME-1, v->narray[0], 'g', 0, (char *)d);
		}
	else
	if((v->size==1)||(!obj->atomic_vectors))
	for(j=0;j<v->size;j++)
		{
		add_histent(obj, MAX_HISTENT_TIME-1, v->narray[j], 'x', 0, NULL);
		}
	else
		{
		add_histent(obj, MAX_HISTENT_TIME-1, v->narray[0], 'x', 0, (char *)calloc_2(1,sizeof(char)));
		}
	}


for(i=0;i<obj->numsyms;i++)
	{
	struct vcdsymbol *v;
	v=obj->sorted[i];
	if(v->vartype==V_REAL)
		{
		double *d;

		d=malloc_2(sizeof(double));
		*d=0.0;
		add_histent(obj, MAX_HISTENT_TIME, v->narray[0], 'g', 0, (char *)d);
		}
	else
	if((v->size==1)||(!obj->atomic_vectors))
	for(j=0;j<v->size;j++)
		{
		add_histent(obj, MAX_HISTENT_TIME, v->narray[j], 'z', 0, NULL);
		}
	else
		{
		add_histent(obj, MAX_HISTENT_TIME, v->narray[0], 'z', 0, (char *)calloc_2(1,sizeof(char)));
		}
	}
}

/*******************************************************************************/

static void vcd_build_symbols(struct globals *obj)
{
int i,j;
int max_slen=-1;
struct sym_chain *sym_chain=NULL, *sym_curr=NULL;

for(i=0;i<obj->numsyms;i++)
	{
	struct vcdsymbol *v, *vprime;
	int msi;
	int delta;

		{
		char *str;
		int slen;
		int substnode;

		v=obj->sorted[i];
		msi=v->msi;
		delta=((v->lsi-v->msi)<0)?-1:1;
		substnode=0;

		slen=strlen(v->name);
		str=(slen>max_slen)?(wave_alloca((max_slen=slen)+32)):(str); /* more than enough */
		strcpy(str,v->name);

		if(v->msi>=0)
			{
			strcpy(str+slen,obj->vcd_hier_delimeter);
			slen++;
			}

		if((vprime=bsearch_vcd(obj, v->id))!=v) /* hash mish means dup net */
			{
			if(v->size!=vprime->size)
				{
				fprintf(stderr,"ERROR: Duplicate IDs with differing width: %s %s\n", v->name, vprime->name);
				}
				else
				{
				substnode=1;
				}
			}

		if(((v->size==1)||(!obj->atomic_vectors))&&(v->vartype!=V_REAL))
			{
			struct symbol *s;

			for(j=0;j<v->size;j++)
				{
				if(v->msi>=0)
					{
					if(!obj->vcd_explicit_zero_subscripts)
						sprintf(str+slen,"%d",msi);
						else
						sprintf(str+slen-1,"[%d]",msi);
					}
				if(!symfind(obj, str))
					{
					s=symadd(obj, str,hash(str));

					s->n=v->narray[j];
					if(substnode)
						{
						struct Node *n, *n2;

						n=s->n;
						n2=vprime->narray[j];
						/* nname stays same */
						n->head=n2->head;
						n->curr=n2->curr;
						/* harray calculated later */
						n->numhist=n2->numhist;
						}

					s->n->nname=s->name;
					s->h=s->n->curr;
					if(!obj->firstnode)
						{
						obj->firstnode=obj->curnode=s;
						}
						else
						{
						obj->curnode->nextinaet=s;
						obj->curnode=s;
						}

					obj->numfacs++;
					DEBUG(fprintf(stderr,"Added: %s\n",str));
					}
					else
					{
					fprintf(stderr,"Warning: %s is a duplicate net name.\n",str);
					}
				msi+=delta;
				}

			if((j==1)&&(v->root))
				{
				s->vec_root=(struct symbol *)v->root;		/* these will get patched over */
				s->vec_chain=(struct symbol *)v->chain;		/* these will get patched over */
				v->sym_chain=s;

				if(!sym_chain)
					{
					sym_curr=(struct sym_chain *)calloc_2(1,sizeof(struct sym_chain));
					sym_chain=sym_curr;
					}
					else
					{
					sym_curr->next=(struct sym_chain *)calloc_2(1,sizeof(struct sym_chain));
					sym_curr=sym_curr->next;
					}
				sym_curr->val=s;
				}
			}
			else	/* atomic vector */
			{
			if(v->vartype!=V_REAL)
				{
				sprintf(str+slen-1,"[%d:%d]",v->msi,v->lsi);
				}
				else
				{
				*(str+slen-1)=0;
				}
			if(!symfind(obj, str))
				{
				struct symbol *s;

				s=symadd(obj, str,hash(str));

				s->n=v->narray[0];
				if(substnode)
					{
					struct Node *n, *n2;

					n=s->n;
					n2=vprime->narray[0];
					/* nname stays same */
					n->head=n2->head;
					n->curr=n2->curr;
					/* harray calculated later */
					n->numhist=n2->numhist;
					n->ext=n2->ext;
					}
					else
					{
					struct ExtNode *en;
					en=(struct ExtNode *)malloc_2(sizeof(struct ExtNode));
					en->msi=v->msi;
					en->lsi=v->lsi;

					s->n->ext=en;
					}

				s->n->nname=s->name;
				s->h=s->n->curr;
				if(!obj->firstnode)
					{
					obj->firstnode=obj->curnode=s;
					}
					else
					{
					obj->curnode->nextinaet=s;
					obj->curnode=s;
					}

				obj->numfacs++;
				DEBUG(fprintf(stderr,"Added: %s\n",str));
				}
				else
				{
				fprintf(stderr,"Warning: %s is a duplicate net name.\n",str);
				}
			}
		}
	}

if(sym_chain)
	{
	sym_curr=sym_chain;
	while(sym_curr)
		{
		sym_curr->val->vec_root= ((struct vcdsymbol *)sym_curr->val->vec_root)->sym_chain;

		if ((struct vcdsymbol *)sym_curr->val->vec_chain)
			sym_curr->val->vec_chain=((struct vcdsymbol *)sym_curr->val->vec_chain)->sym_chain;

		DEBUG(printf("Link: ('%s') '%s' -> '%s'\n",sym_curr->val->vec_root->name, sym_curr->val->name, sym_curr->val->vec_chain?sym_curr->val->vec_chain->name:"(END)"));

		sym_chain=sym_curr;
		sym_curr=sym_curr->next;
		free_2(sym_chain);
		}
	}
}

/*******************************************************************************/

void vcd_sortfacs(struct globals *obj)
{
int i;

obj->facs=(struct symbol **)malloc_2(obj->numfacs*sizeof(struct symbol *));
obj->curnode=obj->firstnode;
for(i=0;i<obj->numfacs;i++)
        {
        char *subst, ch;
        int len;

        obj->facs[i]=obj->curnode;
        if((len=strlen(subst=obj->curnode->name))>obj->longestname) obj->longestname=len;
        obj->curnode=obj->curnode->nextinaet;
        while((ch=(*subst)))
                {
                if(ch==obj->hier_delimeter) { *subst=0x01; } /* forces sort at hier boundaries */
                subst++;
                }
        }
quicksort(obj->facs,0,obj->numfacs-1);

for(i=0;i<obj->numfacs;i++)
        {
        char *subst, ch;

        subst=obj->facs[i]->name;
        while((ch=(*subst)))
                {
                if(ch==0x01) { *subst=obj->hier_delimeter; } /* restore back to normal */
                subst++;
                }

#ifdef DEBUG_FACILITIES
        printf("%-4d %s\n",i,obj->facs[i]->name);
#endif
        }

obj->facs_are_sorted=1;
}

/*******************************************************************************/

static void vcd_cleanup(struct globals *obj)
{
int i;
struct slist *s, *s2;

if(obj->sorted)
	{
	for(i=0;i<obj->numsyms;i++)
		{
		struct vcdsymbol *v;
		v=obj->sorted[i];
		if(v)
			{
			if(v->name) free_2(v->name);
			if(v->id) free_2(v->id);
			if(v->value) free_2(v->value);
			if(v->ev) free_2(v->ev);
			if(v->narray) free_2(v->narray);
			free_2(v);
			}
		}
	free_2(obj->sorted); obj->sorted=NULL;
	}

if(obj->slisthier) { free_2(obj->slisthier); obj->slisthier=NULL; }
s=obj->slistroot;
while(s)
	{
	s2=s->next;
	if(s->str)free_2(s->str);
	free_2(s);
	s=s2;
	}

obj->slistroot=obj->slistcurr=NULL; obj->slisthier_len=0;
queuedevents=NULL; /* deallocated in the symbol stuff */

if(obj->vcd_is_compressed)
	{
	pclose(obj->vcd_handle);
	}
	else
	{
	fclose(obj->vcd_handle);
	}

if(obj->yytext)
	{
	free_2(obj->yytext);
	obj->yytext=NULL;
	}
}

/*******************************************************************************/

TimeType vcd_main(struct globals *obj, char *fname)
{
int flen;

obj->pv=obj->rootv=NULL;
obj->vcd_hier_delimeter[0]=obj->hier_delimeter;

errno=0;	/* reset in case it's set for some reason */

printf("Processing '%s'\n",fname);
obj->yytext=(char *)malloc_2(obj->T_MAX_STR+1);

flen=strlen(fname);
if (((flen>2)&&(!strcmp(fname+flen-3,".gz")))||
   ((flen>3)&&(!strcmp(fname+flen-4,".zip"))))
	{
	char *str;
	int dlen;
	dlen=strlen(WAVE_DECOMPRESSOR);
	str=wave_alloca(strlen(fname)+dlen+1);
	strcpy(str,WAVE_DECOMPRESSOR);
	strcpy(str+dlen,fname);
	obj->vcd_handle=popen(str,"r");
	obj->vcd_is_compressed=~0;
	}
	else
	{
	if(strcmp("-vcd",fname))
		{
		obj->vcd_handle=fopen(fname,"rb");
		}
		else
		{
		obj->vcd_handle=stdin;
		}
	obj->vcd_is_compressed=0;
	}

if(!obj->vcd_handle)
	{
	fprintf(stderr, "Error opening %s .vcd file '%s'.\n",
		obj->vcd_is_compressed?"compressed":"", fname);
	exit(VCD_FAIL);
	}

getch_alloc(obj);		/* alloc membuff for vcd getch buffer */

build_slisthier(obj);
vcd_parse(obj);
add_tail_histents(obj);

vcd_build_symbols(obj);
vcd_sortfacs(obj);
vcd_cleanup(obj);

printf("Found %d symbols.\n", obj->numfacs);
printf("["TTFormat"] start time.\n["TTFormat"] end time.\n", obj->start_time, obj->end_time);
if(obj->num_glitches) printf("Warning: encountered %d glitch%s across %d glitch region%s.\n",
                obj->num_glitches, (obj->num_glitches!=1)?"es":"",
                obj->num_glitch_regions, (obj->num_glitch_regions!=1)?"s":"");

getch_free(obj);		/* free membuff for vcd getch buffer */

obj->min_time=obj->start_time*obj->time_scale;
obj->max_time=obj->end_time*obj->time_scale;

if(obj->min_time==obj->max_time)
        {
        fprintf(stderr, "VCD times range is equal to zero.  Exiting.\n");
        exit(VCD_FAIL);
        }

return(obj->max_time);
}

/*******************************************************************************/
