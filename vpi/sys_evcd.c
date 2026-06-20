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

# include "sys_priv.h"

/*
 * This file implements the IEEE 1364-2005 Clause 18 Extended VCD
 * ($dumpports) task family. The port direction comes from the vpiPort
 * (vpiPortInfo) meta-data; the value-bearing net is reached through the
 * port's vpiLowConn relationship, and its per-bit drive strength is read
 * with vpiStrengthVal. The output is byte-compatible (for scalar ports)
 * with the GHDL --evcd writer so a single waveform reader works across
 * Verilog and VHDL.
 *
 * PHASE 1: scalar (1-bit) input/output/inout ports -- full pipeline
 * (header, $scope/$var/$upscope, initial checkpoint, value changes).
 * Vector ports are declared with their range but only bit 0 is dumped;
 * the full bus encoding is pinned against the GHDL reference in Phase 2.
 */

# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>
# include  <time.h>
# include  "ivl_alloc.h"

static FILE *evcd_file = NULL;
static int   evcd_no_date = 0;
static PLI_UINT64 evcd_cur_time = 0;
static int   evcd_time_set = 0;
static int   evcd_is_off = 0;
static int   evcd_started = 0;   /* initial checkpoint emitted yet? */

static const char*units_names[] = { "s", "ms", "us", "ns", "ps", "fs" };

/* One tracked port. */
struct evcd_port {
      vpiHandle net;          /* value net (vpiLowConn of the port) */
      vpiHandle inst;         /* the dumped instance scope (module side) */
      int       dir;          /* vpiInput / vpiOutput / vpiInout */
      unsigned  width;
      int       id;           /* EVCD identifier integer */
      struct evcd_port*next;
};

static struct evcd_port*evcd_list = NULL;
static struct evcd_port*evcd_tail = NULL;   /* append here to keep id order */
static int evcd_next_id = 0;

/* ------------------------------------------------------------------ */
/* State-char and strength mapping (GHDL byte-compatible)             */
/* ------------------------------------------------------------------ */

/* Map a 4-state logic value (vpi0/vpi1/vpiZ/vpiX, with vpiH/vpiL folded
   onto 1/0) and a port direction to the EVCD state character. */
static char evcd_state_char(int logic, int dir)
{
      int v;       /* 0, 1, 2=Z, 3=X */
      switch (logic) {
	  case vpi0: case vpiL: v = 0; break;
	  case vpi1: case vpiH: v = 1; break;
	  case vpiZ:            v = 2; break;
	  default:              v = 3; break;
      }
      if (dir == vpiInput) {
	    switch (v) { case 0: return 'D'; case 1: return 'U';
			 case 2: return 'Z'; default: return 'N'; }
      } else if (dir == vpiOutput) {
	    switch (v) { case 0: return 'L'; case 1: return 'H';
			 case 2: return 'T'; default: return 'X'; }
      } else {  /* inout / unknown direction */
	    switch (v) { case 0: return '0'; case 1: return '1';
			 case 2: return 'F'; default: return '?'; }
      }
}

/* The strength0 / strength1 components for a value, using the
   GHDL-compatible convention: a driven level is strong (6), the opposite
   rail is high-Z (0); three-state is 0/0; unknown is 6/6. */
static void evcd_strengths(int logic, int*s0, int*s1)
{
      switch (logic) {
	  case vpi0: case vpiL: *s0 = 6; *s1 = 0; break;
	  case vpi1: case vpiH: *s0 = 0; *s1 = 6; break;
	  case vpiZ:            *s0 = 0; *s1 = 0; break;
	  default:              *s0 = 6; *s1 = 6; break;
      }
}

/* Map (input-side value, output-side value) to the IEEE 1364-2005 18.4.3
   extended-VCD state character for an inout (unknown-direction) port. Value
   codes are 0, 1, 2=X, 3=Z, where Z means that side is not driving. A side
   that is hi-Z lets the other side determine the input/output mnemonic; both
   driving give the agreement (0/1) or conflict (A/a/B/b/C/c) characters. */
static char evcd_inout_char(int in_v, int out_v)
{
      static const char tab[4][4] = {
	    /* out=  0    1    X    Z   */
	    /*in0*/ {'0', 'A', 'a', 'D'},
	    /*in1*/ {'B', '1', 'b', 'U'},
	    /*inX*/ {'C', 'c', '?', 'N'},
	    /*inZ*/ {'L', 'H', 'X', 'F'}
      };
      return tab[in_v & 3][out_v & 3];
}

/* Reduce a vpi logic value to the 0/1/2=X/3=Z code used above. */
static int evcd_vcode(int logic)
{
      switch (logic) {
	  case vpi0: case vpiL: return 0;
	  case vpi1: case vpiH: return 1;
	  case vpiZ:            return 3;
	  default:              return 2;
      }
}

/* Combine a new driver value into one side's accumulated value. Z means the
   driver is not contributing; two real drivers that disagree make X. */
static int evcd_side_combine(int acc, int v)
{
      if (v == 3)   return acc;
      if (acc == 3) return v;
      if (acc == v) return acc;
      return 2;
}

/* The drive strength level (0..7) encoded in a vpi strength bitmask
   (1<<level). Per IEEE 1364-2005 18.4.3.2, levels 5..7 are "strong" and
   1..4 are "weak"; this distinction selects the d/u/l/h conflict states. */
static int evcd_strlevel(unsigned mask)
{
      int lvl = 0;
      while (mask > 1) { mask >>= 1; lvl += 1; }
      return lvl;
}

/* Non-zero if scope `s` is the instance `inst` or nested within it. */
static int evcd_scope_within(vpiHandle s, vpiHandle inst)
{
      int guard = 0;
      while (s && guard < 4096) {
	    if (vpi_compare_objects(s, inst)) return 1;
	    s = vpi_handle(vpiScope, s);
	    guard += 1;
      }
      return 0;
}

/* For a scalar inout port, separate the external (input-side) and module
   (output-side) drives by walking the net's vpiDriver objects and
   classifying each by whether its scope lies inside the dumped instance,
   then map the pair to the EVCD state character. s0 and s1 receive the
   resolved strengths. Returns 0 when the net exposes no drivers (the caller
   then falls back to the plain resolved-value encoding). */
static char evcd_inout_state(struct evcd_port*p, int*s0, int*s1)
{
      vpiHandle it = vpi_iterate(vpiDriver, p->net);
      vpiHandle d;
      int in_v = 3, out_v = 3;        /* side values: 0,1,2=X,3=Z */
      int in_lvl = 0, out_lvl = 0;    /* strongest level seen per side */
      int any = 0;
      char c;
      s_vpi_value rv;

      if (it == NULL) return 0;

      while ((d = vpi_scan(it))) {
	    s_vpi_value val;
	    vpiHandle dscope = vpi_handle(vpiScope, d);
	    int v, lvl;
	    any = 1;
	    val.format = vpiStrengthVal;
	    vpi_get_value(d, &val);
	    v = evcd_vcode((int)val.value.strength[0].logic);
	      /* Strength of the driven rail (s1 for a 1, s0 otherwise). */
	    lvl = evcd_strlevel((v == 1) ? val.value.strength[0].s1
				         : val.value.strength[0].s0);
	    if (dscope && evcd_scope_within(dscope, p->inst)) {
		  out_v = evcd_side_combine(out_v, v);
		  if (lvl > out_lvl) out_lvl = lvl;
	    } else {
		  in_v = evcd_side_combine(in_v, v);
		  if (lvl > in_lvl) in_lvl = lvl;
	    }
      }

      if (!any) return 0;

      rv.format = vpiStrengthVal;
      vpi_get_value(p->net, &rv);
      evcd_strengths((int)rv.value.strength[0].logic, s0, s1);

      c = evcd_inout_char(in_v, out_v);

	/* When both sides drive the same level (0 or 1) but with different
	   strength ranges, the IEEE state is d/u (input stronger) or l/h
	   (output stronger) rather than the plain agreement 0/1. */
      if (in_v == out_v && (in_v == 0 || in_v == 1)) {
	    int in_strong  = in_lvl  >= 5;
	    int out_strong = out_lvl >= 5;
	    if (in_strong && !out_strong)
		  c = (in_v == 0) ? 'd' : 'u';
	    else if (!in_strong && out_strong)
		  c = (in_v == 0) ? 'l' : 'h';
      }
      return c;
}

/* Emit one value record for a port:

     scalar  -> p<state> <s0> <s1> <<id>
     vector  -> p<state...> <s0...> <s1...> <<id>

   For an N-bit port the state and each strength component carry N
   characters, most-significant bit first (matching the [msb:lsb] $var
   declaration), and the encoding is byte-compatible with GHDL --evcd. */
static void evcd_emit_value(struct evcd_port*p)
{
      s_vpi_value val;
      int logic, s0, s1, idx;

      val.format = vpiStrengthVal;
      vpi_get_value(p->net, &val);

      if (p->width <= 1) {
	    logic = (int)val.value.strength[0].logic;
	    evcd_strengths(logic, &s0, &s1);
	      /* For an inout, separate the two drive sides so genuine
		 conflicts become the IEEE conflict-state characters
		 (A/a/B/b/C/c) rather than a generic '?'. Falls back to the
		 resolved-value encoding when no per-driver info exists. */
	    if (p->dir == vpiInout) {
		  int cs0 = s0, cs1 = s1;
		  char cc = evcd_inout_state(p, &cs0, &cs1);
		  if (cc) {
			fprintf(evcd_file, "p%c %d %d <%d\n",
				cc, cs0, cs1, p->id);
			return;
		  }
	    }
	    fprintf(evcd_file, "p%c %d %d <%d\n",
		    evcd_state_char(logic, p->dir), s0, s1, p->id);
	    return;
      }

	/* Bit 0 of the strength array is the LSB; emit MSB first. */
      fputc('p', evcd_file);
      for (idx = (int)p->width - 1 ; idx >= 0 ; idx -= 1)
	    fputc(evcd_state_char((int)val.value.strength[idx].logic, p->dir),
		  evcd_file);
      fputc(' ', evcd_file);
      for (idx = (int)p->width - 1 ; idx >= 0 ; idx -= 1) {
	    evcd_strengths((int)val.value.strength[idx].logic, &s0, &s1);
	    fputc('0' + s0, evcd_file);
      }
      fputc(' ', evcd_file);
      for (idx = (int)p->width - 1 ; idx >= 0 ; idx -= 1) {
	    evcd_strengths((int)val.value.strength[idx].logic, &s0, &s1);
	    fputc('0' + s1, evcd_file);
      }
      fprintf(evcd_file, " <%d\n", p->id);
}

/* ------------------------------------------------------------------ */
/* Time + value-change callback                                       */
/* ------------------------------------------------------------------ */

static PLI_UINT64 evcd_now(void)
{
      s_vpi_time now;
      now.type = vpiSimTime;
      vpi_get_time(0, &now);
      return ((PLI_UINT64)now.high << 32) | (PLI_UINT64)now.low;
}

static void evcd_emit_time(PLI_UINT64 t)
{
      if (!evcd_time_set || t != evcd_cur_time) {
	    fprintf(evcd_file, "#%" PLI_UINT64_FMT "\n", t);
	    evcd_cur_time = t;
	    evcd_time_set = 1;
      }
}

static PLI_INT32 evcd_value_cb(struct t_cb_data*cb)
{
      struct evcd_port*p = (struct evcd_port*)cb->user_data;
	/* Value changes while the design settles at the start time are
	   folded into the initial checkpoint, so ignore them until it has
	   been emitted (at the read-only synch of the current step). */
      if (evcd_file == NULL || evcd_is_off || !evcd_started)
	    return 0;
      evcd_emit_time(evcd_now());
      evcd_emit_value(p);
      return 0;
}

/* ------------------------------------------------------------------ */
/* Header                                                             */
/* ------------------------------------------------------------------ */

static void evcd_emit_header(void)
{
      int prec = vpi_get(vpiTimePrecision, 0);
      unsigned scale = 1, udx = 0;
      time_t walltime;

      time(&walltime);
      assert(prec >= -15);
      while (prec < 0) { udx += 1; prec += 3; }
      while (prec > 0) { scale *= 10; prec -= 1; }

      if (!evcd_no_date) {
	    fprintf(evcd_file, "$date\n\t%s$end\n",
		    asctime(localtime(&walltime)));
      }
      fprintf(evcd_file, "$version\n\tIcarus Verilog\n$end\n");
      fprintf(evcd_file, "$timescale\n\t%u%s\n$end\n", scale, units_names[udx]);
}

/* ------------------------------------------------------------------ */
/* Scope / port scan                                                  */
/* ------------------------------------------------------------------ */

static const char*evcd_dir_name(int dir)
{
      switch (dir) {
	  case vpiInput:  return "input";
	  case vpiOutput: return "output";
	  case vpiInout:  return "inout";
	  default:        return "?";
      }
}

/* Declare every port of one module instance scope: emit its $var record,
   track it, and register a value-change callback on its net. */
static void evcd_scan_scope(vpiHandle scope)
{
      vpiHandle ports, port;

      fprintf(evcd_file, "$scope module %s $end\n",
	      vpi_get_str(vpiFullName, scope));

      ports = vpi_iterate(vpiPort, scope);
      while (ports && (port = vpi_scan(ports))) {
	    struct evcd_port*p;
	    s_cb_data cb;
	    s_vpi_time tm;
	    const char*name = vpi_get_str(vpiName, port);
	    int dir = vpi_get(vpiDirection, port);
	    unsigned width = vpi_get(vpiSize, port);
	    vpiHandle net = vpi_handle(vpiLowConn, port);

	    if (net == NULL) {
		  vpi_printf("EVCD warning: port %s has no value net; "
			     "skipping.\n", name ? name : "?");
		  continue;
	    }

	    p = (struct evcd_port*)calloc(1, sizeof *p);
	    p->net   = net;
	    p->inst  = scope;
	    p->dir   = dir;
	    p->width = width;
	    p->id    = evcd_next_id++;
	    p->next  = NULL;
	      /* Append so checkpoints emit records in ascending id order
		 (matching the GHDL --evcd writer). */
	    if (evcd_tail) evcd_tail->next = p;
	    else           evcd_list = p;
	    evcd_tail = p;

	      /* $var port <size> <<id> <reference> $end. Use the port's
		 actual declared [msb:lsb] range when it is a vector. */
	    if (width > 1 || vpi_get(vpiLeftRange, net) != 0)
		  fprintf(evcd_file, "$var port [%d:%d] <%d %s $end\n",
			  (int)vpi_get(vpiLeftRange, net),
			  (int)vpi_get(vpiRightRange, net), p->id, name);
	    else
		  fprintf(evcd_file, "$var port 1 <%d %s $end\n", p->id, name);

	      /* Track value changes on the port's value net. */
	    memset(&cb, 0, sizeof cb);
	    tm.type = vpiSimTime;
	    cb.reason    = cbValueChange;
	    cb.cb_rtn    = evcd_value_cb;
	    cb.obj       = net;
	    cb.time      = &tm;
	    cb.value     = NULL;
	    cb.user_data = (PLI_BYTE8*)p;
	    vpi_register_cb(&cb);

	    (void)evcd_dir_name;  /* reserved for diagnostics */
      }

      fprintf(evcd_file, "$upscope $end\n");
}

/* Dump the current value of every tracked port (checkpoint). */
static void evcd_checkpoint(void)
{
      struct evcd_port*p;
      for (p = evcd_list ; p ; p = p->next)
	    evcd_emit_value(p);
}

/* ------------------------------------------------------------------ */
/* End of simulation                                                  */
/* ------------------------------------------------------------------ */

static PLI_INT32 evcd_end_cb(struct t_cb_data*cb)
{
      (void)cb;
      if (evcd_file) {
	    fprintf(evcd_file, "#%" PLI_UINT64_FMT "\n", evcd_now());
	    fflush(evcd_file);
      }
      return 0;
}

/* The initial checkpoint, deferred to the read-only synch of the start
   time so it records settled values: #<t> $dumpports <values> $end. */
static PLI_INT32 evcd_initial_cb(struct t_cb_data*cb)
{
      (void)cb;
      if (evcd_file == NULL)
	    return 0;
      evcd_cur_time = evcd_now();
      evcd_time_set = 1;
      fprintf(evcd_file, "#%" PLI_UINT64_FMT "\n$dumpports\n", evcd_cur_time);
      evcd_checkpoint();
      fprintf(evcd_file, "$end\n");
      evcd_started = 1;
      return 0;
}

/* ------------------------------------------------------------------ */
/* $dumpports                                                         */
/* ------------------------------------------------------------------ */

static PLI_INT32 sys_dumpports_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle systf, argv, arg;
      vpiHandle scopes[64];
      unsigned nscopes = 0;
      char*path = NULL;
      s_cb_data cb;
      unsigned i;
      (void)name;

      if (evcd_file != NULL) {
	    vpi_printf("EVCD warning: $dumpports already active; ignored.\n");
	    return 0;
      }

      systf = vpi_handle(vpiSysTfCall, 0);
      argv  = vpi_iterate(vpiArgument, systf);

	/* Arguments are module instances to dump plus, optionally, a
	   trailing file name (string). Anything that is not a module is
	   treated as the file-name expression. */
      while (argv && (arg = vpi_scan(argv))) {
	    if (vpi_get(vpiType, arg) == vpiModule) {
		  if (nscopes < 64)
			scopes[nscopes++] = arg;
	    } else if (path == NULL) {
		  s_vpi_value v;
		  v.format = vpiStringVal;
		  vpi_get_value(arg, &v);
		  if (v.value.str && v.value.str[0])
			path = strdup(v.value.str);
	    }
      }

	/* Default scope = the module containing the call; default file. */
      if (nscopes == 0) {
	    vpiHandle sc = vpi_handle(vpiScope, systf);
	    if (sc) scopes[nscopes++] = sc;
      }
      if (path == NULL)
	    path = strdup("dumpports.vcd");

      evcd_file = fopen(path, "w");
      if (evcd_file == NULL) {
	    vpi_printf("EVCD error: cannot open %s for output.\n", path);
	    free(path);
	    return 0;
      }
      vpi_printf("EVCD info: dumpports file %s opened for output.\n", path);
      free(path);

      evcd_emit_header();
      for (i = 0 ; i < nscopes ; i += 1)
	    evcd_scan_scope(scopes[i]);
      fprintf(evcd_file, "$enddefinitions $end\n");

	/* Defer the initial checkpoint to the read-only synch of the
	   current time so it captures settled values. */
      {
	    s_vpi_time tm;
	    tm.type = vpiSimTime;
	    tm.high = 0;
	    tm.low  = 0;
	    memset(&cb, 0, sizeof cb);
	    cb.reason = cbReadOnlySynch;
	    cb.cb_rtn = evcd_initial_cb;
	    cb.time   = &tm;
	    cb.obj    = NULL;
	    vpi_register_cb(&cb);
      }

	/* Close the file cleanly at the end of simulation. */
      memset(&cb, 0, sizeof cb);
      cb.reason = cbEndOfSimulation;
      cb.cb_rtn = evcd_end_cb;
      cb.obj    = NULL;
      vpi_register_cb(&cb);

      return 0;
}

/* $dumpportsall: force a checkpoint of all port values now. */
static PLI_INT32 sys_dumpportsall_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name;
      if (evcd_file == NULL) return 0;
      evcd_emit_time(evcd_now());
      evcd_checkpoint();
      return 0;
}

/* $dumpportsoff / $dumpportson: suspend / resume dumping. */
static PLI_INT32 sys_dumpportsoff_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      struct evcd_port*p;
      (void)name;
      if (evcd_file == NULL || evcd_is_off) return 0;
      evcd_emit_time(evcd_now());
	/* Checkpoint every bit of every port as X while suspended. */
      for (p = evcd_list ; p ; p = p->next) {
	    int s0, s1;
	    unsigned b;
	    char c = evcd_state_char(vpiX, p->dir);
	    evcd_strengths(vpiX, &s0, &s1);
	    fputc('p', evcd_file);
	    for (b = 0 ; b < p->width ; b += 1) fputc(c, evcd_file);
	    fputc(' ', evcd_file);
	    for (b = 0 ; b < p->width ; b += 1) fputc('0' + s0, evcd_file);
	    fputc(' ', evcd_file);
	    for (b = 0 ; b < p->width ; b += 1) fputc('0' + s1, evcd_file);
	    fprintf(evcd_file, " <%d\n", p->id);
      }
      evcd_is_off = 1;
      return 0;
}

static PLI_INT32 sys_dumpportson_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name;
      if (evcd_file == NULL || !evcd_is_off) return 0;
      evcd_is_off = 0;
      evcd_emit_time(evcd_now());
      evcd_checkpoint();
      return 0;
}

/* $dumpportsflush: flush the output buffer. */
static PLI_INT32 sys_dumpportsflush_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name;
      if (evcd_file) fflush(evcd_file);
      return 0;
}

/* $dumpportslimit(size [, file]): accepted for compatibility. The
   file-size cap is not enforced in Phase 1. */
static PLI_INT32 sys_dumpportslimit_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name;
      return 0;
}

/* ------------------------------------------------------------------ */
/* Registration                                                       */
/* ------------------------------------------------------------------ */

static void register_evcd_task(const char*tfname,
			       PLI_INT32 (*calltf)(ICARUS_VPI_CONST PLI_BYTE8*))
{
      s_vpi_systf_data tf;
      memset(&tf, 0, sizeof tf);
      tf.type      = vpiSysTask;
      tf.tfname    = (PLI_BYTE8*)tfname;
      tf.calltf    = calltf;
      tf.compiletf = NULL;
      tf.sizetf    = NULL;
      tf.user_data = (PLI_BYTE8*)tfname;
      vpi_register_systf(&tf);
}

void sys_evcd_register(void)
{
      register_evcd_task("$dumpports",      sys_dumpports_calltf);
      register_evcd_task("$dumpportsall",   sys_dumpportsall_calltf);
      register_evcd_task("$dumpportsoff",   sys_dumpportsoff_calltf);
      register_evcd_task("$dumpportson",    sys_dumpportson_calltf);
      register_evcd_task("$dumpportsflush", sys_dumpportsflush_calltf);
      register_evcd_task("$dumpportslimit", sys_dumpportslimit_calltf);
}
