/*
 * Copyright (c) 2002-2007 Stephen Williams (steve@icarus.com)
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

# include  "statistics.h"

/*
 * This is a count of the instruction opcodes that were created.
 */
unsigned long count_opcodes = 0;

unsigned long count_functors = 0;
unsigned long count_functors_logic = 0;
unsigned long count_functors_bufif = 0;
unsigned long count_functors_resolv= 0;
unsigned long count_functors_sig   = 0;

unsigned long count_filters = 0;
unsigned long count_vpi_nets = 0;

unsigned long count_vpi_scopes = 0;

size_t size_opcodes = 0;

