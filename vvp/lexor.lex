
%{
# include  "parse_misc.h"
# include  "compile.h"
# include  "parse.h"
# include  <string.h>
%}

%%

  /* A label is any non-blank text that appears left justified. */
^[.$_a-zA-Z][.$_a-zA-Z0-9]* {
      yylval.text = strdup(yytext);
      return T_LABEL; }

  /* String tokens are parsed here. Return as the token value the
     contents of the string without the enclosing quotes. */
\"[^\"]*\" {
      yytext[strlen(yytext)-1] = 0;
      yylval.text = strdup(yytext+1);
      return T_STRING; }


  /* These are some keywords that are recognized. */
".functor" { return K_FUNCTOR; }
".scope"   { return K_SCOPE; }
".thread"  { return K_THREAD; }
".var"     { return K_VAR; }


  /* instructions start with a % character. The compiler decides what
     kind of instruction this really is. The few exceptions (that have
     exceptional parameter requirements) are listed first. */

"%vpi_call" {
      return K_vpi_call; }

"%"[.$_/a-zA-Z0-9]+ {
      yylval.text = strdup(yytext);
      return T_INSTR; }

[0-9][0-9]* {
      yylval.numb = strtoul(yytext, 0, 0);
      return T_NUMBER; }

"0x"[0-9a-fA-F]+ {
      yylval.numb = strtoul(yytext, 0, 0);
      return T_NUMBER; }


  /* Symbols are pretty much what is left. They are used to refer to
     labels so the rule must match a string that a label would match. */
[.$_a-zA-Z][.$_a-zA-Z0-9]* {
      yylval.text = strdup(yytext);
      return T_SYMBOL; }


  /* Accept the common assembler style comments, treat them as white
     space. Of course, also skip white space. The semi-colon is
     special, though, in that it is also a statement terminator. */
";".* { return ';'; }
"#".* { ; }

[ \t\b] { ; }

\n { yyline += 1; }

. { return yytext[0]; }

%%

int yywrap()
{
      return -1;
}
