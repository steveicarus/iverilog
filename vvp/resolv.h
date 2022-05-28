#ifndef IVL_resolv_H
#define IVL_resolv_H
/*
 * Copyright (c) 2001-2014 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "vvp_net.h"

/*
 * Resolver nodes are similar to wide functors, in that they may have
 * more than 4 inputs. Unlike wide functors, the functor that provides
 * the core functionality and delivers the output is also used to handle
 * the first 4 inputs, thus each resolver node is implemented by N/4
 * actual functors.
 */

class resolv_core : public vvp_net_fun_t {

    public:
      explicit resolv_core(unsigned nports, vvp_net_t*net);
      virtual ~resolv_core();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t)
            { recv_vec4_(port.port(), bit); }

      void recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit)
            { recv_vec8_(port.port(), bit); }

      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t)
            { recv_vec4_pv_(port.port(), bit, base, vwid); }

      void recv_vec8_pv(vvp_net_ptr_t port, const vvp_vector8_t&bit,
			unsigned base, unsigned vwid)
            { recv_vec8_pv_(port.port(), bit, base, vwid); }

      virtual void count_drivers(unsigned bit_idx, unsigned counts[3]) =0;

    private:
      friend class resolv_extend;
      virtual void recv_vec4_(unsigned port, const vvp_vector4_t&bit) =0;
      virtual void recv_vec8_(unsigned port, const vvp_vector8_t&bit) =0;

      void recv_vec4_pv_(unsigned port, const vvp_vector4_t&bit,
			 unsigned base, unsigned vwid);
      void recv_vec8_pv_(unsigned port, const vvp_vector8_t&bit,
			 unsigned base, unsigned vwid);

    protected:
      unsigned nports_;
      vvp_net_t*net_;
};

class resolv_extend : public vvp_net_fun_t {

    public:
      resolv_extend(resolv_core*core, unsigned port_base);
      ~resolv_extend();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t)
            { core_->recv_vec4_(port_base_ + port.port(), bit); }

      void recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit)
            { core_->recv_vec8_(port_base_ + port.port(), bit); }

      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t)
            { core_->recv_vec4_pv_(port_base_ + port.port(), bit,
                                   base, vwid); }

      void recv_vec8_pv(vvp_net_ptr_t port, const vvp_vector8_t&bit,
			unsigned base, unsigned vwid)
            { core_->recv_vec8_pv_(port_base_ + port.port(), bit,
                                   base, vwid); }

    private:
      resolv_core*core_;
      unsigned port_base_;
};

/*
 * This functor type resolves its inputs using the Verilog method of
 * combining signals, and outputs that resolved value. The puller
 * value is also blended with the result. This helps with the
 * implementation of tri0 and tri1, which have pull constants
 * attached.
 *
 * This node takes in vvp_vector8_t values, and emits a vvp_vector8_t
 * value. It also takes in vvp_vector4_t values, which it treats as
 * strong values (or HiZ) for the sake of resolution. In any case, the
 * propagated value is a vvp_vector8_t value.
 */
class resolv_tri : public resolv_core {

    public:
      explicit resolv_tri(unsigned nports, vvp_net_t*net,
                          vvp_scalar_t hiz_value);
      ~resolv_tri();

      void count_drivers(unsigned bit_idx, unsigned counts[3]);

    private:
      void recv_vec4_(unsigned port, const vvp_vector4_t&bit);
      void recv_vec8_(unsigned port, const vvp_vector8_t&bit);

    private:
        // The puller value to be used when a bit is not driven.
      vvp_scalar_t hiz_value_;
        // The array of input values.
      vvp_vector8_t*val_;
};

/*
 * This functor type resolves its inputs using the wired_logic_math_
 * method implemented by each derived class, and outputs that resolved
 * value.
 *
 * This node takes in vvp_vector4_t values, and emits a vvp_vector4_t
 * value. It also takes in vvp_vector8_t values, which it reduces to
 * 4-state values for the sake of resolution.
 */
class resolv_wired_logic : public resolv_core {

    public:
      explicit resolv_wired_logic(unsigned nports, vvp_net_t*net);
      virtual ~resolv_wired_logic();

      void count_drivers(unsigned bit_idx, unsigned counts[3]);

    protected:
      virtual vvp_vector4_t wired_logic_math_(vvp_vector4_t&a, vvp_vector4_t&b) =0;

    private:
      void recv_vec4_(unsigned port, const vvp_vector4_t&bit);
      void recv_vec8_(unsigned port, const vvp_vector8_t&bit);

    private:
        // The array of input values.
      vvp_vector4_t*val_;
};

class resolv_triand : public resolv_wired_logic {

    public:
      explicit resolv_triand(unsigned nports, vvp_net_t*net);
      ~resolv_triand();

    private:
      virtual vvp_vector4_t wired_logic_math_(vvp_vector4_t&a, vvp_vector4_t&b);
};

class resolv_trior : public resolv_wired_logic {

    public:
      explicit resolv_trior(unsigned nports, vvp_net_t*net);
      ~resolv_trior();

    private:
      virtual vvp_vector4_t wired_logic_math_(vvp_vector4_t&a, vvp_vector4_t&b);
};

#endif /* IVL_resolv_H */
