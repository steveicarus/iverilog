#ifndef __logic_H
#define __logic_H
/*
 * Copyright (c) 2000-2005 Stephen Williams (steve@icarus.com)
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
#ident "$Id: logic.h,v 1.13 2005/02/12 23:05:25 steve Exp $"
#endif

# include  "vvp_net.h"
# include  <stddef.h>

/*
 * Table driven functor. This kind of node takes 4 inputs and
 * generates a single output. The logic is bitwise, and implemented
 * with a lookup table.
 */

class table_functor_s: public vvp_net_fun_t {

    public:
      typedef const unsigned char *truth_t;
      explicit table_functor_s(truth_t t);
      virtual ~table_functor_s();

      void recv_vec4(vvp_net_ptr_t p, vvp_vector4_t bit);

    private:
      truth_t table;
      vvp_vector4_t input_[4];
};

class vvp_fun_boolean_ : public vvp_net_fun_t {

    protected:
      vvp_vector4_t input_[4];
};

class vvp_fun_and  : public vvp_fun_boolean_ {

    public:
      explicit vvp_fun_and();
      ~vvp_fun_and();
      void recv_vec4(vvp_net_ptr_t p, vvp_vector4_t bit);
};

/*
 * The buffer functor is a very primitive functor that takes the input
 * from port-0 (and only port-0) and retransmits it as a vvp_vector4_t.
 * The retransmitted vector has all Z values changed to X, just like
 * the buf(Q,D) gate in Verilog.
 */
class vvp_fun_buf: public vvp_net_fun_t {

    public:
      explicit vvp_fun_buf();
      virtual ~vvp_fun_buf();

      void recv_vec4(vvp_net_ptr_t p, vvp_vector4_t bit);

    private:
};

/*
 * The vvp_fun_bufz is like the vvp_fun_buf, but it does not change
 * Z values to X -- it passes Z values unchanged.
 */
class vvp_fun_bufz: public vvp_net_fun_t {

    public:
      explicit vvp_fun_bufz();
      virtual ~vvp_fun_bufz();

      void recv_vec4(vvp_net_ptr_t p, vvp_vector4_t bit);

    private:
};

/*
 * The muxz functor is an A-B mux device, with the data inputs on
 * ports 0 and 1. port 2 is the select input.
 *
 * The select input must be 1 bit wide. If it is 0, then the port-0
 * vector is passed out. If select is 1, then port-1 is passed
 * out. Otherwise, a vector is passed out that reflects x?: behavior
 * in verilog. The width of the blended output is the width of the largest
 * input (port-0 or port-1) to enter the device. The narrow vector is
 * padded with X values.
 */
class vvp_fun_muxz : public vvp_net_fun_t {

    public:
      explicit vvp_fun_muxz();
      virtual ~vvp_fun_muxz();

      void recv_vec4(vvp_net_ptr_t p, vvp_vector4_t bit);

    private:
      vvp_vector4_t a_;
      vvp_vector4_t b_;
      int select_;
};

// table functor types

extern const unsigned char ft_PMOS[];
extern const unsigned char ft_NMOS[];
extern const unsigned char ft_MUXX[];
extern const unsigned char ft_EEQ[];
extern const unsigned char ft_NAND[];
extern const unsigned char ft_NOR[];
extern const unsigned char ft_NOT[];
extern const unsigned char ft_OR[];
extern const unsigned char ft_TRIAND[];
extern const unsigned char ft_TRIOR[];
extern const unsigned char ft_XNOR[];
extern const unsigned char ft_XOR[];
extern const unsigned char ft_var[];

/*
 * $Log: logic.h,v $
 * Revision 1.13  2005/02/12 23:05:25  steve
 *  Cleanup unused truth tables.
 *
 * Revision 1.11  2005/01/29 17:52:06  steve
 *  move AND to buitin instead of table.
 *
 * Revision 1.10  2004/12/31 05:56:36  steve
 *  Add specific BUFZ functor.
 *
 * Revision 1.9  2004/12/29 23:45:13  steve
 *  Add the part concatenation node (.concat).
 *
 *  Add a vvp_event_anyedge class to handle the special
 *  case of .event statements of edge type. This also
 *  frees the posedge/negedge types to handle all 4 inputs.
 *
 *  Implement table functor recv_vec4 method to receive
 *  and process vectors.
 *
 * Revision 1.8  2004/12/11 02:31:29  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 */
#endif // __logic_H
