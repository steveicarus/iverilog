#ifndef IVL_latch_H
#define IVL_latch_H
/*
 * Copyright (c) 2016 Martin Whitaker (icarus@martin-whitaker.me.uk)
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

# include  "vvp_net.h"

/*
 * The vvp_latch implements an arbitrary width D-type transparent latch.
 * The latch enable is a single bit. Ports are:
 *
 *   port-0:  D input
 *   port-1:  EN input
 */
class vvp_latch : public vvp_net_fun_t {

    public:
      explicit vvp_latch(unsigned width);
      ~vvp_latch();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t);

      void recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t ctx);

    private:
      vvp_bit4_t en_;
      vvp_vector4_t d_;
};

#endif /* IVL_latch_H */
