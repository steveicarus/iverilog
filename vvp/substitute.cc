/*
 * Copyright (c) 2016 Stephen Williams (steve@icarus.com)
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

# include  "compile.h"
# include  "vvp_net.h"
# include  <cstdlib>
# include  <iostream>
# include  <cassert>


class vvp_fun_substitute : public vvp_net_fun_t {

    public:
      vvp_fun_substitute(unsigned wid, unsigned soff, unsigned swid);
      ~vvp_fun_substitute();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit, vvp_context_t);

      void recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t ctx);

    private:
      unsigned wid_;
      unsigned soff_;
      unsigned swid_;

      vvp_vector4_t val_;
};

vvp_fun_substitute::vvp_fun_substitute(unsigned wid, unsigned soff, unsigned swid)
: wid_(wid), soff_(soff), swid_(swid), val_(wid)
{
      for (unsigned idx = 0 ; idx < val_.size() ; idx += 1)
	    val_.set_bit(idx, BIT4_Z);
}

vvp_fun_substitute::~vvp_fun_substitute()
{
}

void vvp_fun_substitute::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
				   vvp_context_t)
{
      unsigned pdx = port.port();
      assert(pdx <= 1);

      if (pdx == 0) {
	    assert(bit.size() == wid_);

	    for (unsigned idx = 0 ; idx < wid_ ; idx += 1) {
		  if (idx >= soff_ && idx < (soff_+swid_))
			continue;

		  val_.set_bit(idx, bit.value(idx));
	    }

      } else {
	    assert(bit.size() == swid_);

	    for (unsigned idx = 0 ; idx < swid_ ; idx += 1)
		  val_.set_bit(idx+soff_, bit.value(idx));
      }

      port.ptr()->send_vec4(val_, 0);
}

void vvp_fun_substitute::recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
				      unsigned base, unsigned vwid, vvp_context_t ctx)
{
      recv_vec4_pv_(ptr, bit, base, vwid, ctx);
}

void compile_substitute(char*label, unsigned width,
			unsigned soff, unsigned swidth,
			unsigned argc, struct symb_s*argv)
{
      vvp_fun_substitute*fun = new vvp_fun_substitute(width, soff, swidth);

      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;

      define_functor_symbol(label, net);
      free(label);

      inputs_connect(net, argc, argv);
      free(argv);
}
