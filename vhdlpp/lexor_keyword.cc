/* C++ code produced by gperf version 3.1 */
/* Command-line: gperf -o -i 7 --ignore-case -C -k '1-4,6,9,$' -H keyword_hash -N check_identifier -t ./lexor_keyword.gperf  */

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

#line 8 "./lexor_keyword.gperf"

/* Command-line: gperf -o -i 7 --ignore-case -C -k '1-4,6,9,$' -H keyword_hash -N check_identifier -t lexor_keyword.gperf  */

#include "vhdlpp_config.h"
#include <cstring>
#include "compiler.h"
#include "parse_api.h"
#include "parse_wrap.h"

#line 18 "./lexor_keyword.gperf"
struct lexor_keyword { const char*name; int mask; int tokenType; };

#define TOTAL_KEYWORDS 112
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 13
#define MIN_HASH_VALUE 33
#define MAX_HASH_VALUE 424
/* maximum key range = 392, duplicates = 0 */

#ifndef GPERF_DOWNCASE
#define GPERF_DOWNCASE 1
static unsigned char gperf_downcase[256] =
  {
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
     30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
     45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
     60,  61,  62,  63,  64,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106,
    107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
    122,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
    195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
    225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
    255
  };
#endif

#ifndef GPERF_CASE_STRCMP
#define GPERF_CASE_STRCMP 1
static int
gperf_case_strcmp (const char *s1, const char *s2)
{
  for (;;)
    {
      unsigned char c1 = gperf_downcase[(unsigned char)*s1++];
      unsigned char c2 = gperf_downcase[(unsigned char)*s2++];
      if (c1 != 0 && c1 == c2)
        continue;
      return (int)c1 - (int)c2;
    }
}
#endif

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
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425,  17,  72,  22,  47,   7,
       22,  39,  37,   7,   7,  72,  12, 107,  12,  57,
       12,  12,   7,  27,  17,  97,  92,  59, 122, 122,
      425, 425, 425, 425, 425, 425, 425,  17,  72,  22,
       47,   7,  22,  39,  37,   7,   7,  72,  12, 107,
       12,  57,  12,  12,   7,  27,  17,  97,  92,  59,
      122, 122, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425, 425, 425, 425, 425,
      425, 425, 425, 425, 425, 425
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
      {"",0,0}, {"",0,0}, {"",0,0},
#line 60 "./lexor_keyword.gperf"
      {"in",		GN_KEYWORD_2008,	K_in},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 58 "./lexor_keyword.gperf"
      {"if",		GN_KEYWORD_2008,	K_if},
      {"",0,0}, {"",0,0},
#line 24 "./lexor_keyword.gperf"
      {"all",		GN_KEYWORD_2008,	K_all},
      {"",0,0}, {"",0,0},
#line 50 "./lexor_keyword.gperf"
      {"file",		GN_KEYWORD_2008,	K_file},
#line 61 "./lexor_keyword.gperf"
      {"inertial",	GN_KEYWORD_2008,	K_inertial},
#line 111 "./lexor_keyword.gperf"
      {"srl",		GN_KEYWORD_2008,	K_srl},
      {"",0,0},
#line 63 "./lexor_keyword.gperf"
      {"is",		GN_KEYWORD_2008,	K_is},
#line 44 "./lexor_keyword.gperf"
      {"else",		GN_KEYWORD_2008,	K_else},
      {"",0,0},
#line 109 "./lexor_keyword.gperf"
      {"sll",		GN_KEYWORD_2008,	K_sll},
      {"",0,0},
#line 94 "./lexor_keyword.gperf"
      {"reject",		GN_KEYWORD_2008,	K_reject},
      {"",0,0}, {"",0,0},
#line 110 "./lexor_keyword.gperf"
      {"sra",		GN_KEYWORD_2008,	K_sra},
      {"",0,0},
#line 80 "./lexor_keyword.gperf"
      {"or",		GN_KEYWORD_2008,	K_or},
#line 95 "./lexor_keyword.gperf"
      {"release",	GN_KEYWORD_2008,	K_release},
#line 22 "./lexor_keyword.gperf"
      {"after",		GN_KEYWORD_2008,	K_after},
#line 108 "./lexor_keyword.gperf"
      {"sla",		GN_KEYWORD_2008,	K_sla},
      {"",0,0}, {"",0,0},
#line 67 "./lexor_keyword.gperf"
      {"literal",	GN_KEYWORD_2008,	K_literal},
#line 45 "./lexor_keyword.gperf"
      {"elsif",		GN_KEYWORD_2008,	K_elsif},
#line 102 "./lexor_keyword.gperf"
      {"ror",		GN_KEYWORD_2008,	K_ror},
      {"",0,0},
#line 78 "./lexor_keyword.gperf"
      {"on",		GN_KEYWORD_2008,	K_on},
#line 35 "./lexor_keyword.gperf"
      {"case",		GN_KEYWORD_2008,	K_case},
#line 23 "./lexor_keyword.gperf"
      {"alias",		GN_KEYWORD_2008,	K_alias},
#line 74 "./lexor_keyword.gperf"
      {"nor",		GN_KEYWORD_2008,	K_nor},
#line 91 "./lexor_keyword.gperf"
      {"range",		GN_KEYWORD_2008,	K_range},
      {"",0,0},
#line 114 "./lexor_keyword.gperf"
      {"then",		GN_KEYWORD_2008,	K_then},
#line 98 "./lexor_keyword.gperf"
      {"restrict",	GN_KEYWORD_2008,	K_restrict},
#line 101 "./lexor_keyword.gperf"
      {"rol",		GN_KEYWORD_2008,	K_rol},
#line 93 "./lexor_keyword.gperf"
      {"register",	GN_KEYWORD_2008,	K_register},
#line 103 "./lexor_keyword.gperf"
      {"select",		GN_KEYWORD_2008,	K_select},
      {"",0,0},
#line 49 "./lexor_keyword.gperf"
      {"fairness",	GN_KEYWORD_2008,	K_fairness},
#line 51 "./lexor_keyword.gperf"
      {"for",		GN_KEYWORD_2008,	K_for},
#line 54 "./lexor_keyword.gperf"
      {"generate",	GN_KEYWORD_2008,	K_generate},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 55 "./lexor_keyword.gperf"
      {"generic",	GN_KEYWORD_2008,	K_generic},
      {"",0,0},
#line 77 "./lexor_keyword.gperf"
      {"of",		GN_KEYWORD_2008,	K_of},
#line 79 "./lexor_keyword.gperf"
      {"open",		GN_KEYWORD_2008,	K_open},
      {"",0,0},
#line 75 "./lexor_keyword.gperf"
      {"not",		GN_KEYWORD_2008,	K_not},
      {"",0,0},
#line 116 "./lexor_keyword.gperf"
      {"transport",	GN_KEYWORD_2008,	K_transport},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 84 "./lexor_keyword.gperf"
      {"port",		GN_KEYWORD_2008,	K_port},
#line 106 "./lexor_keyword.gperf"
      {"signal",		GN_KEYWORD_2008,	K_signal},
#line 46 "./lexor_keyword.gperf"
      {"end",		GN_KEYWORD_2008,	K_end},
      {"",0,0},
#line 28 "./lexor_keyword.gperf"
      {"assert",		GN_KEYWORD_2008,	K_assert},
      {"",0,0},
#line 52 "./lexor_keyword.gperf"
      {"force",		GN_KEYWORD_2008,	K_force},
#line 126 "./lexor_keyword.gperf"
      {"wait",		GN_KEYWORD_2008,	K_wait},
      {"",0,0},
#line 97 "./lexor_keyword.gperf"
      {"report",		GN_KEYWORD_2008,	K_report},
      {"",0,0},
#line 64 "./lexor_keyword.gperf"
      {"label",		GN_KEYWORD_2008,	K_label},
#line 25 "./lexor_keyword.gperf"
      {"and",		GN_KEYWORD_2008,	K_and},
#line 128 "./lexor_keyword.gperf"
      {"while",		GN_KEYWORD_2008,	K_while},
#line 21 "./lexor_keyword.gperf"
      {"access",		GN_KEYWORD_2008,	K_access},
#line 41 "./lexor_keyword.gperf"
      {"default",	GN_KEYWORD_2008,	K_default},
      {"",0,0},
#line 127 "./lexor_keyword.gperf"
      {"when",		GN_KEYWORD_2008,	K_when},
      {"",0,0},
#line 115 "./lexor_keyword.gperf"
      {"to",		GN_KEYWORD_2008,	K_to},
#line 90 "./lexor_keyword.gperf"
      {"pure",		GN_KEYWORD_2008,	K_pure},
      {"",0,0},
#line 26 "./lexor_keyword.gperf"
      {"architecture",	GN_KEYWORD_2008,	K_architecture},
      {"",0,0}, {"",0,0},
#line 71 "./lexor_keyword.gperf"
      {"nand",		GN_KEYWORD_2008,	K_nand},
#line 72 "./lexor_keyword.gperf"
      {"new",		GN_KEYWORD_2008,	K_new},
#line 121 "./lexor_keyword.gperf"
      {"use",		GN_KEYWORD_2008,	K_use},
#line 30 "./lexor_keyword.gperf"
      {"begin",		GN_KEYWORD_2008,	K_begin},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 20 "./lexor_keyword.gperf"
      {"abs",		GN_KEYWORD_2008,	K_abs},
      {"",0,0}, {"",0,0},
#line 76 "./lexor_keyword.gperf"
      {"null",		GN_KEYWORD_2008,	K_null},
#line 120 "./lexor_keyword.gperf"
      {"until",		GN_KEYWORD_2008,	K_until},
#line 69 "./lexor_keyword.gperf"
      {"map",		GN_KEYWORD_2008,	K_map},
      {"",0,0},
#line 29 "./lexor_keyword.gperf"
      {"attribute",	GN_KEYWORD_2008,	K_attribute},
#line 68 "./lexor_keyword.gperf"
      {"loop",		GN_KEYWORD_2008,	K_loop},
      {"",0,0},
#line 66 "./lexor_keyword.gperf"
      {"linkage",	GN_KEYWORD_2008,	K_linkage},
      {"",0,0},
#line 99 "./lexor_keyword.gperf"
      {"return",		GN_KEYWORD_2008,	K_return},
#line 87 "./lexor_keyword.gperf"
      {"process",	GN_KEYWORD_2008,	K_process},
#line 38 "./lexor_keyword.gperf"
      {"constant",	GN_KEYWORD_2008,	K_constant},
#line 129 "./lexor_keyword.gperf"
      {"with",		GN_KEYWORD_2008,	K_with},
      {"",0,0}, {"",0,0},
#line 42 "./lexor_keyword.gperf"
      {"disconnect",	GN_KEYWORD_2008,	K_disconnect},
#line 119 "./lexor_keyword.gperf"
      {"units",		GN_KEYWORD_2008,	K_units},
      {"",0,0},
#line 100 "./lexor_keyword.gperf"
      {"reverse_range",	GN_KEYWORD_2008,	K_reverse_range},
#line 86 "./lexor_keyword.gperf"
      {"procedure",	GN_KEYWORD_2008,	K_procedure},
#line 117 "./lexor_keyword.gperf"
      {"type",		GN_KEYWORD_2008,	K_type},
#line 104 "./lexor_keyword.gperf"
      {"sequence",	GN_KEYWORD_2008,	K_sequence},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 48 "./lexor_keyword.gperf"
      {"exit",		GN_KEYWORD_2008,	K_exit},
#line 27 "./lexor_keyword.gperf"
      {"array",		GN_KEYWORD_2008,	K_array},
#line 83 "./lexor_keyword.gperf"
      {"package",	GN_KEYWORD_2008,	K_package},
      {"",0,0},
#line 81 "./lexor_keyword.gperf"
      {"others",		GN_KEYWORD_2008,	K_others},
#line 73 "./lexor_keyword.gperf"
      {"next",		GN_KEYWORD_2008,	K_next},
#line 53 "./lexor_keyword.gperf"
      {"function",	GN_KEYWORD_2008,	K_function},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 124 "./lexor_keyword.gperf"
      {"vprop",		GN_KEYWORD_2008,	K_vprop},
      {"",0,0}, {"",0,0},
#line 107 "./lexor_keyword.gperf"
      {"shared",		GN_KEYWORD_2008,	K_shared},
      {"",0,0},
#line 40 "./lexor_keyword.gperf"
      {"cover",		GN_KEYWORD_2008,	K_cover},
#line 82 "./lexor_keyword.gperf"
      {"out",		GN_KEYWORD_2008,	K_out},
#line 112 "./lexor_keyword.gperf"
      {"strong",		GN_KEYWORD_2008,	K_strong},
#line 92 "./lexor_keyword.gperf"
      {"record",		GN_KEYWORD_2008,	K_record},
#line 37 "./lexor_keyword.gperf"
      {"configuration",	GN_KEYWORD_2008,	K_configuration},
#line 62 "./lexor_keyword.gperf"
      {"inout",		GN_KEYWORD_2008,	K_inout},
#line 131 "./lexor_keyword.gperf"
      {"xor",		GN_KEYWORD_2008,	K_xor},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0},
#line 130 "./lexor_keyword.gperf"
      {"xnor",		GN_KEYWORD_2008,	K_xnor},
#line 122 "./lexor_keyword.gperf"
      {"variable",	GN_KEYWORD_2008,	K_variable},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0},
#line 56 "./lexor_keyword.gperf"
      {"group",		GN_KEYWORD_2008,	K_group},
#line 89 "./lexor_keyword.gperf"
      {"protected",	GN_KEYWORD_2008,	K_protected},
#line 118 "./lexor_keyword.gperf"
      {"unaffected",	GN_KEYWORD_2008,	K_unaffected},
      {"",0,0},
#line 57 "./lexor_keyword.gperf"
      {"guarded",	GN_KEYWORD_2008,	K_guarded},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 88 "./lexor_keyword.gperf"
      {"property",	GN_KEYWORD_2008,	K_property},
#line 34 "./lexor_keyword.gperf"
      {"bus",		GN_KEYWORD_2008,	K_bus},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 125 "./lexor_keyword.gperf"
      {"vunit",		GN_KEYWORD_2008,	K_vunit},
#line 96 "./lexor_keyword.gperf"
      {"rem",		GN_KEYWORD_2008,	K_rem},
      {"",0,0},
#line 33 "./lexor_keyword.gperf"
      {"buffer",		GN_KEYWORD_2008,	K_buffer},
#line 65 "./lexor_keyword.gperf"
      {"library",	GN_KEYWORD_2008,	K_library},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 113 "./lexor_keyword.gperf"
      {"subtype",	GN_KEYWORD_2008,	K_subtype},
#line 31 "./lexor_keyword.gperf"
      {"block",		GN_KEYWORD_2008,	K_block},
      {"",0,0}, {"",0,0},
#line 59 "./lexor_keyword.gperf"
      {"impure",		GN_KEYWORD_2008,	K_impure},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 36 "./lexor_keyword.gperf"
      {"component",	GN_KEYWORD_2008,	K_component},
#line 39 "./lexor_keyword.gperf"
      {"context",	GN_KEYWORD_2008,	K_context},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0},
#line 70 "./lexor_keyword.gperf"
      {"mod",		GN_KEYWORD_2008,	K_mod},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 105 "./lexor_keyword.gperf"
      {"severity",	GN_KEYWORD_2008,	K_severity},
      {"",0,0}, {"",0,0},
#line 85 "./lexor_keyword.gperf"
      {"postponed",	GN_KEYWORD_2008,	K_postponed},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 47 "./lexor_keyword.gperf"
      {"entity",		GN_KEYWORD_2008,	K_entity},
      {"",0,0},
#line 43 "./lexor_keyword.gperf"
      {"downto",		GN_KEYWORD_2008,	K_downto},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
#line 123 "./lexor_keyword.gperf"
      {"vmode",		GN_KEYWORD_2008,	K_vmode},
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
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
      {"",0,0}, {"",0,0}, {"",0,0},
#line 32 "./lexor_keyword.gperf"
      {"body",		GN_KEYWORD_2008,	K_body}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = keyword_hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          const char *s = wordlist[key].name;

          if ((((unsigned char)*str ^ (unsigned char)*s) & ~32) == 0 && !gperf_case_strcmp (str, s))
            return &wordlist[key];
        }
    }
  return 0;
}
#line 132 "./lexor_keyword.gperf"


int lexor_keyword_mask = GN_KEYWORD_2008;

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
