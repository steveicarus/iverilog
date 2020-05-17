#ifndef IVL_sys_priv_H
#define IVL_sys_priv_H
/*
 * Copyright (c) 2002-2020 Stephen Williams (steve@icarus.com)
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

#include "vpi_config.h"
#include "sv_vpi_user.h"

#define IS_MCD(mcd)     !((mcd)>>31&1)

/*
 * Context structure for PRNG in mt19937int.c
 */
struct context_s {
      int		mti;		/* the array for the state vector */
      unsigned long	mt[1023];	/* mti==N+1 means mt[N] is not init */
};

extern void sgenrand(struct context_s *context, unsigned long seed);
extern unsigned long genrand(struct context_s *context);

extern PLI_UINT64 timerec_to_time64(const struct t_vpi_time*timerec);

extern char *as_escaped(char *arg);
extern char *get_filename(vpiHandle callh, const char *name, vpiHandle file);
extern char *get_filename_with_suffix(vpiHandle callh, const char*name,
				      vpiHandle file, const char*suff);

extern void check_for_extra_args(vpiHandle argv, vpiHandle callh, const char *name,
                                 const char *arg_str, unsigned opt);

struct timeformat_info_s {
      int units;
      unsigned prec;
      char*suff;
      unsigned width;
};

extern struct timeformat_info_s timeformat_info;

extern unsigned is_constant_obj(vpiHandle obj);
extern unsigned is_numeric_obj(vpiHandle obj);
extern unsigned is_string_obj(vpiHandle obj);

extern unsigned is_valid_fd_mcd(PLI_UINT32 fd_mcd);
extern unsigned get_fd_mcd_from_arg(PLI_UINT32 *fd_mcd, vpiHandle arg,
                                    vpiHandle callh, const char *name);

extern vpiHandle sys_func_module(vpiHandle obj);

/*
 * The standard compiletf routines.
 */
extern PLI_INT32 sys_no_arg_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name);
extern PLI_INT32 sys_one_numeric_arg_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name);
extern PLI_INT32 sys_one_opt_numeric_arg_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name);
extern PLI_INT32 sys_two_numeric_args_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name);
extern PLI_INT32 sys_one_string_arg_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name);

/*
 * The standard put/return a value to the caller routines.
 */
extern void put_integer_value(vpiHandle callh, PLI_INT32 result);
extern void put_scalar_value(vpiHandle callh, PLI_INT32 result);

#endif /* IVL_sys_priv_H */
