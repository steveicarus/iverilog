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
#ident "$Id: generic.c,v 1.3 2003/08/26 16:26:02 steve Exp $"
#endif

# include  "generic.h"

edif_t edf = 0;
edif_xlibrary_t xlib = 0;

edif_cell_t cell_0 = 0;
edif_cell_t cell_1 = 0;

edif_cell_t cell_ipad = 0;
edif_cell_t cell_opad = 0;
edif_cell_t cell_iopad = 0;



/*
 * $Log: generic.c,v $
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

