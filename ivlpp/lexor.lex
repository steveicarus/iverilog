
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
#ident "$Id: lexor.lex,v 1.19 2000/08/01 01:38:25 steve Exp $"
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
static void def_finish();
static void def_undefine();
static void do_define();
static int  is_defined(const char*name);

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

static struct include_stack_t*file_queue = 0;
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

static int comment_enter = 0;
%}

%option stack

%x PPINCLUDE
%x PPDEFINE
%x CCOMMENT
%x CSTRING

%x IFDEF_FALSE
%s IFDEF_TRUE
%x IFDEF_SUPR

W [ \t\b\f]+

%%

"//".* { ECHO; }

  /* detect multiline, c-style comments, passing them directly to the
     output. This is necessary to allow for ignoring directives that
     are included within the comments. */

"/*" { comment_enter = YY_START; BEGIN(CCOMMENT); ECHO; }
<CCOMMENT>.    { ECHO; }
<CCOMMENT>\n   { istack->lineno += 1; ECHO; }
<CCOMMENT>"*/" { BEGIN(comment_enter); ECHO; }

  /* Strings do not contain macros or preprocessor directives. */
\"            { comment_enter = YY_START; BEGIN(CSTRING); ECHO; }
<CSTRING>\\\" { yymore(); }
<CSTRING>\n   { yymore(); }
<CSTRING>\"   { BEGIN(comment_enter);  ECHO; }
<CSTRING>.    { yymore(); }

  /* This set of patterns matches the include directive and the name
     that follows it. when the directive ends, the do_include function
     performs the include operation. */

^{W}?`include { yy_push_state(PPINCLUDE); }

<PPINCLUDE>\"[^\"]*\" { include_filename(); }

<PPINCLUDE>[ \t\b\f] { ; }

  /* Catch single-line comments that share the line with an include
     directive. And while I'm at it, I might as well preserve the
     comment in the output stream. */

<PPINCLUDE>"//".* { ECHO; }

  /* These finish the include directive (EOF or EOL) so I revert the
     lexor state and execute the inclusion. */

<PPINCLUDE>\n { istack->lineno += 1; yy_pop_state(); do_include(); }
<PPINCLUDE>\r\n { istack->lineno += 1; yy_pop_state(); do_include(); }
<PPINCLUDE>\n\r { istack->lineno += 1; yy_pop_state(); do_include(); }
<PPINCLUDE><<EOF>> { istack->lineno += 1; yy_pop_state(); do_include(); }


  /* Detect the define directive, and match the name. Match any
     white space that might be found, as well. After I get the
     directive and the name, go into PPDEFINE mode and prepare to
     collect the defined value. */

`define{W}[a-zA-Z][a-zA-Z0-9_]*{W}? { yy_push_state(PPDEFINE); def_start(); }

<PPDEFINE>.* { do_define(); }

<PPDEFINE>(\n|"\r\n"|"\n\r") {
      def_finish();
      istack->lineno += 1;
      fputc('\n', yyout);
      yy_pop_state();
  }

<PPDEFINE><<EOF>> {
      def_finish();
      istack->lineno += 1;
      fputc('\n', yyout);
      yy_pop_state();
  }

`undef{W}[a-zA-Z][a-zA-Z0-9_]*{W}?.* { def_undefine(); }


  /* Detect conditional compilation directives, and parse them. If I
     find the name defined, switch to the IFDEF_TRUE state and stay
     there until I get an `else or `endif. Otherwise, switch to the
     IFDEF_FALSE state and start tossing data.

     Handle suppressed `ifdef with an additional suppress start
     condition that stacks on top of the IFDEF_FALSE so that output is
     not accidentally turned on within nested ifdefs. */

^{W}?`ifdef{W}[a-zA-Z][a-zA-Z0-9_]*.* {
      char*name = strchr(yytext, '`');
      assert(name);
      name += 6;
      name += strspn(name, " \t\b\f");
      name[strcspn(name, " \t\b\f")] = 0;

      if (is_defined(name)) {
	    yy_push_state(IFDEF_TRUE);
      } else {
	    yy_push_state(IFDEF_FALSE);
      }
  }

<IFDEF_FALSE,IFDEF_SUPR>^{W}?`ifdef{W}.* { yy_push_state(IFDEF_SUPR); }

<IFDEF_TRUE>{W}?`else.*  { BEGIN(IFDEF_FALSE); }
<IFDEF_FALSE>{W}?`else.* { BEGIN(IFDEF_TRUE); }
<IFDEF_SUPR>{W}?`else.*  {  }

<IFDEF_FALSE,IFDEF_SUPR>.  {  }
<IFDEF_FALSE,IFDEF_SUPR>\n { istack->lineno += 1; fputc('\n', yyout); }

<IFDEF_FALSE,IFDEF_TRUE,IFDEF_SUPR>^{W}?`endif.* {
      yy_pop_state();
  }

  /* This pattern notices macros and arranges for them to be replaced. */
`[a-zA-Z][a-zA-Z0-9_]* { def_match(); }

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

      struct define_t*left, *right, *up;
};

static struct define_t*def_table = 0;

static struct define_t*def_lookup(const char*name)
{
      struct define_t*cur = def_table;
      if (cur == 0) return 0;
      assert(cur->up == 0);

      while (cur) {
	    int cmp = strcmp(name, cur->name);
	    if (cmp == 0) return cur;
	    if (cmp < 0)
		  cur = cur->left;
	    else
		  cur = cur->right;
      }

      return 0;
}

static int is_defined(const char*name)
{
      return def_lookup(name) != 0;
}

  /* When a macro use is discovered in the source, this function is
     used to look up the name and emit the substitution in its
     place. If the name is not found, then the `name string is written
     out instead. */

static void def_match()
{
      struct define_t*cur = def_lookup(yytext+1);

      if (cur) {
	    struct include_stack_t*isp
		  = calloc(1, sizeof(struct include_stack_t));
	    isp->str = cur->value;
	    isp->next = istack;
	    istack->yybs = YY_CURRENT_BUFFER;
	    istack = isp;
	    yy_switch_to_buffer(yy_new_buffer(istack->file, YY_BUF_SIZE));

      } else {
	    fprintf(yyout, "%s", yytext);
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
      def->up = 0;
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
			      def->up = cur;
			      break;
			} else {
			      cur = cur->left;
			}

		  } else {
			if (cur->right == 0) {
			      cur->right = def;
			      def->up = cur;
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
	/* FIXME: This strips trailing line comments out of the
	   definition. It's not adequate as the "//" may have been
	   quoted or commented, but it'll do for now. */
      char *cp;
      if(cp = strstr(yytext, "//"))
	    *cp = 0;

	/* Trim trailing white space. */
      cp = yytext + strlen(yytext);
      while (cp > yytext) {
	    cp -= 1;
	    if (!isspace(*cp))
		  break;

	    *cp = 0;
      }

      define_macro(def_name, yytext);
      def_name[0] = 0;
}

static void def_finish()
{
      if (def_name[0])
	    define_macro(def_name, "1");
}

static void def_undefine()
{
      struct define_t*cur, *tail;

      sscanf(yytext, "`undef %s", def_name);

      cur = def_lookup(def_name);
      if (cur == 0) return;

      if (cur->up == 0) {
	    if ((cur->left == 0) && (cur->right == 0)) {
		  def_table = 0;

	    } else if (cur->left == 0) {
		  def_table = cur->right;
		  if (cur->right)
			cur->right->up = 0;

	    } else if (cur->right == 0) {
		  assert(cur->left);
		  def_table = cur->left;
		  def_table->up = 0;

	    } else {
		  tail = cur->left;
		  while (tail->right)
			tail = tail->right;

		  tail->right = cur->right;
		  tail->right->up = tail;

		  def_table = cur->left;
		  def_table->up = 0;
	    }

      } else if (cur->left == 0) {

	    if (cur->up->left == cur) {
		  cur->up->left = cur->right;

	    } else {
		  assert(cur->up->right == cur);
		  cur->up->right = cur->right;
	    }
	    if (cur->right)
		  cur->right->up = cur->up;

      } else if (cur->right == 0) {

	    assert(cur->left);

	    if (cur->up->left == cur) {
		  cur->up->left = cur->left;

	    } else {
		  assert(cur->up->right == cur);
		  cur->up->right = cur->left;
	    }
	    cur->left->up = cur->up;

      } else {
	    tail = cur->left;
	    assert(cur->left && cur->right);
	    while (tail->right)
		  tail = tail->right;

	    tail->right = cur->right;
	    tail->right->up = tail;

	    if (cur->up->left == cur) {
		  cur->up->left = cur->left;

	    } else {
		  assert(cur->up->right == cur);
		  cur->up->right = cur->left;
	    }
	    cur->left->up = cur->up;
      }

      free(cur->name);
      free(cur->value);
      free(cur);
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
	    fprintf(yyout, "\n#line \"%s\" %u\n",
		    istack->path, istack->lineno);
}

/*
 * The lexical analyzer calls this function when the current file
 * ends. Here I pop the include stack and resume the previous file. If
 * there is no previous file, then the main input is ended.
 */
static int yywrap()
{
      int line_mask_flag = 0;
      struct include_stack_t*isp = istack;
      istack = isp->next;

	/* Delete the current input buffers, and free the cell. */
      yy_delete_buffer(YY_CURRENT_BUFFER);
      if (isp->file) {
	    fclose(isp->file);
	    free(isp->path);
      } else {
	      /* If I am printing line directives and I just finished
		 macro substitution, I should terminate the line and
		 arrange for a new directive to be printed. */
	    if (line_direct_flag
		&& istack && istack->path
		&& strchr(isp->str, '\n'))
		  fprintf(yyout, "\n");
	    else
		  line_mask_flag = 1;
      }
      free(isp);

	/* If I am out if include stack, the main input is
	   done. Look for another file to process in the input
	   queue. If none are there, give up. Otherwise, open the file
	   and continue parsing. */
      if (istack == 0) {
	    if (file_queue == 0)
		  return 1;

	    istack = file_queue;
	    file_queue = file_queue->next;
	    istack->next = 0;
	    istack->lineno = 0;

	    istack->file = fopen(istack->path, "r");
	    if (istack->file == 0) {
		  perror(istack->path);
		  exit(1);
	    }

	    if (line_direct_flag)
		  fprintf(yyout, "\n#line \"%s\" 0\n", istack->path);

	    yyrestart(istack->file);
	    return 0;
      }


	/* Otherwise, resume the input buffer that is the new stack
	   top. If I need to print a line directive, do so. */

      yy_switch_to_buffer(istack->yybs);

      if (line_direct_flag && istack->path && !line_mask_flag)
	    fprintf(yyout, "\n#line \"%s\" %u\n",
		    istack->path, istack->lineno);

      return 0;
}

/*
 * This function initializes the whole process. The first file is
 * opened, and the lexor is initialized. The include stack is cleared
 * and ready to go.
 */
void reset_lexor(FILE*out, char*paths[])
{
      unsigned idx;
      struct include_stack_t*tail = 0;
      struct include_stack_t*isp = malloc(sizeof(struct include_stack_t));
      isp->path = strdup(paths[0]);
      isp->file = fopen(paths[0], "r");
      isp->str  = 0;
      if (isp->file == 0) {
	    perror(paths[0]);
	    exit(1);
      }

      yyout = out;

      yyrestart(isp->file);

      assert(istack == 0);
      istack = isp;
      isp->next = 0;

	/* Now build up a queue of all the remaining file names, so
	   that yywrap can pull them when needed. */
      file_queue = 0;
      for (idx = 1 ;  paths[idx] ;  idx += 1) {
	    isp = malloc(sizeof(struct include_stack_t));
	    isp->path = strdup(paths[idx]);
	    isp->str = 0;
	    isp->next = 0;
	    if (tail)
		  tail->next = isp;
	    else
		  file_queue = isp;

	    tail = isp;
      }
}
