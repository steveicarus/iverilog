#ifndef __npmos_H
#define __npmos_H
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: npmos.h,v 1.6 2005/06/12 00:44:49 steve Exp $"
#endif

# include  "vvp_net.h"

/*
 * The vvp_fun_pmos functor is similar to the vvp_fun_bufif. The
 * principle difference is that it has no drive of its own, instead
 * taking drive strength from its data input. In other words, it is
 * not a buffer but a uni-directional switch.
 *
 * The truth table for the PMOS device is:
 *
 *     Q = D C  (D is port0, C is port1)
 *     -------
 *     0 | 0 0
 *     Z | 0 1
 *     L | 0 x
 *     1 | 1 0
 *     Z | 1 1
 *     H | 1 x
 *
 * This class also implements the NMOS device, which is the same as
 * the PMOS device, but the Control input inverted. The enable_invert
 * flag to the costructor activates this invertion.
 */

class vvp_fun_pmos  : public vvp_net_fun_t {

    public:
      explicit vvp_fun_pmos(bool enable_invert);

      void recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit);
      void recv_vec8(vvp_net_ptr_t port, vvp_vector8_t bit);

    private:
      void generate_output_(vvp_net_ptr_t port);

      vvp_vector8_t bit_;
      vvp_vector4_t en_;
      bool inv_en_;
};

/*
 * $Log: npmos.h,v $
 * Revision 1.6  2005/06/12 00:44:49  steve
 *  Implement nmos and pmos devices.
 *
 */
#endif
