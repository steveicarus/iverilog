/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: draw_vpi.c,v 1.1 2003/02/28 20:21:13 steve Exp $"
#endif

# include  "vvp_priv.h"
# include  <string.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <assert.h>

struct vector_info draw_vpi_taskfunc_call(ivl_statement_t tnet,
			    ivl_expr_t fnet, unsigned wid)
{
      unsigned idx;
      unsigned parm_count = tnet
	    ? ivl_stmt_parm_count(tnet)
	    : ivl_expr_parms(fnet);
      struct vector_info res;
      struct vector_info *vec = 0x0;
      unsigned int vecs= 0;
      unsigned int veci= 0;
      

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
		case IVL_EX_NUMBER:
		case IVL_EX_STRING:
		case IVL_EX_SCOPE:
		case IVL_EX_SFUNC:
		case IVL_EX_VARIABLE:
		  continue;

		case IVL_EX_SIGNAL:
		    /* If the signal node is narrower then the signal
		       itself, then this is a part select so I'm going
		       to need to evaluate the expression.

		       If I don't need to do any evaluating, then skip
		       it as I'll be passing the handle to the signal
		       itself. */
		  if (ivl_expr_width(expr) !=
		      ivl_signal_pins(ivl_expr_signal(expr))) {
			break;
		  } else {
			continue;
		  }


		case IVL_EX_MEMORY:
		  if (!ivl_expr_oper1(expr)) {
			continue;
		  }

		    /* Everything else will need to be evaluated and
		       passed as a constant to the vpi task. */
		default:
		  break;
	    }

	    vec = (struct vector_info *)
		  realloc(vec, (vecs+1)*sizeof(struct vector_info));

	    switch (ivl_expr_value(expr)) {
		case IVL_VT_VECTOR:
		  vec[vecs] = draw_eval_expr(expr, 0);
		  break;
		case IVL_VT_REAL:
		  vec[vecs].base = draw_eval_real(expr);
		  vec[vecs].wid = 0;
		  break;
		default:
		  assert(0);
	    }
	    vecs++;
      }

      if (tnet != 0) {
	      /* for task calls, the res vector is not used. */
	    res.base = 0;
	    res.wid = 0;
	    fprintf(vvp_out, "    %%vpi_call \"%s\"", ivl_stmt_name(tnet));

      } else {
	    res.base = allocate_vector(wid);
	    res.wid  = wid;
	    fprintf(vvp_out, "    %%vpi_func \"%s\", %u, %u",
		    ivl_expr_name(fnet), res.base, res.wid);
      }

      for (idx = 0 ;  idx < parm_count ;  idx += 1) {
	    ivl_expr_t expr = tnet
		  ? ivl_stmt_parm(tnet, idx)
		  : ivl_expr_parm(fnet, idx);
	    
	    switch (ivl_expr_type(expr)) {
		case IVL_EX_NONE:
		  fprintf(vvp_out, ", \" \"");
		  continue;

		case IVL_EX_NUMBER: {
		      unsigned bit, wid = ivl_expr_width(expr);
		      const char*bits = ivl_expr_bits(expr);

		      fprintf(vvp_out, ", %u'%sb", wid,
			      ivl_expr_signed(expr)? "s" : "");
		      for (bit = wid ;  bit > 0 ;  bit -= 1)
			    fputc(bits[bit-1], vvp_out);
		      continue;
		}

		case IVL_EX_SIGNAL:
		    /* If this is a part select, then the value was
		       calculated above. Otherwise, just pass the
		       signal. */
		  if (ivl_expr_width(expr) !=
		      ivl_signal_pins(ivl_expr_signal(expr))) {
			break;
		  } else {
			fprintf(vvp_out, ", V_%s", 
				vvp_signal_label(ivl_expr_signal(expr)));
			continue;
		  }

		case IVL_EX_VARIABLE: {
		      ivl_variable_t var = ivl_expr_variable(expr);
		      fprintf(vvp_out, ", W_%s", vvp_word_label(var));
		      continue;
		}

		case IVL_EX_STRING:
		  fprintf(vvp_out, ", \"%s\"", 
			  ivl_expr_string(expr));
		  continue;

		case IVL_EX_SCOPE:
		  fprintf(vvp_out, ", S_%s",
			  vvp_mangle_id(ivl_scope_name(ivl_expr_scope(expr))));
		  continue;

		case IVL_EX_SFUNC:
		  if (strcmp("$time", ivl_expr_name(expr)) == 0)
			fprintf(vvp_out, ", $time");
		  else if (strcmp("$stime", ivl_expr_name(expr)) == 0)
			fprintf(vvp_out, ", $stime");
		  else if (strcmp("$realtime", ivl_expr_name(expr)) == 0)
			fprintf(vvp_out, ", $realtime");
		  else if (strcmp("$simtime", ivl_expr_name(expr)) == 0)
			fprintf(vvp_out, ", $simtime");
		  else
			fprintf(vvp_out, ", ?%s?", ivl_expr_name(expr));
		  continue;
		  
		case IVL_EX_MEMORY:
		  if (!ivl_expr_oper1(expr)) {
			fprintf(vvp_out, ", M_%s", 
				vvp_memory_label(ivl_expr_memory(expr)));
			continue;
		  }
		  break;

		default:
		  break;
	    }
	    assert(veci < vecs);

	    switch (ivl_expr_value(expr)) {

		case IVL_VT_VECTOR:
		  fprintf(vvp_out, ", T<%u,%u,%s>", vec[veci].base,
			  vec[veci].wid, ivl_expr_signed(expr)? "s" : "u");
		  break;

		case IVL_VT_REAL:
		  fprintf(vvp_out, ", W<%u,r>", vec[veci].base);
		  break;

		default:
		  assert(0);
	    }
	    veci++;
      }
      
      assert(veci == vecs);

      if (vecs) {
	    for (idx = 0; idx < vecs; idx++) {
		  if (vec[idx].wid > 0)
			clr_vector(vec[idx]);
	    }
	    free(vec);
      }

      fprintf(vvp_out, ";\n");

      return res;
}

/*
 * $Log: draw_vpi.c,v $
 * Revision 1.1  2003/02/28 20:21:13  steve
 *  Merge vpi_call and vpi_func draw functions.
 *
 */

