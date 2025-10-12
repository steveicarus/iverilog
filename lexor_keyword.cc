/* C++ code produced by gperf version 3.1 */
/* Command-line: gperf -o -i 7 -C -k '1-4,6,9,$' -H keyword_hash -N check_identifier -t ./lexor_keyword.gperf  */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 9 "./lexor_keyword.gperf"

/* Command-line: gperf -o -i 7 -C -k '1-4,6,9,$' -H keyword_hash -N check_identifier -t ./lexor_keyword.gperf  */

#include <cstdarg>
#include "config.h"
#include "parse_misc.h"
#include "parse.h"
#include <cstring>
#include "lexor_keyword.h"
#include "compiler.h"

#line 21 "./lexor_keyword.gperf"
struct lexor_keyword { const char*name; int mask; int tokenType; };

#define TOTAL_KEYWORDS 333
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 41
#define MAX_HASH_VALUE 1546
/* maximum key range = 1506, duplicates = 0 */

class Lkwd
{
private:
  static inline unsigned int keyword_hash (const char *str, size_t len);
public:
  static const struct lexor_keyword *check_identifier (const char *str, size_t len);
};

inline unsigned int
Lkwd::keyword_hash (const char *str, size_t len)
{
  static const unsigned short asso_values[] =
    {
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,   92,  247,
        12, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547,  307, 1547,   62,  292,   97,
        12,    7,  202,   57,  427,   37,   37,  202,  102,  217,
         7,  202,  147,  122,   82,    7,   12,  377,  437,  372,
       425,  130,  212, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547, 1547,
      1547, 1547, 1547, 1547, 1547, 1547
    };
  unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[static_cast<unsigned char>(str[8])];
      /*FALLTHROUGH*/
      case 8:
      case 7:
      case 6:
        hval += asso_values[static_cast<unsigned char>(str[5])];
      /*FALLTHROUGH*/
      case 5:
      case 4:
        hval += asso_values[static_cast<unsigned char>(str[3])];
      /*FALLTHROUGH*/
      case 3:
        hval += asso_values[static_cast<unsigned char>(str[2])];
      /*FALLTHROUGH*/
      case 2:
        hval += asso_values[static_cast<unsigned char>(str[1])];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[static_cast<unsigned char>(str[0])];
        break;
    }
  return hval + asso_values[static_cast<unsigned char>(str[len - 1])];
}

const struct lexor_keyword *
Lkwd::check_identifier (const char *str, size_t len)
{
  static const struct lexor_keyword wordlist[] =
    {
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0},
#line 105 "./lexor_keyword.gperf"
      {"end",			GN_KEYWORDS_1364_1995,		K_end},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 89 "./lexor_keyword.gperf"
      {"ddt",			GN_KEYWORDS_VAMS_2_3,		K_ddt},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 279 "./lexor_keyword.gperf"
      {"sin",			GN_KEYWORDS_VAMS_2_3,		K_sin},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0},
#line 118 "./lexor_keyword.gperf"
      {"endnature",		GN_KEYWORDS_VAMS_2_3,		K_endnature},
      {"",0,0}, {"",0,0},
#line 178 "./lexor_keyword.gperf"
      {"int",			GN_KEYWORDS_1800_2005,		K_int},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 158 "./lexor_keyword.gperf"
      {"idt",			GN_KEYWORDS_VAMS_2_3,		K_idt},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0},
#line 95 "./lexor_keyword.gperf"
      {"design",			GN_KEYWORDS_1364_2001_CONFIG,	K_design},
#line 99 "./lexor_keyword.gperf"
      {"dist",			GN_KEYWORDS_1800_2005,		K_dist},
#line 177 "./lexor_keyword.gperf"
      {"instance",		GN_KEYWORDS_1364_2001_CONFIG,	K_instance},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 303 "./lexor_keyword.gperf"
      {"tan",			GN_KEYWORDS_VAMS_2_3,		K_tan},
      {"",0,0}, {"",0,0},
#line 103 "./lexor_keyword.gperf"
      {"edge",			GN_KEYWORDS_1364_1995,		K_edge},
      {"",0,0},
#line 40 "./lexor_keyword.gperf"
      {"and",			GN_KEYWORDS_1364_1995,		K_and},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0},
#line 182 "./lexor_keyword.gperf"
      {"intersect",		GN_KEYWORDS_1800_2005,		K_intersect},
#line 211 "./lexor_keyword.gperf"
      {"nand",			GN_KEYWORDS_1364_1995,		K_nand},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 176 "./lexor_keyword.gperf"
      {"inside",			GN_KEYWORDS_1800_2005,		K_inside},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 44 "./lexor_keyword.gperf"
      {"assert",	GN_KEYWORDS_1800_2005|GN_KEYWORDS_VAMS_2_3,	K_assert},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 196 "./lexor_keyword.gperf"
      {"ln",			GN_KEYWORDS_VAMS_2_3,		K_ln},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 41 "./lexor_keyword.gperf"
      {"asin",			GN_KEYWORDS_VAMS_2_3,		K_asin},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 45 "./lexor_keyword.gperf"
      {"assign",			GN_KEYWORDS_1364_1995,		K_assign},
#line 104 "./lexor_keyword.gperf"
      {"else",			GN_KEYWORDS_1364_1995,		K_else},
      {"",0,0},
#line 192 "./lexor_keyword.gperf"
      {"let",			GN_KEYWORDS_1800_2009,		K_let},
      {"",0,0},
#line 278 "./lexor_keyword.gperf"
      {"signed",			GN_KEYWORDS_1364_2001,		K_signed},
      {"",0,0},
#line 92 "./lexor_keyword.gperf"
      {"deassign",		GN_KEYWORDS_1364_1995,		K_deassign},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 106 "./lexor_keyword.gperf"
      {"endcase",		GN_KEYWORDS_1364_1995,		K_endcase},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 213 "./lexor_keyword.gperf"
      {"negedge",		GN_KEYWORDS_1364_1995,		K_negedge},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 47 "./lexor_keyword.gperf"
      {"atan",			GN_KEYWORDS_VAMS_2_3,		K_atan},
#line 151 "./lexor_keyword.gperf"
      {"generate",		GN_KEYWORDS_1364_2001,		K_generate},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 179 "./lexor_keyword.gperf"
      {"integer",		GN_KEYWORDS_1364_1995,		K_integer},
#line 48 "./lexor_keyword.gperf"
      {"atan2",			GN_KEYWORDS_VAMS_2_3,		K_atan2},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 261 "./lexor_keyword.gperf"
      {"restrict",		GN_KEYWORDS_1800_2009,		K_restrict},
      {"",0,0}, {"",0,0},
#line 257 "./lexor_keyword.gperf"
      {"reject_on",		GN_KEYWORDS_1800_2009,		K_reject_on},
      {"",0,0},
#line 114 "./lexor_keyword.gperf"
      {"endgenerate",		GN_KEYWORDS_1364_2001,		K_endgenerate},
#line 316 "./lexor_keyword.gperf"
      {"tri",			GN_KEYWORDS_1364_1995,		K_tri},
      {"",0,0}, {"",0,0},
#line 312 "./lexor_keyword.gperf"
      {"tran",			GN_KEYWORDS_1364_1995,		K_tran},
#line 98 "./lexor_keyword.gperf"
      {"discrete",		GN_KEYWORDS_VAMS_2_3,		K_discrete},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 248 "./lexor_keyword.gperf"
      {"rand",			GN_KEYWORDS_1800_2005,		K_rand},
#line 125 "./lexor_keyword.gperf"
      {"endsequence",		GN_KEYWORDS_1800_2005,		K_endsequence},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 64 "./lexor_keyword.gperf"
      {"case",			GN_KEYWORDS_1364_1995,		K_case},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0},
#line 180 "./lexor_keyword.gperf"
      {"interconnect",		GN_KEYWORDS_1800_2012,		K_interconnect},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 251 "./lexor_keyword.gperf"
      {"randsequence",		GN_KEYWORDS_1800_2005,		K_randsequence},
      {"",0,0}, {"",0,0},
#line 215 "./lexor_keyword.gperf"
      {"nettype",		GN_KEYWORDS_1800_2012,		K_nettype},
#line 109 "./lexor_keyword.gperf"
      {"endclass",		GN_KEYWORDS_1800_2005,		K_endclass},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 256 "./lexor_keyword.gperf"
      {"reg",			GN_KEYWORDS_1364_1995,		K_reg},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 112 "./lexor_keyword.gperf"
      {"enddiscipline",		GN_KEYWORDS_VAMS_2_3,		K_enddiscipline},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 302 "./lexor_keyword.gperf"
      {"tagged",			GN_KEYWORDS_1800_2005,		K_tagged},
#line 258 "./lexor_keyword.gperf"
      {"release",		GN_KEYWORDS_1364_1995,		K_release},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 319 "./lexor_keyword.gperf"
      {"triand",			GN_KEYWORDS_1364_1995,		K_triand},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 107 "./lexor_keyword.gperf"
      {"endchecker",		GN_KEYWORDS_1800_2009,		K_endchecker},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 96 "./lexor_keyword.gperf"
      {"disable",		GN_KEYWORDS_1364_1995,		K_disable},
      {"",0,0},
#line 222 "./lexor_keyword.gperf"
      {"not",			GN_KEYWORDS_1364_1995,		K_not},
      {"",0,0}, {"",0,0},
#line 288 "./lexor_keyword.gperf"
      {"sqrt",			GN_KEYWORDS_VAMS_2_3,		K_sqrt},
#line 250 "./lexor_keyword.gperf"
      {"randcase",		GN_KEYWORDS_1800_2005,		K_randcase},
#line 121 "./lexor_keyword.gperf"
      {"endprimitive",		GN_KEYWORDS_1364_1995,		K_endprimitive},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0},
#line 111 "./lexor_keyword.gperf"
      {"endconnectrules",	GN_KEYWORDS_VAMS_2_3,		K_endconnectrules},
#line 265 "./lexor_keyword.gperf"
      {"rtran",			GN_KEYWORDS_1364_1995,		K_rtran},
      {"",0,0}, {"",0,0},
#line 108 "./lexor_keyword.gperf"
      {"endconfig",		GN_KEYWORDS_1364_2001_CONFIG,	K_endconfig},
#line 127 "./lexor_keyword.gperf"
      {"endtask",		GN_KEYWORDS_1364_1995,		K_endtask},
#line 39 "./lexor_keyword.gperf"
      {"analysis",		GN_KEYWORDS_VAMS_2_3,		K_analysis},
      {"",0,0}, {"",0,0},
#line 291 "./lexor_keyword.gperf"
      {"string",	GN_KEYWORDS_1800_2005|GN_KEYWORDS_VAMS_2_3,	K_string},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 172 "./lexor_keyword.gperf"
      {"initial",		GN_KEYWORDS_1364_1995,		K_initial},
#line 249 "./lexor_keyword.gperf"
      {"randc",			GN_KEYWORDS_1800_2005,		K_randc},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 170 "./lexor_keyword.gperf"
      {"include",		GN_KEYWORDS_1364_2001_CONFIG,	K_include},
      {"",0,0},
#line 208 "./lexor_keyword.gperf"
      {"min",			GN_KEYWORDS_VAMS_2_3,		K_min},
      {"",0,0},
#line 259 "./lexor_keyword.gperf"
      {"repeat",			GN_KEYWORDS_1364_1995,		K_repeat},
      {"",0,0},
#line 32 "./lexor_keyword.gperf"
      {"alias",			GN_KEYWORDS_1800_2005,		K_alias},
      {"",0,0}, {"",0,0},
#line 117 "./lexor_keyword.gperf"
      {"endmodule",		GN_KEYWORDS_1364_1995,		K_endmodule},
      {"",0,0},
#line 71 "./lexor_keyword.gperf"
      {"class",			GN_KEYWORDS_1800_2005,		K_class},
      {"",0,0}, {"",0,0},
#line 28 "./lexor_keyword.gperf"
      {"access",			GN_KEYWORDS_VAMS_2_3,		K_access},
#line 308 "./lexor_keyword.gperf"
      {"time",			GN_KEYWORDS_1364_1995,		K_time},
#line 120 "./lexor_keyword.gperf"
      {"endparamset",		GN_KEYWORDS_VAMS_2_3,		K_endparamset},
      {"",0,0}, {"",0,0},
#line 181 "./lexor_keyword.gperf"
      {"interface",		GN_KEYWORDS_1800_2005,		K_interface},
      {"",0,0},
#line 113 "./lexor_keyword.gperf"
      {"endfunction",		GN_KEYWORDS_1364_1995,		K_endfunction},
      {"",0,0}, {"",0,0},
#line 289 "./lexor_keyword.gperf"
      {"static",			GN_KEYWORDS_1800_2005,		K_static},
#line 183 "./lexor_keyword.gperf"
      {"join",			GN_KEYWORDS_1364_1995,		K_join},
      {"",0,0},
#line 116 "./lexor_keyword.gperf"
      {"endinterface",		GN_KEYWORDS_1800_2005,		K_endinterface},
      {"",0,0},
#line 27 "./lexor_keyword.gperf"
      {"accept_on",		GN_KEYWORDS_1800_2009,		K_accept_on},
      {"",0,0},
#line 311 "./lexor_keyword.gperf"
      {"timeunit",		GN_KEYWORDS_1800_2005,		K_timeunit},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 254 "./lexor_keyword.gperf"
      {"realtime",		GN_KEYWORDS_1364_1995,		K_realtime},
      {"",0,0},
#line 322 "./lexor_keyword.gperf"
      {"type",			GN_KEYWORDS_1800_2005,		K_type},
#line 159 "./lexor_keyword.gperf"
      {"idtmod",			GN_KEYWORDS_VAMS_2_3,		K_idtmod},
      {"",0,0},
#line 287 "./lexor_keyword.gperf"
      {"split",			GN_KEYWORDS_VAMS_2_3,		K_split},
      {"",0,0}, {"",0,0},
#line 185 "./lexor_keyword.gperf"
      {"join_none",		GN_KEYWORDS_1800_2005,		K_join_none},
      {"",0,0},
#line 190 "./lexor_keyword.gperf"
      {"large",			GN_KEYWORDS_1364_1995,		K_large},
#line 83 "./lexor_keyword.gperf"
      {"cos",			GN_KEYWORDS_VAMS_2_3,		K_cos},
      {"",0,0}, {"",0,0},
#line 317 "./lexor_keyword.gperf"
      {"tri0",			GN_KEYWORDS_1364_1995,		K_tri0},
      {"",0,0},
#line 173 "./lexor_keyword.gperf"
      {"initial_step",		GN_KEYWORDS_VAMS_2_3,		K_initial_step},
      {"",0,0},
#line 169 "./lexor_keyword.gperf"
      {"incdir",			GN_KEYWORDS_1364_2001_CONFIG,	K_incdir},
#line 97 "./lexor_keyword.gperf"
      {"discipline",		GN_KEYWORDS_VAMS_2_3,		K_discipline},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 78 "./lexor_keyword.gperf"
      {"const",			GN_KEYWORDS_1800_2005,		K_const},
#line 299 "./lexor_keyword.gperf"
      {"sync_accept_on",		GN_KEYWORDS_1800_2009,		K_sync_accept_on},
      {"",0,0},
#line 321 "./lexor_keyword.gperf"
      {"trireg",			GN_KEYWORDS_1364_1995,		K_trireg},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0},
#line 81 "./lexor_keyword.gperf"
      {"continue",		GN_KEYWORDS_1800_2005,		K_continue},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 119 "./lexor_keyword.gperf"
      {"endpackage",		GN_KEYWORDS_1800_2005,		K_endpackage},
#line 126 "./lexor_keyword.gperf"
      {"endtable",		GN_KEYWORDS_1364_1995,		K_endtable},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 67 "./lexor_keyword.gperf"
      {"ceil",			GN_KEYWORDS_VAMS_2_3,		K_ceil},
      {"",0,0},
#line 300 "./lexor_keyword.gperf"
      {"sync_reject_on",		GN_KEYWORDS_1800_2009,		K_sync_reject_on},
      {"",0,0},
#line 38 "./lexor_keyword.gperf"
      {"analog",			GN_KEYWORDS_VAMS_2_3,		K_analog},
#line 54 "./lexor_keyword.gperf"
      {"bins",			GN_KEYWORDS_1800_2005,		K_bins},
      {"",0,0},
#line 56 "./lexor_keyword.gperf"
      {"bit",			GN_KEYWORDS_1800_2005,		K_bit},
#line 191 "./lexor_keyword.gperf"
      {"last_crossing",		GN_KEYWORDS_VAMS_2_3,		K_last_crossing},
      {"",0,0},
#line 253 "./lexor_keyword.gperf"
      {"real",			GN_KEYWORDS_1364_1995,		K_real},
#line 310 "./lexor_keyword.gperf"
      {"timer",			GN_KEYWORDS_VAMS_2_3,		K_timer},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 53 "./lexor_keyword.gperf"
      {"bind",			GN_KEYWORDS_1800_2005,		K_bind},
#line 164 "./lexor_keyword.gperf"
      {"ignore_bins",		GN_KEYWORDS_1800_2005,		K_ignore_bins},
      {"",0,0}, {"",0,0},
#line 226 "./lexor_keyword.gperf"
      {"or",			GN_KEYWORDS_1364_1995,		K_or},
      {"",0,0},
#line 273 "./lexor_keyword.gperf"
      {"scalared",		GN_KEYWORDS_1364_1995,		K_scalared},
#line 24 "./lexor_keyword.gperf"
      {"abs",			GN_KEYWORDS_VAMS_2_3,		K_abs},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 266 "./lexor_keyword.gperf"
      {"rtranif0",		GN_KEYWORDS_1364_1995,		K_rtranif0},
#line 220 "./lexor_keyword.gperf"
      {"nor",			GN_KEYWORDS_1364_1995,		K_nor},
      {"",0,0}, {"",0,0},
#line 29 "./lexor_keyword.gperf"
      {"acos",			GN_KEYWORDS_VAMS_2_3,		K_acos},
#line 231 "./lexor_keyword.gperf"
      {"paramset",		GN_KEYWORDS_VAMS_2_3,		K_paramset},
      {"",0,0},
#line 124 "./lexor_keyword.gperf"
      {"endspecify",		GN_KEYWORDS_1364_1995,		K_endspecify},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0},
#line 66 "./lexor_keyword.gperf"
      {"casez",			GN_KEYWORDS_1364_1995,		K_casez},
      {"",0,0}, {"",0,0},
#line 207 "./lexor_keyword.gperf"
      {"merged",			GN_KEYWORDS_VAMS_2_3,		K_merged},
#line 202 "./lexor_keyword.gperf"
      {"longint",		GN_KEYWORDS_1800_2005,		K_longint},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 88 "./lexor_keyword.gperf"
      {"cross",			GN_KEYWORDS_1800_2005,		K_cross},
#line 333 "./lexor_keyword.gperf"
      {"use",			GN_KEYWORDS_1364_2001_CONFIG,	K_use},
      {"",0,0}, {"",0,0},
#line 93 "./lexor_keyword.gperf"
      {"default",		GN_KEYWORDS_1364_1995,		K_default},
#line 52 "./lexor_keyword.gperf"
      {"begin",			GN_KEYWORDS_1364_1995,		K_begin},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 204 "./lexor_keyword.gperf"
      {"matches",		GN_KEYWORDS_1800_2005,		K_matches},
      {"",0,0}, {"",0,0},
#line 309 "./lexor_keyword.gperf"
      {"timeprecision",		GN_KEYWORDS_1800_2005,		K_timeprecision},
      {"",0,0},
#line 68 "./lexor_keyword.gperf"
      {"cell",			GN_KEYWORDS_1364_2001_CONFIG,	K_cell},
#line 137 "./lexor_keyword.gperf"
      {"final",			GN_KEYWORDS_1800_2005,		K_final},
      {"",0,0}, {"",0,0},
#line 100 "./lexor_keyword.gperf"
      {"do",			GN_KEYWORDS_1800_2005,		K_do},
#line 315 "./lexor_keyword.gperf"
      {"transition",		GN_KEYWORDS_VAMS_2_3,		K_transition},
#line 320 "./lexor_keyword.gperf"
      {"trior",			GN_KEYWORDS_1364_1995,		K_trior},
#line 199 "./lexor_keyword.gperf"
      {"log",			GN_KEYWORDS_VAMS_2_3,		K_log},
      {"",0,0},
#line 292 "./lexor_keyword.gperf"
      {"strong",			GN_KEYWORDS_1800_2009,		K_strong},
#line 79 "./lexor_keyword.gperf"
      {"constraint",		GN_KEYWORDS_1800_2005,		K_constraint},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 75 "./lexor_keyword.gperf"
      {"connect",		GN_KEYWORDS_VAMS_2_3,		K_connect},
#line 110 "./lexor_keyword.gperf"
      {"endclocking",		GN_KEYWORDS_1800_2005,		K_endclocking},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 233 "./lexor_keyword.gperf"
      {"posedge",		GN_KEYWORDS_1364_1995,		K_posedge},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 283 "./lexor_keyword.gperf"
      {"soft",			GN_KEYWORDS_1800_2012,		K_soft},
#line 115 "./lexor_keyword.gperf"
      {"endgroup",		GN_KEYWORDS_1800_2005,		K_endgroup},
      {"",0,0}, {"",0,0},
#line 161 "./lexor_keyword.gperf"
      {"if",			GN_KEYWORDS_1364_1995,		K_if},
#line 218 "./lexor_keyword.gperf"
      {"nmos",			GN_KEYWORDS_1364_1995,		K_nmos},
#line 327 "./lexor_keyword.gperf"
      {"units",			GN_KEYWORDS_VAMS_2_3,		K_units},
      {"",0,0}, {"",0,0},
#line 223 "./lexor_keyword.gperf"
      {"notif0",			GN_KEYWORDS_1364_1995,		K_notif0},
      {"",0,0}, {"",0,0},
#line 171 "./lexor_keyword.gperf"
      {"inf",			GN_KEYWORDS_VAMS_2_3,		K_inf},
#line 63 "./lexor_keyword.gperf"
      {"byte",			GN_KEYWORDS_1800_2005,		K_byte},
      {"",0,0}, {"",0,0},
#line 329 "./lexor_keyword.gperf"
      {"unsigned",		GN_KEYWORDS_1364_2001,		K_unsigned},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 293 "./lexor_keyword.gperf"
      {"strong0",		GN_KEYWORDS_1364_1995,		K_strong0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 214 "./lexor_keyword.gperf"
      {"net_resolution",		GN_KEYWORDS_VAMS_2_3,		K_net_resolution},
#line 313 "./lexor_keyword.gperf"
      {"tranif0",		GN_KEYWORDS_1364_1995,		K_tranif0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 163 "./lexor_keyword.gperf"
      {"ifnone",			GN_KEYWORDS_1364_1995,		K_ifnone},
#line 341 "./lexor_keyword.gperf"
      {"wand",			GN_KEYWORDS_1364_1995,		K_wand},
      {"",0,0},
#line 136 "./lexor_keyword.gperf"
      {"extern",			GN_KEYWORDS_1800_2005,		K_extern},
#line 140 "./lexor_keyword.gperf"
      {"flicker_noise",		GN_KEYWORDS_VAMS_2_3,		K_flicker_noise},
#line 46 "./lexor_keyword.gperf"
      {"assume",			GN_KEYWORDS_1800_2005,		K_assume},
      {"",0,0},
#line 129 "./lexor_keyword.gperf"
      {"event",			GN_KEYWORDS_1364_1995,		K_event},
      {"",0,0},
#line 135 "./lexor_keyword.gperf"
      {"extends",		GN_KEYWORDS_1800_2005,		K_extends},
#line 212 "./lexor_keyword.gperf"
      {"nature",			GN_KEYWORDS_VAMS_2_3,		K_nature},
      {"",0,0},
#line 301 "./lexor_keyword.gperf"
      {"table",			GN_KEYWORDS_1364_1995,		K_table},
      {"",0,0}, {"",0,0},
#line 184 "./lexor_keyword.gperf"
      {"join_any",		GN_KEYWORDS_1800_2005,		K_join_any},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 305 "./lexor_keyword.gperf"
      {"task",			GN_KEYWORDS_1364_1995,		K_task},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 306 "./lexor_keyword.gperf"
      {"this",			GN_KEYWORDS_1800_2005,		K_this},
#line 282 "./lexor_keyword.gperf"
      {"small",			GN_KEYWORDS_1364_1995,		K_small},
#line 255 "./lexor_keyword.gperf"
      {"ref",			GN_KEYWORDS_1800_2005,		K_ref},
      {"",0,0},
#line 262 "./lexor_keyword.gperf"
      {"return",			GN_KEYWORDS_1800_2005,		K_return},
#line 339 "./lexor_keyword.gperf"
      {"wait",			GN_KEYWORDS_1364_1995,		K_wait},
#line 201 "./lexor_keyword.gperf"
      {"logic",	GN_KEYWORDS_1800_2005|GN_KEYWORDS_ICARUS,	K_logic},
      {"",0,0}, {"",0,0},
#line 217 "./lexor_keyword.gperf"
      {"nexttime",		GN_KEYWORDS_1800_2009,		K_nexttime},
#line 90 "./lexor_keyword.gperf"
      {"ddt_nature",		GN_KEYWORDS_VAMS_2_3,		K_ddt_nature},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 295 "./lexor_keyword.gperf"
      {"struct",			GN_KEYWORDS_1800_2005,		K_struct},
#line 348 "./lexor_keyword.gperf"
      {"wire",			GN_KEYWORDS_1364_1995,		K_wire},
      {"",0,0}, {"",0,0},
#line 323 "./lexor_keyword.gperf"
      {"typedef",		GN_KEYWORDS_1800_2005,		K_typedef},
#line 101 "./lexor_keyword.gperf"
      {"domain",			GN_KEYWORDS_VAMS_2_3,		K_domain},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 236 "./lexor_keyword.gperf"
      {"primitive",		GN_KEYWORDS_1364_1995,		K_primitive},
      {"",0,0},
#line 263 "./lexor_keyword.gperf"
      {"rnmos",			GN_KEYWORDS_1364_1995,		K_rnmos},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 166 "./lexor_keyword.gperf"
      {"implies",		GN_KEYWORDS_1800_2009,		K_implies},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 160 "./lexor_keyword.gperf"
      {"idt_nature",		GN_KEYWORDS_VAMS_2_3,		K_idt_nature},
#line 267 "./lexor_keyword.gperf"
      {"rtranif1",		GN_KEYWORDS_1364_1995,		K_rtranif1},
      {"",0,0}, {"",0,0},
#line 230 "./lexor_keyword.gperf"
      {"parameter",		GN_KEYWORDS_1364_1995,		K_parameter},
#line 73 "./lexor_keyword.gperf"
      {"cmos",			GN_KEYWORDS_1364_1995,		K_cmos},
#line 274 "./lexor_keyword.gperf"
      {"sequence",		GN_KEYWORDS_1800_2005,		K_sequence},
      {"",0,0}, {"",0,0},
#line 229 "./lexor_keyword.gperf"
      {"packed",			GN_KEYWORDS_1800_2005,		K_packed},
#line 186 "./lexor_keyword.gperf"
      {"laplace_nd",		GN_KEYWORDS_VAMS_2_3,		K_laplace_nd},
#line 330 "./lexor_keyword.gperf"
      {"until",			GN_KEYWORDS_1800_2009,		K_until},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 42 "./lexor_keyword.gperf"
      {"asinh",			GN_KEYWORDS_VAMS_2_3,		K_asinh},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0},
#line 332 "./lexor_keyword.gperf"
      {"untyped",		GN_KEYWORDS_1800_2009,		K_untyped},
      {"",0,0}, {"",0,0},
#line 268 "./lexor_keyword.gperf"
      {"s_always",		GN_KEYWORDS_1800_2009,		K_s_always},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 193 "./lexor_keyword.gperf"
      {"liblist",		GN_KEYWORDS_1364_2001_CONFIG,	K_liblist},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 325 "./lexor_keyword.gperf"
      {"unique",			GN_KEYWORDS_1800_2005,		K_unique},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0},
#line 197 "./lexor_keyword.gperf"
      {"local",			GN_KEYWORDS_1800_2005,		K_local},
#line 143 "./lexor_keyword.gperf"
      {"for",			GN_KEYWORDS_1364_1995,		K_for},
      {"",0,0},
#line 240 "./lexor_keyword.gperf"
      {"protected",		GN_KEYWORDS_1800_2005,		K_protected},
#line 270 "./lexor_keyword.gperf"
      {"s_nexttime",		GN_KEYWORDS_1800_2009,		K_s_nexttime},
#line 49 "./lexor_keyword.gperf"
      {"atanh",			GN_KEYWORDS_VAMS_2_3,		K_atanh},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 228 "./lexor_keyword.gperf"
      {"package",		GN_KEYWORDS_1800_2005,		K_package},
#line 358 "./lexor_keyword.gperf"
      {"zi_nd",			GN_KEYWORDS_VAMS_2_3,		K_zi_nd},
      {"",0,0}, {"",0,0},
#line 26 "./lexor_keyword.gperf"
      {"abstol",			GN_KEYWORDS_VAMS_2_3,		K_abstol},
#line 232 "./lexor_keyword.gperf"
      {"pmos",			GN_KEYWORDS_1364_1995,		K_pmos},
#line 175 "./lexor_keyword.gperf"
      {"input",			GN_KEYWORDS_1364_1995,		K_input},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 146 "./lexor_keyword.gperf"
      {"forever",		GN_KEYWORDS_1364_1995,		K_forever},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 234 "./lexor_keyword.gperf"
      {"potential",		GN_KEYWORDS_VAMS_2_3,		K_potential},
      {"",0,0},
#line 145 "./lexor_keyword.gperf"
      {"force",			GN_KEYWORDS_1364_1995,		K_force},
      {"",0,0},
#line 285 "./lexor_keyword.gperf"
      {"specify",		GN_KEYWORDS_1364_1995,		K_specify},
#line 123 "./lexor_keyword.gperf"
      {"endproperty",		GN_KEYWORDS_1800_2005,		K_endproperty},
#line 352 "./lexor_keyword.gperf"
      {"wone",			GN_KEYWORDS_1364_2005,		K_wone},
#line 72 "./lexor_keyword.gperf"
      {"clocking",		GN_KEYWORDS_1800_2005,		K_clocking},
      {"",0,0}, {"",0,0},
#line 65 "./lexor_keyword.gperf"
      {"casex",			GN_KEYWORDS_1364_1995,		K_casex},
      {"",0,0},
#line 347 "./lexor_keyword.gperf"
      {"wildcard",		GN_KEYWORDS_1800_2005,		K_wildcard},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 252 "./lexor_keyword.gperf"
      {"rcmos",			GN_KEYWORDS_1364_1995,		K_rcmos},
      {"",0,0}, {"",0,0},
#line 25 "./lexor_keyword.gperf"
      {"absdelay",		GN_KEYWORDS_VAMS_2_3,		K_absdelay},
#line 294 "./lexor_keyword.gperf"
      {"strong1",		GN_KEYWORDS_1364_1995,		K_strong1},
      {"",0,0},
#line 133 "./lexor_keyword.gperf"
      {"expect",			GN_KEYWORDS_1800_2005,		K_expect},
      {"",0,0},
#line 34 "./lexor_keyword.gperf"
      {"always",			GN_KEYWORDS_1364_1995,		K_always},
#line 314 "./lexor_keyword.gperf"
      {"tranif1",		GN_KEYWORDS_1364_1995,		K_tranif1},
      {"",0,0},
#line 165 "./lexor_keyword.gperf"
      {"illegal_bins",		GN_KEYWORDS_1800_2005,		K_illegal_bins},
      {"",0,0}, {"",0,0},
#line 247 "./lexor_keyword.gperf"
      {"pure",			GN_KEYWORDS_1800_2005,		K_pure},
#line 296 "./lexor_keyword.gperf"
      {"super",			GN_KEYWORDS_1800_2005,		K_super},
      {"",0,0}, {"",0,0},
#line 74 "./lexor_keyword.gperf"
      {"config",			GN_KEYWORDS_1364_2001_CONFIG,	K_config},
#line 318 "./lexor_keyword.gperf"
      {"tri1",			GN_KEYWORDS_1364_1995,		K_tri1},
#line 355 "./lexor_keyword.gperf"
      {"wreal",	GN_KEYWORDS_VAMS_2_3|GN_KEYWORDS_ICARUS,	K_wreal},
      {"",0,0},
#line 76 "./lexor_keyword.gperf"
      {"connectmodule",		GN_KEYWORDS_VAMS_2_3,		K_connectmodule},
#line 168 "./lexor_keyword.gperf"
      {"import",			GN_KEYWORDS_1800_2005,		K_import},
      {"",0,0},
#line 324 "./lexor_keyword.gperf"
      {"union",			GN_KEYWORDS_1800_2005,		K_union},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 174 "./lexor_keyword.gperf"
      {"inout",			GN_KEYWORDS_1364_1995,		K_inout},
      {"",0,0}, {"",0,0},
#line 237 "./lexor_keyword.gperf"
      {"priority",		GN_KEYWORDS_1800_2005,		K_priority},
      {"",0,0}, {"",0,0},
#line 162 "./lexor_keyword.gperf"
      {"iff",			GN_KEYWORDS_1800_2005,		K_iff},
      {"",0,0}, {"",0,0},
#line 326 "./lexor_keyword.gperf"
      {"unique0",		GN_KEYWORDS_1800_2009,		K_unique0},
#line 58 "./lexor_keyword.gperf"
      {"break",			GN_KEYWORDS_1800_2005,		K_break},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 336 "./lexor_keyword.gperf"
      {"vectored",		GN_KEYWORDS_1364_1995,		K_vectored},
      {"",0,0},
#line 131 "./lexor_keyword.gperf"
      {"exclude",		GN_KEYWORDS_VAMS_2_3,		K_exclude},
      {"",0,0}, {"",0,0},
#line 264 "./lexor_keyword.gperf"
      {"rpmos",			GN_KEYWORDS_1364_1995,		K_rpmos},
      {"",0,0}, {"",0,0},
#line 195 "./lexor_keyword.gperf"
      {"limexp",			GN_KEYWORDS_VAMS_2_3,		K_limexp},
#line 122 "./lexor_keyword.gperf"
      {"endprogram",		GN_KEYWORDS_1800_2005,		K_endprogram},
      {"",0,0},
#line 335 "./lexor_keyword.gperf"
      {"var",			GN_KEYWORDS_1800_2005,		K_var},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0},
#line 187 "./lexor_keyword.gperf"
      {"laplace_np",		GN_KEYWORDS_VAMS_2_3,		K_laplace_np},
#line 94 "./lexor_keyword.gperf"
      {"defparam",		GN_KEYWORDS_1364_1995,		K_defparam},
      {"",0,0}, {"",0,0},
#line 152 "./lexor_keyword.gperf"
      {"genvar",			GN_KEYWORDS_1364_2001,		K_genvar},
#line 209 "./lexor_keyword.gperf"
      {"modport",		GN_KEYWORDS_1800_2005,		K_modport},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 225 "./lexor_keyword.gperf"
      {"null",			GN_KEYWORDS_1800_2005,		K_null},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 33 "./lexor_keyword.gperf"
      {"aliasparam",		GN_KEYWORDS_VAMS_2_3,		K_aliasparam},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 338 "./lexor_keyword.gperf"
      {"void",			GN_KEYWORDS_1800_2005,		K_void},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 69 "./lexor_keyword.gperf"
      {"chandle",		GN_KEYWORDS_1800_2005,		K_chandle},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 206 "./lexor_keyword.gperf"
      {"medium",			GN_KEYWORDS_1364_1995,		K_medium},
      {"",0,0},
#line 359 "./lexor_keyword.gperf"
      {"zi_np",			GN_KEYWORDS_VAMS_2_3,		K_zi_np},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 82 "./lexor_keyword.gperf"
      {"continuous",		GN_KEYWORDS_VAMS_2_3,		K_continuous},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 51 "./lexor_keyword.gperf"
      {"before",			GN_KEYWORDS_1800_2005,		K_before},
#line 70 "./lexor_keyword.gperf"
      {"checker",		GN_KEYWORDS_1800_2009,		K_checker},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 132 "./lexor_keyword.gperf"
      {"exp",			GN_KEYWORDS_VAMS_2_3,		K_exp},
      {"",0,0}, {"",0,0},
#line 194 "./lexor_keyword.gperf"
      {"library",		GN_KEYWORDS_1364_2001_CONFIG,	K_library},
      {"",0,0},
#line 31 "./lexor_keyword.gperf"
      {"ac_stim",		GN_KEYWORDS_VAMS_2_3,		K_ac_stim},
#line 150 "./lexor_keyword.gperf"
      {"function",		GN_KEYWORDS_1364_1995,		K_function},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 337 "./lexor_keyword.gperf"
      {"virtual",		GN_KEYWORDS_1800_2005,		K_virtual},
#line 343 "./lexor_keyword.gperf"
      {"weak0",			GN_KEYWORDS_1364_1995,		K_weak0},
#line 353 "./lexor_keyword.gperf"
      {"wor",			GN_KEYWORDS_1364_1995,		K_wor},
      {"",0,0}, {"",0,0},
#line 188 "./lexor_keyword.gperf"
      {"laplace_zd",		GN_KEYWORDS_VAMS_2_3,		K_laplace_zd},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 154 "./lexor_keyword.gperf"
      {"ground",			GN_KEYWORDS_VAMS_2_3,		K_ground},
#line 167 "./lexor_keyword.gperf"
      {"implements",		GN_KEYWORDS_1800_2012,		K_implements},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 55 "./lexor_keyword.gperf"
      {"binsof",			GN_KEYWORDS_1800_2005,		K_binsof},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 224 "./lexor_keyword.gperf"
      {"notif1",			GN_KEYWORDS_1364_1995,		K_notif1},
      {"",0,0},
#line 284 "./lexor_keyword.gperf"
      {"solve",			GN_KEYWORDS_1800_2005,		K_solve},
#line 216 "./lexor_keyword.gperf"
      {"new",			GN_KEYWORDS_1800_2005,		K_new},
#line 80 "./lexor_keyword.gperf"
      {"context",		GN_KEYWORDS_1800_2005,		K_context},
#line 286 "./lexor_keyword.gperf"
      {"specparam",		GN_KEYWORDS_1364_1995,		K_specparam},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 227 "./lexor_keyword.gperf"
      {"output",			GN_KEYWORDS_1364_1995,		K_output},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 245 "./lexor_keyword.gperf"
      {"pulsestyle_onevent",	GN_KEYWORDS_1364_2001,		K_pulsestyle_onevent},
#line 246 "./lexor_keyword.gperf"
      {"pulsestyle_ondetect",	GN_KEYWORDS_1364_2001,		K_pulsestyle_ondetect},
#line 238 "./lexor_keyword.gperf"
      {"program",		GN_KEYWORDS_1800_2005,		K_program},
#line 275 "./lexor_keyword.gperf"
      {"shortint",		GN_KEYWORDS_1800_2005,		K_shortint},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 138 "./lexor_keyword.gperf"
      {"final_step",		GN_KEYWORDS_VAMS_2_3,		K_final_step},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 340 "./lexor_keyword.gperf"
      {"wait_order",		GN_KEYWORDS_1800_2005,		K_wait_order},
#line 360 "./lexor_keyword.gperf"
      {"zi_zd",			GN_KEYWORDS_VAMS_2_3,		K_zi_zd},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 357 "./lexor_keyword.gperf"
      {"xor",			GN_KEYWORDS_1364_1995,		K_xor},
#line 141 "./lexor_keyword.gperf"
      {"floor",			GN_KEYWORDS_VAMS_2_3,		K_floor},
      {"",0,0}, {"",0,0},
#line 239 "./lexor_keyword.gperf"
      {"property",		GN_KEYWORDS_1800_2005,		K_property},
      {"",0,0},
#line 30 "./lexor_keyword.gperf"
      {"acosh",			GN_KEYWORDS_VAMS_2_3,		K_acosh},
      {"",0,0},
#line 356 "./lexor_keyword.gperf"
      {"xnor",			GN_KEYWORDS_1364_1995,		K_xnor},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 77 "./lexor_keyword.gperf"
      {"connectrules",		GN_KEYWORDS_VAMS_2_3,		K_connectrules},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 134 "./lexor_keyword.gperf"
      {"export",			GN_KEYWORDS_1800_2005,		K_export},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 102 "./lexor_keyword.gperf"
      {"driver_update",		GN_KEYWORDS_VAMS_2_3,		K_driver_update},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0},
#line 241 "./lexor_keyword.gperf"
      {"pull0",			GN_KEYWORDS_1364_1995,		K_pull0},
      {"",0,0}, {"",0,0},
#line 210 "./lexor_keyword.gperf"
      {"module",			GN_KEYWORDS_1364_1995,		K_module},
#line 128 "./lexor_keyword.gperf"
      {"enum",			GN_KEYWORDS_1800_2005,		K_enum},
#line 85 "./lexor_keyword.gperf"
      {"cover",			GN_KEYWORDS_1800_2005,		K_cover},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 271 "./lexor_keyword.gperf"
      {"s_until",		GN_KEYWORDS_1800_2009,		K_s_until},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 342 "./lexor_keyword.gperf"
      {"weak",			GN_KEYWORDS_1800_2009,		K_weak},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 153 "./lexor_keyword.gperf"
      {"global",			GN_KEYWORDS_1800_2009,		K_global},
#line 281 "./lexor_keyword.gperf"
      {"slew",			GN_KEYWORDS_VAMS_2_3,		K_slew},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 350 "./lexor_keyword.gperf"
      {"within",			GN_KEYWORDS_1800_2005,		K_within},
      {"",0,0},
#line 219 "./lexor_keyword.gperf"
      {"noise_table",		GN_KEYWORDS_VAMS_2_3,		K_noise_table},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0},
#line 91 "./lexor_keyword.gperf"
      {"ddx",			GN_KEYWORDS_VAMS_2_3,		K_ddx},
      {"",0,0},
#line 189 "./lexor_keyword.gperf"
      {"laplace_zp",		GN_KEYWORDS_VAMS_2_3,		K_laplace_zp},
#line 334 "./lexor_keyword.gperf"
      {"uwire",			GN_KEYWORDS_1364_2005,		K_uwire},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 147 "./lexor_keyword.gperf"
      {"fork",			GN_KEYWORDS_1364_1995,		K_fork},
#line 344 "./lexor_keyword.gperf"
      {"weak1",			GN_KEYWORDS_1364_1995,		K_weak1},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 198 "./lexor_keyword.gperf"
      {"localparam",		GN_KEYWORDS_1364_2001,		K_localparam},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 59 "./lexor_keyword.gperf"
      {"bool",			GN_KEYWORDS_ICARUS,		K_bool},
#line 148 "./lexor_keyword.gperf"
      {"forkjoin",		GN_KEYWORDS_1800_2005,		K_forkjoin},
      {"",0,0},
#line 297 "./lexor_keyword.gperf"
      {"supply0",		GN_KEYWORDS_1364_1995,		K_supply0},
      {"",0,0},
#line 280 "./lexor_keyword.gperf"
      {"sinh",			GN_KEYWORDS_VAMS_2_3,		K_sinh},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 50 "./lexor_keyword.gperf"
      {"automatic",		GN_KEYWORDS_1364_2001,		K_automatic},
#line 87 "./lexor_keyword.gperf"
      {"coverpoint",		GN_KEYWORDS_1800_2005,		K_coverpoint},
#line 361 "./lexor_keyword.gperf"
      {"zi_zp",			GN_KEYWORDS_VAMS_2_3,		K_zi_zp},
      {"",0,0}, {"",0,0},
#line 157 "./lexor_keyword.gperf"
      {"hypot",			GN_KEYWORDS_VAMS_2_3,		K_hypot},
#line 149 "./lexor_keyword.gperf"
      {"from",			GN_KEYWORDS_VAMS_2_3,		K_from},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 304 "./lexor_keyword.gperf"
      {"tanh",			GN_KEYWORDS_VAMS_2_3,		K_tanh},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 243 "./lexor_keyword.gperf"
      {"pulldown",		GN_KEYWORDS_1364_1995,		K_pulldown},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 345 "./lexor_keyword.gperf"
      {"while",			GN_KEYWORDS_1364_1995,		K_while},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 269 "./lexor_keyword.gperf"
      {"s_eventually",		GN_KEYWORDS_1800_2009,		K_s_eventually},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 242 "./lexor_keyword.gperf"
      {"pull1",			GN_KEYWORDS_1364_1995,		K_pull1},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 23 "./lexor_keyword.gperf"
      {"above",			GN_KEYWORDS_VAMS_2_3,		K_above},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0},
#line 276 "./lexor_keyword.gperf"
      {"shortreal",		GN_KEYWORDS_1800_2005,		K_shortreal},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 36 "./lexor_keyword.gperf"
      {"always_ff",		GN_KEYWORDS_1800_2005,		K_always_ff},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 144 "./lexor_keyword.gperf"
      {"foreach",		GN_KEYWORDS_1800_2005,		K_foreach},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 244 "./lexor_keyword.gperf"
      {"pullup",			GN_KEYWORDS_1364_1995,		K_pullup},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 221 "./lexor_keyword.gperf"
      {"noshowcancelled",	GN_KEYWORDS_1364_2001,		K_noshowcancelled},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0},
#line 298 "./lexor_keyword.gperf"
      {"supply1",		GN_KEYWORDS_1364_1995,		K_supply1},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0},
#line 203 "./lexor_keyword.gperf"
      {"macromodule",		GN_KEYWORDS_1364_1995,		K_macromodule},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 60 "./lexor_keyword.gperf"
      {"buf",			GN_KEYWORDS_1364_1995,		K_buf},
#line 130 "./lexor_keyword.gperf"
      {"eventually",		GN_KEYWORDS_1800_2009,		K_eventually},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0},
#line 139 "./lexor_keyword.gperf"
      {"first_match",		GN_KEYWORDS_1800_2005,		K_first_match},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 235 "./lexor_keyword.gperf"
      {"pow",			GN_KEYWORDS_VAMS_2_3,		K_pow},
      {"",0,0},
#line 61 "./lexor_keyword.gperf"
      {"bufif0",			GN_KEYWORDS_1364_1995,		K_bufif0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 277 "./lexor_keyword.gperf"
      {"showcancelled",		GN_KEYWORDS_1364_2001,		K_showcancelled},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 37 "./lexor_keyword.gperf"
      {"always_latch",		GN_KEYWORDS_1800_2005,		K_always_latch},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 35 "./lexor_keyword.gperf"
      {"always_comb",		GN_KEYWORDS_1800_2005,		K_always_comb},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0},
#line 205 "./lexor_keyword.gperf"
      {"max",			GN_KEYWORDS_VAMS_2_3,		K_max},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 155 "./lexor_keyword.gperf"
      {"highz0",			GN_KEYWORDS_1364_1995,		K_highz0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 260 "./lexor_keyword.gperf"
      {"resolveto",		GN_KEYWORDS_VAMS_2_3,		K_resolveto},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 84 "./lexor_keyword.gperf"
      {"cosh",			GN_KEYWORDS_VAMS_2_3,		K_cosh},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 307 "./lexor_keyword.gperf"
      {"throughout",		GN_KEYWORDS_1800_2005,		K_throughout},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 331 "./lexor_keyword.gperf"
      {"until_with",		GN_KEYWORDS_1800_2009,		K_until_with},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 346 "./lexor_keyword.gperf"
      {"white_noise",		GN_KEYWORDS_VAMS_2_3,		K_white_noise},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 142 "./lexor_keyword.gperf"
      {"flow",			GN_KEYWORDS_VAMS_2_3,		K_flow},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 349 "./lexor_keyword.gperf"
      {"with",			GN_KEYWORDS_1800_2005,		K_with},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 57 "./lexor_keyword.gperf"
      {"branch",			GN_KEYWORDS_VAMS_2_3,		K_branch},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 86 "./lexor_keyword.gperf"
      {"covergroup",		GN_KEYWORDS_1800_2005,		K_covergroup},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 62 "./lexor_keyword.gperf"
      {"bufif1",			GN_KEYWORDS_1364_1995,		K_bufif1},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 156 "./lexor_keyword.gperf"
      {"highz1",			GN_KEYWORDS_1364_1995,		K_highz1},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0},
#line 272 "./lexor_keyword.gperf"
      {"s_until_with",		GN_KEYWORDS_1800_2009,		K_s_until_with}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = keyword_hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}
#line 362 "./lexor_keyword.gperf"


int lexor_keyword_mask = 0;

int lexor_keyword_code(const char*str, unsigned nstr)
{
      const struct lexor_keyword*rc = Lkwd::check_identifier(str, nstr);
      if (rc == 0)
	  return IDENTIFIER;
      else if ((rc->mask & lexor_keyword_mask) == 0)
          return IDENTIFIER;
      else
	  return rc->tokenType;
}
