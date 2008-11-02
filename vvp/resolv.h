#ifndef __resolv_H
#define __resolv_H
/*
 * Copyright (c) 2001-2008 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "vvp_net.h"

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
class resolv_functor : public vvp_net_fun_t {

    public:
      explicit resolv_functor(vvp_scalar_t hiz_value, const char* debug =0);
      ~resolv_functor();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t);
      void recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit);

      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned base, unsigned wid, unsigned vwid,
                        vvp_context_t);
      void recv_vec8_pv(vvp_net_ptr_t port, const vvp_vector8_t&bit,
			unsigned base, unsigned wid, unsigned vwid);

    private:
      vvp_vector8_t val_[4];
	// Bit value to emit for HiZ bits.
      vvp_scalar_t hiz_;
	// True if debugging is enabled
      const char* debug_label_;
};

class resolv_wired_logic : public vvp_net_fun_t {

    public:
      explicit resolv_wired_logic(void);
      ~resolv_wired_logic();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t);

    protected:
      virtual vvp_vector4_t wired_logic_math_(vvp_vector4_t&a, vvp_vector4_t&b) =0;

    private:
      vvp_vector4_t val_[4];
};

class resolv_triand : public resolv_wired_logic {

    public:
      explicit resolv_triand(void) { }
      ~resolv_triand() { }

    private:
      virtual vvp_vector4_t wired_logic_math_(vvp_vector4_t&a, vvp_vector4_t&b);
};

class resolv_trior : public resolv_wired_logic {

    public:
      explicit resolv_trior(void) { }
      ~resolv_trior() { }

    private:
      virtual vvp_vector4_t wired_logic_math_(vvp_vector4_t&a, vvp_vector4_t&b);
};

#endif
