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
#ident "$Id: a_fetch_type_str.c,v 1.5 2003/04/12 18:57:14 steve Exp $"
#endif

#include  <assert.h>
#include  <vpi_user.h>
#include  <acc_user.h>


char* acc_fetch_type_str(PLI_INT32 type)
{
      switch (type) {
	  case accNet:
	    return "accNet";

	  case accReg:
	    return "accReg";

	  case accParameter:
	    return "accParameter";

	  case accConstant:
	    return "accConstant";

	  default:
	    vpi_printf("XXXX acc_fetch_type_str(%d);\n", type);
	    return "acc_fetch_type_str(unknown)";
      }

      return "";
}

/*
 * $Log: a_fetch_type_str.c,v $
 * Revision 1.5  2003/04/12 18:57:14  steve
 *  More acc_ function stubs.
 *
 * Revision 1.4  2003/03/13 05:07:46  steve
 *  Declaration warnings.
 *
 * Revision 1.3  2003/03/13 04:35:09  steve
 *  Add a bunch of new acc_ and tf_ functions.
 *
 * Revision 1.2  2003/02/19 04:37:04  steve
 *  fullname for accConstant.
 *
 * Revision 1.1  2003/02/17 06:39:47  steve
 *  Add at least minimal implementations for several
 *  acc_ functions. Add support for standard ACC
 *  string handling.
 *
 *  Add the _pli_types.h header file to carry the
 *  IEEE1364-2001 standard PLI type declarations.
 *
 */

