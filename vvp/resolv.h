#ifndef __resolv_H
#define __resolv_H
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: resolv.h,v 1.9 2004/12/11 02:31:30 steve Exp $"
#endif

# include  "config.h"
# include  "vvp_net.h"

/*
 * This functor type resolves its inputs using the verilog method of
 * combining signals, and outputs that resolved value. The puller
 * value is also blended with the result. This helps with the
 * implementation of tri0 and tri1, which have pull constants
 * attached.
 *
 * This node takes in vvp_vector8_t values, and emits a vvp_vector8_t
 * value.
 */
class resolv_functor : public vvp_net_fun_t {

    public:
      explicit resolv_functor(vvp_scaler_t hiz_value);
      ~resolv_functor();

      void recv_vec8(vvp_vector8_t bit);

    private:
      vvp_vector8_t driven_;
      vvp_scaler_t hiz_;
};

/*
 * $Log: resolv.h,v $
 * Revision 1.9  2004/12/11 02:31:30  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 */
#endif
