#ifndef __part_H
#define __part_H
/*
 * Copyright (c) 2005-2008 Stephen Williams (steve@icarus.com)
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

# include  "schedule.h"

/* vvp_fun_part
 * This node takes a part select of the input vector. Input 0 is the
 * vector to be selected from, and input 1 is the location where the
 * select starts. Input 2, which is typically constant, is the width
 * of the result.
 */
class vvp_fun_part  : public vvp_net_fun_t, private vvp_gen_event_s {

    public:
      vvp_fun_part(unsigned base, unsigned wid);
      ~vvp_fun_part();

    public:
      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned, unsigned, unsigned);

    private:
      void run_run();

    private:
      unsigned base_;
      unsigned wid_;
      vvp_vector4_t val_;
      vvp_net_t*net_;
};

/* vvp_fun_part_pv
 * This node takes a vector input and turns it into the part select of
 * a wider output network. It used the recv_vec4_pv methods of the
 * destination nodes to propagate the part select.
 */
class vvp_fun_part_pv  : public vvp_net_fun_t {

    public:
      vvp_fun_part_pv(unsigned base, unsigned wid, unsigned vec_wid);
      ~vvp_fun_part_pv();

    public:
      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);
      void recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit);

    private:
      unsigned base_;
      unsigned wid_;
      unsigned vwid_;
};

/*
 * This part select is more flexible in that it takes the vector to
 * part in port 0, and the base of the part in port 1. The width of
 * the part to take out is fixed.
 */
class vvp_fun_part_var  : public vvp_net_fun_t {

    public:
      explicit vvp_fun_part_var(unsigned wid);
      ~vvp_fun_part_var();

    public:
      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned, unsigned, unsigned);

    private:
      unsigned base_;
      unsigned wid_;
      vvp_vector4_t source_;
	// Save the last output, for detecting change.
      vvp_vector4_t ref_;
};

#endif
