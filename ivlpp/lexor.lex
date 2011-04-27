%{
/*
 * Copyright (c) 1999-2011 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <ctype.h>
# include  <assert.h>

# include  "globals.h"

static void output_init();
#define YY_USER_INIT output_init()

static void  def_start();
static void  def_add_arg();
static void  def_finish();
static void  def_undefine();
static void  do_define();
static int   def_is_done();
static int   is_defined(const char*name);

static int   macro_needs_args(const char*name);
static void  macro_start_args();
static void  macro_add_to_arg(int is_white_space);
static void  macro_finish_arg();
static void  do_expand(int use_args);
static const char* macro_name();

static void include_filename();
static void do_include();

static int load_next_input();

struct include_stack_t
{
    char* path;

    /* If the current input is from a file, this member is set. */
    FILE* file;

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
        fprintf
        (
            stderr,
            "%s:%u: warning: This `endif matches an ifdef in another file.\n",
            istack->path,
            istack->lineno+1
        );

        fprintf
        (
            stderr,
            "%s:%u: This is the odd matched `ifdef.\n",
            cur->path,
            cur->lineno+1
        );
    }

    free(cur->path);
    free(cur);
}

#define YY_INPUT(buf,result,max_size) do {                 \
    if (istack->file) {                                    \
        size_t rc = fread(buf, 1, max_size, istack->file); \
        result = (rc == 0) ? YY_NULL : rc;                 \
    } else {                                               \
        if (*istack->str == 0)                             \
            result = YY_NULL;                              \
        else {                                             \
            buf[0] = *istack->str++;                       \
            result = 1;                                    \
        }                                                  \
    }                                                      \
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
    fprintf
    (
        stderr,
        "error: macro names cannot be directive keywords ('%s'); replaced with nothing.\n",
        yytext
    );
}

<PCOMENT>`[a-zA-Z][a-zA-Z0-9_$]* {
    if (macro_needs_args(yytext+1)) yy_push_state(MA_START); else do_expand(0);
}

 /* Strings do not contain preprocessor directives, but can expand
  * macros. If that happens, they get expanded in the context of the
  * string.
  */
\"            { string_enter = YY_START; BEGIN(CSTRING); ECHO; }
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
    fprintf
    (
        stderr,
        "error: macro names cannot be directive keywords ('%s'); replaced with nothing.\n",
        yytext
    );
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
    fprintf
    (
        stderr,
        "error: malformed `define directive: macro names cannot be directive keywords\n"
    );
}

<DEF_NAME>[a-zA-Z_][a-zA-Z0-9_$]*"("{W}? { BEGIN(DEF_ARG); def_start(); }
<DEF_NAME>[a-zA-Z_][a-zA-Z0-9_$]*{W}?    { BEGIN(DEF_TXT); def_start(); }

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
    fprintf
    (
        stderr,
        "%s:%u: `ifdef without a macro name - ignored.\n",
        istack->path, istack->lineno+1
    );
}

`ifndef {
    error_count += 1;
    fprintf
    (
        stderr,
        "%s:%u: `ifndef without a macro name - ignored.\n",
        istack->path, istack->lineno+1
    );
}

`elsif {
    error_count += 1;
    fprintf
    (
        stderr,
        "%s:%u: `elsif without a macro name - ignored.\n",
        istack->path, istack->lineno+1
    );
}

`elsif{W}[a-zA-Z_][a-zA-Z0-9_$]* {
    error_count += 1;
    fprintf
    (
        stderr,
        "%s:%u: `elsif without a matching `ifdef - ignored.\n",
        istack->path, istack->lineno+1
    );
}

`else {
    error_count += 1;
    fprintf
    (
        stderr,
        "%s:%u: `else without a matching `ifdef - ignored.\n",
        istack->path, istack->lineno+1
    );
}

`endif {
    error_count += 1;
    fprintf
    (
        stderr,
        "%s:%u: `endif without a matching `ifdef - ignored.\n",
        istack->path, istack->lineno+1
    );
}

`{keywords} {
    emit_pathline(istack);
    error_count += 1;
    fprintf
    (
        stderr,
        "error: macro names cannot be directive keywords ('%s'); replaced with nothing.\n",
        yytext
    );
}

 /* This pattern notices macros and arranges for them to be replaced. */
`[a-zA-Z_][a-zA-Z0-9_$]* {
    if (macro_needs_args(yytext+1))
        yy_push_state(MA_START);
    else
        do_expand(0);
}

  /* Stringified version of macro expansion. */
``[a-zA-Z_][a-zA-Z0-9_$]* {
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

<MA_ADD>"," { if (ma_parenthesis_level > 0) macro_add_to_arg(0);
              else macro_finish_arg(); }

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

<<EOF>> { if (!load_next_input()) yyterminate(); }

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

    struct define_t*    left;
    struct define_t*    right;
    struct define_t*    up;
};

static struct define_t* def_table = 0;

static struct define_t* def_lookup(const char*name)
{
    struct define_t* cur = def_table;

    if (cur == 0)
        return 0;

    assert(cur->up == 0);

    while (cur)
    {
        int cmp = strcmp(name, cur->name);

        if (cmp == 0)
            return cur;

        cur = (cmp < 0) ? cur->left : cur->right;
    }

    return 0;
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

static char* def_buf = 0;
static int   def_buf_size = 0;
static int   def_buf_free = 0;

static int   def_argc = 0;
static int   def_argo[MAX_DEF_ARG];  /* offset of first character in arg */
static int   def_argl[MAX_DEF_ARG];  /* length of arg string. */

/*
 * Return a pointer to the start of argument 'arg'. Returned pointers
 * may go stale after a call to def_buf_grow_to_fit.
 */
static /* inline */ char* def_argv(int arg)
{
    return def_buf + def_argo[arg];
}

static void check_for_max_args()
{
    if (def_argc == MAX_DEF_ARG)
    {
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

static void def_start()
{
    def_buf_free = def_buf_size;
    def_argc = 0;
    def_add_arg();
}

static void def_add_arg()
{
    int length = yyleng;

    check_for_max_args();

    /* Remove trailing white space and, if necessary, opening brace. */
    while (isspace((int)yytext[length - 1]))
        length--;

    if (yytext[length - 1] == '(')
        length--;

    yytext[length] = 0;

    /* Make sure there's room in the buffer for the new argument. */
    def_buf_grow_to_fit(length);

    /* Store the new argument. */
    def_argl[def_argc] = length;
    def_argo[def_argc] = def_buf_size - def_buf_free;
    strcpy(def_argv(def_argc++), yytext);
    def_buf_free -= length + 1;
}

void define_macro(const char* name, const char* value, int keyword, int argc)
{
    struct define_t* def;

    def = malloc(sizeof(struct define_t));
    def->name = strdup(name);
    def->value = strdup(value);
    def->keyword = keyword;
    def->argc = argc;
    def->left = 0;
    def->right = 0;
    def->up = 0;

    if (def_table == 0)
        def_table = def;
    else
    {
        struct define_t* cur = def_table;

        while (1)
        {
            int cmp = strcmp(def->name, cur->name);

            if (cmp == 0)
            {
                free(cur->value);
                cur->value = def->value;
                free(def->name);
                free(def);
                break;
            }
            else if (cmp < 0)
            {
                if (cur->left != 0)
                    cur = cur->left;
                else
                {
                    cur->left = def;
                    def->up = cur;
                    break;
                }
            }
            else
            {
                if (cur->right != 0)
                    cur = cur->right;
                else
                {
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
    if (def == 0) return;
    free_macro(def->left);
    free_macro(def->right);
    free(def->name);
    free(def->value);
    free(def);
}

void free_macros()
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
 * Define a special character code used to mark the insertion point
 * for arguments in the macro text. This should be a character that
 * will not occur in the Verilog source code.
 */
#define ARG_MARK '\a'

#define _STR1(x) #x
#define _STR2(x) _STR1(x)

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

        /* If we are not at the start of the string verify that this
         * match is not in the middle of another identifier.
         */
        if (cp != head &&
            (isalnum((int)*(cp-1)) || *(cp-1) == '_' || *(cp-1) == '$' ||
             isalnum((int)*(cp+len)) || *(cp+len) == '_' || *(cp+len) == '$')) {
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
static void do_define()
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

    while (cp && *cp)
    {
        if (cp[1] == '/') {
            *cp = 0;
            break;
        }

        if (cp[1] == '*')
        {
            tail = strstr(cp+2, "*/");

            if (tail == 0)
            {
                *cp = 0;
                fprintf
                (
                    stderr,
                    "%s:%u: Unterminated comment in define\n",
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
    while (cp > yytext)
    {
        if (!isspace((int)cp[-1]))
            break;

        cp -= 1;
        *cp = 0;
    }

    /* Detect the continuation sequence. If I find it, remove it
     * and the white space that precedes it, then replace all that
     * with a single newline.
     */
    if ((cp > yytext) && (cp[-1] == '\\'))
    {
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
    define_text = realloc(define_text, define_cnt + (cp-yytext) + 1); assert(define_text != 0);

    head = &define_text[define_cnt];
    strcpy(head, yytext);

    define_cnt += cp-yytext;

    tail = &define_text[define_cnt];

    /* If the text for a macro with arguments contains occurrences
     * of ARG_MARK, issue an error message and suppress the macro.
     */
    if ((def_argc > 1) && strchr(head, ARG_MARK))
    {
        emit_pathline(istack);
        error_count += 1;
        def_argc = 0;

        fprintf
        (
            stderr,
            "error: implementation restriction - macro text may not contain a %s character\n",
            _STR2(ARG_MARK)
        );
    }

    /* Look for formal argument names in the definition, and replace
     * each occurrence with the sequence ARG_MARK,'\ddd' where ddd is
     * the  formal argument index number.
     */
    added_cnt = 0;
    for (arg = 1; arg < def_argc; arg++)
    {
        int argl = def_argl[arg];

        cp = find_arg(head, head, def_argv(arg));

        while (cp && *cp)
        {
            added_cnt += 2 - argl;

            if (added_cnt > 0)
            {
                char* base = define_text;

                define_cnt += added_cnt;
                define_text = realloc(define_text, define_cnt + 1); assert(define_text != 0);

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
static int def_is_done()
{
    return !define_continue_flag;
}

/*
 * After some number of calls to do_define, this function is called to
 * assigned value to the parsed name. If there is no value, then
 * assign the string "" (empty string.)
 */
static void def_finish()
{
    define_continue_flag = 0;

    if (def_argc <= 0)
        return;

    if (!define_text)
        define_macro(def_argv(0), "", 0, def_argc);
    else
    {
        define_macro(def_argv(0), define_text, 0, def_argc);

        free(define_text);

        define_text = 0;
        define_cnt = 0;
    }

    def_argc = 0;
}

static void def_undefine()
{
    struct define_t* cur;
    struct define_t* tail;

    /* def_buf is used to store the macro name. Make sure there is
     * enough space.
     */
    def_buf_grow_to_fit(yyleng);

    sscanf(yytext, "`undef %s", def_buf);

    cur = def_lookup(def_buf);
    if (cur == 0) return;

    if (cur->up == 0)
    {
        if ((cur->left == 0) && (cur->right == 0))
            def_table = 0;
        else if (cur->left == 0)
        {
            def_table = cur->right;
            if (cur->right)
            cur->right->up = 0;
        }
        else if (cur->right == 0)
        {
            assert(cur->left);
            def_table = cur->left;
            def_table->up = 0;
        }
        else
        {
            tail = cur->left;
            while (tail->right)
                tail = tail->right;

            tail->right = cur->right;
            tail->right->up = tail;

            def_table = cur->left;
            def_table->up = 0;
        }
    }
    else if (cur->left == 0)
    {
        if (cur->up->left == cur)
            cur->up->left = cur->right;
        else
        {
            assert(cur->up->right == cur);
            cur->up->right = cur->right;
        }

        if (cur->right)
            cur->right->up = cur->up;
    }
    else if (cur->right == 0)
    {
        assert(cur->left);

        if (cur->up->left == cur)
            cur->up->left = cur->left;
        else
        {
            assert(cur->up->right == cur);
            cur->up->right = cur->left;
        }

        cur->left->up = cur->up;
    }
    else
    {
        tail = cur->left;

        assert(cur->left && cur->right);

        while (tail->right)
            tail = tail->right;

        tail->right = cur->right;
        tail->right->up = tail;

        if (cur->up->left == cur)
            cur->up->left = cur->left;
        else
        {
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
 * When a macro is instantiated in the source, macro_needs_args() is
 * used to look up the name and return whether it is a macro that
 * takes arguments. A pointer to the macro descriptor is stored in
 * cur_macro so that do_expand() doesn't need to look it up again.
 */
static struct define_t* cur_macro = 0;

static int macro_needs_args(const char*text)
{
    cur_macro = def_lookup(text);

    if (cur_macro)
        return (cur_macro->argc > 1);
    else
    {
        emit_pathline(istack);
        fprintf(stderr, "warning: macro %s undefined (and assumed null) at this point.\n", text);
        return 0;
    }
}

static const char* macro_name()
{
    return cur_macro ? cur_macro->name : "";
}

static void macro_start_args()
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
    if (is_white_space)
    {
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

static void macro_finish_arg()
{
    char* tail = &def_buf[def_buf_size - def_buf_free];

    check_for_max_args();

    *tail = 0;

    def_argo[def_argc] = def_argo[def_argc-1] + def_argl[def_argc-1] + 1;
    def_argl[def_argc] = tail - def_argv(def_argc);

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
    while (length >= exp_buf_free)
    {
        exp_buf_size += EXP_BUF_CHUNK;
        exp_buf_free += EXP_BUF_CHUNK;
        exp_buf = realloc(exp_buf, exp_buf_size); assert(exp_buf != 0);
    }
}

static void expand_using_args()
{
    char* head;
    char* tail;
    char* dest;
    int arg;
    int length;

    if (def_argc != cur_macro->argc)
    {
        emit_pathline(istack);
        fprintf(stderr, "error: wrong number of arguments for `%s\n", cur_macro->name);
        return;
    }

    head = cur_macro->value;
    tail = head;

    while (*tail)
    {
        if (*tail != ARG_MARK)
            tail++;
        else
        {
            arg = tail[1]; assert(arg < def_argc);

            length = (tail - head) + def_argl[arg];
            exp_buf_grow_to_fit(length);

            dest = &exp_buf[exp_buf_size - exp_buf_free];
            memcpy(dest, head, tail - head);
            dest += tail - head;
            memcpy(dest, def_argv(arg), def_argl[arg]);

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
    if (cur_macro)
    {
        struct include_stack_t*isp;
        int head = 0;
        int tail = 0;
        const char *cp;
        unsigned escapes = 0;
        char *str_buf = 0;

        if (cur_macro->keyword)
        {
            fprintf(yyout, "%s", cur_macro->value);
	    if (do_expand_stringify_flag) {
		do_expand_stringify_flag = 0;
		fputc('"', yyout);
	    }
            return;
        }

        if (use_args)
        {
            head = exp_buf_size - exp_buf_free;
            expand_using_args();
            tail = exp_buf_size - exp_buf_free;
            exp_buf_free += tail - head;

            if (tail == head)
                return;
        }

        isp = (struct include_stack_t*) calloc(1, sizeof(struct include_stack_t));

	isp->stringify_flag = do_expand_stringify_flag;
	do_expand_stringify_flag = 0;
        if (use_args)
        {
            isp->str = &exp_buf[head];
        }
        else
        {
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
        } else
            isp->str = strdup(isp->str);

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

static void output_init()
{
    if (line_direct_flag)
        fprintf(yyout, "`line 1 \"%s\" 0\n", istack->path);
}

static void include_filename()
{
    if(standby) {
        emit_pathline(istack);
        fprintf
        (
            stderr,
            "error: malformed `include directive. Extra junk on line?\n"
        );
        exit(1);
    }

    standby = malloc(sizeof(struct include_stack_t));
    standby->path = strdup(yytext+1);
    standby->path[strlen(standby->path)-1] = 0;
    standby->lineno = 0;
    standby->comment = NULL;
}

static void do_include()
{
    /* standby is defined by include_filename() */
    if (standby->path[0] == '/') {
        if ((standby->file = fopen(standby->path, "r")))
            goto code_that_switches_buffers;
    } else {
        unsigned idx, start = 1;
        char path[4096];
        char *cp;
        struct include_stack_t* isp;

        /* Add the current path to the start of the include_dir list. */
        isp = istack;
        while(isp && (isp->path == NULL))
	    isp = isp->next;

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

    if(depend_file)
        fprintf(depend_file, "%s\n", standby->path);

    if (line_direct_flag)
        fprintf(yyout, "\n`line 1 \"%s\" 1\n", standby->path);

    standby->next = istack;
    standby->stringify_flag = 0;

    istack->yybs = YY_CURRENT_BUFFER;
    istack = standby;

    standby = 0;

    yy_switch_to_buffer(yy_create_buffer(istack->file, YY_BUF_SIZE));
}

/* walk the include stack until we find an entry with a valid pathname,
 * and print the file and line from that entry for use in an error message.
 * The istack entries created by do_expand() for macro expansions do not
 * contain pathnames. This finds instead the real file in which the outermost
 * macro was used.
 */
static void emit_pathline(struct include_stack_t* isp)
{
    while(isp && (isp->path == NULL))
        isp = isp->next;

    assert(isp);

    fprintf(stderr, "%s:%u: ", isp->path, isp->lineno+1);
}

static void lexor_done()
{
    while (ifdef_stack)
    {
        struct ifdef_stack_t*cur = ifdef_stack;
        ifdef_stack = cur->next;

        fprintf
        (
            stderr,
            "%s:%u: error: This `ifdef lacks an `endif.\n",
            cur->path, cur->lineno+1
        );

        free(cur->path);
        free(cur);
        error_count += 1;
    }
}

static int load_next_input()
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

    if (isp->file)
    {
        free(isp->path);
        fclose(isp->file);
    }
    else
    {
        /* If I am printing line directives and I just finished
         * macro substitution, I should terminate the line and
         * arrange for a new directive to be printed.
         */
        if (line_direct_flag && istack && istack->path && isp->lineno)
            fprintf(yyout, "\n");
        else
            line_mask_flag = 1;

        free(isp->orig_str);
    }

    if (isp->stringify_flag) {
	  fputc('"', yyout);
    }

    free(isp);

    /* If I am out of include stack, the main input is
     * done. Look for another file to process in the input
     * queue. If none are there, give up. Otherwise, open the file
     * and continue parsing.
     */
    if (istack == 0)
    {
        if (file_queue == 0)
        {
            lexor_done();
            return 0;
        }

        istack = file_queue;
        file_queue = file_queue->next;

        istack->next = 0;
        istack->lineno = 0;
        istack->file = fopen(istack->path, "r");

        if (istack->file == 0)
        {
            perror(istack->path);
            error_count += 1;
            return 0;
        }

        if (line_direct_flag)
            fprintf(yyout, "\n`line 1 \"%s\" 0\n", istack->path);

        if(depend_file)
            fprintf(depend_file, "%s\n", istack->path);

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

    if (line_direct_flag && istack->path && !line_mask_flag)
        fprintf(yyout, "\n`line %u \"%s\" 2\n", istack->lineno+1, istack->path);

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
#ifdef __MINGW32__  /* MinGW does not know about z. */
        fprintf(out, "%s:%d:%d:%s\n", table->name, table->argc, strlen(table->value), table->value);
#else
        fprintf(out, "%s:%d:%zd:%s\n", table->name, table->argc, strlen(table->value), table->value);
#endif

    if (table->left)
        do_dump_precompiled_defines(out, table->left);

    if (table->right)
        do_dump_precompiled_defines(out, table->right);
}

void dump_precompiled_defines(FILE* out)
{
    if (def_table)
        do_dump_precompiled_defines(out, def_table);
}

void load_precompiled_defines(FILE* src)
{
    char buf[4096];
    int ch;

    while ((ch = fgetc(src)) != EOF)
    {
        char* cp = buf;
        char* name = 0;
        char* value = 0;

        int argc = 0;
        size_t len = 0;

        name = cp;

	*cp++ = ch;

        while ((ch = fgetc(src)) != EOF && ch != ':')
            *cp++ = ch;

        if (ch != ':')
            return;

        /* Terminate the name string. */
        *cp++ = 0;

        /* Read the argc number */
        while (isdigit(ch = fgetc(src)))
            argc = 10*argc + ch-'0';

        if (ch != ':')
            return;

        while (isdigit(ch = fgetc(src)))
            len = 10*len + ch-'0';

        if (ch != ':')
            return;

        value = cp;

        while (len > 0)
        {
            ch = fgetc(src);
            if (ch == EOF)
                return;

            *cp++ = ch;
            len -= 1;
        }

        *cp++ = 0;

        ch = fgetc(src);
        if (ch != '\n')
            return;

        define_macro(name, value, 0, argc);
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
    isp->file = fopen(paths[0], "r");
    isp->str = 0;
    isp->lineno = 0;
    isp->stringify_flag = 0;
    isp->comment = NULL;

    if (isp->file == 0)
    {
        perror(paths[0]);
        exit(1);
    }

    if(depend_file)
          fprintf(depend_file, "%s\n", paths[0]);

    yyout = out;

    yyrestart(isp->file);

    assert(istack == 0);
    istack = isp;

    /* Now build up a queue of all the remaining file names, so
     * that load_next_input() can pull them when needed.
     */
    for (idx = 1 ; paths[idx] ; idx += 1)
    {
        isp = malloc(sizeof(struct include_stack_t));
        isp->path = strdup(paths[idx]);
        isp->file = 0;
        isp->str = 0;
        isp->next = 0;
        isp->lineno = 0;
        isp->stringify_flag = 0;
        isp->comment = NULL;

        if (tail)
            tail->next = isp;
        else
            file_queue = isp;

        tail = isp;
    }
}

/*
 * Modern version of flex (>=2.5.9) can clean up the scanner data.
 */
void destroy_lexor()
{
# ifdef FLEX_SCANNER
#   if YY_FLEX_MAJOR_VERSION >= 2 && YY_FLEX_MINOR_VERSION >= 5
#     if defined(YY_FLEX_SUBMINOR_VERSION) && YY_FLEX_SUBMINOR_VERSION >= 9
    yylex_destroy();
#     endif
#   endif
# endif
    free(def_buf);
    free(exp_buf);
}
