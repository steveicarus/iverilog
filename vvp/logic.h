#ifndef __logic_H
#define __logic_H
/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: logic.h,v 1.7 2003/07/30 01:13:29 steve Exp $"
#endif

# include  "functor.h"
# include  <stddef.h>

/*
 *  Table driven functor.  oval = table[ival];
 */

class table_functor_s: public functor_s {

    public:
      typedef const unsigned char *truth_t;
      explicit table_functor_s(truth_t t, unsigned str0 =6, unsigned str1 =6);
      virtual ~table_functor_s();

      virtual void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);

    private:
      truth_t table;
};

// table functor types

extern const unsigned char ft_AND[];
extern const unsigned char ft_BUF[];
extern const unsigned char ft_BUFIF0[];
extern const unsigned char ft_BUFIF1[];
extern const unsigned char ft_BUFZ[];
extern const unsigned char ft_PMOS[];
extern const unsigned char ft_NMOS[];
extern const unsigned char ft_MUXX[];
extern const unsigned char ft_MUXZ[];
extern const unsigned char ft_EEQ[];
extern const unsigned char ft_NAND[];
extern const unsigned char ft_NOR[];
extern const unsigned char ft_NOT[];
extern const unsigned char ft_OR[];
extern const unsigned char ft_TRIAND[];
extern const unsigned char ft_TRIOR[];
extern const unsigned char ft_XNOR[];
extern const unsigned char ft_XOR[];
extern const unsigned char ft_var[];

/*
 * $Log: logic.h,v $
 * Revision 1.7  2003/07/30 01:13:29  steve
 *  Add support for triand and trior.
 *
 * Revision 1.6  2002/08/29 03:04:01  steve
 *  Generate x out for x select on wide muxes.
 *
 * Revision 1.5  2002/08/12 01:35:08  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.4  2002/07/05 20:08:44  steve
 *  Count different types of functors.
 *
 * Revision 1.3  2002/01/12 04:02:16  steve
 *  Support the BUFZ logic device.
 *
 * Revision 1.2  2001/12/14 02:04:49  steve
 *  Support strength syntax on functors.
 *
 * Revision 1.1  2001/11/06 03:07:22  steve
 *  Code rearrange. (Stephan Boettcher)
 *
 */
#endif // __logic_H
