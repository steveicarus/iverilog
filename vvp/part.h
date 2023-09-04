#ifndef IVL_part_H
#define IVL_part_H
/*
 * Copyright (c) 2005-2014 Stephen Williams (steve@icarus.com)
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

# include  "schedule.h"
# include  "config.h"

/* vvp_fun_part
 * This node takes a part select of the input vector. Input 0 is the
 * vector to be selected from, and input 1 is the location where the
 * select starts. Input 2, which is typically constant, is the width
 * of the result.
 */
class vvp_fun_part  : public vvp_net_fun_t {

    public:
      vvp_fun_part(unsigned base, unsigned wid);
      ~vvp_fun_part();

      unsigned get_base() const { return base_; }
      unsigned get_wid() const { return wid_; }

    protected:
      unsigned base_;
      unsigned wid_;
};

/*
 * Statically allocated vvp_fun_part.
 */
class vvp_fun_part_sa  : public vvp_fun_part, public vvp_gen_event_s {

    public:
      vvp_fun_part_sa(unsigned base, unsigned wid);
      ~vvp_fun_part_sa();

    public:
      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t);

      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t);

    private:
      void run_run();

    private:
      vvp_vector4_t val_;
      vvp_net_t*net_;
};

/*
 * Automatically allocated vvp_fun_part.
 */
class vvp_fun_part_aa  : public vvp_fun_part, public automatic_hooks_s {

    public:
      vvp_fun_part_aa(unsigned base, unsigned wid);
      ~vvp_fun_part_aa();

    public:
      void alloc_instance(vvp_context_t context);
      void reset_instance(vvp_context_t context);
#ifdef CHECK_WITH_VALGRIND
      void free_instance(vvp_context_t context);
#endif

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t context);

    private:
      __vpiScope*context_scope_;
      unsigned context_idx_;
};

/* vvp_fun_part_pv
 * This node takes a vector input and turns it into the part select of
 * a wider output network. It used the recv_vec4_pv methods of the
 * destination nodes to propagate the part select. It can be used in
 * both statically and automatically allocated scopes, as it has no
 * dynamic state.
 */
class vvp_fun_part_pv  : public vvp_net_fun_t {

    public:
      vvp_fun_part_pv(unsigned base, unsigned wid, unsigned vec_wid);
      ~vvp_fun_part_pv();

    public:
      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                        unsigned base, unsigned vwid, vvp_context_t);

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
      explicit vvp_fun_part_var(unsigned wid, bool is_signed);
      ~vvp_fun_part_var();

    protected:
      bool recv_vec4_(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                      int&base, vvp_vector4_t&source,
                      vvp_vector4_t&ref);

      unsigned wid_;
      bool is_signed_;
};

/*
 * Statically allocated vvp_fun_part_var.
 */
class vvp_fun_part_var_sa  : public vvp_fun_part_var {

    public:
      explicit vvp_fun_part_var_sa(unsigned wid, bool is_signed);
      ~vvp_fun_part_var_sa();

    public:
      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t);

      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t);

    private:
      int base_;
      vvp_vector4_t source_;
	// Save the last output, for detecting change.
      vvp_vector4_t ref_;
};

/*
 * Automatically allocated vvp_fun_part_var.
 */
class vvp_fun_part_var_aa  : public vvp_fun_part_var, public automatic_hooks_s {

    public:
      explicit vvp_fun_part_var_aa(unsigned wid, bool is_signed);
      ~vvp_fun_part_var_aa();

    public:
      void alloc_instance(vvp_context_t context);
      void reset_instance(vvp_context_t context);
#ifdef CHECK_WITH_VALGRIND
      void free_instance(vvp_context_t context);
#endif

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t context);

    private:
      __vpiScope*context_scope_;
      unsigned context_idx_;
};

#endif /* IVL_part_H */
