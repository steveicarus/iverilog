%{
/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: parse.y,v 1.1 2000/10/08 22:36:56 steve Exp $"
#endif

# include  <stdio.h>
# include  <malloc.h>
# include  "globals.h"


void yyerror(const char*);

enum drive_code_t {
      CODE_S,
      CODE_t,
};

struct drive_cond {
      struct drive_cond*next;
      enum drive_code_t code;
      char* text;
};

static struct drive_cond *cur_cond = 0;

static void add_cond(enum drive_code_t code, char*text)
{
      struct drive_cond*tmp;
      tmp = calloc(1, sizeof(struct drive_cond));
      tmp->code = code;
      tmp->text = text;

      tmp->next = cur_cond;
      cur_cond = tmp;
}

struct drive_pattern {
      struct drive_pattern*next;
      char*name;
      char*text;
      struct drive_cond*cond;
};

static struct drive_pattern *patterns = 0;
static struct drive_pattern *plast = 0;

static void add_pattern(char*name, char*text)
{
      struct drive_pattern*tmp;
      tmp = calloc(1, sizeof(struct drive_pattern));
      tmp->name = name;
      tmp->text = text;
      tmp->cond = cur_cond;
      tmp->next = 0;


      if (plast) {
	    plast->next = tmp;
	    plast = tmp;
      } else {
	    patterns = tmp;
	    plast = tmp;
      }
}

const char*lookup_pattern(const char*key)
{
      struct drive_pattern*cur;
      struct drive_cond*cc;

      for (cur = patterns ;  cur ;  cur = cur->next) {

	    if (strcmp(key, cur->name) != 0)
		  continue;

	    for (cc = cur->cond ;  cc ;  cc = cc->next) {

		  switch (cc->code) {
		      case CODE_t:
			if (strcmp(targ, cc->text) == 0)
			      continue;
			break;
		  }

		  break;
	    }

	    if (cc) continue;

	    return cur->text;
      }

      return 0;
}

%}

%union {
      char*text;
};

%token <text> PATTERN_NAME PATTERN_TEXT
%token <text> CT_t
%token CT_S

%%

start: section_list

section_list
	: section
	| section_list section
	;

section : '[' ctoken_list ']' pattern_list { cur_cond = 0; } ;

ctoken_list
	: ctoken
	| ctoken_list ctoken
	;

ctoken  : CT_S  { add_cond(CODE_S, 0); }
	| CT_t  { add_cond(CODE_t, $1); }
	;

pattern_list
	: pattern
	| pattern_list pattern
	;

pattern : PATTERN_NAME PATTERN_TEXT
		{ add_pattern($1, $2); }
	;

%%

void yyerror(const char*msg)
{
      fprintf(stderr, "%s\n", msg);
}
