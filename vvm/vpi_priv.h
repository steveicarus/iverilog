#ifndef __vpi_priv_H
#define __vpi_priv_H
/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: vpi_priv.h,v 1.1 1999/08/15 01:23:56 steve Exp $"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This is the most basic type in the vpi implementation. A handle can
 * exist to refer to most any of the supported vpi objects. The
 * interpretation of the parts depends in general on the type of the
 * object.
 */
struct __vpiHandle {
      int type;
      int subtype;

	/* These are property values. */
      char*full_name;

	/* This pointer is used for to-one references. */
      struct __vpiHandle*referent;

	/* This pointer table is used for to-many refrences to
	   arguments, and is used by the vpiArgument iterator. */
      struct __vpiHandle**arguments;
      unsigned narguments;

	/* These methods support the various vpi_get() functions. */
      int (*get_)(int property, vpiHandle ref);
      char* (*get_str_)(int property, vpiHandle ref);

	/* This method is used to get a value. */
      void (*get_value_)(struct __vpiHandle*expr, s_vpi_value*vp);

	/* This is a value union, that reflect state or the value of a
	   handle. */
      union {
	    unsigned unum;
	    struct vvm_bits_t*bits;
	    struct t_vpi_time time;
      } val;
};


/* TYPE MEANINGS:
 *
 * vpiArgument
 *   This type of handle contains a single referent, the item that has
 *   many arguments. It is an iterator.
 *
 * vpiConstant
 *   Constant values, such as strings and numbers, are this type.
 *
 * vpiSysTaskCall
 *   This handle type represents a call to a system task. It has a
 *   to-many reference to argument expressions, and a to-one reference
 *   to a vpiUserSystf object.
 *
 * vpiTimeVar
 *   This type is a special kind of variable, that holds a time. The
 *   time value is a more complex structure then a single number. The
 *   type really seems to exist to implement the $time system variable.
 */

#ifdef __cplusplus
}
#endif

/*
 * $Log: vpi_priv.h,v $
 * Revision 1.1  1999/08/15 01:23:56  steve
 *  Convert vvm to implement system tasks with vpi.
 *
 */
#endif
