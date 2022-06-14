#ifndef IVL_codes_H
#define IVL_codes_H
/*
 * Copyright (c) 2001-2021 Stephen Williams (steve@icarus.com)
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
# include  "vvp_net.h"
# include  "array.h"
# include  "vthread.h"

typedef bool (*vvp_code_fun)(vthread_t thr, vvp_code_t code);

/*
 * These functions are implementations of executable op-codes. The
 * implementation lives in the vthread.cc file so that they have
 * access to the thread context.
 */
extern bool of_ABS_WR(vthread_t thr, vvp_code_t code);
extern bool of_ADD(vthread_t thr, vvp_code_t code);
extern bool of_ADD_WR(vthread_t thr, vvp_code_t code);
extern bool of_ADDI(vthread_t thr, vvp_code_t code);
extern bool of_ALLOC(vthread_t thr, vvp_code_t code);
extern bool of_AND(vthread_t thr, vvp_code_t code);
extern bool of_ANDR(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_AR(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_ARD(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_ARE(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_VEC4D(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_VEC4E(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_VEC4_A_D(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_VEC4_A_E(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_VEC4_OFF_D(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_VEC4_OFF_E(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_WR(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_WRD(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_WRE(vthread_t thr, vvp_code_t code);
extern bool of_BLEND(vthread_t thr, vvp_code_t code);
extern bool of_BLEND_WR(vthread_t thr, vvp_code_t code);
extern bool of_BREAKPOINT(vthread_t thr, vvp_code_t code);
extern bool of_CALLF_OBJ(vthread_t thr, vvp_code_t code);
extern bool of_CALLF_REAL(vthread_t thr, vvp_code_t code);
extern bool of_CALLF_STR(vthread_t thr, vvp_code_t code);
extern bool of_CALLF_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_CALLF_VOID(vthread_t thr, vvp_code_t code);
extern bool of_CASSIGN_LINK(vthread_t thr, vvp_code_t code);
extern bool of_CASSIGN_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_CASSIGN_VEC4_OFF(vthread_t thr, vvp_code_t code);
extern bool of_CASSIGN_WR(vthread_t thr, vvp_code_t code);
extern bool of_CAST2(vthread_t thr, vvp_code_t code);
extern bool of_CAST_VEC2_DAR(vthread_t thr, vvp_code_t code);
extern bool of_CAST_VEC4_DAR(vthread_t thr, vvp_code_t code);
extern bool of_CAST_VEC4_STR(vthread_t thr, vvp_code_t code);
extern bool of_CMPE(vthread_t thr, vvp_code_t code);
extern bool of_CMPIE(vthread_t thr, vvp_code_t code);
extern bool of_CMPINE(vthread_t thr, vvp_code_t code);
extern bool of_CMPNE(vthread_t thr, vvp_code_t code);
extern bool of_CMPS(vthread_t thr, vvp_code_t code);
extern bool of_CMPIS(vthread_t thr, vvp_code_t code);
extern bool of_CMPSTR(vthread_t thr, vvp_code_t code);
extern bool of_CMPU(vthread_t thr, vvp_code_t code);
extern bool of_CMPIU(vthread_t thr, vvp_code_t code);
extern bool of_CMPWE(vthread_t thr, vvp_code_t code);
extern bool of_CMPWNE(vthread_t thr, vvp_code_t code);
extern bool of_CMPWR(vthread_t thr, vvp_code_t code);
extern bool of_CMPX(vthread_t thr, vvp_code_t code);
extern bool of_CMPZ(vthread_t thr, vvp_code_t code);
extern bool of_CONCAT_STR(vthread_t thr, vvp_code_t code);
extern bool of_CONCATI_STR(vthread_t thr, vvp_code_t code);
extern bool of_CONCAT_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_CONCATI_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_CVT_RV(vthread_t thr, vvp_code_t code);
extern bool of_CVT_RV_S(vthread_t thr, vvp_code_t code);
extern bool of_CVT_SR(vthread_t thr, vvp_code_t code);
extern bool of_CVT_UR(vthread_t thr, vvp_code_t code);
extern bool of_CVT_VR(vthread_t thr, vvp_code_t code);
extern bool of_DEASSIGN(vthread_t thr, vvp_code_t code);
extern bool of_DEASSIGN_WR(vthread_t thr, vvp_code_t code);
extern bool of_DEBUG_THR(vthread_t thr, vvp_code_t code);
extern bool of_DELAY(vthread_t thr, vvp_code_t code);
extern bool of_DELAYX(vthread_t thr, vvp_code_t code);
extern bool of_DELETE_ELEM(vthread_t thr, vvp_code_t code);
extern bool of_DELETE_OBJ(vthread_t thr, vvp_code_t code);
extern bool of_DELETE_TAIL(vthread_t thr, vvp_code_t code);
extern bool of_DISABLE(vthread_t thr, vvp_code_t code);
extern bool of_DISABLE_FLOW(vthread_t thr, vvp_code_t code);
extern bool of_DISABLE_FORK(vthread_t thr, vvp_code_t code);
extern bool of_DIV(vthread_t thr, vvp_code_t code);
extern bool of_DIV_S(vthread_t thr, vvp_code_t code);
extern bool of_DIV_WR(vthread_t thr, vvp_code_t code);
extern bool of_DUP_OBJ(vthread_t thr, vvp_code_t code);
extern bool of_DUP_REAL(vthread_t thr, vvp_code_t code);
extern bool of_DUP_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_END(vthread_t thr, vvp_code_t code);
extern bool of_EVENT(vthread_t thr, vvp_code_t code);
extern bool of_EVENT_NB(vthread_t thr, vvp_code_t code);
extern bool of_EVCTL(vthread_t thr, vvp_code_t code);
extern bool of_EVCTLC(vthread_t thr, vvp_code_t code);
extern bool of_EVCTLI(vthread_t thr, vvp_code_t code);
extern bool of_EVCTLS(vthread_t thr, vvp_code_t code);
extern bool of_FILE_LINE(vthread_t thr, vvp_code_t code);
extern bool of_FLAG_GET_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_FLAG_INV(vthread_t thr, vvp_code_t code);
extern bool of_FLAG_MOV(vthread_t thr, vvp_code_t code);
extern bool of_FLAG_OR(vthread_t thr, vvp_code_t code);
extern bool of_FLAG_SET_IMM(vthread_t thr, vvp_code_t code);
extern bool of_FLAG_SET_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_FORCE_LINK(vthread_t thr, vvp_code_t code);
extern bool of_FORCE_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_FORCE_VEC4_OFF(vthread_t thr, vvp_code_t code);
extern bool of_FORCE_VEC4_OFF_D(vthread_t thr, vvp_code_t code);
extern bool of_FORCE_WR(vthread_t thr, vvp_code_t code);
extern bool of_FORK(vthread_t thr, vvp_code_t code);
extern bool of_FREE(vthread_t thr, vvp_code_t code);
extern bool of_INV(vthread_t thr, vvp_code_t code);
extern bool of_IX_ADD(vthread_t thr, vvp_code_t code);
extern bool of_IX_GETV(vthread_t thr, vvp_code_t code);
extern bool of_IX_GETV_S(vthread_t thr, vvp_code_t code);
extern bool of_IX_LOAD(vthread_t thr, vvp_code_t code);
extern bool of_IX_MOV(vthread_t thr, vvp_code_t code);
extern bool of_IX_MUL(vthread_t thr, vvp_code_t code);
extern bool of_IX_SUB(vthread_t thr, vvp_code_t code);
extern bool of_IX_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_IX_VEC4_S(vthread_t thr, vvp_code_t code);
extern bool of_JMP(vthread_t thr, vvp_code_t code);
extern bool of_JMP0(vthread_t thr, vvp_code_t code);
extern bool of_JMP0XZ(vthread_t thr, vvp_code_t code);
extern bool of_JMP1(vthread_t thr, vvp_code_t code);
extern bool of_JMP1XZ(vthread_t thr, vvp_code_t code);
extern bool of_JOIN(vthread_t thr, vvp_code_t code);
extern bool of_JOIN_DETACH(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_AR(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_REAL(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_DAR_R(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_DAR_STR(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_DAR_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_OBJ(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_OBJA(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_STR(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_STRA(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_VEC4A(vthread_t thr, vvp_code_t code);
extern bool of_MAX_WR(vthread_t thr, vvp_code_t code);
extern bool of_MIN_WR(vthread_t thr, vvp_code_t code);
extern bool of_MOD(vthread_t thr, vvp_code_t code);
extern bool of_MOD_S(vthread_t thr, vvp_code_t code);
extern bool of_MOD_WR(vthread_t thr, vvp_code_t code);
extern bool of_MUL(vthread_t thr, vvp_code_t code);
extern bool of_MULI(vthread_t thr, vvp_code_t code);
extern bool of_MUL_WR(vthread_t thr, vvp_code_t code);
extern bool of_NAND(vthread_t thr, vvp_code_t code);
extern bool of_NANDR(vthread_t thr, vvp_code_t code);
extern bool of_NEW_COBJ(vthread_t thr, vvp_code_t code);
extern bool of_NEW_DARRAY(vthread_t thr, vvp_code_t code);
extern bool of_NOOP(vthread_t thr, vvp_code_t code);
extern bool of_NOR(vthread_t thr, vvp_code_t code);
extern bool of_NORR(vthread_t thr, vvp_code_t code);
extern bool of_NULL(vthread_t thr, vvp_code_t code);
extern bool of_OR(vthread_t thr, vvp_code_t code);
extern bool of_ORR(vthread_t thr, vvp_code_t code);
extern bool of_PAD_S(vthread_t thr, vvp_code_t code);
extern bool of_PAD_U(vthread_t thr, vvp_code_t code);
extern bool of_PART_S(vthread_t thr, vvp_code_t code);
extern bool of_PART_U(vthread_t thr, vvp_code_t code);
extern bool of_PARTI_S(vthread_t thr, vvp_code_t code);
extern bool of_PARTI_U(vthread_t thr, vvp_code_t code);
extern bool of_POP_OBJ(vthread_t thr, vvp_code_t code);
extern bool of_POP_REAL(vthread_t thr, vvp_code_t code);
extern bool of_POP_STR(vthread_t thr, vvp_code_t code);
extern bool of_POP_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_POW(vthread_t thr, vvp_code_t code);
extern bool of_POW_S(vthread_t thr, vvp_code_t code);
extern bool of_POW_WR(vthread_t thr, vvp_code_t code);
extern bool of_QINSERT_REAL(vthread_t thr, vvp_code_t code);
extern bool of_QINSERT_STR(vthread_t thr, vvp_code_t code);
extern bool of_QINSERT_V(vthread_t thr, vvp_code_t code);
extern bool of_QPOP_B_REAL(vthread_t thr, vvp_code_t code);
extern bool of_QPOP_B_STR(vthread_t thr, vvp_code_t code);
extern bool of_QPOP_B_V(vthread_t thr, vvp_code_t code);
extern bool of_QPOP_F_REAL(vthread_t thr, vvp_code_t code);
extern bool of_QPOP_F_STR(vthread_t thr, vvp_code_t code);
extern bool of_QPOP_F_V(vthread_t thr, vvp_code_t code);
extern bool of_PROP_OBJ(vthread_t thr, vvp_code_t code);
extern bool of_PROP_R(vthread_t thr, vvp_code_t code);
extern bool of_PROP_STR(vthread_t thr, vvp_code_t code);
extern bool of_PROP_V(vthread_t thr, vvp_code_t code);
extern bool of_PUSHI_STR(vthread_t thr, vvp_code_t code);
extern bool of_PUSHI_REAL(vthread_t thr, vvp_code_t code);
extern bool of_PUSHI_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_PUSHV_STR(vthread_t thr, vvp_code_t code);
extern bool of_PUTC_STR_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_RELEASE_NET(vthread_t thr, vvp_code_t code);
extern bool of_RELEASE_REG(vthread_t thr, vvp_code_t code);
extern bool of_RELEASE_WR(vthread_t thr, vvp_code_t code);
extern bool of_REPLICATE(vthread_t thr, vvp_code_t code);
extern bool of_RET_REAL(vthread_t thr, vvp_code_t code);
extern bool of_RET_STR(vthread_t thr, vvp_code_t code);
extern bool of_RET_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_RETLOAD_REAL(vthread_t thr, vvp_code_t code);
extern bool of_RETLOAD_STR(vthread_t thr, vvp_code_t code);
extern bool of_RETLOAD_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_SCOPY(vthread_t thr, vvp_code_t code);
extern bool of_SET_DAR_OBJ_REAL(vthread_t thr, vvp_code_t code);
extern bool of_SET_DAR_OBJ_STR(vthread_t thr, vvp_code_t code);
extern bool of_SET_DAR_OBJ_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_SHIFTL(vthread_t thr, vvp_code_t code);
extern bool of_SHIFTR(vthread_t thr, vvp_code_t code);
extern bool of_SHIFTR_S(vthread_t thr, vvp_code_t code);
extern bool of_SPLIT_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_STORE_DAR_R(vthread_t thr, vvp_code_t code);
extern bool of_STORE_DAR_STR(vthread_t thr, vvp_code_t code);
extern bool of_STORE_DAR_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_STORE_OBJ(vthread_t thr, vvp_code_t code);
extern bool of_STORE_OBJA(vthread_t thr, vvp_code_t code);
extern bool of_STORE_PROP_OBJ(vthread_t thr, vvp_code_t code);
extern bool of_STORE_PROP_R(vthread_t thr, vvp_code_t code);
extern bool of_STORE_PROP_STR(vthread_t thr, vvp_code_t code);
extern bool of_STORE_PROP_V(vthread_t thr, vvp_code_t code);
extern bool of_STORE_QB_R(vthread_t thr, vvp_code_t code);
extern bool of_STORE_QB_STR(vthread_t thr, vvp_code_t code);
extern bool of_STORE_QB_V(vthread_t thr, vvp_code_t code);
extern bool of_STORE_QDAR_R(vthread_t thr, vvp_code_t code);
extern bool of_STORE_QDAR_STR(vthread_t thr, vvp_code_t code);
extern bool of_STORE_QDAR_V(vthread_t thr, vvp_code_t code);
extern bool of_STORE_QF_R(vthread_t thr, vvp_code_t code);
extern bool of_STORE_QF_STR(vthread_t thr, vvp_code_t code);
extern bool of_STORE_QF_V(vthread_t thr, vvp_code_t code);
extern bool of_STORE_QOBJ_R(vthread_t thr, vvp_code_t code);
extern bool of_STORE_QOBJ_STR(vthread_t thr, vvp_code_t code);
extern bool of_STORE_QOBJ_V(vthread_t thr, vvp_code_t code);
extern bool of_STORE_REAL(vthread_t thr, vvp_code_t code);
extern bool of_STORE_REALA(vthread_t thr, vvp_code_t code);
extern bool of_STORE_STR(vthread_t thr, vvp_code_t code);
extern bool of_STORE_STRA(vthread_t thr, vvp_code_t code);
extern bool of_STORE_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_STORE_VEC4A(vthread_t thr, vvp_code_t code);
extern bool of_SUB(vthread_t thr, vvp_code_t code);
extern bool of_SUBI(vthread_t thr, vvp_code_t code);
extern bool of_SUB_WR(vthread_t thr, vvp_code_t code);
extern bool of_SUBSTR(vthread_t thr, vvp_code_t code);
extern bool of_SUBSTR_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_TEST_NUL(vthread_t thr, vvp_code_t code);
extern bool of_TEST_NUL_A(vthread_t thr, vvp_code_t code);
extern bool of_TEST_NUL_OBJ(vthread_t thr, vvp_code_t code);
extern bool of_TEST_NUL_PROP(vthread_t thr, vvp_code_t code);
extern bool of_VPI_CALL(vthread_t thr, vvp_code_t code);
extern bool of_WAIT(vthread_t thr, vvp_code_t code);
extern bool of_WAIT_FORK(vthread_t thr, vvp_code_t code);
extern bool of_XNOR(vthread_t thr, vvp_code_t code);
extern bool of_XNORR(vthread_t thr, vvp_code_t code);
extern bool of_XOR(vthread_t thr, vvp_code_t code);
extern bool of_XORR(vthread_t thr, vvp_code_t code);

extern bool of_ZOMBIE(vthread_t thr, vvp_code_t code);

extern bool of_EXEC_UFUNC_REAL(vthread_t thr, vvp_code_t code);
extern bool of_EXEC_UFUNC_VEC4(vthread_t thr, vvp_code_t code);
extern bool of_REAP_UFUNC(vthread_t thr, vvp_code_t code);

extern bool of_CHUNK_LINK(vthread_t thr, vvp_code_t code);

/*
 * This is the format of a machine code instruction.
 */
struct vvp_code_s {
      vvp_code_fun opcode;

      union {
	    unsigned long number;
	    vvp_net_t    *net;
	    vvp_code_t   cptr;
	    vvp_array_t array;
	    class __vpiHandle*handle;
	    __vpiScope*scope;
	    const char*text;
      };

      union {
	    uint32_t    bit_idx[2];
	    vvp_net_t   *net2;
	    vvp_code_t   cptr2;
	    class ufunc_core*ufunc_core_ptr;
      };
};

/*
 * This function clears the code space, ready for initialization. This
 * needs to be done exactly once before any instructions are created.
 */
extern void codespace_init(void);


/*
 * This function returns a pointer to the next free instruction in the
 * code address space. The codespace_next returns a pointer to the
 * next opcode that will be allocated. This is used by label
 * statements to get the address that will be attached to a label in
 * the code.
 */
extern vvp_code_t codespace_allocate(void);
extern vvp_code_t codespace_next(void);
extern vvp_code_t codespace_null(void);

#endif /* IVL_codes_H */
