#ifndef __generic_H
#define __generic_H
/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: generic.h,v 1.3 2003/08/26 16:26:02 steve Exp $"
#endif

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


/*
 * $Log: generic.h,v $
 * Revision 1.3  2003/08/26 16:26:02  steve
 *  ifdef idents correctly.
 *
 * Revision 1.2  2003/07/03 17:46:33  steve
 *  IOPAD support.
 *
 * Revision 1.1  2003/06/25 02:55:57  steve
 *  Virtex and Virtex2 share much code.
 *
 */
#endif
