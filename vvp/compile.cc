/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: compile.cc,v 1.7 2001/03/20 06:16:24 steve Exp $"
#endif

# include  "compile.h"
# include  "functor.h"
# include  "symbols.h"
# include  "codes.h"
# include  "schedule.h"
# include  "vpi_priv.h"
# include  "vthread.h"
# include  "parse_misc.h"
# include  <malloc.h>
# include  <stdlib.h>
# include  <assert.h>


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
	/* The operand is a thread bit index */
      OA_BIT1,
      OA_BIT2,
	/* The operand is a pointer to code space */
      OA_CODE_PTR,
	/* The operand is a variable or net pointer */
      OA_FUNC_PTR
};

struct opcode_table_s {
      const char*mnemonic;
      vvp_code_fun opcode;

      unsigned argc;
      enum operand_e argt[OPERAND_MAX];
};

const static struct opcode_table_s opcode_table[] = {
      { "%assign", of_ASSIGN, 3,  {OA_FUNC_PTR, OA_BIT1,  OA_BIT2} },
      { "%delay",  of_DELAY,  1,  {OA_NUMBER,   OA_NONE,  OA_NONE} },
      { "%end",    of_END,    0,  {OA_NONE,     OA_NONE,  OA_NONE} },
      { "%jmp",    of_JMP,    1,  {OA_CODE_PTR, OA_NONE,  OA_NONE} },
      { "%set",    of_SET,    2,  {OA_FUNC_PTR, OA_BIT1,  OA_NONE} },
      { 0, of_NOOP, 0, {OA_NONE, OA_NONE, OA_NONE} }
};

static unsigned opcode_count = 0;

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
 * If a functor parameter makes a forward reference to a functor, then
 * I need to save that reference and resolve it after the functors are
 * created. Use this structure to keep the unresolved references in an
 * unsorted singly linked list.
 */
struct resolv_list_s {
      struct resolv_list_s*next;
      vvp_ipoint_t port;

      char*source;
      unsigned idx;
};

static struct resolv_list_s*resolv_list = 0;


/*
 * Initialize the compiler by allocation empty symbol tables and
 * initializing the various address spaces.
 */
void compile_init(void)
{
      scope_init();
      sym_functors = new_symbol_table();
      functor_init();
      sym_codespace = new_symbol_table();
      codespace_init();

      opcode_count = 0;
      while (opcode_table[opcode_count].mnemonic)
	    opcode_count += 1;
}

/*
 * The parser calls this function to create a functor. I allocate a
 * functor, and map the name to the vvp_ipoint_t address for the
 * functor. Also resolve the inputs to the functor.
 */
void compile_functor(char*label, char*type, unsigned init,
		     unsigned argc, struct symb_s*argv)
{
      vvp_ipoint_t fdx = functor_allocate(1);
      functor_t obj = functor_index(fdx);

      { symbol_value_t val;
        val.num = fdx;
	sym_set_value(sym_functors, label, val);
      }

      assert(argc <= 4);

	/* Run through the arguments looking for the functors that are
	   connected to my input ports. For each source functor that I
	   find, connect the output of that functor to the indexed
	   input by inserting myself (complete with the port number in
	   the vvp_ipoint_t) into the list that the source heads.

	   If the source functor is not declared yet, then don't do
	   the link yet. Save the reference to be resolved later. */

      for (unsigned idx = 0 ;  idx < argc ;  idx += 1) {
	    symbol_value_t val = sym_get_value(sym_functors, argv[idx].text);
	    vvp_ipoint_t tmp = val.num;

	    if (tmp) {
		  tmp = ipoint_index(tmp, argv[idx].idx);
		  functor_t fport = functor_index(tmp);
		  obj->port[idx] = fport->out;
		  fport->out = ipoint_make(fdx, idx);

		  free(argv[idx].text);

	    } else {
		  struct resolv_list_s*res = (struct resolv_list_s*)
			calloc(1, sizeof(struct resolv_list_s));

		  res->port = ipoint_make(fdx, idx);
		  res->source = argv[idx].text;
		  res->idx  = argv[idx].idx;
		  res->next = resolv_list;
		  resolv_list = res;
	    }
      }

      obj->ival = init;
      obj->oval = 2;

      if (strcmp(type, "OR") == 0) {
	    obj->table = ft_OR;

      } else if (strcmp(type, "AND") == 0) {
	    obj->table = ft_AND;

      } else {
	    yyerror("invalid functor type.");
      }

      free(argv);
      free(label);
      free(type);
}

/*
 * The parser uses this function to compile an link an executable
 * opcode. I do this by looking up the opcode in the opcode_table. The
 * table gives the operand structure that is acceptible, so I can
 * process the operands here as well.
 */
void compile_code(char*label, char*mnem, comp_operands_t opa)
{
      vvp_cpoint_t ptr = codespace_allocate();

	/* First, I can give the label a value that is the current
	   codespace pointer. Don't need the text of the label after
	   this is done. */
      if (label) {
	    symbol_value_t val;
	    val.num = ptr;
	    sym_set_value(sym_codespace, label, val);
	    free(label);
      }

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
	   the information from the comiler. */
      vvp_code_t code = codespace_index(ptr);
      code->opcode = op->opcode;

      if (op->argc != (opa? opa->argc : 0)) {
	    yyerror("operand count");
	    return;
      }

	/* Pull the operands that the instruction expects from the
	   list that the parser supplied. */

      for (unsigned idx = 0 ;  idx < op->argc ;  idx += 1) {
	    symbol_value_t tmp;

	    switch (op->argt[idx]) {
		case OA_NONE:
		  break;

		case OA_BIT1:
		  if (opa->argv[idx].ltype != L_NUMB) {
			yyerror("operand format");
			break;
		  }

		  code->bit_idx1 = opa->argv[idx].numb;
		  break;

		case OA_BIT2:
		  if (opa->argv[idx].ltype != L_NUMB) {
			yyerror("operand format");
			break;
		  }

		  code->bit_idx2 = opa->argv[idx].numb;
		  break;

		case OA_CODE_PTR:
		  if (opa->argv[idx].ltype != L_SYMB) {
			yyerror("operand format");
			break;
		  }

		  assert(opa->argv[idx].symb.idx == 0);
		  tmp = sym_get_value(sym_codespace, opa->argv[idx].symb.text);
		  code->cptr = tmp.num;
		  if (code->cptr == 0) {
			yyerror("functor undefined");
			break;
		  }

		  free(opa->argv[idx].symb.text);
		  break;

		case OA_FUNC_PTR:
		  if (opa->argv[idx].ltype != L_SYMB) {
			yyerror("operand format");
			break;
		  }

		  tmp = sym_get_value(sym_functors, opa->argv[idx].symb.text);
		  if (tmp.num == 0) {
			yyerror("functor undefined");
			break;
		  }
		  code->iptr = ipoint_index(tmp.num, opa->argv[idx].symb.idx);

		  free(opa->argv[idx].symb.text);
		  break;

		case OA_NUMBER:
		  if (opa->argv[idx].ltype != L_NUMB) {
			yyerror("operand format");
			break;
		  }

		  code->number = opa->argv[idx].numb;
		  break;

	    }
      }

      if (opa) free(opa);

      free(mnem);
}

void compile_vpi_call(char*label, char*name, unsigned argc, vpiHandle*argv)
{
      vvp_cpoint_t ptr = codespace_allocate();

	/* First, I can give the label a value that is the current
	   codespace pointer. Don't need the text of the label after
	   this is done. */
      if (label) {
	    symbol_value_t val;
	    val.num = ptr;
	    sym_set_value(sym_codespace, label, val);
	    free(label);
      }

	/* Create an instruction in the code space. */
      vvp_code_t code = codespace_index(ptr);
      code->opcode = &of_VPI_CALL;

	/* Create a vpiHandle that bundles the call information, and
	   store that handle in the instruction. */
      code->handle = vpip_build_vpi_call(name, argc, argv);

	/* Done with the lexor-allocated name string. */
      free(name);
}

/*
 * When the parser finds a thread statement, I create a new thread
 * with the start address referenced by the program symbol passed to
 * me.
 */
void compile_thread(char*start_sym)
{
      symbol_value_t tmp = sym_get_value(sym_codespace, start_sym);
      vvp_cpoint_t pc = tmp.num;
      if (pc == 0) {
	    yyerror("unresolved address");
	    return;
      }

      vthread_t thr = v_newthread(pc);
      schedule_vthread(thr, 0);
      free(start_sym);
}

/*
 * A variable is a special functor, so we allocate that functor and
 * write the label into the symbol table.
 */
void compile_variable(char*label, char*name, int msb, int lsb)
{
      unsigned wid = ((msb > lsb)? msb-lsb : lsb-msb) + 1;
      vvp_ipoint_t fdx = functor_allocate(wid);
      symbol_value_t val;
      val.num = fdx;
      sym_set_value(sym_functors, label, val);

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    functor_t obj = functor_index(ipoint_index(fdx,idx));
	    obj->table = ft_var;
	    obj->ival  = 0x22;
	    obj->oval  = 0x02;
      }
      free(label);

	/* Make the vpiHandle for the reg. */
      vpip_make_reg(name, msb, lsb, fdx);
}

/*
 * When parsing is otherwise complete, this function is called to do
 * the final stuff. Clean up deferred linking here.
 */
void compile_cleanup(void)
{
      struct resolv_list_s*tmp_list = resolv_list;
      resolv_list = 0;

      while (tmp_list) {
	    struct resolv_list_s*res = tmp_list;
	    tmp_list = res->next;

	      /* Get the addressed functor object and select the input
		 port that needs resolution. */
	    functor_t obj = functor_index(res->port);
	    unsigned idx = ipoint_port(res->port);

	      /* Try again to look up the symbol that was not defined
		 the first time around. */
	    symbol_value_t val = sym_get_value(sym_functors, res->source);
	    vvp_ipoint_t tmp = val.num;

	    if (tmp != 0) {
		    /* The symbol is defined, link the functor input
		       to the resolved output. */

		  tmp = ipoint_index(tmp, res->idx);
		  functor_t fport = functor_index(tmp);
		  obj->port[idx] = fport->out;
		  fport->out = res->port;

		  free(res->source);
		  free(res);

	    } else {
		    /* Still not resolved. put back into the list. */
		  res->next = resolv_list;
		  resolv_list = res;
	    }
      }

      scope_cleanup();
}

void compile_dump(FILE*fd)
{
      fprintf(fd, "FUNCTOR SYMBOL TABLE:\n");
      sym_dump(sym_functors, fd);
      fprintf(fd, "FUNCTORS:\n");
      functor_dump(fd);
      fprintf(fd, "UNRESOLVED PORT INPUTS:\n");
      for (struct resolv_list_s*cur = resolv_list ;  cur ;  cur = cur->next)
	    fprintf(fd, "    %p: %s\n", (void*)cur->port, cur->source);

      fprintf(fd, "CODE SPACE SYMBOL TABLE:\n");
      sym_dump(sym_codespace, fd);

      fprintf(fd, "CODE SPACE DISASSEMBLY:\n");
      codespace_dump(fd);
}

/*
 * $Log: compile.cc,v $
 * Revision 1.7  2001/03/20 06:16:24  steve
 *  Add support for variable vectors.
 *
 * Revision 1.6  2001/03/18 04:35:18  steve
 *  Add support for string constants to VPI.
 *
 * Revision 1.5  2001/03/18 00:37:55  steve
 *  Add support for vpi scopes.
 *
 * Revision 1.4  2001/03/16 01:44:34  steve
 *  Add structures for VPI support, and all the %vpi_call
 *  instruction. Get linking of VPI modules to work.
 *
 * Revision 1.3  2001/03/11 23:06:49  steve
 *  Compact the vvp_code_s structure.
 *
 * Revision 1.2  2001/03/11 22:42:11  steve
 *  Functor values and propagation.
 *
 * Revision 1.1  2001/03/11 00:29:38  steve
 *  Add the vvp engine to cvs.
 *
 */

