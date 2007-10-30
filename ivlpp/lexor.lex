
%{
/*
 * Copyright (c) 1999-2002 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: lexor.lex,v 1.46.2.1 2007/05/25 18:35:45 steve Exp $"
#endif

# include "config.h"

# include  <stdio.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>
# include  <ctype.h>
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
static int  def_is_done();
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

static void emit_pathline(struct include_stack_t *isp);

static struct include_stack_t*file_queue = 0;
static struct include_stack_t*istack  = 0;
static struct include_stack_t*standby = 0;

/*
 * Keep a stack of active ifdef, so that I can report errors
 * when there are missing endifs.
 */
struct ifdef_stack_t {
      char*path;
      unsigned lineno;

      struct ifdef_stack_t*next;
};

static struct ifdef_stack_t *ifdef_stack = 0;

static void ifdef_enter(void)
{
      struct ifdef_stack_t*cur;

      cur = (struct ifdef_stack_t*)
	    calloc(1, sizeof(struct ifdef_stack_t));
      cur->path   = strdup(istack->path);
      cur->lineno = istack->lineno;
      cur->next = ifdef_stack;
      ifdef_stack = cur;
}

static void ifdef_leave(void)
{
      struct ifdef_stack_t*cur;

      assert(ifdef_stack);

      cur = ifdef_stack;
      ifdef_stack = cur->next;

      if (strcmp(istack->path,cur->path) != 0) {
	    fprintf(stderr, "%s:%u: warning: "
		    "This `endif matches an ifdef in another file.\n",
		    istack->path, istack->lineno);

	    fprintf(stderr, "%s:%u:        : "
		    "This is the odd matched `ifdef.\n",
		    cur->path, cur->lineno);
      }

      free(cur->path);
      free(cur);

}

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
%x IFCCOMMENT
%x PCOMENT
%x CSTRING
%x ERROR_LINE

%x IFDEF_FALSE
%s IFDEF_TRUE
%x IFDEF_SUPR

W [ \t\b\f]+

%%

"//"[^\r\n]* { ECHO; }

  /* detect multiline, c-style comments, passing them directly to the
     output. This is necessary to allow for ignoring directives that
     are included within the comments. */

"/*" { comment_enter = YY_START; BEGIN(CCOMMENT); ECHO; }
<CCOMMENT>[^\r\n]    { ECHO; }
<CCOMMENT>\n\r { istack->lineno += 1; fputc('\n', yyout); }
<CCOMMENT>\r\n { istack->lineno += 1; fputc('\n', yyout); }
<CCOMMENT>\n   { istack->lineno += 1; fputc('\n', yyout); }
<CCOMMENT>\r   { istack->lineno += 1; fputc('\n', yyout); }
<CCOMMENT>"*/" { BEGIN(comment_enter); ECHO; }

  /* Detect and pass multiline pragma comments. As with C-style
     comments, pragma comments are passed through, and preprocessor
     directives contained within are ignored. Contains macros are
     expanded, however. */

"(*"{W}?")" { ECHO; }
"(*" { comment_enter = YY_START; BEGIN(PCOMENT); ECHO; }
<PCOMENT>[^\r\n]    { ECHO; }
<PCOMENT>\n\r { istack->lineno += 1; fputc('\n', yyout); }
<PCOMENT>\r\n { istack->lineno += 1; fputc('\n', yyout); }
<PCOMENT>\n   { istack->lineno += 1; fputc('\n', yyout); }
<PCOMENT>\r   { istack->lineno += 1; fputc('\n', yyout); }
<PCOMENT>"*)" { BEGIN(comment_enter); ECHO; }
<PCOMENT>`[a-zA-Z][a-zA-Z0-9_$]* { def_match(); }

  /* Strings do not contain preprocessor directives, but can expand
     macros. If that happens, they get expanded in the context of the
     string. */
\"            { comment_enter = YY_START; BEGIN(CSTRING); ECHO; }
<CSTRING>\\\" { ECHO; }
<CSTRING>\r\n { fputc('\n', yyout); }
<CSTRING>\n\r { fputc('\n', yyout); }
<CSTRING>\n   { fputc('\n', yyout); }
<CSTRING>\r   { fputc('\n', yyout); }
<CSTRING>\"   { BEGIN(comment_enter);  ECHO; }
<CSTRING>.    { ECHO; }
<CSTRING>`[a-zA-Z][a-zA-Z0-9_$]* { def_match(); }

  /* This set of patterns matches the include directive and the name
     that follows it. when the directive ends, the do_include function
     performs the include operation. */

^{W}?`include { yy_push_state(PPINCLUDE); }

<PPINCLUDE>`[a-zA-Z][a-zA-Z0-9_]* { def_match(); }
<PPINCLUDE>\"[^\"]*\" { include_filename(); }

<PPINCLUDE>[ \t\b\f] { ; }

  /* Catch single-line comments that share the line with an include
     directive. And while I'm at it, I might as well preserve the
     comment in the output stream. */

<PPINCLUDE>"//".* { ECHO; }

  /* These finish the include directive (EOF or EOL) so I revert the
     lexor state and execute the inclusion. */

<PPINCLUDE>\r\n { istack->lineno += 1; yy_pop_state(); do_include(); }
<PPINCLUDE>\n\r { istack->lineno += 1; yy_pop_state(); do_include(); }
<PPINCLUDE>\n   { istack->lineno += 1; yy_pop_state(); do_include(); }
<PPINCLUDE>\r   { istack->lineno += 1; yy_pop_state(); do_include(); }
<PPINCLUDE><<EOF>> { istack->lineno += 1; yy_pop_state(); do_include(); }

  /* Anything that is not matched by the above is an error of some
     sort. Print and error message and absorb the rest of the line. */
<PPINCLUDE>. {
      emit_pathline(istack);
      fprintf(stderr, "error: malformed `include directive."
	      " Did you quote the file name?\n");
      error_count += 1;
      BEGIN(ERROR_LINE); }

  /* Detect the define directive, and match the name. Match any
     white space that might be found, as well. After I get the
     directive and the name, go into PPDEFINE mode and prepare to
     collect the defined value. */

`define{W}[a-zA-Z_][a-zA-Z0-9_$]*{W}? { yy_push_state(PPDEFINE); def_start(); }

<PPDEFINE>.*[^\r\n] { do_define(); }

<PPDEFINE>(\n|"\r\n"|"\n\r"|\r) {
      if (def_is_done()) {
	    def_finish();
	    istack->lineno += 1;
	    yy_pop_state();
      }
      fputc('\n', yyout);
  }

  /* If the define is terminated by an EOF, then finish the define
     whether there was a continuation or not. */
<PPDEFINE><<EOF>> {
      def_finish();
      istack->lineno += 1;
      fputc('\n', yyout);
      yy_pop_state();
  }

`undef{W}[a-zA-Z_][a-zA-Z0-9_$]*{W}?.* { def_undefine(); }


  /* Detect conditional compilation directives, and parse them. If I
     find the name defined, switch to the IFDEF_TRUE state and stay
     there until I get an `else or `endif. Otherwise, switch to the
     IFDEF_FALSE state and start tossing data.

     Handle suppressed `ifdef with an additional suppress start
     condition that stacks on top of the IFDEF_FALSE so that output is
     not accidentally turned on within nested ifdefs. */

`ifdef{W}[a-zA-Z_][a-zA-Z0-9_$]* {
      char*name = strchr(yytext, '`');
      assert(name);
      name += 6;
      name += strspn(name, " \t\b\f");

      ifdef_enter();

      if (is_defined(name)) {
	    yy_push_state(IFDEF_TRUE);
      } else {
	    yy_push_state(IFDEF_FALSE);
      }
  }

`ifndef{W}[a-zA-Z_][a-zA-Z0-9_$]* {
      char*name = strchr(yytext, '`');
      assert(name);
      name += 7;
      name += strspn(name, " \t\b\f");

      ifdef_enter();

      if (!is_defined(name)) {
	    yy_push_state(IFDEF_TRUE);
      } else {
	    yy_push_state(IFDEF_FALSE);
      }
  }

<IFDEF_FALSE,IFDEF_SUPR>`ifdef{W} {
      ifdef_enter();
      yy_push_state(IFDEF_SUPR);
  }
<IFDEF_FALSE,IFDEF_SUPR>`ifndef{W} {
      ifdef_enter();
      yy_push_state(IFDEF_SUPR);
  }

<IFDEF_TRUE>`else  { BEGIN(IFDEF_FALSE); }
<IFDEF_FALSE>`else { BEGIN(IFDEF_TRUE); }
<IFDEF_SUPR>`else  {  }

<IFDEF_FALSE,IFDEF_SUPR>"//"[^\r\n]* {  }

<IFDEF_FALSE,IFDEF_SUPR>"/*" { comment_enter = YY_START; BEGIN(IFCCOMMENT); }
<IFCCOMMENT>[^\r\n]    {  }
<IFCCOMMENT>\n\r { istack->lineno += 1; }
<IFCCOMMENT>\r\n { istack->lineno += 1; }
<IFCCOMMENT>\n   { istack->lineno += 1; }
<IFCCOMMENT>\r   { istack->lineno += 1; }
<IFCCOMMENT>"*/" { BEGIN(comment_enter); }

<IFDEF_FALSE,IFDEF_SUPR>[^\r\n]  {  }
<IFDEF_FALSE,IFDEF_SUPR>\n\r { istack->lineno += 1; fputc('\n', yyout); }
<IFDEF_FALSE,IFDEF_SUPR>\r\n { istack->lineno += 1; fputc('\n', yyout); }
<IFDEF_FALSE,IFDEF_SUPR>\n   { istack->lineno += 1; fputc('\n', yyout); }
<IFDEF_FALSE,IFDEF_SUPR>\r   { istack->lineno += 1; fputc('\n', yyout); }

<IFDEF_FALSE,IFDEF_TRUE,IFDEF_SUPR>`endif { ifdef_leave(); yy_pop_state(); }

  /* This pattern notices macros and arranges for them to be replaced. */
`[a-zA-Z_][a-zA-Z0-9_$]* { def_match(); }

  /* Any text that is not a directive just gets passed through to the
     output. Very easy. */

[^\r\n] { ECHO; }
\n\r { istack->lineno += 1; fputc('\n', yyout); }
\r\n { istack->lineno += 1; fputc('\n', yyout); }
\n   { istack->lineno += 1; fputc('\n', yyout); }
\r   { istack->lineno += 1; fputc('\n', yyout); }

  /* Absorb the rest of the line when a broken directive is detected. */
<ERROR_LINE>[^\r\n]* { yy_pop_state(); }

%%
  /* Defined macros are kept in this table for convenient lookup. As
     `define directives are matched (and the do_define() function
     called) the tree is built up to match names with values. If a
     define redefines an existing name, the new value it taken. */
struct define_t {
      char*name;
      char*value;
	/* keywords don't get rescanned for fresh values. */
      int keyword;

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
	    struct include_stack_t*isp;

	    if (cur->keyword) {
		  fprintf(yyout, "%s", cur->value);
		  return;
	    }

	    isp = (struct include_stack_t*)
		  calloc(1, sizeof(struct include_stack_t));
	    isp->str = cur->value;
	    isp->next = istack;
	    istack->yybs = YY_CURRENT_BUFFER;
	    istack = isp;
	    yy_switch_to_buffer(yy_create_buffer(istack->file, YY_BUF_SIZE));

      } else {
	    emit_pathline(istack);
	    fprintf(stderr, "warning: macro %s undefined "
		    "(and assumed null) at this point.\n", yytext);
      }
}

static char def_name[256];

static void def_start()
{
      sscanf(yytext, "`define %s", def_name);
}

void define_macro(const char*name, const char*value, int keyword)
{
      struct define_t*def = malloc(sizeof(struct define_t));
      def->name = strdup(name);
      def->value = strdup(value);
      def->keyword = keyword;
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

/*
 * The do_define function accumulates the defined value in these
 * variables. When the define is over, the def_finish() function
 * executes the define and clears this text. The define_continue_flag
 * is set if do_define detects that the definition is to be continued
 * on the next line.
 */
static char* define_text = 0;
static size_t define_cnt = 0;

static int define_continue_flag = 0;

/*
 * Collect the definition. Normally, this returns 0. If there is a
 * continuation, then return 1 and this function may be called again
 * to collect another line of the definition.
 */
static void do_define()
{
      char *cp;

      define_continue_flag = 0;

	/* Look for comments in the definition, and remove them. The
	   "//" style comments go to the end of the line and terminate
	   the definition, but the multi-line comments are simply cut
	   out, and the define continues. */
      cp = strchr(yytext, '/');
      while (cp && *cp) {
	    if (cp[1] == '/') {
		  *cp = 0;
		  break;
	    }

	    if (cp[1] == '*') {
		  char*tail = strstr(cp+2, "*/");
		  if (tail == 0) {
			fprintf(stderr, "%s:%u: Unterminated comment "
				"in define\n", istack->path, istack->lineno);
			*cp = 0;
			break;
		  }

		  memmove(cp, tail+2, strlen(tail+2)+1);
		  continue;
	    }

	    cp = strchr(cp+1, '/');
      }

	/* Trim trailing white space. */
      cp = yytext + strlen(yytext);
      while (cp > yytext) {
	    if (!isspace(cp[-1]))
		  break;

	    cp -= 1;
	    *cp = 0;
      }

	/* Detect the continuation sequence. If I find it, remove it
	   and the white space that preceeds it, then replace all that
	   with a single newline. */
      if ((cp > yytext) && (cp[-1] == '\\')) {

	    cp -= 1;
	    cp[0] = 0;
	    while ((cp > yytext) && isspace(cp[-1])) {
		  cp -= 1;
		  *cp = 0;
	    }

	    *cp++ = '\n';
	    *cp = 0;
	    define_continue_flag = 1;
      }

	/* Accumulate this text into the define_text string. */
      define_text = realloc(define_text, define_cnt + (cp-yytext) + 1);
      strcpy(define_text+define_cnt, yytext);
      define_cnt += cp-yytext;
}

/*
 * Return true if the definition text is done. This is the opposite of
 * the define_continue_flag.
 */
static int def_is_done()
{
      return define_continue_flag? 0 : 1;
}

/*
 * After some number of calls to do_define, this function is called to
 * assigned value to the parsed name. If there is no value, then
 * assign the string "" (empty string.)
 */
static void def_finish()
{
      define_continue_flag = 0;

      if (def_name[0]) {
	    if (define_text) {
		  define_macro(def_name, define_text, 0);
		  free(define_text);
		  define_text = 0;
		  define_cnt = 0;

	    } else {
		  define_macro(def_name, "", 0);
	    }
	    def_name[0] = 0;
      }
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
	    fprintf(yyout, "`line 1 \"%s\" 0\n", istack->path);
}

static void include_filename()
{
      if(standby) {
	      emit_pathline(istack);
	      fprintf(stderr, "error: malformed `include directive. Extra junk on line?\n");
              exit(1);
      }
      standby = malloc(sizeof(struct include_stack_t));
      standby->path = strdup(yytext+1);
      standby->path[strlen(standby->path)-1] = 0;
      standby->lineno = 0;
}

static void do_include()
{

      if (standby->path[0] == '/') {
	    standby->file = fopen(standby->path, "r");
	    if(depend_file && standby->file) {
		    fprintf(depend_file, "%s\n", istack->path);
	    }
      } else {
	    unsigned idx = 0;
	    standby->file = 0;
	    for (idx = 0 ;  idx < include_cnt ;  idx += 1) {
		  char path[4096];
		  sprintf(path, "%s/%s", include_dir[idx], standby->path);
		  standby->file = fopen(path, "r");
		  if (standby->file) {
			if(depend_file) {
			    fprintf(depend_file, "%s\n", path);
			}
			break;
		  }
	    }
      }

      if (standby->file == 0) {
	    fprintf(stderr, "%s:%u: Include file %s not found\n",
		    istack->path, istack->lineno, standby->path);
	    exit(1);
      }

      assert(standby->file);
      standby->next = istack;
      istack->yybs = YY_CURRENT_BUFFER;
      istack = standby;
      standby = 0;
      yy_switch_to_buffer(yy_create_buffer(istack->file, YY_BUF_SIZE));

      if (line_direct_flag && istack->path)
	    fprintf(yyout, "\n`line %u \"%s\" 1\n",
		    istack->lineno+1, istack->path);
}

/* walk the include stack until we find an entry with a valid pathname,
 * and print the file and line from that entry for use in an error message.
 * The istack entries created by def_match() for macro expansions do not
 * contain pathnames.   This finds instead the real file in which the outermost
 * macro was used.
 */
static void emit_pathline(struct include_stack_t*isp)
{
	while(isp && (isp->path == NULL)) {
		isp = isp->next;
	}
	assert(isp);
	fprintf(stderr, "%s:%u: ",
		isp->path, isp->lineno+1);
}

static void lexor_done()
{
      while (ifdef_stack) {
	    struct ifdef_stack_t*cur = ifdef_stack;
	    ifdef_stack = cur->next;

	    fprintf(stderr, "%s:%u: error: This `ifdef lacks an `endif.\n",
		    cur->path, cur->lineno);

	    free(cur->path);
	    free(cur);
	    error_count += 1;
      }
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

	/* If I am out of include stack, the main input is
	   done. Look for another file to process in the input
	   queue. If none are there, give up. Otherwise, open the file
	   and continue parsing. */
      if (istack == 0) {
	    if (file_queue == 0) {
		  lexor_done();
		  return 1;
	    }

	    istack = file_queue;
	    file_queue = file_queue->next;
	    istack->next = 0;
	    istack->lineno = 0;

	    istack->file = fopen(istack->path, "r");
	    if (istack->file == 0) {
		  perror(istack->path);
		  error_count += 1;
		  return 1;
	    }

	    if (line_direct_flag)
		  fprintf(yyout, "\n`line 1 \"%s\" 0\n", istack->path);
	    if(depend_file) {
		  fprintf(depend_file, "%s\n", istack->path);
	    }

	    yyrestart(istack->file);
	    return 0;
      }


	/* Otherwise, resume the input buffer that is the new stack
	   top. If I need to print a line directive, do so. */

      yy_switch_to_buffer(istack->yybs);

      if (line_direct_flag && istack->path && !line_mask_flag)
	    fprintf(yyout, "\n`line %u \"%s\" 2\n",
		    istack->lineno+1, istack->path);

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
      isp->lineno = 0;
      if (isp->file == 0) {
	    perror(paths[0]);
	    exit(1);
      }
      if(depend_file) {
	      fprintf(depend_file, "%s\n", paths[0]);
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
	    isp->file = 0;
	    isp->str = 0;
	    isp->next = 0;
	    isp->lineno = 0;
	    if (tail)
		  tail->next = isp;
	    else
		  file_queue = isp;

	    tail = isp;
      }
}
