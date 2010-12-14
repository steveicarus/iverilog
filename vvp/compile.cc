/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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

# include  "arith.h"
# include  "compile.h"
# include  "functor.h"
# include  "logic.h"
# include  "resolv.h"
# include  "udp.h"
# include  "memory.h"
# include  "force.h"
# include  "symbols.h"
# include  "codes.h"
# include  "schedule.h"
# include  "vpi_priv.h"
# include  "parse_misc.h"
# include  "statistics.h"
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

#ifdef __MINGW32__
#include <windows.h>
#endif

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
	/* The operand is a thread bit index or short integer */
      OA_BIT1,
      OA_BIT2,
	/* The operand is a pointer to code space */
      OA_CODE_PTR,
	/* The operand is a variable or net pointer */
      OA_FUNC_PTR,
 	/* The operand is a second functor pointer */
      OA_FUNC_PTR2,
       /* The operand is a pointer to a memory */
      OA_MEM_PTR,
	/* The operand is a VPI handle */
      OA_VPI_PTR,
};

struct opcode_table_s {
      const char*mnemonic;
      vvp_code_fun opcode;

      unsigned argc;
      enum operand_e argt[OPERAND_MAX];
};

const static struct opcode_table_s opcode_table[] = {
      { "%add",    of_ADD,    3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%add/wr", of_ADD_WR, 2,  {OA_BIT1,     OA_BIT2,     OA_NONE} },
      { "%addi",   of_ADDI,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%and",    of_AND,    3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%and/r",  of_ANDR,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%assign", of_ASSIGN, 3,  {OA_FUNC_PTR, OA_BIT1,     OA_BIT2} },
      { "%assign/d", of_ASSIGN_D, 3,  {OA_FUNC_PTR, OA_BIT1, OA_BIT2} },
      { "%assign/m",of_ASSIGN_MEM,3,{OA_MEM_PTR,OA_BIT1,     OA_BIT2} },
      { "%assign/v0",of_ASSIGN_V0,3,{OA_FUNC_PTR,OA_BIT1,    OA_BIT2} },
      { "%assign/wr",of_ASSIGN_WR,3,{OA_VPI_PTR,OA_BIT1,     OA_BIT2} },
      { "%assign/x0",of_ASSIGN_X0,3,{OA_FUNC_PTR,OA_BIT1,    OA_BIT2} },
      { "%blend",  of_BLEND,  3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%breakpoint", of_BREAKPOINT, 0,  {OA_NONE, OA_NONE, OA_NONE} },
      { "%cassign",of_CASSIGN,2,  {OA_FUNC_PTR, OA_FUNC_PTR2,OA_NONE} },
      { "%cmp/s",  of_CMPS,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%cmp/u",  of_CMPU,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%cmp/wr", of_CMPWR,  2,  {OA_BIT1,     OA_BIT2,     OA_NONE} },
      { "%cmp/x",  of_CMPX,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%cmp/z",  of_CMPZ,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%cmpi/u", of_CMPIU,  3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%cvt/ir", of_CVT_IR, 2,  {OA_BIT1,     OA_BIT2,     OA_NONE} },
      { "%cvt/ri", of_CVT_RI, 2,  {OA_BIT1,     OA_BIT2,     OA_NONE} },
      { "%cvt/vr", of_CVT_VR, 3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%deassign",of_DEASSIGN,2,{OA_FUNC_PTR, OA_BIT1,     OA_NONE} },
      { "%delay",  of_DELAY,  1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%delayx", of_DELAYX, 1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%div",    of_DIV,    3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%div/s",  of_DIV_S,  3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%div/wr", of_DIV_WR, 2,  {OA_BIT1,     OA_BIT2,     OA_NONE} },
      { "%end",    of_END,    0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%force",  of_FORCE,  2,  {OA_FUNC_PTR, OA_BIT1,     OA_NONE} },
      { "%inv",    of_INV,    2,  {OA_BIT1,     OA_BIT2,     OA_NONE} },
      { "%ix/add", of_IX_ADD, 2,  {OA_BIT1,     OA_NUMBER,   OA_NONE} },
      { "%ix/get", of_IX_GET, 3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%ix/load",of_IX_LOAD,2,  {OA_BIT1,     OA_NUMBER,   OA_NONE} },
      { "%ix/mul", of_IX_MUL, 2,  {OA_BIT1,     OA_NUMBER,   OA_NONE} },
      { "%ix/sub", of_IX_SUB, 2,  {OA_BIT1,     OA_NUMBER,   OA_NONE} },
      { "%jmp",    of_JMP,    1,  {OA_CODE_PTR, OA_NONE,     OA_NONE} },
      { "%jmp/0",  of_JMP0,   2,  {OA_CODE_PTR, OA_BIT1,     OA_NONE} },
      { "%jmp/0xz",of_JMP0XZ, 2,  {OA_CODE_PTR, OA_BIT1,     OA_NONE} },
      { "%jmp/1",  of_JMP1,   2,  {OA_CODE_PTR, OA_BIT1,     OA_NONE} },
      { "%join",   of_JOIN,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%load",   of_LOAD,   2,  {OA_BIT1,     OA_FUNC_PTR, OA_NONE} },
      { "%load/m", of_LOAD_MEM,2, {OA_BIT1,     OA_MEM_PTR,  OA_NONE} },
      { "%load/nx",of_LOAD_NX,3,  {OA_BIT1,     OA_VPI_PTR,  OA_BIT2} },
      { "%load/v", of_LOAD_VEC,3, {OA_BIT1,     OA_FUNC_PTR, OA_BIT2} },
      { "%load/wr",of_LOAD_WR,2,  {OA_BIT1,     OA_VPI_PTR,  OA_BIT2} },
      { "%load/x", of_LOAD_X, 3,  {OA_BIT1,     OA_FUNC_PTR, OA_BIT2} },
      { "%loadi/wr",of_LOADI_WR,3,{OA_BIT1,     OA_NUMBER,   OA_BIT2} },
      { "%mod",    of_MOD,    3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%mod/s",  of_MOD_S,  3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%mov",    of_MOV,    3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%mul",    of_MUL,    3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%mul/wr", of_MUL_WR, 2,  {OA_BIT1,     OA_BIT2,     OA_NONE} },
      { "%muli",   of_MULI,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%nand",   of_NAND,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%nand/r", of_NANDR,  3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%noop",   of_NOOP,   0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%nor",    of_NOR,    3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%nor/r",  of_NORR,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%or",     of_OR,     3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%or/r",   of_ORR,    3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%release",of_RELEASE,1,  {OA_FUNC_PTR, OA_NONE,     OA_NONE} },
      { "%set",    of_SET,    2,  {OA_FUNC_PTR, OA_BIT1,     OA_NONE} },
      { "%set/m",  of_SET_MEM,2,  {OA_MEM_PTR,  OA_BIT1,     OA_NONE} },
      { "%set/v",  of_SET_VEC,3,  {OA_FUNC_PTR, OA_BIT1,     OA_BIT2} },
      { "%set/wr", of_SET_WORDR,2,{OA_VPI_PTR,  OA_BIT1,     OA_NONE} },
      { "%set/x0", of_SET_X0, 3,  {OA_FUNC_PTR, OA_BIT1,     OA_BIT2} },
      { "%set/x0/x",of_SET_X0_X,3,{OA_FUNC_PTR, OA_BIT1,     OA_BIT2} },
      { "%shiftl/i0", of_SHIFTL_I0, 2, {OA_BIT1,OA_NUMBER,   OA_NONE} },
      { "%shiftr/i0", of_SHIFTR_I0, 2, {OA_BIT1,OA_NUMBER,   OA_NONE} },
      { "%shiftr/s/i0", of_SHIFTR_S_I0,2,{OA_BIT1,OA_NUMBER, OA_NONE} },
      { "%sub",    of_SUB,    3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%sub/wr", of_SUB_WR, 2,  {OA_BIT1,     OA_BIT2,     OA_NONE} },
      { "%subi",   of_SUBI,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%wait",   of_WAIT,   1,  {OA_FUNC_PTR, OA_NONE,     OA_NONE} },
      { "%xnor",   of_XNOR,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%xnor/r", of_XNORR,  3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%xor",    of_XOR,    3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%xor/r",  of_XORR,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
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

void define_functor_symbol(const char*label, vvp_ipoint_t ipt)
{
      symbol_value_t val;
      val.num = ipt;
      sym_set_value(sym_functors, label, val);
}

static vvp_ipoint_t lookup_functor_symbol(const char*label)
{
      assert(sym_functors);
      symbol_value_t val = sym_get_value(sym_functors, label);
      return val.num;
}

static vvp_ipoint_t ipoint_lookup(const char *label, unsigned idx)
{
        /* First, look to see if the symbol is a vpi object of some
	   sort. If it is, then get the vvp_ipoint_t pointer out of
	   the vpiHandle. */
      symbol_value_t val = sym_get_value(sym_vpi, label);
      if (val.ptr) {
	    vpiHandle vpi = (vpiHandle) val.ptr;
	    switch (vpi->vpi_type->type_code) {
		case vpiNet:
		case vpiReg:
		case vpiIntegerVar: {
		      __vpiSignal*sig = (__vpiSignal*)vpi;
		      return vvp_fvector_get(sig->bits, idx);
		}

		case vpiNamedEvent: {
		      __vpiNamedEvent*tmp = (__vpiNamedEvent*)vpi;
		      return tmp->funct;
		}

		default:
		  assert(0);
	    }
      }

	/* Failing that, look for a general functor. */
      vvp_ipoint_t tmp = lookup_functor_symbol(label);
      if (tmp)
	    tmp = ipoint_index(tmp, idx);

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
static struct resolv_list_s*resolv_list = 0;

struct resolv_list_s {
      struct resolv_list_s*next;
      virtual bool resolve(bool mes = false) = 0;
};

static void resolv_submit(struct resolv_list_s*cur)
{
      if (cur->resolve()) {
	    delete cur;
	    return;
      }

      cur->next = resolv_list;
      resolv_list = cur;
}


/*
 *  And the application to functor input lookup
 */

struct functor_resolv_list_s: public resolv_list_s {
      char*source;
      unsigned idx;
      vvp_ipoint_t port;
      virtual bool resolve(bool mes);
};

bool functor_resolv_list_s::resolve(bool mes)
{
      vvp_ipoint_t tmp = ipoint_lookup(source, idx);

      if (tmp) {
	    functor_t fport = functor_index(tmp);
	    functor_t iobj = functor_index(port);

	    iobj->port[ipoint_port(port)] = fport->out;
	    fport->out = port;

	    free(source);
	    return true;
      }

      if (mes)
	    fprintf(stderr, "unresolved functor reference: %s\n", source);

      return false;
}

inline static
void postpone_functor_input(vvp_ipoint_t ptr, char*lab, unsigned idx)
{
      struct functor_resolv_list_s*res = new struct functor_resolv_list_s;

      res->port   = ptr;
      res->source = lab;
      res->idx    = idx;

      resolv_submit(res);
}

/*
 *  Generic functor reference lookup.
 */

struct functor_gen_resolv_list_s: public resolv_list_s {
      char*source;
      unsigned idx;
      vvp_ipoint_t *ref;
      virtual bool resolve(bool mes);
};

bool functor_gen_resolv_list_s::resolve(bool mes)
{
      vvp_ipoint_t tmp = ipoint_lookup(source, idx);

      if (tmp) {
	    *ref = tmp;

	    free(source);
	    return true;
      }

      if (mes)
	    fprintf(stderr, "unresolved functor reference: %s\n", source);

      return false;
}

void functor_ref_lookup(vvp_ipoint_t *ref, char*lab, unsigned idx)
{
      struct functor_gen_resolv_list_s*res =
	    new struct functor_gen_resolv_list_s;

      res->ref    = ref;
      res->source = lab;
      res->idx    = idx;

      resolv_submit(res);
}

/*
 *  vpiHandle lookup
 */

struct vpi_handle_resolv_list_s: public resolv_list_s {
      vpiHandle *handle;
      char *label;
      virtual bool resolve(bool mes);
};

bool vpi_handle_resolv_list_s::resolve(bool mes)
{
      symbol_value_t val = sym_get_value(sym_vpi, label);
      if (!val.ptr) {
	    // check for thread vector  T<base,wid>
	    unsigned base, wid;
	    unsigned n = 0;
	    char ss[32];
	    if (2 <= sscanf(label, "T<%u,%u>%n", &base, &wid, &n)
		&& n == strlen(label)) {
		  val.ptr = vpip_make_vthr_vector(base, wid, false);
		  sym_set_value(sym_vpi, label, val);

	    } else if (3 <= sscanf(label, "T<%u,%u,%[su]>%n", &base,
				   &wid, ss, &n)
		       && n == strlen(label)) {

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

		  val.ptr = vpip_make_vthr_vector(base, wid, signed_flag);
		  sym_set_value(sym_vpi, label, val);

	    } else if (2 == sscanf(label, "W<%u,%[r]>%n", &base, ss, &n)
		       && n == strlen(label)) {

		  val.ptr = vpip_make_vthr_word(base, ss);
		  sym_set_value(sym_vpi, label, val);
	    }

      }

      if (!val.ptr) {
	    // check for memory word  M<mem,base,wid>
      }

      if (val.ptr) {
	    *handle = (vpiHandle) val.ptr;
	    free(label);
	    return true;
      }

      if (mes)
	    fprintf(stderr, "unresolved vpi name lookup: %s\n", label);

      return false;
}

void compile_vpi_lookup(vpiHandle *handle, char*label)
{
      if (strcmp(label, "$time") == 0) {
	    *handle = vpip_sim_time(vpip_peek_current_scope());
	    free(label);
	    return;
      }

      if (strcmp(label, "$stime") == 0) {
	    *handle = vpip_sim_time(vpip_peek_current_scope());
	    free(label);
	    return;
      }

      if (strcmp(label, "$realtime") == 0) {
	    *handle = vpip_sim_realtime(vpip_peek_current_scope());
	    free(label);
	    return;
      }

      if (strcmp(label, "$simtime") == 0) {
	    *handle = vpip_sim_time(0);
	    free(label);
	    return;
      }

      struct vpi_handle_resolv_list_s*res
	    = new struct vpi_handle_resolv_list_s;

      res->handle = handle;
      res->label  = label;
      resolv_submit(res);
}

/*
 * Code Label lookup
 */

struct code_label_resolv_list_s: public resolv_list_s {
      struct vvp_code_s *code;
      char *label;
      virtual bool resolve(bool mes);
};

bool code_label_resolv_list_s::resolve(bool mes)
{
      symbol_value_t val = sym_get_value(sym_codespace, label);
      if (val.num) {
	    if (code->opcode == of_FORK)
		  code->cptr2 = reinterpret_cast<vvp_code_t>(val.ptr);
	    else
		  code->cptr = reinterpret_cast<vvp_code_t>(val.ptr);
	    free(label);
	    return true;
      }

      if (mes)
	    fprintf(stderr,
		    "unresolved code label: %s\n",
		    label);

      return false;
}

void code_label_lookup(struct vvp_code_s *code, char *label)
{
      struct code_label_resolv_list_s *res
	    = new struct code_label_resolv_list_s;

      res->code  = code;
      res->label = label;

      resolv_submit(res);
}

/*
 * Lookup memories.
 */
struct memory_resolv_list_s: public resolv_list_s {
      struct vvp_code_s *code;
      char *label;
      virtual bool resolve(bool mes);
};

bool memory_resolv_list_s::resolve(bool mes)
{
      code->mem = memory_find(label);
      if (code->mem != 0) {
	    free(label);
	    return true;
      }

      if (mes)
	    fprintf(stderr, "Memory unresolved: %s\n", label);

      return false;
}

static void compile_mem_lookup(struct vvp_code_s *code, char *label)
{
      struct memory_resolv_list_s *res
	    = new struct memory_resolv_list_s;

      res->code  = code;
      res->label = label;

      resolv_submit(res);
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
	    struct resolv_list_s *res = resolv_list;
	    resolv_list = 0x0;
	    last = nerrs == lnerrs;
	    lnerrs = nerrs;
	    nerrs = 0;
	    while (res) {
		  struct resolv_list_s *cur = res;
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
      functor_init();

      sym_codespace = new_symbol_table();
      codespace_init();
}

void compile_load_vpi_module(char*name)
{
      vpip_load_module(name);
      free(name);
}

void compile_vpi_time_precision(long pre)
{
      vpip_set_time_precision(pre);
}

/*
 * Run through the arguments looking for the functors that are
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

void inputs_connect(vvp_ipoint_t fdx, unsigned argc, struct symb_s*argv)
{

      for (unsigned idx = 0;  idx < argc;  idx += 1) {

	      /* Find the functor for this input. This assumes that
		 wide (more then 4 inputs) gates are consecutive
		 functors. */
	    vvp_ipoint_t ifdx = ipoint_input_index(fdx, idx);
	    functor_t iobj = functor_index(ifdx);

	    if (strcmp(argv[idx].text, "C<0>") == 0)
		  iobj->set(ifdx, false, 0, St0);

	    else if (strcmp(argv[idx].text, "C<we0>") == 0)
		  iobj->set(ifdx, false, 0, We0);

	    else if (strcmp(argv[idx].text, "C<pu0>") == 0)
		  iobj->set(ifdx, false, 0, Pu0);

	    else if (strcmp(argv[idx].text, "C<su0>") == 0)
		  iobj->set(ifdx, false, 0, Su0);

	    else if (strcmp(argv[idx].text, "C<1>") == 0)
		  iobj->set(ifdx, false, 1, St1);

	    else if (strcmp(argv[idx].text, "C<we1>") == 0)
		  iobj->set(ifdx, false, 1, We1);

	    else if (strcmp(argv[idx].text, "C<pu1>") == 0)
		  iobj->set(ifdx, false, 1, Pu1);

	    else if (strcmp(argv[idx].text, "C<su1>") == 0)
		  iobj->set(ifdx, false, 1, Su1);

	    else if (strcmp(argv[idx].text, "C<x>") == 0)
		  iobj->set(ifdx, false, 2, StX);

	    else if (strcmp(argv[idx].text, "C<z>") == 0)
		  iobj->set(ifdx, false, 3, HiZ);

	    else {
		  postpone_functor_input(ifdx, argv[idx].text, argv[idx].idx);
		  continue;
	    }

	    free(argv[idx].text);
      }
}


struct const_functor_s: public functor_s {
      const_functor_s(unsigned str0, unsigned str1)
	    { odrive0 = str0; odrive1 = str1; }
      virtual void set(vvp_ipoint_t, bool, unsigned, unsigned);
};
void const_functor_s::set(vvp_ipoint_t p, bool, unsigned val, unsigned)
{
      fprintf(stderr, "internal error: Set value to const_functor 0x%x\n", p);
      fprintf(stderr, "              : Value is %u, trying to set %u\n",
	      oval, val);
      fprintf(stderr, "              : I'm driving functor 0x%x\n", out);
      assert(0);
}


static vvp_ipoint_t make_const_functor(unsigned val,
				       unsigned str0,
				       unsigned str1)
{
      vvp_ipoint_t fdx = functor_allocate(1);
      functor_t obj = new const_functor_s(str0, str1);
      functor_define(fdx, obj);

      obj->put_oval(val, false);

      return fdx;
}

/* Lookup a functor[idx] and save the ipoint in *ref. */

static void functor_reference(vvp_ipoint_t *ref, char *lab, unsigned idx)
{
      if (lab == 0)
	    *ref = make_const_functor(3,6,6);

      else if (strcmp(lab, "C<0>") == 0)
	    *ref = make_const_functor(0,6,6);

      else if (strcmp(lab, "C<su0>") == 0)
	    *ref = make_const_functor(0,7,7);

      else if (strcmp(lab, "C<pu0>") == 0)
	    *ref = make_const_functor(0,5,5);

      else if (strcmp(lab, "C<we0>") == 0)
	    *ref = make_const_functor(0,3,3);

      else if (strcmp(lab, "C<1>") == 0)
	    *ref = make_const_functor(1,6,6);

      else if (strcmp(lab, "C<su1>") == 0)
	    *ref = make_const_functor(1,7,7);

      else if (strcmp(lab, "C<pu1>") == 0)
	    *ref = make_const_functor(1,5,5);

      else if (strcmp(lab, "C<we1>") == 0)
	    *ref = make_const_functor(1,3,3);

      else if (strcmp(lab, "C<x>") == 0)
	    *ref = make_const_functor(2,6,6);

      else if (strcmp(lab, "C<z>") == 0)
	    *ref = make_const_functor(3,6,6);

      else {
	    functor_ref_lookup(ref, lab, idx);
	    return;
      }

      free(lab);
}

static void make_extra_outputs(vvp_ipoint_t fdx, unsigned wid)
{
      for (unsigned i=1;  i < wid;  i++) {
	    extra_outputs_functor_s *fu = new extra_outputs_functor_s;
	    vvp_ipoint_t ipt = ipoint_index(fdx, i);
	    functor_define(ipt, fu);
	    fu->base_ = fdx;
      }
}

static void make_arith(vvp_arith_ *arith,
		       char*label, long wid,
		       unsigned argc, struct symb_s*argv)
{
      vvp_ipoint_t fdx = functor_allocate(wid);
      functor_define(fdx, arith);

      define_functor_symbol(label, fdx);
      free(label);

      make_extra_outputs(fdx, wid);

      unsigned opcount = argc / wid;

      struct symb_s tmp_argv[4];
      for (int idx = 0 ;  idx < wid ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(fdx,idx);
	    functor_t obj = functor_index(ptr);

	    obj->ival = 0xaa >> 2*(4 - opcount);

	    for (unsigned cdx = 0 ;  cdx < opcount ;  cdx += 1)
		  tmp_argv[cdx] = argv[idx + wid*cdx];

	    inputs_connect(ptr, opcount, tmp_argv);
      }

      free(argv);
}

void compile_arith_div(char*label, long wid, bool signed_flag,
		       unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if ((long)argc != 2*wid) {
	    fprintf(stderr, "%s; .arith has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_arith_div(wid, signed_flag);

      make_arith(arith, label, wid, argc, argv);
}

void compile_arith_mod(char*label, long wid,
		       unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if ((long)argc != 2*wid) {
	    fprintf(stderr, "%s; .arith has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_arith_mod(wid);

      make_arith(arith, label, wid, argc, argv);
}

void compile_arith_mult(char*label, long wid,
			unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if ((long)argc != 2*wid) {
	    fprintf(stderr, "%s; .arith has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_arith_mult(wid);

      make_arith(arith, label, wid, argc, argv);
}

void compile_arith_sub(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if ((argc % wid) != 0) {
	    fprintf(stderr, "%s; .arith has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      unsigned opcount = argc / wid;
      if (opcount > 4) {
	    fprintf(stderr, "%s; .arith has too many operands.\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_arith_sub(wid);

      make_arith(arith, label, wid, argc, argv);
}

void compile_arith_sum(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if ((argc % wid) != 0) {
	    fprintf(stderr, "%s; .arith has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      unsigned opcount = argc / wid;
      if (opcount > 4) {
	    fprintf(stderr, "%s; .arith has too many operands.\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_arith_sum(wid);

      make_arith(arith, label, wid, argc, argv);
}

void compile_cmp_eq(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if ((long)argc != 2*wid) {
	    fprintf(stderr, "%s; .cmp has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_cmp_eq(wid);

      make_arith(arith, label, wid, argc, argv);
}

void compile_cmp_ne(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if ((long)argc != 2*wid) {
	    fprintf(stderr, "%s; .cmp has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_cmp_ne(wid);

      make_arith(arith, label, wid, argc, argv);
}

void compile_cmp_ge(char*label, long wid, bool signed_flag,
		    unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if ((long)argc != 2*wid) {
	    fprintf(stderr, "%s; .cmp has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_cmp_ge(wid, signed_flag);

      make_arith(arith, label, wid, argc, argv);
}

void compile_cmp_gt(char*label, long wid, bool signed_flag,
		    unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if ((long)argc != 2*wid) {
	    fprintf(stderr, "%s; .cmp has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_cmp_gt(wid, signed_flag);

      make_arith(arith, label, wid, argc, argv);
}

static void make_shift(vvp_arith_*arith,
		       char*label, long wid,
		       unsigned argc, struct symb_s*argv)
{
      vvp_ipoint_t fdx = functor_allocate(wid);
      functor_define(fdx, arith);

      define_functor_symbol(label, fdx);
      free(label);

      make_extra_outputs(fdx, wid);

      for (int idx = 0 ;  idx < wid ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(fdx,idx);
	    functor_t obj = functor_index(ptr);

	    if ((wid+idx) >= (long)argc)
		  obj->ival = 0x02;
	    else
		  obj->ival = 0x0a;

	    struct symb_s tmp_argv[3];
	    unsigned tmp_argc = 1;
	    tmp_argv[0] = argv[idx];
	    if ((wid+idx) < (long)argc) {
		  tmp_argv[1] = argv[wid+idx];
		  tmp_argc += 1;
	    }

	    inputs_connect(ptr, tmp_argc, tmp_argv);
      }

      free(argv);
}

/*
 * A .shift/l statement creates an array of functors for the
 * width. The 0 input is the data vector to be shifted and the 1 input
 * is the amount of the shift. An unconnected shift amount is set to 0.
 */
void compile_shiftl(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if ((long)argc < (wid+1)) {
	    fprintf(stderr, "%s; .shift/l has too few symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      if ((long)argc > (wid*2)) {
	    fprintf(stderr, "%s; .shift/l has too many symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_shiftl(wid);

      make_shift(arith, label, wid, argc, argv);
}

void compile_shiftr(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if ((long)argc < (wid+1)) {
	    fprintf(stderr, "%s; .shift/r has too few symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      if ((long)argc > (wid*2)) {
	    fprintf(stderr, "%s; .shift/r has too many symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_shiftr(wid);

      make_shift(arith, label, wid, argc, argv);
}

void compile_resolver(char*label, char*type, unsigned argc, struct symb_s*argv)
{
      assert(argc <= 4);

      functor_t obj = 0;

      if (strcmp(type,"tri") == 0) {
	    obj = new resolv_functor_s(HiZ);

      } else if (strcmp(type,"tri0") == 0) {
	    obj = new resolv_functor_s(Pu0);

      } else if (strcmp(type,"tri1") == 0) {
	    obj = new resolv_functor_s(Pu1);

      } else if (strcmp(type,"triand") == 0) {
	    obj = new table_functor_s(ft_TRIAND, 6, 6);

      } else if (strcmp(type,"trior") == 0) {
	    obj = new table_functor_s(ft_TRIOR, 6, 6);

      } else {
	    fprintf(stderr, "invalid resolver type: %s\n", type);
	    compile_errors += 1;
      }

      if (obj) {
	    vvp_ipoint_t fdx = functor_allocate(1);
	    functor_define(fdx, obj);
	    define_functor_symbol(label, fdx);

	    inputs_connect(fdx, argc, argv);
      }

      free(type);
      free(label);
      free(argv);
}

void compile_force(char*label, struct symb_s signal,
		   unsigned argc, struct symb_s*argv)
{
      vvp_ipoint_t ifofu = functor_allocate(argc);
      define_functor_symbol(label, ifofu);

      for (unsigned i=0; i<argc; i++) {
	    functor_t obj = new force_functor_s;
	    vvp_ipoint_t iobj = ipoint_index(ifofu, i);
	    functor_define(iobj, obj);

	    functor_ref_lookup(&obj->out, strdup(signal.text), signal.idx + i);

	    // connect the force expression, one bit.
	    inputs_connect(iobj, 1, &argv[i]);
      }

      free(argv);
      free(signal.text);
      free(label);
}

void compile_udp_def(int sequ, char *label, char *name,
		     unsigned nin, unsigned init, char **table)
{
  struct vvp_udp_s *u = udp_create(label);
  u->name = name;
  u->sequ = sequ;
  u->nin = nin;
  u->init = init;
  u->compile_table(table);
  free(label);
}

char **compile_udp_table(char **table, char *row)
{
  if (table)
    assert(strlen(*table)==strlen(row));

  char **tt;
  for (tt = table; tt && *tt; tt++);
  int n = (tt-table) + 2;

  table = (char**)realloc(table, n*sizeof(char*));
  table[n-2] = row;
  table[n-1] = 0x0;

  return table;
}

void compile_udp_functor(char*label, char*type,
			 vvp_delay_t delay,
			 unsigned argc, struct symb_s*argv)
{
      struct vvp_udp_s *u = udp_find(type);
      assert (argc == u->nin);

      functor_t udp = new udp_functor_s(u);

      unsigned nfun = (argc+3)/4;
      vvp_ipoint_t fdx = functor_allocate(nfun);
      functor_define(fdx, udp);
      define_functor_symbol(label, fdx);
      free(label);

      if (nfun > 1) {
	    for (unsigned i=0;  i < nfun-1;  i++) {
		  functor_t fu = new edge_inputs_functor_s;
		  vvp_ipoint_t ipt = ipoint_index(fdx, i+1);
		  functor_define(ipt, fu);
		  fu->out = fdx;
	    }
      }

      udp->delay = delay;

      inputs_connect(fdx, argc, argv);
      free(argv);

      if (u->sequ)
	    udp->put_oval(u->init, false);
}


void compile_memory(char *label, char *name, int msb, int lsb,
		    unsigned idxs, long *idx)
{
  vvp_memory_t mem = memory_create(label);
  memory_new(mem, name, lsb, msb, idxs, idx);

  vpiHandle obj = vpip_make_memory(mem);
  compile_vpi_symbol(label, obj);
  vpip_attach_to_current_scope(obj);

  free(label);
}

void compile_memory_port(char *label, char *memid,
			 unsigned msb, unsigned lsb,
			 unsigned naddr,
			 unsigned argc, struct symb_s *argv)
{
  vvp_memory_t mem = memory_find(memid);
  free(memid);
  assert(mem);

  // This is not a Verilog bit range.
  // This is a data port bit range.
  assert (lsb >= 0  &&  lsb<=msb);
  assert (msb < memory_data_width(mem));
  unsigned nbits = msb-lsb+1;

  bool writable = argc >= (naddr + 2 + nbits);

  vvp_ipoint_t ix = memory_port_new(mem, nbits, lsb, naddr, writable);

  define_functor_symbol(label, ix);
  free(label);

  inputs_connect(ix, argc, argv);
  free(argv);
}

void compile_memory_init(char *memid, unsigned i, unsigned char val)
{
  static vvp_memory_t mem = 0x0;
  static unsigned idx;
  if (memid)
    {
      mem = memory_find(memid);
      free(memid);
      idx = i/4;
    }
  assert(mem);
  memory_init_nibble(mem, idx, val);
  idx++;
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
	    return;
      }

      assert(op);

	/* Build up the code from the information about the opcode and
	   the information from the compiler. */
      vvp_code_t code = codespace_allocate();
      code->opcode = op->opcode;

      if (op->argc != (opa? opa->argc : 0)) {
	    yyerror("operand count");
	    return;
      }

	/* Pull the operands that the instruction expects from the
	   list that the parser supplied. */

      for (unsigned idx = 0 ;  idx < op->argc ;  idx += 1) {

	    switch (op->argt[idx]) {
		case OA_NONE:
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
		  if (opa->argv[idx].ltype != L_SYMB) {
			yyerror("operand format");
			break;
		  }

		  assert(opa->argv[idx].symb.idx == 0);
		  code_label_lookup(code, opa->argv[idx].symb.text);
		  break;

		case OA_FUNC_PTR:
		    /* The operand is a functor. Resolve the label to
		       a functor pointer, or postpone the resolution
		       if it is not defined yet. */
		  if (opa->argv[idx].ltype != L_SYMB) {
			yyerror("operand format");
			break;
		  }

		  functor_ref_lookup(&code->iptr,
				     opa->argv[idx].symb.text,
				     opa->argv[idx].symb.idx);
		  break;

		case OA_FUNC_PTR2:
		    /* The operand is a functor. Resolve the label to
		       a functor pointer, or postpone the resolution
		       if it is not defined yet. */
		  if (opa->argv[idx].ltype != L_SYMB) {
			yyerror("operand format");
			break;
		  }

		  if (strcmp(opa->argv[idx].symb.text, "C<0>") == 0) {
			code->iptr2 = ipoint_make(0, 0);
			free(opa->argv[idx].symb.text);

		  } else if (strcmp(opa->argv[idx].symb.text, "C<1>") == 0) {
			code->iptr2 = ipoint_make(0, 1);
			free(opa->argv[idx].symb.text);

		  } else if (strcmp(opa->argv[idx].symb.text, "C<x>") == 0) {
			code->iptr2 = ipoint_make(0, 2);
			free(opa->argv[idx].symb.text);

		  } else if (strcmp(opa->argv[idx].symb.text, "C<z>") == 0) {
			code->iptr2 = ipoint_make(0, 3);
			free(opa->argv[idx].symb.text);

		  } else {
			functor_ref_lookup(&code->iptr2,
					   opa->argv[idx].symb.text,
					   opa->argv[idx].symb.idx);
		  }
		  break;

		case OA_NUMBER:
		  if (opa->argv[idx].ltype != L_NUMB) {
			yyerror("operand format");
			break;
		  }

		  code->number = opa->argv[idx].numb;
		  break;

        	case OA_MEM_PTR:
		  if (opa->argv[idx].ltype != L_SYMB) {
			yyerror("operand format");
			break;
		  }

		  compile_mem_lookup(code, opa->argv[idx].symb.text);
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
	    }
      }

      if (opa) free(opa);

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


void compile_disable(char*label, struct symb_s symb)
{
      if (label)
	    compile_codelabel(label);

	/* Fill in the basics of the %disable in the instruction. */
      vvp_code_t code = codespace_allocate();
      code->opcode = of_DISABLE;

      compile_vpi_lookup(&code->handle, symb.text);
}

/*
 * The %fork instruction is a little different from other instructions
 * in that it has an extended field that holds the information needed
 * to create the new thread. This includes the target PC and scope.
 * I get these from the parser in the form of symbols.
 */
void compile_fork(char*label, struct symb_s dest, struct symb_s scope)
{
      if (label)
	    compile_codelabel(label);


	/* Fill in the basics of the %fork in the instruction. */
      vvp_code_t code = codespace_allocate();
      code->opcode = of_FORK;

	/* Figure out the target PC. */
      code_label_lookup(code, dest.text);

	/* Figure out the target SCOPE. */
      compile_vpi_lookup((vpiHandle*)&code->scope, scope.text);
}

void compile_vpi_call(char*label, char*name, unsigned argc, vpiHandle*argv)
{
      if (label)
	    compile_codelabel(label);

	/* Create an instruction in the code space. */
      vvp_code_t code = codespace_allocate();
      code->opcode = &of_VPI_CALL;

	/* Create a vpiHandle that bundles the call information, and
	   store that handle in the instruction. */
      code->handle = vpip_build_vpi_call(name, 0, 0, argc, argv);
      if (code->handle == 0)
	    compile_errors += 1;

	/* Done with the lexor-allocated name string. */
      free(name);
}

void compile_vpi_func_call(char*label, char*name,
			   unsigned vbit, int vwid,
			   unsigned argc, vpiHandle*argv)
{
      if (label)
	    compile_codelabel(label);

	/* Create an instruction in the code space. */
      vvp_code_t code = codespace_allocate();
      code->opcode = &of_VPI_CALL;

	/* Create a vpiHandle that bundles the call information, and
	   store that handle in the instruction. */
      code->handle = vpip_build_vpi_call(name, vbit, vwid, argc, argv);
      if (code->handle == 0)
	    compile_errors += 1;

	/* Done with the lexor-allocated name string. */
      free(name);
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
      schedule_vthread(thr, 0, push_flag);

      free(start_sym);
      if (flag != 0)
	    free(flag);
}

/*
 * A variable is a special functor, so we allocate that functor and
 * write the label into the symbol table.
 */
void compile_variable(char*label, char*name, int msb, int lsb,
		      char signed_flag)
{
      unsigned wid = ((msb > lsb)? msb-lsb : lsb-msb) + 1;

      vvp_ipoint_t fdx = functor_allocate(wid);
      define_functor_symbol(label, fdx);

      functor_t fu = new var_functor_s [wid];
      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    functor_define(ipoint_index(fdx, idx), fu+idx);
      }
      count_functors_var += wid;

	/* Make the vpiHandle for the reg. */
      vvp_fvector_t vec = vvp_fvector_continuous_new(wid, fdx);
      vpiHandle obj = (signed_flag > 1) ?
			vpip_make_int(name, msb, lsb, vec) :
			vpip_make_reg(name, msb, lsb, signed_flag!=0, vec);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      free(name);
}

void compile_net(char*label, char*name, int msb, int lsb, bool signed_flag,
		 unsigned argc, struct symb_s*argv)
{
      unsigned wid = ((msb > lsb)? msb-lsb : lsb-msb) + 1;

      vvp_fvector_t vec = vvp_fvector_new(wid);
	//define_fvector_symbol(label, vec);

      assert(argc == wid);

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    vvp_ipoint_t *ref = vvp_fvector_member(vec, idx);
	    functor_reference(ref, argv[idx].text, argv[idx].idx);
      }

	/* Make the vpiHandle for the reg. */
      vpiHandle obj = vpip_make_net(name, msb, lsb, signed_flag, vec);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      free(name);
      free(argv);
}

void compile_param_string(char*label, char*name, char*str, char*value)
{
      assert(strcmp(str,"string") == 0);
      free(str);

      vpiHandle obj = vpip_make_string_param(name, value);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
}
