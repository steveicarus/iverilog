%option prefix="yy"
%{
/*
 * Copyright (c) 1999-2017 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"

# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <ctype.h>
# include  <assert.h>

# include  "globals.h"
# include  "ivl_alloc.h"

static void output_init(void);
#define YY_USER_INIT output_init()

static void  def_start(void);
static void  def_add_arg(void);
static void  def_finish(void);
static void  def_undefine(void);
static void  do_define(void);
static int   def_is_done(void);
static void  def_continue(void);
static int   is_defined(const char*name);

static int   macro_needs_args(const char*name);
static void  macro_start_args(void);
static void  macro_add_to_arg(int is_whitespace);
static void  macro_finish_arg(void);
static void  do_expand(int use_args);
static const char* do_magic(const char*name);
static const char* macro_name(void);

static void include_filename(void);
static void do_include(void);

static int load_next_input(void);

struct include_stack_t
{
    char* path;

    /* If the current input is from a file, this member is set. */
    FILE* file;
    int (*file_close)(FILE*);

    /* If we are reparsing a macro expansion, file is 0 and this
     * member points to the string in progress
     */
    char* str;
    char* orig_str;

    int stringify_flag;

    unsigned lineno;
    YY_BUFFER_STATE yybs;

    struct include_stack_t* next;

    /* A single line comment can be associated with this include. */
    char* comment;
};

static unsigned get_line(struct include_stack_t* isp);
static const char *get_path(struct include_stack_t* isp);
static void emit_pathline(struct include_stack_t* isp);

/*
 * The file_queue is a singly-linked list of the files that were
 * listed on the command line/file list.
 */
static struct include_stack_t* file_queue = 0;

static int do_expand_stringify_flag = 0;

/*
 * The istack is the inclusion stack.
 */
static struct include_stack_t* istack  = 0;
static struct include_stack_t* standby = 0;

/*
 * Keep a stack of active ifdef, so that I can report errors
 * when there are missing endifs.
 */
struct ifdef_stack_t
{
    char*    path;
    unsigned lineno;

    struct ifdef_stack_t* next;
};

static struct ifdef_stack_t* ifdef_stack = 0;

static void ifdef_enter(void)
{
    struct ifdef_stack_t*cur;

    cur = (struct ifdef_stack_t*) calloc(1, sizeof(struct ifdef_stack_t));
    if (istack->path) cur->path = strdup(istack->path);
    cur->lineno = istack->lineno;
    cur->next = ifdef_stack;

    ifdef_stack = cur;
}

static void ifdef_leave(void)
{
    struct ifdef_stack_t* cur;

    assert(ifdef_stack);

    cur = ifdef_stack;
    ifdef_stack = cur->next;

    /* If either path is from a non-file context e.g.(macro expansion)
     * we assume that the non-file part is from this file. */
    if (istack->path != NULL && cur->path != NULL &&
        strcmp(istack->path,cur->path) != 0) {
        fprintf(stderr, "%s:%u: warning: This `endif matches an ifdef "
                "in another file.\n", istack->path, istack->lineno+1);

        fprintf(stderr, "%s:%u: This is the odd matched `ifdef.\n",
                cur->path, cur->lineno+1);
    }

    free(cur->path);
    free(cur);
}

#define YY_INPUT(buf,result,max_size) do {                              \
    if (istack->file) {                                                 \
        size_t rc = fread(buf, 1, max_size, istack->file);              \
        result = (rc == 0) ? YY_NULL : rc;                              \
    } else {                                                            \
        /* We are expanding a macro. Handle the SV macro escape         \
           sequences. There doesn't seem to be any good reason          \
           not to allow them in traditional Verilog as well. */         \
        while ((istack->str[0] == '`') &&                               \
               (istack->str[1] == '`')) {                               \
            istack->str += 2;                                           \
        }                                                               \
        if (*istack->str == 0) {                                        \
            result = YY_NULL;                                           \
        } else if ((istack->str[0] == '`') &&                           \
                   (istack->str[1] == '"')) {                           \
            istack->str += 2;                                           \
            buf[0] = '"';                                               \
            result = 1;                                                 \
        } else if ((istack->str[0] == '`') &&                           \
                   (istack->str[1] == '\\')&&                           \
                   (istack->str[2] == '`') &&                           \
                   (istack->str[3] == '"')) {                           \
            istack->str += 4;                                           \
            buf[0] = '\\';                                              \
            buf[1] = '"';                                               \
            result = 2;                                                 \
        } else {                                                        \
            buf[0] = *istack->str++;                                    \
            result = 1;                                                 \
        }                                                               \
    }                                                                   \
} while (0)

static int comment_enter = 0;
static int pragma_enter = 0;
static int string_enter = 0;

static int ma_parenthesis_level = 0;
%}

%option stack
%option nounput
%option noinput
%option noyy_top_state
%option noyywrap

%x PPINCLUDE
%x DEF_NAME
%x DEF_ARG
%x DEF_SEP
%x DEF_TXT
%x MA_START
%x MA_ADD
%x CCOMMENT
%x IFCCOMMENT
%x PCOMENT
%x CSTRING
%x ERROR_LINE

%x IFDEF_FALSE
%s IFDEF_TRUE
%x IFDEF_SUPR

W        [ \t\b\f]+

/* The grouping parentheses are necessary for compatibility with
 * older versions of flex (at least 2.5.31); they are supposed to
 * be implied, according to the flex manual.
 */
keywords (include|define|undef|ifdef|ifndef|else|elseif|endif)

%%

"//"[^\r\n]* { ECHO; }

 /* detect multiline, c-style comments, passing them directly to the
  * output. This is necessary to allow for ignoring directives that
  * are included within the comments.
  */

"/*"              { comment_enter = YY_START; BEGIN(CCOMMENT); ECHO; }
<CCOMMENT>[^\r\n] { ECHO; }
<CCOMMENT>\n\r    |
<CCOMMENT>\r\n    |
<CCOMMENT>\n      |
<CCOMMENT>\r      { istack->lineno += 1; fputc('\n', yyout); }
<CCOMMENT>"*/"    { BEGIN(comment_enter); ECHO; }

 /* Detect and pass multiline pragma comments. As with C-style
  * comments, pragma comments are passed through, and preprocessor
  * directives contained within are ignored. Contains macros are
  * expanded, however.
  */
"(*"{W}?")"      { ECHO; }
"(*"             { pragma_enter = YY_START; BEGIN(PCOMENT); ECHO; }
<PCOMENT>[^\r\n] { ECHO; }
<PCOMENT>\n\r    |
<PCOMENT>\r\n    |
<PCOMENT>\n      |
<PCOMENT>\r      { istack->lineno += 1; fputc('\n', yyout); }
<PCOMENT>"*)"    { BEGIN(pragma_enter); ECHO; }

<PCOMENT>`{keywords} {
    emit_pathline(istack);
    error_count += 1;
    fprintf(stderr, "error: macro names cannot be directive keywords "
            "('%s'); replaced with nothing.\n", yytext);
}

<PCOMENT>`[a-zA-Z][a-zA-Z0-9_$]* {
    if (macro_needs_args(yytext+1)) yy_push_state(MA_START); else do_expand(0);
}

 /* Strings do not contain preprocessor directives, but can expand
  * macros. If that happens, they get expanded in the context of the
  * string.
  */
\"            { string_enter = YY_START; BEGIN(CSTRING); ECHO; }
<CSTRING>\\\\ |
<CSTRING>\\\" |
<CSTRING>\\`  { ECHO; }
<CSTRING>\r\n |
<CSTRING>\n\r |
<CSTRING>\n   |
<CSTRING>\r   { fputc('\n', yyout); }
<CSTRING>\"   { BEGIN(string_enter);  ECHO; }
<CSTRING>.    { ECHO; }

 /* This set of patterns matches the include directive and the name
  * that follows it. when the directive ends, the do_include function
  * performs the include operation.
  */
^{W}?`include { yy_push_state(PPINCLUDE); }

<PPINCLUDE>`{keywords} {
    emit_pathline(istack);
    error_count += 1;
    fprintf(stderr, "error: macro names cannot be directive keywords "
            "('%s'); replaced with nothing.\n", yytext);
}

<PPINCLUDE>`[a-zA-Z][a-zA-Z0-9_]* {
    if (macro_needs_args(yytext+1)) yy_push_state(MA_START); else do_expand(0);
}

<PPINCLUDE>\"[^\"]*\" { include_filename(); }

<PPINCLUDE>[ \t\b\f] { ; }

  /* Catch single-line comments that share the line with an include
   * directive. And while I'm at it, I might as well preserve the
   * comment in the output stream. This will be printed after the
   * file has been included.
   */
<PPINCLUDE>"//"[^\r\n]* { standby->comment = strdup(yytext); }

 /* These finish the include directive (EOF or EOL) so I revert the
  * lexor state and execute the inclusion.
  */

 /* There is a bug in flex <= 2.5.34 that prevents the continued action '|'
  * from working properly when the final action is associated with <<EOF>>.
  * Therefore, the action is repeated. */

<PPINCLUDE>\r\n    |
<PPINCLUDE>\n\r    |
<PPINCLUDE>\n      |
<PPINCLUDE>\r      { istack->lineno += 1; yy_pop_state(); do_include(); }
<PPINCLUDE><<EOF>> { istack->lineno += 1; yy_pop_state(); do_include(); }

 /* Anything that is not matched by the above is an error of some
  * sort. Print an error message and absorb the rest of the line.
  */
<PPINCLUDE>. {
    emit_pathline(istack);
    fprintf(stderr, "error: malformed `include directive. Did you quote the file name?\n");
    error_count += 1;
    BEGIN(ERROR_LINE);
}

  /* Detect the define directive, and match the name. If followed by a
   * '(', collect the formal arguments. Consume any white space, then
   * go into DEF_TXT mode and collect the defined value.
   */
`define{W} { yy_push_state(DEF_NAME); }

<DEF_NAME>{keywords}{W}? {
    emit_pathline(istack);
    error_count += 1;
    BEGIN(ERROR_LINE);
    fprintf(stderr, "error: malformed `define directive: macro names "
            "cannot be directive keywords\n");
}

<DEF_NAME>[a-zA-Z_][a-zA-Z0-9_$]*"("{W}? { BEGIN(DEF_ARG); def_start(); }
<DEF_NAME>[a-zA-Z_][a-zA-Z0-9_$]*{W}?    { BEGIN(DEF_TXT); def_start(); }

  /* define arg: <name> = <text> */
<DEF_ARG>[a-zA-Z_][a-zA-Z0-9_$]*{W}*"="[^,\)]*{W}? { BEGIN(DEF_SEP); def_add_arg(); }
  /* define arg: <name> */
<DEF_ARG>[a-zA-Z_][a-zA-Z0-9_$]*{W}? { BEGIN(DEF_SEP); def_add_arg(); }

<DEF_SEP>","{W}? { BEGIN(DEF_ARG); }
<DEF_SEP>")"{W}? { BEGIN(DEF_TXT); }

<DEF_ARG,DEF_SEP>"//"[^\r\n]* { ECHO; }
<DEF_ARG,DEF_SEP>"/*"         { comment_enter = YY_START; BEGIN(CCOMMENT); ECHO; }
<DEF_ARG,DEF_SEP>{W}          {}

<DEF_ARG,DEF_SEP>(\n|"\r\n"|"\n\r"|\r){W}? { istack->lineno += 1; fputc('\n', yyout); }

<DEF_NAME,DEF_ARG,DEF_SEP>. {
    emit_pathline(istack);
    fprintf(stderr, "error: malformed `define directive.\n");
    error_count += 1;
    BEGIN(ERROR_LINE);
}

<DEF_TXT>.*[^\r\n] { do_define(); }

<DEF_TXT>(\n|"\r\n"|"\n\r"|\r) {
    if (def_is_done()) {
        def_finish();
        yy_pop_state();
    } else {
	def_continue();
    }

    istack->lineno += 1;
    fputc('\n', yyout);
}

 /* If the define is terminated by an EOF, then finish the define
  * whether there was a continuation or not.
  */
<DEF_TXT><<EOF>> {
    def_finish();

    istack->lineno += 1;
    fputc('\n', yyout);

    yy_pop_state();

    if (!load_next_input())
        yyterminate();
}

`undef{W}[a-zA-Z_][a-zA-Z0-9_$]*{W}?.* { def_undefine(); }

  /* Detect conditional compilation directives, and parse them. If I
   * find the name defined, switch to the IFDEF_TRUE state and stay
   * there until I get an `else or `endif. Otherwise, switch to the
   * IFDEF_FALSE state and start tossing data.
   *
   * Handle suppressed `ifdef with an additional suppress start
   * condition that stacks on top of the IFDEF_FALSE so that output is
   * not accidentally turned on within nested ifdefs.
   */
`ifdef{W}[a-zA-Z_][a-zA-Z0-9_$]* {
    char* name = strchr(yytext, '`'); assert(name);

    name += 6;
    name += strspn(name, " \t\b\f");

    ifdef_enter();

    if (is_defined(name))
        yy_push_state(IFDEF_TRUE);
    else
        yy_push_state(IFDEF_FALSE);
}

`ifndef{W}[a-zA-Z_][a-zA-Z0-9_$]* {
    char* name = strchr(yytext, '`'); assert(name);

    name += 7;
    name += strspn(name, " \t\b\f");

    ifdef_enter();

    if (!is_defined(name))
        yy_push_state(IFDEF_TRUE);
    else
        yy_push_state(IFDEF_FALSE);
}

<IFDEF_FALSE,IFDEF_SUPR>`ifdef{W}  |
<IFDEF_FALSE,IFDEF_SUPR>`ifndef{W} { ifdef_enter(); yy_push_state(IFDEF_SUPR); }

<IFDEF_TRUE>`elsif{W}[a-zA-Z_][a-zA-Z0-9_$]* { BEGIN(IFDEF_SUPR); }

<IFDEF_FALSE>`elsif{W}[a-zA-Z_][a-zA-Z0-9_$]* {
    char* name = strchr(yytext, '`'); assert(name);

    name += 6;
    name += strspn(name, " \t\b\f");

    if (is_defined(name))
        BEGIN(IFDEF_TRUE);
    else
        BEGIN(IFDEF_FALSE);
}

<IFDEF_SUPR>`elsif{W}[a-zA-Z_][a-zA-Z0-9_$]* {  }

<IFDEF_TRUE>`else  { BEGIN(IFDEF_SUPR); }
<IFDEF_FALSE>`else { BEGIN(IFDEF_TRUE); }
<IFDEF_SUPR>`else  {}

<IFDEF_FALSE,IFDEF_SUPR>"//"[^\r\n]* {}

<IFDEF_FALSE,IFDEF_SUPR>"/*" { comment_enter = YY_START; BEGIN(IFCCOMMENT); }

<IFCCOMMENT>[^\r\n] {}
<IFCCOMMENT>\n\r    |
<IFCCOMMENT>\r\n    |
<IFCCOMMENT>\n      |
<IFCCOMMENT>\r      { istack->lineno += 1; fputc('\n', yyout); }
<IFCCOMMENT>"*/"    { BEGIN(comment_enter); }

<IFDEF_FALSE,IFDEF_SUPR>[^\r\n] {  }
<IFDEF_FALSE,IFDEF_SUPR>\n\r    |
<IFDEF_FALSE,IFDEF_SUPR>\r\n    |
<IFDEF_FALSE,IFDEF_SUPR>\n      |
<IFDEF_FALSE,IFDEF_SUPR>\r      { istack->lineno += 1; fputc('\n', yyout); }

<IFDEF_FALSE,IFDEF_TRUE,IFDEF_SUPR>`endif { ifdef_leave(); yy_pop_state(); }

`ifdef {
    error_count += 1;
    fprintf(stderr, "%s:%u: `ifdef without a macro name - ignored.\n",
            istack->path, istack->lineno+1);
}

`ifndef {
    error_count += 1;
    fprintf(stderr, "%s:%u: `ifndef without a macro name - ignored.\n",
            istack->path, istack->lineno+1);
}

`elsif {
    error_count += 1;
    fprintf(stderr, "%s:%u: `elsif without a macro name - ignored.\n",
            istack->path, istack->lineno+1);
}

`elsif{W}[a-zA-Z_][a-zA-Z0-9_$]* {
    error_count += 1;
    fprintf(stderr, "%s:%u: `elsif without a matching `ifdef - ignored.\n",
            istack->path, istack->lineno+1);
}

`else {
    error_count += 1;
    fprintf(stderr, "%s:%u: `else without a matching `ifdef - ignored.\n",
            istack->path, istack->lineno+1);
}

`endif {
    error_count += 1;
    fprintf(stderr, "%s:%u: `endif without a matching `ifdef - ignored.\n",
            istack->path, istack->lineno+1);
}

`{keywords} {
    emit_pathline(istack);
    error_count += 1;
    fprintf(stderr, "error: macro names cannot be directive keywords "
            "('%s'); replaced with nothing.\n", yytext);
}

 /* This pattern notices macros and arranges for them to be replaced. */
`[a-zA-Z_][a-zA-Z0-9_$]* {
    if (macro_needs_args(yytext+1))
        yy_push_state(MA_START);
    else
        do_expand(0);
}

  /* Stringified version of macro expansion. This is an Icarus extension.
     When expanding macro text, the SV usage of `` takes precedence. */
``[a-zA-Z_][a-zA-Z0-9_$]* {
    assert(istack->file);
    assert(do_expand_stringify_flag == 0);
    do_expand_stringify_flag = 1;
    fputc('"', yyout);
    if (macro_needs_args(yytext+2))
        yy_push_state(MA_START);
    else
        do_expand(0);
}

<MA_START>\(  { BEGIN(MA_ADD); macro_start_args(); }

<MA_START>{W} {}

<MA_START>(\n|"\r\n"|"\n\r"|\r){W}? {
    istack->lineno += 1;
    fputc('\n', yyout);
}

<MA_START>. {
    emit_pathline(istack);

    fprintf(stderr, "error: missing argument list for `%s.\n", macro_name());
    error_count += 1;

    yy_pop_state();
    yyless(0);
}

<MA_ADD>\"[^\"\n\r]*\" { macro_add_to_arg(0); }

<MA_ADD>\"[^\"\n\r]* {
    emit_pathline(istack);

    fprintf(stderr, "error: unterminated string.\n");
    error_count += 1;

    BEGIN(ERROR_LINE);
}

<MA_ADD>'[^\n\r]' { macro_add_to_arg(0); }

<MA_ADD>{W} { macro_add_to_arg(1); }

<MA_ADD>"(" { macro_add_to_arg(0); ma_parenthesis_level++; }

<MA_ADD>"," {
    if (ma_parenthesis_level > 0)
	macro_add_to_arg(0);
    else
	macro_finish_arg();
}

<MA_ADD>")" {
    if (ma_parenthesis_level > 0) {
        macro_add_to_arg(0);
        ma_parenthesis_level--;
    } else {
        macro_finish_arg();
        yy_pop_state();
        do_expand(1);
    }
}

<MA_ADD>(\n|"\r\n"|"\n\r"|\r){W}? {
    macro_add_to_arg(1);
    istack->lineno += 1;
    fputc('\n', yyout);
}

<MA_ADD>. { macro_add_to_arg(0); }

<MA_START,MA_ADD>"//"[^\r\n]* { ECHO; }

<MA_START,MA_ADD>"/*" { comment_enter = YY_START; BEGIN(CCOMMENT); ECHO; }

 /* Any text that is not a directive just gets passed through to the
  * output. Very easy.
  */
[^\r\n] { ECHO; }
\n\r    |
\r\n    |
\n      |
\r      { istack->lineno += 1; fputc('\n', yyout); }

 /* Absorb the rest of the line when a broken directive is detected. */
<ERROR_LINE>[^\r\n]* { yy_pop_state(); }

<ERROR_LINE>(\n|"\r\n"|"\n\r"|\r) {
    yy_pop_state();
    istack->lineno += 1;
    fputc('\n', yyout);
}

<<EOF>> { fputc('\n', yyout); if (!load_next_input()) yyterminate(); }

%%
 /* Defined macros are kept in this table for convenient lookup. As
  * `define directives are matched (and the do_define() function
  * called) the tree is built up to match names with values. If a
  * define redefines an existing name, the new value it taken.
  */
struct define_t
{
    char*   name;
    char*   value;
    int     keyword; /* keywords don't get rescanned for fresh values. */
    int     argc;
    char**  defaults;
    int     magic; /* 1 for 'magic' macros like __FILE__ and __LINE__. magic
                    * macros cannot be undefined. magic macros are expanded
                    * by do_magic. N.B. DON'T set a magic macro with
                    * argc > 1 or with keyword true. */

    struct define_t*    left;
    struct define_t*    right;
    struct define_t*    up;
};

static struct define_t* def_table = 0;

/*
 * magic macros
 */
static struct define_t def_FILE;
static struct define_t def_LINE =
{
    .name       = "__LINE__",
    .value      = "__LINE__",
    .keyword    = 0,
    .argc       = 1,
    .magic      = 1,
    .left       = &def_FILE,
    .right      = 0,
    .up         = 0
};
static struct define_t def_FILE =
{
    .name       = "__FILE__",
    .value      = "__FILE__",
    .keyword    = 0,
    .argc       = 1,
    .magic      = 1,
    .left       = 0,
    .right      = 0,
    .up         = &def_LINE
};
static struct define_t* magic_table = &def_LINE;


/*
 * helper function for def_lookup
 */
static struct define_t* def_lookup_internal(const char*name, struct define_t*cur)
{
    if (cur == 0) return 0;

    assert(cur->up == 0);

    while (cur) {
        int cmp = strcmp(name, cur->name);

        if (cmp == 0) return cur;

        cur = (cmp < 0) ? cur->left : cur->right;
    }

    return 0;
}

static struct define_t* def_lookup(const char*name)
{
    // first, try a magic macro
    if(name[0] == '_' && name[1] == '_' && name[2] != '\0') {
        struct define_t* result = def_lookup_internal(name, magic_table);
        if(result) {
            return result;
        }
    }

    // either there was no matching magic macro, or we didn't try looking
    // look for a normal macro
    return def_lookup_internal(name, def_table);
}


static int is_defined(const char*name)
{
    return def_lookup(name) != 0;
}

/*
 * The following variables are used to temporarily hold the name and
 * formal arguments of a macro definition as it is being parsed. As
 * for C program arguments, def_argc counts the arguments (including
 * the macro name), the value returned by def_argv(0) points to the
 * macro name, the value returned by def_argv(1) points to the first
 * argument, and so on.
 *
 * These variables are also used for storing the actual arguments when
 * a macro is instantiated.
 */
#define MAX_DEF_ARG 256 /* allows argument IDs to be stored in a single char */

#define DEF_BUF_CHUNK 256

/*
 * During parse of macro definitions, and the name/arguments in
 * particular, keep the names and name lengths in a compact stretch of
 * memory. Note that we do not keep the argument names once the
 * definition is fully processed, because arguments are always
 * positional and the definition string is replaced with position
 * tokens.
 */
static char* def_buf = 0;
static int   def_buf_size = 0;
static int   def_buf_free = 0;

static int   def_argc = 0;
static int   def_argo[MAX_DEF_ARG];  /* offset of first character of arg name */
static int   def_argl[MAX_DEF_ARG];  /* lengths of arg names. */
static int   def_argd[MAX_DEF_ARG];  /* Offset of default value */

/*
 * Return a pointer to the start of argument 'arg'. Returned pointers
 * may go stale after a call to def_buf_grow_to_fit.
 */
static /* inline */ char* def_argv(int arg)
{
    return def_buf + def_argo[arg];
}

static void check_for_max_args(void)
{
    if (def_argc == MAX_DEF_ARG) {
        emit_pathline(istack);
        fprintf(stderr, "error: too many macro arguments - aborting\n");
        exit(1);
    }
}

static void def_buf_grow_to_fit(int length)
{
    while (length >= def_buf_free)
    {
        def_buf_size += DEF_BUF_CHUNK;
        def_buf_free += DEF_BUF_CHUNK;
        def_buf = realloc(def_buf, def_buf_size);
        assert(def_buf != 0);
    }
}

static void def_start(void)
{
    def_buf_free = def_buf_size;
    def_argc = 0;
    def_add_arg();
}

static void def_add_arg(void)
{
    int length = yyleng;

    check_for_max_args();

    /* Remove trailing white space and, if necessary, opening brace. */
    while (isspace((int)yytext[length - 1])) length--;

      /* This can happen because we are also processing "argv[0]", the
	 macro name, as a pseudo-argument. The lexor will match that
	 as name(, so chop off the ( here. */
    if (yytext[length -  1] == '(') length--;

    yytext[length] = 0;

    char*arg = yytext;
    char*val;
    int  val_length = 0;

      /* Break into ARG = value. This happens if the source specifies
	 a default value for the formal argument. In that case, the
	 lexor will match the whole thing as the argument and it is up
	 to us to chop it up to name and value. */
    if ( (val=strchr(arg,'=')) ) {
	  *val++ = 0;
	  while (*val && isspace(*val)) val += 1;

	  val_length = strlen(val);
	  while (val_length>0 && isspace(val[val_length-1])) {
		val_length -= 1;
		val[val_length] = 0;
	  }

	    /* Strip white space from between arg and "=". */
	  length = strlen(arg);
	  while (length>0 && isspace(arg[length-1])) {
		length -= 1;
		arg[length] = 0;
	  }
    }

    /* Make sure there's room in the buffer for the new argument. */
    def_buf_grow_to_fit(length);

    /* Store the new argument name. */
    def_argl[def_argc] = length;
    def_argo[def_argc] = def_buf_size - def_buf_free;
    strcpy(def_argv(def_argc), arg);
    def_buf_free -= length + 1;

      /* If there is a default text, then stash it away as well. */
    if (val) {
	  def_buf_grow_to_fit(val_length);
	  def_argd[def_argc] = def_buf_size - def_buf_free;
	  strcpy(def_buf+def_argd[def_argc], val);
	  def_buf_free -= val_length + 1;
    } else {
	  def_argd[def_argc] = 0;
    }

    def_argc += 1;
}

void define_macro(const char* name, const char* value, int keyword, int argc)
{
    int idx;
    struct define_t* def;
    struct define_t* prev;

    /* Verilog has a very nasty system of macros jumping from
     * file to file, resulting in a global macro scope. Here
     * we optionally warn about any redefinitions.
     *
     * If istack is empty, we are processing a configuration
     * or precompiled macro file, so don't want to check for
     * redefinitions - when a precompiled macro file is used,
     * it will contain copies of any predefined macros.
     */
    if (warn_redef && istack) {
	prev = def_lookup(name);
	if (prev && (warn_redef_all || (strcmp(prev->value, value) != 0))) {
	    emit_pathline(istack);
	    fprintf(stderr, "warning: redefinition of macro %s from value '%s' to '%s'\n",
	    name, prev->value, value);
	}
    }

    def = malloc(sizeof(struct define_t));
    def->name = strdup(name);
    def->value = strdup(value);
    def->keyword = keyword;
    def->argc = argc;
    def->magic = 0;
    def->left = 0;
    def->right = 0;
    def->up = 0;
    def->defaults = calloc(argc, sizeof(char*));
    for (idx = 0 ; idx < argc ; idx += 1) {
	  if (def_argd[idx] == 0) {
		def->defaults[idx] = 0;
	  } else {
		def->defaults[idx] = strdup(def_buf+def_argd[idx]);
	  }
    }

    if (def_table == 0) {
        def_table = def;
    } else {
        struct define_t* cur = def_table;

        while (1) {
            int cmp = strcmp(def->name, cur->name);

            if (cmp == 0) {
                free(cur->value);
                cur->value = def->value;
                free(def->name);
                free(def);
                break;
            } else if (cmp < 0) {
                if (cur->left != 0) {
                    cur = cur->left;
                } else {
                    cur->left = def;
                    def->up = cur;
                    break;
                }
            } else {
                if (cur->right != 0) {
                    cur = cur->right;
                } else {
                    cur->right = def;
                    def->up = cur;
                    break;
                }
            }
        }
    }
}

static void free_macro(struct define_t* def)
{
    int idx;
    if (def == 0) return;
    free_macro(def->left);
    free_macro(def->right);
    free(def->name);
    free(def->value);
    for (idx = 0 ; idx < def->argc ; idx += 1) free(def->defaults[idx]);
    free(def->defaults);
    free(def);
}

void free_macros(void)
{
    free_macro(def_table);
}

/*
 * The do_define function accumulates the defined value in these
 * variables. When the define is over, the def_finish() function
 * executes the define and clears this text. The define_continue_flag
 * is set if do_define detects that the definition is to be continued
 * on the next line.
 */
static char*  define_text = 0;
static size_t define_cnt = 0;

static int define_continue_flag = 0;

/*
 * The do_magic function puts the expansions of magic macros into
 * this buffer and returns its address. It reallocs as needed to
 * fit its whole expansion. Because of this, do_magic is
 * -NOT REENTRANT-. It is called from do_expand, which strdups
 * or otherwise copies the result before doing any recursion, so
 * I don't anticipate any problems.
 */
static char* magic_text = 0;
static size_t magic_cnt = 0;

/*
 * Define a special character code used to mark the insertion point
 * for arguments in the macro text. This should be a character that
 * will not occur in the Verilog source code.
 */
#define ARG_MARK '\a'

#define _STR1(x) #x
#define _STR2(x) _STR1(x)

static int is_id_char(char c)
{
    return isalnum((int)c) || c == '_' || c == '$';
}

/*
 * Find an argument, but only if it is not directly preceded by something
 * that would make it part of another simple identifier ([a-zA-Z0-9_$]).
 */
static char *find_arg(char*ptr, char*head, char*arg)
{
    char *cp = ptr;
    size_t len = strlen(arg);

    while (1) {
        /* Look for a candidate match, just return if none is found. */
        cp = strstr(cp, arg);
        if (!cp) break;

        /* Verify that this match is not in the middle of another identifier. */
        if ((cp != head && is_id_char(cp[-1])) || is_id_char(cp[len])) {
            cp++;
            continue;
        }

        break;
    }

    return cp;
}

/*
 * Collect the definition. Normally, this returns 0. If there is a
 * continuation, then return 1 and this function may be called again
 * to collect another line of the definition.
 */
static void do_define(void)
{
    char* cp;
    char* head;
    char* tail;
    int added_cnt;
    int arg;

    define_continue_flag = 0;

    /* Look for comments in the definition, and remove them. The
     * "//" style comments go to the end of the line and terminate
     * the definition, but the multi-line comments are simply cut
     * out, and the define continues.
     */
    cp = strchr(yytext, '/');

    while (cp && *cp) {
        if (cp[1] == '/') {
            *cp = 0;
            break;
        }

        if (cp[1] == '*') {
            tail = strstr(cp+2, "*/");

            if (tail == 0) {
                *cp = 0;
                fprintf(stderr, "%s:%u: Unterminated comment in define\n",
		        istack->path, istack->lineno+1
                );
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
        if (!isspace((int)cp[-1])) break;

        cp -= 1;
        *cp = 0;
    }

    /* Detect the continuation sequence. If I find it, remove it
     * and the white space that precedes it, then replace all that
     * with a single newline.
     */
    if ((cp > yytext) && (cp[-1] == '\\')) {
        cp -= 1;
        cp[0] = 0;

        while ((cp > yytext) && isspace((int)cp[-1])) {
            cp -= 1;
            *cp = 0;
        }

        *cp++ = '\n';
        *cp = 0;

        define_continue_flag = 1;
    }

    /* Accumulate this text into the define_text string. */
    define_text = realloc(define_text, define_cnt + (cp-yytext) + 1);

    head = &define_text[define_cnt];
    strcpy(head, yytext);

    define_cnt += cp-yytext;

    tail = &define_text[define_cnt];

    /* If the text for a macro with arguments contains occurrences
     * of ARG_MARK, issue an error message and suppress the macro.
     */
    if ((def_argc > 1) && strchr(head, ARG_MARK)) {
        emit_pathline(istack);
        error_count += 1;
        def_argc = 0;

        fprintf(stderr, "error: implementation restriction - "
	        "macro text may not contain a %s character\n", _STR2(ARG_MARK));
    }

    /* Look for formal argument names in the definition, and replace
     * each occurrence with the sequence ARG_MARK,'\ddd' where ddd is
     * the  formal argument index number.
     */
    added_cnt = 0;
    for (arg = 1; arg < def_argc; arg++) {
        int argl = def_argl[arg];

        cp = find_arg(head, head, def_argv(arg));

        while (cp && *cp) {
            added_cnt += 2 - argl;

            if (added_cnt > 0) {
                char* base = define_text;

                define_cnt += added_cnt;
                define_text = realloc(define_text, define_cnt + 1);

                head = &define_text[head - base];
                tail = &define_text[tail - base];
                cp   = &define_text[cp   - base];

                added_cnt = 0;
            }

            memmove(cp+2, cp+argl, tail-(cp+argl)+1);

            tail += 2 - argl;

            *cp++ = ARG_MARK;
            *cp++ = arg;
            cp = find_arg(cp, head, def_argv(arg));
        }
    }
    define_cnt += added_cnt;
}

/*
 * Return true if the definition text is done. This is the opposite of
 * the define_continue_flag.
 */
static int def_is_done(void)
{
    return !define_continue_flag;
}

/*
 * Reset the define_continue_flag.
 */
static void def_continue(void)
{
    define_continue_flag = 0;
}

/*
 * After some number of calls to do_define, this function is called to
 * assigned value to the parsed name. If there is no value, then
 * assign the string "" (empty string.)
 */
static void def_finish(void)
{
    define_continue_flag = 0;

    if (def_argc <= 0) return;

    if (!define_text) {
        define_macro(def_argv(0), "", 0, def_argc);
    } else {
        define_macro(def_argv(0), define_text, 0, def_argc);

        free(define_text);

        define_text = 0;
        define_cnt = 0;
    }

    def_argc = 0;
}

static void def_undefine(void)
{
    struct define_t* cur;
    struct define_t* tail;
    int idx;

    /* def_buf is used to store the macro name. Make sure there is
     * enough space.
     */
    def_buf_grow_to_fit(yyleng);

    sscanf(yytext, "`undef %s", def_buf);

    cur = def_lookup(def_buf);
    if (cur == 0) return;
    if (cur->magic) return;

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

        if (cur->right) cur->right->up = cur->up;
    }
    else if (cur->right == 0) {
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

        while (tail->right) tail = tail->right;

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
    for (idx = 0 ; idx < cur->argc ; idx += 1) free(cur->defaults[idx]);
    free(cur->defaults);
    free(cur);
}

/*
 * When a macro is instantiated in the source, macro_needs_args() is
 * used to look up the name and return whether it is a macro that
 * takes arguments. A pointer to the macro descriptor is stored in
 * cur_macro so that do_expand() doesn't need to look it up again.
 */
static struct define_t* cur_macro = 0;

static int macro_needs_args(const char*text)
{
    cur_macro = def_lookup(text);

    if (cur_macro) {
        return (cur_macro->argc > 1);
    } else {
        emit_pathline(istack);
        fprintf(stderr, "warning: macro %s undefined (and assumed null) at this point.\n", text);
        return 0;
    }
}

static const char* macro_name(void)
{
    return cur_macro ? cur_macro->name : "";
}

static void macro_start_args(void)
{
    /* The macro name can be found via cur_macro, so create a null
     * entry for arg 0. This will be used by macro_finish_arg() to
     * calculate the buffer location for arg 1.
     */
    def_buf_free = def_buf_size - 1;
    def_buf[0] = 0;
    def_argo[0] = 0;
    def_argl[0] = 0;
    def_argc = 1;
}

static void macro_add_to_arg(int is_white_space)
{
    char* tail;
    int   length = yyleng;

    check_for_max_args();

    /* Replace any run of white space with a single space */
    if (is_white_space) {
        yytext[0] = ' ';
        yytext[1] = 0;
        length = 1;
    }

    /* Make sure there's room in the buffer for the new argument. */
    def_buf_grow_to_fit(length);

    /* Store the new text. */
    tail = &def_buf[def_buf_size - def_buf_free];
    strcpy(tail, yytext);
    def_buf_free -= length;
}

static void macro_finish_arg(void)
{
    int   offs;
    char* head;
    char* tail;

    check_for_max_args();

    offs = def_argo[def_argc-1] + def_argl[def_argc-1] + 1;
    head = &def_buf[offs];
    tail = &def_buf[def_buf_size - def_buf_free];

    /* Eat any leading and trailing white space. */
    if ((head < tail) && (*head == ' ')) {
	offs++;
	head++;
    }
    if ((tail > head) && (*(tail-1) == ' ')) {
	def_buf_free++;
	tail--;
    }

    *tail = 0;

    def_argo[def_argc] = offs;
    def_argl[def_argc] = tail - head;

    def_buf_free -= 1;
    def_argc++;
}

/*
 * The following variables are used to hold macro expansions that are
 * built dynamically using supplied arguments. Buffer space is allocated
 * as the macro is expanded, and is only released once the expansion has
 * been reparsed. This means that the buffer acts as a stack for nested
 * macro expansions.
 *
 * The expansion buffer is only used for macros with arguments - the
 * text for simple macros can be taken directly from the macro table.
 */
#define EXP_BUF_CHUNK 256

static char*exp_buf = 0;
static int  exp_buf_size = 0;
static int  exp_buf_free = 0;

static void exp_buf_grow_to_fit(int length)
{
    while (length >= exp_buf_free) {
        exp_buf_size += EXP_BUF_CHUNK;
        exp_buf_free += EXP_BUF_CHUNK;
        exp_buf = realloc(exp_buf, exp_buf_size);
    }
}

static void expand_using_args(void)
{
    char* head;
    char* tail;
    char* dest;
    int arg;
    int length;

    if (def_argc > cur_macro->argc) {
        emit_pathline(istack);
        fprintf(stderr, "error: too many arguments for `%s\n", cur_macro->name);
        return;
    }
    while (def_argc < cur_macro->argc) {
	if (cur_macro->defaults[def_argc]) {
	    def_argl[def_argc] = 0;
	    def_argc += 1;
	    continue;
	}
        emit_pathline(istack);
        fprintf(stderr, "error: too few arguments for `%s\n", cur_macro->name);
        return;
    }
    assert(def_argc == cur_macro->argc);

    head = cur_macro->value;
    tail = head;

    while (*tail) {
        if (*tail != ARG_MARK) {
            tail++;
        } else {
            arg = tail[1]; assert(arg < def_argc);

            char*use_argv;
	    int  use_argl;
	    if (def_argl[arg] == 0 && cur_macro->defaults[arg]) {
		  use_argv = cur_macro->defaults[arg];
		  use_argl = strlen(use_argv);
	    } else {
		  use_argv = def_argv(arg);
		  use_argl = def_argl[arg];
	    }
	    length = (tail - head) + use_argl;
            exp_buf_grow_to_fit(length);

            dest = &exp_buf[exp_buf_size - exp_buf_free];
            memcpy(dest, head, tail - head);
            dest += tail - head;
            memcpy(dest, use_argv, use_argl);

            exp_buf_free -= length;

            head = tail + 2;
            tail = head;
        }
    }

    length = tail - head;
    exp_buf_grow_to_fit(length);

    dest = &exp_buf[exp_buf_size - exp_buf_free];
    memcpy(dest, head, length + 1);

    exp_buf_free -= length + 1;
}

/*
 * When a macro use is discovered in the source, this function is
 * used to emit the substitution in its place.
 */
static void do_expand(int use_args)
{
    if (cur_macro) {
        struct include_stack_t*isp;
        int head = 0;
        const char *cp;
        unsigned escapes = 0;
        char *str_buf = 0;

        if (cur_macro->keyword) {
            fprintf(yyout, "%s", cur_macro->value);
	    if (do_expand_stringify_flag) {
		do_expand_stringify_flag = 0;
		fputc('"', yyout);
	    }
            return;
        }

        if (use_args) {
	    int tail = 0;
            head = exp_buf_size - exp_buf_free;
            expand_using_args();
            tail = exp_buf_size - exp_buf_free;
            exp_buf_free += tail - head;

            if (tail == head) return;
        }

        isp = (struct include_stack_t*) calloc(1, sizeof(struct include_stack_t));

	isp->stringify_flag = do_expand_stringify_flag;
	do_expand_stringify_flag = 0;
        if (use_args) {
            isp->str = &exp_buf[head];
        } else if(cur_macro->magic) {
            // cast const char * to char * to suppress warning, since we won't
            // be modifying isp->str in place.
            isp->str = (char*)do_magic(cur_macro->name);
        } else {
            isp->str = cur_macro->value;
        }

        /* Escape some characters if we are making a string version. */
        for (cp = isp->str; (cp = strpbrk(cp, "\"\\")); cp += 1, escapes += 1);
        if (escapes && isp->stringify_flag) {
            unsigned idx = 0;
            str_buf = (char *) malloc(strlen(isp->str)+3*escapes+1);
            for (cp = isp->str; *cp; cp += 1) {
                if (*cp == '"') {
                   str_buf[idx] = '\\';
                   str_buf[idx+1] = '0';
                   str_buf[idx+2] = '4';
                   str_buf[idx+3] = '2';
                   idx += 4;
                   continue;
                }
                if (*cp == '\\') {
                   str_buf[idx] = '\\';
                   str_buf[idx+1] = '1';
                   str_buf[idx+2] = '3';
                   str_buf[idx+3] = '4';
                   idx += 4;
                   continue;
                }
                str_buf[idx] = *cp;
                idx += 1;
            }
            str_buf[idx] = 0;
            idx += 1;

            exp_buf_free -= idx;

            isp->str = str_buf;
        } else isp->str = strdup(isp->str);

        isp->orig_str = isp->str;
        isp->next = istack;
        istack->yybs = YY_CURRENT_BUFFER;
        istack = isp;

        yy_switch_to_buffer(yy_create_buffer(istack->file, YY_BUF_SIZE));
    } else {
	  if (do_expand_stringify_flag) {
		do_expand_stringify_flag = 0;
		fputc('"', yyout);
	  }
    }
}

/*
 * Expand the magic macro named name. Return a buffer containing the expansion.
 */
static const char* do_magic(const char*name)
{
    size_t desired_cnt = 0;

    if(!magic_text) magic_text = malloc(24); // unimportant initial size

    if(!strcmp(name, "__LINE__")) {
        // istack->lineno is unsigned. the largest it could be is 64 bits.
        // 2^64 is between 10^19 and 10^20. So the decimal representation of
        // lineno can't possibly be longer than 23 bytes. I'm generous but
        // bytes are cheap and this is nobody's critical path.
        desired_cnt = 24;

        if(magic_cnt < desired_cnt) {
            magic_text = realloc(magic_text, desired_cnt);
            assert(magic_text);
            magic_cnt = desired_cnt;
        }

        int actual_len = snprintf(magic_text, desired_cnt,
                                  "%u", get_line(istack));
        assert(actual_len >= 0);
        assert((unsigned) actual_len < desired_cnt);
        return magic_text;
    } else if(!strcmp(name, "__FILE__")) {
        const char *path = get_path(istack);
        if(path) {
            desired_cnt = strlen(path)+2+1; // two quotes and a null

            if(magic_cnt < desired_cnt) {
                magic_text = realloc(magic_text, desired_cnt);
                assert(magic_text);
                magic_cnt = desired_cnt;
            }

            int actual_len = snprintf(magic_text, desired_cnt,
                                    "\"%s\"", path);

            assert(actual_len >= 0);
            assert((unsigned) actual_len < (unsigned)desired_cnt);
            return magic_text;
        }
    }

    // if we get here, then either there was no magic macro with the requested
    // name, or for some reason (an error?) we want to return an empty string
    assert(magic_cnt > 0);
    magic_text[0] = '\0';
    return magic_text;
}


/*
 * Include file handling works by keeping an include stack of the
 * files that are opened and being processed. The first item on the
 * stack is the current file being scanned. If I get to an include
 * statement,
 *
 *    open the new file,
 *    save the current buffer context,
 *    create a new buffer context,
 *    and push the new file information.
 *
 * When the file runs out, it is closed and the buffer is deleted
 * If after popping the current file information there is another
 * file on the stack, that file's buffer context is restored and
 * parsing resumes.
 */

static void output_init(void)
{
    if (line_direct_flag) {
        fprintf(yyout, "`line 1 \"%s\" 0\n", istack->path);
    }
}

static void include_filename(void)
{
    if(standby) {
        emit_pathline(istack);
        fprintf(stderr,
                "error: malformed `include directive. Extra junk on line?\n");
        exit(1);
    }

    standby = malloc(sizeof(struct include_stack_t));
    standby->path = strdup(yytext+1);
    standby->path[strlen(standby->path)-1] = 0;
    standby->lineno = 0;
    standby->comment = NULL;
}

static void do_include(void)
{
    /* standby is defined by include_filename() */
    if (standby->path[0] == '/') {
	if ((standby->file = fopen(standby->path, "r"))) {
	    standby->file_close = fclose;
            goto code_that_switches_buffers;
	}
    } else {
        unsigned idx, start = 1;
        char path[4096];
        char *cp;
        struct include_stack_t* isp;

        /* Add the current path to the start of the include_dir list. */
        isp = istack;
        while(isp && (isp->path == NULL)) isp = isp->next;

        assert(isp);

	strcpy(path, isp->path);
	cp = strrchr(path, '/');

        /* I may need the relative path for a planned warning even when
         * we are not in relative mode, so for now keep it around. */
        if (cp != 0) {
            *cp = '\0';
            include_dir[0] = strdup(path);
            if (relative_include) start = 0;
        }

        for (idx = start ;  idx < include_cnt ;  idx += 1) {
            sprintf(path, "%s/%s", include_dir[idx], standby->path);

            if ((standby->file = fopen(path, "r"))) {
		standby->file_close = fclose;
                /* Free the original path before we overwrite it. */
                free(standby->path);
                standby->path = strdup(path);
                goto code_that_switches_buffers;
            }
        }
    }

    emit_pathline(istack);
    fprintf(stderr, "Include file %s not found\n", standby->path);
    exit(1);

code_that_switches_buffers:

    /* Clear the current files path from the search list. */
    free(include_dir[0]);
    include_dir[0] = 0;

    if (depend_file) {
        if (dep_mode == 'p') {
            fprintf(depend_file, "I %s\n", standby->path);
        } else if (dep_mode != 'm') {
            fprintf(depend_file, "%s\n", standby->path);
        }
    }

    if (line_direct_flag) {
        fprintf(yyout, "\n`line 1 \"%s\" 1\n", standby->path);
    }

    standby->next = istack;
    standby->stringify_flag = 0;

    istack->yybs = YY_CURRENT_BUFFER;
    istack = standby;

    standby = 0;

    yy_switch_to_buffer(yy_create_buffer(istack->file, YY_BUF_SIZE));
}

/*
 * walk the include stack until we find an entry with a valid pathname,
 * and return the line from that entry for use in an error message.
 * This is the real file and line in which the outermost macro was used.
 */
static unsigned get_line(struct include_stack_t* isp)
{
    while(isp && (isp->path == NULL)) isp = isp->next;

    assert(isp);

    return isp->lineno+1;
}

/*
 * walk the include stack until we find an entry with a valid pathname,
 * and return the path from that entry for use in an error message.
 * This is the real file and line in which the outermost macro was used.
 */
static const char* get_path(struct include_stack_t* isp)
{
    while(isp && (isp->path == NULL)) isp = isp->next;

    assert(isp);

    return isp->path;
}

/* walk the include stack until we find an entry with a valid pathname,
 * and print the file and line from that entry for use in an error message.
 * The istack entries created by do_expand() for macro expansions do not
 * contain pathnames. This finds instead the real file in which the outermost
 * macro was used.
 */
static void emit_pathline(struct include_stack_t* isp)
{
    while(isp && (isp->path == NULL)) isp = isp->next;

    assert(isp);

    fprintf(stderr, "%s:%u: ", isp->path, isp->lineno+1);
}

static void lexor_done(void)
{
    while (ifdef_stack) {
        struct ifdef_stack_t*cur = ifdef_stack;
        ifdef_stack = cur->next;

        fprintf(stderr, "%s:%u: error: This `ifdef lacks an `endif.\n",
                cur->path, cur->lineno+1);

        free(cur->path);
        free(cur);
        error_count += 1;
    }
}

/*
 * Use this function to open a source file that is to be
 * processed. Do NOT use this function for opening include files,
 * instead only use this file for opening base source files.
 */
static void open_input_file(struct include_stack_t*isp)
{
      char*cp;
      int is_vhdl = 0;
      unsigned idx;

      isp->file = 0;

	/* look for a suffix for the input file. If the suffix
	   indicates that this is a VHDL source file, then invoke
	   vhdlpp to get a data stream. */
      cp = strrchr(isp->path, '.');
      if (cp && vhdlpp_path) {
	    if (strcmp(cp, ".vhd") == 0) {
		  is_vhdl = 1;
	    } else if (strcmp(cp, ".vhdl") == 0) {
		  is_vhdl = 1;
	    }
      }

      if (is_vhdl == 0) {
	    isp->file = fopen(isp->path, "r");
	    isp->file_close = fclose;
	    return;
      }

      size_t cmdlen = strlen(vhdlpp_path);
      cmdlen += strlen(isp->path);
      cmdlen += 8+strlen(vhdlpp_work);

      size_t liblen = 1;
      char*libs = strdup("");
      for (idx = 0 ; idx < vhdlpp_libdir_cnt ; idx += 1) {
	    size_t next_len = 6 + strlen(vhdlpp_libdir[idx]);
	    libs = realloc(libs, liblen+next_len);
	    snprintf(libs+liblen-1, next_len, " -L\"%s\"", vhdlpp_libdir[idx]);
	    liblen = strlen(libs) + 1;
      }

      cmdlen += liblen;

      char*cmd = malloc(cmdlen);
      snprintf(cmd, cmdlen, "%s -w\"%s\"%s %s", vhdlpp_path, vhdlpp_work, libs, isp->path);

      if (verbose_flag) fprintf(stderr, "Invoke vhdlpp: %s\n", cmd);

      isp->file = popen(cmd, "r");
      isp->file_close = pclose;

      free(libs);
      free(cmd);
      return;
}

/*
 * The load_next_input() function is called by the lexical analyzer
 * when the current file runs out. When the EOF of the current input
 * file is matched, this function figures out if this is the end of an
 * included file (in which case the including file is resumed) or the
 * end of a base file, in which case the next base source file is
 * opened.
 */
static int load_next_input(void)
{
    int line_mask_flag = 0;
    struct include_stack_t* isp = istack;
    istack = isp->next;

    /* Delete the current input buffers, and free the cell. */
    yy_delete_buffer(YY_CURRENT_BUFFER);

    /* If there was a comment for this include print it before we
     * return to the previous input stream. This technically belongs
     * to the previous stream, but it should not create any problems
     * since it is only a comment.
     */
    if (isp->comment) {
        fprintf(yyout, "%s\n", isp->comment);
        free(isp->comment);
        isp->comment = NULL;
    }

    if (isp->file) {
        free(isp->path);
	assert(isp->file_close);
        isp->file_close(isp->file);
    } else {
        /* If I am printing line directives and I just finished
         * macro substitution, I should terminate the line and
         * arrange for a new directive to be printed.
         */
        if (line_direct_flag && istack && istack->path && isp->lineno) {
            fprintf(yyout, "\n");
        } else line_mask_flag = 1;

        free(isp->orig_str);
    }

    if (isp->stringify_flag) fputc('"', yyout);

    free(isp);

    /* If I am out of include stack, the main input is
     * done. Look for another file to process in the input
     * queue. If none are there, give up. Otherwise, open the file
     * and continue parsing.
     */
    if (istack == 0) {
        if (file_queue == 0) {
            lexor_done();
            return 0;
        }

        istack = file_queue;
        file_queue = file_queue->next;

        istack->next = 0;
        istack->lineno = 0;
        open_input_file(istack);

        if (istack->file == 0) {
            perror(istack->path);
            error_count += 1;
            return 0;
        }

        if (line_direct_flag) {
            fprintf(yyout, "\n`line 1 \"%s\" 0\n", istack->path);
	}

        if (depend_file) {
            if (dep_mode == 'p') {
                fprintf(depend_file, "M %s\n", istack->path);
            } else if (dep_mode != 'i') {
                fprintf(depend_file, "%s\n", istack->path);
            }
        }

          /* This works around an issue in flex yyrestart() where it
           * uses yyin to create a new buffer when one does not exist.
           * I would have assumed that it would use the file argument.
           * The problem is that we have deleted the buffer and freed
           * yyin (isp->file) above. */
        yyin = 0;
        yyrestart(istack->file);
        return 1;
    }

    /* Otherwise, resume the input buffer that is the new stack
     * top. If I need to print a line directive, do so.
     */
    yy_switch_to_buffer(istack->yybs);

    if (line_direct_flag && istack->path && !line_mask_flag) {
        fprintf(yyout, "\n`line %u \"%s\" 2\n", istack->lineno+1, istack->path);
    }

    return 1;
}

/*
 * The dump_precompiled_defines() and load_precompiled_defines()
 * functions dump/load macro definitions to/from a file. The defines
 * are in the form:
 *
 *       <name>:<argc>:<len>:<value>
 *
 * for easy extraction. The value is already precompiled to handle
 * macro substitution. The <len> is the number of bytes in the
 * <value>. This is necessary because the value may contain arbitrary
 * text, including ':' and \n characters.
 *
 * Each record is terminated by a \n character.
 */
static void do_dump_precompiled_defines(FILE* out, struct define_t* table)
{
    if (!table->keyword)
        fprintf(out, "%s:%d:%zd:%s\n", table->name, table->argc, strlen(table->value), table->value);

    if (table->left) do_dump_precompiled_defines(out, table->left);

    if (table->right) do_dump_precompiled_defines(out, table->right);
}

void dump_precompiled_defines(FILE* out)
{
    if (def_table) do_dump_precompiled_defines(out, def_table);
}

void load_precompiled_defines(FILE* src)
{
    char*buf = malloc(4096);
    size_t buf_len = 4096;
    int ch;

    while ((ch = fgetc(src)) != EOF) {
        char* cp = buf;
        char* name = 0;

        int argc = 0;
        size_t len = 0;


	*cp++ = ch;

        while ((ch = fgetc(src)) != EOF && ch != ':') {
            *cp++ = ch;
	    assert( (size_t)(cp-buf) < buf_len );
	}

        if (ch != ':') return;

        /* Terminate the name string. */
        *cp++ = 0;
	assert( (size_t)(cp-buf) < buf_len );

        /* Read the argc number. (this doesn't need buffer space) */
        while (isdigit(ch = fgetc(src))) argc = 10*argc + ch-'0';

        if (ch != ':') return;

	  /* Read the value len (this doesn't need buffer space) */
        while (isdigit(ch = fgetc(src))) len = 10*len + ch-'0';

        if (ch != ':') return;

	  /* Save the name, and start the buffer over. */
	name = strdup(buf);

	  /* We now know how big the value should be, so if necessary,
	     reallocate the buffer to be sure we can hold it. */
	if ((len+4) >= buf_len) {
	      buf = realloc(buf, len+8);
	      assert(buf);
	}

	cp = buf;

        while (len > 0) {
            ch = fgetc(src);
            if (ch == EOF) {
		  free(name);
		  return;
	    }

            *cp++ = ch;
            len -= 1;
        }

        *cp++ = 0;

        ch = fgetc(src);
        if (ch != '\n') {
	      free(name);
	      free(buf);
	      return;
	}

        define_macro(name, buf, 0, argc);
	free(name);
    }
}

/*
 * This function initializes the whole process. The first file is
 * opened, and the lexor is initialized. The include stack is cleared
 * and ready to go.
 */
void reset_lexor(FILE* out, char* paths[])
{
    unsigned idx;
    struct include_stack_t* isp;
    struct include_stack_t* tail = 0;

    isp = malloc(sizeof(struct include_stack_t));
    isp->next = 0;
    isp->path = strdup(paths[0]);
    open_input_file(isp);
    isp->str = 0;
    isp->lineno = 0;
    isp->stringify_flag = 0;
    isp->comment = NULL;

    if (isp->file == 0) {
        perror(paths[0]);
        exit(1);
    }

    if (depend_file) {
        if (dep_mode == 'p') {
            fprintf(depend_file, "M %s\n", paths[0]);
        } else if (dep_mode != 'i') {
            fprintf(depend_file, "%s\n", paths[0]);
        }
    }

    yyout = out;

    yyrestart(isp->file);

    assert(istack == 0);
    istack = isp;

    /* Now build up a queue of all the remaining file names, so
     * that load_next_input() can pull them when needed.
     */
    for (idx = 1 ; paths[idx] ; idx += 1) {
        isp = malloc(sizeof(struct include_stack_t));
        isp->path = strdup(paths[idx]);
        isp->file = 0;
        isp->str = 0;
        isp->next = 0;
        isp->lineno = 0;
        isp->stringify_flag = 0;
        isp->comment = NULL;

        if (tail) tail->next = isp;
        else file_queue = isp;

        tail = isp;
    }
}

/*
 * Modern version of flex (>=2.5.9) can clean up the scanner data.
 */
void destroy_lexor(void)
{
# ifdef FLEX_SCANNER
#   if YY_FLEX_MAJOR_VERSION >= 2 && YY_FLEX_MINOR_VERSION >= 5
#     if YY_FLEX_MINOR_VERSION > 5 || defined(YY_FLEX_SUBMINOR_VERSION) && YY_FLEX_SUBMINOR_VERSION >= 9
    yylex_destroy();
#     endif
#   endif
# endif
    free(def_buf);
    free(exp_buf);
}
