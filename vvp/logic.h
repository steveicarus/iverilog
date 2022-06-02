#ifndef IVL_logic_H
#define IVL_logic_H
/*
 * Copyright (c) 2000-2020 Stephen Williams (steve@icarus.com)
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
# include  "schedule.h"
# include  <cstddef>

/*
 * vvp_fun_boolean_ is just a common hook for holding operands.
 */
class vvp_fun_boolean_ : public vvp_net_fun_t, protected vvp_gen_event_s {

    public:
      explicit vvp_fun_boolean_(unsigned wid);
      ~vvp_fun_boolean_();

      void recv_vec4(vvp_net_ptr_t p, const vvp_vector4_t&bit,
                     vvp_context_t);
      void recv_vec4_pv(vvp_net_ptr_t p, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t);

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

class vvp_fun_equiv : public vvp_fun_boolean_ {

    public:
      explicit vvp_fun_equiv();
      ~vvp_fun_equiv();

    private:
      void run_run();
};

class vvp_fun_impl : public vvp_fun_boolean_ {

    public:
      explicit vvp_fun_impl();
      ~vvp_fun_impl();

    private:
      void run_run();
};

/*
 * The buffer functor is a very primitive functor that takes the input
 * from port-0 (and only port-0) and retransmits it as a vvp_vector4_t.
 * The retransmitted vector has all Z values changed to X, just like
 * the buf(Q,D) gate in Verilog.
 */
class vvp_fun_buf: public vvp_net_fun_t, private vvp_gen_event_s {

    public:
      explicit vvp_fun_buf(unsigned wid);
      virtual ~vvp_fun_buf();

      void recv_vec4(vvp_net_ptr_t p, const vvp_vector4_t&bit,
                     vvp_context_t);
      void recv_vec4_pv(vvp_net_ptr_t p, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t);

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

      void recv_vec4(vvp_net_ptr_t p, const vvp_vector4_t&bit,
                     vvp_context_t);
      void recv_vec4_pv(vvp_net_ptr_t p, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t);
	//void recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit);
      void recv_real(vvp_net_ptr_t p, double bit,
                     vvp_context_t);

    private:
};

/*
 * The vp_fun_buft is like the vvp_fun_bufz, but is completely
 * transparent to strengths.
 */
class vvp_fun_buft: public vvp_fun_bufz {

    public:
      void recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit);
    private:
};

enum sel_type {SEL_PORT0, SEL_PORT1, SEL_BOTH};

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

      void recv_vec4(vvp_net_ptr_t p, const vvp_vector4_t&bit,
                     vvp_context_t);
      void recv_vec4_pv(vvp_net_ptr_t p, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t);

    private:
      void run_run();

    private:
      vvp_vector4_t a_;
      vvp_vector4_t b_;
      vvp_net_t*net_;
      sel_type select_;
      bool has_run_;
};

class vvp_fun_muxr : public vvp_net_fun_t, private vvp_gen_event_s {

    public:
      explicit vvp_fun_muxr();
      virtual ~vvp_fun_muxr();

      void recv_vec4(vvp_net_ptr_t p, const vvp_vector4_t&bit,
                     vvp_context_t);
      void recv_real(vvp_net_ptr_t p, double bit,
                     vvp_context_t);

    private:
      void run_run();

    private:
      double a_;
      double b_;
      vvp_net_t*net_;
      sel_type select_;
};

class vvp_fun_not: public vvp_net_fun_t, private vvp_gen_event_s {

    public:
      explicit vvp_fun_not(unsigned wid);
      virtual ~vvp_fun_not();

      void recv_vec4(vvp_net_ptr_t p, const vvp_vector4_t&bit,
                     vvp_context_t);
      void recv_vec4_pv(vvp_net_ptr_t p, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t);

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

#endif /* IVL_logic_H */
