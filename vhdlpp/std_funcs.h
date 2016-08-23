#ifndef IVL_std_funcs_H
#define IVL_std_funcs_H
/*
 * Copyright CERN 2016
 * @author Maciej Suminski (maciej.suminski@cern.ch)
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

#include "subprogram.h"

// Generates subprogram headers for standard VHDL library functions.
void preload_std_funcs();

// Destroys subprogram headers for standard VHDL library functions.
void delete_std_funcs();

// Adds a subprogram to the standard library subprogram set
void register_std_subprogram(SubprogramHeader*header);

// Returns subprogram header for a requested function or NULL if it does not exist.
SubHeaderList find_std_subprogram(perm_string name);

#endif /* IVL_std_funcs_H */
