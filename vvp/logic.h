#ifndef __logic_H
#define __logic_H
/*
 * Copyright (c) 2000-2008 Stephen Williams (steve@icarus.com)
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
# include  "schedule.h"
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

      void recv_vec4(vvp_net_ptr_t p, const vvp_vector4_t&bit);

    private:
      truth_t table;
      vvp_vector4_t input_[4];
};

/*
 * vvp_fun_boolean_ is just a common hook for holding operands.
 */
class vvp_fun_boolean_ : public vvp_net_fun_t, protected vvp_gen_event_s {

    public:
      explicit vvp_fun_boolean_(unsigned wid);
      ~vvp_fun_boolean_();

      void recv_vec4(vvp_net_ptr_t p, const vvp_vector4_t&bit);
      void recv_vec4_pv(vvp_net_ptr_t p, const vvp_vector4_t&bit,
			unsigned base, unsigned wid, unsigned vwid);

    protected:
      vvp_vector4_t input_[4];
      vvp_net_t*net_;
};

class vvp_fun_and  : public vvp_fun_boolean_ {

    public:
      explicit vvp_fun_and(unsigned wid, bool invert);
      ~vvp_fun_and();

    private:
      void run_run();
      bool invert_;
};

class vvp_fun_eeq  : public vvp_fun_boolean_ {

    public:
      explicit vvp_fun_eeq(unsigned wid, bool invert);
      ~vvp_fun_eeq();

    private:
      void run_run();
      bool invert_;
};

/*
 * The buffer functor is a very primitive functor that takes the input
 * from port-0 (and only port-0) and retransmits it as a vvp_vector4_t.
 * The retransmitted vector has all Z values changed to X, just like
 * the buf(Q,D) gate in Verilog.
 */
class vvp_fun_buf: public vvp_net_fun_t, private vvp_gen_event_s {

    public:
      explicit vvp_fun_buf();
      virtual ~vvp_fun_buf();

      void recv_vec4(vvp_net_ptr_t p, const vvp_vector4_t&bit);

    private:
      void run_run();

    private:
      vvp_vector4_t input_;
      vvp_net_t*net_;
};

/*
 * The vvp_fun_bufz is like the vvp_fun_buf, but it does not change
 * Z values to X -- it passes Z values unchanged.
 */
class vvp_fun_bufz: public vvp_net_fun_t {

    public:
      explicit vvp_fun_bufz();
      virtual ~vvp_fun_bufz();

      void recv_vec4(vvp_net_ptr_t p, const vvp_vector4_t&bit);
      void recv_real(vvp_net_ptr_t p, double bit);

    private:
};

/*
 * The muxz functor is an A-B mux device, with the data inputs on
 * ports 0 and 1. port 2 is the select input.
 *
 * The select input must be 1 bit wide. If it is 0, then the port-0
 * vector is passed out. If select is 1, then port-1 is passed
 * out. Otherwise, a vector is passed out that reflects x?: behavior
 * in Verilog. The width of the blended output is the width of the largest
 * input (port-0 or port-1) to enter the device. The narrow vector is
 * padded with X values.
 */
class vvp_fun_muxz : public vvp_net_fun_t, private vvp_gen_event_s {

    public:
      explicit vvp_fun_muxz(unsigned width);
      virtual ~vvp_fun_muxz();

      void recv_vec4(vvp_net_ptr_t p, const vvp_vector4_t&bit);

    private:
      void run_run();

    private:
      vvp_vector4_t a_;
      vvp_vector4_t b_;
      int select_;
      vvp_net_t*net_;
      bool has_run_;
};

class vvp_fun_muxr : public vvp_net_fun_t, private vvp_gen_event_s {

    public:
      explicit vvp_fun_muxr();
      virtual ~vvp_fun_muxr();

      void recv_vec4(vvp_net_ptr_t p, const vvp_vector4_t&bit);
      void recv_real(vvp_net_ptr_t p, double bit);

    private:
      void run_run();

    private:
      double a_;
      double b_;
      int select_;
      vvp_net_t*net_;
};

class vvp_fun_not: public vvp_net_fun_t, private vvp_gen_event_s {

    public:
      explicit vvp_fun_not();
      virtual ~vvp_fun_not();

      void recv_vec4(vvp_net_ptr_t p, const vvp_vector4_t&bit);

    private:
      void run_run();

    private:
      vvp_vector4_t input_;
      vvp_net_t*net_;
};

class vvp_fun_or  : public vvp_fun_boolean_ {

    public:
      explicit vvp_fun_or(unsigned wid, bool invert);
      ~vvp_fun_or();

    private:
      void run_run();
      bool invert_;
};

class vvp_fun_xor  : public vvp_fun_boolean_ {

    public:
      explicit vvp_fun_xor(unsigned wid, bool invert);
      ~vvp_fun_xor();

    private:
      void run_run();
      bool invert_;
};

// table functor types

extern const unsigned char ft_MUXX[];
extern const unsigned char ft_EEQ[];
extern const unsigned char ft_TRIAND[];
extern const unsigned char ft_TRIOR[];

#endif // __logic_H
