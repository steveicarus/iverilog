/* C code produced by gperf version 2.7 */
/* Command-line: gperf -o -i 1 -C -k 1-3,$ -L C -H keyword_hash -N check_identifier -tT lexor_keyword.gperf > lexor_keyword.cc */

#include "parse_misc.h"
#include "parse.h"
#include <string.h>


#define TOTAL_KEYWORDS 99
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 12
#define MIN_HASH_VALUE 7
#define MAX_HASH_VALUE 239
/* maximum key range = 233, duplicates = 0 */

#ifdef __GNUC__
__inline
#endif
static unsigned int
keyword_hash (const char *str, int len)
{
  static const unsigned char asso_values[] =
    {
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 126,  66,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240,  31,  11,  81,
        1,   1,  81,  26,  11,  51,  11,  21,  81,  81,
        1,  46,  16, 240,   1,   1,   6,  11,  36,  46,
       21,  16,   6, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
      240, 240, 240, 240, 240, 240
    };
  int hval = len;

  hval += asso_values[(unsigned char)str[len - 1]];
  hval += asso_values[(unsigned char)str[0]];
  hval += asso_values[(unsigned char)str[1]];

  if (len > 2)
    hval += asso_values[(unsigned char)str[2]];

  return hval ;
}

int
check_identifier (const char *str, int len)
{
  static const struct { const char *name; int tokenType; } wordlist[] =
    {
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {"end",		K_end},
      {""}, {""}, {""},
      {"endcase",	K_endcase},
      {"endtable",	K_endtable},
      {"endmodule",	K_endmodule},
      {"rtran",		K_rtran},
      {"endfunction",	K_endfunction},
      {"endprimitive",	K_endprimitive},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
      {"endspecify",	K_endspecify},
      {"repeat",		K_repeat},
      {"endtask",	K_endtask},
      {""},
      {"edge",		K_edge},
      {""}, {""},
      {"negedge",	K_negedge},
      {"and",		K_and},
      {"nand",		K_nand},
      {""},
      {"assign",		K_assign},
      {"specify",	K_specify},
      {"deassign",	K_deassign},
      {"tran",		K_tran},
      {"begin",		K_begin},
      {""}, {""}, {""}, {""},
      {"event",		K_event},
      {"or",		K_or},
      {""},
      {"nor",		K_nor},
      {""},
      {"table",		K_table},
      {""}, {""},
      {"reg",		K_reg},
      {"parameter",	K_parameter},
      {""}, {""},
      {"disable",	K_disable},
      {"not",		K_not},
      {"task",		K_task},
      {"trior",		K_trior},
      {"triand",		K_triand},
      {"integer",	K_integer},
      {""}, {""}, {""}, {""},
      {"posedge",	K_posedge},
      {"xor",		K_xor},
      {"xnor",		K_xnor},
      {""},
      {"output",		K_output},
      {""}, {""},
      {"primitive",	K_primitive},
      {"input",		K_input},
      {""},
      {"strong1",	K_strong1},
      {"rtranif1",	K_rtranif1},
      {"wand",		K_wand},
      {""}, {""}, {""}, {""},
      {"else",		K_else},
      {"rnmos",		K_rnmos},
      {"trireg",		K_trireg},
      {"release",	K_release},
      {""}, {""}, {""}, {""},
      {"default",	K_default},
      {"wor",		K_wor},
      {""}, {""}, {""},
      {"supply1",	K_supply1},
      {"function",	K_function},
      {"wire",		K_wire},
      {"rpmos",		K_rpmos},
      {""}, {""}, {""},
      {"specparam",	K_specparam},
      {"inout",		K_inout},
      {""},
      {"tranif1",	K_tranif1},
      {"tri",		K_tri},
      {"join",		K_join},
      {"while",		K_while},
      {""}, {""},
      {"pulldown",	K_pulldown},
      {"case",		K_case},
      {"large",		K_large},
      {""}, {""},
      {"scalered",	K_scalered},
      {""},
      {"casez",		K_casez},
      {"notif1",		K_notif1},
      {""},
      {"vectored",	K_vectored},
      {"tri1",		K_tri1},
      {""},
      {"pullup",		K_pullup},
      {""},
      {"for",		K_for},
      {"nmos",		K_nmos},
      {"force",		K_force},
      {"module",		K_module},
      {"forever",	K_forever},
      {""},
      {"wait",		K_wait},
      {"casex",		K_casex},
      {""},
      {"strong0",	K_strong0},
      {"rtranif0",	K_rtranif0},
      {"time",		K_time},
      {""}, {""}, {""}, {""},
      {"pmos",		K_pmos},
      {"weak1",		K_weak1},
      {""}, {""}, {""},
      {"fork",		K_fork},
      {""}, {""}, {""}, {""}, {""}, {""},
      {"highz1",		K_highz1},
      {"supply0",	K_supply0},
      {""}, {""}, {""},
      {"always",		K_always},
      {""}, {""}, {""},
      {"rcmos",		K_rcmos},
      {"medium",		K_medium},
      {"tranif0",	K_tranif0},
      {"defparam",	K_defparam},
      {""}, {""},
      {"bufif1",		K_bufif1},
      {""}, {""}, {""},
      {"pull1",		K_pull1},
      {""}, {""}, {""}, {""}, {""},
      {"notif0",		K_notif0},
      {""},
      {"buf",		K_buf},
      {"tri0",		K_tri0},
      {""}, {""},
      {"initial",	K_initial},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {"small",		K_small},
      {""}, {""}, {""}, {""}, {""},
      {"macromodule",	K_macromodule},
      {""}, {""}, {""},
      {"weak0",		K_weak0},
      {""}, {""}, {""},
      {"cmos",		K_cmos},
      {""},
      {"if",		K_if},
      {""}, {""}, {""}, {""},
      {"highz0",		K_highz0},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""},
      {"bufif0",		K_bufif0},
      {""}, {""}, {""},
      {"pull0",		K_pull0}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      int key = keyword_hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return wordlist[key].tokenType;
        }
    }
  return IDENTIFIER;
}