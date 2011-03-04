#ifndef __device_H
#define __device_H
/*
 * Copyright (c) 2005-2010 Stephen Williams
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

/*
 */
extern const struct device_table_s {
      const char*name;
      device_t driver;
} edif_device_table[];

#endif
