/*
 * Copyright (c) 2001-2005 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: compile.cc,v 1.207 2005/06/12 01:10:26 steve Exp $"
#endif

# include  "arith.h"
# include  "compile.h"
# include  "logic.h"
# include  "resolv.h"
# include  "udp.h"
# include  "memory.h"
# include  "symbols.h"
# include  "codes.h"
# include  "schedule.h"
# include  "vpi_priv.h"
# include  "parse_misc.h"
# include  "statistics.h"
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <iostream.h>
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
      { "%assign/d", of_ASSIGN_D, 3,  {OA_FUNC_PTR, OA_BIT1, OA_BIT2} },
      { "%assign/m",of_ASSIGN_MEM,3,{OA_MEM_PTR,OA_BIT1,     OA_BIT2} },
      { "%assign/mv",of_ASSIGN_MV,3,{OA_MEM_PTR,OA_BIT1,     OA_BIT2} },
      { "%assign/v0",of_ASSIGN_V0,3,{OA_FUNC_PTR,OA_BIT1,    OA_BIT2} },
      { "%assign/v0/x1",of_ASSIGN_V0X1,3,{OA_FUNC_PTR,OA_BIT1,OA_BIT2} },
      { "%assign/wr",of_ASSIGN_WR,3,{OA_VPI_PTR,OA_BIT1,     OA_BIT2} },
      { "%assign/x0",of_ASSIGN_X0,3,{OA_FUNC_PTR,OA_BIT1,    OA_BIT2} },
      { "%blend",  of_BLEND,  3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%breakpoint", of_BREAKPOINT, 0,  {OA_NONE, OA_NONE, OA_NONE} },
      { "%cassign/link",of_CASSIGN_LINK,2,{OA_FUNC_PTR,OA_FUNC_PTR2,OA_NONE} },
      { "%cassign/v",of_CASSIGN_V,3,{OA_FUNC_PTR,OA_BIT1,    OA_BIT2} },
      { "%cmp/s",  of_CMPS,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%cmp/u",  of_CMPU,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%cmp/wr", of_CMPWR,  2,  {OA_BIT1,     OA_BIT2,     OA_NONE} },
      { "%cmp/x",  of_CMPX,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%cmp/z",  of_CMPZ,   3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%cmpi/u", of_CMPIU,  3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%cvt/ir", of_CVT_IR, 2,  {OA_BIT1,     OA_BIT2,     OA_NONE} },
      { "%cvt/ri", of_CVT_RI, 2,  {OA_BIT1,     OA_BIT2,     OA_NONE} },
      { "%cvt/vr", of_CVT_VR, 3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%deassign",of_DEASSIGN,1,{OA_FUNC_PTR, OA_NONE,     OA_NONE} },
      { "%delay",  of_DELAY,  1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%delayx", of_DELAYX, 1,  {OA_NUMBER,   OA_NONE,     OA_NONE} },
      { "%div",    of_DIV,    3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%div/s",  of_DIV_S,  3,  {OA_BIT1,     OA_BIT2,     OA_NUMBER} },
      { "%div/wr", of_DIV_WR, 2,  {OA_BIT1,     OA_BIT2,     OA_NONE} },
      { "%end",    of_END,    0,  {OA_NONE,     OA_NONE,     OA_NONE} },
      { "%force/link",of_FORCE_LINK,2,{OA_FUNC_PTR,OA_FUNC_PTR2,OA_NONE} },
      { "%force/v",of_FORCE_V,3,  {OA_FUNC_PTR, OA_BIT1,     OA_BIT2} },
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
      { "%load/m", of_LOAD_MEM,2, {OA_BIT1,     OA_MEM_PTR,  OA_NONE} },
      { "%load/mv",of_LOAD_MV,3,  {OA_BIT1,     OA_MEM_PTR,  OA_BIT2} },
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
      { "%release/net",of_RELEASE_NET,1,{OA_FUNC_PTR,OA_NONE,OA_NONE} },
      { "%release/reg",of_RELEASE_REG,1,{OA_FUNC_PTR,OA_NONE,OA_NONE} },
      { "%set/mv", of_SET_MV, 3,  {OA_MEM_PTR,  OA_BIT1,     OA_BIT2} },
      { "%set/v",  of_SET_VEC,3,  {OA_FUNC_PTR, OA_BIT1,     OA_BIT2} },
      { "%set/wr", of_SET_WORDR,2,{OA_VPI_PTR,  OA_BIT1,     OA_NONE} },
      { "%set/x0", of_SET_X0, 3,  {OA_FUNC_PTR, OA_BIT1,     OA_BIT2} },
//    { "%set/x0/x",of_SET_X0_X,3,{OA_FUNC_PTR, OA_BIT1,     OA_BIT2} },
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

static vvp_net_t* vvp_net_lookup(const char*label)
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
		      return sig->node;
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
      vvp_net_t*tmp = lookup_functor_symbol(label);

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
      virtual ~resolv_list_s() { }
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
 * Look up vvp_nets in the symbol table. The "source" is the label for
 * the net that I want to feed, and net->port[port] is the vvp_net
 * input that I want that node to feed into. When the name is found,
 * put net->port[port] into the fan-out list for that node.
 */
struct vvp_net_resolv_list_s: public resolv_list_s {
	// node to locate
      char*source;
	// port to be driven by the located node.
      vvp_net_ptr_t port;
      virtual bool resolve(bool mes);
};

bool vvp_net_resolv_list_s::resolve(bool mes)
{
      vvp_net_t*tmp = vvp_net_lookup(source);

      if (tmp) {
	      // Link the input port to the located output.
	    vvp_net_t*net = port.ptr();
	    net->port[port.port()] = tmp->out;
	    tmp->out = port;

	    free(source);
	    return true;
      }

      if (mes)
	    fprintf(stderr, "unresolved vvp_net reference: %s\n", source);

      return false;
}

inline static
void postpone_functor_input(vvp_net_ptr_t port, char*lab)
{
      struct vvp_net_resolv_list_s*res = new struct vvp_net_resolv_list_s;

      res->port   = port;
      res->source = lab;

      resolv_submit(res);
}


/*
 *  Generic functor reference lookup.
 */

struct functor_gen_resolv_list_s: public resolv_list_s {
      char*source;
      vvp_net_t**ref;
      virtual bool resolve(bool mes);
};

bool functor_gen_resolv_list_s::resolve(bool mes)
{
      vvp_net_t*tmp = vvp_net_lookup(source);

      if (tmp) {
	    *ref = tmp;

	    free(source);
	    return true;
      }

      if (mes)
	    fprintf(stderr, "unresolved functor reference: %s\n", source);

      return false;
}

void functor_ref_lookup(vvp_net_t**ref, char*lab)
{
      struct functor_gen_resolv_list_s*res =
	    new struct functor_gen_resolv_list_s;

      res->ref    = ref;
      res->source = lab;

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
      char*tp;

	/* Is this a vvp_vector4_t constant value? */
      if ((strncmp(label, "C4<", 3) == 0)
	  && ((tp = strchr(label,'>')))
	  && (tp[1] == 0)
	  && (strspn(label+3, "01xz") == (tp-label-3))) {

	    size_t v4size = tp-label-3;
	    vvp_vector4_t tmp (v4size);

	    for (unsigned idx = 0 ;  idx < v4size ;  idx += 1) {
		  vvp_bit4_t bit;
		  switch (label[3+idx]) {
		      case '0':
			bit = BIT4_0;
			break;
		      case '1':
			bit = BIT4_1;
			break;
		      case 'x':
			bit = BIT4_X;
			break;
		      case 'z':
			bit = BIT4_Z;
			break;
		      default:
			assert(0);
			break;
		  }
		  tmp.set_bit(v4size-idx-1, bit);
	    }

	      // Inputs that are constants are schedule to execute as
	      // soon at the simulation starts. In Verilog, constants
	      // start propagating when the simulation starts, just
	      // like any other signal value. But letting the
	      // scheduler distribute the constant value has the
	      // additional advantage that the constant is not
	      // propagated until the network is fully linked.
	    schedule_set_vector(ifdx, tmp);

	    free(label);
	    return;
      }

	/* Is this a vvp_vector8_t constant value? */
      if ((strncmp(label, "C8<", 3) == 0)
	  && ((tp = strchr(label,'>')))
	  && (tp[1] == 0)
	  && (strspn(label+3, "01234567xz") == (tp-label-3))) {

	    size_t vsize = tp-label-3;
	    assert(vsize%3 == 0);
	    vsize /= 3;

	    vvp_vector8_t tmp (vsize);

	    for (unsigned idx = 0 ;  idx < vsize ;  idx += 1) {
		  vvp_bit4_t bit = BIT4_Z;
		  unsigned dr0 = label[3+idx*3+0] - '0';
		  unsigned dr1 = label[3+idx*3+1] - '0';

		  switch (label[3+idx*3+2]) {
		      case '0':
			bit = BIT4_0;
			break;
		      case '1':
			bit = BIT4_1;
			break;
		      case 'x':
			bit = BIT4_X;
			break;
		      case 'z':
			bit = BIT4_Z;
			break;
		  }

		  tmp.set_bit(vsize-idx-1, vvp_scalar_t(bit, dr0, dr1));
	    }

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
	    cerr << "XXXX argv[0] = " << argv[0].text << endl;
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

static void make_arith(vvp_arith_ *arith,
		       char*label, long wid,
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

void compile_arith_div(char*label, long wid, bool signed_flag,
		       unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s; .arith/div has wrong number of symbols\n", label);
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

      if (argc != 2) {
	    fprintf(stderr, "%s .arith/mod has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_arith_mod(wid, false);

      make_arith(arith, label, wid, argc, argv);
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
      make_arith(arith, label, wid, argc, argv);
}

void compile_arith_sub(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .arith has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_arith_sub(wid);
      make_arith(arith, label, wid, argc, argv);
}

void compile_arith_sum(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      if (argc != 2) {
	    fprintf(stderr, "%s .arith has wrong number of symbols\n", label);
	    compile_errors += 1;
	    return;
      }

      vvp_arith_ *arith = new vvp_arith_sum(wid);
      make_arith(arith, label, wid, argc, argv);
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

      make_arith(arith, label, wid, argc, argv);
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

      make_arith(arith, label, wid, argc, argv);
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
      make_arith(arith, label, wid, argc, argv);
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
      make_arith(arith, label, wid, argc, argv);
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

      make_arith(arith, label, wid, argc, argv);
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

      make_arith(arith, label, wid, argc, argv);
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

/*
 * A .shift/l statement creates an array of functors for the
 * width. The 0 input is the data vector to be shifted and the 1 input
 * is the amount of the shift. An unconnected shift amount is set to 0.
 */
void compile_shiftl(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      vvp_arith_ *arith = new vvp_shiftl(wid);
      make_arith(arith, label, wid, argc, argv);
}

void compile_shiftr(char*label, long wid, unsigned argc, struct symb_s*argv)
{
      assert( wid > 0 );

      vvp_arith_ *arith = new vvp_shiftr(wid);
      make_arith(arith, label, wid, argc, argv);
}

void compile_resolver(char*label, char*type, unsigned argc, struct symb_s*argv)
{
      assert(argc <= 4);
      vvp_net_fun_t* obj = 0;

      if (strcmp(type,"tri") == 0) {
	    obj = new resolv_functor(vvp_scalar_t(BIT4_Z, 0));

      } else if (strcmp(type,"tri0") == 0) {
	    obj = new resolv_functor(vvp_scalar_t(BIT4_0, 5));

      } else if (strcmp(type,"tri1") == 0) {
	    obj = new resolv_functor(vvp_scalar_t(BIT4_1, 5));

      } else if (strcmp(type,"triand") == 0) {
	    obj = new table_functor_s(ft_TRIAND);

      } else if (strcmp(type,"trior") == 0) {
	    obj = new table_functor_s(ft_TRIOR);

      } else {
	    fprintf(stderr, "invalid resolver type: %s\n", type);
	    compile_errors += 1;
      }

      if (obj) {
	    vvp_net_t*net = new vvp_net_t;
	    net->fun = obj;
	    define_functor_symbol(label, net);
	    inputs_connect(net, argc, argv);
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
  for (tt = table; tt && *tt; tt++);
  int n = (tt-table) + 2;

  table = (char**)realloc(table, n*sizeof(char*));
  table[n-2] = row;
  table[n-1] = 0x0;

  return table;
}

/*
 * Take the detailed parse items from a .mem statement and generate
 * the necessary internal structures.
 *
 *     <label>  .mem <name>, <msb>, <lsb>, <idxs...> ;
 *
 */
void compile_memory(char *label, char *name, int msb, int lsb,
		    unsigned narg, long *args)
{
	/* Create an empty memory in the symbol table. */
      vvp_memory_t mem = memory_create(label);

      assert( narg > 0 && narg%2 == 0 );

      struct memory_address_range*ranges
	    = new struct memory_address_range[narg/2];

      for (unsigned idx = 0 ;  idx < narg ;  idx += 2) {
	    ranges[idx/2].msb = args[idx+0];
	    ranges[idx/2].lsb = args[idx+1];
      }

      memory_configure(mem, msb, lsb, narg/2, ranges);

      delete[]ranges;

      vpiHandle obj = vpip_make_memory(mem, name);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      free(name);
}

void compile_memory_port(char *label, char *memid,
			 unsigned argc, struct symb_s *argv)
{
      vvp_memory_t mem = memory_find(memid);
      free(memid);
      assert(mem);

      vvp_net_t*ptr = new vvp_net_t;
      vvp_fun_memport*fun = new vvp_fun_memport(mem, ptr);
      ptr->fun = fun;

      define_functor_symbol(label, ptr);
      free(label);

      inputs_connect(ptr, argc, argv);
      free(argv);
}

/*
 * The parser calls this multiple times to parse a .mem/init
 * statement. The first call includes a memid label and is used to
 * select the memory and the start address. Subsequent calls contain
 * only the word value to assign.
 */
void compile_memory_init(char *memid, unsigned i, long val)
{
      static vvp_memory_t current_mem = 0;
      static unsigned current_word;

      if (memid) {
	    current_mem = memory_find(memid);
	    free(memid);
	    current_word = i;
	    return;
      }

      assert(current_mem);

      unsigned word_wid = memory_word_width(current_mem);

      vvp_vector4_t val4 (word_wid);
      for (unsigned idx = 0 ;  idx < word_wid ;  idx += 1) {
	    vvp_bit4_t bit = val & 1 ? BIT4_1 : BIT4_0;
	    val4.set_bit(idx, bit);
      }

      memory_init_word(current_mem, current_word, val4);
      current_word += 1;
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

      vvp_fun_signal*vsig = new vvp_fun_signal(wid);
      vvp_net_t*node = new vvp_net_t;
      node->fun = vsig;
      define_functor_symbol(label, node);

	/* Make the vpiHandle for the reg. */
      vpiHandle obj = (signed_flag > 1) ?
			vpip_make_int(name, msb, lsb, node) :
			vpip_make_reg(name, msb, lsb, signed_flag!=0, node);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      free(name);
}

/*
 * Here we handle .net records from the vvp source:
 *
 *    <label> .net   <name>, <msb>, <lsb>, <input> ;
 *    <label> .net/s <name>, <msb>, <lsb>, <input> ;
 *
 * Create a VPI handle to represent it, and fill that handle in with
 * references into the net.
 */
void compile_net(char*label, char*name, int msb, int lsb, bool signed_flag,
		 unsigned argc, struct symb_s*argv)
{
      unsigned wid = ((msb > lsb)? msb-lsb : lsb-msb) + 1;

      vvp_net_t*node = new vvp_net_t;

      vvp_fun_signal*vsig = new vvp_fun_signal(wid);
      node->fun = vsig;

	/* Add the label into the functor symbol table. */
      define_functor_symbol(label, node);

      assert(argc == 1);

	/* Connect the source to my input. */
      inputs_connect(node, 1, argv);

	/* Make the vpiHandle for the reg. */
      vpiHandle obj = vpip_make_net(name, msb, lsb, signed_flag, node);
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

/*
 * $Log: compile.cc,v $
 * Revision 1.207  2005/06/12 01:10:26  steve
 *  Remove useless references to functor.h
 *
 * Revision 1.206  2005/06/09 05:04:45  steve
 *  Support UDP initial values.
 *
 * Revision 1.205  2005/06/09 04:12:30  steve
 *  Support sequential UDP devices.
 *
 * Revision 1.204  2005/06/02 16:02:11  steve
 *  Add support for notif0/1 gates.
 *  Make delay nodes support inertial delay.
 *  Add the %force/link instruction.
 *
 * Revision 1.203  2005/05/25 05:44:51  steve
 *  Handle event/or with specific, efficient nodes.
 *
 * Revision 1.202  2005/05/24 01:43:27  steve
 *  Add a sign-extension node.
 *
 * Revision 1.201  2005/05/18 03:46:01  steve
 *  Fixup structural GT comparators.
 *
 * Revision 1.200  2005/05/07 03:15:42  steve
 *  Implement non-blocking part assign.
 *
 * Revision 1.199  2005/05/01 22:05:21  steve
 *  Add cassign/link instruction.
 *
 * Revision 1.198  2005/04/28 04:59:53  steve
 *  Remove dead functor code.
 *
 * Revision 1.197  2005/04/24 20:07:26  steve
 *  Add DFF nodes.
 *
 * Revision 1.196  2005/04/01 06:02:45  steve
 *  Reimplement combinational UDPs.
 *
 * Revision 1.195  2005/03/22 05:18:34  steve
 *  The indexed set can write a vector, not just a bit.
 *
 * Revision 1.194  2005/03/19 06:23:49  steve
 *  Handle LPM shifts.
 *
 * Revision 1.193  2005/03/12 06:42:28  steve
 *  Implement .arith/mod.
 *
 * Revision 1.192  2005/03/12 04:27:42  steve
 *  Implement VPI access to signal strengths,
 *  Fix resolution of ambiguous drive pairs,
 *  Fix spelling of scalar.
 *
 * Revision 1.191  2005/03/09 05:52:04  steve
 *  Handle case inequality in netlists.
 *
 * Revision 1.190  2005/03/09 04:52:40  steve
 *  reimplement memory ports.
 *
 * Revision 1.189  2005/03/03 04:33:10  steve
 *  Rearrange how memories are supported as vvp_vector4 arrays.
 *
 * Revision 1.188  2005/02/19 01:32:53  steve
 *  Implement .arith/div.
 *
 * Revision 1.187  2005/02/14 01:50:23  steve
 *  Signals may receive part vectors from %set/x0
 *  instructions. Re-implement the %set/x0 to do
 *  just that. Remove the useless %set/x0/x instruction.
 *
 * Revision 1.186  2005/02/12 03:27:18  steve
 *  Support C8 constants.
 *
 * Revision 1.185  2005/01/30 05:06:49  steve
 *  Get .arith/sub working.
 *
 * Revision 1.184  2005/01/29 17:53:25  steve
 *  Use scheduler to initialize constant functor inputs.
 */

