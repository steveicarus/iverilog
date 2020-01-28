%option prefix="cf"
%option nounput
%option noinput

%{
/*
 * Copyright (c) 2001-2017 Stephen Williams (steve@icarus.com)
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

# include  "cfparse_misc.h"
# include  "cfparse.h"
# include  "globals.h"
# include  <string.h>

char *current_file = NULL;

static int comment_enter;
static char* trim_trailing_white(char*txt, int trim);

/*
 * Mostly copied from the flex manual. Do not make this arbitrary
 * depth without checking for looping files.
 */
#define MAX_CMDFILE_DEPTH 15

typedef struct t_cmdfile {
      char *cmdfile;
      YY_BUFFER_STATE buffer;
} s_cmdfile;
s_cmdfile cmdfile_stack[MAX_CMDFILE_DEPTH];
int cmdfile_stack_ptr = 0;

%}

%x CCOMMENT
%x LCOMMENT
%x PLUS_ARGS
%x FILE_NAME

%%

  /* Accept C++ style comments. */
"//".* { comment_enter = YY_START; BEGIN(LCOMMENT); }
<LCOMMENT>.    { yymore(); }
<LCOMMENT>\n   { cflloc.first_line += 1; BEGIN(comment_enter); }

  /* Accept C style comments. */
"/*" { comment_enter = YY_START; BEGIN(CCOMMENT); }
<CCOMMENT>.    { yymore(); }
<CCOMMENT>\n   { cflloc.first_line += 1; yymore(); }
<CCOMMENT>"*/" { BEGIN(comment_enter); }

  /* Accept shell type comments. */
^"#".* { ; }

  /* Skip white space. */
[ \t\f\r] { ; }
  /* Skip line ends, but also count the line. */
\n { cflloc.first_line += 1; }


"+define+" { BEGIN(PLUS_ARGS); return TOK_DEFINE; }

"+incdir+" { BEGIN(PLUS_ARGS); return TOK_INCDIR; }

"+integer-width+" { BEGIN(PLUS_ARGS); return TOK_INTEGER_WIDTH; }

"+libdir+" { BEGIN(PLUS_ARGS); return TOK_LIBDIR; }

"+libdir-nocase+" { BEGIN(PLUS_ARGS); return TOK_LIBDIR_NOCASE; }

"+libext+" { BEGIN(PLUS_ARGS); return TOK_LIBEXT; }

"+parameter+" { BEGIN(PLUS_ARGS); return TOK_PARAMETER; }

"+timescale+" { BEGIN(PLUS_ARGS); return TOK_TIMESCALE; }

"+vhdl-work+" { BEGIN(PLUS_ARGS); return TOK_VHDL_WORK; }

"+vhdl-libdir+" { BEGIN(PLUS_ARGS); return TOK_VHDL_LIBDIR; }

"+width-cap+" { BEGIN(PLUS_ARGS); return TOK_WIDTH_CAP; }

  /* If it is not any known plus-flag, return the generic form. */
"+"[^\n \t\b\f\r+]* {
      cflval.text = strdup(yytext);
      BEGIN(PLUS_ARGS);
      return TOK_PLUSWORD; }

  /* Once in PLUS_ARGS mode, words are delimited by +
     characters. White space and line end terminate PLUS_ARGS mode,
     but + terminates only the word. */
<PLUS_ARGS>[^\n \t\b\f\r+]* {
      cflval.text = strdup(yytext);
      return TOK_PLUSARG; }

  /* Within plusargs, this is a delimiter. */
<PLUS_ARGS>"+" { }

  /* White space end plus_args mode. */
<PLUS_ARGS>[ \t\b\f\r] { BEGIN(0); }

<PLUS_ARGS>\n {
      cflloc.first_line += 1;
      BEGIN(0); }

  /* Notice the -a flag. */
"-a" { return TOK_Da; }

  /* Notice the -c or -f flag. */
"-c" { return TOK_Dc; }
"-f" { return TOK_Dc; }

  /* Notice the -l or -v flag. */
"-l" { return TOK_Dv; }
"-v" { return TOK_Dv; }

  /* Notice the -y flag. */
"-y" { return TOK_Dy; }

  /* This rule matches paths and strings that may be file names. This
     is a little bit tricky, as we don't want to mistake a comment for
     a string word. */
"/"[\r\n] { /* Special case of file name "/" */
      cflval.text = trim_trailing_white(yytext, 0);
      return TOK_STRING; }
"/"[^\*\/] { /* A file name that starts with "/". */
      yymore();
      BEGIN(FILE_NAME); }
[^/\n \t\b\r+-][^/\n\r]* { /* A file name that starts with other than "/" */
      yymore();
      BEGIN(FILE_NAME); }

<FILE_NAME>"//" {
	/* Found a trailing comment. Returning the terminated name. */
      cflval.text = trim_trailing_white(yytext, 2);
      BEGIN(LCOMMENT);
      return TOK_STRING; }
<FILE_NAME>"/"?[^/\n\r]* {
      yymore();
      /* not a comment... continuing */; }
<FILE_NAME>[\n\r] {
	/* No trailing comment. Return the file name. */
      cflval.text = trim_trailing_white(yytext, 0);
      BEGIN(0);
      return TOK_STRING; }

  /* Fallback match. */
. { return yytext[0]; }

  /* At the end of file switch back to the previous buffer. */
<<EOF>> {
      if (--cmdfile_stack_ptr < 0) {
	    free(current_file);
	    yyterminate();
      } else {
	    yy_delete_buffer(YY_CURRENT_BUFFER);
	    yy_switch_to_buffer(cmdfile_stack[cmdfile_stack_ptr].buffer);
	    free(current_file);
	    current_file = cmdfile_stack[cmdfile_stack_ptr].cmdfile;
      } }

%%

static char* trim_trailing_white(char*text, int trim)
{
      char*cp = text + strlen(text);
      while (cp > text && trim > 0) {
	    trim -= 1;
	    cp -= 1;
	    *cp = 0;
      }
      while (cp > text && strchr("\n\r\t\b", cp[-1]))
	    cp -= 1;

      cp[0] = 0;
      return strdup(text);
}

int yywrap()
{
      return 1;
}

void switch_to_command_file(const char *file)
{
      char path[4096];

      if (cmdfile_stack_ptr >= MAX_CMDFILE_DEPTH) {
	    fprintf(stderr, "Error: command files nested too deeply (%d) "
                    "at %s:%d.\n", MAX_CMDFILE_DEPTH, current_file,
                    cflloc.first_line);
	    exit(1);
      }

      cmdfile_stack[cmdfile_stack_ptr].buffer = YY_CURRENT_BUFFER;
      cmdfile_stack[cmdfile_stack_ptr].cmdfile = current_file;
      cmdfile_stack_ptr += 1;

        /*
         * If this is a relative file then add the current path to the
         * file name.
         */
      if (file[0] != '/') {
	    char *cp;
	    strcpy(path, current_file);
	    cp = strrchr(path, '/');
	    if (cp == 0) strcpy(path, file);  /* A base file. */
	    else {
	          *(cp+1) = '\0';
	          strcat(path, file);
	    }
      } else strcpy(path, file);  /* An absolute path. */

      yyin = fopen(path, "r");
      if (yyin == NULL) {
	    fprintf(stderr, "Error: unable to read nested command file (%s) "
                    "at %s:%d.\n", path, current_file, cflloc.first_line);
	    exit(1);
      }

      current_file = strdup(path);
      yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
      cflloc.first_line = 1;
}

void cfreset(FILE*fd, const char*path)
{
      yyin = fd;
      yyrestart(fd);
      current_file = strdup(path);
      cflloc.first_line = 1;
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
}
