/*
 * Copyright (c) 2003 Stephen Williams (steve at icarus.com)
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
#ident "$Id: xilinx.c,v 1.1 2003/04/05 05:53:34 steve Exp $"
#endif

# include  "xilinx.h"

edif_cell_t xilinx_cell_buf(edif_xlibrary_t xlib)
{
      static edif_cell_t cell = 0;
      if (cell) return cell;

      cell = edif_xcell_create(xlib, "BUF", 2);
      edif_cell_portconfig(cell, BUF_O, "O", IVL_SIP_OUTPUT);
      edif_cell_portconfig(cell, BUF_I, "I", IVL_SIP_INPUT);
      return cell;
}

edif_cell_t xilinx_cell_bufg(edif_xlibrary_t xlib)
{
      static edif_cell_t cell = 0;
      if (cell) return cell;

      cell = edif_xcell_create(xlib, "BUFG", 2);
      edif_cell_portconfig(cell, BUF_O, "O", IVL_SIP_OUTPUT);
      edif_cell_portconfig(cell, BUF_I, "I", IVL_SIP_INPUT);
      return cell;
}

edif_cell_t xilinx_cell_ibuf(edif_xlibrary_t xlib)
{
      static edif_cell_t cell = 0;
      if (cell) return cell;

      cell = edif_xcell_create(xlib, "IBUF", 2);
      edif_cell_portconfig(cell, BUF_O, "O", IVL_SIP_OUTPUT);
      edif_cell_portconfig(cell, BUF_I, "I", IVL_SIP_INPUT);
      return cell;
}

edif_cell_t xilinx_cell_inv(edif_xlibrary_t xlib)
{
      static edif_cell_t cell = 0;
      if (cell) return cell;

      cell = edif_xcell_create(xlib, "INV", 2);
      edif_cell_portconfig(cell, BUF_O, "O", IVL_SIP_OUTPUT);
      edif_cell_portconfig(cell, BUF_I, "I", IVL_SIP_INPUT);
      return cell;
}

edif_cell_t xilinx_cell_obuf(edif_xlibrary_t xlib)
{
      static edif_cell_t cell = 0;
      if (cell) return cell;

      cell = edif_xcell_create(xlib, "OBUF", 2);
      edif_cell_portconfig(cell, BUF_O, "O", IVL_SIP_OUTPUT);
      edif_cell_portconfig(cell, BUF_I, "I", IVL_SIP_INPUT);
      return cell;
}


edif_cell_t xilinx_cell_lut2(edif_xlibrary_t xlib)
{
      static edif_cell_t cell = 0;
      if (cell != 0) return cell;

      cell = edif_xcell_create(xlib, "LUT2", 3);
      edif_cell_portconfig(cell, LUT_O,  "O",  IVL_SIP_OUTPUT);
      edif_cell_portconfig(cell, LUT_I0, "I0", IVL_SIP_INPUT);
      edif_cell_portconfig(cell, LUT_I1, "I1", IVL_SIP_INPUT);
      return cell;
}

edif_cell_t xilinx_cell_lut3(edif_xlibrary_t xlib)
{
      static edif_cell_t cell = 0;
      if (cell != 0) return cell;

      cell = edif_xcell_create(xlib, "LUT3", 4);
      edif_cell_portconfig(cell, LUT_O,  "O",  IVL_SIP_OUTPUT);
      edif_cell_portconfig(cell, LUT_I0, "I0", IVL_SIP_INPUT);
      edif_cell_portconfig(cell, LUT_I1, "I1", IVL_SIP_INPUT);
      edif_cell_portconfig(cell, LUT_I2, "I2", IVL_SIP_INPUT);
      return cell;
}

edif_cell_t xilinx_cell_lut4(edif_xlibrary_t xlib)
{
      static edif_cell_t cell = 0;
      if (cell != 0) return cell;

      cell = edif_xcell_create(xlib, "LUT4", 5);
      edif_cell_portconfig(cell, LUT_O,  "O",  IVL_SIP_OUTPUT);
      edif_cell_portconfig(cell, LUT_I0, "I0", IVL_SIP_INPUT);
      edif_cell_portconfig(cell, LUT_I1, "I1", IVL_SIP_INPUT);
      edif_cell_portconfig(cell, LUT_I2, "I2", IVL_SIP_INPUT);
      edif_cell_portconfig(cell, LUT_I3, "I3", IVL_SIP_INPUT);
      return cell;
}


edif_cell_t xilinx_cell_fdce(edif_xlibrary_t xlib)
{
      static edif_cell_t cell = 0;
      if (cell != 0) return cell;

      cell = edif_xcell_create(xlib, "FDCE", 5);
      edif_cell_portconfig(cell, FDCE_Q,  "Q",   IVL_SIP_INPUT);
      edif_cell_portconfig(cell, FDCE_D,  "D",   IVL_SIP_OUTPUT);
      edif_cell_portconfig(cell, FDCE_C,  "C",   IVL_SIP_INPUT);
      edif_cell_portconfig(cell, FDCE_CE, "CE",  IVL_SIP_INPUT);
      edif_cell_portconfig(cell, FDCE_CLR,"CLR", IVL_SIP_INPUT);
      return cell;
}

edif_cell_t xilinx_cell_fdcpe(edif_xlibrary_t xlib)
{
      static edif_cell_t cell = 0;
      if (cell != 0) return cell;

      cell = edif_xcell_create(xlib, "FDCPE", 6);
      edif_cell_portconfig(cell, FDCE_Q,  "Q",   IVL_SIP_INPUT);
      edif_cell_portconfig(cell, FDCE_D,  "D",   IVL_SIP_OUTPUT);
      edif_cell_portconfig(cell, FDCE_C,  "C",   IVL_SIP_INPUT);
      edif_cell_portconfig(cell, FDCE_CE, "CE",  IVL_SIP_INPUT);
      edif_cell_portconfig(cell, FDCE_CLR,"CLR", IVL_SIP_INPUT);
      edif_cell_portconfig(cell, FDCE_PRE,"PRE", IVL_SIP_INPUT);
      return cell;
}


edif_cell_t xilinx_cell_mult_and(edif_xlibrary_t xlib)
{
      static edif_cell_t cell = 0;
      if (cell != 0) return cell;

      cell = edif_xcell_create(xlib, "MULT_AND", 3);
      edif_cell_portconfig(cell, MULT_AND_LO, "LO", IVL_SIP_OUTPUT);
      edif_cell_portconfig(cell, MULT_AND_I0, "I0", IVL_SIP_INPUT);
      edif_cell_portconfig(cell, MULT_AND_I1, "I1", IVL_SIP_INPUT);
      return cell;
}

edif_cell_t xilinx_cell_muxcy(edif_xlibrary_t xlib)
{
      static edif_cell_t cell = 0;
      if (cell != 0) return cell;

      cell = edif_xcell_create(xlib, "MUXCY", 4);
      edif_cell_portconfig(cell, MUXCY_O,  "O",  IVL_SIP_OUTPUT);
      edif_cell_portconfig(cell, MUXCY_DI, "DI", IVL_SIP_INPUT);
      edif_cell_portconfig(cell, MUXCY_CI, "CI", IVL_SIP_INPUT);
      edif_cell_portconfig(cell, MUXCY_S,  "S",  IVL_SIP_INPUT);
      return cell;
}

edif_cell_t xilinx_cell_muxcy_l(edif_xlibrary_t xlib)
{
      static edif_cell_t cell = 0;
      if (cell != 0) return cell;

      cell = edif_xcell_create(xlib, "MUXCY_L", 4);
      edif_cell_portconfig(cell, MUXCY_O,  "LO", IVL_SIP_OUTPUT);
      edif_cell_portconfig(cell, MUXCY_DI, "DI", IVL_SIP_INPUT);
      edif_cell_portconfig(cell, MUXCY_CI, "CI", IVL_SIP_INPUT);
      edif_cell_portconfig(cell, MUXCY_S,  "S",  IVL_SIP_INPUT);
      return cell;
}

edif_cell_t xilinx_cell_xorcy(edif_xlibrary_t xlib)
{
      static edif_cell_t cell = 0;
      if (cell != 0) return cell;

      cell = edif_xcell_create(xlib, "XORCY", 3);
      edif_cell_portconfig(cell, XORCY_O,  "O",  IVL_SIP_OUTPUT);
      edif_cell_portconfig(cell, XORCY_CI, "CI", IVL_SIP_INPUT);
      edif_cell_portconfig(cell, XORCY_LI, "LI", IVL_SIP_INPUT);
      return cell;
}



/*
 * $Log: xilinx.c,v $
 * Revision 1.1  2003/04/05 05:53:34  steve
 *  Move library cell management to common file.
 *
 */

