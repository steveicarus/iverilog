/*
 * Copyright (c) 2004-2005 Stephen Williams (steve@icarus.com)
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
#ident "$Id: concat.cc,v 1.5 2005/06/22 00:04:48 steve Exp $"

# include  "compile.h"
# include  "vvp_net.h"
# include  "schedule.h"
# include  <stdlib.h>
# include  <iostream>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <assert.h>

/* vvp_fun_concat
 * This node function creates vectors (vvp_vector4_t) from the
 * concatenation of the inputs. The inputs (4) may be vector or
 * vector8 objects, but they are reduced to vector4 values and
 * strength information lost.
 *
 * The expected widths of the input vectors must be given up front so
 * that the positions in the output vector (and also the size of the
 * output vector) can be worked out. The input vectors must match the
 * expected width.
 */
class vvp_fun_concat  : public vvp_net_fun_t, private vvp_gen_event_s {

    public:
      vvp_fun_concat(unsigned w0, unsigned w1,
		     unsigned w2, unsigned w3);
      ~vvp_fun_concat();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

    private:
      void run_run();
      vvp_net_t*net_;
      vvp_vector4_t input_[4];
};


vvp_fun_concat::vvp_fun_concat(unsigned w0, unsigned w1,
			       unsigned w2, unsigned w3)
: net_(0)
{
      input_[0] = vvp_vector4_t(w0);
      input_[1] = vvp_vector4_t(w1);
      input_[2] = vvp_vector4_t(w2);
      input_[3] = vvp_vector4_t(w3);
}

vvp_fun_concat::~vvp_fun_concat()
{
}

void vvp_fun_concat::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      unsigned pdx = port.port();

      if (bit.size() != input_[pdx].size()) {
	    cerr << "internal error: port " << pdx
		 << " expects wid=" << input_[pdx].size()
		 << ", got wid=" << bit.size() << endl;
	    assert(0);
      }

      if (input_[pdx] .eeq(bit))
	    return;

      input_[pdx] = bit;
      if (net_ == 0) {
	    net_ = port.ptr();
	    schedule_generic(this, 0, false);
      }
}

void vvp_fun_concat::run_run()
{
      vvp_net_t*ptr = net_;
      net_ = 0;

      unsigned off = 0;
      unsigned owid = input_[0].size() + input_[1].size() + input_[2].size() + input_[3].size();

      vvp_vector4_t res (owid);
      for (unsigned idx = 0 ; idx < 4 && (off<owid) ; idx += 1) {
	    res.set_vec(off, input_[idx]);
	    off += input_[idx].size();
      }

      vvp_send_vec4(ptr->out, res);
}

void compile_concat(char*label, unsigned w0, unsigned w1,
		    unsigned w2, unsigned w3,
		    unsigned argc, struct symb_s*argv)
{
      vvp_fun_concat*fun = new vvp_fun_concat(w0, w1, w2, w3);

      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;

      define_functor_symbol(label, net);
      free(label);

      inputs_connect(net, argc, argv);
}

vvp_fun_repeat::vvp_fun_repeat(unsigned width, unsigned repeat)
: wid_(width), rep_(repeat)
{
}

vvp_fun_repeat::~vvp_fun_repeat()
{
}

void vvp_fun_repeat::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      assert(bit.size() == wid_/rep_);

      vvp_vector4_t val (wid_);

      for (unsigned rdx = 0 ;  rdx < rep_ ;  rdx += 1) {
	    unsigned off = rdx * bit.size();

	    for (unsigned idx = 0 ; idx < bit.size() ;  idx += 1)
		  val.set_bit(off+idx, bit.value(idx));

      }

      vvp_send_vec4(port.ptr()->out, val);
}

void compile_repeat(char*label, long width, long repeat, struct symb_s arg)
{
      vvp_fun_repeat*fun = new vvp_fun_repeat(width, repeat);

      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;

      define_functor_symbol(label, net);
      free(label);

      input_connect(net, 0, arg.text);
}


/*
 * $Log: concat.cc,v $
 * Revision 1.5  2005/06/22 00:04:48  steve
 *  Reduce vvp_vector4 copies by using const references.
 *
 * Revision 1.4  2005/06/17 03:46:52  steve
 *  Make functors know their own width.
 *
 * Revision 1.3  2005/04/09 05:30:38  steve
 *  Default behavior for recv_vec8 methods.
 *
 * Revision 1.2  2005/02/07 22:42:42  steve
 *  Add .repeat functor and BIFIF functors.
 *
 * Revision 1.1  2005/01/22 00:01:09  steve
 *  Add missing concat.cc to cvs
 *
 */

