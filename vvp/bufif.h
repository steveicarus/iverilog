#ifndef __bufif_H
#define __bufif_H
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
#ident "$Id: bufif.h,v 1.6 2002/09/06 04:56:29 steve Exp $"
#endif

# include  "functor.h"

class vvp_bufif_s  : public functor_s {

    public:
      vvp_bufif_s(bool en_invert, bool out_invert,
		  unsigned str0, unsigned str1);

      virtual void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);

    private:
      unsigned pol_ : 1;
      unsigned inv_ : 1;
};

/*
 * $Log: bufif.h,v $
 * Revision 1.6  2002/09/06 04:56:29  steve
 *  Add support for %v is the display system task.
 *  Change the encoding of H and L outputs from
 *  the bufif devices so that they are logic x.
 *
 * Revision 1.5  2002/08/12 01:35:07  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.4  2002/07/05 20:08:44  steve
 *  Count different types of functors.
 *
 * Revision 1.3  2001/12/14 06:03:17  steve
 *  Arrange bufif to support notif as well.
 *
 * Revision 1.2  2001/10/31 04:27:46  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.1  2001/05/31 04:12:43  steve
 *  Make the bufif0 and bufif1 gates strength aware,
 *  and accurately propagate strengths of outputs.
 *
 */
#endif
