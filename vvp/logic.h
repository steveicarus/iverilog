#ifndef __logic_H
#define __logic_H
/*
 * Copyright (c) 2000-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: logic.h,v 1.8 2004/12/11 02:31:29 steve Exp $"
#endif

# include  "vvp_net.h"
# include  <stddef.h>

/*
 * Table driven functor. This kind of node takes 4 inputs and
 * generates a single output. The logic is bitwise, and implemented
 * with a lookup table.
 */

class table_functor_s: public vvp_net_fun_t {

    public:
      typedef const unsigned char *truth_t;
      explicit table_functor_s(truth_t t);
      virtual ~table_functor_s();

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
 * Revision 1.8  2004/12/11 02:31:29  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 */
#endif // __logic_H
