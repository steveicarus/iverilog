/*
 * Copyright (c) 2026 Stephen Williams (steve@icarus.com)
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

/*
 * tgt-flow: a dataflow exporter target module for Icarus Verilog.
 *
 * Emits a `.flow` JSON dataflow database (schema flowtracer1.verilog.v0,
 * the Verilog companion to GHDL's flowtracer1.vhdl.v0), so a single IDE /
 * consumer (vpi_flowtracer's dftrace) works across Verilog and VHDL
 * designs.  Source positions are the compact "start:begin:end" byte-offset
 * form; since the ivl_target API exposes only line numbers, the byte
 * offsets and begin/end spans are recovered by scanning the source files.
 *
 * PHASE 1: structural skeleton (envelope, modules[], hierarchy[],
 *          port_map[] resolved through the shared nexus).
 * PHASE 2 (added here): the dataflow graph.
 *   - nets[]  -- one entry per canonical nexus (ivl_nexus_t): id, name,
 *     path, is_port, aliases[], and bidirectional drivers[]/loads[]
 *     (lists of CELL ids).
 *   - cells[] -- the dataflow nodes (processes, logic gates, LPM
 *     devices, constants): id, kind, label, path, pos, clocked,
 *     clock_net (a NET id), and drives[]/reads[] (lists of NET ids).
 *
 * Process reads/drives are extracted by walking the elaborated
 * statement/expression tree (the Verilog analogue of GHDL's
 * Collect_Reads/Collect_Drivers). Because the design is run through
 * cprop/nodangle (not synth), RTL behaviour stays as ivl_process_t,
 * matching GHDL's process-centric model.
 */

# include "version_base.h"
# include "version_tag.h"
# include "config.h"
# include  <stdlib.h>
# include  <string.h>
# include  <stdio.h>
# include  <stdint.h>
# include  <inttypes.h>
# include  <ctype.h>
# include  "ivl_target.h"

static const char*version_string =
"Icarus Verilog FLOW Dataflow Exporter " VERSION " (" VERSION_TAG ")\n";

/* Output file (the -o argument). */
static FILE*out;

/* Running error count returned to the ivl_target environment. */
static int flow_errors = 0;

/* ================================================================== */
/* Generic growable helpers                                           */
/* ================================================================== */

static void intset_add(int**arr, size_t*n, size_t*cap, int v)
{
      size_t i;
      for (i = 0 ; i < *n ; i += 1)
	    if ((*arr)[i] == v)
		  return;
      if (*n == *cap) {
	    size_t nc = *cap ? *cap * 2 : 8;
	    int*g = (int*)realloc(*arr, nc * sizeof(int));
	    if (!g) { flow_errors += 1; return; }
	    *arr = g; *cap = nc;
      }
      (*arr)[(*n)++] = v;
}

static void stradd(const char***arr, size_t*n, size_t*cap, const char*s)
{
      size_t i;
      if (!s) s = "";
      for (i = 0 ; i < *n ; i += 1)
	    if (strcmp((*arr)[i], s) == 0)
		  return;
      if (*n == *cap) {
	    size_t nc = *cap ? *cap * 2 : 8;
	    const char**g = (const char**)realloc(*arr, nc * sizeof(const char*));
	    if (!g) { flow_errors += 1; return; }
	    *arr = g; *cap = nc;
      }
      (*arr)[(*n)++] = s;
}

/* ================================================================== */
/* Module-type set + file-name set (Phase 1)                          */
/* ================================================================== */

struct scope_set { ivl_scope_t*items; size_t count, cap; };
static struct scope_set modules = { NULL, 0, 0 };

static void scope_set_push(struct scope_set*set, ivl_scope_t sc)
{
      if (set->count == set->cap) {
	    size_t nc = set->cap ? set->cap * 2 : 32;
	    ivl_scope_t*g = (ivl_scope_t*)realloc(set->items, nc*sizeof(ivl_scope_t));
	    if (!g) { flow_errors += 1; return; }
	    set->items = g; set->cap = nc;
      }
      set->items[set->count++] = sc;
}

static struct str_set { const char**items; size_t count, cap; } files = { NULL, 0, 0 };

/* ================================================================== */
/* Net table: one entry per canonical nexus                           */
/* ================================================================== */

struct net_ent {
      ivl_nexus_t nex;
      const char*name;
      const char*path;
      int is_port;
      const char**aliases; size_t nalias, alias_cap;
      int*drivers; size_t ndrivers, drivers_cap;
      int*loads;   size_t nloads, loads_cap;
};
static struct { struct net_ent*items; size_t count, cap; } nets = { NULL, 0, 0 };

/* Find or create the net id (1-based) for a nexus. `hint`, when given,
   provides the canonical name/path (the first/parent signal). */
static int net_id(ivl_nexus_t nex, ivl_signal_t hint)
{
      size_t i;
      struct net_ent ent;
      unsigned np, j;

      if (!nex)
	    return 0;
      for (i = 0 ; i < nets.count ; i += 1)
	    if (nets.items[i].nex == nex)
		  return (int)(i + 1);

      memset(&ent, 0, sizeof ent);
      ent.nex = nex;
      ent.name = "";
      ent.path = "";

      np = ivl_nexus_ptrs(nex);
      for (j = 0 ; j < np ; j += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, j);
	    ivl_signal_t s = ivl_nexus_ptr_sig(ptr);
	    if (!s)
		  continue;
	    stradd(&ent.aliases, &ent.nalias, &ent.alias_cap,
		   ivl_signal_basename(s));
	    if (ivl_signal_port(s) != IVL_SIP_NONE)
		  ent.is_port = 1;
	    if (!hint && ent.name[0] == 0) {
		  ent.name = ivl_signal_basename(s);
		  ent.path = ivl_scope_name(ivl_signal_scope(s));
	    }
      }
      if (hint) {
	    ent.name = ivl_signal_basename(hint);
	    ent.path = ivl_scope_name(ivl_signal_scope(hint));
      }

      if (nets.count == nets.cap) {
	    size_t nc = nets.cap ? nets.cap * 2 : 64;
	    struct net_ent*g = (struct net_ent*)
		  realloc(nets.items, nc * sizeof(struct net_ent));
	    if (!g) { flow_errors += 1; return 0; }
	    nets.items = g; nets.cap = nc;
      }
      nets.items[nets.count++] = ent;
      return (int)nets.count;
}

/* Net id for a signal (uses word 0; arrays approximated). */
static int net_of_sig(ivl_signal_t sig, ivl_signal_t hint)
{
      if (!sig) return 0;
      return net_id(ivl_signal_nex(sig, 0), hint);
}

/* ================================================================== */
/* Cell table                                                         */
/* ================================================================== */

struct cell_ent {
      const char*kind;
      char label[64];
      const char*path;
      const char*file;
      unsigned line;
      int clocked;
      int clock_net;
      int*drives; size_t nd, dcap;
      int*reads;  size_t nr, rcap;
};
static struct { struct cell_ent*items; size_t count, cap; } cells = { NULL, 0, 0 };

/* Every elaborated process, kept so emit_module() can render the
   name-based modules[].processes[] the consumer actually traverses. */
static struct { ivl_process_t*items; size_t count, cap; } proclist = { NULL, 0, 0 };

static void proclist_push(ivl_process_t p)
{
      if (proclist.count == proclist.cap) {
	    size_t nc = proclist.cap ? proclist.cap * 2 : 32;
	    ivl_process_t*g = (ivl_process_t*)
		  realloc(proclist.items, nc * sizeof(ivl_process_t));
	    if (!g) { flow_errors += 1; return; }
	    proclist.items = g; proclist.cap = nc;
      }
      proclist.items[proclist.count++] = p;
}

static struct cell_ent* cell_new(const char*kind, const char*path,
				 const char*file, unsigned line)
{
      struct cell_ent*c;
      if (cells.count == cells.cap) {
	    size_t nc = cells.cap ? cells.cap * 2 : 32;
	    struct cell_ent*g = (struct cell_ent*)
		  realloc(cells.items, nc * sizeof(struct cell_ent));
	    if (!g) { flow_errors += 1; return 0; }
	    cells.items = g; cells.cap = nc;
      }
      c = &cells.items[cells.count++];
      memset(c, 0, sizeof *c);
      c->kind = kind;
      c->path = path ? path : "";
      c->file = file ? file : "";
      c->line = line;
      c->clock_net = 0;
      return c;
}

static void cell_drive(struct cell_ent*c, int net) { if (net) intset_add(&c->drives,&c->nd,&c->dcap,net); }
static void cell_read (struct cell_ent*c, int net) { if (net) intset_add(&c->reads, &c->nr,&c->rcap,net); }

/* ================================================================== */
/* Process statement / expression walk                                */
/* ================================================================== */

static void collect_expr(ivl_expr_t e, struct cell_ent*c);
static void collect_stmt(ivl_statement_t s, struct cell_ent*c);

/* Recurse the expression tree collecting signal reads. Only the
   operand accessors that are valid for each expression kind are called
   -- ivl_expr_oper1/2/3 and ivl_expr_signal assert on the wrong kind. */
static void collect_expr(ivl_expr_t e, struct cell_ent*c)
{
      unsigned i, n;
      if (!e)
	    return;
      switch (ivl_expr_type(e)) {
	  case IVL_EX_SIGNAL:
	  case IVL_EX_ARRAY:
	  case IVL_EX_MEMORY:
	    cell_read(c, net_of_sig(ivl_expr_signal(e), 0));
	    collect_expr(ivl_expr_oper1(e), c);  /* array/word index */
	    break;

	  case IVL_EX_UNARY:
	    collect_expr(ivl_expr_oper1(e), c);
	    break;

	  case IVL_EX_BINARY:
	  case IVL_EX_SELECT:
	    collect_expr(ivl_expr_oper1(e), c);
	    collect_expr(ivl_expr_oper2(e), c);
	    break;

	  case IVL_EX_TERNARY:
	    collect_expr(ivl_expr_oper1(e), c);
	    collect_expr(ivl_expr_oper2(e), c);
	    collect_expr(ivl_expr_oper3(e), c);
	    break;

	  case IVL_EX_CONCAT:
	  case IVL_EX_SFUNC:
	  case IVL_EX_UFUNC:
	    n = ivl_expr_parms(e);
	    for (i = 0 ; i < n ; i += 1)
		  collect_expr(ivl_expr_parm(e, i), c);
	    break;

	  default:
	    /* NUMBER, STRING, REALNUM, ULONG, SCOPE, ENUMTYPE, NULL,
	       PROPERTY, etc.: no signal reads to harvest for Phase 2. */
	    break;
      }
}

/* Resolve an l-value to its driven signal, descending nested l-values. */
static ivl_signal_t lval_sig(ivl_lval_t lv)
{
      while (lv && ivl_lval_sig(lv) == 0 && ivl_lval_nest(lv) != 0)
	    lv = ivl_lval_nest(lv);
      return lv ? ivl_lval_sig(lv) : 0;
}

static void collect_lvals(ivl_statement_t s, struct cell_ent*c)
{
      unsigned i, n = ivl_stmt_lvals(s);
      for (i = 0 ; i < n ; i += 1) {
	    ivl_lval_t lv = ivl_stmt_lval(s, i);
	    cell_drive(c, net_of_sig(lval_sig(lv), 0));
	    /* part-select / array-index expressions are reads */
	    collect_expr(ivl_lval_part_off(lv), c);
	    collect_expr(ivl_lval_idx(lv), c);
      }
}

static void collect_events(ivl_statement_t s, struct cell_ent*c)
{
      unsigned ne = ivl_stmt_nevent(s);
      unsigned i, k;
      for (i = 0 ; i < ne ; i += 1) {
	    ivl_event_t ev = ivl_stmt_events(s, i);
	    if (!ev)
		  continue;
	    for (k = 0 ; k < ivl_event_npos(ev) ; k += 1) {
		  int id = net_id(ivl_event_pos(ev, k), 0);
		  cell_read(c, id);
		  c->clocked = 1;
		  if (c->clock_net == 0) c->clock_net = id;
	    }
	    for (k = 0 ; k < ivl_event_nneg(ev) ; k += 1) {
		  int id = net_id(ivl_event_neg(ev, k), 0);
		  cell_read(c, id);
		  c->clocked = 1;
		  if (c->clock_net == 0) c->clock_net = id;
	    }
	    for (k = 0 ; k < ivl_event_nany(ev) ; k += 1)
		  cell_read(c, net_id(ivl_event_any(ev, k), 0));
      }
}

static void collect_stmt(ivl_statement_t s, struct cell_ent*c)
{
      unsigned i, n;
      if (!s)
	    return;
      switch (ivl_statement_type(s)) {
	  case IVL_ST_BLOCK:
	  case IVL_ST_FORK:
	  case IVL_ST_FORK_JOIN_ANY:
	  case IVL_ST_FORK_JOIN_NONE:
	    n = ivl_stmt_block_count(s);
	    for (i = 0 ; i < n ; i += 1)
		  collect_stmt(ivl_stmt_block_stmt(s, i), c);
	    break;

	  case IVL_ST_ASSIGN:
	  case IVL_ST_ASSIGN_NB:
	  case IVL_ST_CASSIGN:
	  case IVL_ST_FORCE:
	    collect_lvals(s, c);
	    collect_expr(ivl_stmt_rval(s), c);
	    break;

	  case IVL_ST_CONDIT:
	    collect_expr(ivl_stmt_cond_expr(s), c);
	    collect_stmt(ivl_stmt_cond_true(s), c);
	    collect_stmt(ivl_stmt_cond_false(s), c);
	    break;

	  case IVL_ST_CASE:
	  case IVL_ST_CASEX:
	  case IVL_ST_CASEZ:
	  case IVL_ST_CASER:
	    collect_expr(ivl_stmt_cond_expr(s), c);
	    n = ivl_stmt_case_count(s);
	    for (i = 0 ; i < n ; i += 1) {
		  collect_expr(ivl_stmt_case_expr(s, i), c);
		  collect_stmt(ivl_stmt_case_stmt(s, i), c);
	    }
	    break;

	  case IVL_ST_WAIT:
	    collect_events(s, c);
	    collect_stmt(ivl_stmt_sub_stmt(s), c);
	    break;

	  case IVL_ST_DELAY:
	    collect_stmt(ivl_stmt_sub_stmt(s), c);
	    break;
	  case IVL_ST_DELAYX:
	    collect_expr(ivl_stmt_delay_expr(s), c);
	    collect_stmt(ivl_stmt_sub_stmt(s), c);
	    break;

	  case IVL_ST_FORLOOP:
	    collect_stmt(ivl_stmt_init_stmt(s), c);
	    collect_expr(ivl_stmt_cond_expr(s), c);
	    collect_stmt(ivl_stmt_step_stmt(s), c);
	    collect_stmt(ivl_stmt_sub_stmt(s), c);
	    break;

	  case IVL_ST_WHILE:
	  case IVL_ST_DO_WHILE:
	    collect_expr(ivl_stmt_cond_expr(s), c);
	    collect_stmt(ivl_stmt_sub_stmt(s), c);
	    break;

	  case IVL_ST_STASK:
	    n = ivl_stmt_parm_count(s);
	    for (i = 0 ; i < n ; i += 1)
		  collect_expr(ivl_stmt_parm(s, i), c);
	    break;

	  default:
	    /* NOOP, DISABLE, TRIGGER, ALLOC/FREE, etc.: no dataflow for
	       Phase 2. Nested statements that carry a sub_stmt are still
	       walked where it matters above. */
	    break;
      }
}

/* ================================================================== */
/* Cell builders                                                      */
/* ================================================================== */

static int build_process(ivl_process_t proc, void*cd)
{
      ivl_scope_t scope = ivl_process_scope(proc);
      ivl_statement_t st = ivl_process_stmt(proc);
      const char*tname;
      struct cell_ent*c;
      (void)cd;

      if (ivl_process_analog(proc))
	    return 0;  /* analog processes: out of scope for Phase 2 */

      proclist_push(proc);

      switch (ivl_process_type(proc)) {
	  case IVL_PR_INITIAL:      tname = "initial";      break;
	  case IVL_PR_FINAL:        tname = "final";        break;
	  case IVL_PR_ALWAYS_COMB:  tname = "always_comb";  break;
	  case IVL_PR_ALWAYS_FF:    tname = "always_ff";    break;
	  case IVL_PR_ALWAYS_LATCH: tname = "always_latch"; break;
	  case IVL_PR_ALWAYS:
	  default:                  tname = "always";       break;
      }

      c = cell_new("process", ivl_scope_name(scope),
		   st ? ivl_stmt_file(st) : ivl_scope_file(scope),
		   st ? ivl_stmt_lineno(st) : ivl_scope_lineno(scope));
      if (!c)
	    return 0;
      snprintf(c->label, sizeof c->label, "%s@%u", tname, c->line);

      /* always_ff / always_comb / always_latch carry the edge intent. */
      if (ivl_process_type(proc) == IVL_PR_ALWAYS_FF)
	    c->clocked = 1;

      collect_stmt(st, c);
      return 0;
}

static void build_logic(ivl_net_logic_t log)
{
      unsigned np = ivl_logic_pins(log);
      unsigned i;
      struct cell_ent*c = cell_new("gate", ivl_scope_name(ivl_logic_scope(log)),
				   ivl_logic_file(log), ivl_logic_lineno(log));
      if (!c)
	    return;
      snprintf(c->label, sizeof c->label, "%s", ivl_logic_basename(log)
	       ? ivl_logic_basename(log) : "gate");
      /* pin 0 is the output, the rest are inputs. */
      if (np > 0)
	    cell_drive(c, net_id(ivl_logic_pin(log, 0), 0));
      for (i = 1 ; i < np ; i += 1)
	    cell_read(c, net_id(ivl_logic_pin(log, i), 0));
}

/* Resolve an LPM's valid input set into `buf` (returns the count). The
   pin accessors assert on the wrong device kind, so the input shape is
   driven off the LPM type. Shared by the integer-cell builder and the
   name-based modules[].assignments[] emitter. */
static unsigned lpm_input_nexuses(ivl_lpm_t lpm, ivl_nexus_t*buf, unsigned max)
{
      ivl_lpm_type_t t = ivl_lpm_type(lpm);
      unsigned i, ndata = 0, n = 0;
      int has_sel = 0, has_enable = 0, has_clk = 0;

      switch (t) {
	  case IVL_LPM_ADD:    case IVL_LPM_SUB:   case IVL_LPM_MULT:
	  case IVL_LPM_DIVIDE: case IVL_LPM_MOD:   case IVL_LPM_POW:
	  case IVL_LPM_SHIFTL: case IVL_LPM_SHIFTR:
	  case IVL_LPM_CMP_EQ:  case IVL_LPM_CMP_NE:  case IVL_LPM_CMP_EEQ:
	  case IVL_LPM_CMP_NEE: case IVL_LPM_CMP_EQX: case IVL_LPM_CMP_EQZ:
	  case IVL_LPM_CMP_WEQ: case IVL_LPM_CMP_WNE:
	  case IVL_LPM_CMP_GE:  case IVL_LPM_CMP_GT:
	    ndata = 2; break;
	  case IVL_LPM_RE_AND:  case IVL_LPM_RE_NAND: case IVL_LPM_RE_NOR:
	  case IVL_LPM_RE_OR:   case IVL_LPM_RE_XNOR: case IVL_LPM_RE_XOR:
	  case IVL_LPM_ABS:     case IVL_LPM_SIGN_EXT:
	  case IVL_LPM_CAST_INT: case IVL_LPM_CAST_INT2: case IVL_LPM_CAST_REAL:
	  case IVL_LPM_PART_VP: case IVL_LPM_PART_PV:  case IVL_LPM_REPEAT:
	    ndata = 1; break;
	  case IVL_LPM_MUX:
	    ndata = ivl_lpm_size(lpm); has_sel = 1; break;
	  case IVL_LPM_CONCAT: case IVL_LPM_CONCATZ:
	  case IVL_LPM_SFUNC:  case IVL_LPM_UFUNC: case IVL_LPM_SUBSTITUTE:
	    ndata = ivl_lpm_size(lpm); break;
	  case IVL_LPM_FF:
	    ndata = 1; has_clk = 1; has_enable = 1; break;
	  case IVL_LPM_LATCH:
	    ndata = 1; has_enable = 1; break;
	  case IVL_LPM_ARRAY:
	    ndata = 1; has_sel = 1; break;
	  default:
	    ndata = 0; break;
      }

      for (i = 0 ; i < ndata && n < max ; i += 1)
	    buf[n++] = ivl_lpm_data(lpm, i);
      if (has_sel && n < max)    buf[n++] = ivl_lpm_select(lpm);
      if (has_enable && n < max) buf[n++] = ivl_lpm_enable(lpm);
      if (has_clk && n < max)    buf[n++] = ivl_lpm_clk(lpm);
      return n;
}

static void build_lpm(ivl_lpm_t lpm)
{
      struct cell_ent*c;
      ivl_nexus_t ins[64];
      unsigned ni = lpm_input_nexuses(lpm, ins, 64), i;

      c = cell_new("lpm", ivl_scope_name(ivl_lpm_scope(lpm)),
		   ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
      if (!c)
	    return;
      snprintf(c->label, sizeof c->label, "%s", ivl_lpm_basename(lpm)
	       ? ivl_lpm_basename(lpm) : "lpm");

      cell_drive(c, net_id(ivl_lpm_q(lpm), 0));
      for (i = 0 ; i < ni ; i += 1)
	    cell_read(c, net_id(ins[i], 0));

      /* Sequential LPMs (FF) carry an explicit clock pin. */
      if (ivl_lpm_type(lpm) == IVL_LPM_FF) {
	    ivl_nexus_t clk = ivl_lpm_clk(lpm);
	    if (clk) {
		  int id = net_id(clk, 0);
		  c->clocked = 1;
		  c->clock_net = id;
	    }
      }
}

static void build_const(ivl_net_const_t con)
{
      struct cell_ent*c = cell_new("const", ivl_scope_name(ivl_const_scope(con)),
				   ivl_const_file(con), ivl_const_lineno(con));
      if (!c)
	    return;
      snprintf(c->label, sizeof c->label, "const");
      cell_drive(c, net_id(ivl_const_nex(con), 0));
}

/* Walk a scope tree collecting logic + lpm cells (processes and consts
   are enumerated design-globally via ivl_design_process/const). */
static int build_scope_cells(ivl_scope_t sc, void*cd)
{
      unsigned i, n;
      (void)cd;
      n = ivl_scope_logs(sc);
      for (i = 0 ; i < n ; i += 1)
	    build_logic(ivl_scope_log(sc, i));
      n = ivl_scope_lpms(sc);
      for (i = 0 ; i < n ; i += 1)
	    build_lpm(ivl_scope_lpm(sc, i));
      ivl_scope_children(sc, build_scope_cells, 0);
      return 0;
}

/* ================================================================== */
/* Pre-registration of signal nets (deterministic top-down order)     */
/* ================================================================== */

static int prereg_nets(ivl_scope_t sc, void*cd)
{
      unsigned i, n = ivl_scope_sigs(sc);
      (void)cd;
      for (i = 0 ; i < n ; i += 1) {
	    ivl_signal_t sig = ivl_scope_sig(sc, i);
	    unsigned w, words = ivl_signal_array_count(sig);
	    if (words == 0) words = 1;
	    for (w = 0 ; w < words ; w += 1)
		  net_id(ivl_signal_nex(sig, w), sig);
      }
      ivl_scope_children(sc, prereg_nets, 0);
      return 0;
}

/* ================================================================== */
/* JSON primitives                                                    */
/* ================================================================== */

static void put_jstr(const char*s)
{
      putc('"', out);
      if (s) {
	    for ( ; *s ; s += 1) {
		  unsigned char c = (unsigned char)*s;
		  switch (c) {
		      case '"':  fputs("\\\"", out); break;
		      case '\\': fputs("\\\\", out); break;
		      case '\n': fputs("\\n", out);  break;
		      case '\r': fputs("\\r", out);  break;
		      case '\t': fputs("\\t", out);  break;
		      default:
			if (c < 0x20) fprintf(out, "\\u%04x", c);
			else putc(c, out);
		  }
	    }
      }
      putc('"', out);
}

/* ================================================================== */
/* Source-file cache + a tiny comment/string-aware scanner.           */
/*                                                                    */
/* The ivl_target API only exposes line numbers, so to report byte    */
/* offsets (and begin/end spans) the exporter reads each source file  */
/* and scans it directly.                                             */
/* ================================================================== */

struct src_file {
      char*name;
      char*buf;
      size_t len;
      long*line_off;   /* line_off[L] = byte offset of 1-based line L */
      unsigned nlines;
};
static struct { struct src_file*items; size_t count, cap; } srcs = {NULL,0,0};

/* Load (and cache) FILE; return NULL if it cannot be read. */
static struct src_file* src_get(const char*name)
{
      size_t i;
      FILE*fp;
      long sz;
      struct src_file*s;
      unsigned ln;
      size_t k;

      if (!name || !name[0])
	    return NULL;
      for (i = 0 ; i < srcs.count ; i += 1)
	    if (strcmp(srcs.items[i].name, name) == 0)
		  return srcs.items[i].buf ? &srcs.items[i] : NULL;

      if (srcs.count == srcs.cap) {
	    size_t nc = srcs.cap ? srcs.cap * 2 : 8;
	    struct src_file*g = (struct src_file*)
		  realloc(srcs.items, nc * sizeof(struct src_file));
	    if (!g) { flow_errors += 1; return NULL; }
	    srcs.items = g; srcs.cap = nc;
      }
      s = &srcs.items[srcs.count++];
      memset(s, 0, sizeof *s);
      s->name = strdup(name);

      fp = fopen(name, "rb");
      if (!fp)
	    return NULL;          /* cached as a negative entry (buf == NULL) */
      fseek(fp, 0, SEEK_END);
      sz = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      if (sz < 0) { fclose(fp); return NULL; }
      s->buf = (char*)malloc((size_t)sz + 1);
      if (!s->buf) { fclose(fp); flow_errors += 1; return NULL; }
      s->len = fread(s->buf, 1, (size_t)sz, fp);
      s->buf[s->len] = 0;
      fclose(fp);

      /* Build the 1-based line table. */
      s->nlines = 1;
      for (k = 0 ; k < s->len ; k += 1)
	    if (s->buf[k] == '\n') s->nlines += 1;
      s->line_off = (long*)malloc((s->nlines + 2) * sizeof(long));
      if (!s->line_off) { flow_errors += 1; return s; }
      ln = 1;
      s->line_off[1] = 0;
      for (k = 0 ; k < s->len ; k += 1)
	    if (s->buf[k] == '\n') { ln += 1; s->line_off[ln] = (long)(k + 1); }
      return s;
}

/* Byte offset of the first non-blank character on 1-based LINE. */
static long src_line_start(struct src_file*s, unsigned line)
{
      long off;
      if (!s || !s->line_off || line == 0 || line > s->nlines)
	    return -1;
      off = s->line_off[line];
      while ((size_t)off < s->len && (s->buf[off] == ' ' || s->buf[off] == '\t'))
	    off += 1;
      return off;
}

/* If P starts a // or /​* *​/ comment or a "string", return the offset
   just past it; otherwise return P unchanged. */
static long src_skip_cs(const struct src_file*s, long p)
{
      if (p + 1 < (long)s->len && s->buf[p] == '/' && s->buf[p+1] == '/') {
	    p += 2;
	    while ((size_t)p < s->len && s->buf[p] != '\n') p += 1;
	    return p;
      }
      if (p + 1 < (long)s->len && s->buf[p] == '/' && s->buf[p+1] == '*') {
	    p += 2;
	    while (p + 1 < (long)s->len && !(s->buf[p]=='*' && s->buf[p+1]=='/'))
		  p += 1;
	    return (p + 1 < (long)s->len) ? p + 2 : (long)s->len;
      }
      if (s->buf[p] == '"') {
	    p += 1;
	    while ((size_t)p < s->len && s->buf[p] != '"') {
		  if (s->buf[p] == '\\') p += 1;
		  p += 1;
	    }
	    return ((size_t)p < s->len) ? p + 1 : (long)s->len;
      }
      return p;
}

/* Whole-word keyword match at offset P. */
static int src_kw_at(const struct src_file*s, long p, const char*kw)
{
      size_t kl = strlen(kw);
      char c;
      if (p < 0 || (size_t)p + kl > s->len) return 0;
      if (memcmp(s->buf + p, kw, kl) != 0) return 0;
      if (p > 0) {
	    c = s->buf[p-1];
	    if (isalnum((unsigned char)c) || c == '_' || c == '$') return 0;
      }
      c = ((size_t)p + kl < s->len) ? s->buf[p+kl] : 0;
      if (isalnum((unsigned char)c) || c == '_' || c == '$') return 0;
      return 1;
}

/* Offset of the terminating ';' at bracket depth 0, from FROM. */
static long src_scan_semi(const struct src_file*s, long from)
{
      long p = from;
      int depth = 0;
      while ((size_t)p < s->len) {
	    long q = src_skip_cs(s, p);
	    if (q != p) { p = q; continue; }
	    switch (s->buf[p]) {
		case '(': case '[': case '{': depth += 1; break;
		case ')': case ']': case '}': if (depth>0) depth -= 1; break;
		case ';': if (depth == 0) return p; break;
		default: break;
	    }
	    p += 1;
      }
      return -1;
}

/* From FROM, the first depth-0 'begin' keyword or ';'.  Sets *IS_BEGIN. */
static long src_scan_begin_or_semi(const struct src_file*s, long from,
				   int*is_begin)
{
      long p = from;
      int depth = 0;
      *is_begin = 0;
      while ((size_t)p < s->len) {
	    long q = src_skip_cs(s, p);
	    if (q != p) { p = q; continue; }
	    switch (s->buf[p]) {
		case '(': case '[': case '{': depth += 1; p += 1; continue;
		case ')': case ']': case '}': if (depth>0) depth -= 1; p += 1;
		      continue;
		default: break;
	    }
	    if (depth == 0) {
		  if (s->buf[p] == ';') return p;
		  if (src_kw_at(s, p, "begin")) { *is_begin = 1; return p; }
	    }
	    p += 1;
      }
      return -1;
}

/* Matching 'end' of the 'begin' at BEGIN_OFF (nested begin/end aware). */
static long src_scan_matching_end(const struct src_file*s, long begin_off)
{
      long p = begin_off;
      int depth = 0;
      while ((size_t)p < s->len) {
	    long q = src_skip_cs(s, p);
	    if (q != p) { p = q; continue; }
	    if (src_kw_at(s, p, "begin")) { depth += 1; p += 5; continue; }
	    if (src_kw_at(s, p, "end")) {
		  depth -= 1;
		  if (depth == 0) return p;
		  p += 3; continue;
	    }
	    p += 1;
      }
      return -1;
}

/* First whole-word KW at or after FROM. */
static long src_scan_kw(const struct src_file*s, long from, const char*kw)
{
      long p = from;
      while ((size_t)p < s->len) {
	    long q = src_skip_cs(s, p);
	    if (q != p) { p = q; continue; }
	    if (src_kw_at(s, p, kw)) return p;
	    p += 1;
      }
      return -1;
}

/* ================================================================== */
/* Position emission: the compact "start:begin:end" byte-offset form.  */
/* Missing parts are left empty ("S::", "S::E").                       */
/* ================================================================== */

static void put_pos3(long start, long begin, long end)
{
      putc('"', out);
      if (start >= 0) fprintf(out, "%ld", start);
      putc(':', out);
      if (begin >= 0) fprintf(out, "%ld", begin);
      putc(':', out);
      if (end   >= 0) fprintf(out, "%ld", end);
      putc('"', out);
}

/* A plain point: "start::". */
static void put_point(const char*file, unsigned line)
{
      struct src_file*s = src_get(file);
      put_pos3(src_line_start(s, line), -1, -1);
}

/* A span ending at keyword END_KW (no begin): "start::end".  Used for a
   module definition ("module" .. "endmodule"). */
static void put_span_kw(const char*file, unsigned line, const char*end_kw)
{
      struct src_file*s = src_get(file);
      long st = src_line_start(s, line);
      long en = (s && st >= 0) ? src_scan_kw(s, st, end_kw) : -1;
      put_pos3(st, -1, en);
}

/* A span ending at the terminating ';' (no begin): "start::end".  Used
   for instances and continuous assignments. */
static void put_span_semi(const char*file, unsigned line)
{
      struct src_file*s = src_get(file);
      long st = src_line_start(s, line);
      long en = (s && st >= 0) ? src_scan_semi(s, st) : -1;
      put_pos3(st, -1, en);
}

/* A procedural/generate block span: "start:begin:end" when wrapped in a
   begin..end block, else "start::end" with end at the ';' of the single
   statement. */
static void put_block_span(const char*file, unsigned line)
{
      struct src_file*s = src_get(file);
      long st = src_line_start(s, line);
      long bg = -1, en = -1;
      if (s && st >= 0) {
	    int is_begin = 0;
	    long hit = src_scan_begin_or_semi(s, st, &is_begin);
	    if (hit >= 0 && is_begin) { bg = hit; en = src_scan_matching_end(s, hit); }
	    else if (hit >= 0)        { en = hit; }
      }
      put_pos3(st, bg, en);
}

static void put_int_array(const int*a, size_t n)
{
      size_t i;
      putc('[', out);
      for (i = 0 ; i < n ; i += 1) {
	    if (i) fputs(", ", out);
	    fprintf(out, "%d", a[i]);
      }
      putc(']', out);
}

static void put_str_array(const char**a, size_t n)
{
      size_t i;
      putc('[', out);
      for (i = 0 ; i < n ; i += 1) {
	    if (i) fputs(", ", out);
	    put_jstr(a[i]);
      }
      putc(']', out);
}

static const char*dir_str(ivl_signal_port_t p)
{
      switch (p) {
	  case IVL_SIP_INPUT:  return "in";
	  case IVL_SIP_OUTPUT: return "out";
	  case IVL_SIP_INOUT:  return "inout";
	  default:             return "";
      }
}

static void put_sig_type(ivl_signal_t sig)
{
      unsigned d, nd = ivl_signal_packed_dimensions(sig);
      const char*base;
      switch (ivl_signal_data_type(sig)) {
	  case IVL_VT_REAL:   base = "real";   break;
	  case IVL_VT_STRING: base = "string"; break;
	  case IVL_VT_BOOL:   base = "bit";    break;
	  case IVL_VT_LOGIC:
	  default:            base = "logic";  break;
      }
      putc('"', out);
      fputs(base, out);
      for (d = 0 ; d < nd ; d += 1)
	    fprintf(out, "[%d:%d]",
		    ivl_signal_packed_msb(sig, d),
		    ivl_signal_packed_lsb(sig, d));
      putc('"', out);
}

static void put_param_value(ivl_parameter_t par)
{
      ivl_expr_t e = par ? ivl_parameter_expr(par) : 0;
      if (e && ivl_expr_type(e) == IVL_EX_NUMBER) {
	    const char*bits = ivl_expr_bits(e);
	    unsigned w = ivl_expr_width(e), i;
	    int pure = 1;
	    if (bits) {
		  for (i = 0 ; i < w ; i += 1)
			if (bits[i] != '0' && bits[i] != '1') { pure = 0; break; }
		  if (pure && w <= 64) {
			uint64_t v = 0;
			for (i = 0 ; i < w ; i += 1)
			      if (bits[i] == '1') v |= (uint64_t)1 << i;
			fprintf(out, "%" PRIu64, v);
			return;
		  }
		  putc('"', out);
		  for (i = w ; i > 0 ; i -= 1) putc(bits[i-1], out);
		  putc('"', out);
		  return;
      }
      }
      fputs("null", out);
}

/* ================================================================== */
/* Module / file collection (Phase 1)                                 */
/* ================================================================== */

static void str_set_add(struct str_set*set, const char*s)
{
      size_t i;
      if (!s || s[0] == 0) return;
      for (i = 0 ; i < set->count ; i += 1)
	    if (strcmp(set->items[i], s) == 0) return;
      if (set->count == set->cap) {
	    size_t nc = set->cap ? set->cap * 2 : 16;
	    const char**g = (const char**)realloc(set->items, nc*sizeof(const char*));
	    if (!g) { flow_errors += 1; return; }
	    set->items = g; set->cap = nc;
      }
      set->items[set->count++] = s;
}

static int module_seen(ivl_scope_t sc)
{
      size_t i;
      const char*tn = ivl_scope_tname(sc);
      for (i = 0 ; i < modules.count ; i += 1)
	    if (strcmp(ivl_scope_tname(modules.items[i]), tn) == 0) return 1;
      return 0;
}

static int collect_modules(ivl_scope_t sc, void*cd)
{
      (void)cd;
      if (ivl_scope_type(sc) == IVL_SCT_MODULE) {
	    str_set_add(&files, ivl_scope_file(sc));
	    if (!module_seen(sc)) scope_set_push(&modules, sc);
      }
      ivl_scope_children(sc, collect_modules, 0);
      return 0;
}

/* ================================================================== */
/* port_map (Phase 1)                                                 */
/* ================================================================== */

/* True if `target` is in the enclosing chain of `port_scope`: the
   immediate parent and any transparent generate/begin frames above it,
   up to and including the first enclosing module. This is exactly the
   set of scopes whose nets are valid port actuals -- it stops at the
   instantiating module so a same-named net further up doesn't leak in. */
static int in_enclosing_chain(ivl_scope_t target, ivl_scope_t port_scope)
{
      ivl_scope_t p = ivl_scope_parent(port_scope);
      while (p) {
	    if (p == target) return 1;
	    if (ivl_scope_type(p) == IVL_SCT_MODULE) return 0;
	    p = ivl_scope_parent(p);
      }
      return 0;
}

/* An actual is a signal sharing the port's nexus that lives in the
   instantiating scope (across transparent generate/begin frames). */
static int sig_is_actual(ivl_signal_t s, ivl_signal_t port_sig)
{
      return s && s != port_sig
	    && in_enclosing_chain(ivl_signal_scope(s), ivl_signal_scope(port_sig));
}

static void emit_port_assoc(ivl_signal_t port_sig, int first)
{
      const char*formal = ivl_signal_basename(port_sig);
      ivl_nexus_t nex = ivl_signal_nex(port_sig, 0);
      const char*actual_first = 0;
      unsigned np, i;
      int afirst;

      if (!first) fputs(", ", out);
      fputs("{\"formal\": ", out);  put_jstr(formal);
      fputs(", \"fbase\": ", out);  put_jstr(formal);

      np = nex ? ivl_nexus_ptrs(nex) : 0;
      for (i = 0 ; i < np ; i += 1) {
	    ivl_signal_t s = ivl_nexus_ptr_sig(ivl_nexus_ptr(nex, i));
	    if (sig_is_actual(s, port_sig)) {
		  actual_first = ivl_signal_basename(s);
		  break;
	    }
      }
      fputs(", \"actual\": ", out);
      put_jstr(actual_first ? actual_first : formal);

      fputs(", \"actuals\": [", out);
      afirst = 1;
      for (i = 0 ; i < np ; i += 1) {
	    ivl_signal_t s = ivl_nexus_ptr_sig(ivl_nexus_ptr(nex, i));
	    if (sig_is_actual(s, port_sig)) {
		  if (!afirst) fputs(", ", out);
		  afirst = 0;
		  put_jstr(ivl_signal_basename(s));
	    }
      }
      fputs("]}", out);
}

static void emit_port_map(ivl_scope_t scope)
{
      unsigned nsig = ivl_scope_sigs(scope), i;
      int first = 1;
      for (i = 0 ; i < nsig ; i += 1) {
	    ivl_signal_t sig = ivl_scope_sig(scope, i);
	    if (ivl_signal_port(sig) == IVL_SIP_NONE) continue;
	    emit_port_assoc(sig, first);
	    first = 0;
      }
}

static void emit_generic_map(ivl_scope_t scope)
{
      unsigned np = ivl_scope_params(scope), i;
      int first = 1;
      for (i = 0 ; i < np ; i += 1) {
	    ivl_parameter_t par = ivl_scope_param(scope, i);
	    if (!first) fputs(", ", out);
	    first = 0;
	    fputs("{\"formal\": ", out);  put_jstr(ivl_parameter_basename(par));
	    fputs(", \"value\": ", out);  put_param_value(par);
	    putc('}', out);
      }
}

/* ================================================================== */
/* modules[] (Phase 1)                                                */
/* ================================================================== */

/* ------------------------------------------------------------------ */
/* Name-based modules[].processes[] (what the consumer traverses)      */
/* ------------------------------------------------------------------ */

struct nameset { const char**items; size_t n, cap; };
static void nameset_add(struct nameset*s, const char*name)
{
      if (name) stradd(&s->items, &s->n, &s->cap, name);
}

/* Best signal basename for a nexus, preferring one in `prefer` scope. */
static const char*nexus_signame(ivl_nexus_t nex, ivl_scope_t prefer)
{
      unsigned np = nex ? ivl_nexus_ptrs(nex) : 0, i;
      const char*any = 0;
      for (i = 0 ; i < np ; i += 1) {
	    ivl_signal_t s = ivl_nexus_ptr_sig(ivl_nexus_ptr(nex, i));
	    if (!s) continue;
	    if (!any) any = ivl_signal_basename(s);
	    if (ivl_signal_scope(s) == prefer)
		  return ivl_signal_basename(s);
      }
      return any;
}

static void cn_expr(ivl_expr_t e, struct nameset*reads)
{
      unsigned i, n;
      if (!e) return;
      switch (ivl_expr_type(e)) {
	  case IVL_EX_SIGNAL:
	  case IVL_EX_ARRAY:
	  case IVL_EX_MEMORY:
	    if (ivl_expr_signal(e))
		  nameset_add(reads, ivl_signal_basename(ivl_expr_signal(e)));
	    cn_expr(ivl_expr_oper1(e), reads);
	    break;
	  case IVL_EX_UNARY:
	    cn_expr(ivl_expr_oper1(e), reads);
	    break;
	  case IVL_EX_BINARY:
	  case IVL_EX_SELECT:
	    cn_expr(ivl_expr_oper1(e), reads);
	    cn_expr(ivl_expr_oper2(e), reads);
	    break;
	  case IVL_EX_TERNARY:
	    cn_expr(ivl_expr_oper1(e), reads);
	    cn_expr(ivl_expr_oper2(e), reads);
	    cn_expr(ivl_expr_oper3(e), reads);
	    break;
	  case IVL_EX_CONCAT:
	  case IVL_EX_SFUNC:
	  case IVL_EX_UFUNC:
	    n = ivl_expr_parms(e);
	    for (i = 0 ; i < n ; i += 1)
		  cn_expr(ivl_expr_parm(e, i), reads);
	    break;
	  default:
	    break;
      }
}

static void cn_stmt(ivl_statement_t s, struct nameset*reads,
		    struct nameset*drives, struct nameset*sens,
		    ivl_scope_t scope)
{
      unsigned i, n;
      if (!s) return;
      switch (ivl_statement_type(s)) {
	  case IVL_ST_BLOCK:
	  case IVL_ST_FORK:
	  case IVL_ST_FORK_JOIN_ANY:
	  case IVL_ST_FORK_JOIN_NONE:
	    n = ivl_stmt_block_count(s);
	    for (i = 0 ; i < n ; i += 1)
		  cn_stmt(ivl_stmt_block_stmt(s, i), reads, drives, sens, scope);
	    break;
	  case IVL_ST_ASSIGN:
	  case IVL_ST_ASSIGN_NB:
	  case IVL_ST_CASSIGN:
	  case IVL_ST_FORCE:
	    n = ivl_stmt_lvals(s);
	    for (i = 0 ; i < n ; i += 1) {
		  ivl_signal_t sig = lval_sig(ivl_stmt_lval(s, i));
		  if (sig) nameset_add(drives, ivl_signal_basename(sig));
		  cn_expr(ivl_lval_part_off(ivl_stmt_lval(s, i)), reads);
		  cn_expr(ivl_lval_idx(ivl_stmt_lval(s, i)), reads);
	    }
	    cn_expr(ivl_stmt_rval(s), reads);
	    break;
	  case IVL_ST_CONDIT:
	    cn_expr(ivl_stmt_cond_expr(s), reads);
	    cn_stmt(ivl_stmt_cond_true(s), reads, drives, sens, scope);
	    cn_stmt(ivl_stmt_cond_false(s), reads, drives, sens, scope);
	    break;
	  case IVL_ST_CASE:
	  case IVL_ST_CASEX:
	  case IVL_ST_CASEZ:
	  case IVL_ST_CASER:
	    cn_expr(ivl_stmt_cond_expr(s), reads);
	    n = ivl_stmt_case_count(s);
	    for (i = 0 ; i < n ; i += 1) {
		  cn_expr(ivl_stmt_case_expr(s, i), reads);
		  cn_stmt(ivl_stmt_case_stmt(s, i), reads, drives, sens, scope);
	    }
	    break;
	  case IVL_ST_WAIT: {
	    unsigned ne = ivl_stmt_nevent(s), k;
	    for (i = 0 ; i < ne ; i += 1) {
		  ivl_event_t ev = ivl_stmt_events(s, i);
		  if (!ev) continue;
		  for (k = 0 ; k < ivl_event_npos(ev) ; k += 1) {
			const char*nm = nexus_signame(ivl_event_pos(ev,k), scope);
			nameset_add(sens, nm); nameset_add(reads, nm);
		  }
		  for (k = 0 ; k < ivl_event_nneg(ev) ; k += 1) {
			const char*nm = nexus_signame(ivl_event_neg(ev,k), scope);
			nameset_add(sens, nm); nameset_add(reads, nm);
		  }
		  for (k = 0 ; k < ivl_event_nany(ev) ; k += 1) {
			const char*nm = nexus_signame(ivl_event_any(ev,k), scope);
			nameset_add(sens, nm); nameset_add(reads, nm);
		  }
	    }
	    cn_stmt(ivl_stmt_sub_stmt(s), reads, drives, sens, scope);
	    break;
	  }
	  case IVL_ST_DELAY:
	    cn_stmt(ivl_stmt_sub_stmt(s), reads, drives, sens, scope);
	    break;
	  case IVL_ST_DELAYX:
	    cn_expr(ivl_stmt_delay_expr(s), reads);
	    cn_stmt(ivl_stmt_sub_stmt(s), reads, drives, sens, scope);
	    break;
	  case IVL_ST_FORLOOP:
	    cn_stmt(ivl_stmt_init_stmt(s), reads, drives, sens, scope);
	    cn_expr(ivl_stmt_cond_expr(s), reads);
	    cn_stmt(ivl_stmt_step_stmt(s), reads, drives, sens, scope);
	    cn_stmt(ivl_stmt_sub_stmt(s), reads, drives, sens, scope);
	    break;
	  case IVL_ST_WHILE:
	  case IVL_ST_DO_WHILE:
	    cn_expr(ivl_stmt_cond_expr(s), reads);
	    cn_stmt(ivl_stmt_sub_stmt(s), reads, drives, sens, scope);
	    break;
	  case IVL_ST_STASK:
	    n = ivl_stmt_parm_count(s);
	    for (i = 0 ; i < n ; i += 1)
		  cn_expr(ivl_stmt_parm(s, i), reads);
	    break;
	  default:
	    break;
      }
}

static void put_name_array(struct nameset*ns)
{
      size_t i;
      putc('[', out);
      for (i = 0 ; i < ns->n ; i += 1) {
	    if (i) fputs(", ", out);
	    put_jstr(ns->items[i]);
      }
      putc(']', out);
}

/* The enclosing module of a scope: walk up through transparent
   generate/begin/fork scopes to the first IVL_SCT_MODULE ancestor (or
   the scope itself). Generate bodies are part of their module, so their
   dataflow is attributed to that module. */
static ivl_scope_t owning_module(ivl_scope_t sc)
{
      while (sc && ivl_scope_type(sc) != IVL_SCT_MODULE)
	    sc = ivl_scope_parent(sc);
      return sc;
}

/* Emit the name-based processes[] for the processes owned by module
   `scope` -- those living directly in it or in a transparent
   generate/begin scope nested under it (but not under a child module). */
static void emit_processes(ivl_scope_t scope)
{
      size_t pi;
      int first = 1;
      for (pi = 0 ; pi < proclist.count ; pi += 1) {
	    ivl_process_t proc = proclist.items[pi];
	    ivl_scope_t pscope = ivl_process_scope(proc);
	    ivl_statement_t st;
	    struct nameset reads = {0,0,0}, drives = {0,0,0}, sens = {0,0,0};
	    const char*tname;
	    const char*file;
	    unsigned line;
	    char label[64];

	    if (owning_module(pscope) != scope)
		  continue;
	    st = ivl_process_stmt(proc);
	    line = st ? ivl_stmt_lineno(st) : ivl_scope_lineno(pscope);
	    file = st ? ivl_stmt_file(st) : ivl_scope_file(pscope);
	    switch (ivl_process_type(proc)) {
		case IVL_PR_INITIAL:      tname = "initial";      break;
		case IVL_PR_FINAL:        tname = "final";        break;
		case IVL_PR_ALWAYS_COMB:  tname = "always_comb";  break;
		case IVL_PR_ALWAYS_FF:    tname = "always_ff";    break;
		case IVL_PR_ALWAYS_LATCH: tname = "always_latch"; break;
		default:                  tname = "always";       break;
	    }
	    snprintf(label, sizeof label, "%s@%u", tname, line);

	    cn_stmt(st, &reads, &drives, &sens, pscope);

	    if (!first) fputs(", ", out);
	    first = 0;
	    fputs("{\"label\": ", out);        put_jstr(label);
	    fputs(", \"pos\": ", out);         put_block_span(file, line);
	    fputs(", \"sensitivity\": ", out); put_name_array(&sens);
	    fputs(", \"drives\": ", out);      put_name_array(&drives);
	    fputs(", \"reads\": ", out);       put_name_array(&reads);
	    putc('}', out);

	    free(reads.items); free(drives.items); free(sens.items);
      }
}

/* Skip iverilog's synthetic temporaries (e.g. "_ivl_3") so the
   name-based assignment view stays in terms of real signals. */
static int synth_name(const char*s)
{
      return s == 0 || s[0] == 0 || strncmp(s, "_ivl_", 5) == 0;
}

/* Collect the real (non-synthetic) source signal names in the backward
   combinational cone of a net, following the integer graph through
   synthetic intermediate nets. Stops at real names and at process /
   sequential drivers. `scope` resolves local signal names. */
static void cone_reads(int net_id, ivl_scope_t scope,
		       struct nameset*acc, char*visited)
{
      struct net_ent*ne;
      size_t di, ri;
      if (net_id < 1 || (size_t)net_id > nets.count || visited[net_id])
	    return;
      visited[net_id] = 1;
      ne = &nets.items[net_id - 1];
      for (di = 0 ; di < ne->ndrivers ; di += 1) {
	    struct cell_ent*c = &cells.items[ne->drivers[di] - 1];
	    if (strcmp(c->kind, "process") == 0)
		  continue;  /* sequential boundary -- a process output */
	    for (ri = 0 ; ri < c->nr ; ri += 1) {
		  int rid = c->reads[ri];
		  const char*nm = nexus_signame(nets.items[rid-1].nex, scope);
		  if (!synth_name(nm))
			nameset_add(acc, nm);
		  else
			cone_reads(rid, scope, acc, visited);
	    }
      }
}

/* True if the net is driven by combinational logic (gate/lpm/const) and
   not by a process (a process output belongs in processes[], not here). */
static int net_comb_driven(int net_id)
{
      struct net_ent*ne;
      size_t di;
      int comb = 0;
      if (net_id < 1 || (size_t)net_id > nets.count)
	    return 0;
      ne = &nets.items[net_id - 1];
      for (di = 0 ; di < ne->ndrivers ; di += 1) {
	    const char*k = cells.items[ne->drivers[di] - 1].kind;
	    if (strcmp(k, "process") == 0) return 0;
	    comb = 1;
      }
      return comb;
}

/* Emit name-based continuous assigns for the real signals of one scope
   that are driven combinationally, with transitive cone reads; recurse
   through transparent generate/begin frames. */
static void emit_assigns_rec(ivl_scope_t scope, int*first)
{
      unsigned ns = ivl_scope_sigs(scope), i;
      size_t nc, ci;
      char*visited = (char*)calloc(nets.count + 1, 1);

      for (i = 0 ; i < ns ; i += 1) {
	    ivl_signal_t sig = ivl_scope_sig(scope, i);
	    const char*name = ivl_signal_basename(sig);
	    int nid;
	    struct nameset reads = {0,0,0};
	    if (synth_name(name) || ivl_signal_local(sig))
		  continue;
	    if (ivl_signal_port(sig) == IVL_SIP_INPUT)
		  continue;  /* inputs are driven from outside */
	    nid = net_of_sig(sig, 0);
	    if (!net_comb_driven(nid))
		  continue;
	    memset(visited, 0, nets.count + 1);
	    cone_reads(nid, scope, &reads, visited);

	    if (!*first) fputs(", ", out);
	    *first = 0;
	    fputs("{\"target\": ", out);  put_jstr(name);
	    fputs(", \"reads\": ", out);  put_name_array(&reads);
	    fputs(", \"pos\": ", out);
	    put_point(ivl_signal_file(sig), ivl_signal_lineno(sig));
	    putc('}', out);
	    free(reads.items);
      }
      free(visited);

      nc = ivl_scope_childs(scope);
      for (ci = 0 ; ci < nc ; ci += 1) {
	    ivl_scope_t ch = ivl_scope_child(scope, ci);
	    if (ivl_scope_type(ch) != IVL_SCT_MODULE)
		  emit_assigns_rec(ch, first);  /* transparent generate/begin */
      }
}

static void emit_assignments(ivl_scope_t scope)
{
      int first = 1;
      emit_assigns_rec(scope, &first);
}

static void emit_ports(ivl_scope_t scope)
{
      unsigned nsig = ivl_scope_sigs(scope), i;
      int first = 1;
      for (i = 0 ; i < nsig ; i += 1) {
	    ivl_signal_t sig = ivl_scope_sig(scope, i);
	    if (ivl_signal_port(sig) == IVL_SIP_NONE) continue;
	    if (!first) fputs(", ", out);
	    first = 0;
	    fputs("{\"name\": ", out);  put_jstr(ivl_signal_basename(sig));
	    fputs(", \"dir\": ", out);  put_jstr(dir_str(ivl_signal_port(sig)));
	    fputs(", \"type\": ", out); put_sig_type(sig);
	    fputs(", \"pos\": ", out);
	    put_point(ivl_signal_file(sig), ivl_signal_lineno(sig));
	    putc('}', out);
      }
}

static void emit_generics(ivl_scope_t scope)
{
      unsigned np = ivl_scope_params(scope), i;
      int first = 1;
      for (i = 0 ; i < np ; i += 1) {
	    ivl_parameter_t par = ivl_scope_param(scope, i);
	    if (!first) fputs(", ", out);
	    first = 0;
	    fputs("{\"name\": ", out);  put_jstr(ivl_parameter_basename(par));
	    fputs(", \"type\": ", out); put_jstr("parameter");
	    fputs(", \"pos\": ", out);
	    put_point(ivl_parameter_file(par), ivl_parameter_lineno(par));
	    putc('}', out);
      }
}

static void emit_signals(ivl_scope_t scope)
{
      unsigned nsig = ivl_scope_sigs(scope), i;
      int first = 1;
      for (i = 0 ; i < nsig ; i += 1) {
	    ivl_signal_t sig = ivl_scope_sig(scope, i);
	    if (ivl_signal_port(sig) != IVL_SIP_NONE) continue;
	    if (ivl_signal_local(sig)) continue;
	    if (!first) fputs(", ", out);
	    first = 0;
	    fputs("{\"name\": ", out);   put_jstr(ivl_signal_basename(sig));
	    fputs(", \"type\": ", out);  put_sig_type(sig);
	    fputs(", \"skind\": ", out); put_jstr("signal");
	    fputs(", \"pos\": ", out);
	    put_point(ivl_signal_file(sig), ivl_signal_lineno(sig));
	    putc('}', out);
      }
}

static void emit_instance_obj(ivl_scope_t child, int first)
{
      if (!first) fputs(", ", out);
      fputs("\n        {\"label\": ", out);  put_jstr(ivl_scope_basename(child));
      fputs(", \"module\": ", out);          put_jstr(ivl_scope_tname(child));
      fputs(", \"is_module_inst\": true, \"pos\": ", out);
      put_span_semi(ivl_scope_file(child), ivl_scope_lineno(child));
      fputs(", \"generic_map\": [", out);  emit_generic_map(child);
      fputs("], \"port_map\": [", out);    emit_port_map(child);
      fputs("]}", out);
}

static void emit_module_instances(ivl_scope_t scope, int*first)
{
      size_t n = ivl_scope_childs(scope), i;
      for (i = 0 ; i < n ; i += 1) {
	    ivl_scope_t ch = ivl_scope_child(scope, i);
	    if (ivl_scope_type(ch) == IVL_SCT_MODULE) {
		  emit_instance_obj(ch, *first);
		  *first = 0;
	    } else {
		  emit_module_instances(ch, first);
	    }
      }
}

static void emit_module(ivl_scope_t scope, int first)
{
      int ifirst = 1;
      if (!first) fputs(",", out);
      fputs("\n    {\"name\": ", out);  put_jstr(ivl_scope_tname(scope));
      fputs(", \"kind\": ", out);       put_jstr("module");
      fputs(", \"lang\": ", out);       put_jstr("verilog");
      fputs(", \"file\": ", out);       put_jstr(ivl_scope_def_file(scope));
      /* module definition span: "module" .. "endmodule" (no begin). */
      fputs(", \"pos\": ", out);
      put_span_kw(ivl_scope_def_file(scope), ivl_scope_def_lineno(scope),
		  "endmodule");
      fputs(", \"arch\": ", out);       put_jstr("");
      fputs(",\n      \"ports\": [", out);     emit_ports(scope);
      fputs("],\n      \"generics\": [", out); emit_generics(scope);
      fputs("],\n      \"signals\": [", out);  emit_signals(scope);
      fputs("],\n      \"constants\": [],\n      \"processes\": [", out);
      emit_processes(scope);
      fputs("],\n      \"assignments\": [", out);
      emit_assignments(scope);
      fputs("], \"generates\": [],", out);
      fputs("\n      \"instances\": [", out);
      emit_module_instances(scope, &ifirst);
      fputs("]}", out);
}

/* ================================================================== */
/* hierarchy[] (Phase 1)                                              */
/* ================================================================== */

static void emit_hier_node(ivl_scope_t scope, unsigned indent, int first);
static void emit_frame_node(ivl_scope_t scope, unsigned indent, int first);

/* Dispatch the children of a hierarchy node: module children become
   instance nodes, generate/begin children become transparent frames. */
static void emit_hier_children(ivl_scope_t scope, unsigned indent, int*first)
{
      size_t n = ivl_scope_childs(scope), i;
      for (i = 0 ; i < n ; i += 1) {
	    ivl_scope_t ch = ivl_scope_child(scope, i);
	    ivl_scope_type_t t = ivl_scope_type(ch);
	    if (t == IVL_SCT_MODULE) {
		  emit_hier_node(ch, indent + 2, *first);
		  *first = 0;
	    } else if (t == IVL_SCT_GENERATE || t == IVL_SCT_BEGIN
		       || t == IVL_SCT_FORK) {
		  emit_frame_node(ch, indent + 2, *first);
		  *first = 0;
	    }
      }
}

/* A transparent generate/begin frame: scope:true + generate{kind,label,
   [index]}. iverilog names for-generate scopes "<label>[<index>]" and
   if/named blocks "<label>"; recover the kind/label/index from that. */
static void emit_frame_node(ivl_scope_t scope, unsigned indent, int first)
{
      const char*bn = ivl_scope_basename(scope);
      const char*lb = strchr(bn, '[');
      int cfirst = 1;

      if (!first) fputs(",", out);
      fprintf(out, "\n%*s{\"name\": ", indent, "");
      put_jstr(bn);
      fputs(", \"inst_pos\": ", out);
      put_block_span(ivl_scope_file(scope), ivl_scope_lineno(scope));
      fputs(", \"scope\": true, \"generate\": {\"kind\": ", out);
      if (lb) {
	    /* for-generate: "<label>[<index>]" */
	    size_t llen = (size_t)(lb - bn);
	    int index = atoi(lb + 1);
	    fputs("\"for\", \"label\": \"", out);
	    fwrite(bn, 1, llen, out);
	    fprintf(out, "\", \"index\": %d}", index);
      } else {
	    fputs("\"if\", \"label\": ", out);
	    put_jstr(bn);
	    fputs(", \"arm\": 0}", out);
      }
      fputs(", \"children\": [", out);
      emit_hier_children(scope, indent, &cfirst);
      putc(']', out);
      putc('}', out);
}

static void emit_hier_node(ivl_scope_t scope, unsigned indent, int first)
{
      int cfirst = 1;
      if (!first) fputs(",", out);
      fprintf(out, "\n%*s{\"name\": ", indent, "");  put_jstr(ivl_scope_basename(scope));
      fputs(", \"module\": ", out);                  put_jstr(ivl_scope_tname(scope));
      fputs(", \"arch\": ", out);                    put_jstr("");
      fputs(", \"instance_label\": ", out);          put_jstr(ivl_scope_basename(scope));
      fputs(", \"inst_pos\": ", out);
      put_point(ivl_scope_file(scope), ivl_scope_lineno(scope));
      fputs(", \"generics\": [], \"generic_map\": [", out);  emit_generic_map(scope);
      fputs("], \"port_map\": [", out);                      emit_port_map(scope);
      fputs("], \"children\": [", out);
      emit_hier_children(scope, indent, &cfirst);
      putc(']', out);
      putc('}', out);
}

/* ================================================================== */
/* nets[] / cells[] (Phase 2)                                         */
/* ================================================================== */

static void link_drivers_loads(void)
{
      size_t ci, k;
      for (ci = 0 ; ci < cells.count ; ci += 1) {
	    struct cell_ent*c = &cells.items[ci];
	    int cell_id = (int)(ci + 1);
	    for (k = 0 ; k < c->nd ; k += 1) {
		  int nid = c->drives[k];
		  if (nid >= 1 && (size_t)nid <= nets.count) {
			struct net_ent*ne = &nets.items[nid-1];
			intset_add(&ne->drivers, &ne->ndrivers, &ne->drivers_cap, cell_id);
		  }
	    }
	    for (k = 0 ; k < c->nr ; k += 1) {
		  int nid = c->reads[k];
		  if (nid >= 1 && (size_t)nid <= nets.count) {
			struct net_ent*ne = &nets.items[nid-1];
			intset_add(&ne->loads, &ne->nloads, &ne->loads_cap, cell_id);
		  }
	    }
      }
}

static void emit_cells(void)
{
      size_t i;
      fputs("  \"cells\": [", out);
      for (i = 0 ; i < cells.count ; i += 1) {
	    struct cell_ent*c = &cells.items[i];
	    if (i) putc(',', out);
	    fputs("\n    {\"id\": ", out);   fprintf(out, "%d", (int)(i+1));
	    fputs(", \"kind\": ", out);      put_jstr(c->kind);
	    fputs(", \"label\": ", out);     put_jstr(c->label);
	    fputs(", \"path\": ", out);      put_jstr(c->path);
	    fputs(", \"pos\": ", out);
	    if (strcmp(c->kind, "process") == 0) put_block_span(c->file, c->line);
	    else                                 put_point(c->file, c->line);
	    fputs(", \"clocked\": ", out);   fputs(c->clocked ? "true" : "false", out);
	    fputs(", \"clock_net\": ", out); fprintf(out, "%d", c->clock_net);
	    fputs(", \"drives\": ", out);    put_int_array(c->drives, c->nd);
	    fputs(", \"reads\": ", out);     put_int_array(c->reads, c->nr);
	    putc('}', out);
      }
      fputs("],\n", out);
}

static void emit_nets(void)
{
      size_t i;
      fputs("  \"nets\": [", out);
      for (i = 0 ; i < nets.count ; i += 1) {
	    struct net_ent*n = &nets.items[i];
	    if (i) putc(',', out);
	    fputs("\n    {\"id\": ", out);    fprintf(out, "%d", (int)(i+1));
	    fputs(", \"name\": ", out);       put_jstr(n->name);
	    fputs(", \"path\": ", out);       put_jstr(n->path);
	    fputs(", \"is_port\": ", out);    fputs(n->is_port ? "true" : "false", out);
	    fputs(", \"aliases\": ", out);    put_str_array(n->aliases, n->nalias);
	    fputs(", \"drivers\": ", out);    put_int_array(n->drivers, n->ndrivers);
	    fputs(", \"loads\": ", out);      put_int_array(n->loads, n->nloads);
	    putc('}', out);
      }
      fputs("],\n", out);
}

/* ================================================================== */
/* top-level                                                          */
/* ================================================================== */

int target_design(ivl_design_t des)
{
      ivl_scope_t*roots = 0;
      unsigned nroots = 0, idx;
      int first;

      const char*path = ivl_design_flag(des, "-o");
      if (path == 0 || path[0] == 0) {
	    fprintf(stderr, "tgt-flow: no output file (-o) specified\n");
	    return -1;
      }
      out = fopen(path, "w");
      if (out == 0) { perror(path); return -2; }

      ivl_design_roots(des, &roots, &nroots);

      /* Phase 1 collection. */
      for (idx = 0 ; idx < nroots ; idx += 1)
	    collect_modules(roots[idx], 0);

      /* Phase 2 collection: assign net ids top-down, then build cells. */
      for (idx = 0 ; idx < nroots ; idx += 1)
	    prereg_nets(roots[idx], 0);
      ivl_design_process(des, build_process, 0);
      for (idx = 0 ; idx < nroots ; idx += 1)
	    build_scope_cells(roots[idx], 0);
      for (idx = 0 ; idx < ivl_design_consts(des) ; idx += 1)
	    build_const(ivl_design_const(des, idx));
      link_drivers_loads();

      /* ---- envelope ---- */
      fputs("{\n", out);
      fputs("  \"schema\": \"flowtracer1.verilog.v0\",\n", out);
      fputs("  \"generated_by\": \"iverilog -tflow (native)\",\n", out);
      fputs("  \"iverilog\": {\"version\": ", out);  put_jstr(VERSION);
      fputs(", \"hash\": ", out);                    put_jstr(VERSION_TAG);
      fputs(", \"std\": \"verilog\"},\n", out);
      fputs("  \"lang\": \"verilog\",\n", out);
      fputs("  \"elaborated\": true,\n", out);
      fputs("  \"hierarchy_prov\": \"elaborated\",\n", out);
      fputs("  \"applied_fallbacks\": [],\n", out);
      /* The top is the first module root. Under -g2009/-g2012 a $unit
         compilation-unit package scope is also a root; skip it. */
      fputs("  \"top\": ", out);
      {
	    const char*topname = "";
	    for (idx = 0 ; idx < nroots ; idx += 1) {
		  if (ivl_scope_type(roots[idx]) == IVL_SCT_MODULE) {
			topname = ivl_scope_basename(roots[idx]);
			break;
		  }
	    }
	    put_jstr(topname);
      }
      fputs(",\n", out);

      /* ---- modules[] ---- */
      fputs("  \"modules\": [", out);
      for (idx = 0 ; idx < modules.count ; idx += 1)
	    emit_module(modules.items[idx], idx == 0);
      fputs("],\n", out);
      fputs("  \"packages\": [],\n", out);

      /* ---- hierarchy[] ---- */
      fputs("  \"hierarchy\": [", out);
      first = 1;
      for (idx = 0 ; idx < nroots ; idx += 1) {
	    if (ivl_scope_type(roots[idx]) != IVL_SCT_MODULE) continue;
	    emit_hier_node(roots[idx], 4, first);
	    first = 0;
      }
      fputs("],\n", out);

      /* ---- nets[] / cells[] ---- */
      emit_nets();
      emit_cells();

      /* ---- files[] ---- */
      fputs("  \"files\": [", out);
      for (idx = 0 ; idx < files.count ; idx += 1) {
	    if (idx) fputs(", ", out);
	    put_jstr(files.items[idx]);
      }
      fputs("]\n}\n", out);

      fclose(out);
      return flow_errors;
}

const char* target_query(const char*key)
{
      if (strcmp(key, "version") == 0)
	    return version_string;
      return 0;
}
