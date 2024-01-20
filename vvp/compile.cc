/*
 * Copyright (c) 2001-2024 Stephen Williams (steve@icarus.com)
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


# include  "config.h"
# include  "delay.h"
# include  "arith.h"
# include  "compile.h"
# include  "logic.h"
# include  "resolv.h"
# include  "udp.h"
# include  "symbols.h"
# include  "codes.h"
# include  "schedule.h"
# include  "vpi_priv.h"
# include  "parse_misc.h"
# include  "statistics.h"
# include  "schedule.h"
# include  <iostream>
# include  <list>
# include  <cstdlib>
# include  <cstring>
# include  <cassert>

#ifdef __MINGW32__
#include <windows.h>
#endif

# include  "ivl_alloc.h"

unsigned compile_errors = 0;

/*
 * The opcode table lists all the code mnemonics, along with their
 * opcode and operand types. The table is written sorted by mnemonic
 * so that it can be searched by binary search. The opcode_compare
 * function is a helper function for that lookup.
 */

enum operand_e {
	/* Place holder for unused operand */
      OA_NONE,
	/* The operand is a number, an immediate unsigned integer */
      OA_NUMBER,
	/* The operand is a pointer to an array. */
      OA_ARR_PTR,
	/* The operand is a thread bit index or short integer */
      OA_BIT1,
      OA_BIT2,
	/* The operand is a pointer to code space */
      OA_CODE_PTR,
      OA_CODE_PTR2,
	/* The operand is a variable or net pointer */
      OA_FUNC_PTR,
	/* The operand is a second functor pointer */
      OA_FUNC_PTR2,
	/* The operand is a VPI handle */
      OA_VPI_PTR,
	/* String */
      OA_STRING
};

struct opcode_table_s {
      const char*mnemonic;
      vvp_code_fun opcode;

      unsigned argc;
      enum operand_e argt[OPERAND_MAX];
};

static const struct opcode_table_s opcode_table[] = {
      { "%abs/wr", of_ABS_WR, 0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%add",    of_ADD,    0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%add/wr", of_ADD_WR, 0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%addi",   of_ADDI,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%alloc",  of_ALLOC,  1,  {OA_VPI_PTR,  OA_NONE,     OA_NONE} },
      { "%and",    of_AND,    0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%and/r",  of_ANDR,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%assign/ar",of_ASSIGN_AR,2,{OA_ARR_PTR,OA_BIT1,     OA_NONE} },
      { "%assign/ar/d",of_ASSIGN_ARD,2,{OA_ARR_PTR,OA_BIT1,  OA_NONE} },
      { "%assign/ar/e",of_ASSIGN_ARE,1,{OA_ARR_PTR,OA_NONE,  OA_NONE} },
      { "%assign/vec4",      of_ASSIGN_VEC4,       2, {OA_FUNC_PTR, OA_BIT1, OA_NONE} },
      { "%assign/vec4/a/d",  of_ASSIGN_VEC4_A_D,   3, {OA_ARR_PTR,  OA_BIT1, OA_BIT2} },
      { "%assign/vec4/a/e",  of_ASSIGN_VEC4_A_E,   2, {OA_ARR_PTR,  OA_BIT1, OA_NONE} },
      { "%assign/vec4/d",    of_ASSIGN_VEC4D,      2, {OA_FUNC_PTR, OA_BIT1, OA_NONE} },
      { "%assign/vec4/e",    of_ASSIGN_VEC4E,      1, {OA_FUNC_PTR, OA_NONE, OA_NONE} },
      { "%assign/vec4/off/d",of_ASSIGN_VEC4_OFF_D, 3, {OA_FUNC_PTR, OA_BIT1, OA_BIT2} },
      { "%assign/vec4/off/e",of_ASSIGN_VEC4_OFF_E, 2, {OA_FUNC_PTR, OA_BIT1, OA_NONE} },
      { "%assign/wr",  of_ASSIGN_WR, 2,{OA_VPI_PTR, OA_BIT1, OA_NONE} },
      { "%assign/wr/d",of_ASSIGN_WRD,2,{OA_VPI_PTR, OA_BIT1, OA_NONE} },
      { "%assign/wr/e",of_ASSIGN_WRE,1,{OA_VPI_PTR, OA_NONE, OA_NONE} },
      { "%blend",    of_BLEND,   0,  {OA_NONE,  OA_NONE,     OA_NONE} },
      { "%blend/wr", of_BLEND_WR,0,  {OA_NONE,  OA_NONE,     OA_NONE} },
      { "%breakpoint", of_BREAKPOINT, 0,  {OA_NONE, OA_NONE, OA_NONE} },
      { "%callf/obj",       of_CALLF_OBJ,       2,{OA_CODE_PTR2,OA_VPI_PTR, OA_NONE} },
      { "%callf/real",      of_CALLF_REAL,      2,{OA_CODE_PTR2,OA_VPI_PTR, OA_NONE} },
      { "%callf/str",       of_CALLF_STR,       2,{OA_CODE_PTR2,OA_VPI_PTR, OA_NONE} },
      { "%callf/vec4",      of_CALLF_VEC4,      2,{OA_CODE_PTR2,OA_VPI_PTR, OA_NONE} },
      { "%callf/void",      of_CALLF_VOID,      2,{OA_CODE_PTR2,OA_VPI_PTR, OA_NONE} },
      { "%cassign/link",    of_CASSIGN_LINK,    2,{OA_FUNC_PTR,OA_FUNC_PTR2,OA_NONE} },
      { "%cassign/vec4",    of_CASSIGN_VEC4,    1,{OA_FUNC_PTR,OA_NONE,     OA_NONE} },
      { "%cassign/vec4/off",of_CASSIGN_VEC4_OFF,2,{OA_FUNC_PTR,OA_BIT1,     OA_NONE} },
      { "%cassign/wr",  of_CASSIGN_WR,  1,{OA_FUNC_PTR,OA_NONE,     OA_NONE} },
      { "%cast/vec2/dar", of_CAST_VEC2_DAR, 1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%cast/vec4/dar", of_CAST_VEC4_DAR, 1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%cast/vec4/str", of_CAST_VEC4_STR, 1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%cast2",   of_CAST2,  0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%cmp/e",   of_CMPE,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%cmp/ne",  of_CMPNE,  0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%cmp/s",   of_CMPS,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%cmp/str", of_CMPSTR, 0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%cmp/u",   of_CMPU,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%cmp/we",  of_CMPWE,  0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%cmp/wne", of_CMPWNE, 0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%cmp/wr",  of_CMPWR,  0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%cmp/x",   of_CMPX,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%cmp/z",   of_CMPZ,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%cmpi/e",  of_CMPIE,  3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%cmpi/ne", of_CMPINE, 3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%cmpi/s",  of_CMPIS,  3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%cmpi/u",  of_CMPIU,  3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%concat/str",  of_CONCAT_STR,  0,{OA_NONE,  OA_NONE,  OA_NONE} },
      { "%concat/vec4", of_CONCAT_VEC4, 0,{OA_NONE,  OA_NONE,  OA_NONE} },
      { "%concati/str", of_CONCATI_STR, 1,{OA_STRING,OA_NONE,  OA_NONE} },
      { "%concati/vec4",of_CONCATI_VEC4,3,{OA_BIT1,  OA_BIT2,  OA_NUMBER} },
      { "%cvt/rv",   of_CVT_RV,  0, {OA_NONE,   OA_NONE,     OA_NONE} },
      { "%cvt/rv/s", of_CVT_RV_S,0, {OA_NONE,   OA_NONE,     OA_NONE} },
      { "%cvt/sr", of_CVT_SR, 1,  {OA_BIT1,     OA_NONE,     OA_NONE} },
      { "%cvt/ur", of_CVT_UR, 1,  {OA_BIT1,     OA_NONE,     OA_NONE} },
      { "%cvt/vr", of_CVT_VR, 1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%deassign",of_DEASSIGN,3,{OA_FUNC_PTR, OA_BIT1,     OA_BIT2} },
      { "%deassign/wr",of_DEASSIGN_WR,1,{OA_FUNC_PTR, OA_NONE,     OA_NONE} },
      { "%debug/thr",  of_DEBUG_THR,  1,{OA_STRING,   OA_NONE,     OA_NONE} },
      { "%delay",  of_DELAY,  2,  {OA_BIT1,     OA_BIT2,     OA_NONE} },
      { "%delayx", of_DELAYX, 1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%delete/elem",of_DELETE_ELEM,1,{OA_FUNC_PTR,OA_NONE,OA_NONE} },
      { "%delete/obj",of_DELETE_OBJ,1,{OA_FUNC_PTR,OA_NONE,  OA_NONE} },
      { "%delete/tail",of_DELETE_TAIL,2,{OA_FUNC_PTR,OA_BIT1,OA_NONE} },
      { "%disable",  of_DISABLE, 1, {OA_VPI_PTR,OA_NONE,     OA_NONE} },
      { "%disable/flow", of_DISABLE_FLOW, 1, {OA_VPI_PTR,OA_NONE, OA_NONE} },
      { "%disable/fork",of_DISABLE_FORK,0,{OA_NONE,OA_NONE,  OA_NONE} },
      { "%div",      of_DIV,     0, {OA_NONE,   OA_NONE,     OA_NONE} },
      { "%div/s",    of_DIV_S,   0, {OA_NONE,   OA_NONE,     OA_NONE} },
      { "%div/wr",   of_DIV_WR,  0, {OA_NONE,   OA_NONE,     OA_NONE} },
      { "%dup/obj",  of_DUP_OBJ, 0, {OA_NONE,   OA_NONE,     OA_NONE} },
      { "%dup/real", of_DUP_REAL,0, {OA_NONE,   OA_NONE,     OA_NONE} },
      { "%dup/vec4", of_DUP_VEC4,0, {OA_NONE,   OA_NONE,     OA_NONE} },
      { "%end",    of_END,    0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%evctl",  of_EVCTL,  2,  {OA_FUNC_PTR, OA_BIT1,     OA_NONE} },
      { "%evctl/c",of_EVCTLC, 0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%evctl/i",of_EVCTLI, 2,  {OA_FUNC_PTR, OA_BIT1,     OA_NONE} },
      { "%evctl/s",of_EVCTLS, 2,  {OA_FUNC_PTR, OA_BIT1,     OA_NONE} },
      { "%event",    of_EVENT,    1, {OA_FUNC_PTR, OA_NONE, OA_NONE} },
      { "%event/nb", of_EVENT_NB, 2, {OA_FUNC_PTR, OA_BIT1, OA_NONE} },
      { "%flag_get/vec4", of_FLAG_GET_VEC4, 1, {OA_NUMBER, OA_NONE, OA_NONE} },
      { "%flag_inv",      of_FLAG_INV,      1, {OA_BIT1,   OA_NONE, OA_NONE} },
      { "%flag_mov",      of_FLAG_MOV,      2, {OA_BIT1,   OA_BIT2, OA_NONE} },
      { "%flag_or",       of_FLAG_OR,       2, {OA_BIT1,   OA_BIT2, OA_NONE} },
      { "%flag_set/imm",  of_FLAG_SET_IMM,  2, {OA_NUMBER, OA_BIT1, OA_NONE} },
      { "%flag_set/vec4", of_FLAG_SET_VEC4, 1, {OA_NUMBER, OA_NONE, OA_NONE} },
      { "%force/link",    of_FORCE_LINK,2,{OA_FUNC_PTR, OA_FUNC_PTR2, OA_NONE} },
      { "%force/vec4",    of_FORCE_VEC4,    1,{OA_FUNC_PTR, OA_NONE,      OA_NONE} },
      { "%force/vec4/off",of_FORCE_VEC4_OFF,2,{OA_FUNC_PTR, OA_BIT1,      OA_NONE} },
      { "%force/vec4/off/d",of_FORCE_VEC4_OFF_D,3,{OA_FUNC_PTR, OA_BIT1,  OA_BIT2} },
      { "%force/wr",      of_FORCE_WR,      1,{OA_FUNC_PTR, OA_NONE,      OA_NONE} },
      { "%fork",   of_FORK,   2,  {OA_CODE_PTR2,OA_VPI_PTR,  OA_NONE} },
      { "%free",   of_FREE,   1,  {OA_VPI_PTR,  OA_NONE,     OA_NONE} },
      { "%inv",    of_INV,    0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%ix/add", of_IX_ADD, 3,  {OA_NUMBER,   OA_BIT1,     OA_BIT2} },
      { "%ix/getv",of_IX_GETV,2,  {OA_BIT1,     OA_FUNC_PTR, OA_NONE} },
      { "%ix/getv/s",of_IX_GETV_S,2, {OA_BIT1,   OA_FUNC_PTR, OA_NONE} },
      { "%ix/load",of_IX_LOAD,3,  {OA_NUMBER,   OA_BIT1,     OA_BIT2} },
      { "%ix/mov", of_IX_MOV, 2,  {OA_BIT1,     OA_BIT2,     OA_NONE} },
      { "%ix/mul", of_IX_MUL, 3,  {OA_NUMBER,   OA_BIT1,     OA_BIT2} },
      { "%ix/sub", of_IX_SUB, 3,  {OA_NUMBER,   OA_BIT1,     OA_BIT2} },
      { "%ix/vec4",  of_IX_VEC4,  1,  {OA_NUMBER,   OA_NONE, OA_NONE} },
      { "%ix/vec4/s",of_IX_VEC4_S,1,  {OA_NUMBER,   OA_NONE, OA_NONE} },
      { "%jmp",    of_JMP,    1,  {OA_CODE_PTR, OA_NONE,     OA_NONE} },
      { "%jmp/0",  of_JMP0,   2,  {OA_CODE_PTR, OA_BIT1,     OA_NONE} },
      { "%jmp/0xz",of_JMP0XZ, 2,  {OA_CODE_PTR, OA_BIT1,     OA_NONE} },
      { "%jmp/1",  of_JMP1,   2,  {OA_CODE_PTR, OA_BIT1,     OA_NONE} },
      { "%jmp/1xz",of_JMP1XZ, 2,  {OA_CODE_PTR, OA_BIT1,     OA_NONE} },
      { "%join",   of_JOIN,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%join/detach",of_JOIN_DETACH,1,{OA_NUMBER,OA_NONE,  OA_NONE} },
      { "%load/ar",of_LOAD_AR,2,  {OA_ARR_PTR,  OA_BIT1,     OA_NONE} },
      { "%load/dar/r",  of_LOAD_DAR_R,    1, {OA_FUNC_PTR, OA_NONE, OA_NONE}},
      { "%load/dar/str",of_LOAD_DAR_STR,  1, {OA_FUNC_PTR, OA_NONE, OA_NONE} },
      { "%load/dar/vec4",of_LOAD_DAR_VEC4,1, {OA_FUNC_PTR, OA_NONE, OA_NONE} },
      { "%load/obj",   of_LOAD_OBJ,  1,{OA_FUNC_PTR,OA_NONE, OA_NONE} },
      { "%load/obja",  of_LOAD_OBJA, 2,{OA_ARR_PTR, OA_BIT1, OA_NONE} },
      { "%load/real",  of_LOAD_REAL, 1,{OA_VPI_PTR, OA_NONE, OA_NONE} },
      { "%load/str",   of_LOAD_STR,  1,{OA_FUNC_PTR,OA_NONE, OA_NONE} },
      { "%load/stra",  of_LOAD_STRA, 2,{OA_ARR_PTR, OA_BIT1, OA_NONE} },
      { "%load/vec4",  of_LOAD_VEC4, 1,{OA_FUNC_PTR,OA_NONE,  OA_NONE} },
      { "%load/vec4a", of_LOAD_VEC4A,2,{OA_ARR_PTR, OA_BIT1, OA_NONE} },
      { "%max/wr", of_MAX_WR, 0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%min/wr", of_MIN_WR, 0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%mod",    of_MOD,    0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%mod/s",  of_MOD_S,  0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%mod/wr", of_MOD_WR, 0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%mul",    of_MUL,    0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%mul/wr", of_MUL_WR, 0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%muli",   of_MULI,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%nand",   of_NAND,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%nand/r", of_NANDR,  0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%new/cobj",  of_NEW_COBJ,  1, {OA_VPI_PTR,OA_NONE,  OA_NONE} },
      { "%new/darray",of_NEW_DARRAY,2, {OA_BIT1,   OA_STRING,OA_NONE} },
      { "%noop",   of_NOOP,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%nor",    of_NOR,    0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%nor/r",  of_NORR,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%null",   of_NULL,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%or",     of_OR,     0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%or/r",   of_ORR,    0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%pad/s",  of_PAD_S,  1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%pad/u",  of_PAD_U,  1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%part/s", of_PART_S, 1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%part/u", of_PART_U, 1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%parti/s",of_PARTI_S,3,  {OA_NUMBER,   OA_BIT1,     OA_BIT2} },
      { "%parti/u",of_PARTI_U,3,  {OA_NUMBER,   OA_BIT1,     OA_BIT2} },
      { "%pop/obj", of_POP_OBJ, 2, {OA_BIT1,    OA_BIT2,     OA_NONE} },
      { "%pop/real",of_POP_REAL,1, {OA_NUMBER,  OA_NONE,     OA_NONE} },
      { "%pop/str", of_POP_STR, 1, {OA_NUMBER,  OA_NONE,     OA_NONE} },
      { "%pop/vec4",of_POP_VEC4,1, {OA_NUMBER,  OA_NONE,     OA_NONE} },
      { "%pow",     of_POW,     0,  {OA_NONE,   OA_NONE,     OA_NONE} },
      { "%pow/s",   of_POW_S,   0,  {OA_NONE,   OA_NONE,     OA_NONE} },
      { "%pow/wr",  of_POW_WR,  0,  {OA_NONE,   OA_NONE,     OA_NONE} },
      { "%prop/obj",of_PROP_OBJ,2,  {OA_NUMBER,   OA_BIT1,     OA_NONE} },
      { "%prop/r",  of_PROP_R,  1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%prop/str",of_PROP_STR,1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%prop/v",  of_PROP_V,  1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%pushi/real",of_PUSHI_REAL,2,{OA_BIT1,   OA_BIT2,   OA_NONE} },
      { "%pushi/str", of_PUSHI_STR, 1,{OA_STRING, OA_NONE,   OA_NONE} },
      { "%pushi/vec4",of_PUSHI_VEC4,3,{OA_BIT1,   OA_BIT2,   OA_NUMBER} },
      { "%pushv/str", of_PUSHV_STR, 0,{OA_NONE,   OA_NONE,   OA_NONE} },
      { "%putc/str/vec4",of_PUTC_STR_VEC4,2,{OA_FUNC_PTR,OA_BIT1,OA_NONE} },
      { "%qinsert/real",of_QINSERT_REAL,2,{OA_FUNC_PTR,OA_BIT1,OA_NONE} },
      { "%qinsert/str", of_QINSERT_STR, 2,{OA_FUNC_PTR,OA_BIT1,OA_NONE} },
      { "%qinsert/v",   of_QINSERT_V,   3,{OA_FUNC_PTR,OA_BIT1,OA_BIT2} },
      { "%qpop/b/real",of_QPOP_B_REAL,1,{OA_FUNC_PTR,OA_NONE,OA_NONE} },
      { "%qpop/b/str", of_QPOP_B_STR, 1,{OA_FUNC_PTR,OA_NONE,OA_NONE} },
      { "%qpop/b/v",   of_QPOP_B_V,   2,{OA_FUNC_PTR,OA_BIT1,OA_NONE} },
      { "%qpop/f/real",of_QPOP_F_REAL,1,{OA_FUNC_PTR,OA_NONE,OA_NONE} },
      { "%qpop/f/str", of_QPOP_F_STR, 1,{OA_FUNC_PTR,OA_NONE,OA_NONE} },
      { "%qpop/f/v",   of_QPOP_F_V,   2,{OA_FUNC_PTR,OA_BIT1,OA_NONE} },
      { "%release/net",of_RELEASE_NET,3,{OA_FUNC_PTR,OA_BIT1,OA_BIT2} },
      { "%release/reg",of_RELEASE_REG,3,{OA_FUNC_PTR,OA_BIT1,OA_BIT2} },
      { "%release/wr", of_RELEASE_WR, 2,{OA_FUNC_PTR,OA_BIT1,OA_NONE} },
      { "%replicate", of_REPLICATE,   1,{OA_NUMBER,  OA_NONE,OA_NONE} },
      { "%ret/real",  of_RET_REAL,    1,{OA_NUMBER,  OA_NONE,OA_NONE} },
      { "%ret/str",   of_RET_STR,     1,{OA_NUMBER,  OA_NONE,OA_NONE} },
      { "%ret/vec4",  of_RET_VEC4,    3,{OA_NUMBER,  OA_BIT1,OA_BIT2} },
      { "%retload/real",of_RETLOAD_REAL,1,{OA_NUMBER,  OA_NONE,OA_NONE} },
      { "%retload/str", of_RETLOAD_STR, 1,{OA_NUMBER,  OA_NONE,OA_NONE} },
      { "%retload/vec4",of_RETLOAD_VEC4,1,{OA_NUMBER,  OA_NONE,OA_NONE} },
      { "%scopy",  of_SCOPY,  0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%set/dar/obj/real",of_SET_DAR_OBJ_REAL,1,{OA_NUMBER,OA_NONE,OA_NONE} },
      { "%set/dar/obj/str", of_SET_DAR_OBJ_STR, 1,{OA_NUMBER,OA_NONE,OA_NONE} },
      { "%set/dar/obj/vec4",of_SET_DAR_OBJ_VEC4,1,{OA_NUMBER,OA_NONE,OA_NONE} },
      { "%shiftl",   of_SHIFTL,   1, {OA_NUMBER, OA_NONE,   OA_NONE} },
      { "%shiftr",   of_SHIFTR,   1, {OA_NUMBER, OA_NONE,   OA_NONE} },
      { "%shiftr/s", of_SHIFTR_S, 1, {OA_NUMBER, OA_NONE,   OA_NONE} },
      { "%split/vec4",    of_SPLIT_VEC4,    1,{OA_NUMBER,   OA_NONE, OA_NONE} },
      { "%store/dar/r",   of_STORE_DAR_R,   1,{OA_FUNC_PTR, OA_NONE, OA_NONE} },
      { "%store/dar/str", of_STORE_DAR_STR, 1,{OA_FUNC_PTR, OA_NONE, OA_NONE} },
      { "%store/dar/vec4",of_STORE_DAR_VEC4,1,{OA_FUNC_PTR, OA_NONE, OA_NONE} },
      { "%store/obj",   of_STORE_OBJ,   1, {OA_FUNC_PTR,OA_NONE, OA_NONE} },
      { "%store/obja",  of_STORE_OBJA,  2, {OA_ARR_PTR, OA_BIT1, OA_NONE} },
      { "%store/prop/obj",of_STORE_PROP_OBJ,2, {OA_NUMBER,  OA_BIT1, OA_NONE} },
      { "%store/prop/r",  of_STORE_PROP_R,  1, {OA_NUMBER,  OA_NONE, OA_NONE} },
      { "%store/prop/str",of_STORE_PROP_STR,1, {OA_NUMBER,  OA_NONE, OA_NONE} },
      { "%store/prop/v",  of_STORE_PROP_V,  2, {OA_NUMBER,  OA_BIT1, OA_NONE} },
      { "%store/qb/r",   of_STORE_QB_R,    2, {OA_FUNC_PTR, OA_BIT1, OA_NONE} },
      { "%store/qb/str", of_STORE_QB_STR,  2, {OA_FUNC_PTR, OA_BIT1, OA_NONE} },
      { "%store/qb/v",   of_STORE_QB_V,    3, {OA_FUNC_PTR, OA_BIT1, OA_BIT2} },
      { "%store/qdar/r",  of_STORE_QDAR_R,  2,{OA_FUNC_PTR, OA_BIT1, OA_NONE} },
      { "%store/qdar/str",of_STORE_QDAR_STR,2,{OA_FUNC_PTR, OA_BIT1, OA_NONE} },
      { "%store/qdar/v",  of_STORE_QDAR_V,  3,{OA_FUNC_PTR, OA_BIT1, OA_BIT2} },
      { "%store/qf/r",   of_STORE_QF_R,    2, {OA_FUNC_PTR, OA_BIT1, OA_NONE} },
      { "%store/qf/str", of_STORE_QF_STR,  2, {OA_FUNC_PTR, OA_BIT1, OA_NONE} },
      { "%store/qf/v",   of_STORE_QF_V,    3, {OA_FUNC_PTR, OA_BIT1, OA_BIT2} },
      { "%store/qobj/r",  of_STORE_QOBJ_R,  2, {OA_FUNC_PTR,OA_BIT1, OA_NONE} },
      { "%store/qobj/str",of_STORE_QOBJ_STR,2, {OA_FUNC_PTR,OA_BIT1, OA_NONE} },
      { "%store/qobj/v",  of_STORE_QOBJ_V,  3, {OA_FUNC_PTR,OA_BIT1, OA_BIT2} },
      { "%store/real",    of_STORE_REAL,    1, {OA_FUNC_PTR,OA_NONE, OA_NONE} },
      { "%store/reala",   of_STORE_REALA,   2, {OA_ARR_PTR, OA_BIT1, OA_NONE} },
      { "%store/str",     of_STORE_STR,     1, {OA_FUNC_PTR,OA_NONE, OA_NONE} },
      { "%store/stra",    of_STORE_STRA,    2, {OA_ARR_PTR, OA_BIT1, OA_NONE} },
      { "%store/vec4",    of_STORE_VEC4,    3, {OA_FUNC_PTR,OA_BIT1, OA_BIT2} },
      { "%store/vec4a",   of_STORE_VEC4A,   3, {OA_ARR_PTR, OA_BIT1, OA_BIT2} },
      { "%sub",    of_SUB,    0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%sub/wr", of_SUB_WR, 0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%subi",   of_SUBI,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%substr",     of_SUBSTR,     2,{OA_BIT1,    OA_BIT2, OA_NONE} },
      { "%substr/vec4",of_SUBSTR_VEC4,2,{OA_BIT1,    OA_BIT2, OA_NONE} },
      { "%test_nul",     of_TEST_NUL,     1,{OA_FUNC_PTR,OA_NONE,    OA_NONE} },
      { "%test_nul/a",   of_TEST_NUL_A,   2,{OA_ARR_PTR, OA_BIT1,    OA_NONE} },
      { "%test_nul/obj", of_TEST_NUL_OBJ, 0,{OA_NONE,    OA_NONE,    OA_NONE} },
      { "%test_nul/prop",of_TEST_NUL_PROP,2,{OA_NUMBER,  OA_BIT1,    OA_NONE} },
      { "%wait",   of_WAIT,   1,  {OA_FUNC_PTR, OA_NONE,     OA_NONE} },
      { "%wait/fork",of_WAIT_FORK,0,{OA_NONE,   OA_NONE,     OA_NONE} },
      { "%xnor",   of_XNOR,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%xnor/r", of_XNORR,  0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%xor",    of_XOR,    0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%xor/r",  of_XORR,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { 0, of_NOOP, 0, {OA_NONE, OA_NONE, OA_NONE} }
};

static const unsigned opcode_count =
                    sizeof(opcode_table)/sizeof(*opcode_table) - 1;

static int opcode_compare(const void*k, const void*r)
{
      const char*kp = (const char*)k;
      const struct opcode_table_s*rp = (const struct opcode_table_s*)r;
      return strcmp(kp, rp->mnemonic);
}

/*
 * Keep a symbol table of addresses within code space. Labels on
 * executable opcodes are mapped to their address here.
 */
static symbol_table_t sym_codespace = 0;

/*
 * Keep a symbol table of functors mentioned in the source. This table
 * is used to resolve references as they come.
 */
static symbol_table_t sym_functors = 0;

/*
 * VPI objects are indexed during compile time so that they can be
 * linked together as they are created. This symbol table matches
 * labels to vpiHandles.
 */
static symbol_table_t sym_vpi = 0;


/*
 * If a functor parameter makes a forward reference to a functor, then
 * I need to save that reference and resolve it after the functors are
 * created. Use this structure to keep the unresolved references in an
 * unsorted singly linked list.
 *
 * The postpone_functor_input arranges for a functor input to be
 * resolved and connected at cleanup. This is used if the symbol is
 * defined after its use in a functor. The ptr parameter is the
 * complete vvp_input_t for the input port.
 */


/*
 *  Add a functor to the symbol table
 */

void define_functor_symbol(const char*label, vvp_net_t*net)
{
      symbol_value_t val;
      val.net = net;
      sym_set_value(sym_functors, label, val);
}

static vvp_net_t*lookup_functor_symbol(const char*label)
{
      assert(sym_functors);
      symbol_value_t val = sym_get_value(sym_functors, label);
      return val.net;
}

vpiHandle vvp_lookup_handle(const char*label)
{
      symbol_value_t val = sym_get_value(sym_vpi, label);
      if (val.ptr) return (vpiHandle) val.ptr;
      return 0;
}

vvp_net_t* vvp_net_lookup(const char*label)
{
      static bool t0_trigger_generated = false;

        /* First, look to see if the symbol is a vpi object of some
	   sort. If it is, then get the vvp_ipoint_t pointer out of
	   the vpiHandle. */
      symbol_value_t val = sym_get_value(sym_vpi, label);
      if (val.ptr) {
	    vpiHandle vpi = (vpiHandle) val.ptr;
	    switch (vpi->get_type_code()) {
		case vpiNet:
		case vpiReg:
		case vpiBitVar:
		case vpiByteVar:
		case vpiShortIntVar:
		case vpiIntVar:
		case vpiLongIntVar:
		case vpiIntegerVar: {
		      __vpiSignal*sig = dynamic_cast<__vpiSignal*>(vpi);
		      return sig->node;
		}

		case vpiRealVar: {
		      __vpiRealVar*sig = dynamic_cast<__vpiRealVar*>(vpi);
		      return sig->net;
		}

		case vpiStringVar:
		case vpiArrayVar:
		case vpiClassVar: {
		      __vpiBaseVar*sig = dynamic_cast<__vpiBaseVar*>(vpi);
		      return sig->get_net();
		}

		case vpiNamedEvent: {
		      __vpiNamedEvent*tmp = dynamic_cast<__vpiNamedEvent*>(vpi);
		      return tmp->funct;
		}

		default:
		  fprintf(stderr, "Unsupported type %d.\n",
		          vpi->get_type_code());
		  assert(0);
	    }
      }


	/* Failing that, look for a general functor. */
      vvp_net_t*tmp = lookup_functor_symbol(label);

	// This is a special label used to create a T0 trigger for the
	// always_comb/latch processes.
     if (! t0_trigger_generated && (strcmp(label, "E_0x0") == 0)) {
	      // This should never happen, but if it does then the E_0x0
	      // event generation may need to be explicitly generated in
	      // the compiler output instead of implicitly in this code.
	    assert(! vpip_peek_current_scope()->is_automatic());
	    t0_trigger_generated = true;
	      // Create a local event with no name for the T0 trigger
	    compile_named_event(strdup(label), strcpy(new char [1],""), true);
	    tmp = vvp_net_lookup(label);
	    assert(tmp);
	      // Create a trigger for the T0 event.
	    vvp_net_ptr_t ptr (tmp, 0);
	    schedule_t0_trigger(ptr);
     }

      return tmp;
}

/*
 * The resolv_list_s is the base class for a symbol resolve action, and
 * the resolv_list is an unordered list of these resolve actions. Some
 * function creates an instance of a resolv_list_s object that
 * contains the data pertinent to that resolution request, and
 * executes it with the resolv_submit function. If the operation can
 * complete, then the resolv_submit deletes the object. Otherwise, it
 * pushes it onto the resolv_list for later processing.
 *
 * Derived classes implement the resolve function to perform the
 * actual binding or resolution that the instance requires. If the
 * function succeeds, the resolve method returns true and the object
 * can be deleted any time.
 *
 * The mes parameter of the resolve method tells the resolver that
 * this call is its last chance. If it cannot complete the operation,
 * it must print an error message and return false.
 */
static resolv_list_s*resolv_list = 0;

resolv_list_s::~resolv_list_s()
{
      free(label_);
}

void resolv_submit(resolv_list_s*cur)
{
      if (cur->resolve()) {
	    delete cur;
	    return;
      }

      cur->next = resolv_list;
      resolv_list = cur;
}


/*
 * Look up vvp_nets in the symbol table. The "source" is the label for
 * the net that I want to feed, and net->port[port] is the vvp_net
 * input that I want that node to feed into. When the name is found,
 * put net->port[port] into the fan-out list for that node.
 */
struct vvp_net_resolv_list_s: public resolv_list_s {

      explicit vvp_net_resolv_list_s(char*l) : resolv_list_s(l) { }
	// port to be driven by the located node.
      vvp_net_ptr_t port;
      virtual bool resolve(bool mes);
};

bool vvp_net_resolv_list_s::resolve(bool mes)
{
      vvp_net_t*tmp = vvp_net_lookup(label());

      if (tmp) {
	      // Link the input port to the located output.
	    tmp->link(port);
	    return true;
      }

      if (mes)
	    fprintf(stderr, "unresolved vvp_net reference: %s\n", label());

      return false;
}

inline static
void postpone_functor_input(vvp_net_ptr_t port, char*lab)
{
      struct vvp_net_resolv_list_s*res = new struct vvp_net_resolv_list_s(lab);
      res->port   = port;

      resolv_submit(res);
}


/*
 *  Generic functor reference lookup.
 */

struct functor_gen_resolv_list_s: public resolv_list_s {
      explicit functor_gen_resolv_list_s(char*txt) : resolv_list_s(txt) {
	    ref = 0;
      }
      vvp_net_t**ref;
      virtual bool resolve(bool mes);
};

bool functor_gen_resolv_list_s::resolve(bool mes)
{
      vvp_net_t*tmp = vvp_net_lookup(label());

      if (tmp) {
	    *ref = tmp;
	    return true;
      }

      if (mes)
	    fprintf(stderr, "unresolved functor reference: %s\n", label());

      return false;
}

void functor_ref_lookup(vvp_net_t**ref, char*lab)
{
      struct functor_gen_resolv_list_s*res =
	    new struct functor_gen_resolv_list_s(lab);

      res->ref    = ref;

      resolv_submit(res);
}

/*
 *  vpiHandle lookup
 */

struct vpi_handle_resolv_list_s: public resolv_list_s {
      explicit vpi_handle_resolv_list_s(char*lab) : resolv_list_s(lab) {
	    handle = NULL;
      }
      virtual bool resolve(bool mes);
      vpiHandle *handle;
};

bool vpi_handle_resolv_list_s::resolve(bool mes)
{
      symbol_value_t val = sym_get_value(sym_vpi, label());
      if (!val.ptr) {
	    // check for thread access symbols
	    unsigned base, wid;
	    size_t n = 0;
	    char ss[32];
	    if (2 == sscanf(label(), "W<%u,%[r]>%zn", &base, ss, &n)
		       && n == strlen(label())) {

		  val.ptr = vpip_make_vthr_word(base, ss);
		  sym_set_value(sym_vpi, label(), val);

	    } else if (1 == sscanf(label(), "S<%u,str>%zn", &base, &n)
		       && n == strlen(label())) {

		  val.ptr = vpip_make_vthr_str_stack(base);
		  sym_set_value(sym_vpi, label(), val);

	    } else if (3 == sscanf(label(), "S<%u,vec4,%[su]%u>%zn", &base, ss, &wid, &n)
		       && n == strlen(label())) {

		  bool signed_flag = false;
		  for (char*fp = ss ;  *fp ;  fp += 1) switch (*fp) {
		      case 's':
			signed_flag = true;
			break;
		      case 'u':
			signed_flag = false;
			break;
		      default:
			break;
		  }
		  val.ptr = vpip_make_vthr_vec4_stack(base, signed_flag, wid);
		  sym_set_value(sym_vpi, label(), val);
	    }

      }

      if (!val.ptr) {
	    // check for memory word  M<mem,base,wid>
      }

      if (strcmp(label(), "null") == 0) {
	    val.ptr = vpip_make_null_const();
	    sym_set_value(sym_vpi, label(), val);
      }

      if (val.ptr) {
	    *handle = (vpiHandle) val.ptr;
	    return true;
      }

      if (mes)
	    fprintf(stderr, "unresolved vpi name lookup: %s\n", label());

      return false;
}

void compile_vpi_lookup(vpiHandle *handle, char*label)
{
      if (strcmp(label, "$time") == 0) {
	    *handle = vpip_sim_time(vpip_peek_current_scope(), false);
	    free(label);
	    return;
      }

      if (strcmp(label, "$stime") == 0) {
	    *handle = vpip_sim_time(vpip_peek_current_scope(), true);
	    free(label);
	    return;
      }

      if (strcmp(label, "$realtime") == 0) {
	    *handle = vpip_sim_realtime(vpip_peek_current_scope());
	    free(label);
	    return;
      }

      if (strcmp(label, "$simtime") == 0) {
	    *handle = vpip_sim_time(0, false);
	    free(label);
	    return;
      }

      struct vpi_handle_resolv_list_s*res
	    = new struct vpi_handle_resolv_list_s(label);

      res->handle = handle;
      resolv_submit(res);
}

/*
 * Code Label lookup
 */

struct code_label_resolv_list_s: public resolv_list_s {
      explicit code_label_resolv_list_s(char*lab, bool cptr2) : resolv_list_s(lab) {
	    code = NULL;
	    cptr2_flag = cptr2;
      }
      struct vvp_code_s *code;
      bool cptr2_flag;
      virtual bool resolve(bool mes);
};

bool code_label_resolv_list_s::resolve(bool mes)
{
      symbol_value_t val = sym_get_value(sym_codespace, label());
      if (val.ptr) {
	    if (cptr2_flag)
		  code->cptr2 = reinterpret_cast<vvp_code_t>(val.ptr);
	    else
		  code->cptr = reinterpret_cast<vvp_code_t>(val.ptr);
	    return true;
      }

      if (mes)
	    fprintf(stderr, "unresolved code label: %s\n", label());

      return false;
}

void code_label_lookup(struct vvp_code_s *code, char *label, bool cptr2)
{
      struct code_label_resolv_list_s *res
	    = new struct code_label_resolv_list_s(label, cptr2);

      res->code  = code;

      resolv_submit(res);
}

struct code_array_resolv_list_s: public resolv_list_s {
      explicit code_array_resolv_list_s(char*lab) : resolv_list_s(lab) {
	    code = NULL;
      }
      struct vvp_code_s *code;
      virtual bool resolve(bool mes);
};

bool code_array_resolv_list_s::resolve(bool mes)
{
      code->array = array_find(label());
      if (code->array != 0) {
	    return true;
      }

      if (mes)
	    fprintf(stderr, "Array unresolved: %s\n", label());

      return false;
}

static void compile_array_lookup(struct vvp_code_s*code, char*label)
{
      struct code_array_resolv_list_s *res
	    = new struct code_array_resolv_list_s(label);

      res->code  = code;

      resolv_submit(res);
}

static std::list<struct __vpiSysTaskCall*> scheduled_compiletf;

void compile_compiletf(struct __vpiSysTaskCall*obj)
{
      if (obj->defn->info.compiletf == 0)
	    return;

      scheduled_compiletf.push_back(obj);
}

/*
 * When parsing is otherwise complete, this function is called to do
 * the final stuff. Clean up deferred linking here.
 */

void compile_cleanup(void)
{
      int lnerrs = -1;
      int nerrs = 0;
      int last;

      if (verbose_flag) {
	    fprintf(stderr, " ... Linking\n");
	    fflush(stderr);
      }

      do {
	    resolv_list_s *res = resolv_list;
	    resolv_list = 0x0;
	    last = nerrs == lnerrs;
	    lnerrs = nerrs;
	    nerrs = 0;
	    while (res) {
		  resolv_list_s *cur = res;
		  res = res->next;
		  if (cur->resolve(last))
			delete cur;
		  else {
			nerrs++;
			cur->next = resolv_list;
			resolv_list = cur;
		  }
	    }
	    if (nerrs && last)
		  fprintf(stderr,
			  "compile_cleanup: %d unresolved items\n",
			  nerrs);
      } while (nerrs && !last);

      compile_errors += nerrs;

      if (verbose_flag) {
	    fprintf(stderr, " ... Removing symbol tables\n");
	    fflush(stderr);
      }

	/* After compile is complete, the vpi symbol table is no
	   longer needed. VPI objects are located by following
	   scopes. */
      delete_symbol_table(sym_vpi);
      sym_vpi = 0;

	/* Don't need the code labels. The instructions have numeric
	   pointers in them, the symbol table is no longer needed. */
      delete_symbol_table(sym_codespace);
      sym_codespace = 0;

      delete_symbol_table(sym_functors);
      sym_functors = 0;

      delete_udp_symbols();

      compile_island_cleanup();
      compile_array_cleanup();

      if (verbose_flag) {
	    fprintf(stderr, " ... Compiletf functions\n");
	    fflush(stderr);
      }

      assert(vpi_mode_flag == VPI_MODE_NONE);
      vpi_mode_flag = VPI_MODE_COMPILETF;

      while (! scheduled_compiletf.empty()) {
	    struct __vpiSysTaskCall*obj = scheduled_compiletf.front();
	    scheduled_compiletf.pop_front();
	    vpip_cur_task = obj;
	    obj->defn->info.compiletf (obj->defn->info.user_data);
	    vpip_cur_task = 0;
      }

      vpi_mode_flag = VPI_MODE_NONE;
}

void compile_vpi_symbol(const char*label, vpiHandle obj)
{
      symbol_value_t val;
      val.ptr = obj;
      sym_set_value(sym_vpi, label, val);
}

/*
 * Initialize the compiler by allocation empty symbol tables and
 * initializing the various address spaces.
 */
void compile_init(void)
{
      sym_vpi = new_symbol_table();

      sym_functors = new_symbol_table();

      sym_codespace = new_symbol_table();
      codespace_init();
}

void compile_load_vpi_module(char*name)
{
      vpip_load_module(name);
      delete[] name;
}

void compile_vpi_time_precision(long pre)
{
      vpip_set_time_precision(pre);
}

/*
 * Convert a Cr string value to double.
 *
 * The format is broken into mantissa and exponent.
 * The exponent in turn includes a sign bit.
 *
 * The mantissa is a 64bit integer value (encoded in hex).
 *
 * The exponent included the sign bit (0x4000) and the binary
 * exponent offset by 0x1000. The actual exponent is the
 * encoded exponent - 0x1000.
 *
 * The real value is sign * (mant ** exp).
 */
static bool crstring_header_test(const char*str)
{
      if ((str[0] != 'C') && (str[0] != 'c'))
	    return false;
      if ((str[1] != 'r') || (str[2] != '<'))
	    return false;

      return true;
}

bool crstring_test(const char*str)
{
      if (!crstring_header_test(str))
	    return false;
      const char*tp = strchr(str, '>');
      if (tp == 0) return false;
      if (tp[1] != 0) return false;

      if ((strspn(str+3, "0123456789abcdefmg")+3) != (size_t)(tp - str))
	  return false;

      return true;
}

double crstring_to_double(const char*label)
{
      assert(crstring_header_test(label));

      const char*cp = label+3;
      assert(*cp == 'm');
      cp += 1;
      char*ep;
      uint64_t mant = strtoull(cp, &ep, 16);
      cp = ep;
      assert(*cp == 'g');
      cp += 1;
      int exp = strtoul(cp, 0, 16);

      double tmp;
      if (mant == 0 && exp == 0x3fff) {
	    tmp = INFINITY;
      } else if (mant == 0 && exp == 0x7fff) {
	    tmp = -INFINITY;
      } else if (exp == 0x3fff) {
	    tmp = nan("");
      } else {
	    double sign = (exp & 0x4000)? -1.0 : 1.0;
	    exp &= 0x1fff;

	    tmp = sign * ldexp((double)mant, exp - 0x1000);
      }

      return tmp;
}

/*
 * Run through the arguments looking for the nodes that are
 * connected to my input ports. For each source functor that I
 * find, connect the output of that functor to the indexed
 * input by inserting myself (complete with the port number in
 * the vvp_ipoint_t) into the list that the source heads.
 *
 * If the source functor is not declared yet, then don't do
 * the link yet. Save the reference to be resolved later.
 *
 * If the source is a constant value, then set the ival of the functor
 * and skip the symbol lookup.
 */

void input_connect(vvp_net_t*fdx, unsigned port, char*label)
{
      vvp_net_ptr_t ifdx = vvp_net_ptr_t(fdx, port);

	/* Is this a vvp_vector4_t constant value? */
      if (c4string_test(label)) {

	    vvp_vector4_t tmp = c4string_to_vector4(label);

	      // Inputs that are constants are scheduled to execute as
	      // soon at the simulation starts. In Verilog, constants
	      // start propagating when the simulation starts, just
	      // like any other signal value. But letting the
	      // scheduler distribute the constant value has the
	      // additional advantage that the constant is not
	      // propagated until the network is fully linked.
	      // For constants that initialise an undriven net, we
	      // schedule execution before time 0, to make sure it
	      // occurs before any sensitive processes are started
	      // or VPI callbacks are executed.
	    if (label[0] == 'c')
		  schedule_init_vector(ifdx, tmp);
	    else
		  schedule_set_vector(ifdx, tmp);

	    free(label);
	    return;
      }

	/* Is this a vvp_vector8_t constant value? */
      if (c8string_test(label)) {

	    vvp_vector8_t tmp = c8string_to_vector8(label);
	    if (label[0] == 'c')
		  schedule_init_vector(ifdx, tmp);
	    else
		  schedule_set_vector(ifdx, tmp);

	    free(label);
	    return;
      }

	/* Handle the Cr<> constant driver, which is a real-value
	   driver. */
      if (crstring_test(label)) {

	    double tmp = crstring_to_double(label);

	    if (label[0] == 'c')
		  schedule_init_vector(ifdx, tmp);
	    else
		  schedule_set_vector(ifdx, tmp);
	    free(label);
	    return;
      }

	/* Handle the general case that this is a label for a node in
	   the vvp net. This arranges for the label to be preserved in
	   a linker list, and linked when the symbol table is
	   complete. */
      postpone_functor_input(ifdx, label);
}

void inputs_connect(vvp_net_t*fdx, unsigned argc, struct symb_s*argv)
{
      if (argc > 4) {
	    std::cerr << "XXXX argv[0] = " << argv[0].text << std::endl;
      }
      assert(argc <= 4);

      for (unsigned idx = 0;  idx < argc;  idx += 1) {

	    input_connect(fdx, idx, argv[idx].text);
      }
}

void wide_inputs_connect(vvp_wide_fun_core*core,
			 unsigned argc, struct symb_s*argv)
{
	/* Create input functors to receive values from the
	   network. These functors pass the data to the core. */
      unsigned input_functors = (argc+3) / 4;
      for (unsigned idx = 0 ;  idx < input_functors ;  idx += 1) {
	    unsigned base = idx*4;
	    unsigned trans = 4;
	    if (base+trans > argc)
		  trans = argc - base;

	    vvp_wide_fun_t*cur = new vvp_wide_fun_t(core, base);
	    vvp_net_t*ptr = new vvp_net_t;
	    ptr->fun = cur;

	    inputs_connect(ptr, trans, argv+base);
      }
}

template <class T_> void make_arith(T_ *arith, char*label,
				    unsigned argc, struct symb_s*argv)
{
      vvp_net_t* ptr = new vvp_net_t;
      ptr->fun = arith;

      define_functor_symbol(label, ptr);
      free(label);

      assert(argc == 2);
      inputs_connect(ptr, argc, argv);

      free(argv);
}

void compile_arith_cast_int(char*label, long width,
                            unsigned argc, struct symb_s*argv)
{
      vvp_arith_cast_int*arith = new vvp_arith_cast_int((unsigned) width);

      vvp_net_t* ptr = new vvp_net_t;
      ptr->fun = arith;

      define_functor_symbol(label, ptr);
      free(label);

      assert(argc == 1);
      inputs_connect(ptr, argc, argv);
      free(argv);
}

void compile_arith_cast_vec2(char*label, long width,
			     unsigned argc, struct symb_s*argv)
{
      vvp_arith_cast_vec2*arith = new vvp_arith_cast_vec2((unsigned) width);

      vvp_net_t* ptr = new vvp_net_t;
      ptr->fun = arith;

      define_functor_symbol(label, ptr);
      free(label);

      assert(argc == 1);
      inputs_connect(ptr, argc, argv);
      free(argv);
}

void compile_arith_cast_real(char*label, bool signed_flag,
                             unsigned argc, struct symb_s*argv)
{
      vvp_arith_cast_real*arith = new vvp_arith_cast_real(signed_flag);

      vvp_net_t* ptr = new vvp_net_t;
      ptr->fun = arith;

      define_functor_symbol(label, ptr);
      free(label);

      assert(argc == 1);
      inputs_connect(ptr, argc, argv);
      free(argv);
}

void compile_arith_abs(char*label, unsigned argc, struct symb_s*argv)
{
      vvp_arith_abs*arith = new vvp_arith_abs;

      vvp_net_t* ptr = new vvp_net_t;
      ptr->fun = arith;

      define_functor_symbol(label, ptr);
      free(label);

      assert(argc == 1);
      inputs_connect(ptr, argc, argv);
      free(argv);
}

void compile_arith_div(char*label, long wid, bool signed_flag,
		       unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    const char *suffix = "";
	    if (signed_flag) suffix = ".s";
	    fprintf(stderr, "%s; .arith/div%s has wrong number of "
	                    "symbols\n", label, suffix);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_arith_div(wid, signed_flag);
      make_arith(arith, label, argc, argv);
}

void compile_arith_div_r(char*label, unsigned argc, struct symb_s*argv)
{
      if (argc != 2) {
	    fprintf(stderr, "%s; .arith/divr has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_real_ *arith = new vvp_arith_div_real;
      make_arith(arith, label, argc, argv);
}

void compile_arith_mod(char*label, long wid, bool signed_flag,
		       unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .arith/mod has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_arith_mod(wid, signed_flag);

      make_arith(arith, label, argc, argv);
}

void compile_arith_mod_r(char*label, unsigned argc, struct symb_s*argv)
{
      if (argc != 2) {
	    fprintf(stderr, "%s .arith/mod.r has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_real_ *arith = new vvp_arith_mod_real;
      make_arith(arith, label, argc, argv);
}

void compile_arith_mult(char*label, long wid,
			unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .arith/mult has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_arith_mult(wid);
      make_arith(arith, label, argc, argv);
}

void compile_arith_mult_r(char*label, unsigned argc, struct symb_s*argv)
{
      if (argc != 2) {
	    fprintf(stderr, "%s .arith/mult.r has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_real_ *arith = new vvp_arith_mult_real;
      make_arith(arith, label, argc, argv);
}


void compile_arith_pow(char*label, long wid, bool signed_flag,
		       unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    const char *suffix = "";
	    if (signed_flag) suffix = ".s";
	    fprintf(stderr, "%s .arith/pow%s has wrong number of "
	                    "symbols\n", label, suffix);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_arith_pow(wid, signed_flag);
      make_arith(arith, label, argc, argv);
}

void compile_arith_pow_r(char*label, unsigned argc, struct symb_s*argv)
{
      if (argc != 2) {
	    fprintf(stderr, "%s .arith/pow.r has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_real_ *arith = new vvp_arith_pow_real;
      make_arith(arith, label, argc, argv);
}

void compile_arith_sub(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .arith/sub has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_arith_sub(wid);
      make_arith(arith, label, argc, argv);
}

void compile_arith_sub_r(char*label, unsigned argc, struct symb_s*argv)
{
      if (argc != 2) {
	    fprintf(stderr, "%s; .arith/sub.r has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_real_ *arith = new vvp_arith_sub_real;
      make_arith(arith, label, argc, argv);
}

void compile_arith_sum(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .arith/sum has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_arith_sum(wid);
      make_arith(arith, label, argc, argv);
}

void compile_arith_sum_r(char*label, unsigned argc, struct symb_s*argv)
{
      if (argc != 2) {
	    fprintf(stderr, "%s .arith/sum.r has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_real_ *arith = new vvp_arith_sum_real;
      make_arith(arith, label, argc, argv);
}

void compile_cmp_eeq(char*label, long wid,
		     unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .cmp/eeq has wrong number of symbols\n",label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_cmp_eeq(wid);

      make_arith(arith, label, argc, argv);
}

void compile_cmp_nee(char*label, long wid,
		     unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .cmp/eeq has wrong number of symbols\n",label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_cmp_nee(wid);

      make_arith(arith, label, argc, argv);
}

void compile_cmp_eq(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .cmp/eq has wrong number of symbols\n",label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_cmp_eq(wid);
      make_arith(arith, label, argc, argv);
}

void compile_cmp_eqx(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .cmp/eqx has wrong number of symbols\n",label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_cmp_eqx(wid);
      make_arith(arith, label, argc, argv);
}

void compile_cmp_eqz(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .cmp/eqz has wrong number of symbols\n",label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_cmp_eqz(wid);
      make_arith(arith, label, argc, argv);
}

void compile_cmp_eq_r(char*label, unsigned argc, struct symb_s*argv)
{
      if (argc != 2) {
	    fprintf(stderr, "%s .cmp/eq.r has wrong number of symbols\n",label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_real_ *arith = new vvp_cmp_eq_real;
      make_arith(arith, label, argc, argv);
}

void compile_cmp_ne(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .cmp/ne has wrong number of symbols\n",label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_cmp_ne(wid);
      make_arith(arith, label, argc, argv);
}

void compile_cmp_ne_r(char*label, unsigned argc, struct symb_s*argv)
{
      if (argc != 2) {
	    fprintf(stderr, "%s .cmp/ne.r has wrong number of symbols\n",label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_real_ *arith = new vvp_cmp_ne_real;
      make_arith(arith, label, argc, argv);
}

void compile_cmp_ge(char*label, long wid, bool signed_flag,
		    unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .cmp/ge has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_cmp_ge(wid, signed_flag);

      make_arith(arith, label, argc, argv);
}

void compile_cmp_ge_r(char*label, unsigned argc, struct symb_s*argv)
{
      if (argc != 2) {
	    fprintf(stderr, "%s .cmp/ge.r has wrong number of symbols\n",label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_real_ *arith = new vvp_cmp_ge_real;
      make_arith(arith, label, argc, argv);
}

void compile_cmp_gt(char*label, long wid, bool signed_flag,
		    unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .cmp/gt has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_cmp_gt(wid, signed_flag);

      make_arith(arith, label, argc, argv);
}

void compile_cmp_gt_r(char*label, unsigned argc, struct symb_s*argv)
{
      if (argc != 2) {
	    fprintf(stderr, "%s .cmp/gt.r has wrong number of symbols\n",label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_real_ *arith = new vvp_cmp_gt_real;
      make_arith(arith, label, argc, argv);
}

void compile_cmp_weq(char*label, long wid,
		     unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .cmp/weq has wrong number of symbols\n",label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_cmp_weq(wid);

      make_arith(arith, label, argc, argv);
}

void compile_cmp_wne(char*label, long wid,
		     unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .cmp/wne has wrong number of symbols\n",label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_cmp_wne(wid);

      make_arith(arith, label, argc, argv);
}


void compile_delay(char*label, unsigned width,
                   vvp_delay_t*delay, struct symb_s arg)
{
      vvp_net_t*net = new vvp_net_t;
      vvp_fun_delay*obj = new vvp_fun_delay(net, width, *delay);
      net->fun = obj;

      delete delay;

      input_connect(net, 0, arg.text);

      define_functor_symbol(label, net);
      free(label);
}

void compile_delay(char*label, unsigned width,
                   unsigned argc, struct symb_s*argv, bool ignore_decay)
{
      vvp_delay_t stub (0, 0, 0);
      if (ignore_decay) stub.set_ignore_decay();
      vvp_net_t*net = new vvp_net_t;
      vvp_fun_delay*obj = new vvp_fun_delay(net, width, stub);
      net->fun = obj;

      inputs_connect(net, argc, argv);
      free(argv);

      define_functor_symbol(label, net);
      free(label);
}

/*
 * Extend nodes.
 */
void compile_extend_signed(char*label, long wid, struct symb_s arg)
{
      assert(wid >= 0);

      vvp_fun_extend_signed*fun = new vvp_fun_extend_signed(wid);
      vvp_net_t*ptr = new vvp_net_t;
      ptr->fun = fun;

      define_functor_symbol(label, ptr);
      free(label);

      input_connect(ptr, 0, arg.text);
}

struct __vpiModPath* compile_modpath(char*label, unsigned width,
                                     struct symb_s drv, struct symb_s dest)
{
      vvp_net_t*net = new vvp_net_t;
      vvp_fun_modpath*obj = new vvp_fun_modpath(net, width);
      net->fun = obj;

      input_connect(net, 0, drv.text);

      define_functor_symbol(label, net);

      __vpiModPath*modpath = vpip_make_modpath(net);

      compile_vpi_lookup(&modpath->path_term_out.expr, dest.text);

      free(label);

      modpath->modpath = obj;
      return modpath;
}

static struct __vpiModPathSrc*make_modpath_src(struct __vpiModPath*path,
					       char edge,
					       const struct symb_s&src,
					       struct numbv_s&vals,
					       bool ifnone)
{
      vvp_fun_modpath*dst = path->modpath;

      vvp_time64_t use_delay[12];
      assert(vals.cnt == 12);
      for (unsigned idx = 0 ; idx < vals.cnt ;  idx += 1) {
	    use_delay[idx] = vals.nvec[idx];
      }

      numbv_clear(&vals);

      vvp_fun_modpath_src*obj = 0;

      int vpi_edge = vpiNoEdge;
      if (edge == 0) {
	    obj = new vvp_fun_modpath_src(use_delay);

      } else {
	    bool posedge, negedge;
	    switch (edge) {
		case '+':
		  vpi_edge = vpiPosedge;
		  posedge = true;
		  negedge = false;
		  break;
		case '-':
		  vpi_edge = vpiNegedge;
		  posedge = false;
		  negedge = true;
		  break;
#if 0
		case '*':
		  posedge = true;
		  negedge = true;
		  break;
#endif
		default:
		  posedge = false;
		  negedge = false;
		  fprintf(stderr, "Unknown edge identifier %c(%d).\n", edge,
		          edge);
		  assert(0);
	    }
	    obj = new vvp_fun_modpath_edge(use_delay, posedge, negedge);
      }

      vvp_net_t*net = new vvp_net_t;
      struct __vpiModPathSrc* srcobj = vpip_make_modpath_src(path, net) ;
      vpip_attach_to_current_scope(srcobj);
      net->fun = obj;

	/* Save the vpiEdge directory into the input path term. */
      srcobj->path_term_in.edge = vpi_edge;
      input_connect(net, 0, src.text);
      dst->add_modpath_src(obj, ifnone);

      return srcobj;
}

void compile_modpath_src(struct __vpiModPath*dst, char edge,
			 const struct symb_s&src,
			 struct numbv_s&vals,
			 const struct symb_s&condit_src,
			 const struct symb_s&path_term_in)
{
      struct __vpiModPathSrc*obj =
	    make_modpath_src(dst, edge, src, vals, false);
      input_connect(obj->net, 1, condit_src.text);
      compile_vpi_lookup(&obj->path_term_in.expr, path_term_in.text);
}

void compile_modpath_src(struct __vpiModPath*dst, char edge,
			 const struct symb_s&src,
			 struct numbv_s&vals,
			 int condit_src,
			 const struct symb_s&path_term_in,
			 bool ifnone)
{
      assert(condit_src == 0);
      struct __vpiModPathSrc*obj =
	    make_modpath_src(dst, edge, src, vals, ifnone);
      compile_vpi_lookup(&obj->path_term_in.expr, path_term_in.text);
}

/*
 * A .shift/l statement creates an array of functors for the
 * width. The 0 input is the data vector to be shifted and the 1 input
 * is the amount of the shift. An unconnected shift amount is set to 0.
 */
void compile_shiftl(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      vvp_arith_ *arith = new vvp_shiftl(wid);
      make_arith(arith, label, argc, argv);
}

void compile_shiftr(char*label, long wid, bool signed_flag,
		    unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      vvp_arith_ *arith = new vvp_shiftr(wid, signed_flag);
      make_arith(arith, label, argc, argv);
}

void compile_resolver(char*label, char*type, unsigned argc, struct symb_s*argv)
{
      vvp_net_t*net = new vvp_net_t;

      resolv_core*core = 0;

      if (strcmp(type,"tri") == 0) {
	    core = new resolv_tri(argc, net, vvp_scalar_t(BIT4_Z, 0,0));

      } else if (strcmp(type,"tri0") == 0) {
	    core = new resolv_tri(argc, net, vvp_scalar_t(BIT4_0, 5,5));

      } else if (strcmp(type,"tri1") == 0) {
	    core = new resolv_tri(argc, net, vvp_scalar_t(BIT4_1, 5,5));

      } else if (strcmp(type,"triand") == 0) {
	    core = new resolv_triand(argc, net);

      } else if (strcmp(type,"trior") == 0) {
	    core = new resolv_trior(argc, net);

      } else {
	    fprintf(stderr, "invalid resolver type: %s\n", type);
	    compile_errors += 1;
            delete net;
      }

      if (core) {
	    net->fun = core;
	    define_functor_symbol(label, net);

            for (unsigned base = 0 ;  base < argc ;  base += 4) {
	          unsigned nports = argc - base;
                  if (nports > 4)
                        nports = 4;

                  if (base > 0) {
                        net = new vvp_net_t;
                        net->fun = new resolv_extend(core, base);
                  }
                  inputs_connect(net, nports, argv+base);
            }
      }
      free(type);
      free(label);
      free(argv);
}

void compile_udp_def(int sequ, char *label, char *name,
		     unsigned nin, unsigned init, char **table)
{
      if (sequ) {
	    vvp_bit4_t init4;
	    if (init == 0)
		  init4 = BIT4_0;
	    else if (init == 1)
		  init4 = BIT4_1;
	    else
		  init4 = BIT4_X;

	    vvp_udp_seq_s *u = new vvp_udp_seq_s(label, name, nin, init4);
	    u->compile_table(table);
      } else {
	    vvp_udp_comb_s *u = new vvp_udp_comb_s(label, name, nin);
	    u->compile_table(table);
      }
      free(label);
}

char **compile_udp_table(char **table, char *row)
{
  if (table)
    assert(strlen(*table)==strlen(row));

  char **tt;
  for (tt = table; tt && *tt; tt++) { }
  int n = (tt-table) + 2;

  table = (char**)realloc(table, n*sizeof(char*));
  table[n-2] = row;
  table[n-1] = 0x0;

  return table;
}


/*
 * The parser uses this function to compile and link an executable
 * opcode. I do this by looking up the opcode in the opcode_table. The
 * table gives the operand structure that is acceptable, so I can
 * process the operands here as well.
 */
void compile_code(char*label, char*mnem, comp_operands_t opa)
{
	/* First, I can give the label a value that is the current
	   codespace pointer. Don't need the text of the label after
	   this is done. */
      if (label)
	    compile_codelabel(label);

	/* Lookup the opcode in the opcode table. */
      struct opcode_table_s*op = (struct opcode_table_s*)
	    bsearch(mnem, opcode_table, opcode_count,
		    sizeof(struct opcode_table_s), &opcode_compare);
      if (op == 0) {
	    yyerror("Invalid opcode");
	    compile_errors += 1;
	    return;
      }

      assert(op);

	/* Build up the code from the information about the opcode and
	   the information from the compiler. */
      vvp_code_t code = codespace_allocate();
      code->opcode = op->opcode;

      if (op->argc != (opa? opa->argc : 0)) {
	    yyerror("operand count");
	    compile_errors += 1;
	    return;
      }

	/* Pull the operands that the instruction expects from the
	   list that the parser supplied. */

      for (unsigned idx = 0 ;  idx < op->argc ;  idx += 1) {

	    switch (op->argt[idx]) {
		case OA_NONE:
		  break;

		case OA_ARR_PTR:
		  if (opa->argv[idx].ltype != L_SYMB) {
			yyerror("operand format");
			break;
		  }

		  compile_array_lookup(code, opa->argv[idx].symb.text);
		  break;

		case OA_BIT1:
		  if (opa->argv[idx].ltype != L_NUMB) {
			yyerror("operand format");
			break;
		  }
		  code->bit_idx[0] = opa->argv[idx].numb;
		  break;

		case OA_BIT2:
		  if (opa->argv[idx].ltype != L_NUMB) {
			yyerror("operand format");
			break;
		  }
		  code->bit_idx[1] = opa->argv[idx].numb;
		  break;

		case OA_CODE_PTR:
		case OA_CODE_PTR2:
		  if (opa->argv[idx].ltype != L_SYMB) {
			yyerror("operand format");
			break;
		  }

		  assert(opa->argv[idx].symb.idx == 0);
		  code_label_lookup(code, opa->argv[idx].symb.text,
				    op->argt[idx]==OA_CODE_PTR2);
		  break;

		case OA_FUNC_PTR:
		    /* The operand is a functor. Resolve the label to
		       a functor pointer, or postpone the resolution
		       if it is not defined yet. */
		  if (opa->argv[idx].ltype != L_SYMB) {
			yyerror("operand format");
			break;
		  }

		  functor_ref_lookup(&code->net, opa->argv[idx].symb.text);
		  break;

		case OA_FUNC_PTR2:
		    /* The operand is a functor. Resolve the label to
		       a functor pointer, or postpone the resolution
		       if it is not defined yet. */
		  if (opa->argv[idx].ltype != L_SYMB) {
			yyerror("operand format");
			break;
		  }

		  functor_ref_lookup(&code->net2, opa->argv[idx].symb.text);
		  break;

		case OA_NUMBER:
		  if (opa->argv[idx].ltype != L_NUMB) {
			yyerror("operand format");
			break;
		  }

		  code->number = opa->argv[idx].numb;
		  break;

		case OA_VPI_PTR:
		    /* The operand is a functor. Resolve the label to
		       a functor pointer, or postpone the resolution
		       if it is not defined yet. */
		  if (opa->argv[idx].ltype != L_SYMB) {
			yyerror("operand format");
			break;
		  }

		  compile_vpi_lookup(&code->handle, opa->argv[idx].symb.text);
		  break;

		case OA_STRING:
		  if (opa->argv[idx].ltype != L_STRING) {
			yyerror("operand format");
			break;
		  }

		  code->text = opa->argv[idx].text;
		  break;
	    }
      }

      free(opa);

      free(mnem);
}

void compile_codelabel(char*label)
{
      symbol_value_t val;
      vvp_code_t ptr = codespace_next();

      val.ptr = ptr;
      sym_set_value(sym_codespace, label, val);

      free(label);
}

void compile_file_line(char*label, long file_idx, long lineno,
                       char*description)
{
      if (label) compile_codelabel(label);

	/* Create an instruction in the code space. */
      vvp_code_t code = codespace_allocate();
      code->opcode = &of_FILE_LINE;

	/* Create a vpiHandle that contains the information. */
      code->handle = vpip_build_file_line(description, file_idx, lineno);
      assert(code->handle);

	/* Done with the lexor-allocated name string. */
      delete[] description;
}

void compile_vpi_call(char*label, char*name,
                      bool func_as_task_err, bool func_as_task_warn,
                      long file_idx, long lineno,
                      unsigned argc, vpiHandle*argv,
		      unsigned vec4_stack, unsigned real_stack, unsigned string_stack)
{
      if (label)
	    compile_codelabel(label);

	/* Create an instruction in the code space. */
      vvp_code_t code = codespace_allocate();
      code->opcode = &of_VPI_CALL;

	/* Create a vpiHandle that bundles the call information, and
	   store that handle in the instruction. */
      code->handle = vpip_build_vpi_call(name, 0, 0,
					 0, func_as_task_err, func_as_task_warn,
                                         argc, argv, vec4_stack, real_stack, string_stack,
					 file_idx, lineno);
      if (code->handle == 0)
	    compile_errors += 1;

	/* Done with the lexor-allocated name string. */
      delete[] name;
}

void compile_vpi_func_call(char*label, char*name,
			   int val_type, unsigned val_wid,
			   long file_idx, long lineno,
			   unsigned argc, vpiHandle*argv,
			   unsigned vec4_stack,
			   unsigned real_stack,
			   unsigned string_stack)
{
      if (label)
	    compile_codelabel(label);

	/* Create an instruction in the code space. */
      vvp_code_t code = codespace_allocate();
      code->opcode = &of_VPI_CALL;

	/* Create a vpiHandle that bundles the call information, and
	   store that handle in the instruction. */
      code->handle = vpip_build_vpi_call(name, val_type, val_wid,
					 0, true, false,
                                         argc, argv, vec4_stack, real_stack, string_stack,
					 file_idx, lineno);
      if (code->handle == 0)
	    compile_errors += 1;

	/* Done with the lexor-allocated name string. */
      delete[] name;
}

/*
 * When the parser finds a thread statement, I create a new thread
 * with the start address referenced by the program symbol passed to
 * me.
 */
void compile_thread(char*start_sym, char*flag)
{
      bool push_flag = false;

      symbol_value_t tmp = sym_get_value(sym_codespace, start_sym);
      vvp_code_t pc = reinterpret_cast<vvp_code_t>(tmp.ptr);
      if (pc == 0) {
	    yyerror("unresolved address");
	    return;
      }

      if (flag && (strcmp(flag,"$push") == 0))
	    push_flag = true;

      vthread_t thr = vthread_new(pc, vpip_peek_current_scope());

      if (flag && (strcmp(flag,"$init") == 0))
	    schedule_init_vthread(thr);
      else if (flag && (strcmp(flag,"$final") == 0))
	    schedule_final_vthread(thr);
      else
	    schedule_vthread(thr, 0, push_flag);

      free(start_sym);
      free(flag);
}

void compile_param_logic(char*label, char*name, char*value, bool signed_flag,
                         bool local_flag,
                         long file_idx, long lineno)
{
      vvp_vector4_t value4 = c4string_to_vector4(value);
      vpiHandle obj = vpip_make_binary_param(name, value4, signed_flag,
                                              local_flag, file_idx, lineno);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      free(value);
}

void compile_param_string(char*label, char*name, char*value,
                          bool local_flag,
                          long file_idx, long lineno)
{
	// name and value become owned by vpip_make_string_param
      vpiHandle obj = vpip_make_string_param(name, value, local_flag, file_idx, lineno);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
}

void compile_param_real(char*label, char*name, char*value,
                        bool local_flag,
                        long file_idx, long lineno)
{
      double dvalue = crstring_to_double(value);
      vpiHandle obj = vpip_make_real_param(name, dvalue, local_flag, file_idx, lineno);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      free(value);
}

void compile_island(char*label, char*type)
{
      if (strcmp(type,"tran") == 0)
	    compile_island_tran(label);
      else
	    assert(0);

      free(type);
}
