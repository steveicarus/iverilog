#ifndef __resolv_H
#define __resolv_H
/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: resolv.h,v 1.3 2001/10/31 04:27:47 steve Exp $"
#endif

# include  "functor.h"

class resolv_functor_s: public functor_s {

    public:
      resolv_functor_s() { istr[0]=istr[1]=istr[2]=istr[3]=StX; }
      virtual void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
      unsigned char istr[4];
};

/*
 * $Log: resolv.h,v $
 * Revision 1.3  2001/10/31 04:27:47  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.2  2001/05/12 01:48:57  steve
 *  Silly copyright typo.
 *
 * Revision 1.1  2001/05/09 02:53:53  steve
 *  Implement the .resolv syntax.
 *
 */
#endif
