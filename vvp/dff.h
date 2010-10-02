#ifndef __vvp_dff_H
#define __vvp_dff_H
/*
 * Copyright (c) 2005 Stephen Williams (steve@icarus.com)
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

# include  "vvp_net.h"

/*
 * The vvp_dff implements a D-type FF that is agnostic to the data
 * type that is holds. The clock and clock-enable inputs are single
 * bits and may be invertible. An output is propagated on the logical
 * rising edge of the clock input, or whenever an asynchronous input
 * is received. Ports are:
 *
 *   port-0:  D input
 *   port-1:  Clock input
 *   port-2:  Clock Enable input
 *   port-3:  Asynchronous D input.
 */
class vvp_dff  : public vvp_net_fun_t {

    public:
      explicit vvp_dff(bool invert_clk =false, bool invert_ce =false);
      ~vvp_dff();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t);

    private:
      bool iclk_, ice_;
      vvp_bit4_t clk_cur_;
      vvp_bit4_t enable_;
      vvp_vector4_t d_;
};

#endif
