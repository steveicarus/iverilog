#ifndef __device_H
#define __device_H
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
#ident "$Id: device.h,v 1.1 2001/08/28 04:14:20 steve Exp $"

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
 * If a device inherits from a parent class, then it put a pointer to
 * a function that invokes the parent method.
 *
 * If a device does not support the method, then the pointer is null.
 */
typedef const struct device_s* device_t;

struct device_s {
	/* This is the parent device type. */
      struct device_s* parent;
	/* Draw basic logic devices. */
      void (*show_logic)(ivl_net_logic_t net);
	/* This method emits a D type Flip-Flop */
      void (*show_dff)(ivl_lpm_t net);
};


/*
 * $Log: device.h,v $
 * Revision 1.1  2001/08/28 04:14:20  steve
 *  Add the fpga target.
 *
 */
#endif
