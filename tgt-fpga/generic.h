#ifndef IVL_generic_H
#define IVL_generic_H
/*
 * Copyright (c) 2003-2014 Stephen Williams (steve@icarus.com)
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

# include  "edif.h"

extern edif_t edf;
extern edif_xlibrary_t xlib;

/*
 * The cell_* variables below are the various kinds of devices that
 * this family supports as primitives. If the cell type is used at
 * least once, then the edif_cell_t is non-zero and will also be
 * included in the library declaration. The constants underneath are
 * pin assignments for the cell.
 */

extern edif_cell_t cell_0;
extern edif_cell_t cell_1;

extern edif_cell_t cell_ipad;
extern edif_cell_t cell_opad;
extern edif_cell_t cell_iopad;

#endif /* IVL_generic_H */
