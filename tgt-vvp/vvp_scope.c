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
#ident "$Id: vvp_scope.c,v 1.29 2001/06/07 02:12:43 steve Exp $"
#endif

# include  "vvp_priv.h"
# include  <assert.h>
# include  <malloc.h>
# include  <string.h>

/*
 * The draw_scope function draws the major functional items within a
 * scope. This includes the scopes themselves, of course. All the
 * other functions in this file are in support of that task.
 */

static const char* draw_net_input(ivl_nexus_t nex);

/*
 * NEXUS
 * ivl builds up the netlist into objects connected together by
 * ivl_nexus_t objects. The nexus receives all the drivers of the
 * point in the net and resolves the value. The result is then sent to
 * all the nets that are connected to the nexus. The nets, then, are
 * read to get the value of the nexus.
 *
 * NETS
 * Nets are interesting and special, because a nexus may be connected
 * to several of them at once. This can happen, for example, as an
 * artifact of module port connects, where the inside and the outside
 * of the module are connected through an in-out port. (In fact, ivl
 * will simply connect signals that are bound through a port, because
 * the input/output/inout properties are enforced as compile time.)
 *
 * This case is handled by choosing one to receive the value of the
 * nexus. This one then feeds to another net at the nexus, and so
 * on. The last net is selected as the output of the nexus.
 */
/*
 * This function takes a nexus and looks for an input functor. It then
 * draws to the output a string that represents that functor. What we
 * are trying to do here is find the input to the net that is attached
 * to this nexus.
 */

static const char* draw_net_input_drive(ivl_nexus_t nex, ivl_nexus_ptr_t nptr)
{
      static char result[2048];
      unsigned idx;
      unsigned nptr_pin = ivl_nexus_ptr_pin(nptr);
      ivl_net_const_t cptr;
      ivl_net_logic_t lptr;
      ivl_signal_t sptr;
      ivl_lpm_t lpm;

      lptr = ivl_nexus_ptr_log(nptr);
      if (lptr && (ivl_logic_type(lptr) == IVL_LO_BUFZ) &&
	  (nptr_pin == 0)) {
	    return draw_net_input(ivl_logic_pin(lptr, 1));
      }

      if (lptr && (ivl_logic_type(lptr) == IVL_LO_PULLDOWN)) {
	    return "C<pu0>";
      }

      if (lptr && (ivl_logic_type(lptr) == IVL_LO_PULLUP)) {
	    return "C<pu1>";
      }

      if (lptr && (nptr_pin == 0)) {
	    sprintf(result, "L_%s", ivl_logic_name(lptr));
	    return result;
      }

      sptr = ivl_nexus_ptr_sig(nptr);
      if (sptr && (ivl_signal_type(sptr) == IVL_SIT_REG)) {
	    sprintf(result, "V_%s[%u]", ivl_signal_name(sptr),
		    nptr_pin);
	    return result;
      }

      cptr = ivl_nexus_ptr_con(nptr);
      if (cptr) {
	    const char*bits = ivl_const_bits(cptr);
	    sprintf(result, "C<%c>", bits[nptr_pin]);
	    return result;
      }

      lpm = ivl_nexus_ptr_lpm(nptr);
      if (lpm) switch (ivl_lpm_type(lpm)) {

	  case IVL_LPM_MUX:
	    for (idx = 0 ;  idx < ivl_lpm_width(lpm) ;  idx += 1)
		  if (ivl_lpm_q(lpm, idx) == nex) {
			sprintf(result, "L_%s/%u",
				ivl_lpm_name(lpm), idx);
			return result;
		  }

	  case IVL_LPM_ADD:
	    for (idx = 0 ;  idx < ivl_lpm_width(lpm) ;  idx += 1)
		  if (ivl_lpm_q(lpm, idx) == nex) {
			sprintf(result, "L_%s[%u]",
				ivl_lpm_name(lpm), idx);
			return result;
		  }

      }

      assert(0);
}

static const char* draw_net_input(ivl_nexus_t nex)
{
      static char result[512];
      unsigned idx;
      ivl_nexus_ptr_t drivers[4];
      unsigned ndrivers = 0;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t nptr = ivl_nexus_ptr(nex, idx);

	      /* Skip input only pins. */
	    if ((ivl_nexus_ptr_drive0(nptr) == IVL_DR_HiZ)
		&& (ivl_nexus_ptr_drive1(nptr) == IVL_DR_HiZ))
		  continue;

	      /* Save this driver. */
	    drivers[ndrivers] = nptr;
	    ndrivers += 1;
      }

	/* If the nexus has no drivers, then send a constant HiZ into
	   the net. */
      if (ndrivers == 0)
	    return "C<z>";


	/* If the nexus has exactly one driver, then simply draw it. */
      if (ndrivers == 1)
	    return draw_net_input_drive(nex, drivers[0]);


      assert(ndrivers <= 4);

	/* Draw a resolver to combine the inputs. */
      fprintf(vvp_out, "RS_%s .resolv tri, %s", ivl_nexus_name(nex),
	      draw_net_input_drive(nex, drivers[0]));

      for (idx = 1 ;  idx < ndrivers ;  idx += 1)
	    fprintf(vvp_out, ", %s", draw_net_input_drive(nex, drivers[idx]));

      for ( ;  idx < 4 ;  idx += 1)
	    fprintf(vvp_out, ", C<z>");

      fprintf(vvp_out, ";\n");

      sprintf(result, "RS_%s", ivl_nexus_name(nex));
      return result;
}



/*
 * This function looks at the nexus in search for the net to attach
 * functor inputs to. Sort the signals in the nexus by name, and
 * choose the lexically earliest one.
 */
void draw_input_from_net(ivl_nexus_t nex)
{
      unsigned idx;
      ivl_signal_t sig = 0;
      unsigned sig_pin = 0;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t tmp = ivl_nexus_ptr_sig(ptr);

	    if (tmp == 0)
		  continue;

	    if (sig == 0) {
		  sig = tmp;
		  sig_pin = ivl_nexus_ptr_pin(ptr);
		  continue;
	    }

	    if (strcmp(ivl_signal_name(tmp),ivl_signal_name(sig)) < 0) {
		  sig = tmp;
		  sig_pin = ivl_nexus_ptr_pin(ptr);
		  continue;
	    }

      }

      assert(sig);
      fprintf(vvp_out, "V_%s[%u]", ivl_signal_name(sig), sig_pin);
}

/*
 * This function draws a reg/int/variable in the scope. This is a very
 * simple device to draw as there are no inputs to connect so no need
 * to scan the nexus.
 */
static void draw_reg_in_scope(ivl_signal_t sig)
{
      int msb = ivl_signal_pins(sig) - 1;
      int lsb = 0;

      const char*signed_flag = ivl_signal_signed(sig)? "/s" : "";

      fprintf(vvp_out, "V_%s .var%s \"%s\", %d, %d;\n",
	      ivl_signal_name(sig), signed_flag,
	      ivl_signal_basename(sig), msb, lsb);
}

/*
 * This function takes a nexus and a signal, and finds the next signal
 * (lexically) that is connected to at the same nexus. Return the
 * ivl_nexus_ptr_t object to represent that junction.
 */
static ivl_nexus_ptr_t find_net_just_after(ivl_nexus_t nex,
					   ivl_signal_t sig)
{
      ivl_nexus_ptr_t res = 0;
      ivl_signal_t res_sig = 0;
      unsigned idx;

      for (idx = 0 ; idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t tmp = ivl_nexus_ptr_sig(ptr);

	    if (tmp == 0)
		  continue;

	    if (strcmp(ivl_signal_name(tmp),ivl_signal_name(sig)) <= 0)
		  continue;

	    if (res == 0) {
		  res = ptr;
		  res_sig = tmp;
		  continue;
	    }

	    if (strcmp(ivl_signal_name(tmp),ivl_signal_name(res_sig)) < 0) {
		  res = ptr;
		  res_sig = tmp;
	    }
      }

      return res;
}


/*
 * This function draws a net. This is a bit more complicated as we
 * have to find an appropriate functor to connect to the input.
 */
static void draw_net_in_scope(ivl_signal_t sig)
{
      unsigned idx;
      int msb = ivl_signal_pins(sig) - 1;
      int lsb = 0;
      char**args;

      const char*signed_flag = ivl_signal_signed(sig)? "/s" : "";

      args = (char**)calloc(ivl_signal_pins(sig), sizeof(char*));

	/* Connect all the pins of the signal to something. */
      for (idx = 0 ;  idx < ivl_signal_pins(sig) ;  idx += 1) {
	    ivl_nexus_ptr_t ptr;
	    ivl_nexus_t nex = ivl_signal_pin(sig, idx);

	    ptr = find_net_just_after(nex, sig);
	    if (ptr) {
		  char tmp[512];
		  sprintf(tmp, "V_%s[%u]",
			  ivl_signal_name(ivl_nexus_ptr_sig(ptr)),
			  ivl_nexus_ptr_pin(ptr));
		  args[idx] = strdup(tmp);
	    } else {
		  args[idx] = strdup(draw_net_input(nex));
	    }
      }

      fprintf(vvp_out, "V_%s .net%s \"%s\", %d, %d, %s",
	      ivl_signal_name(sig), signed_flag,
	      ivl_signal_basename(sig), msb, lsb, args[0]);
      for (idx = 1 ;  idx < ivl_signal_pins(sig) ;  idx += 1)
	    fprintf(vvp_out, ", %s", args[idx]);
      fprintf(vvp_out, ";\n");

      free(args);
}

static void draw_udp_def(ivl_udp_t udp)
{
  unsigned init;
  int i;

  switch (ivl_udp_init(udp))
    {
    case '0':
      init = 0;
      break;
    case '1':
      init = 1;
      break;
    default:
      init = 2;
      break;
    }

  if (ivl_udp_sequ(udp))
	fprintf(vvp_out, 
		"UDP_%s .udp/sequ \"%s\", %d, %d",
		ivl_udp_name(udp),
		ivl_udp_name(udp),
		ivl_udp_nin(udp),
		init );
  else
	fprintf(vvp_out, 
		"UDP_%s .udp/comb \"%s\", %d",
		ivl_udp_name(udp),
		ivl_udp_name(udp),
		ivl_udp_nin(udp));

  for (i=0; i<ivl_udp_rows(udp); i++)
    fprintf(vvp_out, "\n ,\"%s\"", ivl_udp_row(udp, i) );

  fprintf(vvp_out, ";\n");
}

static void draw_udp_in_scope(ivl_net_logic_t lptr)
{
  unsigned pdx;

  ivl_udp_t udp = ivl_logic_udp(lptr);
  
  static ivl_udp_t *udps = 0x0;
  static int nudps = 0;
  int i;

  for (i=0; i<nudps; i++)
    if (udps[i] == udp)
      break;
  
  if (i >= nudps)
    {
      udps = (ivl_udp_t*)realloc(udps, (nudps+1)*sizeof(ivl_udp_t));
      assert(udps);
      udps[nudps++] = udp;
      draw_udp_def(udp);
    }

  fprintf(vvp_out, "L_%s .udp UDP_%s",
	  ivl_logic_name(lptr), ivl_udp_name(udp));
  
  for (pdx = 1 ;  pdx < ivl_logic_pins(lptr) ;  pdx += 1) 
    {
      ivl_nexus_t nex = ivl_logic_pin(lptr, pdx);
      fprintf(vvp_out, ", ");
      draw_input_from_net(nex);
    }
  
  fprintf(vvp_out, ";\n");
}

static void draw_logic_in_scope(ivl_net_logic_t lptr)
{
	    unsigned pdx;
      const char*ltype = "?";
      char identity_val = '0';


      switch (ivl_logic_type(lptr)) {

          case IVL_LO_UDP:
	    draw_udp_in_scope(lptr);
	    return;

          case IVL_LO_BUFZ:
	/* Skip BUFZ objects. Things that have a bufz as input
	   will use the input to bufz instead. */
	    return;

	  case IVL_LO_PULLDOWN:
	  case IVL_LO_PULLUP:
	      /* Skip pullup and pulldown objects. Things that have
		 pull objects as inputs will instead generate the
		 appropriate C<?> symbol. */
	    return;

	  case IVL_LO_AND:
	    ltype = "AND";
	    identity_val = '1';
	    break;

	  case IVL_LO_BUF:
	    ltype = "BUF";
	    break;

	  case IVL_LO_BUFIF0:
	    ltype = "BUFIF0";
	    break;

	  case IVL_LO_BUFIF1:
	    ltype = "BUFIF1";
	    break;

	  case IVL_LO_NAND:
	    ltype = "NAND";
	    identity_val = '1';
	    break;

	  case IVL_LO_NOR:
	    ltype = "NOR";
	    break;

	  case IVL_LO_NOT:
	    ltype = "NOT";
	    break;

	  case IVL_LO_OR:
	    ltype = "OR";
	    break;

	  case IVL_LO_XNOR:
	    ltype = "XNOR";
	    break;

	  case IVL_LO_XOR:
	    ltype = "XOR";
	    break;

	  default:
	    fprintf(stderr, "vvp.tgt: error: Unhandled logic type: %u\n",
		    ivl_logic_type(lptr));
	    ltype = "?";
	    break;
      }

      assert(ivl_logic_pins(lptr) <= 5);

      fprintf(vvp_out, "L_%s .functor %s", ivl_logic_name(lptr), ltype);

      for (pdx = 1 ;  pdx < ivl_logic_pins(lptr) ;  pdx += 1) {
	    ivl_nexus_t nex = ivl_logic_pin(lptr, pdx);
	    fprintf(vvp_out, ", ");
	    draw_input_from_net(nex);
      }

      for ( ;  pdx < 5 ;  pdx += 1) {
	    fprintf(vvp_out, ", C<%c>", identity_val);
      }

      fprintf(vvp_out, ";\n");
}

static void draw_event_in_scope(ivl_event_t obj)
{
      unsigned nany = ivl_event_nany(obj);
      unsigned nneg = ivl_event_nneg(obj);
      unsigned npos = ivl_event_npos(obj);

      unsigned cnt = 0;

	/* Figure out how many probe functors are needed. */
      if (nany > 0)
	    cnt += (nany+3) / 4;

      if (nneg > 0)
	    cnt += (nneg+3) / 4;

      if (npos > 0)
	    cnt += (npos+3) / 4;

      if (cnt == 0) {
	      /* If none are needed, then this is a named event. The
		 code needed is easy. */
	    fprintf(vvp_out, "E_%s .event \"%s\";\n",
		    ivl_event_name(obj), ivl_event_basename(obj));

      } else if (cnt > 1) {
	    unsigned idx;
	    unsigned ecnt = 0;

	    for (idx = 0 ;  idx < nany ;  idx += 4, ecnt += 1) {
		  unsigned sub, top;

		  fprintf(vvp_out, "E_%s/%u .event edge",
			  ivl_event_name(obj), ecnt);

		  top = idx + 4;
		  if (nany < top)
			top = nany;
		  for (sub = idx ;  sub < top ;  sub += 1) {
			ivl_nexus_t nex = ivl_event_any(obj, sub);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }
		  fprintf(vvp_out, ";\n");
	    }

	    for (idx = 0 ;  idx < nneg ;  idx += 4, ecnt += 1) {
		  unsigned sub, top;

		  fprintf(vvp_out, "E_%s/%u .event negedge",
			  ivl_event_name(obj), ecnt);

		  top = idx + 4;
		  if (nneg < top)
			top = nneg;
		  for (sub = idx ;  sub < top ;  sub += 1) {
			ivl_nexus_t nex = ivl_event_neg(obj, sub);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }
		  fprintf(vvp_out, ";\n");
	    }

	    for (idx = 0 ;  idx < npos ;  idx += 4, ecnt += 1) {
		  unsigned sub, top;

		  fprintf(vvp_out, "E_%s/%u .event posedge",
			  ivl_event_name(obj), ecnt);

		  top = idx + 4;
		  if (npos < top)
			top = npos;
		  for (sub = idx ;  sub < top ;  sub += 1) {
			ivl_nexus_t nex = ivl_event_pos(obj, sub);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }
		  fprintf(vvp_out, ";\n");
	    }

	    assert(ecnt == cnt);

	    fprintf(vvp_out, "E_%s .event/or E_%s/0",
		    ivl_event_name(obj), ivl_event_name(obj));

	    for (idx = 1 ;  idx < cnt ;  idx += 1)
		  fprintf(vvp_out, ", E_%s/%u", ivl_event_name(obj), idx);

	    fprintf(vvp_out, ";\n");

      } else {
	    unsigned idx;

	    fprintf(vvp_out, "E_%s .event ", ivl_event_name(obj));

	    if (nany > 0) {
		  assert((nneg + npos) == 0);
		  assert(nany <= 4);

		  fprintf(vvp_out, "edge");

		  for (idx = 0 ;  idx < nany ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_any(obj, idx);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }

	    } else if (nneg > 0) {
		  assert((nany + npos) == 0);
		  fprintf(vvp_out, "negedge");

		  for (idx = 0 ;  idx < nneg ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_neg(obj, idx);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }

	    } else {
		  assert((nany + nneg) == 0);
		  fprintf(vvp_out, "posedge");

		  for (idx = 0 ;  idx < npos ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_pos(obj, idx);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }
	    }

	    fprintf(vvp_out, ";\n");
      }
}

static void draw_lpm_add(ivl_lpm_t net)
{
      unsigned idx, width;

      width = ivl_lpm_width(net);

      fprintf(vvp_out, "L_%s .arith/sum %u", ivl_lpm_name(net), width);

      for (idx = 0 ;  idx < width ;  idx += 1) {
	    ivl_nexus_t a = ivl_lpm_data(net, idx);
	    fprintf(vvp_out, ", ");
	    draw_input_from_net(a);
      }

      for (idx = 0 ;  idx < width ;  idx += 1) {
	    ivl_nexus_t b = ivl_lpm_datab(net, idx);
	    fprintf(vvp_out, ", ");
	    draw_input_from_net(b);
      }

      fprintf(vvp_out, ";\n");
}

static void draw_lpm_mux(ivl_lpm_t net)
{
      ivl_nexus_t s;
      unsigned idx, width;

	/* XXXX Only support A-B muxes for now. */
      assert(ivl_lpm_size(net) == 2);
      assert(ivl_lpm_selects(net) == 1);

      width = ivl_lpm_width(net);
      s = ivl_lpm_select(net, 0);

      for (idx = 0 ;  idx < width ;  idx += 1) {
	    ivl_nexus_t a = ivl_lpm_data2(net, 0, idx);
	    ivl_nexus_t b = ivl_lpm_data2(net, 1, idx);
	    fprintf(vvp_out, "L_%s/%u .functor MUXZ, ",
		    ivl_lpm_name(net), idx);
	    draw_input_from_net(a);
	    fprintf(vvp_out, ", ");
	    draw_input_from_net(b);
	    fprintf(vvp_out, ", ");
	    draw_input_from_net(s);
	    fprintf(vvp_out, ", C<1>;\n");
      }

}

static void draw_lpm_in_scope(ivl_lpm_t net)
{
      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_ADD:
	    draw_lpm_add(net);
	    return;

	  case IVL_LPM_MUX:
	    draw_lpm_mux(net);
	    return;

	  default:
	    fprintf(stderr, "XXXX LPM not supported: %s\n",
		    ivl_lpm_name(net));
      }
}


static void draw_mem_in_scope(ivl_memory_t net)
{
      int root = ivl_memory_root(net);
      int last = root + ivl_memory_size(net) - 1;
      int msb = ivl_memory_width(net) - 1;
      int lsb = 0;
      fprintf(vvp_out, "M_%s .mem \"%s\", %u,%u, %u,%u;\n",
	      ivl_memory_name(net), ivl_memory_basename(net),
	      msb, lsb, root, last);
}


int draw_scope(ivl_scope_t net, ivl_scope_t parent)
{
      unsigned idx;

      if (parent)
	    fprintf(vvp_out, "S_%s .scope \"%s\", S_%s;\n",
		    ivl_scope_name(net), ivl_scope_name(net),
		    ivl_scope_name(parent));
      else
	    fprintf(vvp_out, "S_%s .scope \"%s\";\n",
		    ivl_scope_name(net), ivl_scope_name(net));


	/* Scan the scope for logic devices. For each device, draw out
	   a functor that connects pin 0 to the output, and the
	   remaining pins to inputs. */

      for (idx = 0 ;  idx < ivl_scope_logs(net) ;  idx += 1) {
	    ivl_net_logic_t lptr = ivl_scope_log(net, idx);
	    draw_logic_in_scope(lptr);
      }


	/* Scan the signals (reg and net) and draw the appropriate
	   statements to make the signal function. */

      for (idx = 0 ;  idx < ivl_scope_sigs(net) ;  idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(net, idx);

	    switch (ivl_signal_type(sig)) {
		case IVL_SIT_REG:
		  draw_reg_in_scope(sig);
		  break;
		default:
		  draw_net_in_scope(sig);
		  break;
	    }
      }

      for (idx = 0 ;  idx < ivl_scope_events(net) ;  idx += 1) {
	    ivl_event_t event = ivl_scope_event(net, idx);
	    draw_event_in_scope(event);
      }

      for (idx = 0 ;  idx < ivl_scope_lpms(net) ;  idx += 1) {
	    ivl_lpm_t lpm = ivl_scope_lpm(net, idx);
	    draw_lpm_in_scope(lpm);
      }

      for (idx = 0 ;  idx < ivl_scope_mems(net) ;  idx += 1) {
	    ivl_memory_t mem = ivl_scope_mem(net, idx);
	    draw_mem_in_scope(mem);
      }

      if (ivl_scope_type(net) == IVL_SCT_TASK)
	    draw_task_definition(net);

      if (ivl_scope_type(net) == IVL_SCT_FUNCTION)
	    draw_func_definition(net);

      ivl_scope_children(net, (ivl_scope_f*) draw_scope, net);
      return 0;
}

/*
 * $Log: vvp_scope.c,v $
 * Revision 1.29  2001/06/07 02:12:43  steve
 *  Support structural addition.
 *
 * Revision 1.28  2001/05/12 16:34:47  steve
 *  Fixup the resolver syntax.
 *
 * Revision 1.27  2001/05/12 03:31:01  steve
 *  Generate resolvers for multiple drivers.
 *
 * Revision 1.26  2001/05/08 23:59:33  steve
 *  Add ivl and vvp.tgt support for memories in
 *  expressions and l-values. (Stephan Boettcher)
 *
 * Revision 1.25  2001/05/06 00:01:02  steve
 *  Generate code that causes the value of a net to be passed
 *  passed through all nets of a nexus.
 *
 * Revision 1.24  2001/05/03 04:55:46  steve
 *  Generate code for the fully general event or.
 *
 * Revision 1.23  2001/05/02 04:05:16  steve
 *  Remove the init parameter of functors, and instead use
 *  the special C<?> symbols to initialize inputs. This is
 *  clearer and more regular.
 *
 * Revision 1.22  2001/04/30 00:00:27  steve
 *  detect multiple drivers on nexa.
 *
 * Revision 1.21  2001/04/29 23:16:31  steve
 *  Add bufif and pull devices.
 *
 * Revision 1.20  2001/04/26 05:12:02  steve
 *  Implement simple MUXZ for ?: operators.
 *
 * Revision 1.19  2001/04/24 02:59:52  steve
 *  Fix generation of udp/comb definitions.
 *
 * Revision 1.18  2001/04/24 02:23:58  steve
 *  Support for UDP devices in VVP (Stephen Boettcher)
 *
 * Revision 1.17  2001/04/21 02:04:01  steve
 *  Add NAND and XNOR functors.
 *
 * Revision 1.16  2001/04/15 16:37:48  steve
 *  add XOR support.
 *
 * Revision 1.15  2001/04/14 05:11:49  steve
 *  Use event/or for wide anyedge statements.
 *
 * Revision 1.14  2001/04/06 02:28:03  steve
 *  Generate vvp code for functions with ports.
 *
 * Revision 1.13  2001/04/05 01:38:24  steve
 *  Generate signed .net and .var statements.
 *
 * Revision 1.12  2001/04/02 02:28:13  steve
 *  Generate code for task calls.
 *
 * Revision 1.11  2001/04/01 21:34:48  steve
 *  Recognize the BUF device.
 *
 * Revision 1.10  2001/04/01 01:48:21  steve
 *  Redesign event information to support arbitrary edge combining.
 *
 * Revision 1.9  2001/03/31 19:29:23  steve
 *  Fix compilation warnings.
 *
 * Revision 1.8  2001/03/29 03:47:13  steve
 *  events can take up to 4 inputs.
 *
 * Revision 1.7  2001/03/28 06:07:40  steve
 *  Add the ivl_event_t to ivl_target, and use that to generate
 *  .event statements in vvp way ahead of the thread that uses it.
 *
 * Revision 1.6  2001/03/27 06:27:41  steve
 *  Generate code for simple @ statements.
 *
 * Revision 1.5  2001/03/25 19:36:12  steve
 *  Draw AND NOR and NOT gates.
 *
 * Revision 1.4  2001/03/25 05:59:47  steve
 *  Recursive make check target.
 *
 * Revision 1.3  2001/03/25 03:53:40  steve
 *  Include signal bit index in functor input.
 *
 * Revision 1.2  2001/03/25 03:25:43  steve
 *  Generate .net statements, and nexus inputs.
 *
 * Revision 1.1  2001/03/21 01:49:43  steve
 *  Scan the scopes of a design, and draw behavioral
 *  blocking  assignments of constants to vectors.
 *
 */

