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
#ifdef HAVE_CVS_IDENT
#ident "$Id: resolv.h,v 1.8 2003/03/13 04:36:57 steve Exp $"
#endif

# include  "config.h"
# include  "functor.h"

/*
 * This functor type resolves its inputs using the verilog method of
 * combining signals, and outputs that resolved value. The puller
 * value is also blended with the result. This helps with the
 * implementation of tri0 and tri1, which have pull constants attached.
 */
class resolv_functor_s: public functor_s {

    public:
      explicit resolv_functor_s(unsigned char hiz_value);
      ~resolv_functor_s();

      virtual void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);


    private:
      unsigned char istr[4];
      unsigned char hiz_;
};

/*
 * $Log: resolv.h,v $
 * Revision 1.8  2003/03/13 04:36:57  steve
 *  Remove the obsolete functor delete functions.
 *
 * Revision 1.7  2002/08/12 01:35:08  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.6  2001/12/18 05:32:11  steve
 *  Improved functor debug dumps.
 *
 * Revision 1.5  2001/12/15 02:11:51  steve
 *  Give tri0 and tri1 their proper strengths.
 *
 * Revision 1.4  2001/12/15 01:54:39  steve
 *  Support tri0 and tri1 resolvers.
 *
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
