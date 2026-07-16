/*
 * Copyright (c) 2012-2025 Stephen Williams (steve@icarus.com)
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

# include  "vvp_priv.h"
# include  <string.h>
# include  <assert.h>

void darray_new(ivl_type_t element_type, unsigned size_reg)
{
      int wid;
      const char*signed_char;
      ivl_variable_type_t type = ivl_type_base(element_type);

      if ((type == IVL_VT_BOOL) || (type == IVL_VT_LOGIC)) {
	    wid = ivl_type_packed_width(element_type);
	    signed_char = ivl_type_signed(element_type) ? "s" : "";
      } else {
	      // REAL or STRING objects are not packable.
	    assert(ivl_type_packed_dimensions(element_type) == 0);
	    wid = 0;
	    signed_char = "";
      }

      switch (type) {
	  case IVL_VT_REAL:
	    fprintf(vvp_out, "    %%new/darray %u, \"r\";\n",
	                     size_reg);
	    break;

	  case IVL_VT_STRING:
	    fprintf(vvp_out, "    %%new/darray %u, \"S\";\n",
	                     size_reg);
	    break;

	  case IVL_VT_BOOL:
	    fprintf(vvp_out, "    %%new/darray %u, \"%sb%d\";\n",
	                     size_reg, signed_char, wid);
	    break;

	  case IVL_VT_LOGIC:
	    fprintf(vvp_out, "    %%new/darray %u, \"%sv%d\";\n",
	                     size_reg, signed_char, wid);
	    break;

	  default:
	    assert(0);
	    break;
      }

      clr_word(size_reg);
}

static int eval_darray_new(ivl_expr_t ex)
{
      int errors = 0;
      unsigned size_reg = allocate_word();
      ivl_expr_t size_expr = ivl_expr_oper1(ex);
      ivl_expr_t init_expr = ivl_expr_oper2(ex);
      draw_eval_expr_into_integer(size_expr, size_reg);

	// The new function has a net_type that contains the details
	// of the type.
      ivl_type_t net_type = ivl_expr_net_type(ex);
      assert(net_type);

      ivl_type_t element_type = ivl_type_element(net_type);
      assert(element_type);

      darray_new(element_type, size_reg);

      if (init_expr && ivl_expr_type(init_expr)==IVL_EX_ARRAY_PATTERN) {
	    unsigned idx;
	    switch (ivl_type_base(element_type)) {
		case IVL_VT_BOOL:
		case IVL_VT_LOGIC:
		  for (idx = 0 ; idx < ivl_expr_parms(init_expr) ; idx += 1) {
			draw_eval_vec4(ivl_expr_parm(init_expr,idx));
			fprintf(vvp_out, "    %%ix/load 3, %u, 0;\n", idx);
			fprintf(vvp_out, "    %%set/dar/obj/vec4 3;\n");
			fprintf(vvp_out, "    %%pop/vec4 1;\n");
		  }
		  break;
		case IVL_VT_REAL:
		  for (idx = 0 ; idx < ivl_expr_parms(init_expr) ; idx += 1) {
			draw_eval_real(ivl_expr_parm(init_expr,idx));
			fprintf(vvp_out, "    %%ix/load 3, %u, 0;\n", idx);
			fprintf(vvp_out, "    %%set/dar/obj/real 3;\n");
			fprintf(vvp_out, "    %%pop/real 1;\n");
		  }
		  break;
		case IVL_VT_STRING:
		  for (idx = 0 ; idx < ivl_expr_parms(init_expr) ; idx += 1) {
			draw_eval_string(ivl_expr_parm(init_expr,idx));
			fprintf(vvp_out, "    %%ix/load 3, %u, 0;\n", idx);
			fprintf(vvp_out, "    %%set/dar/obj/str 3;\n");
			fprintf(vvp_out, "    %%pop/str 1;\n");
		  }
		  break;
		default:
		  fprintf(vvp_out, "; ERROR: Sorry, this type not supported here.\n");
		  errors += 1;
		  break;
	    }
      } else if (init_expr && (ivl_expr_value(init_expr) == IVL_VT_DARRAY)) {
		  ivl_signal_t sig = ivl_expr_signal(init_expr);
		  fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
		  fprintf(vvp_out, "    %%scopy;\n");

      } else if (init_expr && number_is_immediate(size_expr,32,0)) {
	      /* In this case, there is an init expression, the
		 expression is NOT an array_pattern, and the size
		 expression used to calculate the size of the array is
		 a constant. Generate an unrolled set of assignments. */
	    long idx;
	    long cnt = get_number_immediate(size_expr);
	    unsigned wid;
	    switch (ivl_type_base(element_type)) {
		case IVL_VT_BOOL:
		case IVL_VT_LOGIC:
		  wid = ivl_type_packed_width(element_type);
		  for (idx = 0 ; idx < cnt ; idx += 1) {
			draw_eval_vec4(init_expr);
			fprintf(vvp_out, "    %%parti/%c %u, %ld, 6;\n",
                                ivl_expr_signed(init_expr) ? 's' : 'u', wid, idx * wid);
			fprintf(vvp_out, "    %%ix/load 3, %ld, 0;\n", cnt - idx - 1);
			fprintf(vvp_out, "    %%set/dar/obj/vec4 3;\n");
			fprintf(vvp_out, "    %%pop/vec4 1;\n");
		  }
		  break;
		case IVL_VT_REAL:
		  draw_eval_real(init_expr);
		  for (idx = 0 ; idx < cnt ; idx += 1) {
			fprintf(vvp_out, "    %%ix/load 3, %ld, 0;\n", idx);
			fprintf(vvp_out, "    %%set/dar/obj/real 3;\n");
		  }
		  fprintf(vvp_out, "    %%pop/real 1;\n");
		  break;
		case IVL_VT_STRING:
		  draw_eval_string(init_expr);
		  for (idx = 0 ; idx < cnt ; idx += 1) {
			fprintf(vvp_out, "    %%ix/load 3, %ld, 0;\n", idx);
			fprintf(vvp_out, "    %%set/dar/obj/str 3;\n");
		  }
		  fprintf(vvp_out, "    %%pop/str 1;\n");
		  break;
		default:
		  fprintf(vvp_out, "; ERROR: Sorry, this type not supported here.\n");
		  errors += 1;
		  break;
	    }

      } else if (init_expr) {
	    fprintf(vvp_out, "; ERROR: Sorry, I don't know how to work with this size expr.\n");
	    errors += 1;
      }

      return errors;
}

static int eval_class_new(ivl_expr_t ex)
{
      ivl_type_t class_type = ivl_expr_net_type(ex);
      fprintf(vvp_out, "    %%new/cobj C%p;\n", class_type);
      return 0;
}

static int eval_object_null(ivl_expr_t ex)
{
      (void)ex; /* Parameter is not used. */
      fprintf(vvp_out, "    %%null;\n");
      return 0;
}

static int eval_object_property(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      unsigned pidx = ivl_expr_property_idx(expr);

      int idx = 0;
      ivl_expr_t idx_expr = 0;

	/* If there is an array index expression, then this is an
	   array'ed property, and we need to calculate the index for
	   the expression. */
      if ( (idx_expr = ivl_expr_oper1(expr)) ) {
	    idx = allocate_word();
	    draw_eval_expr_into_integer(idx_expr, idx);
      }

      fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
      fprintf(vvp_out, "    %%prop/obj %u, %d; eval_object_property\n", pidx, idx);
      fprintf(vvp_out, "    %%pop/obj 1, 1;\n");

      if (idx != 0) clr_word(idx);
      return 0;
}

static int eval_object_shallowcopy(ivl_expr_t ex)
{
      int errors = 0;
      ivl_expr_t dest = ivl_expr_oper1(ex);
      ivl_expr_t src  = ivl_expr_oper2(ex);

      errors += draw_eval_object(dest);
      errors += draw_eval_object(src);

	/* The %scopy opcode pops the top of the object stack as the
	   source object, and shallow-copies it to the new top, the
	   destination object. The destination is left on the top of
	   the stack. */
      fprintf(vvp_out, "    %%scopy;\n");

      return errors;
}

static int eval_object_signal(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);

	/* Simple case: This is a simple variable. Generate a load
	   statement to load the string into the stack. */
      if (ivl_signal_dimensions(sig) == 0) {
	    fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
	    return 0;
      }

	/* There is a word select expression, so load the index into a
	   register and load from the array. */
      ivl_expr_t word_ex = ivl_expr_oper1(expr);
      int word_ix = allocate_word();
      draw_eval_expr_into_integer(word_ex, word_ix);
      fprintf(vvp_out, "    %%load/obja v%p, %d;\n", sig, word_ix);
      clr_word(word_ix);

      return 0;
}

static int eval_object_ufunc(ivl_expr_t ex)
{
      draw_ufunc_object(ex);
      return 0;
}

static unsigned queue_unique_src_elem_wid(ivl_expr_t arg)
{
      ivl_type_t src_q = 0;
      if (ivl_expr_type(arg) == IVL_EX_PROPERTY) {
	    ivl_signal_t cl = ivl_expr_signal(arg);
	    unsigned pidx = ivl_expr_property_idx(arg);
	    src_q = ivl_type_prop_type(ivl_signal_net_type(cl), pidx);
      } else if (ivl_expr_type(arg) == IVL_EX_SIGNAL) {
	    src_q = ivl_signal_net_type(ivl_expr_signal(arg));
      } else {
	    return 0;
      }
      if (ivl_type_base(src_q) == IVL_VT_QUEUE ||
	  ivl_type_base(src_q) == IVL_VT_DARRAY) {
	    return ivl_type_packed_width(ivl_type_element(src_q));
      }
      return 0;
}

/*
 * Emit a queue locator opcode. Plain signal uses `op_v`; class property
 * uses `op_prop_v`. Returning a queue object leaves it on the object stack.
 *
 * Note: opcode names are still type-specific (vec4) for this slice. Future
 * real/string/class support should prefer parameterized opcodes or typed
 * helpers rather than cloning this table per element type.
 */
static int emit_queue_locator_opcode(ivl_expr_t arg, unsigned elem_wid,
				     const char* op_v, const char* op_prop_v)
{
      if (ivl_expr_type(arg) == IVL_EX_PROPERTY) {
	    ivl_signal_t cl = ivl_expr_signal(arg);
	    unsigned pidx = ivl_expr_property_idx(arg);
	    fprintf(vvp_out, "    %%load/obj v%p_0;\n", cl);
	    fprintf(vvp_out, "    %s %u, %u;\n", op_prop_v, pidx, elem_wid);
	    fprintf(vvp_out, "    %%pop/obj 1, 0;\n");
      } else {
	    ivl_signal_t sig = ivl_expr_signal(arg);
	    fprintf(vvp_out, "    %s v%p_0, %u;\n", op_v, sig, elem_wid);
      }
      return 0;
}

static int eval_queue_method_unique(ivl_expr_t expr)
{
      static const struct {
	    const char* name;
	    const char* op_v;
	    const char* op_prop_v;
      } table[] = {
	    { "$ivl_queue_method$unique",
	      "%queue/unique/v", "%queue/unique/prop/v" },
	    { "$ivl_queue_method$unique_index",
	      "%queue/unique/index/v", "%queue/unique/index/prop/v" },
      };

      const char* name = ivl_expr_name(expr);
      ivl_expr_t arg = ivl_expr_parm(expr, 0);
      unsigned elem_wid = queue_unique_src_elem_wid(arg);
      if (elem_wid == 0) return 1;

      for (unsigned idx = 0; idx < sizeof table / sizeof table[0]; idx += 1) {
	    if (strcmp(name, table[idx].name) == 0) {
		  return emit_queue_locator_opcode(arg, elem_wid,
						   table[idx].op_v,
						   table[idx].op_prop_v);
	    }
      }
      return 1;
}

/*
 * Array locator methods with `with (predicate)` — parms:
 *   0: queue, 1: predicate, 2: item ivl_signal, 3: index ivl_signal
 */
enum queue_locator_with_mode_e {
      QUEUE_WITH_FIND,
      QUEUE_WITH_FIND_INDEX,
      QUEUE_WITH_FIND_FIRST,
      QUEUE_WITH_FIND_FIRST_INDEX,
      QUEUE_WITH_FIND_LAST,
      QUEUE_WITH_FIND_LAST_INDEX
};

static enum queue_locator_with_mode_e queue_locator_with_mode_from_name(
    const char* name)
{
      if (strcmp(name, "$ivl_queue_method$find_with") == 0) {
	    return QUEUE_WITH_FIND;
      } else if (strcmp(name, "$ivl_queue_method$find_index_with") == 0) {
	    return QUEUE_WITH_FIND_INDEX;
      } else if (strcmp(name, "$ivl_queue_method$find_first_with") == 0) {
	    return QUEUE_WITH_FIND_FIRST;
      } else if (strcmp(name, "$ivl_queue_method$find_first_index_with") == 0) {
	    return QUEUE_WITH_FIND_FIRST_INDEX;
      } else if (strcmp(name, "$ivl_queue_method$find_last_with") == 0) {
	    return QUEUE_WITH_FIND_LAST;
      } else if (strcmp(name, "$ivl_queue_method$find_last_index_with") == 0) {
	    return QUEUE_WITH_FIND_LAST_INDEX;
      }
      return (enum queue_locator_with_mode_e) -1;
}

static int queue_with_reverse(enum queue_locator_with_mode_e mode)
{
      return mode == QUEUE_WITH_FIND_LAST ||
	     mode == QUEUE_WITH_FIND_LAST_INDEX;
}

static int queue_with_multi(enum queue_locator_with_mode_e mode)
{
      return mode == QUEUE_WITH_FIND || mode == QUEUE_WITH_FIND_INDEX;
}

static int queue_with_as_index(enum queue_locator_with_mode_e mode)
{
      return mode == QUEUE_WITH_FIND_INDEX ||
	     mode == QUEUE_WITH_FIND_FIRST_INDEX ||
	     mode == QUEUE_WITH_FIND_LAST_INDEX;
}

/* On predicate match: append value/index and continue or stop. */
static void emit_queue_with_on_match(enum queue_locator_with_mode_e mode,
				     ivl_signal_t item_sig, int i_reg,
				     unsigned elem_wid, unsigned match_lab)
{
      int multi = queue_with_multi(mode);
      int as_index = queue_with_as_index(mode);

      if (!multi) {
	    fprintf(vvp_out, "    %%queue/new_empty/v;\n");
      }
      if (as_index) {
	    fprintf(vvp_out, "    %%push/ix/vec4 %u, 32, 1;\n", i_reg);
	    fprintf(vvp_out, "    %%queue/append_word/v 32;\n");
      } else {
	    fprintf(vvp_out, "    %%load/vec4 v%p_0;\n", item_sig);
	    fprintf(vvp_out, "    %%queue/append_word/v %u;\n", elem_wid);
      }
      fprintf(vvp_out, "    %%jmp T_%u.%u;\n", thread_count, match_lab);
}

static void emit_queue_with_init_index(int reverse, int i_reg, int n_reg)
{
      if (!reverse) {
	    fprintf(vvp_out, "    %%ix/load %u, 0, 0;\n", i_reg);
	    fprintf(vvp_out, "    %%flag_set/imm 4, 0;\n");
      } else {
	    fprintf(vvp_out, "    %%ix/load %u, 0, 0;\n", i_reg);
	    fprintf(vvp_out, "    %%ix/mov %u, %u;\n", i_reg, n_reg);
	    fprintf(vvp_out, "    %%ix/sub %u, 1, 0;\n", i_reg);
      }
}

static void emit_queue_with_loop_test(int reverse, int i_reg, int n_reg,
				      unsigned lab_loop_end)
{
      if (!reverse) {
	    fprintf(vvp_out, "    %%cmpix/ltu %u, %u;\n", i_reg, n_reg);
	    fprintf(vvp_out, "    %%jmp/0 T_%u.%u, 4;\n", thread_count,
		    lab_loop_end);
      } else {
	    fprintf(vvp_out, "    %%cmpix/slt0 %u;\n", i_reg);
	    fprintf(vvp_out, "    %%jmp/1 T_%u.%u, 4;\n", thread_count,
		    lab_loop_end);
      }
}

static void emit_queue_with_step_index(int reverse, int i_reg, unsigned lab_top)
{
      if (!reverse) {
	    fprintf(vvp_out, "    %%ix/add %u, 1, 0;\n", i_reg);
      } else {
	    fprintf(vvp_out, "    %%ix/sub %u, 1, 0;\n", i_reg);
      }
      fprintf(vvp_out, "    %%jmp T_%u.%u;\n", thread_count, lab_top);
}

static int eval_queue_method_find_with(ivl_expr_t expr)
{
      const char* name = ivl_expr_name(expr);
      if (ivl_expr_parms(expr) != 4) return 1;
      if (strncmp(name, "$ivl_queue_method$",
		  sizeof("$ivl_queue_method$") - 1) != 0) return 1;
      if (strstr(name, "_with") == 0) return 1;

      ivl_expr_t qarg = ivl_expr_parm(expr, 0);
      ivl_expr_t pred = ivl_expr_parm(expr, 1);
      ivl_signal_t item_sig = ivl_expr_signal(ivl_expr_parm(expr, 2));
      ivl_signal_t idx_sig = ivl_expr_signal(ivl_expr_parm(expr, 3));
      unsigned elem_wid = queue_unique_src_elem_wid(qarg);
      if (elem_wid == 0) return 1;

      enum queue_locator_with_mode_e mode =
	    queue_locator_with_mode_from_name(name);
      if ((int) mode < 0) return 1;

      int i_reg = allocate_word();
      int n_reg = allocate_word();
      unsigned lab_top = local_count++;
      unsigned lab_loop_end = local_count++;
      unsigned lab_nom = local_count++;
      unsigned lab_found = local_count++;
      unsigned lab_end = local_count++;
      int reverse = queue_with_reverse(mode);
      int multi = queue_with_multi(mode);
      int is_prop = (ivl_expr_type(qarg) == IVL_EX_PROPERTY);
      /* First/last stop label: property path needs an extra pop. */
      unsigned stop_lab = is_prop ? lab_found : lab_end;
      unsigned match_lab = multi ? lab_nom : stop_lab;

      if (is_prop) {
	    ivl_signal_t cl = ivl_expr_signal(qarg);
	    unsigned pidx = ivl_expr_property_idx(qarg);
	    fprintf(vvp_out, "    %%load/obj v%p_0;\n", cl);
	    fprintf(vvp_out, "    %%prop/queue/size %u;\n", pidx);
	    fprintf(vvp_out, "    %%ix/vec4/s %u;\n", n_reg);
	    if (multi) {
		  fprintf(vvp_out, "    %%queue/new_empty/v;\n");
	    }
	    emit_queue_with_init_index(reverse, i_reg, n_reg);
	    fprintf(vvp_out, "T_%u.%u ; queue with loop\n", thread_count, lab_top);
	    emit_queue_with_loop_test(reverse, i_reg, n_reg, lab_loop_end);
	    /* cmpix leaves flag 4 set; %queue/word* uses flag 4 for X push */
	    fprintf(vvp_out, "    %%flag_set/imm 4, 0;\n");
	    fprintf(vvp_out, "    %%queue/word/prop/v %u, %u, %u;\n", pidx,
		    elem_wid, i_reg);
      } else {
	    ivl_signal_t sig = ivl_expr_signal(qarg);
	    fprintf(vvp_out, "    %%queue/size/v v%p_0;\n", sig);
	    fprintf(vvp_out, "    %%ix/vec4/s %u;\n", n_reg);
	    if (multi) {
		  fprintf(vvp_out, "    %%queue/new_empty/v;\n");
	    }
	    emit_queue_with_init_index(reverse, i_reg, n_reg);
	    fprintf(vvp_out, "T_%u.%u ; queue with loop (var)\n", thread_count,
		    lab_top);
	    emit_queue_with_loop_test(reverse, i_reg, n_reg, lab_loop_end);
	    fprintf(vvp_out, "    %%flag_set/imm 4, 0;\n");
	    fprintf(vvp_out, "    %%queue/word/v v%p_0, %u, %u;\n", sig, elem_wid,
		    i_reg);
      }

      fprintf(vvp_out, "    %%store/vec4 v%p_0, 0, %u;\n", item_sig, elem_wid);
      fprintf(vvp_out, "    %%push/ix/vec4 %u, 32, 1;\n", i_reg);
      fprintf(vvp_out, "    %%store/vec4 v%p_0, 0, 32;\n", idx_sig);

      int pf = draw_eval_condition(pred);
      fprintf(vvp_out, "    %%jmp/0 T_%u.%u, %d;\n", thread_count, lab_nom, pf);
      clr_flag(pf);

      emit_queue_with_on_match(mode, item_sig, i_reg, elem_wid, match_lab);

      fprintf(vvp_out, "T_%u.%u ; nomatch\n", thread_count, lab_nom);
      emit_queue_with_step_index(reverse, i_reg, lab_top);

      if (is_prop) {
	    fprintf(vvp_out, "T_%u.%u ; found (queue prop)\n", thread_count,
		    lab_found);
	    fprintf(vvp_out, "    %%pop/obj 1, 1;\n");
	    fprintf(vvp_out, "    %%jmp T_%u.%u;\n", thread_count, lab_end);

	    fprintf(vvp_out, "T_%u.%u ; loop end (prop)\n", thread_count,
		    lab_loop_end);
	    if (multi) {
		  fprintf(vvp_out, "    %%pop/obj 1, 0;\n");
	    } else {
		  fprintf(vvp_out, "    %%queue/new_empty/v;\n");
		  fprintf(vvp_out, "    %%pop/obj 1, 1;\n");
	    }
	    fprintf(vvp_out, "T_%u.%u ; with end (prop)\n", thread_count, lab_end);
      } else {
	    fprintf(vvp_out, "T_%u.%u ; loop end (var)\n", thread_count,
		    lab_loop_end);
	    if (!multi) {
		  fprintf(vvp_out, "    %%queue/new_empty/v;\n");
	    }
	    fprintf(vvp_out, "T_%u.%u ; with end (var)\n", thread_count, lab_end);
      }

      clr_word(i_reg);
      clr_word(n_reg);
      return 0;
}

static int eval_queue_method_find(ivl_expr_t expr)
{
      static const struct {
	    const char* name;
	    const char* op_v;
	    const char* op_prop_v;
      } table[] = {
	    { "$ivl_queue_method$find",
	      "%queue/find/v", "%queue/find/prop/v" },
	    { "$ivl_queue_method$find_index",
	      "%queue/find/index/v", "%queue/find/index/prop/v" },
	    { "$ivl_queue_method$find_first",
	      "%queue/find_first/v", "%queue/find_first/prop/v" },
	    { "$ivl_queue_method$find_first_index",
	      "%queue/find_first/index/v", "%queue/find_first/index/prop/v" },
	    { "$ivl_queue_method$find_last",
	      "%queue/find_last/v", "%queue/find_last/prop/v" },
	    { "$ivl_queue_method$find_last_index",
	      "%queue/find_last/index/v", "%queue/find_last/index/prop/v" },
      };

      const char* name = ivl_expr_name(expr);
      if (ivl_expr_parms(expr) == 4 &&
	  strstr(ivl_expr_name(expr), "_with") != 0) {
	    return eval_queue_method_find_with(expr);
      }

      ivl_expr_t qarg = ivl_expr_parm(expr, 0);
      ivl_expr_t carg = ivl_expr_parm(expr, 1);
      unsigned elem_wid = queue_unique_src_elem_wid(qarg);
      if (elem_wid == 0) return 1;

      draw_eval_vec4(carg);

      for (unsigned idx = 0; idx < sizeof table / sizeof table[0]; idx += 1) {
	    if (strcmp(name, table[idx].name) == 0) {
		  return emit_queue_locator_opcode(qarg, elem_wid,
						   table[idx].op_v,
						   table[idx].op_prop_v);
	    }
      }
      return 1;
}

int draw_queue_method_find_sfunc(ivl_expr_t expr)
{
      return eval_queue_method_find(expr);
}

int draw_eval_object(ivl_expr_t ex)
{
      switch (ivl_expr_type(ex)) {

	  case IVL_EX_NEW:
	    switch (ivl_expr_value(ex)) {
		case IVL_VT_CLASS:
		  return eval_class_new(ex);
		case IVL_VT_DARRAY:
		  return eval_darray_new(ex);
		default:
		  fprintf(vvp_out, "; ERROR: draw_eval_object: Invalid type (%d) for <new>\n",
			  ivl_expr_value(ex));
		  return 0;
	    }

	  case IVL_EX_NULL:
	    return eval_object_null(ex);

	  case IVL_EX_PROPERTY:
	    return eval_object_property(ex);

	  case IVL_EX_SHALLOWCOPY:
	    return eval_object_shallowcopy(ex);

	  case IVL_EX_SIGNAL:
	    return eval_object_signal(ex);

	  case IVL_EX_UFUNC:
	    return eval_object_ufunc(ex);

	  case IVL_EX_SFUNC:
	    /* Queue locator `with` may report IVL_VT_DARRAY in ivl; handle by name. */
	    if (ivl_expr_parms(ex) == 4 &&
		strncmp(ivl_expr_name(ex), "$ivl_queue_method$",
		       sizeof("$ivl_queue_method$") - 1) == 0 &&
		strstr(ivl_expr_name(ex), "_with") != 0) {
		  if (draw_queue_method_find_sfunc(ex) == 0) return 0;
	    }
	    if (ivl_expr_value(ex) == IVL_VT_QUEUE ||
		ivl_expr_value(ex) == IVL_VT_DARRAY) {
		  if (eval_queue_method_unique(ex) == 0) return 0;
		  if (eval_queue_method_find(ex) == 0) return 0;
	    }
	    fprintf(vvp_out, "; ERROR: draw_eval_object: unsupported IVL_EX_SFUNC "
	                     "for object context\n");
	    return 1;

	  default:
	    fprintf(vvp_out, "; ERROR: draw_eval_object: Invalid expression type %d\n", ivl_expr_type(ex));
	    return 1;

      }
}
