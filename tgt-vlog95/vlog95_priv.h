#ifndef __vlog95_priv_H
#define __vlog95_priv_H
/*
 * Copyright (C) 2010 Cary R. (cygcary@yahoo.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *
 * This is the vlog95 target module. It generates a 1364-1995 compliant
 * netlist from the input netlist. The generated netlist is expected to
 * be simulation equivalent to the original.
 */

# include "config.h"
# include "ivl_target.h"
# include <stdio.h>
# include <assert.h>

/*
 * This is the file that the converted design is written to.
 */
extern FILE*vlog_out;

/*
 * Keep a count of the fatal errors that happen during code generation.
 */
extern int vlog_errors;

#endif /* __vlog95_priv_H */
