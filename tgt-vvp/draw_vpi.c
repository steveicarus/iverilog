/*
 * Copyright (c) 2003-2020 Stephen Williams (steve@icarus.com)
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
# include  <stdlib.h>
# include  <assert.h>
# include  "ivl_alloc.h"

struct args_info {
      char*text;
	/* True ('s' or 'u' if this argument is a calculated vec4. */
      char vec_flag;
	/* True if this argument is a calculated string. */
      char str_flag;
	/* True if this argument is a calculated real. */
      char real_flag;
	/* Stack position if this argument is a calculated value. */
      unsigned stack;
	/* Expression width: Only used of vec_flag is true. */
      unsigned vec_wid;
      struct args_info *child; /* Arguments can be nested. */
};

static const char* magic_sfuncs[] = {
      "$time",
      "$stime",
      "$realtime",
      "$simtime",
      0
};

static int is_magic_sfunc(const char*name)
{
      int idx;
      for (idx = 0 ;  magic_sfuncs[idx] ;  idx += 1)
	    if (strcmp(magic_sfuncs[idx],name) == 0)
		  return 1;

      return 0;
}

static int is_fixed_memory_word(ivl_expr_t net)
{
      ivl_signal_t sig;

      if (ivl_expr_type(net) != IVL_EX_SIGNAL)
	    return 0;

      sig = ivl_expr_signal(net);

      if (ivl_signal_dimensions(sig) == 0)
	    return 1;

      if (ivl_signal_type(sig) == IVL_SIT_REG)
	    return 0;

      if (number_is_immediate(ivl_expr_oper1(net), IMM_WID, 0))
	    return 1;

      return 0;
}

static int get_vpi_taskfunc_signal_arg(struct args_info *result,
                                       ivl_expr_t expr)
{
      char buffer[4096];

      switch (ivl_expr_type(expr)) {
	  case IVL_EX_SIGNAL:
	      /* If the signal node is narrower than the signal itself,
	         then this is a part select so I'm going to need to
	         evaluate the expression.

	         Also, if the signedness of the expression is different
	         from the signedness of the signal. This could be
	         caused by a $signed or $unsigned system function.

	         If I don't need to do any evaluating, then skip it as
	         I'll be passing the handle to the signal itself. */
	    if ((ivl_expr_width(expr) !=
	         ivl_signal_width(ivl_expr_signal(expr))) &&
	         ivl_expr_value(expr) != IVL_VT_DARRAY) {
		    /* This should never happen since we have IVL_EX_SELECT. */
		    /* For a queue (type of darray) we return the maximum
		       size of the queue as the signal width. */
		  assert(0);
		  return 0;

	    } else if (signal_is_return_value(ivl_expr_signal(expr))) {
		    /* If the signal is the return value of a function,
		       then this can't be handled as a true signal, so
		       fall back on general expression processing. */
		  return 0;

	    } else if (ivl_expr_signed(expr) !=
	               ivl_signal_signed(ivl_expr_signal(expr))) {
		  return 0;
	    } else if (is_fixed_memory_word(expr)) {
		  /* This is a word of a non-array, or a word of a net
		     array, so we can address the word directly. */
		  ivl_signal_t sig = ivl_expr_signal(expr);
		  unsigned use_word = 0;
		  ivl_expr_t word_ex = ivl_expr_oper1(expr);
		  if (word_ex) {
			  /* Some array select have been evaluated. */
			if (number_is_immediate(word_ex,IMM_WID, 0)) {
			      assert(! number_is_unknown(word_ex));
			      use_word = get_number_immediate(word_ex);
			      word_ex = 0;
			}
		  }
		  if (word_ex) return 0;

		  assert(word_ex == 0);
		  snprintf(buffer, sizeof buffer, "v%p_%u", sig, use_word);
		  result->text = strdup(buffer);
		  return 1;

	    } else {
		  /* What's left, this is the work of a var array.
		     Create the right code to handle it. */
		  ivl_signal_t sig = ivl_expr_signal(expr);
		  unsigned use_word = 0;
		  unsigned use_word_defined = 0;
		  ivl_expr_t word_ex = ivl_expr_oper1(expr);
		  if (word_ex) {
			  /* Some array select have been evaluated. */
			if (number_is_immediate(word_ex, IMM_WID, 0)) {
			      assert(! number_is_unknown(word_ex));
			      use_word = get_number_immediate(word_ex);
			      use_word_defined = 1;
			      word_ex = 0;
			}
		  }
		  if (word_ex && (ivl_expr_type(word_ex)==IVL_EX_SIGNAL ||
		                  ivl_expr_type(word_ex)==IVL_EX_SELECT)) {
			  /* Special case: the index is a signal/select. */
			result->child = calloc(1, sizeof(struct args_info));
			if (get_vpi_taskfunc_signal_arg(result->child,
			                                word_ex)) {
			      snprintf(buffer, sizeof buffer, "&A<v%p, %s >",
			               sig, result->child->text);
			      free(result->child->text);
			} else {
			      free(result->child);
			      result->child = NULL;
			      return 0;
			}
		  } else if (word_ex) {
			/* Fallback case: Give up and evaluate expression. */
			return 0;

		  } else {
			assert(use_word_defined);
			snprintf(buffer, sizeof buffer, "&A<v%p, %u>",
			         sig, use_word);
		  }
		  result->text = strdup(buffer);
		  return 1;
	    }

	  case IVL_EX_SELECT: {
	    ivl_expr_t vexpr = ivl_expr_oper1(expr);
	    ivl_expr_t bexpr;
	    ivl_expr_t wexpr;

	    assert(vexpr);

	      /* This code is only for signals or selects. */
	    if (ivl_expr_type(vexpr) != IVL_EX_SIGNAL &&
	        ivl_expr_type(vexpr) != IVL_EX_SELECT) return 0;

	      /* If the expression is a substring expression, then
		 the xPV method of passing the argument will not work
		 and we have to resort to the default method. */
	    if (ivl_expr_value(vexpr) == IVL_VT_STRING)
		  return 0;

	      /* If the sub-expression is a DARRAY, then this select
		 is a dynamic-array word select. Handle that
		 elsewhere. */
	    if (ivl_expr_value(vexpr) == IVL_VT_DARRAY)
		  return 0;

	      /* Part select is always unsigned. If the expression is signed
	       * fallback. */
	    if (ivl_expr_signed(expr))
		  return 0;

	      /* The signal is part of an array. */
	      /* Add &APV<> code here when it is finished. */
	    bexpr = ivl_expr_oper2(expr);

              /* This is a pad operation. */
	    if (!bexpr) return 0;

	    wexpr = ivl_expr_oper1(vexpr);

	      /* If vexpr has an operand, then that operand is a word
		 index and we are taking a select from an array
		 word. This would come up in expressions like
		 "array[<word>][<part>]" where wexpr is <word> */
	    if (wexpr && number_is_immediate(wexpr, 64, 1)
		&& number_is_immediate(bexpr, 64, 1)) {
		  assert(! number_is_unknown(bexpr));
		  assert(! number_is_unknown(wexpr));
		  snprintf(buffer, sizeof buffer, "&APV<v%p, %ld, %ld, %u>",
			   ivl_expr_signal(vexpr),
			   get_number_immediate(wexpr),
			   get_number_immediate(bexpr),
			   ivl_expr_width(expr));

	    } else if (wexpr) {
		  return 0;

	      /* This is a constant bit/part select. */
	    } else if (number_is_immediate(bexpr, 64, 1)) {
		  assert(! number_is_unknown(bexpr));
		  snprintf(buffer, sizeof buffer, "&PV<v%p_0, %ld, %u>",
		           ivl_expr_signal(vexpr),
		           get_number_immediate(bexpr),
		           ivl_expr_width(expr));

	      /* This is an indexed bit/part select. */
	    } else if (ivl_expr_type(bexpr) == IVL_EX_SIGNAL ||
	               ivl_expr_type(bexpr) == IVL_EX_SELECT) {
		    /* Special case: the base is a signal/select. */
		  result->child = calloc(1, sizeof(struct args_info));
		  if (get_vpi_taskfunc_signal_arg(result->child, bexpr)) {
			snprintf(buffer, sizeof buffer, "&PV<v%p_0, %s, %u>",
			         ivl_expr_signal(vexpr),
			         result->child->text,
			         ivl_expr_width(expr));
			free(result->child->text);
		  } else {
			free(result->child);
			result->child = NULL;
			return 0;
		  }
	    } else {
		    /* Fallback case: Punt and let caller handle it. */
		  return 0;
	    }
	    result->text = strdup(buffer);
	    return 1;
	  }

	  default:
	    return 0;
      }
}

static void draw_vpi_taskfunc_args(const char*call_string,
				   ivl_statement_t tnet,
				   ivl_expr_t fnet)
{
      unsigned idx;
      unsigned parm_count = tnet
	    ? ivl_stmt_parm_count(tnet)
	    : ivl_expr_parms(fnet);

      struct args_info *args = calloc(parm_count, sizeof(struct args_info));

      char buffer[4096];

      ivl_parameter_t par;

	/* Keep track of how much string stack this function call is
	   going to need. We'll need this for making stack references,
	   and also to clean out the stack when done. */
      unsigned vec4_stack_need = 0;
      unsigned str_stack_need = 0;
      unsigned real_stack_need = 0;

	/* Figure out how many expressions are going to be evaluated
	   for this task call. I won't need to evaluate expressions
	   for items that are VPI objects directly. */
      for (idx = 0 ;  idx < parm_count ;  idx += 1) {
	    ivl_expr_t expr = tnet
		  ? ivl_stmt_parm(tnet, idx)
		  : ivl_expr_parm(fnet, idx);

	    switch (ivl_expr_type(expr)) {

		    /* These expression types can be handled directly,
		       with VPI handles of their own. Therefore, skip
		       them in the process of evaluating expressions. */
		case IVL_EX_NONE:
		  args[idx].text = strdup("\" \"");
		  continue;

		case IVL_EX_ARRAY:
		  snprintf(buffer, sizeof buffer,
			   "v%p", ivl_expr_signal(expr));
		  args[idx].text = strdup(buffer);
		  continue;

		case IVL_EX_NUMBER: {
		  if (( par = ivl_expr_parameter(expr) )) {
			snprintf(buffer, sizeof buffer, "P_%p", par);
		  } else {
			unsigned bit, wid = ivl_expr_width(expr);
			const char*bits = ivl_expr_bits(expr);
			char*dp;

			snprintf(buffer, sizeof buffer, "%u'%sb",
			         wid, ivl_expr_signed(expr)? "s" : "");
			dp = buffer + strlen(buffer);
			for (bit = wid ;  bit > 0 ;  bit -= 1)
			      *dp++ = bits[bit-1];
			*dp++ = 0;
			assert(dp >= buffer);
			assert((unsigned)(dp - buffer) <= sizeof buffer);
		  }
		  args[idx].text = strdup(buffer);
		  continue;
		}

		case IVL_EX_STRING:
		  if (( par = ivl_expr_parameter(expr) )) {
			snprintf(buffer, sizeof buffer, "P_%p", par);
			args[idx].text = strdup(buffer);

		  } else {
			size_t needed_len = strlen(ivl_expr_string(expr)) + 3;
			args[idx].text = malloc(needed_len);
			snprintf(args[idx].text, needed_len, "\"%s\"",
			         ivl_expr_string(expr));
		  }
		  continue;

		case IVL_EX_REALNUM:
		  if (( par = ivl_expr_parameter(expr) )) {
			snprintf(buffer, sizeof buffer, "P_%p", par);
			args[idx].text = strdup(buffer);
			continue;
		  }
		  break;

		case IVL_EX_ENUMTYPE:
		  snprintf(buffer, sizeof buffer, "enum%p", ivl_expr_enumtype(expr));
		  args[idx].text = strdup(buffer);
		  continue;
		case IVL_EX_EVENT:
		  snprintf(buffer, sizeof buffer, "E_%p", ivl_expr_event(expr));
		  args[idx].text = strdup(buffer);
		  continue;
		case IVL_EX_SCOPE:
		  snprintf(buffer, sizeof buffer, "S_%p", ivl_expr_scope(expr));
		  args[idx].text = strdup(buffer);
		  continue;

		case IVL_EX_SFUNC:
		  if (is_magic_sfunc(ivl_expr_name(expr))) {
			snprintf(buffer, sizeof buffer, "%s", ivl_expr_name(expr));
			args[idx].text = strdup(buffer);
			continue;
		  }
		  break;

		case IVL_EX_SIGNAL:
		case IVL_EX_SELECT:
		  args[idx].stack = vec4_stack_need;
		  if (get_vpi_taskfunc_signal_arg(&args[idx], expr)) {
			if (args[idx].vec_flag) {
			      vec4_stack_need += 1;
			} else {
			      args[idx].stack = 0;
			}
			continue;
		  } else {
			args[idx].stack = 0;
			break;
		  }
		case IVL_EX_NULL:
		  snprintf(buffer, sizeof buffer, "null");
		  args[idx].text = strdup(buffer);
		  continue;
		    /* Everything else will need to be evaluated and
		       passed as a constant to the vpi task. */
		default:
		  break;
	    }

	    switch (ivl_expr_value(expr)) {
		case IVL_VT_LOGIC:
		case IVL_VT_BOOL:
		  draw_eval_vec4(expr);
		  args[idx].vec_flag = ivl_expr_signed(expr)? 's' : 'u';
		  args[idx].str_flag = 0;
		  args[idx].real_flag = 0;
		  args[idx].stack = vec4_stack_need;
		  args[idx].vec_wid = ivl_expr_width(expr);
		  vec4_stack_need += 1;
		  buffer[0] = 0;
		  break;
		case IVL_VT_REAL:
		  draw_eval_real(expr);
		  args[idx].vec_flag = 0;
		  args[idx].str_flag = 0;
		  args[idx].real_flag = 1;
		  args[idx].stack = real_stack_need;
		  real_stack_need += 1;
		  buffer[0] = 0;
		  break;
		case IVL_VT_STRING:
		    /* Eval the string into the stack, and tell VPI
		       about the stack position. */
		  draw_eval_string(expr);
		  args[idx].vec_flag = 0;
		  args[idx].str_flag = 1;
		  args[idx].real_flag = 0;
		  args[idx].stack = str_stack_need;
		  args[idx].real_flag = 0;
		  str_stack_need += 1;
		  buffer[0] = 0;
		  break;
		default:
		  fprintf(stderr, "%s:%u: Sorry, cannot generate code for argument %u.\n",
		                  ivl_expr_file(expr), ivl_expr_lineno(expr), idx+1);
		  fprintf(vvp_out, "\nXXXX Unexpected argument: call_string=<%s>, arg=%u, type=%d\n",
			  call_string, idx, ivl_expr_value(expr));
		  fflush(vvp_out);
		  assert(0);
	    }
	    args[idx].text = strdup(buffer);
      }

      fprintf(vvp_out, "%s", call_string);

      for (idx = 0 ;  idx < parm_count ;  idx += 1) {
	    struct args_info*ptr;

	    if (args[idx].str_flag) {
		    /* If this is a stack reference, then
		       calculate the stack depth and use that to
		       generate the completed string. */
		  unsigned pos = str_stack_need - args[idx].stack - 1;
		  fprintf(vvp_out, ", S<%u,str>",pos);
	    } else if (args[idx].real_flag) {
		  unsigned pos = real_stack_need - args[idx].stack - 1;
		  fprintf(vvp_out, ", W<%u,r>",pos);
	    } else if (args[idx].vec_flag) {
		  unsigned pos = vec4_stack_need - args[idx].stack - 1;
		  char sign_flag = args[idx].vec_flag;
		  unsigned wid = args[idx].vec_wid;
		  fprintf(vvp_out, ", S<%u,vec4,%c%u>",pos, sign_flag, wid);
	    } else {
		  fprintf(vvp_out, ", %s", args[idx].text);
	    }

	    free(args[idx].text);
	      /* Free the nested children. */
	    ptr = args[idx].child;
	    while (ptr != NULL) {
		struct args_info*tptr = ptr;
		ptr = ptr->child;
		free(tptr);
	    }
      }

      free(args);

      fprintf(vvp_out, " {%u %u %u}", vec4_stack_need, real_stack_need, str_stack_need);
      fprintf(vvp_out, ";\n");
}

void draw_vpi_task_call(ivl_statement_t tnet)
{
      unsigned parm_count = ivl_stmt_parm_count(tnet);
      const char *command = "error";

      switch (ivl_stmt_sfunc_as_task(tnet)) {
	  case IVL_SFUNC_AS_TASK_ERROR:
	    command = "%vpi_call";
	    break;
	  case IVL_SFUNC_AS_TASK_WARNING:
	    command = "%vpi_call/w";
	    break;
	  case IVL_SFUNC_AS_TASK_IGNORE:
	    command = "%vpi_call/i";
	    break;
      }

      if (parm_count == 0) {
            fprintf(vvp_out, "    %s %u %u \"%s\" {0 0 0};\n", command,
                    ivl_file_table_index(ivl_stmt_file(tnet)),
                    ivl_stmt_lineno(tnet), ivl_stmt_name(tnet));
      } else {
	    char call_string[1024];
	    snprintf(call_string, sizeof(call_string),
		     "    %s %u %u \"%s\"", command,
		     ivl_file_table_index(ivl_stmt_file(tnet)),
		     ivl_stmt_lineno(tnet), ivl_stmt_name(tnet));
	    draw_vpi_taskfunc_args(call_string, tnet, 0);
      }
}

void draw_vpi_func_call(ivl_expr_t fnet)
{
      char call_string[1024];

      snprintf(call_string, sizeof(call_string),
	       "    %%vpi_func %u %u \"%s\" %u",
	       ivl_file_table_index(ivl_expr_file(fnet)),
	       ivl_expr_lineno(fnet), ivl_expr_name(fnet),
	       ivl_expr_width(fnet));

      draw_vpi_taskfunc_args(call_string, 0, fnet);
}

void draw_vpi_rfunc_call(ivl_expr_t fnet)
{
      char call_string[1024];

      snprintf(call_string, sizeof(call_string),
	       "    %%vpi_func/r %u %u \"%s\"",
	       ivl_file_table_index(ivl_expr_file(fnet)),
	       ivl_expr_lineno(fnet), ivl_expr_name(fnet));

      draw_vpi_taskfunc_args(call_string, 0, fnet);
}

void draw_vpi_sfunc_call(ivl_expr_t fnet)
{
      char call_string[1024];

      snprintf(call_string, sizeof(call_string),
	       "    %%vpi_func/s %u %u \"%s\"",
	       ivl_file_table_index(ivl_expr_file(fnet)),
	       ivl_expr_lineno(fnet), ivl_expr_name(fnet));

      draw_vpi_taskfunc_args(call_string, 0, fnet);
}
