#ifndef IVL_device_H
#define IVL_device_H
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

# include  <ivl_target.h>

/*
 * This code generator supports a variety of device types. It does
 * this by keeping a device "driver" structure for each device
 * type. The device structure contains pointers to functions that emit
 * the proper XNF for a given type of device.
 *
 * If a device supports a method, the function pointer is filled in
 * with a pointer to the proper function.
 *
 * If a device does not support the method, then the pointer is null.
 */
typedef const struct device_s* device_t;

struct device_s {
	/* These methods draw leading and trailing format text. */
      void (*show_header)(ivl_design_t des);
      void (*show_footer)(ivl_design_t des);
	/* Draw scopes marked by ivl_synthesis_cell */
      void (*show_cell_scope)(ivl_scope_t net);
	/* Draw pads connected to the specified signal. */
      void (*show_pad)(ivl_signal_t sig, const char*str);
	/* Draw basic logic devices. */
      void (*show_logic)(ivl_net_logic_t net);
	/* This method emits a D type Flip-Flop */
      void (*show_dff)(ivl_lpm_t net);
	/* These methods show various comparators */
      void (*show_cmp_eq)(ivl_lpm_t net);
      void (*show_cmp_ne)(ivl_lpm_t net);
      void (*show_cmp_ge)(ivl_lpm_t net);
      void (*show_cmp_gt)(ivl_lpm_t net);
	/* This method draws MUX devices */
      void (*show_mux)(ivl_lpm_t net);
	/* This method draws ADD devices */
      void (*show_add)(ivl_lpm_t net);
      void (*show_sub)(ivl_lpm_t net);
	/* These methods draw SHIFT devices */
      void (*show_shiftl)(ivl_lpm_t net);
      void (*show_shiftr)(ivl_lpm_t net);
	/* Multipliers */
      void (*show_mult)(ivl_lpm_t net);
	/* Constants */
      void (*show_constant)(ivl_net_const_t net);
};

/*
 * Return the device_t cookie given the name of the architecture. If
 * the device is not found, return 0.
 *
 * This function is used if the user specifies the architecture
 * explicitly, with the -parch=name flag.
 */
extern device_t device_from_arch(const char*arch);

#endif /* IVL_device_H */
