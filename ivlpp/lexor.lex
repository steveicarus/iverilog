
%{
/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: lexor.lex,v 1.3 1999/07/07 01:07:57 steve Exp $"
#endif

# include  <stdio.h>
# include  <malloc.h>
# include  <string.h>
# include  <assert.h>

# include  "parse.h"
# include  "globals.h"

static void output_init();
#define YY_USER_INIT output_init()

static void def_match();
static void def_start();
static void do_define();

static void include_filename();
static void do_include();
static int yywrap();


struct include_stack_t {
      char* path;

        /* If the current input is the the file, this member is set. */
      FILE*file;

        /* If we are reparsing a macro expansion, file is 0 and this
	   member points to the string is progress */
      const char*str;

      unsigned lineno;
      YY_BUFFER_STATE yybs;

      struct include_stack_t*next;
};

static struct include_stack_t*istack  = 0;
static struct include_stack_t*standby = 0;

#define YY_INPUT(buf,result,max_size) do { \
    if (istack->file) { \
        size_t rc = fread(buf, 1, max_size, istack->file); \
        if (rc == 0) result = YY_NULL; \
        else result = rc; \
    } else { \
        if (*istack->str == 0) result = YY_NULL; \
        else { buf[0] = *istack->str++; result = 1; } \
    } \
} while(0)

%}

%x PPINCLUDE
%x PPDEFINE

%%

  /* This set of patterns matches the include directive and the name
     that follows it. when the directive ends, the do_include function
     performs the include operation. */

^`include { BEGIN(PPINCLUDE); }

<PPINCLUDE>\"[^\"]*\" { include_filename(); }

<PPINCLUDE>[ \t\b\f\r] { ; }

<PPINCLUDE>\n { istack->lineno += 1; BEGIN(0); do_include(); }


  /* Detect the define directive, and match the name. Match any
     white space that might be found, as well. After I get the
     directive and the name, go into PPDEFINE mode and prepare to
     collect the defined value. */

^`define[ \t]+[a-zA-Z][a-zA-Z0-9]*[ \t]+ { BEGIN(PPDEFINE); def_start(); }

<PPDEFINE>.* { do_define(); }

<PPDEFINE>\n { istack->lineno += 1; BEGIN(0); ECHO; }

  /* This pattern notices macros and arranges for it to be replaced. */
`[a-zA-Z][a-zA-Z0-9]* { def_match(); }

  /* Any text that is not a directive just gets passed through to the
     output. Very easy. */

. { ECHO; }
\n { istack->lineno += 1; ECHO; }

%%
  /* Defined macros are kept in this table for convenient lookup. As
     `define directives are matched (and the do_define() function
     called) the tree is built up to match names with values. If a
     define redefines an existing name, the new value it taken. */
struct define_t {
      char*name;
      char*value;

      struct define_t*left, *right;
};

static struct define_t*def_table = 0;

  /* When a macro use is discovered in the source, this function is
     used to look up the name and emit the substitution in its
     place. If the name is not found, then the `name string is written
     out instead. */

static void def_match()
{
      struct define_t*cur = def_table;
      while (cur) {
	    int cmp = strcmp(yytext+1, cur->name);
	    if (cmp == 0) break;
	    if (cmp < 0)
		  cur = cur->left;
	    else
		  cur = cur->right;
      }

      if (cur) {
	    struct include_stack_t*isp
		  = calloc(1, sizeof(struct include_stack_t));
	    isp->str = cur->value;
	    isp->next = istack;
	    istack = isp;
	    yy_switch_to_buffer(yy_new_buffer(istack->file, YY_BUF_SIZE));

      } else {
      }
}

static char def_name[256];

static void def_start()
{
      sscanf(yytext, "`define %s", def_name);
}

void define_macro(const char*name, const char*value)
{
      struct define_t*def = malloc(sizeof(struct define_t));
      def->name = strdup(name);
      def->value = strdup(value);
      def->left = 0;
      def->right = 0;
      if (def_table == 0) {
	    def_table = def;

      } else {
	    struct define_t*cur = def_table;
	    for (;;) {
		  int cmp = strcmp(def->name, cur->name);
		  if (cmp == 0) {
			free(cur->value);
			cur->value = def->value;
			free(def->name);
			free(def);
			break;

		  } else if (cmp < 0) {
			if (cur->left == 0) {
			      cur->left = def;
			      break;
			} else {
			      cur = cur->left;
			}

		  } else {
			if (cur->right == 0) {
			      cur->right = def;
			      break;
			} else {
			      cur = cur->right;
			}
		  }
	    }

      }
}

static void do_define()
{
      define_macro(def_name, yytext);
}

/*
 * Include file handling works by keeping an include stack of the
 * files that are opened and being processed. The first item on the
 * stack is the current file being scanned. If I get to an include
 * statement,
 *    open the new file,
 *    save the current buffer context,
 *    create a new buffer context,
 *    and push the new file information.
 *
 * When the file runs out, the yywrap closes the file and deletes the
 * buffer. If after popping the current file information there is
 * another file on the stack, restore its buffer context and resume
 * parsing.
 */

static void output_init()
{
      if (line_direct_flag)
	    fprintf(yyout, "#line \"%s\" 0\n", istack->path);
}

static void include_filename()
{
      assert(standby == 0);
      standby = malloc(sizeof(struct include_stack_t));
      standby->path = strdup(yytext+1);
      standby->path[strlen(standby->path)-1] = 0;
      standby->lineno = 0;
}

static void do_include()
{

      if (standby->path[0] == '/') {
	    standby->file = fopen(standby->path, "r");

      } else {
	    unsigned idx = 0;
	    standby->file = 0;
	    for (idx = 0 ;  idx < include_cnt ;  idx += 1) {
		  char path[4096];
		  sprintf(path, "%s/%s", include_dir[idx], standby->path);
		  standby->file = fopen(path, "r");
		  if (standby->file)
			break;

	    }
      }

      if (standby->file == 0) {
	    perror(standby->path);
	    exit(1);
      }

      assert(standby->file);
      standby->next = istack;
      istack->yybs = YY_CURRENT_BUFFER;
      istack = standby;
      standby = 0;
      yy_switch_to_buffer(yy_new_buffer(istack->file, YY_BUF_SIZE));

      if (line_direct_flag && istack->path)
	    fprintf(yyout, "#line \"%s\" %u\n", istack->path, istack->lineno);
}

static int yywrap()
{
      struct include_stack_t*isp = istack;
      istack = isp->next;

      yy_delete_buffer(YY_CURRENT_BUFFER);
      if (isp->file) {
	    fclose(isp->file);
	    free(isp->path);
      } else {
	      /* If I am printing line directives and I just finished
		 macro substitution, I should terminate the line and
		 arrange for a new directive to be printed. */
	    if (line_direct_flag && istack->path)
		  fprintf(yyout, "\n");
      }
      free(isp);

      if (istack == 0)
	    return 1;

      yy_switch_to_buffer(istack->yybs);

      if (line_direct_flag && istack->path)
	    fprintf(yyout, "#line \"%s\" %u\n", istack->path, istack->lineno);
      return 0;
}

/*
 * This function initializes the whole process. The first file is
 * opened, and the lexor is initialized. The include stack is cleared
 * and ready to go.
 */
void reset_lexor(FILE*out, const char*path)
{
      struct include_stack_t*isp = malloc(sizeof(struct include_stack_t));
      isp->path = strdup(path);
      isp->file = fopen(path, "r");
      isp->str  = 0;
      if (isp->file == 0) {
	    perror(path);
	    exit(1);
      }

      yyout = out;

      yyrestart(isp->file);

      assert(istack == 0);
      istack = isp;
      isp->next = 0;
}
